#include "Clock.h"
#include <stdio.h>
#include "AT24CXX.h"
#include "BSP_UDP.h"
#include "BSP_DS1302.h"

Clock_devTypedef Clock_dev;
uint8_t clk_sendbuf[100];
extern DS1302_Time_t TimeData;

/**
 * @brief	获取表盘时间
*/
void Clock_GetLocal(void)
{
	sprintf((char *)clk_sendbuf, "Local time:%d:%d:%d\r\n", Clock_dev.local_time.hour, Clock_dev.local_time.minute, Clock_dev.local_time.second);
    printf("%s\n", clk_sendbuf);
    Config_Send(clk_sendbuf);
}

/**
 * @brief	设置表盘时间
*/
void Clock_SetLocal(char *time)
{
	int hour,minute,second;
	sscanf(time, "%d:%d:%d", &hour, &minute, &second);
	Clock_dev.local_time.hour = (uint8_t)hour;
	Clock_dev.local_time.minute = (uint8_t)minute;
	Clock_dev.local_time.second = (uint8_t)second;
	if (Clock_CheckTime(&Clock_dev.local_time))
	{
	    Clcok_Timer24to12(&Clock_dev.local_time);
	    AT24CXX_Write(LOCAL_TIME, (uint8_t*)&Clock_dev.local_time, 7);
	    AT24CXX_Write(REG_POLARITY, (uint8_t*)&Clock_dev.polarity, 1);
	}
    else
    {
		printf("Invalid time\r\n");
		return;
    }
    sprintf((char *)clk_sendbuf, "Set Local time:%d:%d:%d\r\n", Clock_dev.local_time.hour, Clock_dev.local_time.minute, Clock_dev.local_time.second);
    printf("%s\n", clk_sendbuf);
    Config_Send(clk_sendbuf);
}

/**
 * @brief	开启表盘走时
*/
void Clock_SetRun(void)
{
	if (Clock_dev.clock_status == Clock_Stop)
	{
		Clock_dev.clock_status = Clock_Normal;
	}
}

/**
 * @brief	停止表盘走时
*/
void Clock_SetStop(void)
{
	Clock_dev.clock_status = Clock_Stop;
}

/**
 * @brief	AT指令获取表盘状态
*/
void Clock_GetClockStatus(void)
{
	Clock_Status status;

	status = Clock_GetStatus();
	switch (status)
	{
	case Clock_Normal:
        sprintf((char *)clk_sendbuf, "Normal\r\n");
		break;
	case Clock_Run:
        sprintf((char *)clk_sendbuf, "Run\r\n");
		break;
	case Clock_Wait:
        sprintf((char *)clk_sendbuf, "Wait\r\n");
		break;
	case Clock_Stop:
        sprintf((char *)clk_sendbuf, "Stop\r\n");
		break;
	default:
        sprintf((char *)clk_sendbuf, "Error\r\n");
		break;
	}
    printf("%s\n", clk_sendbuf);
    Config_Send(clk_sendbuf);
}

/**
 * @brief	获取版本信息
*/
void Clock_GetVersion(void)
{
    sprintf((char *)clk_sendbuf, CLOCK_VERSION);
    printf("%s\n", clk_sendbuf);
    Config_Send(clk_sendbuf);
}

/**
 * @brief	检查表盘当前应当是什么状态
*/
Clock_Status Clock_GetStatus(void)
{
    uint64_t rtcStamp;
	uint64_t localStamp;

	if (Clock_dev.clock_status == Clock_Stop)	/* 强制停止走时 */
	{
		return Clock_Stop;
	}

    Clcok_Timer24to12((TimeTypedef *)&TimeData);
	/* 正常走时 */
	if (TimeData.hour ==Clock_dev.local_time.hour && \
		TimeData.minute ==Clock_dev.local_time.minute && \
		TimeData.second ==Clock_dev.local_time.second	
	)
	{
		return Clock_Normal;
	}
	
	if (TimeData.hour==12 && Clock_dev.local_time.hour==1)
	{
        TimeData.hour -= 12;
	}
	rtcStamp = Clock_TimetoStamp((TimeTypedef *)&TimeData);
	localStamp = Clock_TimetoStamp((TimeTypedef *)&Clock_dev.local_time);
	if ((localStamp-rtcStamp) < 3600)
	{
		return Clock_Wait;
	}
	
	return Clock_Run;
}

/**
 * @brief			24小时转12小时
 * @param	time	时间：Hour-Minutes-Seconds
*/
void Clcok_Timer24to12(TimeTypedef *time)
{
	if (time->hour>12)
	{
		time->hour-=12;
	}
	if (time->hour==0)
	{
		time->hour = 12;
	}
}

/**
 * @brief			检查时间有效性
 * @param	time	时间：Hour-Minutes-Seconds
*/
uint8_t Clock_CheckTime(TimeTypedef *time)
{
	if (time->hour>23 || time->minute>59 || time->second>59)
	{
		return 0;
	}
	return 1;
}

/**
 * @brief			time转时间戳
 * @param	time	时间：Hour-Minutes-Seconds
 * @retval			时间戳
*/
uint64_t Clock_TimetoStamp(TimeTypedef *time)
{
	return time->hour*3600+time->minute*60+time->second;
}

/**
 * @brief			时间戳转time
 * @param	stamp	时间戳
 * @param	time	时间地址
*/
void Clock_StamptoTime(uint64_t stamp, TimeTypedef *time)
{
	time->hour = stamp/3600;
	time->minute = stamp/60;
	time->second = stamp%60;
}


/**
 * @brief			表盘走时
*/
void Clock_RunOneSecond(void)
{
	if(Clock_dev.polarity)
	{
		CLOCK_A;
	}
	else
	{
		CLOCK_B;
	}
	HAL_Delay(30);
	CLOCK_STOP;

	Clock_dev.polarity=!Clock_dev.polarity;

	/* 时间自增 */
	if (Clock_dev.local_time.second == 59)
	{
		Clock_dev.local_time.second = 0;
		if (Clock_dev.local_time.minute == 59)
		{
			Clock_dev.local_time.minute = 0;
			if (Clock_dev.local_time.hour == 12)
			{
				Clock_dev.local_time.hour = 1;
			}
			else
			{
				Clock_dev.local_time.hour++;
			}
		}
		else
		{
			Clock_dev.local_time.minute++;
		}
	}
	else
	{
		Clock_dev.local_time.second++;
	}


	AT24CXX_Write(LOCAL_TIME, (uint8_t*)&Clock_dev.local_time, 7);
	AT24CXX_Write(REG_POLARITY, (uint8_t*)&Clock_dev.polarity, 1);
}


/**
 * @brief	从EEPROM中读取数据
*/
void Clock_Init(void)
{
	AT24CXX_Read(LOCAL_TIME, (uint8_t*)&Clock_dev.local_time, 7);				/* 时间 */
	AT24CXX_Read(REG_POLARITY, (uint8_t*)&Clock_dev.polarity, 1);			/* 输出极性 */
	AT24CXX_Read(REG_TIMINGCYCLE, (uint8_t*)&Clock_dev.timingcycle, 1);		/* 校准周期 */
}
