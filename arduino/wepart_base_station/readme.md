## WePaRT Base Station source code

This is the code you must upload on the module in the Base Station. The Base Station is the one where Sensors are attached, aka the Transmitter Station.  

Please remember those things first than upload the code:  
- Remove the connection between SDS011 and board. The SDS011 implementation uses the hardware UART, the same used for code upload
- Remove the microSD card from the slot
- Select the right board in Arduino IDE: ESP32 Arduino > TTGO LoRa32-OLED v1
- Select the right COM port in Arduino IDE
- Install the right libraries
