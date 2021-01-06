## WePaRT Source Code  

Source code is intended to be used along Arduino IDE.  
There is a source code for the base station and another one for the remote station.
You need to install the ESP32 support for Arduino IDE.

### Libraries to install

- [Thingspeak-Arduino](https://github.com/mathworks/thingspeak-arduino) (by Mathworks)
- [EasyNTPClient](https://github.com/aharshac/EasyNTPClient) (by Harsha Alva)
- [Time](https://github.com/PaulStoffregen/Time) (by Michael Margolis)
- [PubSubClient](https://github.com/knolleary/pubsubclient) (by Nick O’Leary)
- [Adafruit DHT sensor](https://github.com/adafruit/DHT-sensor-library)
- [Adafruit Unified Sensor](https://github.com/adafruit/Adafruit_Sensor)
- [Adafruit BME280](https://github.com/adafruit/Adafruit_BME280_Library)
- [LoRa](https://github.com/sandeepmistry/arduino-LoRa) (by Sandeep Mistry)
- [ESP8266 and ESP32 Oled Driver for SSD1306 Displays+](https://github.com/ThingPulse/esp8266-oled-ssd1306) (by ThingPulse)

If you want to use Pushover for Push Notifications, you must install also [Pushover by Arduino Hannover](https://github.com/ArduinoHannover/Pushover). You can copy the two files Pushover.cpp and Pushover.h in the `wepart_remote_station/` folder
