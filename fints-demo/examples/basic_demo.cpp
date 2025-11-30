#include "../src/fints.h"
#include <iostream>
#include <chrono>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <model_path> [prompt]\n";
        return 1;
    }
    
    std::string model_path = argv[1];
    std::string user_prompt = argc >= 3 ? argv[2] : "Tell me a short story about a robot";
    
    std::cout << "======================================\n";
    std::cout << "🚀 FINTs Demo\n";
    std::cout << "======================================\n\n";
    
    // Initialize
    llama_backend_init();
    
    // Load model
    std::cout << "Loading model...\n";
    llama_model_params model_params = llama_model_default_params();
    llama_model* model = llama_model_load_from_file(model_path.c_str(), model_params);
    if (!model) {
        std::cerr << "Failed to load model\n";
        return 1;
    }
    std::cout << "✓ Model loaded\n\n";
    
    // Create context
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048;
    llama_context* ctx = llama_init_from_model(model, ctx_params);
    if (!ctx) {
        std::cerr << "Failed to create context\n";
        llama_model_free(model);
        return 1;
    }
    std::cout << "✓ Context created\n\n";
    
    // Define contrastive pairs
    std::vector<fints::ContrastivePair> pairs = {
        {"Explain AI", "AI leverages neural networks for pattern recognition.", "AI is like computers thinking!"},
        {"Tell me about space", "The cosmos extends billions of light-years.", "Space is super big and has lots of stars!"}
    };
    
    // Test WITHOUT FINTs
    std::cout << "Testing without FINTs...\n";
    fints::Config config1;
    config1.enabled = false;
    
    auto start1 = std::chrono::high_resolution_clock::now();
    auto [response1, tokens1] = fints::generate_with_steering(model, ctx, user_prompt, config1);
    auto end1 = std::chrono::high_resolution_clock::now();
    auto dur1 = std::chrono::duration_cast<std::chrono::milliseconds>(end1 - start1).count();
    
    std::cout << "Response: " << response1 << "\n";
    std::cout << "Time: " << dur1 << "ms, Tokens: " << tokens1 << "\n\n";
    
    // Test WITH FINTs
    std::cout << "Extracting steering vectors...\n";
    fints::Config config2;
    config2.enabled = true;
    config2.scale = 2.0f;
    config2.start_layer = 17;
    config2.end_layer = 26;
    
    auto vectors = fints::extract_steering_vectors(model, ctx, pairs, config2);
    
    std::cout << "\nTesting with FINTs...\n";
    auto start2 = std::chrono::high_resolution_clock::now();
    auto [response2, tokens2] = fints::generate_with_steering(model, ctx, user_prompt, config2, vectors);
    auto end2 = std::chrono::high_resolution_clock::now();
    auto dur2 = std::chrono::duration_cast<std::chrono::milliseconds>(end2 - start2).count();
    
    std::cout << "Response: " << response2 << "\n";
    std::cout << "Time: " << dur2 << "ms, Tokens: " << tokens2 << "\n\n";
    
    // Cleanup
    fints::clear_steering(ctx, config2);
    llama_free(ctx);
    llama_model_free(model);
    llama_backend_free();
    
    std::cout << "✓ Demo complete!\n";
    return 0;
}
