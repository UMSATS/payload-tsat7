/*
 * pcb_sensor.h
 *
 *  Created on: Dec 6, 2023
 *      Author: Logan Furedi
 */

#ifndef PCB_SENSOR_H_
#define PCB_SENSOR_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Gets a temperature reading from the on-board ADC.
 *
 * @param out	output parameter. Raw 12 bit integer ADC reading.
 * @return		true on success. false on error.
 */
bool PCB_Sensor_Read_Temp(uint16_t *out);

#endif /* PCB_SENSOR_H_ */
