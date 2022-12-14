// (c) Copyright 2022 Aaron Kimball

#include "uiwidgets.h"

void Label::render(TFT_eSPI &lcd) {
  drawBackground(lcd);
  drawBorder(lcd);

  // Render the text within the child area bounding box.
  // Tee up all the settings...
  lcd.setTextFont(_fontId);
  if (_bg_color != BG_NONE) {
    lcd.setTextColor(_color, _bg_color);
  } else {
    lcd.setTextColor(_color);
  }

  // Then call virtual method to actually print the text.
  renderText(lcd);
}

void StrLabel::renderText(TFT_eSPI &lcd) {
  int16_t childX, childY, childW, childH;
  getChildAreaBoundingBox(childX, childY, childW, childH);
  lcd.drawString(_str, childX, childY);
}

void IntLabel::renderText(TFT_eSPI &lcd) {
  int16_t childX, childY, childW, childH;
  getChildAreaBoundingBox(childX, childY, childW, childH);
  lcd.drawNumber(_val, childX, childY);
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