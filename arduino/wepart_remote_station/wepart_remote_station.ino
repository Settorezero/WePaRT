/*
 * WePaRT - Weather and Particulate Recorder Transmitter
 * REMOTE STATION
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
 * EasyNTP client - https://github.com/aharshac/EasyNTPClient
 * PubSub client - https://github.com/knolleary/pubsubclient
 * TimeLib - https://github.com/PaulStoffregen/Time
 * Thingspeak - https://github.com/mathworks/thingspeak-arduino
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
#include <WiFiUdp.h>
#include <EasyNTPClient.h>
#include <TimeLib.h>
#include <PubSubClient.h>
#include "ThingSpeak.h"

#include "board_defs.h" // in this header I've defined the pins used by the module LILYGO® TTGO LoRa32 V2.1_1.6
#include "images.h"
#include "my_fonts.h"
#include "webpage.h"
#include "secret.h"

// START OF CUSTOMIZE SECTION --------------------------------------------------------------------------------------------------------------------

// **************************************
// WI-Fi Network settings
// **************************************
#define RETRIES_WIFI 100  // number of WiFi re-connection retries after a no-connection, at 500mS reconnection rate
#define USE_STATIC_IP // comment for using DHCP OR if you have errors on NTP server connection
// if you want to use static IP, change also following parameters:
#ifdef USE_STATIC_IP
    IPAddress deviceIP(192, 168, 1, 162); // static address will be assigned to the device
    IPAddress gateway(192, 168, 1, 1); // router address
    IPAddress subnet(255, 255, 255, 0); // subnet mask, usually you don't need to change this value
    // following are Google DNS, you don't really need to change them
    IPAddress dns1 (8, 8, 8, 8); // first DNS, required for easyntp with static ip (https://github.com/aharshac/EasyNTPClient/issues/4)
    IPAddress dns2 (8, 8, 4, 4); // second DNS, required for easyntp with static ip
#endif

// **************************************
// THINGSPEAK settings
// **************************************
#define USE_THINGSPEAK // comment if you don't want to use thingspeak

// **************************************
// MQTT settings
// **************************************
#define USE_MQTT // comment if you don't use an MQTT server
#define RETRIES_MQTT 10  // number of MQTT re-connection retries after a no-connection
#define MQTT_USE_PASSWORD // Comment this if your MQTT server does not require user/password
IPAddress mqtt_server(192,168,1,101); // this is the address of your MQTT server
const uint16_t mqtt_port=1883; // this is the port where your MQTT server is listening. 1883 is the standard value
const char* mqtt_clientID = "WePaRT"; // unique name for this MQTT client, you can leave WePaRT
const char* topic_wepart="wepart"; // MQTT topic where message will be published

// **************************************
// NTP settings
// **************************************
// WARNING: use DHCP (NO static IP) if you encounter problems with NTP server
// NTP server returns UTC time. Italy use the CET (Central European Time). 
// during the ST (Standard Time="ora solare" in italian language) CET=UTC+1
// during the DST (Daylight Saving Time="ora legale" in italian language) CET=UTC+2
// DST starts at 2:00AM of the last Sunday of March and ends at 3:00AM of the last Sunday of October
#define TIME_OFFSET 1 // local time offset during Standard Time (Italy uses CET=UTC+1)
const char* NTPServer="it.pool.ntp.org"; //"ntp1.inrim.it";  // NTP server, use one of your country
#define NTP_RETRY_MINUTES 10  // if first NTP connection gone bad, I'll retry after those minutes

// **************************************
// OTHER settings
// **************************************

// uncomment for verbose messages on the serial port (115200 baud)
#define DEBUG

// remove comment if you want a fast-startup (no delays, nor logos during initializazion - used mainly for a rapid check of the code after debug)
//#define FASTINIT

// normal pressure at 25° at my altidude
// I used the calculator at https://www.mide.com/air-pressure-at-altitude-calculator
// not used in the code, here for future implementations
float nPressure=983.4253; // hPa = mBar

// END OF CUSTOMIZE SECTION ----------------------------------------------------------------------------------------------------------------------
// do not change nothing below

// variables used for this (remote) station
bool sdpresent=false; // keep in mind if there is an SD card mounted
String rssi = "--"; // received signal strength indicator value
float mybattery=0; // battery voltage on this device (not the base station)
bool datavalid=false; // keep in memory if received packet over LoRa is good
String dataFile=""; // file name where data will be saved on the SD card
bool firstStart=true; // used for first time file-creation on first valid received data
const char* badPacketFile="/badpack.txt"; // file where bad packets will be saved, don't forget the / in front of filename!

// variables used for Time keeping
bool clockIsSet=false; // flag that indicates that the time was updated the first time at startup
int8_t prevDay=-1; // used for time refresh once a day or for forcing time update
String wd[7]={"dom","lun","mar","mer","gio","ven","sab"}; // name of weekdays, 9 chars max length in italian
String mo[12]={"Gen","Feb","Mar","Apr","Mag","Giu","Lug","Ago","Set","Ott","Nov","Dic"}; // name of months

// good received data from base station
uint8_t fields=0; // number of transmitted fields
String basetimestamp=""; // timestamp from remote station
String battery="-.-"; // base station battery voltage
String sdstatus="-"; // base station SD status: -1=not mounted, 0=full, 1=working
// data from BME280/BMP280
String pressure="---.-";
String temperatureB="--.-";
String humidityB="--.-"; // not used if your base station has the BME280 instead of the BMP280
// data from DHT22
String temperatureD="--.-";
String humidityD="--.-";
// data from SDS011
String pm10="---";
String pm25="---";
String pmIsNew="0";
bool stoprecording=false; // for future use to stop the SD saving

// objects
SSD1306Wire display(OLED_ADDRESS, OLED_SDA, OLED_SCL);
SPIClass sdSPI(HSPI); // we'll use the SD card on the HSPI (SPI2) module
SPIClass loraSPI(VSPI); // we'll use the LoRa module on the VSPI (SPI3) module
WiFiClient MQTT_WiFi_Client;
WiFiUDP udp; // UDP client for NTP server
WiFiClient TS_client;
PubSubClient MQTTClient(mqtt_server, mqtt_port, MQTT_WiFi_Client);
EasyNTPClient ntpClient(udp, NTPServer, (TIME_OFFSET*60*60));
WebServer server(80);

/******************************************************************************************************************
* SETUP
******************************************************************************************************************/
void setup() 
  {
  // Begins some pins
  pinMode(LED,OUTPUT); // Green Led on the board
  pinMode(VBAT,INPUT); // Input for Battery Voltage
  
  bool someerror=false;
  
  /*******************************************************************************************************************
   * UART setup
   * 115200Baud
   ******************************************************************************************************************/
  Serial.begin(115200);
  while (!Serial);
  delay(100);
  #ifdef DEBUG
    Serial.println();
    Serial.println("WePaRT - Weather and Particulate Recorder Transmitter");
    Serial.println("REMOTE STATION");
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
  display.flipScreenVertically(); 
  display.clear();
  delay(100); 
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Monospaced_bold_11);
  #ifdef DEBUG
    Serial.println("OLED initialized");
  #endif
   
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
    display.drawString(0,0,"LoRa FAIL");
    display.display();
    someerror=true;
    }
  #ifdef DEBUG
    Serial.println("LoRa OK");
  #endif
  display.drawString(0,0,"LoRa OK");
  display.display();
  // set a sync word to be used with both modules for synchronization
  LoRa.setSyncWord(0xF3); // ranges from 0-0xFF, default 0x34, see API docs
  LoRa.receive(); // enable receiving on this device
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
    display.drawString(0,10,"No SDCard Or FAIL");
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
    display.drawString(0,10,"SDCard is "+String(SDCardSize) + "MB");
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
  display.drawString(0,20,"Battery: "+String(battery)+"V");
  display.display();
  #ifndef FASTINIT
    delay(1000);
  #endif
    
  /*******************************************************************************************************************
   * WiFi,NTP,MQTT,Thingspeak connections
   ******************************************************************************************************************/
  display.drawString(0,30,"Waiting WiFi");
  display.display();
  if (wifi_connect())
    {
    // connect to WiFi
    display.setColor(BLACK);
    display.fillRect(0, 31, 128, 11); // clear previous line (x,y,width,height)
    display.setColor(WHITE);
    display.drawString(0,30,"WiFi connected");
    display.drawString(0,40,"Waiting NTP");
    display.display();    
    #ifndef FASTINIT
      delay(1500);
    #endif
    
    // update time
    uint8_t i=0;
    while (!clockIsSet)
      {
      updateTime(true);
      delay(100);
      i++;
      if (i>10) break; // exit after 10 retries
      }
    
   #ifdef USE_THINGSPEAK
    #ifdef DEBUG
        Serial.println("Initializing Thingspeak");
    #endif
    ThingSpeak.begin(TS_client);  // Initialize ThingSpeak
   #endif
    
    display.setColor(BLACK);
    display.fillRect(0, 41, 128, 11); // clear previous line (x,y,width,height)
    display.setColor(WHITE);
    if (clockIsSet)
      {
      display.drawString(0,40,"Time updated");
      }
    else
      {
      display.drawString(0,40,"NTP error");
      someerror=true;
      }
    display.drawString(0,50,"Waiting MQTT");
    display.display();
    #ifndef FASTINIT
      delay(1500);
    #endif    
    
    #ifdef USE_MQTT
    if (mqtt_connect())
        {
        // connected to MQTT
        display.setColor(BLACK);
        display.fillRect(0, 51, 128, 11); // clear previous line (x,y,width,height)
        display.setColor(WHITE);
        display.drawString(0,50,"MQTT connected");
        }
    else
        {
        // not connected to MQTT
        display.setColor(BLACK);
        display.fillRect(0, 51, 128, 11); // clear previous line (x,y,width,height)
        display.setColor(WHITE);
        display.drawString(0,50,"MQTT error  ");
        someerror=true;
        }
    display.display();
	
	// start the webserver
	// attach webserver functions
	server.on("/", server_connect);
	server.on("/getSensors", server_request);
	server.on("/fs", server_filesystem);
	server.on("/download", server_download);
	server.onNotFound(server_notfound);
	server.begin();
    
	#endif
    }
  else
    {
    // not connected to wifi
    display.setColor(BLACK);
    display.fillRect(0, 31, 128, 11); // clear previous line (x,y,width,height)
    display.setColor(WHITE);
    display.drawString(0,30,"WiFi error    ");
    display.display();
    someerror=true;
    }
  #ifndef FASTINIT
    delay(1500);
  #endif

  /*******************************************************************************************************************
   * Initialization finished
   ******************************************************************************************************************/
  display.clear();
  #ifdef DEBUG
    Serial.println("Init Finished");
    if (someerror) Serial.println("Some initialization errors detected. Something can go wrong");
  #endif
  if (someerror)
    {
    display.drawString(0,0,"SOME ERRORS!");
    #ifndef FASTINIT
      delay(3000);   
    #endif
    }
  else
    {
    display.drawString(0,0,"Init OK!");  
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

/******************************************************************************************************************
* MAIN LOOP
******************************************************************************************************************/
void loop() 
  {
  static bool lastDst=checkDST(); // DST status, used for forcing time update since normally will occur after 00:00
 
  // check for wifi/MQTT connection
  wl_status_t wifiStatus = WiFi.status();
  if(wifiStatus != WL_CONNECTED)
    {
    wifi_connect();
    }
  #ifdef USE_MQTT
  else
    {
     if (!MQTTClient.connected()) 
      {
      mqtt_connect();
      }
    }
  MQTTClient.loop(); // needed by pubsubclient
  #endif

  // doing the Webserver required stuff
  server.handleClient();
  
  // check if clock update if needed every hour
  if (clockIsSet) 
    {
    bool dst=checkDST(); // check if we're in DST for forcing time update
    if (lastDst!=dst)
      {
      if(updateTime(true)) lastDst=dst;
      }
   else
      {
      updateTime(false); // normal update once a day after 00:00
      }
    } // \clockIsSet
  else
    {
    if(updateTime(true)) lastDst=checkDST();
    }
    
   // write stuff on the oled display
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(Monospaced_bold_11);
  display.drawString(0,0, "WePaRT RX ["+rssi+"]");
  display.drawString(0, 10,"TB:"+temperatureB+"° TD:"+temperatureD+"°"); // temperature from the BME280/BMP280 - Temperature from the DHT22
  if (fields==12)
    {
    display.drawString(0, 20,"HB:"+humidityB+"% HD:"+humidityD+"%"); // humidity from the BME280 - Humidity from the DHT22
    }
  else
    {
    display.drawString(0, 20,"Hu: "+humidityD+"%"); // humidity from the DHT22
    }
  display.drawString(0, 30,"PRESS: "+pressure+"mBar"); // pressure
  display.drawString(0, 40,"PM 10: "+pm10+"ppm"); // PM10 value (whole, contains also the PM2.5)
  display.drawString(0, 50,"PM2.5: "+pm25+"ppm"); // PM2.5 value
  display.display();
  
  int packetSize=LoRa.parsePacket();
  if (packetSize) getLoRaPacket(packetSize);
  }

/******************************************************************************************************************
* Receive LoRa Packet and transmit data
******************************************************************************************************************/
void getLoRaPacket(int packetSize) 
  {
  datavalid=false; // become true?
  LedOn();
  String packet=""; // reset the received packet string
  String error=""; // error description if any
  String packetField[13]; // separate fields of the packet
  String packSize=String(packetSize,DEC); // bytes of the packet (was passed to the function)
  rssi=String(LoRa.packetRssi(), DEC); // received signal strength indicator
  // re-construct packet char by char
  for (int i=0; i<packetSize; i++) 
    {
    packet +=(char)LoRa.read(); 
    }
  
  #ifdef DEBUG
    Serial.print("Packet received: ");
    Serial.println(packet);
    Serial.print("RSSI: ");
    Serial.println(rssi);
  #endif

  // set the timestamp first than all other things
  String mytimestamp=setNiceTimeStamp();
    
  // packet must start with @ and end with #
  if ((packet.startsWith("@") && packet.endsWith("#")))
    {
    // Received packet is composed as:
	// https://docs.google.com/spreadsheets/d/1uwv2ZbNlVsGTnG6Q6OH3sNCoVa-4uajZjfyRlMmH_ik/edit?usp=sharing
	
   
    // I must remove the @ and the #
    String upacket=packet.substring(1,packet.length()-1);
    
    // parse the packet
    int cc=0; // field separator counter
    int k=-1; // first substring must start from index 0 and I add 1 for others
    // scanning in search of separators one char at time
    for (int i=0; i<upacket.length(); i++) 
        {
        if (upacket.substring(i, i+1)==";") // found the field separator
            {
            packetField[cc]=upacket.substring(k+1,i); // extract field from next char after previous comma until this char
            k=i; // keep in mind the position of last ; found
            cc++; // ; counter is the same of field counter
            } // \; found
        } // \for
    
    // last element has no ; after it and it's the checksum value. I must extract it from the last ; to the end of the string
    packetField[cc]=upacket.substring(k+1,upacket.length());
    
    // scan for null characters in the packets (I don't use them in my packet)
    bool nullChars=false;
    for (int o=0; o<cc+1; o++) // scan all packets
      {
      for (int p=0; p<packetField[o].length(); p++) // scan single packet chars
        {
        if ((packetField[o].charAt(p)<32) || (packetField[o].charAt(p)==127))
          {
          nullChars=true;
          break;      
          }
        }
      if (nullChars) break;
      }

    // if there are no null chars I can continue
    if (!nullChars)
      {
      // check the checksum (lol)
      uint16_t chkR=packetField[cc].toInt(); // checksum contained in the packet (last field), number format
      uint16_t chk=myChecksum(upacket.substring(0,upacket.lastIndexOf(';'))); // calculate actual packet checksum excluding last part with comma and checksum
      if (chk==chkR)
        {
        datavalid=true; // 'till now, data seems valid
        }
      else
        {
        error="Checksum Error. In packet:"+String(chkR)+" Calculated:"+String(chk);
        }
    
      // check the number of fields in the packet
      // number of transmitted fields is the first (0) field
      // number of counted fields is the variable cc, that counts the number of field separators
      if (datavalid)
        {
        uint8_t u=0;
        if (packetField[0]=="12") 
          {u=12;}
        else if (packetField[0]=="11")
          {u=11;}
    
        if (cc==u)
            {
            datavalid=true; // data are really valid
            fields=u; // number of fields
            }
        else    
            {
            datavalid=false; // sorry... reassign false to datavalid
            error="Bad fields count. In packet:"+packetField[0]+" counted:"+String(cc);
            }
        } // checksum ok, i checked the fields number
      } // !nullChars
    else
      {
      // there are nullchars
      datavalid=false;
      error="Null chars in packet";
      }
   } // correct header and tail
  else
    {
    //not correct head and tail
    error="Missing delimiters";
    }

  // data valid => transfer in global used variables, record a new packet on the SD, send data to the world
  if (datavalid)
    {
    basetimestamp=packetField[1];
    temperatureB=packetField[2];
    temperatureD=packetField[3];
    if (fields==12) // base station has the BME280, so there is humidity value from it
        {
        humidityB=packetField[4];
        humidityD=packetField[5];
        pressure=packetField[6];
        pm10=packetField[7];
        pm25=packetField[8];
        pmIsNew=packetField[9];
        battery=packetField[10];
        sdstatus=packetField[11];
        }
    else // base station has the BMP280, so there is no humidity value from pressure sensor
        {
        humidityD=packetField[4];
        pressure=packetField[5];
        pm10=packetField[6];
        pm25=packetField[7];
        pmIsNew=packetField[8];
        battery=packetField[9];
        sdstatus=packetField[10];
        }    

    // reconstruct the new packet: it will be used for saving it onto the SD card or for sending it via MQTT
    // if SD is not mounted and MQTT is not used, this packet will not used anyway
    String npacket=setNewPacket();
    
    // save entire packet on SD if present by adding the current timestamp on this device
    // so in case of loss of data you can compare the timestamps on the SD cards
    if (sdpresent) saveDataToSD(npacket, mytimestamp);

    // send entire packet over MQTT
    #ifdef USE_MQTT
        sendDataOverMQTT(npacket, mytimestamp);
    #endif
    
    // send single data fields to thingspeak
    #ifdef USE_THINGSPEAK
        sendDataToThingSpeak(String("data valid on ")+mytimestamp,true);
    #endif
    }
  else // data not valid
    {
    #ifdef DEBUG
        Serial.println(error);  
    #endif
    
    // record bad packet in another file
    packet="Error: "+error+"\nTimestamp: "+mytimestamp+"\nRSSI: "+rssi+"\nPacket: "+packet+"\n\n";
    File badfile = SD.open(badPacketFile);
    if(!badfile)
      {
      #ifdef DEBUG
        Serial.println("Creating badpacket file");
      #endif
      writeFile(SD, badPacketFile, packet.c_str()); // create badpacket file  
      }
    else
      {
      #ifdef DEBUG
        Serial.println("Appending badpacket file");
      #endif
      badfile.close();
      appendFile(SD, badPacketFile, packet.c_str()); // append new data to badpacketfile
      }
      
    #ifdef USE_THINGSPEAK
      // update thingspeak status with the error, by putting false data will not be sent
      sendDataToThingSpeak(error+" on "+mytimestamp,false);
    #endif
    }
  LedOff();
  } // \getLoRaPacket
  
/******************************************************************************************************************
* Save data to SD
******************************************************************************************************************/
bool saveDataToSD(String packet, String mytimestamp)
    {
    // first time system start, create the empty file on the SD having the right cell headers
    // following the first received data
    if (firstStart)
        {
        setDataFileName();
        // ; is used as separator since Excel recognizes it giving correct formatting
        // this file will be different from the one created on the base station
        // here will be some additional fields
        // 0 : nice formatted timestamp from this device
        // 1 : timestamp of base station (basetimestamp)
        // 2 : temperature from BME280/BMP280
        // 3 : temperature from DHT22
        // 4 : humidity from BME280 (eventually)
        // 5 (4) : humidity from DHT22
        // 6 (5) : pressure
        // 7 (6) : PM10
        // 8 (7) : PM2.5
        // 9 (8) : Pm values are fresh
        // 10 (9) : base station Battery voltage
        // 11 (10): base station SD status (-1:not present, 1:present and working, 0:reached maximum amount of files)
        // 12 (11): RSSI
        if (fields==12) // base station sent 12 fields, "fields" field included, checksum excluded
            {
            writeFile(SD, dataFile.c_str(), "Timestamp;Base Timestamp;Temp(BME);Temp(DHT);Humi(BME);Humi(DHT);Pressure;PM10;PM2.5;New PM;Base Battery;Base SD Status;RSSI\n");    
            }
        else // base station sent 11 fields, since uses a BMP280 and there is no humidity value from the pressure sensor
            {
            writeFile(SD, dataFile.c_str(), "Timestamp;Base Timestamp;Temp(BMP);Temp(DHT);Humi(DHT);Pressure;PM10;PM2.5;New PM;Base Battery;Base SD Status;RSSI\n");    
            }
        firstStart=false;
        } // \firstStart
        
    packet=mytimestamp+";"+packet+"\n"; // add the nice formatted timestamp in front of the packet and a newline at the end
    appendFile(SD, dataFile.c_str(), packet.c_str()); // log on the SD card 
    }

/******************************************************************************************************************
* Set the Timestamp used for the CSV recorder on this device
******************************************************************************************************************/
String setNiceTimeStamp(void)
  {
  String mytimestamp="";
  if (clockIsSet)
    {
    // set timestamp used in the CSV File to the format HH:mm:SS DDD DD-MMM-YYYY
    // H,m and s are in number format, so I must add a zero in front of
    // HH:
    if (hour()<10) mytimestamp="0";
    mytimestamp += String(hour())+":";
    // mm:
    if (minute()<10) mytimestamp+="0";
    mytimestamp+=String(minute())+":";
    // ss
    if (second()<10) mytimestamp+="0";
    mytimestamp+=String(second())+" ";
    // DDD 
    mytimestamp+=String(wd[weekday()-1])+" ";
    // DD-
    if (day()<10) mytimestamp+="0";
    mytimestamp+=String(day())+"-";
    // MMM-
    mytimestamp+=String(mo[month()-1])+"-";
    // YYYY
    mytimestamp+=String(year());
    #ifdef DEBUG
      Serial.print("Nice formatted timestamp: ");
      Serial.println(mytimestamp);
    #endif
    }
 else    
    { // clock not set... I'll use millis()
    mytimestamp=String(millis());
    #ifdef DEBUG
      Serial.print("Clock not set... Not very nice formatted timestamp: ");
      Serial.println(mytimestamp);
    #endif
    }
  return (mytimestamp);  
  }

/******************************************************************************************************************
* Reconstruct the new packet
* this function don't adds the nice formatted timestamp, don't adds the number of fields
******************************************************************************************************************/
String setNewPacket(void)
  {
  // re-construct the packet for saving on the SD or send it via MQTT
  // my nice timestamp in front of the packet will be added by other functions
  String packet=basetimestamp+";"+temperatureB+";"+temperatureD+";";
  if (fields==12)
        {
        packet+=humidityB+";";
        }
  packet+=humidityD+";"+pressure+";"+pm10+";"+pm25+";"+pmIsNew+";"+battery+";"+sdstatus+";"+rssi;  
  #ifdef DEBUG
      Serial.print("Reconstructed packet: ");
      Serial.println(packet);
  #endif  
  return (packet);
  }
/******************************************************************************************************************
* Send data to ThingSpeak
******************************************************************************************************************/
bool sendDataToThingSpeak(String TSstatus, bool dataisvalid)
    {
    static bool prevSentFailed=false; // keep in mind if connection to thingspeak failed
    // I do this since sometimes thingspeak gives 401 error and new data from the SDS011 
    // are not sent everytime. In this case if previuos connection failed, I'll retry to send PM values again
    
    // I've choosen to use the following fields in my thingspeak
    // Field 1: temperature, I'll use only the temperature from the BME280/BMP280
    // Field 2: humidity, since I'm using the BME280, I'll use this humidity value, if you're using the BMP280 you can use humidity from this
    // Field 3: pressure 
    // Field 4: PM10 
    // Field 5: PM2.5
    // you can also send all values since thingspeak allows up to 8 fields!

    if (dataisvalid)
      {
      // data are in String format but Thingspeak works nice anyway
      ThingSpeak.setField(1, temperatureB); // or temperatureD
      ThingSpeak.setField(2, humidityB); // or humidityD
      ThingSpeak.setField(3, pressure);
      if ((pmIsNew=="1") || prevSentFailed)
        {
        ThingSpeak.setField(4, pm10);
        ThingSpeak.setField(5, pm25);
        }
      }
    
    // set the status even if data are not valid
    ThingSpeak.setStatus(TSstatus); // you can set a status
  
    // write to the ThingSpeak channel
    int x=ThingSpeak.writeFields(TS_channel, TS_writekey);
    if(x==200)
        {
        #ifdef DEBUG
            Serial.println("Thingspeak update successful.");
        #endif
        prevSentFailed=false;
        return true;
        }
    else
        {
        #ifdef DEBUG
            Serial.println("Problem updating Thingspeak channel. HTTP error code: " + String(x));
        #endif
        prevSentFailed=true;
        return false;
        }
    }

/******************************************************************************************************************
* Send data over MQTT
******************************************************************************************************************/
bool sendDataOverMQTT(String packet, String mytimestamp)
    {
    packet=mytimestamp+";"+packet; // add the nice formatted timestamp in front of the packet
    return (MQTTClient.publish(topic_wepart, packet.c_str()));
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
* Connect to MQTT Broker
******************************************************************************************************************/
bool mqtt_connect(void) 
  {
  while (!MQTTClient.connected()) 
    {
    static uint16_t retr=0; //re-connection retries counter
    #ifdef DEBUG
        Serial.println("Trying to connect to MQTT Broker");  
    #endif
    // Attempt to connect
    #ifdef MQTT_USE_PASSWORD
        if (MQTTClient.connect(mqtt_clientID,mqtt_user,mqtt_password)) 
    #else
        if (MQTTClient.connect(mqtt_clientID))  
    #endif
        {
        #ifdef DEBUG
            Serial.println("MQTT connected");
        #endif
        return (true);
        break;
        } 
    else 
        {
        #ifdef DEBUG
            Serial.print("MQTT failed, rc=");
            Serial.println(MQTTClient.state());
        #endif
        // Wait 2 seconds before retrying
        delay(2000);
        retr++;
        if (retr==RETRIES_MQTT)
            {
            #ifdef DEBUG
                Serial.println("Too many retries");
                Serial.println("Please check MQTT settings");
            #endif
            return (false);
            break;
            }
        }
      }
  }

/******************************************************************************************************************
* Connect to WiFi
******************************************************************************************************************/
bool wifi_connect(void)
  {
  static uint16_t retr=0; //re-connection retries counter
  WiFi.mode(WIFI_STA);
  #ifdef USE_STATIC_IP
    WiFi.config(deviceIP, gateway, subnet, dns1, dns2);
  #endif
  WiFi.begin(ssid, password);
  #ifdef DEBUG
    Serial.print("Trying to connect to WiFi. SSID: ");  
    Serial.println(ssid);
  #endif
  
  while (WiFi.status()!=WL_CONNECTED) 
    {
    retr++;
    if (retr==RETRIES_WIFI)
        {
        // too many retries with same connection status: something is gone wrong
        #ifdef DEBUG
            Serial.println("Too many retries");
            Serial.println("Please check wifi settings");
        #endif
        return (false);
        break;
        }
    delay(500);
    } // \not connected
  
  #ifdef DEBUG
    Serial.println("WiFi connected. ");
    Serial.print("Device IP address is: ");
    Serial.println(WiFi.localIP());
  #endif
  return (true);
  }

/******************************************************************************************************************
* connects to NTP server for time updating
* normally this function will connect to NTP server only if a day is passed (forced=false)
* if forced=true, time will be updated anyway
* if update does not work, I'll recheck after NTP_RETRY_MINUTES minutes
* this routine uses the checkDST function for adding an hour if we're in DST period
******************************************************************************************************************/
bool updateTime(bool forced)
  {
  static long lastChecked=0;
  static bool reCheck=false;
  unsigned long t=0;

  if (prevDay!=weekday() || forced) // day changed or forced to update
    {
    // if is a re-check, I'll do it every NTP_RETRY_MINUTES minutes
    if (reCheck)
          {
          // millis has 'rollovered' (can say it?!)
          if (millis()<lastChecked) lastChecked=millis();
          // NTP_RETRY_MINUTES minutes passed from the last check => try to re-get time
           long retrymillis=lastChecked+(NTP_RETRY_MINUTES*60*1000);
          if (millis()>retrymillis) 
              {
              #ifdef DEBUG
                Serial.println("Re-Check Time");
              #endif
              unsigned long t=ntpClient.getUnixTime();
              }
          }
    else // it's no a re-check: it's the first time I check
          {
          t=ntpClient.getUnixTime();
          }
    if (t>0) // time is updated!
      {
      clockIsSet=true; // set global flag saying clock is set
      setTime(t); // set the time
      #ifdef DEBUG
        Serial.print("Clock successfully updated. ");
        Serial.print("t value=");
        Serial.println(t);
      #endif
      if (checkDST) 
        {
        #ifdef DEBUG
            Serial.println("We're in DST. I add an hour");
        #endif
        adjustTime(3600); // add an hour if we're in DST
        }
      reCheck=false;
      prevDay=weekday(); // this will prevent further updating for today!
      return(true);
      }
  else
      {
      // time not updated, I'll recheck later...
      lastChecked=millis();
      reCheck=true;
      return(false);
      #ifdef DEBUG
        Serial.println("Time not updated");
      #endif
      }
    } // prevDay!=today
  } // \updateTime

/******************************************************************************************************************
* check if actual time is Standard Time or Daylight Saving Time
* returns DST status (true if we're between the 2:00AM of the last Sunday of March
* and the 03:00AM of the last Sunday of October)
******************************************************************************************************************/
bool checkDST(void)
  {
  // In italy DST goes from last Sunday of March (we must set UTC+2 at 2:00AM)
  // until last sunday of October (we must set UTC+1 at 3:00AM)
  bool DST=false;
  
  // Month is Apr,May,Jun,Jul,Aug,Sep => for sure we're in DST
  if ((month()>3) && (month()<10))  
    {
    return (true);
    }
    
  // Month is March or October: we must check day
  if ((month()==3) || (month()==10))
    {
    // Last sunday can occurr only after day 24, March and October have both 31 days:
    // if 24 is Sunday, last Sunday will occurr on 31th
    if (day()<25) // if day is <25 and we're in March, cannot be DST, but if we're in October... yes!
      {
      DST=(month()==3?false:true);
      }
    // today is sunday or sunday is already passed
    // Sunday is 1 and Saturday is 7
    // the value (31-actual day number+weekday number) is a number from 1 to 7 if today is Sunday or
    // Sunday is already passed. Is a number between 8 and 13 if Sunday has to come
    if (((31-day())+weekday()) <8) // It's Sunday or Sunday is already passed
      {
      // today is Sunday and it's the 2:00AM or 2:00AM are passed if in March? 
      // or is Sunday and it's the 3:00AM or 3:00 AM are passed if in October?
      if (weekday()==1 && (((month()==3) && (hour()>1)) || ((month()==10) && (hour()>2)))) 
        {
        // If March, we're still in DST, if October, DST has ended
        DST=(month()==3?true:false);
        }
      // it's not sunday, but sunday is passed
      else if (weekday()>1)
        {
        // If March, we're in DST, if October, DST has ended
        DST=(month()==3?true:false);
        }
      else
        // it's Sunday but are not the 2:00AM in March nor the 3:00AM in October
        {
        // If March, no DST, if October, we're still in DST
        DST=(month()==3?false:true);
        }
      }
    else
      // it's not sunday or sunday has to come
      // If March, no DST, if October, we're still in DST
      {
      DST=(month()==3?false:true);
      }
    } // this month is 3 or 10
   // in all other cases there is no DST (Month is Jan,Feb,Nov,Dec)
   return (DST);
  } // \checkDST
  
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
    chk+=u; // sum the byte  
    }
  return chk;  
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
* WEBSERVER FUNCTIONS
******************************************************************************************************************/

/******************************************************************************************************************
* First connection to the webserver (AP mode)
******************************************************************************************************************/
void server_connect(void)
  {

  String p3= "<body>";
  p3 += "<div style=\"text-align:center\">\n\r";
  p3 +="<div class=\"ti\">WePaRT by Giovanni Bernardo</div>";
  p3 +="<div class=\"ti2\">Weather and Particulate Recorder-Transmitter<br/>- REMOTE STATION (rx) -</div>";
 
  if (fields==0)
    {
    // still not received first data
    p3+="<div class=\"ti2\"> Still not received first data. Please wait some seconds.";
    }
  else
    {
    // temperature from BME280/BMP280
    if (fields==12)
      {
      p3 += "<div class=\"st\" style=\"background-color:#46B1C9; color:#FFFFFF;\">Temp. (BME): \n\r";
      }
    else
      {
      p3 += "<div class=\"st\" style=\"background-color:#46B1C9; color:#FFFFFF;\">Temp. (BMP): \n\r";
      }
    p3 += "<span id=\"TB\">";
    p3 += temperatureB;
    p3 += "&deg;C</span></div>\n\r";
  
    // temperature from DHT22
    p3 += "<div class=\"st\" style=\"background-color:#048BA8; color:#FFFFFF;\">Temp. (DHT): \n\r";
    p3 += "<span id=\"TD\">";
    p3 += temperatureD;
    p3 += "&deg;C</span></div>\n\r";
  
    // Humidity from BME280
    if (fields==12)
      {
      p3 += "<div class=\"st\" style=\"background-color:#16DB93; color:#FFFFFF;\">Hum. (BME): \n\r";
      p3 += "<span id=\"HB\">";
      p3 += humidityB;
      p3 += "%</span></div>\n\r";
      }
  
    // Humidity from DHT22
    p3 += "<div class=\"st\" style=\"background-color:#EFEA5A; color:#000000;\">Hum. (DHT): \n\r";
    p3 += "<span id=\"HD\">";
    p3 += humidityD;
    p3 += "%</span></div>\n\r";
  
    // Pressure
    p3 += "<div class=\"st\" style=\"background-color:#F29E4C; color:#000000;\">Pressure: \n\r";
    p3 += "<span id=\"P\">";
    p3 += pressure;
    p3 += "mBar</span></div>\n\r";
  
    // PM10
    p3 += "<div class=\"st\" style=\"background-color:#EEABC4; color:#000000;\">PM 10: \n\r";
    p3 += "<span id=\"PM10\">";
    p3 += pm10;
    p3 += "&micro;g/m<sup>3</sup></span></div>\n\r";
  
    // PM2.5
    p3 += "<div class=\"st\" style=\"background-color:#A53860; color:#FFFFFF;\">PM 2.5: \n\r";
    p3 += "<span id=\"PM25\">";
    p3 += pm25;
    p3 += "&micro;g/m<sup>3</sup></span></div>\n\r";

    p3 += "<div class=\"s\">&nbsp;</div>";
    p3 +="<a class=\"l\" href=\"/fs\">FILESYSTEM</a>";
    p3 +="<a class=\"l\" href=\"https://thingspeak.com/channels/"+String(TS_channel)+"\">THINGSPEAK</a>";
    p3 +="<a class=\"l\" href=\"https://www.settorezero.com?wepart\">SETTOREZERO</a>";
        
    p3 +="<br/>";
    p3 += "<div class=\"s\">timestamp: <span id=\"TIMESTAMP\">"+basetimestamp+"</span></div>";
    p3 += "<div class=\"s\">RSSI: <span id=\"RSSI\">"+rssi+"</span></div>";
    p3 += "<div class=\"s\">&copy;2020 Giovanni Bernardo</div>";
    p3 += "<div class=\"s\">www.settorezero.com</div>";
    }  
    // close document      
  p3 += "</div>\n\r";
  p3 += "</body>\n\r";
  p3 += "</html>";
  #ifdef DEBUG
    Serial.println("webpage requested");
  #endif
  if (fields==0)
    {
    server.send(200, "text/html", doctype+p1a+ref+p1b+p2+p3); // send the part with the refresh
    }
  else
    {
    server.send(200, "text/html", doctype+p1a+p1b+p2+p3);
    }
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

  // setNewPacket gives: basetimestamp,temperatureB,temperatureD,(humidityB),humidityD,pressure,pm10,pm25,pmIsNew,battery,sdstatus,rssi;  
  // we need also fields, required for humidityB/NO humidityB
  server.send(200, "text/html", String(fields)+";"+setNewPacket()+"\0");
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
          response+=String(file.name())+"&nbsp;("+String(file.size())+")&nbsp;<a href=\"/download?file="+file.name()+"\">[download]</a><br/>";
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
    response+="<div class=\"ti\">No SD Card mounted</div>";
    }
  response+="</div><br/>";
  response+="<a class=\"l\" href=\"/\">Return to home</a>";
  response+="</body></html>";
  server.send(200, "text/html", doctype+p1a+p1b+response);  
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
