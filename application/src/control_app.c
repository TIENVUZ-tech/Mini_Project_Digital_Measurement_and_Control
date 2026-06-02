#include "control_app.h"

static PID_Controller_t fan_pid;

void ControlApp_Init(void) {
    fan_pid.kp = 2.5f;           
    fan_pid.ki = 0.1f;           
    fan_pid.kd = 0.4f;           
    fan_pid.integral = 0.0f;
    fan_pid.prev_error = 0.0f;
    fan_pid.max_output = 100.0f; 
    fan_pid.min_output = 0.0f;   
    fan_pid.max_integral = 40.0f;
}

static float PID_Compute(PID_Controller_t *pid, float setpoint, float feedback) {
    float error = feedback - setpoint; 
    if (error < 0) {
        pid->integral = 0;
        pid->prev_error = 0;
        return 0;
    }
    float p_term = pid->kp * error;
    pid->integral += error;
    if (pid->integral > pid->max_integral) pid->integral = pid->max_integral;
    if (pid->integral < -pid->max_integral) pid->integral = -pid->max_integral;
    float i_term = pid->ki * pid->integral;
    float d_term = pid->kd * (error - pid->prev_error);
    pid->prev_error = error;
    float output = p_term + i_term + d_term;
    if (output > pid->max_output) output = pid->max_output;
    if (output < pid->min_output) output = pid->min_output;
    return output;
}

void ControlApp_Update(GreenhouseData_t *data) {
    if (data == 0) return;

    // 1. KI?M TRA L?I C?M BI?N
    if (data->temperature < -40.0f || data->temperature > 80.0f || 
        data->air_humidity < 0.0f   || data->air_humidity > 100.0f) {
        data->state = STATE_ERROR; 
    }

    // 2. MÁY TR?NG THÁI CHÍNH
    switch (data->state) {
        case STATE_IDLE:
            data->fan_pwm = 0;
            data->fan_state = ACT_OFF;
            data->heater_state = ACT_OFF;
            data->mist_state = ACT_OFF;
            break;

        case STATE_AUTO:
            // PID Qu?t
            float pid_output = PID_Compute(&fan_pid, data->temp_setpoint, data->temperature);
            data->fan_pwm = (uint16_t)pid_output;
            data->fan_state = (data->fan_pwm > 0) ? ACT_ON : ACT_OFF;

            // Hysteresis Nhi?t ð? (T_set +/- 1)
            float temp_hysteresis = 1.0f;
            if (data->temperature < (data->temp_setpoint - temp_hysteresis)) {
                data->heater_state = ACT_ON;   
                data->fan_pwm = 0;
                data->fan_state = ACT_OFF;
            } else if (data->temperature > (data->temp_setpoint + temp_hysteresis)) {
                data->heater_state = ACT_OFF;  
            }

            // Hysteresis Ð? ?m (H_set +/- 5)
            float hum_hysteresis = 5.0f;
            if (data->air_humidity < (data->hum_setpoint - hum_hysteresis)) {
                data->mist_state = ACT_ON;     
            } else if (data->air_humidity > (data->hum_setpoint + hum_hysteresis)) {
                data->mist_state = ACT_OFF;    
            }
            break;

        case STATE_MANUAL:
            break;

        case STATE_ERROR:
            data->fan_pwm = 0;
            data->fan_state = ACT_OFF;
            data->heater_state = ACT_OFF;
            data->mist_state = ACT_OFF;
            break;

        default:
            data->state = STATE_IDLE;
            break;
    }
}

void ControlApp_SetMode(SystemState_t state) {
    fan_pid.integral = 0;
    fan_pid.prev_error = 0;
}
void ControlApp_SetTempSetpoint(float temp) {}
void ControlApp_SetHumSetpoint(float hum) {}
void ControlApp_SetPIDTune(float p, float i, float d) {
    fan_pid.kp = p; fan_pid.ki = i; fan_pid.kd = d;
}
