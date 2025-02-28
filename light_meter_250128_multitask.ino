/*   Exposure Meter   */
/*   Glacc 25-01-22   */
/*                    */
/* Board: Arduino Uno */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2591.h>
#include <Encoder.h>
#include <TaskScheduler.hpp>

#include "DisplayGlobal.hpp"
#include "CameraSettingsLUT.hpp"
#include "ExposureMetering.hpp"
#include "ExposureInfoDisp.hpp"
#include "CustomGlyph.hpp"

int taskIdMetering = -1;

Task tasks[3];
const int maxTasks = 3;

/* Selected camera settings from LUTs */

int isoSelected = CameraSettingsLUT::isoDefaultPos;
int fStopSelected = CameraSettingsLUT::fStopDefaultPos;
int shutterSpeedSelected = CameraSettingsLUT::shutterSpeedDefaultPos;

/* Rotation sensor */

bool buttonPressedFlag = false;
bool buttonReleaseFlag = false;
const int pinEncoderS1A = 2;
const int pinEncoderS1B = 4;
const int pinEncoderSW = 3;
Encoder rotationSensor(pinEncoderS1A, pinEncoderS1B);
bool DebounceCheck(unsigned long &time, int durationMs = 10)
{
  unsigned long newTime = millis();
  bool debounce = ((newTime - time) > durationMs);
  time = newTime;

  return debounce;
}

void SetupRotationSensor()
{
  pinMode(pinEncoderSW, INPUT);

  attachInterrupt(digitalPinToInterrupt(pinEncoderSW), OnEncoderPress, CHANGE);
}

void OnEncoderPress()
{
  cli();

  int pinStateSW = digitalRead(pinEncoderSW);

  static uint8_t statesSw = 0b11;

  static unsigned long time = 0;
  if (!DebounceCheck(time, 8))
    return;

  statesSw = ((statesSw << 1) | ((pinStateSW == HIGH) ? 0b1 : 0b0)) & 0b11;

  if (statesSw == 0b10)
    buttonPressedFlag = true;

  if (statesSw == 0b01)
    buttonReleaseFlag = true;

  sei();
}

/* Display and menus */

int selectedItem = 0;

/*
  Is setting:
    0: Nothing
    1: Aperture
    2: Shutter
    3: ISO
*/
int isSetting = 0;
int settingValue;

void RestoreToSelected()
{
  ExposureMetering::iso = pgm_read_float(CameraSettingsLUT::isoList + isoSelected);
  ExposureMetering::shutter = CameraSettingsLUT::GetShutterSpeed(shutterSpeedSelected);
  ExposureMetering::fStops = pgm_read_float(CameraSettingsLUT::fStopsList + fStopSelected);
}

void UpdateAndDrawMenu()
{
  const uint8_t *glyphToDraw[4] = 
  {
    CustomGlyph::glyphAv,
    CustomGlyph::glyphTv,
    CustomGlyph::glyphIso,
    CustomGlyph::glyphSet
  };

  if (selectedItem < 0)
    selectedItem = 0;
  if (selectedItem >= 4)
    selectedItem = 3;

  if (buttonPressedFlag)
  {
    switch (selectedItem)
    {
      case 0:
        // Aperture priority mode
        if (!ExposureMetering::shutterPriority)
          break;

        TaskScheduler::SuspendTask(taskIdMetering);

        RestoreToSelected();
        ExposureMetering::shutterPriority = false;

        ExposureInfoDisp::RedrawTAndF();

        TaskScheduler::ResumeTask(taskIdMetering);
        break;
      case 1:
        // Shutter priority mode
        if (ExposureMetering::shutterPriority)
          break;

        TaskScheduler::SuspendTask(taskIdMetering);

        RestoreToSelected();
        ExposureMetering::shutterPriority = true;

        ExposureInfoDisp::RedrawTAndF();

        TaskScheduler::ResumeTask(taskIdMetering);
        break;
      case 2:
        // ISO
        isSetting = 3;

        settingValue = isoSelected;

        buttonPressedFlag = false;
        UpdateAndDrawSettings();

        TaskScheduler::SuspendTask(taskIdMetering);

        return;
      case 3:
        // Set
        if (ExposureMetering::shutterPriority)
        {
          settingValue = shutterSpeedSelected;
          isSetting = 2;
        }
        else
        {
          settingValue = fStopSelected;
          isSetting = 1;
        }

        buttonPressedFlag = false;
        UpdateAndDrawSettings();

        TaskScheduler::SuspendTask(taskIdMetering);

        return;
    }

    buttonPressedFlag = false;
  }

  for (int i = 0; i < 4; i++)
  {
    uint8_t *glyph = (uint8_t *)glyphToDraw[i];

    display.setCursor(i * 84 / 5, 5);
    CustomGlyph::SetInverse(glyph, selectedItem == i);
    CustomGlyph::DrawGlyph(glyph);
  }
}

void DrawSettingNumber()
{
  const int line = 4;

  if (isSetting == 0)
    return;

  display.setCursor(0, line);
  display.clearLine();

  display.setCursor(1, line);
  CustomGlyph::SetInverse(CustomGlyph::glyphSet, false);
  CustomGlyph::DrawGlyph(CustomGlyph::glyphSet);

  /*
    Is setting:
      0: Nothing
      1: Aperture
      2: Shutter
      3: ISO
  */
  
  float fStop, shutter, iso;
  const int startColumn = 16;
  display.setCursor(startColumn, line);
  switch (isSetting)
  {
    case 1:
      CustomGlyph::SetInverse(CustomGlyph::glyphFSmall, false);
      CustomGlyph::DrawGlyph(CustomGlyph::glyphFSmall);

      fStop = pgm_read_float(CameraSettingsLUT::fStopsList + settingValue);
      ExposureInfoDisp::DispFStop(fStop, startColumn + 16, line);

      break;
    case 2:
      CustomGlyph::SetInverse(CustomGlyph::glyphTSmall, false);
      CustomGlyph::DrawGlyph(CustomGlyph::glyphTSmall);

      shutter = CameraSettingsLUT::GetShutterSpeed(settingValue);
      ExposureInfoDisp::DispShutter(shutter, startColumn + 16, line);

      break;
    case 3:
      CustomGlyph::SetInverse(CustomGlyph::glyphIso, false);
      CustomGlyph::DrawGlyph(CustomGlyph::glyphIso);

      iso = pgm_read_float(CameraSettingsLUT::isoList + settingValue);
      ExposureInfoDisp::DispIso(iso, startColumn + 16, line);

      break;
  }
}

void UpdateAndDrawSettings()
{
  /*
    Is setting:
      0: Nothing
      1: Aperture
      2: Shutter
      3: ISO
  */

  // Clamp value
  switch (isSetting)
  {
    case 1:
      if (settingValue < 0)
        settingValue = 0;
      if (settingValue >= CameraSettingsLUT::fStopCount)
        settingValue = CameraSettingsLUT::fStopCount - 1;
      break;
    case 2:
      if (settingValue < -CameraSettingsLUT::shutterSpeedCountAbove1)
        settingValue = -CameraSettingsLUT::shutterSpeedCountAbove1;
      if (settingValue >= CameraSettingsLUT::shutterSpeedCountSub1)
        settingValue = CameraSettingsLUT::shutterSpeedCountSub1 - 1;
      break;
    case 3:
      if (settingValue < 0)
        settingValue = 0;
      if (settingValue >= CameraSettingsLUT::isoCount)
        settingValue = CameraSettingsLUT::isoCount - 1;
      break;
  }

  if (buttonPressedFlag)
  {
    switch (isSetting)
    {
      case 1:
        fStopSelected = settingValue;
        ExposureMetering::fStops = pgm_read_float(CameraSettingsLUT::fStopsList + fStopSelected);
        break;
      case 2:
        shutterSpeedSelected = settingValue;
        ExposureMetering::shutter = CameraSettingsLUT::GetShutterSpeed(shutterSpeedSelected);
        break;
      case 3:
        isoSelected = settingValue;
        ExposureMetering::iso = pgm_read_float(CameraSettingsLUT::isoList + settingValue);
        break;
    }

    isSetting = 0;

    display.setCursor(0, 4);
    display.clearLine();

    buttonPressedFlag = false;
    
    UpdateAndDrawMenu();

    TaskScheduler::ResumeTask(taskIdMetering);

    return;
  }

  DrawSettingNumber();
}

uint8_t stackDisplay[256];
void TaskDisplay(void *)
{
  static int rotationCount, rotationCountOld;
  rotationCount = rotationCountOld = rotationSensor.read() / 4;

  ExposureInfoDisp::RedrawTAndF();
  UpdateAndDrawMenu();

  while (true)
  {
    if (ExposureMetering::measurementUpdated)
    {
      ExposureInfoDisp::DispCameraSettings();
      ExposureMetering::measurementUpdated = false;
    }

    bool updateMenu = false;

    rotationCount = rotationSensor.read() / 4;
    if (rotationCount != rotationCountOld)
    {
      int delta = -(rotationCount - rotationCountOld);
      if (isSetting == 0)
        selectedItem += delta;
      else
        settingValue += delta;

      updateMenu = true;
    }
    rotationCountOld = rotationCount;

    if (buttonPressedFlag)
    {
      rotationSensor.write(0);
      rotationCount = rotationCountOld = 0;
      
      updateMenu = true;
    }

    if (updateMenu)
    {
      if (isSetting == 0)
        UpdateAndDrawMenu();
      else
        UpdateAndDrawSettings();
    }

    TaskScheduler::Yield();
  }
}

/* Idle task that never calls delay */

uint8_t stackIdle[80];
void TaskIdle(void *)
{
  while (true)
    TaskScheduler::Yield();
}

/* Task initializer */

void InitTasks()
{
  TaskScheduler::Init(tasks, maxTasks);

  TaskScheduler::AddTask(TaskIdle, stackIdle, sizeof(stackIdle), 0);

  TaskScheduler::AddTask(TaskDisplay, stackDisplay, sizeof(stackDisplay), 1);
  taskIdMetering = TaskScheduler::AddTask(ExposureMetering::TaskMeasure, ExposureMetering::stackTask, sizeof(ExposureMetering::stackTask), 1);

  TaskScheduler::Run();
}

/* Setup */

void setup()
{
  RestoreToSelected();

  ExposureMetering::SetupLightSensor();
  SetupRotationSensor();

  display.begin(84, 48);

  InitTasks();
}

void loop()
{
}
