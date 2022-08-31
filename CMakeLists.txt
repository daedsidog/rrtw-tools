cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

project("rrtw-tools")

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(DCC_INC_DIR ${CMAKE_SOURCE_DIR}/ext/dcc/include)

include_directories(SYSTEM ${DCC_INC_DIR})

link_directories(${CMAKE_SOURCE_DIR}/ext/lib)

add_library(common STATIC ${SRC_DIR}/common.cpp)
file(GLOB dcc_libs ${CMAKE_SOURCE_DIR}/${DCC_LIB_DIR}/*.${STATIC_LIB_SUFFIX})

add_executable(verificator ${SRC_DIR}/verificator.cpp)
target_link_libraries(common ${dcc_libs})
target_link_libraries(verificator common ${dcc_libs})
