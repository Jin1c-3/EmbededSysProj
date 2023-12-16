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
#include "touch.h"
#include "lsens.h"
#include "my_mqtt.h"
#include "exti.h"
#include "iwdg.h"
#include "hwjs.h"
#include "touch_key.h"

#ifndef _mymqtt_H
#define MQTT_CLIENT_ID "���clientid"
#define MQTT_USERNAME "����û���"
#define MQTT_PASSWORD "�������"
#define MQTT_IP "���ip"
#define MQTT_PORT "��Ķ˿�"
#define MQTT_TOPIC "�������"
#endif

char *WIFI_NAME_LIST[] = {"qing", "zhang"};
char *WIFI_KEY_LIST[] = {"18289255", "12345678"};
u8 WIFI_COUNT = sizeof(WIFI_NAME_LIST) / sizeof(WIFI_NAME_LIST[0]);

#define MESSAGE_y 60
#define BUTTON_FIRST_x 0
#define BUTTON_FIRST_y 80
#define BUTTON_HEIGHT 80
#define BUTTON_WIDTH 120

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
	u8 wifi_switch = 0;
	u16 okoffset = 162;
	u8 fsize;
	u16 ypos = 0;
	u16 j = 0;

	char command[120] = {0};

	SysTick_Init(72);								// ��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �ж����ȼ����� ��2��
	USART1_Init(115200);							// ��ʼ�����ڲ�����Ϊ115200
	LED_Init();										// ��ʼ��LED
	TFTLCD_Init();									// LCD��ʼ��
	BEEP_Init();									// ��������ʼ��
	KEY_Init();										// ������ʼ��
	TIM3_CH2_PWM_Init(500, 72 - 1);					// Ƶ����2Kh
	TIM4_CH3_PWN_INIT(500, 72 - 1);					// Ƶ����2Kh
	Lsens_Init();									// ��ʼ������������
	TP_Init();										// ��������ʼ��
	Touch_Key_Init(6);								// ����Ƶ��Ϊ12M
	Hwjs_Init();

	TIM_SetCompare2(TIM3, 300);
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
	ESP8266_Cmd("AT+RST", 0, 0, 1500);

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "ESP8266_Net_Mode_Choose...");
	while (!ESP8266_Net_Mode_Choose(STA))
	{
		system_error_show(5, ypos + fsize * j, "ESP8266_Net_Mode_Choose Error!", fsize);
		printf("%s\r\n", command);
	};

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "AT+CWDHCP=1,1...");
	ESP8266_Cmd("AT+CWDHCP=1,1", 0, 0, 1500);

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "ESP8266_JoinAP...");
	while (!ESP8266_JoinAP(WIFI_NAME_LIST[wifi_switch % WIFI_COUNT], WIFI_KEY_LIST[wifi_switch % WIFI_COUNT]))
	{
		system_error_show(5, ypos + fsize * j, "wifi connection Error!", fsize);
		printf("���������ȵ�...[WIFI_NAME - %s] [WIFI_KEY - %s]\r\n", WIFI_NAME_LIST[wifi_switch % WIFI_COUNT], WIFI_KEY_LIST[wifi_switch % WIFI_COUNT]);
		wifi_switch++;
	};

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "AT+MQTTUSERCFG...");
	sprintf(command, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"", MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD);
	while (!ESP8266_Cmd(command, "OK", "", 1500))
	{
		system_error_show(5, ypos + fsize * j, "mqtt usercfg Error!", fsize);
		printf("%s\r\n", command);
	};

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "AT+MQTTCONN...");
	sprintf(command, "AT+MQTTCONN=0,\"%s\",%s,1", MQTT_IP, MQTT_PORT);
	while (!ESP8266_Cmd(command, "", "OK", 1500))
	{
		system_error_show(5, ypos + fsize * j, "mqtt connection Error!", fsize);
		printf("%s\r\n", command);
	};

	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "AT+MQTTSUB...");
	sprintf(command, "AT+MQTTSUB=0,\"%s\",1", MQTT_TOPIC);
	while (!ESP8266_Cmd(command, "OK", "", 1500))
	{
		system_error_show(5, ypos + fsize * j, "mqtt subscribe Error!", fsize);
		printf("%s\r\n", command);
	};

	/* LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "say hello to mqtt server...");
	sprintf(command, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-status\\\"\\, \\\"status\\\": \\\"OK\\\"\\}\",0,0", MQTT_TOPIC);
	while (!ESP8266_Cmd(command, "OK", "", 3000))
	{
		system_error_show(5, ypos + fsize * j, "mqtt sending Error!", fsize);
		printf("%s\r\n", command);
	}; */
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
	TIM_SetCompare3(TIM4, 499);
	delay_ms(100);
	TIM_SetCompare3(TIM4, 0);
	printf("��������,test english\r\n");
	printf("ϵͳ��ʼ�����\r\n");
	LCD_Clear(BLACK);
}

int main()
{
	int fancy_beep_walker = 499;
	u8 fancy_beep_convertor = 0;
	u8 fancy_beep_on = 0;

	u8 temp, humi, lsens = 0;
	u8 temp_buf[3], humi_buf[3], lsens_buf[3];
	char response[120] = {0};
	cJSON *json;
	char *request;
	u8 motor_status = 0;
	int temp_humi_stop_timer = 0;
	int lsens_stop_timer = 0;
	u32 rgb_color[] = {RGB_COLOR_RED, RGB_COLOR_GREEN, RGB_COLOR_BLUE, RGB_COLOR_WHITE, RGB_COLOR_YELLOW, RGB_COLOR_PINK};
	u8 rgb_color_count = sizeof(rgb_color) / sizeof(rgb_color[0]);
	char rgb_on = 0;
	char rgb_refresh = 0;
	int rgb_current_main_color = -1; // ���ڱ��浱ǰ��ɫ��������ʼֵΪ-1��ʾû��������ɫ
	int rgb_stop_timer = 0;			 // ��ʱ��������
	int rgb_current_char = -1;		 // RGB�ʵƵ����֣�Ϊ-1ʱ��ʾû������
	char hw_buf[9];

	Hardware_Check();

	sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-status\\\"\\, \\\"status\\\": \\\"OK\\\"\\}\",0,0", MQTT_TOPIC);
	// printf("����״̬ȷ�ϣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
	LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending status message...");
	ESP8266_Cmd(response, "OK", 0, 5000);
	LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);

	LCD_ShowString(0, 0, tftlcd_data.width, tftlcd_data.height, 16, "Temp:   degree celsius");
	LCD_ShowString(0, 20, tftlcd_data.width, tftlcd_data.height, 16, "Humi:   %");
	LCD_ShowString(0, 40, tftlcd_data.width, tftlcd_data.height, 16, "Lsens:   ");

	LCD_DrawRectangle(BUTTON_FIRST_x, BUTTON_FIRST_y, BUTTON_FIRST_x + BUTTON_WIDTH, BUTTON_FIRST_y + BUTTON_HEIGHT);
	LCD_DrawRectangle(BUTTON_FIRST_x + BUTTON_WIDTH, BUTTON_FIRST_y, BUTTON_FIRST_x + BUTTON_WIDTH * 2, BUTTON_FIRST_y + BUTTON_HEIGHT);
	LCD_DrawRectangle(BUTTON_FIRST_x, BUTTON_FIRST_y + BUTTON_HEIGHT, BUTTON_FIRST_x + BUTTON_WIDTH, BUTTON_FIRST_y + BUTTON_HEIGHT * 2);
	LCD_DrawRectangle(BUTTON_FIRST_x + BUTTON_WIDTH, BUTTON_FIRST_y + BUTTON_HEIGHT, BUTTON_FIRST_x + BUTTON_WIDTH * 2, BUTTON_FIRST_y + BUTTON_HEIGHT * 2);
	LCD_DrawRectangle(BUTTON_FIRST_x, BUTTON_FIRST_y + BUTTON_HEIGHT * 2, BUTTON_FIRST_x + BUTTON_WIDTH, BUTTON_FIRST_y + BUTTON_HEIGHT * 3);
	LCD_DrawRectangle(BUTTON_FIRST_x + BUTTON_WIDTH, BUTTON_FIRST_y + BUTTON_HEIGHT * 2, BUTTON_FIRST_x + BUTTON_WIDTH * 2, BUTTON_FIRST_y + BUTTON_HEIGHT * 3);
	LCD_ShowString(BUTTON_FIRST_x + BUTTON_WIDTH / 2 - 32, BUTTON_FIRST_y + BUTTON_HEIGHT / 2 - 8, tftlcd_data.width, tftlcd_data.height, 16, "LED1");
	LCD_ShowString(BUTTON_FIRST_x + BUTTON_WIDTH * 3 / 2 - 32, BUTTON_FIRST_y + BUTTON_HEIGHT / 2 - 8, tftlcd_data.width, tftlcd_data.height, 16, "LED2");
	LCD_ShowString(BUTTON_FIRST_x + BUTTON_WIDTH / 2 - 32, BUTTON_FIRST_y + BUTTON_HEIGHT * 3 / 2 - 8, tftlcd_data.width, tftlcd_data.height, 16, "BEEP");
	LCD_ShowString(BUTTON_FIRST_x + BUTTON_WIDTH * 3 / 2 - 40, BUTTON_FIRST_y + BUTTON_HEIGHT * 3 / 2 - 8, tftlcd_data.width, tftlcd_data.height, 16, "MOTOR");
	LCD_ShowString(BUTTON_FIRST_x + BUTTON_WIDTH / 2 - 50, BUTTON_FIRST_y + BUTTON_HEIGHT * 5 / 2 - 8, tftlcd_data.width, tftlcd_data.height, 16, "Switch Color");
	LCD_ShowString(BUTTON_FIRST_x + BUTTON_WIDTH * 3 / 2 - 40, BUTTON_FIRST_y + BUTTON_HEIGHT * 5 / 2 - 8, tftlcd_data.width, tftlcd_data.height, 16, "Color Off");

	IWDG_Init(4, 800); // ֻҪ��1280ms�ڽ���ι���Ͳ��Ḵλϵͳ
	My_EXTI_Init();	   // �ⲿ�жϳ�ʼ��
	do
	{
		IWDG_FeedDog(); // ι��

		// ��������ģ��
		if (Touch_Key_Scan(0))
		{
			fancy_beep_on = !fancy_beep_on;
		}
		if (fancy_beep_on)
		{
			if (fancy_beep_convertor == 0)
			{
				fancy_beep_walker += 10;
				if (fancy_beep_walker == 450)
				{
					fancy_beep_convertor = 1;
				}
			}
			else
			{
				fancy_beep_walker -= 10;
				if (fancy_beep_walker == 0)
				{
					fancy_beep_convertor = 0;
				}
			}
		}
		else
		{
			fancy_beep_walker = 0;
		}
		TIM_SetCompare3(TIM4, fancy_beep_walker); // iֵ������ȡ499����ΪARR���ֵ��499.

		// �����Ӧģ��
		if (hw_jsbz == 1) // ���������յ�
		{
			hw_jsbz = 0;								 // ����
			printf("���յ��������ݣ�%0.8X\r\n", hw_jsm); // ��ӡ���յ�������
			sprintf(hw_buf, "%0.8X", hw_jsm);

			// ���ⰴ��������һ��
			if (!strcmp(hw_buf, "00FF02FD"))
			{
				rgb_on = 1;
				rgb_refresh = 1;
				if (--rgb_current_main_color < 0)
				{
					rgb_current_main_color += rgb_color_count;
				}
				// rgb��ǰû����ʾ�ַ�
				if (rgb_current_char == -1)
				{
					RGB_DrawRectangle(0, 0, 4, 4, rgb_color[rgb_current_main_color]); // ʹ����ɫ�����е���ɫ
					RGB_DrawRectangle(1, 1, 3, 3, rgb_color[(rgb_current_main_color + 1) % rgb_color_count]);
					RGB_DrawDotColor(2, 2, 1, rgb_color[(rgb_current_main_color + 2) % rgb_color_count]);
				}
				// rbg������ʾ�ַ��Ļ�ֻ�ı���ɫ
				else
				{
					RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color]);
				}
			}
			// ���ⰴ��������һ��
			else if (!strcmp(hw_buf, "00FFC23D"))
			{
				rgb_on = 1;
				rgb_refresh = 1;
				// rgb��ǰû����ʾ�ַ�
				if (rgb_current_char == -1)
				{
					RGB_DrawRectangle(0, 0, 4, 4, rgb_color[++rgb_current_main_color % rgb_color_count]); // ʹ����ɫ�����е���ɫ
					RGB_DrawRectangle(1, 1, 3, 3, rgb_color[(rgb_current_main_color + 1) % rgb_color_count]);
					RGB_DrawDotColor(2, 2, 1, rgb_color[(rgb_current_main_color + 2) % rgb_color_count]);
				}
				// rbg������ʾ�ַ��Ļ�ֻ�ı���ɫ
				else
				{
					RGB_ShowCharNum(rgb_current_char, rgb_color[++rgb_current_main_color % rgb_color_count]);
				}
			}
			// ���ⰴ������ͣ��
			else if (!strcmp(hw_buf, "00FF22DD"))
			{
				rgb_on = 0;
				RGB_LED_Clear();
				rgb_current_main_color = -1;
				rgb_current_char = -1;
			}
			// ���ⰴ������0
			else if (!strcmp(hw_buf, "00FF6897"))
			{
				rgb_current_char = 0;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// ���ⰴ������1
			else if (!strcmp(hw_buf, "00FF30CF"))
			{
				rgb_current_char = 1;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// ���ⰴ������2
			else if (!strcmp(hw_buf, "00FF18E7"))
			{
				rgb_current_char = 2;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// ���ⰴ������3
			else if (!strcmp(hw_buf, "00FF7A85"))
			{
				rgb_current_char = 3;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// ���ⰴ������4
			else if (!strcmp(hw_buf, "00FF10EF"))
			{
				rgb_current_char = 4;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// ���ⰴ������5
			else if (!strcmp(hw_buf, "00FF38C7"))
			{
				rgb_current_char = 5;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// ���ⰴ������6
			else if (!strcmp(hw_buf, "00FF5AA5"))
			{
				rgb_current_char = 6;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// ���ⰴ������7
			else if (!strcmp(hw_buf, "00FF42BD"))
			{
				rgb_current_char = 7;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// ���ⰴ������8
			else if (!strcmp(hw_buf, "00FF4AB5"))
			{
				rgb_current_char = 8;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// ���ⰴ������9
			else if (!strcmp(hw_buf, "00FF52AD"))
			{
				rgb_current_char = 9;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			hw_jsm = 0; // ����������
		}

		if (!(temp_humi_stop_timer++ % 4000))
		{
			// ��ʪ��ģ��
			DHT11_Read_Data(&temp, &humi); // ��ȡһ��DHT11��������Ҫ����100ms
			temp_buf[0] = temp / 10 + 0x30;
			temp_buf[1] = temp % 10 + 0x30;
			temp_buf[2] = '\0';
			humi_buf[0] = humi / 10 + 0x30;
			humi_buf[1] = humi % 10 + 0x30;
			humi_buf[2] = '\0';
			LCD_ShowString(45, 0, tftlcd_data.width, tftlcd_data.height, 16, temp_buf);
			LCD_ShowString(45, 20, tftlcd_data.width, tftlcd_data.height, 16, humi_buf);
			// ������ʪ��
			sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-DHT11\\\"\\, \\\"temp\\\": \\\"%d\\\"\\, \\\"humi\\\": \\\"%d\\\"\\}\",0,0", MQTT_TOPIC, temp, humi);
			// printf("������ʪ�����ݣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
			LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending temp_humi message...");
			ESP8266_Cmd(response, 0, 0, 5000);
			LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);
			printf("����һ����ʪ����Ϣ\r\n");
		}

		if (!(lsens_stop_timer++ % 3000))
		{
			// ����ģ��
			lsens = Lsens_Get_Val();
			lsens_buf[0] = lsens / 10 + 0x30;
			lsens_buf[1] = lsens % 10 + 0x30;
			lsens_buf[2] = '\0';
			LCD_ShowString(51, 40, tftlcd_data.width, tftlcd_data.height, 16, lsens_buf);
			sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-lsens\\\"\\, \\\"lsens\\\": \\\"%d\\\"\\}\",0,0", MQTT_TOPIC, lsens);
			// printf("������ʪ�����ݣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
			LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending lsens message...");
			ESP8266_Cmd(response, 0, 0, 5000);
			LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);
			printf("����һ�ι�����Ϣ��%d\r\n", lsens);
		}

		// wifiģ��
		request = ESP8266_ReceiveString(DISABLE);
		if (request)
		{
			json = cJSON_Parse(strchr(request, '{'));
			memset(response, 0, strlen(response));
			if (json)
			{
				if (strstr(cJSON_GetObjectItem(json, "type")->valuestring, "browser"))
				{
					printf("browser���ݣ����ù�\r\n");
				}
				else if (!strcmp(cJSON_GetObjectItem(json, "type")->valuestring, "LED"))
				{
					printf("type:LED\r\n");
					LED1 = !cJSON_GetObjectItem(json, "led1")->valueint;
					LED2 = !cJSON_GetObjectItem(json, "led2")->valueint;
					sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-LED\\\"\\, \\\"led1\\\": %d\\, \\\"led2\\\": %d\\}\",0,0", MQTT_TOPIC, !LED1, !LED2);
					// printf("����LED���ݣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
					LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending LED message...");
					ESP8266_Cmd(response, 0, 0, 5000);
					LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);
				}
				else if (!strcmp(cJSON_GetObjectItem(json, "type")->valuestring, "buzzer"))
				{
					printf("type:buzzer\r\n");
					if (cJSON_GetObjectItem(json, "buzzer1")->valueint)
					{
						TIM_SetCompare3(TIM4, 499);
						delay_ms(100);
						TIM_SetCompare3(TIM4, 0);
						delay_ms(50);
						TIM_SetCompare3(TIM4, 499);
						delay_ms(100);
						TIM_SetCompare3(TIM4, 0);
						delay_ms(50);
						TIM_SetCompare3(TIM4, 499);
						delay_ms(100);
						TIM_SetCompare3(TIM4, 0);
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
					sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-motor\\\"\\, \\\"motor1\\\": %d\\}\",0,0", MQTT_TOPIC, motor_status);
					// printf("����motor���ݣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
					LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending motor message...");
					ESP8266_Cmd(response, 0, 0, 5000);
					LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);
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
					rgb_current_char = cJSON_GetObjectItem(json, "num")->valueint % 16;
					rgb_current_main_color = cJSON_GetObjectItem(json, "color")->valueint % rgb_color_count;
					RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color]);
					rgb_on = 1;
					rgb_refresh = 1;
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
			cJSON_Delete(json);
		}

		// ������ģ��
		if (TP_Scan(0))
		{
			// ��ֹ������������
			delay_ms(200);
			if (tp_dev.y[0] > BUTTON_FIRST_y && tp_dev.y[0] <= BUTTON_FIRST_y + BUTTON_HEIGHT)
			{
				// LED1��ת�仯
				if (tp_dev.x[0] > BUTTON_FIRST_x && tp_dev.x[0] <= BUTTON_FIRST_x + BUTTON_WIDTH)
				{
					LED1 = !LED1;
					sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-LED\\\"\\, \\\"led1\\\": %d\\, \\\"led2\\\": %d\\}\",0,0", MQTT_TOPIC, !LED1, !LED2);
					// printf("����LED���ݣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
					LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending LED message...");
					ESP8266_Cmd(response, 0, 0, 5000);
					LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);
				}
				// LED2��ת�仯
				else if (tp_dev.x[0] > BUTTON_FIRST_x + BUTTON_WIDTH && tp_dev.x[0] <= BUTTON_FIRST_x + BUTTON_WIDTH * 2)
				{
					LED2 = !LED2;
					sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-LED\\\"\\, \\\"led1\\\": %d\\, \\\"led2\\\": %d\\}\",0,0", MQTT_TOPIC, !LED1, !LED2);
					// printf("����LED���ݣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
					LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending LED message...");
					ESP8266_Cmd(response, 0, 0, 5000);
					LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);
				}
			}
			else if (tp_dev.y[0] > BUTTON_FIRST_y + BUTTON_HEIGHT && tp_dev.y[0] <= BUTTON_FIRST_y + BUTTON_HEIGHT * 2)
			{
				// ����������
				if (tp_dev.x[0] > BUTTON_FIRST_x && tp_dev.x[0] <= BUTTON_FIRST_x + BUTTON_WIDTH)
				{
					TIM_SetCompare3(TIM4, 499);
					delay_ms(100);
					TIM_SetCompare3(TIM4, 0);
					delay_ms(50);
					TIM_SetCompare3(TIM4, 499);
					delay_ms(100);
					TIM_SetCompare3(TIM4, 0);
					delay_ms(50);
					TIM_SetCompare3(TIM4, 499);
					delay_ms(100);
					TIM_SetCompare3(TIM4, 0);
				}
				// ���ѭ���仯
				else if (tp_dev.x[0] > BUTTON_FIRST_x + BUTTON_WIDTH && tp_dev.x[0] <= BUTTON_FIRST_x + BUTTON_WIDTH * 2)
				{
					++motor_status > 2 ? motor_status = 0 : motor_status;
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
					sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-motor\\\"\\, \\\"motor1\\\": %d\\}\",0,0", MQTT_TOPIC, motor_status);
					// printf("����motor���ݣ�%s���ɹ���־��%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
					LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending motor message...");
					ESP8266_Cmd(response, 0, 0, 5000);
					LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);
				}
			}
			else if (tp_dev.y[0] > BUTTON_FIRST_y + BUTTON_HEIGHT * 2 && tp_dev.y[0] <= BUTTON_FIRST_y + BUTTON_HEIGHT * 3)
			{
				rgb_stop_timer = 0;
				if (tp_dev.x[0] > BUTTON_FIRST_x && tp_dev.x[0] <= BUTTON_FIRST_x + BUTTON_WIDTH)
				{
					rgb_on = 1;
					rgb_refresh = 1;
					// rgb��ǰû����ʾ�ַ�
					if (rgb_current_char == -1)
					{
						RGB_DrawRectangle(0, 0, 4, 4, rgb_color[++rgb_current_main_color % rgb_color_count]); // ʹ����ɫ�����е���ɫ
						RGB_DrawRectangle(1, 1, 3, 3, rgb_color[(rgb_current_main_color + 1) % rgb_color_count]);
						RGB_DrawDotColor(2, 2, 1, rgb_color[(rgb_current_main_color + 2) % rgb_color_count]);
					}
					// rbg������ʾ�ַ��Ļ�ֻ�ı���ɫ
					else
					{
						RGB_ShowCharNum(rgb_current_char, rgb_color[++rgb_current_main_color % rgb_color_count]);
					}
				}
				else if (tp_dev.x[0] > BUTTON_FIRST_x + BUTTON_WIDTH && tp_dev.x[0] <= BUTTON_FIRST_x + BUTTON_WIDTH * 2)
				{
					rgb_on = 0;
					RGB_LED_Clear();
					rgb_current_main_color = -1;
					rgb_current_char = -1;
				}
			}
		}
		if (rgb_refresh)
		{
			rgb_refresh = 0;
			rgb_stop_timer = 0;
		}
		if (rgb_on && ++rgb_stop_timer > 4000)
		{
			rgb_on = 0;			// �رյ�
			rgb_stop_timer = 0; // ���ü�ʱ��
			RGB_LED_Clear();
			rgb_current_main_color = -1;
			rgb_current_char = -1;
		}
	} while (1);
}
