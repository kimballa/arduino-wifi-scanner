// (c) Copyright 2022 Aaron Kimball
//
// Test the wifi functionality.

#include<Arduino.h>
#include<rpcWiFi.h>
#include<TFT_eSPI.h>

#define DEBUG
#define DBG_PRETTY_FUNCTIONS
//#define DBG_WAIT_FOR_CONNECT
//#define DBG_START_PAUSED
#include<dbg.h>

TFT_eSPI tft;

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
}

void loop() {
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



