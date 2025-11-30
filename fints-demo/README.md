# FINTs Demo - Zero-Shot LLM Personalization

**Fast Input-aware No-Training Steering (FINTs)** - Dynamic LLM personalization without fine-tuning.

## 🚀 Quick Start (LaMP-4 Evaluation)

This demo evaluates FINTs on the LaMP-4 news headline generation benchmark with author-specific personalization.

### Prerequisites

- macOS or Linux
- C++17 compiler
- CMake 3.10+
- 8GB+ RAM

### Setup (3 steps)

```bash
# 1. Run setup (builds everything)
cd fints-demo
./scripts/setup.sh

# 2. Download LaMP-4 dataset
./scripts/download_lamp4.sh

# 3. Run evaluation (5 samples, ~10-15 min on CPU)
./scripts/run_lamp.sh \
  /path/to/your/model.gguf \
  lamp_data/LaMP_4 \
  5
```

**Example with specific model:**
```bash
./scripts/run_lamp.sh \
  ~/models/Llama-3.1-8B-Instruct-Q4_K_M.gguf \
  lamp_data/LaMP_4 \
  10
```

## 📊 What You'll See

```
🚀 FINTs LaMP-4 Evaluation
======================================

[1/10] Sample: 310
  Reference: Social Media Gone Awry: Tips for Teens to Stay Safe...
  Baseline: Protecting Your Teen from Online Dangers...
    ROUGE-1: 0.10 | Latency: 2850ms
  Building FINTs from 10 pairs...
  FINTs: 5 Ways to Keep Your Teen Safe Online...
    ROUGE-1: 0.25 | Latency: 3200ms
    Improvement: +0.15

... (9 more samples) ...

📊 Results
======================================
Samples: 10

🔵 Baseline ROUGE-1: 0.107
🟢 FINTs ROUGE-1:    0.130
📈 Improvement:      +0.023
✅ Samples improved: 3/10 (30.0%)

⏱️  Avg latency (baseline): 10862ms
⏱️  Avg latency (FINTs):    12184ms
📊 Overhead: 1322ms
```

## 🎯 What FINTs Does

1. **Loads LaMP-4 test samples** - News articles needing headlines
2. **Builds author profiles** - Uses their past headlines as positive examples
3. **Extracts steering vectors** - From contrastive pairs (author's style vs others)
4. **Generates with steering** - Produces headlines matching author's style
5. **Compares results** - ROUGE-1 scores baseline vs FINTs

## ⚙️ Configuration

**Quick config changes** in `examples/lamp_demo.cpp`:

```cpp
// Line ~273: Steering strength
config_fints.scale = 3.0f;  // Try 1.0-5.0

// Line ~275-276: Layer range
config_fints.start_layer = 17;
config_fints.end_layer = 26;
```

**Sample count:**
```bash
./scripts/run_lamp.sh model.gguf lamp_data/LaMP_4 3   # Fast (3 samples)
./scripts/run_lamp.sh model.gguf lamp_data/LaMP_4 50  # Full eval
```

## 📁 Project Structure

```
fints-demo/
├── src/
│   ├── fints.h           # FINTs API
│   └── fints.cpp         # Implementation
├── examples/
│   ├── basic_demo.cpp    # Simple demo
│   └── lamp_demo.cpp     # LaMP-4 evaluation ⭐
├── scripts/
│   ├── setup.sh          # Build everything
│   ├── download_lamp4.sh # Get dataset
│   └── run_lamp.sh       # Run evaluation
├── README.md             # This file
├── LAMP_GUIDE.md         # Detailed LaMP guide
└── INTEGRATION.md        # API integration guide
```

## 🔧 Troubleshooting

**"Model not found":**
- Check model path is correct and file exists
- Use absolute paths for reliability

**"Dataset not found":**
- Run `./scripts/download_lamp4.sh` first
- Check `lamp_data/LaMP_4/` contains JSON files

**Slow evaluation:**
- CPU-only mode is used (avoids GPU timeout)
- Try fewer samples: `./scripts/run_lamp.sh model.gguf lamp_data/LaMP_4 3`
- Use smaller quantized model (Q4_K_M recommended)

**Out of memory:**
- Reduce sample count
- Use smaller model
- Close other applications

## 📚 Additional Demos

**Basic demo (hardcoded examples):**
```bash
./build/basic_demo models/model.gguf "Tell me a story"
```

**See also:**
- `LAMP_GUIDE.md` - Detailed LaMP-4 guide
- `INTEGRATION.md` - Use FINTs in your code
- `GITHUB.md` - Publish to GitHub

## 🛠️ Technical Details

**Dependencies:**
- Custom llama.cpp fork: [`llama.cpp-fints`](https://github.com/guzzle17/llama.cpp-fints)
- nlohmann-json (installed via `brew install nlohmann-json`)

**Custom APIs added to llama.cpp:**
- `llama_get_activation_info()` - Get layer dimensions
- `llama_set_activation_extraction()` - Enable activation capture
- `llama_get_layer_activations()` - Extract activations
- `llama_apply_adapter_cvec_fints()` - Apply steering vectors

**Performance:**
- LaMP-4 (5 samples): ~10-15 minutes on CPU
- ROUGE-1 improvements: typically +2-5% over baseline
- Memory overhead: ~700MB for steering vectors

## 📖 Citation

```bibtex
@article{fints2024,
  title={FINTs: Fast Input-aware No-Training Steering for LLMs},
  author={Your Name},
  year={2024}
}
```

## 📄 License

MIT License - See LICENSE file

---

**For questions or issues**: Check `LAMP_GUIDE.md` for detailed instructions or `INTEGRATION.md` for API usage.
