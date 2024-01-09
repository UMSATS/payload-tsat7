/*
 * heaters.h
 *
 *  Created on: Jan 9, 2024
 *      Author: Logan Furedi
 */

#ifndef HIGHLEVEL_INC_HEATERS_H_
#define HIGHLEVEL_INC_HEATERS_H_

#include "well_id.h"

#include <stdbool.h>

bool Heaters_Set_Heater(WellID well_id, bool power);

#endif /* HIGHLEVEL_INC_HEATERS_H_ */
