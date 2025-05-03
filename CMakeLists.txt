cmake_minimum_required(VERSION 3.31)
project(rmod)

set(CMAKE_CXX_STANDARD 20)

add_library(rmod SHARED library.cpp)
