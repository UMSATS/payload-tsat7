/*
 * FILENAME: can_message_queue.h
 *
 * DESCRIPTION: CAN message queue implementation header file.
 *
 * AUTHORS:
 *  - Om Sevak (om.sevak@umsats.ca)
 *  - Logan Furedi
 *
 * CREATED ON: April 30, 2023
 */

#ifndef HARDWAREPERIPHERALS_INC_CAN_MESSAGE_QUEUE_H_
#define HARDWAREPERIPHERALS_INC_CAN_MESSAGE_QUEUE_H_

#include "can_message.h"

#include <stdint.h>
#include <stdbool.h>

#define CAN_QUEUE_SIZE 100

typedef struct {
    uint32_t head;
    uint32_t tail;
    CANMessage messages[CAN_QUEUE_SIZE];
} CANQueue;

/*
 * DESCRIPTION: Creates a new CAN queue.
 */
CANQueue CANQueue_Create();

/*
 * DESCRIPTION: Check if the given CAN message queue is empty.
 *
 * PARAMETERS:
 *  queue: The CAN message queue.
 */
bool CANQueue_IsEmpty(const CANQueue* queue);

/*
 * DESCRIPTION: Check if the given CAN message queue is full.
 *
 * PARAMETERS:
 *  queue: The CAN message queue.
 */
bool CANQueue_IsFull(const CANQueue* queue);

/*
 * DESCRIPTION: Enqueue a message into the given CAN message queue.
 *
 * PARAMETERS:
 *  queue: The CAN message queue.
 *  message: The CAN message.
 */
bool CANQueue_Enqueue(CANQueue* queue, CANMessage message);

/*
 * DESCRIPTION: Dequeue a message out of the given CAN message queue.
 *
 * PARAMETERS:
 *  queue: The CAN message queue.
 *  out_message: Output location for CAN message.
 */
bool CANQueue_Dequeue(CANQueue* queue, CANMessage* out_message);

#endif /* HARDWAREPERIPHERALS_INC_CAN_MESSAGE_QUEUE_H_ */
