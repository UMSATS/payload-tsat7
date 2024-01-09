/*
 * tca9539.h
 *
 *  Created on: Dec 18, 2023
 *      Author: Jacob Petersen
 *
 *  Purpose: this is the driver file for the TCA9539 IO expander IC.
 */

#ifndef TCA9539_H_
#define TCA9539_H_

#include "power.h"

#include <stdbool.h>

typedef enum
{
	EXPANDER_1 = 0, // Wells 0-7.
	EXPANDER_2 = 1  // Wells 8-15.
} ExpanderID;

typedef enum
{
	EXPANDER_PIN_0 = 0,
	EXPANDER_PIN_1,
	EXPANDER_PIN_2,
	EXPANDER_PIN_3,
	EXPANDER_PIN_4,
	EXPANDER_PIN_5,
	EXPANDER_PIN_6,
	EXPANDER_PIN_7,
	EXPANDER_PIN_10,
	EXPANDER_PIN_11,
	EXPANDER_PIN_12,
	EXPANDER_PIN_13,
	EXPANDER_PIN_14,
	EXPANDER_PIN_15,
	EXPANDER_PIN_16,
	EXPANDER_PIN_17,
} ExpanderPinID;

/**
 * @brief Performs necessary initialisation of each pin.
 *
 * @return true on success. false on error.
 */
bool TCA9539_Init();

/**
 * @brief Gets the state of a pin on one of the expanders.
 *
 * @return 1 if set, 0 if unset. -1 on error.
 */
int TCA9539_Get_Pin(ExpanderID device, ExpanderPinID pin);

/**
 * @brief Sets or unsets a pin on one of the expanders.
 *
 * @return true on success. false on error.
 */
bool TCA9539_Set_Pin(ExpanderID device, ExpanderPinID pin, Power power);

/**
 * @brief Sets all pins to low.
 *
 * @return true on success. false on error.
 */
bool TCA9539_Clear_Pins();

#endif
