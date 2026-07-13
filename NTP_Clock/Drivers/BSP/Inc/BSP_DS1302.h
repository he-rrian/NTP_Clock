#ifndef _BSP_DS1302_H__
#define _BSP_DS1302_H__

#include "main.h"

//存放时间
typedef struct
{
    uint8_t  year;        /* 年 */
    uint8_t  month;       /* 月 */
    uint8_t  day;         /* 天 */
    uint8_t  week;        /* 星期 */
    uint8_t  hour;        /* 时 */
    uint8_t  minute;      /* 分 */
    uint8_t  second;      /* 秒 */
}DS1302_Time_t;

extern char DS1302_data_2[8];
extern DS1302_Time_t TimeData;

void DA1302_Init(void);
void DS1302_Read_RealTime(void);
void DS1302_Write_Time(DS1302_Time_t *t);
void SET_RTC(uint8_t *recvnum);
void GET_RTC(void);

#endif
