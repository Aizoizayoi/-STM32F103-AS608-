/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#define FINGERPRINT_Tx_Pin GPIO_PIN_2
#define FINGERPRINT_Tx_GPIO_Port GPIOA
#define FINGERPRINT_Rx_Pin GPIO_PIN_3
#define FINGERPRINT_Rx_GPIO_Port GPIOA
#define FLASH_CS_Pin GPIO_PIN_4
#define FLASH_CS_GPIO_Port GPIOA
#define FLASH_SCL_Pin GPIO_PIN_5
#define FLASH_SCL_GPIO_Port GPIOA
#define FLASH_DO_Pin GPIO_PIN_6
#define FLASH_DO_GPIO_Port GPIOA
#define FLASH_DI_Pin GPIO_PIN_7
#define FLASH_DI_GPIO_Port GPIOA
#define PCF8574_INT_Pin GPIO_PIN_1
#define PCF8574_INT_GPIO_Port GPIOB
#define PCF8574_INT_EXTI_IRQn EXTI1_IRQn
#define PCF8574T_SCL_Pin GPIO_PIN_10
#define PCF8574T_SCL_GPIO_Port GPIOB
#define PCF8574T_SDA_Pin GPIO_PIN_11
#define PCF8574T_SDA_GPIO_Port GPIOB
#define TTL2USB_Tx_Pin GPIO_PIN_9
#define TTL2USB_Tx_GPIO_Port GPIOA
#define TTL2USB_Rx_Pin GPIO_PIN_10
#define TTL2USB_Rx_GPIO_Port GPIOA
#define OLED_SCL_Pin GPIO_PIN_6
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_7
#define OLED_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
