#include "../inc/queue_manager.h"

static FrameQueue_t tx_queue;
static FrameQueue_t rx_queue;

void QueueMnr_Init() {
	FrameQueue_Init(&tx_queue);
	FrameQueue_Init(&rx_queue);
}

uint8_t TX_Queue_Push(const FrameUnion_t *frame) {
	return FrameQueue_Push(&tx_queue, frame);
}

uint8_t TX_Queue_Pop(FrameUnion_t *frame) {
	return FrameQueue_Pop(&tx_queue, frame);
}

uint8_t RX_Queue_Push(const FrameUnion_t *frame) {
	return FrameQueue_Push(&rx_queue, frame);
}

uint8_t RX_Queue_Pop(FrameUnion_t *frame) {
	return FrameQueue_Pop(&rx_queue, frame);
}