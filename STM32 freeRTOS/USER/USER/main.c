#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sys.h"
#include "IM1281B.h"
#include "DHT11.h"
#include "w5500.h"
#include "socket.h"
#include "wizchip_conf.h"
#include "spi.h"
#include "string.h"
#include "jdq.h"
#include "queue.h"
#include "mb.h"
#include "mbport.h"

uint8_t ucTCPRequestFrame[MB_TCP_BUF_SIZE];
uint16_t ucTCPRequestLen;
uint8_t  ucTCPResponseFrame[MB_TCP_BUF_SIZE];
uint16_t ucTCPResponslen;
uint8_t bFrameSent = FALSE;

extern uint16_t usRegHoldingBuf[REG_HOLDING_NREGS];
extern uint8_t ucRegCoilsBuf[REG_COILS_SIZE];

//�˿ںŶ���
#define TCPSEVER_PORT     504

//��ʼ����
#define START_TASK_PRIO			1            
#define START_STK_SIZE			128
TaskHandle_t StartTask_Handler;
void start_task(void *pvParameters);

//W5500 TCP����������
#define TCPSEVER_TASK_PRIO  2
#define TCPSEVER_STK_SIZE   256
TaskHandle_t TcpSeverTask_Handler;
void tcpsever_task(void* pvParameters);

//DHT11 ��ʪ������
#define DHT11_TASK_PRIO     3
#define DHT11_STK_SIZE      128
TaskHandle_t DHT11Task_Handler;
void dht11_task(void *pvParameters);

//�̵�������
#define JDQ_TASK_PRIO  4
#define JDQ_STK_SIZE   128
TaskHandle_t JdqTask_Handler;
void jdq_task(void* pvParameters);

//����������
#define IM1281B_TASK_PRIO 5
#define IM1281B_STK_SIZE 256
TaskHandle_t IM1281BTask_Handler;
void im1281b_task(void* pvParameters);

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);     //�����ĸ����ȼ�
	delay_init();                                       //��ʱ����
	uart_init(115200);                                  //����1��ʼ�� ����
	usart3_init(4800);                                  //����3��ʼ�� �����Ʋ�����4800
	spi_init();                                         //spi��ʼ��
  relay_init();                                       //�̵�����ʼ��
	W5500_ChipInit();                                   //W5500��ʼ��
	
//������ʼ����
	xTaskCreate((TaskFunction_t		)start_task,          //������ں���
							(const char*			)"start_task",				//ָ����������
							(uint16_t					)START_STK_SIZE,			//ָ�������ջ��С
							(void*						)NULL,								//ָ���������
							(UBaseType_t			)START_TASK_PRIO,			//ָ���������ȼ�
							(TaskHandle_t*		)&StartTask_Handler);	//ָ����ȡ��������ָ��

	vTaskStartScheduler();                              //�����������
}

void start_task(void *pvParameters)
{
	taskENTER_CRITICAL();                               //�����ٽ�����ȷ���������	

//����W5500 TCP����������1
	xTaskCreate((TaskFunction_t		)tcpsever_task,
							(const char*			)"tcpsever_task",
							(uint16_t					)TCPSEVER_STK_SIZE,
							(void*						)NULL,
							(UBaseType_t			)TCPSEVER_TASK_PRIO,
							(TaskHandle_t*		)&TcpSeverTask_Handler);	
	
//����DHT11 ��ʪ������
	xTaskCreate((TaskFunction_t		)dht11_task,
							(const char*			)"dht11_task",
							(uint16_t					)DHT11_STK_SIZE,
							(void*						)NULL,
							(UBaseType_t			)DHT11_TASK_PRIO,
							(TaskHandle_t*		)&DHT11Task_Handler);	

//�����̵�������							
	xTaskCreate((TaskFunction_t		)jdq_task,
							(const char*			)"jdq_task",
							(uint16_t					)JDQ_STK_SIZE,
							(void*						)NULL,
							(UBaseType_t			)JDQ_TASK_PRIO,
							(TaskHandle_t*		)&IM1281BTask_Handler);								
							
//��������������					
	xTaskCreate((TaskFunction_t		)im1281b_task,
							(const char*			)"im1281b_task",
							(uint16_t					)IM1281B_STK_SIZE,
							(void*						)NULL,
							(UBaseType_t			)IM1281B_TASK_PRIO,
							(TaskHandle_t*		)&IM1281BTask_Handler);														
	
	vTaskDelete(StartTask_Handler);                           //ɾ���������ͷ���Դ
	taskEXIT_CRITICAL();                                      //�˳��ٽ����������ж�
}


extern unsigned int rec_data[4];

void dht11_task(void* pvParameters)
{
	while(1)
	{
	
		DHT11_REC_Data();
		printf("�¶ȣ�%d\r\n",rec_data[2]);
		printf("ʪ�ȣ�%d\r\n",rec_data[0]);	
		usRegHoldingBuf[0] = rec_data[2];
		usRegHoldingBuf[1] = rec_data[0];
		vTaskDelay(500);	
	}
}

void tcpsever_task(void* pvParameters)
{
	eMBTCPInit(TCPSEVER_PORT);                       //��ʼ��������
  eMBEnable();	
	while(1)
	{
		switch(getSn_SR(0))
		{		
			case SOCK_CLOSED:                                       //socket���ڹر�״̬			
				printf("Tcp Sever Start!\r\n");
				socket(0,Sn_MR_TCP,TCPSEVER_PORT,Sn_MR_ND);			
				printf("socket 0 open success.\r\n");
				break;		
				case SOCK_INIT:                                        //socket���ڳ�ʼ��״̬   			
				listen(0);                                             //�������ģʽ
				break;
				case  SOCK_ESTABLISHED:   
					if(getSn_IR(0) & Sn_IR_CON)
						{
								printf("socket 0: connected.\r\n");
								setSn_IR(0,Sn_IR_CON);                         //��ʾ�����Ѿ�����
						}			
			        //�������Է��������ݵĴ�С
					ucTCPRequestLen = getSn_RX_RSR(0);												
					if(ucTCPRequestLen>0)
						{
							//�������Է�����������
							memset(ucTCPRequestFrame,0,MB_TCP_BUF_SIZE);     //���TCP�������ݻ�����
							recv(0,ucTCPRequestFrame,ucTCPRequestLen);       //��������
							xMBPortEventPost(EV_FRAME_RECEIVED);             //��������֡��ִ�й�����
							eMBPoll();                                       //��ѯ����
							eMBPoll();
							if(bFrameSent)                                   //�ж���Ӧ֡�Ƿ���
							{
								bFrameSent = FALSE;
								send(0,ucTCPResponseFrame,ucTCPResponslen);		 //��������
							}							
						}
				break;
				case SOCK_CLOSE_WAIT: //socket���ڵȴ��ر�״̬
					close(0);
				break;
		}
			vTaskDelay(10);
	}
}

void jdq_task(void* pvParameters)
{
	while(1)
	{
		if(ucRegCoilsBuf[0] == 0)
		{
			printf("�̵����ر�.\r\n");
			relay_off();
		}
		else
		{	
			printf("�̵�������.\r\n");
		  relay_on();
		}
		vTaskDelay(200);
	}
}

extern int receive_finished;

void im1281b_task(void* pvParameters)
{
	int read_enable = 1;
	read_data(read_enable);	
		while(1)
		{
			if(receive_finished)
			{
				Analysis_data();
				receive_finished = 0;
				read_data(read_enable);
				vTaskDelay(5000);
		}
	}					
}

