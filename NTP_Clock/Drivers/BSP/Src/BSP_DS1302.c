#include "BSP_DS1302.h"
#include "gpio.h"
#include "BSP_74HC245.h"
#include "BSP_UDP.h"
#include "clock.h"
#include "AT24CXX.h"

uint8_t read_time[7];
DS1302_Time_t TimeData;
char DS1302_data_1[10];
char DS1302_data_2[8];

void DS1302_Write_OneByte(uint8_t temp);
void Write_DS1302_Rig(uint8_t addr, uint8_t dat);
uint8_t DS1302_Read_Rig(uint8_t addr);

//改变DATA模式，0-输入，1-输出
void DS1302_DATA_Direction(uint8_t direct)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = DS1302_DATA_Pin;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  if(direct)
  {
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  }
  else
  {
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  }
  HAL_GPIO_Init(DS1302_DATA_GPIO_Port, &GPIO_InitStruct);
}

void DS1302_Write_OneByte(uint8_t temp)
{
    uint8_t i;
    DS1302_DATA_Direction(1);
    for(i = 0; i < 8; i++)
    {
        HAL_GPIO_WritePin(DS1302_CLK_GPIO_Port, DS1302_CLK_Pin, GPIO_PIN_RESET);
        if(temp & 0x01)
            HAL_GPIO_WritePin(DS1302_DATA_GPIO_Port, DS1302_DATA_Pin, GPIO_PIN_SET);
        else
            HAL_GPIO_WritePin(DS1302_DATA_GPIO_Port, DS1302_DATA_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(DS1302_CLK_GPIO_Port, DS1302_CLK_Pin, GPIO_PIN_SET);
        temp >>= 1;
    }
}

//向指定寄存器地址发送数据
void Write_DS1302_Rig(uint8_t addr, uint8_t dat)
{
    HAL_GPIO_WritePin(DS1302_CE_GPIO_Port, DS1302_CE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DS1302_CLK_GPIO_Port, DS1302_CLK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DS1302_CE_GPIO_Port, DS1302_CE_Pin, GPIO_PIN_SET);
    DS1302_Write_OneByte(addr);
    DS1302_Write_OneByte(dat);
    HAL_GPIO_WritePin(DS1302_CE_GPIO_Port, DS1302_CE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DS1302_CLK_GPIO_Port, DS1302_CLK_Pin, GPIO_PIN_RESET);
}

//从指定地址读取一字节数据
uint8_t DS1302_Read_Rig(uint8_t addr)
{
 	uint8_t i,temp=0x00;
    DS1302_DATA_Direction(0);
 	HAL_GPIO_WritePin(DS1302_CE_GPIO_Port, DS1302_CE_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DS1302_CLK_GPIO_Port, DS1302_CLK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(DS1302_CE_GPIO_Port, DS1302_CE_Pin, GPIO_PIN_SET);
 	DS1302_Write_OneByte(addr);
    DS1302_DATA_Direction(0);
 	for (i=0;i<8;i++) 	
 	{	
        temp>>=1;
        HAL_GPIO_WritePin(DS1302_CLK_GPIO_Port, DS1302_CLK_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(DS1302_CLK_GPIO_Port, DS1302_CLK_Pin, GPIO_PIN_RESET);
 		if(HAL_GPIO_ReadPin(DS1302_DATA_GPIO_Port, DS1302_DATA_Pin))
 		    temp|=0x80;	
 		HAL_GPIO_WritePin(DS1302_CLK_GPIO_Port, DS1302_CLK_Pin, GPIO_PIN_SET);
	} 
 	HAL_GPIO_WritePin(DS1302_CE_GPIO_Port, DS1302_CE_Pin, GPIO_PIN_RESET);
    DS1302_DATA_Direction(1);
	HAL_GPIO_WritePin(DS1302_DATA_GPIO_Port, DS1302_DATA_Pin, GPIO_PIN_RESET);
	return (temp);			
}


/*
 * 
注意，写的时候D0配置为0
写入时
秒：1000 0000即0x80
分：1000 0010即0x82
时：1000 0100即0x84
日：1000 0110即0x86
月：1000 1000即0x88
周：1000 1010即0x8a
年：1000 1100即0x8c
 */

void DS1302_Write_Time(DS1302_Time_t *t)
{
    Write_DS1302_Rig(0x8e,0x00);//关闭写保护
    Write_DS1302_Rig(0x80,(((t->second/10)<<4) + t->second%10));      //秒
    Write_DS1302_Rig(0x82,(((t->minute/10)<<4) + t->minute%10));      //分
    Write_DS1302_Rig(0x84,(((t->hour/10)<<4) + t->hour%10));          //时
    Write_DS1302_Rig(0x86,(((t->day/10)<<4) + t->day%10));            //日
    Write_DS1302_Rig(0x88,(((t->month/10)<<4) + t->month%10));        //月
    Write_DS1302_Rig(0x8a,(((t->week/10)<<4) + t->week%10));          //星期
    Write_DS1302_Rig(0x8c,(((t->year/10)<<4) + t->year%10));          //年
    Write_DS1302_Rig(0x8e,0x80);//打开写保护
}
 
/*
 * 读取DS1302数据
注意，读的时候D0配置为1
读取时
秒：1000 0001即0x81
分：1000 0011即0x83
时：1000 0101即0x85
日：1000 0111即0x87
月：1000 1001即0x89
周：1000 1011即0x8b
年：1000 1101即0x8d
 */
void DS1302_Read_Time(void)
{
	read_time[0]=DS1302_Read_Rig(0x81);//读秒
	read_time[1]=DS1302_Read_Rig(0x83);//读分
	read_time[2]=DS1302_Read_Rig(0x85);//读时
	read_time[3]=DS1302_Read_Rig(0x87);//读日
	read_time[4]=DS1302_Read_Rig(0x89);//读月
	read_time[5]=DS1302_Read_Rig(0x8B);//读星期
	read_time[6]=DS1302_Read_Rig(0x8D);//读年
}

/**
 * @brief	  将读取到的数据换算成实际时间
*/
void DS1302_Read_RealTime(void)
{
	DS1302_Read_Time();  //BCD码转换为10进制

	TimeData.second=(read_time[0]>>4)*10+(read_time[0]&0x0f);
	TimeData.minute=((read_time[1]>>4)&(0x07))*10+(read_time[1]&0x0f);
	TimeData.hour=(read_time[2]>>4)*10+(read_time[2]&0x0f);
	TimeData.day=(read_time[3]>>4)*10+(read_time[3]&0x0f);
	TimeData.month=(read_time[4]>>4)*10+(read_time[4]&0x0f);
	TimeData.week=(read_time[5]>>4)*10+(read_time[5]&0x0f);
	TimeData.year=(read_time[6]>>4)*10+(read_time[6]&0x0f);

    if(TimeData.second==80)
	{
		Write_DS1302_Rig(0x80,0x00);	//时钟开始运行
	}

	//DS1302_data_1[0]='2';
	//DS1302_data_1[1]='0';
	//DS1302_data_1[2]='0'+(TimeData_lwip.year-2000)/10;
	//DS1302_data_1[3]='0'+(TimeData_lwip.year-2000)%10;
	//DS1302_data_1[4]='-';
	//DS1302_data_1[5]='0'+TimeData_lwip.month/10;
	//DS1302_data_1[6]='0'+TimeData_lwip.month%10;
	//DS1302_data_1[7]='-';
	//DS1302_data_1[8]='0'+TimeData_lwip.day/10;
	//DS1302_data_1[9]='0'+TimeData_lwip.day%10;
	//
	//DS1302_data_2[0]='0'+TimeData_lwip.hour/10;
	//DS1302_data_2[1]='0'+TimeData_lwip.hour%10;
	//DS1302_data_2[2]=':';
	//DS1302_data_2[3]='0'+TimeData_lwip.minute/10;
	//DS1302_data_2[4]='0'+TimeData_lwip.minute%10;
	//DS1302_data_2[5]=':';
	//DS1302_data_2[6]='0'+TimeData_lwip.second/10;
	//DS1302_data_2[7]='0'+TimeData_lwip.second%10;
}

void DA1302_Init(void)
{
    Write_DS1302_Rig(0x8E, 0x00);
    Write_DS1302_Rig(0x82, 0);
    Write_DS1302_Rig(0x84, 0);
    Write_DS1302_Rig(0x80,0x00);	//时钟开始运行
    Write_DS1302_Rig(0x8E, 0x80);
}

/**
 * @brief	  设置RTC时间
*/
void SET_RTC(uint8_t *recvnum)
{
	int hour, minute, second;
	uint8_t sendbuf[100];
	
    sscanf((const char *)recvnum, "%d:%d:%d", &hour, &minute, &second);
    
    TimeData.hour = (uint8_t)hour;
    TimeData.minute = (uint8_t)minute;
    TimeData.second = (uint8_t)second;
    if (Clock_CheckTime((TimeTypedef *)&TimeData))
	{
		printf("Valid time\r\n");
        DS1302_Write_Time(&TimeData);
        
	    sprintf((char *)sendbuf, "Set RTC time:%d:%d:%d \r\n", TimeData.hour, TimeData.minute, TimeData.second);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);
	}
    else
    {
        printf("Invalid time\r\n");
    }
}

/**
 * @brief	  获取RTC时间
*/
void GET_RTC(void)
{
	uint8_t sendbuf[100];
	
    sprintf((char *)sendbuf, "RTC time:%d-%d-%d 星期%d %d:%d:%d\r\n", TimeData.year + 2000, TimeData.month, TimeData.day, TimeData.week, TimeData.hour, TimeData.minute, TimeData.second);
    printf("%s\n", sendbuf);
    Config_Send(sendbuf);
}
