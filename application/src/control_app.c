#include "control_app.h"

static PID_Controller_t fan_pid;

static float target_temperature = 30.0f;
static float target_humidity = 70.0f;

/* ========================================================================= */
/* 🛠️ CHỨNG NĂNG 1: KHỞI TẠO HỆ THỐNG                                         */
/* ========================================================================= */
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

/* ========================================================================= */
/* 🧮 CHỨNG NĂNG 2: THUẬT TOÁN TOÁN HỌC PID QUẠT                             */
/* ========================================================================= */
static float PID_Compute(PID_Controller_t *pid, float setpoint, float feedback) {
    // Sỹ lưu ý: Quạt dùng để LÀM MÁT, nên nếu Nhiệt độ thực tế (feedback)
    // lớn hơn nhiệt độ cài đặt (setpoint) thì mới có sai số dương để quạt quay nhé!
    float error = feedback - setpoint; 

    // Nếu môi trường đang mát hơn hoặc bằng nhiệt độ cài đặt -> Tắt quạt, xóa tích phân
    if (error < 0.0f) {
        pid->integral = 0.0f;
        pid->prev_error = 0.0f;
        return 0.0f;
    }

    // Khâu tỷ lệ P
    float p_term = pid->kp * error;

    // Khâu tích phân I (Tích lũy sai số)
    pid->integral += error;

    // Chống bão hòa tích phân (Anti-windup bằng biến max_integral nhóm mới thêm)
    if (pid->integral > pid->max_integral)  pid->integral = pid->max_integral;
    if (pid->integral < -pid->max_integral) pid->integral = -pid->max_integral;
    float i_term = pid->ki * pid->integral;

    // Khâu đạo hàm D
    float d_term = pid->kd * (error - pid->prev_error);
    pid->prev_error = error;

    // Tổng hợp đầu ra
    float output = p_term + i_term + d_term;

    // Giới hạn đầu ra PWM từ 0 - 100%
    if (output > pid->max_output) output = pid->max_output;
    if (output < pid->min_output) output = pid->min_output;

    return output;
}

/* ========================================================================= */
/* 🔄 CHỨNG NĂNG 3: VÒNG LẶP ĐIỀU KHIỂN CHÍNH (Được gọi chu kỳ)               */
/* ========================================================================= */
void ControlApp_Update(GreenhouseData_t *data) {
    if (data == 0) return;

    // 1. KIỂM TRA LỖI CẢM BIẾN (Bảo vệ hệ thống của Sỹ cực tốt)
    if (data->temperature < -40.0f || data->temperature > 80.0f || 
        data->air_humidity < 0.0f   || data->air_humidity > 100.0f) {
        data->state = STATE_ERROR; 
    }

    // Cập nhật các Setpoint từ cấu hình nội bộ vào struct chung để Tiến dễ đọc
    data->temp_setpoint = target_temperature;
    data->hum_setpoint = target_humidity;

    // 2. MÁY TRẠNG THÁI CHÍNH
    switch (data->state) {
        case STATE_IDLE:
            data->fan_pwm = 0;
            data->fan_state = ACT_OFF;
            data->heater_state = ACT_OFF;
            data->mist_state = ACT_OFF;
            break;

        case STATE_AUTO:
            // --- ĐIỀU KHIỂN NHIỆT ĐỘ (Quạt PID + Sưởi Hysteresis) ---
            float pid_output = PID_Compute(&fan_pid, data->temp_setpoint, data->temperature);
            data->fan_pwm = (uint16_t)pid_output;
            data->fan_state = (data->fan_pwm > 0) ? ACT_ON : ACT_OFF;

            // Vòng trễ Hysteresis cho Máy sưởi (T_set +/- 1)
            float temp_hysteresis = 1.0f;
            if (data->temperature < (data->temp_setpoint - temp_hysteresis)) {
                data->heater_state = ACT_ON;   // Lạnh quá -> Bật sưởi
                data->fan_pwm = 0;             // Đang sưởi thì tắt quạt kẻo lãng phí nhiệt
                data->fan_state = ACT_OFF;
            } else if (data->temperature > (data->temp_setpoint + temp_hysteresis)) {
                data->heater_state = ACT_OFF;  // Đủ ấm -> Tắt sưởi
            }

            // --- ĐIỀU KHIỂN ĐỘ ẨM (Phun sương Hysteresis) ---
            float hum_hysteresis = 5.0f;
            if (data->air_humidity < (data->hum_setpoint - hum_hysteresis)) {
                data->mist_state = ACT_ON;     // Khô quá -> Bật phun sương
            } else if (data->air_humidity > (data->hum_setpoint + hum_hysteresis)) {
                data->mist_state = ACT_OFF;    // Đủ ẩm -> Tắt phun sương
            }
            break;

        case STATE_MANUAL:
            // Chế độ tay: Giữ nguyên các giá trị thiết bị.
            // Ông Tiến (Comm) nhận dữ liệu từ PC Tool sẽ tự ghi đè thẳng vào các biến
            // data->fan_pwm, data->heater_state... Sỹ không can thiệp thuật toán ở đây.
            break;

        case STATE_ERROR:
            // Trạng thái khẩn cấp: Ép tắt toàn bộ phần cứng
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

/* ========================================================================= */
/* 🤝 CHỨNG NĂNG 4: CÁC HÀM API GIAO TIẾP (Bản hợp đồng với ông Tiến)        */
/* ========================================================================= */

// Hàm đổi chế độ hệ thống (Xóa bộ nhớ PID cũ để tránh bị sốc dòng khi đổi chế độ)
void ControlApp_SetMode(SystemState_t state) {
    // Sỹ ép kiểu an toàn từ SystemState_t sang GreenhouseState_t nếu cần
    // Nhưng nhiệm vụ chính ở đây là reset khâu tích phân PID
    fan_pid.integral = 0.0f;
    fan_pid.prev_error = 0.0f;
}

// Cập nhật nhiệt độ mong muốn từ PC gửi xuống
void ControlApp_SetTempSetpoint(float temp) {
    target_temperature = temp;
}

// Cập nhật độ ẩm mong muốn từ PC gửi xuống
void ControlApp_SetHumSetpoint(float hum) {
    target_humidity = hum;
}

// Hàm Tune PID trực tiếp từ giao diện máy tính của Thúy
void ControlApp_SetPIDTune(float p, float i, float d) {
    fan_pid.kp = p;
    fan_pid.ki = i;
    fan_pid.kd = d;
}
