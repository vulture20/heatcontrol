/*
 * HeatControl v0.1
 * 14.01.2015
 * (C) 2015 by Thorsten Schröpel
 *
 */

#include <Time.h>
#include <Wire.h>
#include <DS1307RTC.h>
#include <SoftwareSerial.h>
#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <tmp102.h>
#include "HeatControl.h"

const byte sensorAddress = 0x90;           // I2C address of the tmp102 sensor
const unsigned long tempInterval = 15000;  // read temperature only every x ms
const int pwmPin = 9;                      // Pin for PWM output to heating
const int xbeeRXPin = 10;                  // RX Pin for the xbee
const int xbeeTXPin = 11;                  // TX Pin for the xbee

SoftwareSerial xbee(xbeeRXPin, xbeeTXPin); // Pins for the serial xbee connection
int cmdRecvd = 0;            // is received Command complete?
String cmd = "";             // received Command
float temperature = 0.0;     // Buffer for the last read temperature
unsigned long tempLast;      // last time the temp was read
sDOWSettings DOWSettings;

tmp102 *thermometer = new tmp102(&Wire);

void setup() {
  pinMode(pwmPin, OUTPUT);
  analogWrite(pwmPin, 0);
  
  Serial.begin(57600); // for debugging
  xbee.begin(9600); // Uplink to server over xbee
  cmd.reserve(30); // reserve 30 bytes for the command string
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet) {
    Serial.println(F("Unable to sync with the RTC"));
  }
  thermometer->init(sensorAddress);
  thermometer->writeConf(TMP102_DEFAULT_CONF); // set default config
  EEPROM.setMemPool(0, EEPROMSizeATmega328);
  
  xbee.println(F("HC connect")); // send status info
}

void loop() {
  getTemp();

  if (xbee.available()) {
    char inChar = (char)xbee.read();
    if (inChar == '\n') {
      cmdRecvd = 1;
    } else {
      cmd += inChar;
    }
  }

  if (cmdRecvd == 1) {
    switch (cmd.charAt(1)) {
      case 'T': // set time according to next 10 ascii digits
                setRTC(cmd.substring(2, cmd.length()));
                break;
    }
  }
}

// get temperature from sensor and store it in global variable temperature
void getTemp() {
  int temp = 0;
  
  if (millis()-tempLast > tempInterval) { // only get temperature after tempInterval ms
    if (thermometer->readTemp(temp)) { // read temperature from sensor
      temperature = temp * 0.0625; // convert it to °C
    }
  }
}

// set RTC to given time
void setRTC(String timeStr) {
  time_t pctime = 0;
  
  if (timeStr.length() == 10) { // the timestring must have 10 ascii digits
    for (int i=1; i < 10; i++) {
      if (timeStr.charAt(i) >= '0' && timeStr.charAt(i) <= '9') {
        pctime = (10 * pctime) + (timeStr.charAt(i) - '0'); // convert string to time_t
      }
    }
    if (pctime > 0) { // is pctime valid?
      RTC.set(pctime); // if its valid, set time
      setTime(pctime);
    }
  }
}

// is the Holiday mode active?
boolean isHolidayActive() {
  return (EEPROM.readByte(122) == 1);
}

// set the holiday mode
boolean setHolidayMode(boolean mode) {
  return (EEPROM.updateByte(122, mode));
}

// set the pwm output to x percent (mapped to 10 bit)
void setPWM(byte percent) {
  analogWrite(pwmPin, map(percent, 0, 100, 0, 1023));
}

// read settings for the given day of the week from eeprom
void getDOWSettings(byte DOW) {
  DOWSettings.set1BeginHour =   EEPROM.readByte (((DOW - 1) * 16 + 2));
  DOWSettings.set1BeginMinute = EEPROM.readByte (((DOW - 1) * 16 + 2) + 1);
  DOWSettings.set1EndHour =     EEPROM.readByte (((DOW - 1) * 16 + 2) + 2);
  DOWSettings.set1EndMinute =   EEPROM.readByte (((DOW - 1) * 16 + 2) + 3);
  DOWSettings.set1Temp =        EEPROM.readFloat(((DOW - 1) * 16 + 2) + 4);
  DOWSettings.set2BeginHour =   EEPROM.readByte (((DOW - 1) * 16 + 2) + 8);
  DOWSettings.set2BeginMinute = EEPROM.readByte (((DOW - 1) * 16 + 2) + 9);
  DOWSettings.set2EndHour =     EEPROM.readByte (((DOW - 1) * 16 + 2) + 10);
  DOWSettings.set2EndMinute =   EEPROM.readByte (((DOW - 1) * 16 + 2) + 11);
  DOWSettings.set2Temp =        EEPROM.readFloat(((DOW - 1) * 16 + 2) + 12);
}
