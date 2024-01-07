/*
 * tca9548.c
 *
 *  Created on: Nov 26, 2023
 *      Author: Jacob Petersen
 *
 *  Purpose: This is the driver file for the I2C multiplexer, part # TCA9548.
 */

#include "tca9548.h"

#include "i2c.h"

#include <stdint.h>
#include <assert.h>

static const uint16_t TCA9548_ADDR = 0x70; // I2C address of the multiplexer
static const uint32_t TIMEOUT = 100;       // in ms

/**
 * @brief Sets the I2C channel for the multiplexer.
 *
 * @return -1 on error. 0 on success.
 */
int tca9548_set_i2c_channel(TCA9548Channel channel)
{
	ASSERT(TCA9548_CHANNEL_0 <= channel && channel <= TCA9548_CHANNEL_5,
			"%d is not a valid channel number.", channel);

	if (channel < TCA9548_CHANNEL_0 || channel > TCA9548_CHANNEL_5)
	{
		return -1;
	}

	// create an array of 1 byte and copy the value in channel_number
	uint8_t command_register[1] = {channel};

	// usage: i2c object, device addr, payload, payload size (bytes), timeout (ms)
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, TCA9548_ADDR, command_register, 1, TIMEOUT);

	if (status != HAL_OK)
	{
		return -1;
	}

	return 0;
}
