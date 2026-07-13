#include "BSP_SHT2X.h"
#include "i2c.h"

/**
 * @brief				从指定地址读多个数据
 * @param	type	    0读取温度 1读取湿度
 * @param	pBuffer		存储位置
 * 
*/
void SHT2X_Read(uint8_t type, float *pBuffer)
{
    HAL_Delay(20);
    uint8_t Readbuf[2]={0,0};
    if(!type)
    {
        HAL_I2C_Mem_Read(&hi2c1, SHT2X_ADDR, 0xe3, I2C_MEMADD_SIZE_8BIT, Readbuf, 2, 500);
        *pBuffer = ((Readbuf[0]*256) + (Readbuf[1] & 0xfc)) * 175.72 / 65536 - 46.85;
    }
    else
    {
        HAL_I2C_Mem_Read(&hi2c1, SHT2X_ADDR, 0xe5, I2C_MEMADD_SIZE_8BIT, Readbuf, 2, 500);
        *pBuffer = ((Readbuf[0]*256) + (Readbuf[1] & 0xfc)) * 125.0 / 65536 - 6.0;
    }
}
