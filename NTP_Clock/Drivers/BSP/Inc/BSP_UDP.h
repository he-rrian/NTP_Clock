#ifndef _BSP_UDP_H__
#define _BSP_UDP_H__

#include "main.h"
#include "lwip.h"
#include "udp.h"

#define LWIP_RX_BUFSIZE 2000
#define NTP_TIMESTAMP_DELTA  2208988800UL

typedef struct _DEVRECOURCE
{
    uint8_t DEV_ADDR[4];
    uint8_t DEV_MASK[4];
    uint8_t DEV_GATE[4];
    uint8_t DEST_NTP_ADDR[4];
    uint8_t DEST_CONFIG_ADDR[4];
    uint16_t q_DEV_PORT;
    uint16_t q_NTP_PORT;
    uint16_t q_CONFIG_PORT;
}DEV_RECOURCE;

typedef struct _NPTformat
{
    char    version;            /* 版本号 */
    char    leap;               /* 时钟同步 */
    char    mode;               /* 模式 */
    char    stratum;            /* 系统时钟的层数 */
    char    poll;               /* 更新间隔 */
    signed char  precision;     /* 精密度 */
    unsigned int   rootdelay;   /* 本地到主参考时钟源的往返时间 */
    unsigned int   rootdisp;    /* 统时钟相对于主参考时钟的最大误差 */
    char    refid;              /* 参考识别码 */
    unsigned long long  reftime;/* 参考时间 */
    unsigned long long  org;    /* 开始的时间戳 */
    unsigned long long  rec;    /* 收到的时间戳 */
    unsigned long long  xmt;    /* 传输时间戳 */
} NPTformat;

typedef struct _DateTime  /*此结构体定义了NTP时间同步的相关变量*/
{
    uint8_t  year;        /* 年 */
    uint8_t  month;       /* 月 */
    uint8_t  day;         /* 天 */
    uint8_t  week;
    uint8_t  hour;        /* 时 */
    uint8_t  minute;      /* 分 */
    uint8_t  second;      /* 秒 */
} DateTime;

extern DEV_RECOURCE g_devrecource;

void udp_client_init(void);
void udp_client_send(struct udp_pcb *upcb, void *pData);
void Config_Send(uint8_t* sendbuf);
void udp_client_send_ntp(void *pData);
void SendNTPreq(void);
void LWIP_Read_AT24CXX(void);
void LWIP_Write_AT24CXX(void);
void Net_Update(DEV_RECOURCE netupdate);
uint8_t IP_Check(char *cmd);

void SET_DEVID(uint8_t *recvnum);
void SET_DEVMASK(uint8_t *recvnum);
void SET_DEVGATE(uint8_t *recvnum);
void SET_DEVPORT(uint8_t *recvnum);
void SET_NTPID(uint8_t *recvnum);
void SET_NTPPORT(uint8_t *recvnum);
void SET_CONFIGID(uint8_t *recvnum);
void SET_CONFIGPORT(uint8_t *recvnum);

#endif

