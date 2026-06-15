#include "../inc/protocol_handler.h"
#include "string.h"

// Build and send a response frame
static uint8_t send_frame(uint8_t gid, uint8_t mid, const uint8_t *payload, uint16_t len) {
	FrameUnion_t rsp;
	
	rsp.data.sof = FRAME_SOF;
	rsp.data.type = FRAME_TYPE_RSP;
	rsp.data.id = Frame_MakeId(gid, mid);
	Frame_SetLength(&rsp.data, len);
	
	if (len == 0 || payload == NULL) {
		return 0;
	}
	
	memcpy(rsp.data.payload, payload, len);
	CommHAL_SendFrame(&rsp);
	return 1;
}

static uint8_t send_status_frame(uint8_t gid, uint8_t mid, uint8_t status) {
	uint8_t payload[1] = {status};
	return send_frame(gid, mid, payload, 1);
}

// Process the command after the frame has been parsed
static void handle_system(const Frame_t *cmd, GreenhouseData_t *data) {
	uint8_t mid = Frame_GetMid(cmd->id);
	uint8_t len = Frame_GetLength(cmd);
	
	switch (mid) {
		
		case MID_SET_MODE:
			// Payload[0]: 0x00 = AUTO, 0x01 = MANUAL
			if (len >= 1) {
				if (cmd->payload[0] == 0x00) {
					data->mode = MODE_AUTO;
					send_status_frame(GID_SYSTEM, MID_SET_MODE, RSP_STATUS_OK);
				} else if (cmd->payload[0] == 0x01) {
					data->mode = MODE_MANUAL;
					send_status_frame(GID_SYSTEM, MID_SET_MODE, RSP_STATUS_OK);
				} else {
					send_status_frame(GID_SYSTEM, MID_SET_MODE, RSP_STATUS_FAIL);
				}
			} else {
				send_status_frame(GID_SYSTEM, MID_SET_MODE, RSP_STATUS_FAIL);
			}
			break;
			
		case MID_SET_SETPOINT:
			// Payload: 4-byte float (temp_setpoint)
			if (len >= 4) {
				float new_setpoint;
				memcpy(&new_setpoint, cmd->payload, sizeof(float));
				
				if (new_setpoint >= 0.0f && new_setpoint <= 50.0f) {
					data->temp_setpoint = new_setpoint;
					send_status_frame(GID_SYSTEM, MID_SET_SETPOINT, RSP_STATUS_OK);
				} else {
					send_status_frame(GID_SYSTEM, MID_SET_SETPOINT, RSP_STATUS_FAIL);
				}
			} else {
				send_status_frame(GID_SYSTEM, MID_SET_SETPOINT, RSP_STATUS_FAIL);
			}
			break;
			
		default:
			send_status_frame(GID_SYSTEM, mid, RSP_STATUS_FAIL);
			break;
	}
}

// GID_ACTUATOR
static void handle_actuator(const Frame_t *cmd, GreenhouseData_t *data) {
	uint8_t mid = Frame_GetMid(cmd->id);
	uint16_t len = Frame_GetLength(cmd);
	
	switch(mid) {
		
		case MID_MANUAL_ACTUATOR:
			/* Payload[0]: actuator id (0=fan, 1=heater)
				 Payload[1]: state(0=OFF, 1=ON) */
			if (len >= 2 && data->mode == MODE_MANUAL) {
				if (cmd->payload[0] == 0x00) {
					data->fan_state = cmd->payload[1];
				} else if (cmd->payload[0] == 0x01) {
					data->heater_state = cmd->payload[1];
				}
				send_status_frame(GID_ACTUATOR, MID_MANUAL_ACTUATOR, RSP_STATUS_OK);
			} else {
				send_status_frame(GID_ACTUATOR, MID_MANUAL_ACTUATOR, RSP_STATUS_FAIL);
			}
			break;
			
		default: 
			send_status_frame(GID_ACTUATOR, mid, RSP_STATUS_FAIL);
			break;
	}
}

// GID_PID
static void handle_pid(const Frame_t *cmd, GreenhouseData_t *data) {
	uint8_t mid = Frame_GetMid(cmd->id);
	uint16_t len = Frame_GetLength(cmd);
	
	switch(mid) {
		case MID_SET_PID:
			// Payload: Kp 4B float, Ki 4B float, Kd 4B float
			if (len >= 12) {
				// Call PID_SetGains()
				send_status_frame(GID_PID, MID_SET_PID, RSP_STATUS_OK);
			} else {
				send_status_frame(GID_PID, MID_SET_PID, RSP_STATUS_FAIL);
			}
			break;
			
		default:
			// Unknown MSG_ID
			send_status_frame(GID_PID, mid, RSP_STATUS_FAIL);
		  break;
	}
}

// Init packet state
void Protocol_Init(void) {}

// Read all bytes from CommHAL, parse each byte
void Protocol_Run(GreenhouseData_t *data) {
	FrameUnion_t frame;
	
	while (CommHAL_ReceiveFrame(&frame)) {
		uint8_t gid = Frame_GetGid(frame.data.id);
		
		switch(gid) {
			case GID_SYSTEM:
				handle_system(&frame.data, data);
				break;
			
			case GID_ACTUATOR:
				handle_actuator(&frame.data, data);
				break;
			
			case GID_PID:
				handle_pid(&frame.data, data);
				break;
			
			default:
				send_status_frame(gid, Frame_GetMid(frame.data.id), RSP_STATUS_FAIL);
		}
	}
}


// Build frame sensor data and sent it
uint8_t Protocol_SendSensorData(GreenhouseData_t *data) {
	uint8_t payload[8];
	
	memcpy(&payload[0], &data->temperature, sizeof(float));
	memcpy(&payload[4], &data->temp_setpoint, sizeof(float));
	
	return send_frame(GID_SENSOR, MID_SENSOR_REPORT, payload, 8);
}

// Build frame state and send it
uint8_t Protocol_SendStatus(GreenhouseData_t *data) {
	uint8_t payload[12];
	
	memcpy(&payload[0], &data->temperature, sizeof(float));
	memcpy(&payload[4], &data->temp_setpoint, sizeof(float));
	payload[8] = data->fan_state;
	payload[9] = data->heater_state;
	payload[10] = (uint8_t)(data->fan_pwm >> 8);
	payload[11] = (uint8_t)(data->fan_pwm & 0xFF);
	
	return send_frame(GID_SYSTEM, MID_STATUS_REPORT, payload, 12);
}
