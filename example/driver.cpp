#include <vl53l1_api.h>
#include <pigpio.h>

#include <string>
#include <iostream>

int main(int c, char** v) {
   int budget = 20000; // us
   int period = 55;    // ms
   if(c == 3) {
      budget = std::stoi(v[1]);
      period = std::stoi(v[2]);
   }
   printf("range settings:\tbudget=%d\tperiod=%d\n", budget, period);

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


   int ec;

   ec = VL53L1_WaitDeviceBooted(&Dev);
   if(ec) {
      std::cerr << "VL53L1_WaitDeviceBooted failed " << ec << std::endl;
      i2cClose(handle);
      return 1;
   }

   ec = VL53L1_DataInit(&Dev);
   if(ec) {
      std::cerr << "VL53L1_DataInit failed " << ec << std::endl;
      i2cClose(handle);
      return 1;
   }

   ec = VL53L1_StaticInit(&Dev);
   if(ec) {
      std::cerr << "VL53L1_StaticInit failed " << ec << std::endl;
      i2cClose(handle);
      return 1;
   }

   ec = VL53L1_SetPresetMode(&Dev, VL53L1_PRESETMODE_LOWPOWER_AUTONOMOUS);
   if(ec) {
      std::cerr << "VL53L1_SetPresetMode failed " << ec << std::endl;
      i2cClose(handle);
      return 1;
   }


   ec = VL53L1_SetDistanceMode(&Dev, VL53L1_DISTANCEMODE_SHORT);
   if(ec) {
      std::cerr << "VL53L1_SetDistanceMode failed " << ec << std::endl;
      i2cClose(handle);
      return 1;
   }

   ec = VL53L1_SetMeasurementTimingBudgetMicroSeconds(&Dev, 50000);
   if(ec) {
      std::cerr << "VL53L1_SetMeasurementTimingBudgetMicroSeconds failed " << ec << std::endl;
      i2cClose(handle);
      return 1;
   }

   ec = VL53L1_SetInterMeasurementPeriodMilliSeconds(&Dev, 55);
   if(ec) {
      std::cerr << "VL53L1_SetInterMeasurementPeriodMilliSeconds failed " << ec << std::endl;
      i2cClose(handle);
      return 1;
   }

   ec = VL53L1_StartMeasurement(&Dev);
   if(ec) {
      std::cerr << "VL53L1_StartMeasurement failed " << ec << std::endl;
      i2cClose(handle);
      return 1;
   }

   for(int i = 0; i < 100; ++i) {
      VL53L1_RangingMeasurementData_t range;

      ec = VL53L1_WaitMeasurementDataReady(&Dev);
      if(!ec) {
         printf("measurement data ready\n");
         ec = VL53L1_GetRangingMeasurementData(&Dev, &range);
         if(!ec)
            printf("range: %d [%d] mm\n", range.RangeMilliMeter, range.RangeStatus);
         ec = VL53L1_ClearInterruptAndStartMeasurement(&Dev);
         if(ec) {
            std::cerr << "VL53L1_ClearInterruptAndStartMeasurement failed " << ec << std::endl;
            i2cClose(handle);
            return 1;
         }
      }
      else {
         printf("=========================== VL53L1_WaitMeasurementDataReady failed with %d =============================", ec);
         break;
      }
   }

   i2cClose(handle);

   return 0;
}
