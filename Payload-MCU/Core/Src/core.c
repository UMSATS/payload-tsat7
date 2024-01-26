/*
 * core.c
 *
 *  Created on: Jan 9, 2024
 *      Author: Logan Furedi
 */

#include "core.h"
#include "can_driver.h"
#include "can_message_queue.h"
#include "max6822.h"
#include "tca9539.h"
#include "tca9548.h"
#include "tmp235.h"
#include "heaters.h"
#include "leds.h"
#include "photocells.h"
#include "thermistors.h"
#include "log.h"
#include "error_context.h"

#include "stm32l4xx_hal_def.h"

// include htim2.
#include "tim.h"

#include <string.h>

// TODO:
// [ ] Setup timer for TCS.
// [ ] Add TCS logic.
// [ ] Add temperature formula.
// [ ] Finish flash code.
// [ ] Fix CAN not working with SOTI.
// [ ] Finish idle/active state logic.
// [ ] Finish adding PUSH_ERROR calls next to LOG_ERROR calls.

// Can ID's
#define DEVICE_ID 0x03 // this device.
#define CDH_ID    0x01 // CDH.

// Payload commands.
#define CMD_RESET          0xA0
#define CMD_LED_ON         0xA1
#define CMD_LED_OFF        0xA2
#define CMD_HEATER_ON      0xA5
#define CMD_HEATER_OFF     0xA6
#define CMD_GET_BOARD_TEMP 0xA7
#define CMD_GET_WELL_LIGHT 0xA8
#define CMD_GET_WELL_TEMP  0xA9
#define CMD_DATA_INTERVAL  0xAA
#define CMD_LED_TEST       0xAB
#define CMD_GET_BASELINE   0xAC

// CDH commands
#define CMD_REPORT_ERROR      0x51
#define CMD_REPORT_WELL_LIGHT 0x33
#define CMD_REPORT_WELL_TEMP  0x34

typedef enum {
	IDLE = 0,
	ACTIVE
} State;

static State state = IDLE;
static uint8_t temp_sequence = 0;
static uint8_t light_sequence = 0;

static ErrorBuffer s_error_buffer; // default error buffer.

static void on_message_received(CANMessage msg);
static void report_well_temp_data(WellID well_id);
static void report_well_light_data(WellID well_id);
static void report_errors();

#define LOG_SUBJECT "Core"

void Core_Init()
{
	bool success;
	HAL_StatusTypeDef status;

	state = IDLE;

	LOG_INFO("Initialising Drivers...");

	ErrorContext_Init(&s_error_buffer);

	success = TCA9539_Init();
	if (!success)
	{
		LOG_ERROR("failed to initialise IO Expander driver.");
		PUSH_ERROR(ERROR_TCA9539_INIT);
	}

	status = CAN_Init(DEVICE_ID, on_message_received);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to initialise CAN driver.");
		PUSH_ERROR(ERROR_CAN_INIT);
	}
}

void Core_Update()
{
	MAX6822_Reset_Timer();

	CAN_Poll_Messages();

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
	ErrorBuffer cmd_error_buffer; // stores command errors.
	CANMessageBody response_body = { .data = { msg.command_id } };

	ErrorContext_Push_Buffer(&cmd_error_buffer);

	// the reset command is a special case.
	if (msg.command_id == CMD_RESET)
	{
		CAN_Send_Response(response_body, true);
		MAX6822_Manual_Reset();
		Core_Halt(); // wait for the hard reset.
		return;
	}

	bool success = true;
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
			if (!success)
			{
				PUSH_ERROR(ERROR_TMP235_READ_TEMP);
			}

			response_body.data[1] = (uint8_t)(temp & 0x00FF);
			response_body.data[2] = (uint8_t)(temp & 0xFF00);
			response_data_size = 2;
			break;
		}
		case CMD_GET_WELL_LIGHT:
		{
			uint16_t light;
			success = Photocells_Get_Light_Level(msg.data[1], &light);

			response_body.data[1] = msg.data[1];
			response_body.data[2] = (uint8_t)(light & 0x00FF);
			response_body.data[3] = (uint8_t)(light & 0xFF00);
			response_data_size = 3;
			break;
		}
		case CMD_GET_WELL_TEMP:
		{
			uint16_t temp;
			success = Thermistors_Get_Temp(msg.data[1], &temp);

			response_body.data[1] = msg.data[1];
			response_body.data[2] = (uint8_t)(temp & 0x00FF);
			response_body.data[3] = (uint8_t)(temp & 0xFF00);
			response_data_size = 3;
			break;
		}
		case CMD_DATA_INTERVAL:
		{
			uint32_t period;

			period  = msg.data[1];
			period |= msg.data[2] << 8;
			period |= msg.data[3] << 16;
			period |= msg.data[4] << 24;

			// set interrupt timer period.
			__HAL_TIM_SET_AUTORELOAD(&htim2, period);
			break;
		}
		default:
		{
			success = false;
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

	CAN_Send_Response(response_body, success);

	ErrorContext_Pop_Buffer();
}

// callback for timers.
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance == TIM2)
	{
		for (int i = WELL_0; i < WELL_15; i++)
		{
			report_well_temp_data(i);
		}
		for (int i = WELL_0; i < WELL_15; i++)
		{
			report_well_light_data(i);
		}
	}
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
			.destination_id = CDH_ID,
			.priority = 3,
			.command_id = CMD_REPORT_WELL_TEMP,
			.body = {
					.data = {
						temp_sequence,
						(uint8_t)well_id,
						(uint8_t)(temp & 0x00FF),
						(uint8_t)((temp & 0xFF00) >> 8)
					}
			},
	};

	temp_sequence++;

	CAN_Send_Message(msg);
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
			.destination_id = CDH_ID,
			.priority = 31,
			.command_id = CMD_REPORT_WELL_LIGHT,
			.body = {
					.data = {
						light_sequence,
						(uint8_t)well_id,
						(uint8_t)(light & 0x00FF),
						(uint8_t)((light & 0xFF00) >> 8)
					}
			},
	};

	temp_sequence++;

	CAN_Send_Message(msg);
}

/*
 * @brief	reports errors to CDH if any.
 */
static void report_errors()
{
	if (ErrorBuffer_Has_Error(&s_error_buffer))
	{
		ssize_t body_length = s_error_buffer.size;

		if (body_length > CAN_MESSAGE_LENGTH-1)
			body_length = CAN_MESSAGE_LENGTH-1;

		CANMessage msg = {
				.destination_id = CDH_ID,
				.priority = 10,
				.command_id = CMD_REPORT_ERROR,
				.body = {
						.data = {0}
				},
		};

		memcpy(msg.body.data, &s_error_buffer.data, (size_t)body_length);

		CAN_Send_Message(msg);

		ErrorBuffer_Clear(&s_error_buffer);
	}
}
