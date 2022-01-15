## Note
Some of these things may never get implemented.
And even if they do, there is no guarantee that these things are the next things I will implement.

## TODO
 * Switch sensor connection to hardware SPI?(if using software SPI atm, and if worth it)
 * Split sensor reading and data storage to different Tasks?(might not be worth it)
 * Add client with live graphs(Java standalone or Javascript in the web interface)
 * Show size of measurement csvs on downloads page
 * Automatically scale up sensor ranges if the read value is close to the current max
 * Show the actual measurement rate on the download page
 * Add SD data storage(might make extra task for data storage more worth it)
 * Consider using [SparkFun LSM9DS1 IMU Library](https://github.com/sparkfun/SparkFun_LSM9DS1_Arduino_Library) for reading data from the sensor
 * Consider using [Adafruit Sensor Lab](https://github.com/adafruit/Adafruit_SensorLab) / [Adafruit Sensor Calibration](https://github.com/adafruit/Adafruit_Sensor_Calibration) for more accurate sensor readings
 * Stop the esp from crashing when more then 2 downloads are attempted simultaneously
