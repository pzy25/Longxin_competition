/**
  ******************************************************************************
  * @file    WIZnet MDK5 Project Template 
  * @author  WIZnet Software Team
  * @version V1.0.0
  * @date    2018-09-25
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2018 WIZnet H.K. Ltd.</center></h2>
  ******************************************************************************
  */  

/* Includes ------------------------------------------------------------------*/
#include "spi.h"
#include "usart.h"
#include "delay.h"

/**
  * @brief  配置指定SPI的引脚
  * @retval None
  */
void spi_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStrucuture;
	
	
	//使能SPI2和GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	
	//设置SPI2引脚：SCK，MISO，MOSI
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	
	
	
	//设置芯片片选信号
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	
  //RST
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_8); 
	
	//设置SPI引脚
	SPI_InitStrucuture.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStrucuture.SPI_Mode = SPI_Mode_Master;
	SPI_InitStrucuture.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStrucuture.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStrucuture.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStrucuture.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStrucuture.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
	SPI_InitStrucuture.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStrucuture.SPI_CRCPolynomial = 7;
	
	SPI_Init(SPI2,&SPI_InitStrucuture);
	SPI_Cmd(SPI2,ENABLE);
	
	SPI_I2S_ClearFlag(SPI2,SPI_I2S_FLAG_TXE);
	SPI_I2S_ClearFlag(SPI2,SPI_I2S_FLAG_RXNE);
	
}




/**
  * @brief  写1字节数据到SPI总线
  * @param  TxData 写到总线的数据
  * @retval None
  */
void SPI_WriteByte(uint8_t TxData)
{	
	#if 1
	while((SPI2->SR&SPI_I2S_FLAG_TXE)==(uint16_t)RESET);	//等待发送区空		  
	SPI2->DR=TxData;	 	  									              //发送一个byte 
	while((SPI2->SR&SPI_I2S_FLAG_RXNE)==(uint16_t)RESET); //等待接收完一个byte  
	SPI2->DR;	
	#else
	while((SPI1->SR&SPI_I2S_FLAG_TXE)==(uint16_t)RESET);	//等待发送区空		  
	SPI1->DR=TxData;	 	  									//发送一个byte 
	while((SPI1->SR&SPI_I2S_FLAG_RXNE)==(uint16_t)RESET); //等待接收完一个byte  
	SPI1->DR;	
	#endif	
}
/**
  * @brief  从SPI总线读取1字节数据
  * @retval 读到的数据
  */
uint8_t SPI_ReadByte(void)
{	
	#if 1
	while((SPI2->SR&SPI_I2S_FLAG_TXE)==(uint16_t)RESET);	//等待发送区空			  
	SPI2->DR=0xFF;	 	  										//发送一个空数据产生输入数据的时钟 
	while((SPI2->SR&SPI_I2S_FLAG_RXNE)==(uint16_t)RESET); //等待接收完一个byte  
	return SPI2->DR;	
  #else 
	while((SPI1->SR&SPI_I2S_FLAG_TXE)==(uint16_t)RESET);	//等待发送区空			  
	SPI1->DR=0xFF;	 	  										//发送一个空数据产生输入数据的时钟 
	while((SPI1->SR&SPI_I2S_FLAG_RXNE)==(uint16_t)RESET); //等待接收完一个byte  	
	return SPI1->DR;	
  #endif	
}
/**
  * @brief  进入临界区
  * @retval None
  */
void SPI_CrisEnter(void)
{
	__set_PRIMASK(1);
}
/**
  * @brief  退出临界区
  * @retval None
  */
void SPI_CrisExit(void)
{
	__set_PRIMASK(0);
}

/**
  * @brief  片选信号输出低电平
  * @retval None
  */
void SPI_CS_Select(void)
{
	#if 1
	GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	#else 
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
	#endif
}
/**
  * @brief  片选信号输出高电平
  * @retval None
  */
void SPI_CS_Deselect(void)
{
	#if 1
	GPIO_SetBits(GPIOB,GPIO_Pin_12);
	#else 
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
	#endif
}


//W5500复位													
void W5500_RESET(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_8);
	delay_us(2);
	GPIO_SetBits(GPIOB,GPIO_Pin_8);
	delay_ms(1000);
}

/*********************************END OF FILE**********************************/

