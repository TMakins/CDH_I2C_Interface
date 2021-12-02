* This schematic was not originally intended to be publicly published, so is far from the neatest
* The header J5 is intended to be pin compatible with the ESP32 dev board, but is upside down
* J9 and J8 can be used to add/remove the I2C pull ups as needed (if no other pull ups are already present)
* J1, J2 and J3 can be used to select between 5V and 3.3V I2C levels, where the SDA_LV and SCL_LV are the 3v3 levels
* Green LED is a power LED
* Red LED (firmware depending) is used to indicate that the board has succesfully connected to a heater. This is regardless of whether or not the board is connected to an I2C master.
