# ESP32-Accelerometer
ESP32-Accelerometer is a tool to record acceleration, rotation, and magnetometer values using an ESP32 and a LSM9DS1.<br/>
This is currently implemented so you first set for how long you want to record, then wait for it to finish, and then download the recordings.<br/>
Because of this an ESP32 with 4+ MB of psram is required.<br/>
This program reads the values from the LSM9DS1 using I2C, and makes them available on a web server.<br/>
The maximum measuring frequency this program can handle over I2C is ~100hz, it might be possible to increase this by using SPI in the future.

# Required Components
 * ESP32 with 4+MB psram(for example ESP32-DevKitCVE or ESP32-DevKitCVIE)
 * LSM9DS1

# Setup
 1. Install [PlatformIO](https://docs.platformio.org/en/latest/core/installation.html)
 2. Clone this git repository, for example using `git clone https://www.github.com/ToMe25/ESP32-Accelerometer.git/`
 3. Connect the LSM9DS1 and the ESP32 according to the table below this list
 4. Attach the ESP32 to your PC
 5. Add a `wifissid.txt` and `wifipass.txt` file containing your wifi ssid and passphrase. Make sure they do not end with an empty line!
 6. Build and Upload using your IDE or by running `pio run --target upload`

Connections:<br/>
(Temporary description until I can find an adequate tool to make circuit diagrams)<br/>
|ESP32 Pin|LSM9DS1 Pin|
|---------|-----------|
|3V3      |VIN        |
|GND      |GND        |
|GPIO 22  |SCL        |
|GPIO 21  |SDA        |

# Usage
 1. Open http://esp-accelerometer/ in your browser
 2. Enter the measuring frequency and the number of measurements on this page<br/>
 ![pre-recording-settings](https://raw.githubusercontent.com/ToMe25/ESP32-Accelerometer/master/images/pre-recording-settings.png)
 3. Wait for this page to go away<br/>
 ![recording-please-wait](https://raw.githubusercontent.com/ToMe25/ESP32-Accelerometer/master/images/recording-please-wait.png)
 4. Download the data you are interested from this page<br/>
 ![recording-downloads](https://raw.githubusercontent.com/ToMe25/ESP32-Accelerometer/master/images/recording-downloads.png)
