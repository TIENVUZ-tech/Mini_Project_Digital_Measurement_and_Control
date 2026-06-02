#include "../inc/protocol_handler.h"
#include "../../platform/inc/comm_hal.h"
#include "string.h"

void Protocol_Init(void);
uint16_t Protocol_BuildSensorFrame(uint8_t *buffer, GreenhouseData_t *data);
uint16_t Protocol_BuildStatusFrame(uint8_t *buffer, GreenhouseData_t *data);
uint8_t Protocol_ParseFrame(uint8_t *buffer, uint16_t len);
void Protocol_HandleCommand(GreenhouseData_t *data);