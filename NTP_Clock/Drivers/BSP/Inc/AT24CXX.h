#ifndef _AT24Cxx_H__
#define _AT24Cxx_H__

#include "main.h"

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767  
#define AT24C512	65535  

#define EE_TYPE 	AT24C02

#define AT24CXX_ADDR  0xA0      
#define IIC_OUTTIMES    500

void AT24CXX_Write(uint16_t WriteAddr,uint8_t *pBuffer,uint16_t NumToWrite);
void AT24CXX_Read(uint16_t ReadAddr,uint8_t *pBuffer,uint16_t NumToRead);
uint8_t AT24CXX_Check(void);

#endif
