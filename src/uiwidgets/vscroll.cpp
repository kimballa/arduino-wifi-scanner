// (c) Copyright 2022 Aaron Kimball

#include "uiwidgets.h"

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

void VScroll::render(TFT_eSPI &lcd) {
  drawBackground(lcd);
  drawBorder(lcd);


  uint16_t scrollbarColor = _focused ? invertColor(_border_color) : _border_color;
  // X position of the left-most edge of the scrollbar.
  int16_t scrollbarX = _x + _w - VSCROLL_SCROLLBAR_W;

  // Vertical bars for sides of scrollbar.
  lcd.drawFastVLine(scrollbarX, _y, _h, scrollbarColor);
  lcd.drawFastVLine(_x + _w - 1, _y, _h, scrollbarColor);
  // 4 horizontal bars at top and bottom for outline of the ^ and v boxes
  constexpr int16_t scrollBoxWidgetHeight = 12; // Those boxes are 12 px tall.
  lcd.drawFastHLine(scrollbarX, _y, VSCROLL_SCROLLBAR_W, scrollbarColor);
  lcd.drawFastHLine(scrollbarX, _y + scrollBoxWidgetHeight, VSCROLL_SCROLLBAR_W, scrollbarColor);
  lcd.drawFastHLine(scrollbarX, _y + _h - 1 - scrollBoxWidgetHeight, VSCROLL_SCROLLBAR_W, scrollbarColor);
  lcd.drawFastHLine(scrollbarX, _y + _h - 1, VSCROLL_SCROLLBAR_W, scrollbarColor);

  // Draw carets

  // Upward facing ^ for scroll-up, at the top.
  lcd.drawTriangle(scrollbarX + 2, _y + scrollBoxWidgetHeight - 2,
                   _x + _w - 3,    _y + scrollBoxWidgetHeight - 2,
                   scrollbarX + (VSCROLL_SCROLLBAR_W / 2), _y + 2,
                   scrollbarColor);

  // Downward facing v for scroll-down, at the bottom.
  lcd.drawTriangle(scrollbarX + 2, _y + _h - scrollBoxWidgetHeight + 2,
                   _x + _w - 3,    _y + _h - scrollBoxWidgetHeight + 2,
                   scrollbarX + (VSCROLL_SCROLLBAR_W / 2), _y + _h - 3,
                   scrollbarColor);

  // Draw the scroll position indicator.
  // Use MacOS-style fixed-size box whose position is proportional to the position of the viewing
  // window. The viewing window is at the "bottom" is when the last screenful of rows are shown,
  // so remove that many items from _elements.size() when calculating this percentage.
  float frac = (float)_topIdx / max(1, (signed)_entries.size() - (_h / _itemHeight));
  int16_t boxStart = frac * (_h - 3 * scrollBoxWidgetHeight);
  lcd.fillRect(scrollbarX, _y + scrollBoxWidgetHeight + boxStart,
      VSCROLL_SCROLLBAR_W, scrollBoxWidgetHeight, scrollbarColor);

  // Now we can iterate through all the entries and render them.
  for (unsigned int i = _topIdx; i < _lastIdx; i++) {
    UIWidget *pEntry = _entries[i];
    if (pEntry != NULL) {
      pEntry->render(lcd);
    }
  }
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

void VScroll::scrollUp() {
  if (_topIdx > 0) {
    _topIdx--;
    cascadeBoundingBox();
  }
}

// specify the idx of the elem to show @ the top of the scroll box.
void VScroll::scrollTo(unsigned int idx) {
  if (idx < _entries.size()) {
    _topIdx = idx;
    cascadeBoundingBox();
  }
}

void VScroll::scrollDown() {
  if (_topIdx < _entries.size() - 1) {
    _topIdx++;
    cascadeBoundingBox();
  }
}

bool VScroll::redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd) {
  if (NULL == widget) {
    return false;
  } else if (this == widget) {
    render(lcd);
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
        drawBackgroundUnderWidget(widget, lcd);
        pEntry->render(lcd);
        return true;
      }
    }
  }

  return false;
}