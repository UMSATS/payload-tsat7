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
#include "heaters.h"
#include "leds.h"
#include "photocells.h"
#include "thermistors.h"
#include "log.h"

// include __disable_irq.
#include "stm32l4xx_hal_def.h"

// CAN commands.
#define CMD_RESET      0xA0
#define CMD_LED_ON     0xA1
#define CMD_LED_OFF    0xA2
#define CMD_HEATER_ON  0xA5
#define CMD_HEATER_OFF 0xA6

CANQueue_t can_queue;

static void on_message_received(CANMessage_t msg);

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
		default:
			break;
	}
}
