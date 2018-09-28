#include <vl53l1_api.h>
#include <pigpio.h>

#include <string>
#include <chrono>
#include <thread>
#include <iostream>

constexpr int Dev1 = 22;
constexpr int Dev2 = 24;

constexpr unsigned char DefaultAddr = 0x29;
constexpr unsigned char Dev2Addr = 0x10;

void configure(VL53L1_Dev_t* Dev, int budget, int period);
void sample(VL53L1_Dev_t* Dev);

int main(int c, char** v) {
   int budget = 20000; // us
   int period = 55;    // ms
   if(c == 3) {
      budget = std::stoi(v[1]) * 1000;
      period = std::stoi(v[2]);
   }
   std::cout << "range timing budget (us): " << budget << std::endl;
   std::cout << "range timing period (ms): " << period << std::endl;

   if(gpioInitialise() < 0) {
      std::cerr << "pigpio init failed" << std::endl;
      return 1;
   }

   // take devices down
   gpioWrite(Dev1, 0);
   gpioWrite(Dev2, 0);
   std::this_thread::sleep_for(std::chrono::milliseconds(10));

   // bring device 1 back up
   gpioWrite(Dev1, 1);
   std::this_thread::sleep_for(std::chrono::milliseconds(10));

   {
      auto handle = i2cOpen(1, DefaultAddr, 0);
      VL53L1_Dev_t Dev;
      Dev.I2cHandle = new I2C_HandleTypeDef{static_cast<uint32_t>(handle)};
      VL53L1_SetDeviceAddress(&Dev, Dev2Addr * 2);
      i2cClose(handle);
   }

   auto handle1 = i2cOpen(1, DefaultAddr, 0);
   VL53L1_Dev_t Dev1;
   Dev1.I2cHandle = new I2C_HandleTypeDef{static_cast<uint32_t>(handle1)};

   // bring device 2 back up
   gpioWrite(Dev2, 1);
   std::this_thread::sleep_for(std::chrono::milliseconds(10));

   auto handle2 = i2cOpen(1, Dev2Addr, 0);
   VL53L1_Dev_t Dev2;
   Dev2.I2cHandle = new I2C_HandleTypeDef{static_cast<uint32_t>(handle2)};

   uint8_t model_id;
   VL53L1_RdByte(&Dev1, VL53L1_IDENTIFICATION__MODEL_ID, &model_id);
   std::cout << "Dev1 Model_ID: 0x" << std::hex << static_cast<int>(model_id) << std::endl;

   model_id = 0;
   VL53L1_RdByte(&Dev2, VL53L1_IDENTIFICATION__MODEL_ID, &model_id);
   std::cout << "Dev2 Model_ID: 0x" << std::hex << static_cast<int>(model_id) << std::endl;

   configure(&Dev1, budget, period);
   configure(&Dev2, budget, period);

   std::cout << "sampling dev1" << std::endl;
   sample(&Dev1);

   std::cout << "sampling dev2" << std::endl;
   sample(&Dev2);

   i2cClose(handle1);
   i2cClose(handle2);

   return 0;
}

void configure(VL53L1_Dev_t* Dev, int budget, int period) {
   int ec;

   ec = VL53L1_WaitDeviceBooted(Dev);
   ec = VL53L1_DataInit(Dev);
   ec = VL53L1_StaticInit(Dev);
   ec = VL53L1_SetPresetMode(Dev, VL53L1_PRESETMODE_LITE_RANGING);
   ec = VL53L1_SetDistanceMode(Dev, VL53L1_DISTANCEMODE_SHORT);
   ec = VL53L1_SetMeasurementTimingBudgetMicroSeconds(Dev, budget);
   ec = VL53L1_SetInterMeasurementPeriodMilliSeconds(Dev, period);
   ec = VL53L1_StartMeasurement(Dev);
}

void sample(VL53L1_Dev_t* Dev) {
   int ec;

   std::cout << std::dec;
   std::cout << "starting measurements" << std::endl;

   uint32_t prevRangeT = 0;
   for(int i = 0; i < 10; ++i) {
      VL53L1_RangingMeasurementData_t range;

      ec = VL53L1_WaitMeasurementDataReady(Dev);
      if(!ec) {
         ec = VL53L1_GetRangingMeasurementData(Dev, &range);
         if(!ec) {
            uint32_t rangeT;
            VL53L1_GetTickCount(&rangeT);
            auto interval = rangeT - prevRangeT;

            std::cout << "range: " << range.RangeMilliMeter
                      // quality not yet implemented
                      // << " quality: " << range.RangeQualityLevel
                      << " interval: " << interval;

            if(range.RangeStatus)
               std::cout << " status: " << static_cast<int>(range.RangeStatus);

            std::cout << std::endl;

            prevRangeT = rangeT;
         }
         ec = VL53L1_ClearInterruptAndStartMeasurement(Dev);
         if(ec)
            std::cerr << "VL53L1_ClearInterruptAndStartMeasurement failed with " << ec << std::endl;
      }
      else
         std::cerr << "VL53L1_WaitMeasurementDataReady failed with " << ec << std::endl;
   }
}
