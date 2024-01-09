/*
 * therms.h
 *
 *  Created on: Jan 8, 2024
 *      Author: Logan Furedi
 */

#ifndef HIGHLEVEL_INC_THERMISTORS_H_
#define HIGHLEVEL_INC_THERMISTORS_H_

#include "well_id.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 	Reads the current temperature of a well via its corresponding ADC.
 *
 * @param well_id 	The well to retrieve sensor data from.
 * @param out 		Where to store the raw reading from an MCP3221 ADC unit.
 * @return 			true on success. false on error.
 */
bool Thermistors_Get_Temp(WellID well_id, uint16_t *out);

#endif /* HIGHLEVEL_INC_THERMISTORS_H_ */
