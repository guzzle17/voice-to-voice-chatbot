@echo off
REM FINTs Demo Setup Script for Windows
REM Requires: Git, CMake, Visual Studio or MinGW

echo ==================================
echo FINTs Demo Setup (Windows)
echo ==================================
echo.

REM [1/4] Check prerequisites
echo [1/4] Checking prerequisites...

where git >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: Git not found. Please install Git for Windows.
    exit /b 1
)

where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: CMake not found. Please install CMake.
    exit /b 1
)

echo Prerequisites OK
echo.

REM [2/4] Setup llama.cpp
echo [2/4] Setting up llama.cpp ^(FINTs fork^)...

if not exist "llama.cpp" (
    echo Cloning llama.cpp with FINTs support...
    git clone https://github.com/guzzle17/llama.cpp-fints.git llama.cpp
    cd llama.cpp
) else (
    echo llama.cpp already exists, pulling latest...
    cd llama.cpp
    git pull
)

echo Building llama.cpp...
if not exist "build" mkdir build
cd build
cmake .. -DBUILD_SHARED_LIBS=OFF -DGGML_NATIVE=OFF
cmake --build . --target llama --config Release

cd ..\..
echo llama.cpp built
echo.

REM [3/4] Build FINTs demo
echo [3/4] Building FINTs demo...

if not exist "build" mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

cd ..
echo FINTs demo built
echo.

echo [4/4] Setup complete!
echo.
echo ==================================
echo Next steps:
echo ==================================
echo 1. Download a GGUF model
echo 2. Download LaMP-4: scripts\download_lamp4.bat
echo 3. Run demo: scripts\run_lamp.bat models\your-model.gguf lamp_data\LaMP_4 5
echo ==================================
