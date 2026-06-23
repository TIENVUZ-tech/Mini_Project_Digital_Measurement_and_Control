#ifndef PROTOCOL_HANDLER_H_
#define PROTOCOL_HANDLER_H_

#include "../../platform/inc/comm_hal.h"
#include "../../common/inc/frame_common.h"
#include "../../common/inc/queue_manager.h"
#include "../../platform/inc/actuator_hal.h"
#include "../inc/control_app.h"
#include "../../platform/inc/soft_timer.h"
#include "string.h"
#include "./greenhouse_types.h"
#include "stdint.h"

// Init packet state
void Protocol_Init(void);

void Protocol_SystickUpdate(void);

// Read all bytes from CommHAL, parse each byte
void Protocol_Run(GreenhouseData_t *data);

// Build frame sensor data and sent it
uint8_t Protocol_SendSensorData(GreenhouseData_t *data);

// Build frame state and send it
uint8_t Protocol_SendStatus(GreenhouseData_t *data);

#endif
