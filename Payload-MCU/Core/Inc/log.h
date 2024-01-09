/*
 * log.h
 *
 *  Created on: Jan 7, 2024
 *      Author: Logan Furedi
 */

#ifndef INC_LOG_H_
#define INC_LOG_H_

#include <stdio.h>

extern const char *LOG_SUBJECT;

#define LOG_ERROR(...) \
	do { \
		fprintf(stderr, "[" LOG_SUBJECT "] Error: " __VA_ARGS__); \
		fprintf(stderr, " ('%s':%d)\n", __FILE__, __LINE__); \
	} while (0)

#define LOG_WARN(...) \
	do { \
		printf("[" LOG_SUBJECT "] Warning: " __VA_ARGS__); \
		printf(" ('%s':%d)\n", __FILE__, __LINE__); \
	} while (0)

#endif /* INC_LOG_H_ */
