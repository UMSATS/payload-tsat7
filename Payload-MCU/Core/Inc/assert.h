/*
 * assert.h
 *
 *  Created on: Jan 6, 2024
 *      Author: Logan Furedi
 */

#ifndef INC_ASSERT_H_
#define INC_ASSERT_H_

#include "pp.h"

#ifdef USE_FULL_ASSERT

	#define ASSERT1(cond) \
		do { \
			if (!(cond)) \
			{ \
				fprintf(stderr, "[Core] Assertion failed: '" #cond "' (%s:%i)\r\n", __FILE__, __LINE__); \
				assertion_failed(); \
			} \
		} while (0)

	#define ASSERT0(cond, ...) \
		do { \
			if (!(cond)) \
			{ \
				fprintf(stderr, "[Core] Assertion failed: '" #cond "' (%s:%i)\r\n", __FILE__, __LINE__); \
				fprintf(stderr, "  Debug Message: " __VA_ARGS__); \
				fprintf(stderr, "\r\n"); \
				assertion_failed();
			} \
		} while (0)

	#define ASSERT(...)CONCAT(ASSERT,IS_ONE_ARG(__VA_ARGS__))(__VA_ARGS__)

	void assertion_failed();

#else

	#define ASSERT(...)((void)0U)

#endif

#endif /* INC_ASSERT_H_ */
