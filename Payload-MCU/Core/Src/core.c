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

// include __disable_irq.
#include "stm32l4xx_hal_def.h"

// TODO:
// [ ] Set command ID's to their correct values.
// [ ] Board temp, well temp, and well light commands.
// [ ] Handle command failures with appropriate error messages.
// [ ] Fix LOG_ERROR, LOG_WARN, LOG_INFO, & ASSERT so they print to console.
// [ ] Refactor CAN driver.

// CAN commands.
#define CMD_RESET        0xA0
#define CMD_LED_ON       0xA1
#define CMD_LED_OFF      0xA2
#define CMD_HEATER_ON    0xA5
#define CMD_HEATER_OFF   0xA6
#define CMD_BOARD_TEMP   0x00
#define CMD_WELL_TEMP    0x00
#define CMD_WELL_LIGHT   0x00

CANQueue_t can_queue;

static uint8_t temp_sequence = 0;
static uint8_t light_sequence = 0;

static void on_message_received(CANMessage_t msg);
static void transmit_well_temp_data(WellID well_id);
static void transmit_light_level_data(WellID well_id);

#define LOG_SUBJECT "Core"

void Core_Init()
{
	bool success;
	HAL_StatusTypeDef status;

	LOG_INFO("Initialising Drivers...");

	success = TCA9539_Init();
	if (!success)
	{
		LOG_ERROR("failed to initialise.");
		Core_Halt();
	}

	status = CAN_Init();
	if (status != HAL_OK)
	{
		LOG_ERROR("failed to initialise.");
		Core_Halt();
	}

	CAN_Queue_Init(&can_queue);
}

void Core_Update()
{
	MAX6822_Reset_Timer();

	if (!CAN_Queue_IsEmpty(&can_queue))
	{
		CANMessage_t can_message;
		CAN_Queue_Dequeue(&can_queue, &can_message);

		on_message_received(can_message);
	}
}

void Core_Halt()
{
	LOG_INFO("Halting program.");

	// disable interrupts.
	__disable_irq();

	// hang indefinitely.
	while (1) {}
}

static void on_message_received(CANMessage_t msg)
{
	uint8_t response_data[6] = {0};

	switch (msg.command)
	{
		case CMD_RESET:
		{
			CAN_Send_Default_ACK(msg);
			MAX6822_Manual_Reset();
			break;
		}
		case CMD_LED_ON:
		{
			uint8_t led_id = msg.data[0];
			LEDs_Set_LED(led_id, ON);
			response_data[0] = led_id;

			CAN_Send_Default_ACK_With_Data(msg, response_data);
			break;
		}
		case CMD_LED_OFF:
		{
			uint8_t led_id = msg.data[0];
			LEDs_Set_LED(led_id, OFF);
			response_data[0] = led_id;

			CAN_Send_Default_ACK_With_Data(msg, response_data);
			break;
		}
		case CMD_HEATER_ON:
		{
			uint8_t heater_id = msg.data[0];
			Heaters_Set_Heater(heater_id, ON);
			response_data[0] = heater_id;

			CAN_Send_Default_ACK_With_Data(msg, response_data);
			break;
		}
		case CMD_HEATER_OFF:
		{
			uint8_t heater_id = msg.data[0];
			Heaters_Set_Heater(heater_id, OFF);
			response_data[0] = heater_id;

			CAN_Send_Default_ACK_With_Data(msg, response_data);
			break;
		}
		case CMD_BOARD_TEMP:
		{
		/*
			uint16_t temp;
			TMP235_Read_Temp(&temp);

			CAN_Send_Default_ACK(msg);
			break;
		*/
			// TODO
		}
		case CMD_WELL_TEMP:
		{
			// TODO
		}
		case CMD_WELL_LIGHT:
		{
			// TODO
		}
		default:
			LOG_ERROR("unknown command: 0x%02X.", msg.command);
			break;
	}
}

// function to get temperature data, package it and send it through CAN
static void transmit_well_temp_data(WellID well_id)
{
	uint16_t temp;
	bool success = Thermistors_Get_Temp(well_id, &temp);

	if (!success)
	{
		LOG_ERRROR("failed to transmit temperature of well %d: could not get temperature.", well_id);
		return;
	}

	CANMessage_t message;

	message.SenderID = 0x3;
	message.DestinationID = 0x1;
	message.command = 0x34;
	message.priority = 0b0000011;

	message.data[0] = temp_sequence++;
	message.data[1] = (uint8_t)well_id;
	message.data[2] = temp & 0x00FF;
	message.data[3] = temp & 0xFF00;
	message.data[4] = 0x00;
    message.data[5] = 0x00;
    message.data[6] = 0x00;

	CAN_Transmit_Message(message);
}

// function to get light level data, package it and send it through CAN
static void transmit_light_level_data(WellID well_id)
{
	uint16_t light;
	bool success = Photocells_Get_Light_Level(well_id, &light);

	if (!success)
	{
		LOG_ERRROR("failed to transmit temperature of well %d: could not get temperature.", well_id);
		return;
	}

	CANMessage_t message;

	message.SenderID = 0x3;
	message.DestinationID = 0x1;
	message.command = 0x33;
	message.priority = 0b0011111;

	message.data[0] = light_sequence++;
	message.data[1] = (uint8_t)well_id;
	message.data[2] = light & 0x00FF;
	message.data[3] = light & 0xFF00;
	message.data[4] = 0x00;
	message.data[5] = 0x00;
	message.data[6] = 0x00;

	CAN_Transmit_Message(message);
}
