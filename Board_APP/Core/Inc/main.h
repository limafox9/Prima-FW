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
#define SW_SOLENOID_Pin GPIO_PIN_1
#define SW_SOLENOID_GPIO_Port GPIOC
#define TEMP_ZFQ_Pin GPIO_PIN_0
#define TEMP_ZFQ_GPIO_Port GPIOA
#define TEMP_GB_Pin GPIO_PIN_1
#define TEMP_GB_GPIO_Port GPIOA
#define TEMP_T2_Pin GPIO_PIN_5
#define TEMP_T2_GPIO_Port GPIOA
#define LED_HEART_Pin GPIO_PIN_6
#define LED_HEART_GPIO_Port GPIOA
#define SOLENOID_V1_Pin GPIO_PIN_7
#define SOLENOID_V1_GPIO_Port GPIOE
#define SOLENOID_V2_Pin GPIO_PIN_8
#define SOLENOID_V2_GPIO_Port GPIOE
#define VAL_VACUUM_Pin GPIO_PIN_9
#define VAL_VACUUM_GPIO_Port GPIOE
#define VAL_WATER_STEAM_Pin GPIO_PIN_10
#define VAL_WATER_STEAM_GPIO_Port GPIOE
#define VAL_DRY_AIR_Pin GPIO_PIN_11
#define VAL_DRY_AIR_GPIO_Port GPIOE
#define VAL_DRAIN_STEAM_Pin GPIO_PIN_13
#define VAL_DRAIN_STEAM_GPIO_Port GPIOE
#define USART3_CTS_Pin GPIO_PIN_15
#define USART3_CTS_GPIO_Port GPIOE
#define SPI2_CS_Pin GPIO_PIN_12
#define SPI2_CS_GPIO_Port GPIOB
#define HEATER_RING_Pin GPIO_PIN_8
#define HEATER_RING_GPIO_Port GPIOD
#define HEATER_STEAM_GEN_Pin GPIO_PIN_9
#define HEATER_STEAM_GEN_GPIO_Port GPIOD
#define PUMP_WATER_LOAD_Pin GPIO_PIN_10
#define PUMP_WATER_LOAD_GPIO_Port GPIOD
#define PUMP_VACUUM_Pin GPIO_PIN_11
#define PUMP_VACUUM_GPIO_Port GPIOD
#define PUMP_STEAM_GEN_Pin GPIO_PIN_12
#define PUMP_STEAM_GEN_GPIO_Port GPIOD
#define BEEP_Pin GPIO_PIN_13
#define BEEP_GPIO_Port GPIOD
#define KEY_S1_Pin GPIO_PIN_6
#define KEY_S1_GPIO_Port GPIOC
#define KEY_S2_Pin GPIO_PIN_7
#define KEY_S2_GPIO_Port GPIOC
#define KEY_S3_Pin GPIO_PIN_8
#define KEY_S3_GPIO_Port GPIOC
#define FAN_Pin GPIO_PIN_8
#define FAN_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_7
#define SPI1_CS_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
