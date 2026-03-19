@echo off
setlocal

set "OPENCL=OFF"
set "VULKAN=OFF"
:parse
if "%~1"=="" goto done
if /i "%~1"=="--opencl" ( set "OPENCL=ON" & shift & goto parse )
if /i "%~1"=="--vulkan" ( set "VULKAN=ON" & shift & goto parse )
echo Usage: build.cmd [--opencl] [--vulkan] & exit /b 1
:done

set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
for %%I in ("%SCRIPT_DIR%\..") do set "PROJECT_ROOT=%%~fI"
set "VENV_DIR=%PROJECT_ROOT%\.venv"

if not exist "%VENV_DIR%\Scripts\activate.bat" (
    python -m venv "%VENV_DIR%"
)
call "%VENV_DIR%\Scripts\activate.bat"

python -m pip install --upgrade pip setuptools wheel -q
set "CMAKE_GENERATOR=MinGW Makefiles"
set "SKBUILD_CMAKE_ARGS=-DINDK_OPENCL_SUPPORT=%OPENCL%;-DINDK_VULKAN_SUPPORT=%VULKAN%"
python -m pip install "%SCRIPT_DIR%" -v

echo.
echo Test:
echo    %VENV_DIR%\Scripts\activate.bat
echo    cd .\samples
echo    python -c "import indk; print(indk.__version__)"

endlocal