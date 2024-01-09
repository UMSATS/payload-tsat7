/*
 * tmp235.h
 *
 *  Created on: Dec 6, 2023
 *      Author: Logan Furedi
 */

#ifndef HARDWAREPERIPHERALS_INC_TMP235_H_
#define HARDWAREPERIPHERALS_INC_TMP235_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Gets a temperature reading from the on-board temperature sensor IC.
 *
 * @param out	output parameter. Raw 12 bit integer ADC reading.
 * @return		true on success. false on error.
 */
bool TMP235_Read_Temp(uint16_t *out);

#endif /* HARDWAREPERIPHERALS_INC_TMP235_H_ */
