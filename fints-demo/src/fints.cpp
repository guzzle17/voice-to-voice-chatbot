#include "fints.h"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace fints {

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b) {
    if (a.size() != b.size() || a.empty()) return 0.0f;
    
    float dot = 0.0f, norm_a = 0.0f, norm_b = 0.0f;
    for (size_t i = 0; i < a.size(); i++) {
        dot += a[i] * b[i];
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }
    
    if (norm_a == 0.0f || norm_b == 0.0f) return 0.0f;
    return dot / (std::sqrt(norm_a) * std::sqrt(norm_b));
}

std::string build_chat_prompt(const std::string& user_input) {
    return "<|im_start|>system\nYou are a friendly assistant.<|im_end|>\n<|im_start|>user\n" + 
           user_input + "<|im_end|>\n<|im_start|>assistant\n";
}

std::vector<float> extract_embedding_from_activations(
    llama_context* ctx,
    int start_layer,
    int end_layer
) {
    int32_t n_layer, n_embd;
    llama_get_activation_info(ctx, &n_layer, &n_embd);
    
    int num_layers = end_layer - start_layer + 1;
    std::vector<float> embedding(n_embd, 0.0f);
    
    for (int layer = start_layer; layer <= end_layer; layer++) {
        std::vector<float> layer_attn(n_embd);
        std::vector<float> layer_mlp(n_embd);
        
        llama_get_layer_activations(ctx, layer, layer_attn.data(), layer_mlp.data(), n_embd * sizeof(float));
        
        for (int i = 0; i < n_embd; i++) {
            embedding[i] += (layer_attn[i] + layer_mlp[i]) / 2.0f;
        }
    }
    
    for (int i = 0; i < n_embd; i++) {
        embedding[i] /= num_layers;
    }
    
    return embedding;
}

std::vector<SteeringVectors> extract_steering_vectors(
    llama_model* model,
    llama_context* ctx,
    const std::vector<ContrastivePair>& pairs,
    const Config& config
) {
    std::cout << "\n🧠 Extracting Steering Vectors...\n";
    std::cout << "   Pairs: " << pairs.size() << "\n";
    
    const llama_vocab* vocab = llama_model_get_vocab(model);
    
    // Create temporary context for extraction
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2024;   // Increased from 512 to handle longer pairs
    ctx_params.n_batch = 2024;  // Match n_ctx
    
    llama_context* temp_ctx = llama_init_from_model(model, ctx_params);
    if (!temp_ctx) {
        std::cerr << "Failed to create extraction context\n";
        return {};
    }
    
    // Suppress verbose llama.cpp logs during extraction
    llama_log_set([](ggml_log_level, const char *, void *) {}, nullptr);
    
    llama_set_activation_extraction(temp_ctx, true);
    
    int32_t n_layer, n_embd;
    llama_get_activation_info(temp_ctx, &n_layer, &n_embd);
    
    int num_layers = config.end_layer - config.start_layer + 1;
    size_t vec_size = num_layers * n_embd;
    
    std::vector<SteeringVectors> result;
    
    for (const auto& pair : pairs) {
        // Build contrastive examples
        std::string x_pos = "User: " + pair.query + "\nAssistant: " + pair.positive;
        std::string x_neg = "User: " + pair.query + "\nAssistant: " + pair.negative;
        
        // Tokenize positive
        std::vector<llama_token> pos_tokens(512);
        int n_pos = llama_tokenize(vocab, x_pos.c_str(), x_pos.length(),
                                   pos_tokens.data(), pos_tokens.size(), true, false);
        if (n_pos < 0) {
            pos_tokens.resize(-n_pos);
            n_pos = llama_tokenize(vocab, x_pos.c_str(), x_pos.length(),
                                  pos_tokens.data(), pos_tokens.size(), true, false);
        }
if (n_pos > 512) n_pos = 512;
        
        // Decode positive
        llama_batch pos_batch = llama_batch_get_one(pos_tokens.data(), n_pos);
        if (llama_decode(temp_ctx, pos_batch) != 0) continue;
        
        // Extract positive activations
        std::vector<float> pos_attn(vec_size);
        std::vector<float> pos_mlp(vec_size);
        
        for (int layer = config.start_layer; layer <= config.end_layer; layer++) {
            std::vector<float> layer_attn(n_embd);
            std::vector<float> layer_mlp(n_embd);
            
            llama_get_layer_activations(temp_ctx, layer,
                                        layer_attn.data(),
                                        layer_mlp.data(),
                                        n_embd * sizeof(float));
            
            int layer_idx = layer - config.start_layer;
            for (int i = 0; i < n_embd; i++) {
                pos_attn[layer_idx * n_embd + i] = layer_attn[i];
                pos_mlp[layer_idx * n_embd + i] = layer_mlp[i];
            }
        }
        
        // Tokenize negative
        std::vector<llama_token> neg_tokens(512);
        int n_neg = llama_tokenize(vocab, x_neg.c_str(), x_neg.length(),
                                   neg_tokens.data(), neg_tokens.size(), true, false);
        if (n_neg < 0) {
            neg_tokens.resize(-n_neg);
            n_neg = llama_tokenize(vocab, x_neg.c_str(), x_neg.length(),
                                  neg_tokens.data(), neg_tokens.size(), true, false);
        }
        if (n_neg > 512) n_neg = 512;
        
        // Decode negative
        llama_batch neg_batch = llama_batch_get_one(neg_tokens.data(), n_neg);
        if (llama_decode(temp_ctx, neg_batch) != 0) continue;
        
        // Extract negative activations
        std::vector<float> neg_attn(vec_size);
        std::vector<float> neg_mlp(vec_size);
        
        for (int layer = config.start_layer; layer <= config.end_layer; layer++) {
            std::vector<float> layer_attn(n_embd);
            std::vector<float> layer_mlp(n_embd);
            
            llama_get_layer_activations(temp_ctx, layer,
                                        layer_attn.data(),
                                        layer_mlp.data(),
                                        n_embd * sizeof(float));
            
            int layer_idx = layer - config.start_layer;
            for (int i = 0; i < n_embd; i++) {
                neg_attn[layer_idx * n_embd + i] = layer_attn[i];
                neg_mlp[layer_idx * n_embd + i] = layer_mlp[i];
            }
        }
        
        // Compute steering vectors (positive - negative)
        SteeringVectors sv;
        sv.attn.resize(vec_size);
        sv.mlp.resize(vec_size);
        sv.query = pair.query;
        
        for (size_t i = 0; i < vec_size; i++) {
            sv.attn[i] = pos_attn[i] - neg_attn[i];
            sv.mlp[i] = pos_mlp[i] - neg_mlp[i];
        }
        
        // Compute query embedding if input-aware
        if (config.input_aware) {
            std::vector<llama_token> query_tokens(512);
            int n_query = llama_tokenize(vocab, pair.query.c_str(), pair.query.length(),
                                        query_tokens.data(), query_tokens.size(), true, false);
            if (n_query > 0) {
                llama_batch query_batch = llama_batch_get_one(query_tokens.data(), std::min(n_query, 512));
                if (llama_decode(temp_ctx, query_batch) == 0) {
                    sv.query_embedding = extract_embedding_from_activations(temp_ctx, config.start_layer, config.end_layer);
                }
            }
        }
        
        result.push_back(sv);
    }
    
    llama_free(temp_ctx);
    std::cout << "   ✅ Extracted " << result.size() << " steering vectors\n";
    
    return result;
}

bool apply_steering(
    llama_context* ctx,
    const std::vector<SteeringVectors>& vectors,
    const Config& config,
    const std::vector<float>& input_embedding
) {
    if (vectors.empty()) return false;
    
    int32_t n_layer, n_embd;
    llama_get_activation_info(ctx, &n_layer, &n_embd);
    
    int num_layers = config.end_layer - config.start_layer + 1;
    size_t vec_size = num_layers * n_embd;
    
    std::vector<float> agg_attn(vec_size, 0.0f);
    std::vector<float> agg_mlp(vec_size, 0.0f);
    
    std::vector<size_t> selected;
    
    // Select vectors (input-aware or all)
    if (config.input_aware && !input_embedding.empty()) {
        std::vector<std::pair<float, size_t>> sims;
        for (size_t i = 0; i < vectors.size(); i++) {
            if (!vectors[i].query_embedding.empty()) {
                float sim = cosine_similarity(input_embedding, vectors[i].query_embedding);
                sims.push_back({sim, i});
            }
        }
        std::sort(sims.begin(), sims.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
        
        int k = std::min(config.top_k, (int)sims.size());
        for (int i = 0; i < k; i++) {
            selected.push_back(sims[i].second);
        }
    } else {
        for (size_t i = 0; i < vectors.size(); i++) {
            selected.push_back(i);
        }
    }
    
    // Aggregate
    for (size_t idx : selected) {
        for (size_t i = 0; i < vec_size; i++) {
            agg_attn[i] += vectors[idx].attn[i];
            agg_mlp[i] += vectors[idx].mlp[i];
        }
    }
    
    float n = selected.size();
    for (size_t i = 0; i < vec_size; i++) {
        agg_attn[i] = (agg_attn[i] / n) * config.scale;
        agg_mlp[i] = (agg_mlp[i] / n) * config.scale;
    }
    
    // Create full-size buffers (llama.cpp expects layers 1-31)
    const int total_layers = 32;
    size_t full_size = (total_layers - 1) * n_embd;
    std::vector<float> full_attn(full_size, 0.0f);
    std::vector<float> full_mlp(full_size, 0.0f);
    
    // Copy to correct positions
    for (int i = 0; i < num_layers; i++) {
        int actual_layer = config.start_layer + i;
        size_t src_offset = i * n_embd;
        size_t dst_offset = (actual_layer - 1) * n_embd;
        
        for (int j = 0; j < n_embd; j++) {
            full_attn[dst_offset + j] = agg_attn[src_offset + j];
            full_mlp[dst_offset + j] = agg_mlp[src_offset + j];
        }
    }
    
    // Apply
    int result = llama_apply_adapter_cvec_fints(ctx,
                                                 full_attn.data(),
                                                 full_mlp.data(),
                                                 full_size * sizeof(float),
                                                 n_embd,
                                                 config.start_layer,
                                                 config.end_layer);
    
    return result == 0;
}

void clear_steering(llama_context* ctx, const Config& config) {
    int32_t n_layer, n_embd;
    llama_get_activation_info(ctx, &n_layer, &n_embd);
    
    const int total_layers = 32;
    size_t full_size = (total_layers - 1) * n_embd;
    std::vector<float> zeros(full_size, 0.0f);
    
    llama_apply_adapter_cvec_fints(ctx, zeros.data(), zeros.data(), 
                                   full_size * sizeof(float), n_embd, 
                                   config.start_layer, config.end_layer);
}

std::pair<std::string, int> generate_with_steering(
    llama_model* model,
    llama_context* ctx,
    const std::string& prompt,
    const Config& config,
    const std::vector<SteeringVectors>& vectors
) {
    std::string full_prompt = build_chat_prompt(prompt);
    const llama_vocab* vocab = llama_model_get_vocab(model);
    
    // Tokenize
    std::vector<llama_token> tokens(2048);
    int n_tokens = llama_tokenize(vocab, full_prompt.c_str(), full_prompt.length(),
                                   tokens.data(), tokens.size(), true, false);
    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(vocab, full_prompt.c_str(), full_prompt.length(),
                                  tokens.data(), tokens.size(), true, false);
    }
    
    // Single-pass optimization: extract embedding during prompt decode
    std::vector<float> input_embedding;
    if (config.enabled && config.input_aware && !vectors.empty()) {
        llama_set_activation_extraction(ctx, true);
        
        llama_batch batch = llama_batch_get_one(tokens.data(), n_tokens);
        if (llama_decode(ctx, batch) != 0) {
            return {"Error: Failed to decode", 0};
        }
        
        input_embedding = extract_embedding_from_activations(ctx, config.start_layer, config.end_layer);
        llama_set_activation_extraction(ctx, false);
        
        // Apply steering with extracted embedding
        apply_steering(ctx, vectors, config, input_embedding);
    } else {
        // Normal decode
        llama_batch batch = llama_batch_get_one(tokens.data(), n_tokens);
        if (llama_decode(ctx, batch) != 0) {
            return {"Error: Failed to decode", 0};
        }
    }
    
    // Generate tokens
    llama_sampler_chain_params sampler_params = llama_sampler_chain_default_params();
    sampler_params.no_perf = true;
    llama_sampler* sampler = llama_sampler_chain_init(sampler_params);
    llama_sampler_chain_add(sampler, llama_sampler_init_min_p(0.05f, 1));
    llama_sampler_chain_add(sampler, llama_sampler_init_temp(0.3f));
    llama_sampler_chain_add(sampler, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
    
    std::string response;
    int generated_tokens = 0;
    
    for (int i = 0; i < 30; i++) {
        llama_token next_token = llama_sampler_sample(sampler, ctx, -1);
        if (llama_vocab_is_eog(vocab, next_token)) break;
        
        generated_tokens++;
        
        char buf[256];
        int n = llama_token_to_piece(vocab, next_token, buf, sizeof(buf), 0, false);
        if (n > 0) response += std::string(buf, n);
        
        llama_batch next_batch = llama_batch_get_one(&next_token, 1);
        if (llama_decode(ctx, next_batch) != 0) break;
    }
    
    llama_sampler_free(sampler);
    return {trim(response), generated_tokens};
}

} // namespace fints
