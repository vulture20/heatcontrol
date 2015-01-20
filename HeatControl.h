#ifndef HeatControl_h
#define HeatControl_h

#define minValue(a, b) ((a < b) ? a : b)

struct sDOWSettings {
  byte set1BeginHour;
  byte set1BeginMinute;
  byte set1EndHour;
  byte set1EndMinute;
  float set1Temp;
  byte set2BeginHour;
  byte set2BeginMinute;
  byte set2EndHour;
  byte set2EndMinute;
  float set2Temp;
};

namespace State {
  typedef enum SysteMode_e {
    None        = 0,
    Default     = (1<<0),
    Settings    = (1<<1),
    HolidayMode = (1<<2)
  } SystemMode;
};

bool menuExit(const Menu::Action_t a);
bool menuDummy(const Menu::Action_t a);

// Name, Label, Next, Previous, Parent, Child, Callback
MenuItem(miExit, "", Menu::NullItem, Menu::NullItem, Menu::NullItem, miSettings, menuExit);
  MenuItem(miSettings, "Einstellungen", miHolidayMode, Menu::NullItem, miExit, miSetDateTime, menuDummy);
    MenuItem(miSetDateTime, "Datum/Uhrzeit einstellen", miStandbyTemp, Menu::NullItem, miSettings, miSetDate, menuDummy);
      MenuItem(miSetDate, "Datum einstellen", miSetTime,      Menu::NullItem, miSetDateTime, Menu::NullItem, menuDummy);
      MenuItem(miSetTime, "Zeit einstellen",  Menu::NullItem, miSetDate,      miSetDateTime, Menu::NullItem, menuDummy);
    MenuItem(miStandbyTemp, "Standby-Temperatur", miDOWPattern, miSetDateTime, miSettings, Menu::NullItem, menuDummy);
    MenuItem(miDOWPattern, "Schaltpunkte definieren", miSetHysterese, miStandbyTemp, miSettings, miMonday, menuDummy);
      MenuItem(miMonday,    "Montag",     miTuesday,      Menu::NullItem, miDOWPattern, miMo1, menuDummy);
        MenuItem(miMo1, "Schaltzeit 1", miMo2,          Menu::NullItem, miMonday, miMo1Begin, menuDummy);
          MenuItem(miMo1Begin, "Anfang",     miMo1End,       Menu::NullItem, miMo1, Menu::NullItem, menuDummy);
          MenuItem(miMo1End,   "Ende",       miMo1Temp,      miMo1Begin,     miMo1, Menu::NullItem, menuDummy);
          MenuItem(miMo1Temp,  "Temperatur", Menu::NullItem, miMo1End,       miMo1, Menu::NullItem, menuDummy);
        MenuItem(miMo2, "Schaltzeit 2", Menu::NullItem, miMo1,          miMonday, miMo2Begin, menuDummy);
          MenuItem(miMo2Begin, "Anfang",     miMo2End,       Menu::NullItem, miMo2, Menu::NullItem, menuDummy);
          MenuItem(miMo2End,   "Ende",       miMo2Temp,      miMo2Begin,     miMo2, Menu::NullItem, menuDummy);
          MenuItem(miMo2Temp,  "Temperatur", Menu::NullItem, miMo2End,       miMo2, Menu::NullItem, menuDummy);
      MenuItem(miTuesday,   "Dienstag",   miWednesday,    miMonday,       miDOWPattern, miTu1, menuDummy);
        MenuItem(miTu1, "Schaltzeit 1", miTu2,          Menu::NullItem, miTuesday, miTu1Begin, menuDummy);
          MenuItem(miTu1Begin, "Anfang",     miTu1End,       Menu::NullItem, miTu1, Menu::NullItem, menuDummy);
          MenuItem(miTu1End,   "Ende",       miTu1Temp,      miTu1Begin,     miTu1, Menu::NullItem, menuDummy);
          MenuItem(miTu1Temp,  "Temperatur", Menu::NullItem, miTu1End,       miTu1, Menu::NullItem, menuDummy);
        MenuItem(miTu2, "Schaltzeit 2", Menu::NullItem, miTu1,          miTuesday, miTu2Begin, menuDummy);
          MenuItem(miTu2Begin, "Anfang",     miTu2End,       Menu::NullItem, miTu2, Menu::NullItem, menuDummy);
          MenuItem(miTu2End,   "Ende",       miTu2Temp,      miTu2Begin,     miTu2, Menu::NullItem, menuDummy);
          MenuItem(miTu2Temp,  "Temperatur", Menu::NullItem, miTu2End,       miTu2, Menu::NullItem, menuDummy);
      MenuItem(miWednesday, "Mittwoch",   miThursday,     miTuesday,      miDOWPattern, miWe1, menuDummy);
        MenuItem(miWe1, "Schaltzeit 1", miWe2,          Menu::NullItem, miWednesday, miWe1Begin, menuDummy);
          MenuItem(miWe1Begin, "Anfang",     miWe1End,       Menu::NullItem, miWe1, Menu::NullItem, menuDummy);
          MenuItem(miWe1End,   "Ende",       miWe1Temp,      miWe1Begin,     miWe1, Menu::NullItem, menuDummy);
          MenuItem(miWe1Temp,  "Temperatur", Menu::NullItem, miWe1End,       miWe1, Menu::NullItem, menuDummy);
        MenuItem(miWe2, "Schaltzeit 2", Menu::NullItem, miWe1,          miWednesday, miWe2Begin, menuDummy);
          MenuItem(miWe2Begin, "Anfang",     miWe2End,       Menu::NullItem, miWe2, Menu::NullItem, menuDummy);
          MenuItem(miWe2End,   "Ende",       miWe2Temp,      miWe2Begin,     miWe2, Menu::NullItem, menuDummy);
          MenuItem(miWe2Temp,  "Temperatur", Menu::NullItem, miWe2End,       miWe2, Menu::NullItem, menuDummy);
      MenuItem(miThursday,  "Donnerstag", miFriday,       miWednesday,    miDOWPattern, miTh1, menuDummy);
        MenuItem(miTh1, "Schaltzeit 1", miTh2,          Menu::NullItem, miThursday, miTh1Begin, menuDummy);
          MenuItem(miTh1Begin, "Anfang",     miTh1End,       Menu::NullItem, miTh1, Menu::NullItem, menuDummy);
          MenuItem(miTh1End,   "Ende",       miTh1Temp,      miTh1Begin,     miTh1, Menu::NullItem, menuDummy);
          MenuItem(miTh1Temp,  "Temperatur", Menu::NullItem, miTh1End,       miTh1, Menu::NullItem, menuDummy);
        MenuItem(miTh2, "Schaltzeit 2", Menu::NullItem, miTh1,          miThursday, miTh2Begin, menuDummy);
          MenuItem(miTh2Begin, "Anfang",     miTh2End,       Menu::NullItem, miTh2, Menu::NullItem, menuDummy);
          MenuItem(miTh2End,   "Ende",       miTh2Temp,      miTh2Begin,     miTh2, Menu::NullItem, menuDummy);
          MenuItem(miTh2Temp,  "Temperatur", Menu::NullItem, miTh2End,       miTh2, Menu::NullItem, menuDummy);
      MenuItem(miFriday,    "Freitag",    miSaturday,     miThursday,     miDOWPattern, miFr1, menuDummy);
        MenuItem(miFr1, "Schaltzeit 1", miFr2,          Menu::NullItem, miFriday, miFr1Begin, menuDummy);
          MenuItem(miFr1Begin, "Anfang",     miFr1End,       Menu::NullItem, miFr1, Menu::NullItem, menuDummy);
          MenuItem(miFr1End,   "Ende",       miFr1Temp,      miFr1Begin,     miFr1, Menu::NullItem, menuDummy);
          MenuItem(miFr1Temp,  "Temperatur", Menu::NullItem, miFr1End,       miFr1, Menu::NullItem, menuDummy);
        MenuItem(miFr2, "Schaltzeit 2", Menu::NullItem, miFr1,          miFriday, miFr2Begin, menuDummy);
          MenuItem(miFr2Begin, "Anfang",     miFr2End,       Menu::NullItem, miFr2, Menu::NullItem, menuDummy);
          MenuItem(miFr2End,   "Ende",       miFr2Temp,      miFr2Begin,     miFr2, Menu::NullItem, menuDummy);
          MenuItem(miFr2Temp,  "Temperatur", Menu::NullItem, miFr2End,       miFr2, Menu::NullItem, menuDummy);
      MenuItem(miSaturday,  "Samstag",    miSunday,       miFriday,       miDOWPattern, miSa1, menuDummy);
        MenuItem(miSa1, "Schaltzeit 1", miSa2,          Menu::NullItem, miSaturday, miSa1Begin, menuDummy);
          MenuItem(miSa1Begin, "Anfang",     miSa1End,       Menu::NullItem, miSa1, Menu::NullItem, menuDummy);
          MenuItem(miSa1End,   "Ende",       miSa1Temp,      miSa1Begin,     miSa1, Menu::NullItem, menuDummy);
          MenuItem(miSa1Temp,  "Temperatur", Menu::NullItem, miSa1End,       miSa1, Menu::NullItem, menuDummy);
        MenuItem(miSa2, "Schaltzeit 2", Menu::NullItem, miSa1,          miSaturday, miSa2Begin, menuDummy);
          MenuItem(miSa2Begin, "Anfang",     miSa2End,       Menu::NullItem, miSa2, Menu::NullItem, menuDummy);
          MenuItem(miSa2End,   "Ende",       miSa2Temp,      miSa2Begin,     miSa2, Menu::NullItem, menuDummy);
          MenuItem(miSa2Temp,  "Temperatur", Menu::NullItem, miSa2End,       miSa2, Menu::NullItem, menuDummy);
      MenuItem(miSunday,    "Sonntag",    Menu::NullItem, miSaturday,     miDOWPattern, miSu1, menuDummy);
        MenuItem(miSu1, "Schaltzeit 1", miSu2,          Menu::NullItem, miSunday, miSu1Begin, menuDummy);
          MenuItem(miSu1Begin, "Anfang",     miSu1End,       Menu::NullItem, miSu1, Menu::NullItem, menuDummy);
          MenuItem(miSu1End,   "Ende",       miSu1Temp,      miSu1Begin,     miSu1, Menu::NullItem, menuDummy);
          MenuItem(miSu1Temp,  "Temperatur", Menu::NullItem, miSu1End,       miSu1, Menu::NullItem, menuDummy);
        MenuItem(miSu2, "Schaltzeit 2", Menu::NullItem, miSu1,          miSunday, miSu2Begin, menuDummy);
          MenuItem(miSu2Begin, "Anfang",     miSu2End,       Menu::NullItem, miSu2, Menu::NullItem, menuDummy);
          MenuItem(miSu2End,   "Ende",       miSu2Temp,      miSu2Begin,     miSu2, Menu::NullItem, menuDummy);
          MenuItem(miSu2Temp,  "Temperatur", Menu::NullItem, miSu2End,       miSu2, Menu::NullItem, menuDummy);
    MenuItem(miSetHysterese, "Hysterese einstellen", Menu::NullItem, miDOWPattern, miSettings, miHystTop, menuDummy);
      MenuItem(miHystTop,    "Obere Grenze",  miHystBottom,   Menu::NullItem, miSetHysterese, Menu::NullItem, menuDummy);
      MenuItem(miHystBottom, "Untere Grenze", Menu::NullItem, miHystTop,      miSetHysterese, Menu::NullItem, menuDummy);
  MenuItem(miHolidayMode, "Urlaubsmodus", miDiagnose, miSettings, miExit, miHolidayActivate, menuDummy);
    MenuItem(miHolidayActivate, "Urlaubsmodus aktivieren", miHolidayEnddate, Menu::NullItem,    miHolidayMode, Menu::NullItem, menuDummy);
    MenuItem(miHolidayEnddate,  "Enddatum einstellen",     Menu::NullItem,   miHolidayActivate, miHolidayMode, Menu::NullItem, menuDummy);
  MenuItem(miDiagnose, "Diagnose", miUber, miHolidayMode, miSettings, miGetDateTime, menuDummy);
    MenuItem(miGetDateTime, "Datum/Uhrzeit anfordern", miShowTemp, Menu::NullItem, miDiagnose, Menu::NullItem, menuDummy);
    MenuItem(miShowTemp, "Temperatur anzeigen", miRFTest, miGetDateTime, miDiagnose, miTempInt, menuDummy);
      MenuItem(miTempInt, "Interner Sensor", miTempExt,      Menu::NullItem, miShowTemp, Menu::NullItem, menuDummy);
      MenuItem(miTempExt, "Externer Sensor", Menu::NullItem, miTempInt,      miShowTemp, Menu::NullItem, menuDummy);
    MenuItem(miRFTest, "Funkanbindung testen", miVoltBatt, miShowTemp, miDiagnose, miRFServer, menuDummy);
      MenuItem(miRFServer, "Server", miRFSensor,     Menu::NullItem, miRFTest, Menu::NullItem, menuDummy);
      MenuItem(miRFSensor, "Sensor", Menu::NullItem, miRFServer,     miRFTest, Menu::NullItem, menuDummy);
    MenuItem(miVoltBatt,  "Restspannung Batterie",  miVoltPWM,      miRFTest,   miDiagnose, Menu::NullItem, menuDummy);
    MenuItem(miVoltPWM,   "Regelspannung anzeigen", miDispTest,     miVoltBatt, miDiagnose, Menu::NullItem, menuDummy);
    MenuItem(miDispTest,  "Displaytest",            miFactReset,    miVoltPWM,  miDiagnose, Menu::NullItem, menuDummy);
    MenuItem(miFactReset, "Werkseinstellungen",     Menu::NullItem, miDispTest, miDiagnose, Menu::NullItem, menuDummy);
  MenuItem(miUber, "Über", Menu::NullItem, miDiagnose, miSettings, Menu::NullItem, menuDummy);

#endif /* HeatControl_h */
