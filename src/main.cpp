#include "Arduino.h"

//#include "AiEsp32RotaryEncoder.h"
//#include <LiquidCrystal_I2C.h>
//#include <TinyGPSPlus.h>
#include <RTClib.h>

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
RTC_DS1307 rtc;

MENU_SCREEN(settingsScreen, settingsItems,
    ITEM_WIDGET(
        "Backlight",
        [](bool backlight) { lcd.setBacklight(backlight); },
        WIDGET_BOOL(true, "  On", " Off", "%s")),
//    ITEM_VALUE("RTC", rtc_running, "%i"),
    ITEM_BASIC("Contrast2"));



MENU_SCREEN(mainScreen, mainItems,
    ITEM_BASIC("Dashboard"),
    ITEM_SUBMENU("Settings", settingsScreen),
    ITEM_BASIC("Test 1"),
    ITEM_BASIC("Test 2"),
    ITEM_BASIC("Test 3"),
    ITEM_BASIC("Test 4"),
    ITEM_BASIC("Test 5"),
    ITEM_BASIC("Test 6"),
    ITEM_BASIC("Test 7"));



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
 
        Serial.println("Hello from task 1");
        delay(1000);
    }
 
    Serial.println("Ending task 1");
    vTaskDelete( NULL );
 
}

void setup()
{
    Serial.begin(9600);

    rtc.begin();

    renderer.begin();
    menu.setScreen(mainScreen);

/*    
    rotaryEncoder.begin();
	rotaryEncoder.setup(readEncoderISR);
    rotaryEncoder.setBoundaries(0, 8, true);
    rotaryEncoder.disableAcceleration();
*/

    lcd.init();
    lcd.backlight();

//    printl("FancyBot", 0, 0);
//    printl("Version 0.0.1", 0, 1);

  xTaskCreate(
                    taskOne,          /* Task function. */
                    "TaskOne",        /* String with name of task. */
                    10000,            /* Stack size in bytes. */
                    NULL,             /* Parameter passed as input of the task */
                    1,                /* Priority of the task. */
                    NULL);            /* Task handle. */

}


void loop()
{
    encoderA.observe();
    //LcdMenu::poll();
}