cmake_minimum_required(VERSION 2.8.9)

set(PROGRAM_NAME solver)
set(SRC ./src)

project(${PROGRAM_NAME})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/Modules )

include_directories(${SRC})
include_directories (${SRC}/types)
include_directories (${SRC}/utils)
include_directories (gtest)
link_directories (lib)

add_executable(${PROGRAM_NAME} ${SRC}/utils/Helpers.cpp
															 ${SRC}/types/Literal.cpp
															 ${SRC}/types/Clause.cpp
															 ${SRC}/types/VarDatabase.cpp
															 ${SRC}/Solver.cpp
															 ${SRC}/Test.cpp)
target_link_libraries(${PROGRAM_NAME} gtest_main gtest pthread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -O2")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()