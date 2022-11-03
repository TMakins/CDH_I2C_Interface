# Chinese Diesel Heater interface module
This repo contains the files for a small module designed to interface between a Chinese diesel heater and any microcontroller (either 5v or 3.3v) using I2C. This opens up the possibility of controlling a diesel heater with, for example, a Raspberry Pi, Arduino or ESP32. 

## Schematic/board notes
* The current schematic contains circuitry to allow this to be interfaced by both 5V and 3.3V microcontrollers, selectable via jumpers. This includes level shifted I2C. 
* An on board voltage regulator allows the 5V supplied by the heater to be used to power both 5V and 3.3V devices. 
* An EEPROM is also included, but not currently used for anything.
* Programming the chip requires a UPDI programmer

## Firmware notes
* The firmware appears to work, but has not been extensively tested.
* The I2C API hasn't yet been documented, but an Arduino library has been created here: https://github.com/TMakins/CDHInterface

A copy of the compiled software is available under Releases. 

## Compatibility
It's getting increasingly difficult to determine if a Chinese Diesel heater uses the protocol used here, often referred to as the "Blue Wire" protocol, as initially reverse engineered by Ray Jones. However, Ray has put a lot of working into documenting which heaters are compatible with his Afterburner controller, so it's worth looking there (or ideally just buying one of his controllers!). If your heater is compatible with the Afterburner, it will be compatible with the software in this repo. 

## License
There is no license, everything here is open source, do what you want with it. 

## Credit
This is based on the initial protocol research done by Ray Jones, who now makes the excellent Afterburner controller for these heaters. 

## Disclaimer
If you use this software, whether running on hardware I've made or on hardware you've made yourself, and it causes damage of any form (including to your heater, to you, to the hardware) I am not responsible. 

# Boards
A small number of modules have been made, which are pin-compatible with the ESP32 dev board. These do not include header pins and are **not** programmed. 

# Availability
A small number of boards have been made which are available here:
https://www.ebay.co.uk/itm/285025256020 (UK)
https://www.ebay.com/itm/285025256020 (US)
Other countries just substitute ebay.co.uk with your local eBay. 

# Assembly
To assemble you will need to:
* Add jumpers to select the voltage, either 3.3v or 5v
* Add the 3-pin UPDI programming header and flash the firmware
* Connect your heater the 3-pin header
* Connect your I2C master device to the I2C pins (make sure you're using the right voltage for your I2C master)

# Programming
These boards are **not** programmed, so you will need to program the boards using a UPDI programmer.

The cheapest option is to use an Arduino as a UPDI programmer to flash the hex available here: [v0.1.0-beta release](https://github.com/TMakins/CDH_I2C_Interface/releases/download/v0.1.0-beta/firmware.hex).

# Pin Description
See pinout below, or schematic in the Schematic folder.

![interface board pinout](https://github.com/TMakins/CDH_I2C_Interface/blob/master/pinout.jpg?raw=true)