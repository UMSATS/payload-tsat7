/*
 * pcb_sensor.c
 *
 *  Created on: Dec 6, 2023
 *      Author: Logan Furedi, Jacob Petersen
 */

#include "pcb_sensor.h"
#include "assert.h"
#include "log.h"

#include "adc.h"

#include <stdint.h>
#include <stdbool.h>

static const uint32_t TIMEOUT = 100; // in ms

#define LOG_SUBJECT "PCB Sensor"

bool PCB_Sensor_Read_Temp(uint16_t *out)
{
	// perform self-calibration.
	HAL_StatusTypeDef status;
	status = HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to calibrate. (HAL error code: %d)", status);
		return false;
	}

	// start conversion.
	status = HAL_ADC_Start(&hadc1);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to start conversion. (HAL error code: %d)", status);
		return false;
	}

	status = HAL_ADC_PollForConversion(&hadc1, TIMEOUT);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to complete conversion. (HAL error code: %d)", status);
		return false;
	}

	// get reading.
	uint32_t temp = HAL_ADC_GetValue(&hadc1);

	status = HAL_ADC_Stop(&hadc1);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to complete conversion. (HAL error code: %d)", status);
		return false;
	}

	// QUESTION: what purpose does this serve?
	*out = (int16_t)(temp & 0xFFF); // 12 bits, pad 0s

	return true;
}
