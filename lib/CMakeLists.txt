cmake_minimum_required(VERSION 2.8)
project(spout_cmd)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # require at least gcc 4.8
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.4)
        message(FATAL_ERROR "GCC version must be at least 4.4!")
    endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

include_directories(./include/)

set(SOURCE_FILES
    include/spout_cmd.hpp
    include/scsi_lli.hpp
    include/scsi_constants.hpp
    src/main.cpp
    src/spout_cmd.cpp
    src/scsi_lli.cpp)

add_executable(spout_cmd ${SOURCE_FILES})