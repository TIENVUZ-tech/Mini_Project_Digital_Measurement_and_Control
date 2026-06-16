#include "control_app.h"
#include "pid_control.h"
#include "actuator_hal.h"
#include "sensor_app.h"
#include "protocol_handler.h"

static PIDController_t pid_fan;
static PIDController_t pid_heater;

static const float SAMPLE_TIME_DT = 0.2f; 

void ControlApp_Init(void) {
    pid_fan.Kp = 8.5f;
    pid_fan.Ki = 0.1f;
    pid_fan.Kd = 1.5f;
    pid_fan.tau = 0.02f;
    pid_fan.T = SAMPLE_TIME_DT;
    pid_fan.limMin = 0.0f;
    pid_fan.limMax = 100.0f;
    pid_fan.limMinInt = -20.0f;
    pid_fan.limMaxInt = 20.0f;
    PID_Init(&pid_fan);

    pid_heater.Kp = 12.0f;
    pid_heater.Ki = 0.2f;
    pid_heater.Kd = 2.0f;
    pid_heater.tau = 0.02f;
    pid_heater.T = SAMPLE_TIME_DT;
    pid_heater.limMin = 0.0f;
    pid_heater.limMax = 100.0f;
    pid_heater.limMinInt = -20.0f;
    pid_heater.limMaxInt = 20.0f;
    PID_Init(&pid_heater);
}

void ControlApp_Process(GreenhouseData_t *data) {
    data->temperature = readTemperature();

    float fan_cmd = 0.0f;
    float heater_cmd = 0.0f;

    if (data->mode == MODE_AUTO) {
        fan_cmd = PID_Compute(&pid_fan, data->temp_setpoint, data->temperature);
        heater_cmd = PID_Compute(&pid_heater, data->temperature, data->temp_setpoint);
        data->fan_pwm = (uint16_t)fan_cmd;
			  data->heater_pwm = (uint16_t)heater_cmd;
    } 
    else if (data->mode == MODE_MANUAL) {
        fan_cmd = (float)data->fan_pwm;
			  heater_cmd = (float)data->heater_pwm;
       

        pid_fan.integrator = 0.0f;
        pid_fan.prevError = 0.0f;
        pid_fan.differentiator = 0.0f;
        pid_fan.prevMeasurement = 0.0f;

        pid_heater.integrator = 0.0f;
        pid_heater.prevError = 0.0f;
        pid_heater.differentiator = 0.0f;
        pid_heater.prevMeasurement = 0.0f;
    }

    data->fan_state    = (fan_cmd > 0.0f) ? ACT_ON : ACT_OFF;
    data->heater_state = (heater_cmd > 0.0f) ? ACT_ON : ACT_OFF;

    ActuatorHAL_SetFan(UNIT_PERCENT, fan_cmd);
    ActuatorHAL_SetHeater(UNIT_PERCENT, heater_cmd);
}
