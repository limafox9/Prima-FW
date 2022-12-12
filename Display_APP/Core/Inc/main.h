/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PROGRESS_STROBE_Pin GPIO_PIN_13
#define PROGRESS_STROBE_GPIO_Port GPIOC
#define PROGRESS_DATA_Pin GPIO_PIN_14
#define PROGRESS_DATA_GPIO_Port GPIOC
#define PROGRESS_CLOCK_Pin GPIO_PIN_15
#define PROGRESS_CLOCK_GPIO_Port GPIOC
#define KEY_PROG_Pin GPIO_PIN_0
#define KEY_PROG_GPIO_Port GPIOA
#define KEY_TEST_Pin GPIO_PIN_1
#define KEY_TEST_GPIO_Port GPIOA
#define KEY_START_Pin GPIO_PIN_2
#define KEY_START_GPIO_Port GPIOA
#define LED_TEMP_134_Pin GPIO_PIN_4
#define LED_TEMP_134_GPIO_Port GPIOA
#define LED_TEST_HELIX_Pin GPIO_PIN_5
#define LED_TEST_HELIX_GPIO_Port GPIOA
#define LED_TEST_VACUUM_Pin GPIO_PIN_6
#define LED_TEST_VACUUM_GPIO_Port GPIOA
#define LED_TEMP_121_Pin GPIO_PIN_7
#define LED_TEMP_121_GPIO_Port GPIOA
#define DISPLAY_DIGIT_4_Pin GPIO_PIN_0
#define DISPLAY_DIGIT_4_GPIO_Port GPIOB
#define DISPLAY_DIGIT_3_Pin GPIO_PIN_1
#define DISPLAY_DIGIT_3_GPIO_Port GPIOB
#define DISPLAY_DIGIT_2_Pin GPIO_PIN_2
#define DISPLAY_DIGIT_2_GPIO_Port GPIOB
#define DISPLAY_DIGIT_1_Pin GPIO_PIN_10
#define DISPLAY_DIGIT_1_GPIO_Port GPIOB
#define DISPLAY_CLOCK_Pin GPIO_PIN_12
#define DISPLAY_CLOCK_GPIO_Port GPIOB
#define DISPLAY_DATA_Pin GPIO_PIN_13
#define DISPLAY_DATA_GPIO_Port GPIOB
#define DISPLAY_STROBE_Pin GPIO_PIN_14
#define DISPLAY_STROBE_GPIO_Port GPIOB
#define USART1_CTS_Pin GPIO_PIN_8
#define USART1_CTS_GPIO_Port GPIOA
#define DISPLAY_TIME_DOTS_Pin GPIO_PIN_11
#define DISPLAY_TIME_DOTS_GPIO_Port GPIOA
#define BEEP_Pin GPIO_PIN_12
#define BEEP_GPIO_Port GPIOA
#define LED_MODE_UNWRAPPED_Pin GPIO_PIN_3
#define LED_MODE_UNWRAPPED_GPIO_Port GPIOB
#define LED_WARNING_TDS_Pin GPIO_PIN_4
#define LED_WARNING_TDS_GPIO_Port GPIOB
#define LED_MODE_WRAPPED_Pin GPIO_PIN_5
#define LED_MODE_WRAPPED_GPIO_Port GPIOB
#define LED_MODE_PRION_Pin GPIO_PIN_6
#define LED_MODE_PRION_GPIO_Port GPIOB
#define LED_MODE_GENTLE_Pin GPIO_PIN_7
#define LED_MODE_GENTLE_GPIO_Port GPIOB
#define LED_WARNING_CTL_Pin GPIO_PIN_8
#define LED_WARNING_CTL_GPIO_Port GPIOB
#define LED_WARNING_LSH_Pin GPIO_PIN_9
#define LED_WARNING_LSH_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
