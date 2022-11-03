/*
 * i2c.h
 *
 * Created: 31/01/2021 17:46:36
 *  Author: Toby
 */ 


#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>

class I2C
{
	public:
	I2C();
	void init(uint8_t *i2c_reg, uint8_t reg_size);
};

#endif /* I2C_H_ */