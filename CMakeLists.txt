cmake_minimum_required(VERSION 3.6)
project(tpr)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

# MPI
find_package(MPI REQUIRED)
include_directories(SYSTEM ${MPI_INCLUDE_PATH})

# broadcast
set(BROADCAST_FILES broadcast/main.c)
add_executable(broadcast ${BROADCAST_FILES})
target_link_libraries(broadcast ${MPI_C_LIBRARIES})

# perf_meas
set(PERFMEAS_FILES perf_meas/main.c)
add_executable(perf_meas ${PERFMEAS_FILES})
target_link_libraries(perf_meas ${MPI_C_LIBRARIES})

# reduce
set(REDUCE_FILES reduce/main.c)
add_executable(reduce ${PERFMEAS_FILES})
target_link_libraries(reduce ${MPI_C_LIBRARIES})

# openmp
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(OPENMP_FILES openmp/main.cpp)
add_executable(openmp ${OPENMP_FILES})

set(PREPROC_OPENMP_FILES openmp/preprocessed.cpp)
add_executable(preproc_openmp ${PREPROC_OPENMP_FILES})