/*
 * assert.c
 *
 *  Created on: Jan 6, 2024
 *      Author: Logan Furedi
 */

#ifdef USE_FULL_ASSERTS

#include "core.h"
#include "log.h"

#define PRINT_SUBJECT "Assert"

// called after an assertion fails and an error message has been logged.
void assertion_failed()
{
	Core_Halt();
}

#endif
