/*
 * heater.h
 *
 * Created: 29/01/2021 21:23:44
 *  Author: Toby
 */ 


#ifndef HEATER_H_
#define HEATER_H_

#include <stdint.h>

// The I2c registers, not what's sent to the heater
// Horribly nested but it'll do
typedef struct {
	struct {
		struct {
			uint8_t state; //				0			0 - off, 1 - on, 2 - prime (see enum)
			uint8_t mode; //				1			0 = fixed Hz, 1 = temp mode
			uint8_t current_temp; //		2			0 in Hz mode, otherwise 1C/digit
			uint8_t desired_temp; //		3			1C / digit
			uint8_t desired_hz; //			4			0.1Hz / digit
			uint16_t altitude; //			5,6			m
		} control;
		struct {
			uint8_t min_pump_freq; //		7			0.1Hz / digit
			uint8_t max_pump_freq; //		8			0.1Hz / digit
			uint16_t min_fan_speed; //		9, 10		1RPM / digit
			uint16_t max_fan_speed; //		11,12		1RPM / digit
			uint8_t operating_voltage; //	13			0.1V / digit
			uint8_t num_fan_magnets; //		14			1 or 2
			uint8_t glow_plug_power; //		15			1 - 6
		} config;
	} controller;
	struct {
		uint8_t on; //						16			0 - off, 1 - on, 2 - disconnected
		uint8_t run_state; //				17			0 - 8
		uint16_t voltage; //				18, 19		0.1V / digit
		uint16_t fan_rpm; //				20, 21		1RPM / digit
		uint16_t fan_voltage; //			22, 23		0.1V / digit
		uint16_t body_temp; //				24, 25		1C / digit
		uint16_t glow_plug_voltage; //		26, 27		0.1V / digit
		uint16_t glow_plug_current; //		28, 29		10mA / digit
		uint8_t current_pump_hz; //			30			0.1Hz / digit
		uint8_t requested_pump_hz; //		31			0.1Hz / digit
		uint8_t error_code;	//				32
		uint8_t last_error; //				33
	} heater;
	struct {
		uint8_t version; //					34
		uint8_t config_a; //				35
		uint8_t status_a; //				36
		uint8_t debug; //					37
	} settings;
} i2c_reg_t;

typedef union {
	i2c_reg_t regs;
	uint8_t data[38];
} i2c_reg_union_t;

extern i2c_reg_union_t i2c_regs;

typedef enum save_reason {
	WATCHDOG = 0,
	NO_CONNECTION,
} save_reason_t;

class Heater
{
	public:
	Heater();
	void init();
	void process_rx_packet(uint8_t* data);
	uint8_t* prepare_tx_packet();
	void disconnected();
	void save_to_eeprom(save_reason_t reason);
	
	uint8_t _connected;
	uint8_t _force_stop;
	uint8_t _last_run_state;
};

extern Heater heater;

#endif /* HEATER_H_ */