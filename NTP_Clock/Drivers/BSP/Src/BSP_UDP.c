#include "BSP_UDP.h"
#include "AT24CXX.h"
#include "string.h"
#include "BSP_DS1302.h"
#include "lwip.h"
#include "clock.h"
#include "AT_CMD.h"

ip_addr_t serverIP, serverIP_ntp;
err_t err;
uint8_t port[2];
DEV_RECOURCE g_devrecource;         //存放设备id结构体

uint8_t g_lwip_recvbuf[LWIP_RX_BUFSIZE];    //接收数据缓冲区
const char g_days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
NPTformat g_ntpformat;                                                        /* NT数据包结构体 */
DateTime g_nowdate;                                                           /* 时间结构体 */
uint8_t g_ntp_message[48];                                                    /* 发送数据包的缓存区 */ 
uint8_t sendbuf[100];

/* 第一次上电时写入 */
uint8_t default_devid[4] = {192, 168, 1, 123};
uint8_t default_devmask[4] = {255,255,255,0};
uint8_t default_devgate[4] = {192,168,1,1};
uint8_t default_ntpid[4] = {192,168,1,100};
uint8_t default_configid[4] = {192,168,1,156};
uint16_t default_devport = 2000;
uint16_t default_ntpport = 123;
uint16_t default_configport = 2345;

extern uint8_t cnt_timing;

static struct udp_pcb *upcb_config_server;	//连接管理端
static struct udp_pcb *upcb_ntp_server;		//连接NTP服务器
void lwip_calc_date_time(unsigned long long time);
void lwip_get_seconds_from_ntp_server(uint8_t *buf, uint16_t idx);
void lwip_ntp_client_init(void);

/**
 * @brief		config接收回调函数
*/
static void udp_config_server_callback(void *arg, struct udp_pcb *upcb,
    struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	uint32_t data_len = 0;
    struct pbuf *q; 
    // printf("into config_callback\r\n");

    if (p != NULL) /* 接收到不为空的数据时 */
    {
        memset(g_lwip_recvbuf, 0, LWIP_RX_BUFSIZE); /* 数据接收缓冲区清零 */  
        for (q = p; q != NULL; q = q->next) /* 遍历完整个pbuf链表 */
        {
            /* 判断要拷贝到LWIP_RX_BUFSIZE中的数据是否大于LWIP_RX_BUFSIZE的剩余空间，如果大于 */
            /* 的话就只拷贝LWIP_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据 */
            if (q->len > (LWIP_RX_BUFSIZE - data_len)) 
            {
                memcpy(g_lwip_recvbuf + data_len, q->payload, (LWIP_RX_BUFSIZE - data_len)); /* 拷贝数据 */
            }
            else 
            {
                memcpy(g_lwip_recvbuf + data_len, q->payload, q->len);
            }
            data_len += q->len; 
            if (data_len > LWIP_RX_BUFSIZE) 
            {
                break; /* 超出UDP客户端接收数组,跳出 */
            }
        }   
        /* 释放缓冲区数据 */
        pbuf_free(p);
        printf("%s\r\n",g_lwip_recvbuf);
        if(AT_Check((char*)g_lwip_recvbuf))
        {
            AT_CMD(g_lwip_recvbuf);
        }
    }
}

/**
 * @brief		ntp接收回调函数
*/
static void udp_ntp_server_callback(void *arg, struct udp_pcb *upcb,
    struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    uint32_t data_len = 0;
    struct pbuf *q; 
    // printf("into ntp_callback\r\n");

	if (p != NULL) /* 接收到不为空的数据时 */
    {
        memset(g_lwip_recvbuf, 0, LWIP_RX_BUFSIZE); /* 数据接收缓冲区清零 */  
        for (q = p; q != NULL; q = q->next) /* 遍历完整个pbuf链表 */
        {
            /* 判断要拷贝到LWIP_RX_BUFSIZE中的数据是否大于LWIP_RX_BUFSIZE的剩余空间，如果大于 */
            /* 的话就只拷贝LWIP_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据 */
            if (q->len > (LWIP_RX_BUFSIZE - data_len)) 
            {
                memcpy(g_lwip_recvbuf + data_len, q->payload, (LWIP_RX_BUFSIZE - data_len)); /* 拷贝数据 */
            }
            else 
            {
                memcpy(g_lwip_recvbuf + data_len, q->payload, q->len);
            }
            data_len += q->len; 
            if (data_len > LWIP_RX_BUFSIZE) 
            {
                break; /* 超出UDP客户端接收数组,跳出 */
            }
        }   
        // printf("%s\r\n",g_lwip_recvbuf);

        lwip_get_seconds_from_ntp_server(g_lwip_recvbuf, 40);   /* 从NTP服务器获取时间 */
        DS1302_Write_Time((DS1302_Time_t *)&g_nowdate);

        sprintf((char *)sendbuf, "Current RTC=%d-%d-%d 星期%d %d:%d:%d", g_nowdate.year + 2000, g_nowdate.month, g_nowdate.day, g_nowdate.week, g_nowdate.hour, g_nowdate.minute, g_nowdate.second);
        printf("%s\n", sendbuf);
        udp_client_send(upcb_config_server, sendbuf);
    
        /* 释放缓冲区数据 */
        pbuf_free(p);

        cnt_timing = 0;
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    }
}

/**
 *@brief     计算日期时间
 *@param     seconds?UUTC 世界标准时间
 *@retval    无
*/
void lwip_calc_date_time(unsigned long long time)
{
    unsigned int Pass4year;
    int hours_per_year;
    int year;

    if (time <= 0)
    {
        time = 0;
    }

    // g_nowdate.week = (time - 86400) % (86400 * 7) / 86400;

    // if(g_nowdate.week == 0)
    // {
    //     g_nowdate.week = 7;
    // }

    g_nowdate.second = (int)(time % 60);   /* 取秒时间 */
    time /= 60;

    g_nowdate.minute = (int)(time % 60);   /* 取分钟时间 */
    time /= 60;
    
    g_nowdate.hour = (int)(time % 24);     /* 小时数 */

    Pass4year = ((unsigned int)time / (1461L * 24L));/* 取过去多少个四年，每四年有 1461*24 小时 */

    year = (Pass4year << 2) + 1970;    /* 计算年份 */

    time %= 1461 * 24;     /* 四年中剩下的小时数 */

    for (;;)               /* 校正闰年影响的年份，计算一年中剩下的小时数 */
    {
        hours_per_year = 365 * 24;         /* 一年的小时数 */

        if ((year & 3) == 0) /* 判断闰年 */
        {
            hours_per_year += 24;          /* 是闰年，一年则多24小时，即一天 */
        }

        if (time < hours_per_year)
        {
            break;
        }

        year++;
        time -= hours_per_year;
    }

    time /= 24;   /* 一年中剩下的天数 */

    time++;       /* 假定为闰年 */

    if ((year & 3) == 0)      /* 校正闰年的误差，计算月份，日期 */
    {
        if (time > 60)
        {
            time--;
        }
        else
        {
            if (time == 60)
            {
                g_nowdate.month = 1;
                g_nowdate.day = 29;
                return ;
            }
        }
    }

    for (g_nowdate.month = 0; g_days[g_nowdate.month] < time; g_nowdate.month++)   /* 计算月日 */
    {
        time -= g_days[g_nowdate.month];
    }

    g_nowdate.day = (int)(time);
    g_nowdate.month = g_nowdate.month + 1;
    g_nowdate.hour = g_nowdate.hour + 8;
    g_nowdate.year = year - 2000;

    if (g_nowdate.month == 1 || g_nowdate.month == 2)
    {
        g_nowdate.month += 12;
        year--;
    }
    
    // 使用Zeller公式计算星期几
    g_nowdate.week = (g_nowdate.day + 2 * g_nowdate.month + 3 * (g_nowdate.month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7;
    g_nowdate.week ++;

    return;
}

/**
 *@brief     从NTP服务器获取时间
 *@param     buf：存放缓存
 *@param     idx：定义存放数据起始位置
 *@retval    无
*/
void lwip_get_seconds_from_ntp_server(uint8_t *buf, uint16_t idx)
{
    unsigned long long atk_seconds = 0; 
    uint8_t i = 0;

    for (i = 0; i < 4; i++)  /* 获取40~43位的数据 */
    {
        atk_seconds = (atk_seconds << 8) | buf[idx + i]; /* 把40~43位转成16进制再转成十进制 */
    }

    atk_seconds -= NTP_TIMESTAMP_DELTA;/* 减去减去1900-1970的时间差（2208988800秒） */
    lwip_calc_date_time(atk_seconds);       /* 由UTC时间计算日期 */
}

/**
 *@brief     初始化NTP Client信息
 *@param     无
 *@retval    无
*/
void lwip_ntp_client_init(void)
{
    uint8_t flag;

    g_ntpformat.leap = 0;           /* leap indicator */
    g_ntpformat.version = 3;        /* version number */
    g_ntpformat.mode = 3;           /* mode */
    g_ntpformat.stratum = 0;        /* stratum */
    g_ntpformat.poll = 0;           /* poll interval */
    g_ntpformat.precision = 0;      /* precision */
    g_ntpformat.rootdelay = 0;      /* root delay */
    g_ntpformat.rootdisp = 0;       /* root dispersion */
    g_ntpformat.refid = 0;          /* reference ID */
    g_ntpformat.reftime = 0;        /* reference time */
    g_ntpformat.org = 0;            /* origin timestamp */
    g_ntpformat.rec = 0;            /* receive timestamp */
    g_ntpformat.xmt = 0;            /* transmit timestamp */

    flag = (g_ntpformat.version << 3) + g_ntpformat.mode; /* one byte Flag */
    memcpy(g_ntp_message, (void const *)(&flag), 1);
}

/**
 * @brief		udp发送函数
 * @param     upcb:udp控制块,
 * @param     pData:发送数据
*/
void udp_client_send(struct udp_pcb *upcb, void *pData)
{
    struct pbuf *p;
    
    /* 分配缓冲区空间 */
    p = pbuf_alloc(PBUF_TRANSPORT, strlen(pData), PBUF_POOL);
    
    if (p != NULL)
    {
        /* 填充缓冲区数据 */
        pbuf_take(p, (char *)pData, strlen(pData));

        /* 发送udp数据 */
        udp_send(upcb, p);

        /* 释放缓冲区空间 */
        pbuf_free(p);
    }
}

/**
 * @brief	  发送请求包函数
 * @param     pData:请求包
*/
void udp_client_send_ntp(void *pData)
{
    struct pbuf *p;
    
    /* 分配缓冲区空间 */
    p = pbuf_alloc(PBUF_TRANSPORT, 48, PBUF_POOL);
    
    if (p != NULL)
    {
        /* 填充缓冲区数据 */
        pbuf_take(p, (char *)pData, 48);

        /* 发送udp数据 */
        udp_send(upcb_ntp_server, p);

        /* 释放缓冲区空间 */
        pbuf_free(p);
    }
}

/**
 * @brief	  udp初始化，创建config和udp两个udp控制块
*/
void udp_client_init(void)
{
    lwip_ntp_client_init();

    IP4_ADDR(&serverIP, g_devrecource.DEST_CONFIG_ADDR[0], g_devrecource.DEST_CONFIG_ADDR[1], g_devrecource.DEST_CONFIG_ADDR[2], g_devrecource.DEST_CONFIG_ADDR[3]);

    /* 创建udp(控制端)控制块 */
    upcb_config_server = udp_new();

    if (upcb_config_server)
    {
        /* 配置本地config端口 */
        upcb_config_server->local_port = g_devrecource.q_DEV_PORT;
        
        /* 配置服务器IP和端口 */
        err= udp_connect(upcb_config_server, &serverIP, g_devrecource.q_CONFIG_PORT);

        if (err == ERR_OK)
        {
            /* 注册接收回调函数 */
            udp_recv(upcb_config_server, udp_config_server_callback, NULL);
            
            printf("udp config connected\r\n");
        }
        else
        {
            udp_remove(upcb_config_server);
            
            printf("can not connect udp config\r\n");
        }
    }

    IP4_ADDR(&serverIP_ntp, g_devrecource.DEST_NTP_ADDR[0], g_devrecource.DEST_NTP_ADDR[1], g_devrecource.DEST_NTP_ADDR[2], g_devrecource.DEST_NTP_ADDR[3]);
    /* 创建udp(NTP端)控制块 */
    upcb_ntp_server = udp_new();
    if (upcb_ntp_server)
    {
        /* 配置本地ntp端口 */
        upcb_ntp_server->local_port = g_devrecource.q_DEV_PORT;
        
        /* 配置服务器IP和端口 */
        err= udp_connect(upcb_ntp_server, &serverIP_ntp, g_devrecource.q_NTP_PORT);

        if (err == ERR_OK)
        {
            /* 注册接收回调函数 */
            udp_recv(upcb_ntp_server, udp_ntp_server_callback, NULL);
            
            printf("udp NTP connected\r\n");
        }
        else
        {
            udp_remove(upcb_ntp_server);
            
            printf("can not connect udp NTP\r\n");
        }
    }
}

/**
 * @brief	  从AT24CXX读取设备信息
*/
void LWIP_Read_AT24CXX(void)
{
    AT24CXX_Read(REG_DEVID, g_devrecource.DEV_ADDR, 4);
    AT24CXX_Read(DEV_PORT, port, 2);
    g_devrecource.q_DEV_PORT = (port[0]<<8)+port[1];
	printf("dev_addr:%d ",g_devrecource.DEV_ADDR[0]);
	printf("%d ",g_devrecource.DEV_ADDR[1]);
	printf("%d ",g_devrecource.DEV_ADDR[2]);
	printf("%d\n",g_devrecource.DEV_ADDR[3]);
    printf("dev_port:%d\n", g_devrecource.q_DEV_PORT);

    AT24CXX_Read(REG_DEVMASK, g_devrecource.DEV_MASK, 4);
    printf("dev_mask:%d ",g_devrecource.DEV_MASK[0]);
	printf("%d ",g_devrecource.DEV_MASK[1]);
	printf("%d ",g_devrecource.DEV_MASK[2]);
	printf("%d\n",g_devrecource.DEV_MASK[3]);

    AT24CXX_Read(REG_DEVGATE, g_devrecource.DEV_GATE, 4);
	printf("dev_gate:%d ",g_devrecource.DEV_GATE[0]);
	printf("%d ",g_devrecource.DEV_GATE[1]);
	printf("%d ",g_devrecource.DEV_GATE[2]);
	printf("%d\n",g_devrecource.DEV_GATE[3]);

    AT24CXX_Read(REG_NTPID, g_devrecource.DEST_NTP_ADDR, 4);
	AT24CXX_Read(NTP_PORT, port, 2);
    g_devrecource.q_NTP_PORT = (port[0]<<8)+port[1];
	printf("ntp_addr:%d ",g_devrecource.DEST_NTP_ADDR[0]);
	printf("%d ",g_devrecource.DEST_NTP_ADDR[1]);
	printf("%d ",g_devrecource.DEST_NTP_ADDR[2]);
	printf("%d\n",g_devrecource.DEST_NTP_ADDR[3]);
    printf("ntp_port:%d\n", g_devrecource.q_NTP_PORT);

    AT24CXX_Read(REG_CONID, g_devrecource.DEST_CONFIG_ADDR, 4);
	AT24CXX_Read(CONF_PORT, port, 2);
    g_devrecource.q_CONFIG_PORT = (port[0]<<8)+port[1];
	printf("config_addr:%d ",g_devrecource.DEST_CONFIG_ADDR[0]);
	printf("%d ",g_devrecource.DEST_CONFIG_ADDR[1]);
	printf("%d ",g_devrecource.DEST_CONFIG_ADDR[2]);
	printf("%d\n",g_devrecource.DEST_CONFIG_ADDR[3]);
    printf("config_port:%d\n", g_devrecource.q_CONFIG_PORT);
}


/**
 * @brief	  第一次上电时，向AT24CXX写入基本设备信息
*/
void LWIP_Write_AT24CXX(void)
{
    uint8_t port[2];

    printf("Write AT24CXX\n");
    AT24CXX_Write(REG_DEVID, default_devid, 4);
    AT24CXX_Write(REG_DEVMASK, default_devmask, 4);
    AT24CXX_Write(REG_DEVGATE, default_devgate, 4);
    AT24CXX_Write(REG_NTPID, default_ntpid, 4);
    AT24CXX_Write(REG_CONID, default_configid, 4);
    port[0] = default_devport >> 8;
    port[1] = default_devport;
	AT24CXX_Write(DEV_PORT, port, 2);
    port[0] = default_ntpport >> 8;
	port[1] = default_ntpport;
	AT24CXX_Write(NTP_PORT, port, 2);
    port[0] = default_configport >> 8;
	port[1] = default_configport;
	AT24CXX_Write(CONF_PORT, port, 2);

    HAL_Delay(10);
}

/**
 * @brief	  更新本地IP
 * @param     netupdate:IP
*/
void Net_Update(DEV_RECOURCE netupdate)
{
    ip_addr_t ip_update;
    ip_addr_t mask_update;
    ip_addr_t gw_update;

    IP4_ADDR(&ip_update, g_devrecource.DEV_ADDR[0], g_devrecource.DEV_ADDR[1], g_devrecource.DEV_ADDR[2], g_devrecource.DEV_ADDR[3]);
    IP4_ADDR(&mask_update, g_devrecource.DEV_MASK[0], g_devrecource.DEV_MASK[1] , g_devrecource.DEV_MASK[2], g_devrecource.DEV_MASK[3]);
    IP4_ADDR(&gw_update, g_devrecource.DEV_GATE[0], g_devrecource.DEV_GATE[1], g_devrecource.DEV_GATE[2], g_devrecource.DEV_GATE[3]);

    netif_set_down(&gnetif);
    netif_set_gw(&gnetif, &gw_update);
    netif_set_netmask(&gnetif, &mask_update);
    netif_set_ipaddr(&gnetif, &ip_update);

    netif_set_up(&gnetif);
}

/**
 * @brief	  设置本地IP
 * @param     recvnum:IP
*/
void SET_DEVID(uint8_t *recvnum)
{
    int add1, add2, add3, add4;

    if(IP_Check((char *)recvnum))
    {
        sscanf((const char *)recvnum, "%d.%d.%d.%d", &add1, &add2, &add3, &add4);
        g_devrecource.DEV_ADDR[0] = add1;
        g_devrecource.DEV_ADDR[1] = add2;
        g_devrecource.DEV_ADDR[2] = add3;
        g_devrecource.DEV_ADDR[3] = add4;
        AT24CXX_Write(REG_DEVID, g_devrecource.DEV_ADDR, 4);
        AT24CXX_Read(REG_DEVID, g_devrecource.DEV_ADDR, 4);
        sprintf((char *)sendbuf, "Current DEVID=%d.%d.%d.%d", g_devrecource.DEV_ADDR[0], g_devrecource.DEV_ADDR[1], g_devrecource.DEV_ADDR[2], g_devrecource.DEV_ADDR[3]);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);

        udp_disconnect(upcb_config_server);
        udp_disconnect(upcb_ntp_server);
        Net_Update(g_devrecource);
        udp_bind(upcb_config_server, IP_ADDR_ANY, g_devrecource.q_DEV_PORT);
        udp_bind(upcb_ntp_server, IP_ADDR_ANY, g_devrecource.q_DEV_PORT);
        udp_connect(upcb_config_server, &serverIP, g_devrecource.q_CONFIG_PORT);
        udp_connect(upcb_ntp_server, &serverIP_ntp, g_devrecource.q_NTP_PORT);
    }
}

/**
 * @brief	  设置本地掩码
 * @param     recvnum:掩码
*/
void SET_DEVMASK(uint8_t *recvnum)
{
    int add1, add2, add3, add4;

    if(IP_Check((char *)recvnum))
    {
        sscanf((const char *)recvnum, "%d.%d.%d.%d", &add1, &add2, &add3, &add4);
        g_devrecource.DEV_MASK[0] = add1;
        g_devrecource.DEV_MASK[1] = add2;
        g_devrecource.DEV_MASK[2] = add3;
        g_devrecource.DEV_MASK[3] = add4;
        AT24CXX_Write(REG_DEVMASK, g_devrecource.DEV_MASK, 4);
        AT24CXX_Read(REG_DEVMASK, g_devrecource.DEV_MASK, 4);
        sprintf((char *)sendbuf, "Current DEVMASK=%d.%d.%d.%d", g_devrecource.DEV_MASK[0], g_devrecource.DEV_MASK[1], g_devrecource.DEV_MASK[2], g_devrecource.DEV_MASK[3]);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);

        udp_disconnect(upcb_config_server);
        udp_disconnect(upcb_ntp_server);
        Net_Update(g_devrecource);
        udp_bind(upcb_config_server, IP_ADDR_ANY, g_devrecource.q_DEV_PORT);
        udp_bind(upcb_ntp_server, IP_ADDR_ANY, g_devrecource.q_DEV_PORT);
        udp_connect(upcb_config_server, &serverIP, g_devrecource.q_CONFIG_PORT);
        udp_connect(upcb_ntp_server, &serverIP_ntp, g_devrecource.q_NTP_PORT);
    }
}

/**
 * @brief	  设置本地网关
 * @param     recvnum:网关
*/
void SET_DEVGATE(uint8_t *recvnum)
{
    int add1, add2, add3, add4;

    if(IP_Check((char *)recvnum))
    {
        sscanf((const char *)recvnum, "%d.%d.%d.%d", &add1, &add2, &add3, &add4);
        g_devrecource.DEV_GATE[0] = add1;
        g_devrecource.DEV_GATE[1] = add2;
        g_devrecource.DEV_GATE[2] = add3;
        g_devrecource.DEV_GATE[3] = add4;
        AT24CXX_Write(REG_DEVGATE, g_devrecource.DEV_GATE, 4);
        AT24CXX_Read(REG_DEVGATE, g_devrecource.DEV_GATE, 4);
        sprintf((char *)sendbuf, "Current DEVGATE=%d.%d.%d.%d", g_devrecource.DEV_GATE[0], g_devrecource.DEV_GATE[1], g_devrecource.DEV_GATE[2], g_devrecource.DEV_GATE[3]);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);

        udp_disconnect(upcb_config_server);
        udp_disconnect(upcb_ntp_server);
        Net_Update(g_devrecource);
        udp_bind(upcb_config_server, IP_ADDR_ANY, g_devrecource.q_DEV_PORT);
        udp_bind(upcb_ntp_server, IP_ADDR_ANY, g_devrecource.q_DEV_PORT);
        udp_connect(upcb_config_server, &serverIP, g_devrecource.q_CONFIG_PORT);
        udp_connect(upcb_ntp_server, &serverIP_ntp, g_devrecource.q_NTP_PORT);
    }
}

/**
 * @brief	  设置本地端口
 * @param     recvnum:端口
*/
void SET_DEVPORT(uint8_t *recvnum)
{
    int equalport;
    uint8_t equal_temp2[4];

    sscanf((const char *)recvnum, "%d", &equalport);
    g_devrecource.q_DEV_PORT = equalport;
    equal_temp2[0] = equalport >> 8;
	equal_temp2[1] = equalport;
    AT24CXX_Write(DEV_PORT, equal_temp2, 2);
	AT24CXX_Read(DEV_PORT, equal_temp2, 2);
    equalport= (equal_temp2[0]<<8)+equal_temp2[1];
    sprintf((char *)sendbuf, "修改完成\nDEVPORT=%d", equalport);
    printf("%s\n", sendbuf);
    Config_Send(sendbuf);
    udp_disconnect(upcb_config_server);
    udp_disconnect(upcb_ntp_server);
    upcb_config_server->local_port = g_devrecource.q_DEV_PORT;
    upcb_ntp_server->local_port = g_devrecource.q_DEV_PORT;
    udp_connect(upcb_config_server, &serverIP, g_devrecource.q_CONFIG_PORT);
    udp_connect(upcb_ntp_server, &serverIP_ntp, g_devrecource.q_NTP_PORT);
}

/**
 * @brief	  设置NTP IP
 * @param     recvnum:IP
*/
void SET_NTPID(uint8_t *recvnum)
{
    int add1, add2, add3, add4;
    uint8_t equal_temp2[4];

    if(IP_Check((char *)recvnum))
    {
        sscanf((const char *)recvnum, "%d.%d.%d.%d", &add1, &add2, &add3, &add4);
        equal_temp2[0] = add1;
        equal_temp2[1] = add2;
        equal_temp2[2] = add3;
        equal_temp2[3] = add4;
        AT24CXX_Write(REG_NTPID, equal_temp2, 4);
        AT24CXX_Read(REG_NTPID, g_devrecource.DEST_NTP_ADDR, 4);
        sprintf((char *)sendbuf, "Current NTPID=%d.%d.%d.%d", g_devrecource.DEST_NTP_ADDR[0], g_devrecource.DEST_NTP_ADDR[1], g_devrecource.DEST_NTP_ADDR[2], g_devrecource.DEST_NTP_ADDR[3]);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);
        udp_disconnect(upcb_ntp_server);
        IP4_ADDR(&serverIP_ntp, g_devrecource.DEST_NTP_ADDR[0], g_devrecource.DEST_NTP_ADDR[1], g_devrecource.DEST_NTP_ADDR[2], g_devrecource.DEST_NTP_ADDR[3]);
        udp_connect(upcb_ntp_server, &serverIP_ntp, g_devrecource.q_NTP_PORT);
    }
}

/**
 * @brief	  设置NTP端口
 * @param     recvnum:端口
*/
void SET_NTPPORT(uint8_t *recvnum)
{
    int equalport;
    uint8_t equal_temp2[4];

    sscanf((const char *)recvnum, "%d", &equalport);
    g_devrecource.q_NTP_PORT = equalport;
    equal_temp2[0] = equalport >> 8;
	equal_temp2[1] = equalport;
    AT24CXX_Write(NTP_PORT, equal_temp2, 2);
	AT24CXX_Read(NTP_PORT, equal_temp2, 2);
    equalport= (equal_temp2[0]<<8)+equal_temp2[1];
    sprintf((char *)sendbuf, "修改完成\nNTPPORT=%d", equalport);
    printf("%s\n", sendbuf);
    Config_Send(sendbuf);
    udp_disconnect(upcb_ntp_server);
    udp_connect(upcb_ntp_server, &serverIP_ntp, g_devrecource.q_NTP_PORT);
}

/**
 * @brief	  设置CONFIG IP
 * @param     recvnum:IP
*/
void SET_CONFIGID(uint8_t *recvnum)
{
    int add1, add2, add3, add4;
    uint8_t equal_temp2[4];

    if(IP_Check((char *)recvnum))
    {
        sscanf((const char *)recvnum, "%d.%d.%d.%d", &add1, &add2, &add3, &add4);
        equal_temp2[0] = add1;
        equal_temp2[1] = add2;
        equal_temp2[2] = add3;
        equal_temp2[3] = add4;
        AT24CXX_Write(REG_CONID, equal_temp2, 4);
        AT24CXX_Read(REG_CONID, g_devrecource.DEST_CONFIG_ADDR, 4);
        sprintf((char *)sendbuf, "Current CONFIGID=%d.%d.%d.%d", g_devrecource.DEST_CONFIG_ADDR[0], g_devrecource.DEST_CONFIG_ADDR[1], g_devrecource.DEST_CONFIG_ADDR[2], g_devrecource.DEST_CONFIG_ADDR[3]);
        printf("%s\n", sendbuf);
        Config_Send(sendbuf);
        udp_disconnect(upcb_config_server);
        IP4_ADDR(&serverIP, g_devrecource.DEST_CONFIG_ADDR[0], g_devrecource.DEST_CONFIG_ADDR[1], g_devrecource.DEST_CONFIG_ADDR[2], g_devrecource.DEST_CONFIG_ADDR[3]);
        udp_connect(upcb_config_server, &serverIP, g_devrecource.q_CONFIG_PORT);
    }
}

/**
 * @brief	  设置CONFIG 端口
 * @param     recvnum:端口
*/
void SET_CONFIGPORT(uint8_t *recvnum)
{
    int equalport;
    uint8_t equal_temp2[4];

    sscanf((const char *)recvnum, "%d", &equalport);
    g_devrecource.q_CONFIG_PORT = equalport;
    equal_temp2[0] = equalport >> 8;
	equal_temp2[1] = equalport;
    AT24CXX_Write(CONF_PORT, equal_temp2, 2);
    AT24CXX_Read(CONF_PORT, equal_temp2, 2);
    equalport= (equal_temp2[0]<<8)+equal_temp2[1];
    sprintf((char *)sendbuf, "修改完成\nCONFIGPORT=%d", equalport);
    printf("%s\n", sendbuf);
    Config_Send(sendbuf);
    udp_disconnect(upcb_config_server);
    udp_connect(upcb_config_server, &serverIP, g_devrecource.q_CONFIG_PORT);
}

/**
 * @brief	  向NTP发送请求包函数
*/
void SendNTPreq(void)
{
    udp_client_send_ntp((char*)g_ntp_message);
}

/**
 * @brief	  CONFIG发送函数
 * @param     sendbuf:发送数据
*/
void Config_Send(uint8_t* sendbuf)
{
    udp_client_send(upcb_config_server, sendbuf);
}

/**
 * @brief		判断修改IP是否合法
 * @param	    cmd	命令字符串
 * @retval		0-不合法 1-合法
*/
uint8_t IP_Check(char *cmd)
{
    int add1, add2, add3, add4;

    sscanf(cmd, "%d.%d.%d.%d", &add1, &add2, &add3, &add4);
    if((add1 < 255) && (add2 < 255) && (add3 < 255) && (add4 < 255))
    {
        return 1;
    }
    sprintf((char *)sendbuf, "Invalid IP");
    printf("%s\n", sendbuf);
    Config_Send(sendbuf);
    return 0;
}
