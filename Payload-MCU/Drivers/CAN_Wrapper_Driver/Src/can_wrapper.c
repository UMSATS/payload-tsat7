/**
 * @file can_wrapper.c
 * CAN wrapper for simplified initialisation, message reception, and message
 * transmission.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 * @author Graham Driver <graham.driver@umsats.ca>
 *
 * @date February 12, 2024
 */

#include <can_queue.h>
#include <can_wrapper.h>
#include <stddef.h>

// Command:___________________________
//|CMD|DA0|DA1|DA2|DA3|DA4|DA5|DA6|DA7|
// ````````````````````````````````````
// Acknowledgement:___________________
//|ACK|CMD|DA0|DA1|DA2|DA3|DA4|DA5|DA6|
// ````````````````````````````````````

#define PRIORITY_MASK     0b11111110000
#define SENDER_ID_MASK    0b00000001100
#define RECIPIENT_ID_MASK 0b00000000011



static CANWrapper_InitTypeDef s_init_struct = {0};

static CANQueue s_msg_queue = {0};
static CANMessage s_received_msg = {0}; // the current message being processed.
static bool s_init = false;

CANWrapper_StatusTypeDef CANWrapper_Init(CANWrapper_InitTypeDef init_struct)
{
	if (!(init_struct.can_id <= 0x3
		&& init_struct.message_callback != NULL
		&& init_struct.hcan != NULL))
	{
		return CAN_WRAPPER_INVALID_ARGS;
	}

	const CAN_FilterTypeDef filter_config = {
			.FilterIdHigh         = 0x0000,
			.FilterIdLow          = 0x0000,
			.FilterMaskIdHigh     = 0x0000,
			.FilterMaskIdLow      = 0x0000,
			.FilterFIFOAssignment = CAN_FILTER_FIFO0,
			.FilterBank           = 0,
			.FilterMode           = CAN_FILTERMODE_IDMASK,
			.FilterScale          = CAN_FILTERSCALE_32BIT,
			.FilterActivation     = ENABLE,
			.SlaveStartFilterBank = 14,
	};

	if (HAL_CAN_ConfigFilter(init_struct.hcan, &filter_config) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_CONFIG_FILTER;
	}

	if (HAL_CAN_Start(init_struct.hcan) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_START;
	}

	// enable CAN interrupt.
	if (HAL_CAN_ActivateNotification(init_struct.hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_ENABLE_INTERRUPT;
	}

	s_msg_queue = CANQueue_Create();

	s_init_struct = init_struct;
	s_received_msg = (CANMessage){0};

	s_init = true;
	return CAN_WRAPPER_HAL_OK;
}

CANWrapper_StatusTypeDef CANWrapper_Poll_Messages()
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;

	if (CANQueue_Dequeue(&s_msg_queue, &s_received_msg))
	{
		s_init_struct.message_callback(s_received_msg);
	}

	return CAN_WRAPPER_HAL_OK;
}

CANWrapper_StatusTypeDef CANWrapper_Send_Message(CANMessage message)
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;

	uint32_t tx_mailbox; // transmit mailbox.
	CAN_TxHeaderTypeDef tx_header;

	// TX message parameters.
	uint16_t id = (message.priority << 4) | (s_init_struct.can_id << 2) | (0x0F & message.recipient_id);

	tx_header.StdId = id;
	tx_header.IDE = CAN_ID_STD;
	tx_header.RTR = CAN_RTR_DATA;
	tx_header.DLC = CAN_MESSAGE_LENGTH;

	// wait to send CAN message.
	while (HAL_CAN_GetTxMailboxesFreeLevel(s_init_struct.hcan) == 0){}

	return HAL_CAN_AddTxMessage(s_init_struct.hcan, &tx_header, message.data, &tx_mailbox);
}

CANWrapper_StatusTypeDef CANWrapper_Send_Response(bool success, CANMessageBody msg_body)
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;

	uint8_t ack_or_nack = success ? CMD_ACK : CMD_NACK;

    CANMessage msg = {
    		.sender_id = s_init_struct.can_id,
    		.recipient_id = s_received_msg.sender_id,
			.priority = s_received_msg.priority,
    		.command_id = ack_or_nack,
			.body = msg_body
    };

    return CANWrapper_Send_Message(msg);
}

// called by HAL when a new CAN message is received and pending.
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan_ptr)
{
	if (hcan_ptr == s_init_struct.hcan)
	{
		HAL_StatusTypeDef status;

		CAN_RxHeaderTypeDef rx_header; // message header.
		CANMessage msg = {0};

		// get CAN message.
		status = HAL_CAN_GetRxMessage(hcan_ptr, CAN_RX_FIFO0, &rx_header, msg.data);
		if (status != HAL_OK)
			return; // in theory this should never happen. :p

		msg.recipient_id = RECIPIENT_ID_MASK & rx_header.StdId;

		if (msg.recipient_id == s_init_struct.can_id)
		{
			msg.priority = rx_header.RTR == CAN_RTR_REMOTE ? 0x7F : rx_header.ExtId >> 24;
			msg.sender_id = (SENDER_ID_MASK & rx_header.StdId) >> 2;

			// send ACK.

			CANQueue_Enqueue(&s_msg_queue, msg);
		}
	}
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_ACK)
	{
		// timed out.
	}

	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_EWG)
	{
		// error warning. (96 errors recorded from transmission or receipt)
	}

	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_EPV)
	{
		// entered error passive state. (more than 16 failed transmission attempts and/or 128 failed receipts)
	}

	if (HAL_CAN_GetError(hcan) & HAL_CAN_ERROR_BOF)
	{
		// entered bus-off state. (more than 32 failed transmission attempts)
	}
}
