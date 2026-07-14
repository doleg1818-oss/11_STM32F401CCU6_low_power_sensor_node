/*
 * power_manager.h
 *
 *  Created on: Jul 13, 2026
 *      Author: Olegd
 */

#ifndef INC_POWER_MANAGER_H_
#define INC_POWER_MANAGER_H_

#include "stdbool.h"
#include "stdint.h"

#include "stm32f4xx_hal.h"

typedef enum
{
	POWER_WAKEUP_SOURCE_NONE = 0,
	POWER_WAKEUP_SOURCE_RTC,
	POWER_WAKEUP_SOURCE_BUTTON
}power_wakeup_source_t;

typedef enum
{
	POWER_RESET_SOURCE_POWER_ON = 0,
	POWER_RESET_SOURCE_STENDBY_RTC,
	POWER_RESET_SOURCE_STANDBY_BUTTON
}power_reset_source_t;

typedef struct
{
	RTC_HandleTypeDef *hrtc;

	uint32_t rtc_wakeup_counter;
	uint32_t rtc_wakeup_clock;

	uint32_t wakeup_pin;
	uint16_t button_gpio_pin;
}power_manager_config_t;

bool power_manager_init(const power_manager_config_t *config);
bool power_manager_configure_rtc_wokeup(void);

void power_manager_enter_sleep(void);
void power_manager_enter_stop(void);
void power_manager_enter_stenbby(void);

power_wakeup_source_t power_manager_get_wakeup_source(void);
void power_managere_clear_wakeup_source(void);
power_reset_source_t power_manager_detect_reset_sourse(RTC_HandleTypeDef *hrtc);


#endif /* INC_POWER_MANAGER_H_ */
