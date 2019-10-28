cmake_minimum_required(VERSION 3.14)
project(DragonsLakeEntertainmentHackaton)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_DIR "./")

include_directories(includes source)
link_directories(includes source)

file(GLOB all
        "${SOURCE_DIR}/source/*.cpp"
        "${SOURCE_DIR}/includes/*.hpp")

add_executable(${PROJECT_NAME} ${all} source/IgorStalevskiyPathFinder.cpp includes/IgorStalevskiyPathFinder.hpp)
