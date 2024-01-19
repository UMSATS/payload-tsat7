/*
 * error_stack.h
 *
 *  Created on: Jan 15, 2024
 *      Author: Logan Furedi
 */

#ifndef INC_ERROR_STACK_H_
#define INC_ERROR_STACK_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ERROR_STACK_CAPACITY 6

typedef struct ErrorStack ErrorStack;

void Error_Stack_Clear();
void Error_Stack_Push_Error_Code(uint8_t error_code);
void Error_Stack_Push_Data(uint8_t *data, size_t size);
bool Error_Stack_Has_Errors();
const uint8_t *Error_Stack_Get_Errors();

typedef enum {
	ERROR_ADC_CALIBRATION_START = 0,
	ERROR_ADC_START,
	ERROR_ADC_POLL,
	ERROR_ADC_GET_VALUE,
	ERROR_ADC_STOP,
	ERROR_CAN_INIT,
	ERROR_I2C_TRANSMIT,
	ERROR_I2C_RECEIVE,
	ERROR_TCA9539_INIT,
	ERROR_TCA9539_GET_PIN,
	ERROR_TCA9539_SET_PIN,
	ERROR_TCA9539_CLEAR_PINS,
	ERROR_TCA9539_GET_PORT,
	ERROR_TCA9539_SET_PORT,
	ERROR_TCA9548_INIT,
	ERROR_TCA9548_SET_CHANNEL,
	ERROR_TMP235_READ_TEMP,
	ERROR_UNKNOWN_COMMAND
} PAYLOAD_ERRORS;

#define PUSH_ERROR1(error_code) Error_Stack_Push_Error_Code(error_code)

#define PUSH_ERROR0(error_code, ...) \
	do { \
		Error_Stack_Push_Error_Code(error_code); \
		uint8_t data[ERROR_STACK_CAPACITY] = {__VA_ARGS__}; \
		Error_Stack_Push_Data(data, NUM_ARGS(__VA_ARGS__)); \
	} while (0)

#define PUSH_ERROR(...)CONCAT(PUSH_ERROR,IS_ONE_ARG(__VA_ARGS__))(__VA_ARGS__)

/*
#define PUSH_ERROR() \
	do { \
		Error_Stack_Push_Error_Code(__COUNTER__); \
	} while (0)

#define PUSH_ERROR_WITH_DATA(...) \
	do { \
		Error_Stack_Push_Error_Code(__COUNTER__); \
		uint8_t data[ERROR_STACK_CAPACITY] = {__VA_ARGS__}; \
		Error_Stack_Push_Data(data, NUM_ARGS(__VA_ARGS__)); \
	} while (0)
*/
#endif /* INC_ERROR_STACK_H_ */
