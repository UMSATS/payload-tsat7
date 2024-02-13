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

#define CMD_ACK   0x01
#define CMD_NACK  0x02

static CANQueue s_can_queue;
static uint8_t s_device_id; // the ID for THIS device. max value: 0x3
static CANMessageCallback s_message_callback;
static CANMessage s_received_msg; // the current message being processed.
static CAN_HandleTypeDef *s_hcan_ptr = NULL;
static bool s_init = false;

CANWrapper_StatusTypeDef CANWrapper_Init(CAN_HandleTypeDef *hcan_ptr, uint8_t device_id, CANMessageCallback callback)
{
	if (!(device_id <= 0x3
		&& callback != NULL
		&& hcan_ptr != NULL))
	{
		return CAN_WRAPPER_INVALID_ARGS;
	}

	s_device_id = device_id;
	s_message_callback = callback;
	s_received_msg = (CANMessage){0};
	s_hcan_ptr = hcan_ptr;

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

	if (HAL_CAN_ConfigFilter(hcan_ptr, &filter_config) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_CONFIG_FILTER;
	}

	if (HAL_CAN_Start(hcan_ptr) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_START;
	}

	// enable CAN interrupt.
	if (HAL_CAN_ActivateNotification(hcan_ptr, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	{
		return CAN_WRAPPER_FAILED_TO_ENABLE_INTERRUPT;
	}

	s_can_queue = CANQueue_Create();

	s_init = true;
	return CAN_WRAPPER_HAL_OK;
}

CANWrapper_StatusTypeDef CANWrapper_Poll_Messages()
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;

	if (CANQueue_Dequeue(&s_can_queue, &s_received_msg))
	{
		s_message_callback(s_received_msg);
	}

	return CAN_WRAPPER_HAL_OK;
}

CANWrapper_StatusTypeDef CANWrapper_Send_Message(CANMessage message)
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;

	uint32_t tx_mailbox; // transmit mailbox.
	CAN_TxHeaderTypeDef tx_header;

	// TX message parameters.
	uint16_t id = (message.priority << 4) | (s_device_id << 2) | (0x0F & message.recipient_id);

	tx_header.StdId = id;
	tx_header.IDE = CAN_ID_STD;
	tx_header.RTR = CAN_RTR_DATA;
	tx_header.DLC = CAN_MESSAGE_LENGTH;

	// wait to send CAN message.
	while (HAL_CAN_GetTxMailboxesFreeLevel(s_hcan_ptr) == 0){}

	return HAL_CAN_AddTxMessage(s_hcan_ptr, &tx_header, message.data, &tx_mailbox);
}

CANWrapper_StatusTypeDef CANWrapper_Send_Response(bool success, CANMessageBody msg_body)
{
	if (!s_init) return CAN_WRAPPER_NOT_INITIALISED;

	uint8_t ack_or_nack = success ? CMD_ACK : CMD_NACK;

    CANMessage msg = {
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
	if (hcan_ptr == s_hcan_ptr)
	{
		HAL_StatusTypeDef status;

		CAN_RxHeaderTypeDef rx_header; // message header.
		CANMessage msg = {0};

		// get CAN message.
		status = HAL_CAN_GetRxMessage(hcan_ptr, CAN_RX_FIFO0, &rx_header, msg.data);
		if (status != HAL_OK)
			return; // in theory this should never happen. :p

		msg.recipient_id = RECIPIENT_ID_MASK & rx_header.StdId;

		if (msg.recipient_id == s_device_id)
		{
			msg.priority = rx_header.RTR == CAN_RTR_REMOTE ? 0x7F : rx_header.ExtId >> 24;
			msg.sender_id = (SENDER_ID_MASK & rx_header.StdId) >> 2;

			CANQueue_Enqueue(&s_can_queue, msg);
		}
	}
}
