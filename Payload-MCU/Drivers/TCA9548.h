/*
 * tca9548.h
 *
 *  Created on: Nov 26, 2023
 *      Author: Jacob Petersen
 *
 *  Purpose: This is the driver file for the I2C multiplexer, part # TCA9548.
 */

#ifndef TCA9548_H_
#define TCA9548_H_

typedef enum {
	TCA9548_CHANNEL_0 = 0,
	TCA9548_CHANNEL_1,
	TCA9548_CHANNEL_2,
	TCA9548_CHANNEL_3,
	TCA9548_CHANNEL_4,
	TCA9548_CHANNEL_5,
} TCA9548Channel;

/**
 * @brief Sets the I2C channel for the multiplexer.
 *
 * @return -1 on error. 0 on success.
 */
int tca9548_set_i2c_channel(TCA9548Channel channel);

#endif /* TCA9548_H_ */
