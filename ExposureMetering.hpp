#pragma once

#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>

namespace ExposureMetering
{
  extern bool shutterPriority;

  extern float iso;
  extern float shutter;
  extern float fStops;
  extern float evWithOffset;

  extern bool measurementUpdated;

  /* Gain levels for TSL2591 */
  extern Adafruit_TSL2591 lightSensor;

  struct GainLevel
  {
    tsl2591Gain_t gain = TSL2591_GAIN_LOW;
    tsl2591IntegrationTime_t timing = TSL2591_INTEGRATIONTIME_100MS;

    int32_t incThreshold = 2560;
    int32_t decThreshold = 65535;

    GainLevel(tsl2591Gain_t newGain, tsl2591IntegrationTime_t newTiming, int32_t newIncThreshold = 2560, int32_t newDecThreshold = 65535);

    GainLevel() { }
  };
  extern const GainLevel gainLevels[];
  extern const int maxGainLevel;
  extern int gainLevel;

  extern void SetGainLevel(int newGainLevel);

  extern void SetupLightSensor();

  /* Routine for metering */

  extern uint8_t stackTask[128];
  extern void TaskMeasure(void *);
}