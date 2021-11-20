cmake_minimum_required(VERSION 3.10)
project(p_1)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Wno-deprecated -pedantic -pedantic-errors -O3 -Ofast -march=native)

# usado para detectar leaks de memoria
# https://github.com/google/sanitizers/wiki/AddressSanitizerLeakSanitizer
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")

add_executable(sim-aos sim.cpp obj.cpp util.cpp)
