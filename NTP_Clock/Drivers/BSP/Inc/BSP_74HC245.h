#ifndef _BSP_74HC245_H__
#define _BSP_74HC245_H__

#include "main.h"

//SMG595数码管相关代码，驱动74HC245芯片进行显示

typedef struct
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t week;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}Time;

#define DAT_DIS_H   //HAL_GPIO_WritePin(DIS_DAT_GPIO_Port , DIS_DAT_Pin , GPIO_PIN_SET)
#define DAT_DIS_L   //HAL_GPIO_WritePin(DIS_DAT_GPIO_Port , DIS_DAT_Pin , GPIO_PIN_RESET)
#define CLK_DIS_H   //HAL_GPIO_WritePin(DIS_CLK_GPIO_Port , DIS_CLK_Pin , GPIO_PIN_SET)
#define CLK_DIS_L   //HAL_GPIO_WritePin(DIS_CLK_GPIO_Port , DIS_CLK_Pin , GPIO_PIN_RESET)
#define CS_DIS_H    //HAL_GPIO_WritePin(DIS_CS_GPIO_Port , DIS_CS_Pin , GPIO_PIN_SET)
#define CS_DIS_L    //HAL_GPIO_WritePin(DIS_CS_GPIO_Port , DIS_CS_Pin , GPIO_PIN_RESET)

void SMG595_Init(void);
void SMG595_Write_0ne_Byte(uint8_t data);
void SMG595_Write_Data(Time *t,uint8_t temple,uint8_t humidity);
void SMG595_SelectSeg_Negative(uint8_t data,uint8_t piont);     //共阴极
void SMG595_SelectSeg_Negative_OP(uint8_t data,uint8_t piont);
void SMG595_SelectSeg_Positive(uint8_t data,uint8_t piont);     //共阳极
char GetWeek(uint8_t iY, uint8_t iM, uint8_t iD);
#endif


