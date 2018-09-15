add_executable(driver driver.cpp)
target_link_libraries(driver vl53l1 pigpio pthread)

add_executable(read_model read_model.cpp)
target_link_libraries(read_model pigpio pthread)

add_executable(read_model2 read_model2.cpp)
target_link_libraries(read_model2 vl53l1 pigpio pthread)
