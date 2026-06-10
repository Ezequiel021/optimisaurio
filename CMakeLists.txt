cmake_minimum_required(VERSION 3.20)
project(opti VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(MPI REQUIRED COMPONENTS CXX)

add_executable(opti src/opti.cpp)

target_link_libraries(opti PRIVATE MPI::MPI_CXX)
