#include "pid_control.h"


void PID_Init(PIDController_t *pid) {
    
    pid->integrator      = 0.0f;
    pid->prevError       = 0.0f;
    pid->differentiator  = 0.0f;
    pid->prevMeasurement = 0.0f;
    pid->out             = 0.0f;
}

/**
 * @brief Thuật toán toán học PID nâng cao (Tích phân hình thang + Lọc khâu D)
 * @note Đã được cấu hình cho hệ LÀM MÁT (Quạt)
 */
float PID_Compute(PIDController_t *pid, float setpoint, float measurement) {
    
    /* ------------------------------------------------------------------------- */
    /* 1. LÀM MÁT LOGIC: Sai số = Thực tế (Measurement) - Cài đặt (Setpoint)     */
    /* ------------------------------------------------------------------------- */
    float error = measurement - setpoint;

    // Nếu môi trường đang mát hơn hoặc bằng nhiệt độ cài đặt -> Tắt quạt, xóa nhớ
    if (error < 0.0f) {
        pid->integrator = 0.0f;
        pid->prevError  = 0.0f;
        return 0.0f;
    }

    /* ------------------------------------------------------------------------- */
    /* 2. KHÂU TỶ LỆ (P Term)                                                    */
    /* ------------------------------------------------------------------------- */
    float proportional = pid->Kp * error;

    /* ------------------------------------------------------------------------- */
    /* 3. KHÂU TÍCH PHÂN (I Term) - Tính theo phương pháp hình thang mượt mà     */
    /* ------------------------------------------------------------------------- */
    pid->integrator = pid->integrator + 0.5f * pid->Ki * pid->T * (error + pid->prevError);

    /* Chống bão hòa tích phân chủ động (Anti-windup Clamping) */
    if (pid->integrator > pid->limMaxInt) {
        pid->integrator = pid->limMaxInt;
    } else if (pid->integrator < pid->limMinInt) {
        pid->integrator = pid->limMinInt;
    }

    /* ------------------------------------------------------------------------- */
    /* 4. KHÂU ĐẠO HÀM (D Term) - Tích hợp bộ lọc thông thấp chống nhiễu cảm biến*/
    /* ------------------------------------------------------------------------- */
    // Tính toán khâu D dựa trên sự thay đổi của Measurement để chống sốc đạo hàm (Derivative Kick)
    pid->differentiator = (2.0f * pid->Kd * (measurement - pid->prevMeasurement)
                          + (2.0f * pid->tau - pid->T) * pid->differentiator)
                          / (2.0f * pid->tau + pid->T);

    /* ------------------------------------------------------------------------- */
    /* 5. TỔNG HỢP ĐẦU RA VÀ GIỚI HẠN CHU KỲ XUNG PWM                             */
    /* ------------------------------------------------------------------------- */
    // Vì khâu D tính trên giá trị đo thực tế, xu hướng tăng nhiệt độ sẽ làm khâu D dương,
    // hỗ trợ quạt tăng tốc nhanh hơn để làm mát nên tụi mình dùng dấu CỘNG (+) khâu D.
    pid->out = proportional + pid->integrator + pid->differentiator;

    /* Ép đầu ra nằm trong giới hạn cứng [limMin, limMax] (0.0% đến 100.0%) */
    if (pid->out > pid->limMax) {
        pid->out = pid->limMax;
    } else if (pid->out < pid->limMin) {
        pid->out = pid->limMin;
    }

    /* Lưu lại trạng thái chu kỳ này để dùng cho chu kỳ kế tiếp */
    pid->prevError       = error;
    pid->prevMeasurement = measurement;

    /* Trả về giá trị điều khiển cuối cùng */
    return pid->out;
}
