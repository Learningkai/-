/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
		//简介
	******************************************************************************
	PB2（左二）为高电平输出
	
	
	//oled
	PB8（右3）-SCL
	PB9（右5）—SDA
	PB12（右16）-RES
	PB11（左1）-DC
	
	//USART
	PB6（右34）-RX
	PB7(右6)	-TX
	
	//I2C
	PA9（右19）-SCL
	PA10（右15）-SDA
	
	//I2C 3
	PA7(右15)-SCL
	PC1(左30)-SDA
	
	//电流传感器
	PC0(左28)-input
	
	
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "memorymap.h"
#include "rf.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "pn532_hsu.h"  // 自己定义的PN532头文件
#include <stdio.h>
#include <string.h>
//#include "bsp_debug_usart.h"  // 串口版极支持包

#include "pn532.h"
#include "oled.h"

#include "pn532_stm32f1.h"//pn532文件

#include "math.h" // 添加 math.h 头文件

//网络协议层
#include "onenet.h"
//网络设备驱动
#include "esp8266.h"
//C库
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef uint8_t u8;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ESP8266_ONENET_INFO		"AT+CIPSTART=\"TCP\",\"mqtts.heclouds.com\",1883\r\n"


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
PN532 pn532;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */
void OLED_DisplayWorld(unsigned char* world);
	
extern uint8_t Uart1_RxData;	//接收中断缓冲


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 温度示波器绘制函数
float cur=8.00;
float ele=5000.00 ;
// 定义一个全局布尔变量
int led_info = 1;  // 初始化为false
int usb_info = 1;  // 初始化为false
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t buff[255];
  uint8_t uid[MIFARE_UID_MAX_LENGTH];
  int32_t uid_len = 0;
	// 管理员 UID 数组
    uint8_t target_uid_admin[MIFARE_UID_MAX_LENGTH] = {0xf0, 0x9f, 0x53, 0xaf};
	//成员1 uid 数组
		uint8_t target_uid_1[MIFARE_UID_MAX_LENGTH] = {0xa3, 0x41, 0x39, 0x0d};
		//oled显示参数
		char buffer[20];
		//esp8266
		unsigned short timeCount = 0;	//发送间隔变量
	
		unsigned char *dataPtr = NULL;
		//esp8266end
		
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USB_PCD_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  MX_I2C3_Init();
  MX_RF_Init();
  /* USER CODE BEGIN 2 */
	/***************OLED*****************/
  OLED_Init();
  OLED_ColorTurn(0);
  OLED_DisplayTurn(0);
  
  OLED_Clear();
  OLED_ShowString(0, 0, (u8*)"WORLD", 16, 1);
  OLED_Refresh();
	HAL_Delay(200);
	


	

	  /*初始化USART 配置模式为 115200 8-N-1，中断接收*/
  //DEBUG_USART_Config();
  
	/*调用printf函数，因为重定向了fputc，printf的内容会输出到串口*/
	//printf("欢迎使用野火开发板\n");	

	/*自定义函数方式*/
//	Usart_SendString( (uint8_t *)"串口接收不定长字节，请查看程序内注释\r\n" );
	
	/*开启第一次接收状态*/
//	HAL_UART_Receive_IT(&UartHandle,(uint8_t*)RxBuff,BUFFSIZE);
	
	
  /*ADC的内容*/
	char msg[50];
  uint32_t adcValue;
	
	
	/*esp8266*/
		HAL_UART_Receive_IT(&huart1,(uint8_t *)&Uart1_RxData, 1);//开启串口中断
	
	/* esp8266连接wifi+连接Onenet */
	HAL_Delay(2000);
	ESP8266_Init();					//初始化ESP8266，连接wifi
	HAL_Delay(2000);
	printf("ConnectMQTT");
	
	while(ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
		HAL_Delay(500);
	printf( "SUCCESS\n");
	
	while(OneNet_DevLink())			//接入OneNET
		HAL_Delay(500);

	OneNET_Subscribe();   //ONEnet订阅
	OLED_Clear();
  OLED_ShowString(0, 0, (u8*)"success", 16, 1);
  OLED_Refresh();
  /*esp8266end*/
	/***************PN532*****************/
	
	printf("Hello!\r\n");
  PN532 pn532;
  PN532_I2C_Init(&pn532);
  PN532_GetFirmwareVersion(&pn532, buff);
  if (PN532_GetFirmwareVersion(&pn532, buff) == PN532_STATUS_OK) {
    printf("Found PN532 with firmware version: 1.0\r\n");
  } else {
    return -1;
  }
  PN532_SamConfiguration(&pn532);
 // OLED_DisplayWorld("Waiting for RFID/NFC card...\r\n");
	/***************PN532END*****************/
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
		
while(1)
 {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	 /****************ADC电流传感器**************/
		 HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
            adcValue = HAL_ADC_GetValue(&hadc1);
        }
        HAL_ADC_Stop(&hadc1);

		float voltage = ((float)adcValue / 4095) * 3.3f; // 假设3.3V参考电压
    float current = (voltage - 2.44f) / 0.100f; // 中间点电压为2.5V——current为实际电流

    //snprintf(msg, sizeof(msg), "\nADC Value: %lu\n, Voltage: %.2f V\n，Current:%.2f A \r\n", adcValue, voltage, current);
		
		

			// 清除需要刷新的部分
			OLED_ClearArea(65, 0, 128, 16); // 清除电流显示区域
			OLED_ClearArea(65, 16, 128, 36); // 清除状态显示区域
			OLED_ClearArea(65, 32, 128, 36); // 清除USB显示区域
			OLED_ClearArea(65, 48, 128, 56); // 清除电压显示区域

			// 显示当前值
			OLED_ShowString(0, 0, (u8*)"current:", 16, 1);
			snprintf(buffer, sizeof(buffer), "%.2f", current); // 将浮点数转换为字符串，保留两位小数
			OLED_ShowString(65, 0, (u8*)buffer, 16, 1);
			

			// 显示开关值
			OLED_ShowString(0, 16, (u8*)"Status:", 16, 1);
			if (led_info == 1) {
					snprintf(buffer, sizeof(buffer), "Open");
			} else {
					snprintf(buffer, sizeof(buffer), "Close");
			}
			OLED_ShowString(65, 16, (u8*)buffer, 16, 1);
			
			OLED_ShowString(0, 32, (u8*)"USB:", 16, 1);
			if (usb_info == 1) {
					snprintf(buffer, sizeof(buffer), "Open");
			} else {
					snprintf(buffer, sizeof(buffer), "Close");
			}
			OLED_ShowString(65, 32, (u8*)buffer, 16, 1);
			// 显示电压值
			OLED_ShowString(0, 48, (u8*)"voltage", 16, 1);
			snprintf(buffer, sizeof(buffer), "%.2f", voltage); // 将浮点数转换为字符串，保留两位小数
			OLED_ShowString(65, 48,(u8*)buffer, 16, 1);

			// 刷新 OLED 显示屏
			OLED_Refresh();

		
		cur=current;
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);


        HAL_Delay(50); // 延时1秒
				
				
		/***************PN532******************/
	  // Check if a card is available to read
    uid_len = PN532_ReadPassiveTarget(&pn532, uid, PN532_MIFARE_ISO14443A, 100);
//    if (uid_len == PN532_STATUS_ERROR) {
//      printf(".");
//    } else {
//      printf("Found card with UID: ");
//      for (uint8_t i = 0; i < uid_len; i++) {
//        printf("%02x ", uid[i]);
//      }
//      printf("\r\n");
//    }
		/***************PN532******************/
		  // 比较读取卡密钥与密钥
    if (!(uid_len == PN532_STATUS_ERROR)&&(memcmp(uid, target_uid_admin, MIFARE_UID_MAX_LENGTH) == 0)) {
        printf("HSK(Admin)Welcome\n");
				OLED_Clear();
				OLED_ShowString(0, 0,(u8*) "HSK(Admin)", 16, 1);
				HAL_Delay(1000);
				OLED_Refresh();

				HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_2);//改变电平
    } 
    
		else if (!(uid_len == PN532_STATUS_ERROR)&&(memcmp(uid, target_uid_1, MIFARE_UID_MAX_LENGTH) == 0)) {
        printf("yonghu_1 Welcome\n");
				OLED_Clear();
				OLED_ShowString(0, 0, (u8*)"yonghu_1", 16, 1);
			  
				OLED_Refresh();
        HAL_Delay(1000);
				HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_3);//改变电平
    } 
//		else {
//        printf("Not Admin\n");
//				OLED_Clear();
//				OLED_ShowString(0, 0, "Not Admin", 16, 1);
//				OLED_Refresh();
//    }
		
		HAL_Delay(50);
	
		/*esp8266*/
		/********发送**********/
			//UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
		 if(++timeCount >= 5)									//发送间隔5s
		{
			//UsartPrintf(USART_DEBUG, "OneNet_SendData\r\n");
		  OneNet_SendData();									//发送数据
			
			timeCount = 0;
			ESP8266_Clear();
		}
		/********接收**********/
		dataPtr = ESP8266_GetIPD(100);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
		HAL_Delay(100);		
		
		//判断onenet接收消息
//		if(led_info==0 )		
//		{
//			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
//		}
//		else{
//		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
//		}
		
		//判断nfc开关高低电平
		
				
				/******************同步当前开关状态*******************/
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_SET) {
        // PB2 是高电平
				led_info=1;
    } else {
        // PB2 是低电平
			  led_info=0;
    }
		
		if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == GPIO_PIN_SET) {
        // PA3 是高电平
				usb_info=1;
    } else {
        // PA3 是低电平
			  usb_info=0;
    }
		
		
		if(current >= 5.0)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
		}
		/******************同步当前开关状态END*******************/
		/*esp8266end*/
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE
                              |RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 24;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV3;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS|RCC_PERIPHCLK_RFWAKEUP
                              |RCC_PERIPHCLK_USB|RCC_PERIPHCLK_ADC;
  PeriphClkInitStruct.PLLSAI1.PLLN = 24;
  PeriphClkInitStruct.PLLSAI1.PLLP = RCC_PLLP_DIV2;
  PeriphClkInitStruct.PLLSAI1.PLLQ = RCC_PLLQ_DIV2;
  PeriphClkInitStruct.PLLSAI1.PLLR = RCC_PLLR_DIV2;
  PeriphClkInitStruct.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_USBCLK|RCC_PLLSAI1_ADCCLK;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLLSAI1;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInitStruct.RFWakeUpClockSelection = RCC_RFWKPCLKSOURCE_HSE_DIV1024;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE0;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/* USER CODE BEGIN 4 */

void OLED_DisplayWorld(unsigned char* world) {
    OLED_Clear();
    OLED_ShowString(0, 0, world, 16, 1);
    OLED_Refresh();
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
