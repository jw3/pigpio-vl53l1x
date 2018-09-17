ST VL53L1X API implementation for pigpio
===

Using VL53L1X API [v2.3.3](https://www.st.com/content/st_com/en/products/embedded-software/proximity-sensors-software/stsw-img007.html)

### driver

The `example/driver` executable will perform a simple usage of the driver

`sudo ./example/driver 20000 25 | grep range`


### java bindings

The Travis build publishes a JNA based Java API for this driver to bintray

- `com.github.jw3:pigpio-vl53l1x:0.1.0`
- https://bintray.com/jw3/maven/pigpio-vl53l1x


### references
- [reference documentation](doc/README.md)
- https://www.pololu.com/product/3415
- http://www.robot-electronics.co.uk/i2c-tutorial
- https://www.st.com/resource/en/data_brief/stsw-img007.pdf
- https://community.st.com


### other implementations
- https://github.com/cassou/VL53L0X_rasp
- https://github.com/adafruit/Adafruit_VL53L0X
- https://github.com/pimoroni/vl53l1x-python
- https://github.com/pololu/vl53l1x-st-api-arduino


### ranging modes [reference](https://community.st.com/s/question/0D50X00009XkWSGSA3/vl53l1x-timing-issue)
- L53L1_PRESETMODE_LITE_RANGING
  - Does a range A; Provides result;  does a range B; provides Result
  - Needs the VL53L1_ClearInterruptAndStartMeasurement to continue – 10ms min timing budget in SHORT mode, 16ms with LONG mode
  - This is best for predictable timing. But the timing here is for only the range A or B, not both
- VL53L1_PRESETMODE_AUTONOMOUS
  - Does a range A, then range B
  - Needs the VL53L1_ClearInterruptAndStartMeasurement to continue
  - 46ms min timing budget
- VL53L1_PRESETMODE_LOWPOWER_AUTONOMOUS
  - Does range A, then B
  - No need to read or clear the interrup, will continue to range
  - 20ms min timing budget
  - Intermeasurement period should be 5ms longer than the timing budget

For consistent timing and the fastest results use LITE_RANGING mode.
