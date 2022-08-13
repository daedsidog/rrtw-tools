cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)

project("daedsidog-tools")

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(EXTERN_INC_DIR ${CMAKE_SOURCE_DIR}/ext/include)

include_directories(SYSTEM ${EXTERN_INC_DIR})

add_library(common STATIC ${SRC_DIR}/common.cpp)
add_executable(verificator ${SRC_DIR}/verificator.cpp)
target_link_libraries(verificator common)
