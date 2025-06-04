#include "Arduino.h"

#include "AiEsp32RotaryEncoder.h"
#include <LiquidCrystal_I2C.h>
//#include <TinyGPSPlus.h>
#include <RTClib.h>

#define ROTARY_ENCODER_DT_PIN 27
#define ROTARY_ENCODER_CLK_PIN 26
#define ROTARY_ENCODER_BUTTON_PIN 14
#define ROTARY_ENCODER_STEPS 4

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_DT_PIN, ROTARY_ENCODER_CLK_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS);
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

void IRAM_ATTR readEncoderISR()
{
	rotaryEncoder.readEncoder_ISR();
}

void printl(char* string, int x, int y)
{
    lcd.setCursor(x, y);
    lcd.print(string);
}


void rotary_onButtonClick()
{
	static unsigned long lastTimePressed = 0;
	//ignore multiple press in that time milliseconds
	if (millis() - lastTimePressed < 500)
	{
		return;
	}
	lastTimePressed = millis();
	Serial.print("button pressed ");
	Serial.print(millis());
	Serial.println(" milliseconds after restart");
}

void setup()
{
    Serial.begin(9600);
    rtc.begin();

    rotaryEncoder.begin();
	rotaryEncoder.setup(readEncoderISR);
    rotaryEncoder.setBoundaries(0, 20, true);
    rotaryEncoder.disableAcceleration();

    lcd.init();
    lcd.backlight();

    printl("FancyBot", 0, 0);
    printl("Version 0.0.1", 0, 1);
}

void loop()
{
	//dont print anything unless value changed
	if (rotaryEncoder.encoderChanged())
	{
		Serial.print("Value: ");
		Serial.println(rotaryEncoder.readEncoder());
	}
	if (rotaryEncoder.isEncoderButtonClicked())
	{
		rotary_onButtonClick();
	}
}