/*
 * heater.cpp
 *
 * Created: 29/01/2021 21:23:54
 *  Author: Toby
 */ 

#include "heater.h"
#include <one_wire/one_wire.h>
#include <crc/crc.h>
#include <avr/io.h>
#include <avr/eeprom.h>

//#include <debug/debug.h>
#include <stdio.h>

#include <string.h>

#define PUMP_MODE_PRIME		0x5A
#define PUMP_MODE_NORMAL	0

#define COMMAND_START		0xA0
#define COMMAND_STOP		0x05
#define COMMAND_NORMAL		0

#define MODE_HZ				0xCD
#define MODE_TEMP			0x32

// Config A bit positions
#define CFG_A_HEATER_STORE	0 // store the value in heater eeprom, i.e. mimic LCD not rotary


enum act_state {
	HTR_OFF,
	HTR_ON,
	HTR_DISCONNECTED,
};

enum state {
	OFF,
	ON,
	PRIME
};

enum mode {
	FIXED_HZ,
	TEMPERATURE
};

enum run_state {
	STOPPED,
	STARTING,
	IGNITING,
	IGNITION_RETRY,
	IGNITED,
	RUNNING,
	STOPPING,
	POST_RUN,
	COOLING
};

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
		uint8_t config_a; //				34
	} settings;
} i2c_reg_t;

union i2c_regs {
	i2c_reg_t regs;
	uint8_t data[34];
} i2c_regs;

typedef struct tx_packet {
	uint8_t sof;
	uint8_t size;
	uint8_t command;
	uint8_t temp;
	uint8_t desired_temp;
	uint8_t min_pump_freq;
	uint8_t max_pump_freq;
	uint8_t min_fan_speed_msb;
	uint8_t min_fan_speed_lsb;
	uint8_t max_fan_speed_msb;
	uint8_t max_fan_speed_lsb;
	uint8_t voltage;
	uint8_t fan_sensor;
	uint8_t mode;
	uint8_t min_temp;
	uint8_t max_temp;
	uint8_t glow_plug_power;
	uint8_t pump_mode;
	uint8_t unknown_1;
	uint8_t unknown_2;
	uint8_t altitude_msb;
	uint8_t altitude_lsb;
	uint8_t crc_msb;
	uint8_t crc_lsb;
} tx_packet;

typedef struct rx_packet {
	uint8_t sof;
	uint8_t size;
	uint8_t run_state;
	uint8_t error_state;
	uint8_t supply_voltage_msb;
	uint8_t supply_voltage_lsb;
	uint8_t fan_speed_msb;
	uint8_t fan_speed_lsb;
	uint8_t fan_voltage_msb;
	uint8_t fan_voltage_lsb;
	uint8_t body_temp_msb;
	uint8_t body_temp_lsb;
	uint8_t glow_plug_voltage_msb;
	uint8_t glow_plug_voltage_lsb;
	uint8_t glow_plug_current_msb;
	uint8_t glow_plug_current_lsb;
	uint8_t pump_frequency;
	uint8_t error_code;
	uint8_t unknown_1;
	uint8_t requested_hz;
	uint8_t unknown_2;
	uint8_t unknown_3;
	uint8_t crc_msb;
	uint8_t crc_lsb;
} rx_packet;

union packet {
	rx_packet rx_data;
	tx_packet tx_data;
	uint8_t buff[24];
};

Heater::Heater() 
{
	// Set all to zero just to be sure
	memset(i2c_regs.data, 0, sizeof(i2c_regs.data));
	
	// Setup defaults
	// Heater control
	i2c_regs.regs.controller.control.state = 0;
	i2c_regs.regs.controller.control.mode = 1;
	i2c_regs.regs.controller.control.current_temp = 0;
	i2c_regs.regs.controller.control.desired_temp = 22;
	i2c_regs.regs.controller.control.desired_hz = 0;
	i2c_regs.regs.controller.control.altitude = 3500;
	// Heater config
	i2c_regs.regs.controller.config.min_pump_freq = 16;
	i2c_regs.regs.controller.config.max_pump_freq = 55;
	i2c_regs.regs.controller.config.min_fan_speed = 1680;
	i2c_regs.regs.controller.config.max_fan_speed = 4500;
	i2c_regs.regs.controller.config.operating_voltage = 120;
	i2c_regs.regs.controller.config.num_fan_magnets = 1;
	i2c_regs.regs.controller.config.glow_plug_power = 5;
	// Settings
	i2c_regs.regs.settings.config_a = (1 << CFG_A_HEATER_STORE) | 0;
	
	// Heater 
	i2c_regs.regs.heater.on = HTR_DISCONNECTED;
	
	// Setup LED as output
	PORTA.DIRSET = 1 << 1;
	PORTA.OUTCLR = 1 << 1;
}

uint8_t* Heater::get_i2c_regs()
{
	return i2c_regs.data;
}

uint8_t Heater::get_i2c_regs_len()
{
	return sizeof(i2c_regs.data);
}

void Heater::process_rx_packet(uint8_t* data)
{
	// Connected again
	_connected = 1;	
	// Set LED1 on
	PORTA.OUTSET = 1 << 1;
	
	packet packet;
	memcpy(packet.buff, data, 24);
	
	// Check CRC, discard packet if invalid
	uint16_t crc = generate_crc(data, 22);
	if(crc != (uint16_t)(packet.rx_data.crc_msb << 8 | packet.rx_data.crc_lsb))
	{
		return;
	}
	
	i2c_regs.regs.heater.on = (packet.rx_data.error_state == 1) ? HTR_ON : HTR_OFF; // 0x01 when running normally, 0 idle, others error states
	
	i2c_regs.regs.heater.run_state = packet.rx_data.run_state;
	
	i2c_regs.regs.heater.voltage = (packet.rx_data.supply_voltage_msb << 8) | (packet.rx_data.supply_voltage_lsb);
	
	i2c_regs.regs.heater.fan_rpm = (packet.rx_data.fan_speed_msb << 8) | (packet.rx_data.fan_speed_lsb);
	
	i2c_regs.regs.heater.fan_voltage = (packet.rx_data.fan_voltage_msb << 8) | (packet.rx_data.fan_voltage_lsb);
	
	i2c_regs.regs.heater.body_temp = (packet.rx_data.body_temp_msb << 8) | (packet.rx_data.body_temp_lsb);
	
	i2c_regs.regs.heater.glow_plug_voltage = (packet.rx_data.glow_plug_voltage_msb << 8) | (packet.rx_data.glow_plug_voltage_lsb);
	
	i2c_regs.regs.heater.glow_plug_current = (packet.rx_data.glow_plug_current_msb << 8) | (packet.rx_data.glow_plug_current_lsb);
	
	i2c_regs.regs.heater.current_pump_hz = packet.rx_data.pump_frequency;
	
	i2c_regs.regs.heater.error_code = packet.rx_data.error_state;
	
	i2c_regs.regs.heater.last_error = packet.rx_data.error_code;
	
	i2c_regs.regs.heater.requested_pump_hz = packet.rx_data.requested_hz;
}

uint8_t* Heater::prepare_tx_packet()
{	
	i2c_reg_t regs = i2c_regs.regs;
	
	static packet packet;
	
	// Fixed bytes
	packet.tx_data.sof = (regs.settings.config_a & (1 << CFG_A_HEATER_STORE)) ? 0x76 : 0x78;
	packet.tx_data.size = 0x16;
	packet.tx_data.min_temp = 0x08;
	packet.tx_data.max_temp = 0x23;
	packet.tx_data.unknown_1 = 0xEB;
	packet.tx_data.unknown_2 = 0x47;
	
	// Config stuff
	packet.tx_data.min_pump_freq = regs.controller.config.min_pump_freq;
	packet.tx_data.max_pump_freq = regs.controller.config.max_pump_freq;
	packet.tx_data.min_fan_speed_msb = (regs.controller.config.min_fan_speed >> 8) & 0xFF;
	packet.tx_data.min_fan_speed_lsb = regs.controller.config.min_fan_speed & 0xFF;
	packet.tx_data.max_fan_speed_msb = (regs.controller.config.max_fan_speed >> 8) & 0xFF;
	packet.tx_data.max_fan_speed_lsb = regs.controller.config.max_fan_speed & 0xFF;
	packet.tx_data.voltage = regs.controller.config.operating_voltage;
	packet.tx_data.fan_sensor = regs.controller.config.num_fan_magnets;
	packet.tx_data.glow_plug_power = regs.controller.config.glow_plug_power;
	
	// Turn off priming most of the time
	packet.tx_data.pump_mode = PUMP_MODE_NORMAL;
	
	
	// Prime fuel pump
	if(regs.controller.control.state == PRIME) 
	{
		if(regs.heater.run_state == STOPPED) 
		{
			// Only if not running
			packet.tx_data.pump_mode = PUMP_MODE_PRIME;
			
			// Defaults for everything else, so return early
			// (after CRC)
			uint16_t crc = generate_crc(packet.buff, 22);
			packet.tx_data.crc_msb = (crc >> 8) & 0xFF;
			packet.tx_data.crc_lsb = crc & 0xFF;
			return packet.buff;
		}
		else 
		{
			// Clear the register - we don't want to potentially wait hours (or however long before heater is off) before we pump diesel, probably unexpectedly..
			// This will turn the heater off next time round the loop
			regs.controller.control.state = OFF;
		}
	}
	
	// On/off state
	if(regs.controller.control.state == ON && regs.heater.on == HTR_OFF) 
	{
		packet.tx_data.command = COMMAND_START; // Stopped + should be running, so send start
	}
	else if(regs.controller.control.state == OFF
			&& regs.heater.run_state > STOPPED
			&& regs.heater.run_state < STOPPING) 
	{
				packet.tx_data.command = COMMAND_STOP; // Running + should be stopped, so send stop
	}
	else 
	{
		packet.tx_data.command = COMMAND_NORMAL; // Stay in same state
	}
	
	// Current temp, desired temp + fixed/stat mode
	if(regs.controller.control.mode == FIXED_HZ) 
	{
		// Fixed Hz mode, so request frequency based on min/max temp and pump frequency
		// NEED TO TEST THIS
		uint8_t min_p = regs.controller.config.min_pump_freq;
		uint8_t max_p = regs.controller.config.max_pump_freq;
		uint8_t desired_hz = regs.controller.control.desired_hz;
		// Magic nums are min and max heater temp (hardcoded values) + 0.5 to round to nearest int
		packet.tx_data.desired_temp = (uint8_t)((((float)(desired_hz - min_p) / (float)(max_p - min_p)) * (35.0 - 8.0)) + 8.5);
		
		packet.tx_data.mode = MODE_HZ;
		packet.tx_data.temp = 0; // fixed hz mode, actual temp forced to 0
	}
	else 
	{
		// Temp mode
		packet.tx_data.desired_temp = regs.controller.control.desired_temp;
		packet.tx_data.mode = MODE_TEMP;
		packet.tx_data.temp = regs.controller.control.current_temp;
	}
	
	// Altitude, not used by the heater but included anyway
	packet.tx_data.altitude_msb = (regs.controller.control.altitude >> 8) & 0xFF;
	packet.tx_data.altitude_lsb = regs.controller.control.altitude & 0xFF;
	
	// Finally, CRC`
	uint16_t crc = generate_crc(packet.buff, 22);
	packet.tx_data.crc_msb = (crc >> 8) & 0xFF;
	packet.tx_data.crc_lsb = crc & 0xFF;
	
	return packet.buff;
}

void Heater::disconnected()
{
	_connected = 0;
	i2c_regs.regs.heater.on = HTR_DISCONNECTED;
	// Set LED1 off
	PORTA.OUTCLR = 1 << 1;
}

void Heater::save_to_eeprom(save_reason_t reason)
{
	eeprom_busy_wait();
	eeprom_write_byte((uint8_t *)0, reason);
	eeprom_write_block(i2c_regs.data, (uint8_t *)1, sizeof(i2c_regs.data));
	eeprom_busy_wait();
}

Heater heater = Heater();