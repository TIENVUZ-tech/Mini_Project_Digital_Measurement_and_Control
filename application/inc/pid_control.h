#ifndef PID_CONTROL_H
#define PID_CONTROL_H

typedef struct {
    /* Hệ số bộ điều khiển */
    float Kp;
    float Ki;
    float Kd;

    /* Hằng số thời gian bộ lọc thông thấp khâu D (Low-pass filter) */
    float tau;

    /* Giới hạn đầu ra PWM (0.0 đến 100.0%) */
    float limMin;
    float limMax;
    
    /* Giới hạn khâu tích phân (Chống bão hòa Anti-windup) */
    float limMinInt;
    float limMaxInt;

    /* Thời gian lấy mẫu chu kỳ */
    float T;

    /* Bộ nhớ trạng thái của bộ điều khiển */
    float integrator;
    float prevError;
    float differentiator;
    float prevMeasurement;

    /* Đầu ra tính toán của bộ điều khiển */
    float out;
} PIDController_t;

/* Khai báo API công khai */
void PID_Init(PIDController_t *pid);
float PID_Compute(PIDController_t *pid, float setpoint, float measurement);

#endif /* PID_CONTROL_H */
