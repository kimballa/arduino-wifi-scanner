
wifi-test
==========

An Arduino sketch that tests the Seeed Wio Terminal's wifi capabilities.

This is explicitly designed to operate on the `Seeeduino:samd:wio_terminal` board.

Dependencies
------------

This depends on the `Seeed_Arduino_rpcWifi` library and its extensive
net of transitive dependencies for WiFi, and `Seeed_Arduino_LCD` for LCD
screen usage.

Also uses my [debounce library](https://github.com/kimballa/button-debounce) for buttons
and [PyArduinoDebug](https://github.com/kimballa/PyArduinoDebug) for debugger/logging
support.

Compiling
---------

I build this with my [Arduino makefile](https://github.com/kimballa/arduino-makefile).

* Clone the makefile project such that `arduino-makefile/` is a sibling of this project directory.
* Create `~/arduino_mk.conf` from the template in that directory and customize it to your board
  and local environment. See other one-time setup instructions in that project's README.md and/or
  the comment header of `arduino.mk`.
* Clone the `libardinodbg` library project as a sibling as well.
* In the library directory, build with `make install`.
* Build this with `make image` or build and upload with `make verify`.

License
-------

This project is licensed under the BSD 3-Clause license. See LICENSE.txt for complete details.
