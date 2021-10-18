cmake_minimum_required(VERSION 3.10)
project(p_1)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Wno-deprecated -pedantic -pedantic-errors -O3 -Ofast -march=native)

add_executable(sim-soa sim.cpp obj.cpp util.cpp)
