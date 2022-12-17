// (c) Copyright 2022 Aaron Kimball
//
// Implement a visual heatmap / histograms that visualizes signal strenths
// observed on a wifi channel.

#include "wifi-scanner.h"

void Heatmap::defineChannel(int channelNum) {
  _channels.push_back(channelNum); // idx 'n' points to channel # 'channelNum'.
  _rssiLevels.push_back(tc::vector<int>()); // Add a vector to hold its rssi levels.
}

void Heatmap::addSignal(int channelNum, int rssi) {
  unsigned int channelIdx = idxForChannelNum(channelNum);
  if (CHANNEL_NOT_FOUND == channelIdx) {
    // Disregard this signal; invalid channel.
    return;
  }

  _rssiLevels[channelIdx].push_back(rssi);
}


unsigned int Heatmap::idxForChannelNum(int channelNum) const {
  for (unsigned int i = 0; i < _channels.size(); i++) {
    if (_channels[i] == channelNum) {
      return i;
    }
  }

  return CHANNEL_NOT_FOUND; // Couldn't find it.
}

void Heatmap::render(TFT_eSPI &lcd) {
  drawBackground(lcd);
  drawBorder(lcd);

  // Establish our available canvas space inside of padding, etc.
  int16_t childX, childY, childW, childH;
  getChildAreaBoundingBox(childX, childY, childW, childH);

  constexpr int xAxisHeight = 12; // 12 px reserved for X axis.
  constexpr int maxBlockHeightLimit = 16; // blocks are variable height, but no taller than 16 px.

  int maxColWidth = childW / _channels.size(); // width per col + associated padding
  constexpr int colPad = 2; // 2 px padding between columns.
  int colWidth = max(maxColWidth - colPad, 1);

  // Which channel has the most signals?
  unsigned int maxSignals = 0;
  for (auto rssiLevelsForChannel: _rssiLevels) {
    maxSignals = max(maxSignals, rssiLevelsForChannel.size());
  }

  // Height per block (+ padding) within col:
  int maxBlockHeight = min(maxBlockHeightLimit, (childH - xAxisHeight) / maxSignals);
  constexpr int blockPad = 1; // 1 px padding between blocks in a columnar stack.
  int blockHeight = max(maxBlockHeight - blockPad, 1);

  // Draw a line below the heatmap area to separate x axis labels from heatmap blocks.
  lcd.drawFastHLine(childX, childY + childH - xAxisHeight, childW, TFT_WHITE);

  // Loop through all the channels; draw blocks indicating all the received signals on that
  // channel, in a vertical stack; tint them based on RSSI value.
  int cursorX = childX;
  int cursorY;
  lcd.setTextColor(TFT_WHITE);
  for (unsigned int chanIdx = 0; chanIdx < _channels.size(); chanIdx++) {
    const tc::vector<int> &rssiLevelsForChannel = _rssiLevels[chanIdx];

    // Our chart starts at the bottom of our Y-axis space and grows upward.
    cursorY = childY + childH - xAxisHeight - blockHeight - 1;
    for (auto rssi : rssiLevelsForChannel) {
      // TODO: Adjust color based on RSSI value.
      lcd.fillRect(cursorX, cursorY, colWidth, blockHeight, _color);
      cursorY -= blockHeight + blockPad;
    }

    // Draw the category label below heat blocks (font 0 for small size)
    lcd.drawNumber(_channels[chanIdx], cursorX, childY + childH - xAxisHeight + 2, 0);

    // Advance cursor to the right for the next column
    cursorX += colWidth + colPad;
  }
}

int16_t Heatmap::getContentWidth(TFT_eSPI &lcd) const {
  int16_t cx, cy, cw, ch;
  getChildAreaBoundingBox(cx, cy, cw, ch);
  return cw;
}

int16_t Heatmap::getContentHeight(TFT_eSPI &lcd) const {
  int16_t cx, cy, cw, ch;
  getChildAreaBoundingBox(cx, cy, cw, ch);
  return ch;
}

