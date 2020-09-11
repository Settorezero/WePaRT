// WiFi settings
const char* ssid="[YOUR-SSID]"; // Service Set IDentifier : aka the WiFi Nice Name
const char* password="[YOUR-SSID-PASSWORD]"; // leave blank you want to access using a password

// ThingSpeak settings
unsigned long TS_channel=YOUR_CHANNEL_ID; // this is the ID of the channel you created for this application (https://thingspeak.com/channels/[TS_CHANNEL])
const char* TS_writekey="[YOUR-THINGSPEAK-WRITE-KEY]"; // this is your API key used for writing to thingspeak

// MQTT Settings
// leave blank if you don't use a password
const char* mqtt_user="[YOUR-MQTT-USERNAME]";
const char* mqtt_password="[YOUT-MQTT-PASSWORD]";
// here is a tutorial for set user and password for mosquitto: http://www.steves-internet-guide.com/mqtt-username-password-example/
