#include "ExposureMetering.hpp"

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <TaskScheduler.hpp>

bool ExposureMetering::shutterPriority = false;

float ExposureMetering::iso = 400.0f;
float ExposureMetering::shutter = 1.0f/125.0f;
float ExposureMetering::fStops = 5.6f;
float ExposureMetering::evWithOffset = 0.0f;

bool ExposureMetering::measurementUpdated = false;

Adafruit_TSL2591 ExposureMetering::lightSensor = Adafruit_TSL2591(2591);

ExposureMetering::GainLevel::GainLevel(tsl2591Gain_t newGain, tsl2591IntegrationTime_t newTiming, int32_t newIncThreshold, int32_t newDecThreshold)
{
  gain = newGain;
  timing = newTiming;

  incThreshold = newIncThreshold;
  decThreshold = newDecThreshold;
}

const ExposureMetering::GainLevel ExposureMetering::gainLevels[] = 
{
  GainLevel(TSL2591_GAIN_LOW, TSL2591_INTEGRATIONTIME_100MS, 16384, 65536),
  GainLevel(TSL2591_GAIN_LOW, TSL2591_INTEGRATIONTIME_300MS, 24768, 65535),
  GainLevel(TSL2591_GAIN_LOW, TSL2591_INTEGRATIONTIME_600MS, 2560, 65535),
  GainLevel(TSL2591_GAIN_MED, TSL2591_INTEGRATIONTIME_600MS, 2560, 65535),
  GainLevel(TSL2591_GAIN_HIGH, TSL2591_INTEGRATIONTIME_600MS, 2560, 65535),
  GainLevel(TSL2591_GAIN_MAX, TSL2591_INTEGRATIONTIME_600MS, -1, 65535)
};
const int ExposureMetering::maxGainLevel = 5;
int ExposureMetering::gainLevel = 0;

void ExposureMetering::SetGainLevel(int newGainLevel)
{
  if (newGainLevel < 0)
    newGainLevel = 0;
  if (newGainLevel > maxGainLevel)
    newGainLevel = maxGainLevel;

  gainLevel = newGainLevel;

  lightSensor.setGain(gainLevels[gainLevel].gain);
  lightSensor.setTiming(gainLevels[gainLevel].timing);
}

void ExposureMetering::SetupLightSensor()
{
  lightSensor.begin();
  SetGainLevel(0);
}

uint8_t ExposureMetering::stackTask[128];
void ExposureMetering::TaskMeasure(void *)
{
  while (true)
  {
    if (measurementUpdated)
    {
      TaskScheduler::Yield();
      continue;
    }
    
    /* Get luminosity in lux from TSL2591X */
    bool switchGain = true;
    uint32_t luminosityRaw;
    uint16_t luminosityIr, luminosityFull;
    while (switchGain)
    {
      luminosityRaw = lightSensor.getFullLuminosity();
      luminosityIr = luminosityRaw >> 16;
      luminosityFull = luminosityRaw & 0xFFFF;

      bool incGain = ((int32_t)luminosityFull <= gainLevels[gainLevel].incThreshold);
      bool decGain = ((int32_t)luminosityFull >= gainLevels[gainLevel].decThreshold);
      switchGain = (incGain || decGain);
      if (switchGain)
      {
        if (decGain)
          SetGainLevel(gainLevel - 1);
        if (incGain)
          SetGainLevel(gainLevel + 1);
      }
    }
    float luminosityLux = lightSensor.calculateLux(luminosityFull, luminosityIr);

    /* Convert lux to exposure value */
    float exposureVal = (logf(luminosityLux / 0.18f) - logf(2.5f)) / logf(2.0f);

    /* Calculate camera settings using the exposure value */
    /*
    float iso = 400.0f;
    float shutter;
    float fStops = 5.6f;
    */

    float evOffset = logf(iso / 100.0f) / logf(2.0f);
    // float evWithOfs = exposureVal + evOffset;
    evWithOffset = exposureVal + evOffset;

    /* Formula: EV = log2((N ^ 2) / t) */

    if (shutterPriority)
    {
      /* Shutter priority */
      fStops = sqrt(pow(2, evWithOffset) * shutter);
    }
    else
    {
      /* Aperture priority */
      shutter = powf(fStops, 2.0f) / powf(2.0f, evWithOffset);
    }

    measurementUpdated = true;
  }
}
