#include "../inc/protocol_handler.h"

// Soft timmer
static SoftTimer_t s_rx_timeout;
static SoftTimer_t s_tx_timeout;

// Flags set by timer callbacks, cleared by main loop
static volatile uint8_t s_rx_timeout_flag = 0;
static volatile uint8_t s_tx_timeout_flag = 0;

// Timer callbacks
static void on_rx_timeout(void) {
	s_rx_timeout_flag = 1;
}

static void on_tx_timeout(void) {
	s_tx_timeout_flag = 1;
}

// Build and send a response frame
static uint8_t enqueue_frame(uint8_t gid, uint8_t mid, const uint8_t *payload, uint16_t len) {
	FrameUnion_t rsp;
	rsp.data.sof = FRAME_SOF;
	rsp.data.type = FRAME_TYPE_RSP;
	rsp.data.id = Frame_MakeId(gid, mid);
	Frame_SetLength(&rsp.data, len);
	
	if (len > 0 && payload != NULL) {
		memcpy(rsp.data.payload, payload, len);
	}
	SoftTimer_Start(&s_tx_timeout, 2000, on_tx_timeout);
	return TX_Queue_Push(&rsp);
}

static uint8_t send_status_frame(uint8_t gid, uint8_t mid, uint8_t status) {
	uint8_t payload[1] = {status};
	return enqueue_frame(gid, mid, payload, 1);
}

// Process the command after the frame has been parsed
static void handle_system(const Frame_t *cmd, GreenhouseData_t *data) {
	uint8_t mid = Frame_GetMid(cmd->id);
	uint8_t len = Frame_GetLength(cmd);
	
	switch (mid) {
		
		case MID_SET_MODE:
			// Payload[0]: 0x00 = AUTO, 0x01 = MANUAL
			if (len == 1) {
				if (cmd->payload[0] == 0x00) {
					data->mode = MODE_AUTO;
					ControlApp_SetMode(MODE_AUTO);
					send_status_frame(GID_SYSTEM, MID_SET_MODE, RSP_SET_MODE_OK);
				} else if (cmd->payload[0] == 0x01) {
					data->mode = MODE_MANUAL;
					ControlApp_SetMode(MODE_MANUAL);
					send_status_frame(GID_SYSTEM, MID_SET_MODE, RSP_SET_MODE_OK);
				} else {
					send_status_frame(GID_SYSTEM, MID_SET_MODE, RSP_SET_MODE_FAIL);
				}
			} else {
				send_status_frame(GID_SYSTEM, MID_SET_MODE, RSP_INVALID_LENGTH);
			}
			break;
		
		case MID_STATUS_REPORT:
			if (len == 0) {
				uint8_t complete = Protocol_SendStatus(data);
				if (complete) {
					send_status_frame(GID_SYSTEM, MID_STATUS_REPORT, RSP_STATUS_REPORT_OK);
				} else {
					send_status_frame(GID_SYSTEM, MID_STATUS_REPORT, RSP_STATUS_REPORT_FAIL);
				}
			} else {
				send_status_frame(GID_SYSTEM, MID_STATUS_REPORT, RSP_INVALID_LENGTH);
			}
			break;
			
		case MID_SET_SETPOINT:
			// Payload: 4-byte float (temp_setpoint)
			if (len == 4) {
				float new_setpoint;
				memcpy(&new_setpoint, cmd->payload, sizeof(float));
				
				if (new_setpoint >= 0.0f && new_setpoint <= 50.0f) {
					data->temp_setpoint = new_setpoint;
					ControlApp_SetTempSetPoint(new_setpoint);
					send_status_frame(GID_SYSTEM, MID_SET_SETPOINT, RSP_SET_SETPOINT_OK);
				} else {
					send_status_frame(GID_SYSTEM, MID_SET_SETPOINT, RSP_SET_SETPOINT_FAIL);
				}
			} else {
				send_status_frame(GID_SYSTEM, MID_SET_SETPOINT, RSP_INVALID_LENGTH);
			}
			break;
			
		case MID_SET_REPORT_MODE:
			// Payload: 1 byte (0x00 = one shot, 0x01 = stream every 2s)
			if (len == 1) {
				if (cmd->payload[0] == REPORT_MODE_ONE_SHOT) {
					data->report_mode = REPORT_MODE_ONE_SHOT;
					data->report_once_pending = 1;
					send_status_frame(GID_SYSTEM, MID_SET_REPORT_MODE, RSP_SET_REPORT_MODE_OK);
				} else if (cmd->payload[0] == REPORT_MODE_STREAM) {
					data->report_mode = REPORT_MODE_STREAM;
					data->report_once_pending = 0;
					send_status_frame(GID_SYSTEM, MID_SET_REPORT_MODE, RSP_SET_REPORT_MODE_OK);
				} else {
					send_status_frame(GID_SYSTEM, MID_SET_REPORT_MODE, RSP_SET_REPORT_MODE_FAIL);
				}
			} else {
				send_status_frame(GID_SYSTEM, MID_SET_REPORT_MODE, RSP_INVALID_LENGTH);
			}
			break;
			
		default:
			send_status_frame(GID_SYSTEM, mid, RSP_SYSTEM_FAIL);
			break;
	}
}

// GID_ACTUATOR
static void handle_actuator(const Frame_t *cmd, GreenhouseData_t *data) {
	uint8_t mid = Frame_GetMid(cmd->id);
	uint16_t len = Frame_GetLength(cmd);

	switch(mid) {

		case MID_MANUAL_ACTUATOR: {
			/* Payload[0]: actuator id (0=fan, 1=heater)
			   Payload[1]: state (0=OFF, 1=ON) */

			if (len != 2) {
				send_status_frame(GID_ACTUATOR, MID_MANUAL_ACTUATOR, RSP_INVALID_LENGTH);
				break;
			}

			if (data->mode != MODE_MANUAL) {
				send_status_frame(GID_ACTUATOR, MID_MANUAL_ACTUATOR, RSP_MANUAL_ACTUATOR_FAIL);
				break;
			}

			uint8_t actuator = cmd->payload[0];
			uint8_t state = cmd->payload[1];

			if (state != ACT_OFF && state != ACT_ON) {
				send_status_frame(GID_ACTUATOR, MID_MANUAL_ACTUATOR, RSP_MANUAL_ACTUATOR_FAIL);
				break;
			}

			if (actuator != ACTUATOR_FAN && actuator != ACTUATOR_HEATER) {
				send_status_frame(GID_ACTUATOR, MID_MANUAL_ACTUATOR, RSP_MANUAL_ACTUATOR_FAIL);
				break;
			}

			ActuatorPrams_struct_t actuator_cmd;
			actuator_cmd.index = (ActuatorDevice_t)actuator;
			actuator_cmd.mode = ACT_MODE_ONOFF;
			actuator_cmd.unit = UNIT_PERCENT;
			actuator_cmd.value = (state == ACT_ON) ? 100.0f : 0.0f;

			if (ActuatorHAL_Set(actuator_cmd) != ACTUATOR_OK) {
				send_status_frame(GID_ACTUATOR, MID_MANUAL_ACTUATOR, RSP_MANUAL_ACTUATOR_FAIL);
				break;
			}

			if (actuator == ACTUATOR_FAN) {
				data->fan_state = state;
				data->fan_pwm = (state == ACT_ON) ? 100 : 0;
			} else {
				data->heater_state = state;
				data->heater_pwm = (state == ACT_ON) ? 100 : 0;
			}

			send_status_frame(GID_ACTUATOR, MID_MANUAL_ACTUATOR, RSP_MANUAL_ACTUATOR_OK);
			break;
		}

		default:
			send_status_frame(GID_ACTUATOR, mid, RSP_ACTUATOR_FAIL);
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
			if (len == 12) {
				float kp, ki, kd;
				memcpy(&kp, &cmd->payload[0], 4);
				memcpy(&ki, &cmd->payload[4], 4);
				memcpy(&kd, &cmd->payload[8], 4);
				ControlApp_SetPIDTune(kp, ki, kd);
				send_status_frame(GID_PID, MID_SET_PID, RSP_SET_PID_OK);
			} else {
				send_status_frame(GID_PID, MID_SET_PID, RSP_INVALID_LENGTH);
			}
			break;
			
		default:
			// Unknown MSG_ID
			send_status_frame(GID_PID, mid, RSP_PID_FAIL);
		  break;
	}
}

// Init packet state
void Protocol_Init(void) {
	CommHAL_Init();
}

void Protocol_SystickUpdate(void)
{
	SoftTimer_Update(&s_rx_timeout);
	SoftTimer_Update(&s_tx_timeout);
	CommHAL_SystickUpdate();
}

// Read all bytes from CommHAL, parse each byte
void Protocol_Run(GreenhouseData_t *data) {
	FrameUnion_t frame;
	SoftTimer_Start(&s_rx_timeout, 2000, on_rx_timeout);
	while (CommHAL_ReceiveFrame(&frame)) {
		SoftTimer_Stop(&s_rx_timeout);
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
				send_status_frame(gid, Frame_GetMid(frame.data.id), RSP_GID_INVALID);
		}
	}
	
	FrameUnion_t tx_frame;
	if (TX_Queue_Pop(&tx_frame)) {
		CommHAL_SendFrame(&tx_frame);
		SoftTimer_Stop(&s_tx_timeout);
	}
}


// Build frame sensor data and sent it
uint8_t Protocol_SendSensorData(GreenhouseData_t *data) {
	uint8_t payload[8];
	
	memcpy(&payload[0], &data->temperature, sizeof(float));
	memcpy(&payload[4], &data->temp_setpoint, sizeof(float));
	
	return enqueue_frame(GID_SENSOR, MID_SENSOR_REPORT, payload, 8);
}

// Build frame state and send it
uint8_t Protocol_SendStatus(GreenhouseData_t *data) {
	uint8_t payload[14];
	
	memcpy(&payload[0], &data->temperature, sizeof(float));
	memcpy(&payload[4], &data->temp_setpoint, sizeof(float));
	payload[8] = data->fan_state;
	payload[9] = data->heater_state;
	payload[10] = (uint8_t)(data->fan_pwm >> 8);
	payload[11] = (uint8_t)(data->fan_pwm & 0xFF);
	payload[12] = (uint8_t) (data->heater_pwm >> 8);
	payload[13] = (uint8_t) (data->heater_pwm & 0xFF);
	
	return enqueue_frame(GID_SYSTEM, MID_STATUS_REPORT, payload, 14);
}
