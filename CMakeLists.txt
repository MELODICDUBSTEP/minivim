cmake_minimum_required(VERSION 3.16)

project(minivim)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(MY_SOURCES
${PROJECT_SOURCE_DIR}/src/main.cpp)

add_executable(bin ${MY_SOURCES})

target_link_libraries(bin ${CURSES_LIBRARY})
