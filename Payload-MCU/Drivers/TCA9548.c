/*
 * TCA9548A.c
 *
 *  Created on: Nov 26, 2023
 *      Author: Jacob Petersen
 *
 *  Purpose: This is the driver file for the I2C multiplexer, part # TCA9548.
 */

#include "TCA9548.h"
#include "i2c.h"

#include <stdint.h>

#define TCA9548_ADDR 0x70 				// I2C address of the multiplexer

// returns -1 on error
int TCA9548_set_i2c_channel(int channel_number)
{
	if (channel_number < 0 || channel_number > 7)
	{
		return -1;
	}

	// create an array of 1 byte and copy the value in channel_number
	uint8_t command_register[1] = {channel_number};

	// usage: i2c object, device addr, payload, payload size (bytes), timeout (ms)
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, TCA9548_ADDR, command_register, 1, 100);

	if (status != HAL_OK)
	{
		return -1;
	}

	return 0;
}
