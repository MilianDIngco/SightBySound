cmake_minimum_required(VERSION 3.22)

project(sbs VERSION 2.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set_executable(main main.cpp)

target_include_directories(sbs PUBLIC "${PROJECT_BINARY_DIR}")

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
