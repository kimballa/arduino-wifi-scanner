// (c) Copyright 2022 Aaron Kimball

#ifndef __UIW_SCREEN_H
#define __UIW_SCREEN_H


class UIWidget {
public:
  UIWidget(): _x(0), _y(0), _w(0), _h(0),
      _border_flags(BORDER_NONE), _border_color(TFT_BLACK), _bg_color(BG_NONE) {
  };

  /** Render the widget to the screen, along with any child widgets. */
  virtual void render(TFT_eSPI &lcd) = 0;

  /** Set the bounding box for this widget. */
  void setBoundingBox(int16_t x, int16_t y, int16_t w, int16_t h);

  /** Called by setBoundingBox(); cascades bounding box requirements to any child elements. */
  virtual void cascadeBoundingBox() { };

  void setBorder(const border_flags_t flags, uint16_t color);
  /**
   * A color to fill in for the background, or BG_NONE for no background (i.e., inherit bg from
   * container). */
  void setBackground(uint16_t color);

protected:
  void drawBorder(TFT_eSPI &lcd);
  void drawBackground(TFT_eSPI &lcd);
  /** Get area bounding box available for rendering within the context of any border or other
   * padding that belongs to this widget.
   */
  void getChildAreaBoundingBox(int16_t &childX, int16_t &childY, int16_t &childW, int16_t &childH);

  int16_t _x, _y;
  int16_t _w, _h;

  border_flags_t _border_flags;
  uint16_t _border_color;
  uint16_t _bg_color;
};

class Screen {
public:
  Screen(TFT_eSPI &lcd): _lcd(lcd), _bgColor(TFT_BLACK), _widget(NULL) {};

  void setWidget(UIWidget *w) { _widget = w; };
  UIWidget *getWidget() const { return _widget; };

  void render();

  int16_t getWidth() const { return _lcd.width(); };
  int16_t getHeight() const { return _lcd.height(); };

  void setBackground(uint16_t bgColor) { _bgColor = bgColor; };

private:
  TFT_eSPI &_lcd;
  UIWidget *_widget; // The top-most widget for rendering the screen.

  uint16_t _bgColor;
};

class Panel : public UIWidget {
public:
  Panel(): UIWidget(), _child(NULL) {};

  void setChild(UIWidget *widget) { _child = widget; };

  virtual void render(TFT_eSPI &lcd);
  virtual void cascadeBoundingBox();

private:
  UIWidget *_child;
};

#endif // __UIW_SCREEN_H
