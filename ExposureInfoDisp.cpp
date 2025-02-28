#include "ExposureInfoDisp.hpp"

#include "CustomGlyph.hpp"

/* Routine for displaying camera settings */

void ExposureInfoDisp::RedrawTAndF()
{
  int line = 2;
  display.setCursor(0, line);
  display.clearLine();
  display.setCursor(1, line);
  CustomGlyph::SetInverse(CustomGlyph::glyphTSmall, ExposureMetering::shutterPriority);
  CustomGlyph::DrawGlyph(CustomGlyph::glyphTSmall);

  display.setCursor(43, line);
  CustomGlyph::SetInverse(CustomGlyph::glyphFSmall, !ExposureMetering::shutterPriority);
  CustomGlyph::DrawGlyph(CustomGlyph::glyphFSmall);
}

namespace
{
  void TrimTrailingZeros(char *str, int len)
  {
    bool isNonInteger = false;

    int offset = 0;
    while (offset < len)
    {
      if (str[offset] != ' ')
        break;

      offset++;
    }

    while (offset < len)
    {
      char currChar = str[offset];

      if (currChar == '.')
        isNonInteger = true;

      if (currChar == '\0' || currChar == ' ')
        break;
      
      offset++;
    }

    if (isNonInteger)
    {
      offset--;
      while (offset >= 0)
      {
        char currChar = str[offset];

        if (currChar != '0')
        {
          if (currChar != '.')
            str[offset + 1] = '\0';
          else
            str[offset] = '\0';

          break;
        }

        offset--;
      }
    }
  }
}

void ExposureInfoDisp::DispIso(float iso, int column, int line)
{
  char isoStr[12];

  dtostrf(iso, -9, 2, isoStr);
  TrimTrailingZeros(isoStr, 12);

  display.setCursor(column, line);
  display.print(isoStr);
}

void ExposureInfoDisp::DispShutter(float shutter, int column, int line)
{
  if (shutter < 1.0f)
  {
    float shutterDenominator = 1.0f / shutter;

    display.setCursor(column, line);
    // display.print("1/");
    CustomGlyph::DrawGlyph(CustomGlyph::glyphShutterFraction);

    display.setCursor(column + 8, line);
    if (round(shutterDenominator) >= 10.0f)
      display.print(shutterDenominator, 0);
    else
      display.print(shutterDenominator, 1);
  }
  else
  {
    display.setCursor(column + 8, line);
    if (round(shutter) >= 10.0f)
      display.print(shutter, 0);
    else
      display.print(shutter, 1);

    display.print('\"');
  }
}

void ExposureInfoDisp::DispFStop(float fStop, int column, int line)
{
  display.setCursor(column, line);
  if (round(fStop) >= 10.0f)
    display.print(fStop, 0);
  else if (round(fStop * 100.0f) >= 100.0f)
    display.print(fStop, 1);
  else
    display.print(fStop, 2);
}

void ExposureInfoDisp::DispCameraSettings()
{
  /* Information display starts here */

  int line;

  /* ISO and EV */

  line = 0;
  display.setCursor(0, line);
  display.clearLine();
  display.setCursor(1, line);
  // display.print("ISO");
  CustomGlyph::SetInverse(CustomGlyph::glyphIso, false);
  CustomGlyph::DrawGlyph(CustomGlyph::glyphIso);
  // display.setCursor(15, line);
  // display.print(ExposureMetering::iso, 0);
  DispIso(ExposureMetering::iso, 15, line);
  display.setCursor(43, line);
  // display.print("EV");
  CustomGlyph::DrawGlyph(CustomGlyph::glyphEv);
  display.setCursor(53, line);
  display.print(ExposureMetering::evWithOffset, 1);

  /* Shutter and aperture */

  line = 2;
  RedrawTAndF();
  DispShutter(ExposureMetering::shutter, 7, line);
  DispFStop(ExposureMetering::fStops, 53, line);
}