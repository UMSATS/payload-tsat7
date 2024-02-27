/**
 * @file can_wrapper.h
 * CAN wrapper for simplified initialisation, message reception, and message
 * transmission.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 * @author Graham Driver <graham.driver@umsats.ca>
 *
 * @date February 12, 2024
 */

#ifndef CAN_WRAPPER_DRIVER_INC_CAN_WRAPPER_H_
#define CAN_WRAPPER_DRIVER_INC_CAN_WRAPPER_H_

#include <can_message.h>
#include <stdbool.h>
#include <stm32l4xx.h>
#include <stm32l4xx_hal_can.h>
#include <stm32l4xx_hal_def.h>
#include <sys/_stdint.h>

typedef enum {
	CAN_WRAPPER_HAL_OK = HAL_OK,
	CAN_WRAPPER_HAL_ERROR,
	CAN_WRAPPER_HAL_BUSY,
	CAN_WRAPPER_HAL_TIMEOUT,
	CAN_WRAPPER_INVALID_ARGS,
	CAN_WRAPPER_NOT_INITIALISED,
	CAN_WRAPPER_FAILED_TO_CONFIG_FILTER,
	CAN_WRAPPER_FAILED_TO_START,
	CAN_WRAPPER_FAILED_TO_ENABLE_INTERRUPT,
} CANWrapper_StatusTypeDef;

typedef enum {
	CAN_WRAPPER_TIMEOUT = 0,
	CAN_WRAPPER_NACK,
} CANWrapper_SendError;

typedef void (*CANMessageCallback)(CANMessage);
typedef void (*CANSendFailureCallback)(CANWrapper_SendError, CANMessage);

typedef struct
{
	CAN_HandleTypeDef *hcan; // pointer to the HAL handle for CAN processing.
	uint8_t can_id; // unique CAN ID for this device. (max value: 0x03)
	CANMessageCallback message_callback; // called when a new message is polled.
	CANSendFailureCallback send_failure_callback; // called when a message fails to send. (either due to timeout or NACK)
} CANWrapper_InitTypeDef;

#define CMD_ACK   0x01
#define CMD_NACK  0x02

/**
 * @brief				Performs necessary setup for normal functioning of CAN.
 *
 * @param init_struct   Configuration for initialisation.
 */
CANWrapper_StatusTypeDef CANWrapper_Init(CANWrapper_InitTypeDef init_struct);

/**
 * @brief               Polls the CAN queue for incoming messages. The callback
 *                      is called for each new message.
 */
CANWrapper_StatusTypeDef CANWrapper_Poll_Messages();

/**
 * @brief               Sends a message over CAN.
 *
 * All necessary data such as recipient ID, priority, message contents, etc. are
 * packaged with the CANMessage structure.
 *
 * @param message       See CANMessage type definition.
 */
CANWrapper_StatusTypeDef CANWrapper_Send_Message(CANMessage message);

/**
 * @brief               Sends a response message to the most recent sender.
 *
 * Sends an ACK or a NACK message with the message body attached. The command
 * ID, recipient ID and priority are decided for you. If more control is needed,
 * consider using CANWrapper_Send_Message.
 *
 * @remark              Your data is limited to 7 bytes.
 *
 * @param success       true sends an ACK. false sends a NACK.
 * @param msg_body      In most cases the message body should be a copy of the
 *                      received message, with the last remaining bytes being
 *                      reserved for error information. Consult the command list
 *                      in the google docs for details.
 */
CANWrapper_StatusTypeDef CANWrapper_Send_Response(bool success, CANMessageBody msg_body);

#endif /* CAN_WRAPPER_DRIVER_INC_CAN_WRAPPER_H_ */
