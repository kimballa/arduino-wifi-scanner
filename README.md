
Wifi scanner
============

An Arduino sketch that uses the Seeed Wio Terminal's wifi capabilities to map out
channel congestion in 2.4 GHz (802.11b/g/n) and 5 GHz (802.11a/n/ac) frequency bands.

This is explicitly designed to operate on the `Seeeduino:samd:wio_terminal` board,
with its companion RTL8721D dual-band wifi SoC.

Dependencies
------------

This depends on the [Seeed\_Arduino\_rpcWiFi](https://github.com/Seeed-Studio/Seeed_Arduino_rpcWiFi)
library (and its extensive net of transitive dependencies for WiFi), and
[Seeed\_Arduino\_LCD](https://github.com/Seeed-Studio/Seeed_Arduino_LCD)
for LCD screen usage.

Also uses the following libraries of mine:
* [debounce](https://github.com/kimballa/button-debounce) for buttons
* [PyArduinoDebug](https://github.com/kimballa/PyArduinoDebug) for debugger/logging
  support
* [tiny-collections](https://github.com/kimballa/tiny-collections) for STL-like collections
  (`vector<T>`...)
* [uiwidgets](https://github.com/kimballa/uiwidgets) for a UI widget interface built on top of
  `TFT_eSPI`

See the full `libs` list in the
[Makefile](https://github.com/kimballa/arduino-wifi-scanner/blob/main/Makefile) for all
direct and transitive library dependencies.

Compiling
---------

If you add the requisite libraries to a sketch based on this source, you should be able to
compile and deploy with the Arduino IDE.

I build this with my [Arduino makefile](https://github.com/kimballa/arduino-makefile).

* Clone the makefile project such that `arduino-makefile/` is a sibling of this project directory.
* Create `~/arduino_mk.conf` from the template in that directory and customize it to your board
  and local environment. See other one-time setup instructions in that project's README.md and/or
  the comment header of `arduino.mk`.
* Clone `PyArduinoDebug`, `button-debounce`, and other necessary library projects as siblings as well.
* In the library directory, build with `make install`.
* After building all the libraries, build this with `make image` or build and upload with `make verify`.

License
-------

This project is licensed under the BSD 3-Clause license. See LICENSE.txt for complete details.
