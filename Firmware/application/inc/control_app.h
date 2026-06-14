#ifndef CONTROL_APP_H
#define CONTROL_APP_H

#include "greenhouse_types.h"

/* Khai báo các hàm công khai */
void ControlApp_Init(void);
void ControlApp_Update(GreenhouseData_t *data);

/* CHÚ Ý: Ch? này ph?i s?a thành SystemMode_t cho kh?p v?i file .c */
void ControlApp_SetMode(SystemMode_t mode);

void ControlApp_SetTempSetpoint(float temp);
void ControlApp_SetPIDTune(float p, float i, float d);

/* Ðã xóa hàm ControlApp_SetHumSetpoint() di r?i nhé */

#endif /* CONTROL_APP_H */