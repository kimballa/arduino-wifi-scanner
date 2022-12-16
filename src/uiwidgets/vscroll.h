// (c) Copyright 2022 Aaron Kimball

#ifndef _UIW_VSCROLL_H
#define _UIW_VSCROLL_H

#include<vector>

using std::vector;

constexpr int16_t VSCROLL_SCROLLBAR_W = 12; // width of the rendered scrollbar itself
constexpr int16_t VSCROLL_SCROLLBAR_MARGIN = 2; // px between content and scrollbar

constexpr int16_t DEFAULT_VSCROLL_ITEM_HEIGHT = 16;

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
  VScroll(): UIWidget(), _entries(), _itemHeight(DEFAULT_VSCROLL_ITEM_HEIGHT),
      _topIdx(0), _lastIdx(0), _scrollbar_bg_color(TFT_BLACK),
      _content_bg_color(TRANSPARENT_COLOR) {};

  // Add the specified widget to the end of the list.
  void add(UIWidget *widget) { _entries.push_back(widget); cascadeBoundingBox(); };
  // Remove the specified widget from the list.
  void remove(UIWidget *widget);
  void clear() { _entries.clear(); cascadeBoundingBox(); }; // Remove all widgets from the list.

  // Return number of entries in the list.
  unsigned int count() const { return _entries.size(); };
  unsigned int position() const { return _topIdx; }; // idx of the elem @ the top of the viewport
  unsigned int bottomIdx() const { return _lastIdx; }; // idx of the elem @ the bottom of the viewport.

  virtual void render(TFT_eSPI &lcd) { render(lcd, RF_NONE); };
  void render(TFT_eSPI &lcd, uint32_t renderFlags);
  void renderScrollUp(TFT_eSPI &lcd, bool btnActive);
  void renderScrollDown(TFT_eSPI &lcd, bool btnActive);
  virtual void cascadeBoundingBox();
  virtual int16_t getContentWidth(TFT_eSPI &lcd) const;
  virtual int16_t getContentHeight(TFT_eSPI &lcd) const;

  bool scrollUp(); // scroll 1 element higher.
  bool scrollTo(unsigned int idx); // specify the idx of the elem to show @ the top of the scroll box.
  bool scrollDown(); // scroll 1 element lower.

  // Specify height available to each entry to render within.
  void setItemHeight(int16_t newItemHeight);
  int16_t getItemHeight() const { return _itemHeight; };

  virtual bool redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags=0);

  void setContentBackground(uint16_t color) { _content_bg_color = color; };
  void setScrollbarBackground(uint16_t color) { _scrollbar_bg_color = color; };

protected:
  void renderScrollbar(TFT_eSPI &lcd);
  void renderContentArea(TFT_eSPI &lcd);

private:
  vector<UIWidget*> _entries;
  int16_t _itemHeight; // Fixed height for all elements.
  unsigned int _topIdx; // Index of the first element to display.
  unsigned int _lastIdx; // Index of the last visible element.

  uint16_t _scrollbar_bg_color;
  uint16_t _content_bg_color;
};


#endif // _UIW_VSCROLL_H
