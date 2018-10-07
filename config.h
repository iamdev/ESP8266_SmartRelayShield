#ifndef _CONFIG_H
#define _CONFIG_H

#define DEBUG 1
#define EEPROM_DEVICE_ADDR  0x50
#define EEPROM_SIZE         0x8000

#define PCF8547_I2C_ADDR    0x21
#define DS3231_I2C_ADDR     0x68
#define PCF8574_I2C_ALTADDR 0x18

#define LED_PIN             D4
#define IO_INT_PIN          D3
#define RTC_INT_PIN         D4

#define RELAY_STAUS_DATA_ADDRESS  0

#define HAS_LIQUID_CRYSTAL_LCD   1
#define LCD_ADDRESS         0x3F
#define LCD_CHARS           16
#define LCD_ROWS            2

#define PLATFORM_BLINK
//#define PLATFORM_MQTT

char blynk_auth[] = "blynk_token";
#define blynk_server "192.168.1.100"
#define blynk_port 8080

int button_delay = 50;

int wifi_connection_timeout = 30000;
const char* wifi_ssid = "wifi-ssid";
const char* wifi_password = "wifi-pass";
#define mqtt_server "192.168.1.100"
#define mqtt_port 1883
#define mqtt_user "TEST"
#define mqtt_password "12345"

#define MQTT_MAX_SUBSCRIBE 16

#endif

