#ifndef _CLOCK_H__
#define _CLOCK_H__

#include "main.h"

#define CLOCK_VERSION \
"NTP指针子钟v1.1\r\n\
\"NTP指针子钟\"，是一款基于NTP的三针石英时钟，产品基于NTP协议对时，可实时校正显示时间,\r\n\
确保时间准确性，产品具有无光、静音等特点。\r\n\
开发日期：2023年9月2日\r\n\
开发团队：明理楼D402\r\n\
"

#define	CLOCK_A	\
	// HAL_GPIO_WritePin(Clock_B1_GPIO_Port, Clock_B1_Pin|Clock_B2_Pin, GPIO_PIN_RESET); \
	// HAL_GPIO_WritePin(Clock_A1_GPIO_Port, Clock_A1_Pin|Clock_A2_Pin, GPIO_PIN_SET);

#define	CLOCK_B	\
	// HAL_GPIO_WritePin(Clock_A1_GPIO_Port, Clock_A1_Pin|Clock_A2_Pin, GPIO_PIN_RESET); \
	// HAL_GPIO_WritePin(Clock_B1_GPIO_Port, Clock_B1_Pin|Clock_B2_Pin, GPIO_PIN_SET);

#define CLOCK_STOP \
	// HAL_GPIO_WritePin(Clock_A1_GPIO_Port, Clock_A1_Pin|Clock_A2_Pin|Clock_B1_Pin|Clock_B2_Pin, GPIO_PIN_RESET);


typedef enum 
{
	Clock_Normal = 0,
	Clock_Run,
	Clock_Wait,
	Clock_Stop
}Clock_Status;

typedef struct
{
	uint8_t		year;
	uint8_t		month;
	uint8_t		day;
    uint8_t     week;
    uint8_t		hour;
	uint8_t		minute;
	uint8_t		second;
}TimeTypedef;

typedef struct
{
	uint8_t		devid;
	TimeTypedef	local_time;
	uint8_t		clock_status;
	uint8_t		polarity;
	uint8_t		timingcycle;
}Clock_devTypedef;

extern Clock_devTypedef Clock_dev;

void Clock_GetLocal(void);
void Clock_SetLocal(char *time);
void Clock_SetRun(void);
void Clock_SetStop(void);
void Clock_GetClockStatus(void);
void Clock_GetVersion(void);

void Clock_Init(void);
void Clock_RunOneSecond(void);
Clock_Status Clock_GetStatus(void);
uint8_t Clock_CheckTime(TimeTypedef *time);
void Clcok_Timer24to12(TimeTypedef *time);
uint64_t Clock_TimetoStamp(TimeTypedef *time);
void Clock_StamptoTime(uint64_t stamp, TimeTypedef *time);
#endif


