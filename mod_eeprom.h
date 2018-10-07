#ifndef _MOD_EEPROM_H
#define _MOD_EEPROM_H
#include "config.h"

void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
    int rdata = data;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(rdata);
    Wire.endTransmission();
}

void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddresspage >> 8)); // MSB
    Wire.write((int)(eeaddresspage & 0xFF)); // LSB
    byte c;
    for ( c = 0; c < length; c++)
        Wire.write(data[c]);
    Wire.endTransmission();
}

byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {
    byte rdata = 0xFF;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,1);
    if (Wire.available()) rdata = Wire.read();
    return rdata;
}

void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ) {
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8)); // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(deviceaddress,length);
    int c = 0;
    for ( c = 0; c < length; c++ )
        if (Wire.available()) buffer[c] = Wire.read();
}

byte eeprom_read(unsigned int eeaddress){
  byte data;
  i2c_eeprom_read_buffer(EEPROM_DEVICE_ADDR,eeaddress,&data,1);
  return data;
}

void eeprom_write(unsigned int eeaddress, byte data){
  i2c_eeprom_write_page(EEPROM_DEVICE_ADDR,eeaddress,&data,1);
}


void eeprom_read_object(unsigned int eeaddress,void * obj,int length){
  i2c_eeprom_read_buffer(EEPROM_DEVICE_ADDR,eeaddress,(byte *)obj,length);
}

void eeprom_write_object(unsigned int eeaddress,void * obj,int length){
  i2c_eeprom_write_page(EEPROM_DEVICE_ADDR,eeaddress,(byte*)obj,length);
}


int eeprom_readInt(unsigned int eeaddress){
  int data;
  i2c_eeprom_read_buffer(EEPROM_DEVICE_ADDR,eeaddress,(byte*)&data,2);
  return data;
}

void eeprom_writeInt(unsigned int eeaddress, int data){
  i2c_eeprom_write_page(EEPROM_DEVICE_ADDR,eeaddress,(byte*)&data,2);
}

long eeprom_readLong(unsigned int eeaddress){
  long data;
  i2c_eeprom_read_buffer(EEPROM_DEVICE_ADDR,eeaddress,(byte*)&data,4);
  return data;
}

void eeprom_writeLong(unsigned int eeaddress, long data){
  i2c_eeprom_write_page(EEPROM_DEVICE_ADDR,eeaddress,(byte*)&data,4);
}

#endif
