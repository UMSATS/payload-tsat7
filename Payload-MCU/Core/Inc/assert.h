/*
 * assert.h
 *
 *  Created on: Jan 6, 2024
 *      Author: Logan Furedi
 */

#ifndef INC_ASSERT_H_
#define INC_ASSERT_H_

#ifdef USE_FULL_ASSERT

	// expands args before concatenation
	#define CONCAT_(a, b) a ## b
	#define CONCAT(a, b) CONCAT_(a, b)

	// 1 when one argument passed. 0 otherwise.
	#define IS_ONE_ARG_(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,...)_10
	#define IS_ONE_ARG(...)IS_ONE_ARG_(__VA_ARGS__,0,0,0,0,0,0,0,0,0,1,)

	#define ASSERT1(cond) \
		do { \
			if (!(cond)) \
			{ \
				fprintf(stderr, "[Core] Assertion failed: '" #cond "' (%s:%i)\r\n", __FILE__, __LINE__); \
				halt_program(); \
			} \
		} while (0)

	#define ASSERT0(cond, ...) \
		do { \
			if (!(cond)) \
			{ \
				fprintf(stderr, "[Core] Assertion failed: '" #cond "' (%s:%i)\r\n", __FILE__, __LINE__); \
				fprintf(stderr, "  Debug Message: " __VA_ARGS__); \
				fprintf(stderr, "\r\n"); \
				halt_program();
			} \
		} while (0)

	#define ASSERT(...)CONCAT(ASSERT,IS_ONE_ARG(__VA_ARGS__))(__VA_ARGS__)

	void assertion_failed();

#else

	#define ASSERT(...)((void)0U)

#endif

#endif /* INC_ASSERT_H_ */
