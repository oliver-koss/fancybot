#include "Arduino.h"

//#include "AiEsp32RotaryEncoder.h"
//#include <LiquidCrystal_I2C.h>
//#include <TinyGPSPlus.h>
#include <RTClib.h>

#include <Wire.h>
#include <Adafruit_Sensor.h> 
#include <Adafruit_ADXL345_U.h>

#include <TinyGPS++.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <ArduinoJson.h>

#include <SimpleRotary.h>
#include <input/SimpleRotaryAdapter.h>

#include <ItemSubMenu.h>
#include <LcdMenu.h>
#include <MenuScreen.h>
#include <display/LiquidCrystal_I2CAdapter.h>
#include <renderer/CharacterDisplayRenderer.h>
#include <ItemWidget.h>
#include <widget/WidgetBool.h>
#include <widget/WidgetList.h>
#include <widget/WidgetRange.h>
#include <ItemValue.h>

#define ROTARY_ENCODER_DT_PIN 27
#define ROTARY_ENCODER_CLK_PIN 26
#define ROTARY_ENCODER_BUTTON_PIN 14
#define ROTARY_ENCODER_STEPS 4

SimpleRotary encoder(ROTARY_ENCODER_DT_PIN, ROTARY_ENCODER_CLK_PIN, ROTARY_ENCODER_BUTTON_PIN);


Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
sensors_event_t event;

int test = 1;
int hour = 0;
int minute = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);
LiquidCrystal_I2CAdapter lcdAdapter(&lcd);
CharacterDisplayRenderer renderer(&lcdAdapter, 16, 2);
LcdMenu menu(renderer);
SimpleRotaryAdapter encoderA(&menu, &encoder);

bool gps_enabled = false;
bool adxl345_enabled = false;
bool logging_enabled = false;
bool rtc_enabled = false;
bool rtc_set = false;
bool display_enabled = false;

MENU_SCREEN(settingsScreen, settingsItems,
    ITEM_WIDGET(
        "Backlight",
        [](bool backlight) { lcd.setBacklight(backlight); },
        WIDGET_BOOL(true, "  On", " Off", "%s")),
//    ITEM_VALUE("RTC", rtc_running, "%i"),
    ITEM_BASIC("Contrast2"));

MENU_SCREEN(Dashboard, DashboardItems,
    ITEM_VALUE("Time: ", rtc_enabled, "%i"),
    ITEM_VALUE("X: ", event.acceleration.x, "%f"),
    ITEM_VALUE("Y: ", event.acceleration.y, "%f"),
    ITEM_VALUE("Z: ", event.acceleration.z, "%f"),
    ITEM_SUBMENU("Settings", settingsScreen));


File logfile;

RTC_DS1307 rtc;
TinyGPSPlus gps;

HardwareSerial gpsSerial(2);

int i2c_valid(byte address)
{
    Wire.beginTransmission(address);
    int error = Wire.endTransmission();

    return error;
}

void printl(char* string, int x, int y)
{
    lcd.setCursor(x, y);
    lcd.print(string);
}


void taskOne( void * parameter )
{
 
    while(true){
 
//        Serial.println(esp_timer_get_time());
        test++;
        delay(100);
    }
 
}

void gps_task(void* parameter)
{
    while(true)
    {
        gps.encode(gpsSerial.read());
        if (gps.time.isValid() && !rtc_set && rtc_enabled)
        {
            DateTime dt(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
            rtc.adjust(dt);
         //   rtc.adjust(gps.time.second(), gps.time.minute(), gps.time.hour(), gps.date.day(), gps.date.month(), gps.date.year());
            rtc_set = true;
            Serial.print("Time set!");
        }
    }
}

void time(void* parameter)
{
    unsigned long millis_now = 0;
    while(true)
    {
        if((millis() - millis_now) > 10000)
        {
            DateTime now = rtc.now();

            hour = now.hour();
            minute = now.minute();

            /*
            Serial.print(now.hour(), DEC);
            Serial.print(':');
            Serial.print(now.minute(), DEC);
            Serial.print(':');
            Serial.println(now.second(), DEC);
            */

            millis_now = millis();
        }
    }
}

void lcd_menu(void* parameter)
{
    while(true)
    {
        encoderA.observe();
        menu.poll();
    }
}

void adxl345(void* parameter)
{
    while(true){
 
        accel.getEvent(&event);
        Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
        Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
        Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");
        Serial.println("m/s^2 ");
        delay(500);

        if (logging_enabled)
        {
            logfile = SD.open("/logs.json");
            logfile.print("X: ");
            logfile.print(event.acceleration.x);
            logfile.print("\n");
            Serial.print("loggged");
            logfile.close();
        }
    }
}

void json_logger()
{
//    esp_timer_get_time()
}

void setup()
{
    Serial.begin(9600);
    gpsSerial.begin(9600, SERIAL_8N1, 33, 25);

    delay(2000);
    Wire.begin();
    int i2c = i2c_valid(0x27);
    if (!i2c)
    {
        Serial.println("i2c device at 0x27 found");
        display_enabled = true;
    } else {
        Serial.println("No i2c device found");
    }

    delay(2000);
    if(!SD.begin(5)){
        Serial.println("Cannot mount SD Card!");
    } else {
        Serial.println("SD Card mounted.");
        logfile = SD.open("/logs.json", FILE_WRITE);
        if (SD.exists("/logs.json"))
        {
            logging_enabled = true;
            logfile.println("lol");
            Serial.println("logged");
            logfile.close();
        }
    }

/*    
    File file = SD.open("/");
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
*/

    if (rtc.begin())
    {
        rtc_enabled = true;
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
//        xTaskCreate(time, "time", 10000, NULL, 1, NULL);
    }

    if (display_enabled)
    {
        renderer.begin();
        menu.setScreen(Dashboard);

        lcd.init();
        lcd.backlight();

        printl("FancyBot", 0, 0);
        printl("Version 0.0.1", 0, 1);

        xTaskCreate(lcd_menu, "lcd_menu", 10000, NULL, 1, NULL);
    }



    if (accel.begin())
    {
        adxl345_enabled = true;
        xTaskCreate(adxl345, "adxl345", 10000, NULL, 1, NULL);
    }

  xTaskCreate(
                    taskOne,          /* Task function. */
                    "TaskOne",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */


//  xTaskCreate(
//                    taskTwo,          /* Task function. */
//                    "TaskTwo",        /* String with name of task. */
//                    10000,            /* Stack size in bytes. */
//                    NULL,             /* Parameter passed as input of the task */
//                    1,                /* Priority of the task. */
//                    NULL);            /* Task handle. */

}


void loop()
{
    /*
    if (display_enabled)
    {
        encoderA.observe();
        menu.poll();
    }
    */

/*
  while (gpsSerial.available() > 0){
    // get the byte data from the GPS
    char gpsData = gpsSerial.read();
    Serial.print(gpsData);
  }
  delay(1000);
  Serial.println("-------------------------------");
  */
}
