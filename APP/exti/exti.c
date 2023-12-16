#include "exti.h"
#include "led.h"
#include "SysTick.h"
#include "key.h"
#include "beep.h"
#include "my_mqtt.h"
#include "wifi_function.h"
#include "stdio.h"

/*******************************************************************************
 * 函 数 名         : My_EXTI_Init
 * 函数功能		   : 外部中断初始化
 * 输    入         : 无
 * 输    出         : 无
 *******************************************************************************/
void My_EXTI_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource2);//选择GPIO管脚用作外部中断线路
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource3);//选择GPIO管脚用作外部中断线路
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource4);//选择GPIO管脚用作外部中断线路
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);//选择GPIO管脚用作外部中断线路

	// EXTI0 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;		  // EXTI0中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // 子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器

	// EXTI3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;		  // EXTI2中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // 子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器

	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_InitStructure.EXTI_Line = EXTI_Line3;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

void EXTI0_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line0) == 1)
	{
		if (KEY_UP == 1)
		{
			printf("程序即将崩溃！\r\n");
			while (1)
				;
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line0);
}

void EXTI3_IRQHandler(void)
{
	char response[120] = {0};
	if (EXTI_GetITStatus(EXTI_Line3) == 1)
	{
		if (KEY1 == 1)
		{
			sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-warning\\\"\\, \\\"warn\\\": \\\"1\\\"\\}\",0,0", MQTT_TOPIC);
			ESP8266_Cmd(response, 0, 0, 5000);
			printf("发送一次紧急通知信息\r\n");
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line3);
}
