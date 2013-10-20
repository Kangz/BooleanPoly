cmake_minimum_required(VERSION 2.6)

project(Poly)

SET(CMAKE_CXX_FLAGS "-g -O4 -Wall -Wextra -pedantic -std=c++11")

set(poly_sources poly.cpp utils.cpp bit_utils.cpp)

add_executable(poly main.cpp ${poly_sources})
