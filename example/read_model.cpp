#include <pigpio.h>

#include <iostream>
#include <cstdio>
#include <thread>

#include <vl53l1_error_codes.h>
#include <vl53l1_register_map.h>

int main(int c, char** v) {
   std::cout << "example of manually booting the firmware" << std::endl;

   if(gpioInitialise() < 0) {
      std::cerr << "pigpio init failed" << std::endl;
      return 1;
   }

   int handle = i2cOpen(1, 0x29, 0);
   if(handle < 0) {
      std::cerr << "i2c init failed" << std::endl;
      return 1;
   }
   std::cout << "i2c handle of " << handle << std::endl;

   //-----------------------------------

   int status;
   unsigned char command[2];
   uint8_t firmware__system_status;

   i2cWriteDevice(handle, reinterpret_cast<char*>(&command), 2);

   status = i2cWriteByteData(handle, VL53L1_SOFT_RESET, 0x00) ? VL53L1_ERROR_UNDEFINED : VL53L1_ERROR_NONE;
   if(status) {
      std::cerr << "enter VL53L1_SOFT_RESET failed" << std::endl;
      i2cClose(handle);
      return 1;
   }

   std::this_thread::sleep_for(std::chrono::milliseconds(1000));

   status = i2cWriteByteData(handle, VL53L1_SOFT_RESET, 0x01) ? VL53L1_ERROR_UNDEFINED : VL53L1_ERROR_NONE;
   if(status) {
      std::cerr << "leave VL53L1_SOFT_RESET failed" << std::endl;
      i2cClose(handle);
      return 1;
   }

   // query firmware status
   // waiting 20 iterations then bailing

   command[0] = 0x00;
   command[1] = VL53L1_FIRMWARE__SYSTEM_STATUS;

   for(int i = 0; i < 20 && !status; ++i) {
      i2cWriteDevice(handle, reinterpret_cast<char*>(&command), 2);
      status = i2cReadByte(handle);
      std::cerr << ".";
   }
   std::cerr << std::endl;

   if(!status) {
      std::cerr << "firmware failed to boot" << std::endl;
      i2cClose(handle);
      return 1;
   }

   printf("firmware status is %d\n", status);

   // firmware booted now query the model id
   command[0] = (unsigned char) ((VL53L1_IDENTIFICATION__MODEL_ID >> 8) & 0xFF);
   command[1] = (unsigned char) (VL53L1_IDENTIFICATION__MODEL_ID & 0xFF);

   i2cWriteDevice(handle, reinterpret_cast<char*>(&command), 2);
   unsigned char value = i2cReadByte(handle);

   printf("model: %d, expected %d\n", value, 0xEA);

   return 0;
}

