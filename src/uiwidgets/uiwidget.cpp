// (c) Copyright 2022 Aaron Kimball
//

#include "uiwidgets.h"

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

void UIWidget::getRect(int16_t &cx, int16_t &cy, int16_t &cw, int16_t &ch) const {
  cx = _x;
  cy = _y;
  cw = _w;
  ch = _h;
}

void UIWidget::drawBorder(TFT_eSPI &lcd) {
  // TODO: Implement flex-height / flex-width border.

  if (_border_color == TRANSPARENT_COLOR) {
    return; // Nothing to actually render; border is transparent.
  }

  uint16_t border_color = _focused ? invertColor(_border_color) : _border_color;

  if ((_border_flags & BORDER_ROUNDED) == BORDER_ROUNDED) {
    lcd.drawRoundRect(_x, _y, _w, _h, BORDER_ROUNDED_RADIUS, border_color);
  } else if ((_border_flags & BORDER_RECT) == BORDER_RECT) {
    lcd.drawRect(_x, _y, _w, _h, border_color);
  } else if (_border_flags != BORDER_NONE) {
    if (_border_flags & BORDER_TOP) {
      lcd.drawFastHLine(_x, _y, _w, border_color);
    }

    if (_border_flags & BORDER_BOTTOM) {
      lcd.drawFastHLine(_x, _y + _h - 1, _w, border_color);
    }

    if (_border_flags & BORDER_LEFT) {
      lcd.drawFastVLine(_x, _y, _h, border_color);
    }

    if (_border_flags & BORDER_RIGHT) {
      lcd.drawFastVLine(_x + _w - 1, _y, _h, border_color);
    }
  }
}

void UIWidget::drawBackground(TFT_eSPI &lcd) {
  // TODO: Implement flex-height / flex-width background.

  if (_bg_color == TRANSPARENT_COLOR) {
    return; // Nothing to actually render; background is transparent.
  }

  uint16_t bg_color = _focused ? invertColor(_bg_color) : _bg_color;

  if (_border_flags & BORDER_ROUNDED) {
    lcd.fillRoundRect(_x, _y, _w, _h, BORDER_ROUNDED_RADIUS, bg_color);
  } else {
    lcd.fillRect(_x, _y, _w, _h, bg_color);
  }
}

void UIWidget::setPadding(int16_t padL, int16_t padR, int16_t padT, int16_t padB) {
  _paddingL = padL;
  _paddingR = padR;
  _paddingTop = padT;
  _paddingBottom = padB;

  cascadeBoundingBox();
}

void UIWidget::getPadding(int16_t &padL, int16_t &padR, int16_t &padT, int16_t &padB) const {
  padL = _paddingL;
  padR = _paddingR;
  padT = _paddingTop;
  padB = _paddingBottom;
}

bool UIWidget::containsWidget(UIWidget *widget) const {
  if (NULL == widget) {
    return false;
  } else if (widget == this) {
    return true;
  }

  if (widget->_x >= _x && widget->_x < _x + _w && widget->_y >= _y && widget->_y < _y + _h) {
    return true; // top-left corner is in.
  }
  /* Widgets must be fully nested within other widgets, there are no "partial overlaps."
   * Therefore, the top-left corner is inside this widget's BB iff the whole widget is inside this
   * one. Hanging onto these additional checks for now, just in case though...
   * TODO(aaron): Remove once we're confident they're not needed.
   * (Note that non-visible VScroll elements still "belong to" the VScroll but
   * myVScroll.contains(anOffscreenElem) will return false.)
  else if (w->_x + w->_w - 1 >= _x && w->_x + w->_w - 1 < _x + _w && w->_y >= _y && w->_y < _y + _h) {
    return true; // top-right corner
  } else if (w->_x >= _x && w->_x < _x + _w && w->_y + w->_h - 1 >= _y && w->_y + w->_h - 1< _y + _h) {
    return true; // bottom-left corner is in.
  } else if (w->_x + w->_w - 1 >= _x && w->_x + w->_w - 1 < _x + _w && w->_y + w->_h - 1 >= _y
      && w->_y + w->_h - 1 < _y + _h) {
    return true; // bottom-right corner.
  }
  */

  return false;
}

bool UIWidget::redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags) {
  // Default implementation for widgets that do not contain nested/child widgets.
  if (NULL == widget) {
    return false;
  } else if (this == widget) {
    render(lcd);
    return true;
  }

  return false;
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

  // Interior padding also removes from the child area within our borders.
  childX += _paddingL;
  childY += _paddingTop;

  childW -= _paddingL;
  childH -= _paddingTop;

  childW -= _paddingR;
  childH -= _paddingBottom;

}


int16_t UIWidget::addBorderWidth(int16_t contentWidth) const {
  // Given the width of the interior content as an arg, pad it as required to account
  // for our border.

  if ((_border_flags & BORDER_ROUNDED) == BORDER_ROUNDED) {
    contentWidth += 2 * BORDER_ROUNDED_INNER_MARGIN;
  } else {
    if (_border_flags & BORDER_LEFT) {
      contentWidth += BORDER_ACTIVE_INNER_MARGIN;
    }

    if (_border_flags & BORDER_RIGHT) {
      contentWidth += BORDER_ACTIVE_INNER_MARGIN;
    }
  }

  return contentWidth;
}

int16_t UIWidget::addBorderHeight(int16_t contentHeight) const {
  // Given the height of the interior content as an arg, pad it as required to account
  // for our border.

  if ((_border_flags & BORDER_ROUNDED) == BORDER_ROUNDED) {
    contentHeight += 2 * BORDER_ROUNDED_INNER_MARGIN;
  } else {
    if (_border_flags & BORDER_TOP) {
      contentHeight += BORDER_ACTIVE_INNER_MARGIN;
    }

    if (_border_flags & BORDER_BOTTOM) {
      contentHeight += BORDER_ACTIVE_INNER_MARGIN;
    }
  }

  return contentHeight;
}

/**
 * Fill in the background that this widget would render underneath another widget being redrawn.
 */
void UIWidget::drawBackgroundUnderWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags) {
  if (NULL == widget) {
    return;
  }

  if (_bg_color == TRANSPARENT_COLOR) {
    return; // Nothing to draw.
  }

  if ((renderFlags & RF_NO_BACKGROUNDS) == 0) {
    return; // Don't draw backgrounds in this refresh process.
  }

  uint16_t bg = _focused ? invertColor(_bg_color) : _bg_color;
  lcd.fillRect(widget->_x, widget->_y, widget->_w, widget->_h, bg);
}


//////////////////////////// Panel /////////////////////

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

int16_t Panel::getContentWidth(TFT_eSPI &lcd) const {
  int16_t w = 0;
  if (_child != NULL) {
    w = _child->getContentWidth(lcd);
  }

  return addBorderWidth(w);
}

int16_t Panel::getContentHeight(TFT_eSPI &lcd) const {
  int16_t h = 0;
  if (_child != NULL) {
    h = _child->getContentHeight(lcd);
  }

  return addBorderHeight(h);
}

bool Panel::redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags) {
  if (NULL == widget) {
    return false;
  } else if (widget == this) {
    render(lcd);
    return true;
  } else if (containsWidget(widget) && NULL != _child) {
    drawBackgroundUnderWidget(widget, lcd, renderFlags);
    return _child->redrawChildWidget(widget, lcd);
  }

  return false;
}
