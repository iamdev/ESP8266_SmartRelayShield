#ifndef _MOD_MQTT_H
#define _MOD_MQTT_H

#include "config.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);
bool mqtt_initialized = false;

#define SUBSCRIBE_HANDLER(fn) void(*fn)(byte * payload,int length)
struct subscribe_handler_t{
  const char * topic;
  SUBSCRIBE_HANDLER(handler);
};

subscribe_handler_t * subscribe_handlers;
int subscribeCount = 0;
void mqtt_subscribe(char * topic,SUBSCRIBE_HANDLER(handler)){
  subscribe_handlers[subscribeCount].topic = topic;
  subscribe_handlers[subscribeCount].handler = handler;
  subscribeCount++;
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
 Serial.print("Message arrived [");
 Serial.print(topic);
 Serial.print("] ");
 char tmp[128];
 strncpy(tmp,(char *)payload,length);
 tmp[length]=0;
 Serial.println(tmp);

 for(int i=0;i<subscribeCount;i++){
    if(strcmp(subscribe_handlers[i].topic,topic) == 0){
      subscribe_handlers[i].handler((byte*)tmp,length);
      break;
    }
 }  
 Serial.println();
}

int mqtt_connect(int timeout) {
if(mqttClient.connected())return 1;
unsigned long ms = millis();

while (ms < millis()+timeout && !mqttClient.connected()) {
 Serial.print("Attempting MQTT connection...");
 if (mqttClient.connect("ESP8266-RelayShield")) {
  Serial.println("connected");
  for(int i=0;i<subscribeCount;i++){
    mqttClient.subscribe(subscribe_handlers[i].topic);  
  }
  return 1;
 } else {
  Serial.print("failed, rc=");
  Serial.print(mqttClient.state());
  Serial.println(" try again in 5 seconds");
  // Wait 5 seconds before retrying
  delay(1000);
  }
 }
 Serial.println("\nTimeout.");
 return 0;
}

bool mqtt_init(){
  if (mqtt_initialized && WiFi.status() == WL_CONNECTED) return true;
  subscribe_handlers = (subscribe_handler_t *) realloc(subscribe_handlers, (MQTT_MAX_SUBSCRIBE + 1) * sizeof(subscribe_handler_t));  
  mqtt_initialized = false;
  WiFi.begin(wifi_ssid, wifi_password);
  unsigned long t = millis();
  int i=0;
  while (millis()<(t+wifi_connection_timeout) && WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if(i%20==0)Serial.println();
  }
  Serial.println();
  if(WiFi.status() == WL_CONNECTED){  
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());    
    mqttClient.setServer(mqtt_server, mqtt_port);
    mqttClient.setCallback(mqtt_callback);
    mqtt_initialized = true;
    return true;
  }
  return false;
}

#endif
