#ifndef COMM_HAL_H_
#define COMM_HAL_H_

#include "../../mcu_drivers/inc/drv_uart.h"

#include "../inc/soft_timer.h"

#include "../../common/inc/frame_common.h"
#include "../../common/inc/frame_queue.h"
#include "../../common/inc/queue_manager.h"

#include <stdint.h>

// Initialize: init UART driver, init queue, reset state machine
void CommHAL_Init(void);

void CommHAL_SystickUpdate(void);

// Callback calls from DRV_USART1_IRQ every time 1 byte is received
void CommHAL_RxByteCallback(uint8_t byte);

// Pop 1 received frame from RX queue
uint8_t CommHAL_ReceiveFrame(FrameUnion_t *frame);

// Send 1 completed frame
void CommHAL_SendFrame(const FrameUnion_t *frame);


void CommHAL_SendByte(uint8_t data);
void CommHAL_SendBytes(uint8_t *data, uint8_t len);

#endif 

