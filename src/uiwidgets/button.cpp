// (c) Copyright 2022 Aaron Kimball

#include "uiwidgets.h"

void UIButton::render(TFT_eSPI &lcd, uint32_t renderFlags) {
  // We explicitly ignore (clobber, actually) border and background.
  _border_flags = BORDER_NONE;

  // Calculate usable region inside any padding the user has specified.
  int16_t childX, childY, childW, childH;
  getChildAreaBoundingBox(childX, childY, childW, childH);

  lcd.setTextFont(_fontId);

  if (isFocused(renderFlags)) {
    lcd.fillRoundRect(childX, childY, childW, childH, BORDER_ROUNDED_RADIUS, _color);
    lcd.setTextColor(invertColor(_color));
  } else {
    lcd.drawRoundRect(childX, childY, childW, childH, BORDER_ROUNDED_RADIUS, _color);
    lcd.setTextColor(_color);
  }

  lcd.drawString(_btnLabel, childX + BORDER_ROUNDED_INNER_MARGIN, childY + BORDER_ROUNDED_INNER_MARGIN);
}

int16_t UIButton::getContentWidth(TFT_eSPI &lcd) const {
  return lcd.textWidth(_btnLabel, _fontId) + 2 * BORDER_ROUNDED_INNER_MARGIN;
}

int16_t UIButton::getContentHeight(TFT_eSPI &lcd) const {
  return lcd.fontHeight(_fontId) + 2 * BORDER_ROUNDED_INNER_MARGIN;
}

