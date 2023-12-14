#include "exti.h"
#include "led.h"
#include "SysTick.h"
#include "key.h"
#include "beep.h"
#include "my_mqtt.h"
#include "wifi_function.h"
#include "stdio.h"

/*******************************************************************************
 * �� �� ��         : My_EXTI_Init
 * ��������		   : �ⲿ�жϳ�ʼ��
 * ��    ��         : ��
 * ��    ��         : ��
 *******************************************************************************/
void My_EXTI_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE, GPIO_PinSource2); // ѡ��GPIO�ܽ������ⲿ�ж���·
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0); // ѡ��GPIO�ܽ������ⲿ�ж���·

	// EXTI0 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;		  // EXTI0�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // ��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		  // �����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ����ָ���Ĳ�����ʼ��VIC�Ĵ���

	// EXTI2 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;		  // EXTI2�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // ��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // �����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ����ָ���Ĳ�����ʼ��VIC�Ĵ���

	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_InitStructure.EXTI_Line = EXTI_Line2;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

/*******************************************************************************
 * �� �� ��         : EXTI0_IRQHandler
 * ��������		   : �ⲿ�ж�0����
 * ��    ��         : ��
 * ��    ��         : ��
 *******************************************************************************/
void EXTI0_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line0) == 1)
	{
		if (KEY_UP == 1)
		{
			printf("���򼴽�������\r\n");
			while (1)
				;
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line0);
}

/*******************************************************************************
 * �� �� ��         : EXTI2_IRQHandler
 * ��������		   : �ⲿ�ж�2����
 * ��    ��         : ��
 * ��    ��         : ��
 *******************************************************************************/
void EXTI2_IRQHandler(void)
{
	char response[120] = {0};
	if (EXTI_GetITStatus(EXTI_Line2) == 1)
	{
		delay_ms(10);
		if (KEY2 == 0)
		{
			sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-warning\\\"\\, \\\"warn\\\": \\\"1\\\"\\}\",0,0", MQTT_TOPIC);
			ESP8266_Cmd(response, 0, 0, 5000);
			printf("����һ�ν���֪ͨ��Ϣ��%s\r\n", response);
		}
	}
	EXTI_ClearITPendingBit(EXTI_Line2);
}