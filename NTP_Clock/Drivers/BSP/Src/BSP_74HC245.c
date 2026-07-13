#include "BSP_74HC245.h"

void delay_us(uint32_t us, uint16_t CLK_MHz )
{
    uint32_t Delay = us * CLK_MHz / 12;
    do
    {
        __NOP();
    } while (Delay --);
}

char GetWeek(uint8_t iY, uint8_t iM, uint8_t iD)
{
	int iWeekDay = -1;
	if (1 == iM || 2 == iM) 
    {   
        iM += 12; 
        iY--;
    }   
    iWeekDay = (iD + 1 + 2 * iM + 3 * (iM + 1) / 5 + iY + iY / 4 - iY / 100 + iY / 400) % 7;	
	return iWeekDay;//0-6
}

void SMG595_Init(void)
{
    DAT_DIS_H;
    CLK_DIS_H;
    CS_DIS_H;
}

void SMG595_Write_0ne_Byte(uint8_t data)
{
    uint8_t i;
	
    for(i=0;i<8;i++)//循环8次，写入1字节数据
    {
        if((data<<i)&0x80)//检测数据位
        {
            DAT_DIS_H;//置高DATA位
        }
        else
        {
            DAT_DIS_L;//置低DATA位
        } 	
        delay_us(1, 72);//延时
        CLK_DIS_L;  //置低CLK位
        delay_us(1, 72);//延时等待时钟稳定
        CLK_DIS_H;  //置高CLK位
        delay_us(1, 72);
    }
    CLK_DIS_H;      //置高CLK位
}

//数码管显示数据
//dat顺序应为：年、月、日、星期、温度、湿度、时、分、秒
void SMG595_Write_Data(Time *t,uint8_t temple,uint8_t humidity)
{
    // if(t->year>=100)    //取十位和个位数
    // {
    //     t->year%=100;
    // }

    SMG595_SelectSeg_Negative(t->second%10,0);
	SMG595_SelectSeg_Negative(t->second/10,0);//秒
	SMG595_SelectSeg_Negative(t->minute%10,0);
	SMG595_SelectSeg_Negative(t->minute/10,0);//分
	SMG595_SelectSeg_Negative(t->hour%10,0);
	SMG595_SelectSeg_Negative(t->hour/10,0);//时

    SMG595_SelectSeg_Positive(humidity%10, 0);    
    SMG595_SelectSeg_Positive(humidity/10, 0);   //湿度
    SMG595_SelectSeg_Positive(temple%10, 0);    
    SMG595_SelectSeg_Positive(temple/10, 0);   //温度

    SMG595_SelectSeg_Positive(t->week%10, 0);    
    SMG595_SelectSeg_Positive(t->week/10, 0);    //星期

    SMG595_SelectSeg_Positive(t->day%10, 0);    
    SMG595_SelectSeg_Positive(t->day/10, 0);    //日期
    SMG595_SelectSeg_Positive(t->month%10, 0);    
    SMG595_SelectSeg_Positive(t->month/10, 0);    //月
    SMG595_SelectSeg_Positive(t->year%10, 0);
    SMG595_SelectSeg_Positive(t->year/10, 0);    
    SMG595_SelectSeg_Positive(0, 0);
    SMG595_SelectSeg_Positive(2, 0);            //年

    // printf("%d",(t->year) + 2000);
    // printf("-");
    // printf("%d",t->month);
    // printf("-");
    // printf("%d\n",t->day);
    // printf("-");
    // printf("%d\n",t->week);

    // printf("%d",t->hour);
    // printf(":");
    // printf("%d",t->minute);
    // printf(":");
    // printf("%d\n",t->second);
    // /*****************************************************/
	// //以下是共阳极
	// SMG595_SelectSeg_Positive(temple%10,0);    
	// SMG595_SelectSeg_Positive(temple/10,0);//温度

	// SMG595_SelectSeg_Positive(humidity%10,0);
	// SMG595_SelectSeg_Positive(humidity/10,0); 
	
	// //发送年月日数据
	// SMG595_SelectSeg_Positive(GetWeek(t->year,t->month,t->day),0);//星期
	// SMG595_SelectSeg_Positive(16,0);            //“-”
	// SMG595_SelectSeg_Positive(t->day%10,0);    //
	// SMG595_SelectSeg_Positive(t->day/10,0);     //日
	// SMG595_SelectSeg_Positive(16,0);            //“-”
	// SMG595_SelectSeg_Positive(t->month%10,0);  //
	// SMG595_SelectSeg_Positive(t->month/10,0);   //月
	// SMG595_SelectSeg_Positive(16,0);            //“-”
	// SMG595_SelectSeg_Positive(t->year%10,0);   //
	// SMG595_SelectSeg_Positive(t->year/10,0);    //
	// SMG595_SelectSeg_Positive(0,0);             //
	// SMG595_SelectSeg_Positive(2,0);             //年
    CS_DIS_H; 
	delay_us(1, 72);
	CS_DIS_L; //数据锁存
	delay_us(1, 72);
	CS_DIS_H; 
}

/**
 * @brief		SMG595共阴极片选
*/
void SMG595_SelectSeg_Negative(uint8_t data,uint8_t piont)
{
    uint8_t Buff = 0x00;//默认所有段都不亮
    switch (data)
    {
    case 0:  Buff = 0x3f;break; //显示0
    case 1:  Buff = 0x06;break; //显示1
    case 2:  Buff = 0x5b;break; //显示2
    case 3:  Buff = 0x4f;break; //显示3
    case 4:  Buff = 0x66;break; //显示4
    case 5:  Buff = 0x6d;break; //显示5
    case 6:  Buff = 0x7d;break; //显示6
    case 7:  Buff = 0x07;break; //显示7
    case 8:  Buff = 0x7f;break; //显示8
    case 9:  Buff = 0x6f;break; //显示9
    case 10: Buff = 0x77;break; //显示A	
    case 11: Buff = 0x7f;break; //显示B
    case 12: Buff = 0x39;break; //显示C
    case 13: Buff = 0x3f;break; //显示D
    case 14: Buff = 0x79;break; //显示E
    case 15: Buff = 0x71;break; //显示F	
	case 16: Buff = 0x40;break; //显示-
    }
	if(piont == 1)//判断显示小数点
	{
	   Buff |= 0X80;  //显示小数点
	}
	SMG595_Write_0ne_Byte(Buff);//写入数据
}

/**
 * @brief		SMG595共阴极片选，反相
*/
void 
SMG595_SelectSeg_Negative_OP(uint8_t data,uint8_t piont)
{
    uint8_t Buff = 0x00;//默认所有段都不亮
    switch (data)
    {
    case 0:  Buff = 0x3f;break; //显示0
    case 1:  Buff = 0x30;break; //显示1
    case 2:  Buff = 0x5b;break; //显示2
    case 3:  Buff = 0x79;break; //显示3
    case 4:  Buff = 0x74;break; //显示4
    case 5:  Buff = 0x6d;break; //显示5
    case 6:  Buff = 0x6f;break; //显示6
    case 7:  Buff = 0x38;break; //显示7
    case 8:  Buff = 0x7f;break; //显示8
    case 9:  Buff = 0x7d;break; //显示9
    case 10: Buff = 0x7E;break; //显示A
    case 11: Buff = 0x7f;break; //显示B
    case 12: Buff = 0x0f;break; //显示C
    case 13: Buff = 0x3f;break; //显示D
    case 14: Buff = 0x4f;break; //显示E
    case 15: Buff = 0x4e;break; //显示F
	case 16: Buff = 0x40;break; //显示-
    }
	if(piont == 1)//判断显示小数点
	{
	   Buff |= 0X80;  //显示小数点
	}
	SMG595_Write_0ne_Byte(Buff);//写入数据
}

/**
 * @brief		SMG595共阳极片选
*/
void SMG595_SelectSeg_Positive(uint8_t data,uint8_t piont)//年月日  写0对应亮  共阳极
{
    uint8_t Buff = 0xFF;//默认所有段都不亮
    switch (data)
    {
    case 0:  Buff = 0xc0;break; //显示0
    case 1:  Buff = 0xf9;break; //显示1
    case 2:  Buff = 0xa4;break; //显示2
    case 3:  Buff = 0xb0;break; //显示3
    case 4:  Buff = 0x99;break; //显示4
    case 5:  Buff = 0x92;break; //显示5
    case 6:  Buff = 0x82;break; //显示6
    case 7:  Buff = 0xf8;break; //显示7
    case 8:  Buff = 0x80;break; //显示8
    case 9:  Buff = 0x90;break; //显示9
    case 10: Buff = 0x88;break; //显示A	
    case 11: Buff = 0x80;break; //显示B
    case 12: Buff = 0xc6;break; //显示C
    case 13: Buff = 0xc0;break; //显示D
    case 14: Buff = 0x86;break; //显示E
    case 15: Buff = 0x8e;break; //显示F	
	case 16: Buff = 0xBF;break; //显示-
    }
	if(piont == 1)//判断显示小数点
	{
	   piont &= 0X7F;  //显示小数点
	}
    SMG595_Write_0ne_Byte(Buff);//写入数据
}
