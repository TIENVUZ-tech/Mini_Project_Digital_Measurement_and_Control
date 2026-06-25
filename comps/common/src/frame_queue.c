#include "../inc/frame_queue.h"

void FrameQueue_Init(FrameQueue_t *q)
{
	q->head = 0;
	q->tail = 0;
}

uint8_t FrameQueue_IsEmpty(const FrameQueue_t *q)
{
	return (q->head == q->tail);
}

uint8_t FrameQueue_IsFull(const FrameQueue_t *q)
{
	uint8_t next_head = (uint8_t)((q->head + 1) % FRAME_QUEUE_SIZE);
	return (next_head == q->tail);
}

uint8_t FrameQueue_Push(FrameQueue_t *q, const FrameUnion_t *frame)
{
	uint8_t next_head = (uint8_t)((q->head + 1) % FRAME_QUEUE_SIZE);

	if (next_head == q->tail) {
			return 0;   // the queue is full
	}

	q->buf[q->head] = *frame;
	q->head = next_head;
	return 1;
}

uint8_t FrameQueue_Pop(FrameQueue_t *q, FrameUnion_t *frame)
{
    if (q->head == q->tail) {
        return 0;   // the queue is empty
    }

    *frame = q->buf[q->tail];
    q->tail = (uint8_t)((q->tail + 1) % FRAME_QUEUE_SIZE);
    return 1;
}
