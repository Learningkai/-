/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	onenet.c
	*
	*	���ߣ� 		�ż���
	*
	*	���ڣ� 		2017-05-08
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		��onenetƽ̨�����ݽ����ӿڲ�
	*
	*	�޸ļ�¼��	V1.0��Э���װ�������ж϶���ͬһ���ļ������Ҳ�ͬЭ��ӿڲ�ͬ��
	*				V1.1���ṩͳһ�ӿڹ�Ӧ�ò�ʹ�ã����ݲ�ͬЭ���ļ�����װЭ����ص����ݡ�
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32wbxx_hal.h"

//�����豸
#include "esp8266.h"

//Э���ļ�
#include "onenet.h"
#include "mqttkit.h"
//�㷨
#include "base64.h"
#include "hmac_sha1.h"

//Ӳ������
//#include "usart.h"
//#include "delay.h"
//#include "led.h"
//#include "led.h"
//#include "main.h"
#include "oled.h"
//C��
#include <string.h>
#include <stdio.h>

//CJSON��
#include "cJSON.h"

#define PROID			"YN6kc7Q2C1"

#define ACCESS_KEY		"ZnFTTEk0c01rSEdUaTdwV2RWZHJ5aGcyclp6UkU4dmw="

#define DEVICE_NAME		"c1"


char devid[16];

char key[48];


extern unsigned char esp8266_buf[512];

extern int led_info;

extern int usb_info;

/*
************************************************************
*	�������ƣ�	OTA_UrlEncode
*
*	�������ܣ�	sign��Ҫ����URL����
*
*	��ڲ�����	sign�����ܽ��
*
*	���ز�����	0-�ɹ�	����-ʧ��
*
*	˵����		+			%2B
*				�ո�		%20
*				/			%2F
*				?			%3F
*				%			%25
*				#			%23
*				&			%26
*				=			%3D
************************************************************
*/
static unsigned char OTA_UrlEncode(char *sign)
{

	char sign_t[40];
	unsigned char i = 0, j = 0;
	unsigned char sign_len = strlen(sign);
	
	if(sign == (void *)0 || sign_len < 28)
		return 1;
	
	for(; i < sign_len; i++)
	{
		sign_t[i] = sign[i];
		sign[i] = 0;
	}
	sign_t[i] = 0;
	
	for(i = 0, j = 0; i < sign_len; i++)
	{
		switch(sign_t[i])
		{
			case '+':
				strcat(sign + j, "%2B");j += 3;
			break;
			
			case ' ':
				strcat(sign + j, "%20");j += 3;
			break;
			
			case '/':
				strcat(sign + j, "%2F");j += 3;
			break;
			
			case '?':
				strcat(sign + j, "%3F");j += 3;
			break;
			
			case '%':
				strcat(sign + j, "%25");j += 3;
			break;
			
			case '#':
				strcat(sign + j, "%23");j += 3;
			break;
			
			case '&':
				strcat(sign + j, "%26");j += 3;
			break;
			
			case '=':
				strcat(sign + j, "%3D");j += 3;
			break;
			
			default:
				sign[j] = sign_t[i];j++;
			break;
		}
	}
	
	sign[j] = 0;
	
	return 0;

}

/*
************************************************************
*	�������ƣ�	OTA_Authorization
*
*	�������ܣ�	����Authorization
*
*	��ڲ�����	ver��������汾�ţ����ڸ�ʽ��Ŀǰ��֧�ָ�ʽ"2018-10-31"
*				res����Ʒid
*				et������ʱ�䣬UTC��ֵ
*				access_key��������Կ
*				dev_name���豸��
*				authorization_buf������token��ָ��
*				authorization_buf_len������������(�ֽ�)
*
*	���ز�����	0-�ɹ�	����-ʧ��
*
*	˵����		��ǰ��֧��sha1
************************************************************
*/
#define METHOD		"sha1"
static unsigned char OneNET_Authorization(char *ver, char *res, unsigned int et, char *access_key, char *dev_name,
											char *authorization_buf, unsigned short authorization_buf_len, _Bool flag)
{
	
	size_t olen = 0;
	
	char sign_buf[64];								//����ǩ����Base64������ �� URL������
	char hmac_sha1_buf[64];							//����ǩ��
	char access_key_base64[64];						//����access_key��Base64������
	char string_for_signature[72];					//����string_for_signature������Ǽ��ܵ�key

//----------------------------------------------------�����Ϸ���--------------------------------------------------------------------
	if(ver == (void *)0 || res == (void *)0 || et < 1564562581 || access_key == (void *)0
		|| authorization_buf == (void *)0 || authorization_buf_len < 120)
		return 1;
	
//----------------------------------------------------��access_key����Base64����----------------------------------------------------
	memset(access_key_base64, 0, sizeof(access_key_base64));
	BASE64_Decode((unsigned char *)access_key_base64, sizeof(access_key_base64), &olen, (unsigned char *)access_key, strlen(access_key));
	printf( "access_key_base64: %s\r\n", access_key_base64);
	
//----------------------------------------------------����string_for_signature-----------------------------------------------------
	memset(string_for_signature, 0, sizeof(string_for_signature));
	if(flag)
		snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s\n%s", et, METHOD, res, ver);
	else
		snprintf(string_for_signature, sizeof(string_for_signature), "%d\n%s\nproducts/%s/devices/%s\n%s", et, METHOD, res, dev_name, ver);
	printf( "string_for_signature: %s\r\n", string_for_signature);
	
//----------------------------------------------------����-------------------------------------------------------------------------
	memset(hmac_sha1_buf, 0, sizeof(hmac_sha1_buf));
	
	hmac_sha1((unsigned char *)access_key_base64, strlen(access_key_base64),
				(unsigned char *)string_for_signature, strlen(string_for_signature),
				(unsigned char *)hmac_sha1_buf);
	
	printf( "hmac_sha1_buf: %s\r\n", hmac_sha1_buf);
	
//----------------------------------------------------�����ܽ������Base64����------------------------------------------------------
	olen = 0;
	memset(sign_buf, 0, sizeof(sign_buf));
	BASE64_Encode((unsigned char *)sign_buf, sizeof(sign_buf), &olen, (unsigned char *)hmac_sha1_buf, strlen(hmac_sha1_buf));

//----------------------------------------------------��Base64����������URL����---------------------------------------------------
	OTA_UrlEncode(sign_buf);
	printf( "sign_buf: %s\r\n", sign_buf);
	
//----------------------------------------------------����Token--------------------------------------------------------------------
	if(flag)
		snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s&et=%d&method=%s&sign=%s", ver, res, et, METHOD, sign_buf);
	else
		snprintf(authorization_buf, authorization_buf_len, "version=%s&res=products%%2F%s%%2Fdevices%%2F%s&et=%d&method=%s&sign=%s", ver, res, dev_name, et, METHOD, sign_buf);
	printf( "Token: %s\r\n", authorization_buf);
	
	return 0;

}

//==========================================================
//	�������ƣ�	OneNET_RegisterDevice
//
//	�������ܣ�	�ڲ�Ʒ��ע��һ���豸
//
//	��ڲ�����	access_key��������Կ
//				pro_id����ƷID
//				serial��Ψһ�豸��
//				devid�����淵�ص�devid
//				key�����淵�ص�key
//
//	���ز�����	0-�ɹ�		1-ʧ��
//
//	˵����		
//==========================================================
_Bool OneNET_RegisterDevice(void)
{

	_Bool result = 1;
	unsigned short send_len = 11 + strlen(DEVICE_NAME);
	char *send_ptr = NULL, *data_ptr = NULL;
	
	char authorization_buf[144];													//���ܵ�key
	
	send_ptr = malloc(send_len + 240);
	if(send_ptr == NULL)
		return result;
	
	while(ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"183.230.40.33\",80\r\n", "CONNECT"))
		HAL_Delay(500);
	
	OneNET_Authorization("2018-10-31", PROID, 1956499200, ACCESS_KEY, NULL,
							authorization_buf, sizeof(authorization_buf), 1);
	
	snprintf(send_ptr, 240 + send_len, "POST /mqtt/v1/devices/reg HTTP/1.1\r\n"
					"Authorization:%s\r\n"
					"Host:ota.heclouds.com\r\n"
					"Content-Type:application/json\r\n"
					"Content-Length:%d\r\n\r\n"
					"{\"name\":\"%s\"}",
	
					authorization_buf, 11 + strlen(DEVICE_NAME), DEVICE_NAME);
	
	ESP8266_SendData((unsigned char *)send_ptr, strlen(send_ptr));
	
	/*
	{
	  "request_id" : "f55a5a37-36e4-43a6-905c-cc8f958437b0",
	  "code" : "onenet_common_success",
	  "code_no" : "000000",
	  "message" : null,
	  "data" : {
		"device_id" : "589804481",
		"name" : "mcu_id_43057127",
		
	"pid" : 282932,
		"key" : "indu/peTFlsgQGL060Gp7GhJOn9DnuRecadrybv9/XY="
	  }
	}
	*/
	
	data_ptr = (char *)ESP8266_GetIPD(250);							//�ȴ�ƽ̨��Ӧ
	
	if(data_ptr)
	{
		data_ptr = strstr(data_ptr, "device_id");
	}
	
	if(data_ptr)
	{
		char name[16];
		int pid = 0;
		
		if(sscanf(data_ptr, "device_id\" : \"%[^\"]\",\r\n\"name\" : \"%[^\"]\",\r\n\r\n\"pid\" : %d,\r\n\"key\" : \"%[^\"]\"", devid, name, &pid, key) == 4)
		{
			printf( "create device: %s, %s, %d, %s\r\n", devid, name, pid, key);
			result = 0;
		}
	}
	
	free(send_ptr);
	ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK");
	
	return result;

}

//==========================================================
//	�������ƣ�	OneNet_DevLink
//
//	�������ܣ�	��onenet��������
//
//	��ڲ�����	��
//
//	���ز�����	1-�ɹ�	0-ʧ��
//
//	˵����		��onenetƽ̨��������
//==========================================================
_Bool OneNet_DevLink(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};					//Э���

	unsigned char *dataPtr;
	
	char authorization_buf[160];
	
	_Bool status = 1;
	
	OneNET_Authorization("2018-10-31", PROID, 1956499200, ACCESS_KEY, DEVICE_NAME,
								authorization_buf, sizeof(authorization_buf), 0);
	
	printf( "OneNET_DevLink\r\n"
							"NAME: %s,	PROID: %s,	KEY:%s\r\n"
                        , DEVICE_NAME, PROID, authorization_buf);
	
	if(MQTT_PacketConnect(PROID, authorization_buf, DEVICE_NAME, 256, 1, MQTT_QOS_LEVEL0, NULL, NULL, 0, &mqttPacket) == 0)
	{
		ESP8266_SendData(mqttPacket._data, mqttPacket._len);			//�ϴ�ƽ̨
		
		dataPtr = ESP8266_GetIPD(250);									//�ȴ�ƽ̨��Ӧ
		if(dataPtr != NULL)
		{
			if(MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch(MQTT_UnPacketConnectAck(dataPtr))
				{
					case 0:printf( "Tips:	���ӳɹ�\r\n");status = 0;break;
					
					case 1:printf( "WARN:	����ʧ�ܣ�Э�����\r\n");break;
					case 2:printf( "WARN:	����ʧ�ܣ��Ƿ���clientid\r\n");break;
					case 3:printf( "WARN:	����ʧ�ܣ�������ʧ��\r\n");break;
					case 4:printf( "WARN:	����ʧ�ܣ��û������������\r\n");break;
					case 5:printf( "WARN:	����ʧ�ܣ��Ƿ�����(����token�Ƿ�)\r\n");break;
					
					default:printf( "ERR:	����ʧ�ܣ�δ֪����\r\n");break;
				}
			}
		}
		
		MQTT_DeleteBuffer(&mqttPacket);								//ɾ��
	}
	else
		printf( "WARN:	MQTT_PacketConnect Failed\r\n");
	
	return status;
	
}

extern float cur;

extern float ele;
unsigned char OneNet_FillBuf(char *buf)
{
	
//    // ����һ���ַ����� text����СΪ48�ֽ�
//    char text[48];
//    
//    // ��� text ���飬�������ֽ�����Ϊ 0
//    memset(text, 0, sizeof(text));
//    
//    // ����ʼ�� JSON Ƭ�ο����� buf ��
//    strcpy(buf, "{\"id\":\"123\",\"params\":{");
//    
//    // ��� text ����
//    memset(text, 0, sizeof(text));
//    
//    // ʹ�� sprintf ��������ʽ�����ַ���д�� text ��
//    // ������ַ�����ʽΪ "cur\":{\"value\":3}"
//    sprintf(text, "\"cur\":{\"value\":%d}", 3);
//    
//    // �� text ƴ�ӵ� buf ��ĩβ
//    strcat(buf, text);
//    
//    // �����Ҫ��Ӹ���Ĳ���������ȡ��ע�����´��벢�����޸�
//    // ��� text ����
//    // memset(text, 0, sizeof(text));
//    // ʹ�� sprintf ��������һ����ʽ�����ַ���д�� text ��
//    // ������ַ�����ʽΪ "kai\":{\"value\":true}"
//    // sprintf(text, "\"kai\":{\"value\":%s},", "true");
//    // �� text ƴ�ӵ� buf ��ĩβ
//    // strcat(buf, text);
//    
//    // �� buf ��ĩβ��� JSON �������
//    strcat(buf, "}}");
//    
//    // ���� buf �ĳ���
//    return strlen(buf);



    char text[48];  // ���� text �Ĵ�С����Ӧ��������

    // ��ʼ�� buf �� text
    strcpy(buf, "{\"id\":\"123\",\"params\":{");
		
		// ��� temp ����
    memset(text, 0, sizeof(text));
    sprintf(text, "\"led\":{\"value\":%s},", led_info ? "true" :"false" );
    strcat(buf, text);
		
		// ��� temp ����
    memset(text, 0, sizeof(text));
    sprintf(text, "\"usb\":{\"value\":%s},", usb_info ? "true" :"false" );
    strcat(buf, text);
		
		if(ele != 5000.00)
		{
			// ��� temp ����
			memset(text, 0, sizeof(text));
			sprintf(text, "\"ele\":{\"value\":%.2f},",ele);
			strcat(buf, text);
		}
		// ��� temp ����
    memset(text, 0, sizeof(text));
    sprintf(text, "\"cur\":{\"value\":%.2f}",cur);
    strcat(buf, text);

    // �ر� JSON ����
    strcat(buf, "}}");

    // ���� buf �ĳ���
    return strlen(buf);


}

//==========================================================
//	�������ƣ�	OneNet_SendData
//
//	�������ܣ�	�ϴ����ݵ�ƽ̨
//
//	��ڲ�����	type���������ݵĸ�ʽ
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_SendData(void)
{
	
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};												//Э���
	
	char buf[256];
	
	short body_len = 0, i = 0;
	
	//printf( "Tips:OneNet_SendData-MQTT\r\n");
	
	memset(buf, 0, sizeof(buf));
	
	body_len = OneNet_FillBuf(buf);																	//��ȡ��ǰ��Ҫ���͵����������ܳ���
	
	//printf( "Body length: %d\r\n", body_len);

    // ������ݳ����Ƿ񳬹�����
    if (body_len > 256) {
        printf( "ERROR: Data length exceeds limit\r\n");
        return;
    }
	
	
	if(body_len)
	{
		if(MQTT_PacketSaveData(PROID, DEVICE_NAME, body_len, NULL, &mqttPacket) == 0)				//���
		{
			for(; i < body_len; i++)
				mqttPacket._data[mqttPacket._len++] = buf[i];
			
      ESP8266_SendData(mqttPacket._data, mqttPacket._len);									//�ϴ����ݵ�ƽ̨
			//printf( "Send %d Bytes\r\n", mqttPacket._len);
			
			MQTT_DeleteBuffer(&mqttPacket);														//ɾ��
		}
		else
			printf( "WARN:	EDP_NewBuffer Failed\r\n");
	}
	
}


//==========================================================
//	�������ƣ�	OneNET_Publish
//
//	�������ܣ�	������Ϣ
//
//	��ڲ�����	topic������������
//				msg����Ϣ����
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNET_Publish(const char *topic, const char *msg)
{

	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						//Э���
	
	printf( "Publish Topic: %s, Msg: %s\r\n", topic, msg);
	
	if(MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg), MQTT_QOS_LEVEL0, 0, 1, &mqtt_packet) == 0)
	{
		ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					//��ƽ̨���Ͷ�������
		
		MQTT_DeleteBuffer(&mqtt_packet);										//ɾ��
	}

}

//==========================================================
//	�������ƣ�	OneNET_Subscribe
//
//	�������ܣ�	����
//
//	��ڲ�����	��
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNET_Subscribe(void)
{
	
	MQTT_PACKET_STRUCTURE mqtt_packet = {NULL, 0, 0, 0};						//Э���
	
	char topic_buf[56];
	const char *topic = topic_buf;
	
	snprintf(topic_buf, sizeof(topic_buf), "$sys/%s/%s/thing/property/set", PROID, DEVICE_NAME);
	
	printf( "Subscribe Topic: %s\r\n", topic_buf);
	
	if(MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0, &topic, 1, &mqtt_packet) == 0)
	{
		ESP8266_SendData(mqtt_packet._data, mqtt_packet._len);					//��ƽ̨���Ͷ�������
		
		MQTT_DeleteBuffer(&mqtt_packet);										//ɾ��
	}

}

//==========================================================
//	�������ƣ�	OneNet_RevPro
//
//	�������ܣ�	ƽ̨�������ݼ��
//
//	��ڲ�����	dataPtr��ƽ̨���ص�����
//
//	���ز�����	��
//
//	˵����		
//==========================================================
void OneNet_RevPro(unsigned char *cmd)
{
	
	char *req_payload = NULL;
	char *cmdid_topic = NULL;
	
	unsigned short topic_len = 0;
	unsigned short req_len = 0;
	
	unsigned char qos = 0;
	static unsigned short pkt_id = 0;
	
	unsigned char type = 0;
	
	short result = 0;

//	char *dataPtr = NULL;
//	char numBuf[10];
//	int num = 0;

	cJSON *row_json,*params_json,*led_json,*ele_json,*usb_json;
	
	type = MQTT_UnPacketRecv(cmd);
	switch(type)
	{
		case MQTT_PKT_PUBLISH:																//���յ�Publish��Ϣ
		
			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
			if(result == 0)
			{
				char *data_ptr = NULL;
				
				//printf( "topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",cmdid_topic, topic_len, req_payload, req_len);
					row_json = cJSON_Parse(req_payload);
					params_json = cJSON_GetObjectItem(row_json,"params");
					led_json = cJSON_GetObjectItem(params_json,"led");
					usb_json = cJSON_GetObjectItem(params_json,"usb");
					ele_json = cJSON_GetObjectItem(params_json,"ele");
					
				  
					if (led_json != NULL)
					{
						
						if(led_json->type == cJSON_True) {
              //Led_Set(LED_ON);
						// �� PB2 ����Ϊ�ߵ�ƽ
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
							led_info=1;
							OLED_Clear();
							OLED_ShowString(0, 0, "open", 16, 1);
							OLED_Refresh();
					
						}
						else{ 
							OLED_Clear();
							OLED_ShowString(0, 0, "close", 16, 1);
							OLED_Refresh();
							led_info=0;
							// �� PB2 ����Ϊ�͵�ƽ
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
						//		Led_Set(LED_OFF);
							
						}
						
					}
					
					if (usb_json != NULL)
					{
						
						if(usb_json->type == cJSON_True) {
              //Led_Set(LED_ON);
						// �� PB2 ����Ϊ�ߵ�ƽ
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
							usb_info=1;
							
					
						}
						else{ 
							// �� PB2 ����Ϊ�͵�ƽ
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
						//		Led_Set(LED_OFF);
							usb_info=0;
						}
						
					}
					
				 if (ele_json != NULL)
					{
							HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);//�ı��ƽ
							ele = (float)ele_json->valuedouble;
							printf("Received ele value: %.2f\n", ele);
							
					}

					
					cJSON_Delete(row_json);
				
				data_ptr = strstr(cmdid_topic, "request/");									//����cmdid
				if(data_ptr)
				{
					char topic_buf[80], cmdid[40];
					
					data_ptr = strchr(data_ptr, '/');
					data_ptr++;
					
					memcpy(cmdid, data_ptr, 36);											//����cmdid
					cmdid[36] = 0;
					
					snprintf(topic_buf, sizeof(topic_buf), "$sys/%s/%s/cmd/response/%s",
															PROID, DEVICE_NAME, cmdid);
					OneNET_Publish(topic_buf, "ojbk");										//�ظ�����
				}
			}
			
		case MQTT_PKT_PUBACK:														//����Publish��Ϣ��ƽ̨�ظ���Ack
		
			if(MQTT_UnPacketPublishAck(cmd) == 0)
				printf( "Tips:	MQTT Publish Send OK\r\n");
			
		break;
		
		case MQTT_PKT_SUBACK:																//����Subscribe��Ϣ��Ack
		
			if(MQTT_UnPacketSubscribe(cmd) == 0)
				printf( "Tips:	MQTT Subscribe OK\r\n");
			else
				printf( "Tips:	MQTT Subscribe Err\r\n");
		
		break;
		
		default:
			result = -1;
		break;
	}
	
	ESP8266_Clear();									//��ջ���
	
	if(result == -1)
		return;
	
//	dataPtr = strchr(req_payload, ':');					//����':'

//	if(dataPtr != NULL && result != -1)					//����ҵ���
//	{
//		dataPtr++;
//		
//		while(*dataPtr >= '0' && *dataPtr <= '9')		//�ж��Ƿ����·��������������
//		{
//			numBuf[num++] = *dataPtr++;
//		}
//		numBuf[num] = 0;
//		
//		num = atoi((const char *)numBuf);				//תΪ��ֵ��ʽ
//	}

	if(type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		MQTT_FreeBuffer(cmdid_topic);
		MQTT_FreeBuffer(req_payload);
	}

}
