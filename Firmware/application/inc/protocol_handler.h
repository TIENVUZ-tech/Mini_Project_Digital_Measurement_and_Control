#ifndef PROTOCOL_HANDLER_H_
#define PROTOCOL_HANDLER_H_

#include "../../platform/inc/comm_hal.h"
#include "./greenhouse_types.h"
#include <stdint.h>

// Start of Frame
#define FRAME_SOF           0xAA

// Type of message
#define FRAME_TYPE_CMD      0x0F
#define FRAME_TYPE_RSP      0x1F

// Message ID
#define MSG_SENSOR_REPORT   0x01
#define MSG_STATUS_REPORT   0x02
#define MSG_SET_PID         0x03
#define MSG_SET_MODE        0x04
#define MSG_MANUAL_ACTUATOR 0x05

// Response STATE
#define RSP_STATUS_OK       0x00
#define RSP_STATUS_FAIL     0xFF

// Frame size limits
#define FRAME_MAX_PAYLOAD   64
#define FRAME_HEADER_SIZE   4  // SOF+TYPE+ID+LEN
#define FRAME_MAX_SIZE      (FRAME_MAX_PAYLOAD + FRAME_HEADER_SIZE)

typedef enum {
	WAIT_SOF_STATE = 0,
	WAIT_TYPE_STATE,
	WAIT_ID_STATE,
	WAIT_LEN_STATE,
	WAIT_PAYLOAD_STATE,
} FrameState_t;

// Init packet state
void Protocol_Init(void);

// Read all bytes from CommHAL, parse each byte
void Protocol_Run(GreenhouseData_t *data);

// Parse 1 byte into state machine - return 1 if the frame is complete
uint8_t Protocol_ProcessByte(uint8_t byte);

// Process the command after the frame has been parsed
void Protocol_HandleCommand(GreenhouseData_t *data);

// Send simple response (ACK/NACK for 1 command)
void Protocol_SendResponse(uint8_t msg_id, uint8_t status);

// Build frame sensor data and sent it
void Protocol_SendSensorData(GreenhouseData_t *data);

// Build frame STATE and send it
void Protocol_SendStatus(GreenhouseData_t *data);

#endif
