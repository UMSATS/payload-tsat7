/**
 * @file can_message.h
 * Structures for storing CAN message data.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 *
 * @date February 12, 2024
 */

#ifndef CAN_WRAPPER_DRIVER_INC_CAN_MESSAGE_H_
#define CAN_WRAPPER_DRIVER_INC_CAN_MESSAGE_H_

#include <sys/_stdint.h>

#define CAN_MESSAGE_LENGTH 8

typedef struct {
	uint8_t data[CAN_MESSAGE_LENGTH-1];
} CANMessageBody;

typedef struct {
	uint8_t priority;       // max value: 0x7F.
	uint8_t sender_id;      // max value: 0x3.
	uint8_t recipient_id;   // max value: 0x3.
	union {
		struct {
			uint8_t command_id;
			CANMessageBody body; // just the message body.
		};
		uint8_t data[CAN_MESSAGE_LENGTH]; // the entire message (command ID + body).
	};
} CANMessage;

#endif /* CAN_WRAPPER_DRIVER_INC_CAN_MESSAGE_H_ */
