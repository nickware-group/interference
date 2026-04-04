
set "OPENCL=OFF"
set "VULKAN=OFF"
:parse
if "%~1"=="" goto done
if /i "%~1"=="--opencl" ( set "OPENCL=ON" & shift & goto parse )
if /i "%~1"=="--vulkan" ( set "VULKAN=ON" & shift & goto parse )
echo Usage: build.cmd [--opencl] [--vulkan] & exit /b 1
:done

mkdir cmake-build
cd cmake-build
cmake -DCMAKE_BUILD_TYPE=Release -DINDK_OPENCL_SUPPORT=%OPENCL% -DINDK_VULKAN_SUPPORT=%VULKAN% -DCMAKE_MAKE_PROGRAM=mingw32-make -G "MinGW Makefiles" ..
mingw32-make install
pause
