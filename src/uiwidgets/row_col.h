// (c) Copyright 2022 Aaron Kimball

#ifndef __UIW_ROW_COL_H
#define __UIW_ROW_COL_H

#include "screen.h"

class Rows: public UIWidget {
public:
  Rows(uint16_t numRows);
  ~Rows();

  virtual void render(TFT_eSPI &lcd);
  virtual void cascadeBoundingBox();

  void setNumRows(uint16_t numRows); // update number of rows. Properties like fixed height do not carry
                                     // over to any new rows.
  uint16_t getNumRows() const { return _numRows; };
  void setRow(uint16_t offset, UIWidget *widget, int16_t height);
  void setFixedHeight(int16_t height); // a height applied to each row.
  // Row height in pixels, or EQUAL to evenly distribute rows, or FLEX to use content-sensitive
  // sizing.
  void setRowHeight(uint16_t offset, int16_t height);
  int16_t getRowHeight(uint16_t offset);

private:
  uint16_t _numRows;
  UIWidget **_elements; // one for each row.
  int16_t *_heights; // one for each row.
};

class Cols: public UIWidget {
public:
  Cols(uint16_t numCols);
  ~Cols();

  virtual void render(TFT_eSPI &lcd);
  virtual void cascadeBoundingBox();

  void setNumCols(uint16_t numCols); // update number of cols. Properties like fixed height do not carry
                                     // over to any new cols.
  uint16_t getNumCols() const { return _numCols; };
  void setColumn(uint16_t offset, UIWidget *widget, int16_t width);
  void setFixedWidth(int16_t width); // a width applied to each column.
  // Col width in pixels, or EQUAL to evenly distribute columns, or FLEX to use content-sensitive
  // sizing.
  void setColumnWidth(uint16_t offset, int16_t width);
  int16_t getColumnWidth(uint16_t offset);

private:
  uint16_t _numCols;
  UIWidget **_elements; // one for each col.
  int16_t *_widths; // one for each col
};

#endif
