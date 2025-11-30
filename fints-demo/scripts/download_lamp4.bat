@echo off
REM Download LaMP-4 dataset for Windows

echo ==================================
echo Downloading LaMP-4 Dataset
echo ==================================
echo.

if not exist "lamp_data" mkdir lamp_data
if not exist "lamp_data\LaMP_4" mkdir lamp_data\LaMP_4

cd lamp_data\LaMP_4

REM URLs
set DEV_Q=https://raw.githubusercontent.com/LaMP-Benchmark/LaMP/main/data/LaMP_4/dev_questions.json
set DEV_O=https://raw.githubusercontent.com/LaMP-Benchmark/LaMP/main/data/LaMP_4/dev_outputs.json
set TRAIN_Q=https://raw.githubusercontent.com/LaMP-Benchmark/LaMP/main/data/LaMP_4/train_questions.json
set TRAIN_O=https://raw.githubusercontent.com/LaMP-Benchmark/LaMP/main/data/LaMP_4/train_outputs.json

echo Downloading dev_questions.json (this may take a while, ~72MB)...
curl -L -o dev_questions.json %DEV_Q%
if %errorlevel% neq 0 (
    echo ERROR: Failed to download dev_questions.json
    exit /b 1
)

echo Downloading dev_outputs.json...
curl -L -o dev_outputs.json %DEV_O%
if %errorlevel% neq 0 (
    echo ERROR: Failed to download dev_outputs.json
    exit /b 1
)

echo Downloading train_questions.json (this may take a while, ~788MB)...
curl -L -o train_questions.json %TRAIN_Q%
if %errorlevel% neq 0 (
    echo ERROR: Failed to download train_questions.json
    exit /b 1
)

echo Downloading train_outputs.json...
curl -L -o train_outputs.json %TRAIN_O%
if %errorlevel% neq 0 (
    echo ERROR: Failed to download train_outputs.json
    exit /b 1
)

cd ..\..

echo.
echo ==================================
echo Download complete!
echo ==================================
echo Files saved to: lamp_data\LaMP_4\
echo.
echo Next: Run evaluation with scripts\run_lamp.bat
echo ==================================
