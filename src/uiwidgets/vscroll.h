// (c) Copyright 2022 Aaron Kimball

#ifndef _UIW_VSCROLL_H
#define _UIW_VSCROLL_H

#include<vector>

using std::vector;

constexpr int16_t VSCROLL_SCROLLBAR_W = 12; // width of the rendered scrollbar itself
constexpr int16_t VSCROLL_SCROLLBAR_MARGIN = 2; // px between content and scrollbar

constexpr int16_t DEFAULT_VSCROLL_ITEM_HEIGHT = 16;

/** A container for a variable number of entries, displayed with a scrollbar. */
class VScroll : public UIWidget {
public:
  VScroll(): UIWidget(), _entries(), _itemHeight(DEFAULT_VSCROLL_ITEM_HEIGHT),
      _topIdx(0), _lastIdx(0) {};

  // Add the specified widget to the end of the list.
  void add(UIWidget *widget) { _entries.push_back(widget); cascadeBoundingBox(); };
  // Remove the specified widget from the list.
  void remove(UIWidget *widget);
  void clear() { _entries.clear(); cascadeBoundingBox(); }; // Remove all widgets from the list.

  // Return number of entries in the list.
  unsigned int getCount() const { return _entries.size(); };

  virtual void render(TFT_eSPI &lcd);
  virtual void cascadeBoundingBox();
  virtual int16_t getContentWidth(TFT_eSPI &lcd) const;
  virtual int16_t getContentHeight(TFT_eSPI &lcd) const;

  void scrollUp(); // scroll 1 element higher.
  void scrollTo(unsigned int idx); // specify the idx of the elem to show @ the top of the scroll box.
  void scrollDown(); // scroll 1 element lower.

  // Specify height available to each entry to render within.
  void setItemHeight(int16_t newItemHeight);
  int16_t getItemHeight() const { return _itemHeight; };

private:
  vector<UIWidget*> _entries;
  int16_t _itemHeight; // Fixed height for all elements.
  unsigned int _topIdx; // Index of the first element to display.
  unsigned int _lastIdx; // Index of the last visible element.
};


#endif // _UIW_VSCROLL_H
