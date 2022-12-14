// (c) Copyright 2022 Aaron Kimball
//

#include "uiwidgets.h"

void Screen::render() {
  _lcd.fillScreen(_bgColor);
  if (NULL != _widget) {
    _widget->render(_lcd);
  }
}

void Screen::setWidget(UIWidget *w) {
  _widget = w;
  if (NULL != _widget) {
    _widget->setBoundingBox(0, 0, getWidth(), getHeight());
  }
}

void UIWidget::setBoundingBox(int16_t x, int16_t y, int16_t w, int16_t h) {
  // Update the bounding box for our own rendering.
  _x = x;
  _y = y;
  _w = w;
  _h = h;

  // Update the bounding boxes of any nested elements.
  cascadeBoundingBox();
}

void UIWidget::setBorder(const border_flags_t flags, uint16_t color) {
  _border_flags = flags;
  _border_color = color;

  // Update the bounding boxes of any nested elements.
  cascadeBoundingBox();
}

void UIWidget::setBackground(uint16_t color) {
  _bg_color = color;
}

void UIWidget::drawBorder(TFT_eSPI &lcd) {
  // TODO: Implement flex-height / flex-width border.

  if (_border_color == TRANSPARENT_COLOR) {
    return; // Nothing to actually render; border is transparent.
  }

  if (_border_flags & BORDER_ROUNDED) {
    lcd.drawRoundRect(_x, _y, _w, _h, BORDER_ROUNDED_RADIUS, _border_color);
  } else {
    if (_border_flags & BORDER_TOP) {
      lcd.drawFastHLine(_x, _y, _w, _border_color);
    }

    if (_border_flags & BORDER_BOTTOM) {
      lcd.drawFastHLine(_x, _y + _h - 1, _w, _border_color);
    }

    if (_border_flags & BORDER_LEFT) {
      lcd.drawFastVLine(_x, _y, _h, _border_color);
    }

    if (_border_flags & BORDER_RIGHT) {
      lcd.drawFastVLine(_x + _w - 1, _y, _h, _border_color);
    }
  }
}

void UIWidget::drawBackground(TFT_eSPI &lcd) {
  // TODO: Implement flex-height / flex-width background.

  if (_bg_color == TRANSPARENT_COLOR) {
    return; // Nothing to actually render; background is transparent.
  }

  if (_border_flags & BORDER_ROUNDED) {
    lcd.fillRoundRect(_x, _y, _w, _h, BORDER_ROUNDED_RADIUS, _bg_color);
  } else {
    lcd.fillRect(_x, _y, _w, _h, _bg_color);
  }
}

void UIWidget::getChildAreaBoundingBox(int16_t &childX, int16_t &childY, int16_t &childW, int16_t &childH) {
  // The bounding box for a whole-content child is the same as that of this object,
  // unless there's a border, in which case we need to give that border some breathing room.

  childX = _x;
  childY = _y;
  childW = _w;
  childH = _h;

  if (_border_flags & BORDER_ROUNDED) {
    childX += BORDER_ROUNDED_INNER_MARGIN;
    childY += BORDER_ROUNDED_INNER_MARGIN;
    childW -= 2 * BORDER_ROUNDED_INNER_MARGIN;
    childH -= 2 * BORDER_ROUNDED_INNER_MARGIN;
  } else {
    if (_border_flags & BORDER_TOP) {
      childY += BORDER_ACTIVE_INNER_MARGIN;
      childH -=  BORDER_ACTIVE_INNER_MARGIN;
    }

    if (_border_flags & BORDER_BOTTOM) {
      childH -= BORDER_ACTIVE_INNER_MARGIN;
    }

    if (_border_flags & BORDER_LEFT) {
      childX += BORDER_ACTIVE_INNER_MARGIN;
      childW -= BORDER_ACTIVE_INNER_MARGIN;
    }

    if (_border_flags & BORDER_RIGHT) {
      childW -= BORDER_ACTIVE_INNER_MARGIN;
    }
  }
}

void Panel::render(TFT_eSPI &lcd) {
  drawBackground(lcd);
  drawBorder(lcd);
  if (_child != NULL) {
    _child->render(lcd);
  }
}

void Panel::cascadeBoundingBox() {
  if (_child == NULL) {
    return; // Nothing to cascade.
  }

  int16_t childX, childY, childW, childH;
  getChildAreaBoundingBox(childX, childY, childW, childH);

  // Pass the entire inner child bounding box on to our sole child.
  _child->setBoundingBox(childX, childY, childW, childH);
}
