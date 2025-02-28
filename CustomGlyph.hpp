#pragma once

#include <Arduino.h>

namespace CustomGlyph
{
  extern uint8_t glyphAv[];
  extern uint8_t glyphTv[];
  extern uint8_t glyphSet[];
  extern uint8_t glyphIso[];
  extern uint8_t glyphEv[];
  extern uint8_t glyphTSmall[];
  extern uint8_t glyphShutterFraction[];
  extern uint8_t glyphFSmall[];

  extern void SetInverse(uint8_t *glyph, bool newInverse);

  extern void DrawGlyph(uint8_t *glyph);
}
