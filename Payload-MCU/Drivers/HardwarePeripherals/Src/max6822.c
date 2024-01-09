/*
 * max6822.c
 *
 *  Created on: Jan 9, 2024
 *      Author: Logan Furedi
 *
 *  Purpose: The driver file for the Watchdog.
 */

#include "gpio.h"

void MAX6822_Reset_Timer()
{
	HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_11);
}

void MAX6822_Manual_Reset()
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
}
