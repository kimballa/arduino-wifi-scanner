// (c) Copyright 2022 Aaron Kimball

#ifndef __UIW_LABELS_H
#define __UIW_LABELS_H

class Label: public UIWidget {
public:
  Label(): UIWidget(), _fontId(0), _color(TFT_WHITE) { };

  // Sets up the font, color, bg, etc., and then defers to renderText() to text-ify a particular
  // piece of data on this surface with these settings.
  virtual void render(TFT_eSPI &lcd);

  // Subclasses: Implement how to draw the text representation of the label's reference data.
  virtual void renderText(TFT_eSPI &lcd) = 0;

  // TODO(aaron): Handle FreeFont fonts too.
  void setFont(int fontId) { _fontId = fontId; };
  void setColor(uint16_t color) { _color = color; };

  // TODO(aaron): Handle center and right justification via setTextDatum().
  // TODO(aaron): Implement font size multiplier and set textsize.

protected:
  int _fontId;
  uint16_t _color;
};

class StrLabel: public Label {
public:
  StrLabel(): Label(), _str() { };

  virtual void renderText(TFT_eSPI &lcd);

  void setText(char *str) { _str = str; };
  char* getText() const { return _str; };

private:
  char *_str;
};

class IntLabel: public Label {
public:
  IntLabel(): Label(), _val(0) { };

  virtual void renderText(TFT_eSPI &lcd);

  void setValue(long val) { _val = val; };
  long getValue() const { return _val; };

private:
  long _val;
};


class FloatLabel: public Label {
public:
  FloatLabel(): Label(), _val(0.0f), _maxDecimalDigits(7) { };

  virtual void renderText(TFT_eSPI &lcd);

  void setValue(float val) { _val = val; };
  float getValue() const { return _val; };

  void setMaxDecimalDigits(uint8_t digits);
  uint8_t getMaxDecimalDigits() const { return _maxDecimalDigits; };

private:
  float _val;
  uint8_t _maxDecimalDigits;
};



#endif // __UIW_LABELS_H
