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
  StrLabel(const char *text=NULL): Label(), _str(text) { };

  virtual void renderText(TFT_eSPI &lcd);

  void setText(const char *str) { _str = str; };
  const char* getText() const { return _str; };

  virtual int16_t getContentWidth(TFT_eSPI &lcd) const;
  virtual int16_t getContentHeight(TFT_eSPI &lcd) const;

private:
  const char *_str;
};

class IntLabel: public Label {
public:
  IntLabel(long x=0): Label(), _val(x) { };

  virtual void renderText(TFT_eSPI &lcd);

  void setValue(long val) { _val = val; };
  long getValue() const { return _val; };

  virtual int16_t getContentWidth(TFT_eSPI &lcd) const;
  virtual int16_t getContentHeight(TFT_eSPI &lcd) const;

private:
  long _val;
};


class FloatLabel: public Label {
public:
  FloatLabel(float f=0.0f, uint8_t d=7): Label(), _val(f), _maxDecimalDigits(d) { };

  virtual void renderText(TFT_eSPI &lcd);

  void setValue(float val) { _val = val; };
  float getValue() const { return _val; };

  void setMaxDecimalDigits(uint8_t digits);
  uint8_t getMaxDecimalDigits() const { return _maxDecimalDigits; };

  virtual int16_t getContentWidth(TFT_eSPI &lcd) const;
  virtual int16_t getContentHeight(TFT_eSPI &lcd) const;

private:
  float _val;
  uint8_t _maxDecimalDigits;
};



#endif // __UIW_LABELS_H
