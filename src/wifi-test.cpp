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
String msg1("First Row label.");
String msg2("Second Row label.");
String msg3("And the mightly THIRD row label.");
StrLabel label1(msg1.c_str());
StrLabel label2(msg2.c_str());
StrLabel label3(msg3.c_str());
Rows rows(3);

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
    rows.setRow(0, &label1, 200);
    rows.setRow(1, &label2, EQUAL);
    rows.setRow(2, &label3, EQUAL);
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
  screen.render();
  delay(1000);
}
