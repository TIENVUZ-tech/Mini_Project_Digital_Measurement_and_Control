#include "control_app.h"
#include "pid_control.h"

/* Khai báo bộ điều khiển PID cho Quạt theo kiểu dữ liệu mới */
static PIDController_t fan_pid;

static float target_temperature = 30.0f;
static float target_humidity = 70.0f;

/* ========================================================================= */
/* 🛠️ CHỨNG NĂNG 1: KHỞI TẠO HỆ THỐNG                                         */
/* ========================================================================= */
void ControlApp_Init(void) {
    /* 1. Gán các thông số Kp, Ki, Kd nâng cao */

    fan_pid.Kp = 2.5f;
    fan_pid.Ki = 0.1f;
    fan_pid.Kd = 0.4f;

    /* 2. Gán thông số cấu hình chu kỳ và bộ lọc thông thấp khâu D */
    fan_pid.T   = 0.1f;          // Chu kỳ gọi hàm Update
    fan_pid.tau = 0.05f;         // Hằng số lọc nhiễu tần số cao cho khâu D (thường = 0.5 * T)

    /* 3. Cài đặt các ngưỡng giới hạn cứng cho đầu ra PWM và khâu tích phân */
    fan_pid.limMin = 0.0f;
    fan_pid.limMax = 100.0f;
    fan_pid.limMinInt = -40.0f;  // Giới hạn dưới khâu I chống vọt lố (Anti-windup)
    fan_pid.limMaxInt = 40.0f;   // Giới hạn trên khâu I chống vọt lố (Anti-windup)

    /* 4. Reset toàn bộ biến nhớ nội bộ về 0 */
    PID_Init(&fan_pid);
}

/* ========================================================================= */
/* 🔄 CHỨNG NĂNG 2: VÒNG LẶP ĐIỀU KHIỂN CHÍNH (Được gọi chu kỳ)               */
/* ========================================================================= */
void ControlApp_Update(GreenhouseData_t *data) {
    if (data == 0) return;

    // 1. KIỂM TRA LỖI CẢM BIẾN (Chức năng Fail-safe bảo vệ phần cứng)
    if (data->temperature < -40.0f || data->temperature > 80.0f || 
        data->air_humidity < 0.0f   || data->air_humidity > 100.0f) {
        data->state = STATE_ERROR; 
    }

    // Cập nhật các Setpoint từ cấu hình nội bộ vào struct chung
    data->temp_setpoint = target_temperature;
    data->hum_setpoint = target_humidity;

    // 2. MÁY TRẠNG THÁI CHÍNH ĐIỀU PHỐI NHÀ KÍNH
    switch (data->state) {
        case STATE_IDLE:
            data->fan_pwm = 0;
            data->fan_state = ACT_OFF;
            data->heater_state = ACT_OFF;
            data->mist_state = ACT_OFF;
            break;

        case STATE_AUTO:
            /* --- ĐIỀU KHIỂN NHIỆT ĐỘ KHÉP KÍN --- */
            // Gọi hàm tính toán từ thư viện pid_control chuyên dụng
            float pid_output = PID_Compute(&fan_pid, data->temp_setpoint, data->temperature);
            data->fan_pwm = (uint16_t)pid_output;
            data->fan_state = (data->fan_pwm > 0) ? ACT_ON : ACT_OFF;

            // Thuật toán vòng trễ Hysteresis cho Máy sưởi (Biên độ trễ +/- 1.0 độ C)
            float temp_hysteresis = 1.0f;
            if (data->temperature < (data->temp_setpoint - temp_hysteresis)) {
                data->heater_state = ACT_ON;   // Trời lạnh quá -> Bật sưởi
                data->fan_pwm = 0;             // Đang sưởi thì ép tắt quạt để tiết kiệm nhiệt
                data->fan_state = ACT_OFF;
            } else if (data->temperature > (data->temp_setpoint + temp_hysteresis)) {
                data->heater_state = ACT_OFF;  // Đủ ấm -> Tắt sưởi
            }

            /* --- ĐIỀU KHIỂN ĐỘ ẨM --- */
            // Thuật toán vòng trễ Hysteresis cho Phun sương (Biên độ trễ +/- 5.0%)
            float hum_hysteresis = 5.0f;
            if (data->air_humidity < (data->hum_setpoint - hum_hysteresis)) {
                data->mist_state = ACT_ON;     // Khô quá -> Bật phun sương
            } else if (data->air_humidity > (data->hum_setpoint + hum_hysteresis)) {
                data->mist_state = ACT_OFF;    // Đủ ẩm -> Tắt phun sương
            }
            break;

        case STATE_MANUAL:
            // Chế độ tay:nhận dữ liệu từ PC Tool sẽ ghi đè thẳng vào các biến phần cứng.
            break;

        case STATE_ERROR:
            // Trạng thái khẩn cấp: Ép tắt sạch sành sanh mọi thiết bị phần cứng
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
/* 🤝 CHỨNG NĂNG 3: CÁ C HÀM API GIAO TIẾP VỚI CÁC FILE KHÁC                  */
/* ========================================================================= */

// Hàm đổi chế độ hệ thống (Gọi hàm Init để reset sạch bộ nhớ tránh sốc dòng khi đổi chế độ)
void ControlApp_SetMode(SystemState_t state) {
    PID_Init(&fan_pid);
}

// Cập nhật nhiệt độ mong muốn từ PC gửi xuống
void ControlApp_SetTempSetpoint(float temp) {
    target_temperature = temp;
}

// Cập nhật độ ẩm mong muốn từ PC gửi xuống thông qua ông Tiến
void ControlApp_SetHumSetpoint(float hum) {
    target_humidity = hum;
}

// Hàm Tune thông số PID thời gian thực trực tiếp từ giao diện máy tính
void ControlApp_SetPIDTune(float p, float i, float d) {
    fan_pid.Kp = p;
    fan_pid.Ki = i;
    fan_pid.Kd = d;
}
