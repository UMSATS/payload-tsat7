/*
 * utils.h
 *
 *  Created on: Jan 6, 2024
 *      Author: Logan Furedi
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include <stdint.h>

/**
 * @brief Converts a big-endian (MSB first) buffer of two 8-bit values into a
 *        16-bit value with the processor's native endianness.
 * @author Logan Furedi
 */
static inline uint16_t utils_be_to_native_16(const uint8_t *buf)
{
	return (uint16_t)((uint16_t)buf[0] << 8 | (uint16_t)buf[1]);
}

/**
 * @brief Converts a big-endian (MSB first) buffer of four 8-bit values into a
 *        32-bit value with the processor's native endianness.
 * @author Logan Furedi
 */
static inline uint32_t utils_be_to_native_32(const uint8_t *buf)
{
	return (uint32_t)buf[0] << 24 |
	       (uint32_t)buf[1] << 16 |
	       (uint32_t)buf[2] << 8 |
	       (uint32_t)buf[3];
}

#endif /* INC_UTILS_H_ */
