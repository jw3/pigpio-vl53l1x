#include <vl53l1_api.h>
#include <pigpio.h>

#include <iostream>

int main(int c, char** v) {
   if(gpioInitialise() < 0) {
      std::cout << "pigpio init failed" << std::endl;
      return 1;
   }

   auto handle = i2cOpen(1, 0x29, 0);
   if(handle < 0) {
      std::cerr << "i2c init failed" << std::endl;
      return 1;
   }
   std::cout << "i2c handle of " << handle << std::endl;

   VL53L1_Dev_t Dev;
   Dev.I2cHandle = new I2C_HandleTypeDef{static_cast<uint32_t>(handle)};

   VL53L1_software_reset(&Dev);

   uint8_t model_id;
   VL53L1_RdByte(&Dev, VL53L1_IDENTIFICATION__MODEL_ID, &model_id);
   printf("VL53L1X Model_ID: %d\n", model_id);


   int status;

   status = VL53L1_WaitDeviceBooted(&Dev);
   if(status) {
      std::cerr << "VL53L1_WaitDeviceBooted failed " << status << std::endl;
      i2cClose(handle);
      return 1;
   }

   status = VL53L1_DataInit(&Dev);
   if(status) {
      std::cerr << "VL53L1_DataInit failed " << status << std::endl;
      i2cClose(handle);
      return 1;
   }

   status = VL53L1_StaticInit(&Dev);
   if(status) {
      std::cerr << "VL53L1_StaticInit failed " << status << std::endl;
      i2cClose(handle);
      return 1;
   }

   status = VL53L1_SetDistanceMode(&Dev, VL53L1_DISTANCEMODE_LONG);
   if(status) {
      std::cerr << "VL53L1_SetDistanceMode failed " << status << std::endl;
      i2cClose(handle);
      return 1;
   }

   status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(&Dev, 50000);
   if(status) {
      std::cerr << "VL53L1_SetMeasurementTimingBudgetMicroSeconds failed " << status << std::endl;
      i2cClose(handle);
      return 1;
   }

   status = VL53L1_SetInterMeasurementPeriodMilliSeconds(&Dev, 500);
   if(status) {
      std::cerr << "VL53L1_SetInterMeasurementPeriodMilliSeconds failed " << status << std::endl;
      i2cClose(handle);
      return 1;
   }

   status = VL53L1_StartMeasurement(&Dev);
   if(status) {
      std::cerr << "VL53L1_StartMeasurement failed " << status << std::endl;
      i2cClose(handle);
      return 1;
   }


   VL53L1_RangingMeasurementData_t range;

   for(int i = 0; i < 100; ++i) {
      status = VL53L1_WaitMeasurementDataReady(&Dev);
      if(!status) {
         printf("measurement data ready\n");
         status = VL53L1_GetRangingMeasurementData(&Dev, &range);
         if(!status)
            printf("range: %d [%d] mm\n", range.RangeMilliMeter, range.RangeStatus);
         VL53L1_ClearInterruptAndStartMeasurement(&Dev);
      }

      VL53L1_WaitMs(&Dev, 100);
   }

   i2cClose(handle);

   return 0;
}
