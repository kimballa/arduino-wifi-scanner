// (c) Copyright 2022 Aaron Kimball

#include "uiwidgets.h"
#include <math.h>

void Label::render(TFT_eSPI &lcd) {
  drawBackground(lcd);
  drawBorder(lcd);

  // Render the text within the child area bounding box.
  // Tee up all the settings...
  lcd.setTextFont(_fontId);
  uint16_t text_color = _focused ? invertColor(_color) : _color;
  if (_bg_color != BG_NONE) {
    uint16_t bg = _focused ? invertColor(_bg_color) : _bg_color;
    lcd.setTextColor(text_color, bg);
  } else {
    lcd.setTextColor(text_color);
  }

  // Then call virtual method to actually print the text.
  renderText(lcd);
}

void StrLabel::renderText(TFT_eSPI &lcd) {
  int16_t childX, childY, childW, childH;
  getChildAreaBoundingBox(childX, childY, childW, childH);
  lcd.drawString(_str, childX, childY);
}

int16_t StrLabel::getContentWidth(TFT_eSPI &lcd) const {
  return addBorderWidth(lcd.textWidth(_str, _fontId));
}

int16_t StrLabel::getContentHeight(TFT_eSPI &lcd) const {
  return addBorderHeight(lcd.fontHeight(_fontId));
}

void IntLabel::renderText(TFT_eSPI &lcd) {
  int16_t childX, childY, childW, childH;
  getChildAreaBoundingBox(childX, childY, childW, childH);
  lcd.drawNumber(_val, childX, childY);
}

int16_t IntLabel::getContentWidth(TFT_eSPI &lcd) const {
  if (_val == 0) {
    return addBorderWidth(lcd.textWidth("0", _fontId));
  } else {
    // There is no textWidth() for numbers, but we know the number of digits is
    // proportional to the base10 log of the number.
    int neg = 0;
    long num = _val;
    if (_val < 0) {
      neg = 1; // Add a character for the leading '-' sign.
      num = -_val;
    }
    return addBorderWidth((neg + ceil(log10(num))) * lcd.textWidth("0", _fontId));
  }

}

int16_t IntLabel::getContentHeight(TFT_eSPI &lcd) const {
  return addBorderHeight(lcd.fontHeight(_fontId));
}

void FloatLabel::renderText(TFT_eSPI &lcd) {
  int16_t childX, childY, childW, childH;
  getChildAreaBoundingBox(childX, childY, childW, childH);
  lcd.drawFloat(_val, _maxDecimalDigits, childX, childY);
}

void FloatLabel::setMaxDecimalDigits(uint8_t digits) {
  if (digits > 7) {
    digits = 7; // Max supported by TFT_eSPI::drawFloat().
  }

  _maxDecimalDigits = digits;
}

int16_t FloatLabel::getContentWidth(TFT_eSPI &lcd) const {
  if (_val == 0) {
    return addBorderWidth(lcd.textWidth("0", _fontId));
  } else {
    // There is no textWidth() for numbers, but we know the number of digits for
    // the integer part is proportional to the base10 log of the number, and we
    // know the max digits we plan to place after the decimal. 
    // TODO(aaron): Can we just drawFloat() someplace offscreen and use the return value of
    // that method?
    return addBorderWidth((ceil(log10(fabs(_val))) + _maxDecimalDigits + 1) * lcd.textWidth("0", _fontId));
  }
}

int16_t FloatLabel::getContentHeight(TFT_eSPI &lcd) const {
  return addBorderHeight(lcd.fontHeight(_fontId));
}
