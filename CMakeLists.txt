cmake_minimum_required(VERSION 3.10)

project(qml_threadpool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(Qt5 REQUIRED COMPONENTS Core Quick Widgets)
find_package(GMP REQUIRED)

set(SOURCES
    src/main.cpp
    src/task_model.cpp
    src/thread_pool.cpp
)

set(QT_SOURCES
    # QML resource file
    qml/qml.qrc
    # Headers below required for MOC generations
    include/task_model.hpp
    include/task_info.hpp
)

add_executable(qml_threadpool ${SOURCES} ${QT_SOURCES})
target_include_directories(qml_threadpool PRIVATE include) 
target_link_libraries(qml_threadpool PRIVATE Qt5::Core Qt5::Quick Qt5::Widgets gmp gmpxx)