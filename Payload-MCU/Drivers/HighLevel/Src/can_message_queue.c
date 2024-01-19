/*
 * FILENAME: can_message_queue.c
 *
 * DESCRIPTION: CAN message queue implementation source file.
 *
 * AUTHORS:
 *  - Om Sevak (om.sevak@umsats.ca)
 *  - Logan Furedi
 *
 * CREATED ON: April 30, 2023
 */

#include "can_message_queue.h"

#include <string.h>
#include <stdbool.h>

CANQueue CANQueue_Create()
{
	CANQueue queue;

    queue.head = 0;
    queue.tail = 0;

    return queue;
}

bool CANQueue_IsEmpty(const CANQueue* queue)
{
    return queue->head == queue->tail;
}

bool CANQueue_IsFull(const CANQueue* queue)
{
    return (queue->tail + 1) % CAN_QUEUE_SIZE == queue->head;
}

bool CANQueue_Enqueue(CANQueue* queue, CANMessage message)
{
    if (CANQueue_IsFull(queue))
        return false;

    queue->messages[queue->tail] = message;
    queue->tail = (queue->tail + 1) % CAN_QUEUE_SIZE;

    return true;
}

bool CANQueue_Dequeue(CANQueue* queue, CANMessage* message_out)
{
    if (CANQueue_IsEmpty(queue))
        return false;

    *message_out = queue->messages[queue->head];
	queue->head = (queue->head + 1) % CAN_QUEUE_SIZE;

    return true;
}
