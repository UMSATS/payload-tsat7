/*
 * error_context.h
 *
 *  Created on: Jan 15, 2024
 *      Author: Logan Furedi
 */

#ifndef INC_ERROR_CONTEXT_H_
#define INC_ERROR_CONTEXT_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef enum {
	ERROR_ADC_CALIBRATION_START = 0,
	ERROR_ADC_GET_VALUE,
	ERROR_ADC_POLL,
	ERROR_ADC_START,
	ERROR_ADC_STOP,
	ERROR_CAN_ACTIVATE_NOTIFICATION,
	ERROR_CAN_CONFIG_FILTER,
	ERROR_CAN_START,
	ERROR_CAN_WRAPPER_INIT,
	ERROR_FLASH_LOCK,
	ERROR_FLASH_READ_DATA,
	ERROR_FLASH_UNLOCK,
	ERROR_FLASH_WRITE_DATA,
	ERROR_I2C_RECEIVE,
	ERROR_I2C_TRANSMIT,
	ERROR_INVALID_WELL_ID,
	ERROR_TCA9539_CLEAR_PINS,
	ERROR_TCA9539_GET_PIN,
	ERROR_TCA9539_GET_PORT,
	ERROR_TCA9539_INIT,
	ERROR_TCA9539_INVALID_EXPANDER_ID,
	ERROR_TCA9539_INVALID_EXPANDER_PIN_ID,
	ERROR_TCA9539_SET_PIN,
	ERROR_TCA9539_SET_PORT,
	ERROR_TCA9548_INIT,
	ERROR_TCA9548_INVALID_CHANNEL,
	ERROR_TCA9548_SET_CHANNEL,
	ERROR_UNKNOWN_COMMAND,
} ErrorID;

#define ERROR_BUFFER_CAPACITY 6

typedef struct
{
	uint8_t data[ERROR_BUFFER_CAPACITY]; // buffer containing error data.
	size_t size;
} ErrorBuffer;

void ErrorContext_Init(ErrorBuffer *init_errror_buffer);
void ErrorContext_Push_Buffer(ErrorBuffer *error_buffer);
void ErrorContext_Pop_Buffer();

bool ErrorBuffer_Has_Error(ErrorBuffer *error_buffer);
void ErrorBuffer_Clear(ErrorBuffer *error_buffer);

// internal function used by PUSH_ERROR macro.
void ErrorContext_Push_Error_(int n, ...);

#define PUSH_ERROR(...)ErrorContext_Push_Error_(NUM_ARGS(__VA_ARGS__),__VA_ARGS__)

#endif /* INC_ERROR_CONTEXT_H_ */
