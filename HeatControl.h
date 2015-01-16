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
    None     = 0,
    Default  = (1<<0),
    Settings = (1<<1)
  } SystemMode;
};

#endif /* HeatControl_h */
