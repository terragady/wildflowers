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
static uint8_t cmdbuf[8] = {0};

//Timers
//internal timer when setting alarm
int setupAlarmTimer = 0;
int alarmIntTimer = 0;
int mainTimer = 0;

//Statuses
boolean alarmSet = false;
boolean alarmON = false;

void playerCommand(int8_t cmd, int16_t dat)
{
  delay(20);
  cmdbuf[0] = 0x7e;               // bajt startu
  cmdbuf[1] = 0xFF;               // wersja
  cmdbuf[2] = 0x06;               // liczba bajtow polecenia
  cmdbuf[3] = cmd;                // polecenie
  cmdbuf[4] = 0x00;               // 0x00 = no feedback, 0x01 = feedback
  cmdbuf[5] = (int8_t)(dat >> 8); // parametr DAT1
  cmdbuf[6] = (int8_t)(dat);      //  parametr DAT2
  cmdbuf[7] = 0xef;               // bajt konczacy

  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.write(cmdbuf[i]);
  }
}
void blinkBlue(int times = 1)
{
  for (int i = 0; i < times; i++)
  {
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(100);
    FastLED.setBrightness(255);
    leds[0] = CRGB::Blue;
    FastLED.show();
    delay(100);
    leds[0] = CRGB::Black;
    FastLED.show();
  }
}
void blinkRed(int times = 1)
{
  for (int i = 0; i < times; i++)
  {
    leds[0] = CRGB::Black;
    FastLED.show();
    delay(100);
    FastLED.setBrightness(255);
    leds[0] = CRGB::Red;
    FastLED.show();
    delay(100);
    leds[0] = CRGB::Black;
    FastLED.show();
  }
}
void updateTime()
{
  DateTime now = rtc.now();
  // if (timeH >= 18 || timeH <= 6)
  // {
  LCDBrightness = 2;
  // }
  // else
  // {
  // LCDBrightness = 7;
  // }
  if (now.minute() != timeM)
  {
    timeM = now.minute();
    LCD.setBrightness(LCDBrightness);
    LCD.showNumberDecEx(timeM, (0x80 >> 1), true, 2, 2);
  }
  if (now.hour() != timeH)
  {
    blinkBlue();
    timeH = now.hour();
    LCD.showNumberDecEx(timeH, (0x80 >> 1), false, 2, 0);
  }
}
void alarmLED(int br = 255)
{
  if (alarmSet)
  {
    leds[0] = CHSV(0, 255, br);
    FastLED.show();
  }
  else
  {
    leds[0] = CRGB::Black;
    FastLED.show();
  }
}
void setAlarm()
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

    if (digitalRead(5) == LOW && setupAlarmTimer > 150)
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
void setTime()
{
  LCD.setBrightness(7);
  LCD.showNumberDec(timeH, false, 2, 0);
  LCD.showNumberDec(timeM, true, 2, 2);
  while (setupAlarmTimer < 2500)
  {
    leds[0] = CHSV(120, 255, 255);
    FastLED.show();
    if (digitalRead(3) == LOW && setupAlarmTimer > (spin > 4 ? 20 : 80))
    {
      leds[0] = CRGB::Black;
      FastLED.show();
      spin++;
      setupAlarmTimer = 0;
      timeH++;
      if (timeH > 23)
      {
        timeH = 0;
      }
      LCD.showNumberDec(timeH, false, 2, 0);
    }

    if (digitalRead(4) == LOW && setupAlarmTimer > (spin > 4 ? 20 : 80))
    {
      spin++;
      setupAlarmTimer = 0;
      timeM++;
      if (timeM > 59)
      {
        timeM = 0;
      }
      LCD.showNumberDec(timeM, true, 2, 2);
    }
    digitalRead(3) == HIGH &&digitalRead(4) == HIGH ? spin = 0 : spin = spin;

    if (digitalRead(6) == LOW && setupAlarmTimer > 200)
    {
      //bring back the clock after setup
      DateTime now = rtc.now();
      if (now.hour() != timeH || now.minute() != timeM)
      {
        rtc.adjust(DateTime(now.year(), now.month(), now.day(), timeH, timeM, 0));
      }
      break;
    }
    setupAlarmTimer++;
  }

  LCD.setBrightness(LCDBrightness);
  LCD.showNumberDecEx(timeM, (0x80 >> 1), true, 2, 2);
  LCD.showNumberDecEx(timeH, (0x80 >> 1), false, 2, 0);

  mainTimer = 0;
  setupAlarmTimer = 0;
}
void fireAlarm()
{
  if (timeH == alarmH && timeM == alarmM && !alarmON)
  {
    alarmON = true;
  }
  if (alarmON)
  {
    
  }
}

void setup()
{
  LCD.setBrightness(7);
  FastLED.addLeds<WS2812B, 8, GRB>(leds, 1); // GRB ordering is typical
  Serial.begin(9600);
  blinkRed();
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
  pinMode(6, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  delay(500);
  blinkRed();
  
  // inicjalizacja karty SD
  playerCommand(0x09, 0x0002);
  delay(200);
  blinkRed(2);

  for (int i = 0; i < 256; i++)
  {
    leds[0] = CHSV(i, 255, 255);
    FastLED.show();
    delay(5);
  }
  for (int i = 255; i > 0; i--)
  {
    leds[0] = CHSV(i, 255, 255);
    FastLED.show();
    delay(5);
  }
  blinkBlue(3);

  // this is to set a time from a computer
  //  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop()
{
  updateTime();
  fireAlarm();
  if (digitalRead(2) == LOW && mainTimer > 150)
  {
    setAlarm();
  }
  if (digitalRead(6) == LOW && mainTimer > 150)
  {
    setTime();
  }

  alarmLED(100);
  if (mainTimer < 200)
  {
    mainTimer++;
  }
}