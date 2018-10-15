# ESP8266_SmartRelayShield
Example code for MakeArduino-ESP8266 Smart-Relay Shield

<h2>Library Required</h2>
<ul>

<li type="disc"><a href="https://github.com/blynkkk/blynk-library" target="_blank">Blynk</a>
<li type="disc"><a href="https://github.com/gmag11/NtpClient" target="_blank">NtpClientLib</a> by German Martin
<li type="disc"><a href="https://github.com/marcoschwartz/LiquidCrystal_I2C" target="_blank">LiquidCrystal I2C</a> by Frank de Brabander 
</ul>

<b>Edit all user config in "config.h"</b>
---
<h3>Serial Command:</h3>
<ul>
<li> D yyyy-mm-dd hh:mm:ss         -> Set Clock to RTC</li>
<li> T HH:mm:ss                    -> Set Time to RTC</li>
<li> A S ss                        -> Set Alarm to Toggle Relay 1 (alarm when Second matched)</li>
<li> A M mm:ss                     -> Set Alarm to Toggle Relay 1 (alarm when Minute and Second matched)</li>
<li> A H hh:mm:ss                  -> Set Alarm to Toggle Relay 1 (alarm when match all Hour,Minute and Second)</li>
</ul>

<h2>Blynk Example</h2>
<img src="./img/blynk_clone_40184036.png" width="150px"/>

<h2>Board Schematic</h1>
<img src="./img/Schematic_20181009202613.png"/>


