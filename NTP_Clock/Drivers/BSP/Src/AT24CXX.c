#include "AT24CXX.h"
#include "i2c.h"

/**
 * @brief	检查EEPROM是否正常
 * @retval	1-检测失败	0-成功
*/
uint8_t   AT24CXX_Check(void)
{
	uint8_t temp;
	uint8_t data = 0x55;
	AT24CXX_Read(255, &temp, 1);
	if(temp== 0x55)
	{
		printf("success\n");
		return 0;	
	}  
	else//排除第一次初始化的情况
	{
		AT24CXX_Write(255, &data, 1);
	    AT24CXX_Read(255, &temp, 1); 
		if(temp == 0x55)
		{
			printf("success\n");
			return 0;
		}
	}
	printf("fail\n");
	printf("%d\n",temp);
	return 1;											  
}

/**
 * @brief				从指定地址读多个数据
 * @param	ReadAddr	数据地址
 * @param	pBuffer		存储位置
 * @param	NumToRead	读取长度
 * 
*/
void AT24CXX_Read(uint16_t ReadAddr,uint8_t *pBuffer,uint16_t NumToRead)
{
	if(EE_TYPE < AT24C16) 
		HAL_I2C_Mem_Read(&hi2c1, AT24CXX_ADDR, ReadAddr, I2C_MEMADD_SIZE_8BIT, pBuffer, NumToRead, IIC_OUTTIMES);
	else
		HAL_I2C_Mem_Read(&hi2c1, AT24CXX_ADDR, ReadAddr, I2C_MEMADD_SIZE_16BIT, pBuffer, NumToRead, IIC_OUTTIMES);	
	HAL_Delay(10);
}


/**
* @brief				向指定地址写多个数据
* @param	WriteAddr	数据地址
* @param	pBuffer 	数据首地址
* @param	NumToWrite 	要写入数据的个数
*/
void AT24CXX_Write(uint16_t WriteAddr,uint8_t *pBuffer,uint16_t NumToWrite)
{
	if(EE_TYPE < AT24C16)
		HAL_I2C_Mem_Write(&hi2c1, AT24CXX_ADDR, WriteAddr, I2C_MEMADD_SIZE_8BIT, pBuffer, NumToWrite, IIC_OUTTIMES);
	else
		HAL_I2C_Mem_Write(&hi2c1, AT24CXX_ADDR, WriteAddr, I2C_MEMADD_SIZE_16BIT, pBuffer, NumToWrite, IIC_OUTTIMES);							    
	HAL_Delay(10);
}
