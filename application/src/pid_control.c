#include "pid_control.h"

// Khởi tạo các thông số cơ bản cho PID
void PID_Init(PID_Controller_t *pid, float kp, float ki, float kd, float min, float max) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    
    pid->out_min = min;
    pid->out_max = max;
    
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}

// Hàm tính toán logic, gọi liên tục trong ControlApp_Run()
float PID_Compute(PID_Controller_t *pid, float setpoint, float feedback) {
    float error;
    float p_term, d_term;
    float output;

    error = setpoint - feedback;

    p_term = pid->kp * error;

    pid->integral += error;
    
    d_term = pid->kd * (error - pid->prev_error);
    pid->prev_error = error;


    output = p_term + (pid->ki * pid->integral) + d_term;

   
    if (output > pid->out_max) {
        output = pid->out_max;
    
        pid->integral -= error; 
    } 
    else if (output < pid->out_min) {
        output = pid->out_min;
        pid->integral -= error;
    }

    return output;
}