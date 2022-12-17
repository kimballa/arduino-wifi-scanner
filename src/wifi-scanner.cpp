// (c) Copyright 2022 Aaron Kimball
//
// Measure wifi signal usage.

#include "wifi-scanner.h"


static void scanWifi();

TFT_eSPI lcd;

static constexpr int16_t SSID_WIDTH = 140;
static constexpr int16_t CHAN_WIDTH = 30;
static constexpr int16_t RSSI_WIDTH = 30;
static constexpr int16_t BSSID_WIDTH = 80;

Screen screen(lcd);
// The screen is a set of rows: row of buttons, then the main vscroll.
static Rows rowLayout(4); // 3 rows.
// the top row (#0) is a set of buttons.
static Cols topRow(4); // 4 columns
static const char *detailsStr = "Details";
static const char *rescanStr = "Refresh";
static const char *heatmapStr = "Heatmap";
static UIButton detailsButton(detailsStr);
static UIButton rescanButton(rescanStr);
static UIButton heatmapButton(heatmapStr);

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

// Row 2: The main focus of the screen is a scrollable list of wifi SSIDs.
static VScroll wifiListScroll;
static Panel wifiScrollContainer; // Wrap VScroll in a container for padding.

static Heatmap wifi24GHzHeatmap; // Heatmap of congestion on 2.4 GHz channels
static Heatmap wifi50GHzHeatmap; // Heatmap of congestion on 5 GHz channels

// Enumerate all the possible main-area content panels.
constexpr unsigned int ContentCarousel_SignalList = 0;  // Show a list of wifi SSIDs
constexpr unsigned int ContentCarousel_Heatmap24 = 1;   // Show a heatmap of 2.4 GHz channel usage
constexpr unsigned int ContentCarousel_Heatmap50 = 2;   // Show a heatmap of 5 GHz channel usage
constexpr unsigned int MaxContentCarousel = ContentCarousel_Heatmap50;

static unsigned int carouselPos = ContentCarousel_SignalList;

// Row 3: Status / msg line.
constexpr unsigned int MAX_STATUS_LINE_LEN = 80; // max len in chars; buffer is +1 more for '\0'
static char statusLine[MAX_STATUS_LINE_LEN + 1];
static StrLabel statusLineLabel = StrLabel(statusLine);


// physical pushbuttons
static tc::vector<Button> buttons;
static tc::vector<uint8_t> buttonGpioPins;

static constexpr unsigned int wifi24GHzChannelPlan[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
static constexpr unsigned int wifi50GHzChannelPlan[] = {
  32, 36, 40, 44, 48,        // U-NII-1 channels, unrestricted
  // 52, 56, 60, 64, 68, 96, // U-NII-1 and 2A, use DFS or below 500mW
  // 100, 104, 108, 112,
  // 116, 120, 124, 128,
  // 132, 136, 140, 144,     // U-NII-2C and 3; use DFS or below 500mW
  149, 153, 157, 161, 165,   // U-NII-3 channels,  unrestricted
  // 169, 173, 177,             // U-NII-4 1W, indoor usage only (since 2020)
};

/** Return the heatmap associated with a particular channel. */
static Heatmap *getHeatmapForChannel(int chan) {
  // TODO(aaron): This will always return a heatmap, including for invalid channels. OK?
  if (chan < 12) {
    return &wifi24GHzHeatmap;
  } else {
    return &wifi50GHzHeatmap;
  }
}


// Adjust the main display area content
void rotateContentCarousel() {
  carouselPos++;
  if (carouselPos > MaxContentCarousel) {
    carouselPos = 0;
  }

  switch (carouselPos) {
  case ContentCarousel_SignalList:
    // TODO(aaron): Note code clone of these 2 rows in setup().
    rowLayout.setRow(1, &dataHeaderRow, 16);          // Add back header row.
    rowLayout.setRow(2, &wifiScrollContainer, EQUAL); // VScroll expands to fill available space.
    setStatusLine("Scan complete.");
    break;
  case ContentCarousel_Heatmap24:
    rowLayout.setRow(1, NULL, 0); // Blank out header row above vscroll.
    rowLayout.setRow(2, &wifi24GHzHeatmap, EQUAL); // Put in the 2.4 GHz spectrum heatmap
    setStatusLine("2.4 GHz spectrum congestion");
    break;
  case ContentCarousel_Heatmap50:
    rowLayout.setRow(1, NULL, 0); // Blank out header row above vscroll.
    rowLayout.setRow(2, &wifi50GHzHeatmap, EQUAL); // Put in the 5 GHz spectrum heatmap
    setStatusLine("5 GHz spectrum congestion");
    break;
  default:
    // ??? How'd we get here? Conform the data so it rotates back to the signal list.
    carouselPos = MaxContentCarousel;
    rotateContentCarousel();
    break;
  }
}

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

// 5-way hat "up" -- scroll up the list.
static void scrollUpHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) {
    wifiListScroll.renderScrollUp(lcd, true);
    return;
  }

  // Button released; perform action.
  if (wifiListScroll.scrollUp()) {
    screen.renderWidget(&wifiListScroll, RF_VSCROLL_SCROLLBAR | RF_VSCROLL_CONTENT);
  } else {
    // Cannot scroll further up. Only redraw the up-caret to finish the animation.
    wifiListScroll.renderScrollUp(lcd, false);
  }
}

// 5-way hat "down" -- scroll down the list.
static void scrollDownHandler(uint8_t btnId, uint8_t btnState) {
  if (btnState == BTN_PRESSED) {
    wifiListScroll.renderScrollDown(lcd, true);
    return;
  }

  // Button released; perform action.
  if (wifiListScroll.scrollDown()) {
    screen.renderWidget(&wifiListScroll, RF_VSCROLL_SCROLLBAR | RF_VSCROLL_CONTENT);
  } else {
    // Cannot scroll further down. Only redraw the down-caret to finish the animation.
    wifiListScroll.renderScrollDown(lcd, false);
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

  buttons.push_back(Button(0, scrollUpHandler));
  buttons.push_back(Button(1, scrollDownHandler));
  buttons.push_back(Button(2, emptyBtnHandler));
  buttons.push_back(Button(3, emptyBtnHandler));
  buttons.push_back(Button(4, emptyBtnHandler));
  buttons.push_back(Button(5, emptyBtnHandler));
  buttons.push_back(Button(6, emptyBtnHandler));
  buttons.push_back(Button(7, toggleHeatmapButtonHandler));

  lcd.begin();
  lcd.setRotation(3);
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextFont(2); // 0 for 8px, 2 for 16px.
  lcd.setTextColor(TFT_WHITE);
  lcd.drawString("Wifi analyzer starting up...", 0, 0);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  screen.setBackground(TRANSPARENT_COLOR);
  screen.setWidget(&rowLayout);
  rowLayout.setRow(0, &topRow, 30);
  rowLayout.setRow(1, &dataHeaderRow, 16);
  rowLayout.setRow(2, &wifiScrollContainer, EQUAL); // VScroll expands to fill available space.
  rowLayout.setRow(3, &statusLineLabel, 16);

  wifiScrollContainer.setChild(&wifiListScroll);
  wifiScrollContainer.setPadding(2, 2, 1, 1); // Add 1px padding around wifi list VScroll.

  // topRow is a Cols that holds some buttons.
  topRow.setBorder(BORDER_BOTTOM, TFT_BLUE);
  topRow.setBackground(TFT_LIGHTGREY);
  topRow.setPadding(0, 0, 4, 2); // 4 px padding top; 2 px on bottom b/c we get padding from border.
  topRow.setColumn(0, &detailsButton, 70);
  topRow.setColumn(1, &rescanButton, 70);
  topRow.setColumn(2, &heatmapButton, 75);
  topRow.setColumn(3, NULL, EQUAL); // Rest of space to the right is empty

  detailsButton.setColor(TFT_BLUE);
  detailsButton.setPadding(4, 4, 0, 0);
  detailsButton.setFocus(true);

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

  // Set up channel plans in heatmaps.
  for (auto channel: wifi24GHzChannelPlan) {
    wifi24GHzHeatmap.defineChannel(channel);
  }

  for (auto channel: wifi50GHzChannelPlan) {
    wifi50GHzHeatmap.defineChannel(channel);
  }

  scanWifi(); // Populates VScroll and heatmap elements.
  lcd.fillScreen(TFT_BLACK); // Clear 'loading' screen msg.
  screen.render();

  setStatusLine("Scan complete.");
}

static String ssids[SCAN_MAX_NUMBER];
static StrLabel* ssidLabels[SCAN_MAX_NUMBER];
static IntLabel* chanLabels[SCAN_MAX_NUMBER];
static IntLabel* rssiLabels[SCAN_MAX_NUMBER];
static String bssids[SCAN_MAX_NUMBER];
static StrLabel* bssidLabels[SCAN_MAX_NUMBER];
static Cols* wifiRows[SCAN_MAX_NUMBER]; // Each row is a Cols for (ssid, chan, rssi, bssid)

static void makeWifiRow(int wifiIdx) {
  ssids[wifiIdx] = String(WiFi.SSID(wifiIdx));
  StrLabel *ssid = new StrLabel(ssids[wifiIdx].c_str());
  ssid->setFont(2); // Use larger 16px font for SSID.
  ssidLabels[wifiIdx] = ssid;

  int channelNum = WiFi.channel(wifiIdx);
  IntLabel *chan = new IntLabel(channelNum);
  chan->setPadding(0, 0, 4, 0); // Push default small font to middle of row height.
  chanLabels[wifiIdx] = chan;

  int rssiVal = WiFi.RSSI(wifiIdx);
  IntLabel *rssi = new IntLabel(rssiVal);
  rssi->setPadding(0, 0, 4, 0);
  rssiLabels[wifiIdx] = rssi;

  // Now that we have channel & RSSI, register this on the appropriate heatmap.
  getHeatmapForChannel(channelNum)->addSignal(channelNum, rssiVal);

  bssids[wifiIdx] = String(WiFi.BSSIDstr(wifiIdx));
  StrLabel *bssid = new StrLabel(bssids[wifiIdx].c_str());
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
}


static void scanWifi() {
  DBGPRINT("scan start");

  // TODO(aaron): If this has already been called, free all the existing used memory,
  // by deleting all the pointed-to things from the ptrs in the various arrays.

  // Initialize all our arrays to have no data / NULL ptrs.
  for (int i = 0; i < SCAN_MAX_NUMBER; i++) {
    ssids[i] = String();
    bssids[i] = String();
  }

  memset(ssidLabels, 0, sizeof(StrLabel*) * SCAN_MAX_NUMBER);
  memset(chanLabels, 0, sizeof(IntLabel*) * SCAN_MAX_NUMBER);
  memset(rssiLabels, 0, sizeof(IntLabel*) * SCAN_MAX_NUMBER);
  memset(bssidLabels, 0, sizeof(StrLabel*) * SCAN_MAX_NUMBER);
  memset(wifiRows, 0, sizeof(Cols*) * SCAN_MAX_NUMBER);

  wifiListScroll.clear(); // Wipe scrollbox contents.

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();

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
