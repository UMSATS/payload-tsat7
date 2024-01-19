/*
 * FILENAME: can_driver.h
 *
 * DESCRIPTION: Functions for CAN initialization, message reception, and message transmission.
 *
 * Link to Documentation: https://drive.google.com/file/d/1HHNWpN6vo-JKY5VvzY14uecxMsGIISU7/view?usp=share_link
 *
 * AUTHORS:
 *  - Graham Driver (graham.driver@umsats.ca)
 *
 * CREATED ON: May 25, 2022
 */

#ifndef HIGHLEVEL_INC_CAN_DRIVER_H_
#define HIGHLEVEL_INC_CAN_DRIVER_H_

#include "can_message.h"
#include "stm32l4xx_hal.h"

#include <stdint.h>
#include <stdbool.h>

extern CAN_HandleTypeDef hcan1; // Set this to the CAN type found in generated main.c file

typedef void (*CANMessageCallback)(CANMessage);

/*
 * @brief				starts the CAN bus.
 * @param device_id		the CAN ID for this device. (max value: 0x03)
 * @param callback		called when a new message is polled.
 */
HAL_StatusTypeDef CAN_Init(uint8_t device_id, CANMessageCallback callback);

// polls for new messages. (callback is called from here)
void CAN_Poll_Messages();

HAL_StatusTypeDef CAN_Send_Message(CANMessage message);

HAL_StatusTypeDef CAN_Send_Response(CANMessageBody message_body, bool success);

#endif /* HIGHLEVEL_INC_CAN_DRIVER_H_ */
