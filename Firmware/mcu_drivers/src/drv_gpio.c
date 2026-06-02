#include "drv_gpio.h"
#include "stm32f103xb.h" 


static GPIO_TypeDef* Get_STM32_Port(DRV_Port_t port) {
    switch(port) {
        case DRV_PORT_A: return GPIOA; 
        case DRV_PORT_B: return GPIOB; 
        case DRV_PORT_C: return GPIOC; 
        default:         return GPIOA; 
    }
}

/* ========================================================================= */
/* HÀM 1: Khởi tạo cứng (Cấu hình chân xuất tín hiệu thông thường)           */
/* ========================================================================= */
void DRV_GPIO_Init(void) {
    
    RCC->APB2ENR |= (RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN);

    
    GPIOB->CRL &= ~((0xF << 0) | (0xF << 4));

    
    GPIOB->CRL |=  ((0x3 << 0) | (0x3 << 4));

    
    GPIOB->BSRR = (1 << 16) | (1 << 17);
}

/* ========================================================================= */
/* HÀM 2: Ghi trạng thái chân (Nhận vào Số thứ tự chân: 0, 1, 2...)          */
/* ========================================================================= */
void DRV_GPIO_WritePin(DRV_Port_t port, uint16_t pin, uint8_t state) {
    GPIO_TypeDef* stm32_port = Get_STM32_Port(port);

    
    uint32_t bitmask = (1 << pin);

    if (state) {
        stm32_port->BSRR = bitmask;         
    } else {
        stm32_port->BSRR = (bitmask << 16); 
    }
}

/* ========================================================================= */
/* HÀM 3: Đọc trạng thái chân (Nhận vào Số thứ tự chân)                      */
/* ========================================================================= */
uint8_t DRV_GPIO_ReadPin(DRV_Port_t port, uint16_t pin) {
    GPIO_TypeDef* stm32_port = Get_STM32_Port(port);
    
    uint32_t bitmask = (1 << pin);

    if ((stm32_port->IDR & bitmask) != 0) {
        return 1; 
    } else {
        return 0; 
    }
}

/* ========================================================================= */
/* HÀM 4: Đảo trạng thái chân (Nhận vào Số thứ tự chân)                     */
/* ========================================================================= */
void DRV_GPIO_TogglePin(DRV_Port_t port, uint16_t pin) {
    GPIO_TypeDef* stm32_port = Get_STM32_Port(port);
    
    uint32_t bitmask = (1 << pin);

    
    if (stm32_port->ODR & bitmask) {
        stm32_port->BSRR = (bitmask << 16); 
    } else {
        stm32_port->BSRR = bitmask;         
    }
}
