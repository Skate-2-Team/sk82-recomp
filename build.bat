@echo off

mkdir build
cd build

cmake .. -G "MinGW Makefiles" ^
  -DCMAKE_C_COMPILER=clang ^
  -DCMAKE_CXX_COMPILER=clang++ ^
  -DCMAKE_BUILD_TYPE=Debug ^
  -DCMAKE_CXX_FLAGS_DEBUG="-g"

cmake --build . -- -j%NUMBER_OF_PROCESSORS%

