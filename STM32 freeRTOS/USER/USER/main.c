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

//端口号定义
#define TCPSEVER_PORT     504

//开始任务
#define START_TASK_PRIO			1            
#define START_STK_SIZE			128
TaskHandle_t StartTask_Handler;
void start_task(void *pvParameters);

//W5500 TCP服务器任务
#define TCPSEVER_TASK_PRIO  2
#define TCPSEVER_STK_SIZE   256
TaskHandle_t TcpSeverTask_Handler;
void tcpsever_task(void* pvParameters);

//DHT11 温湿度任务
#define DHT11_TASK_PRIO     3
#define DHT11_STK_SIZE      128
TaskHandle_t DHT11Task_Handler;
void dht11_task(void *pvParameters);

//继电器任务
#define JDQ_TASK_PRIO  4
#define JDQ_STK_SIZE   128
TaskHandle_t JdqTask_Handler;
void jdq_task(void* pvParameters);

//电流计任务
#define IM1281B_TASK_PRIO 5
#define IM1281B_STK_SIZE 256
TaskHandle_t IM1281BTask_Handler;
void im1281b_task(void* pvParameters);

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);     //定义四个优先级
	delay_init();                                       //延时函数
	uart_init(115200);                                  //串口1初始化 调试
	usart3_init(4800);                                  //串口3初始化 电流计波特率4800
	spi_init();                                         //spi初始化
  relay_init();                                       //继电器初始化
	W5500_ChipInit();                                   //W5500初始化
	
//创建开始任务
	xTaskCreate((TaskFunction_t		)start_task,          //任务入口函数
							(const char*			)"start_task",				//指定任务名称
							(uint16_t					)START_STK_SIZE,			//指定任务堆栈大小
							(void*						)NULL,								//指定任务参数
							(UBaseType_t			)START_TASK_PRIO,			//指定任务优先级
							(TaskHandle_t*		)&StartTask_Handler);	//指定获取任务句柄的指针

	vTaskStartScheduler();                              //开启任务调度
}

void start_task(void *pvParameters)
{
	taskENTER_CRITICAL();                               //进入临界区，确保不被打断	

//创建W5500 TCP服务器任务1
	xTaskCreate((TaskFunction_t		)tcpsever_task,
							(const char*			)"tcpsever_task",
							(uint16_t					)TCPSEVER_STK_SIZE,
							(void*						)NULL,
							(UBaseType_t			)TCPSEVER_TASK_PRIO,
							(TaskHandle_t*		)&TcpSeverTask_Handler);	
	
//创建DHT11 温湿度任务
	xTaskCreate((TaskFunction_t		)dht11_task,
							(const char*			)"dht11_task",
							(uint16_t					)DHT11_STK_SIZE,
							(void*						)NULL,
							(UBaseType_t			)DHT11_TASK_PRIO,
							(TaskHandle_t*		)&DHT11Task_Handler);	

//创建继电器任务							
	xTaskCreate((TaskFunction_t		)jdq_task,
							(const char*			)"jdq_task",
							(uint16_t					)JDQ_STK_SIZE,
							(void*						)NULL,
							(UBaseType_t			)JDQ_TASK_PRIO,
							(TaskHandle_t*		)&IM1281BTask_Handler);								
							
//创建电流计任务					
	xTaskCreate((TaskFunction_t		)im1281b_task,
							(const char*			)"im1281b_task",
							(uint16_t					)IM1281B_STK_SIZE,
							(void*						)NULL,
							(UBaseType_t			)IM1281B_TASK_PRIO,
							(TaskHandle_t*		)&IM1281BTask_Handler);														
	
	vTaskDelete(StartTask_Handler);                           //删除总任务，释放资源
	taskEXIT_CRITICAL();                                      //退出临界区，允许中断
}


extern unsigned int rec_data[4];

void dht11_task(void* pvParameters)
{
	while(1)
	{
	
		DHT11_REC_Data();
		printf("温度：%d\r\n",rec_data[2]);
		printf("湿度：%d\r\n",rec_data[0]);	
		usRegHoldingBuf[0] = rec_data[2];
		usRegHoldingBuf[1] = rec_data[0];
		vTaskDelay(500);	
	}
}

void tcpsever_task(void* pvParameters)
{
	eMBTCPInit(TCPSEVER_PORT);                       //初始化服务器
  eMBEnable();	
	while(1)
	{
		switch(getSn_SR(0))
		{		
			case SOCK_CLOSED:                                       //socket处于关闭状态			
				printf("Tcp Sever Start!\r\n");
				socket(0,Sn_MR_TCP,TCPSEVER_PORT,Sn_MR_ND);			
				printf("socket 0 open success.\r\n");
				break;		
				case SOCK_INIT:                                        //socket处于初始化状态   			
				listen(0);                                             //进入监听模式
				break;
				case  SOCK_ESTABLISHED:   
					if(getSn_IR(0) & Sn_IR_CON)
						{
								printf("socket 0: connected.\r\n");
								setSn_IR(0,Sn_IR_CON);                         //表示连接已经建立
						}			
			        //接收来自服务器数据的大小
					ucTCPRequestLen = getSn_RX_RSR(0);												
					if(ucTCPRequestLen>0)
						{
							//接收来自服务器的数据
							memset(ucTCPRequestFrame,0,MB_TCP_BUF_SIZE);     //清除TCP请求数据缓存区
							recv(0,ucTCPRequestFrame,ucTCPRequestLen);       //接收数据
							xMBPortEventPost(EV_FRAME_RECEIVED);             //解析数据帧，执行功能码
							eMBPoll();                                       //轮询处理
							eMBPoll();
							if(bFrameSent)                                   //判断响应帧是否发送
							{
								bFrameSent = FALSE;
								send(0,ucTCPResponseFrame,ucTCPResponslen);		 //发送数据
							}							
						}
				break;
				case SOCK_CLOSE_WAIT: //socket处于等待关闭状态
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
			printf("继电器关闭.\r\n");
			relay_off();
		}
		else
		{	
			printf("继电器开启.\r\n");
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

