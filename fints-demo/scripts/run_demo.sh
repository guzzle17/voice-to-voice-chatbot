#!/bin/bash
set -e

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

# Check arguments
if [ $# -lt 1 ]; then
    echo -e "${RED}Usage: $0 <model_path> [prompt]${NC}"
    echo "Example: $0 ./models/Llama-3.1-8B-Instruct-Q4_K_M.gguf"
    exit 1
fi

MODEL_PATH="$1"
PROMPT="${2:-Tell me a short story about a robot}"

# Check if model exists
if [ ! -f "$MODEL_PATH" ]; then
    echo -e "${RED}Error: Model file not found: $MODEL_PATH${NC}"
    exit 1
fi

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Check if built
if [ ! -f "$PROJECT_ROOT/build/basic_demo" ]; then
    echo -e "${RED}Error: Demo not built. Run ./scripts/setup.sh first${NC}"
    exit 1
fi

echo -e "${BLUE}=================================="
echo "FINTs Demo"
echo "==================================${NC}"
echo "Model: $MODEL_PATH"
echo "Prompt: $PROMPT"
echo ""

# Run demo
cd "$PROJECT_ROOT"
./build/basic_demo "$MODEL_PATH" "$PROMPT"
