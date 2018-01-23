#ifndef STM32F3_HAL_H
#define STM32F3_HAL_H

//You probably don't need this from rest of code
#include "stm32f3_hal.h"
#include "stm32f3_hal_lowlevel.h"
#include "stm32f3xx_hal_rcc.h"
#include "stm32f3xx_hal_gpio.h"
#include "stm32f3xx_hal_dma.h"
#include "stm32f3xx_hal_pwr.h"


void init_uart(void);
void putch(char c);
char getch(void);

void trigger_setup(void);
void trigger_low(void);
void trigger_high(void);

#endif // STM32F3_HAL_H
