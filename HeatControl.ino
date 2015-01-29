/*
 * HeatControl
 * 14.01.2015
 * (C) 2015 by Thorsten Schröpel
 *
 */

#define VERSION "v0.1"

#include <Time.h>
#include <SPI.h>
#include <Wire.h>
#include <DS1307RTC.h>
#include <SoftwareSerial.h>
#include <EEPROMex.h>
#include <EEPROMVar.h>
#include "Adafruit_MCP9808.h"
#include <ClickEncoder.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Menu.h>
#include <TimerOne.h>
#include <avr/wdt.h>
#include <stdio.h>
#include "HeatControl.h"

/*
 * Define the standard settings and pins
 *
 */

const byte sensorAddress = 0x90;           // I2C address of the tmp102 sensor
const unsigned long tempInterval = 15000;  // read temperature only every x ms
const unsigned long tempExtInterval = 300000; //  mark connection as lost after 5 minutes
const int pwmPin = 9;                      // Pin for PWM output to heating
const int xbeeRXPin = 10;                  // RX Pin for the xbee
const int xbeeTXPin = 11;                  // TX Pin for the xbee
const int encoderAPin = A1;                // Pin A from rotary encoder
const int encoderBPin = A0;                // Pin B from rotary encoder
const int encoderButtonPin = A2;           // Button Pin from rotary encoder
const int oledResetPin = 4;                // OLED Reset Pin
const uint8_t menuItemsVisible = 4;        // how many items should/could be visible at a time
const uint8_t menuItemHeight = 7;          // height of one menu item

/*
 * Definition of the variables
 *
 */

SoftwareSerial xbee(xbeeRXPin, xbeeTXPin); // Pins for the serial xbee connection
int cmdRecvd = 0;            // is received Command complete?
String cmd = "";             // received Command
float temperature = 0.0;     // Buffer for the last read temperature
float extTemp = 0.0;         // Buffer for the external temperature sensor
unsigned long tempLast;      // last time the temp was read
unsigned long tempLastExt;   // last time the external sensor was seen
sDOWSettings DOWSettings;    // settings for the current day of the week
uint8_t systemState = State::Default; // current state of the menu
uint8_t previousSystemState = State::None; // previous state of the menu
int16_t encMovement;
int16_t encAbsolute;
bool updateMenu = false;

/*
 * Initialize the objects
 *
 */

Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();
ClickEncoder *encoder;
Menu::Engine *engine;
Adafruit_SSD1306 display(oledResetPin);

/*
 * Timer Definition
 *
 */

void timerIsr() {
  encoder->service();
}

/*
 * Menu Section
 *
 */

bool menuExit(const Menu::Action_t a) {
  systemState = State::Default;
  return true;
}

bool menuDummy(const Menu::Action_t a) {
  return true;
}

bool menuBack(const Menu::Action_t a) {
  if (a == Menu::actionDisplay) {
    engine->navigate(engine->getParent(engine->getParent()));
  }
  return true;
}

// renders the given menuitem at the position pos
void renderMenuItem(const Menu::Item_t *mi, uint8_t pos) {
  uint8_t y = pos * menuItemHeight + 2;
  
  // draw the item
  display.setCursor(1, y);
  if (engine->currentItem == mi) {  
    display.setTextColor(BLACK, WHITE); // 'inverted' text
  } else {
    display.setTextColor(WHITE, BLACK); // 'inverted' text
  }
  /*
  // mark items that have children
  if (engine->getChild(mi) != &menu::NullItem) {
    display.print("> ");
  }
  */
  display.println(engine->getLabel(mi));
}

// render the default screen with setpoint and actual temperature
void renderDefaultScreen() {
  char* tmp;
  
  displayTemp();
  display.setTextSize(1);
  display.setCursor(1, 1);
  sprintf(tmp, "%s, %2d. %s %4d", weekday(), day(), monthShortStr(month()), year());
  display.println(tmp);
  display.setCursor(1, 9);
  sprintf(tmp, "%-2d:%02d", hour(), minute());
  display.println(tmp);
}

/*
 * Helper-Functions
 *
 */

// display the splash screen with some infos
void displaySplashScreen() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(F("HeatControl "));
  display.println(VERSION);
  display.println(F("(C)2015 T. Schröpel"));
  display.display();
}

// draw the temperature temp at x, y
void drawTemp(int x, int y, int textSize, float temp) {
  display.setCursor(x, y);
  display.setTextSize(textSize);
  display.print(String(temp, 1));
  display.println(F("°C"));
}

// display the various temperatures for the default screen
void displayTemp() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  drawTemp(52, 1, 2, getActiveDOWSetting() ? DOWSettings.set1Temp : DOWSettings.set2Temp);
  drawTemp(94, 22, 1, temperature);
  if (connectionLost) {
    display.setCursor(94, 22);
    display.print("-.--°C");
  } else {
    drawTemp(94, 30, 1, extTemp);
  }
}

// get temperature from sensor and store it in global variable temperature
void getTemp() {
  int temp = 0;
  
  if (millis()-tempLast > tempInterval) { // only get temperature after tempInterval ms
    temperature = tempsensor.readTempC();
    tempLast = millis();
  }
}

void getExtTemp(String temp) {
  extTemp = (float)strtod(temp.c_str(), NULL);
  tempLastExt = millis();
}

boolean connectionLost() {
  if (millis()-tempLastExt > tempExtInterval) {
    return true;
  } else {
    return false;
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

int getActiveDOWSetting() {
  return 1; // TODO
}

/*
 * Arduino-Standard Section
 *
 */

// setup
void setup() {
  wdt_enable(WDTO_8S);
  pinMode(pwmPin, OUTPUT);
  analogWrite(pwmPin, 0);
  
  Serial.begin(57600); // for debugging
  xbee.begin(9600); // Uplink to server over xbee
  cmd.reserve(30); // reserve 30 bytes for the command string
  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet) {
    Serial.println(F("Unable to sync with the RTC"));
  }
  if (!tempsensor.begin()) {
    Serial.println(F("Couldn't find MCP9808!"));
    display.setCursor(1, 1);
    display.println(F("Problem mit Temperatursensor!"));
    while (1);
  }
  EEPROM.setMemPool(0, EEPROMSizeATmega328);
  encoder = new ClickEncoder(encoderAPin, encoderBPin, encoderButtonPin);
  encoder->setAccelerationEnabled(false);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display.setTextWrap(false);
//  display.display();
  displaySplashScreen();
  delay(2000);

  engine = new Menu::Engine(&Menu::NullItem);
  menuExit(Menu::actionDisplay); // reset to inital state
  
  xbee.println(F("HC connect")); // send status info
}

// loop
void loop() {
  wdt_reset();    // reset the watchdog timeout
  getTemp();      // get the current temperature

  // handle the xbee communication
  if (xbee.available()) {
    char inChar = (char)xbee.read();
    if (inChar == '\n') {
      cmdRecvd = 1;
    } else {
      cmd += inChar;
    }
  }

  // execute the command buffer
  if (cmdRecvd == 1) {
    switch (cmd.charAt(1)) {
      case 'T': // set time according to next 10 ascii digits
                setRTC(cmd.substring(2, cmd.length()));
                break;
      case 'E': // set external temp from sensor
                getExtTemp(cmd.substring(2, cmd.length()));
                break;
    }
  }
  
  // handle encoder
  encMovement = encoder->getValue();
  if (encMovement) {
    encAbsolute += encMovement;
    
    if (systemState == State::Settings) {
      engine->navigate((encMovement > 0) ? engine->getNext() : engine->getPrev());
      updateMenu = true;
    }
  }
  
  // handle button
  switch (encoder->getButton()) {
    case ClickEncoder::Clicked:
      if (systemState == State::Settings) {
        engine->invoke();
        updateMenu = true;
      }
      break;
  }
  if (updateMenu) {
    updateMenu = false;
    
    if (!encMovement) { // clear menu on child/parent navigation
      // clear display
    }
    
    // simple scrollbar
    Menu::Info_t mi = engine->getItemInfo(engine->currentItem);
    uint8_t sbTop = 0, sbWidth = 4, sbLeft = 100;
    uint8_t sbItems = minValue(menuItemsVisible, mi.siblings);
    uint8_t sbHeight = sbItems * menuItemHeight;
    uint8_t sbMarkHeight = sbHeight * sbItems / mi.siblings;
    uint8_t sbMarkTop = ((sbHeight - sbMarkHeight) / mi.siblings) * (mi.position - 1);
    // tft.fillRect(sbLeft, sbTop, sbWidth, sbHeight, WHITE);
    // tft.fillRect(sbLeft, sbMarkTop, sbWidth, sbMarkHeight, RED);
    
    // render the menu
    engine->render(renderMenuItem, menuItemsVisible);
    
    // temperature screen
    if (systemState == State::Default) {
      if (systemState != previousSystemState) {
        previousSystemState = systemState;
        // render temp screen
      }
    }
  }
}
