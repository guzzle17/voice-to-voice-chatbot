@echo off
REM Run basic FINTs demo on Windows

if "%~1"=="" (
    echo Usage: %~nx0 MODEL_PATH [PROMPT]
    echo Example: %~nx0 models\model.gguf "Tell me a story"
    exit /b 1
)

set MODEL_PATH=%~1
set PROMPT=%~2

if "%PROMPT%"=="" set PROMPT=Tell me a story

if not exist "%MODEL_PATH%" (
    echo ERROR: Model not found: %MODEL_PATH%
    exit /b 1
)

if not exist "build\Release\basic_demo.exe" (
    echo ERROR: basic_demo.exe not built
    echo Run scripts\setup.bat first
    exit /b 1
)

build\Release\basic_demo.exe "%MODEL_PATH%" "%PROMPT%"
