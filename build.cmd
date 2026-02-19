mkdir cmake-build
cd cmake-build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=mingw32-make -G "MinGW Makefiles" ..
mingw32-make install
pause
