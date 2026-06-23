#ifndef __CONTROL_APP_H
#define __CONTROL_APP_H

#include "greenhouse_types.h"


void ControlApp_Init(void);
void ControlApp_Update(GreenhouseData_t *data);
void ControlApp_SetMode(SystemMode_t mode);
void ControlApp_SetTempSetPoint(float temp);
void ControlApp_SetPIDTune(float p, float i, float d);

#endif
