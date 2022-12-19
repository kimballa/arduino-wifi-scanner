# (c) Copyright 2022 Aaron Kimball

BOARD := Seeeduino:samd:seeed_wio_terminal

prog_name := wifi_test
src_dirs := src

libs := tinycollections uiwidgets \
  seeed_arduino_lcd seeed_arduino_rpcwifi seeed_arduino_rpcunified \
	seeed_arduino_mbedtls seeed_arduino_freertos seeed_arduino_sfud seeed_arduino_fs \
	spi adafruit_zerodma debounce PyArduinoDebug

include_dirs += $(arch_include_root)/seeed_arduino_lcd
include_dirs += $(arch_include_root)/seeed_arduino_rpcwifi
include_dirs += $(arch_include_root)/seeed_arduino_rpcunified
include_dirs += $(arch_include_root)/seeed_arduino_freertos
include_dirs += $(arch_include_root)/seeed_arduino_mbedtls
include_dirs += $(include_root)/debounce
include_dirs += $(include_root)/uiwidgets

XFLAGS += -Wall

include ../arduino-makefile/arduino.mk
