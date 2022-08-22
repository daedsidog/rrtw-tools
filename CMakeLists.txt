cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

project("rrtw-tools")

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(EXTERN_INC_DIR ${CMAKE_SOURCE_DIR}/ext/include)

include_directories(SYSTEM ${EXTERN_INC_DIR})

link_directories(${CMAKE_SOURCE_DIR}/ext/lib)

add_library(common STATIC ${SRC_DIR}/common.cpp)
file(GLOB libs ext/lib/*)

add_executable(verificator ${SRC_DIR}/verificator.cpp)
target_link_libraries(common ${libs})
target_link_libraries(verificator common ${libs})
