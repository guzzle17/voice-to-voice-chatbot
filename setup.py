#!/usr/bin/env python3
"""
Automated setup script to download all required models
Cross-platform: Works on Linux, macOS, and Windows
Run this after cloning the repository
"""

import os
import sys
import platform
import urllib.request
import subprocess
from pathlib import Path

# Detect OS
IS_WINDOWS = platform.system() == "Windows"
IS_MACOS = platform.system() == "Darwin"
IS_LINUX = platform.system() == "Linux"

def download_file(url, dest, desc="Downloading"):
    """Download file with progress"""
    print(f"{desc}...")
    try:
        urllib.request.urlretrieve(url, dest)
        print(f"✓ Downloaded to {dest}")
        return True
    except Exception as e:
        print(f"✗ Failed: {e}")
        return False

def setup_whisper():
    """Clone and build Whisper.cpp"""
    print("\n📦 Setting up Whisper.cpp...")
    
    if Path("whisper.cpp").exists():
        print("✓ Whisper.cpp already exists")
        return True
    
    try:
        # Clone repository
        subprocess.run(["git", "clone", "https://github.com/ggerganov/whisper.cpp.git"], check=True)
        
        # Build based on OS
        if IS_WINDOWS:
            print("Building with CMake (Windows)...")
            os.chdir("whisper.cpp")
            os.makedirs("build", exist_ok=True)
            os.chdir("build")
            subprocess.run(["cmake", ".."], check=True)
            subprocess.run(["cmake", "--build", ".", "--config", "Release"], check=True)
            os.chdir("../..")
            
            # Download model manually on Windows
            print("Downloading Whisper model...")
            model_dir = Path("whisper.cpp/models")
            model_dir.mkdir(exist_ok=True)
            download_file(
                "https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.en.bin",
                model_dir / "ggml-base.en.bin",
                "Downloading Whisper base.en model"
            )
        else:
            print("Building with Make (Linux/macOS)...")
            os.chdir("whisper.cpp")
            subprocess.run(["make"], check=True)
            
            # Download model
            if IS_MACOS:
                subprocess.run(["bash", "models/download-ggml-model.sh", "base.en"], check=True)
            else:
                subprocess.run(["bash", "models/download-ggml-model.sh", "base.en"], check=True)
            
            os.chdir("..")
        
        print("✓ Whisper.cpp setup complete")
        return True
    except Exception as e:
        print(f"✗ Failed: {e}")
        if IS_WINDOWS:
            print("\n⚠️  Windows users: Ensure you have CMake and Visual Studio Build Tools installed")
        return False

def setup_models():
    """Download LLaMA and Piper models"""
    print("\n📦 Downloading models...")
    
    # Create directories
    models_dir = Path("models")
    piper_dir = Path("piper")
    models_dir.mkdir(exist_ok=True)
    piper_dir.mkdir(exist_ok=True)
    
    success = True
    
    # Download LLaMA model (~800 MB)
    llama_path = models_dir / "Llama-3.2-1B-Instruct-Q4_K_M.gguf"
    if not llama_path.exists():
        success &= download_file(
            "https://huggingface.co/lmstudio-community/Llama-3.2-1B-Instruct-GGUF/resolve/main/Llama-3.2-1B-Instruct-Q4_K_M.gguf",
            llama_path,
            "Downloading LLaMA 3.2 1B (~800 MB)"
        )
    else:
        print(f"✓ LLaMA model already exists")
    
    # Download Piper voice model
    piper_model = piper_dir / "en_US-kristin-medium.onnx"
    piper_config = piper_dir / "en_US-kristin-medium.onnx.json"
    
    if not piper_model.exists():
        success &= download_file(
            "https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/kristin/medium/en_US-kristin-medium.onnx",
            piper_model,
            "Downloading Piper voice model"
        )
    else:
        print(f"✓ Piper model already exists")
    
    if not piper_config.exists():
        success &= download_file(
            "https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/kristin/medium/en_US-kristin-medium.onnx.json",
            piper_config,
            "Downloading Piper config"
        )
    else:
        print(f"✓ Piper config already exists")
    
    return success

def update_paths():
    """Update model paths in the script"""
    print("\n📝 Updating model paths...")
    
    script_path = Path("interactive_voice_assistant.py")
    if not script_path.exists():
        print("✗ interactive_voice_assistant.py not found")
        return False
    
    with open(script_path, 'r') as f:
        content = f.read()
    
    # Get absolute path to project directory
    project_dir = Path.cwd().absolute()
    
    # Set paths based on OS
    if IS_WINDOWS:
        # Windows: use forward slashes in Python paths
        whisper_model = f"{project_dir}/whisper.cpp/models/ggml-base.en.bin".replace("\\", "/")
        whisper_exe = f"{project_dir}/whisper.cpp/build/bin/Release/whisper-cli.exe".replace("\\", "/")
        llama_model = f"{project_dir}/models/Llama-3.2-1B-Instruct-Q4_K_M.gguf".replace("\\", "/")
        piper_model = f"{project_dir}/piper/en_US-kristin-medium.onnx".replace("\\", "/")
    else:
        # Linux/macOS: use normal paths
        whisper_model = f"{project_dir}/whisper.cpp/models/ggml-base.en.bin"
        whisper_exe = f"{project_dir}/whisper.cpp/build/bin/whisper-cli"
        llama_model = f"{project_dir}/models/Llama-3.2-1B-Instruct-Q4_K_M.gguf"
        piper_model = f"{project_dir}/piper/en_US-kristin-medium.onnx"
    
    # Update paths (handle both possible existing formats)
    lines = content.split('\n')
    updated_lines = []
    
    for line in lines:
        if 'WHISPER_MODEL_PATH =' in line and not line.strip().startswith('#'):
            updated_lines.append(f'WHISPER_MODEL_PATH = "{whisper_model}"')
        elif 'WHISPER_EXECUTABLE =' in line and not line.strip().startswith('#'):
            updated_lines.append(f'WHISPER_EXECUTABLE = "{whisper_exe}"')
        elif 'LLAMA_MODEL_PATH =' in line and not line.strip().startswith('#'):
            updated_lines.append(f'LLAMA_MODEL_PATH = "{llama_model}"')
        elif 'PIPER_MODEL_PATH =' in line and not line.strip().startswith('#'):
            updated_lines.append(f'PIPER_MODEL_PATH = "{piper_model}"')
        else:
            updated_lines.append(line)
    
    with open(script_path, 'w') as f:
        f.write('\n'.join(updated_lines))
    
    print(f"✓ Paths updated for {platform.system()}")
    return True

def main():
    print("=" * 60)
    print("Interactive Voice Assistant - Setup")
    print(f"OS: {platform.system()} ({platform.machine()})")
    print("=" * 60)
    
    # Check Python version
    if sys.version_info < (3, 8):
        print("✗ Python 3.8+ required")
        sys.exit(1)
    
    print(f"✓ Python {sys.version_info.major}.{sys.version_info.minor}")
    
    # Check for required tools
    if IS_WINDOWS:
        print("\n⚠️  Windows detected - ensure you have:")
        print("   - CMake installed")
        print("   - Visual Studio Build Tools installed")
        print("   - Git installed")
    
    # Check if requirements.txt exists
    if not Path("requirements.txt").exists():
        print("\n⚠️  requirements.txt not found, creating it...")
        with open("requirements.txt", "w") as f:
            f.write("llama-cpp-python>=0.2.0\n")
            f.write("piper-tts>=1.2.0\n")
            f.write("pyaudio>=0.2.13\n")
        print("✓ requirements.txt created")
    
    # Install Python dependencies
    print("\n📦 Installing Python dependencies...")
    try:
        subprocess.run([sys.executable, "-m", "pip", "install", "-r", "requirements.txt"], check=True)
        print("✓ Dependencies installed")
    except subprocess.CalledProcessError:
        print("⚠️  Some dependencies failed to install")
        if IS_WINDOWS:
            print("   For PyAudio on Windows, try:")
            print("   pip install pipwin")
            print("   pipwin install pyaudio")
    
    # Setup components
    success = True
    success &= setup_whisper()
    success &= setup_models()
    success &= update_paths()
    
    print("\n" + "=" * 60)
    if success:
        print("✅ Setup complete!")
        print("\nRun the assistant with:")
        if IS_WINDOWS:
            print("  python interactive_voice_assistant.py")
        else:
            print("  python3 interactive_voice_assistant.py")
        print("\nOr with environment variables:")
        if IS_WINDOWS:
            print("  $env:LLAMA_MAX_TOKENS=64")
            print("  python interactive_voice_assistant.py")
        else:
            print("  export LLAMA_MAX_TOKENS=64")
            print("  python3 interactive_voice_assistant.py")
    else:
        print("⚠️  Setup completed with errors")
        print("Please check the messages above and try again")
    print("=" * 60)

if __name__ == "__main__":
    main()
