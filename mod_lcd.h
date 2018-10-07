#ifndef _MOD_LCD_H
#define _MOD_LCD_H
#include "config.h"
#if HAS_LIQUID_CRYSTAL_LCD >0
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(LCD_ADDRESS,LCD_CHARS,LCD_ROWS); 
bool is_lcd_init = false;

void lcd_init(){
    //Wire.beginTransmission(LCD_ADDRESS);
    //if(Wire.endTransmission()!=0){
      lcd.init(); // initialize the lcd    
      lcd.backlight();
      is_lcd_init = true;
    //}
}
#define lcd_setCursor(x,y) { \
  if(is_lcd_init)lcd.setCursor(x,y); \
}

#define lcd_backlight(__VA_ARGS__) { \
  if(is_lcd_init)lcd.backlight(__VA_ARGS__); \
}

#define lcd_print(...){ \
  if(is_lcd_init)lcd.print(__VA_ARGS__); \
}
#define lcd_println(...){ \
  if(is_lcd_init)lcd.println(__VA_ARGS__); \
}
#else
#define lcd_init()
#define lcd_setCursor(x,y)
#define lcd_print(...)
#define lcd_println(...)
#define lcd_backlight(__VA_ARGS__)
#endif

#endif
