#pragma once

#include "../llama.cpp/include/llama.h"
#include <vector>
#include <string>
#include <utility>

namespace fints {

// Configuration for FINTs steering
struct Config {
    float scale = 2.0f;           // Steering strength multiplier
    int start_layer = 17;         // First layer to apply steering  
    int end_layer = 26;           // Last layer to apply steering
    int top_k = 5;                // Number of top vectors for input-aware mode
    bool enabled = false;         // Enable/disable steering
    bool input_aware = true;      // Use input-aware mode (similarity-based selection)
};

// A single contrastive pair for steering vector extraction
struct ContrastivePair {
    std::string query;            // The input query/context
    std::string positive;         // Desired response style
    std::string negative;         // Undesired response style
};

// Steering vectors for all layers (compact format)
struct SteeringVectors {
    std::vector<float> attn;      // Attention steering vectors [num_layers * n_embd]
    std::vector<float> mlp;       // MLP steering vectors [num_layers * n_embd]
    std::vector<float> query_embedding;  // Query embedding for input-aware mode
    std::string query;            // Original query text
};

// Extract steering vectors from contrastive pairs
std::vector<SteeringVectors> extract_steering_vectors(
    llama_model* model,
    llama_context* ctx,
    const std::vector<ContrastivePair>& pairs,
    const Config& config
);

// Apply steering vectors to context
bool apply_steering(
    llama_context* ctx,
    const std::vector<SteeringVectors>& vectors,
    const Config& config,
    const std::vector<float>& input_embedding = {}
);

// Clear all steering
void clear_steering(llama_context* ctx, const Config& config);

// Generate text with optional steering
// Returns pair of (response, token_count)
std::pair<std::string, int> generate_with_steering(
    llama_model* model,
    llama_context* ctx,
    const std::string& prompt,
    const Config& config,
    const std::vector<SteeringVectors>& vectors = {}
);

// Build chat-formatted prompt
std::string build_chat_prompt(const std::string& user_input);

// Compute cosine similarity between two vectors
float cosine_similarity(const std::vector<float>& a, const std::vector<float>& b);

// Extract embedding from activations (for input-aware mode)
std::vector<float> extract_embedding_from_activations(
    llama_context* ctx,
    int start_layer,
    int end_layer
);

} // namespace fints
