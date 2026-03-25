cmake_minimum_required(VERSION 3.10)
project(test_liboqs)

find_package(liboqs REQUIRED)

add_executable(test_liboqs main.c)
target_link_libraries(test_liboqs liboqs)
