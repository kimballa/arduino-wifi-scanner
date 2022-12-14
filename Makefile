# (c) Copyright 2021 Aaron Kimball

prog_name := wifi_test
src_dirs := src src/uiwidgets

libs := seeed_arduino_lcd seeed_arduino_rpcwifi seeed_arduino_rpcunified \
	seeed_arduino_mbedtls seeed_arduino_freertos seeed_arduino_sfud seeed_arduino_fs \
	spi adafruit_zerodma PyArduinoDebug

include_dirs += $(arch_include_root)/seeed_arduino_lcd
include_dirs += $(arch_include_root)/seeed_arduino_rpcwifi
include_dirs += $(arch_include_root)/seeed_arduino_rpcunified
include_dirs += $(arch_include_root)/seeed_arduino_freertos
include_dirs += $(arch_include_root)/seeed_arduino_mbedtls

XFLAGS += -Wall

include ../arduino-makefile/arduino.mk
