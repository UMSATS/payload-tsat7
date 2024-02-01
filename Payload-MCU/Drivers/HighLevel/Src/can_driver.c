/*
 * FILENAME: can_driver.c
 *
 * DESCRIPTION: Functions for CAN initialization, message reception, and message transmission.
 *              Received messages are read into a Queue, which can be handled by a dedicated task.
 *
 * Link to Documentation: https://drive.google.com/file/d/1HHNWpN6vo-JKY5VvzY14uecxMsGIISU7/view?usp=share_link
 *
 * AUTHORS:
 *  - Graham Driver (graham.driver@umsats.ca)
 *  - Logan Furedi
 *
 * CREATED ON: May 25, 2022
 */

// Command:___________________________
//|CMD|DA0|DA1|DA2|DA3|DA4|DA5|DA6|DA7|
// ````````````````````````````````````
// Acknowledgement:___________________
//|ACK|CMD|DA0|DA1|DA2|DA3|DA4|DA5|DA6|
// ````````````````````````````````````

#include <error_context.h>
#include "can_driver.h"
#include "can_message_queue.h"
#include "assert.h"
#include <stdio.h>
#include <string.h>

#define RECEIVED_SENDER_ID_MASK  0xC
#define RECEIVED_DESTINATION_ID_MASK  0x3

#define CMD_ACK   0x01
#define CMD_NACK  0x02

static CANQueue s_can_queue;
static uint8_t s_device_id; // the ID for THIS device. max value: 0x3
static CANMessageCallback s_message_callback;
static CANMessage s_received_msg; // the current message being processed.

/**
 * @brief Boots the CAN Bus
 *
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef CAN_Init(uint8_t device_id, CANMessageCallback callback)
{
	ASSERT(device_id <= 0x03);
	ASSERT(callback != NULL);

	HAL_StatusTypeDef operation_status;

	s_device_id = device_id;
	s_message_callback = callback;
	s_received_msg = (CANMessage){0};

	CAN_FilterTypeDef sFilterConfig;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;

	operation_status = HAL_CAN_ConfigFilter(&hcan1, &filter_config);
	if (operation_status != HAL_OK)
	{
		PUSH_ERROR(ERROR_CAN_CONFIG_FILTER);
		goto error;
	}

	operation_status = HAL_CAN_Start(&hcan1);

	operation_status = HAL_CAN_ConfigFilter(&hcan1, &sFilterConfig);
	if (operation_status != HAL_OK) goto error;
	operation_status = HAL_CAN_Start(&hcan1); // Turn on the CAN Bus

	if (operation_status != HAL_OK) goto error;

	operation_status = HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

	s_can_queue = CANQueue_Create();

error:
	return operation_status;
}

void CAN_Poll_Messages()
{
	if (CANQueue_Dequeue(&s_can_queue, &s_received_msg))
	{
		s_message_callback(s_received_msg);
	}
}

/**
 * @brief Used to send messages over CAN
 *
 * @param message: The CAN message
 *
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef CAN_Send_Message(CANMessage message)
{
	uint32_t tx_mailbox; // transmit mailbox.
	CAN_TxHeaderTypeDef tx_header;

	// TX message parameters.
	uint16_t id = (0b00000000 << 4) | (0x3 << 2) | (0x1);

	tx_header.StdId = id;
	tx_header.IDE = CAN_ID_STD;
	tx_header.RTR = CAN_RTR_DATA;
	tx_header.DLC = CAN_MESSAGE_LENGTH;

	// wait to send CAN message.
	while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0){}

	uint8_t myarray[] =  {0x1, 0xa1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	return HAL_CAN_AddTxMessage(&hcan1, &tx_header, myarray, &tx_mailbox);
}

/**
 * @brief Interrupt Handler for received CAN messages
 *
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef CAN_Message_Received()
{
    HAL_StatusTypeDef operation_status;

	CAN_RxHeaderTypeDef rx_header; // message header.
	CANMessage msg = {0};

	// get CAN message.
	operation_status = HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &rx_header, msg.data);
	if (operation_status != HAL_OK) goto error;

	msg.destination_id = RECEIVED_DESTINATION_ID_MASK & rx_header.StdId;

	if (msg.destination_id == s_device_id)
	{
		msg.priority = rx_header.RTR == CAN_RTR_REMOTE ? 0x7F : rx_header.ExtId >> 24;
		msg.sender_id = (RECEIVED_SENDER_ID_MASK & rx_header.StdId) >> 2;

	    CANQueue_Enqueue(&s_can_queue, msg);
	}

error:
	return operation_status;
}

/**
 * @brief Send a CAN default ACK message for the given CAN message
 *
 * @param message: The received CAN message to send the ACK for
 *
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef CAN_Send_Response(CANMessageBody msg_body, bool success)
{
	uint8_t ack_or_nack = success ? CMD_ACK : CMD_NACK;

    CANMessage msg = {
    		.destination_id = s_received_msg.sender_id,
			.priority = s_received_msg.priority,
    		.command_id = ack_or_nack,
			.body = msg_body
    };
/*
    // append data to response.
    size_t bytes_to_copy = sizeof(msg.body);
    if (bytes_to_copy > data_size)
    	bytes_to_copy = data_size;
    memcpy(msg.body, data, bytes_to_copy);
*/
    return CAN_Send_Message(msg);
}

// called when a new CAN message is received and pending.
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan1) {
    CAN_Message_Received();
}
