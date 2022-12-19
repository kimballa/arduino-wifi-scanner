// (c) Copyright 2022 Aaron Kimball

#ifndef _WIFI_SCANNER_H
#define _WIFI_SCANNER_H

// C/C++ includes
#include<cstring>

// Arduino itself
#include<Arduino.h>

// Libraries
#include<rpcWiFi.h> // Seeed rpcWiFi
#include<rtl_wifi/wifi_constants.h>
#include<esp/esp_wifi_types.h> // Seeed rpcUnified
#include<TFT_eSPI.h> // Seeed LCD
#include<debounce.h>
#include <tiny-collections.h>
#include <uiwidgets.h>

#define DEBUG
#define DBG_PRETTY_FUNCTIONS
//#define DBG_WAIT_FOR_CONNECT
//#define DBG_START_PAUSED
#include<dbg.h>

#include "heatmap.h"

// Copies the specified text (up to 80 chars) into the status line buffer
// and renders it to the bottom of the screen. If immediateRedraw=false,
// the visible widget will not be updated until your next screen.render() call.
extern void setStatusLine(const char *in, bool immediateRedraw=true);

#endif
