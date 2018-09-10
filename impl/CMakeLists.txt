add_library(vl53l1 SHARED ${CMAKE_SOURCE_DIR}/impl/vl53l1_platform.cpp)

FILE(GLOB api_sources ${CMAKE_SOURCE_DIR}/api/*.c)
target_sources(vl53l1 PRIVATE ${api_sources})
