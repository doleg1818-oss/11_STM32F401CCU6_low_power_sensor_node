/*
 * power_manager.c
 *
 *  Created on: Jul 13, 2026
 *      Author: Olegd
 */

#include "power_manager.h"
#include "board_power.h"

static power_manager_config_t power_config;

static bool power_manager_initialized = false;

static volatile power_wakeup_source_t wakeup_souerce = POWER_WAKEUP_SOURCE_NONE;

bool power_manager_init(const power_manager_config_t *config)
{
	if(config == NULL || config->hrtc == NULL)
	{
		return false;
	}
	power_config = *config;
	power_manager_initialized = true;
	return true;
}

bool power_manager_configure_rtc_wakeup(void)
{
	if(power_manager_initialized == false)
	{
		return false;
	}
	if(HAL_RTCEx_DeactivateWakeUpTimer(power_config.hrtc) != HAL_OK)
	{
		return false;
	}
	if(HAL_RTCEx_SetWakeUpTimer_IT(power_config.hrtc, power_config.rtc_wakeup_counter, power_config.rtc_wakeup_clock))
	{
		return false;
	}
	return true;
}
void power_manager_enter_sleep(void)
{
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}
void power_manager_enter_stop(void)
{
	if(power_manager_initialized == false)
	{
		return;
	}
	board_power_prepare_for_stop();

	HAL_SuspendTick();
	HAL_PWREx_EnableFlashPowerDown();		// Power down flash memory when MCU is in STOM mode

	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );

	board_power_prepare_after_stop();
	HAL_ResumeTick();
}
void power_manager_enter_stenbby(void)
{
	if(power_manager_initialized == false)
	{
		return;
	}
	board_power_prepare_for_stop();

	HAL_PWR_DisableWakeUpPin(power_config.wakeup_pin);

	if(power_manager_configure_rtc_wakeup() == false)
	{
		Error_Handler();
	}
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	HAL_PWR_EnableWakeUpPin(power_config.wakeup_pin);

	HAL_PWR_EnterSTANDBYMode();
}

power_wakeup_source_t power_manager_get_wakeup_source(void)
{
	return wakeup_souerce;
}
void power_managere_clear_wakeup_source(void)
{
	wakeup_souerce = POWER_WAKEUP_SOURCE_NONE;
}
power_reset_source_t power_manager_detect_reset_sourse(RTC_HandleTypeDef *hrtc)
{
	if(hrtc == NULL)
	{
		return POWER_RESET_SOURCE_POWER_ON;
	}
	__HAL_RCC_PWR_CLK_ENABLE();

	hrtc->Instance = RTC;
	if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) == RESET)
	{
		return POWER_RESET_SOURCE_POWER_ON;
	}

	power_reset_source_t sourse;

	if(__HAL_RTC_WAKEUPTIMER_GET_FLAG(hrtc, RTC_FLAG_WUTF) != RESET)
	{
		sourse = POWER_RESET_SOURCE_STENDBY_RTC;
	}
	else
	{
		sourse = POWER_RESET_SOURCE_STANDBY_BUTTON;
	}

	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

	return sourse;
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
	if(hrtc->Instance == RTC)
	{
		wakeup_souerce = POWER_WAKEUP_SOURCE_RTC;
	}

}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_0)
	{
		wakeup_souerce = POWER_WAKEUP_SOURCE_BUTTON;
	}
}


















