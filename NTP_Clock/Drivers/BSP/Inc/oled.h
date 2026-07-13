/*
 * oled.h
 *
 *  Created on: 2023年11月10日
 *      Author: liyup
 */

#ifndef _OLED_H_
#define _OLED_H_


#include "stdint.h"


#define OLED_MODE 0
#define SIZE 16
#define XLevelL     0x00
#define XLevelH     0x10
#define Max_Column  128
#define Max_Row     64
#define Brightness  0xFF
#define X_WIDTH     128
#define Y_WIDTH     64

#define OLED_CMD  0
#define OLED_DATA 1

#define OLED_CS_SET     HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_SET)
#define OLED_CS_RESET   HAL_GPIO_WritePin(OLED_CS_GPIO_Port, OLED_CS_Pin, GPIO_PIN_RESET)
#define OLED_DC_SET     HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET)
#define OLED_DC_RESET   HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET)

#define OLED_CLK_SET        HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, GPIO_PIN_SET)
#define OLED_CLK_RESET      HAL_GPIO_WritePin(OLED_CLK_GPIO_Port, OLED_CLK_Pin, GPIO_PIN_RESET)
#define OLED_MOSI_SET       HAL_GPIO_WritePin(OLED_MOSI_GPIO_Port, OLED_MOSI_Pin, GPIO_PIN_SET)
#define OLED_MOSI_RESET     HAL_GPIO_WritePin(OLED_MOSI_GPIO_Port, OLED_MOSI_Pin, GPIO_PIN_RESET)

void Screen_Init(void);
void OLED_WR_Byte(uint8_t dat,uint8_t cmd);
void OLED_Display_On(void);
void OLED_Display_Off(void);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr);
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);
void OLED_ShowString(uint8_t x,uint8_t y, uint8_t *p);
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);
void OLED_Refresh_Gram(void);

#endif /* OLED_H_ */
