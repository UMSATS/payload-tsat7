/*
 * temp.c
 *
 *  Created on: Dec 29, 2023
 *      Author: Logan Furedi
 */
#include "wells.h"

#include "tca9548.h"
#include "assert.h"

#include "adc.h"
#include "i2c.h"

#include <stdint.h>
#include <stdbool.h>
#include <utils.h>

static const uint32_t TIMEOUT = 100; // in ms

// I2C addresses for thermistor ADCs & light sensors.
typedef enum {
	I2C_A0 = 0b1001000,
	I2C_A1 = 0b1001001,
	I2C_A2 = 0b1001010,
//	I2C_A3 = 0b1001011, // NOTE: unused.
	I2C_A4 = 0b1001100,
	I2C_A5 = 0b1001101,
	I2C_A6 = 0b1001110,
	I2C_A7 = 0b1001111,
} I2CAddress;

typedef struct
{
	TCA9548Channel channel;
	I2CAddress address;
} Location;

const Location THERM_LOCATIONS[] = {
		{ TCA9548_CHANNEL_3, I2C_A0 }, // THERM 0
		{ TCA9548_CHANNEL_3, I2C_A1 }, // THERM 1
		{ TCA9548_CHANNEL_3, I2C_A2 }, // THERM 2
		{ TCA9548_CHANNEL_3, I2C_A4 }, // THERM 3
		{ TCA9548_CHANNEL_3, I2C_A5 }, // THERM 4
		{ TCA9548_CHANNEL_3, I2C_A6 }, // THERM 5
		{ TCA9548_CHANNEL_3, I2C_A7 }, // THERM 6
		{ TCA9548_CHANNEL_5, I2C_A0 }, // THERM 7
		{ TCA9548_CHANNEL_0, I2C_A0 }, // THERM 8
		{ TCA9548_CHANNEL_0, I2C_A1 }, // THERM 9
		{ TCA9548_CHANNEL_0, I2C_A2 }, // THERM 10
		{ TCA9548_CHANNEL_0, I2C_A4 }, // THERM 11
		{ TCA9548_CHANNEL_0, I2C_A5 }, // THERM 12
		{ TCA9548_CHANNEL_0, I2C_A6 }, // THERM 13
		{ TCA9548_CHANNEL_0, I2C_A7 }, // THERM 14
		{ TCA9548_CHANNEL_2, I2C_A0 }, // THERM 15
};

const Location SENSOR_LOCATIONS[] = {
		{ TCA9548_CHANNEL_4, I2C_A0 }, // SENSOR 0
		{ TCA9548_CHANNEL_4, I2C_A1 }, // SENSOR 1
		{ TCA9548_CHANNEL_4, I2C_A2 }, // SENSOR 2
		{ TCA9548_CHANNEL_4, I2C_A4 }, // SENSOR 3
		{ TCA9548_CHANNEL_4, I2C_A5 }, // SENSOR 4
		{ TCA9548_CHANNEL_4, I2C_A6 }, // SENSOR 5
		{ TCA9548_CHANNEL_4, I2C_A7 }, // SENSOR 6
		{ TCA9548_CHANNEL_5, I2C_A1 }, // SENSOR 7
		{ TCA9548_CHANNEL_1, I2C_A0 }, // SENSOR 8
		{ TCA9548_CHANNEL_1, I2C_A1 }, // SENSOR 9
		{ TCA9548_CHANNEL_1, I2C_A2 }, // SENSOR 10
		{ TCA9548_CHANNEL_1, I2C_A4 }, // SENSOR 11
		{ TCA9548_CHANNEL_1, I2C_A5 }, // SENSOR 12
		{ TCA9548_CHANNEL_1, I2C_A6 }, // SENSOR 13
		{ TCA9548_CHANNEL_1, I2C_A7 }, // SENSOR 14
		{ TCA9548_CHANNEL_2, I2C_A1 }, // SENSOR 15
};

uint16_t Wells_Get_Temperature(WellID well_id)
{
	ASSERT(WELL_1 <= well_id && well_id <= WELL_16, "%d is not a valid well ID.", well_id);

	uint16_t temp;

	tca9548_set_i2c_channel(THERM_LOCATIONS[well_id].channel);
	HAL_I2C_Master_Receive(&hi2c1, THERM_LOCATIONS[well_id].address,
			(uint8_t *)&temp, sizeof(temp), TIMEOUT);

	return temp;
}

uint16_t Wells_Get_Light_Level(WellID well_id)
{
	ASSERT(WELL_1 <= well_id && well_id <= WELL_16, "%d is not a valid well ID.", well_id);

	uint16_t light;

	tca9548_set_i2c_channel(SENSOR_LOCATIONS[well_id].channel);
	HAL_I2C_Master_Transmit(&hi2c1, SENSOR_LOCATIONS[well_id].address,
			(uint8_t *)&light, sizeof(light), TIMEOUT);

	return light;
}


