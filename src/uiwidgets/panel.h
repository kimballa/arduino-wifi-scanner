// (c) Copyright 2022 Aaron Kimball

#ifndef __UIW_PANEL_H
#define __UIW_PANEL_H

/**
 * A basic "box" that holds another widget. No specific rendering by default,
 * although it can control background & border that fill under/around the child widget.
 */
class Panel : public UIWidget {
public:
  Panel(): UIWidget(), _child(NULL) {};

  void setChild(UIWidget *widget) { _child = widget; };

  virtual void render(TFT_eSPI &lcd, uint32_t renderFlags);
  virtual void cascadeBoundingBox();
  virtual int16_t getContentWidth(TFT_eSPI &lcd) const;
  virtual int16_t getContentHeight(TFT_eSPI &lcd) const;
  virtual bool redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags=0);

private:
  UIWidget *_child;
};

#endif // __UIW_PANEL_H
