/*
 * expander_pin_location.h
 *
 *  Created on: Jan 9, 2024
 *      Author: lfipa
 */

#ifndef HIGHLEVEL_INC_EXPANDER_PIN_LOCATION_H_
#define HIGHLEVEL_INC_EXPANDER_PIN_LOCATION_H_

/*
 * Represents the union of an expander ID and a pin number.
 *
 * Used for storing the location of a pin connected to one of the IO expander
 * pins.
 */
typedef struct
{
	ExpanderID device;
	ExpanderPinID pin;
} ExpanderPinLocation;

#endif /* HIGHLEVEL_INC_EXPANDER_PIN_LOCATION_H_ */
