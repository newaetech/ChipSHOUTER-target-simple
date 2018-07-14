/*_
	This file is part of the ChipWhisperer Example Targets
	Copyright (C) 2018 NewAE Technology Inc.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hal.h"
#include <stdint.h>
#include <stdlib.h>

volatile uint8_t glitched = 0;
void delay100ms(uint32_t s);

//glitch() constants
#define RUN_CNT 2000
#define GLITCH_CNT 3

#define OUTER_LOOP_CNT 300
#define INNER_LOOP_CNT 300

//delay500ms() constants
#ifdef DIV_HSI_512
#define BLINK_PERIOD 129
#else
#define BLINK_PERIOD 66000
#endif

//startup_blink() constants
#define BLINK_TOT 3    

void glitch(void)
{
	volatile uint32_t i, j;
	volatile uint32_t cnt;
    uint32_t blink_status = 1;
	
	uint32_t run_cnt = 0;
	uint32_t glitch_cnt = 0;
	for(run_cnt = 0; run_cnt < RUN_CNT; run_cnt++){
		//run led on
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, blink_status);
        blink_status ^= 1;
        cnt = 0;
		for(i = 0; i < OUTER_LOOP_CNT; i++) {
			for(j=0; j < INNER_LOOP_CNT; j++){
				cnt++;
			}
		}
		
		//look for glitch
		if (i != OUTER_LOOP_CNT || j != INNER_LOOP_CNT || cnt != (OUTER_LOOP_CNT * INNER_LOOP_CNT)) {
			//if glitched, reset the run count and blink the fault LED
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, SET);
			delay100ms(3);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, RESET);
            run_cnt = 0;
		}
	}
}


void delay100ms(uint32_t s)
{
	volatile uint32_t blink_time = 0;
	volatile uint32_t blink_num = 0;
	for (blink_num = 0; blink_num < s; blink_num++)
		for (blink_time = 0; blink_time < BLINK_PERIOD; blink_time++);
}


void startup_blink(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4 | GPIO_PIN_5, RESET);
	volatile uint32_t blink_num;
	for (blink_num = 0; blink_num < 1; blink_num++) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, SET);
		delay100ms(1);
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, RESET);
		delay100ms(1);
	}
	
}


void osc_setup(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI;
	RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
	RCC_OscInitStruct.HSIState       = RCC_HSI_OFF;
	RCC_OscInitStruct.PLL.PLLSource  = RCC_PLL_NONE;
	RCC_OscInitStruct.LSIState		 = RCC_LSI_OFF;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);
	
/*
    Previous code had option for slower clock - this does not work well for 
    actually inserting the clock itself.
	#ifdef DIV_HSI_512
	//can't use LSI as system clock, but can divide HSI clock
	RCC_ClkInitTypeDef clk;
	clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK;
	clk.AHBCLKDivider = RCC_SYSCLK_DIV512; //as slow as possible
	clk.SYSCLKSource   = RCC_SYSCLKSOURCE_HSI;
	uint32_t flash_latency = 5;
	HAL_RCC_ClockConfig(&clk, flash_latency);
	#endif
*/
}

void power_setup(void)
{
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWR_EnableBkUpAccess();
}

void init_GPIOB(void)
{
	__HAL_RCC_GPIOB_CLK_ENABLE();
	GPIO_InitTypeDef GpioInit;
	GpioInit.Pin       = GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3;
	GpioInit.Mode      = GPIO_MODE_OUTPUT_PP;
	GpioInit.Pull      = GPIO_NOPULL;
	GpioInit.Speed     = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &GpioInit);
}

void enter_standby(void)
{
	HAL_PWR_DisablePVD(); //disable power voltage detector
	HAL_PWR_EnterSTANDBYMode();
}

int main(void)
{
	osc_setup();
	power_setup();
	init_GPIOB();
	
	startup_blink();
	//enter glitch loop
	glitch();
	
	//enter standby mode
	enter_standby();
	
	while(1);
	while(1);

	return 0;
}
