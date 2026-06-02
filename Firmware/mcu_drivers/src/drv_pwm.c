#include "drv_pwm.h"
#include "stm32f103xb.h"

/* ========================================================================= */
/* HÀM 1: Khởi tạo bộ Timer 3 chạy chế độ PWM 1 tại chân PA6                */
/* ========================================================================= */
void DRV_PWM_Init(void) {
    // 1. Bật Clock hệ thống
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // 2. Cấu hình chân PA6 làm Alternate Function Push-Pull 50MHz
    GPIOA->CRL &= ~(0xF << 24);
    GPIOA->CRL |=  (0xB << 24); 

    // 3. Cấu hình tần số đếm (PSC=71, ARR=999 -> Tần số PWM = 1kHz)
    TIM3->PSC = 71;           
    TIM3->ARR = 999;          

    // 4. Cấu hình chế độ PWM Mode 1 và bật Preload cho Channel 1
    TIM3->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_OC1PE);
    TIM3->CCMR1 |= (0x6 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;

    // 5. Mặc định ban đầu tắt xung (Duty = 0%)
    TIM3->CCR1 = 0;

    // BAN ĐẦU CHƯA CHO PHÁT XUNG (Để hàm Start lo)
    TIM3->CCER &= ~TIM_CCER_CC1E;
    TIM3->CR1  &= ~TIM_CR1_CEN;
}

/* ========================================================================= */
/* HÀM 2: Bắt đầu phát xung PWM                                             */
/* ========================================================================= */
void DRV_PWM_Start(uint8_t channel) {
    if (channel == 1) {
        TIM3->CCER |= TIM_CCER_CC1E; // Cho phép xuất xung ra chân PA6
        TIM3->CR1  |= TIM_CR1_CEN;  // Cho phép bộ đếm Timer chạy
    }
}

/* ========================================================================= */
/* HÀM 3: Set Duty từ 0 đến 100% (Quy đổi thông minh)                       */
/* ========================================================================= */
void DRV_PWM_SetDuty(uint8_t channel, uint16_t duty) {
    if (channel == 1) {
        // Khống chế giá trị đầu vào tối đa là 100%
        if (duty > 100) duty = 100;
        
        // Ép kiểu sang uint32_t khi tính toán để tuyệt đối an toàn, tránh tràn số
        uint32_t ccr_value = ((uint32_t)duty * 999) / 100;
        
        TIM3->CCR1 = ccr_value; 
    }
}

/* ========================================================================= */
/* HÀM 4: Dừng phát xung PWM                                                */
/* ========================================================================= */
void DRV_PWM_Stop(uint8_t channel) {
    if (channel == 1) {
        TIM3->CCR1 = 0;              // Đưa công suất về 0%
        TIM3->CCER &= ~TIM_CCER_CC1E; // Ngắt xuất xung ra chân vật lý
        TIM3->CR1  &= ~TIM_CR1_CEN;  // Dừng bộ đếm Timer để tiết kiệm điện
    }
}
