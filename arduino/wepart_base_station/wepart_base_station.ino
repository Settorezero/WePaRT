/*
 * WePaRT - Weather and Particulate Recorder Transmitter
 * BASE STATION
 * board used: LILYGO® TTGO LoRa32 V2.1_1.6
 * 
 * Code and design by Bernardo Giovanni aka CyB3rn0id (https://www.settorezero.com)
 * If you want to share the code please mention: Giovanni Bernardo of settorezero.com
 * after sending me a message
 *
 * For further information please read the article at:
 * https://www.settorezero.com/wordpress/wepart-stazione-monitoraggio-meteo-polveri-sottili-particolato
 *
 * Repo URL:
 * https://github.com/Settorezero/WePaRT
 *
 * Select the board ESP32 Arduino > TTGO LoRa32-OLED v1 
 * 
 * Libraries required for this part of code:
 * Adafruit DHT Sensor: https://github.com/adafruit/DHT-sensor-library
 * Adafruit Unified Sensor: https://github.com/adafruit/Adafruit_Sensor
 * Adafruit BME280: https://github.com/adafruit/Adafruit_BME280_Library
 * LoRa by Sandeep Mistry: https://github.com/sandeepmistry/arduino-LoRa
 * ESP8266 and ESP32 Oled Driver for SSD1306 Displays by Thingpulse: https://github.com/ThingPulse/esp8266-oled-ssd1306
 * 
 * LICENSE
 * Attribution-NonCommercial-ShareAlike 4.0 International 
 * (CC BY-NC-SA 4.0)
 * 
 * WARNING: this license may be valid for the project (design, schematics, diagrams, own documentation, solutions to problems, ideas)
 * and for my own written code (some routines and ideas). 
 * Is not appliable for used libraries and commercial used hardware.
 * Please read licenses for every library used and hardware.
 * 
 * This is a human-readable summary of (and not a substitute for) the license:
 * You are free to:
 * SHARE — copy and redistribute the material in any medium or format
 * ADAPT — remix, transform, and build upon the material
 * The licensor cannot revoke these freedoms as long as you follow the license terms.
 * Under the following terms:
 * ATTRIBUTION — You must give appropriate credit, provide a link to the license, 
 * and indicate if changes were made. You may do so in any reasonable manner, 
 * but not in any way that suggests the licensor endorses you or your use.
 * NON COMMERCIAL — You may not use the material for commercial purposes.
 * SHARE ALIKE — If you remix, transform, or build upon the material,
 * you must distribute your contributions under the same license as the original.
 * NO ADDITIONAL RESTRICTIONS — You may not apply legal terms or technological 
 * measures that legally restrict others from doing anything the license permits.
 * 
 * Warranties
 * The Licensor offers the Licensed Material as-is and as-available, and makes
 * no representations or warranties of any kind concerning the Licensed Material, 
 * whether express, implied, statutory, or other. This includes, without limitation, 
 * warranties of title, merchantability, fitness for a particular purpose, 
 * non-infringement, absence of latent or other defects, accuracy, or the presence
 * or absence of errors, whether or not known or discoverable. Where disclaimers 
 * of warranties are not allowed in full or in part, this disclaimer may not apply to You.
 * 
 * Please read the Full License text at the following link:
 * https://creativecommons.org/licenses/by-nc-sa/4.0/legalcode
 *
 */
#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h> 
#include <SSD1306Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "DHT.h"

#include "board_defs.h" // in this header I've defined the pins used by the module LILYGO® TTGO LoRa32 V2.1_1.6
#include "images.h"
#include "my_fonts.h"
#include "webpage.h"

// START OF CUSTOMIZE SECTION --------------------------------------------------------------------------------------------------------------------

// data used for creating the access point
const char* ssid="WePaRT (192.168.4.1)"; // Service Set IDentifier : aka the WiFi Nice Name
const char* password=""; // leave blank you want to access using a password

// uncomment for verbose messages on the serial port (9600bps)
// notice: serial port is also used by SDS011 so you'll see unrecognizable chars sometimes
// since the SDS011 will transmit data
//#define DEBUG

// remove comment if you want a fast-startup (no delays, nor logos during initializazion - used mainly for a rapid check of the code after debug)
//#define FASTINIT

// Select which kind of pressure sensor you're using
// so comment what you're NOT using and uncomment what you're using
// the BME280 measures pressure, temperature and humidity
// the BMP280 measures pressure and temperature
// read more on my article: https://www.settorezero.com/wordpress/sensori-di-pressione-bme280-e-bmp280/
#define USE_BME280
//#define USE_BMP280

// BME280/BMP280 I2C 7bit address
// normally the BME280/BMP280 breakout boards (use the one having 6 pin)
// has address 0x76, the one from Adafruit uses 0x77
#define PRESS_SENS_ADDR 0x76

// normal pressure at 25° at my altidude
// I used the calculator at https://www.mide.com/air-pressure-at-altitude-calculator
// not used in the code, here for future implementations
float nPressure=983.4253; // hPa = mBar

// URL where wifi users can obtain informations about the project
String infourl="https://thingspeak.com/channels/1107465/";

// number of readings on sensors (SDS011 excluded) that will be averaged
// data will be transmitted+recorded every AVERAGE_SENSORS readings
// or when SDS011 has data available
// not reccomended to lower this value: you can send max 1 message every 11 seconds
// to thingspeak, so this value will give more accurate and stable results
// and also some delay. We'll add other delay for Thingspeak using a delay function.
#define AVERAGE_SENSORS 200

// comment if you don't want CSV can be downloadable from the webpage
//#define ALLOW_DOWNLOAD

// stuff used by DHT22 temperature/humidity sensor
#define DHTPIN 4 // (pin 34 don't work, pin 12 gives "A fatal error occurred: MD5 of file does not match data in flash!")
#define DHTTYPE DHT22

// stuff used by my SDS011 Particulate Matter sensor implementation
#define SDS_TIMEOUT 5 // seconds for no-response at initialization
#define SDS_MINUTES 10 // from 1 to 30. 0=works continuously - really not reccomended

// END OF CUSTOMIZE SECTION ----------------------------------------------------------------------------------------------------------------------

#define SDS_OK 0
#define SDS_NODATA  1
#define SDS_CHECKSUM 2
#define SDS_INVALID 3

#ifdef USE_BME280
  #define FIELDS 12
#endif
#ifdef USE_BMP280
  #define FIELDS 11
#endif

String packet; // packet to be sent over LoRa, to be saved, eventually, on the SDcard and used for Ajax refresh of html page
bool firstStart=true; // first time loop starts
bool sdpresent=false; // keep in mind if there is an SD mounted
String dataFile=""; // file name where data are saved on the SD
long lastTimeStamp=0; // last time data was saved
bool stoprecording=false; // SD full, not used, here for future implementations
float battery=0; // battery voltage (battery is not used in the project, but leaved if you want do something)

// data from BME280/BMP280
float pressure=0;
float temperatureB=0;
#ifdef USE_BME280
    float humidityB=0;
#endif

// data from DHT22
float temperatureD=0;
float humidityD=0;

// data from SDS011
float pm10=0;
float pm25=0;

// objects
SSD1306Wire display(OLED_ADDRESS, OLED_SDA, OLED_SCL);
SPIClass sdSPI(HSPI); // we'll use the SD card on the HSPI (SPI2) module
SPIClass loraSPI(VSPI); // we'll use the LoRa module on the VSPI (SPI3) module
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BME280 bme; // I2C used, the same of OLED
WebServer server(80);

void setup() 
  {
  // Begins some pins
  pinMode(LED,OUTPUT); // Green Led on the board
  pinMode(VBAT,INPUT); // Input for Battery Voltage

  bool someerror=false; // keep in mind if there are initializazion errors

  /*******************************************************************************************************************
   * UART setup
   * 9600baud, used for SPS011 and eventually DEBUG
   ******************************************************************************************************************/
  Serial.begin(9600); // 9600 needed for the SDS011
  while (!Serial);
  delay(100);
  #ifdef DEBUG
    Serial.println();
    Serial.println("WePaRT - Weather and Particulate Recorder Transmitter");
    Serial.println("BASE STATION");
    Serial.println("by Giovanni Bernardo");
    Serial.println("https://www.settorezero.com");
    Serial.println();
    Serial.flush();
    delay(200);
  #endif

  /*******************************************************************************************************************
   * OLED display setup
   * 0.96", 128x64, connected on the I2C
   ******************************************************************************************************************/
  display.init();
  // uncomment the following setting if you want to read the display keeping the antenna on top-right
  // display.flipScreenVertically(); 
  display.clear();
  delay(100); 
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Monospaced_bold_11);
  #ifdef DEBUG
    Serial.println("OLED initialized");
  #endif

  /*******************************************************************************************************************
   * SDS011 setup
   * No libraries used
   ******************************************************************************************************************/
  // Send the command for setting working period. I'll transmit following bytes:
  // AA : header
  // B4 : Command ID (always B4)
  // 08 : Data byte 0x08 is "Set Working Period"
  // 01 : WRITE the setting (READ the setting requires 0 instead)
  // [SDS_MINUTES] : SDS011 will work every SDS_MINUTES minutes
  // [ten 0s] : reserved/not used values
  // FF and FF : command sent to all devices on the line
  // [sdschk] : checksum (bytes from 0x08 to the last 0xFF) = sum of all bytes, then take only the lower byte
  // 0xAB : tail
  uint8_t sdschk=char((0x08+0x01+SDS_MINUTES+0xFF+0xFF)&0xFF); // checksum including all datas but head,command id and tail
  uint8_t sdscom[19]={0xAA,0xB4,0x08,0x01,SDS_MINUTES,0,0,0,0,0,0,0,0,0,0,0xFF,0xFF,sdschk,0xAB}; // command sent over serial port
  display.drawString(0,0,"Waiting SDS011");
  display.display();
  // purge RX buffer
  Serial.read();
  Serial.flush();
  bool uarttimeout=false;
  long timecycle=millis();
  uint8_t sec=1;
  #ifdef DEBUG
    Serial.println("Sending Command to SDS011. Wait for response");
  #endif
  delay(500);
  Serial.write(sdscom,19); // set active mode with SDS_MINUTES working period (buffer: sdscom, 19 bytes)
  // now I wait for a response from the SDS011.
  // if nothing is attached on the UART, so there are no response, I'll exit from the cycle
  // after SDS_TIMEOUT seconds
  while(1)
    {
    if (Serial.available()>9) break; // I expect 10 bytes
    if ((millis()-timecycle)>(sec*1000)) // 1 second passed
      {
      sec++;
      if (sec==SDS_TIMEOUT)
        {
        uarttimeout=true;
        break;    
        }
      // re-send command
      Serial.write(sdscom,19);
      }
    }
  display.clear();
  display.display();
  // we've at least 10 bytes in the UART buffer
  if (!uarttimeout)
    {
    // sensor must respond "AA C5 08 01 [SDS_MINUTES] 00 [ID 1] [ID 2] [checksum] AB"
    uint8_t i=Serial.readBytes(sdscom, 10);
    #ifdef DEBUG
      Serial.print("Received from SDS011: ");
      for (uint8_t u=0; u<i; u++)
        {
        Serial.print(sdscom[u],HEX);  
        Serial.print(' ');
        }
      Serial.println();
    #endif
    // check the correct format of the response, this time I'll not verify the checksum  
    if ((sdscom[0]==0xAA) && (sdscom[1]==0xC5) && (sdscom[2]==0x08) && (sdscom[4]==SDS_MINUTES) && (sdscom[9]==0xAB))
      {
      #ifdef DEBUG
        Serial.println("SDS011 success");
      #endif
      display.drawString(0,0,"SDS011 OK"); 
      }
    else
      {
      #ifdef DEBUG
        Serial.println("SDS011 error");
      #endif
      display.drawString(0,0,"SDS011 Error"); 
      someerror=true;
      }
    }
else
    {
    // uart timeout
     #ifdef DEBUG
        Serial.println("SDS011 Timeout");
      #endif
      display.drawString(0,0,"SDS011 Timeout"); 
      someerror=true;
    }
  display.display();
  #ifndef FASTINIT
    delay(1000);
  #endif
  
  /*******************************************************************************************************************
   * BME280/BMP280 setup
   * connected on the I2C, same as OLED
   ******************************************************************************************************************/
  if (!bme.begin(PRESS_SENS_ADDR))
    {
    #ifdef DEBUG
      Serial.println("Pressure Sensor Error");   
    #endif
    #ifdef USE_BME280
        display.drawString(0,10,"BME280 FAIL");
    #else
      #ifdef USE_BMP280
        display.drawString(0,10,"BMP280 FAIL");
      #endif
    #endif
    someerror=true;
    }
 else
    {
    #ifdef DEBUG  
      Serial.println("Pressure sensor connected");
    #endif
    #ifdef USE_BME280
        display.drawString(0,10,"BME280 OK");
    #else
      #ifdef USE_BMP280
        display.drawString(0,10,"BMP280 OK");
      #endif
    #endif
    }
  display.display();
  #ifndef FASTINIT
    delay(1000);
  #endif
  
  /*******************************************************************************************************************
   * DHT22 setup
   ******************************************************************************************************************/
  dht.begin();
  // dht sensor does not give an initialization status
    
  /*******************************************************************************************************************
   * LoRa Module setup
   * connected on the VSPI
   ******************************************************************************************************************/
  loraSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  LoRa.setSPI(loraSPI);
  if (!LoRa.begin(LORA_BAND)) 
    {
    #ifdef DEBUG
      Serial.println("LoRa failed");
    #endif
    display.drawString(0,20,"LoRa FAIL");
    display.display();
    someerror=true;
    }
  #ifdef DEBUG
    Serial.println("LoRa OK");
  #endif
  display.drawString(0,20,"LoRa OK");
  display.display();
  // set a sync word to be used with both modules for synchronization
  LoRa.setSyncWord(0xF3); // ranges from 0-0xFF, default 0x34, see API docs
  #ifndef FASTINIT
    delay(1000);
  #endif
  
  /*******************************************************************************************************************
   * SD Card setup
   * connected on the HSPI
   ******************************************************************************************************************/
  sdSPI.begin(SDCARD_SCK, SDCARD_MISO, SDCARD_MOSI, SDCARD_CS);
  // the board does not differentiate No SD card or mounting failed
  // probably for doing this would be necessary a switch for detecting SD Card insertion
  if (!SD.begin(SDCARD_CS, sdSPI)) 
    {
    #ifdef DEBUG
      Serial.println("No SDCard OR Failed");
    #endif
    display.drawString(0,30,"No SDCard Or FAIL");
    display.display();
    } 
  else 
    {
    uint8_t cardType = SD.cardType();
    sdpresent=true;
    uint32_t SDCardSize = SD.cardSize()/(1024*1024);
    #ifdef DEBUG
      Serial.print("SDCard OK");
      Serial.print(" - Size: ");
      Serial.print(SDCardSize);
      Serial.println("MB");
    #endif
    display.drawString(0,30,"SDCard is "+String(SDCardSize) + "MB");
    display.display();
      
    #ifdef DEBUG
    Serial.print("SD Card Type: ");
    switch(cardType)
      {
      case CARD_MMC:
      Serial.println("MMC");
      break;
      case CARD_SD:
      Serial.println("SDSC");
      break;
      case CARD_SDHC:
      Serial.println("SDHC");
      break;
      // a 4th case is CARD_NONE
      // but is not evaluated with this TFreader
      default:
      Serial.println("UNKNOWN");
      break;
      }
    #endif

  /*******************************************************************************************************************
   * Create file
   ******************************************************************************************************************/
    setDataFileName();
    
    // create the file if don't exists or truncate it to zero. Add cell headers for the CSV
    // ; is used as separator since Excel recognizes it giving correct formatting
    // fields:
    // 0 : number of fields (12, or 11 if BMP280 used, this field included, checksum excluded)
    // 1 : timestamp
    // 2 : temperature from BME280/BMP280
    // 3 : temperature from DHT22
    // 4 : humidity from BME280 (eventually)
    // 5 (4) : humidity from DHT22
    // 6 (5) : pressure
    // 7 (6) : PM10
    // 8 (7) : PM2.5
    // 9 (8) : PM is new (1=new data, 0=old data, still not refreshed)
    // 10 (9) : Battery voltage
    // 11 (10): SD status (-1:not present, 1:present and working, 0:recording is stopped)
    #ifdef USE_BME280 // 12 fields
        writeFile(SD, dataFile.c_str(), "Fields;Timestamp;Temp(BME);Temp(DHT);Humi(BME);Humi(DHT);Pressure;PM10;PM2.5;New PM;Battery;SD Status\n");
    #else
      #ifdef USE_BMP280 // 11 fields, no Humidity from this pressure sensor
        writeFile(SD, dataFile.c_str(), "Fields;Timestamp;Temp(BMP);Temp(DHT);Humi(DHT);Pressure;PM10;PM2.5;New PM;Battery;SD Status\n");
      #endif
    #endif
    } // SD Mounted and Present
  #ifndef FASTINIT
    delay(1000);
  #endif
  
  /*******************************************************************************************************************
   * Check the battery if eventually mounted
   ******************************************************************************************************************/
  battery=checkBattery(); // first battery check at start-up
  #ifdef DEBUG
    Serial.print("Battery Voltage: ");
    Serial.print(battery);
    Serial.println("V");
  #endif
  display.drawString(0,40,"Battery: "+String(battery)+"V");
  display.display();
  #ifndef FASTINIT
    delay(1000);
  #endif
  
  #ifdef DEBUG
    Serial.println("Reading first data from sensors. Please wait...");
  #endif

  /*******************************************************************************************************************
   * First readings of the sensors
   ******************************************************************************************************************/
  // BME280/BMP280
  temperatureB=bme.readTemperature();
  #ifdef USE_BME280
    humidityB=bme.readHumidity();
  #endif
  pressure=bme.readPressure() / 100.0F; // pressure expressed in hPa (=mBar)
  // DHT22
  humidityD = dht.readHumidity();
  temperatureD = dht.readTemperature();

  /*******************************************************************************************************************
   * Access Point setup
   ******************************************************************************************************************/
  #ifdef DEBUG
    Serial.println("Setting Access Point");
    Serial.print("Access point name: ");
    Serial.println(ssid);
  #endif

  WiFi.softAP(ssid,password);
  delay(100);
  IPAddress myIP=WiFi.softAPIP();
  #ifdef DEBUG
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  #endif
  // attach webserver functions
  server.on("/", server_connect);
  server.on("/getSensors", server_request);
  server.on("/fs", server_filesystem);
  server.on("/download", server_download);
  server.onNotFound(server_notfound);
  server.begin();

  /*******************************************************************************************************************
   * Initialization finished
   ******************************************************************************************************************/
  #ifdef DEBUG
    Serial.println("Init Finished");
    if (someerror) Serial.println("Some initialization errors detected. Something can go wrong");
  #endif
  if (someerror)
    {
    display.drawString(0,50,"SOME ERRORS!");
    #ifndef FASTINIT
      delay(3000);   
    #endif
    }
  else
    {
    display.drawString(0,50,"Init OK!");  
    }
  display.display();
  #ifndef FASTINIT
    delay(1000);
  #endif
  display.clear();

  // Draw my Logo, please don't remove
  #ifndef FASTINIT
    display.drawXbm(32, 0, SZ_logo_width, SZ_logo_height, SZ_logo);
    display.display();
    delay(1200);
    display.clear();
  #endif
  }

/*******************************************************************************************************************
* Main Loop
******************************************************************************************************************/
void loop() 
  {
  // variables used for checking weather sensors and give an average value
  static uint8_t chksens=0;
  static float temptemperatureB=0;
  static float temphumidityB=0;
  static float temppressure=0;
  static float temptemperatureD=0;
  static float temphumidityD=0;
  static float tempbatt=0;
  static bool firstSDSDataReceived=false;
  
  // doing the Webserver required stuff
  server.handleClient();
  
  // write stuff on the oled display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Monospaced_bold_11);
  display.drawString(0,0, "WePaRT TX");
  display.drawString(0, 10,"TB:"+String(temperatureB,1)+"° TD:"+String(temperatureD,1)+"°"); // temperature from the BME280/BMP280 - Temperature from the DHT22
  #ifdef USE_BME280
        display.drawString(0, 20,"HB:"+String(humidityB,1)+"% HD:"+String(humidityD,1)+"%"); // humidity from the BME280 - Humidity from the DHT22
  #else
    #ifdef USE_BMP280
        display.drawString(0, 20,"Hu: "+String(humidityD,1)+"%"); // humidity from the DHT22
    #endif
  #endif
  display.drawString(0, 30,"PRESS: "+String(pressure,1)+"mBar"); // pressure
  display.drawString(0, 40,"PM 10: "+String(pm10,1)+"ppm"); // PM10 value (whole, contains also the PM2.5)
  display.drawString(0, 50,"PM2.5: "+String(pm25,1)+"ppm"); // PM2.5 value
  display.display();

  // Reading data from the sensors
  temptemperatureB+=bme.readTemperature();
  #ifdef USE_BME280
    temphumidityB+=bme.readHumidity();
  #endif
  temppressure+=bme.readPressure() / 100.0F;
  temptemperatureD+=dht.readTemperature();
  temphumidityD+=dht.readHumidity();
  tempbatt+=checkBattery();
  chksens++; // increment the number of readings
  
  // Average of values and output to variable
  if (chksens==AVERAGE_SENSORS)
    {
    temperatureB=temptemperatureB/AVERAGE_SENSORS;
    #ifdef USE_BME280
      humidityB=temphumidityB/AVERAGE_SENSORS;
    #endif
    pressure=temppressure/AVERAGE_SENSORS;
    temperatureD=temptemperatureD/AVERAGE_SENSORS;
    humidityD=temphumidityD/AVERAGE_SENSORS;
    battery=tempbatt/AVERAGE_SENSORS;
    
    // send+record data
    dataTx(false,firstSDSDataReceived); // parameter=false => SDS011 data are the old ones, still not refreshed
    
    // reset
    chksens=0;
    temptemperatureB=0;
    #ifdef USE_BME280
      temphumidityB=0;
    #endif
    temppressure=0;
    temptemperatureD=0;
    temphumidityD=0;
    tempbatt=0;
    }
  
  // SDS011 reading if available
  if (readSDS011()==SDS_OK) 
    {
    firstSDSDataReceived=true;
    dataTx(true,true); // parameter=true => SDS011 data are new, along with other data
    }

  // with used delays and the average over 200 (AVERAGE_SENSORS) values
  // data will be transmitted about every 21-22 seconds. This is good for Thingspeak
  delay(150);
  } // loop

/******************************************************************************************************************
* First connection to the webserver (AP mode)
******************************************************************************************************************/
void server_connect(void)
  {
  String p3= "<body>";
  p3 += "<div style=\"text-align:center\">\n\r";
  p3 +="<div class=\"ti\">WePaRT by Giovanni Bernardo</div>";
  p3 +="<div class=\"ti2\">Weather and Particulate<br/>Recorder-Transmitter</div>";
  
  // temperature from BME280/BMP280
  #ifdef USE_BME280
    p3 += "<div class=\"st\" style=\"background-color:#46B1C9; color:#FFFFFF;\">Temp. (BME): \n\r";
  #else
    #ifdef USE_BMP280
      p3 += "<div class=\"st\" style=\"background-color:#46B1C9; color:#FFFFFF;\">Temp. (BMP): \n\r";
    #endif
  #endif
  p3 += "<span id=\"TB\">";
  p3 += String(temperatureB,1);
  p3 += "&deg;C</span></div>\n\r";
  
  // temperature from DHT22
  p3 += "<div class=\"st\" style=\"background-color:#048BA8; color:#FFFFFF;\">Temp. (DHT): \n\r";
  p3 += "<span id=\"TD\">";
  p3 += String(temperatureD,1);
  p3 += "&deg;C</span></div>\n\r";
  
  // Humidity from BME280
  #ifdef USE_BME280
    p3 += "<div class=\"st\" style=\"background-color:#16DB93; color:#FFFFFF;\">Hum. (BME): \n\r";
    p3 += "<span id=\"HB\">";
    p3 += String(humidityB,1);
    p3 += "%</span></div>\n\r";
  #endif
  
  // Humidity from DHT22
  p3 += "<div class=\"st\" style=\"background-color:#EFEA5A; color:#000000;\">Hum. (DHT): \n\r";
  p3 += "<span id=\"HD\">";
  p3 += String(humidityD,1);
  p3 += "%</span></div>\n\r";
  
  // Pressure
  p3 += "<div class=\"st\" style=\"background-color:#F29E4C; color:#000000;\">Pressure: \n\r";
  p3 += "<span id=\"P\">";
  p3 += String(pressure,1);
  p3 += "mBar</span></div>\n\r";
  
  // PM10
  p3 += "<div class=\"st\" style=\"background-color:#EEABC4; color:#000000;\">PM 10: \n\r";
  p3 += "<span id=\"PM10\">";
  p3 += String(pm10,1);
  p3 += "&micro;g/m<sup>3</sup></span></div>\n\r";
  
  // PM2.5
  p3 += "<div class=\"st\" style=\"background-color:#A53860; color:#FFFFFF;\">PM 2.5: \n\r";
  p3 += "<span id=\"PM25\">";
  p3 += String(pm25,1);
  p3 += "&micro;g/m<sup>3</sup></span></div>\n\r";

  p3 += "<div class=\"s\">&nbsp;</div>";
  if (sdpresent)
    {
    p3 +="<a class=\"l\" href=\"/fs\">Check filesystem</a>";
    }
  p3 +="<a class=\"l\" href=\""+infourl+"\">INFO (needs internet)</a>";
  p3 +="<br/>";
  p3 += "<div class=\"s\">timestamp: <span id=\"TIMESTAMP\">"+String(lastTimeStamp)+"</span></div>";
  p3 += "<div class=\"s\">&copy;2020 Giovanni Bernardo</div>";
  p3 += "<div class=\"s\">www.settorezero.com</div>";
    
  // close document      
  p3 += "</div>\n\r";
  p3 += "</body>\n\r";
  p3 += "</html>";

  #ifdef DEBUG
    Serial.println("webpage requested");
  #endif
  server.send(200, "text/html", doctype+p1+p2+p3);
  // p1 defined in webpage.h, contains page header with meta-values (icons) and stylesheet
  // p2 defined in webpage.h, contains javascript function for ajax refresh
  // p3 generated here  
  }

/******************************************************************************************************************
* AJAX script will send a request using the string "getSensors"
* The webserver will respond here, giving a simple string with the packet containing sensors data
******************************************************************************************************************/
void server_request(void)
  {
  #ifdef DEBUG
    Serial.println("data request on webserver");
  #endif
  server.send(200, "text/html", packet+"\0");
  }

/******************************************************************************************************************
* Returns info on the SD Card
******************************************************************************************************************/
void server_filesystem(void)
  {
  #ifdef DEBUG
    Serial.println("SD card request on webserver");
  #endif
  String response="</head><body>";
  if (sdpresent)
    {
    uint32_t SDCardSize = SD.cardSize()/(1024*1024);
    uint32_t Total=SD.totalBytes();
    uint32_t Used=SD.usedBytes();
    uint32_t Free=Total-Used;
    response+="<div class=\"sfb\">SD INFO</div>";
    response+="<div class=\"sf\">";
    response+="SD size: "+String(SDCardSize)+"MB<br/>";
    response+="Total: "+String(Total/(1024*1024))+"MB<br/>";
    response+="Used: ";
    if ((Used/(1024*1024))==0) // 0MB
      {
      if ((Used/1024)==0) // 0kB
        {
        response+=String(Used)+"B<br/>"; // write bytes
        }
      else // some kB
        {
        response+=String(Used/1024)+"kB<br/>"; 
        }
      }
    else // some MB
      {
      response+=String(Used/(1024*1024))+"MB<br/>";
      }
    
    response+="Free: "+String(Free/(1024*1024))+"MB<br/>";;
    response+="</div>";
    response+="<br/>";
    
    response+="<div class=\"sfb\">FILE LIST</div>";
    response+="<div class=\"sf\">";
    File root = SD.open("/");
    if(!root)
      {
      response+="Failed to open root";
      }
    else
      {
      File file = root.openNextFile();
      while(file)
        {
        if(file.isDirectory())
          {
          response+="&lt;"+String(file.name())+"&gt;<br/>";
          } 
        else 
          {
          response+=String(file.name())+"&nbsp;("+String(file.size())+")";
          #ifdef ALLOW_DOWNLOAD
            response+="&nbsp;<a href=\"/download?file="+String(file.name())+"\">[download]</a>";
          #endif
          response+="<br/>";
          }
        file = root.openNextFile();
        }
      response+="</div><br/>";
      response+="<div class=\"sfb\">NOW RECORDING ON</div>";
      response+="<div class=\"sf\">";
      if (stoprecording)
        {
        response+="Recording stopped.<br/>Reached the maximum amount of files"; 
        }
      else
        {
        response+=dataFile;
        }
      } // root opened
    } // sd present
  else // microSD not mounted
    {
    response+="No SD Card mounted";
    }
  response+="</div><br/>";
  response+="<a class=\"l\" href=\"/\">Return to home</a>";
  response+="</body></html>";
  server.send(200, "text/html", doctype+p1+response);  
  }
  
/******************************************************************************************************************
* Response to a non-existent page
******************************************************************************************************************/
void server_notfound()
  {
  server.send(404, "text/plain", "Not found");
  }

/******************************************************************************************************************
* Downloads a CSV file
******************************************************************************************************************/
void server_download()
  {
  String filename=server.arg("file");
  if (sdpresent) 
    { 
    File file=SD.open(filename.c_str());
    if (file) 
      {
      filename.remove(0,1); // remove the / at start of the string
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(file, "application/octet-stream");
      file.close();
      } 
   else 
      {
      // file not present
      server_notfound();
      }
    }
  else
    {
    // sd not mounted
    server_notfound();  
    }
  }

/******************************************************************************************************************
* Routine used for:
* - creating the packet
* - send packet over LoRa
* - record the packet on the SD if present
******************************************************************************************************************/
void dataTx(bool newDataFromSDS, bool firstSDSDataReceived)
  {
  LedOn();
  
  // send packet over LoRa
  // packet is composed as
  // @A;B;C;D;E;F;G;H;I;J;K;L;M#
  // @A : message header (@) followed by number of fields (12, or 11 if BMP280 used)
  // B : timestamp
  // C : temperature from BME280/BMP280
  // D : temperature from DHT22
  // E : humidity from BME280 (if BMP280 is used)
  // F (E) : humidity from DHT22
  // G (F) : pressure
  // H (G) : PM10
  // I (H) : PM2.5
  // J (I) : PM is new (1=data from SDS011 are new, 0=data from SDS011 is the previous one, still not refreshed)
  // K (J) : Battery Voltage
  // L (K) : SD Status
  // M# (L#) : checksum by summing all previous chars (@ excluded), dots and commas included. In numeric-ascii format, #?=message tail
  
  // Central part of the packet (all but header, checksum and tail)
  // this part is also stored on the SD card if present
  packet=String(FIELDS)+";"+String(millis())+";"+String(temperatureB,1)+";"+String(temperatureD,1)+";";
  #ifdef USE_BME280
        packet+=String(humidityB,1)+";";
  #endif
  packet+=String(humidityD,1)+";"+String(pressure,1)+";";
  
  // first time we receive data from SDS011
  if (firstSDSDataReceived)
    {
    packet+=String(pm10,1)+";"+String(pm25,1)+";";
    }
  else // first data from SDS011 never arrived, sent empty fields
    {
    packet+=" ; ;";  
    }
  
  if (newDataFromSDS)
    {
    packet+="1;"; // new data from SDS011
    }
  else
    {
    packet+="0;"; // PM values are the previous ones, still not refreshed
    }  
  packet+=String(battery,1)+";";
  
  // add SD status
  if (sdpresent)
    {
    // stoprecording is not really used
    if (stoprecording) {packet+="0";} // think as "SD present but reached maximum amount of files"
        else {packet+="1";} // SD working
    }
  else {packet+="-1";} // SD not present
  
  // calculate checksum
  uint16_t cs=myChecksum(packet);
  #ifdef DEBUG
    Serial.print("Packet: ");
    Serial.println(packet);
    Serial.print("Checksum: ");
    Serial.println(cs);
  #endif
  // send data over LoRa
  LoRa.beginPacket();
  LoRa.print("@"+packet+";"+String(cs)+"#"); // add header, checksum and tail to the packet over LoRa
  LoRa.endPacket();
  
  // log on the SD card if present
  // stoprecording flag is for future use
  if (sdpresent && !stoprecording)
    {
    // millis() rollovered
    if (millis()<lastTimeStamp)
      {
      #ifdef DEBUG
        Serial.println("millis() rollover, need to create new filename");
      #endif
      setDataFileName();   
      }
    packet+=String("\n"); // attach newline for the file
    appendFile(SD, dataFile.c_str(), packet.c_str()); 
    }
  lastTimeStamp=millis();
  delay(200);
  LedOff();  
  }

/******************************************************************************************************************
* Set the useable name of the datafile
******************************************************************************************************************/
void setDataFileName(void)
  {
  // dataFileIndex is a global variable
  // search for the dataFileIdx file
  #ifdef DEBUG
    Serial.println("Reading last id from the id file");
  #endif
  int i=0; // set file index
  String dataFileId="/id"; 
  File file = SD.open(dataFileId.c_str()); // open file index
  if(!file)
    {
    // file index not present
    #ifdef DEBUG
      Serial.println("No id file present");
    #endif
    // create a new one with 0 in it 
    writeFile(SD, dataFileId.c_str(), "0");
    }
  else
    {
    // file present, open it
    String t="";
    while(file.available())
      {
      t+=(file.read()-48); // append single bytes, read gives the byte value, so "0" will be returned as 48
      }
    if (t=="-1") // file empty
      {
      #ifdef DEBUG
        Serial.println("File is empty");
      #endif
      // create a new one with 0 in it 
      writeFile(SD, dataFileId.c_str(), "0");    
      }
    else // file contains something
      {
      i=t.toInt();  
      #ifdef DEBUG
        Serial.print("File contains ");
        Serial.println(i);
      #endif
      if (i==99999999) // maximum file number
        {
        i=-1;
        #ifdef DEBUG
          Serial.print("Too many files, restart from 0");
        #endif
        }
      i++; // increment
      writeFile(SD, dataFileId.c_str(), String(i).c_str()); // update file index in the id file
      }
    } // id file present
  file.close();
  // create filename for the file where save the data from sensors
  char b[9];
  sprintf(b, "%08d", i); // a number left-padded with 0es to reach 8 chars
  dataFile="/"+String(b)+".csv";
  #ifdef DEBUG
    Serial.print("Data File name is now: ");
    Serial.println(dataFile);
  #endif
  return;
  }

/******************************************************************************************************************
* Simply user Led ON / Led OFF routines
******************************************************************************************************************/
void LedOn()
    {
    digitalWrite(LED, HIGH);
    }
void LedOff()
    {
    digitalWrite(LED, LOW);
    }

/******************************************************************************************************************
* Read the battery value
* if battery not connected or in charge, will give the voltage given from the battery charging circuit
******************************************************************************************************************/
float checkBattery(void)
  {
  // The ADC value is a 12-bit number (values from 0 to 4095)
  // To convert the ADC integer value to a real voltage you’ll need to divide it by the maximum value of 4095
  // then double it since there is an 1:2 voltage divider on the battery
  // then you must multiply that by the reference voltage of the ESP32 which is 3.3V 
  // and then multiply that again by the ADC Reference Voltage of 1100mV.
  return (float)(analogRead(VBAT))/4095*2*3.3*1.1;
  }

/******************************************************************************************************************
* Read data from the SDS011
* checksum will be evaluated
******************************************************************************************************************/
uint8_t readSDS011(void)
  {
  if (Serial.available()<10) return SDS_NODATA; // if buffer contains less than 10 bytes, exit
  uint8_t buf[10];
  uint8_t i=Serial.readBytes(buf,10);
  #ifdef DEBUG
    Serial.print("Received from SDS011: ");
    for (uint8_t u=0; u<i; u++)
      {
      Serial.print(buf[u],HEX);  
      Serial.print(' ');
      }
    Serial.println();
  #endif
  // response example: AA C0 D4 04 3A 0A A1 60 1D AB
  // AA = header
  // C0 = response ID
  // D4 = PM2.5 low byte
  // 04 = PM2.5 high byte
  // 3A = PM10 low byte
  // 0A = PM10 high byte
  // A1 = device ID 1
  // 60 = device ID 2
  // 1D = checksum (D4+04+3A+0A+A1+60)=0x021D => 1D (only lower byte)
  // AB = tail
  if ((buf[0]==0xAA) && (buf[1]==0xC0) && (buf[9]==0xAB)) // correct data format
    {
    char chk=char((buf[2]+buf[3]+buf[4]+buf[5]+buf[6]+buf[7])&0xFF); // calculate checksum
    if (chk!=buf[8]) return SDS_CHECKSUM; //checksum error
    pm25=((buf[3]<<8)+buf[2])/10;
    pm10=((buf[5]<<8)+buf[4])/10;  
    return SDS_OK;
    }
  else
    {
    return SDS_INVALID; // data in not correct format
    }
  }
  
/******************************************************************************************************************
* This is the checksum function I wrote for validating the packet transmitted over LoRa
******************************************************************************************************************/
// calculate a simple checksum by summing single byte chars from a passed string
uint16_t myChecksum(String s)
  {
  uint8_t l=s.length(); // string lenght
  uint16_t chk=0; // checksum value
  for (uint8_t i=0; i<l; i++)
    {
    char u=s.charAt(i); // extract single char
    chk+=u; // sum byte value
    }
  return chk;  
  }

/******************************************************************************************************************
* SD File functions EXAMPLES
* Copied and Pasted from the official Arduino-ESP32 repository - SD Examples:
* https://github.com/espressif/arduino-esp32/tree/master/libraries/SD
* Added only the Serial Writing on debug
******************************************************************************************************************/
void listDir(fs::FS &fs, const char * dirname, uint8_t levels)
  {
  #ifdef DEBUG
    Serial.printf("Listing directory: %s\n", dirname);
  #endif
  File root = fs.open(dirname);
  if(!root)
    {
    #ifdef DEBUG
      Serial.println("Failed to open directory");
    #endif
    return;
    }
  if(!root.isDirectory())
    {
    #ifdef DEBUG
      Serial.println("Not a directory");
    #endif
    return;
    }

  File file = root.openNextFile();
  while(file)
    {
    if(file.isDirectory())
      {
      #ifdef DEBUG
        Serial.print("  DIR : ");
        Serial.println(file.name());
      #endif
      if(levels)
        {
        listDir(fs, file.name(), levels -1);
        }
      } 
    else 
      {
      #ifdef DEBUG
        Serial.print("  FILE: ");
        Serial.print(file.name());
        Serial.print("  SIZE: ");
        Serial.println(file.size());
      #endif
      }
    file = root.openNextFile();
    }
  }

void createDir(fs::FS &fs, const char * path)
  {
  #ifdef DEBUG
    Serial.printf("Creating Dir: %s\n", path);
  #endif
  if(fs.mkdir(path))
    {
    #ifdef DEBUG
      Serial.println("Dir created");
    #endif
    } 
  else 
    {
    #ifdef DEBUG
      Serial.println("mkdir failed");
    #endif
    }
  }

void removeDir(fs::FS &fs, const char * path)
  {
  #ifdef DEBUG
    Serial.printf("Removing Dir: %s\n", path);
  #endif
  if(fs.rmdir(path))
    {
    #ifdef DEBUG
      Serial.println("Dir removed");
    #endif
    } 
  else 
    {
    #ifdef DEBUG
      Serial.println("rmdir failed");
    #endif
    }
  }

void readFile(fs::FS &fs, const char * path)
  {
  #ifdef DEBUG
    Serial.printf("Reading file: %s\n", path);
  #endif
  File file = fs.open(path);
  if(!file)
    {
    #ifdef DEBUG
      Serial.println("Failed to open file for reading");
    #endif
    return;
    }
  #ifdef DEBUG
    Serial.print("Read from file: ");
  #endif
  while(file.available())
    {
    #ifdef DEBUG
      Serial.write(file.read());
    #endif
    }
  file.close();
  }

void writeFile(fs::FS &fs, const char * path, const char * message)
  {
  #ifdef DEBUG
    Serial.printf("Writing file: %s\n", path);
  #endif
  File file = fs.open(path, FILE_WRITE);
  if(!file)
    {
    #ifdef DEBUG
      Serial.println("Failed to open file for writing");
    #endif
    return;
    }
  if(file.print(message))
    {
    #ifdef DEBUG
      Serial.println("File written");
    #endif
    } 
  else 
    {
    #ifdef DEBUG
      Serial.println("Write failed");
    #endif
    }
  file.close();
  }

void appendFile(fs::FS &fs, const char * path, const char * message)
  {
  #ifdef DEBUG
    Serial.printf("Appending to file: %s\n", path);
  #endif
  File file = fs.open(path, FILE_APPEND);
  if(!file)
    {
    #ifdef DEBUG
      Serial.println("Failed to open file for appending");
    #endif
    return;
    }
  if(file.print(message))
    {
    #ifdef DEBUG
      Serial.println("Message appended");
    #endif
    } 
  else 
    {
    #ifdef DEBUG
      Serial.println("Append failed");
    #endif
    }
  file.close();
  }

void renameFile(fs::FS &fs, const char * path1, const char * path2)
  {
  #ifdef DEBUG
    Serial.printf("Renaming file %s to %s\n", path1, path2);
  #endif
  if (fs.rename(path1, path2)) 
    {
    #ifdef DEBUG
      Serial.println("File renamed");
    #endif
    } 
  else 
    {
    #ifdef DEBUG
      Serial.println("Rename failed");
    #endif
    }
  }

void deleteFile(fs::FS &fs, const char * path)
  {
  #ifdef DEBUG  
    Serial.printf("Deleting file: %s\n", path);
  #endif
  if(fs.remove(path))
    {
    #ifdef DEBUG
      Serial.println("File deleted");
    #endif
    } 
  else 
    {
    #ifdef DEBUG
      Serial.println("Delete failed");
    #endif
    }
  }
  
/******************************************************************************************************************
* END Of the file
* 
* if you reached this part, you're a smart person and I want you as follower of my social networks:
* 
* Facebook: https://www.facebook.com/settorezero (Italian language)
* Twitter: https://www.twitter.com/settorezero (English language)
* Instagram - my private profile: https://www.instagram.com/cyb3rn0id (english language)
* Instagram - profile of my blog: https://www.instagram.com/settorezero (italian language)
* Youtube: https://www.youtube.com/settorezero (mainly italian language, but sometimes also English language)
* 
* PLEASE SUBSCRIBE
******************************************************************************************************************/
