/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "iwdg.h"
#include "lwip.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "BSP_UDP.h"
#include "AT24CXX.h"
#include "BSP_74HC245.h"
#include "BSP_DS1302.h"
#include "clock.h"
#include "BSP_SHT2X.h"
#include "oled.h"
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t lastSecond;
uint8_t cnt_timing;
uint8_t cnt_seconds;

extern DS1302_Time_t TimeData;
typedef struct
{
    uint8_t year;   /* 年 */
    uint8_t month;  /* 月 */
    uint8_t day;    /* 日 */
    uint8_t week;   /* 星期 */
    uint8_t hour;   /* 时 */
    uint8_t minute; /* 分 */
    uint8_t second; /* 秒 */
    uint8_t temper;
    uint8_t humid;
} Screen_t;

Screen_t screen_data;
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */
    float temper = 0, humid = 0;
    uint8_t temper_t = 0, humid_t = 0;
    uint8_t sendbuf[200];
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_LWIP_Init();
    MX_IWDG_Init();
    MX_I2C1_Init();
    /* USER CODE BEGIN 2 */
    HAL_Delay(5000);      // 初始化完别直接进，延时，等lan8720初始化结束
    LWIP_Write_AT24CXX(); // 第一次上电使用
    LWIP_Read_AT24CXX();
    Net_Update(g_devrecource);
    udp_client_init();
    // DA1302_Init();          //第一次上电使用，向时钟写0

    OLED_DC_SET;
    OLED_Init();
    OLED_ShowNum(2, 0, 20, 2, 17);

    Clock_dev.timingcycle = 120;
    lastSecond = 0;
    cnt_timing = 2;
    cnt_seconds = 118;
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

    SendNTPreq();

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        DS1302_Read_RealTime();
        if (TimeData.second != lastSecond) /* 1 second */
        {
            SHT2X_Read(0, &temper);
            SHT2X_Read(1, &humid);
            temper_t = (uint8_t)temper;
            humid_t = (uint8_t)humid;
            sprintf((char *)sendbuf, "Current RTC??%d-%d-%d ????%d %d:%d:%d ???%d ???%d", TimeData.year, TimeData.month, TimeData.day, TimeData.week, TimeData.hour, TimeData.minute, TimeData.second, temper_t, humid_t);
            // printf("%s\n", sendbuf);

            Config_Send(sendbuf);
            OLED_ShowNum(18, 0, TimeData.year, 2, 17);
            OLED_ShowString(34, 0, "-");
            OLED_ShowNum(46, 0, TimeData.month, 2, 17);
            OLED_ShowString(64, 0, "-");
            OLED_ShowNum(72, 0, TimeData.day, 2, 17);
            OLED_ShowNum(100, 0, TimeData.week, 2, 17);

            OLED_ShowNum(2, 2, TimeData.hour, 2, 17);
            OLED_ShowString(18, 2, ":");
            OLED_ShowNum(26, 2, TimeData.minute, 2, 17);
            OLED_ShowString(42, 2, ":");
            OLED_ShowNum(50, 2, TimeData.second, 2, 17);

            OLED_ShowNum(74, 2, temper_t, 2, 17);

            OLED_ShowNum(98, 2, humid_t, 2, 17);

            lastSecond = TimeData.second;
            cnt_seconds++;
        }

        if (!HAL_GPIO_ReadPin(OLED_S1_GPIO_Port, OLED_S1_Pin)) //??????????
        {
            HAL_Delay(20);
            while (!HAL_GPIO_ReadPin(OLED_S1_GPIO_Port, OLED_S1_Pin))
                ;
            HAL_Delay(20);
            AT24CXX_Write(T_Year, (uint8_t *)&TimeData.year, 1);
            AT24CXX_Write(T_Month, (uint8_t *)&TimeData.month, 1);
            AT24CXX_Write(T_Day, (uint8_t *)&TimeData.day, 1);
            AT24CXX_Write(T_Week, (uint8_t *)&TimeData.week, 1);
            AT24CXX_Write(T_Hour, (uint8_t *)&TimeData.hour, 1);
            AT24CXX_Write(T_Minu, (uint8_t *)&TimeData.minute, 1);
            AT24CXX_Write(T_Secon, (uint8_t *)&TimeData.second, 1);
            AT24CXX_Write(T_Temper, (uint8_t *)&temper_t, 1);
            AT24CXX_Write(T_Humid, (uint8_t *)&humid_t, 1);
        }

        if (!HAL_GPIO_ReadPin(OLED_S2_GPIO_Port, OLED_S2_Pin)) //???????????
        {
            HAL_Delay(20);
            while (!HAL_GPIO_ReadPin(OLED_S2_GPIO_Port, OLED_S2_Pin))
                ;
            HAL_Delay(20);
            AT24CXX_Read(T_Year, (uint8_t *)&screen_data.year, 1);
            AT24CXX_Read(T_Month, (uint8_t *)&screen_data.month, 1);
            AT24CXX_Read(T_Day, (uint8_t *)&screen_data.day, 1);
            AT24CXX_Read(T_Week, (uint8_t *)&screen_data.week, 1);
            AT24CXX_Read(T_Hour, (uint8_t *)&screen_data.hour, 1);
            AT24CXX_Read(T_Minu, (uint8_t *)&screen_data.minute, 1);
            AT24CXX_Read(T_Secon, (uint8_t *)&screen_data.second, 1);
            AT24CXX_Read(T_Temper, (uint8_t *)&screen_data.temper, 1);
            AT24CXX_Read(T_Humid, (uint8_t *)&screen_data.humid, 1);

            sprintf((char *)sendbuf, "Last RTC=%d-%d-%d ????%d %d:%d:%d ???%d ???%d", screen_data.year, screen_data.month, screen_data.day, screen_data.week, screen_data.hour, screen_data.minute, screen_data.second, screen_data.temper, screen_data.humid);
            // printf("%s\n", sendbuf);
            Config_Send(sendbuf);

            OLED_ShowNum(2, 4, 20, 2, 17);
            OLED_ShowNum(18, 4, screen_data.year, 2, 17);
            OLED_ShowString(34, 4, "-");
            OLED_ShowNum(46, 4, screen_data.month, 2, 17);
            OLED_ShowString(64, 4, "-");
            OLED_ShowNum(72, 4, screen_data.day, 2, 17);
            OLED_ShowNum(100, 4, screen_data.week, 2, 17);

            OLED_ShowNum(2, 6, screen_data.hour, 2, 17);
            OLED_ShowString(18, 6, ":");
            OLED_ShowNum(26, 6, screen_data.minute, 2, 17);
            OLED_ShowString(42, 6, ":");
            OLED_ShowNum(50, 6, screen_data.second, 2, 17);

            OLED_ShowNum(74, 6, screen_data.temper, 2, 17);

            OLED_ShowNum(98, 6, screen_data.humid, 2, 17);
        }

        if (!HAL_GPIO_ReadPin(OLED_S3_GPIO_Port, OLED_S3_Pin)) //??????????
        {
            HAL_Delay(20);
            while (!HAL_GPIO_ReadPin(OLED_S3_GPIO_Port, OLED_S3_Pin))
                ;
            HAL_Delay(20);
            SendNTPreq();
            // printf("?????\n");
        }

        if (cnt_seconds >= Clock_dev.timingcycle) /* time to timing */
        {
            cnt_seconds = 0;
            cnt_timing++;
            SendNTPreq();
        }

        if (cnt_timing > 1)
        {
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        }

        Clock_dev.clock_status = Clock_GetStatus();
        if (Clock_dev.clock_status == Clock_Run)
        {
            Clock_RunOneSecond();
        }
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        MX_LWIP_Process();
        HAL_IWDG_Refresh(&hiwdg); /* IWDG Refresh */
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

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV5;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.Prediv1Source = RCC_PREDIV1_SOURCE_PLL2;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    RCC_OscInitStruct.PLL2.PLL2State = RCC_PLL2_ON;
    RCC_OscInitStruct.PLL2.PLL2MUL = RCC_PLL2_MUL8;
    RCC_OscInitStruct.PLL2.HSEPrediv2Value = RCC_HSE_PREDIV2_DIV5;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
    HAL_RCC_MCOConfig(RCC_MCO, RCC_MCO1SOURCE_HSE, RCC_MCODIV_1);

    /** Configure the Systick interrupt time
     */
    __HAL_RCC_PLLI2S_ENABLE();
}

/* USER CODE BEGIN 4 */

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
