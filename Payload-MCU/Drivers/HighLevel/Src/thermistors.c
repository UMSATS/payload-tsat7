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
#include "utils.h"
#include "assert.h"
#include "log.h"

// remove later
#include "power.h"
#include "tca9539.h"

#include "i2c.h"

#include <stdint.h>
#include <stdbool.h>

static const uint32_t TIMEOUT = 100; // in ms

static const uint16_t ADC_MAX_OUTPUT = 4095;

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

	uint8_t data[2];
	HAL_StatusTypeDef status;
	status = HAL_I2C_Master_Receive(&hi2c1,
			ADC_LOCATIONS[well_id].address, data, sizeof(data), TIMEOUT);

	if (status != HAL_OK)
	{
		LOG_ERROR("failed to read temperature in well %d. (HAL error code: %d)", well_id, status);
		return false;
	}

	uint16_t temp = utils_be_to_native_16(data); // convert from BE to LE.

	*out = temp;

	return true;
}

bool Thermistors_Get_Temp_Celsius(WellID well_id, double *out)
{
	bool success;

	uint16_t adc_value;
	success = Thermistors_Get_Temp(well_id, &adc_value);

	double normalised_value = (double)adc_value / (double)ADC_MAX_OUTPUT;

	return true;
}

void Thermistors_Print_Debug_Info()
{
	uint16_t therm_data[16];
	uint16_t light_data[16];

	uint16_t temp;

	for (int i = 0; i < 16; i++)
	{
		Thermistors_Get_Temp(i, &temp);
		therm_data[i] = temp;

		Photocells_Get_Light_Level(i, &temp);
		light_data[i] = temp;
	}

	LOG_INFO("_________________________");
	LOG_INFO("| WELL  | TEMPS | LIGHT |");
	LOG_INFO("|-----------------------|");
	for (int i = WELL_0; i <= WELL_15; i++)
	{
		LOG_INFO("| %6d| %6d| %6d|", i, therm_data[i], light_data[i]);
	}
	LOG_INFO("-------------------------");
}
