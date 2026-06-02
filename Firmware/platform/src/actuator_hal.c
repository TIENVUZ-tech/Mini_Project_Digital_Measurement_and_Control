#include "actuator_hal.h"
#include "drv_gpio.h"
#include "drv_pwm.h"

/* ========================================================================= */
/* ĐỊNH NGHĨA PHẦN CỨNG (Dùng chuẩn Enum và Số thứ tự chân của Driver)      */
/* ========================================================================= */
#define HAL_PORT_HEATER     DRV_PORT_B  // Dùng đúng Enum định nghĩa trong drv_gpio.h
#define HAL_PIN_HEATER      0           // Chân số 0 (PB0) - Driver sẽ tự dịch bit

#define HAL_PORT_MIST       DRV_PORT_B  // Dùng đúng Enum định nghĩa trong drv_gpio.h
#define HAL_PIN_MIST        1           // Chân số 1 (PB1) - Driver sẽ tự dịch bit

#define HAL_PWM_CH_FAN      1           // Channel 1 của Timer 3 (PA6)

/* ========================================================================= */
/* HÀM 1: Khởi tạo tầng HAL (Cấu hình và đưa thiết bị về trạng thái an toàn)  */
/* ========================================================================= */
void ActuatorHAL_Init(void) {
    // 1. Phải gọi hàm khởi tạo phần cứng từ tầng Driver dưới lên
    DRV_GPIO_Init();
    DRV_PWM_Init();
    
    // 2. Kích hoạt bộ phát xung PWM cho Quạt
    DRV_PWM_Start(HAL_PWM_CH_FAN);
    
    // 3. Đảm bảo trạng thái ban đầu an toàn (Tắt hết thiết bị khi vừa có điện)
    DRV_GPIO_WritePin(HAL_PORT_HEATER, HAL_PIN_HEATER, 0); // Tắt máy sưởi
    DRV_GPIO_WritePin(HAL_PORT_MIST, HAL_PIN_MIST, 0);     // Tắt máy phun sương
    DRV_PWM_SetDuty(HAL_PWM_CH_FAN, 0);                    // Quạt 0% công suất
}

/* ========================================================================= */
/* HÀM 2: Điều khiển tốc độ Quạt bằng phần trăm (0 - 100%)                  */
/* ========================================================================= */
void ActuatorHAL_SetFanPWM(uint16_t duty) {
    DRV_PWM_SetDuty(HAL_PWM_CH_FAN, duty);
}

/* ========================================================================= */
/* HÀM 3: Điều khiển bật/tắt Máy Sưởi (1 = Bật, 0 = Tắt)                    */
/* ========================================================================= */
void ActuatorHAL_SetHeater(uint8_t state) {
    DRV_GPIO_WritePin(HAL_PORT_HEATER, HAL_PIN_HEATER, state);
}

/* ========================================================================= */
/* HÀM 4: Điều khiển bật/tắt Máy Phun Sương (1 = Bật, 0 = Tắt)               */
/* ========================================================================= */
void ActuatorHAL_SetMist(uint8_t state) {
    DRV_GPIO_WritePin(HAL_PORT_MIST, HAL_PIN_MIST, state);
}
