#include <Arduino.h>
#include <TM1637Display.h>
#include <Wire.h>
#include <RTClib.h>
#include <PinChangeInterrupt.h>

RTC_DS1307 rtc;
TM1637Display LCD(12, 11);

//Variables
int timeH;
int timeM;
int alarmH = 16;
int alarmM = 17;
int interval = 0;
int alarmIntTimer = 0;
int LEDState = 0;

//Table to store 4 digits, used in time and Alarm setup procedure
int digits[3];
int setupAlarmTimer = 0;

//Statuses
boolean alarm_set = false;
boolean setupAlarm = false;
boolean alarmON = false;
boolean setupTime = false;

void setup()
{
  LCD.setBrightness(7);

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

  pinMode(LED_BUILTIN, OUTPUT);
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
}

void fireAlarm(){
  if (timeH == alarmH && timeM == alarmM){
    alarmON = true;
  }
    if(alarmIntTimer > 100 && alarmON){
    LEDState = !LEDState;
    digitalWrite(LED_BUILTIN, LEDState);
        alarmIntTimer = 0;
    }
    alarmIntTimer++;
}

void loop()
{
  updateTime();
  fireAlarm();
  if (digitalRead(2) == LOW && interval > 150)
  {
    while (setupAlarmTimer < 250)
    {
      LCD.showNumberDec(alarmH, false, 2, 0);
      LCD.showNumberDec(alarmM, true, 2, 2);

      if (digitalRead(3) == LOW && setupAlarmTimer > 5)
      {
        setupAlarmTimer = 0;
        alarmH++;
        if (alarmH >= 23)
        {
          alarmH = 0;
        }
      }
      if (digitalRead(4) == LOW && setupAlarmTimer > 5)
      {
        setupAlarmTimer = 0;
        alarmM++;
        if (alarmM >= 59)
        {
          alarmM = 0;
        }
      }
      if (digitalRead(2) == LOW && setupAlarmTimer > 10)
      {
        break;
      }
      setupAlarmTimer++;
    }

    //bring back the clock after setup
    LCD.showNumberDecEx(timeM, (0x80 >> 1), true, 2, 2);
    LCD.showNumberDecEx(timeH, (0x80 >> 1), false, 2, 0);
    interval = 0;
    setupAlarmTimer = 0;
  }

  if (interval < 200)
  {
    interval++;
  }
}