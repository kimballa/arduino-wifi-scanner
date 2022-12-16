// (c) Copyright 2022 Aaron Kimball
//

#include "uiwidgets.h"

void Screen::render() {
  _lcd.fillScreen(_bgColor);
  if (NULL != _widget) {
    _widget->render(_lcd);
  }
}

void Screen::renderWidget(UIWidget *widget, uint32_t renderFlags) {
  if (widget == NULL || _widget == NULL) {
    return;
  }

  if (_bgColor != TRANSPARENT_COLOR && (renderFlags & RF_NO_BACKGROUNDS) == 0) {
    _lcd.fillRect(widget->_x, widget->_y, widget->_w, widget->_h, _bgColor);
  }

  _widget->redrawChildWidget(widget, _lcd, renderFlags);
}

void Screen::setWidget(UIWidget *w) {
  _widget = w;
  if (NULL != _widget) {
    _widget->setBoundingBox(0, 0, getWidth(), getHeight());
  }
}

