#include "fints.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <random>
#include <sstream>
#include <set>
#include <map>
#include <nlohmann/json.hpp>  // You may need to install this: brew install nlohmann-json

using json = nlohmann::json;

// Load LaMP-4 data
struct LaMP4Sample {
    std::string id;
    std::string input;
    std::string output;
    std::vector<std::pair<std::string, std::string>> profile;  // (title, text) pairs
};

std::vector<LaMP4Sample> load_lamp4_data(const std::string& dataset_dir, const std::string& split, int max_samples = -1) {
    std::vector<LaMP4Sample> samples;
    
    std::string questions_file = dataset_dir + "/" + split + "_questions.json";
    std::string outputs_file = dataset_dir + "/" + split + "_outputs.json";
    
    // Load questions
    std::ifstream qf(questions_file);
    if (!qf.is_open()) {
        std::cerr << "Error: Could not open " << questions_file << "\n";
        return samples;
    }
    
    json questions_json;
    qf >> questions_json;
    qf.close();
    
    // Load outputs
    std::map<std::string, std::string> output_map;
    std::ifstream of(outputs_file);
    if (of.is_open()) {
        json outputs_json;
        of >> outputs_json;
        of.close();
        
        if (outputs_json.contains("golds")) {
            for (const auto& item : outputs_json["golds"]) {
                output_map[item["id"]] = item["output"];
            }
        }
    }
    
    // Parse samples
    int count = 0;
    for (const auto& q : questions_json) {
        if (max_samples > 0 && count >= max_samples) break;
        
        LaMP4Sample sample;
        sample.id = q["id"];
        sample.input = q["input"];
        sample.output = output_map.count(sample.id) ? output_map[sample.id] : "";
        
        if (q.contains("profile")) {
            for (const auto& p : q["profile"]) {
                std::string title = p.contains("title") ? p["title"] : "";
                std::string text = p.contains("text") ? p["text"] : "";
                sample.profile.push_back({title, text});
            }
        }
        
        samples.push_back(sample);
        count++;
    }
    
    return samples;
}

// Build contrastive pairs from author profile
std::vector<fints::ContrastivePair> build_author_pairs(
    const LaMP4Sample& sample,
    const std::vector<LaMP4Sample>& other_samples,
    int max_pairs = 10
) {
    std::vector<fints::ContrastivePair> pairs;
    
    // Extract article from input
    std::string article = sample.input;
    size_t pos = article.find("Generate a headline for the following article: ");
    if (pos != std::string::npos) {
        article = article.substr(pos + 48);
    }
    article = article.substr(0, 400);  // Truncate for efficiency
    
    std::string query = "Generate only one headline:\n\n" + article + "\n\nHeadline:";
    
    // Use author's past headlines as positive examples
    std::random_device rd;
    std::mt19937 gen(rd());
    
    int pairs_created = 0;
    for (const auto& [title, text] : sample.profile) {
        if (title.empty() || pairs_created >= max_pairs) break;
        
        // Pick random negative from other samples
        if (other_samples.empty()) continue;
        std::uniform_int_distribution<> dis(0, other_samples.size() - 1);
        
        for (int tries = 0; tries < 5; tries++) {
            int rand_idx = dis(gen);
            const auto& other = other_samples[rand_idx];
            
            if (!other.profile.empty() && other.id != sample.id) {
                std::uniform_int_distribution<> profile_dis(0, other.profile.size() - 1);
                const auto& [neg_title, neg_text] = other.profile[profile_dis(gen)];
                
                if (!neg_title.empty() && neg_title != title) {
                    pairs.push_back({query, title, neg_title});
                    pairs_created++;
                    break;
                }
            }
        }
    }
    
    return pairs;
}

// Calculate simple ROUGE-1 F1
float calculate_rouge1(const std::string& reference, const std::string& hypothesis) {
    // Tokenize (simple split by space)
    auto tokenize = [](const std::string& s) {
        std::set<std::string> tokens;
        std::istringstream iss(s);
        std::string word;
        while (iss >> word) {
            // Simple lowercase
            for (auto& c : word) c = std::tolower(c);
            tokens.insert(word);
        }
        return tokens;
    };
    
    auto ref_tokens = tokenize(reference);
    auto hyp_tokens = tokenize(hypothesis);
    
    if (hyp_tokens.empty() || ref_tokens.empty()) return 0.0f;
    
    // Count overlap
    int overlap = 0;
    for (const auto& token : hyp_tokens) {
        if (ref_tokens.count(token)) overlap++;
    }
    
    float precision = static_cast<float>(overlap) / hyp_tokens.size();
    float recall = static_cast<float>(overlap) / ref_tokens.size();
    
    if (precision + recall == 0) return 0.0f;
    return 2 * precision * recall / (precision + recall);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <model_path> <lamp4_data_dir> [num_samples]\n";
        std::cerr << "Example: " << argv[0] << " models/model.gguf lamp_data/LaMP_4 5\n";
        return 1;
    }
    
    std::string model_path = argv[1];
    std::string data_dir = argv[2];
    int num_samples = argc >= 4 ? std::stoi(argv[3]) : 5;
    
    std::cout << "======================================\n";
    std::cout << "🚀 FINTs LaMP-4 Evaluation\n";
    std::cout << "======================================\n\n";
    
    // Initialize
    llama_backend_init();
    
    // Suppress verbose llama.cpp logs
    llama_log_set([](ggml_log_level /*level*/, const char * /*text*/, void * /*user_data*/) {
        // No operation - this effectively suppresses logs
    }, nullptr);
    
    // Load model
    std::cout << "📚 Loading model: " << model_path << "\n";
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = 0;  // Use CPU only to avoid GPU timeout during activation extraction
    llama_model* model = llama_model_load_from_file(model_path.c_str(), model_params);
    if (!model) {
        std::cerr << "Failed to load model\n";
        return 1;
    }
    std::cout << "✓ Model loaded\n\n";
    
    // Load LaMP-4 data
    std::cout << "📂 Loading LaMP-4 data...\n";
    auto test_samples = load_lamp4_data(data_dir, "dev", num_samples);
    auto train_samples = load_lamp4_data(data_dir, "train", 50);  // For negative examples
    
    if (test_samples.empty()) {
        std::cerr << "Failed to load test samples\n";
        return 1;
    }
    std::cout << "✓ Loaded " << test_samples.size() << " test samples\n";
    std::cout << "✓ Loaded " << train_samples.size() << " train samples (for negatives)\n\n";
    
    // Results tracking
    std::vector<float> baseline_rouge, fints_rouge;
    std::vector<long> baseline_latency, fints_latency;
    
    std::cout << "======================================\n";
    std::cout << "Running Evaluation\n";
    std::cout << "======================================\n\n";
    
    for (size_t i = 0; i < test_samples.size(); i++) {
        const auto& sample = test_samples[i];
        
        std::cout << "[" << (i+1) << "/" << test_samples.size() << "] Sample: " << sample.id << "\n";
        std::cout << "  Reference: " << sample.output.substr(0, 60) << "...\n";
        
        // Create fresh context for this sample to avoid KV cache contamination
        llama_log_set([](ggml_log_level, const char *, void *) {}, nullptr);
        llama_context_params ctx_params = llama_context_default_params();
        ctx_params.n_ctx = 2048;
        ctx_params.n_batch = 512;
        llama_context* ctx = llama_init_from_model(model, ctx_params);
        if (!ctx) {
            std::cerr << "Failed to create context for sample " << sample.id << "\n";
            continue;
        }
        
        // Extract article
        std::string article = sample.input;
        size_t pos = article.find("Generate a headline for the following article: ");
        if (pos != std::string::npos) {
            article = article.substr(pos + 48);
        }
        article = article.substr(0, 400);
        std::string prompt = "Generate only one headline:\n\n" + article + "\n\nHeadline:";
        
        // === BASELINE ===
        fints::Config config_baseline;
        config_baseline.enabled = false;
        
        auto start1 = std::chrono::high_resolution_clock::now();
        auto [baseline_output, baseline_tokens] = fints::generate_with_steering(model, ctx, prompt, config_baseline);
        auto end1 = std::chrono::high_resolution_clock::now();
        auto dur1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();
        
        float baseline_score = calculate_rouge1(sample.output, baseline_output);
        baseline_rouge.push_back(baseline_score);
        baseline_latency.push_back(dur1);
        
        std::cout << "  Baseline: " << baseline_output.substr(0, 60) << "...\n";
        std::cout << "    ROUGE-1: " << baseline_score << " | Latency: " << dur1 << "ms\n";
        
        // Free context and create fresh one for FINTs
        llama_free(ctx);
        llama_log_set([](ggml_log_level, const char *, void *) {}, nullptr);
        ctx = llama_init_from_model(model, ctx_params);
        if (!ctx) {
            std::cerr << "Failed to create context for FINTs\n";
            continue;
        }
        
        // === FINTs ===
        // Build author-specific pairs
        auto pairs = build_author_pairs(sample, train_samples, 10);
        std::cout << "  Building FINTs from " << pairs.size() << " pairs...\n";
        
        fints::Config config_fints;
    config_fints.enabled = true;
    config_fints.scale = 3.0f;
    config_fints.start_layer = 17;
    config_fints.end_layer = 26;
    
    auto vectors = fints::extract_steering_vectors(model, ctx, pairs, config_fints);
        

        
        auto start2 = std::chrono::high_resolution_clock::now();
        auto [fints_output, fints_tokens] = fints::generate_with_steering(model, ctx, prompt, config_fints, vectors);
        auto end2 = std::chrono::high_resolution_clock::now();
        auto dur2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();
        
        float fints_score = calculate_rouge1(sample.output, fints_output);
        fints_rouge.push_back(fints_score);
        fints_latency.push_back(dur2);
        
        std::cout << "  FINTs:    " << fints_output.substr(0, 60) << "...\n";
        std::cout << "    ROUGE-1: " << fints_score << " | Latency: " << dur2 << "ms\n";
        std::cout << "    Improvement: " << (fints_score - baseline_score > 0 ? "+" : "") 
                  << (fints_score - baseline_score) << "\n\n";
        
        // Free context after this sample
        llama_free(ctx);
    }
    
    // === RESULTS ===
    std::cout << "======================================\n";
    std::cout << "📊 Results\n";
    std::cout << "======================================\n\n";
    
    float avg_baseline = 0, avg_fints = 0;
    long avg_lat_baseline = 0, avg_lat_fints = 0;
    int improved = 0;
    
    for (size_t i = 0; i < baseline_rouge.size(); i++) {
        avg_baseline += baseline_rouge[i];
        avg_fints += fints_rouge[i];
        avg_lat_baseline += baseline_latency[i];
        avg_lat_fints += fints_latency[i];
        if (fints_rouge[i] > baseline_rouge[i]) improved++;
    }
    
    avg_baseline /= baseline_rouge.size();
    avg_fints /= fints_rouge.size();
    avg_lat_baseline /= baseline_latency.size();
    avg_lat_fints /= fints_latency.size();
    
    std::cout << "Samples: " << baseline_rouge.size() << "\n\n";
    std::cout << "🔵 Baseline ROUGE-1: " << avg_baseline << "\n";
    std::cout << "🟢 FINTs ROUGE-1:    " << avg_fints << "\n";
    std::cout << "📈 Improvement:      " << (avg_fints - avg_baseline > 0 ? "+" : "") 
              << (avg_fints - avg_baseline) << "\n";
    std::cout << "✅ Samples improved: " << improved << "/" << baseline_rouge.size() 
              << " (" << (100.0 * improved / baseline_rouge.size()) << "%)\n\n";
    std::cout << "⏱️  Avg latency (baseline): " << avg_lat_baseline << "ms\n";
    std::cout << "⏱️  Avg latency (FINTs):    " << avg_lat_fints << "ms\n";
    std::cout << "📊 Overhead: " << (avg_lat_fints - avg_lat_baseline) << "ms\n\n";
    
    // Cleanup
    llama_model_free(model);
    llama_backend_free();
    
    std::cout << "======================================\n";
    std::cout << "✓ Evaluation complete!\n";
    std::cout << "======================================\n";
    
    return 0;
}
