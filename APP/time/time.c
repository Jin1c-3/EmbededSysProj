#include "time.h"
#include "led.h"
#include "touch_key.h"
#include "usart.h"

int fancy_beep_walker;
u8 fancy_beep_convertor = 0;
u8 fancy_beep_on = 0;

void TIM6_Init(u16 per, u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE); // 使能TIM4时钟
	TIM_TimeBaseInitStructure.TIM_Period = per;			 // 自动装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler = psc;		 // 分频系数
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; // 设置向上计数模式
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStructure);

	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE); // 开启定时器中断
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);

	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;			  // 定时器中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);

	TIM_Cmd(TIM6, ENABLE); // 使能定时器
}

void TIM6_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM6, TIM_IT_Update))
	{
		if (Touch_Key_Scan())
		{
			fancy_beep_on = !fancy_beep_on;
		}
		if (fancy_beep_on)
		{
			if (fancy_beep_convertor && fancy_beep_walker > 0)
			{
				fancy_beep_walker -= 30;
				if (fancy_beep_walker == 0)
				{
					fancy_beep_convertor = 0;
				}
			}
			else
			{
				fancy_beep_walker += 30;
				if (fancy_beep_walker >= 450)
				{
					fancy_beep_convertor = 1;
				}
			}
		}
		else
		{
			fancy_beep_walker = 0;
			fancy_beep_convertor = 0;
		}
		TIM_SetCompare3(TIM4, fancy_beep_walker); // i值最大可以取499，因为ARR最大值是499.
	}
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
}
