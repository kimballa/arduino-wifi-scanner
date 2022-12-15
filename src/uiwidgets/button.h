// (c) Copyright 2022 Aaron Kimball

#ifndef _IUW_BUTTON_H
#define _IUW_BUTTON_H

// A "clickable" / selectable button.
class Button : public UIWidget {
public:
  Button(): UIWidget(), _fontId(0), _color(TFT_WHITE), _btnLabel(NULL) {};

  void setText(const char *str) { _btnLabel = str; };
  const char *getText() const { return _btnLabel; };

  virtual void render(TFT_eSPI &lcd);

  virtual int16_t getContentWidth(TFT_eSPI &lcd) const;
  virtual int16_t getContentHeight(TFT_eSPI &lcd) const;

  // TODO(aaron): Handle FreeFont fonts too.
  void setFont(int fontId) { _fontId = fontId; };
  void setColor(uint16_t color) { _color = color; };
  // TODO(aaron): Implement font size multiplier and set textsize.

protected:
  int _fontId;
  uint16_t _color;
  const char *_btnLabel;

};

#endif // _IUW_BUTTON_H
