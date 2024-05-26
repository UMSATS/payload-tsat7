/*
 * leds.c
 *
 *  Created on: Jan 7, 2024
 *      Author: Logan Furedi
 */

#include "leds.h"
#include "well_id.h"
#include "tca9539.h"
#include "expander_pin_location.h"
#include "tuk/log.h"
#include "tuk/error_tracker.h"

#include <stdbool.h>

static const ExpanderPinLocation LED_LOCATIONS[] = {
		{ EXPANDER_1, EXPANDER_PIN_2  }, // LED 0
		{ EXPANDER_1, EXPANDER_PIN_0  }, // LED 1
		{ EXPANDER_1, EXPANDER_PIN_16 }, // LED 2
		{ EXPANDER_1, EXPANDER_PIN_14 }, // LED 3
		{ EXPANDER_1, EXPANDER_PIN_4  }, // LED 4
		{ EXPANDER_1, EXPANDER_PIN_6  }, // LED 5
		{ EXPANDER_1, EXPANDER_PIN_10 }, // LED 6
		{ EXPANDER_1, EXPANDER_PIN_12 }, // LED 7
		{ EXPANDER_2, EXPANDER_PIN_2  }, // LED 8
		{ EXPANDER_2, EXPANDER_PIN_0  }, // LED 9
		{ EXPANDER_2, EXPANDER_PIN_16 }, // LED 10
		{ EXPANDER_2, EXPANDER_PIN_14 }, // LED 11
		{ EXPANDER_2, EXPANDER_PIN_4  }, // LED 12
		{ EXPANDER_2, EXPANDER_PIN_6  }, // LED 13
		{ EXPANDER_2, EXPANDER_PIN_10 }, // LED 14
		{ EXPANDER_2, EXPANDER_PIN_12 }, // LED 15
};

#define LOG_SUBJECT "LEDs"

bool LEDs_Set_LED(WellID well_id, Power power)
{
	ASSERT(WELL_0 <= well_id && well_id <= WELL_15, "invalid well id: %d.", well_id);

	if (well_id < WELL_0 || well_id > WELL_15)
	{
		LOG_ERROR("invalid well id: %d.", well_id);
		PUT_ERROR(ERROR_PLD_INVALID_WELL_ID);
		return false;
	}

	ExpanderPinLocation location = LED_LOCATIONS[well_id];
	bool success = TCA9539_Set_Pin(location.device, location.pin, power);

	if (!success)
	{
		LOG_ERROR("failed to set LED %d to %s", well_id, power ? "ON" : "OFF");
		PUT_ERROR(ERROR_PLD_TCA9539_SET_PIN);
	}

	return success;
}
