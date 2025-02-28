#include "CameraSettingsLUT.hpp"

PROGMEM const float CameraSettingsLUT::isoList[61] =
{
  0.1f,     0.125f,   0.16f,
  0.2f,     0.25f,    0.32f,
  0.4f,     0.5f,     0.64f,
  0.8f,     1.0f,     1.25f,
  1.6f,     2.0f,     2.5f,
  3.2f,     4.0f,     5.0f,
  6.4f,     8.0f,     10.0f,
  12.5f,    16.0f,    20.0f,
  25.0f,    32.0f,    40.0f,
  50.0f,    64.0f,    80.0f,
  100.0f,   125.0f,   160.0f,
  200.0f,   250.0f,   320.0f,
  400.0f,   500.0f,   640.0f,
  800.0f,   1000.0f,  1250.0f,
  1600.0f,  2000.0f,  2500.0f,
  3200.0f,  4000.0f,  5000.0f,
  6400.0f,  8000.0f,  10000.0f,
  12800.0f, 16000.0f, 20000.0f,
  25600.0f, 32000.0f, 40000.0f,
  51200.0f, 64000.0f, 80000.0f,
  102400.0f
};
PROGMEM const int CameraSettingsLUT::isoCount = 61;
PROGMEM const int CameraSettingsLUT::isoDefaultPos = 30;

// Reference: https://scantips.com/lights/exposurecalc.html
PROGMEM const float CameraSettingsLUT::fStopsList[63] = 
{
  0.50f,  0.56f,  0.63f,
  0.71f,  0.79f,  0.89f,
  0.95f,
  1.0f,   1.1f,   1.2f,
  1.3f,
  1.4f,   1.6f,   1.8f,
  2.0f,   2.2f,   2.5f,
  2.8f,   3.2f,   3.5f,
  4.0f,   4.5f,   5.0f,
  5.6f,   6.3f,   7.1f,
  8.0f,   9.0f,   10.0f,
  11.0f,  13.0f,  14.0f,
  16.0f,  18.0f,  20.0f,
  22.0f,  25.0f,  29.0f,
  32.0f,  35.0f,  39.0f,
  45.0f,  50.0f,  56.0f,
  64.0f,  72.0f,  80.0f,
  90.0f,  100.0f, 114.0f,
  128.0f, 145.0f, 160.0f,
  180.0f, 200.0f, 230.0f,
  256.0f, 290.0f, 320.0f,
  360.0f, 400.0f, 460.0f,
  512.0f
};
PROGMEM const int CameraSettingsLUT::fStopCount = 63;
PROGMEM const int CameraSettingsLUT::fStopDefaultPos = 23;

PROGMEM const float CameraSettingsLUT::shutterSpeedSecs[16] = 
{
  1.0f,  1.3f,  1.6f,
  2.0f,  2.5f,  3.0f,
  4.0f,  5.0f,  6.0f,
  8.0f,  10.0f, 13.0f,
  15.0f, 20.0f, 25.0f,
  30.0f
};
PROGMEM const float CameraSettingsLUT::shutterSpeedDenominators[39]
{
  1.3f,    1.6f,
  2.0f,    2.5f,    3.0f,
  4.0f,    5.0f,    6.0f,
  8.0f,    10.0f,   13.0f,
  15.0f,   20.0f,   25.0f,
  30.0f,   40.0f,   50.0f,
  60.0f,   80.0f,   100.0f,
  125.0f,  160.0f,  200.0f,
  250.0f,  320.0f,  400.0f,
  500.0f,  640.0f,  800.0f,
  1000.0f, 1250.0f, 1600.0f,
  2000.0f, 2500.0f, 3200.0f,
  4000.0f, 5000.0f, 6400.0f,
  8000.0f
};
PROGMEM const int CameraSettingsLUT::shutterSpeedCountAbove1 = 16;
PROGMEM const int CameraSettingsLUT::shutterSpeedCountSub1 = 39;
PROGMEM const int CameraSettingsLUT::shutterSpeedDefaultPos = 20;
float CameraSettingsLUT::GetShutterSpeed(int n)
{
  if (n < 0)
  {
    if (n < -shutterSpeedCountAbove1)
      n = -shutterSpeedCountAbove1;
    
    return pgm_read_float(shutterSpeedSecs + (-n - 1));
  }

  if (n > (shutterSpeedCountSub1 - 1))
    n = shutterSpeedCountSub1 - 1;

  return (1.0f / pgm_read_float(shutterSpeedDenominators + n));
}