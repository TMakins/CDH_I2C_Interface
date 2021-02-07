/*
 * i2c.cpp
 *
 * Created: 31/01/2021 17:45:46
 *  Author: Toby
 */ 

#include "i2c.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include <stdio.h>
//#include <debug/debug.h>

#define SLAVE_ADDR	0x32

enum twi_state_t {
	IDLE,
	MASTER_READ,
	MASTER_WRITE,
	MASTER_WRITE_ADDR,
	MASTER_WRITE_ADDR_DONE
	};

twi_state_t twi_state = IDLE;

uint8_t _reg_index = 0;
uint8_t *_reg_data;
uint8_t _reg_len;

I2C::I2C()
{
}

void I2C::init(uint8_t *i2c_reg, uint8_t reg_size) 
{
	_reg_index = 0;
	_reg_data = i2c_reg;
	_reg_len = reg_size;
	
	// 7 bit address + general call
	TWI0.SADDR = (SLAVE_ADDR << 1) | 1;
	// Enable master for bus error detection
	TWI0.MCTRLA = TWI_ENABLE_bm;
	// Data interrupt, address/stop interrupt, smart mode, slave enable
	TWI0.SCTRLA = TWI_DIEN_bm | TWI_APIEN_bm | TWI_PIEN_bm | TWI_ENABLE_bm;
}

ISR(TWI0_TWIS_vect)
{
	// Address or stop interrupt
	if(TWI0.SSTATUS & TWI_APIF_bm)
	{
		// Address interrupt
		if(TWI0.SSTATUS & TWI_AP_bm)
		{
			// Buss error or collision
			if(TWI0.SSTATUS & (TWI_BUSERR_bm | TWI_COLL_bm)) 
			{
				// We're not ready to read/write
				twi_state = IDLE;
				// Nack and wait for new start condition
				TWI0.SCTRLB = TWI_ACKACT_bm | TWI_SCMD_COMPTRANS_gc;
				return;
			}
		
			// Master read, slave write
			if(TWI0.SSTATUS & TWI_DIR_bm)
			{
				// Read data must be a repeated start either after a write (so we have an address to read from)
				// or possibly just after a read (read -> repeated start -> read)
				// nack and wait for a new start
				
				if(twi_state == MASTER_READ || twi_state == MASTER_WRITE_ADDR_DONE)
				{
					// Repeated start
					twi_state = MASTER_READ;
					// Ack and wait for data interrupt
					TWI0.SCTRLB = TWI_SCMD_RESPONSE_gc;
					return;	
				}
				else 
				{
					// Read without writing a reg address to read from, so nack
					TWI0.SCTRLB = TWI_ACKACT_bm | TWI_SCMD_COMPTRANS_gc;
					return;
				}
			}
			// Master write
			else 
			{
				/*
				if(twi_state == MASTER_WRITE || twi_state == MASTER_WRITE_ADDR || twi_state == MASTER_WRITE_ADDR_DONE)
				{
					// Repeated start no change in direction, unexpected but legal
					// Don't change TWI state
					// Ack and wait for data interrupt
					TWI0.SCTRLB = TWI_SCMD_RESPONSE_gc;
					return;
				}
				else
				{
					*/
					// Either repeated start with switch from read to write, or normal start
					// Either way, wait for reg address write
					twi_state = MASTER_WRITE_ADDR;
					// Ack and wait for data interrupt (no need to write to ctrl B as we're using smart mode)
					TWI0.SCTRLB = TWI_SCMD_RESPONSE_gc;
					return;
					
				//}
			}
		}
		// Stop condition
		else 
		{
			//twi_state = IDLE;
			
			// Don't ack or nack, just clear interrupt flag
			TWI0.SSTATUS = TWI_APIF_bm;
		}
	}
	
	// Data interrupt
	if(TWI0.SSTATUS & TWI_DIF_bm)
	{
		// Write address
		if(twi_state == MASTER_WRITE_ADDR)
		{
			twi_state = MASTER_WRITE_ADDR_DONE;
			_reg_index = TWI0.SDATA;
			// Ack and wait for interrupt
			TWI0.SCTRLB = TWI_SCMD_RESPONSE_gc;
			return;
		}
		
		// Write data
		if(twi_state == MASTER_WRITE_ADDR_DONE || twi_state == MASTER_WRITE)
		{
			twi_state = MASTER_WRITE;
			
			// Check pointer bounds
			if(_reg_index >= _reg_len)
			{
				// Overflow, nack and wait for new start
				TWI0.SCTRLB = TWI_ACKACT_bm | TWI_SCMD_COMPTRANS_gc;
			}
			
			// Write data to reg and increment pointer
			_reg_data[_reg_index++] = TWI0.SDATA;
			// Ack and wait for interrupt
			TWI0.SCTRLB = TWI_SCMD_RESPONSE_gc;
			return;
		}
		
		// Read data
		if(twi_state == MASTER_READ)
		{
			// Check pointer bounds
			if(_reg_index >= _reg_len)
			{
				// Overflow, nack and wait for new start
				TWI0.SCTRLB = TWI_ACKACT_bm | TWI_SCMD_COMPTRANS_gc;
			}
			
			// Read data and increment pointer
			TWI0.SDATA = _reg_data[_reg_index++];
			// Send and wait for another byte
			TWI0.SCTRLB = TWI_SCMD_RESPONSE_gc;
			return;
		}
	}
	
	// We've fallen out the bottom somehow, nack and wait for new start condition
	TWI0.SCTRLB = TWI_ACKACT_bm | TWI_SCMD_COMPTRANS_gc;
}