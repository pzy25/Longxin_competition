#include "IM1281B.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "mbport.h"

extern uint16_t usRegHoldingBuf[REG_HOLDING_NREGS];

u8 USART3_RX_BUF[USART3_MAX_RECV_LEN];
u8 USART3_TX_BUF[USART3_MAX_SEND_LEN];
unsigned char Read_ID = 0x01;
unsigned char Tx_Buffer1[8];
unsigned char Rx_Buffer1[40];
unsigned char Rx_Buffer[40];
int receive_finished;

int i = 8; //此次发送数据的长度
int j = 0;


float Pf_data1;
int reveive_numbe = 0;
vu16 USART3_RX_STA = 0;
//中断服务函数    之前找用串口发送数据的程序自己带的函数
void USART3_IRQHandler(void)
{
	

	char Res;
	
	
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		Res = USART_ReceiveData(USART3);
		if (USART3_RX_STA < USART3_MAX_RECV_LEN) //接收的字符串
		{
			Rx_Buffer1[USART3_RX_STA] = Res;
			
			USART3_RX_STA++;
		}
	}

	if (USART_GetITStatus(USART3, USART_IT_IDLE) != SET)
	{
		
		receive_finished = 1;   		                   // 这个自动的发送的重点啊   这个IF利用的是空闲中断，在收完数据后自动发送
		USART_ClearITPendingBit(USART3, USART_IT_IDLE);
	}
	
	USART3->SR;
	USART3->DR;
	
	
}
//这个是计算CRC函数           买模块自身资料带的函数
unsigned int calccrc(unsigned char crcbuf, unsigned int crc)
{
	unsigned char i;
	unsigned char chk;
	crc = crc ^ crcbuf; for (i = 0; i < 8; i++)
	{
		chk = (unsigned char)(crc & 1);
		crc = crc >> 1;
		crc = crc & 0x7fff;
		if (chk == 1) crc = crc ^ 0xa001;
		crc = crc & 0xffff;
	}
	return crc;
}
//这个是核对CRC校验函数       买模块自身资料带的函数   不用看
unsigned int chkcrc(unsigned char* buf, unsigned char len)
{
	unsigned char hi, lo;
	unsigned int i;
	unsigned int crc;
	crc = 0xFFFF;
	for (i = 0; i < len; i++)
	{
		crc = calccrc(*buf, crc);
		buf++;
	}

	hi = (unsigned char)(crc % 256);
	lo = (unsigned char)(crc / 256);
	crc = (((unsigned int)(hi)) << 8) | lo;
	return crc;
}

//读电能模块的函数       买模块自身资料带的函数    重点看for循环
void read_data(int read_enable)
{
	union	crcdata
	{
		unsigned int word16;
		unsigned char	byte[2];
	}crcnow;

	if (read_enable == 1)	// 到时间抄读模块，抄读间隔 1 秒钟(或其他)
	{
		read_enable = 0;
		Tx_Buffer1[0] = Read_ID;	//模块的 ID 号，默认 ID 为 0x01 
		Tx_Buffer1[1] = 0x03;
		Tx_Buffer1[2] = 0x00;
		Tx_Buffer1[3] = 0x48;
		Tx_Buffer1[4] = 0x00;
		Tx_Buffer1[5] = 0x08;
		crcnow.word16 = chkcrc(Tx_Buffer1, 6);
		Tx_Buffer1[6] = crcnow.byte[1];	//CRC 效验低字节在前
		Tx_Buffer1[7] = crcnow.byte[0];
	}

	for (j = 0; j < i; j++)							//循环发送数据01 03 00 48 00 08 C4 1A recv_data.
	{
		while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET); //串口三 循环发送,直到发送完毕   
		USART_SendData(USART3, Tx_Buffer1[j]);

	}
	
}



DataArray Analysis_data(void)
{
	
	DataArray tmp;
	unsigned char i;
	
	union	crcdata
	{
		unsigned int word16;
		unsigned char	byte[2];
	}crcnow;
	if (receive_finished == 1)	//接收完成
	{

		reveive_numbe = USART3_RX_STA; //这个也是好像写的
		receive_finished = 0;

		if ((Rx_Buffer1[0] == 0x01) || (Rx_Buffer1[0] == 0x00))	//确认 ID 正确
		{			
			printf("\r\n确认 ID 正确\r\n");//串口一来打印接受电压值
			crcnow.word16 = chkcrc(Rx_Buffer1, reveive_numbe - 2);	//reveive_numbe 是接收数据总长度
			if ((crcnow.byte[0] == Rx_Buffer1[reveive_numbe - 1]) && (crcnow.byte[1] == Rx_Buffer1[reveive_numbe - 2]))//确认 CRC 校验正确
			{
				
				for(i = 0;i<24;i++)
				{
					usRegHoldingBuf[i+2] = Rx_Buffer1[i+3];
				}

				printf("\r\n%d\r\n", USART3_RX_STA); //这个是自己加的，，下面的是资料自己带的
				tmp.Voltage_data = (((unsigned long)(Rx_Buffer1[3])) << 24) | (((unsigned long)(Rx_Buffer1[4])) << 16) | (((unsigned	long)(Rx_Buffer1[5])) << 8) | Rx_Buffer1[6];
				printf("\r\n电压值:%lu\r\n", tmp.Voltage_data);//串口一来打印电压值  自己加的
				
				

				tmp.Current_data = (((unsigned long)(Rx_Buffer1[7])) << 24) | (((unsigned long)(Rx_Buffer1[8])) << 16) | (((unsigned	long)(Rx_Buffer1[9])) << 8) | Rx_Buffer1[10];
				printf("电流:%lu\r\n", tmp.Current_data);//串口一来打印电流

				

//				tmp.Power_data = (((unsigned	long)(Rx_Buffer1[11])) << 24) | (((unsigned long)(Rx_Buffer1[12])) << 16) | (((unsigned	long)(Rx_Buffer1[13])) << 8) | Rx_Buffer1[14];
//				printf("功率:%lu\r\n", tmp.Power_data);//串口一来打印功率

//				tmp.Energy_data = (((unsigned	long)(Rx_Buffer1[15])) << 24) | (((unsigned long)(Rx_Buffer1[16])) << 16) | (((unsigned	long)(Rx_Buffer1[17])) << 8) | Rx_Buffer1[18];
//				printf("电能:%lu\r\n", tmp.Energy_data);//串口一来打印电能

//				tmp.Pf_data = (((unsigned	long)(Rx_Buffer1[19])) << 24) | (((unsigned long)(Rx_Buffer1[20])) << 16) | (((unsigned	long)(Rx_Buffer1[21])) << 8) | Rx_Buffer1[22];
//				Pf_data1 = (float)(tmp.Pf_data * 0.001);
//				printf("功率因数:%.3lf\r\n", Pf_data1); //串口一来打印接受功率因数


//				tmp.CO2_data = (((unsigned long)(Rx_Buffer1[23])) << 24) | (((unsigned long)(Rx_Buffer1[24])) << 16) | (((unsigned	long)(Rx_Buffer1[25])) << 8) | Rx_Buffer1[26];
//				printf("二氧化碳:%lu\r\n", tmp.CO2_data);//串口一来打印接受二氧化碳


				
				
//				HZ = (((unsigned long)(Rx_Buffer1[31])) << 24) | (((unsigned long)(Rx_Buffer1[32])) << 16) | (((unsigned	long)(Rx_Buffer1[33])) << 8) | Rx_Buffer1[34];
//				HZ1 = HZ * 0.01;
//				printf("赫兹:%.2lfHZ\r\n", HZ1);//串口一来打印接受单相交流电的赫兹
				USART3_RX_STA = 0;
				
				
				
				
//	      err = xQueueSend(SendMsg_Queue,&Rx_Buffer1,10);
//				if(err == errQUEUE_FULL)
//				{
//					printf("队列SendMsg_Queue已满，发送失败");
//				}

			}
			// else printf("// CRC 校验错误\r\n");//串口一来打印接受电压值
		}

	}
	// else printf("\r\nreceive_finished标志位为0\r\n");
		return tmp;
}
void wifi_null(void) //清空USART3_RX_BUF内存
{
	for (i = 0; i < 40; i++)
		Rx_Buffer1[i] = 0;
}


//初始化IO 串口3
//pclk1:PCLK1时钟频率(Mhz)
//bound:波特率
void usart3_init(u32 bound)
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //串口3时钟使能
	USART_DeInit(USART3);  //复位串口3
		 //USART3_TX   PB10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PB10

	//USART3_RX	  PB11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PB11

	USART_InitStructure.USART_BaudRate = bound;//波特率一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART3, &USART_InitStructure); //初始化串口	3


	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
	wifi_null();
	//使能接收中断
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//开启空闲中断
	USART_Cmd(USART3, ENABLE); //使能串口
}
