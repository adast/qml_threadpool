cmake_minimum_required(VERSION 3.10)

project(qml_threadpool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(GMP REQUIRED)

set(SOURCES src/main.cpp)

add_executable(qml_threadpool ${SOURCES})
target_include_directories(qml_threadpool PRIVATE include) 
target_link_libraries(qml_threadpool PRIVATE gmp gmpxx)