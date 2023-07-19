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

int i = 8; //�˴η������ݵĳ���
int j = 0;


float Pf_data1;
int reveive_numbe = 0;
vu16 USART3_RX_STA = 0;
//�жϷ�����    ֮ǰ���ô��ڷ������ݵĳ����Լ����ĺ���
void USART3_IRQHandler(void)
{
	

	char Res;
	
	
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		Res = USART_ReceiveData(USART3);
		if (USART3_RX_STA < USART3_MAX_RECV_LEN) //���յ��ַ���
		{
			Rx_Buffer1[USART3_RX_STA] = Res;
			
			USART3_RX_STA++;
		}
	}

	if (USART_GetITStatus(USART3, USART_IT_IDLE) != SET)
	{
		
		receive_finished = 1;   		                   // ����Զ��ķ��͵��ص㰡   ���IF���õ��ǿ����жϣ����������ݺ��Զ�����
		USART_ClearITPendingBit(USART3, USART_IT_IDLE);
	}
	
	USART3->SR;
	USART3->DR;
	
	
}
//����Ǽ���CRC����           ��ģ���������ϴ��ĺ���
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
//����Ǻ˶�CRCУ�麯��       ��ģ���������ϴ��ĺ���   ���ÿ�
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

//������ģ��ĺ���       ��ģ���������ϴ��ĺ���    �ص㿴forѭ��
void read_data(int read_enable)
{
	union	crcdata
	{
		unsigned int word16;
		unsigned char	byte[2];
	}crcnow;

	if (read_enable == 1)	// ��ʱ�䳭��ģ�飬������� 1 ����(������)
	{
		read_enable = 0;
		Tx_Buffer1[0] = Read_ID;	//ģ��� ID �ţ�Ĭ�� ID Ϊ 0x01 
		Tx_Buffer1[1] = 0x03;
		Tx_Buffer1[2] = 0x00;
		Tx_Buffer1[3] = 0x48;
		Tx_Buffer1[4] = 0x00;
		Tx_Buffer1[5] = 0x08;
		crcnow.word16 = chkcrc(Tx_Buffer1, 6);
		Tx_Buffer1[6] = crcnow.byte[1];	//CRC Ч����ֽ���ǰ
		Tx_Buffer1[7] = crcnow.byte[0];
	}

	for (j = 0; j < i; j++)							//ѭ����������01 03 00 48 00 08 C4 1A recv_data.
	{
		while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET); //������ ѭ������,ֱ���������   
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
	if (receive_finished == 1)	//�������
	{

		reveive_numbe = USART3_RX_STA; //���Ҳ�Ǻ���д��
		receive_finished = 0;

		if ((Rx_Buffer1[0] == 0x01) || (Rx_Buffer1[0] == 0x00))	//ȷ�� ID ��ȷ
		{			
			printf("\r\nȷ�� ID ��ȷ\r\n");//����һ����ӡ���ܵ�ѹֵ
			crcnow.word16 = chkcrc(Rx_Buffer1, reveive_numbe - 2);	//reveive_numbe �ǽ��������ܳ���
			if ((crcnow.byte[0] == Rx_Buffer1[reveive_numbe - 1]) && (crcnow.byte[1] == Rx_Buffer1[reveive_numbe - 2]))//ȷ�� CRC У����ȷ
			{
				
				for(i = 0;i<24;i++)
				{
					usRegHoldingBuf[i+2] = Rx_Buffer1[i+3];
				}

				printf("\r\n%d\r\n", USART3_RX_STA); //������Լ��ӵģ���������������Լ�����
				tmp.Voltage_data = (((unsigned long)(Rx_Buffer1[3])) << 24) | (((unsigned long)(Rx_Buffer1[4])) << 16) | (((unsigned	long)(Rx_Buffer1[5])) << 8) | Rx_Buffer1[6];
				printf("\r\n��ѹֵ:%lu\r\n", tmp.Voltage_data);//����һ����ӡ��ѹֵ  �Լ��ӵ�
				
				

				tmp.Current_data = (((unsigned long)(Rx_Buffer1[7])) << 24) | (((unsigned long)(Rx_Buffer1[8])) << 16) | (((unsigned	long)(Rx_Buffer1[9])) << 8) | Rx_Buffer1[10];
				printf("����:%lu\r\n", tmp.Current_data);//����һ����ӡ����

				

//				tmp.Power_data = (((unsigned	long)(Rx_Buffer1[11])) << 24) | (((unsigned long)(Rx_Buffer1[12])) << 16) | (((unsigned	long)(Rx_Buffer1[13])) << 8) | Rx_Buffer1[14];
//				printf("����:%lu\r\n", tmp.Power_data);//����һ����ӡ����

//				tmp.Energy_data = (((unsigned	long)(Rx_Buffer1[15])) << 24) | (((unsigned long)(Rx_Buffer1[16])) << 16) | (((unsigned	long)(Rx_Buffer1[17])) << 8) | Rx_Buffer1[18];
//				printf("����:%lu\r\n", tmp.Energy_data);//����һ����ӡ����

//				tmp.Pf_data = (((unsigned	long)(Rx_Buffer1[19])) << 24) | (((unsigned long)(Rx_Buffer1[20])) << 16) | (((unsigned	long)(Rx_Buffer1[21])) << 8) | Rx_Buffer1[22];
//				Pf_data1 = (float)(tmp.Pf_data * 0.001);
//				printf("��������:%.3lf\r\n", Pf_data1); //����һ����ӡ���ܹ�������


//				tmp.CO2_data = (((unsigned long)(Rx_Buffer1[23])) << 24) | (((unsigned long)(Rx_Buffer1[24])) << 16) | (((unsigned	long)(Rx_Buffer1[25])) << 8) | Rx_Buffer1[26];
//				printf("������̼:%lu\r\n", tmp.CO2_data);//����һ����ӡ���ܶ�����̼


				
				
//				HZ = (((unsigned long)(Rx_Buffer1[31])) << 24) | (((unsigned long)(Rx_Buffer1[32])) << 16) | (((unsigned	long)(Rx_Buffer1[33])) << 8) | Rx_Buffer1[34];
//				HZ1 = HZ * 0.01;
//				printf("����:%.2lfHZ\r\n", HZ1);//����һ����ӡ���ܵ��ཻ����ĺ���
				USART3_RX_STA = 0;
				
				
				
				
//	      err = xQueueSend(SendMsg_Queue,&Rx_Buffer1,10);
//				if(err == errQUEUE_FULL)
//				{
//					printf("����SendMsg_Queue����������ʧ��");
//				}

			}
			// else printf("// CRC У�����\r\n");//����һ����ӡ���ܵ�ѹֵ
		}

	}
	// else printf("\r\nreceive_finished��־λΪ0\r\n");
		return tmp;
}
void wifi_null(void) //���USART3_RX_BUF�ڴ�
{
	for (i = 0; i < 40; i++)
		Rx_Buffer1[i] = 0;
}


//��ʼ��IO ����3
//pclk1:PCLK1ʱ��Ƶ��(Mhz)
//bound:������
void usart3_init(u32 bound)
{

	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// GPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); //����3ʱ��ʹ��
	USART_DeInit(USART3);  //��λ����3
		 //USART3_TX   PB10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure); //��ʼ��PB10

	//USART3_RX	  PB11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure); //��ʼ��PB11

	USART_InitStructure.USART_BaudRate = bound;//������һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART3, &USART_InitStructure); //��ʼ������	3


	//�����ж����ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
	wifi_null();
	//ʹ�ܽ����ж�
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//���������ж�
	USART_Cmd(USART3, ENABLE); //ʹ�ܴ���
}
