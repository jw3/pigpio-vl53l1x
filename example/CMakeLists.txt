add_executable(driver driver.cpp)
target_link_libraries(driver vl53l1 pigpio pthread)

add_executable(address address.cpp)
target_link_libraries(address vl53l1 pigpio pthread)
