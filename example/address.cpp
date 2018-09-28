#include <vl53l1_api.h>
#include <pigpio.h>

#include <string>
#include <chrono>
#include <thread>
#include <iostream>


int main(int c, char** v) {
   if(gpioInitialise() < 0) {
      std::cerr << "pigpio init failed" << std::endl;
      return 1;
   }

   // take device down
   gpioWrite(22, 0);
   std::this_thread::sleep_for(std::chrono::milliseconds(100));

   // bring device back up
   gpioWrite(22, 1);
   std::this_thread::sleep_for(std::chrono::milliseconds(100));

   auto addr2 = 0x10;

   {
      auto handle = i2cOpen(1, 0x29, 0);
      VL53L1_Dev_t Dev;
      Dev.I2cHandle = new I2C_HandleTypeDef{static_cast<uint32_t>(handle)};
      VL53L1_SetDeviceAddress(&Dev, addr2);
      i2cClose(handle);
   }
   {
      auto handle = i2cOpen(1, addr2, 0);
      if(handle < 0) {
         std::cerr << "i2c reinit failed" << std::endl;
         return 1;
      }

      VL53L1_Dev_t Dev;
      Dev.I2cHandle = new I2C_HandleTypeDef{static_cast<uint32_t>(handle)};

      uint8_t model_id;
      VL53L1_RdByte(&Dev, VL53L1_IDENTIFICATION__MODEL_ID, &model_id);
      std::cout << "VL53L1X Model_ID: 0x" << std::hex << static_cast<int>(model_id) << std::endl;

      i2cClose(handle);
   }

   return 0;
}
