/*
 * photocells.h
 *
 *  Created on: Jan 8, 2024
 *      Author: Logan Furedi
 */

#ifndef HIGHLEVEL_INC_PHOTOCELLS_H_
#define HIGHLEVEL_INC_PHOTOCELLS_H_

#include "well_id.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 	Reads the current light conditions of a well via its corresponding
 *        	ADC unit.
 *
 * @param well_id 	The well to retrieve sensor data from.
 * @param out 		Where to store the raw reading from an MCP3221 ADC unit.
 * @return 			true on success. false on error.
 */
bool Photocells_Get_Light_Level(WellID well_id, uint16_t *out);

#endif /* HIGHLEVEL_INC_PHOTOCELLS_H_ */
