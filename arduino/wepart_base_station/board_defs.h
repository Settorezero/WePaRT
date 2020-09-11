/*
 * Pin definitions for LILYGO® TTGO LoRa32 V2.1_1.6
 * This file cannot be used with other boards even if are similar
 * By Giovanni Bernardo (https://www.settorezero.com)
 * Read the full article here:
 * https://www.settorezero.com/wordpress/lilygo_ttgo_lora32_esp32_point_to_point
 * if you want to share the code please mention: Giovanni Bernardo of settorezero.com
 */

// LoRa Chip, we'll use the LoRa on the VSPI module (SPI3)
// since the VSPI is normally tied to pins 5, 18, 19 and 23
// even if we'll re-arrange them
#define LORA_SCK      5
#define LORA_MISO     19
#define LORA_MOSI     27
#define LORA_SS       18
#define LORA_RST      23
#define LORA_DI0      26
#define LORA_BAND     915E6  // Values: 433E6, 866E6, 915E6 => Check you country, module and Antenna!

// OLED Display (SSD1306 128x64)
#define OLED_SDA      21
#define OLED_SCL      22
#define OLED_ADDRESS  0x3C

// SD Card, wel'll use the SD Card on the HSPI module (SPI2)
// since the HSPI is normally tied to pins 12, 13, 14 and 15
// even if we'll re-arrange then
#define SDCARD_MOSI   15  // CMD
#define SDCARD_MISO   2   // DAT0
#define SDCARD_SCK    14
#define SDCARD_CS     13  // DAT3
#define SDCARD_DAT1   4   // not really used
#define SDCARD_DAT2   12  // not really used

// Others
#define LED           25 // the green led near the on/off button
#define VBAT          35 // Analog Input from 1:2 voltage divider on LiPo Battery
