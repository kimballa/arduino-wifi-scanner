// (c) Copyright 2022 Aaron Kimball

#ifndef __UIW_SCREEN_H
#define __UIW_SCREEN_H

/**
 * A Screen is the top-level UIWidgets container. This is not itself a UIWidget;
 * it holds a UIWidget (likely a Panel, Rows, or Cols) to be drawn.
 *
 * You can have several Screens that represent different menus, displays, etc. but
 * only one Screen is shown at a time.
 */
class Screen {
public:
  Screen(TFT_eSPI &lcd): _lcd(lcd), _widget(NULL), _bgColor(TFT_BLACK) {};

  void setWidget(UIWidget *w);
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

#endif // __UIW_SCREEN_H
