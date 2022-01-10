# ESP32-Accelerometer
ESP32-Accelerometer is a tool to record acceleration, rotation, and magnetometer values using an ESP32 and a LSM9DS1.  
This is currently implemented so you first set for how long you want to record, then wait for it to finish, and then download the recordings.  
Because of this an ESP32 with 4+ MB of psram is required.  
This program reads the values from the LSM9DS1 using either I2C or software(?) SPI.  
Using I2C this program can reach a sample rate of ~230hz, using SPI it can reach the full almost 1000hz the sensor supports.

# Required Components
 * ESP32 with 4+MB psram(for example ESP32-DevKitCVE or ESP32-DevKitCVIE)
 * LSM9DS1

# Setup
 1. Install [PlatformIO](https://docs.platformio.org/en/latest/core/installation.html)
 2. Clone this git repository, for example using `git clone https://www.github.com/ToMe25/ESP32-Accelerometer.git/`
 3. Connect the LSM9DS1 and the ESP32 according to the table for your preferred protocol below this list.
    If you want to use I2C you also need to uncomment `#define LSM9DS1_I2C` in `src/LSM9DS1Handler.h` line 30.
 4. Attach the ESP32 to your PC
 5. Add a `wifissid.txt` and `wifipass.txt` file containing your wifi ssid and passphrase. Make sure they do not end with an empty line!
 6. Build and Upload using your IDE or by running `pio run --target upload`

Connections for I2C:  
(Temporary description until I can find an adequate tool to make circuit diagrams)  
|ESP32 Pin|LSM9DS1 Pin|
|---------|-----------|
|3V3      |VIN        |
|GND      |GND        |
|GPIO 22  |SCL        |
|GPIO 21  |SDA        |

Connections for SPI:  
(Temporary description until I can find an adequate tool to make circuit diagrams)  
|ESP32 Pin|LSM9DS1 Pin(s)|
|---------|--------------|
|3V3      |VIN           |
|GND      |GND           |
|GPIO 22  |SCL           |
|GPIO 21  |SDA           |
|GPIO 27  |CSAG          |
|GPIO 26  |CSM           |
|GPIO 32  |SDOAG, SDOM   |

# Usage
 1. Open http://esp-accelerometer/ in your browser
 2. Enter the measuring frequency and the number of measurements on this page<br/>
 ![pre-recording-settings](https://raw.githubusercontent.com/ToMe25/ESP32-Accelerometer/master/images/pre-recording-settings.png)
 3. Wait for this page to go away<br/>
 ![recording-please-wait](https://raw.githubusercontent.com/ToMe25/ESP32-Accelerometer/master/images/recording-please-wait.png)
 4. Download the data you are interested from this page<br/>
 ![recording-downloads](https://raw.githubusercontent.com/ToMe25/ESP32-Accelerometer/master/images/recording-downloads.png)
