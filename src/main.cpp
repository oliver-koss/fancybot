#include "Arduino.h"

//#include "AiEsp32RotaryEncoder.h"
//#include <LiquidCrystal_I2C.h>
//#include <TinyGPSPlus.h>
#include <RTClib.h>

#include <Wire.h>
#include <Adafruit_Sensor.h> 
#include <Adafruit_ADXL345_U.h>

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


LiquidCrystal_I2C lcd(0x27, 16, 2);
LiquidCrystal_I2CAdapter lcdAdapter(&lcd);
CharacterDisplayRenderer renderer(&lcdAdapter, 16, 2);
LcdMenu menu(renderer);
SimpleRotaryAdapter encoderA(&menu, &encoder);


MENU_SCREEN(settingsScreen, settingsItems,
    ITEM_WIDGET(
        "Backlight",
        [](bool backlight) { lcd.setBacklight(backlight); },
        WIDGET_BOOL(true, "  On", " Off", "%s")),
//    ITEM_VALUE("RTC", rtc_running, "%i"),
    ITEM_BASIC("Contrast2"));

MENU_SCREEN(Dashboard, DashboardItems,
    ITEM_VALUE("Test: ", test, "%i"),
    ITEM_VALUE("X: ", event.acceleration.x, "%f"),
    ITEM_VALUE("Y: ", event.acceleration.y, "%f"),
    ITEM_VALUE("Z: ", event.acceleration.z, "%f"),
    ITEM_SUBMENU("Settings", settingsScreen));


FILE *logfile;

bool gps_enabled = false;
bool adxl345_enabled = false;
bool logging_enabled = false;
bool rtc_enabled = false;
bool display_enabled = false;


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


void adxl345(void* parameter)
{
    while(true){
 
        accel.getEvent(&event);
        Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
        Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
        Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");
        Serial.println("m/s^2 ");
        delay(500);
    }
}

void json_logger()
{
//    esp_timer_get_time()
}

void setup()
{
    Serial.begin(9600);

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
        logfile = fopen("logs.json", "rw");
        if (logfile != NULL)
        {
            logging_enabled = true;
        }
    }

/*    
    File file = SD.open("/");
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
*/


    if (display_enabled)
    {
        renderer.begin();
        menu.setScreen(Dashboard);

        lcd.init();
        lcd.backlight();

        printl("FancyBot", 0, 0);
        printl("Version 0.0.1", 0, 1);
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
    if (display_enabled)
    {
        encoderA.observe();
        menu.poll();
    }

}
