#include "AT_CMD.h"
#include "BSP_DS1302.h"
#include "clock.h"
#include "BSP_UDP.h"
#include "AT24CXX.h"
#include "string.h"

void Equality_Handler(uint8_t *recvbuf, uint8_t *recvnum);
void Quention_Handler(uint8_t *recvbuf);

/**
 * @brief		AT指令处理函数
 * @param	recvbuf	命令字符串
*/
void AT_CMD(uint8_t  *recvbuf)
{
    char check_temp, i;
    uint8_t check_temp1[20], check_temp2[20];
    uint8_t *check_temp3, *check_temp4;

    recvbuf += 3;

    for(i = 0; check_temp != '\0'; i++)
    {
        check_temp = *(recvbuf + i);
        strcpy((char*)check_temp1, (char*)recvbuf);
        check_temp3 = check_temp1;
        if(check_temp == '=')
        {
            strcpy((char*)check_temp2, (char*)recvbuf);
            check_temp1[i] = '\0';
            check_temp3 = check_temp1;
            check_temp4 = check_temp2;
            check_temp4 += (i + 1);
            Equality_Handler(check_temp3, check_temp4);
        }
        else if(check_temp == '?')
        {
            check_temp1[i] = '\0';
            check_temp3 = check_temp1;
            Quention_Handler(check_temp3);
        }
        
    }
    if (!strncmp((const char *)check_temp3, CMD_AT_RUN, strlen(CMD_AT_RUN)))
	{
		Clock_SetRun();
	}else if (!strncmp((const char *)check_temp3, CMD_AT_STOP, strlen(CMD_AT_STOP)))
	{
		Clock_SetStop();
	}else if(!strncmp((const char *)check_temp3, CMD_AT_VERSION, strlen(CMD_AT_VERSION)))
	{
		Clock_GetVersion();
	}else if(!strncmp((const char *)check_temp3, CMD_AT_RESET, strlen(CMD_AT_RESET)))
	{
		HAL_NVIC_SystemReset();
	}else if(!strncmp((const char *)check_temp3, CMD_AT_TIMING, strlen(CMD_AT_TIMING)))
	{
		SendNTPreq();
	}
}

/**
 * @brief		AT指令修改函数
 * @param	recvbuf	命令字符串
*/
void Equality_Handler(uint8_t *recvbuf, uint8_t *recvnum)
{
    if(!strncmp((const char *)recvbuf, SETGET_DEVID, sizeof(SETGET_DEVID)))
    {
        SET_DEVID(recvnum);
    }
    else if(!strncmp((const char *)recvbuf, SETGET_DEVMASK, sizeof(SETGET_DEVMASK)))
    {
        SET_DEVMASK(recvnum);
    }
    else if(strncmp((const char *)recvbuf, SETGET_DEVGATE, sizeof(SETGET_DEVGATE)) == 0)
    {
        SET_DEVGATE(recvnum);
    }
    else if(strncmp((const char *)recvbuf, SETGET_DEVPORT, sizeof(SETGET_DEVPORT)) == 0)
    {
        SET_DEVPORT(recvnum);
    }

    else if(strncmp((const char *)recvbuf, SETGET_NTPID, sizeof(SETGET_NTPID)) == 0)
    {
        SET_NTPID(recvnum);
    }
    else if(strncmp((const char *)recvbuf, SETGET_NTPPORT, sizeof(SETGET_NTPPORT)) == 0)
    {
        SET_NTPPORT(recvnum);
    }

    else if(strncmp((const char *)recvbuf, SETGET_CONFIGID, sizeof(SETGET_CONFIGID)) == 0)
    {
        SET_CONFIGID(recvnum);
    }
    else if(strncmp((const char *)recvbuf, SETGET_CONFIGPORT, sizeof(SETGET_CONFIGPORT)) == 0)
    {
        SET_CONFIGPORT(recvnum);
    }

    else if(strncmp((const char *)recvbuf, CMD_AT_GETRTC, sizeof(CMD_AT_GETRTC)) == 0)
    {
        SET_RTC(recvnum);
    }
    else if(strncmp((const char *)recvbuf, CMD_AT_GETLOCAL, sizeof(CMD_AT_GETLOCAL)) == 0)
    {
        Clock_SetLocal((char *)recvnum);
    }
}

/**
 * @brief		AT指令询问函数
 * @param	recvbuf	命令字符串
*/
void Quention_Handler(uint8_t *recvbuf)
{
    uint8_t ques_temp[4], sendbuf[100];
    uint16_t port;

    if(strncmp((const char *)recvbuf, SETGET_DEVID, sizeof(SETGET_DEVID)) == 0)
    {
        AT24CXX_Read(REG_DEVID, ques_temp, 4);
        sprintf((char *)sendbuf, "Current DEVID=%d.%d.%d.%d", ques_temp[0], ques_temp[1], ques_temp[2], ques_temp[3]);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);
    }
    else if(strncmp((const char *)recvbuf, SETGET_DEVPORT, sizeof(SETGET_DEVPORT)) == 0)
    {
        AT24CXX_Read(DEV_PORT, ques_temp, 2);
        port= (ques_temp[0]<<8)+ques_temp[1];
        sprintf((char *)sendbuf, "Current DEVPORT=%d", port);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);
    }
    else if(strncmp((const char *)recvbuf, SETGET_DEVMASK, sizeof(SETGET_DEVMASK)) == 0)
    {
        AT24CXX_Read(REG_DEVMASK, ques_temp, 4);
        sprintf((char *)sendbuf, "Current DEVMASK=%d.%d.%d.%d", ques_temp[0], ques_temp[1], ques_temp[2], ques_temp[3]);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);
    }
    else if(strncmp((const char *)recvbuf, SETGET_DEVGATE, sizeof(SETGET_DEVGATE)) == 0)
    {
        AT24CXX_Read(REG_DEVGATE, ques_temp, 4);
        sprintf((char *)sendbuf, "Current DEVGATE=%d.%d.%d.%d", ques_temp[0], ques_temp[1], ques_temp[2], ques_temp[3]);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);
    }
    else if(strncmp((const char *)recvbuf, SETGET_NTPID, sizeof(SETGET_NTPID)) == 0)
    {
        AT24CXX_Read(REG_NTPID, ques_temp, 4);
        sprintf((char *)sendbuf, "Current NTPID=%d.%d.%d.%d", ques_temp[0], ques_temp[1], ques_temp[2], ques_temp[3]);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);
    }
    else if(strncmp((const char *)recvbuf, SETGET_NTPPORT, sizeof(SETGET_NTPPORT)) == 0)
    {
        AT24CXX_Read(NTP_PORT, ques_temp, 2);
        port= (ques_temp[0]<<8)+ques_temp[1];
        sprintf((char *)sendbuf, "Current NTPPORT=%d", port);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);
    }
    else if(strncmp((const char *)recvbuf, SETGET_CONFIGID, sizeof(SETGET_CONFIGID)) == 0)
    {
        AT24CXX_Read(REG_CONID, ques_temp, 4);
        sprintf((char *)sendbuf, "Current CONFIGID=%d.%d.%d.%d", ques_temp[0], ques_temp[1], ques_temp[2], ques_temp[3]);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);
    }
    else if(strncmp((const char *)recvbuf, SETGET_CONFIGPORT, sizeof(SETGET_CONFIGPORT)) == 0)
    {
        AT24CXX_Read(CONF_PORT, ques_temp, 2);
        port= (ques_temp[0]<<8)+ques_temp[1];
        sprintf((char *)sendbuf, "Current CONFIGPORT=%d", port);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);
    }

    else if(strncmp((const char *)recvbuf, CMD_AT_GETRTC, sizeof(CMD_AT_GETRTC)) == 0)
    {
        GET_RTC();
    }
    else if(strncmp((const char *)recvbuf, CMD_AT_GETLOCAL, sizeof(CMD_AT_GETLOCAL)) == 0)
    {
        Clock_GetLocal();
    }
    else if (!strncmp((const char *)recvbuf, CMD_AT_GETSTATUS, strlen(CMD_AT_GETSTATUS)))
	{
		Clock_GetClockStatus();
	}
}

/**
 * @brief		判断命令是否为AT指令
 * @param	    cmd	命令字符串
 * @retval		0-非AT指令 1-AT指令
*/
uint8_t AT_Check(char *cmd)
{
	char *buf;
	if (strncmp(cmd,"AT+",3))
	{
		return 0;
	}
	buf = strstr(cmd, "\r\n");
	if (buf == NULL)
	{
		return 0;
	}
	return 1;
}
