#include <NtpClientLib.h>
#include <ESP8266WiFi.h>
#include "mod_rtc.h"
bool isNtpSyncCompleted = false;
boolean syncEventTriggered = false; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent; // Last triggered event
bool wifiFirstConnected = true;

int timeZone = 7;
int minutesTimeZone = 0; 

void ntp_processNtpSyncEvent (NTPSyncEvent_t event) {
    if (event) {
        Serial.print ("Time Sync error: ");
        if (event == noResponse)
            Serial.println ("NTP server not reachable");
        else if (event == invalidAddress)
            Serial.println ("Invalid NTP server address");
    } else {
        Serial.print ("Got NTP time: ");
        Serial.println (NTP.getTimeDateString (NTP.getLastNTPSync ()));
        time_t t = NTP.getTime ();
        if(year(t)>2000){
          struct ts_t ts;
          ts.hour = hour(t);
          ts.minute = minute(t);
          ts.second = second(t);
          ts.day = day(t);
          ts.month = month(t);
          ts.year = year(t);
          rtc_setDateTime(ts);
        }
    }
}

void ntp_init(){
  Serial.println("NTP sync initializing...");
  NTP.onNTPSyncEvent ([](NTPSyncEvent_t event) {
    ntpEvent = event;
    syncEventTriggered = true;
  });    
}

void ntp_sync(){    
    if (wifiFirstConnected && WiFi.status () == WL_CONNECTED) {
        Serial.println("Start NTP sync...");
        wifiFirstConnected = false;
        NTP.begin (ntpServer, timeZone, false, minutesTimeZone);
        NTP.setInterval (63);
    }
    if (syncEventTriggered) {
        ntp_processNtpSyncEvent (ntpEvent);
        syncEventTriggered = false;
    }
}

