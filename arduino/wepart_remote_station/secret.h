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

// Pushover settings
// If you want to use Pushover for Push Notifications, you must install also
// Pushover by Arduino Hannover - https://github.com/ArduinoHannover/Pushover
// (you can copy the two files Pushover.cpp and Pushover.h in this folder)
const char* pushover_token="[PUSHOVER-TOKEN]"; // token of your created application
const char* pushover_user="[PUSHOVER-USER]"; // your personal user key
const char* pushover_device=""; // leave blank if you don't want set a device and send the message to all registered devices
const char* pushover_sound="alien"; // here are listed all sounds: https://pushover.net/api#sounds
int8_t pushover_priority=1; // -2, -1, 0, 1, 2 (-2:lower priority, 2:highest priority)