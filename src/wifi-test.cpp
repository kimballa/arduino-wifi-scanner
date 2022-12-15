// (c) Copyright 2022 Aaron Kimball
//
// Test the wifi functionality.

#include<Arduino.h>
#include<rpcWiFi.h>
#include<TFT_eSPI.h>
#include "uiwidgets/uiwidgets.h"

#define DEBUG
#define DBG_PRETTY_FUNCTIONS
//#define DBG_WAIT_FOR_CONNECT
//#define DBG_START_PAUSED
#include<dbg.h>

static void scanWifi();

TFT_eSPI tft;

constexpr int16_t SSID_WIDTH = 120;
constexpr int16_t RSSI_WIDTH = 40;
constexpr int16_t CHAN_WIDTH = 40;

Screen screen(tft);
// The screen is a set of rows: row of buttons, then the main vscroll.
Rows rowLayout(3); // 3 rows.
// the top row is a set of buttons.
Cols topRow(4); // 4 columns
const char *detailsStr = "Details";
const char *rescanStr = "Refresh";
const char *fooStr = "Foo!";
UIButton detailsButton(detailsStr);
UIButton rescanButton(rescanStr);
UIButton fooButton(fooStr);
// The main focus of the screen is a scrollable list of wifi SSIDs.
VScroll wifiListScroll;
Panel wifiScrollContainer; // Wrap VScroll in a container for padding.

const char *hdrSsidStr = "SSID";
const char *hdrRssiStr = "RSSI";
const char *hdrChannelStr = "Chan";

StrLabel hdrSsid = StrLabel(hdrSsidStr);
StrLabel hdrRssi = StrLabel(hdrRssiStr);
StrLabel hdrChannel = StrLabel(hdrChannelStr);
Cols dataHeaderRow(3); // 3 columns.

void setup() {
  DBGSETUP();
  //while (!Serial) { delay(10); }

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2); // 0 for 8px, 2 for 16px.
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Wifi analyzer starting up...", 0, 0);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  screen.setWidget(&rowLayout);
  rowLayout.setRow(0, &topRow, 30);
  rowLayout.setRow(1, &dataHeaderRow, 16);
  rowLayout.setRow(2, &wifiScrollContainer, EQUAL); // VScroll expands to fill available space.

  wifiScrollContainer.setChild(&wifiListScroll);
  wifiScrollContainer.setPadding(2, 2, 1, 1); // Add 1px padding around wifi list VScroll.

  // topRow is a Cols that holds some buttons.
  topRow.setBorder(BORDER_BOTTOM, TFT_BLUE);
  topRow.setBackground(TFT_LIGHTGREY);
  topRow.setPadding(0, 0, 4, 2); // 4 px padding top; 2 px on bottom b/c we get padding from border.
  topRow.setColumn(0, &detailsButton, 70);
  topRow.setColumn(1, &rescanButton, 70);
  topRow.setColumn(2, &fooButton, 50);
  topRow.setColumn(3, NULL, EQUAL); // Rest of space to the right is empty

  detailsButton.setColor(TFT_BLUE);
  detailsButton.setPadding(4, 4, 0, 0);
  detailsButton.setFocus(true);

  rescanButton.setColor(TFT_BLUE);
  rescanButton.setPadding(4, 4, 0, 0);

  fooButton.setColor(TFT_BLUE);
  fooButton.setPadding(4, 4, 0, 0);

  dataHeaderRow.setColumn(0, &hdrSsid, SSID_WIDTH);
  dataHeaderRow.setColumn(1, &hdrRssi, RSSI_WIDTH);
  dataHeaderRow.setColumn(2, &hdrChannel, CHAN_WIDTH);
  dataHeaderRow.setBackground(TFT_BLUE);

  hdrSsid.setBackground(TFT_BLUE);
  hdrSsid.setColor(TFT_WHITE);

  hdrRssi.setBackground(TFT_BLUE);
  hdrRssi.setColor(TFT_WHITE);

  hdrChannel.setBackground(TFT_BLUE);
  hdrChannel.setColor(TFT_WHITE);

  scanWifi();
  screen.render();
}

vector<String> ssids;
vector<StrLabel *> ssidLabels;
vector<IntLabel *> rssiLabels;
vector<IntLabel *> chanLabels;
vector<Cols *> wifiRows; // Each row is a Cols for (ssid, rssi)

static void makeWifiRow(int wifiIdx) {
  ssids.push_back(WiFi.SSID(wifiIdx));
  StrLabel *ssid = new StrLabel(ssids[wifiIdx].c_str());
  ssid->setFont(2);
  ssidLabels.push_back(ssid);

  IntLabel *rssi = new IntLabel(WiFi.RSSI(wifiIdx));
  rssiLabels.push_back(rssi);

  IntLabel *chan = new IntLabel(WiFi.channel(wifiIdx));
  chanLabels.push_back(chan);

  Cols *wifiRow = new Cols(3);
  wifiRow->setColumn(0, ssid, SSID_WIDTH);
  wifiRow->setColumn(1, rssi, RSSI_WIDTH);
  wifiRow->setColumn(2, chan, CHAN_WIDTH);
  wifiRows.push_back(wifiRow);

  wifiListScroll.add(wifiRow);
}


static void scanWifi() {
  DBGPRINT("scan start");
  // TODO(aaron): If this has already been called, free all the existing used memory.

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  ssids.reserve(n);
  ssidLabels.reserve(n);
  rssiLabels.reserve(n);
  wifiRows.reserve(n);

  DBGPRINT("scan done");
  if (n == 0) {
    DBGPRINT("no networks found");
  } else {
    for (int i = 0; i < n; i++) {
      makeWifiRow(i);
    }
  }
}



void loop() {
  delay(1000);
  /*
  DBGPRINT("x --- width");
  DBGPRINT(label1.getX());
  DBGPRINTI("      ", label1.getWidth());
  DBGPRINT(label2.getX());
  DBGPRINTI("      ", label2.getWidth());
  DBGPRINT(label3.getX());
  DBGPRINTI("      ", label3.getWidth());
  */

  //DBGPRINTI("vscroll h", vscroll.getHeight());
}
