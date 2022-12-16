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
#define SW_CSH_Pin GPIO_PIN_2
#define SW_CSH_GPIO_Port GPIOE
#define SW_CSL_Pin GPIO_PIN_3
#define SW_CSL_GPIO_Port GPIOE
#define SW_LSH_Pin GPIO_PIN_4
#define SW_LSH_GPIO_Port GPIOE
#define SW_DOOR_Pin GPIO_PIN_5
#define SW_DOOR_GPIO_Port GPIOE
#define LED_HEART_Pin GPIO_PIN_6
#define LED_HEART_GPIO_Port GPIOA
#define USART3_CTS_Pin GPIO_PIN_15
#define USART3_CTS_GPIO_Port GPIOE
#define BEEP_Pin GPIO_PIN_13
#define BEEP_GPIO_Port GPIOD
#define KEY_S1_Pin GPIO_PIN_6
#define KEY_S1_GPIO_Port GPIOC
#define KEY_S2_Pin GPIO_PIN_7
#define KEY_S2_GPIO_Port GPIOC
#define KEY_S3_Pin GPIO_PIN_8
#define KEY_S3_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
