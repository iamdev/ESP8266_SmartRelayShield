#include <Wire.h>
#include "config.h"
#include "SerialCommand.h"
#include "mod_rtc.h"
#include "mod_utility.h"
#include "mod_eeprom.h"
#include "mod_expand.h"    
#include "mod_lcd.h"
#include "mod_blynk.h"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

//#include "mod_mqtt.h"

SerialCommand cmd(&Serial,256);

alarm_t alarms[10];

void setDateTimeRTC(const char*arg){
  Serial.print("Set RTC :");
  Serial.println(arg);
  struct ts_t dt = rtc_parseStrDateTime(arg);   
   
  DS3231_set(dt);
  printDateTime(rtc_getDateTime());
  Serial.println(); 
}

void setTimeRTC(const char*arg){
  Serial.print("Set RTC :");
  Serial.println(arg);
  struct ts_t dt = rtc_parseStrTime(arg);      
  rtc_setDateTime(dt);
  printDateTime(rtc_getDateTime());
  Serial.println(); 
}

void setAlarmRTC(const char * arg){ 
  int n= 0;
  const char*  ptr = arg;
  alarm_t a;
  int ar[10];
  int i=0;
  char s = *ptr++;  
  if(n>=0 && n<10){       
    switch(s){
      case 'S':                
        Serial.print("Set alarm Match Second : ");
        a.flag = ALARM_SECOND;         
        a.datetime.second = atol(ptr);
        Serial.println(a.datetime.second);
        break;
      case 'M':
        Serial.print("Set alarm Match Second and Minute: ");
        Serial.println(ptr);
        i = split_int(ptr,":",ar);
        if(i>1){
           a.datetime.minute = (uint8_t)ar[0];
        }
        if(i>0){
          a.datetime.second = (uint8_t)ar[1];
        }
        a.flag = ALARM_MINUTE; 
        break;
      case 'H':
        Serial.print("Set alarm Match Second and Minute and Hour: ");
        Serial.println(ptr);
        i = split_int(ptr,":",ar);
        if(i>2){
           a.datetime.hour = (uint8_t)ar[0];
        }
        if(i>1){
           a.datetime.minute = (uint8_t)ar[1];
        }
        if(i>0){
          a.datetime.second = (uint8_t)ar[2];
        }
        a.flag = ALARM_HOUR; 
        break;
      case 'D':
        Serial.print("Set alarm Match Date and Time: ");
        Serial.println(ptr);
        i = split_int(ptr,": ",ar);
        if(n>3){
           a.datetime.hour = (uint8_t)ar[0];
        }
        if(i>2){
           a.datetime.hour = (uint8_t)ar[1];
        }
        if(i>1){
           a.datetime.minute = (uint8_t)ar[2];
        }
        if(i>0){
          a.datetime.second = (uint8_t)ar[3];
        }
        a.flag = ALARM_DAY; 
        break;
      case 'C':
        a.datetime = rtc_parseStrTime(ptr);    
        a.flag = ALARM_DATE; 
        break;
    }
    rtc_setAlarm(a);
    Serial.print("Set Alarm ");
    Serial.print(n);
    Serial.print(" :");
    //const ts_t dt = a.datetime; 
    //printDateTime(dt);    
    Serial.println();
  }
}

void setup() {
    Serial.begin(115200);
    Wire.begin(D2,D1);    
    delay(500);    
    Serial.println("\n");
    blynk_init();
    rtc_init();
    exp_init();
    lcd_init();
    //mqtt_init();
    //mqtt_subscribe("relay",sub_relay);
    cmd.registerCommand("D",setDateTimeRTC);
    cmd.registerCommand("T",setTimeRTC);    
    cmd.registerCommand("A",setAlarmRTC);
    Serial.println("\n");
}    

unsigned long t_read_rtc = 0;
unsigned long t_read_lcd_time = 0;

void update_relay_status(int n,int s){
  lcd_setCursor(11+n,1);
  lcd_print(s?"O":"X");
  
}

bool btn[4];
void loop() {
  exp_loop();
  cmd.read();  
  unsigned long ms = millis();
  if(ms > (t_read_lcd_time + 1000)){
    struct ts_t dt = rtc_getDateTime();
    float tem = rtc_getTemp();
    t_read_lcd_time = ms;
    char date_str[16];
    char time_str[16];    
    snprintf_P(date_str, 
        countof(date_str),
        PSTR("%s %02u/%02u/%04u"),
        wdays[dt.wday],
        dt.day,
        dt.month,            
        dt.year);
    snprintf_P(time_str, 
        countof(time_str),
        PSTR("%02u:%02u:%02u"),
        dt.hour,
        dt.minute,            
        dt.second);
        lcd_setCursor(0,0);
        lcd_print(date_str);
        lcd_setCursor(0,1);
        lcd_print(time_str);
    if(ms > (t_read_rtc + 5000)){
        if(dt.second%5==0){
        Serial.print(date_str);
        Serial.print(" ");
        Serial.print(time_str);
        Serial.print(" Temperature:");
        Serial.println(rtc_getTemp(),2);
        t_read_rtc = ms;
      }
    }        
  }
  if(rtc_alarm ){
    Serial.println("Alarm!!!");
    exp_toggle(1);
    rtc_clear_alarm();
  }
  for(int n=1;n<=4;n++){
    if(!btn[n-1] && exp_btn_pressed(n)>button_delay){
        Serial.print("Button ");
        Serial.print(n);
        Serial.println(" Pressed");
        btn[n-1] = true;
    }
    else {
        int r = exp_btn_released(n);
        if(r>button_delay){
          Serial.print("Button ");
          Serial.print(n);
          if(r>2000) Serial.print(" Long ");
          Serial.print(" Released [");
          Serial.print(exp_btn_time(n));      
          Serial.println("ms.]");                    
          int s = exp_toggle(n);
          Serial.print("Toggle Output ");
          Serial.print(n);
          Serial.print(" : ");
          Serial.println(s,BIN);
          update_relay_status(n,s);
          if(s>=0){
            switch(n){
              case 1:
                Blynk.virtualWrite(V1,s);
                break;
              case 2:
                Blynk.virtualWrite(V2,s);
                break;
              case 3:
                Blynk.virtualWrite(V3,s);
                break;
              case 4:
                Blynk.virtualWrite(V4,s);
                break;
            }
          }
        } 
        if(r>0)btn[n-1] = false;
    }
  }

  if(blynk_initialized)Blynk.run();
}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  exp_write(1,pinValue==1); 
  update_relay_status(1,pinValue==1);
}

BLYNK_WRITE(V2)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  exp_write(2,pinValue==1);
  update_relay_status(2,pinValue==1);
}

BLYNK_WRITE(V3)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  exp_write(3,pinValue==1);
  update_relay_status(3,pinValue==1);
}

BLYNK_WRITE(V4)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  exp_write(4,pinValue==1);
  update_relay_status(4,pinValue==1);
}


