/*
 * core.c
 *
 *  Created on: Jan 9, 2024
 *      Author: Logan Furedi
 */

#include <can.h>
#include <cmsis_gcc.h>
#include <heaters.h>
#include <leds.h>
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
#include "tuk/tuk.h"

typedef enum {
	IDLE = 0,
	ACTIVE
} State;

static State s_state = IDLE;
static uint8_t s_temp_sequence = 0;
static uint8_t s_light_sequence = 0;

static void on_message_received(CANMessage msg, NodeID sender, bool is_ack);
static void on_error_occured(CANWrapper_ErrorInfo error);
static void report_well_temp_data(WellID well_id);
static void report_well_light_data(WellID well_id);
//static void process_errors(ErrorBuffer *p_error_buffer);
static void print_well_info();

#define PRINT_SUBJECT "Core"

void Core_Init()
{
	bool success;
	CANWrapper_StatusTypeDef cw_status;

	s_state = IDLE;

	PRINT_INFO("Initialising Drivers...");

	DebugLogger_Init();

	success = TCA9539_Init();
	if (!success)
	{
		PRINT_ERROR("failed to initialise IO Expander driver.");
		//PUT_ERROR(ERR_PLD_TCA9539_INIT);
	}

	CANWrapper_InitTypeDef cw_init = {
			.node_id = NODE_PAYLOAD,
			.hcan = &hcan1,
			.htim = &htim16,
			.message_callback = &on_message_received,
			.error_callback = &on_error_occured
	};

	cw_status = CANWrapper_Init(cw_init);
	if (cw_status != CAN_WRAPPER_HAL_OK)
	{
		PRINT_ERROR("failed to initialise CAN wrapper.");
		//PUT_ERROR(ERR_CAN_WRAPPER_INIT, cw_status);
	}
	else
	{
		// TODO: disable CAN in this case?
	}
/*
	if (ErrorBuffer_Has_Error(&s_error_buffer)) // TODO: replace with error code
	{
		process_errors(&s_error_buffer);
	}
*/
}

void Core_Update()
{
	MAX6822_Reset_Timer();

	CANWrapper_Poll_Messages();
	CANWrapper_Poll_Errors();
/*
	if (ErrorBuffer_Has_Error(&s_error_buffer))
	{
		process_errors(&s_error_buffer);
	}
	*/
}

void Core_Halt()
{
	PRINT_INFO("Halting program.");

	// disable interrupts.
	__disable_irq();

	// hang indefinitely.
	while (1) {}
}

static void on_message_received(CANMessage msg, NodeID sender, bool is_ack)
{
	LogBuffer buffer; // stores debug information.
	DebugLogger_Push_Buffer(&buffer);

	bool success = false;

	switch (msg.cmd)
	{
	case CMD_COMM_RESET:
	{
		// trigger a hardware reset.
		MAX6822_Manual_Reset();
		Core_Halt(); // wait for the reset.
		break;
	}
	/*
	case CMD_PLD_SET_WELL_LED:
	{
		uint8_t well_id = GET_ARG(msg, 0, uint8_t);
		uint8_t power   = GET_ARG(msg, 1, uint8_t);

		success = LEDs_Set_LED(well_id, power);
		break;
	}
	case CMD_PLD_SET_WELL_HEATER:
	{
		uint8_t well_id = GET_ARG(msg, 0, uint8_t);
		uint8_t power   = GET_ARG(msg, 1, uint8_t);

		success = Heaters_Set_Heater(well_id, power);
		break;
	}
	case CMD_PLD_SET_SETPOINT:
	{
		uint8_t well_id = GET_ARG(msg, 0, uint8_t);
		float temp      = GET_ARG(msg, 1, float);

		// TODO: update the target temperature of one of the wells in the TCS.
		break;
	}
	*/
	/*
	case CMD_PLD_GET_WELL_LIGHT:
	{
		uint8_t well_id = GET_ARG(msg, 0, uint8_t);

		uint16_t light;
		success = Photocells_Get_Light_Level(well_id, &light);
		if (success)
		{
			CANMessage response;
			response.cmd = CMD_CDH_PROCESS_WELL_LIGHT;
			SET_ARG(response, 0, light);
			CANWrapper_Transmit(NODE_CDH, &response);
		}
		break;
	}
	case CMD_PLD_GET_WELL_TEMP:
	{
		uint8_t well_id = GET_ARG(msg, 0, uint8_t);

		uint16_t temp;
		success = Thermistors_Get_Temp(well_id, &temp);
		if (success)
		{
			CANMessage response;
			response.cmd = CMD_CDH_PROCESS_WELL_TEMP;
			SET_ARG(response, 0, temp);
			CANWrapper_Transmit(NODE_CDH, &response);
		}
		break;
	}
	*/
	case CMD_COMM_SET_TELEMETRY_INTERVAL:
	{
		uint32_t period = GET_ARG(msg, 0, uint32_t);

		// set interrupt timer period.
		__HAL_TIM_SET_AUTORELOAD(&htim2, period);

		success = true;
		break;
	}
	case CMD_PLD_TEST_LEDS:
	{
		// TODO
		break;
	}
	default:
	{
		PRINT_ERROR("unknown command: 0x%02X.", msg.cmd);
		//PUT_ERROR(ERR_UNKNOWN_COMMAND, (uint8_t)msg.cmd);
		break;
	}
	}

	DebugLogger_Pop_Buffer();

	/*
	if (ErrorBuffer_Has_Error(&cmd_error_buffer))
	{
		success = false; // FIXME: either do something with this variable or remove it.
		process_errors(&cmd_error_buffer);
	}
	*/
}

static void on_error_occured(CANWrapper_ErrorInfo error)
{
	switch (error.error)
	{
		case CAN_WRAPPER_ERROR_TIMEOUT:
		{
			// your transmission attempt timed out.
			// Here you can resolve the issue as appropriate.

			// You can re-send the message to the intended recipient like so.
			//CANWrapper_Transmit(error.recipient, &error.msg);

			// TODO

			break;
		}
		case CAN_WRAPPER_ERROR_CAN_TIMEOUT:
		{
			// This is a lower-level type of error that occurs when there is a
			// likely more serious issue with CAN communications. Your message
			// wasn't acknowledged by *any* of the other nodes in the network.
			// This *might* indicate your CAN connection is defective.

			// TODO

			break;
		}
	}
}

// callback for timers.
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//ErrorBuffer error_buffer;
	//ErrorTracker_Push_Buffer(&error_buffer);

	if (htim == &htim2)
	{
		// FIXME: these repeated calls can result in an error buffer overflow if they produce errors.
		for (int i = WELL_0; i <= WELL_15; i++)
		{
			report_well_temp_data(i);
		}
		for (int i = WELL_0; i <= WELL_15; i++)
		{
			report_well_light_data(i);
		}
	}

	//process_errors(&error_buffer);

	//ErrorTracker_Pop_Buffer();
}



// function to get temperature data, package it and send it through CAN
static void report_well_temp_data(WellID well_id)
{
	uint16_t temp;
	bool success = Thermistors_Get_Temp(well_id, &temp);

	if (!success)
	{
		PRINT_ERROR("failed to report temperature of well %d: could not get temperature.", well_id);
		// TODO: Add PUT_ERROR?
		return;
	}

	CANMessage msg;
	msg.cmd = CMD_CDH_PROCESS_TELEMETRY_REPORT;
	uint8_t tel_key = CREATE_TELEMETRY_KEY(TEL_WELL_TEMP, well_id);
	SET_ARG(msg, 0, uint8_t, tel_key);
	SET_ARG(msg, 1, uint8_t, s_temp_sequence);
	SET_ARG(msg, 2, uint8_t, 0); // packet #
	SET_ARG(msg, 3, uint16_t, temp);

	s_temp_sequence++;

	CANWrapper_Transmit(NODE_CDH, &msg);
}

// function to get light level data, package it and send it through CAN
static void report_well_light_data(WellID well_id)
{
	uint16_t light;
	bool success = Photocells_Get_Light_Level(well_id, &light);

	if (!success)
	{
		PRINT_ERROR("failed to report temperature of well %d: could not get temperature.", well_id);
		// TODO: Add PUT_ERROR?
		return;
	}

	CANMessage msg;
	msg.cmd = CMD_CDH_PROCESS_TELEMETRY_REPORT;
	uint8_t tel_key = CREATE_TELEMETRY_KEY(TEL_WELL_LUMINOSITY, well_id);
	SET_ARG(msg, 0, uint8_t, tel_key);
	SET_ARG(msg, 1, uint8_t, s_light_sequence);
	SET_ARG(msg, 2, uint8_t, 0); // packet #
	SET_ARG(msg, 3, uint16_t, light);

	s_light_sequence++;

	CANWrapper_Transmit(NODE_CDH, &msg);
}

/*
static void process_errors(ErrorBuffer *p_error_buffer)
{
	if (ErrorBuffer_Has_Error(p_error_buffer))
	{
		CANMessage error_report;
		error_report.cmd = CMD_CDH_PROCESS_RUNTIME_ERROR;
		//SET_ARG(error_report, 0, *p_error_buffer); // TODO

		CANWrapper_Transmit(NODE_CDH, &error_report);

		ErrorBuffer_Clear(p_error_buffer);
	}
}
*/

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

	PRINT_INFO("_________________________");
	PRINT_INFO("| WELL  | TEMPS | LIGHT |");
	PRINT_INFO("|-----------------------|");
	for (int i = WELL_0; i <= WELL_15; i++)
	{
		PRINT_INFO("| %6d| %6d| %6d|", i, therm_data[i], light_data[i]);
	}
	PRINT_INFO("-------------------------");
}
