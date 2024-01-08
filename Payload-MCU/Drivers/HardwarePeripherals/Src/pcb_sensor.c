/*
 * pcb_sensor.c
 *
 *  Created on: Dec 6, 2023
 *      Author: Logan Furedi, Jacob Petersen
 */

#include "pcb_sensor.h"
#include "log.h"

#include "adc.h"

#include <stdint.h>

// how long to wait for a reading before trying again
static const uint32_t TIMEOUT = 1; // in ms

// how many times to try getting a reading before giving up
static const unsigned short MAX_TRIES = 100;

int16_t PCB_Sensor_Read_Temp()
{
	int16_t temp; // return value.

	// Perform self-calibration.
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);

	// Start conversion.
	HAL_ADC_Start(&hadc1);

	// Poll for conversion with timeout.
	HAL_StatusTypeDef status = HAL_ADC_PollForConversion(&hadc1, TIMEOUT);

	unsigned short i = 0;
	while (status != HAL_OK && i < MAX_TRIES)
	{
		status = HAL_ADC_PollForConversion(&hadc1, TIMEOUT);
		i++;
	}

	if (status == HAL_OK)
	{
		// get reading.
		uint32_t adcValue = HAL_ADC_GetValue(&hadc1);

		HAL_ADC_Stop(&hadc1);

		temp = (int16_t)(adcValue & 0xFFF); // 12 bits, pad 0s
	}
	else
	{
		// fail gracefully.
		LOG_ERROR("failed to read temperature from PCB ADC.");
		temp = -1;
	}

	return temp;
}
