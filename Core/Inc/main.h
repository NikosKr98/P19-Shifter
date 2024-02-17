/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define DOWN_PORT_Pin GPIO_PIN_6
#define DOWN_PORT_GPIO_Port GPIOA
#define UP_PORT_Pin GPIO_PIN_7
#define UP_PORT_GPIO_Port GPIOA
#define DIN01_Pin GPIO_PIN_0
#define DIN01_GPIO_Port GPIOB
#define DIN02_Pin GPIO_PIN_1
#define DIN02_GPIO_Port GPIOB
#define DO01_Pin GPIO_PIN_12
#define DO01_GPIO_Port GPIOB
#define DO02_Pin GPIO_PIN_13
#define DO02_GPIO_Port GPIOB
#define DO03_Pin GPIO_PIN_14
#define DO03_GPIO_Port GPIOB
#define DO04_Pin GPIO_PIN_15
#define DO04_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
