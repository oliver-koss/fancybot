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

//AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_DT_PIN, ROTARY_ENCODER_CLK_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS);
SimpleRotary encoder(ROTARY_ENCODER_DT_PIN, ROTARY_ENCODER_CLK_PIN, ROTARY_ENCODER_BUTTON_PIN);

LiquidCrystal_I2C lcd(0x27, 16, 2);
//RTC_DS1307 rtc;

Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
sensors_event_t event;

int test = 1;


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


LiquidCrystal_I2CAdapter lcdAdapter(&lcd);
CharacterDisplayRenderer renderer(&lcdAdapter, 16, 2);
LcdMenu menu(renderer);
SimpleRotaryAdapter encoderA(&menu, &encoder);


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
        delay(1000);
    }
 
}

void taskTwo( void * parameter)
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

void adxl345(sensors_event_t* event)
{
//    accel.getEvent(&event);
}

void setup()
{
    Serial.begin(9600);

//    rtc.begin();

/*    
    rotaryEncoder.begin();
	rotaryEncoder.setup(readEncoderISR);
    rotaryEncoder.setBoundaries(0, 8, true);
    rotaryEncoder.disableAcceleration();
*/

    delay(2000);
    if(!SD.begin(5)){
        Serial.println("Cannot mount SD Card!");
    }

    uint8_t cardType = SD.cardType();

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

/*    
    File file = SD.open("/");
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
*/

    SD.mkdir("/lol");

    lcd.init();
    lcd.backlight();

    printl("FancyBot", 0, 0);
    printl("Version 0.0.1", 0, 1);


    accel.begin();
/*
    if (accel.begin())
    {
        xTaskCreate(adxl345, "adxl345", 10000, &event, 1, NULL); 
    }
*/

  xTaskCreate(
                    taskOne,          /* Task function. */
                    "TaskOne",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */

  xTaskCreate(
                    taskTwo,          /* Task function. */
                    "TaskTwo",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */


    renderer.begin();
    delay(1000);
    menu.setScreen(Dashboard);
                    
}


void loop()
{
    encoderA.observe();
    menu.poll();
}