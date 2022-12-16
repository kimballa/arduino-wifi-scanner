// (c) Copyright 2022 Aaron Kimball

#include "uiwidgets.h"

// The button boxes at top and bottom of the scrollbar are 12 px tall.
static constexpr int16_t scrollBoxWidgetHeight = 12;

void VScroll::remove(UIWidget *widget) {
  for (auto it = _entries.begin(); it != _entries.end(); it++) {
    if (*it == widget) {
      // Erase the element at cur iterator position.
      _entries.erase(it);
      cascadeBoundingBox();
      return; // All done.
    }
  }
}

void VScroll::render(TFT_eSPI &lcd, uint32_t renderFlags) {
  if (_content_bg_color != TRANSPARENT_COLOR && (renderFlags & RF_NO_BACKGROUNDS) == 0) {
    // Instead of the standard drawBackground(), we fill the content area
    // with the content-area-specific background color.
    lcd.fillRect(_x, _y, _w - VSCROLL_SCROLLBAR_W, _h, _content_bg_color);
  }

  renderScrollbar(lcd);
  renderContentArea(lcd);
}

void VScroll::renderScrollbar(TFT_eSPI &lcd) {
  // X position of the left-most edge of the scrollbar.
  int16_t scrollbarX = _x + _w - VSCROLL_SCROLLBAR_W;

  if (_scrollbar_bg_color != TRANSPARENT_COLOR) {
    // Fill in the background of the scrollbar area.
    // Since the left & right edges will be taken up completely with the vertical borders of
    // the scrollbar, do not include them in the fill. The areas near the top and bottom
    // containing the carets will also be filled in renderScrollUp()/renderScrollDown(), so
    // we only need to focus on the main scroll indicator.
    uint16_t scrollbarBg = _focused ? invertColor(_scrollbar_bg_color) : _scrollbar_bg_color;
    lcd.fillRect(scrollbarX + 1, _y + scrollBoxWidgetHeight + 1,
        VSCROLL_SCROLLBAR_W - 2, _h - 2 * scrollBoxWidgetHeight - 1,
        scrollbarBg);
  }

  drawBorder(lcd);

  uint16_t scrollbarColor = _focused ? invertColor(_border_color) : _border_color;

  // Vertical bars for sides of scrollbar.
  lcd.drawFastVLine(scrollbarX, _y, _h, scrollbarColor);
  lcd.drawFastVLine(_x + _w - 1, _y, _h, scrollbarColor);
  // 4 horizontal bars at top and bottom for outline of the ^ and v boxes
  lcd.drawFastHLine(scrollbarX, _y, VSCROLL_SCROLLBAR_W, scrollbarColor);
  lcd.drawFastHLine(scrollbarX, _y + scrollBoxWidgetHeight, VSCROLL_SCROLLBAR_W, scrollbarColor);
  lcd.drawFastHLine(scrollbarX, _y + _h - 1 - scrollBoxWidgetHeight, VSCROLL_SCROLLBAR_W, scrollbarColor);
  lcd.drawFastHLine(scrollbarX, _y + _h - 1, VSCROLL_SCROLLBAR_W, scrollbarColor);

  // Draw carets

  renderScrollUp(lcd, false);
  renderScrollDown(lcd, false);

  // Draw the scroll position indicator.
  // Use MacOS-style fixed-size box whose position is proportional to the position of the viewing
  // window. The viewing window is at the "bottom" is when the last screenful of rows are shown,
  // so remove that many items from _elements.size() when calculating this percentage.
  float frac = min(1.0, (float)_topIdx / max(1, (signed)_entries.size() - (_h / _itemHeight)));
  int16_t boxStart = frac * (_h - 3 * scrollBoxWidgetHeight);
  lcd.fillRect(scrollbarX, _y + scrollBoxWidgetHeight + boxStart,
      VSCROLL_SCROLLBAR_W, scrollBoxWidgetHeight, scrollbarColor);
}

void VScroll::renderContentArea(TFT_eSPI &lcd) {
  // Iterate through all the entries and render them.
  for (unsigned int i = _topIdx; i < _lastIdx; i++) {
    UIWidget *pEntry = _entries[i];
    if (pEntry != NULL) {
      pEntry->render(lcd);
    }
  }
}

void VScroll::renderScrollUp(TFT_eSPI &lcd, bool btnActive) {
  // X position of the left-most edge of the scrollbar.
  int16_t scrollbarX = _x + _w - VSCROLL_SCROLLBAR_W;

  if (_scrollbar_bg_color != TRANSPARENT_COLOR) {
    // Fill in the background of the scrollbar area under the caret.
    // Since the edges will be taken up completely with the horiz and vertical borders of
    // the scrollbar, do not include them in the fill.
    uint16_t scrollbarBg = _focused ? invertColor(_scrollbar_bg_color) : _scrollbar_bg_color;
    lcd.fillRect(scrollbarX + 1, _y + 1, VSCROLL_SCROLLBAR_W - 2, scrollBoxWidgetHeight - 1,
        scrollbarBg);
  }

  uint16_t scrollbarColor = _focused ? invertColor(_border_color) : _border_color;

  // Upward facing ^ for scroll-up, at the top.
  if (btnActive) {
    lcd.fillTriangle(scrollbarX + 2, _y + scrollBoxWidgetHeight - 2,
                     _x + _w - 3,    _y + scrollBoxWidgetHeight - 2,
                     scrollbarX + (VSCROLL_SCROLLBAR_W / 2), _y + 2,
                     scrollbarColor);
  } else {
    lcd.drawTriangle(scrollbarX + 2, _y + scrollBoxWidgetHeight - 2,
                     _x + _w - 3,    _y + scrollBoxWidgetHeight - 2,
                     scrollbarX + (VSCROLL_SCROLLBAR_W / 2), _y + 2,
                     scrollbarColor);
  }
}
void VScroll::renderScrollDown(TFT_eSPI &lcd, bool btnActive) {
  // X position of the left-most edge of the scrollbar.
  int16_t scrollbarX = _x + _w - VSCROLL_SCROLLBAR_W;

  if (_scrollbar_bg_color != TRANSPARENT_COLOR) {
    // Fill in the background of the scrollbar area under the caret.
    // Since the edges will be taken up completely with the horiz and vertical borders of
    // the scrollbar, do not include them in the fill.
    uint16_t scrollbarBg = _focused ? invertColor(_scrollbar_bg_color) : _scrollbar_bg_color;
    lcd.fillRect(scrollbarX + 1, _y + _h - scrollBoxWidgetHeight, VSCROLL_SCROLLBAR_W - 2,
        scrollBoxWidgetHeight - 1, scrollbarBg);
  }

  uint16_t scrollbarColor = _focused ? invertColor(_border_color) : _border_color;

  // Downward facing v for scroll-down, at the bottom.
  if (btnActive) {
    lcd.fillTriangle(scrollbarX + 2, _y + _h - scrollBoxWidgetHeight + 2,
                     _x + _w - 3,    _y + _h - scrollBoxWidgetHeight + 2,
                     scrollbarX + (VSCROLL_SCROLLBAR_W / 2), _y + _h - 3,
                     scrollbarColor);
  } else {
    lcd.drawTriangle(scrollbarX + 2, _y + _h - scrollBoxWidgetHeight + 2,
                     _x + _w - 3,    _y + _h - scrollBoxWidgetHeight + 2,
                     scrollbarX + (VSCROLL_SCROLLBAR_W / 2), _y + _h - 3,
                     scrollbarColor);
  }
}


bool VScroll::redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags) {
  if (NULL == widget) {
    return false;
  } else if (this == widget) {
    bool partialRedraw = false;

    if ((renderFlags & RF_VSCROLL_CONTENT) == RF_VSCROLL_CONTENT) {

      partialRedraw = true;
    }

    if ((renderFlags & RF_VSCROLL_SCROLLBAR) == RF_VSCROLL_SCROLLBAR) {
      partialRedraw = true;
    }

    if (!partialRedraw) {
      // If we weren't requested to render specific components of this widget, redraw all of it.
      render(lcd, renderFlags);
    }

    return true;
  } else if (containsWidget(widget)) {
    // This widget thinks its inside our bounding box. It's definitely a child of this
    // widget, but depending on where we're scrolled to, this may not actually be visible.

    // The containment algorithm is flaky in that containsWidget() will have already returned true
    // before we enter this method. Meaning our parent/ancestor widget(s) may have called
    // drawBackgroundUnderWidget() and made a rectangle in our draw area. If we then called
    // pEntry->redrawChildWidget() for the appropriate slice of our view, there's a possibility
    // that it wouldn't draw (because the invalidated widget thinks its in a space on our screen
    // but is actually scrolled out of view, and a different widget is actually in that spot).
    // So we do a full re-render of the pEntry with the bounding box that contains 'widget'.

    // Iterate through all the entries and check.
    for (unsigned int i = _topIdx; i < _lastIdx; i++) {
      UIWidget *pEntry = _entries[i];
      if (pEntry != NULL && pEntry->containsWidget(widget)) {
        // Found a visible row that applies to this widget.
        if ((renderFlags & RF_NO_BACKGROUNDS) == 0 && _content_bg_color != TRANSPARENT_COLOR) {
          // Instead of the standard drawBackground(), we fill the content area
          // with the content-area-specific background color.
          int16_t cx, cy, cw, ch;
          pEntry->getRect(cx, cy, cw, ch);
          lcd.fillRect(cx, cy, cw, ch, _content_bg_color);
        }
        pEntry->render(lcd);
        return true;
      }
    }
  }

  return false;
}

void VScroll::cascadeBoundingBox() {
  // Recompute bounding boxes for visible items.
  int16_t childX, childY, childW, childH;
  getChildAreaBoundingBox(childX, childY, childW, childH);
  // Because the scrollbar is on the right and clobbers the border there, we disregard childW
  // and recalculate it ourselves.
  childW = _w;
  // Calculate width adjustment for left-side border.
  if (_border_flags & BORDER_LEFT) {
    childW -= BORDER_ACTIVE_INNER_MARGIN;
  } else if (_border_flags & BORDER_ROUNDED) {
    childW -= BORDER_ROUNDED_INNER_MARGIN;
  }
  childW -= _paddingL + _paddingR; // Adjust width to compensate for user-specified padding.

  // Adjust width to provide room for the scrollbar.
  childW -= VSCROLL_SCROLLBAR_W + VSCROLL_SCROLLBAR_MARGIN;

  unsigned int idx = 0;
  for (auto it = _entries.begin(); it < _entries.end() && childH > 0; idx++, it++) {
    if (idx < _topIdx) {
      // Not part of the visible set of entries; just skip it.
      continue;
    }

    UIWidget *pEntry = *it;
    if (pEntry == NULL) {
      // Empty entry; takes zero height.
      continue;
    }

    pEntry->setBoundingBox(childX, childY, childW, min(_itemHeight, childH));
    childY += _itemHeight; // Next item is further down by 1 item height.
    childH -= _itemHeight; // Next item's available height reduced by 1 item height.
  }

  _lastIdx = idx;
}

int16_t VScroll::getContentWidth(TFT_eSPI &lcd) const {
  return _w; // We always flex to the width of our container.
}

int16_t VScroll::getContentHeight(TFT_eSPI &lcd) const {
  return _h; // We always flex to the height of our container.
}

void VScroll::setItemHeight(int16_t newItemHeight) {
  _itemHeight = newItemHeight;
  if (_itemHeight < 0) {
    _itemHeight = DEFAULT_VSCROLL_ITEM_HEIGHT;
  }

  cascadeBoundingBox();
}

bool VScroll::scrollUp() {
  if (_topIdx > 0) {
    _topIdx--;
    cascadeBoundingBox();
    return true;
  }

  return false;
}

// specify the idx of the elem to show @ the top of the scroll box.
bool VScroll::scrollTo(unsigned int idx) {
  if (idx < _entries.size()) {
    _topIdx = idx;
    cascadeBoundingBox();
    return true;
  }

  return false;
}

bool VScroll::scrollDown() {
  if (_topIdx >= _entries.size() - 1) {
    // Hard limit; cannot scroll past final element in vector.
    return false;
  } else if (_lastIdx >= _entries.size()) {
    // Should not let user scroll down past the last "full page".
    return false;
  }

  // We can continue to scroll.
  _topIdx++;
  cascadeBoundingBox();
  return true;
}
