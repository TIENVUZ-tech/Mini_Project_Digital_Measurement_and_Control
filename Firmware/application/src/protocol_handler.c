#include "../inc/protocol_handler.h"
#include "string.h"

// Internal state machine
static FrameState_t s_state = WAIT_SOF_STATE;
static uint8_t s_type = 0;
static uint8_t s_msg_id = 0;
static uint8_t s_length = 0;
static uint8_t s_rx_idx = 0;
static uint8_t s_payload[FRAME_MAX_PAYLOAD];
static uint8_t s_frame_ready = 0; // (=1 when there is a complete frame

// Build and send a frame
static void send_frame(uint8_t type, uint8_t msg_id, uint8_t len, uint8_t *payload) {
	// Send header
	CommHAL_SendByte(FRAME_SOF);
	CommHAL_SendByte(type);
	CommHAL_SendByte(msg_id);
	CommHAL_SendByte(len);
	
	// Send payload
	CommHAL_SendBytes(payload, len);
}

// Init packet state
void Protocol_Init(void) {
	s_state = WAIT_SOF_STATE;
	s_frame_ready = 0;
	s_rx_idx = 0;
}

// Read all bytes from CommHAL, parse each byte
void Protocol_Run(GreenhouseData_t *data) {
	while (CommHAL_Available()) {
		uint8_t byte = CommHAL_ReadByte();
		
		if (Protocol_ProcessByte(byte)) {
			Protocol_HandleCommand(data);
		}
	}
}

// Parse 1 byte into state machine - return 1 if the frame is complete
uint8_t Protocol_ProcessByte(uint8_t byte) {
	switch(s_state) {
		
		case WAIT_SOF_STATE:
			if (byte == FRAME_SOF) {
				s_state = WAIT_TYPE_STATE;
			}
			break;
			
		case WAIT_TYPE_STATE:
			if (byte == FRAME_TYPE_CMD) {
				s_type = byte;
				s_state = WAIT_ID_STATE;
			} else {
				s_state = WAIT_SOF_STATE; // reset if type is incorrect
			}
			break;
		
		case WAIT_ID_STATE:
			s_msg_id = byte;
			s_state = WAIT_LEN_STATE;
			break;
		
		case WAIT_LEN_STATE:
			s_length = byte;
			s_rx_idx = 0;
			if (s_length == 0) {
				s_state = WAIT_SOF_STATE;
				s_frame_ready = 1;
				return 1;
			} else if (s_length > FRAME_MAX_PAYLOAD)  {
				s_state = WAIT_SOF_STATE;
			} else {
				s_state = WAIT_PAYLOAD_STATE;
			}
			break;
			
		case WAIT_PAYLOAD_STATE:
			s_payload[s_rx_idx++] = byte;
			if (s_rx_idx >= s_length) {
				s_state = WAIT_SOF_STATE;
				s_frame_ready = 1;
				return 1;
			}
			break;
		default:
			s_state = WAIT_SOF_STATE;
			break;
	}
	return 0;
}

// Process the command after the frame has been parsed
void Protocol_HandleCommand(GreenhouseData_t *data) {
	if (!s_frame_ready) return;
	
	switch (s_msg_id) {
		
		case MSG_SET_MODE:
			// Payload[0]: 0x00 = AUTO, 0x01 = MANUAL
			if (s_length >= 1) {
				if (s_payload[0] == 0x00) {
					data->mode = MODE_AUTO;
					Protocol_SendResponse(MSG_SET_MODE, RSP_STATUS_OK);
				} else if (s_payload[0] == 0x01) {
					data->mode = MODE_MANUAL;
					Protocol_SendResponse(MSG_SET_MODE, RSP_STATUS_OK);
				} else {
					Protocol_SendResponse(MSG_SET_MODE, RSP_STATUS_FAIL);
				}
			} else {
				Protocol_SendResponse(MSG_SET_MODE, RSP_STATUS_FAIL);
			}
			break;
		case MSG_MANUAL_ACTUATOR:
			/* Payload[0]: actuator id (0=fan, 1=heater)
				 Payload[1]: state(0=OFF, 1=ON) */
			if (s_length >= 2 && data->mode == MODE_MANUAL) {
				if (s_payload[0] == 0x00) {
					data->fan_state = s_payload[1];
				} else if (s_payload[0] == 0x01) {
					data->heater_state = s_payload[1];
				}
				Protocol_SendResponse(MSG_MANUAL_ACTUATOR, RSP_STATUS_OK);
			} else {
				Protocol_SendResponse(MSG_MANUAL_ACTUATOR, RSP_STATUS_FAIL);
			}
			break;
			
		case MSG_SET_PID:
			// Payload: Kp 4B float, Ki 4B float, Kd 4B float
			if (s_length >= 12) {
				// Call PID_SetGains()
				Protocol_SendResponse(MSG_SET_PID, RSP_STATUS_OK);
			} else {
				Protocol_SendResponse(MSG_SET_PID, RSP_STATUS_FAIL);
			}
			break;
		default:
			// Unknown MSG_ID
			Protocol_SendResponse(s_msg_id, RSP_STATUS_FAIL);
		  break;
	}
}

// Send simple response (ACK/NACK for 1 command)
void Protocol_SendResponse(uint8_t msg_id, uint8_t status) {
	uint8_t payload[1] = {status};
	send_frame(FRAME_TYPE_RSP, msg_id, 1, payload);
}

// Build frame sensor data and sent it
void Protocol_SendSensorData(GreenhouseData_t *data) {
	uint8_t payload[8];
	
	memcpy(&payload[0], &data->temperature, sizeof(float));
	memcpy(&payload[4], &data->temp_setpoint, sizeof(float));
	
	send_frame(FRAME_TYPE_RSP, MSG_SENSOR_REPORT, 8, payload);
}

// Build frame STATE and send it
void Protocol_SendStatus(GreenhouseData_t *data) {
	uint8_t payload[12];
	
	memcpy(&payload[0], &data->temperature, sizeof(float));
	memcpy(&payload[4], &data->temp_setpoint, sizeof(float));
	payload[8] = data->fan_state;
	payload[9] = data->heater_state;
	payload[10] = (uint8_t)(data->fan_pwm >> 8);
	payload[11] = (uint8_t)(data->fan_pwm & 0xFF);
	
	send_frame(FRAME_TYPE_RSP, MSG_STATUS_REPORT, 12, payload);
}
