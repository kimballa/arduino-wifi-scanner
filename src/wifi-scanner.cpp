// (c) Copyright 2022 Aaron Kimball
//
// Measure wifi spectrum usage

#include "wifi-scanner.h"

// fwd declarations.
static void scanWifi();
static void displayDetails(size_t wifiIdx);
static void populateStationDetails(size_t wifiIdx);
static void disableStation(size_t wifiIdx);
static void enableStation(size_t wifiIdx);
static void populateHeatmapChannelPlan(Heatmap *heatmap, const tc::const_array<int> &channelPlan);
static void recordSignalHeatmap(const wifi_ap_record_t *pWifiAPRecord, Heatmap *bandHeatmap);

// Button handler functions.
static void stationDetailsHandler(uint8_t btnId, uint8_t btnState);
static void refreshHandler(uint8_t btnId, uint8_t btnState);
static void toggleHeatmapButtonHandler(uint8_t btnId, uint8_t btnState);
static void backToStationListHandler(uint8_t btnId, uint8_t btnState);
static void scrollUpHandler(uint8_t btnId, uint8_t btnState);
static void scrollDownHandler(uint8_t btnId, uint8_t btnState);
static void enableStationHandler(uint8_t btnId, uint8_t btnState);
static void disableStationHandler(uint8_t btnId, uint8_t btnState);


////////    GUI widgets and layout   ////////

TFT_eSPI lcd;
Screen screen(lcd);

// Buffer for status message at bottom of display.
constexpr unsigned int MAX_STATUS_LINE_LEN = 80; // max len in chars; buffer is +1 more for '\0'
static char statusLine[MAX_STATUS_LINE_LEN + 1];

// Widths for columns in main display
static constexpr int16_t SSID_WIDTH = 140;
static constexpr int16_t CHAN_WIDTH = 30;
static constexpr int16_t RSSI_WIDTH = 30;
static constexpr int16_t BSSID_WIDTH = 80;

// The screen is a set of rows: row of buttons, then the main vscroll (or heatmap or details).
static Rows rowLayout(4); // 3 rows.
// the top row (#0) is a set of buttons.
static Cols topRow(4); // 4 columns
static const char detailsStr[] = "Details";
static const char rescanStr[] = "Refresh";
static const char heatmapStr[] = "Heatmap";
static const char backStr[] = "Back";
static UIButton detailsButton(detailsStr);
static UIButton rescanButton(rescanStr);
static UIButton heatmapButton(heatmapStr);
static UIButton heatmapBackButton(backStr);

// Row 1: Column headers for the main data table.
static const char *hdrSsidStr = "SSID";
static const char *hdrChannelStr = "Chan";
static const char *hdrRssiStr = "RSSI";
static const char *hdrBssidStr = "BSSID";

static StrLabel hdrSsid = StrLabel(hdrSsidStr);
static StrLabel hdrChannel = StrLabel(hdrChannelStr);
static StrLabel hdrRssi = StrLabel(hdrRssiStr);
static StrLabel hdrBssid = StrLabel(hdrBssidStr);
static Cols dataHeaderRow(4); // 4 columns.

// Row 2: The main focus of the screen:
// * Starts as a VScroll list of wifi SSIDs.
// * May also be a Panel with details for the selected SSID.
// * May also be a Heatmap with viz of current wifi band congestion. (x2 for 2.4 and 5 GHz)
static VScroll wifiListScroll;
static Panel wifiScrollContainer; // Wrap VScroll in a container for padding.

static Heatmap wifi24GHzHeatmap; // Heatmap of congestion on 2.4 GHz channels
static Heatmap wifi50GHzHeatmap; // Heatmap of congestion on 5 GHz channels

static Panel detailsPanel;
/**
 * Details panel contains detailsRows, which has the following layout
 *
 * 0 StationInfoCols:: ChanHdr:  Chan  (empty) RssiHdr: Rssi
 * 1 SsidCols::        SsidHdr:  SSID
 * 2 BssidCols::       BssidHdr: BSSID
 * 3 ModeBwCols::      Modes (empty) Bandwidth
 * 4 SecurityCols::    SecurityHdr: Security
 * 5 (empty row; EQUAL space to bottom-justify detailsHeatmap)
 * 6 (HeatmapHdr)
 * 7 (detailsHeatmap)
 */
static StrLabel detailsChanHdr(hdrChannelStr);
static IntLabel detailsChan;
static StrLabel detailsRssiHdr(hdrRssiStr);
static IntLabel detailsRssi;

static StrLabel detailsSsidHdr(hdrSsidStr);
static StrLabel detailsSsid; // holds actual SSID of selected station.

static StrLabel detailsBssidHdr(hdrBssidStr);
static StrLabel detailsBssid; // holds actual BSSID str of selected station.

static constexpr size_t MODES_TEXT_LEN = 16;
static char detailsModesText[MODES_TEXT_LEN]; // Long enough for "802.11: b, g, n"
static StrLabel detailsModes(detailsModesText); // holds 'b/g/n' flags.
static constexpr size_t BANDWIDTH_TEXT_LEN = 8;
static char detailsBandwidthText[BANDWIDTH_TEXT_LEN]; // Long enough for "20 MHz"
static StrLabel detailsBandwidth(detailsBandwidthText); // holds 20/40 MHz indicator.

static const char hdrDetailsSecurityStr[] = "Security:";
static StrLabel detailsSecurityHdr(hdrDetailsSecurityStr);
static const char SECURITY_OPEN[] = "Open";
static const char SECURITY_WEP[] = "WEP";
static const char SECURITY_WPA_PSK[] = "WPA PSK-Personal";
static const char SECURITY_WPA2_PSK[] = "WPA2 PSK-Personal";
static const char SECURITY_WPA_WPA2_PSK[] = "WPA/WPA2 PSK-Personal";
static const char SECURITY_WPA2_ENTERPRISE[] = "WPA2-Enterprise";
static const char SECURITY_UNKNOWN[] = "(Unknown)";
static StrLabel detailsSecurity(SECURITY_UNKNOWN);

static const char hdrDetailsHeatmapStr[] = "Channel spectrum interference:";
static StrLabel detailsHeatmapHdr(hdrDetailsHeatmapStr);

static const char disableStr[] = "Disable"; // Button text to disable this SSID in heatmap analysis.
static const char enableStr[] = "Enable"; // Button text to enable this SSID in heatmap analysis.
static UIButton detailsBackBtn(backStr);
static UIButton detailsDisableBtn(disableStr);
static Heatmap detailsHeatmap;
static Rows detailsRows(8);
static Cols detailsStationInfoCols(5);
static Cols detailsSsidCols(2);
static Cols detailsBssidCols(2);
static Cols detailsModeBwCols(3);
static Cols detailsSecurityCols(2);


// Enumerate all main-area content panels the user can cycle through.
constexpr unsigned int ContentCarousel_SignalList = 0;  // Show a list of wifi SSIDs
constexpr unsigned int ContentCarousel_Heatmap24 = 1;   // Show a heatmap of 2.4 GHz channel usage
constexpr unsigned int ContentCarousel_Heatmap50 = 2;   // Show a heatmap of 5 GHz channel usage
constexpr unsigned int MaxContentCarousel = ContentCarousel_Heatmap50;
constexpr unsigned int ContentCarousel_Details = 3; // Show details of a given ssid.
// (Note that detailsPanel isn't accessed through the 'cycle carousel' button, it's activated
// by pressing the 5-way hat "in" button on a selectable line of the VScroll. Thus, MaxCC is
// one below that.)

static unsigned int carouselPos = ContentCarousel_SignalList;

// Row 3: Status / msg line.
static StrLabel statusLineLabel = StrLabel(statusLine);


////////   physical pushbutton I/O   ////////

static tc::vector<Button> buttons;
static tc::vector<uint8_t> buttonGpioPins;

static constexpr uint8_t HAT_UP_DEBOUNCE_ID = 0;
static constexpr uint8_t HAT_DOWN_DEBOUNCE_ID = 1;
static constexpr uint8_t HAT_IN_DEBOUNCE_ID = 4; // debounce id for 5-way hat "IN" / "OK"
// Btn 1 is the debouncer id for WIO_KEY_C (left-most button on top):
static constexpr uint8_t TOP_BUTTON_1_DEBOUNCE_ID = 5;
static constexpr uint8_t TOP_BUTTON_2_DEBOUNCE_ID = 6;
static constexpr uint8_t TOP_BUTTON_3_DEBOUNCE_ID = 7; // handler for WIO_KEY_A (right-most).

// Set the button to display in the upper left (Either "Details" or "Back"), and assign
// the appropriate physical button handler function for it.
static inline void setButton1(UIButton *uiButton, buttonHandler_t handlerFn) {
  buttons[TOP_BUTTON_1_DEBOUNCE_ID].setHandler(handlerFn);
  topRow.setColumn(0, uiButton, 70);
}

// Middle button configuration.
static inline void setButton2(UIButton *uiButton, buttonHandler_t handlerFn) {
  buttons[TOP_BUTTON_2_DEBOUNCE_ID].setHandler(handlerFn);
  topRow.setColumn(1, uiButton, 70);
}

// Right button configuration
static inline void setButton3(UIButton *uiButton, buttonHandler_t handlerFn) {
  buttons[TOP_BUTTON_3_DEBOUNCE_ID].setHandler(handlerFn);
  topRow.setColumn(2, uiButton, 75);
}

////////   US FCC 802.11 channel band plan   ////////

// Arrays that define the channel numbers in each frequency range
static constexpr tc::const_array<int> wifi24GHzChannelPlan = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
static constexpr tc::const_array<int> wifi50GHzChannelPlan = {
  32, 36, 40, 44, 48,        // U-NII-1 channels, unrestricted
  // 52, 56, 60, 64, 68, 96, // U-NII-1 and 2A, use DFS or below 500mW
  // 100, 104, 108, 112,
  // 116, 120, 124, 128,
  // 132, 136, 140, 144,     // U-NII-2C and 3; use DFS or below 500mW
  149, 153, 157, 161, 165,   // U-NII-3 channels,  unrestricted
  // 169, 173, 177,             // U-NII-4 1W, indoor usage only (since 2020)
};

////////   802.11 bandwidth spectral masks   ////////

/**
 * The 802.11b spectral mask specifies that the signal overlaps adjacent channels
 * in the following way:
 * at +/- 5 MHz (1 channel away): 0 dBm signal diff
 * at +/-10 MHz (2 channels out): 0 dBm
 * at +/-15 MHz (3 channels out): -30 dBm
 * at +/-20 MHz (4 channels out): -30 dBm
 * Further out is -50 dBm, effectively no interference.
 * (See https://www.rfcafe.com/references/electrical/wlan-masks.htm)
 *
 * The spectral mask definitions here are one-sided, but are applied symmetrically around
 * the primary (center) channel.
 */
static tc::vector<int> spectralMask80211B = { 0, 0, -30, -30 };

// 802.11a and g have the following mask, along with 20 MHz 802.11n:
// +/-  5 MHz:     0 dBm
// +/- 10 MHz:   -10 dBm
// +/- 15 MHz:   -26 dBm (actually -25.9 dBm)
// +/- 20 MHz:   -28 dBm
// Further out is -35 dBm or lower; non-interfering.
static tc::vector<int> spectralMask80211G = { 0, -10, -26, -28 };

// 802.11n in 40 MHz mode on 2.4 GHz wifi blocks an enormous number of channels:
// +/-  5 MHz:    0 dBm
// +/- 10 MHz:    0 dBm
// +/- 15 MHz:    0 dBm
// +/- 20 MHz:  -10 dBm
// +/- 25 MHz:  -22 dBm
// +/- 30 MHz:  -25 dBm
// +/- 35 MHz:  -27 dBm
// +/- 40 MHz:  -30 dBm
//
// This mask array reflects that for a 40 MHz bandwidth true channel, the primary channel num
// is at -10 MHz and secondary at +10 MHz (or vice versa).
//
// The center of the
// true channel is offset from the reported primary channel and we need to look at the data
// struct to see whether the true center is above or below the primary channel id.
//
// So at +15 MHz from the true channel, we are at +5 MHz in the array below (  0 dBm)
//    at +20 MHz from the true channel, we are at +10 MHz in array below    (-10 dBm)
//    ... and so on...
//
// See https://www.researchgate.net/figure/80211-spectral-masks_fig3_261382549
static tc::vector<int> spectralMask80211N40MHz24G = { 0, -10, -22, -25, -27, -30 };

// 802.11n in 40 MHz mode on 5 GHz wifi (20 MHz channel spacing):
// TODO(aaron): Adjust for offset true channel center.
static tc::vector<int> spectralMask80211N40MHz50G = { 0, -10, -30 };

static tc::vector<int> emptySpectralMask = {};

// Ignore signals with power < -90 dBm when constructing the interference heatmap.
static constexpr int noiseFloorDBm = -90;

// Max 2.4 GHz channel id in US band plan is 11.
static constexpr int max24GHzChannelNum = 11;

/** Return the heatmap associated with a particular channel. */
static Heatmap *getHeatmapForChannel(int chan) {
  // TODO(aaron): This will always return a heatmap, including for invalid channels. OK?
  if (chan <= max24GHzChannelNum) {
    return &wifi24GHzHeatmap;
  } else {
    return &wifi50GHzHeatmap;
  }
}

////////    bitfield for suppression of stations in interference chart    ////////

// A bitfield of at least 60 bits where a 1 bit at position i indicates that wifi_idx 'i'
// is a *disabled* SSID that should be ignored in heatmaps.
static uint32_t stationDisabledBits[2] = {0, 0};

static bool isStationDisabled(size_t wifiIdx) {
  size_t arrayOffset = wifiIdx / 32;
  size_t bitPosition = wifiIdx % 32;

  return (stationDisabledBits[arrayOffset] & (1 << bitPosition)) != 0;
}

// Update the bitfield for a particular wifiIdx's disabled status.
static void setStationDisabledBit(size_t wifiIdx, bool disabled) {
  size_t arrayOffset = wifiIdx / 32;
  size_t bitPosition = wifiIdx % 32;

  if (disabled) {
    stationDisabledBits[arrayOffset] |= (1 << bitPosition);
  } else {
    stationDisabledBits[arrayOffset] &= ~(1 << bitPosition);
  }
}

void clearDisabledStations() {
  stationDisabledBits[0] = 0;
  stationDisabledBits[1] = 0;
}


////////    Transitions between different main content area states    ////////

// Set the main display area content to be the station list.
void displayStationList() {
  carouselPos = ContentCarousel_SignalList;

  rowLayout.setRow(1, &dataHeaderRow, 16);          // Enable header row.
  rowLayout.setRow(2, &wifiScrollContainer, EQUAL); // VScroll expands to fill available space.
  setButton1(&detailsButton, stationDetailsHandler);
  setButton2(&rescanButton, refreshHandler);
  setButton3(&heatmapButton, toggleHeatmapButtonHandler);
  heatmapButton.setText(heatmapStr);
  buttons[HAT_IN_DEBOUNCE_ID].setHandler(stationDetailsHandler); // hat-in enabled.
  buttons[HAT_UP_DEBOUNCE_ID].setHandler(scrollUpHandler); // hat scrolling enabled.
  buttons[HAT_DOWN_DEBOUNCE_ID].setHandler(scrollDownHandler);
  carouselPos = ContentCarousel_SignalList;
  setStatusLine("");
}

void displayHeatmap24GHz() {
  carouselPos = ContentCarousel_Heatmap24;

  rowLayout.setRow(1, NULL, 0); // Blank out header row above vscroll.
  rowLayout.setRow(2, &wifi24GHzHeatmap, EQUAL); // Put in the 2.4 GHz spectrum heatmap
  setStatusLine("2.4 GHz spectrum congestion");
  setButton1(NULL, emptyBtnHandler); // disable 'details' btn.
  setButton2(&rescanButton, refreshHandler);
  setButton3(&heatmapButton, toggleHeatmapButtonHandler);
  heatmapButton.setText(heatmapStr);
  buttons[HAT_IN_DEBOUNCE_ID].setHandler(emptyBtnHandler); // hat-in disabled.
  buttons[HAT_UP_DEBOUNCE_ID].setHandler(emptyBtnHandler); // hat scrolling disabled.
  buttons[HAT_DOWN_DEBOUNCE_ID].setHandler(emptyBtnHandler);
}

void displayHeatmap50GHz() {
  carouselPos = ContentCarousel_Heatmap50;

  rowLayout.setRow(1, NULL, 0); // Blank out header row above vscroll.
  rowLayout.setRow(2, &wifi50GHzHeatmap, EQUAL); // Put in the 5 GHz spectrum heatmap
  setStatusLine("5 GHz spectrum congestion");
  setButton1(NULL, emptyBtnHandler); // disable 'details' btn.
  setButton2(&rescanButton, refreshHandler);
  setButton3(&heatmapButton, toggleHeatmapButtonHandler);
  // heatmapButton, when pressed again, goes back to station list.
  heatmapButton.setText(backStr);
  buttons[HAT_IN_DEBOUNCE_ID].setHandler(emptyBtnHandler); // hat-in disabled.
  buttons[HAT_UP_DEBOUNCE_ID].setHandler(emptyBtnHandler); // hat scrolling disabled.
  buttons[HAT_DOWN_DEBOUNCE_ID].setHandler(emptyBtnHandler);
}

// Set main display to be the Details page for a particular wifi station idx.
void displayDetails(size_t wifiIdx) {
  carouselPos = ContentCarousel_Details;

  // populate all the widgets on the details page with data about this station
  populateStationDetails(wifiIdx);

  // We've filled out all the new data. Present it to the user.
  rowLayout.setRow(1, NULL, 0); // Hide VScroll header row, if any.
  rowLayout.setRow(2, &detailsPanel, EQUAL); // Show the detailsPanel; use all vertical space.

  // Change 'Details' button to 'Back' button.
  setButton1(&detailsBackBtn, backToStationListHandler);

  // Second button position is either 'enable' or 'disable' depending on whether this
  // station is currently disabled or enabled, respectively.
  if (isStationDisabled(wifiIdx)) {
    setButton2(&detailsDisableBtn, enableStationHandler);
    detailsDisableBtn.setText(enableStr);
  } else {
    setButton2(&detailsDisableBtn, disableStationHandler);
    detailsDisableBtn.setText(disableStr);
  }

  setButton3(NULL, emptyBtnHandler);
  buttons[HAT_IN_DEBOUNCE_ID].setHandler(emptyBtnHandler); // hat-in disabled.
  buttons[HAT_UP_DEBOUNCE_ID].setHandler(scrollUpHandler); // hat scrolling enabled.
  buttons[HAT_DOWN_DEBOUNCE_ID].setHandler(scrollDownHandler);

  carouselPos = ContentCarousel_Details;
}

// Adjust the main display area content
void rotateContentCarousel() {
  carouselPos++;
  if (carouselPos > MaxContentCarousel) {
    carouselPos = 0;
  }

  switch (carouselPos) {
  case ContentCarousel_SignalList:
    displayStationList();
    break;
  case ContentCarousel_Heatmap24:
    displayHeatmap24GHz();
    break;
  case ContentCarousel_Heatmap50:
    displayHeatmap50GHz();
    break;
  default:
    // We are not in the ring carousel of pages so cannot go to the 'next' one. Probably
    // because we are on the details page. Go back to the main station list.
    displayStationList();
    break;
  }
}

////////    Update status line    ////////

// Copies the specified text (up to 80 chars) into the status line buffer
// and renders it to the bottom of the screen. If immediateRedraw=false,
// the visible widget will not be updated until your next screen.render() call.
void setStatusLine(const char *in, bool immediateRedraw) {
  if (NULL == in) {
    statusLine[0] = '\0';
    return;
  }

  strncpy(statusLine, in, MAX_STATUS_LINE_LEN);
  statusLine[MAX_STATUS_LINE_LEN] = '\0'; // Ensure null term if we copied 80 printable chars.
  if (immediateRedraw) {
    screen.renderWidget(&statusLineLabel);
  }
}


////////    Enable and disable stations from inclusion in interference heatmap    ////////

char disableMessage[MAX_STATUS_LINE_LEN + 1];

// Set the disabled bit to 'true' for wifiIdx and all other stations with the same SSID.
// Recompute the heatmap without the disabled stations.
static void disableStation(size_t wifiIdx) {
  const char *disableSSID;
  const wifi_ap_record_t *pWifiAPRecord =
      reinterpret_cast<const wifi_ap_record_t*>(WiFi.getScanInfoByIndex(wifiIdx));
  disableSSID = reinterpret_cast<const char*>(pWifiAPRecord->ssid);

  for (size_t i = 0; i < SCAN_MAX_NUMBER; i++) {
    pWifiAPRecord = reinterpret_cast<const wifi_ap_record_t*>(WiFi.getScanInfoByIndex(i));
    if (strcmp(reinterpret_cast<const char*>(pWifiAPRecord->ssid), disableSSID) == 0) {
      // This SSID should be disabled.
      setStationDisabledBit(i, true);
    }
  }

  // Recompute heatmaps minus all the disabled stations.
  wifi24GHzHeatmap.clear();
  wifi50GHzHeatmap.clear();

  // Set up channel plans for global heatmaps.
  populateHeatmapChannelPlan(&wifi24GHzHeatmap, wifi24GHzChannelPlan);
  populateHeatmapChannelPlan(&wifi50GHzHeatmap, wifi50GHzChannelPlan);

  for (size_t i = 0; i < SCAN_MAX_NUMBER; i++) {
    pWifiAPRecord = reinterpret_cast<const wifi_ap_record_t*>(WiFi.getScanInfoByIndex(i));
    int channelNum = pWifiAPRecord->primary;

    if (!isStationDisabled(i)) {
      recordSignalHeatmap(pWifiAPRecord, getHeatmapForChannel(channelNum));
    }
  }

  memset(disableMessage, 0, MAX_STATUS_LINE_LEN + 1);
  snprintf(disableMessage, MAX_STATUS_LINE_LEN, "Disabled station %u: %s",
      wifiIdx, disableSSID);
  setStatusLine(disableMessage);
}

// Set the disabled bit to 'false' for wifiIdx and all other stations with the same SSID.
// Add the newly-enabled stations to the heatmap.
static void enableStation(size_t wifiIdx) {
  const char *enableSSID;
  const wifi_ap_record_t *pWifiAPRecord =
      reinterpret_cast<const wifi_ap_record_t*>(WiFi.getScanInfoByIndex(wifiIdx));
  enableSSID = reinterpret_cast<const char*>(pWifiAPRecord->ssid);

  for (size_t i = 0; i < SCAN_MAX_NUMBER; i++) {
    pWifiAPRecord = reinterpret_cast<const wifi_ap_record_t*>(WiFi.getScanInfoByIndex(i));
    if (strcmp(reinterpret_cast<const char*>(pWifiAPRecord->ssid), enableSSID) == 0) {
      // This SSID should be enabled.
      setStationDisabledBit(i, false);
      // Add it to the heatmap.
      int channelNum = pWifiAPRecord->primary;
      recordSignalHeatmap(pWifiAPRecord, getHeatmapForChannel(channelNum));
    }
  }

  memset(disableMessage, 0, MAX_STATUS_LINE_LEN + 1);
  snprintf(disableMessage, MAX_STATUS_LINE_LEN, "Enabled station %u: %s",
      wifiIdx, enableSSID);
  setStatusLine(disableMessage);
}


////////    GPIO Button handlers    ////////

// 5-way hat "in" -- show details for current station.
static void stationDetailsHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) {
    detailsButton.setFocus(true);
    screen.renderWidget(&detailsButton);
    return;
  }

  // button released; defocus button and do action.
  detailsButton.setFocus(false);
  screen.renderWidget(&detailsButton);
  displayDetails(wifiListScroll.selectIdx());
  screen.render();
}

// Refresh the list.
static void refreshHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) {
    rescanButton.setFocus(true);
    screen.renderWidget(&rescanButton);
    return;
  }

  // button released; defocus button and do action.
  rescanButton.setFocus(false);
  screen.renderWidget(&rescanButton);
  scanWifi();
  screen.render();
}

// Clicking the 'back' button in Station Details goes back to the station list.
static void backToStationListHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) {
    detailsBackBtn.setFocus(true);
    screen.renderWidget(&detailsBackBtn);
    return;
  }

  // button released; defocus button and do action.
  detailsBackBtn.setFocus(false);
  screen.renderWidget(&detailsBackBtn);
  // Go back to the main signal list.
  // Replace this UI button and handler fn with the 'Details' button that moves in the other
  // direction.
  displayStationList();
  screen.render(); // Redraw entire screen.
}

// 5-way hat "up" -- scroll up the list.
static void scrollUpHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) {
    if (carouselPos == ContentCarousel_SignalList) {
      wifiListScroll.renderScrollUp(lcd, true);
    }
    return;
  }

  // Button released; perform action.
  bool scrollOK = false;
  if (wifiListScroll.selectIdx() == wifiListScroll.position()) {
    // Current selection is at the top of the window, so we need to scroll the window
    // up by one element as we also move the selection up.
    scrollOK = wifiListScroll.scrollUp();
  }

  // In all cases, move the selection up by 1 element.
  bool selectOK = wifiListScroll.selectUp();

  if (carouselPos == ContentCarousel_SignalList) {
    // Only re-render if we're in the signal list vscroll.
    uint32_t flags = 0;
    if (scrollOK) {
      flags |= RF_VSCROLL_SCROLLBAR | RF_VSCROLL_CONTENT;
    }

    if (selectOK) {
      flags |= RF_VSCROLL_SELECTED;
    }

    if (!scrollOK) {
      // Cannot scroll further up. Only redraw the up-caret to finish the animation.
      wifiListScroll.renderScrollUp(lcd, false);
    }

    if (flags) {
      // Some portion of the widget broader than the scrollbar arrow needs redrawing.
      screen.renderWidget(&wifiListScroll, flags);
    }
  } else if (carouselPos == ContentCarousel_Details) {
    // Just flip to the previous 'page' of details.
    displayDetails(wifiListScroll.selectIdx());
    screen.render();
  }
}

// 5-way hat "down" -- scroll down the list.
static void scrollDownHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) {
    if (carouselPos == ContentCarousel_SignalList) {
      wifiListScroll.renderScrollDown(lcd, true);
    }
    return;
  }

  // Button released; perform action.
  bool scrollOK = false;
  if (wifiListScroll.selectIdx() == wifiListScroll.bottomIdx() - 1) {
    // We've already moved the cursor to the last line of the visible page and need to
    // keep moving down. We need to both scroll down the VScroll page and select the
    // subsequent item.
    scrollOK = wifiListScroll.scrollDown();
  }

  // In all cases, move the selection down by one element.
  bool selectOK = wifiListScroll.selectDown();

  if (carouselPos == ContentCarousel_SignalList) {
    // Only re-render scrollbox if the station list is displayed.
    uint32_t flags = 0;
    if (scrollOK) {
      flags |= RF_VSCROLL_SCROLLBAR | RF_VSCROLL_CONTENT;
    }

    if (selectOK) {
      flags |= RF_VSCROLL_SELECTED;
    }

    if (!scrollOK) {
      // Cannot scroll further up. Only redraw the down-caret to finish the animation.
      wifiListScroll.renderScrollDown(lcd, false);
    }

    if (flags) {
      // Some portion of the widget broader than the scrollbar arrow needs redrawing.
      screen.renderWidget(&wifiListScroll, flags);
    }
  } else if (carouselPos == ContentCarousel_Details) {
    // Just flip to the next 'page' of details.
    displayDetails(wifiListScroll.selectIdx());
    screen.render();
  }
}

static void toggleHeatmapButtonHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) {
    heatmapButton.setFocus(true);
    screen.renderWidget(&heatmapButton);
    return;
  }

  // Button released; perform action.
  heatmapButton.setFocus(false);
  screen.renderWidget(&heatmapButton);
  rotateContentCarousel(); // Move to the next heatmap (or channel list view)
  screen.render(); // Redraw entire screen.
}

// We are currently on the Details page and the user wants to enable a currently-disabled
// station in the heatmap.
static void enableStationHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) {
    detailsDisableBtn.setFocus(true);
    screen.renderWidget(&detailsDisableBtn);
    return;
  }

  // Button released; perform action.
  detailsDisableBtn.setFocus(false);
  detailsDisableBtn.setText(disableStr); // Change button label to "disable"
  screen.renderWidget(&detailsDisableBtn);
  buttons[TOP_BUTTON_2_DEBOUNCE_ID].setHandler(disableStationHandler); // Change button handler fn.

  size_t curWifiIdx = wifiListScroll.selectIdx();
  enableStation(curWifiIdx);
}

// We are currently on the Details page and the user wants to disable a currently-enabled
// station in the heatmap.
static void disableStationHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) {
    detailsDisableBtn.setFocus(true);
    screen.renderWidget(&detailsDisableBtn);
    return;
  }

  // Button released; perform action.
  detailsDisableBtn.setFocus(false);
  detailsDisableBtn.setText(enableStr); // Change button label to "enable"
  screen.renderWidget(&detailsDisableBtn);
  buttons[TOP_BUTTON_2_DEBOUNCE_ID].setHandler(enableStationHandler); // Change button handler fn.

  size_t curWifiIdx = wifiListScroll.selectIdx();
  disableStation(curWifiIdx);
}

static void populateHeatmapChannelPlan(Heatmap *heatmap, const tc::const_array<int> &channelPlan) {
  for (auto channel: channelPlan) {
    heatmap->defineChannel(channel);
  }
}


////////    Spectrum scanning; building the main station list VScroll & heatmap    ////////

static bool hasScanned = false;
static StrLabel* ssidLabels[SCAN_MAX_NUMBER];
static IntLabel* chanLabels[SCAN_MAX_NUMBER];
static IntLabel* rssiLabels[SCAN_MAX_NUMBER];
static String bssids[SCAN_MAX_NUMBER];
static StrLabel* bssidLabels[SCAN_MAX_NUMBER];
static Cols* wifiRows[SCAN_MAX_NUMBER]; // Each row is a Cols for (ssid, chan, rssi, bssid)

// Register a channel's bandwidth usage on an appropriate heatmap.
static void recordSignalHeatmap(const wifi_ap_record_t *pWifiAPRecord, Heatmap *bandHeatmap) {
  int channelNum = pWifiAPRecord->primary;
  int rssiVal = pWifiAPRecord->rssi;

  bool is24GHz = channelNum <= max24GHzChannelNum;
  tc::vector<int> *pSpectralMask = NULL;

  int upperChannelNum = channelNum;
  int lowerChannelNum = channelNum;

  switch (pWifiAPRecord->second) {
  case wifi_second_chan_t::WIFI_SECOND_CHAN_NONE:
    upperChannelNum = channelNum;
    lowerChannelNum = channelNum;
    break;
  case wifi_second_chan_t::WIFI_SECOND_CHAN_ABOVE:
    if (is24GHz) {
      upperChannelNum = channelNum + 4; // +20 MHz from primary channel.
    } else {
      // 5 GHz
      upperChannelNum = channelNum; //TODO: next up;
    }
    lowerChannelNum = channelNum;
    break;
  case wifi_second_chan_t::WIFI_SECOND_CHAN_BELOW:
    upperChannelNum = channelNum;
    if (is24GHz) {
      lowerChannelNum = channelNum - 4; // -20 MHz from primary channel.
    } else {
      // 5 GHz
      lowerChannelNum = channelNum; //TODO: next below;
    }
    break;
  }

  // Determine which mode(s) are active and load the appropriate cross-channel interference data.
  if (pWifiAPRecord->phy_11b) {
    // We are on 2.4 GHz 802.11b. (g or n may also be enabled, but the mask for 802.11b is
    // more punishing to nearby channels, so apply this one to the interference chart.)
    pSpectralMask = &spectralMask80211B;
  } else if (pWifiAPRecord->phy_11n) {
    // We are on 802.11n.
    if (pWifiAPRecord->second == wifi_second_chan_t::WIFI_SECOND_CHAN_NONE) {
      // 20 MHz bandwidth.
      if (is24GHz) {
        // 20 MHz 2.4 GHz 802.11n shares spectral mask with 802.11g
        pSpectralMask = &spectralMask80211G;
      } else {
        // 20 MHz 5 GHz 802.11n does not interfere with any adjacent channels.
        pSpectralMask = &emptySpectralMask;
      }
    } else {
      // 40 MHz bandwidth
      if (is24GHz) {
        // We are going to interfere with basically all the channels.
        pSpectralMask = &spectralMask80211N40MHz24G;
      } else {
        pSpectralMask = &spectralMask80211N40MHz50G;
      }
    }
  } else {
    // We are on 2.4 GHz 802.11g
    pSpectralMask = &spectralMask80211G;
  }

  if (is24GHz) {
    // First, fill in the range of [lowerChannelId, upperChannelId] with +0 dBm
    for (int i = lowerChannelNum; i <= upperChannelNum; i++) {
      if (i >= 0 && i <= max24GHzChannelNum) {
        bandHeatmap->addSignal(i, rssiVal);
      }
    }

    // Add in cross-channel interference.
    // For a two-sided channel definition, walk from upperChannelId + 0, 1, 2, 3... and apply the
    // decay factor, as well as walking down from lowerChannelId - 0, 1, 2, 3...
    int offset = 1;
    for (int dbm: *pSpectralMask) {
      int crosstalkRssi = rssiVal + dbm;
      if (crosstalkRssi >= noiseFloorDBm) {
        if (upperChannelNum + offset <= max24GHzChannelNum) {
          bandHeatmap->addSignal(upperChannelNum + offset, crosstalkRssi);
        }

        if (lowerChannelNum - offset > 0) {
          bandHeatmap->addSignal(lowerChannelNum - offset, crosstalkRssi);
        }
      }

      offset++;
    }
  } else {
    // TODO(aaron): 5 GHz interference
    bandHeatmap->addSignal(lowerChannelNum, rssiVal);
  }
}

/**
 * Populate the UI widget fields for the Details page for a particular wifi station.
 */
static void populateStationDetails(size_t wifiIdx) {
  const wifi_ap_record_t *pWifiAPRecord =
      reinterpret_cast<const wifi_ap_record_t*>(WiFi.getScanInfoByIndex(wifiIdx));

  detailsChan.setValue(pWifiAPRecord->primary);
  detailsRssi.setValue(pWifiAPRecord->rssi);
  detailsSsid.setText(reinterpret_cast<const char*>(&(pWifiAPRecord->ssid[0])));
  detailsBssid.setText(bssids[wifiIdx]);

  // Reformat char buffer that underwrites detailsBandwidth StrLabel.
  memset(detailsBandwidthText, 0, BANDWIDTH_TEXT_LEN);
  if (pWifiAPRecord->second == wifi_second_chan_t::WIFI_SECOND_CHAN_NONE) {
    strncpy(detailsBandwidthText, "20 MHz", BANDWIDTH_TEXT_LEN);
  } else {
    strncpy(detailsBandwidthText, "40 MHz", BANDWIDTH_TEXT_LEN);
  }

  // Reformat char buffer that underwrites detailsModes StrLabel.
  // Construct the string '802.11: [b][, g][, n]' depending on which modes are active.
  memset(detailsModesText, 0, MODES_TEXT_LEN);
  strncpy(detailsModesText, "802.11: ", MODES_TEXT_LEN);
  int numModes = 0;
  if (pWifiAPRecord->phy_11b) {
    strcat(detailsModesText, "b");
    numModes++;
  }

  if (pWifiAPRecord->phy_11g) {
    if (numModes) {
      strcat(detailsModesText, ", ");
    }
    strcat(detailsModesText, "g");
    numModes++;
  }

  if (pWifiAPRecord->phy_11n) {
    if (numModes) {
      strcat(detailsModesText, ", ");
    }
    strcat(detailsModesText, "n");
    numModes++;
  }

  if (numModes == 0 && pWifiAPRecord->primary > max24GHzChannelNum) {
    // Channel mode bit was not specified, but this is a 5 GHz channel so it must be N.
    strcat(detailsModesText, "n");
    numModes++;
  }

  if (numModes == 0 && pWifiAPRecord->second != wifi_second_chan_t::WIFI_SECOND_CHAN_NONE) {
    // Channel mode bit was not specified but this is a 40 MHz bandwidth channel so it must be N.
    strcat(detailsModesText, "n");
    numModes++;
  }

  switch(pWifiAPRecord->authmode) {
  case wifi_auth_mode_t::WIFI_AUTH_OPEN:
    detailsSecurity.setText(SECURITY_OPEN);
    break;
  case wifi_auth_mode_t::WIFI_AUTH_WEP:
    detailsSecurity.setText(SECURITY_WEP);
    break;
  case wifi_auth_mode_t::WIFI_AUTH_WPA_PSK:
    detailsSecurity.setText(SECURITY_WPA_PSK);
    break;
  case wifi_auth_mode_t::WIFI_AUTH_WPA2_PSK:
    detailsSecurity.setText(SECURITY_WPA2_PSK);
    break;
  case wifi_auth_mode_t::WIFI_AUTH_WPA_WPA2_PSK:
    detailsSecurity.setText(SECURITY_WPA_WPA2_PSK);
    break;
  case wifi_auth_mode_t::WIFI_AUTH_WPA2_ENTERPRISE:
    detailsSecurity.setText(SECURITY_WPA2_ENTERPRISE);
    break;
  default:
    detailsSecurity.setText(SECURITY_UNKNOWN);
    break;
  }

  // Fill out a heatmap for this station only.
  detailsHeatmap.clear();
  if (pWifiAPRecord->primary <= max24GHzChannelNum) {
    // Populate the heatmap channel spectrum from the 2.4 GHz channel list
    populateHeatmapChannelPlan(&detailsHeatmap, wifi24GHzChannelPlan);
  } else {
    // Populate the heatmap channel spectrum from the 5 GHz channel list
    populateHeatmapChannelPlan(&detailsHeatmap, wifi50GHzChannelPlan);
  }
  recordSignalHeatmap(pWifiAPRecord, &detailsHeatmap);
}

static void makeWifiRow(int wifiIdx) {
  const wifi_ap_record_t *pWifiAPRecord =
      reinterpret_cast<const wifi_ap_record_t*>(WiFi.getScanInfoByIndex(wifiIdx));

  StrLabel *ssid = new StrLabel(reinterpret_cast<const char*>(&(pWifiAPRecord->ssid[0])));
  ssid->setFont(2); // Use larger 16px font for SSID.
  ssidLabels[wifiIdx] = ssid;

  int channelNum = pWifiAPRecord->primary;
  IntLabel *chan = new IntLabel(channelNum);
  chan->setPadding(0, 0, 4, 0); // Push default small font to middle of row height.
  chanLabels[wifiIdx] = chan;

  IntLabel *rssi = new IntLabel(pWifiAPRecord->rssi);
  rssi->setPadding(0, 0, 4, 0);
  rssiLabels[wifiIdx] = rssi;

  bssids[wifiIdx] = String(WiFi.BSSIDstr(wifiIdx)); // Formats 6-octet BSSID to hex str.
  StrLabel *bssid = new StrLabel(bssids[wifiIdx]);
  bssid->setPadding(0, 0, 4, 0);
  bssidLabels[wifiIdx] = bssid;

  Cols *wifiRow = new Cols(4);
  wifiRow->setColumn(0, ssid, SSID_WIDTH);
  wifiRow->setColumn(1, chan, CHAN_WIDTH);
  wifiRow->setColumn(2, rssi, RSSI_WIDTH);
  wifiRow->setColumn(3, bssid, BSSID_WIDTH);
  if (wifiIdx % 2 == 1) {
    // Every other row should have a non-black bg to zebra-stripe the table.
    wifiRow->setBackground(TFT_NAVY);
  } else {
    wifiRow->setBackground(TFT_BLACK);
  }

  wifiRows[wifiIdx] = wifiRow;
  wifiListScroll.add(wifiRow);

  // Get the appropriate heatmap (2.4 GHz or 5 GHz) based on the channel id.
  Heatmap *bandHeatmap = getHeatmapForChannel(channelNum);
  // Add this wifi signal to the appropriate heatmap.
  recordSignalHeatmap(pWifiAPRecord, bandHeatmap);
}


static void scanWifi() {
  DBGPRINT("scan start");

  setStatusLine("Searching for stations...");

  // If this has already been called before, free all the existing used memory,
  // by deleting all the pointed-to things from the ptrs in the various arrays.
  if (hasScanned) {
    for (int i = 0; i < SCAN_MAX_NUMBER; i++) {
      delete ssidLabels[i];
      delete chanLabels[i];
      delete rssiLabels[i];
      delete bssidLabels[i];
      delete wifiRows[i];
    }
  }

  // Initialize all our arrays to have no data / NULL ptrs.
  for (int i = 0; i < SCAN_MAX_NUMBER; i++) {
    bssids[i] = String();
  }

  memset(ssidLabels, 0, sizeof(StrLabel*) * SCAN_MAX_NUMBER);
  memset(chanLabels, 0, sizeof(IntLabel*) * SCAN_MAX_NUMBER);
  memset(rssiLabels, 0, sizeof(IntLabel*) * SCAN_MAX_NUMBER);
  memset(bssidLabels, 0, sizeof(StrLabel*) * SCAN_MAX_NUMBER);
  memset(wifiRows, 0, sizeof(Cols*) * SCAN_MAX_NUMBER);

  clearDisabledStations(); // indices of 'disabled' stations are invalid; clear out.

  wifiListScroll.clear(); // Wipe scrollbox contents.

  wifi24GHzHeatmap.clear();
  wifi50GHzHeatmap.clear();

  // Set up channel plans for global heatmaps.
  populateHeatmapChannelPlan(&wifi24GHzHeatmap, wifi24GHzChannelPlan);
  populateHeatmapChannelPlan(&wifi50GHzHeatmap, wifi50GHzChannelPlan);

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  hasScanned = true;

  DBGPRINT("scan done");
  if (n == 0) {
    DBGPRINT("no networks found");
  } else {
    for (int i = 0; i < n; i++) {
      makeWifiRow(i);
    }
  }

  wifiListScroll.setSelection(0);
  wifiListScroll.scrollTo(0);
  setStatusLine("Scan complete.", false);
}


////////    Arduino main setup & loop    ////////

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
  buttonGpioPins.push_back(WIO_KEY_C); // Button 5 ("C" is left-most btn on top)
  buttonGpioPins.push_back(WIO_KEY_B); // Button 6
  buttonGpioPins.push_back(WIO_KEY_A); // Button 7 ("A" is right-most btn on top)

  for (auto pin: buttonGpioPins) {
    pinMode(pin, INPUT_PULLUP);
  }

  buttons.push_back(Button(HAT_UP_DEBOUNCE_ID, scrollUpHandler));    // hat up
  buttons.push_back(Button(HAT_DOWN_DEBOUNCE_ID, scrollDownHandler));  // hat down
  buttons.push_back(Button(2, emptyBtnHandler)); // hat left (unused)
  buttons.push_back(Button(3, emptyBtnHandler)); // hat right (unused)
  buttons.push_back(Button(HAT_IN_DEBOUNCE_ID, stationDetailsHandler)); // 4: hat "in"/"OK"
  buttons.push_back(Button(TOP_BUTTON_1_DEBOUNCE_ID, stationDetailsHandler)); // 5: top left "details" button
  buttons.push_back(Button(TOP_BUTTON_2_DEBOUNCE_ID, emptyBtnHandler)); // top middle "refresh" button
  buttons.push_back(Button(TOP_BUTTON_3_DEBOUNCE_ID, toggleHeatmapButtonHandler)); // right: "heatmap" btn.

  lcd.begin();
  lcd.setRotation(3);
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextFont(2); // 0 for 8px, 2 for 16px.
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString("Wifi analyzer starting up...", 4, 4);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Set up main layout, with nav buttons, status, etc. and the station list vscroll.
  screen.setBackground(TRANSPARENT_COLOR);
  screen.setWidget(&rowLayout);
  rowLayout.setRow(0, &topRow, 30);
  displayStationList(); // set dataHeaderRow as row 1, wifiScrollContainer as row 2.
  rowLayout.setRow(3, &statusLineLabel, 16);

  wifiScrollContainer.setChild(&wifiListScroll);
  wifiScrollContainer.setPadding(2, 2, 1, 1); // Add 1px padding around wifi list VScroll.

  // topRow is a Cols that holds some buttons.
  topRow.setBorder(BORDER_BOTTOM, TFT_BLUE);
  topRow.setBackground(TFT_LIGHTGREY);
  topRow.setPadding(0, 0, 4, 2); // 4 px padding top; 2 px on bottom b/c we get padding from border.
  // topRow column 0 (70 px) set by setButton1() via displayStationList(), above.
  // topRow column 1 (70 px) set by setButton2() via displayStationList(), above.
  // topRow column 2 (75 px) set by setButton3() via displayStationList(), above.
  topRow.setColumn(3, NULL, EQUAL); // Rest of space to the right is empty

  detailsButton.setColor(TFT_BLUE);
  detailsButton.setPadding(4, 4, 0, 0);

  rescanButton.setColor(TFT_BLUE);
  rescanButton.setPadding(4, 4, 0, 0);

  heatmapButton.setColor(TFT_BLUE);
  heatmapButton.setPadding(4, 4, 0, 0);

  dataHeaderRow.setColumn(0, &hdrSsid, SSID_WIDTH);
  dataHeaderRow.setColumn(1, &hdrChannel, CHAN_WIDTH);
  dataHeaderRow.setColumn(2, &hdrRssi, RSSI_WIDTH);
  dataHeaderRow.setColumn(3, &hdrBssid, BSSID_WIDTH);
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

  hdrBssid.setBackground(TFT_BLUE);
  hdrBssid.setColor(TFT_WHITE);
  hdrBssid.setPadding(0, 0, 2, 0);

  statusLineLabel.setBackground(TFT_BLUE);
  statusLineLabel.setPadding(2, 0, 3, 0);

  // Set up Details page UI widgets.
  detailsPanel.setChild(&detailsRows);
  detailsPanel.setBackground(TFT_NAVY);
  detailsPanel.setPadding(4, 4, 4, 4);
  detailsRows.setRow(0, &detailsStationInfoCols, 24);
  detailsRows.setRow(1, &detailsSsidCols, 24);
  detailsRows.setRow(2, &detailsBssidCols, 24);
  detailsRows.setRow(3, &detailsModeBwCols, 24);
  detailsRows.setRow(4, &detailsSecurityCols, 24);
  detailsRows.setRow(5, NULL, EQUAL); // Empty row: Fill available space.
  detailsRows.setRow(6, &detailsHeatmapHdr, 16); // Entire row is header label for heatmap.
  detailsRows.setRow(7, &detailsHeatmap, 32); // Entire bottom row is spectrum heatmap

  detailsStationInfoCols.setColumn(0, &detailsChanHdr, 40);
  detailsStationInfoCols.setColumn(1, &detailsChan, 40);
  detailsStationInfoCols.setColumn(2, NULL, EQUAL);
  detailsStationInfoCols.setColumn(3, &detailsRssiHdr, 40);
  detailsStationInfoCols.setColumn(4, &detailsRssi, 40);
  detailsStationInfoCols.setBorder(BORDER_BOTTOM);
  detailsChanHdr.setColor(TFT_YELLOW);
  detailsChanHdr.setFont(2); // larger font size for channel id.
  detailsRssiHdr.setColor(TFT_YELLOW);
  detailsRssiHdr.setFont(0);
  detailsChan.setFont(2);
  detailsRssi.setFont(0);

  detailsSsidCols.setColumn(0, &detailsSsidHdr, 40);
  detailsSsidCols.setColumn(1, &detailsSsid, EQUAL);
  detailsSsidHdr.setColor(TFT_YELLOW);
  detailsSsidHdr.setFont(2); // larger font size for SSID.
  detailsSsid.setFont(2);

  detailsBssidCols.setColumn(0, &detailsBssidHdr, 40);
  detailsBssidCols.setColumn(1, &detailsBssid, EQUAL);
  detailsBssidHdr.setColor(TFT_YELLOW);
  detailsBssidHdr.setFont(2); // larger font size for BSSID.
  detailsBssid.setFont(2);

  detailsModeBwCols.setColumn(0, &detailsModes, EQUAL);
  detailsModeBwCols.setColumn(1, &detailsBandwidth, EQUAL);
  detailsModes.setColor(TFT_YELLOW);
  detailsModes.setFont(0);
  detailsBandwidth.setColor(TFT_YELLOW);
  detailsBandwidth.setFont(0);

  detailsSecurityCols.setColumn(0, &detailsSecurityHdr, 60);
  detailsSecurityCols.setColumn(1, &detailsSecurity, EQUAL);
  detailsSecurityHdr.setColor(TFT_YELLOW);
  detailsSecurityHdr.setFont(2);
  detailsSecurity.setFont(2);

  detailsHeatmapHdr.setColor(TFT_YELLOW);
  detailsHeatmapHdr.setFont(0);

  detailsHeatmap.setColor(TFT_WHITE);

  // also theme the buttons displayed on the details page
  detailsBackBtn.setColor(TFT_BLUE);
  detailsBackBtn.setPadding(4, 4, 0, 0);

  detailsDisableBtn.setColor(TFT_BLUE);
  detailsDisableBtn.setPadding(4, 4, 0, 0);

  scanWifi(); // Populates VScroll and global heatmap elements.
  lcd.fillScreen(TFT_BLACK); // Clear 'loading' screen msg.
  screen.render();
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
