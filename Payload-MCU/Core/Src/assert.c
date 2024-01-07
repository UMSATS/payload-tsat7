/*
 * assert.c
 *
 *  Created on: Jan 6, 2024
 *      Author: Logan Furedi
 */

#ifdef USE_FULL_ASSERTS

#include "cmsis_iccarm.h"

static void halt_program()
{
	fprintf(stderr, "[Core] Halting program.\r\n");

	// disable interrupts.
	__disable_irq();

	// hang indefinitely.
	while (1) {}
}

// called after an assertion fails and an error message has been written.
void assertion_failed()
{
	halt_program();
}

#endif
