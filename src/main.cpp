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

#include <mutex>
#include <queue>

#define ROTARY_ENCODER_DT_PIN 27
#define ROTARY_ENCODER_CLK_PIN 26
#define ROTARY_ENCODER_BUTTON_PIN 32
#define ROTARY_ENCODER_STEPS 4

#define FANCYBOT \
"  _____                            ___.              \r\n" \
"_/ ____\\____    ____   ____ ___..\\_ |   _____/  |_ \r\n" \
"\\   __\\\\  \\  /    \\_/ ___<   |  | |  \\ /  _ \\   __\\\r\n" \
" |  |   /  \\|   |  \\  \\___\\___  | | \\_\\ (  <_> )  |  \r\n" \
" ||  (____  /___|  /\\___  > ____| |___  /\\____/||  \r\n" \
"            \\/     \\/     \\/\\/          \\/            \r\n" \
"(This is why I dont use AI for anything)\r\n\n\n"


SimpleRotary encoder(ROTARY_ENCODER_DT_PIN, ROTARY_ENCODER_CLK_PIN, ROTARY_ENCODER_BUTTON_PIN);


Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();

std::mutex adxl_lock;
sensors_event_t event;

int test = 1;
int hour = 0;
int minute = 0;

char* current_time;
//char* time_string;

typedef struct log_event_struct_ {
    char* type;
    char* sensor;
    bool status;
} log_event_struct;

std::mutex log_event_lock;
std::queue<log_event_struct> log_events;

LiquidCrystal_I2C lcd(0x27, 16, 2);
LiquidCrystal_I2CAdapter lcdAdapter(&lcd);
CharacterDisplayRenderer renderer(&lcdAdapter, 16, 2);
LcdMenu menu(renderer);
SimpleRotaryAdapter encoderA(&menu, &encoder);

bool gps_enabled = true;
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
    ITEM_VALUE("Time", current_time, "%s"),
    ITEM_VALUE("X: ", event.acceleration.x, "%f"),
    ITEM_VALUE("Y: ", event.acceleration.y, "%f"),
    ITEM_VALUE("Z: ", event.acceleration.z, "%f"),
    ITEM_SUBMENU("Settings", settingsScreen));


File logfile;

std::mutex rtc_lock;
std::mutex gps_lock;

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

void log_event(char* type, char* sensor, bool status)
{
    log_event_struct buffer = {type, sensor, status};

    log_event_lock.lock();
    log_events.push(buffer);
    log_event_lock.unlock();
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
        gps_lock.lock();
//        gps.encode(gpsSerial.read());

        while (gpsSerial.available() > 0){
            // get the byte data from the GPS
            gps.encode(gpsSerial.read());
        }

        /*
        while (gpsSerial.available() > 0){
            // get the byte data from the GPS
            char gpsData = gpsSerial.read();
            Serial.print(gpsData);
        }
        */

        if(gps.time.age() > 1500 && gps_enabled)
        {
            log_event("hardware", "gps_working", 0);
            gps_enabled = false;
        }

        if(gps.time.age() < 1500 && !gps_enabled)
        {
            log_event("hardware", "gps_working", 1);
            gps_enabled = true;
        }

        if (gps.time.isValid() && !rtc_set && rtc_enabled)
        {
            DateTime dt(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
            rtc.adjust(dt);
         //   rtc.adjust(gps.time.second(), gps.time.minute(), gps.time.hour(), gps.date.day(), gps.date.month(), gps.date.year());
            rtc_set = true;
            log_event("hardware", "rtc_set", 1);
//            Serial.print("Time set!");
        }
        gps_lock.unlock();

        delay(500);
//        log_event("test", "test_logging", 1);
    }
}

void time(void* parameter)
{
    unsigned long millis_now = 0;
    while(true)
    {
//        if((millis() - millis_now) > 10000)
        if(true)
        {
            DateTime now = rtc.now();

//            hour = now.hour();
//            minute = now.minute();

//            char* timee_string = now.toString("hh:mm:ss");
            char buffer[] = "hh:mm:ss";
            current_time = now.toString(buffer);

            /*
            Serial.print(current_time);
            if (rtc_set)
            {
                Serial.println("!");
            } else {
                Serial.println();
            }
            */

//            Serial.print("Time: ");
//            Serial.println(time_string);

            /*
            Serial.print(now.hour(), DEC);
            Serial.print(':');
            Serial.print(now.minute(), DEC);
            Serial.print(':');
            Serial.println(now.second(), DEC);
            */

//            millis_now = millis();

            delay(500);
        }
    }
}

void lcd_menu(void* parameter)
{
    while(true)
    {
        encoderA.observe();
    }
}

void lcd_menu_poll(void* parameter)
{
    while(true)
    {
        adxl_lock.lock();
        menu.poll();
        adxl_lock.unlock();
        delay(500);
    }
}

void adxl345(void* parameter)
{
    while(true){
 
        adxl_lock.lock();
        accel.getEvent(&event);
        adxl_lock.unlock();
//        Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  ");
//        Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  ");
//        Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  ");
//        Serial.println("m/s^2 ");

        if (logging_enabled)
        {
            /*
            JsonDocument doc;

            doc["type"] = "stats";
            doc["time"] = current_time;
            doc["millis"] = esp_timer_get_time();
            doc["x"] = event.acceleration.x;
            doc["y"] = event.acceleration.y;
            doc["z"] = event.acceleration.z;


            logfile = SD.open("/logs.json", FILE_APPEND);

            serializeJson(doc, logfile);
            serializeJson(doc, Serial);
            Serial.println();

//            logfile.print("X: ");
//            logfile.print(event.acceleration.x);
            logfile.print("\r\n");
//            Serial.print("logged");
            logfile.close();
            */
        }
        delay(500);
    }
}

void json_logger(void* parameter)
{

//    unsigned long millis_now = millis();  //For debugging

    logfile = SD.open("/logs.json", FILE_APPEND);

    while (true)
    {
        log_event_lock.lock();
        while (log_events.size() > 0)
        {
            log_event_struct buffer = log_events.front();

            JsonDocument event;

            event["type"] = buffer.type;
            event[buffer.sensor] = buffer.status;

            serializeJson(event, logfile);
            serializeJson(event, Serial);

            logfile.print("\r\n");
            Serial.println();
            

            log_events.pop();

        }
        log_event_lock.unlock();


        JsonDocument doc;

        doc["type"] = "stats";
        doc["time"] = current_time;
        doc["millis"] = esp_timer_get_time();

        if(gps_enabled)
        {
            JsonArray gps_data = doc["gps"].to<JsonArray>();

            gps_data.add(gps.location.lng());
            gps_data.add(gps.location.lat());
        }

        if(adxl345_enabled)
        {
            JsonArray adxl_data = doc["adxl345"].to<JsonArray>();

            adxl_lock.lock();
            adxl_data.add(event.acceleration.x);
            adxl_data.add(event.acceleration.y);
            adxl_data.add(event.acceleration.z);
            adxl_lock.unlock();
        }
        serializeJson(doc, logfile);
        serializeJson(doc, Serial);
        Serial.println();


        logfile.print("\r\n");

        logfile.flush();

        /*
        Serial.print("Took ");
        Serial.print((millis() - millis_now));
        Serial.println(" ms");
        */
        delay(500);
    }
}

void setup()
{
    Serial.begin(9600);
    gpsSerial.begin(9600, SERIAL_8N1, 33, 25);

    Wire.begin();
    int i2c = i2c_valid(0x27);
    if (!i2c)
    {
        Serial.println("i2c device at 0x27 found");
        display_enabled = true;
    } else {
        Serial.println("No i2c device found");
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
//        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        xTaskCreate(time, "time", 10000, NULL, 1, NULL);
    }

    if (display_enabled)
    {
        renderer.begin();
        menu.setScreen(Dashboard);

        lcd.init();
        lcd.backlight();

        printl("FancyBot", 0, 0);
        printl("Version 0.0.1", 0, 1);

//        xTaskCreate(lcd_menu, "lcd_menu", 10000, NULL, 1, NULL);
        xTaskCreate(lcd_menu_poll, "lcd_menu_poll", 10000, NULL, 1, NULL);
    }



    if (accel.begin())
    {
        adxl345_enabled = true;
        xTaskCreate(adxl345, "adxl345", 10000, NULL, 1, NULL);
    }

    xTaskCreate(gps_task, "gps_task", 10000, NULL, 1, NULL);

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

    if(!SD.begin(5)){
        Serial.println("Cannot mount SD Card!");
    } else {
        Serial.println("SD Card mounted.");
        logfile = SD.open("/logs.json", FILE_APPEND);
        if (SD.exists("/logs.json"))
        {
            logging_enabled = true;
            Serial.print(FANCYBOT);

            JsonDocument doc;

            doc["type"] = "boot";
            doc["gps"] = gps_enabled;
            doc["adxl345"] = adxl345_enabled;
            doc["rtc"] = rtc_enabled;
            doc["display"] = display_enabled;

            serializeJson(doc, logfile);
            serializeJson(doc, Serial);

            logfile.print("\r\n");
            Serial.println();

            logfile.close();

//            log_event("hardware", "logging", 1);

            xTaskCreate(json_logger, "json_logger", 10000, NULL, 1, NULL);
        }
    }

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
    encoderA.observe();
}
