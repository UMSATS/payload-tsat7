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
#include "error_stack.h"

// include __disable_irq.
#include "stm32l4xx_hal_def.h"

// include htim2.
#include "tim.h"

#include <string.h>

// TODO:
// [v] Set command ID's to their correct values.
// [v] Board temp, well temp, and well light commands.
// [ ] Handle command failures with appropriate error messages.
// [v] Fix LOG_ERROR, LOG_WARN, LOG_INFO, & ASSERT so they print to console.
// [ ] Refactor CAN driver.
// [v] Setup TIM2.
// [ ] Create function to write to non-volatile memory.
// [ ] Finish idle/active states.
// [ ] Report unprovoked errors to CDH.
// [ ] Implement temperature regulation controller.

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
} State; // TODO

static uint8_t temp_sequence = 0;
static uint8_t light_sequence = 0;

static void on_message_received(CANMessage msg);
static void report_well_temp_data(WellID well_id);
static void report_well_light_data(WellID well_id);
static void report_errors();

#define LOG_SUBJECT "Core"

void Core_Init()
{
	bool success;
	HAL_StatusTypeDef status;

	LOG_INFO("Initialising Drivers...");

	Error_Stack_Clear();

	success = TCA9539_Init();
	if (!success)
	{
		LOG_ERROR("failed to initialise.");
		PUSH_ERROR(ERROR_TCA9539_INIT);
		Core_Halt();
	}

	status = CAN_Init(DEVICE_ID, on_message_received);
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to initialise.");
		PUSH_ERROR(ERROR_CAN_INIT);
		Core_Halt();
	}
}

void Core_Update()
{
	MAX6822_Reset_Timer();

	CAN_Poll_Messages();
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
	Error_Stack_Clear();

	CANMessageBody response_body = { .data = { msg.command_id } };

	// the reset command is a special case.
	if (msg.command_id == CMD_RESET)
	{
		CAN_Send_Response(response_body, true);
		MAX6822_Manual_Reset();
		Core_Halt();
		return;
	}

	bool success = true;

	switch (msg.command_id)
	{
		case CMD_LED_ON:
		{
			success = LEDs_Set_LED(msg.data[1], ON);
			response_body.data[1] = msg.data[1];
			break;
		}
		case CMD_LED_OFF:
		{
			success = LEDs_Set_LED(msg.data[1], OFF);
			response_body.data[1] = msg.data[1];
			break;
		}
		case CMD_HEATER_ON:
		{
			success = Heaters_Set_Heater(msg.data[1], ON);
			response_body.data[1] = msg.data[1];
			break;
		}
		case CMD_HEATER_OFF:
		{
			success = Heaters_Set_Heater(msg.data[1], OFF);
			response_body.data[1] = msg.data[1];
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
			break;
		}
		case CMD_GET_WELL_LIGHT:
		{
			uint16_t light;
			success = Photocells_Get_Light_Level(msg.data[1], &light);

			response_body.data[1] = msg.data[1];
			response_body.data[2] = (uint8_t)(light & 0x00FF);
			response_body.data[3] = (uint8_t)(light & 0xFF00);
			break;
		}
		case CMD_GET_WELL_TEMP:
		{
			uint16_t temp;
			success = Thermistors_Get_Temp(msg.data[1], &temp);

			response_body.data[1] = msg.data[1];
			response_body.data[2] = (uint8_t)(temp & 0x00FF);
			response_body.data[3] = (uint8_t)(temp & 0xFF00);
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

	CAN_Send_Response(response_body, success);
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

static void report_errors()
{
	CANMessage msg;
	// TODO
	CAN_Send_Message(msg);
}
