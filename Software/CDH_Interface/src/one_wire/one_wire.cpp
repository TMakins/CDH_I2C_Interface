/*
 * one_wire.cpp
 *
 * Created: 29/01/2021 17:51:03
 *  Author: Toby
 */ 

#include "one_wire.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <timer/timer.h>
#include <heater/heater.h>
#include <string.h>

#define OW_USART		USART0
#define OW_TX_PORT		PORTB
#define OW_TX_PINCTRL	PIN2CTRL
#define OW_RX_ISR_VECT	USART0_RXC_vect

#define RX_BUFF_SIZE	128

uint8_t _rx_data[RX_BUFF_SIZE];
uint8_t _rx_counter = 0;

OneWire::OneWire()
{
}

void OneWire::init(uint32_t baud_rate)
{	
	OW_TX_PORT.OW_TX_PINCTRL = PORT_PULLUPEN_bm;
	// Set baud rate to 115200, with sigrow error correction
	int8_t  sigrow_value = SIGROW.OSC20ERR3V;
	int32_t baud = (F_CPU * 64) / (baud_rate * 16);
	baud *= (1024 + sigrow_value);
	baud /= 1024;
	OW_USART.BAUD = (int16_t)baud;
	
	// Set loop back mode on RXD and TXD (i.e. make one-wire), enable rx interrupts
	OW_USART.CTRLA = USART_LBME_bm | USART_RXCIE_bm;
	// Enable transmision and reception buffers, and enable open drain
	OW_USART.CTRLB = USART_TXEN_bm | USART_RXEN_bm | USART_ODME_bm;
	
	_rx_counter = 0;
}

uint8_t OneWire::available()
{
	return _rx_counter;
}

// Reads the most recent bytes, rather than the first, and resets buffer
void OneWire::read(uint8_t *buff, uint8_t len) 
{
	if(len > _rx_counter)
		return;
		
	memcpy(buff, &_rx_data[_rx_counter - len], len);
	
	_rx_counter = 0;	
}

uint32_t start_time = 0;
void OneWire::write(uint8_t *buff, uint8_t len) 
{
	_tx_reset();
	_rx_disable();
	for(uint8_t i = 0; i < len; i++) 
	{
		start_time = timer.millis();
		while (!_tx_ready())
		{
			if(timer.millis() - start_time > 200)
				reset();
		}
		OW_USART.TXDATAL = buff[i];
	}
	start_time = timer.millis();
	while (!_tx_complete())
	{
		if(timer.millis() - start_time > 200)
			reset();
	}
	_rx_enable();
}

void OneWire::clear_rx()
{
	_rx_counter = 0;
	// Flush buffer
	while (OW_USART.RXDATAH  & USART_RXCIF_bm)
	{
		OW_USART.RXDATAL;
	}
}

void OneWire::_tx_reset()
{
	OW_USART.STATUS = USART_TXCIF_bm;
}

bool OneWire::_tx_ready() 
{
	return (OW_USART.STATUS  & USART_DREIF_bm);
}

void OneWire::_tx_disable()
{
	OW_USART.CTRLB &= ~USART_TXCIF_bm;
}

void OneWire::_rx_disable()
{
	OW_USART.CTRLB &= ~USART_RXCIF_bm;
}

void OneWire::_rx_enable()
{
	OW_USART.CTRLB |= USART_RXEN_bm;
}

bool OneWire::_tx_complete()
{
	return (OW_USART.STATUS & USART_TXCIF_bm);
}

void OneWire::reset() 
{
	i2c_regs.regs.settings.debug++;
	
	// Reset counter
	_rx_counter = 0;
	// Flush buffer
	while (OW_USART.RXDATAH  & USART_RXCIF_bm)
	{
		OW_USART.RXDATAL;
	}	
	// Disable rx and tx to clear interrupts
	_rx_disable();
	_tx_disable();
	// Delay for good measure
	_delay_ms(100);
	// Re-enable rx
	_rx_enable();
}


ISR(OW_RX_ISR_VECT)
{
	if(_rx_counter < RX_BUFF_SIZE)
		_rx_data[_rx_counter++] = OW_USART.RXDATAL;
	else
		OW_USART.RXDATAL; // read to clear
}