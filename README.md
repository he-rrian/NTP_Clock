# NTP_Clock
基于STM32F107+LAN8720设计网络校时终端，通过LwIP协议栈获 取NTP时间，校准RTC并驱动显示

AD —— Altium Designer硬件设计源文件<br>
原理图（NTP_Clock.SchDoc）：基于STM32F107VCT6 + LAN8720方案，包含ETH RMII接口、DS1302 RTC、AT24C02<br> EEPROM、电源及LED指示电路<br>
PCB设计（NTP_Clock.PcbDoc）：双层板，主控及外围电路全部手工焊接验证<br>

NTP_Clock —— STM32 HAL库完整工程（Keil MDK<br>
基于STM32CubeMX配置生成，包含ETH、I2C、GPIO、TIM等外设初始化<br>
移植LwIP协议栈（UDP Client），实现NTP时间获取<br>
应用层：NTP协议解析、AT指令框架、EEPROM配置管理<br>
