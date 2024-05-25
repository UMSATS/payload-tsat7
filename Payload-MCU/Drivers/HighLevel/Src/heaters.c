/*
 * heaters.c
 *
 *  Created on: Jan 9, 2024
 *      Author: Logan Furedi
 */

#include "heaters.h"
#include "well_id.h"
#include "power.h"
#include "tca9539.h"
#include "expander_pin_location.h"
#include "tuk/log.h"
#include "tuk/error_context.h"

#include <stdbool.h>

static const ExpanderPinLocation HEATER_LOCATIONS[] = {
		{ EXPANDER_1, EXPANDER_PIN_3  }, // HEATER 0
		{ EXPANDER_1, EXPANDER_PIN_1  }, // HEATER 1
		{ EXPANDER_1, EXPANDER_PIN_17 }, // HEATER 2
		{ EXPANDER_1, EXPANDER_PIN_15 }, // HEATER 3
		{ EXPANDER_1, EXPANDER_PIN_5  }, // HEATER 4
		{ EXPANDER_1, EXPANDER_PIN_7  }, // HEATER 5
		{ EXPANDER_1, EXPANDER_PIN_11 }, // HEATER 6
		{ EXPANDER_1, EXPANDER_PIN_13 }, // HEATER 7
		{ EXPANDER_2, EXPANDER_PIN_3  }, // HEATER 8
		{ EXPANDER_2, EXPANDER_PIN_1  }, // HEATER 9
		{ EXPANDER_2, EXPANDER_PIN_17 }, // HEATER 10
		{ EXPANDER_2, EXPANDER_PIN_15 }, // HEATER 11
		{ EXPANDER_2, EXPANDER_PIN_5  }, // HEATER 12
		{ EXPANDER_2, EXPANDER_PIN_7  }, // HEATER 13
		{ EXPANDER_2, EXPANDER_PIN_11 }, // HEATER 14
		{ EXPANDER_2, EXPANDER_PIN_13 }, // HEATER 15
};

#define LOG_SUBJECT "Heaters"

bool Heaters_Set_Heater(WellID well_id, Power power)
{
	ASSERT(WELL_0 <= well_id && well_id <= WELL_15, "invalid well id: %d.", well_id);

	if (well_id < WELL_0 || well_id > WELL_15)
	{
		LOG_ERROR("invalid well id: %d.", well_id);
		PUSH_ERROR(ERROR_PLD_INVALID_WELL_ID);
		return false;
	}

	ExpanderPinLocation location = HEATER_LOCATIONS[well_id];
	bool success = TCA9539_Set_Pin(location.device, location.pin, power);

	if (!success)
	{
		LOG_ERROR("failed to set heater %d to %s", well_id, power ? "ON" : "OFF");
		PUSH_ERROR(ERROR_PLD_TCA9539_SET_PIN);
	}

	return success;
}
