/*
 * thermistors.c
 *
 *  Created on: Jan 8, 2024
 *      Author: Logan Furedi
 */

#include "thermistors.h"
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
		{ MUX_CHANNEL_3, ADC_A0 }, // THERM 0
		{ MUX_CHANNEL_3, ADC_A1 }, // THERM 1
		{ MUX_CHANNEL_3, ADC_A2 }, // THERM 2
		{ MUX_CHANNEL_3, ADC_A4 }, // THERM 3
		{ MUX_CHANNEL_3, ADC_A5 }, // THERM 4
		{ MUX_CHANNEL_3, ADC_A6 }, // THERM 5
		{ MUX_CHANNEL_3, ADC_A7 }, // THERM 6
		{ MUX_CHANNEL_5, ADC_A0 }, // THERM 7
		{ MUX_CHANNEL_0, ADC_A0 }, // THERM 8
		{ MUX_CHANNEL_0, ADC_A1 }, // THERM 9
		{ MUX_CHANNEL_0, ADC_A2 }, // THERM 10
		{ MUX_CHANNEL_0, ADC_A4 }, // THERM 11
		{ MUX_CHANNEL_0, ADC_A5 }, // THERM 12
		{ MUX_CHANNEL_0, ADC_A6 }, // THERM 13
		{ MUX_CHANNEL_0, ADC_A7 }, // THERM 14
		{ MUX_CHANNEL_2, ADC_A0 }, // THERM 15
};

#define LOG_SUBJECT "Thermistors"

bool Thermistors_Get_Temp(WellID well_id, uint16_t *out)
{
	ASSERT(WELL_0 <= well_id && well_id <= WELL_15, "invalid well id: %d.", well_id);

	if (well_id < WELL_0 || well_id > WELL_15)
	{
		LOG_ERROR("invalid well id: %d.", well_id);
		return false;
	}

	bool success = TCA9548_Set_I2C_Channel(ADC_LOCATIONS[well_id].channel);
	if (!success)
	{
		LOG_ERROR("failed to read temperature in well %d: could not switch channel.", well_id);
		return false;
	}

	uint16_t data;
	HAL_StatusTypeDef status;
	status = HAL_I2C_Master_Receive(&hi2c1, ADC_LOCATIONS[well_id].address,
			(uint8_t *)&data, sizeof(data), TIMEOUT);

	if (status != HAL_OK)
	{
		LOG_ERROR("failed to read temperature in well %d. (HAL error code: %d)", well_id, status);
		return false;
	}

	*out = data;

	return true;
}
