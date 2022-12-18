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
constexpr int16_t BORDER_ROUNDED_RADIUS = 5;
constexpr int16_t BORDER_ROUNDED_INNER_MARGIN = 4; // rounded border moves contents in by 4 px.
constexpr int16_t BORDER_ACTIVE_INNER_MARGIN = 3; // rectangular border moves contents in by 3 px.

/** Given 3 8-bit values for (R, G, B) scale this to a 5-6-5 bit color representation. */
constexpr uint16_t makeColor565(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/** Given a 5-6-5 color representation, return the inverted color. */
constexpr uint16_t invertColor(uint16_t original565color) {
  return ~original565color;
}

// A color representation that, if used for border or background color, is not blitted to the screen.
constexpr uint16_t TRANSPARENT_COLOR = TFT_TRANSPARENT;
constexpr uint16_t BG_NONE = TRANSPARENT_COLOR;

class Screen; // fwd-declaration needed.

/** Base UIWidget class that all widget instances extend. */
class UIWidget {
public:
  UIWidget(): _x(0), _y(0), _w(0), _h(0),
      _border_flags(BORDER_NONE), _border_color(TFT_WHITE), _bg_color(BG_NONE), _focused(false),
      _paddingL(0), _paddingR(0), _paddingTop(0), _paddingBottom(0) {
  };

  virtual ~UIWidget() {};

  /** Render the widget to the screen, along with any child widgets. */
  virtual void render(TFT_eSPI &lcd, uint32_t renderFlags) = 0;

  /** Set the bounding box for this widget. */
  void setBoundingBox(int16_t x, int16_t y, int16_t w, int16_t h);

  /** Called by setBoundingBox(); cascades bounding box requirements to any child elements. */
  virtual void cascadeBoundingBox() { };

  void setBorder(const border_flags_t flags, uint16_t color=TFT_WHITE);
  /**
   * A color to fill in for the background, or BG_NONE for no background (i.e., inherit bg from
   * container).
   */
  void setBackground(uint16_t color);

  int16_t getX() const { return _x; };
  int16_t getY() const { return _y; };

  int16_t getWidth() const { return _w; }; // Return width of widget apportioned for rendering.
  int16_t getHeight() const { return _h; }; // Return height of widget apportioned for rendering.

  // Return the entire rect of getX(), getY(), getWidth() and getHeight().
  void getRect(int16_t &cx, int16_t &cy, int16_t &cw, int16_t &ch) const;

  // Return width required for widget to render all content without overflow.
  virtual int16_t getContentWidth(TFT_eSPI &lcd) const = 0;
  // Return height required for widget to render all content without overflow.
  virtual int16_t getContentHeight(TFT_eSPI &lcd) const = 0;

  // Return true if this item is explicitly focused with this->setFocus(true).
  bool isFocused() const { return _focused; };
  // Return true if this item is explicitly focused, or has inherited focus from parent widget
  // for this rendering context.
  bool isFocused(uint32_t renderFlags) const;
  void setFocus(bool focus) { _focused = focus; };

  void setPadding(int16_t padL, int16_t padR, int16_t padT, int16_t padB);
  void getPadding(int16_t &padL, int16_t &padR, int16_t &padT, int16_t &padB) const;

  // Returns true if 'widget' is within this widget's containment / draw area.
  bool containsWidget(UIWidget *widget) const;
  // Redraw only the part of the screen contained by widget 'widget'.
  // Returns true if we handled the redraw.
  virtual bool redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags=0);

protected:
  void drawBorder(TFT_eSPI &lcd, uint32_t renderFlags);
  void drawBackground(TFT_eSPI &lcd, uint32_t renderFlags);
  void drawBackgroundUnderWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags=0);

  /** Get area bounding box available for rendering within the context of any border or other
   * padding that belongs to this widget.
   */
  void getChildAreaBoundingBox(int16_t &childX, int16_t &childY, int16_t &childW, int16_t &childH) const;

  /** Adds required margin for L/R border to specified content width. */
  int16_t addBorderWidth(int16_t contentWidth) const;
  /** Adds required margin for top/bottom border to specified content height. */
  int16_t addBorderHeight(int16_t contentHeight) const;

  int16_t _x, _y;
  int16_t _w, _h;

  border_flags_t _border_flags;
  uint16_t _border_color;
  uint16_t _bg_color;

  bool _focused;

  // Additional user controlled interior padding.
  int16_t _paddingL, _paddingR, _paddingTop, _paddingBottom;

  friend class Screen;
};


#include "screen.h"
#include "panel.h"
#include "row_col.h"
#include "labels.h"
#include "vscroll.h"
#include "button.h"

#endif // __UI_WIDGETS_H
