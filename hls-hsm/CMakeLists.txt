cmake_minimum_required(VERSION 3.20)
project(hls_hsm)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)
add_executable(hls_hsm src/main.cpp src/hsm.cpp)
