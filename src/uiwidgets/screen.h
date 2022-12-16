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

  // Render the entire screen.
  void render();

  // Re-render one widget whose view is invalidated (along with any backgrounds, etc.
  // underneath it).
  void renderWidget(UIWidget *widget, uint32_t renderFlags=0);

  int16_t getWidth() const { return _lcd.width(); };
  int16_t getHeight() const { return _lcd.height(); };

  void setBackground(uint16_t bgColor) { _bgColor = bgColor; };

private:
  TFT_eSPI &_lcd;
  UIWidget *_widget; // The top-most widget for rendering the screen.

  uint16_t _bgColor;
};

// Flags that can be passed to renderWidget().

constexpr uint32_t RF_NONE              =     0x0;
constexpr uint32_t RF_NO_BACKGROUNDS    =     0x1; // Ignore underlying background fields.

constexpr uint32_t RF_WIDGET_SPECIFIC   =  0x1000; // Indicates widget-specific interpretations
                                                   // for flags masked by FFFF0000.

// Render flags specific to the VScroll widget.
constexpr uint32_t RF_VSCROLL_CONTENT   = 0x10000 | RF_WIDGET_SPECIFIC | RF_NO_BACKGROUNDS;
constexpr uint32_t RF_VSCROLL_SCROLLBAR = 0x20000 | RF_WIDGET_SPECIFIC | RF_NO_BACKGROUNDS;



#endif // __UIW_SCREEN_H
