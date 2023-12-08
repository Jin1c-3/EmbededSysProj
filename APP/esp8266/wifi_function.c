#include "wifi_function.h"
#include "wifi_config.h"
#include "SysTick.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/*
 * ��������ESP8266_Choose
 * ����  ��ʹ��/����WF-ESP8266ģ��
 * ����  ��enumChoose = ENABLE��ʹ��ģ��
 *         enumChoose = DISABLE������ģ��
 * ����  : ��
 * ����  �����ⲿ����
 */
void ESP8266_Choose(FunctionalState enumChoose)
{
	if (enumChoose == ENABLE)
		ESP8266_CH_HIGH_LEVEL();

	else
		ESP8266_CH_LOW_LEVEL();
}

/*
 * ��������ESP8266_Rst
 * ����  ������WF-ESP8266ģ��
 * ����  ����
 * ����  : ��
 * ����  ����ESP8266_AT_Test����
 */
void ESP8266_Rst(void)
{
#if 0
	 ESP8266_Cmd ( "AT+RST", "OK", "ready", 2500 );

#else
	ESP8266_RST_LOW_LEVEL();
	delay_ms(500);
	ESP8266_RST_HIGH_LEVEL();

#endif
}

/*
 * ��������ESP8266_AT_Test
 * ����  ����WF-ESP8266ģ�����AT��������
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
void ESP8266_AT_Test(void)
{
	ESP8266_RST_HIGH_LEVEL();

	delay_ms(1000);

	while (!ESP8266_Cmd("AT", "OK", NULL, 2000))
	{
		ESP8266_Rst();
	}
}

/*
 * ��������ESP8266_Cmd
 * ����  ����WF-ESP8266ģ�鷢��ATָ��
 * ����  ��cmd�������͵�ָ��
 *         reply1��reply2���ڴ�����Ӧ��ΪNULL��������Ӧ������Ϊ���߼���ϵ
 *         waittime���ȴ���Ӧ��ʱ��
 * ����  : 1��ָ��ͳɹ�
 *         0��ָ���ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_Cmd(char *cmd, char *reply1, char *reply2, u32 waittime)
{
	strEsp8266_Fram_Record.InfBit.FramLength = 0; // ���¿�ʼ�����µ����ݰ�
	ESP8266_Usart("%s\r\n", cmd);

	if ((reply1 == 0) && (reply2 == 0)) // ����Ҫ��������
		return true;

	delay_ms(waittime); // ��ʱ

	strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';

	if ((reply1 != 0) && (reply2 != 0))
	{
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply1) ||
				(bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply2));
	}

	else if (reply1 != 0)
	{
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply1));
	}

	else
	{
		return ((bool)strstr(strEsp8266_Fram_Record.Data_RX_BUF, reply2));
	}
}

/*
 * ��������ESP8266_Net_Mode_Choose
 * ����  ��ѡ��WF-ESP8266ģ��Ĺ���ģʽ
 * ����  ��enumMode������ģʽ
 * ����  : 1��ѡ��ɹ�
 *         0��ѡ��ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_Net_Mode_Choose(ENUM_Net_ModeTypeDef enumMode)
{
	switch (enumMode)
	{
	case STA:
		return ESP8266_Cmd("AT+CWMODE=1", "OK", "no change", 2500);

	case AP:
		return ESP8266_Cmd("AT+CWMODE=2", "OK", "no change", 2500);

	case STA_AP:
		return ESP8266_Cmd("AT+CWMODE=3", "OK", "no change", 2500);

	default:
		return false;
	}
}

/*
 * ��������ESP8266_JoinAP
 * ����  ��WF-ESP8266ģ�������ⲿWiFi
 * ����  ��pSSID��WiFi�����ַ���
 *       ��pPassWord��WiFi�����ַ���
 * ����  : 1�����ӳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_JoinAP(char *pSSID, char *pPassWord)
{
	char cCmd[120];

	sprintf(cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord);

	return ESP8266_Cmd(cCmd, "OK", NULL, 7000);
}

/*
 * ��������ESP8266_BuildAP
 * ����  ��WF-ESP8266ģ�鴴��WiFi�ȵ�
 * ����  ��pSSID��WiFi�����ַ���
 *       ��pPassWord��WiFi�����ַ���
 *       ��enunPsdMode��WiFi���ܷ�ʽ�����ַ���
 * ����  : 1�������ɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_BuildAP(char *pSSID, char *pPassWord, char *enunPsdMode)
{
	char cCmd[120];

	sprintf(cCmd, "AT+CWSAP=\"%s\",\"%s\",1,%s", pSSID, pPassWord, enunPsdMode);

	return ESP8266_Cmd(cCmd, "OK", 0, 1000);
}

/*
 * ��������ESP8266_Enable_MultipleId
 * ����  ��WF-ESP8266ģ������������
 * ����  ��enumEnUnvarnishTx�������Ƿ������
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_Enable_MultipleId(FunctionalState enumEnUnvarnishTx)
{
	char cStr[20];

	sprintf(cStr, "AT+CIPMUX=%d", (enumEnUnvarnishTx ? 1 : 0));

	return ESP8266_Cmd(cStr, "OK", 0, 500);
}

/*
 * ��������ESP8266_Link_Server
 * ����  ��WF-ESP8266ģ�������ⲿ������
 * ����  ��enumE������Э��
 *       ��ip��������IP�ַ���
 *       ��ComNum���������˿��ַ���
 *       ��id��ģ�����ӷ�������ID
 * ����  : 1�����ӳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_Link_Server(ENUM_NetPro_TypeDef enumE, char *ip, char *ComNum, ENUM_ID_NO_TypeDef id)
{
	char cStr[100] = {0}, cCmd[120];

	switch (enumE)
	{
	case enumTCP:
		sprintf(cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum);
		break;

	case enumUDP:
		sprintf(cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum);
		break;

	default:
		break;
	}

	if (id < 5)
		sprintf(cCmd, "AT+CIPSTART=%d,%s", id, cStr);

	else
		sprintf(cCmd, "AT+CIPSTART=%s", cStr);

	return ESP8266_Cmd(cCmd, "OK", "ALREAY CONNECT", 500);
}

/*
 * ��������ESP8266_StartOrShutServer
 * ����  ��WF-ESP8266ģ�鿪����رշ�����ģʽ
 * ����  ��enumMode������/�ر�
 *       ��pPortNum���������˿ں��ַ���
 *       ��pTimeOver����������ʱʱ���ַ�������λ����
 * ����  : 1�������ɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_StartOrShutServer(FunctionalState enumMode, char *pPortNum, char *pTimeOver)
{
	char cCmd1[120], cCmd2[120];

	if (enumMode)
	{
		sprintf(cCmd1, "AT+CIPSERVER=%d,%s", 1, pPortNum);
		sprintf(cCmd2, "AT+CIPSTO=%s", pTimeOver);

		return (ESP8266_Cmd(cCmd1, "OK", 0, 500) &&
				ESP8266_Cmd(cCmd2, "OK", 0, 500));
	}

	else
	{
		sprintf(cCmd1, "AT+CIPSERVER=%d,%s", 0, pPortNum);

		return ESP8266_Cmd(cCmd1, "OK", 0, 500);
	}
}

/*
 * ��������ESP8266_UnvarnishSend
 * ����  ������WF-ESP8266ģ�����͸������
 * ����  ����
 * ����  : 1�����óɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_UnvarnishSend(void)
{
	return (
		ESP8266_Cmd("AT+CIPMODE=1", "OK", 0, 500) &&
		ESP8266_Cmd("AT+CIPSEND", "\r\n", ">", 500));
}

/*
 * ��������ESP8266_SendString
 * ����  ��WF-ESP8266ģ�鷢���ַ���
 * ����  ��enumEnUnvarnishTx�������Ƿ���ʹ����͸��ģʽ
 *       ��pStr��Ҫ���͵��ַ���
 *       ��ulStrLength��Ҫ���͵��ַ������ֽ���
 *       ��ucId���ĸ�ID���͵��ַ���
 * ����  : 1�����ͳɹ�
 *         0������ʧ��
 * ����  �����ⲿ����
 */
bool ESP8266_SendString(FunctionalState enumEnUnvarnishTx, char *pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId)
{
	char cStr[20];
	bool bRet = false;

	printf("ESP8266_SendString���ڸ�ID%d��������...\r\n", ucId);

	if (enumEnUnvarnishTx)
		ESP8266_Usart("%s", pStr);

	else
	{
		if (ucId < 5)
			sprintf(cStr, "AT+CIPSEND=%d,%d", ucId, ulStrLength + 2);

		else
			sprintf(cStr, "AT+CIPSEND=%d", ulStrLength + 2);

		ESP8266_Cmd(cStr, "> ", 0, 1000);

		bRet = ESP8266_Cmd(pStr, "SEND OK", 0, 1000);
	}

	return bRet;
}

/*
 * ��������ESP8266_ReceiveString
 * ����  ��WF-ESP8266ģ������ַ���
 * ����  ��enumEnUnvarnishTx�������Ƿ���ʹ����͸��ģʽ
 * ����  : ���յ����ַ����׵�ַ
 * ����  �����ⲿ����
 */
char *ESP8266_ReceiveString(FunctionalState enumEnUnvarnishTx)
{
	char *pRecStr = 0;
	u16 out_flag = 300;

	strEsp8266_Fram_Record.InfBit.FramLength = 0;
	strEsp8266_Fram_Record.InfBit.FramFinishFlag = 0;
	// ��ֹ̫���˱�����
	while(out_flag--);
	// ���û�����ڽ�����Ϣ���Ǿ�ֱ�ӷ��ؿ�
	if (!strEsp8266_Fram_Record.InfBit.FramReceivingFlag)
	{
		return NULL;
	}
	// ˵�����ڽ��գ��ȴ��������
	while (!strEsp8266_Fram_Record.InfBit.FramFinishFlag)
		;
	strEsp8266_Fram_Record.Data_RX_BUF[strEsp8266_Fram_Record.InfBit.FramLength] = '\0';
	pRecStr = strEsp8266_Fram_Record.Data_RX_BUF;
	printf("[ESP8266_ReceiveString] %s\r\n", pRecStr);
	if (enumEnUnvarnishTx)
	{
		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, ">"))
			pRecStr = strEsp8266_Fram_Record.Data_RX_BUF;
	}

	else
	{
		if (strstr(strEsp8266_Fram_Record.Data_RX_BUF, "+IPD"))
			pRecStr = strEsp8266_Fram_Record.Data_RX_BUF;
	}

	return pRecStr;
}