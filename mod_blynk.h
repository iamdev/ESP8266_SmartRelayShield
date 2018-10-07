#include "config.h"
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

bool blynk_initialized = false;
void blynk_init(){
  pinMode(D0,INPUT_PULLUP);
  if(!digitalRead(D0)){
    pinMode(LED_PIN,OUTPUT);
    digitalWrite(LED_PIN,LOW);    
    WiFi.mode(WIFI_STA); 
    int cnt = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (cnt++ >= 10) {
        WiFi.beginSmartConfig();
        while (1) {
          delay(1000);
          if (WiFi.smartConfigDone()) {
            Serial.println();
            Serial.println("SmartConfig: Success");
            break;
          }
          Serial.print("|");
        }
      }
    }
    WiFi.printDiag(Serial);
    digitalWrite(LED_PIN,HIGH);    
  }  
  Blynk.config(blynk_auth,blynk_server,blynk_port);
  blynk_initialized =true;

}

