# How to Include/Use FINTs in Your Code

## Option 1: Use the Built Library (Recommended)

### In Your C++ File:

```cpp
#include "fints.h"

int main() {
    // 1. Initialize llama backend
    llama_backend_init();
    
    // 2. Load your model
    llama_model_params model_params = llama_model_default_params();
    llama_model* model = llama_model_load_from_file("path/to/model.gguf", model_params);
    
    // 3. Create context
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = 2048;
    llama_context* ctx = llama_init_from_model(model, ctx_params);
    
    // 4. Define contrastive pairs (your style examples)
    std::vector<fints::ContrastivePair> pairs = {
        {"Query", "Positive style example", "Negative style example"},
        // Add more pairs...
    };
    
    // 5. Extract steering vectors
    auto vectors = fints::extract_steering_vectors(model, ctx, pairs);
    
    // 6. Configure and apply steering
    fints::Config config;
    config.enabled = true;
    config.scale = 2.0f;      // Steering strength (1.0-5.0)
    config.start_layer = 17;  // First layer to steer
    config.end_layer = 26;    // Last layer to steer
    
    fints::apply_steering(ctx, vectors, config);
    
    // 7. Generate with steering
    auto [response, tokens] = fints::generate_with_steering(
        model, ctx, "Your prompt here", config
    );
    
    std::cout << "Response: " << response << "\n";
    std::cout << "Tokens: " << tokens << "\n";
    
    // 8. Cleanup
    fints::clear_steering(ctx);
    llama_free(ctx);
    llama_model_free(model);
    llama_backend_free();
    
    return 0;
}
```

### Compile Your Code:

```bash
# Add to your CMakeLists.txt or compile command
g++ -std=c++17 my_app.cpp \
    -I/path/to/fints-demo/src \
    -I/path/to/llama.cpp/include \
    -I/path/to/llama.cpp/ggml/include \
    -L/path/to/fints-demo/build \
    -L/path/to/llama.cpp/build/bin \
    -lfints -lllama -lggml \
    -framework Accelerate -pthread \
    -o my_app
```

### Or with CMake:

```cmake
# In your CMakeLists.txt
find_library(FINTS_LIB fints PATHS /path/to/fints-demo/build)
find_library(LLAMA_LIB llama PATHS /path/to/llama.cpp/build/bin)
find_library(GGML_LIB ggml PATHS /path/to/llama.cpp/build/bin)

add_executable(my_app my_app.cpp)
target_include_directories(my_app PRIVATE
    /path/to/fints-demo/src
    /path/to/llama.cpp/include
    /path/to/llama.cpp/ggml/include
)
target_link_libraries(my_app
    ${FINTS_LIB}
    ${LLAMA_LIB}
    ${GGML_LIB}
    "-framework Accelerate"
    pthread
)
```

---

## Option 2: Copy Source Files

If you want to bundle FINTs directly in your project:

### Copy Files:
```bash
cp fints-demo/src/fints.h your-project/include/
cp fints-demo/src/fints.cpp your-project/src/
```

### Include in Your Build:
```cmake
add_executable(your_app
    src/main.cpp
    src/fints.cpp  # Add this
)
```

---

## Option 3: As a Submodule (If Using Git)

```bash
cd your-project
git submodule add /path/to/fints-demo external/fints
```

Then in your CMakeLists.txt:
```cmake
add_subdirectory(external/fints)
target_link_libraries(your_app fints)
```

---

## Quick API Reference

### Core Functions:

```cpp
// Extract steering vectors from examples
std::vector<fints::LayerVectors> fints::extract_steering_vectors(
    llama_model* model,
    llama_context* ctx,
    const std::vector<ContrastivePair>& pairs
);

// Apply steering to context
bool fints::apply_steering(
    llama_context* ctx,
    const std::vector<LayerVectors>& vectors,
    const Config& config
);

// Generate with steering
std::pair<std::string, int> fints::generate_with_steering(
    llama_model* model,
    llama_context* ctx,
    const std::string& prompt,
    const Config& config
);

// Clear steering
void fints::clear_steering(llama_context* ctx);
```

### Config Options:

```cpp
fints::Config config;
config.enabled = true;        // Enable/disable steering
config.scale = 2.0f;         // Strength (0.5-5.0)
config.start_layer = 17;     // First layer (0-31)
config.end_layer = 26;       // Last layer (0-31)
config.top_k = 5;            // Top-K vector selection
config.input_aware = true;   // Single-pass optimization
```

---

## Example: Custom Style Steering

```cpp
// Steer toward formal, technical writing
std::vector<fints::ContrastivePair> tech_style = {
    {
        "Explain this concept",
        "The algorithm leverages dynamic programming for optimal substructure.",
        "It's like a really smart way to solve problems by breaking them down!"
    },
    {
        "Describe the process",
        "The system employs a three-stage pipeline for data transformation.",
        "So basically it does stuff in three steps to change the data."
    }
};

auto vectors = fints::extract_steering_vectors(model, ctx, tech_style);
fints::apply_steering(ctx, vectors, config);
// Now all generations will be more formal/technical
```

---

## Troubleshooting

**"fints.h not found":**
- Check include paths: `-I/path/to/fints-demo/src`

**"undefined reference to fints::":**
- Check library path: `-L/path/to/fints-demo/build`
- Link library: `-lfints`
- Make sure you built the library first: `cd fints-demo && ./scripts/setup.sh`

**"Cannot find -lfints":**
- Build the library: `cd fints-demo/build && cmake --build .`

Need help with a specific integration? Let me know your setup!
