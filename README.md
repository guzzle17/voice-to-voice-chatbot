# Interactive Voice Assistant

**Full voice-to-voice AI assistant running 100% locally with no servers required!**

```
🎤 Audio Input (Microphone)
    ↓
🗣️  Whisper.cpp (Speech-to-Text)
    ↓
🤖 LLaMA 3.1 8B Instruct (Chat AI with streaming)
    ↓
🔊 Piper TTS (Text-to-Speech with streaming)
    ↓
🎵 Audio Output (Real-time)
```

---

## 🚀 Quick Start

```bash
# 1. Clone the repository
git clone https://github.com/guzzle17/voice-to-voice-chatbot.git
cd voice-to-voice-chatbot

# 2. Run automated setup (downloads all models)
python3 setup.py   # or: python setup.py (Windows)

# 3. Run the assistant
python3 interactive_voice_assistant.py
```

**That's it!** The setup script automatically:
- ✅ Installs Python dependencies
- ✅ Downloads and builds Whisper.cpp
- ✅ Downloads LLaMA 3.1 8B Instruct model (~4.9 GB)
- ✅ Downloads Piper TTS voice model (~63 MB)
- ✅ Configures paths for your OS

**First run may take 10-20 minutes** depending on your internet speed.

---

<details>
<summary><b>📋 Manual Installation (Advanced Users)</b></summary>

If you prefer to set up components manually, expand your OS below:

<details>
<summary><b>🐧 Linux Manual Installation</b></summary>

### 1. Install Dependencies

```bash
# Update system
sudo apt update && sudo apt upgrade -y

# Install build tools and Python
sudo apt install -y build-essential python3 python3-pip python3-venv git cmake

# Install audio libraries
sudo apt install -y portaudio19-dev libsndfile1 alsa-utils
```

### 2. Set Up Python Virtual Environment

```bash
cd ~/vscode_projects
python3 -m venv venv
source venv/bin/activate
pip install --upgrade pip
```

### 3. Install Python Packages

```bash
# Install llama-cpp-python (CPU version)
pip install llama-cpp-python

# For GPU support (NVIDIA CUDA):
# CMAKE_ARGS="-DGGML_CUDA=on" pip install llama-cpp-python --force-reinstall --no-cache-dir

# Install other dependencies (required for streaming)
pip install piper-tts pyaudio
```

**Note:** `piper-tts` and `pyaudio` are **required** for the interactive voice assistant.

### 4. Install Whisper.cpp

```bash
cd ~
git clone https://github.com/ggerganov/whisper.cpp.git
cd whisper.cpp

# Build
make

# Download Whisper model (base English)
bash models/download-ggml-model.sh base.en
```

### 5. Download LLaMA Model

```bash
# Create models directory
mkdir -p ~/llama.cpp/models
cd ~/llama.cpp/models

# Download LLaMA 3.1 8B Instruct Q4_K_M (~4.9 GB)
wget https://huggingface.co/unsloth/Llama-3.1-8B-Instruct-GGUF/resolve/main/Llama-3.1-8B-Instruct-Q4_K_M.gguf
```

### 6. Install Piper TTS

```bash
# Download voice model
mkdir -p ~/piper
cd ~/piper

# Download Kristin voice (medium quality)
wget https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/kristin/medium/en_US-kristin-medium.onnx
wget https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/kristin/medium/en_US-kristin-medium.onnx.json
```

### 7. Update Model Paths

Edit `interactive_voice_assistant.py` and update these paths:

```python
WHISPER_MODEL_PATH = "/home/YOUR_USER/whisper.cpp/models/ggml-base.en.bin"
WHISPER_EXECUTABLE = "/home/YOUR_USER/whisper.cpp/build/bin/whisper-cli"
LLAMA_MODEL_PATH = "/home/YOUR_USER/llama.cpp/models/Llama-3.1-8B-Instruct-Q4_K_M.gguf"
PIPER_MODEL_PATH = "/home/YOUR_USER/piper/en_US-kristin-medium.onnx"
```

</details>

<details>
<summary><b>🪟 Windows Installation</b></summary>

### 1. Install Prerequisites

1. **Install Python 3.10+**: Download from [python.org](https://www.python.org/downloads/)
   - ✅ Check "Add Python to PATH" during installation
   
2. **Install Git**: Download from [git-scm.com](https://git-scm.com/download/win)

3. **Install Visual Studio Build Tools**: Download from [visualstudio.microsoft.com](https://visualstudio.microsoft.com/downloads/)
   - Select "Desktop development with C++"
   - Required for compiling native extensions

4. **Install CMake**: Download from [cmake.org](https://cmake.org/download/)

### 2. Set Up Python Virtual Environment

```powershell
# Open PowerShell or Command Prompt
cd C:\Users\YourUser\vscode_projects
python -m venv venv
.\venv\Scripts\activate
pip install --upgrade pip
```

### 3. Install Python Packages

```powershell
# Install llama-cpp-python (CPU version)
pip install llama-cpp-python

# For GPU support (NVIDIA CUDA - requires CUDA Toolkit):
# $env:CMAKE_ARGS="-DGGML_CUDA=on"
# pip install llama-cpp-python --force-reinstall --no-cache-dir

# Install other dependencies
pip install piper-tts pyaudio
```

**Note:** PyAudio on Windows may require pre-built wheels. If it fails:
```powershell
pip install pipwin
pipwin install pyaudio
```

### 4. Install Whisper.cpp

```powershell
cd ~
git clone https://github.com/ggerganov/whisper.cpp.git
cd whisper.cpp

# Build with CMake
mkdir build
cd build
cmake ..
cmake --build . --config Release

# Download model
cd ..
powershell -c "Invoke-WebRequest -Uri 'https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.en.bin' -OutFile 'models/ggml-base.en.bin'"
```

### 5. Download LLaMA Model

```powershell
# Create models directory
mkdir C:\Users\YourUser\llama_models
cd C:\Users\YourUser\llama_models

# Download LLaMA model (~4.9 GB)
Invoke-WebRequest -Uri "https://huggingface.co/unsloth/Llama-3.1-8B-Instruct-GGUF/resolve/main/Llama-3.1-8B-Instruct-Q4_K_M.gguf" -OutFile "Llama-3.1-8B-Instruct-Q4_K_M.gguf"
```

### 6. Install Piper TTS

```powershell
# Create piper directory
mkdir C:\Users\YourUser\piper
cd C:\Users\YourUser\piper

# Download voice model
Invoke-WebRequest -Uri "https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/kristin/medium/en_US-kristin-medium.onnx" -OutFile "en_US-kristin-medium.onnx"
Invoke-WebRequest -Uri "https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/kristin/medium/en_US-kristin-medium.onnx.json" -OutFile "en_US-kristin-medium.onnx.json"
```

### 7. Update Model Paths

Edit `interactive_voice_assistant.py` and update these paths:

```python
WHISPER_MODEL_PATH = "C:/Users/YourUser/whisper.cpp/models/ggml-base.en.bin"
WHISPER_EXECUTABLE = "C:/Users/YourUser/whisper.cpp/build/bin/Release/whisper-cli.exe"
LLAMA_MODEL_PATH = "C:/Users/YourUser/llama_models/Llama-3.1-8B-Instruct-Q4_K_M.gguf"
PIPER_MODEL_PATH = "C:/Users/YourUser/piper/en_US-kristin-medium.onnx"
```

**Note:** Use forward slashes `/` or escaped backslashes `\\` in Python paths.

</details>

<details>
<summary><b>🍎 macOS Installation</b></summary>

### 1. Install Prerequisites

```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install python3 git cmake portaudio
```

### 2. Set Up Python Virtual Environment

```bash
cd ~/vscode_projects
python3 -m venv venv
source venv/bin/activate
pip install --upgrade pip
```

### 3. Install Python Packages

```bash
# Install llama-cpp-python with Metal support (Apple Silicon GPU)
CMAKE_ARGS="-DGGML_METAL=on" pip install llama-cpp-python --force-reinstall --no-cache-dir

# For Intel Macs (CPU only):
# pip install llama-cpp-python

# Install other dependencies
pip install piper-tts pyaudio
```

### 4. Install Whisper.cpp

```bash
cd ~
git clone https://github.com/ggerganov/whisper.cpp.git
cd whisper.cpp

# Build
make

# Download Whisper model (base English)
bash models/download-ggml-model.sh base.en
```

### 5. Download LLaMA Model

```bash
# Create models directory
mkdir -p ~/llama_models
cd ~/llama_models

# Download LLaMA 3.1 8B Instruct Q4_K_M (~4.9 GB)
curl -L -o Llama-3.1-8B-Instruct-Q4_K_M.gguf \
  "https://huggingface.co/unsloth/Llama-3.1-8B-Instruct-GGUF/resolve/main/Llama-3.1-8B-Instruct-Q4_K_M.gguf"
```

### 6. Install Piper TTS

```bash
# Download voice model
mkdir -p ~/piper
cd ~/piper

# Download Kristin voice (medium quality)
curl -L -o en_US-kristin-medium.onnx \
  "https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/kristin/medium/en_US-kristin-medium.onnx"
curl -L -o en_US-kristin-medium.onnx.json \
  "https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/kristin/medium/en_US-kristin-medium.onnx.json"
```

### 7. Update Model Paths

Edit `interactive_voice_assistant.py` and update these paths:

```python
WHISPER_MODEL_PATH = "/Users/YOUR_USER/whisper.cpp/models/ggml-base.en.bin"
WHISPER_EXECUTABLE = "/Users/YOUR_USER/whisper.cpp/build/bin/whisper-cli"
LLAMA_MODEL_PATH = "/Users/YOUR_USER/llama_models/Llama-3.1-8B-Instruct-Q4_K_M.gguf"
PIPER_MODEL_PATH = "/Users/YOUR_USER/piper/en_US-kristin-medium.onnx"
```

**Note for Apple Silicon (M1/M2/M3):** Make sure to use the Metal-enabled llama-cpp-python for GPU acceleration!

</details>


</details>

</details>

---

## Usage

### Running the Interactive Assistant

<details open>
<summary><b>🐧 Linux / 🍎 macOS</b></summary>

```bash
# Run the interactive assistant
python3 interactive_voice_assistant.py
```

</details>

<details>
<summary><b>🪟 Windows</b></summary>

```powershell
# Run the interactive assistant
python interactive_voice_assistant.py
```

</details>

**Features:**
- 🎤 **Press Enter** to start recording from your microphone
- 🛑 **Press Ctrl+C** while recording to stop and process
- 🔊 **Streaming TTS**: Audio plays while text is being generated (real-time)
- 💬 Maintains conversation history for context-aware responses
- 🧠 Smart text splitting with sliding window algorithm
- 📝 Optimized prompts for speech-friendly responses (no emojis/formatting)
- 📝 Optimized prompts for speech-friendly responses (no emojis/formatting)

#### Interactive Mode Controls

Once running, you can:
- **Press Enter**: Start recording your voice
- **Ctrl+C (while recording)**: Stop recording and get a response
- **Type `quit` or `exit`**: End the session
- **Type `history`**: View conversation history
- **Type `clear`**: Clear conversation history

#### Environment Variables for Interactive Mode

<details open>
<summary><b>🐧 Linux / 🍎 macOS</b></summary>

```bash
# Adjust response length (default: -1 = unlimited)
export LLAMA_MAX_TOKENS=128

# GPU acceleration (if available)
export LLAMA_N_GPU_LAYERS=35
export LLAMA_N_THREADS=4

# Run the assistant
python3 interactive_voice_assistant.py
```

</details>

<details>
<summary><b>🪟 Windows</b></summary>

```powershell
# Adjust response length (default: -1 = unlimited)
$env:LLAMA_MAX_TOKENS=128

# GPU acceleration (if available)
$env:LLAMA_N_GPU_LAYERS=35
$env:LLAMA_N_THREADS=4

# Run the assistant
python interactive_voice_assistant.py
```

</details>

**Note:** Streaming TTS is always enabled - the assistant speaks while generating text in real-time.

#### Single-Shot Mode

Process a single audio file instead of continuous conversation:

<details open>
<summary><b>🐧 Linux / 🍎 macOS</b></summary>

```bash
# Process audio file with playback
python3 interactive_voice_assistant.py --audio input.wav

# Process without audio playback
python3 interactive_voice_assistant.py --audio input.wav --no-play

# Save transcript to file
python3 interactive_voice_assistant.py --audio input.wav --output response.wav
```

</details>

<details>
<summary><b>🪟 Windows</b></summary>

```powershell
# Process audio file with playback
python interactive_voice_assistant.py --audio input.wav

# Process without audio playback
python interactive_voice_assistant.py --audio input.wav --no-play

# Save transcript to file
python interactive_voice_assistant.py --audio input.wav --output response.wav
```

</details>

---

### Basic Usage (Single Query)

```bash
# Activate virtual environment
source venv/bin/activate

# Run the pipeline
python3 full_pipeline_no_server.py --audio input.wav --output response.wav

# Play the response
aplay response.wav  # Linux
# or
afplay response.wav  # macOS
```

### Performance Tuning (Environment Variables)

```bash
# Fast responses (16 tokens, streaming)
export LLAMA_MAX_TOKENS=16
export LLAMA_N_THREADS=4
python3 full_pipeline_no_server.py --audio input.wav --output response.wav

# GPU acceleration (if available)
export LLAMA_N_GPU_LAYERS=35
export LLAMA_N_THREADS=4
python3 full_pipeline_no_server.py --audio input.wav --output response.wav
```

### Environment Variables Reference

| Variable | Default | Description |
|----------|---------|-------------|
| `LLAMA_MAX_TOKENS` | -1 | Max tokens to generate (-1 = unlimited) |
| `LLAMA_N_THREADS` | min(8, CPU cores) | CPU threads for inference |
| `LLAMA_N_GPU_LAYERS` | 0 | GPU layers (0 = CPU only) |
| `LLAMA_N_CTX` | 4096 | Context window size |
| `LLAMA_TEMPERATURE` | 0.2 | Sampling temperature (0.1-1.0) |

---

## Troubleshooting

### Issue: "No such file or directory" errors
**Solution**: Update model paths in `interactive_voice_assistant.py` to match your system

### Issue: Piper Python API not found
**Solution**: 
- Install: `pip install piper-tts`
- The assistant requires Piper Python API and will exit if not found

### Issue: PyAudio not found
**Solution**:
- Install: `pip install pyaudio`
- Or on Linux: `sudo apt install python3-pyaudio`

### Issue: LLaMA is very slow
**Solution**: 
- Reduce `LLAMA_MAX_TOKENS` (try 16)
- Enable streaming: `export LLAMA_STREAM=1`
- Use fewer threads: `export LLAMA_N_THREADS=4`
- Consider GPU acceleration if available

### Issue: Out of memory
**Solution**:
- Reduce context: `export LLAMA_N_CTX=512`
- Close other applications
- Use lighter quantization (IQ2_XXS instead of Q4_K_M)

### Issue: Whisper crashes
**Solution**:
- Check Whisper.cpp build: `cd ~/whisper.cpp && make clean && make`
- Verify model exists: `ls ~/whisper.cpp/models/ggml-base.en.bin`

### Issue: Piper not found
**Solution**:
- Reinstall: `pip install --force-reinstall piper-tts`
- Check it's in PATH: `which piper`

---

## File Structure

```

~/whisper.cpp/
├── build/bin/whisper-cli          # Whisper executable
└── models/ggml-base.en.bin        # Whisper model

~/llama.cpp/models/
└── Llama-3.1-8B-Instruct-Q4_K_M.gguf  # LLaMA model (~4.9 GB)

~/piper/
├── en_US-kristin-medium.onnx      # Piper voice model
└── en_US-kristin-medium.onnx.json # Voice config
```

---

## 📦 Repository Structure

```
interactive-voice-assistant/
├── interactive_voice_assistant.py  # Main application
├── setup.py                        # Automated setup script
├── requirements.txt                # Python dependencies
├── .gitignore                      # Git ignore rules
├── README.md                       # This file
└── LICENSE                         # Software license (add your own)

# Generated by setup.py (not in git):
├── models/                         # LLaMA models (~4.9 GB)
├── piper/                          # Piper voice models (~63 MB)
└── whisper.cpp/                    # Whisper.cpp + models (~142 MB)
```

**Note:** Models are downloaded by `setup.py` and excluded from git (too large).

---

## 🎯 Features

- ✅ **100% Local**: No API keys, no internet after setup
- ✅ **Streaming TTS**: Audio plays while generating (low latency)
- ✅ **Cross-platform**: Linux, macOS, Windows
- ✅ **Smart Text Splitting**: Sliding window algorithm for natural speech
- ✅ **Context-aware**: Maintains conversation history
- ✅ **Speech-optimized**: Prompts exclude emojis/formatting
- ✅ **One-command setup**: `python3 setup.py` downloads everything

---

## License

This pipeline uses:
- **Whisper.cpp**: MIT License
- **LLaMA**: Meta's LLaMA 3.2 License
- **Piper TTS**: MIT License

---

## Credits

- Whisper: OpenAI
- LLaMA: Meta AI
- Piper TTS: Rhasspy Project
- llama.cpp: Georgi Gerganov

---

## Support

For issues or questions:
1. Verify all model paths are correct
2. Check system requirements
3. Monitor resource usage with `htop` or `top`

**Enjoy your fully local AI voice assistant!** 🚀
