// (c) Copyright 2022 Aaron Kimball

#ifndef _HEATMAP_H
#define _HEATMAP_H

#include "uiwidgets/uiwidgets.h"

class Heatmap : public UIWidget {
public:
  Heatmap(): UIWidget(), channels(), rssiLevels(), _color(TFT_RED) { };

  virtual void render(TFT_eSPI &lcd);
  virtual int16_t getContentWidth(TFT_eSPI &lcd) const;
  virtual int16_t getContentHeight(TFT_eSPI &lcd) const;
  virtual bool redrawChildWidget(UIWidget *widget, TFT_eSPI &lcd, uint32_t renderFlags=0) {
    return widget == this ? render(lcd), true : false;
  };

  // Define a wifi channel id in the spectrum
  void defineChannel(int channelNum);
  // Add the signal strength for a signal heard on the specified channel.
  void addSignal(int channelNum, int rssi);
  // Discard existing signal data.
  void clear() { channels.clear(); rssiLevels.clear(); };

  void setColor(uint16_t color) { _color = color; };


private:
  unsigned int idxForChannelNum(int channelNum) const;

  vector<int> channels;
  vector<vector<int>> rssiLevels;

  uint16_t _color;
};

constexpr unsigned int CHANNEL_NOT_FOUND = 0xFFFFFFFF;

#endif
