#ifndef _MOD_UTILITY_H
#define _MOD_UTILITY_H
#include "mod_rtc.h"

#define countof(a) (sizeof(a) / sizeof(a[0]))

int split_int(const char * str,const char * delim, int* ar){
  char * tok= strtok((char*)str,delim);
  int n = 0;
  while(n<3 && tok){      
      int v = atol(tok);
      ar[n++] = v;
      tok = strtok(NULL,delim);
  }
  return n;
}

char*wdays[] = {"","Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
void printDateTime(const ts_t &dt)
{
    char datestring[32];
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%s %02u/%02u/%04u %02u:%02u:%02u"),
            wdays[dt.wday],
            dt.day,
            dt.month,            
            dt.year,
            dt.hour,
            dt.minute,
            dt.second);
    Serial.print(datestring);
}

void printTime(const ts_t &dt)
{
    char datestring[32];
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u:%02u:%02u"),
            dt.hour,
            dt.minute,
            dt.second);
    Serial.print(datestring);
}

struct ts_t secondToTimeStamp(long second){
  struct ts_t ts;  
  ts.hour = second/3600;
  second %=3600;
  ts.minute = second/60;
  ts.second = second%60;   
  return ts;  
}


#endif
