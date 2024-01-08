/*
 * pcb_sensor.h
 *
 *  Created on: Dec 6, 2023
 *      Author: Logan Furedi
 */

#ifndef PCB_SENSOR_H_
#define PCB_SENSOR_H_

#include <stdint.h>

/**
 * @brief Gets a temperature reading from the on-board ADC.
 *
 * @return 12 bit integer representation of the reading, or -1 if the operation
 *         failed.
 */
int16_t PCB_Sensor_Read_Temp();

#endif /* PCB_SENSOR_H_ */
