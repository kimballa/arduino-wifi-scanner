// (c) Copyright 2022 Aaron Kimball

#ifndef __UI_WIDGETS_H
#define __UI_WIDGETS_H

#include <cstdint>

#include <TFT_eSPI.h>


// When given for width or height argument, FLEX indicates that the widget size
// should be dynamic w.r.t. fitting its contents.
constexpr int16_t FLEX = -1;

// When given for width or height argument, EQUAL indicates that the widgets in a
// Rows or Cols should be distributed equally across the bounding box of the container.
constexpr int16_t EQUAL = -2;

// A width or height return value indicating an error in the function call.
constexpr int16_t ERROR_INVALID_ELEMENT = -3;

// bitflags representing what border to draw around a given widget's bounding box.
typedef uint16_t border_flags_t;
constexpr border_flags_t BORDER_NONE   = 0x0;
constexpr border_flags_t BORDER_LEFT   = 0x1;
constexpr border_flags_t BORDER_RIGHT  = 0x2;
constexpr border_flags_t BORDER_TOP    = 0x4;
constexpr border_flags_t BORDER_BOTTOM = 0x8;
constexpr border_flags_t BORDER_RECT   = BORDER_LEFT | BORDER_RIGHT | BORDER_TOP | BORDER_BOTTOM;
constexpr border_flags_t BORDER_ROUNDED = 0x10;

// 3 pixel radius for all rounded rectangles.
constexpr int16_t BORDER_ROUNDED_RADIUS = 4;
constexpr int16_t BORDER_ROUNDED_INNER_MARGIN = 5; // rounded border moves contents in by 5 px.
constexpr int16_t BORDER_ACTIVE_INNER_MARGIN = 3; // rectangular border moves contents in by 3 px.

/** Given 3 8-bit values for (R, G, B) scale this to a 5-6-5 bit color representation. */
constexpr uint16_t makeColor565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// A color representation that, if used for border or background color, is not blitted to the screen.
constexpr uint16_t TRANSPARENT_COLOR = makeColor565(0, 255, 255);
constexpr uint16_t BG_NONE = TRANSPARENT_COLOR;


#include "screen.h"
#include "row_col.h"
#include "labels.h"

#endif // __UI_WIDGETS_H
