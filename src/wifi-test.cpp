// (c) Copyright 2022 Aaron Kimball
//
// Test the wifi functionality.

#include<Arduino.h>
#include<rpcWiFi.h>
#include<TFT_eSPI.h>
#include<debounce.h>

#include "uiwidgets/uiwidgets.h"

#define DEBUG
#define DBG_PRETTY_FUNCTIONS
//#define DBG_WAIT_FOR_CONNECT
//#define DBG_START_PAUSED
#include<dbg.h>

static void scanWifi();

TFT_eSPI tft;

static constexpr int16_t SSID_WIDTH = 120;
static constexpr int16_t CHAN_WIDTH = 40;
static constexpr int16_t RSSI_WIDTH = 40;

Screen screen(tft);
// The screen is a set of rows: row of buttons, then the main vscroll.
static Rows rowLayout(3); // 3 rows.
// the top row is a set of buttons.
static Cols topRow(4); // 4 columns
static const char *detailsStr = "Details";
static const char *rescanStr = "Refresh";
static const char *fooStr = "Foo!";
static UIButton detailsButton(detailsStr);
static UIButton rescanButton(rescanStr);
static UIButton fooButton(fooStr);
// The main focus of the screen is a scrollable list of wifi SSIDs.
static VScroll wifiListScroll;
static Panel wifiScrollContainer; // Wrap VScroll in a container for padding.

static const char *hdrSsidStr = "SSID";
static const char *hdrChannelStr = "Chan";
static const char *hdrRssiStr = "RSSI";

static StrLabel hdrSsid = StrLabel(hdrSsidStr);
static StrLabel hdrChannel = StrLabel(hdrChannelStr);
static StrLabel hdrRssi = StrLabel(hdrRssiStr);
static Cols dataHeaderRow(3); // 3 columns.

// physical pushbuttons
static vector<Button> buttons;
static vector<uint8_t> buttonGpioPins;

// 5-way hat "up" -- scroll up the list.
static void scrollUpHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) { return; };

  wifiListScroll.scrollUp();
  screen.renderWidget(&wifiListScroll);
}

// 5-way hat "down" -- scroll down the list.
static void scrollDownHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) { return; };

  wifiListScroll.scrollDown();
  screen.renderWidget(&wifiListScroll);
}

void setup() {
  DBGSETUP();
  //while (!Serial) { delay(10); }

  // Register the pins for the 5-way hat button.
  buttonGpioPins.push_back(WIO_5S_UP);    // Button 0
  buttonGpioPins.push_back(WIO_5S_DOWN);  // Button 1
  buttonGpioPins.push_back(WIO_5S_LEFT);  // Button 2
  buttonGpioPins.push_back(WIO_5S_RIGHT); // Button 3
  buttonGpioPins.push_back(WIO_5S_PRESS); // Button 4
  // Add the 3 top-side buttons.
  buttonGpioPins.push_back(WIO_KEY_A); // Button 5
  buttonGpioPins.push_back(WIO_KEY_B); // Button 6
  buttonGpioPins.push_back(WIO_KEY_C); // Button 7

  for (auto pin: buttonGpioPins) {
    pinMode(pin, INPUT_PULLUP);
  }

  buttons.emplace_back(0, scrollUpHandler);
  buttons.emplace_back(1, scrollDownHandler);
  // TODO(aaron): Add Button / handlers for btns 2--7.

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
  dataHeaderRow.setColumn(1, &hdrChannel, CHAN_WIDTH);
  dataHeaderRow.setColumn(2, &hdrRssi, RSSI_WIDTH);
  dataHeaderRow.setBackground(TFT_BLUE);

  hdrSsid.setBackground(TFT_BLUE);
  hdrSsid.setColor(TFT_WHITE);
  hdrSsid.setPadding(2, 0, 2, 0);

  hdrRssi.setBackground(TFT_BLUE);
  hdrRssi.setColor(TFT_WHITE);
  hdrRssi.setPadding(0, 0, 2, 0);

  hdrChannel.setBackground(TFT_BLUE);
  hdrChannel.setColor(TFT_WHITE);
  hdrChannel.setPadding(0, 0, 2, 0);

  scanWifi();
  screen.render();
}

static vector<String> ssids;
static vector<StrLabel *> ssidLabels;
static vector<IntLabel *> chanLabels;
static vector<IntLabel *> rssiLabels;
static vector<Cols *> wifiRows; // Each row is a Cols for (ssid, rssi)

static void makeWifiRow(int wifiIdx) {
  ssids.push_back(WiFi.SSID(wifiIdx));
  StrLabel *ssid = new StrLabel(ssids[wifiIdx].c_str());
  ssid->setFont(2);
  ssidLabels.push_back(ssid);

  IntLabel *chan = new IntLabel(WiFi.channel(wifiIdx));
  chanLabels.push_back(chan);

  IntLabel *rssi = new IntLabel(WiFi.RSSI(wifiIdx));
  rssiLabels.push_back(rssi);

  Cols *wifiRow = new Cols(3);
  wifiRow->setColumn(0, ssid, SSID_WIDTH);
  wifiRow->setColumn(1, chan, CHAN_WIDTH);
  wifiRow->setColumn(2, rssi, RSSI_WIDTH);
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


static void pollButtons() {
  for (unsigned int i = 0; i < buttons.size(); i++) {
    buttons[i].update(digitalRead(buttonGpioPins[i]));
  }
}

void loop() {
  pollButtons();
  delay(10);
}
