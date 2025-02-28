#pragma once

#include "DisplayGlobal.hpp"
#include "ExposureMetering.hpp"

namespace ExposureInfoDisp
{
  extern void RedrawTAndF();

  extern void DispIso(float iso, int column, int line);
  extern void DispShutter(float shutter, int column, int line);
  extern void DispFStop(float fStop, int column, int line);

  extern void DispCameraSettings();
}
