/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

bool woke_from_standby = false;


typedef enum
{
	WAKEUP_SOURCE_NONE = 0,
	WAKEUP_SOURCE_RTC,
	WAKEUP_SOURCE_BUTTON
}wakeup_source_t;
static volatile wakeup_source_t wakeup_source = WAKEUP_SOURCE_NONE;


typedef enum
{
	RESET_SOURCE_POWER_ON = 0,
	RESET_SOURCE_STENDBY_RTC,
	RESET_SOURCE_STANDBY_BUTTON
}reset_source_stendby_t;
static reset_source_stendby_t reset_source_stendby = RESET_SOURCE_POWER_ON;


void set_led(uint8_t state)
{
	if(state == 0)
	{
		HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_SET);
	}
	if(state == 1)
	{
		HAL_GPIO_WritePin(GPIOC, LED_Pin, GPIO_PIN_RESET);
	}
}

void stanby_mode_check_interrupt_sourse(void)
{
	 __HAL_RCC_PWR_CLK_ENABLE();

	 hrtc.Instance = RTC;

	 if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
	 {
		 if(__HAL_RTC_WAKEUPTIMER_GET_FLAG(&hrtc, RTC_FLAG_WUTF) != RESET)
		 {
			 reset_source_stendby = RESET_SOURCE_STENDBY_RTC;
		 }
		 else
		 {
			 reset_source_stendby = RESET_SOURCE_STANDBY_BUTTON;
		 }
		  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
		  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	  }
	 else
	 {
		 reset_source_stendby = RESET_SOURCE_POWER_ON;
	 }
}

void deinit_uart(void)
{
	HAL_UART_DeInit(&huart1);

	// Set UART pin as Analog GPIO
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

static void rtc_wakeup_timer_settings(void)
{
	HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

	if(HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 2048, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
	{
		Error_Handler();
	}
}

static void enter_stop_mode(void)
{
	// ALL UNUSED PINS HAS TOBE IN "ANALOG MODE" MODE Mo pull-downn and no pull up !!!!!!
	// PA 0 HAS BE "GPIO EXTI0" Mode !!!!
	deinit_uart();

	HAL_SuspendTick();

	HAL_PWREx_EnableFlashPowerDown();		// Power down flash memory when MCU is in STOM mode

	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );

	SystemClock_Config();
	HAL_ResumeTick();

	MX_USART1_UART_Init();
}
static void enter_standby_mode(void)
{
	// ALL UNUSED PINS HAS TOBE IN "ANALOG MODE" MODE Mo pull-downn and no pull up!!!!!!
	// PA 0 HAS BE "SYS WKUP" Mode !!!!
	deinit_uart();
	HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
	HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
	if(HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 18123, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)  // 10 sec
	{
		Error_Handler();
	}
//	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
	HAL_PWR_EnterSTANDBYMode();

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  stanby_mode_check_interrupt_sourse();


  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */





  //HAL_Delay(1000);
  for(uint8_t i = 0; i<20; i++)
  {
	  set_led(1);
 	  HAL_Delay(100);
 	  set_led(0);
 	  HAL_Delay(100);
   }
   set_led(0);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  // Sleep MODE. Power consumption 3mA  ///////////////////////////////////////////////////////////////////////////////////////////////////////////
//	  set_led(1);
//
//	  const char msg[] = "Run mode active\n\r";
//	  HAL_UART_Transmit(&huart1, (uint8_t *)msg, sizeof(msg)-1, HAL_MAX_DELAY);
//	  HAL_Delay(100);
//
//	  set_led(0);
//	  uint32_t stop_start = HAL_GetTick();
//	  while((HAL_GetTick() - stop_start) < 900)
//	  {
//		  enter_sleep_mode();
//	  }
	  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



	  // Stop mode. Power consumption 25uA. Wake up from RTC or button ///////////////////////////////////////////////////////////////////////////////
//	  static bool init = 0;
//	  if(init == 0)
//	  {
//		  init = 1;
//		  rtc_wakeup_timer_settings();
//	  }
//
//	  switch(wakeup_source)
//	  {
//	  	  case WAKEUP_SOURCE_RTC:
//	  	  {
//	  		  char buff[50] = "Wake up from RTC\n\r";
//	  		  HAL_UART_Transmit(&huart1,(uint8_t*) buff, strlen(buff), HAL_MAX_DELAY);
//
//	  		  set_led(1);
//	  		  HAL_Delay(100);
//	  		  set_led(0);
//	  		  break;
//	  	  }
//
//	  	  case WAKEUP_SOURCE_BUTTON:
//	  	  {
//	  		  char buff[50] = "Wake up from BUTTON\n\r";
//	  		  HAL_UART_Transmit(&huart1,(uint8_t*) buff, strlen(buff), HAL_MAX_DELAY);
//
//	  		  for(uint8_t i = 0; i < 10; i++)
//	  		  {
//	  			  set_led(1);
//	  			  HAL_Delay(50);
//	  			  set_led(0);
//	  			  HAL_Delay(50);
//	  		  }
//	  		  break;
//	  	  }
//
//	  	  case WAKEUP_SOURCE_NONE:
//	  	  {
//	  		  char buff[50] = "System start..\n\r";
//	  		  HAL_UART_Transmit(&huart1,(uint8_t*) buff, strlen(buff), HAL_MAX_DELAY);
//
//	  		  break;
//	  	  }
//	  }
//
//	  wakeup_source = WAKEUP_SOURCE_NONE;
//	  enter_stop_mode();
	  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	  // Standby mode. Power consumption 16 uA. Wake up from RTC or button //////////////////////////////////////////////////////////////////////////////

	  switch(reset_source_stendby)
	  {
	  	  case RESET_SOURCE_STENDBY_RTC:
	  	  {
	  		  char buff[50] = "Wakeup from RTC\n\r";
	  		  HAL_UART_Transmit(&huart1,(uint8_t*) buff, strlen(buff), HAL_MAX_DELAY);

	  		  set_led(1);
	  		  HAL_Delay(100);
	  		  set_led(0);
	  		  break;
	  	  }
	  	  case RESET_SOURCE_STANDBY_BUTTON:
	  	  {
			  char buff[50] = "Wakeup from BUTTON\n\r";
			  HAL_UART_Transmit(&huart1,(uint8_t*) buff, strlen(buff), HAL_MAX_DELAY);

			  for(uint8_t i = 0; i < 10; i++)
			  {
				  set_led(1);
				  HAL_Delay(50);
				  set_led(0);
				  HAL_Delay(50);
			  }
	  		  break;
	  	  }
	  	  case RESET_SOURCE_POWER_ON:
	  	  {
	  		 char buff[50] = "Wakeup from reset\n\r";
	  		 HAL_UART_Transmit(&huart1,(uint8_t*) buff, strlen(buff), HAL_MAX_DELAY);
	  		 break;
	  	  }

	  }
	  enter_standby_mode();

	  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the WakeUp
  */
  if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC14 PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PH0 PH1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 PA3 PA4
                           PA5 PA6 PA7 PA8
                           PA11 PA12 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4
                          |GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10
                           PB12 PB13 PB14 PB15
                           PB3 PB4 PB5 PB6
                           PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
	if(hrtc->Instance == RTC)
	{
		wakeup_source = WAKEUP_SOURCE_RTC;
	}

}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_0)
	{
		wakeup_source = WAKEUP_SOURCE_BUTTON;
	}
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
