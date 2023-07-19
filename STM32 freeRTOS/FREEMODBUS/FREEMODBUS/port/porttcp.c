#include "stm32f10x.h"
#include <stdio.h>
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
/*----------------------------W5500-------------------------------------------*/
#include "socket.h"
#include "string.h"

/* ----------------------- MBAP Header --------------------------------------*/
#define MB_TCP_UID          6
#define MB_TCP_LEN          4
#define MB_TCP_FUNC         7


/* ----------------------- Defines  -----------------------------------------*/

extern uint8_t ucTCPRequestFrame[MB_TCP_BUF_SIZE];
extern uint16_t ucTCPRequestLen;
extern uint8_t  ucTCPResponseFrame[MB_TCP_BUF_SIZE];
extern uint16_t ucTCPResponslen;
extern uint8_t bFrameSent;

extern void xMBUtilSetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits,UCHAR ucValue );
extern UCHAR xMBUtilGetBits( UCHAR * ucByteBuf, USHORT usBitOffset, UCHAR ucNBits );

//��ʼ��socket����
BOOL
xMBTCPPortInit( USHORT usTCPPort )
{
	
    SOCKET sn;
    sn=0;
    if(getSn_SR(sn)==SOCK_CLOSED)
    {
			printf("Tcp Sever Start!\r\n");
       
			socket(sn,Sn_MR_TCP,usTCPPort,0x00);   //��socket
			
			printf("socket 0 open success.\r\n");
    }
    if (getSn_SR(sn)==SOCK_INIT)
    {
     listen(sn);
		 printf("socket 0 listening .\r\n");
     return TRUE;
    }
    return FALSE;
}



BOOL xMBTCPPortGetRequest(UCHAR **ppucMBTCPFrame,USHORT *usTCPLength)
{
	  *ppucMBTCPFrame = (uint8_t *) &ucTCPRequestFrame[0];  
    *usTCPLength = ucTCPRequestLen;    
    /* Reset the buffer. */  
    ucTCPRequestLen = 0;  
    return TRUE; 

}




//��������
BOOL
xMBTCPPortSendResponse( const UCHAR * pucMBTCPFrame, USHORT usTCPLength )
{
		memcpy(ucTCPResponseFrame,pucMBTCPFrame,usTCPLength);
		ucTCPResponslen = usTCPLength;
		bFrameSent = TRUE;
		return bFrameSent;
}

void vMBTCPPortClose(void)
{
	
}

void vMBTCPPortDisable(void)
{

}


//����Ĵ�������
uint16_t usRegInputBuf[REG_INPUT_NREGS] = {0};
//���ּĴ�������               
uint16_t usRegHoldingBuf[REG_HOLDING_NREGS] = {0};                                       
//��Ȧ״̬
uint8_t ucRegCoilsBuf[REG_COILS_SIZE] = {0};
//��ɢ�Ĵ�������
uint8_t ucRegDiscreteBuf[REG_DISCRETE_SIZE] = {0};


/****************************************************************************
* ��          �ƣ�eMBRegInputCB
* ��    �ܣ���ȡ����Ĵ�������Ӧ�������� 04 eMBFuncReadInputRegister
* ��ڲ�����pucRegBuffer: ���ݻ�������������Ӧ����   
*                                                usAddress: �Ĵ�����ַ
*                                                usNRegs: Ҫ��ȡ�ļĴ�������
* ���ڲ�����
* ע          �⣺��λ�������� ֡��ʽ��: SlaveAddr(1 Byte)+FuncCode(1 Byte)
*                                                                +StartAddrHiByte(1 Byte)+StartAddrLoByte(1 Byte)
*                                                                +LenAddrHiByte(1 Byte)+LenAddrLoByte(1 Byte)+
*                                                                +CRCAddrHiByte(1 Byte)+CRCAddrLoByte(1 Byte)
*                                                        3 ��
****************************************************************************/
eMBErrorCode eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    USHORT          iRegIndex;


    if ((usAddress >= REG_INPUT_START) && (usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS))
    {
        iRegIndex = usAddress - REG_INPUT_START;
        while (usNRegs > 0)
        {
            *pucRegBuffer++ = (UCHAR) (usRegInputBuf[iRegIndex] >> 8);
            *pucRegBuffer++ = (UCHAR) (usRegInputBuf[iRegIndex] & 0xFF);
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}

/****************************************************************************
* ��          �ƣ�eMBRegHoldingCB
* ��    �ܣ���Ӧ�������У�06 д���ּĴ��� eMBFuncWriteHoldingRegister
*                                                                                                        16 д������ּĴ��� eMBFuncWriteMultipleHoldingRegister
*                                                                                                        03 �����ּĴ��� eMBFuncReadHoldingRegister
*                                                                                                        23 ��д������ּĴ��� eMBFuncReadWriteMultipleHoldingRegister
* ��ڲ�����pucRegBuffer: ���ݻ�������������Ӧ����   
*                                                usAddress: �Ĵ�����ַ
*                                                usNRegs: Ҫ��д�ļĴ�������
*                                                eMode: ������
* ���ڲ�����
* ע          �⣺4 ��
****************************************************************************/
eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
        eMBErrorCode    eStatus = MB_ENOERR;
        int             iRegIndex;


        if((usAddress >= REG_HOLDING_START)&&((usAddress+usNRegs) <= (REG_HOLDING_START + REG_HOLDING_NREGS)))
        {
                iRegIndex = (int)(usAddress - REG_HOLDING_START);
                switch(eMode)
                {                                       
                        case MB_REG_READ://�� MB_REG_READ = 0
                                while(usNRegs > 0)
                                {
                                        *pucRegBuffer++ = (u8)(usRegHoldingBuf[iRegIndex] >> 8);            
                                        *pucRegBuffer++ = (u8)(usRegHoldingBuf[iRegIndex] & 0xFF);
                                        iRegIndex++;
                                        usNRegs--;                                       
                                }                           
                        break;
                        case MB_REG_WRITE://д MB_REG_WRITE = 0
                                while(usNRegs > 0)
                                {         
                                        usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                                        usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                                        iRegIndex++;
                                        usNRegs--;
                                }
                        break;                                
                }
        }
        else//����
        {
                eStatus = MB_ENOREG;
        }        
        
        return eStatus;
}



/****************************************************************************
* ��          �ƣ�eMBRegCoilsCB
* ��    �ܣ���Ӧ�������У�01 ����Ȧ eMBFuncReadCoils
*                                                  05 д��Ȧ eMBFuncWriteCoil
*                                                  15 д�����Ȧ eMBFuncWriteMultipleCoils
* ��ڲ�����pucRegBuffer: ���ݻ�������������Ӧ����   
*                                                usAddress: ��Ȧ��ַ
*                                                usNCoils: Ҫ��д����Ȧ����
*                                                eMode: ������
* ���ڲ�����
* ע          �⣺��̵���
*                                                0 ��
****************************************************************************/
eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils,
              eMBRegisterMode eMode )
{
  //����״̬
  eMBErrorCode eStatus = MB_ENOERR;
  //�Ĵ�������
  int16_t iNCoils = ( int16_t )usNCoils;
  //�Ĵ���ƫ����
  int16_t usBitOffset;
  
  //���Ĵ����Ƿ���ָ����Χ��
  if( ( (int16_t)usAddress >= REG_COILS_START ) && ( usAddress + usNCoils <= REG_COILS_START + REG_COILS_SIZE ) )
  {
    //����Ĵ���ƫ����
    usBitOffset = ( int16_t )( usAddress - REG_COILS_START );
    switch ( eMode )
    {
      //������
    case MB_REG_READ:
      while( iNCoils > 0 )
      {
        *pucRegBuffer++ = xMBUtilGetBits( ucRegCoilsBuf, usBitOffset,
                                         ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ) );
        iNCoils -= 8;
        usBitOffset += 8;
      }
      break;
      
      //д����
    case MB_REG_WRITE:
      while( iNCoils > 0 )
      {
        xMBUtilSetBits( ucRegCoilsBuf, usBitOffset,
                       ( uint8_t )( iNCoils > 8 ? 8 : iNCoils ),
                       *pucRegBuffer++ );
        iNCoils -= 8;
      }
      break;
    }
    
  }
  else
  {
    eStatus = MB_ENOREG;
  }
  return eStatus;
}
/****************************************************************************
* ��          �ƣ�eMBRegDiscreteCB
* ��    �ܣ���ȡ��ɢ�Ĵ�������Ӧ�������У�02 ����ɢ�Ĵ��� eMBFuncReadDiscreteInputs
* ��ڲ�����pucRegBuffer: ���ݻ�������������Ӧ����   
*                                                usAddress: �Ĵ�����ַ
*                                                usNDiscrete: Ҫ��ȡ�ļĴ�������
* ���ڲ�����
* ע          �⣺1 ��
****************************************************************************/
eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
        eMBErrorCode    eStatus = MB_ENOERR;
        int             iRegIndex;
        u8 i;
        USHORT readNumber=usNDiscrete;
        USHORT coilValue=0x0000;
        iRegIndex = (int)(usAddress + usNDiscrete-REG_DISCRETE_START);
        if((usAddress >= REG_DISCRETE_START)&&\
        ((usAddress+usNDiscrete) <= (REG_DISCRETE_START + REG_DISCRETE_SIZE)))
        {
                for(i=0;i<usNDiscrete;i++)
                {
                        readNumber--;
                        iRegIndex--;
                        coilValue|=ucRegDiscreteBuf[iRegIndex]<<readNumber;
                }
                if(usNDiscrete<=8)
                {
                        * pucRegBuffer=coilValue;
                }
                else
                {
                        * pucRegBuffer++ = (coilValue)&0x00ff;
                        * pucRegBuffer++ = (coilValue>>8)&0x00ff;
                }
        }
        else
        {
                eStatus = MB_ENOREG;
        }
    return eStatus;
}

