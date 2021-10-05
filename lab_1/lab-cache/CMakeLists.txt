cmake_minimum_required(VERSION 3.10)
project(cache_2021)

set(CMAKE_CXX_STANDARD 17)

add_executable(loop_merge loop_merge.cpp)
add_executable(loop_merge_opt loop_merge_opt.cpp)
add_executable(aos aos.cpp)
add_executable(soa soa.cpp)
