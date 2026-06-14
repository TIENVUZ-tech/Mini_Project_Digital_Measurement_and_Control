#ifndef FRAME_QUEUE_H_
#define FRAME_QUEUE_H_

#include "../inc/frame_common.h"
#include <stdint.h>

#define FRAME_QUEUE_SIZE   8U

typedef struct {
    FrameUnion_t buf[FRAME_QUEUE_SIZE];
    volatile uint8_t head;   // producer
    volatile uint8_t tail;   // consumer
} FrameQueue_t;

// Initialize queue (reset head/tail)
void FrameQueue_Init(FrameQueue_t *q);

// Push 1 frame into the queue
uint8_t FrameQueue_Push(FrameQueue_t *q, const FrameUnion_t *frame);

// Pop 1 frame from queue
uint8_t FrameQueue_Pop(FrameQueue_t *q, FrameUnion_t *frame);

// Check if the queue is empty
uint8_t FrameQueue_IsEmpty(const FrameQueue_t *q);

// Check if the queue is full
uint8_t FrameQueue_IsFull(const FrameQueue_t *q);

#endif
