#include "wifi_config.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "wifi_function.h"

struct STRUCT_USARTx_Fram strEsp8266_Fram_Record = {0};

/**
 * @brief  WiFi_Config wifi 初始化
 * @param  无
 * @retval 无
 */
void WiFi_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

	/* 配置WiFi模块的片选（CH）引脚	复位重启（RST）引脚*/
	/*选择要控制的PA4（CH）引脚和PA15（RST）引脚*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	/* 拉低WiFi模块的片选引脚	*/
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	/* 拉高WiFi模块的复位重启引脚	*/
	GPIO_SetBits(GPIOA, GPIO_Pin_15);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;			  // PB11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);				  // 初始化GPIOB 11

	// USART3 初始化设置
	USART_InitStructure.USART_BaudRate = 115200;									// 串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式
	USART_Init(USART3, &USART_InitStructure);										// 初始化串口3

	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // 开启串口接受和总线空闲中断
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);

	USART_Cmd(USART3, ENABLE); // 使能串口3

	// Usart3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // 子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器
}

void USART3_IRQHandler(void)
{
	char ch;

	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		strEsp8266_Fram_Record.InfBit.FramReceivingFlag = 1;
		ch = USART_ReceiveData(USART3);

		if (strEsp8266_Fram_Record.InfBit.FramLength < (RX_BUF_MAX_LEN - 1)) // 预留1个字节写结束符
		{
			strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength++] = ch;
		}
	}

	if (USART_GetITStatus(USART3, USART_IT_IDLE) == SET) // 数据帧接收完毕
	{
		strEsp8266_Fram_Record.InfBit.FramFinishFlag = 1;
		strEsp8266_Fram_Record.InfBit.FramReceivingFlag = 0;
		ch = USART_ReceiveData(USART3); // 由软件序列清除中断标志位(先读USART_SR，然后读USART_DR)
	}
}

/*
 * 函数名：itoa
 * 描述  ：将整形数据转换成字符串
 * 输入  ：-radix =10 表示10进制，其他结果为0
 *         -value 要转换的整形数
 *         -buf 转换后的字符串
 *         -radix = 10
 * 输出  ：无
 * 返回  ：无
 * 调用  ：被USART3_printf()调用
 */
static char *itoa(int value, char *string, int radix)
{
	int i, d;
	int flag = 0;
	char *ptr = string;

	/* This implementation only works for decimal numbers. */
	if (radix != 10)
	{
		*ptr = 0;
		return string;
	}

	if (!value)
	{
		*ptr++ = 0x30;
		*ptr = 0;
		return string;
	}

	/* if this is a negative value insert the minus sign. */
	if (value < 0)
	{
		*ptr++ = '-';

		/* Make the value positive. */
		value *= -1;
	}

	for (i = 10000; i > 0; i /= 10)
	{
		d = value / i;

		if (d || flag)
		{
			*ptr++ = (char)(d + 0x30);
			value -= (d * i);
			flag = 1;
		}
	}

	/* Null terminate the string. */
	*ptr = 0;

	return string;

} /* NCL_Itoa */

/*
 * 函数名：USART3_printf
 * 描述  ：格式化输出，类似于C库中的printf，但这里没有用到C库
 * 输入  ：-USARTx 串口通道，这里只用到了串口2，即USART2
 *		     -Data   要发送到串口的内容的指针
 *			   -...    其他参数
 * 输出  ：无
 * 返回  ：无
 * 调用  ：外部调用
 *         典型应用USART3_printf( USART3, "\r\n this is a demo \r\n" );
 *            		 USART3_printf( USART3, "\r\n %d \r\n", i );
 *            		 USART3_printf( USART3, "\r\n %s \r\n", j );
 */
void USART3_printf(USART_TypeDef *USARTx, char *Data, ...)
{
	const char *s;
	int d;
	char buf[16];

	va_list ap;
	va_start(ap, Data);

	while (*Data != 0) // 判断是否到达字符串结束符
	{
		if (*Data == 0x5c) //'\'
		{
			switch (*++Data)
			{
			case 'r': // 回车符
				USART_SendData(USARTx, 0x0d);
				Data++;
				break;

			case 'n': // 换行符
				USART_SendData(USARTx, 0x0a);
				Data++;
				break;

			default:
				Data++;
				break;
			}
		}
		else if (*Data == '%')
		{ //
			switch (*++Data)
			{
			case 's': // 字符串
				s = va_arg(ap, const char *);
				for (; *s; s++)
				{
					USART_SendData(USARTx, *s);
					while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
						;
				}
				Data++;
				break;

			case 'd': // 十进制
				d = va_arg(ap, int);
				itoa(d, buf, 10);
				for (s = buf; *s; s++)
				{
					USART_SendData(USARTx, *s);
					while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
						;
				}
				Data++;
				break;
			default:
				Data++;
				break;
			}
		} /* end of else if */
		else
			USART_SendData(USARTx, *Data++);
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
			;
	}
}
