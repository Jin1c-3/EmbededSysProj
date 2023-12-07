#include <stdio.h>
#include <string.h>
#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "beep.h"
#include "usart.h"
#include "wifi_config.h"
#include "wifi_function.h"
#include "tftlcd.h"
#include "dht11.h"
#include "key.h"
#include "pwm.h"
#include "cJSON.h"
#include "ws2812.h"

#define WIFI_NAME "qing"
#define WIFI_KEY "18289255"

// ��ʾ������Ϣ
// x,y:����
// fsize:�����С
// x,y:����.err:������Ϣ
void system_error_show(u16 x, u16 y, u8 *err, u8 fsize)
{
	u16 wait_time = 5;
	FRONT_COLOR = RED;
	while (wait_time--)
	{
		LCD_ShowString(x, y, tftlcd_data.width, tftlcd_data.height, fsize, err);
		delay_ms(200);
		LCD_Fill(x, y, tftlcd_data.width, y + fsize, BLACK);
		delay_ms(200);
	}
	FRONT_COLOR = WHITE;
	printf("��������...\r\n");
}

void Hardware_Check(void)
{
	u16 okoffset = 162;
	u8 fsize;
	u16 ypos = 0;
	u16 j = 0;

	SysTick_Init(72);								// ��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �ж����ȼ����� ��2��
	USART1_Init(115200);							// ��ʼ�����ڲ�����Ϊ115200
	LED_Init();										// ��ʼ��LED
	TFTLCD_Init();									// LCD��ʼ��
	BEEP_Init();									// ��������ʼ��
	KEY_Init();										// ������ʼ��
	TIM3_CH2_PWM_Init(500, 72 - 1);					// Ƶ����2Kh

	TIM_SetCompare2(TIM3, 0);
	delay_ms(50);
	TIM_SetCompare2(TIM3, 499);

	RGB_LED_Init();

	LCD_Clear(BLACK); // ����
	FRONT_COLOR = WHITE;
	BACK_COLOR = BLACK;

	// ͬʱ��������LED
	LED1 = 0;
	LED2 = 0;

	// ��Ļ��̬ѡ��
	if (tftlcd_data.width <= 272)
	{
		fsize = 16;
		okoffset = 190;
	}
	else if (tftlcd_data.width == 320)
	{
		fsize = 16;
		okoffset = 250;
	}
	else if (tftlcd_data.width == 480)
	{
		fsize = 24;
		okoffset = 370;
	}

	// �����ʪ��ģ��
	while (DHT11_Init())
	{
		system_error_show(5, ypos + fsize * j, "DHT11 Error!", fsize);
		printf("DHT11������!\r\n");
	}
	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "DHT11 Success");
	printf("DHT11��ʼ���ɹ�!\r\n");

	// ��ʼ��wifiģ��
	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "initiating wifi...");
	WiFi_Config();

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "ESP8266_Choose...");
	ESP8266_Choose(ENABLE);

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "ESP8266_AT_Test...");
	ESP8266_AT_Test();

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "AT+RST...");
	ESP8266_Cmd("AT+RST", "OK", "ready", 1500);

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "ESP8266_Net_Mode_Choose...");
	ESP8266_Net_Mode_Choose(STA);

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "AT+CWDHCP=1,1...");
	ESP8266_Cmd("AT+CWDHCP=1,1", "OK", "", 1500);

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "ESP8266_JoinAP...");
	while (!ESP8266_JoinAP(WIFI_NAME, WIFI_KEY))
	{
		system_error_show(5, ypos + fsize * j, "wifi connection Error!", fsize);
		printf("���������ȵ�...[WIFI_NAME - %s] [WIFI_KEY - %s]\r\n", WIFI_NAME, WIFI_KEY);
	};

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "AT+MQTTUSERCFG...");
	while (!ESP8266_Cmd("AT+MQTTUSERCFG=0,1,\"yjy\",\"user\",\"135791\",0,0,\"\"", "OK", "", 1500))
	{
		system_error_show(5, ypos + fsize * j, "wifi connection Error!", fsize);
		printf("AT+MQTTUSERCFG=0,1,\"yjy\",\"user\",\"135791\",0,0,\"\"\r\n");
	};

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "AT+MQTTCONN...");
	while (!ESP8266_Cmd("AT+MQTTCONN=0,\"101.132.78.200\",1883,1", "", "OK", 1500))
	{
		system_error_show(5, ypos + fsize * j, "mqtt connection Error!", fsize);
		printf("AT+MQTTCONN=0,\"101.132.78.200\",1883,1\r\n");
	};

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "AT+MQTTSUB...");
	while (!ESP8266_Cmd("AT+MQTTSUB=0,\"stm32\",1", "OK", "", 1500))
	{
		system_error_show(5, ypos + fsize * j, "mqtt subscribe Error!", fsize);
		printf("AT+MQTTSUB=0,\"stm32\",1\r\n");
	};

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "say hello to mqtt server...");
	while (!ESP8266_Cmd("AT+MQTTPUB=0,\"stm32\",\"{\\\"test\\\":\\\"hello from stm!\\\"}\",0,0", "OK", "", 3000))
	{
		system_error_show(5, ypos + fsize * j, "mqtt sending Error!", fsize);
		printf("AT+MQTTPUB=0,\"stm32\",\"{\\\"test\\\":\\\"hello from stm!\\\"\\}\",0,0\r\n");
	};
	// ESP8266_SendString(DISABLE, "AT+MQTTPUB=0,\"stm32\",\"{\"test\":\"hello from stm!\"}\",0,0", strlen("AT+MQTTPUB=0,\"stm32\",\"{\"test\":\"hello from stm!\"}\",0,0"), Single_ID_0);
	RGB_DrawRectangle(0, 0, 4, 4, RGB_COLOR_RED);
	delay_ms(100);
	RGB_DrawRectangle(1, 1, 3, 3, RGB_COLOR_BLUE);
	delay_ms(100);
	RGB_DrawDotColor(2, 2, 1, RGB_COLOR_GREEN);
	delay_ms(100);
	RGB_LED_Clear();

	// �ر�LED
	LED1 = 1;
	LED2 = 1;

	// �������̽�,��ʾ��������
	BEEP = 1;
	delay_ms(100);
	BEEP = 0;
	printf("��������,test english\r\n");
	printf("ϵͳ��ʼ�����\r\n");
	LCD_Clear(BLACK);
}

int main()
{
	u8 temp;
	u8 humi;
	u8 temp_buf[3], humi_buf[3];
	char response[120] = {0};
	cJSON *json;
	char *request;
	u8 motor_status;
	u8 temp_humi_flag = 0;
	u32 rgb_color[] = {RGB_COLOR_RED, RGB_COLOR_GREEN, RGB_COLOR_BLUE, RGB_COLOR_WHITE, RGB_COLOR_YELLOW, RGB_COLOR_PINK};

	Hardware_Check();

	sprintf(response, "AT+MQTTPUB=0,\"stm32\",\"{\\\"type\\\": \\\"browser-status\\\"\\, \\\"status\\\": \\\"OK\\\"\\}\",0,0");
	// printf("����״̬ȷ�ϣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
	LCD_ShowString(0, 40, tftlcd_data.width, tftlcd_data.height, 16, "sending status message...");
	ESP8266_Cmd(response, "OK", "", 5000);
	LCD_Fill(0, 40, tftlcd_data.width, 56, BLACK);

	do
	{
		temp_humi_flag++;
		// ��ʪ��ģ��
		DHT11_Read_Data(&temp, &humi); // ��ȡһ��DHT11��������Ҫ����100ms
		temp_buf[0] = temp / 10 + 0x30;
		temp_buf[1] = temp % 10 + 0x30;
		temp_buf[2] = '\0';
		humi_buf[0] = humi / 10 + 0x30;
		humi_buf[1] = humi % 10 + 0x30;
		humi_buf[2] = '\0';
		LCD_ShowString(0, 0, tftlcd_data.width, tftlcd_data.height, 16, "Temp:   degree celsius");
		LCD_ShowString(0, 20, tftlcd_data.width, tftlcd_data.height, 16, "Humi:   %");
		LCD_ShowString(45, 0, tftlcd_data.width, tftlcd_data.height, 16, temp_buf);
		LCD_ShowString(45, 20, tftlcd_data.width, tftlcd_data.height, 16, humi_buf);
		// ������ʪ��
		if (!((temp_humi_flag - 1) % 6))
		{
			sprintf(response, "AT+MQTTPUB=0,\"stm32\",\"{\\\"type\\\": \\\"browser-DHT11\\\"\\, \\\"temp\\\": \\\"%d\\\"\\, \\\"humi\\\": \\\"%d\\\"\\}\",0,0", temp, humi);
			// printf("������ʪ�����ݣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
			LCD_ShowString(0, 40, tftlcd_data.width, tftlcd_data.height, 16, "sending temp_humi message...");
			ESP8266_Cmd(response, 0, 0, 5000);
			LCD_Fill(0, 40, tftlcd_data.width, 56, BLACK);
		}

		// wifiģ��
		request = ESP8266_ReceiveString(DISABLE);
		if (request)
		{
			json = cJSON_Parse(strchr(request, '{'));
			if (json)
			{
				if (!strcmp(cJSON_GetObjectItem(json, "type")->valuestring, "LED"))
				{
					printf("type:LED\r\n");
					LED1 = !cJSON_GetObjectItem(json, "led1")->valueint;
					LED2 = !cJSON_GetObjectItem(json, "led2")->valueint;
					sprintf(response, "AT+MQTTPUB=0,\"stm32\",\"{\\\"type\\\": \\\"browser-LED\\\"\\, \\\"led1\\\": %d\\, \\\"led2\\\": %d\\}\",0,0", !LED1, !LED2);
					// printf("����LED���ݣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
					LCD_ShowString(0, 40, tftlcd_data.width, tftlcd_data.height, 16, "sending LED message...");
					ESP8266_Cmd(response, 0, 0, 5000);
					LCD_Fill(0, 40, tftlcd_data.width, 56, BLACK);
				}
				else if (!strcmp(cJSON_GetObjectItem(json, "type")->valuestring, "buzzer"))
				{
					printf("type:buzzer\r\n");
					if (cJSON_GetObjectItem(json, "buzzer1")->valueint)
					{
						BEEP = 1;
						delay_ms(100);
						BEEP = 0;
						delay_ms(50);
						BEEP = 1;
						delay_ms(100);
						BEEP = 0;
						delay_ms(50);
						BEEP = 1;
						delay_ms(100);
						BEEP = 0;
					}
				}
				else if (!strcmp(cJSON_GetObjectItem(json, "type")->valuestring, "motor"))
				{
					printf("type:motor\r\n");
					motor_status = cJSON_GetObjectItem(json, "motor1")->valueint;
					if (motor_status == 0)
					{
						TIM_SetCompare2(TIM3, 499); // ֵ������ȡ499����ΪARR���ֵ��499.
					}
					else if (motor_status == 1)
					{
						TIM_SetCompare2(TIM3, 400); // ֵ������ȡ499����ΪARR���ֵ��499.
					}
					else if (motor_status == 2)
					{
						TIM_SetCompare2(TIM3, 300); // ֵ������ȡ499����ΪARR���ֵ��
					}
					sprintf(response, "AT+MQTTPUB=0,\"stm32\",\"{\\\"type\\\": \\\"browser-motor\\\"\\, \\\"motor1\\\": %d\\}\",0,0", motor_status);
					// printf("����motor���ݣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
					LCD_ShowString(0, 40, tftlcd_data.width, tftlcd_data.height, 16, "sending motor message...");
					ESP8266_Cmd(response, 0, 0, 5000);
					LCD_Fill(0, 40, tftlcd_data.width, 56, BLACK);
				}
				else if (!strcmp(cJSON_GetObjectItem(json, "type")->valuestring, "RGB"))
				{
					printf("type:RGB\r\n");
					RGB_DrawRectangle(0, 0, 4, 4, RGB_COLOR_RED);
					delay_ms(100);
					RGB_DrawRectangle(1, 1, 3, 3, RGB_COLOR_BLUE);
					delay_ms(100);
					RGB_DrawDotColor(2, 2, 1, RGB_COLOR_GREEN);
					delay_ms(100);
					RGB_ShowCharNum(cJSON_GetObjectItem(json, "num")->valueint % 16, rgb_color[cJSON_GetObjectItem(json, "color")->valueint % 6]);
				}
				else
				{
					printf("δ֪�����ݣ�%s\r\n", request);
				}
			}
			else
			{
				printf("json����ʧ�ܣ�\r\n");
			}
		}
		cJSON_Delete(json);
	} while (1);
}
