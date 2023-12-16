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
#define MQTT_CLIENT_ID "你的clientid"
#define MQTT_USERNAME "你的用户名"
#define MQTT_PASSWORD "你的密码"
#define MQTT_IP "你的ip"
#define MQTT_PORT "你的端口"
#define MQTT_TOPIC "你的主题"
#endif

char *WIFI_NAME_LIST[] = {"qing", "zhang"};
char *WIFI_KEY_LIST[] = {"18289255", "12345678"};
u8 WIFI_COUNT = sizeof(WIFI_NAME_LIST) / sizeof(WIFI_NAME_LIST[0]);

#define MESSAGE_y 60
#define BUTTON_FIRST_x 0
#define BUTTON_FIRST_y 80
#define BUTTON_HEIGHT 80
#define BUTTON_WIDTH 120

// 显示错误信息
// x,y:坐标
// fsize:字体大小
// x,y:坐标.err:错误信息
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
	printf("正在重做...\r\n");
}

void Hardware_Check(void)
{
	u8 wifi_switch = 0;
	u16 okoffset = 162;
	u8 fsize;
	u16 ypos = 0;
	u16 j = 0;

	char command[120] = {0};

	SysTick_Init(72);								// 延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 中断优先级分组 分2组
	USART1_Init(115200);							// 初始化串口波特率为115200
	LED_Init();										// 初始化LED
	TFTLCD_Init();									// LCD初始化
	BEEP_Init();									// 蜂鸣器初始化
	KEY_Init();										// 按键初始化
	TIM3_CH2_PWM_Init(500, 72 - 1);					// 频率是2Kh
	TIM4_CH3_PWN_INIT(500, 72 - 1);					// 频率是2Kh
	Lsens_Init();									// 初始化光敏传感器
	TP_Init();										// 触摸屏初始化
	Touch_Key_Init(6);								// 计数频率为12M
	Hwjs_Init();

	TIM_SetCompare2(TIM3, 300);
	delay_ms(50);
	TIM_SetCompare2(TIM3, 499);

	RGB_LED_Init();

	LCD_Clear(BLACK); // 黑屏
	FRONT_COLOR = WHITE;
	BACK_COLOR = BLACK;

	// 同时点亮两个LED
	LED1 = 0;
	LED2 = 0;

	// 屏幕动态选择
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

	// 检查温湿度模块
	while (DHT11_Init())
	{
		system_error_show(5, ypos + fsize * j, "DHT11 Error!", fsize);
		printf("DHT11不存在!\r\n");
	}
	LCD_ShowString(5, ypos + fsize * j++, tftlcd_data.width, tftlcd_data.height, fsize, "DHT11 Success");
	printf("DHT11初始化成功!\r\n");

	// 初始化wifi模块
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
		printf("正在连接热点...[WIFI_NAME - %s] [WIFI_KEY - %s]\r\n", WIFI_NAME_LIST[wifi_switch % WIFI_COUNT], WIFI_KEY_LIST[wifi_switch % WIFI_COUNT]);
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

	// 关闭LED
	LED1 = 1;
	LED2 = 1;

	// 蜂鸣器短叫,提示正常启动
	TIM_SetCompare3(TIM4, 499);
	delay_ms(100);
	TIM_SetCompare3(TIM4, 0);
	printf("测试中文,test english\r\n");
	printf("系统初始化完成\r\n");
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
	int rgb_current_main_color = -1; // 用于保存当前颜色索引，初始值为-1表示没有设置颜色
	int rgb_stop_timer = 0;			 // 定时器计数器
	int rgb_current_char = -1;		 // RGB彩灯的数字，为-1时表示没有数字
	char hw_buf[9];

	Hardware_Check();

	sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-status\\\"\\, \\\"status\\\": \\\"OK\\\"\\}\",0,0", MQTT_TOPIC);
	// printf("发送状态确认：%s，成功标志：%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
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

	IWDG_Init(4, 800); // 只要在1280ms内进行喂狗就不会复位系统
	My_EXTI_Init();	   // 外部中断初始化
	do
	{
		IWDG_FeedDog(); // 喂狗

		// 触摸按键模块
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
		TIM_SetCompare3(TIM4, fancy_beep_walker); // i值最大可以取499，因为ARR最大值是499.

		// 红外感应模块
		if (hw_jsbz == 1) // 如果红外接收到
		{
			hw_jsbz = 0;								 // 清零
			printf("接收到红外数据：%0.8X\r\n", hw_jsm); // 打印接收到的数据
			sprintf(hw_buf, "%0.8X", hw_jsm);

			// 红外按键——上一首
			if (!strcmp(hw_buf, "00FF02FD"))
			{
				rgb_on = 1;
				rgb_refresh = 1;
				if (--rgb_current_main_color < 0)
				{
					rgb_current_main_color += rgb_color_count;
				}
				// rgb当前没有显示字符
				if (rgb_current_char == -1)
				{
					RGB_DrawRectangle(0, 0, 4, 4, rgb_color[rgb_current_main_color]); // 使用颜色数组中的颜色
					RGB_DrawRectangle(1, 1, 3, 3, rgb_color[(rgb_current_main_color + 1) % rgb_color_count]);
					RGB_DrawDotColor(2, 2, 1, rgb_color[(rgb_current_main_color + 2) % rgb_color_count]);
				}
				// rbg正在显示字符的话只改变颜色
				else
				{
					RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color]);
				}
			}
			// 红外按键——下一首
			else if (!strcmp(hw_buf, "00FFC23D"))
			{
				rgb_on = 1;
				rgb_refresh = 1;
				// rgb当前没有显示字符
				if (rgb_current_char == -1)
				{
					RGB_DrawRectangle(0, 0, 4, 4, rgb_color[++rgb_current_main_color % rgb_color_count]); // 使用颜色数组中的颜色
					RGB_DrawRectangle(1, 1, 3, 3, rgb_color[(rgb_current_main_color + 1) % rgb_color_count]);
					RGB_DrawDotColor(2, 2, 1, rgb_color[(rgb_current_main_color + 2) % rgb_color_count]);
				}
				// rbg正在显示字符的话只改变颜色
				else
				{
					RGB_ShowCharNum(rgb_current_char, rgb_color[++rgb_current_main_color % rgb_color_count]);
				}
			}
			// 红外按键——停播
			else if (!strcmp(hw_buf, "00FF22DD"))
			{
				rgb_on = 0;
				RGB_LED_Clear();
				rgb_current_main_color = -1;
				rgb_current_char = -1;
			}
			// 红外按键——0
			else if (!strcmp(hw_buf, "00FF6897"))
			{
				rgb_current_char = 0;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// 红外按键——1
			else if (!strcmp(hw_buf, "00FF30CF"))
			{
				rgb_current_char = 1;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// 红外按键——2
			else if (!strcmp(hw_buf, "00FF18E7"))
			{
				rgb_current_char = 2;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// 红外按键——3
			else if (!strcmp(hw_buf, "00FF7A85"))
			{
				rgb_current_char = 3;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// 红外按键——4
			else if (!strcmp(hw_buf, "00FF10EF"))
			{
				rgb_current_char = 4;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// 红外按键——5
			else if (!strcmp(hw_buf, "00FF38C7"))
			{
				rgb_current_char = 5;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// 红外按键——6
			else if (!strcmp(hw_buf, "00FF5AA5"))
			{
				rgb_current_char = 6;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// 红外按键——7
			else if (!strcmp(hw_buf, "00FF42BD"))
			{
				rgb_current_char = 7;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// 红外按键——8
			else if (!strcmp(hw_buf, "00FF4AB5"))
			{
				rgb_current_char = 8;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			// 红外按键——9
			else if (!strcmp(hw_buf, "00FF52AD"))
			{
				rgb_current_char = 9;
				RGB_ShowCharNum(rgb_current_char, rgb_color[rgb_current_main_color == -1 ? 0 : rgb_current_main_color]);
				rgb_on = 1;
				rgb_refresh = 1;
			}
			hw_jsm = 0; // 接收码清零
		}

		if (!(temp_humi_stop_timer++ % 4000))
		{
			// 温湿度模块
			DHT11_Read_Data(&temp, &humi); // 读取一次DHT11数据最少要大于100ms
			temp_buf[0] = temp / 10 + 0x30;
			temp_buf[1] = temp % 10 + 0x30;
			temp_buf[2] = '\0';
			humi_buf[0] = humi / 10 + 0x30;
			humi_buf[1] = humi % 10 + 0x30;
			humi_buf[2] = '\0';
			LCD_ShowString(45, 0, tftlcd_data.width, tftlcd_data.height, 16, temp_buf);
			LCD_ShowString(45, 20, tftlcd_data.width, tftlcd_data.height, 16, humi_buf);
			// 发送温湿度
			sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-DHT11\\\"\\, \\\"temp\\\": \\\"%d\\\"\\, \\\"humi\\\": \\\"%d\\\"\\}\",0,0", MQTT_TOPIC, temp, humi);
			// printf("发送温湿度数据：%s，成功标志：%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
			LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending temp_humi message...");
			ESP8266_Cmd(response, 0, 0, 5000);
			LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);
			printf("发送一次温湿度信息\r\n");
		}

		if (!(lsens_stop_timer++ % 3000))
		{
			// 光敏模块
			lsens = Lsens_Get_Val();
			lsens_buf[0] = lsens / 10 + 0x30;
			lsens_buf[1] = lsens % 10 + 0x30;
			lsens_buf[2] = '\0';
			LCD_ShowString(51, 40, tftlcd_data.width, tftlcd_data.height, 16, lsens_buf);
			sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-lsens\\\"\\, \\\"lsens\\\": \\\"%d\\\"\\}\",0,0", MQTT_TOPIC, lsens);
			// printf("发送温湿度数据：%s，成功标志：%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
			LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending lsens message...");
			ESP8266_Cmd(response, 0, 0, 5000);
			LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);
			printf("发送一次光敏信息：%d\r\n", lsens);
		}

		// wifi模块
		request = ESP8266_ReceiveString(DISABLE);
		if (request)
		{
			json = cJSON_Parse(strchr(request, '{'));
			memset(response, 0, strlen(response));
			if (json)
			{
				if (strstr(cJSON_GetObjectItem(json, "type")->valuestring, "browser"))
				{
					printf("browser数据，不用管\r\n");
				}
				else if (!strcmp(cJSON_GetObjectItem(json, "type")->valuestring, "LED"))
				{
					printf("type:LED\r\n");
					LED1 = !cJSON_GetObjectItem(json, "led1")->valueint;
					LED2 = !cJSON_GetObjectItem(json, "led2")->valueint;
					sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-LED\\\"\\, \\\"led1\\\": %d\\, \\\"led2\\\": %d\\}\",0,0", MQTT_TOPIC, !LED1, !LED2);
					// printf("发送LED数据：%s，成功标志：%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
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
						TIM_SetCompare2(TIM3, 499); // 值最大可以取499，因为ARR最大值是499.
					}
					else if (motor_status == 1)
					{
						TIM_SetCompare2(TIM3, 400); // 值最大可以取499，因为ARR最大值是499.
					}
					else if (motor_status == 2)
					{
						TIM_SetCompare2(TIM3, 300); // 值最大可以取499，因为ARR最大值是
					}
					sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-motor\\\"\\, \\\"motor1\\\": %d\\}\",0,0", MQTT_TOPIC, motor_status);
					// printf("发送motor数据：%s，成功标志：%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
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
					printf("未知的数据，%s\r\n", request);
				}
			}
			else
			{
				printf("json解析失败！\r\n");
			}
			cJSON_Delete(json);
		}

		// 触摸屏模块
		if (TP_Scan(0))
		{
			// 防止点击后过于敏感
			delay_ms(200);
			if (tp_dev.y[0] > BUTTON_FIRST_y && tp_dev.y[0] <= BUTTON_FIRST_y + BUTTON_HEIGHT)
			{
				// LED1反转变化
				if (tp_dev.x[0] > BUTTON_FIRST_x && tp_dev.x[0] <= BUTTON_FIRST_x + BUTTON_WIDTH)
				{
					LED1 = !LED1;
					sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-LED\\\"\\, \\\"led1\\\": %d\\, \\\"led2\\\": %d\\}\",0,0", MQTT_TOPIC, !LED1, !LED2);
					// printf("发送LED数据：%s，成功标志：%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
					LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending LED message...");
					ESP8266_Cmd(response, 0, 0, 5000);
					LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);
				}
				// LED2反转变化
				else if (tp_dev.x[0] > BUTTON_FIRST_x + BUTTON_WIDTH && tp_dev.x[0] <= BUTTON_FIRST_x + BUTTON_WIDTH * 2)
				{
					LED2 = !LED2;
					sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-LED\\\"\\, \\\"led1\\\": %d\\, \\\"led2\\\": %d\\}\",0,0", MQTT_TOPIC, !LED1, !LED2);
					// printf("发送LED数据：%s，成功标志：%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
					LCD_ShowString(0, MESSAGE_y, tftlcd_data.width, tftlcd_data.height, 16, "sending LED message...");
					ESP8266_Cmd(response, 0, 0, 5000);
					LCD_Fill(0, MESSAGE_y, tftlcd_data.width, MESSAGE_y + 16, BLACK);
				}
			}
			else if (tp_dev.y[0] > BUTTON_FIRST_y + BUTTON_HEIGHT && tp_dev.y[0] <= BUTTON_FIRST_y + BUTTON_HEIGHT * 2)
			{
				// 蜂鸣器三连
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
				// 电机循环变化
				else if (tp_dev.x[0] > BUTTON_FIRST_x + BUTTON_WIDTH && tp_dev.x[0] <= BUTTON_FIRST_x + BUTTON_WIDTH * 2)
				{
					++motor_status > 2 ? motor_status = 0 : motor_status;
					if (motor_status == 0)
					{
						TIM_SetCompare2(TIM3, 499); // 值最大可以取499，因为ARR最大值是499.
					}
					else if (motor_status == 1)
					{
						TIM_SetCompare2(TIM3, 400); // 值最大可以取499，因为ARR最大值是499.
					}
					else if (motor_status == 2)
					{
						TIM_SetCompare2(TIM3, 300); // 值最大可以取499，因为ARR最大值是
					}
					sprintf(response, "AT+MQTTPUB=0,\"%s\",\"{\\\"type\\\": \\\"browser-motor\\\"\\, \\\"motor1\\\": %d\\}\",0,0", MQTT_TOPIC, motor_status);
					// printf("发送motor数据：%s，成功标志：%d\r\n", response, ESP8266_Cmd(response, "OK", "", 5000));
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
					// rgb当前没有显示字符
					if (rgb_current_char == -1)
					{
						RGB_DrawRectangle(0, 0, 4, 4, rgb_color[++rgb_current_main_color % rgb_color_count]); // 使用颜色数组中的颜色
						RGB_DrawRectangle(1, 1, 3, 3, rgb_color[(rgb_current_main_color + 1) % rgb_color_count]);
						RGB_DrawDotColor(2, 2, 1, rgb_color[(rgb_current_main_color + 2) % rgb_color_count]);
					}
					// rbg正在显示字符的话只改变颜色
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
			rgb_on = 0;			// 关闭灯
			rgb_stop_timer = 0; // 重置计时器
			RGB_LED_Clear();
			rgb_current_main_color = -1;
			rgb_current_char = -1;
		}
	} while (1);
}
