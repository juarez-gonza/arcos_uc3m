cmake_minimum_required(VERSION 3.20)
project(counter)

set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

link_libraries(Threads::Threads)

add_compile_options(-Wall -Wextra -Werror -pedantic -pedantic-errors)

add_executable(counter counter.cpp)

add_executable(counter_mutex counter_mutex.cpp)
add_executable(counter_atomic counter_atomic.cpp)

add_executable(counter_spin_seq counter_spin_seq.cpp)
add_executable(counter_spin_seq_opt counter_spin_seq_opt.cpp)

add_executable(counter_spin_ra counter_spin_ra.cpp)
add_executable(counter_spin_ra_opt counter_spin_ra_opt.cpp)
