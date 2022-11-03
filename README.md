# Chinese Diesel Heater interface module
This repo contains the files for a small module designed to interface between a Chinese diesel heater and any microcontroller (either 5v or 3.3v) using I2C. This opens up the possibility of controlling a diesel heater with, for example, a Raspberry Pi, Arduino or ESP32. 

## Hardware 

### Availability
A small number of modules have been made. These do not include header pins and are **not** programmed:
* https://www.ebay.co.uk/itm/285025256020 (UK)
* https://www.ebay.com/itm/285025256020 (US)  

Other countries just substitute ebay.co.uk with your local eBay. 

### Programming
These boards are **not** programmed, so you will need to program the boards using a UPDI programmer (see pinout for UPDI header).

A good option is to use an Arduino as a UPDI programmer to flash the hex available here: [v0.1.0-beta release](https://github.com/TMakins/CDH_I2C_Interface/releases/download/v0.1.0-beta/firmware.hex).

### Board Description
See pinout below, or schematic in the Schematic folder.

Note: NC = Not Connected

![interface board pinout](https://github.com/TMakins/CDH_I2C_Interface/blob/main/pinout.jpg?raw=true)

#### I2C Voltage Selection
You must select the voltage used for I2C (5V or 3.3V) to match the microcontroller you are using by adding jumpers on J1, J2 and J3 (red/purple in the pinout above). Not putting jumpers here will result in I2C not working. 

#### I2C Pullups
If you don't have any other I2C devices, it's likely that there are no terminating pullup resistors connected - adding jumpers here (light blue in the pinout above) enables the required pullups. 

#### NodeMCU/ESP Header
One side of the board mirrors a standard NodeMCU development board, so that it can be soldered directly to one. Check the pinout matches your board. 

### Voltage Regulator
Input voltage is supplied by the heater at 5V. A regulator is fitted to the board to convert this to 3.3V, which is then output on the 3.3V pins. There is no need to supply the board with power besides through the heater header, or through the programming header when initially programming. 

The output pins (3.3V and 5V) can be used to power the microcontroller you are connecting to this module. 

## Software

### Notes
* The firmware appears to work, but has not been extensively tested.
* The I2C API hasn't yet been documented, but an Arduino library has been created here: https://github.com/TMakins/CDHInterface

### Compatibility
It's getting increasingly difficult to determine if a Chinese Diesel heater uses the protocol used here, often referred to as the "Blue Wire" protocol, as initially reverse engineered by Ray Jones. However, Ray has put a lot of working into documenting which heaters are compatible with his Afterburner controller, so it's worth looking at [his site](http://www.mrjones.id.au/afterburner/) (or ideally just buying one of his controllers!). If your heater is compatible with the Afterburner, it will be compatible with the software in this repo. 

## License
There is no license, everything here is open source, do what you want with it. 

## Credit
This is based on the initial protocol research done by Ray Jones, who now makes the excellent Afterburner controller for these heaters. 

## Disclaimer
If you use this software, whether running on hardware I've made or on hardware you've made yourself, and it causes damage of any form (including to your heater, to you, to the hardware) I am not responsible. 
