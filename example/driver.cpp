#include <vl53l1_api.h>
#include <pigpio.h>

#include <iostream>

int main(int c, char** v) {
   if(gpioInitialise() < 0) {
      std::cout << "pigpio init failed" << std::endl;
      return 1;
   }

   VL53L1_Dev_t Dev;
   auto pigpioI2c = i2cOpen(1, 0x52, 0);
   Dev.I2cHandle = new I2C_HandleTypeDef{static_cast<uint32_t>(pigpioI2c)};;

   Dev.I2cDevAddr = 0x52;
   VL53L1_software_reset(&Dev);

   uint8_t byteData;
   VL53L1_RdByte(&Dev, 0x010F, &byteData);

   std::cout << "VL53L1X Model_ID: " << byteData << std::endl;

   return 0;
}
