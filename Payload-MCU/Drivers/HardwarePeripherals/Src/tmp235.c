/*
 * tmp235.c
 *
 *  Created on: Dec 6, 2023
 *      Author: Logan Furedi, Jacob Petersen
 */

#include "tmp235.h"
#include "assert.h"
#include "log.h"
#include "adc.h"
#include "error_context.h"

#include <stdint.h>
#include <stdbool.h>

static const uint32_t TIMEOUT = 100; // in ms

#define LOG_SUBJECT "PCB Sensor"

bool TMP235_Read_Temp(uint16_t *out)
{
	// perform self-calibration.
	HAL_StatusTypeDef status;
	status = HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to calibrate. (HAL error code: %d)", status);
		PUSH_ERROR(ERROR_ADC_CALIBRATION_START, status);
		return false;
	}

	// start conversion.
	status = HAL_ADC_Start(&hadc1);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to start conversion. (HAL error code: %d)", status);
		PUSH_ERROR(ERROR_ADC_START, status);
		return false;
	}

	status = HAL_ADC_PollForConversion(&hadc1, TIMEOUT);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to complete conversion. (HAL error code: %d)", status);
		PUSH_ERROR(ERROR_ADC_POLL, status);
		return false;
	}

	// get reading.
	uint32_t temp = HAL_ADC_GetValue(&hadc1);

	status = HAL_ADC_Stop(&hadc1);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to complete conversion. (HAL error code: %d)", status);
		PUSH_ERROR(ERROR_ADC_GET_VALUE, status);
		return false;
	}

	// QUESTION: what purpose does this serve?
	*out = (int16_t)(temp & 0xFFF); // 12 bits, pad 0s

	return true;
}
