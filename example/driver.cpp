#include <vl53l1_api.h>
#include <iostream>

int main(int c, char** v)
{
   VL53L1_Dev_t Dev;

   Dev.I2cDevAddr = 0x52;
   VL53L1_software_reset(&Dev);

   uint8_t byteData;
   VL53L1_RdByte(&Dev, 0x010F, &byteData);

   std::cout << "VL53L1X Model_ID: " << byteData << std::endl;

   return 0;
}
