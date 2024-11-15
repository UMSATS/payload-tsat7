/*
 * tca9548.c
 *
 *  Created on: Nov 26, 2023
 *      Author: Jacob Petersen, Logan Furedi
 *
 *  Purpose: This is the driver file for the I2C multiplexer, part # TCA9548.
 */

#include "tca9548.h"
#include "assert.h"

#include "i2c.h"

#include <stdint.h>
#include <stdbool.h>
#include "tuk/debug/print.h"

static const uint16_t I2C_ADDRESS = 0x70 << 1;  // I2C address of the multiplexer
static const uint32_t TIMEOUT = 100;       // in ms

#define PRINT_SUBJECT "TCA9548"

bool TCA9548_Set_I2C_Channel(MuxChannel channel)
{
	ASSERT(MUX_CHANNEL_0 <= channel && channel <= MUX_CHANNEL_5, "invalid mux channel: %d.", channel);

	if (channel < MUX_CHANNEL_0 || channel > MUX_CHANNEL_5)
	{
		PRINT_ERROR("invalid mux channel: %d.", channel);
		//PUT_ERROR(ERR_PLD_TCA9548_INVALID_CHANNEL);
		return false;
	}

	// create an array of 1 byte and copy the value in channel_number
	uint8_t command_register[1] = {1 << channel};

	// usage: i2c object, device addr, payload, payload size (bytes), timeout (ms)
	HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, I2C_ADDRESS << 1, command_register, 1, TIMEOUT);

	if (status != HAL_OK)
	{
		PRINT_ERROR("failed to switch to I2C channel %d. (HAL error code: %d)", channel, status);
		//PUT_ERROR(ERR_I2C_TRANSMIT, status);
		return false;
	}

	return true;
}
