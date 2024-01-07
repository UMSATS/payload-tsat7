/*
 * temp.h
 *
 *  Created on: Jan 5, 2024
 *      Author: Logan Furedi
 */

#ifndef WELLS_H_
#define WELLS_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	WELL_1 = 0,
	WELL_2,
	WELL_3,
	WELL_4,
	WELL_5,
	WELL_6,
	WELL_7,
	WELL_8,
	WELL_9,
	WELL_10,
	WELL_11,
	WELL_12,
	WELL_13,
	WELL_14,
	WELL_15,
	WELL_16
} WellID;

/**
 * @brief Reads the current temperature of a well via its corresponding on-board
 *        ADC unit.
 * @return Raw reading from a MCP3221 ADC unit.
 */
uint16_t wells_get_temperature(WellID well_id);

/**
 * @brief Reads the current temperature of a well via its corresponding on-board
 *        ADC unit.
 * @return Raw reading from a MCP3221 ADC unit.
 */
uint16_t wells_get_light_level(WellID well_id);

#endif /* WELLS_H_ */
