/*
 * TCA9548A.h
 *
 *  Created on: Nov 26, 2023
 *      Author: Jacob Petersen
 *
 *  Purpose: This is the driver file for the I2C multiplexer, part # TCA9548.
 */

#ifndef TCA9548_H_
#define TCA9548_H_

#include <stdint.h>

/**
 * @brief Sets the I2C channel for the multiplexer.
 *
 * @return -1 on error. 0 on success.
 */
int TCA9548_set_i2c_channel(int channel_number);

#endif
