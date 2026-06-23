#ifndef GREENHOUSE_TYPES_H_
#define GREENHOUSE_TYPES_H_

#include <stdint.h>

// Definition of operating modes
typedef enum {
    MODE_AUTO = 0,
    MODE_MANUAL
} SystemMode_t;

// Definition of actuator states
typedef enum {
    ACT_OFF = 0,
    ACT_ON
} ActuatorState_t;

typedef enum {
	REPORT_MODE_ONE_SHOT = 0,
	REPORT_MODE_STREAM
} ReportMode_t;

// Data structure of greenhouse
typedef struct {
	float temperature;
	float temp_setpoint;
	uint8_t fan_state;
	uint8_t heater_state;
	uint16_t fan_pwm;
	uint16_t heater_pwm;
	SystemMode_t mode;
	ReportMode_t report_mode;
	uint8_t report_once_pending;
} GreenhouseData_t;

#endif
