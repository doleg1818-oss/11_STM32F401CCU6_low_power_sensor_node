/*
 * board_power.c
 *
 *  Created on: Jul 13, 2026
 *      Author: Olegd
 */


#include "board_power.h"
#include "main.h"

extern UART_HandleTypeDef huart1;

void SystemClock_Config(void);


static void uart_pins_set_analog(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	// Set UART pin as Analog GPIO
	GPIO_InitTypeDef gpio = {0};
	gpio.Pin = GPIO_PIN_9|GPIO_PIN_10;
	gpio.Mode = GPIO_MODE_ANALOG;
	gpio.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &gpio);
}

void board_power_prepare_for_stop(void)
{
	HAL_UART_DeInit(&huart1);

	uart_pins_set_analog();
}
void board_power_prepare_after_stop(void)
{
	SystemClock_Config();

	if(HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
}
