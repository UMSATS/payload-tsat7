/*
 * tca9548.h
 *
 *  Created on: Nov 26, 2023
 *      Author: Jacob Petersen, Logan Furedi
 *
 *  Purpose: This is the driver file for the I2C multiplexer, part # TCA9548.
 */

#ifndef HARDWAREPERIPHERALS_INC_TCA9548_H_
#define HARDWAREPERIPHERALS_INC_TCA9548_H_

#include <stdbool.h>

typedef enum {
	MUX_CHANNEL_0 = 0,
	MUX_CHANNEL_1,
	MUX_CHANNEL_2,
	MUX_CHANNEL_3,
	MUX_CHANNEL_4,
	MUX_CHANNEL_5,
} MuxChannel;

/**
 * @brief Sets the I2C channel for the multiplexer.
 *
 * @return true on success. false on error.
 */
bool TCA9548_Set_I2C_Channel(MuxChannel channel);

#endif /* HARDWAREPERIPHERALS_INC_TCA9548_H_ */
