// (c) Copyright 2022 Aaron Kimball

#ifndef _UIW_VSCROLL_H
#define _UIW_VSCROLL_H

#include "../collections/collections.h"
#include "screen.h"

constexpr int16_t VSCROLL_SCROLLBAR_W = 12; // width of the rendered scrollbar itself
constexpr int16_t VSCROLL_SCROLLBAR_MARGIN = 2; // px between content and scrollbar

constexpr int16_t DEFAULT_VSCROLL_ITEM_HEIGHT = 16;

// constant signaling that no element of a VScroll is selected.
constexpr size_t NO_SELECTION = 0xFFFFFFFF;

// Render flags specific to the VScroll widget.
constexpr uint32_t RF_VSCROLL_CONTENT   = 0x10000 | RF_WIDGET_SPECIFIC | RF_NO_BACKGROUNDS;
constexpr uint32_t RF_VSCROLL_SCROLLBAR = 0x20000 | RF_WIDGET_SPECIFIC | RF_NO_BACKGROUNDS;
constexpr uint32_t RF_VSCROLL_SELECTED  = 0x40000 | RF_WIDGET_SPECIFIC | RF_NO_BACKGROUNDS;

/**
 * A container for a variable number of entries, displayed with a scrollbar.
 *
 * This widget disregards the standard `background` color (from `setBackground()`).
 * Use `setContentBackground()` and `setScrollbarBackground()` to specify background
 * colors for different segments of the widget.
 *
 * This widget does not work especially well with the BORDER_ROUNDED border style, as
 * the scrollbar expects to encroach on a rectilinear border (if specified).
 */
class VScroll : public UIWidget {
public:
  VScroll(): UIWidget(), _entries(),
      _topIdx(0), _lastIdx(0), _selectIdx(NO_SELECTION), _priorSelectIdx(NO_SELECTION),
      _itemHeight(DEFAULT_VSCROLL_ITEM_HEIGHT),
      _scrollbar_bg_color(TFT_BLACK),
      _content_bg_color(TRANSPARENT_COLOR) {};

  // Add the specified widget to the end of the list.
  void add(UIWidget *widget) { _entries.push_back(widget); cascadeBoundingBox(); };
  // Remove the specified widget from the list.
  void remove(UIWidget *widget);
  // Remove all widgets from the list.
  void clear() {
    _entries.clear();
    _selectIdx = NO_SELECTION;
    _priorSelectIdx = NO_SELECTION;
    cascadeBoundingBox();
  };

  // Return number of entries in the list.
  size_t count() const { return _entries.size(); };
  size_t position() const { return _topIdx; }; // idx of the elem @ the top of the viewport
  size_t bottomIdx() const { return _lastIdx; }; // idx of the elem @ the bottom of the viewport.

  virtual void render(TFT_eSPI &lcd, uint32_t renderFlags);
  // Render the up-facing scrollbar chevron
  void renderScrollUp(TFT_eSPI &lcd, bool btnActive, uint32_t renderFlags=0);
  // Render the down-facing scrollbar chevron
  void renderScrollDown(TFT_eSPI &lcd, bool btnActive, uint32_t renderFlags=0);
  virtual void cascadeBoundingBox();
  virtual int16_t getContentWidth(TFT_eSPI &lcd) const;
  virtual int16_t getContentHeight(TFT_eSPI &lcd) const;

  bool scrollUp(); // scroll 1 element higher.
  bool scrollTo(size_t idx); // specify the idx of the elem to show @ the top of the scroll box.
  bool scrollDown(); // scroll 1 element lower.

  bool setSelection(size_t idx); // specify the idx of an elem to select.
  bool selectUp(); // select the element 1 above the current one.
  bool selectDown(); // select the element 1 lower than the current one.
  size_t selectIdx() const { return _selectIdx; }; // return idx of selected element.
  UIWidget* getSelected() const; // Return the selected elem (or NULL if none).

  // Specify height available to each entry to render within.
  void setItemHeight(int16_t newItemHeight);
  int16_t getItemHeight() const { return _itemHeight; };

  virtual bool redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags=0);

  void setContentBackground(uint16_t color) { _content_bg_color = color; };
  void setScrollbarBackground(uint16_t color) { _scrollbar_bg_color = color; };

protected:
  void _renderScrollbar(TFT_eSPI &lcd, uint32_t renderFlags);
  void _renderContentArea(TFT_eSPI &lcd, uint32_t renderFlags);

private:
  bool _setSelection(size_t idx);

  tc::vector<UIWidget*> _entries;

  size_t _topIdx; // Index of the first element to display.
  size_t _lastIdx; // Index of the last visible element.
  size_t _selectIdx; // Index of a selected element, if any (or NO_SELECTION otherwise).
  size_t _priorSelectIdx; // Index of previously-selected element, if any (or NO_SELECTION).
                          // Tracked so we can re-render this element w/o focus when we re-render
                          // the newly-focused _selectIdx entry.

  int16_t _itemHeight; // Fixed height for all elements.
  uint16_t _scrollbar_bg_color;
  uint16_t _content_bg_color;
};


#endif // _UIW_VSCROLL_H
