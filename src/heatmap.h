// (c) Copyright 2022 Aaron Kimball

#ifndef _HEATMAP_H
#define _HEATMAP_H

#include "uiwidgets/uiwidgets.h"
#include "collections/collections.h"

class Heatmap : public UIWidget {
public:
  Heatmap(): UIWidget(), _channels(), _rssiLevels(), _color(TFT_RED) { };

  virtual void render(TFT_eSPI &lcd, uint32_t renderFlags);
  virtual int16_t getContentWidth(TFT_eSPI &lcd) const;
  virtual int16_t getContentHeight(TFT_eSPI &lcd) const;
  virtual bool redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags=0) {
    return widget == this ? render(lcd, renderFlags), true : false;
  };

  // Define a wifi channel id in the spectrum
  void defineChannel(int channelNum);
  // Add the signal strength for a signal heard on the specified channel.
  void addSignal(int channelNum, int rssi);
  // Discard existing signal data.
  void clear() { _channels.clear(); _rssiLevels.clear(); };

  void setColor(uint16_t color) { _color = color; };

  // Return the next (higher) channel number in the band plan above `channelNum` or
  // NO_CHANNEL if none is found. (i.e., channelNum is the highest in the band plan.)
  int channelNumAbove(int channelNum) const;
  // Return the previous (lower) channel number in the band plan below `channelNum` or
  // NO_CHANNEL if none is found. (i.e., channelNum is the lowest in the band plan.)
  int channelNumBelow(int channelNum) const;

private:
  size_t _idxForChannelNum(int channelNum) const;

  tc::vector<int> _channels;
  tc::vector<tc::vector<int>> _rssiLevels;

  uint16_t _color;
};

constexpr unsigned int CHANNEL_NOT_FOUND = 0xFFFFFFFF;
constexpr int NO_CHANNEL = static_cast<int>(CHANNEL_NOT_FOUND);

#endif
