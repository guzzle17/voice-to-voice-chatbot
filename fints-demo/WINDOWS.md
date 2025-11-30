# FINTs Demo - Windows Setup Guide

## Prerequisites

1. **Visual Studio** (2019 or later) with C++ Desktop Development
   - Or **MinGW-w64** if using GCC
   - Download: https://visualstudio.microsoft.com/downloads/

2. **CMake** (3.10+)
   - Download: https://cmake.org/download/
   - Make sure to add to PATH during installation

3. **Git for Windows**
   - Download: https://git-scm.com/download/win
   - Use default settings

4. **curl** (usually included with Windows 10+)
   - Check: Open PowerShell and type `curl --version`

## Quick Start

### Option 1: Using Command Prompt (Recommended)

```cmd
REM Clone and navigate
git clone -b fints-demo https://github.com/guzzle17/voice-to-voice-chatbot.git
cd voice-to-voice-chatbot\fints-demo

REM Run setup
scripts\setup.bat

REM Download dataset
scripts\download_lamp4.bat

REM Run evaluation (adjust model path)
scripts\run_lamp.bat C:\path\to\model.gguf lamp_data\LaMP_4 5
```

### Option 2: Using PowerShell

```powershell
# Clone and navigate
git clone -b fints-demo https://github.com/guzzle17/voice-to-voice-chatbot.git
cd voice-to-voice-chatbot\fints-demo

# Run setup
.\scripts\setup.bat

# Download dataset
.\scripts\download_lamp4.bat

# Run evaluation
.\scripts\run_lamp.bat "C:\path\to\model.gguf" "lamp_data\LaMP_4" 5
```

## Manual Build (If scripts fail)

### 1. Clone llama.cpp fork

```cmd
git clone https://github.com/guzzle17/llama.cpp-fints.git llama.cpp
cd llama.cpp
mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF
cmake --build . --target llama --config Release
cd ..\..
```

### 2. Build FINTs demo

```cmd
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

### 3. Download LaMP-4 dataset

Use `curl` in PowerShell or download manually from:
- https://github.com/LaMP-Benchmark/LaMP/tree/main/data/LaMP_4

Place files in `lamp_data\LaMP_4\`

### 4. Run

```cmd
build\Release\lamp_demo.exe C:\path\to\model.gguf lamp_data\LaMP_4 5
```

## Common Windows Issues

### Issue: "cmake not found"
**Solution:** Add CMake to PATH
1. Find CMake installation (usually `C:\Program Files\CMake\bin`)
2. Add to System Environment Variables → Path

### Issue: "MSVC not found"
**Solution:** Open "Developer Command Prompt for VS" from Start Menu instead of regular Command Prompt

### Issue: "curl failed"
**Solution:** Try PowerShell with different curl syntax:
```powershell
Invoke-WebRequest -Uri "URL" -OutFile "filename.json"
```

### Issue: Build fails with "Metal not found"
**Solution:** This is expected on Windows. The code will use CPU backend automatically.

### Issue: Very slow evaluation
**Solution:** 
- CPU-only mode is used on Windows (no GPU acceleration)
- Use fewer samples: `scripts\run_lamp.bat model.gguf lamp_data\LaMP_4 3`
- Use smaller quantized model (Q4_K_M)

## Performance Notes

**Windows performance:**
- CPU-only (no CUDA support in demo by default)
- Expect evaluation to take 2-3x longer than macOS/Linux
- 5 samples: ~20-30 minutes
- Consider running overnight for full evaluation

**Recommended:**
- Start with 3-5 samples to test
- Use Q4_K_M quantized models for speed
- Close other applications during evaluation

## File Paths

Remember to use backslashes (`\`) on Windows:
- **Correct:** `C:\models\model.gguf`
- **Correct:** `lamp_data\LaMP_4`
- **Incorrect:** `C:/models/model.gguf` (might work but not standard)

## Getting Models

Download GGUF models from Hugging Face:
```powershell
# Example: Download Llama-3.1-8B-Instruct Q4_K_M
curl -L "https://huggingface.co/bartowski/Meta-Llama-3.1-8B-Instruct-GGUF/resolve/main/Meta-Llama-3.1-8B-Instruct-Q4_K_M.gguf" -o "models\llama-3.1-8b-instruct.gguf"
```

Or use a download manager for large files.

## Next Steps

After successful setup:
1. See main [README.md](README.md) for configuration options
2. Check [LAMP_GUIDE.md](LAMP_GUIDE.md) for detailed evaluation info
3. Read [INTEGRATION.md](INTEGRATION.md) to use FINTs in your code

## Need Help?

Check the main README.md troubleshooting section or verify:
- [ ] CMake in PATH
- [ ] Visual Studio installed
- [ ] Git installed
- [ ] Model file exists and is correct GGUF format
- [ ] Dataset downloaded completely
