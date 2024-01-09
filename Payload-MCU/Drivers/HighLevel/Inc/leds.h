/*
 * leds.h
 *
 *  Created on: Jan 7, 2024
 *      Author: Logan Furedi
 */

#ifndef HIGHLEVEL_INC_LEDS_H_
#define HIGHLEVEL_INC_LEDS_H_

#include "well_id.h"

#include <stdbool.h>

/**
 * @brief Sets the power of an LED in a specific well on or off.
 * @return true on success. false on error.
 */
bool LEDs_Set_LED(WellID well_id, bool power);

#endif /* HIGHLEVEL_INC_LEDS_H_ */
