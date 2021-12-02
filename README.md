# Chinese Diesel Heater interface module
This repo contains the files for a small module designed to interface between a Chinese diesel heater and any microcontroller (either 5v or 3.3v) using I2C. This opens up the possibility of controlling a diesel heater with, for example, a Raspberry Pi, Arduino or ESP32. 

## Schematic/board notes
* The current schematic contains circuitry to allow this to be interfaced by both 5V and 3.3V microcontrollers, selectable via jumpers. This includes level shifted I2C. 
* An on board voltage regulator allows the 5V supplied by the heater to be used to power both 5V and 3.3V devices. 
* An EEPROM is also included, but not currently used for anything.
* Programming the chip requires a UPDI programmer

## Firmware notes
* The firmware appears to work, but has not been extensively tested.
* The I2C API hasn't been documented, but an Arduino driver has been created here: https://github.com/TMakins/CDHInterface

## Availability
A small number of modules have been made, which are pin-compatible with the ESP32 dev board. Contact me if you'd like to buy one. 
