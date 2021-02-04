#include <Arduino.h>
#include <TM1637Display.h>
#include <Wire.h>
#include <RTClib.h>
#include <PinChangeInterrupt.h>
#include <FastLED.h>

//Setup Libraries
CRGB leds[1];
RTC_DS1307 rtc;
TM1637Display LCD(12, 11);

//Variables
int timeH;
int timeM;
int alarmH = 16;
int alarmM = 17;
int LEDState = 0;
int spin = 0;
int LCDBrightness = 7;

//Timers
//internal timer when setting alarm
int setupAlarmTimer = 0;
int alarmIntTimer = 0;
int mainTimer = 0;

//Statuses
boolean alarmSet = false;
boolean alarmON = false;

void setup()
{
  LCD.setBrightness(7);
  FastLED.addLeds<WS2812B, 8, GRB>(leds, 1); // GRB ordering is typical

  Serial.begin(9600);
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);

  pinMode(LED_BUILTIN, OUTPUT);
  // int startup = 0;
  // while (startup < 2)
  // {
  //   for (int i = 0; i < 256; i++)
  //   {
  //     leds[0] = CHSV(i, 255, 255);
  //     FastLED.show();
  //     delay(10);
  //   }
  //   for (int i = 255; i > 0; i--)
  //   {
  //     leds[0] = CHSV(i, 255, 255);
  //     FastLED.show();
  //     delay(10);
  //   }
  //   startup++;
  // }
}

void updateTime()
{
  DateTime now = rtc.now();
  if (now.minute() != timeM)
  {
    timeM = now.minute();
    LCD.showNumberDecEx(timeM, (0x80 >> 1), true, 2, 2);
  }
  if (now.hour() != timeH)
  {
    timeH = now.hour();
    LCD.showNumberDecEx(timeH, (0x80 >> 1), false, 2, 0);
  }
  if (timeH >= 18 || timeH <= 6)
  {
    LCDBrightness = 2;
  }
  else
  {
    LCDBrightness = 7;
  }
}

void fireAlarm()
{
  if (timeH == alarmH && timeM == alarmM)
  {
    alarmON = true;
  }
  if (alarmIntTimer > 100 && alarmON)
  {
    LEDState = !LEDState;
    digitalWrite(LED_BUILTIN, LEDState);
    alarmIntTimer = 0;
  }
  alarmIntTimer++;
}

void alarmLED(int br = 255)
{
  if (alarmSet)
  {
    leds[0] = CHSV(0, 255, br);
    FastLED.show();
  } else
  {
    leds[0] = CRGB::Black;
    FastLED.show();
  }
}

void loop()
{
  updateTime();
  fireAlarm();
  if (digitalRead(2) == LOW && mainTimer > 150)
  {
    LCD.setBrightness(7);
    LCD.showNumberDec(alarmH, false, 2, 0);
    LCD.showNumberDec(alarmM, true, 2, 2);
    while (setupAlarmTimer < 2500)
    {
      if (digitalRead(3) == LOW && setupAlarmTimer > (spin > 4 ? 20 : 80))
      {
        spin++;
        setupAlarmTimer = 0;
        alarmH++;
        if (alarmH > 23)
        {
          alarmH = 0;
        }
        LCD.showNumberDec(alarmH, false, 2, 0);
      }

      if (digitalRead(4) == LOW && setupAlarmTimer > (spin > 4 ? 20 : 80))
      {
        spin++;
        setupAlarmTimer = 0;
        alarmM++;
        if (alarmM > 59)
        {
          alarmM = 0;
        }
        LCD.showNumberDec(alarmM, true, 2, 2);
      }
      digitalRead(3) == HIGH &&digitalRead(4) == HIGH ? spin = 0 : spin = spin;

      if (digitalRead(5) == LOW && setupAlarmTimer > 200)
      {
        setupAlarmTimer = 0;
        alarmSet = !alarmSet;
      }
      if (digitalRead(2) == LOW && setupAlarmTimer > 200)
      {
        break;
      }
      setupAlarmTimer++;
      alarmLED(255);
    }

    //bring back the clock after setup
    LCD.setBrightness(LCDBrightness);
    LCD.showNumberDecEx(timeM, (0x80 >> 1), true, 2, 2);
    LCD.showNumberDecEx(timeH, (0x80 >> 1), false, 2, 0);

    mainTimer = 0;
    setupAlarmTimer = 0;
  }
  alarmLED(100);
  if (mainTimer < 200)
  {
    mainTimer++;
  }
}