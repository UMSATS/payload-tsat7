/*
 * core.c
 *
 *  Created on: Jan 9, 2024
 *      Author: Logan Furedi
 */

#include <can.h>
#include <can_message.h>
#include <can_wrapper.h>
#include <cmsis_gcc.h>
#include <error_context.h>
#include <heaters.h>
#include <leds.h>
#include <log.h>
#include <max6822.h>
#include <photocells.h>
#include <power.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stm32l452xx.h>
#include <stm32l4xx_hal_def.h>
#include <stm32l4xx_hal_tim.h>
#include <string.h>
#include <sys/_stdint.h>
#include <tca9539.h>
#include <thermistors.h>
#include <tim.h>
#include <tmp235.h>
#include <well_id.h>
#include "core.h"

// TODO:
// [ ] Setup timer for TCS.
// [ ] Add TCS logic.
// [ ] Add temperature formula.
// [ ] Finish flash code.
// [ ] Fix CAN not working with SOTI.
// [ ] Finish idle/active state logic.
// [ ] Finish adding PUSH_ERROR calls next to LOG_ERROR calls.
// [ ] Implement remaining commands.
// [ ] Test changing interrupt intervals on the fly.
// [ ] Possibly create wrapper around HAL I2C transmit/receive functions to
//     improve clarity and possibly automatically push errors if they arise. (?)

// CAN ID's
#define DEVICE_ID 0x03 // this device.
#define CDH_ID    0x01 // CDH.

// common commands.
#define CMD_SHUTDOWN 0x10 // TODO

// Payload commands.
#define CMD_RESET          0xA0 // TODO: test.
#define CMD_LED_ON         0xA1
#define CMD_LED_OFF        0xA2
#define CMD_HEATER_ON      0xA5
#define CMD_HEATER_OFF     0xA6
#define CMD_GET_BOARD_TEMP 0xA7
#define CMD_GET_WELL_LIGHT 0xA8
#define CMD_GET_WELL_TEMP  0xA9
#define CMD_DATA_INTERVAL  0xAA // TODO: test if functional.
#define CMD_LED_TEST       0xAB // TODO
#define CMD_GET_BASELINE   0xAC // TODO

// CDH commands
#define CMD_REPORT_ERROR      0x51
#define CMD_REPORT_WELL_LIGHT 0x33
#define CMD_REPORT_WELL_TEMP  0x34

typedef enum {
	IDLE = 0,
	ACTIVE
} State;

static State s_state = IDLE;
static uint8_t s_temp_sequence = 0;
static uint8_t s_light_sequence = 0;

static ErrorBuffer s_error_buffer; // default error buffer.

static void on_message_received(CANMessage msg);
static void report_well_temp_data(WellID well_id);
static void report_well_light_data(WellID well_id);
static void report_errors();
static void print_well_info();

#define LOG_SUBJECT "Core"

void Core_Init()
{
	bool success;
	CANWrapper_StatusTypeDef cw_status;

	s_state = IDLE;

	LOG_INFO("Initialising Drivers...");

	ErrorContext_Init(&s_error_buffer);

	success = TCA9539_Init();
	if (!success)
	{
		LOG_ERROR("failed to initialise IO Expander driver.");
		PUSH_ERROR(ERROR_TCA9539_INIT); // TODO: rename to WRITE_ERROR?
	}

	CANWrapper_InitTypeDef cw_init = {
			.hcan = &hcan1,
			.can_id = DEVICE_ID,
			.message_callback = &on_message_received
	};

	cw_status = CANWrapper_Init(cw_init);
	if (cw_status != CAN_WRAPPER_HAL_OK)
	{
		LOG_ERROR("failed to initialise CAN wrapper.");
		PUSH_ERROR(ERROR_CAN_WRAPPER_INIT, cw_status);
	}
	else
	{
		// TODO: disable CAN in this case?
	}

	report_errors();
}

void Core_Update()
{
	MAX6822_Reset_Timer();

	CANWrapper_Poll_Messages();

	report_errors();
}

void Core_Halt()
{
	LOG_INFO("Halting program.");

	// disable interrupts.
	__disable_irq();

	// hang indefinitely.
	while (1) {}
}

static void on_message_received(CANMessage msg)
{
	if (msg.command_id == CMD_ACK)
		return;

	ErrorBuffer cmd_error_buffer; // stores command errors.
	ErrorContext_Push_Buffer(&cmd_error_buffer);

	CANMessageBody response_body = { .data = { msg.command_id } };

	// the reset command is a special case.
	if (msg.command_id == CMD_RESET)
	{
		CANWrapper_Send_Response(true, response_body);
		MAX6822_Manual_Reset();
		Core_Halt(); // wait for the hard reset.
		return;
	}

	bool success = false;
	uint8_t response_data_size = 0;

	switch (msg.command_id)
	{
		case CMD_LED_ON:
		{
			success = LEDs_Set_LED(msg.data[1], ON);
			response_body.data[1] = msg.data[1];
			response_data_size = 1;
			break;
		}
		case CMD_LED_OFF:
		{
			success = LEDs_Set_LED(msg.data[1], OFF);
			response_body.data[1] = msg.data[1];
			response_data_size = 1;
			break;
		}
		case CMD_HEATER_ON:
		{
			success = Heaters_Set_Heater(msg.data[1], ON);
			response_body.data[1] = msg.data[1];
			response_data_size = 1;
			break;
		}
		case CMD_HEATER_OFF:
		{
			success = Heaters_Set_Heater(msg.data[1], OFF);
			response_body.data[1] = msg.data[1];
			response_data_size = 1;
			break;
		}
		case CMD_GET_BOARD_TEMP:
		{
			uint16_t temp;
			success = TMP235_Read_Temp(&temp);
			if (success)
			{
				response_body.data[1] = (uint8_t)(temp & 0x00FF);
				response_body.data[2] = (uint8_t)(temp & 0xFF00 >> 8);
				response_data_size = 2;
			}
			break;
		}
		case CMD_GET_WELL_LIGHT:
		{
			uint16_t light;
			success = Photocells_Get_Light_Level(msg.data[1], &light);
			if (success)
			{
				response_body.data[1] = msg.data[1];
				response_body.data[2] = (uint8_t)(light & 0x00FF);
				response_body.data[3] = (uint8_t)(light & 0xFF00 >> 8);
				response_data_size = 3;
			}
			break;
		}
		case CMD_GET_WELL_TEMP:
		{
			uint16_t temp;
			success = Thermistors_Get_Temp(msg.data[1], &temp);
			if (success)
			{
				response_body.data[1] = msg.data[1];
				response_body.data[2] = (uint8_t)(temp & 0x00FF);
				response_body.data[3] = (uint8_t)(temp & 0xFF00 >> 8);
				response_data_size = 3;
			}
			break;
		}
		case CMD_DATA_INTERVAL:
		{
			uint32_t period = 0;

			period |= msg.data[1];
			period |= msg.data[2] << 8;
			period |= msg.data[3] << 16;
			period |= msg.data[4] << 24;

			// set interrupt timer period.
			__HAL_TIM_SET_AUTORELOAD(&htim2, period);

			success = true;
			break;
		}
		default:
		{
			LOG_ERROR("unknown command: 0x%02X.", msg.command_id);
			PUSH_ERROR(ERROR_UNKNOWN_COMMAND, (uint8_t)msg.command_id);
			break;
		}
	}

	if (ErrorBuffer_Has_Error(&cmd_error_buffer))
	{
		success = false;

		// copy error data to the end of the response.

		size_t bytes_left = CAN_MESSAGE_LENGTH - 1 - response_data_size;
		size_t bytes_to_copy = cmd_error_buffer.size;
		if (bytes_to_copy > bytes_left)
			bytes_to_copy = bytes_left;

		memcpy(response_body.data + 1 + response_data_size, cmd_error_buffer.data, bytes_to_copy);
	}

	CANWrapper_Send_Response(success, response_body);

	ErrorContext_Pop_Buffer();
}

// callback for timers.
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	ErrorBuffer error_buffer;

	ErrorContext_Push_Buffer(&error_buffer);

	if (htim->Instance == TIM2)
	{
		for (int i = WELL_0; i <= WELL_15; i++)
		{
			report_well_temp_data(i);
		}
		for (int i = WELL_0; i <= WELL_15; i++)
		{
			report_well_light_data(i);
		}
	}

	if (ErrorBuffer_Has_Error(&error_buffer))
	{
		// TODO
	}

	ErrorContext_Pop_Buffer();
}

// function to get temperature data, package it and send it through CAN
static void report_well_temp_data(WellID well_id)
{
	uint16_t temp;
	bool success = Thermistors_Get_Temp(well_id, &temp);

	if (!success)
	{
		LOG_ERROR("failed to report temperature of well %d: could not get temperature.", well_id);
		return;
	}

	CANMessage msg = {
			.recipient_id = CDH_ID,
			.priority = 3,
			.command_id = CMD_REPORT_WELL_TEMP,
			.body = {
					.data = {
						s_temp_sequence,
						(uint8_t)well_id,
						(uint8_t)(temp & 0x00FF),
						(uint8_t)((temp & 0xFF00) >> 8)
					}
			},
	};

	s_temp_sequence++;

	CANWrapper_Send_Message(msg);
}

// function to get light level data, package it and send it through CAN
static void report_well_light_data(WellID well_id)
{
	uint16_t light;
	bool success = Photocells_Get_Light_Level(well_id, &light);

	if (!success)
	{
		LOG_ERROR("failed to report temperature of well %d: could not get temperature.", well_id);
		return;
	}

	CANMessage msg = {
			.recipient_id = CDH_ID,
			.priority = 31,
			.command_id = CMD_REPORT_WELL_LIGHT,
			.body = {
					.data = {
						s_light_sequence,
						(uint8_t)well_id,
						(uint8_t)(light & 0x00FF),
						(uint8_t)((light & 0xFF00) >> 8)
					}
			},
	};

	s_light_sequence++;

	CANWrapper_Send_Message(msg);
}

/*
 * @brief	reports errors to CDH if any.
 */
static void report_errors() // TODO: generalise this function to be used for other error buffers.
{
	if (ErrorBuffer_Has_Error(&s_error_buffer))
	{
		ssize_t body_length = s_error_buffer.size;

		if (body_length > CAN_MESSAGE_LENGTH-1)
			body_length = CAN_MESSAGE_LENGTH-1;

		CANMessage msg = {
				.recipient_id = CDH_ID,
				.priority = 0,
				.command_id = CMD_REPORT_ERROR,
				.body = {
						.data = {0}
				},
		};

		memcpy(msg.body.data, &s_error_buffer.data, (size_t)body_length);

		CANWrapper_Send_Message(msg);

		ErrorBuffer_Clear(&s_error_buffer);
	}
}

static void print_well_info()
{
	uint16_t therm_data[16];
	uint16_t light_data[16];

	uint16_t temp;

	for (int i = 0; i < 16; i++)
	{
		Thermistors_Get_Temp(i, &temp);
		therm_data[i] = temp;

		Photocells_Get_Light_Level(i, &temp);
		light_data[i] = temp;
	}

	LOG_INFO("_________________________");
	LOG_INFO("| WELL  | TEMPS | LIGHT |");
	LOG_INFO("|-----------------------|");
	for (int i = WELL_0; i <= WELL_15; i++)
	{
		LOG_INFO("| %6d| %6d| %6d|", i, therm_data[i], light_data[i]);
	}
	LOG_INFO("-------------------------");
}
