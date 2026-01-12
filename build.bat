@echo off
setlocal enabledelayedexpansion

if "%1"=="" (
    echo Usage: build.bat [Debug^|Release]
    echo Default: Debug
    set BUILD_TYPE=Debug
) else (
    set BUILD_TYPE=%1
)

set SOLUTION_DIR=%~dp0
set BUILD_DIR=%SOLUTION_DIR%build
set OUTPUT_DIR=%SOLUTION_DIR%bin\%BUILD_TYPE%

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

pushd "%BUILD_DIR%"

echo Building LumX (%BUILD_TYPE%)...

cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed
    popd
    exit /b 1
)

cmake --build . --config %BUILD_TYPE% --parallel 4
if %ERRORLEVEL% neq 0 (
    echo Build failed
    popd
    exit /b 1
)

echo Build completed successfully
echo Output: %OUTPUT_DIR%

popd

if "%BUILD_TYPE%"=="Release" (
    echo.
    echo Creating Release package...
    if not exist "%SOLUTION_DIR%dist" mkdir "%SOLUTION_DIR%dist"
    copy "%OUTPUT_DIR%\LumX.exe" "%SOLUTION_DIR%dist\"
)

endlocal
