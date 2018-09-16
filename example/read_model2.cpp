#include <pigpio.h>

#include <iostream>
#include <cstdio>
#include <thread>

#include <vl53l1_api.h>
#include <vl53l1_error_codes.h>
#include <vl53l1_register_map.h>

int main(int c, char** v) {
   std::cout << "example of manually booting the firmware using api" << std::endl;

   if(gpioInitialise() < 0) {
      std::cerr << "pigpio init failed" << std::endl;
      return 1;
   }

   int handle = i2cOpen(1, 0x29, 0);
   if(handle < 0) {
      std::cerr << "i2c init failed" << std::endl;
      return 1;
   }

   VL53L1_Dev_t Dev;
   Dev.I2cHandle = new I2C_HandleTypeDef{static_cast<uint32_t>(handle)};
   std::cout << "i2c handle of " << Dev.I2cHandle->dummy << std::endl;

   VL53L1_data_init(&Dev, 1);


   //-----------------------------------

   VL53L1_Error ec;
   unsigned char command[2];
   uint8_t firmware__system_status;

   ec = VL53L1_WrByte(
         &Dev,
         VL53L1_SOFT_RESET,
         0x00);

   if(ec) {
      printf("enter VL53L1_SOFT_RESET failed wth %d\n", ec);
      i2cClose(handle);
      return 1;
   }

   std::this_thread::sleep_for(std::chrono::milliseconds(1000));

   ec = VL53L1_WrByte(
         &Dev,
         VL53L1_SOFT_RESET,
         0x01);

   if(ec) {
      printf("leave VL53L1_SOFT_RESET failed wth %d\n", ec);
      i2cClose(handle);
      return 1;
   }

   // query firmware status
   // waiting 20 iterations then bailing

   for(int i = 0; i < 20 && !firmware__system_status; ++i) {
      ec =
            VL53L1_RdByte(
                  &Dev,
                  VL53L1_FIRMWARE__SYSTEM_STATUS,
                  &firmware__system_status);

      std::cerr << ".";
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
   }
   std::cerr << std::endl;

   if(ec) {
      printf("firmware failed to boot with %d\n", ec);
      i2cClose(handle);
      return 1;
   }

   printf("firmware status is %d\n", firmware__system_status);

   // firmware booted now query the model id
   command[0] = (unsigned char) ((VL53L1_IDENTIFICATION__MODEL_ID >> 8) & 0xFF);
   command[1] = (unsigned char) (VL53L1_IDENTIFICATION__MODEL_ID & 0xFF);

   i2cWriteDevice(handle, reinterpret_cast<char*>(&command), 2);
   unsigned char value = i2cReadByte(handle);

   printf("model: %d, expected %d\n", value, 0xEA);

   return 0;
}

