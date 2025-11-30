#!/bin/bash
set -e

echo "======================================"
echo "Downloading LaMP-4 Dataset"
echo "======================================"
echo ""

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_ROOT"
mkdir -p lamp_data

# Download LaMP-4 from official source
echo "Downloading LaMP-4 dataset from lamp-benchmark.github.io..."
echo ""

# LaMP-4 download URLs (from https://lamp-benchmark.github.io/download)
DEV_QUESTIONS_URL="https://ciir.cs.umass.edu/downloads/LaMP/LaMP_4/dev/dev_questions.json"
DEV_OUTPUTS_URL="https://ciir.cs.umass.edu/downloads/LaMP/LaMP_4/dev/dev_outputs.json"
TRAIN_QUESTIONS_URL="https://ciir.cs.umass.edu/downloads/LaMP/LaMP_4/train/train_questions.json"
TRAIN_OUTPUTS_URL="https://ciir.cs.umass.edu/downloads/LaMP/LaMP_4/train/train_outputs.json"

mkdir -p lamp_data/LaMP_4

echo "Downloading dev_questions.json..."
curl -L "$DEV_QUESTIONS_URL" -o lamp_data/LaMP_4/dev_questions.json

echo "Downloading dev_outputs.json..."
curl -L "$DEV_OUTPUTS_URL" -o lamp_data/LaMP_4/dev_outputs.json

echo "Downloading train_questions.json..."
curl -L "$TRAIN_QUESTIONS_URL" -o lamp_data/LaMP_4/train_questions.json

echo "Downloading train_outputs.json..."
curl -L "$TRAIN_OUTPUTS_URL" -o lamp_data/LaMP_4/train_outputs.json

# Verify files
echo ""
echo "Verifying dataset files..."
required_files=("dev_questions.json" "dev_outputs.json" "train_questions.json" "train_outputs.json")
all_found=true

for file in "${required_files[@]}"; do
    if [ -f "lamp_data/LaMP_4/$file" ]; then
        size=$(du -h "lamp_data/LaMP_4/$file" | cut -f1)
        echo "✓ $file ($size)"
    else
        echo "✗ $file - MISSING"
        all_found=false
    fi
done

echo ""
if [ "$all_found" = true ]; then
    echo "======================================"
    echo "✓ LaMP-4 dataset ready!"
    echo "======================================"
    echo ""
    echo "Dataset source: https://lamp-benchmark.github.io/download"
    echo ""
    echo "You can now run:"
    echo "  ./scripts/run_lamp.sh models/your-model.gguf lamp_data/LaMP_4 10"
else
    echo "Error: Some files failed to download."
    echo "Please check your internet connection and try again."
    echo "Or download manually from: https://lamp-benchmark.github.io/download"
    exit 1
fi
