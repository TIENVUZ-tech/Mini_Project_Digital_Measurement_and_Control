#include "control_app.h"
#include "pid_control.h"

static PIDController_t fan_pid;

static float target_temperature = 30.0f;

void ControlApp_Init(void) {
    fan_pid.Kp = 2.5f;
    fan_pid.Ki = 0.1f;
    fan_pid.Kd = 0.4f;

    fan_pid.T   = 0.1f;          
    fan_pid.tau = 0.05f;         

    fan_pid.limMin = 0.0f;
    fan_pid.limMax = 100.0f;
    fan_pid.limMinInt = -40.0f;  
    fan_pid.limMaxInt = 40.0f;   

    PID_Init(&fan_pid);
}

void ControlApp_Update(GreenhouseData_t *data) {
    if (data == 0) return;

    data->temp_setpoint = target_temperature;

    switch (data->mode) {
        case MODE_AUTO: {
            
            if (data->temperature < -40.0f || data->temperature > 80.0f) {
                data->fan_pwm = 0;
                data->fan_state = ACT_OFF;
                data->heater_state = ACT_OFF;
                break; 
            }

            float pid_output = PID_Compute(&fan_pid, data->temp_setpoint, data->temperature);
            data->fan_pwm = (uint16_t)pid_output;
            data->fan_state = (data->fan_pwm > 0) ? ACT_ON : ACT_OFF;

            float temp_hysteresis = 1.0f;
            if (data->temperature < (data->temp_setpoint - temp_hysteresis)) {
                data->heater_state = ACT_ON;   
                data->fan_pwm = 0;             
                data->fan_state = ACT_OFF;
            } else if (data->temperature > (data->temp_setpoint + temp_hysteresis)) {
                data->heater_state = ACT_OFF;  
            }
            break;
        }

        case MODE_MANUAL:
            break;

        default:
            data->mode = MODE_MANUAL;
            break;
    }
}

void ControlApp_SetMode(SystemMode_t mode) {
    PID_Init(&fan_pid);
}

void ControlApp_SetTempSetpoint(float temp) {
    target_temperature = temp;
}

void ControlApp_SetPIDTune(float p, float i, float d) {
    fan_pid.Kp = p;
    fan_pid.Ki = i;
    fan_pid.Kd = d;
}