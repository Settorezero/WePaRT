# WePaRT

![WePaRT diagram](./media/wepart_diagram.png)

This project is based on the LiLyGO TTGO LoRa32 version 2.1_1.6 and is composed of two parts:  
- A base station
- A remote station

### Base Station
Think of this as a station that is placed where WiFi and Telephone is not accessible.  
Is the one where sensors are attached. Actually those sensors are used:  
- DHT22 (humidity, temperature)
- BME280 (pressure, temperature, humidity) Or BMP280 (pressure, temperature)
- SDS011 (particulate matter: PM10 and PM2.5)  

Base station gets data from sensors and:
- Save them on a microSD card in CSV format
- propagate them through LoRa to the remote station
- act as an access point for showing real-time data on a webpage

### Remote Station
Is the one attached to a WiFi infrastructure. Receives data from the Base Station and:
- transmits them to Thingspeak for graph generation
- transmits them over MQTT
- saves the good received data in a CSV, adding the current time/date taken from an NTP server
- saves the bad received data in a text file for further analisys of bad received packets
- shows a webpage by navigating to his IP address in the same wifi network were device is connected
