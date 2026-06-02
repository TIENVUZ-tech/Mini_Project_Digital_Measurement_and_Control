#ifndef ACTUATOR_HAL_H_
#define ACTUATOR_HAL_H_

#include <stdint.h> 



// Hàm khởi tạo tầng HAL (bên trong sẽ gọi các Init của tầng Driver)
void ActuatorHAL_Init(void);

// Hàm điều khiển Quạt bằng phần trăm tốc độ (Truyền vào từ 0 đến 100%)
void ActuatorHAL_SetFanPWM(uint16_t duty);

// Hàm điều khiển bật/tắt Máy Sưởi (state: 1 = Bật, 0 = Tắt)
void ActuatorHAL_SetHeater(uint8_t state);

// Hàm điều khiển bật/tắt Máy Phun Sương (state: 1 = Bật, 0 = Tắt)
void ActuatorHAL_SetMist(uint8_t state);

#endif 
