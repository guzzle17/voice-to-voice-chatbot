@echo off
REM Run LaMP-4 evaluation on Windows

if "%~1"=="" (
    echo Usage: %~nx0 MODEL_PATH DATA_DIR [NUM_SAMPLES]
    echo Example: %~nx0 models\model.gguf lamp_data\LaMP_4 10
    exit /b 1
)

set MODEL_PATH=%~1
set DATA_DIR=%~2
set NUM_SAMPLES=%~3

if "%NUM_SAMPLES%"=="" set NUM_SAMPLES=5

echo ==================================
echo FINTs LaMP-4 Evaluation
echo ==================================
echo Model: %MODEL_PATH%
echo Dataset: %DATA_DIR%
echo Samples: %NUM_SAMPLES%
echo ==================================
echo.

if not exist "%MODEL_PATH%" (
    echo ERROR: Model not found: %MODEL_PATH%
    exit /b 1
)

if not exist "%DATA_DIR%" (
    echo ERROR: Dataset not found: %DATA_DIR%
    echo Run scripts\download_lamp4.bat first
    exit /b 1
)

if not exist "build\Release\lamp_demo.exe" (
    echo ERROR: lamp_demo.exe not built
    echo Run scripts\setup.bat first
    exit /b 1
)

echo Running evaluation...
echo.

build\Release\lamp_demo.exe "%MODEL_PATH%" "%DATA_DIR%" %NUM_SAMPLES%

echo.
echo ==================================
echo Evaluation complete!
echo ==================================
