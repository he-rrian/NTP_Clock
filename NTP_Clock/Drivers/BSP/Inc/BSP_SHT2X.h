#ifndef _BSP_SHT2X_H__
#define _BSP_SHT2X_H__

#include "main.h"

#define SHT2X_ADDR  0x80

#define HOLD_TEMPER     0xE3
#define HOLD_HUMID      0xE5
#define NO_HOLD_TEMPER  0xF3
#define NO_HOLD_HUMID   0xF5
#define SHT2X_RESET     0xFE

#define SHT2X_SCL_SET     HAL_GPIO_WritePin(SHT2X_SCL_GPIO_Port, SHT2X_SCL_Pin, GPIO_PIN_SET)
#define SHT2X_SCL_RESET   HAL_GPIO_WritePin(SHT2X_SCL_GPIO_Port, SHT2X_SCL_Pin, GPIO_PIN_RESET)
#define SHT2X_SDA_SET     HAL_GPIO_WritePin(SHT2X_SDA_GPIO_Port, SHT2X_SDA_Pin, GPIO_PIN_SET)
#define SHT2X_SDA_RESET   HAL_GPIO_WritePin(SHT2X_SDA_GPIO_Port, SHT2X_SDA_Pin, GPIO_PIN_RESET)

void SHT2X_Read(uint8_t type, float *pBuffer);

#endif
