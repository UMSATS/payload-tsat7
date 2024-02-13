/**
 * @file can_queue.c
 * Queue ADT for storing CAN messages.
 *
 * @author Logan Furedi <logan.furedi@umsats.ca>
 * @author Om Sevak <om.sevak@umsats.ca>
 *
 * @date February 12, 2024
 */

#include <can_queue.h>

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
