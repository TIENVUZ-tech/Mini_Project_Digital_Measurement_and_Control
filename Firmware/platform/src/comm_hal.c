#include "../inc/comm_hal.h"
#include "../inc/soft_timer.h"
#include "../../common/inc/queue_manager.h"

#define BAUD_RATE 115200

typedef enum {
	WAIT_SOF_STATE = 0,
	WAIT_TYPE_STATE,
	WAIT_ID_STATE,
	WAIT_LEN_H_STATE,
	WAIT_LEN_L_STATE,
	WAIT_PAYLOAD_STATE,
} FrameState_t;

static FrameState_t s_rx_state = WAIT_SOF_STATE;
static FrameUnion_t s_rx_frame;
static uint16_t s_rx_payload_idx = 0;
static uint16_t s_rx_length = 0;

static SoftTimer_t s_rx_byte_timeout;
static volatile uint8_t s_rx_byte_timeout_flag = 0;

static void on_rx_byte_timeout() {
	s_rx_byte_timeout_flag = 1;
}

void CommHAL_Init(void){
	DRV_UART1_Init(BAUD_RATE);
	QueueMnr_Init();
	
	s_rx_state = WAIT_SOF_STATE;
	s_rx_payload_idx = 0;
	s_rx_length = 0;
}

void CommHAL_SystickUpdate(void)
{
    SoftTimer_Update(&s_rx_byte_timeout);
}

void CommHAL_RxByteCallback(uint8_t byte) {
	
	switch(s_rx_state) {
		
		case WAIT_SOF_STATE:
			if (byte == FRAME_SOF) {
				SoftTimer_Start(&s_rx_byte_timeout, 2000, on_rx_byte_timeout);
				s_rx_state = WAIT_TYPE_STATE;
			}
			break;
			
		case WAIT_TYPE_STATE:
			if (byte == FRAME_TYPE_CMD) {
				s_rx_frame.data.type = byte;
				s_rx_state = WAIT_ID_STATE;
			} else {
				s_rx_state = WAIT_SOF_STATE; // reset if the type is incorrect
			}
			break;
		
		case WAIT_ID_STATE:
			s_rx_frame.data.id = byte;
			s_rx_state = WAIT_LEN_H_STATE;
			break;
		
		case WAIT_LEN_H_STATE:
			s_rx_frame.data.len_h = byte;
			s_rx_state = WAIT_LEN_L_STATE;
			break;
		
		case WAIT_LEN_L_STATE:
			s_rx_frame.data.len_l = byte;
			s_rx_length = Frame_GetLength(&s_rx_frame.data);
			s_rx_payload_idx = 0;
		
			if (s_rx_length == 0) {
				SoftTimer_Stop(&s_rx_byte_timeout);
				RX_Queue_Push(&s_rx_frame);
				s_rx_state = WAIT_SOF_STATE;
			} else if (s_rx_length > FRAME_MAX_PAYLOAD) {
				s_rx_state = WAIT_SOF_STATE;
			} else {
				s_rx_state = WAIT_PAYLOAD_STATE;
			}
			break;
			
		case WAIT_PAYLOAD_STATE:
			s_rx_frame.data.payload[s_rx_payload_idx++] = byte;
			if (s_rx_payload_idx >= s_rx_length) {
				SoftTimer_Stop(&s_rx_byte_timeout);
				uint8_t state = RX_Queue_Push(&s_rx_frame);
				s_rx_state = WAIT_SOF_STATE;
			}
			break;
			
		default:
			s_rx_state = WAIT_SOF_STATE;
			break;
		}
}

uint8_t CommHAL_ReceiveFrame(FrameUnion_t *frame) {
	return RX_Queue_Pop(frame);
}

void CommHAL_SendFrame(const FrameUnion_t *frame) {

	uint16_t length = Frame_GetLength(&frame->data);
	uint16_t total_len = FRAME_HEADER_SIZE + length;
	
	DRV_UART1_SendBytes((uint8_t *)frame->bytes, total_len);
}

void CommHAL_SendByte(uint8_t data) {
	DRV_UART1_SendByte(data);
}

void CommHAL_SendBytes(uint8_t *data, uint8_t len) {
	DRV_UART1_SendBytes(data, len);
}
