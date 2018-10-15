#ifndef _MOD_EXPAND_H
#define _MOD_EXPAND_H
#include "config.h"
#include "mod_eeprom.h"
byte bitout = 0xff;
uint8_t pcf8574_addr = 0;
#define EXP_INPUT_CH1   1
#define EXP_INPUT_CH2   2
#define EXP_INPUT_CH3   4
#define EXP_INPUT_CH4   8

#define EXP_OUTPUT_CH1   0
#define EXP_OUTPUT_CH2   1
#define EXP_OUTPUT_CH3   2
#define EXP_OUTPUT_CH4   3

#define toggle_bit(x,b) (~(1<<b)&x) | (~x & (1<<b))

bool exp_input_int = false;
uint8_t exp_input = 0xFF;
struct btn_state_t {
  int state;
  long press_ms;  
};

struct btn_state_t btn_state[4];
void exp_write(int n,int s){
  if(n<1 ||n>4)return;
  n--;
  if(s){
    bitout &= ~(1<<n);
  }else{
    bitout |= (1<<n);
  }
  Serial.print("Set Bit:0b");
  Serial.println(bitout,BIN);
  Wire.beginTransmission(pcf8574_addr);
  Wire.write(bitout|0xF0);
  Wire.endTransmission(); 
  eeprom_write(RELAY_STAUS_DATA_ADDRESS,bitout);
}

int exp_toggle(int n){
  if(n<1 ||n>4)return -1;
  n--;
  bitout = toggle_bit(bitout,n);
  Serial.print("Set Bit:0b");
  Serial.println(bitout,BIN);
  Wire.beginTransmission(pcf8574_addr);
  Wire.write(bitout|0xF0);
  Wire.endTransmission();    
  eeprom_write(RELAY_STAUS_DATA_ADDRESS,bitout);
  return ((~(bitout|0xF0))>>n)&0x1;
}


void exp_interrupt_handler(){
    exp_input_int = true;
}

uint8_t exp_readInput(){
  Wire.beginTransmission( pcf8574_addr );
  Wire.endTransmission();
  Wire.requestFrom(pcf8574_addr, (uint8_t)1 ); // request only one byte
  unsigned long t=millis();
  uint8_t data = 0xFF;
  while ( millis() < t+1000 && Wire.available() == 0 ); // waiting 
  if(Wire.available()){
    data = Wire.read();
  }
  Wire.endTransmission();  
  return data>>4;
}

void exp_init(){
  pinMode(IO_INT_PIN,INPUT);
  attachInterrupt(digitalPinToInterrupt(IO_INT_PIN), exp_interrupt_handler, FALLING); 
  Wire.beginTransmission(PCF8547_I2C_ADDR);
  int error = Wire.endTransmission();
  Serial.print("PCF8574 I2C Addr :");
  pcf8574_addr = PCF8547_I2C_ADDR;
  if(error!=0){
    Wire.beginTransmission(PCF8547_I2C_ADDR|PCF8574_I2C_ALTADDR);
    
    error = Wire.endTransmission();   
    if(error==0){
      pcf8574_addr = PCF8547_I2C_ADDR|PCF8574_I2C_ALTADDR;
      Serial.print(pcf8574_addr,HEX);          
    }else{
      Serial.print("NOT FOUND!!");          
    }
  }else{
    Serial.print(PCF8547_I2C_ADDR,HEX);
  }
  exp_input_int = false; 
  bitout = eeprom_read(RELAY_STAUS_DATA_ADDRESS);
  Wire.beginTransmission(pcf8574_addr);
  Wire.write(bitout|0xF0);
  Wire.endTransmission();      
}

bool exp_out_state(int n){
  if(n<1 ||n>4)return false;
  n--;
  return ((~(bitout|0xF0))>>n)&0x1;
}

uint32_t t_exp_read;
uint32_t t_exp_scan;
void exp_loop(){  
  uint32_t ms = millis();  
  if(exp_input_int || (t_exp_scan+1000)<ms){  
    t_exp_scan = ms;
    if((t_exp_read+2)<ms){
      t_exp_read = ms;
      uint8_t v = exp_readInput();    
      //Serial.print("Input:");
      //Serial.println(v,BIN);
      for(int i=0;i<4;i++){        
        bool p = (v&(1<<i))==0;        
        //if(p){
        //  Serial.print("BTN");
        //  Serial.print(i+1);
        //  Serial.println(" DOWN");      
        //}
        if(btn_state[i].state==0){
           if(p){
              btn_state[i].state = 1;          
              btn_state[i].press_ms = millis();
           }
        }else if(btn_state[i].state==1){
          if(!p){
            btn_state[i].state = 2;          
          }
        }
      }
    }
    exp_input_int = false;    
  }
}

int exp_btn_pressed(int n){
  if(n<1 ||n>4)return 0;
  n--;
  return btn_state[n].state==1?millis()-btn_state[n].press_ms:0;
}
int exp_btn_released(int n){
  if(n<1 ||n>4)return 0;
  n--;
  if(btn_state[n].state==2){
    btn_state[n].state = 0;
    btn_state[n].press_ms = millis()-btn_state[n].press_ms;
    return btn_state[n].press_ms;
  }
  return 0;
}

void exp_btn_clear(int n){
  if(n<1 ||n>4)return;
  n--;
  btn_state[n].state = 0;
}

void exp_btn_clear_all(){
  for(int n=0;n<4;n++)btn_state[n].state = 0;
}

int exp_btn_time(int n){
  if(n<1 ||n>4)return 0;
  n--;
  return btn_state[n].press_ms;
}
#endif
