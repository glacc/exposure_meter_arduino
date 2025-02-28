#include "CustomGlyph.hpp"
#include "DisplayGlobal.hpp"

uint8_t CustomGlyph::glyphAv[] =
{
  false,
  9,
  0b00000000,
  0b01111000,
  0b00010100,
  0b01111000,
  0b00000000,
  0b00110000,
  0b01000000,
  0b00110000,
  0b00000000
};

uint8_t CustomGlyph::glyphTv[] =
{
  false,
  9,
  0b00000000,
  0b00000100,
  0b01111100,
  0b00000100,
  0b00000000,
  0b00110000,
  0b01000000,
  0b00110000,
  0b00000000
};

uint8_t CustomGlyph::glyphSet[] =
{
  false,
  12,
  0b00000000,
  0b01001000,
  0b01010100,
  0b00100100,
  0b00000000,
  0b00110000,
  0b01101000,
  0b01010000,
  0b00000000,
  0b00111100,
  0b01001000,
  0b00000000
};

uint8_t CustomGlyph::glyphIso[] =
{
  false,
  13,
  0b00000000,
  0b01000100,
  0b01111100,
  0b01000100,
  0b00000000,
  0b01001000,
  0b01010100,
  0b00100100,
  0b00000000,
  0b00111000,
  0b01000100,
  0b00111000,
  0b00000000
};

uint8_t CustomGlyph::glyphEv[] =
{
  false,
  9,
  0b00000000,
  0b01111100,
  0b01010100,
  0b01000100,
  0b00000000,
  0b00111100,
  0b01000000,
  0b00111100,
  0b00000000
};

uint8_t CustomGlyph::glyphTSmall[] = 
{
  false,
  5,
  0b00000000,
  0b00000100,
  0b01111100,
  0b00000100,
  0b00000000
};

uint8_t CustomGlyph::glyphShutterFraction[] =
{
  false,
  7,
  0b00010010,
  0b00011111,
  0b00010000,
  0b00000000,
  0b00010000,
  0b00001110,
  0b00000001
};

uint8_t CustomGlyph::glyphFSmall[] =
{
  false,
  5,
  0b00000000,
  0b01111100,
  0b00010100,
  0b00000100,
  0b00000000
};

void CustomGlyph::SetInverse(uint8_t *glyph, bool newInverse)
{
  uint8_t *glyphPtr = glyph;

  if (*glyphPtr == newInverse)
    return;

  *glyphPtr = newInverse;
  glyphPtr++;

  uint8_t glyphLength = *glyphPtr++;
  for (uint8_t i = 0; i < glyphLength; i++)
  {
    *glyphPtr = ~(*glyphPtr);
    glyphPtr++;
  }
}

void CustomGlyph::DrawGlyph(uint8_t *glyph)
{
  display.drawBitmap(&glyph[2], glyph[1], 1);
}
