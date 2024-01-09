/*
 * photocells.c
 *
 *  Created on: Jan 8, 2024
 *      Author: Logan Furedi
 */

#include "photocells.h"
#include "well_id.h"
#include "mux_adc_location.h"
#include "tca9548.h"
#include "assert.h"
#include "log.h"

#include "i2c.h"

#include <stdint.h>
#include <stdbool.h>

static const uint32_t TIMEOUT = 100; // in ms

static const MuxADCLocation ADC_LOCATIONS[] = {
		{ MUX_CHANNEL_4, ADC_A0 }, // PHOTOCELL 0
		{ MUX_CHANNEL_4, ADC_A1 }, // PHOTOCELL 1
		{ MUX_CHANNEL_4, ADC_A2 }, // PHOTOCELL 2
		{ MUX_CHANNEL_4, ADC_A4 }, // PHOTOCELL 3
		{ MUX_CHANNEL_4, ADC_A5 }, // PHOTOCELL 4
		{ MUX_CHANNEL_4, ADC_A6 }, // PHOTOCELL 5
		{ MUX_CHANNEL_4, ADC_A7 }, // PHOTOCELL 6
		{ MUX_CHANNEL_5, ADC_A1 }, // PHOTOCELL 7
		{ MUX_CHANNEL_1, ADC_A0 }, // PHOTOCELL 8
		{ MUX_CHANNEL_1, ADC_A1 }, // PHOTOCELL 9
		{ MUX_CHANNEL_1, ADC_A2 }, // PHOTOCELL 10
		{ MUX_CHANNEL_1, ADC_A4 }, // PHOTOCELL 11
		{ MUX_CHANNEL_1, ADC_A5 }, // PHOTOCELL 12
		{ MUX_CHANNEL_1, ADC_A6 }, // PHOTOCELL 13
		{ MUX_CHANNEL_1, ADC_A7 }, // PHOTOCELL 14
		{ MUX_CHANNEL_2, ADC_A1 }, // PHOTOCELL 15
};

#define LOG_SUBJECT "Photocells"

bool Photocells_Get_Light_Level(WellID well_id, uint16_t *out)
{
	ASSERT(WELL_1 <= well_id && well_id <= WELL_16, "invalid well id: %d.", well_id);

	if (well_id < WELL_1 || well_id > WELL_16)
	{
		LOG_ERROR("invalid well id: %d.", well_id);
		return false;
	}

	bool success = TCA9548_Set_I2C_Channel(ADC_LOCATIONS[well_id].channel);
	if (!success)
	{
		LOG_ERROR("failed to read light level in well %d: could not switch channel.", well_id);
		return false;
	}

	uint16_t data;
	HAL_StatusTypeDef status;
	status = HAL_I2C_Master_Receive(&hi2c1, ADC_LOCATIONS[well_id].address,
			(uint8_t *)&data, sizeof(data), TIMEOUT);

	if (status != HAL_OK)
	{
		LOG_ERROR("failed to read light level in well %d. (HAL error code: %d)", well_id, status);
		return false;
	}

	*out = data;

	return true;
}
