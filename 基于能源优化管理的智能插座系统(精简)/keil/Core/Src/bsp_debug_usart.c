///**
//  ******************************************************************************
//  * @file    bsp_debug_usart.c
//  * @author  fire
//  * @version V1.0
//  * @date    2016-xx-xx
//  * @brief   使用串口1，重定向c库printf函数到usart端口，中断接收模式
//  ******************************************************************************
//  * @attention
//  *
//  * 实验平台:野火 STM32 F103 开发板  
//  * 论坛    :http://www.firebbs.cn
//  * 淘宝    :http://firestm32.taobao.com
//  *
//  ******************************************************************************
//  */ 
//  
//#include "bsp_debug_usart.h"

//UART_HandleTypeDef UartHandle;

//char RxBuff[BUFFSIZE];
//uint32_t Rxlen;
//uint8_t  Rxflag;

////extern uint8_t ucTemp;  

// /**
//  * @brief  DEBUG_USART GPIO 配置,工作模式配置。115200 8-N-1
//  * @param  无
//  * @retval 无
//  */  
//void DEBUG_USART_Config(void)
//{ 
//  
//  UartHandle.Instance          = DEBUG_USART;
//  
//  UartHandle.Init.BaudRate     = DEBUG_USART_BAUDRATE;
//  UartHandle.Init.WordLength   = UART_WORDLENGTH_8B;
//  UartHandle.Init.StopBits     = UART_STOPBITS_1;
//  UartHandle.Init.Parity       = UART_PARITY_NONE;
//  UartHandle.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
//  UartHandle.Init.Mode         = UART_MODE_TX_RX;
//  
//  HAL_UART_Init(&UartHandle);
//    
// /*使能串口接收断 */
//  __HAL_UART_ENABLE_IT(&UartHandle,UART_IT_RXNE); 
//	
//	/*使能串口空闲中断 */
//  __HAL_UART_ENABLE_IT(&UartHandle,UART_IT_IDLE);  	
//	__HAL_UART_CLEAR_IDLEFLAG(&UartHandle);
//}


///**
//  * @brief UART MSP 初始化 
//  * @param huart: UART handle
//  * @retval 无
//  */
///*
//void HAL_UART_MspInit(UART_HandleTypeDef *huart)
//{  
//  GPIO_InitTypeDef  GPIO_InitStruct;
//  
//  DEBUG_USART_CLK_ENABLE();
//	
//	DEBUG_USART_RX_GPIO_CLK_ENABLE();
//  DEBUG_USART_TX_GPIO_CLK_ENABLE();

//}
//*/
///*****************  发送字符串 **********************/
//void Usart_SendString(uint8_t *str)
//{
//	unsigned int k=0;
//  do 
//  {
//      HAL_UART_Transmit(&UartHandle,(uint8_t *)(str + k) ,1,1000);
//      k++;
//  } while(*(str + k)!='\0');
//}

////重定向c库函数printf到串口DEBUG_USART，重定向后可使用printf函数
//int fputc(int ch, FILE *f)
//{
//	/* 发送一个字节数据到串口DEBUG_USART */
//	HAL_UART_Transmit(&UartHandle, (uint8_t *)&ch, 1, 1000);	
//	
//	return (ch);
//}

////重定向c库函数scanf到串口DEBUG_USART，重写向后可使用scanf、getchar等函数
//int fgetc(FILE *f)
//{		
//	int ch;
//	HAL_UART_Receive(&UartHandle, (uint8_t *)&ch, 1, 1000);	
//	return (ch);
//}

///*********************************************END OF FILE**********************/
