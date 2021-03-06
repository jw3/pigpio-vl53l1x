/*
* Copyright (c) 2017, STMicroelectronics - All Rights Reserved
*
* This file is part of VL53L1 Core and is dual licensed,
* either 'STMicroelectronics
* Proprietary license'
* or 'BSD 3-clause "New" or "Revised" License' , at your option.
*
********************************************************************************
*
* 'STMicroelectronics Proprietary license'
*
********************************************************************************
*
* License terms: STMicroelectronics Proprietary in accordance with licensing
* terms at www.st.com/sla0081
*
* STMicroelectronics confidential
* Reproduction and Communication of this document is strictly prohibited unless
* specifically authorized in writing by STMicroelectronics.
*
*
********************************************************************************
*
* Alternatively, VL53L1 Core may be distributed under the terms of
* 'BSD 3-clause "New" or "Revised" License', in which case the following
* provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
********************************************************************************
*
*/


#include "vl53l1_platform.h"
// #include "vl53l1_platform_log.h"
#include "vl53l1_api.h"
// #include "stm32xxx_hal.h"

#include <chrono>
#include <thread>
#include <algorithm>

#include <pigpio.h>

// #include <time.h>
// #include <math.h>


// #define I2C_TIME_OUT_BASE   10
// #define I2C_TIME_OUT_BYTE   1

// #ifdef VL53L1_LOG_ENABLE
// #define trace_print(level, ...) VL53L1_trace_print_module_function(VL53L1_TRACE_MODULE_PLATFORM, level, VL53L1_TRACE_FUNCTION_NONE, ##__VA_ARGS__)
// #define trace_i2c(...) VL53L1_trace_print_module_function(VL53L1_TRACE_MODULE_NONE, VL53L1_TRACE_LEVEL_NONE, VL53L1_TRACE_FUNCTION_I2C, ##__VA_ARGS__)
// #endif

// #ifndef HAL_I2C_MODULE_ENABLED
// #warning "HAL I2C module must be enable "
// #endif

//extern I2C_HandleTypeDef hi2c1;
//#define VL53L0X_pI2cHandle    (&hi2c1)

/* when not customized by application define dummy one */
// #ifndef VL53L1_GetI2cBus
/** This macro can be overloaded by user to enforce i2c sharing in RTOS context
 */
// #   define VL53L1_GetI2cBus(...) (void)0
// #endif

// #ifndef VL53L1_PutI2cBus
/** This macro can be overloaded by user to enforce i2c sharing in RTOS context
 */
// #   define VL53L1_PutI2cBus(...) (void)0
// #endif

// uint8_t _I2CBuffer[256];

// int _I2CWrite(VL53L1_DEV Dev, uint8_t *pdata, uint32_t count) {
//     int status = 0;
//     return status;
// }

// int _I2CRead(VL53L1_DEV Dev, uint8_t *pdata, uint32_t count) {
//    int status = 0;
//    return Status;
// }

uint32_t H(VL53L1_DEV dev) {
   return dev->I2cHandle->dummy;
}

int write(VL53L1_DEV Dev, uint16_t index, uint8_t* pdata = nullptr, uint32_t count = 0) {
   const uint32_t sz = count + 2;

   uint8_t buffer[sz];
   buffer[0] = (uint8_t) ((index >> 8) & 0xFF); // msb
   buffer[1] = (uint8_t) (index & 0xFF);        // lsb

   if(count)
      memcpy(buffer + 2, pdata, count);

   const auto res = i2cWriteDevice(H(Dev), reinterpret_cast<char*>(&buffer), sz);
   return res == sz ? 0 : res;
}

VL53L1_Error VL53L1_WriteMulti(VL53L1_DEV Dev, uint16_t index, uint8_t* pdata, uint32_t count) {
   return write(Dev, index, pdata, count) ? -97 : VL53L1_ERROR_NONE;
}

// the ranging_sensor_comms.dll will take care of the page selection
VL53L1_Error VL53L1_ReadMulti(VL53L1_DEV Dev, uint16_t index, uint8_t* pdata, uint32_t count) {
   unsigned char cmd[2] = {
         (unsigned char) ((index >> 8) & 0xFF), // msb
         (unsigned char) (index & 0xFF)         // lsb
   };

   int status = i2cWriteDevice(H(Dev), reinterpret_cast<char*>(&cmd), 2);
   if(!status) {
      auto read = i2cReadDevice(H(Dev), reinterpret_cast<char*>(pdata), count);
      if(count >= 0)
         return VL53L1_ERROR_NONE;
      else
         return -80;
   }
   return -90;
}

VL53L1_Error VL53L1_WrByte(VL53L1_DEV Dev, uint16_t index, uint8_t data) {
   return write(Dev, index, &data, 1) ? -95 : VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WrWord(VL53L1_DEV Dev, uint16_t index, uint16_t data) {
   return write(Dev, index, reinterpret_cast<uint8_t*>(&data), 2) ? -94 : VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WrDWord(VL53L1_DEV Dev, uint16_t index, uint32_t data) {
   return write(Dev, index, reinterpret_cast<uint8_t*>(&data), 4) ? -93 : VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_UpdateByte(VL53L1_DEV Dev, uint16_t index, uint8_t AndData, uint8_t OrData) {
   return VL53L1_ERROR_NOT_IMPLEMENTED;
}

VL53L1_Error VL53L1_RdByte(VL53L1_DEV Dev, uint16_t index, uint8_t* data) {
   int status = write(Dev, index);
   if(!status) {
      auto val = i2cReadByte(H(Dev));
      if(val >= 0) {
         *data = static_cast<uint8_t>(val);
         return VL53L1_ERROR_NONE;
      }
   }
   return -90;
}

VL53L1_Error VL53L1_RdWord(VL53L1_DEV Dev, uint16_t index, uint16_t* data) {
   int status = write(Dev, index);
   if(!status) {
      auto val = i2cReadDevice(H(Dev), reinterpret_cast<char*>(data), 2);
      if(val >= 0)
         return VL53L1_ERROR_NONE;
   }
   return -89;
}

VL53L1_Error VL53L1_RdDWord(VL53L1_DEV Dev, uint16_t index, uint32_t* data) {
   int status = write(Dev, index);
   if(!status) {
      auto val = i2cReadDevice(H(Dev), reinterpret_cast<char*>(data), 4);
      if(val >= 0)
         return VL53L1_ERROR_NONE;
   }
   return -88;
}

VL53L1_Error VL53L1_GetTickCount(uint32_t* ptick_count_ms) {
   *ptick_count_ms = static_cast<uint32_t>(
         std::chrono::time_point_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now()
         ).time_since_epoch().count()
   );
   return VL53L1_ERROR_NONE;
}

//#define trace_print(level, ...) \
//    _LOG_TRACE_PRINT(VL53L1_TRACE_MODULE_PLATFORM, \
//    level, VL53L1_TRACE_FUNCTION_NONE, ##__VA_ARGS__)

//#define trace_i2c(...) \
//    _LOG_TRACE_PRINT(VL53L1_TRACE_MODULE_NONE, \
//    VL53L1_TRACE_LEVEL_NONE, VL53L1_TRACE_FUNCTION_I2C, ##__VA_ARGS__)

VL53L1_Error VL53L1_GetTimerFrequency(int32_t* ptimer_freq_hz) {
   return VL53L1_ERROR_NOT_IMPLEMENTED;
}

VL53L1_Error VL53L1_WaitMs(VL53L1_Dev_t* Dev, int32_t wait_ms) {
   std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms));
   return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WaitUs(VL53L1_Dev_t* Dev, int32_t wait_us) {
   std::this_thread::sleep_for(std::chrono::microseconds(wait_us));
   return VL53L1_ERROR_NONE;
}

VL53L1_Error VL53L1_WaitValueMaskEx(
      VL53L1_Dev_t* Dev,
      uint32_t timeout_ms,
      uint16_t index,
      uint8_t value,
      uint8_t mask,
      uint32_t poll_delay_ms) {

   uint8_t data;
   uint32_t start_time_ms;
   uint32_t current_time_ms;
   uint32_t polling_time_ms;

   VL53L1_GetTickCount(&start_time_ms);

   while(timeout_ms > 0) {
      if(auto ec = VL53L1_RdByte(Dev, index, &data)) return ec;
      if((data & mask) == value) return VL53L1_ERROR_NONE;

      std::this_thread::sleep_for(std::chrono::milliseconds(poll_delay_ms));
      VL53L1_WaitMs(Dev, poll_delay_ms);
      VL53L1_GetTickCount(&current_time_ms);
      timeout_ms -= current_time_ms - start_time_ms;
   }

   return -79;
}
