#ifndef _AT_CMD_H__
#define _AT_CMD_H__

#include "main.h"

#define SETGET_DEVID       "DEVID"          /* 本地IP */
#define SETGET_DEVPORT     "DEVPORT"        /* 本地端口 */
#define SETGET_DEVMASK     "DEVMASK"        /* 本地掩码 */
#define SETGET_DEVGATE     "DEVGATE"        /* 本地网关 */

#define SETGET_NTPID       "NTPID"          /* NTP地址 */
#define SETGET_NTPPORT     "NTPPORT"        /* NTP端口 */

#define SETGET_CONFIGID    "CONFIGID"       /* CONFIG地址 */
#define SETGET_CONFIGPORT  "CONFIGPORT"     /* CONFIG端口 */

#define CMD_AT_GETRTC		"RTC"		    /* RTC时间 */
#define CMD_AT_GETLOCAL		"LOCAL"	        /* Local时间 */

#define CMD_AT_RUN			"RUN"		/* 指针开始走时 */
#define CMD_AT_STOP			"STOP"		/* 停止指针走时 */
#define CMD_AT_GETSTATUS	"STATUS"	/* 获取时钟状态 */
#define CMD_AT_VERSION		"VERSION"	/* 产品介绍 */
#define CMD_AT_TIMING		"TIMING"	/* 校时 */
#define CMD_AT_RESET		"RESET"		/* 复位 */

void AT_CMD(uint8_t  *recvbuf);
uint8_t AT_Check(char *cmd);

#endif


