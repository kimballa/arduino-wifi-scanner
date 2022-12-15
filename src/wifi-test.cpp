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

TFT_eSPI tft;

Screen screen(tft);
// The screen is a set of rows: row of buttons, then the main vscroll.
Rows rowLayout(2); // 2 rows.
// the top row is a set of buttons.
Cols topRow(4); // 4 columns
const char *detailsStr = "Details";
const char *rescanStr = "Refresh";
const char *fooStr = "Foo!";
Button detailsButton(detailsStr);
Button rescanButton(rescanStr);
Button fooButton(fooStr);
// The main focus of the screen is a scrollable list of wifi SSIDs.
VScroll wifiListScroll;


const char *testStr = "Testing";
StrLabel testLabel(testStr);

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
  rowLayout.setRow(1, &wifiListScroll, EQUAL); // VScroll expands to fill available space.

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


  wifiListScroll.add(&testLabel);

  screen.render();
}

void wifiLoop() {
  DBGPRINT("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  DBGPRINT("scan done");
  if (n == 0) {
    DBGPRINT("no networks found");
  } else {
    DBGPRINTI("networks found:", n);
    for (int i = 0; i < n; i++) {
      // Print SSID and RSSI for each network found
      DBGPRINTI("#", i + 1);
      DBGPRINT(WiFi.SSID(i));
      DBGPRINT(WiFi.RSSI(i));
      delay(10);
    }
  }
  DBGPRINT("");

  // Wait a bit before scanning again
  delay(5000);
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
