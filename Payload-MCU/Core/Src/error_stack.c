/*
 * error_stack.c
 *
 *  Created on: Jan 15, 2024
 *      Author: Logan Furedi
 */

#include "error_stack.h"

#include <string.h>
#include <stdint.h>
#include <stddef.h>

struct ErrorStack
{
	uint8_t errors[ERROR_STACK_CAPACITY];
	size_t size;
};

static ErrorStack error_stack = {0};

void Error_Stack_Clear()
{
	error_stack = (ErrorStack){0};
}

void Error_Stack_Push_Error_Code(uint8_t error_code)
{
	if (error_stack.size == ERROR_STACK_CAPACITY)
		return;

	error_stack.errors[error_stack.size] = error_code;
	error_stack.size++;
}


void Error_Stack_Push_Data(uint8_t *data, size_t size)
{
	if (error_stack.size == ERROR_STACK_CAPACITY)
		return;

	size_t remaining_bytes = ERROR_STACK_CAPACITY - error_stack.size;
	size_t bytes_to_copy = size;
	if (bytes_to_copy > remaining_bytes)
		bytes_to_copy = remaining_bytes;

	memcpy(&error_stack.errors[error_stack.size], data, bytes_to_copy);
	error_stack.size += bytes_to_copy;
}

bool Error_Stack_Has_Errors()
{
	return error_stack.size > 0;
}

const uint8_t *Error_Stack_Get_Errors()
{
	return error_stack.errors;
}
