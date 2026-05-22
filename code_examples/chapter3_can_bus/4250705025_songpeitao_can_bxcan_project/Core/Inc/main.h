#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

#define CAN_SIMULATION_LOOPBACK 0
#define CAN_PICSIMLAB_SOFT_DEMO 0

#define CAN_STATUS_LED_Pin GPIO_PIN_13
#define CAN_STATUS_LED_GPIO_Port GPIOC

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
