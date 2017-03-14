cmake_minimum_required(VERSION 3.6)
project(broadcast)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES main.c)
add_executable(broadcast ${SOURCE_FILES})

find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
target_link_libraries(broadcast ${MPI_C_LIBRARIES})