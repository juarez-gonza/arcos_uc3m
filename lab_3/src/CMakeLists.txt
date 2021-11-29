cmake_minimum_required(VERSION 3.20)
project(counter)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

link_libraries(Threads::Threads)

add_compile_options(-Wall -Wextra -Werror -pedantic -pedantic-errors)
add_executable(counter counter.cpp)
