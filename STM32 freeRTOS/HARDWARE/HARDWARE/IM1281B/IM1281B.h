#ifndef __IM1281B_H
#define __IM1281B_H	 

#include "stm32f10x.h"  

#define USART3_MAX_RECV_LEN 200
#define USART3_MAX_SEND_LEN 200

typedef struct data{

	 unsigned long Voltage_data;
	 unsigned long Current_data;
	 unsigned long Power_data;
	 unsigned long Energy_data;
	 unsigned long Pf_data;
	 unsigned long CO2_data;
}DataArray;

unsigned int calccrc(unsigned char crcbuf,unsigned int crc);

unsigned int chkcrc(unsigned char* buf,unsigned char len);

void read_data(int read_enbale);

DataArray Analysis_data(void);
		 				    
void usart3_init(u32 bound);
void USART3_IRQHandler(void);
#endif
