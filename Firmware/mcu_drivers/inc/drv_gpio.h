#ifndef DRV_GPIO_H_     
#define DRV_GPIO_H_     

#include <stdint.h>     
typedef enum {
    DRV_PORT_A = 0,
    DRV_PORT_B = 1,
    DRV_PORT_C = 2
} DRV_Port_t;


void DRV_GPIO_Init(void);


void DRV_GPIO_WritePin(DRV_Port_t port, uint16_t pin, uint8_t state);


uint8_t DRV_GPIO_ReadPin(DRV_Port_t port, uint16_t pin);


void DRV_GPIO_TogglePin(DRV_Port_t port, uint16_t pin);

#endif 

