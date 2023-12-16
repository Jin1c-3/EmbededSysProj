#include "time.h"
#include "led.h"
#include "touch_key.h"
#include "usart.h"
/*******************************************************************************
* �� �� ��         : TIM4_Init
* ��������		   : TIM4��ʼ������
* ��    ��         : per:��װ��ֵ
					 psc:��Ƶϵ��
* ��    ��         : ��
*******************************************************************************/
int fancy_beep_walker;
u8 fancy_beep_convertor = 0;
u8 fancy_beep_on = 0;
void TIM6_Init(u16 per,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);//ʹ��TIM4ʱ��
	TIM_TimeBaseInitStructure.TIM_Period=per;   //�Զ�װ��ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc; //��Ƶϵ��
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //�������ϼ���ģʽ
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE); //������ʱ���ж�
	TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;//��ʱ���ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	
	
	TIM_Cmd(TIM6,ENABLE); //ʹ�ܶ�ʱ��	
}

/*******************************************************************************
* �� �� ��         : TIM4_IRQHandler
* ��������		   : TIM4�жϺ���
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void TIM6_IRQHandler(void)
{
	printf("here1");
	if(TIM_GetITStatus(TIM6,TIM_IT_Update))
	{


		
		if(Touch_Key_Scan())
		{fancy_beep_on = !fancy_beep_on;
					printf("here2");
		}
		
		if (fancy_beep_on)
		{		printf("here3");
			if (fancy_beep_convertor)
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
		TIM_SetCompare3(TIM4, fancy_beep_walker); // iֵ������ȡ499����ΪARR���ֵ��499.
		
//		printf("fancy_beep_walker %d\r\n",fancy_beep_walker);      
		}
		TIM_ClearITPendingBit(TIM6,TIM_IT_Update);	
	}


