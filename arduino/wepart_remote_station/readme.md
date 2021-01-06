## WePaRT Remote Station source code

This is the code you must upload on the module in the Remote Station. The Remote Station is the one connected to WiFi and that receives data over LoRa from the Base for sending them over MQTT and Thingspeak, aka the Receiver Station.

Please remember those things first than upload the code:  
- Remove the microSD card from the slot
- Select the right board in Arduino IDE: <kbd>ESP32 Arduino > TTGO LoRa32-OLED v1</kbd>
- Select the right COM port in Arduino IDE
- Install the right libraries
