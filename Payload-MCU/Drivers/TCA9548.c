/*
 * TCA9548A.c
 *
 *  Created on: Nov 26, 2023
 *      Author: Jascha
 */


#include "i2c.h"

#define TCA9548_ADDR 0x70 				// I2C address of the multiplexer

// returns -1 on error
int TCA9548_set_i2c_channel(int channel_number) {

	if (channel_number < 0 || channel_number > 7) {
		return -1;
	}

	// create an array of 1 byte and copy the value in channel_number
	uint8_t command_register[1] = {channel_number};

	// usage: i2c object, device addr, payload, payload size (bytes), timeout (ms)
	HAL_I2C_Master_Transmit(&hi2c1, TCA9548_ADDR, command_register, 1, 100);

}
