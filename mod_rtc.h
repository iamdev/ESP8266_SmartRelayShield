#ifndef _MOD_RTC_H
#define _MOD_RTC_H
#include "config.h"

struct ts_t {
    uint8_t second;         /* seconds */
    uint8_t minute;         /* minutes */
    uint8_t hour;           /* hours */
    uint8_t wday;           /* day of week */
    uint8_t day;            /* day of month */
    uint8_t month;          /* month */
    uint8_t year_s;         /* short year 2 last digit*/
    int16_t year;           /* year */
};

enum AlarmControlFlag{
  ALARM_SECOND,
  ALARM_MINUTE,
  ALARM_HOUR,
  ALARM_DAY,
  ALARM_DATE  
};

struct alarm_t{
  struct ts_t datetime;
  enum AlarmControlFlag flag;
  long delay;
  bool repeat;
  bool active;  
};
/*
void rtc_clear_alarm();
void rtc_init();
struct ts_t rtc_getDateTime();
float rtc_getTemp();
struct ts_t rtc_parseStrDateTime(const char * str);
struct ts_t rtc_parseStrTime(const char * str);
void rtc_setDateTime(struct ts_t dt);
void rtc_setAlarm(struct ts_t ts);
*/

/**********************************************************************************************************/

#define SECONDS_FROM_1970_TO_2000 946684800

// timekeeping registers
#define DS3231_TIME_CAL_ADDR        0x00
#define DS3231_ALARM1_ADDR          0x07
#define DS3231_ALARM2_ADDR          0x0B
#define DS3231_CONTROL_ADDR         0x0E
#define DS3231_STATUS_ADDR          0x0F
#define DS3231_AGING_OFFSET_ADDR    0x10
#define DS3231_TEMPERATURE_ADDR     0x11

// control register bits
#define DS3231_A1IE     0x1
#define DS3231_A2IE     0x2
#define DS3231_INTCN    0x4

// status register bits
#define DS3231_OSF      0x80

bool rtc_alarm = false;

void DS3231_set_addr(const uint8_t addr, const uint8_t val)
{
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(addr);
    Wire.write(val);
    Wire.endTransmission();
}

uint8_t DS3231_get_addr(const uint8_t addr)
{
    uint8_t rv;

    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(addr);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_I2C_ADDR, 1);
    rv = Wire.read();

    return rv;
}


void DS3231_set_creg(const uint8_t val)
{
    DS3231_set_addr(DS3231_CONTROL_ADDR, val);
}

void DS3231_set_sreg(const uint8_t val)
{
    DS3231_set_addr(DS3231_STATUS_ADDR, val);
}

uint8_t DS3231_get_sreg(void)
{
    uint8_t rv;
    rv = DS3231_get_addr(DS3231_STATUS_ADDR);
    return rv;
}

float DS3231_get_treg()
{
    float rv;
    uint8_t temp_msb, temp_lsb;
    int8_t nint;

    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_TEMPERATURE_ADDR);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_I2C_ADDR, 2);
    temp_msb = Wire.read();
    temp_lsb = Wire.read() >> 6;

    if ((temp_msb & 0x80) != 0)
        nint = temp_msb | ~((1 << 8) - 1);      // if negative get two's complement
    else
        nint = temp_msb;

    rv = 0.25 * temp_lsb + nint;

    return rv;
}

uint8_t dectobcd(const uint8_t val)
{
    return ((val / 10 * 16) + (val % 10));
}

uint8_t bcdtodec(const uint8_t val)
{
    return ((val / 16 * 10) + (val % 16));
}

uint8_t inp2toi(char *cmd, const uint16_t seek)
{
    uint8_t rv;
    rv = (cmd[seek] - 48) * 10 + cmd[seek + 1] - 48;
    return rv;
}

void DS3231_get(struct ts_t *t)
{
    uint8_t TimeDate[7];        //second,minute,hour,dow,day,month,year
    uint8_t century = 0;
    uint8_t i, n;
    uint16_t year_full;

    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_TIME_CAL_ADDR);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_I2C_ADDR, 7);

    for (i = 0; i <= 6; i++) {
        n = Wire.read();
        if (i == 5) {
            TimeDate[5] = bcdtodec(n & 0x1F);
            century = (n & 0x80) >> 7;
        } else
            TimeDate[i] = bcdtodec(n);
    }

    if (century == 1) {
        year_full = 2000 + TimeDate[6];
    } else {
        year_full = 1900 + TimeDate[6];
    }

    t->second = TimeDate[0];
    t->minute = TimeDate[1];
    t->hour = TimeDate[2];
    t->day = TimeDate[4];
    t->month = TimeDate[5];
    t->year = year_full;
    t->wday = TimeDate[3];
    t->year_s = TimeDate[6];
}

void DS3231_set(struct ts_t t)
{
    uint8_t i, century;
    if (t.year >= 2000) {
        century = 0x80;
        t.year_s = t.year - 2000;
    } else {
        century = 0;
        t.year_s = t.year - 1900;
    }
    uint8_t TimeDate[7] = { t.second, t.minute, t.hour, t.wday, t.day, t.month, t.year_s };
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_TIME_CAL_ADDR);
    for (i = 0; i <= 6; i++) {
        TimeDate[i] = dectobcd(TimeDate[i]);
        if (i == 5)
            TimeDate[5] += century;
        Wire.write(TimeDate[i]);
    }
    Wire.endTransmission();
}

/**********************************************************************************************************/
/**********************************************************************************************************/

byte rtc_DayOfWeek(int y, byte m, byte d) {   // y > 1752, 1 <= m <= 12
  static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};  
  y -= m < 3;
  return ((y + y/4 - y/100 + y/400 + t[m-1] + d) % 7) + 1; // 01 - 07, 01 = Sunday
}

void rtc_clear_alarm(){
  rtc_alarm = false;
  DS3231_set_addr(DS3231_STATUS_ADDR,(DS3231_get_addr(DS3231_STATUS_ADDR) & ~DS3231_A1IE)); //Clear alarm flag && Interrupt
  DS3231_set_addr(DS3231_STATUS_ADDR,(DS3231_get_addr(DS3231_STATUS_ADDR) & ~DS3231_A2IE)); //Clear alarm flag && Interrupt
  DS3231_set_creg(DS3231_INTCN & ~DS3231_A1IE);
  DS3231_set_creg(DS3231_INTCN & ~DS3231_A2IE);
}

void rtc_alarm_interrupt_handler(){
  rtc_clear_alarm();
  rtc_alarm = true;
}

void rtc_init(){
  Wire.begin();
  DS3231_set_creg(DS3231_INTCN);
  rtc_clear_alarm();
  pinMode(RTC_INT_PIN,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RTC_INT_PIN), rtc_alarm_interrupt_handler, FALLING);  
}

struct ts_t rtc_getDateTime(){
  struct ts_t t;
  DS3231_get(&t);
  return t;
}

float rtc_getTemp(){
  return DS3231_get_treg();
}

struct ts_t rtc_parseStrDateTime(const char * str){
  struct ts_t dt = rtc_getDateTime();
  uint8_t buf[6] = {dt.year,dt.month,dt.day,dt.hour,dt.minute,dt.second};  
  char del[] = ":/- ";
  char * tok= strtok((char*)str,del);
  int i = 0;
  bool yearFirst = false;
  while(i<6 && tok){      
      int v = atol(tok);      
      if(i==0 && v>99) yearFirst = true;
      if(v>99)v=v%2000;
      buf[i++] = v;
      tok = strtok(NULL,del);
  }
  if(yearFirst){
    dt.year = buf[0]+2000;
    dt.month = buf[1];
    dt.day = buf[2];
  }else{
    dt.year = buf[2]+2000;
    dt.month = buf[1];
    dt.day = buf[0];    
  }
  dt.hour = buf[3];
  dt.minute = buf[4];
  dt.second = buf[5];
  dt.wday = rtc_DayOfWeek(dt.year,dt.month,dt.day);
  return dt;
}

struct ts_t rtc_parseStrTime(const char * str){
  struct ts_t dt = rtc_getDateTime();
  uint8_t buf[3] = {dt.hour,dt.minute,dt.second};  
  char del[] = ":/- ";
  char * tok= strtok((char*)str,del);
  int i = 0;
  while(i<3 && tok){      
      int v = atol(tok);      
      if(i==0)v=v%2000;
      buf[i++] = v;
      tok = strtok(NULL,del);
  }
  dt.hour = buf[0];
  dt.minute = buf[1];
  dt.second = buf[2];
  return dt;
}

void rtc_setDateTime(struct ts_t dt){
  DS3231_set(dt);
}

void rtc_setAlarm(struct alarm_t a)
{    
    Wire.beginTransmission(DS3231_I2C_ADDR);
    Wire.write(DS3231_ALARM1_ADDR);
    Wire.write(0x0|dectobcd(a.datetime.second));
    Wire.write(a.flag==ALARM_MINUTE?0:0x80|dectobcd(a.datetime.minute));
    Wire.write(a.flag==ALARM_HOUR?0:0x80|dectobcd(a.datetime.hour));
    Wire.write(0x80|dectobcd(a.datetime.wday));    
    Wire.endTransmission();    
    DS3231_set_creg(DS3231_INTCN|DS3231_A1IE);    
}
#endif


