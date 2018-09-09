cmake_minimum_required(VERSION 3.0)
project(pigpio_vl53l1x)
set(CMAKE_CXX_STANDARD 14)

include_directories(api)

add_library(vl53l1 SHARED impl/vl53l1_platform.cpp)
