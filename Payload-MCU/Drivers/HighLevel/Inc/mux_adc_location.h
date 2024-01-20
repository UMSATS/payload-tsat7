/*
 * mux_adc_location.h
 *
 *  Created on: Jan 8, 2024
 *      Author: Logan Furedi
 */

#ifndef HIGHLEVEL_INC_MUX_ADC_LOCATION_H_
#define HIGHLEVEL_INC_MUX_ADC_LOCATION_H_

#include "tca9548.h"

// all possible I2C addresses for ADC's.
typedef enum {
	ADC_A0 = 0b1001000 << 1,
	ADC_A1 = 0b1001001 << 1,
	ADC_A2 = 0b1001010 << 1,
//	ADC_A3 = 0b1001011, // NOTE: unused.
	ADC_A4 = 0b1001100 << 1,
	ADC_A5 = 0b1001101 << 1,
	ADC_A6 = 0b1001110 << 1,
	ADC_A7 = 0b1001111 << 1,
} ADCAddress;

/*
 * Represents the union of an I2C address and a multiplexer channel.
 *
 * Used for storing the location of an ADC that is connected to one of the
 * multiplexer channels.
 *
 * Note that there are in fact other ADC's on the PCB that are not interfaced
 * with via the multiplexer.
 */
typedef struct
{
	MuxChannel channel;
	ADCAddress address;
} MuxADCLocation;

#endif /* HIGHLEVEL_INC_MUX_ADC_LOCATION_H_ */
