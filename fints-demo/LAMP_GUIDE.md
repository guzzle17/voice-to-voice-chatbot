# LaMP-4 Evaluation Quick Start

## What You'll Get

Real evaluation on actual news headlines with **author-specific personalization**:
- Load test samples from LaMP-4 benchmark
- Build contrastive pairs from each author's writing history
- Compare baseline vs FINTs-steered generation
- Measure ROUGE-1 scores

## Prerequisites

1. **Install nlohmann-json** (for JSON parsing):
   ```bash
   brew install nlohmann-json
   ```

2. **Download LaMP-4 dataset**:
   ```bash
   cd fints-demo
   ./scripts/download_lamp4.sh
   ```
   
   This downloads from the official source: https://lamp-benchmark.github.io/download

3. **Rebuild** (to include lamp_demo):
   ```bash
   ./scripts/setup.sh
   ```

## Run Evaluation

```bash
./scripts/run_lamp.sh models/Llama-3.1-8B-Instruct-Q4_K_M.gguf lamp_data/LaMP_4 10
```

**Arguments:**
- `models/...`: Your GGUF model file
- `lamp_data/LaMP_4`: Dataset directory
- `10`: Number of test samples (default: 5, max: 2070)

## Expected Output

```
======================================
🚀 FINTs LaMP-4 Evaluation
======================================

📚 Loading model: ./models/Llama-3.1-8B-Instruct-Q4_K_M.gguf
✓ Model loaded

📂 Loading LaMP-4 data...
✓ Loaded 10 test samples
✓ Loaded 50 train samples (for negatives)

======================================
Running Evaluation
======================================

[1/10] Sample: 310
  Reference: Social Media Gone Awry: Tips for Teens to Stay Safe...
  Baseline: "Protecting Your Teen from Online Dangers: A Guide for Paren...
    ROUGE-1: 0.1000 | Latency: 2850ms
  Building FINTs from 10 pairs...
  FINTs:    5 Ways to Keep Your Teen Safe Online...
    ROUGE-1: 0.2500 | Latency: 3200ms
    Improvement: +0.1500

...

======================================
📊 Results
======================================

Samples: 10

🔵 Baseline ROUGE-1: 0.1070
🟢 FINTs ROUGE-1:    0.1300
📈 Improvement:      +0.0230
✅ Samples improved: 3/10 (30.0%)

⏱️  Avg latency (baseline): 10862ms
⏱️  Avg latency (FINTs):    12184ms
📊 Overhead: 1322ms
```

## How It Works

For each test sample:

1. **Load author profile** - All past headlines by that author
2. **Build contrastive pairs**:
   - Positive: Author's actual headline for different articles
   - Negative: Headlines from other authors
3. **Extract steering vectors** - From the contrastive pairs
4. **Generate headline**:
   - Baseline: No steering
   - FINTs: With author-style steering applied
5. **Compare** - Calculate ROUGE-1 against reference

## Customization

**Fewer samples (faster):**
```bash
./scripts/run_lamp.sh models/model.gguf lamp_data/LaMP_4 3
```

**More samples (better statistics):**
```bash
./scripts/run_lamp.sh models/model.gguf lamp_data/LaMP_4 50
```

**Modify steering strength** - Edit `lamp_demo.cpp` line ~210:
```cpp
config_fints.scale = 3.0f;  // Try 1.0-5.0
```

## Troubleshooting

**"lamp_demo not built":**
- Make sure nlohmann-json is installed: `brew install nlohmann-json`
- Rebuild: `./scripts/setup.sh`

**"Data directory not found":**
- Download dataset: `./scripts/download_lamp4.sh`

**Slow evaluation:**
- Use fewer samples: reduce last argument in run_lamp.sh
- Use smaller model (Q4_K_M or Q5_K_M quantization)

**Out of memory:**
- Reduce max_pairs in lamp_demo.cpp line ~205: `build_author_pairs(sample, train_samples, 5)`
