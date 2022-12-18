// (c) Copyright 2022 Aaron Kimball
//

#include "uiwidgets.h"

void Panel::render(TFT_eSPI &lcd, uint32_t renderFlags) {
  drawBackground(lcd, renderFlags);
  drawBorder(lcd, renderFlags);
  if (_child != NULL) {
    if (isFocused(renderFlags)) {
      // If this panel is itself focused, propagate that fact to child widget.
      renderFlags |= RF_FOCUSED;
    }
    _child->render(lcd, renderFlags);
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
    render(lcd, renderFlags);
    return true;
  } else if (containsWidget(widget) && NULL != _child) {
    drawBackgroundUnderWidget(widget, lcd, renderFlags);
    return _child->redrawChildWidget(widget, lcd);
  }

  return false;
}

