/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "common.h"
#include "LuxNET.h"
#include "TinyFrame.h"
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
void RS485_Tick(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ADDRESS0_Pin GPIO_PIN_13
#define ADDRESS0_GPIO_Port GPIOC
#define ADDRESS1_Pin GPIO_PIN_14
#define ADDRESS1_GPIO_Port GPIOC
#define ADDRESS2_Pin GPIO_PIN_15
#define ADDRESS2_GPIO_Port GPIOC
#define ADDRESS3_Pin GPIO_PIN_0
#define ADDRESS3_GPIO_Port GPIOF
#define ADDRESS4_Pin GPIO_PIN_1
#define ADDRESS4_GPIO_Port GPIOF
#define CURRENT_Pin GPIO_PIN_0
#define CURRENT_GPIO_Port GPIOA
#define LED_R1_Pin GPIO_PIN_4
#define LED_R1_GPIO_Port GPIOA
#define BTN_DN_Pin GPIO_PIN_5
#define BTN_DN_GPIO_Port GPIOA
#define BTN_UP_Pin GPIO_PIN_6
#define BTN_UP_GPIO_Port GPIOA
#define BTN_MOVE_Pin GPIO_PIN_7
#define BTN_MOVE_GPIO_Port GPIOA
#define JAL7_DN_Pin GPIO_PIN_0
#define JAL7_DN_GPIO_Port GPIOB
#define JAL7_UP_Pin GPIO_PIN_1
#define JAL7_UP_GPIO_Port GPIOB
#define JAL6_DN_Pin GPIO_PIN_2
#define JAL6_DN_GPIO_Port GPIOB
#define JAL6_UP_Pin GPIO_PIN_10
#define JAL6_UP_GPIO_Port GPIOB
#define JAL5_DN_Pin GPIO_PIN_11
#define JAL5_DN_GPIO_Port GPIOB
#define JAL5_UP_Pin GPIO_PIN_12
#define JAL5_UP_GPIO_Port GPIOB
#define JAL4_DN_Pin GPIO_PIN_13
#define JAL4_DN_GPIO_Port GPIOB
#define JAL4_UP_Pin GPIO_PIN_14
#define JAL4_UP_GPIO_Port GPIOB
#define RELAY_Pin GPIO_PIN_15
#define RELAY_GPIO_Port GPIOB
#define JAL1_DN_Pin GPIO_PIN_8
#define JAL1_DN_GPIO_Port GPIOA
#define JAL1_UP_Pin GPIO_PIN_11
#define JAL1_UP_GPIO_Port GPIOA
#define JAL0_DN_Pin GPIO_PIN_12
#define JAL0_DN_GPIO_Port GPIOA
#define JAL0_UP_Pin GPIO_PIN_6
#define JAL0_UP_GPIO_Port GPIOF
#define JAL3_DN_Pin GPIO_PIN_7
#define JAL3_DN_GPIO_Port GPIOF
#define JAL3_UP_Pin GPIO_PIN_15
#define JAL3_UP_GPIO_Port GPIOA
#define JAL2_DN_Pin GPIO_PIN_3
#define JAL2_DN_GPIO_Port GPIOB
#define JAL2_UP_Pin GPIO_PIN_4
#define JAL2_UP_GPIO_Port GPIOB
#define LED_C3_Pin GPIO_PIN_5
#define LED_C3_GPIO_Port GPIOB
#define LED_C2_Pin GPIO_PIN_6
#define LED_C2_GPIO_Port GPIOB
#define LED_C1_Pin GPIO_PIN_7
#define LED_C1_GPIO_Port GPIOB
#define LED_R3_Pin GPIO_PIN_8
#define LED_R3_GPIO_Port GPIOB
#define LED_R2_Pin GPIO_PIN_9
#define LED_R2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
