add_executable(driver driver.cpp)
target_link_libraries(driver vl53l1 pigpio pthread)
