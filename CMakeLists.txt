cmake_minimum_required (VERSION 3.0)
project (ATtiny13a)

include_directories (include)
file (GLOB HEADERS include/*.h)
file (GLOB SOURCES source/*.cpp)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
add_executable (emulator ${SOURCES} ${HEADERS})
