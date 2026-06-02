#ifndef PROTOCOL_HANDLER_H_
#define PROTOCOL_HANDLER_H_

#include "greenhouse_types.h"
#include <stdint.h>

// Start of Frame
#define FRAME_SOF           0xAA 

// Message ID
#define MSG_SENSOR_REPORT   0x0100
#define MSG_STATUS_REPORT   0x0200
#define MSG_SET_PID         0x0300
#define MSG_SET_MODE        0x0400
#define MSG_MANUAL_ACTUATOR 0x0500

void Protocol_Init(void);
uint16_t Protocol_BuildSensorFrame(uint8_t *buffer, GreenhouseData_t *data);
uint16_t Protocol_BuildStatusFrame(uint8_t *buffer, GreenhouseData_t *data);
uint8_t Protocol_ParseFrame(uint8_t *buffer, uint16_t len);
void Protocol_HandleCommand(GreenhouseData_t *data);

#endif