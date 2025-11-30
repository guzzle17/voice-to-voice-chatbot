#!/bin/bash
set -e

echo "=================================="
echo "FINTs Demo Setup"
echo "=================================="
echo ""

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_ROOT"

echo -e "${BLUE}[1/4] Checking prerequisites...${NC}"
# Check for required tools
command -v cmake >/dev/null 2>&1 || { echo "Error: cmake not found. Install with: brew install cmake"; exit 1; }
command -v g++ >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1 || { echo "Error: C++ compiler not found"; exit 1; }

echo -e "${GREEN}✓ Prerequisites OK${NC}"
echo ""

echo -e "${BLUE}[2/4] Setting up llama.cpp (FINTs fork)...${NC}"
# Clone llama.cpp FINTs fork if not exists
if [ ! -d "llama.cpp" ]; then
    echo "Cloning llama.cpp with FINTs support..."
    git clone https://github.com/guzzle17/llama.cpp-fints.git llama.cpp
    cd llama.cpp
else
    echo "llama.cpp already exists, pulling latest..."
    cd llama.cpp
    git pull
fi

# Build llama.cpp
echo "Building llama.cpp..."
if [ ! -d "build" ]; then
    cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
fi
# Build only the llama library (skip tests to avoid mtmd.h error)
cmake --build build --target llama --config Release -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

cd "$PROJECT_ROOT"
echo -e "${GREEN}✓ llama.cpp built${NC}"
echo ""

echo -e "${BLUE}[3/4] Building FINTs demo...${NC}"
# Build FINTs
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

cd "$PROJECT_ROOT"
echo -e "${GREEN}✓ FINTs demo built${NC}"
echo ""

echo -e "${BLUE}[4/4] Setup complete!${NC}"
echo ""
echo "=================================="
echo "Next steps:"
echo "=================================="
echo "1. Download a GGUF model to models/"
echo "   Example: wget https://huggingface.co/.../model.gguf -O models/model.gguf"
echo ""
echo "2. Run the demo:"
echo "   ./scripts/run_demo.sh models/your-model.gguf"
echo ""
echo "=================================="
