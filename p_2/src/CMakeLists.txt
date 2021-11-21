cmake_minimum_required(VERSION 3.10)
project(p_2)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP REQUIRED)
if (OpenMP_CXX_FOUND)
	message("OPENMP FOUND")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()
add_compile_options(-Wall -Wextra -Wno-deprecated -pedantic -pedantic-errors -O3 -march=native -DOMP_NUM_THREADS=16
-fexcess-precision=fast -fno-rounding-math -fno-signaling-nans -fno-math-errno -fno-signed-zeros -fno-trapping-math)

add_executable(sim-soa sim.cpp obj.cpp util.cpp)
