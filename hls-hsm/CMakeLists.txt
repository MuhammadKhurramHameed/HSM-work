cmake_minimum_required(VERSION 3.20)
project(hls_hsm)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

find_path(OQS_INCLUDE_DIR oqs/oqs.h PATHS /usr/local/include /usr/include REQUIRED)
find_library(OQS_LIB oqs PATHS /usr/local/lib /usr/lib /usr/lib/x86_64-linux-gnu REQUIRED)

add_executable(hls_hsm
    src/main.cpp
    src/hsm.cpp
)
target_include_directories(hls_hsm PRIVATE include)

add_executable(hls_pqc_demo
    src/pqc_demo.cpp
)
target_include_directories(hls_pqc_demo PRIVATE ${OQS_INCLUDE_DIR})
target_link_libraries(hls_pqc_demo PRIVATE ${OQS_LIB})
