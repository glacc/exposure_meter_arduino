#pragma once

#include <Arduino.h>

namespace CameraSettingsLUT
{
  extern PROGMEM const float isoList[];
  extern PROGMEM const int isoCount;
  extern PROGMEM const int isoDefaultPos;

  extern PROGMEM const float fStopsList[];
  extern PROGMEM const int fStopCount;
  extern PROGMEM const int fStopDefaultPos;

  extern PROGMEM const float shutterSpeedSecs[];
  extern PROGMEM const float shutterSpeedDenominators[];
  extern PROGMEM const int shutterSpeedCountAbove1;
  extern PROGMEM const int shutterSpeedCountSub1;
  extern PROGMEM const int shutterSpeedDefaultPos;
  extern float GetShutterSpeed(int n);
}
