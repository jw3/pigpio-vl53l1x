cmake_minimum_required(VERSION 3.0)
project(pigpio_vl53l1x)
set(CMAKE_CXX_STANDARD 14)

include_directories(api api/platform)
add_subdirectory(impl)
add_subdirectory(example)
