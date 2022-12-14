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
String msg1("First Col label.");
String msg2("Second Col label.");
String msg3("And the mightly THIRD row label.");
IntLabel label1(211);
IntLabel label2(312);
FloatLabel label3(3.14159);
Rows rows(3);
Cols midCols(3);

void setup() {
  DBGSETUP();

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextFont(2); // 0 for 8px, 2 for 16px.
  tft.setTextColor(TFT_RED);
  tft.drawString("This is a test", 0, 0);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  DBGPRINT("Setup done");

  screen.setWidget(&rows);
  rows.setRow(1, &midCols, EQUAL);
  rows.setFixedHeight(EQUAL);

  midCols.setColumn(0, &label1, 50);
  midCols.setColumn(1, &label2, EQUAL);
  midCols.setColumn(2, &label3, EQUAL);

  label1.setBorder(BORDER_BOTTOM | BORDER_RIGHT);

  label2.setColor(TFT_BLACK);
  label2.setBackground(TFT_GREEN);
  label2.setBorder(BORDER_ROUNDED, TFT_RED);

  label3.setBorder(BORDER_RECT);

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

}
