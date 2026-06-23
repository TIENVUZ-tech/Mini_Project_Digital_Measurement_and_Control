#ifndef QUEUE_MANAGER_H_
#define QUEUE_MANAGER_H_

#include "../inc/frame_queue.h"

// Initialize both TX and RX queues
void QueueMnr_Init(void);

// Push a frame into the TX queue (frames waiting to be sent)
uint8_t TX_Queue_Push(const FrameUnion_t *frame);

// Pop a frame from the TX queue
uint8_t TX_Queue_Pop(FrameUnion_t *frame);

// Push a frame into the RX queue (frames received from UART, waiting to be parsed)
uint8_t RX_Queue_Push(const FrameUnion_t *frame);

// Pop a frame from the RX queue
uint8_t RX_Queue_Pop(FrameUnion_t *frame);

#endif
