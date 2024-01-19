/*
 * can_message.h
 *
 *  Created on: Jan 18, 2024
 *      Author: Logan Furedi
 */

#ifndef HIGHLEVEL_INC_CAN_MESSAGE_H_
#define HIGHLEVEL_INC_CAN_MESSAGE_H_

#include <stdint.h>
#include <stdbool.h>

#define CAN_MESSAGE_LENGTH 8

typedef struct {
	uint8_t data[CAN_MESSAGE_LENGTH-1];
} CANMessageBody;

typedef struct {
	uint8_t priority;       // max value: 0x7F.
	uint8_t sender_id;      // max value: 0x3.
	uint8_t destination_id; // max value: 0x3.
	union {
		struct {
			uint8_t command_id;
			CANMessageBody body;
		};
		uint8_t data[CAN_MESSAGE_LENGTH];
	};
} CANMessage;

#endif /* HIGHLEVEL_INC_CAN_MESSAGE_H_ */
