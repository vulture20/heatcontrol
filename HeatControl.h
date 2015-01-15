#ifndef HeatControl_h
#define HeatControl_h

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

#endif /* HeatControl_h */
