/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stdio.h"
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
#define ETH_RST_Pin GPIO_PIN_0
#define ETH_RST_GPIO_Port GPIOC
#define LED_Pin GPIO_PIN_3
#define LED_GPIO_Port GPIOA
#define OLED_S2_Pin GPIO_PIN_0
#define OLED_S2_GPIO_Port GPIOB
#define OLED_S1_Pin GPIO_PIN_1
#define OLED_S1_GPIO_Port GPIOB
#define OLED_S3_Pin GPIO_PIN_2
#define OLED_S3_GPIO_Port GPIOB
#define OLED_S4_Pin GPIO_PIN_10
#define OLED_S4_GPIO_Port GPIOB
#define OLED_MOSI_Pin GPIO_PIN_6
#define OLED_MOSI_GPIO_Port GPIOC
#define OLED_CLK_Pin GPIO_PIN_7
#define OLED_CLK_GPIO_Port GPIOC
#define OLED_DC_Pin GPIO_PIN_8
#define OLED_DC_GPIO_Port GPIOC
#define OLED_CS_Pin GPIO_PIN_9
#define OLED_CS_GPIO_Port GPIOC
#define DS1302_CE_Pin GPIO_PIN_10
#define DS1302_CE_GPIO_Port GPIOC
#define DS1302_DATA_Pin GPIO_PIN_11
#define DS1302_DATA_GPIO_Port GPIOC
#define DS1302_CLK_Pin GPIO_PIN_12
#define DS1302_CLK_GPIO_Port GPIOC

/* USER CODE BEGIN Private defines */
/* 存储在AT24C02的位置 */
#define	REG_DEVID		      0x00	        /* 设备ID */
#define	REG_DEVMASK		    0x04	        /* 设备MASK */
#define	REG_DEVGATE		    0x08	        /* 设备GATE */
#define	REG_NTPID		      0x0C	        /* NTPID */
#define	REG_CONID		      0x10	        /* ConfigID */
#define	DEV_PORT		      0x14	        /* DevPort */
#define	NTP_PORT		      0x16	        /* NTPPort */
#define	CONF_PORT 	      0x18	        /* ConfigPort */
#define REG_POLARITY	    0	        /* 输出极性 */
#define	REG_TIMINGCYCLE   0	        /* 校时周期，单位秒 */
#define LOCAL_TIME        0

#define T_Year            0x1A
#define T_Month           0x1B
#define T_Day             0x1C
#define T_Week            0x1D
#define T_Hour            0x1E
#define T_Minu            0x1F
#define T_Secon           0x20
#define T_Temper          0x21
#define T_Humid            0x22
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
