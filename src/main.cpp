#include "Arduino.h"

#include <LiquidCrystal_I2C.h>
#include <TinyGPSPlus.h>
#include <RTClib.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

void printl(char* string, int x, int y)
{
    lcd.setCursor(x, y);
    lcd.print(string);
}

void setup()
{
    Serial.begin(9600);
    Serial1.begin(9600, SERIAL_8N1, 1, 3);

    rtc.begin();

    lcd.init();
    lcd.backlight();

    printl("FancyBot", 0, 0);
    printl("Version 0.0.1", 0, 1);
}

void loop()
{
    Serial.write(Serial1.read());
}