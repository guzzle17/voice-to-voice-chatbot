#!/bin/bash
set -e

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

# Check arguments
if [ $# -lt 2 ]; then
    echo -e "${RED}Usage: $0 <model_path> <lamp4_data_dir> [num_samples]${NC}"
    echo "Example: $0 ./models/model.gguf ./lamp_data/LaMP_4 10"
    echo ""
    echo "LaMP-4 data structure:"
    echo "  lamp_data/LaMP_4/"
    echo "  ├── dev_questions.json"
    echo "  ├── dev_outputs.json"
    echo "  ├── train_questions.json"
    echo "  └── train_outputs.json"
    exit 1
fi

MODEL_PATH="$1"
DATA_DIR="$2"
NUM_SAMPLES="${3:-5}"

# Check if model exists
if [ ! -f "$MODEL_PATH" ]; then
    echo -e "${RED}Error: Model file not found: $MODEL_PATH${NC}"
    exit 1
fi

# Check if data directory exists
if [ ! -d "$DATA_DIR" ]; then
    echo -e "${RED}Error: Data directory not found: $DATA_DIR${NC}"
    echo "Download LaMP-4 from: https://github.com/LaMP-Benchmark/LaMP"
    exit 1
fi

# Check for required JSON files
if [ ! -f "$DATA_DIR/dev_questions.json" ]; then
    echo -e "${RED}Error: dev_questions.json not found in $DATA_DIR${NC}"
    exit 1
fi

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Check if built
if [ ! -f "$PROJECT_ROOT/build/lamp_demo" ]; then
    echo -e "${RED}Error: lamp_demo not built.${NC}"
    echo "Make sure nlohmann-json is installed: brew install nlohmann-json"
    echo "Then run: ./scripts/setup.sh"
    exit 1
fi

echo -e "${BLUE}=================================="
echo "FINTs LaMP-4 Evaluation"
echo "==================================${NC}"
echo "Model: $MODEL_PATH"
echo "Data: $DATA_DIR"
echo "Samples: $NUM_SAMPLES"
echo ""

# Run evaluation
cd "$PROJECT_ROOT"
./build/lamp_demo "$MODEL_PATH" "$DATA_DIR" "$NUM_SAMPLES"
