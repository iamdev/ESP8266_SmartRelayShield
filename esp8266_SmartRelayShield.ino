#include "config.h"

#include <Wire.h>
#include <ESP8266WiFi.h>
#include "mod_ntp.h"
#include <BlynkSimpleEsp8266.h>

#include "SerialCommand.h"
#include "mod_rtc.h"
#include "mod_utility.h"
#include "mod_eeprom.h"
#include "mod_expand.h"    
#include "mod_lcd.h"
#include "mod_blynk.h"
//#include "mod_mqtt.h"

void update_relay_status(int n,int s);

SerialCommand cmd(&Serial,256);

struct relay_alarm{
  ts_t ts_on;
  ts_t ts_off;
  byte channel;
  bool state;
  bool active;
}alarms[4];
#define totalSeconds(ts) (ts.second+ts.minute*60+ts.hour*3600)

int alarm_btn_vpin [] = {V6,V8,V10,V12};

void save_alarm(int n){
  long addr = ALARM_DATA_ADDRESS+(sizeof(relay_alarm)*n);
  int data_size = sizeof(alarms[n]);
  byte * data_ptr = (byte *)&(alarms[n]);
  eeprom_write_object(addr,data_ptr,data_size);
  Serial.print("Write alarm to EEPROM address ");
  Serial.print(addr,DEC);
  Serial.print(" for ");
  Serial.print(data_size);
  Serial.print("bytes : ");
  for(int i=0;i<data_size;i++){
    byte b = *(data_ptr+i);
    if(b<0x10)Serial.print("0");
    Serial.print(*(data_ptr+i),HEX);
    Serial.print(" ");
  }
  Serial.println();
}
void load_alarm(){
  Serial.println("Load Alarm Data");  
  for(int i=0;i<4;i++){    
    int data_size = sizeof(alarms[0]);
    int data_addr = ALARM_DATA_ADDRESS + (i*data_size);
    byte * data_ptr=(byte*)&(alarms[i]);
    eeprom_read_object(data_addr,data_ptr,data_size);
    for(int i=0;i<data_size;i++){      
      byte b = *data_ptr++;
      if(b<0x10)Serial.print("0");
      Serial.print(b,HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

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
    Wire.begin(I2C_SDA,I2C_SCL);    
    delay(100);    
    Serial.println("\n");
    rtc_init();
    exp_init();
    lcd_init();    
    //mqtt_init();
    //mqtt_subscribe("relay",sub_relay);
    cmd.registerCommand("D",setDateTimeRTC);
    cmd.registerCommand("T",setTimeRTC);    
    cmd.registerCommand("A",setAlarmRTC);
    Serial.println("\n");    
    load_alarm();
    for(int i=1;i<=4;i++){      
      update_relay_status(i,exp_out_state(i));
    }

    for(int i=0;i<4;i++){
      Serial.print("Alarm ");
      Serial.print(i+1);
      Serial.print(" ");
      printTime(alarms[i].ts_on);
      Serial.print(" - ");
      printTime(alarms[i].ts_off);
      Serial.print(" : ");
      int act = alarms[i].active ?1:0;
      Serial.println(act? "Active":"Inactive");
      Blynk.virtualWrite(alarm_btn_vpin[alarms[i].channel-1],act);
    }
    
    blynk_init();
    ntp_init();
}    

unsigned long t_read_rtc = 0;
unsigned long t_read_lcd_time = 0;

void update_relay_status(int n,int s){
  lcd_setCursor(11+n,1);
  lcd_print(s?"O":"-");  
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

bool btn[4];
void loop() {
  exp_loop();
  ntp_sync();
  cmd.read();  
  unsigned long ms = millis();
  if(ms > (t_read_lcd_time + 900)){
    lcd_setCursor(2,1);
    lcd_print(" ");
  }
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
        PSTR("%02u:%02u"),
        dt.hour,
        dt.minute);
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

    for(int i=0;i<4;i++){
      if(alarms[i].active && totalSeconds(alarms[i].ts_on)!=totalSeconds(alarms[i].ts_off)){
        if(!alarms[i].state && totalSeconds(dt)>totalSeconds(alarms[i].ts_on) && totalSeconds(dt)<totalSeconds(alarms[i].ts_off))
        {
          alarms[i].state = true;
          exp_write(alarms[i].channel,1);
          update_relay_status(alarms[i].channel,1);
          save_alarm(i);
        }else if(alarms[i].state && totalSeconds(dt)>totalSeconds(alarms[i].ts_off)){
          exp_write(alarms[i].channel,0);
          update_relay_status(alarms[i].channel,0);
          alarms[i].state = false;
          alarms[i].active = false;          
          Blynk.virtualWrite(alarm_btn_vpin[alarms[i].channel-1],0);
          save_alarm(i);
        }
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
        } 
        if(r>0)btn[n-1] = false;
    }
  }

  if(blynk_initialized)Blynk.run();
}

/***************************/
/* Blynk Virtual PIN handler */
/***************************/
void read_btn(int n,const BlynkParam& param){
  int pinValue = param.asInt();
  exp_write(n,pinValue==1);
  update_relay_status(n,pinValue==1);
}

void set_alarm(int n,const BlynkParam& param){
  ts_t startTime = secondToTimeStamp(param[0].asLong());
  ts_t stopTime = secondToTimeStamp(param[1].asLong());
  Serial.print("Set Alarm ");
  Serial.print(n);
  Serial.print(" : ");
  printTime(startTime);
  Serial.print(" to ");
  printTime(stopTime);
  alarms[n-1] = {startTime,stopTime,n,false,true};
  Serial.println();
  save_alarm(n-1);  
}
void toggle_alarm(int n,const BlynkParam& param){
  alarms[n-1].active = param.asInt() != 0; 
  Serial.print("Alarm "); 
  Serial.print(n); 
  Serial.println(alarms[n-1].active?":Active":":Inactive");
  save_alarm(n-1);
}

BLYNK_WRITE(V1)
{
  read_btn(1,param);
}

BLYNK_WRITE(V2)
{
  read_btn(2,param);
}

BLYNK_WRITE(V3)
{
  read_btn(3,param);
}

BLYNK_WRITE(V4)
{
  read_btn(4,param);
}

BLYNK_WRITE(V5)
{
  set_alarm(1,param);
  Blynk.virtualWrite(V6,1);
}

BLYNK_WRITE(V6)
{
  toggle_alarm(1,param);
}

BLYNK_WRITE(V7)
{
  set_alarm(2,param);
  Blynk.virtualWrite(V8,1);
}

BLYNK_WRITE(V8)
{
  toggle_alarm(2,param);
}

BLYNK_WRITE(V9)
{
  set_alarm(3,param);
  Blynk.virtualWrite(V10,1);
}

BLYNK_WRITE(V10)
{
  toggle_alarm(3,param);
}
BLYNK_WRITE(V11)
{
  set_alarm(4,param);
  Blynk.virtualWrite(V12,1);
}

BLYNK_WRITE(V12)
{
  toggle_alarm(4,param);
}

