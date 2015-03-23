#include "SDP_uart.h"
#include "SDP_std_include.h"
#include <string.h>
#include <services\services.h>				// system services
#include <drivers\adi_dev.h>				// device manager includes
#include <drivers\uart\adi_uart.h>				// uart driver includes

extern ADI_DEV_MANAGER_HANDLE ghDevManager;
// Handle to the UART driver
static ADI_DEV_DEVICE_HANDLE 	uartDriverHandle;

static bool bUartConfiged = false;

ADI_DEV_1D_BUFFER uartInBuffer;
ADI_DEV_1D_BUFFER uartOutBuffer;

// data for the inbound and outbound buffer
u8 uartInData;
u8 uartOutData;


u8 HSPYPowerOnOff[HSPYSetCommandLength] = {0x00,0x10,0x10,0x04,0x00,0x01,0x02,0x00,0x01,0x7A,0x45};
u8 HSPYSetCommand[HSPYSetCommandLength] = {0x00,0x10,0x10,0x00,0x00,0x01,0x02,0x03,0xE8,0xBA,0xBF };



static void uartInitilize( u32 bitRate, u32 stopBits);
static void uartWrite( u32 uCommend, u32 uParameter);
static void SendString ( u8 *pString );
static void SendChar ( u8  cChar );
static void SendHSPYCommand ( u8 nNum, u8 *pCommand);
static void SendValue ( u8 uValue );
static void uartCallback();
static void HextoASCII ( u8 nHexValue );
static u16 CRC16( u8 *arr_buff, u8 len);



//#define DEBUG

#ifdef DEBUG
u32 resultArray[32];
#endif


//-----------------------------------------------------------------------------
void processUartCmd(SDP_USB_HEADER *pUsbHeader)
{
	switch (pUsbHeader->cmd)
	{
		case ADI_SDP_CMD_UART_INIT:
			#ifdef DEBUG
			flashLed();
			#endif
			if (!bUartConfiged)
			{
				uartInitilize( 4800, 1);
			}
			break; 
			
		case ADI_SDP_CMD_UART_WRITE:
			//sportStreamFrom(pUsbHeader->paramArray);
			uartWrite( pUsbHeader->downByteCount, pUsbHeader->upByteCount);
			break;
			
		case ADI_SDP_CMD_UART_READ:
			//sportStreamFrom(pUsbHeader->paramArray);
			//adcSignalPath( pUsbHeader->numParam );
			break;
			
		default:
			unknownCommand(pUsbHeader);
			break;
	}
}


//-----------------------------------------------------------------------------
void uartInitilize( u32 bitRate, u32 stopBits)
{
	u32 result = 0;
	
	ADI_DEV_CMD_VALUE_PAIR ConfigurationTable [] = {	// configuration table for the UART driver
		{ ADI_DEV_CMD_SET_DATAFLOW_METHOD, 	(void *)ADI_DEV_MODE_CHAINED	},
		{ ADI_UART_CMD_SET_DATA_BITS, 		(void *)8						},
		{ ADI_UART_CMD_ENABLE_PARITY, 		(void *)FALSE					},
		{ ADI_UART_CMD_SET_STOP_BITS, 		(void *)1						},
		{ ADI_UART_CMD_SET_BAUD_RATE, 		(void *)bitRate					},
		{ ADI_DEV_CMD_END,					NULL							},
	};
	
	uartInBuffer.Data = &uartInData;
	uartInBuffer.ElementCount = 1;
	uartInBuffer.ElementWidth = 1;
	uartInBuffer.CallbackParameter = &uartInBuffer;
	uartInBuffer.ProcessedFlag = FALSE;
	uartInBuffer.pNext = NULL;
	
	uartOutBuffer.Data = &uartOutData;
	uartOutBuffer.ElementCount = 1;
	uartOutBuffer.ElementWidth = 1;
	uartOutBuffer.CallbackParameter = NULL;
	uartOutBuffer.ProcessedFlag = FALSE;
	uartOutBuffer.pNext = NULL;
	
	// open UART driver for bidirectional data flow
	result = adi_dev_Open(ghDevManager, &ADIUARTEntryPoint, 0, NULL, &uartDriverHandle, ADI_DEV_DIRECTION_BIDIRECTIONAL, NULL, NULL, uartCallback);
	#ifdef DEBUG
	resultArray[0] = result;
	#endif
	if( result )
	{
		return;
	}
	
	// config uart
	result = adi_dev_Control(uartDriverHandle, ADI_DEV_CMD_TABLE, ConfigurationTable);
	#ifdef DEBUG
	resultArray[1] = result;
	#endif
	if( result )
	{
		return;
	}
	
	// Submit Inbound buffer to UART
    result = adi_dev_Read (uartDriverHandle, ADI_DEV_1D, (ADI_DEV_BUFFER *)&uartInBuffer);
  	#ifdef DEBUG
	resultArray[2] = result;
	#endif
	if( result )
	{
		return;
	}
       
    // Submit Outbound buffer to UART
    //result = adi_dev_Write(uartDriverHandle, ADI_DEV_1D, (ADI_DEV_BUFFER *)&uartOutBuffer);
   	#ifdef DEBUG
	resultArray[3] = result;
	#endif
	if( result )
	{
		return;
	}
        
    // Enable UART dataflow 
    result = adi_dev_Control(uartDriverHandle, ADI_DEV_CMD_SET_DATAFLOW, (void *)TRUE);
    #ifdef DEBUG
	resultArray[4] = result;
	#endif
	if( result )
	{
		return;
	}
	
	//config success
	bUartConfiged = true;
}


//-----------------------------------------------------------------------------
static void uartWrite( u32 uCommend, u32 uParameter)
{
	u8 *pUartCommend;
	u8 *pUartParameter;
	u8 UartSetValue = 0;
	
	
	UartSetValue = (u8)uParameter;
	
	switch ( uCommend )
	{
		
		case ADI_SDP_CMD_UART_REMOTE:				//0X61
			#ifdef DEBUG
			flashLed();
			#endif
			
			pUartCommend = "SYST:REMOTE\r\n";
			SendString( pUartCommend );

			break;
			
		case ADI_SDP_CMD_UART_LOCAL:				//0X61
			#ifdef DEBUG
			flashLed();
			#endif
			
			pUartCommend = "SYST:LOCAL\r\n";
			SendString( pUartCommend );

			break;
		
		case ADI_SDP_CMD_UART_OUTPUTON:				//0X61
			#ifdef DEBUG
			flashLed();
			#endif
			
			//pUartCommend = "OUTP 1\r\n";
			//SendString( pUartCommend );
			//HSPYSetCommand[] = {0x00,0x10,0x10,0x04,0x00,0x01,0x02,0x00,0x01,0x7A,0x45};
			SendHSPYCommand( HSPYSetCommandLength, HSPYPowerOnOff );

			break;
			
		case ADI_SDP_CMD_UART_OUTPUTOFF:				//0X61
			#ifdef DEBUG
			flashLed();
			#endif
			
			//pUartCommend = "OUTP 0\r\n";
			//SendString( pUartCommend );
			SendHSPYCommand( HSPYSetCommandLength, HSPYPowerOnOff );
			
			break;
		
		case ADI_SDP_CMD_UART_SETVOLT:				//0X61
			#ifdef DEBUG
			flashLed();
			#endif
			
			
			//pUartCommend = "VOLT ";
			//pUartParameter = "V\r\n";

			//SendString( pUartCommend );
			//HextoASCII( uParameter );
			//SendString( pUartParameter );
			uParameter *= 100;
			
			HSPYSetCommand[3] = 0x00;
			//HSPYSetCommand[7] = 0x00;
 			//HSPYSetCommand[8] = 0x64;
 			HSPYSetCommand[7] = uParameter>>8;
 			HSPYSetCommand[8] = uParameter;
 			HSPYSetCommand[9] = CRC16( HSPYSetCommand , HSPYSetCommandLength-2)>>8; 
 			HSPYSetCommand[10] = CRC16( HSPYSetCommand , HSPYSetCommandLength-2);
 			SendHSPYCommand( HSPYSetCommandLength, HSPYSetCommand );
			
			
			break;
			
		case ADI_SDP_CMD_UART_SETCURR:				//0X61
			#ifdef DEBUG
			flashLed();
			#endif
			
			//pUartCommend = "CURR ";
			//pUartParameter = "A\r\n";
			
			//if(uParameter<10)
			//{
			//	SendString( pUartCommend );
			//	HextoASCII( uParameter );
			//	SendString( pUartParameter );
			//}
			//else
			//{
			//	SendString( pUartCommend );
			//	HextoASCII( uParameter/10 );
			//	HextoASCII( uParameter%10 );
			//	SendString( pUartParameter );
			//}
			uParameter *= 1000;
			HSPYSetCommand[3] = 0x01;
			//HSPYSetCommand[7] = 0x4E;
 			//HSPYSetCommand[8] = 0x20;
 			HSPYSetCommand[7] = uParameter>>8;
 			HSPYSetCommand[8] = uParameter;
 			HSPYSetCommand[9] = CRC16( HSPYSetCommand , HSPYSetCommandLength-2)>>8; 
 			HSPYSetCommand[10] = CRC16( HSPYSetCommand , HSPYSetCommandLength-2);
 			SendHSPYCommand( HSPYSetCommandLength, HSPYSetCommand );
			
			break;
					
		default:
			break;
	} 

}



//-----------------------------------------------------------------------------
static void SendString ( u8 *pString )
{
    /* Index/Counter */
    u32  nIndex;
    
    /* Send char until we reach end of string (NULL) */
    for(nIndex = 0; pString[nIndex] != '\0'; nIndex++)
    {
        SendChar(pString[nIndex]);
    }
}




//-----------------------------------------------------------------------------
static void SendChar ( u8  cChar )
{
    u32 nDDSSResult = ADI_DEV_RESULT_SUCCESS;
    /* To hold UART Transmit Holding register status */
    u32 nTHRstatus = 0;

    while (nDDSSResult == ADI_DEV_RESULT_SUCCESS)
    {
        /* IF (UART transmitter ready to accept next data?) */
        if (nTHRstatus)
        {     
            /* send the next data */
            uartOutData = cChar;
            /* send the outbound buffer */
            /* Submit Outbound buffer to UART */
            adi_dev_Write(uartDriverHandle, ADI_DEV_1D, (ADI_DEV_BUFFER *)&uartOutBuffer);
            /* exit this loop */
            break;
        }
        /* ELSE (get Transmit holding register status) */
        else
        {
            nDDSSResult = adi_dev_Control(uartDriverHandle, ADI_UART_CMD_GET_TEMT, (void *)&nTHRstatus);
        }
    }

}



//---------------------------------------------------------------------------
static void SendHSPYCommand ( u8 nNum, u8 *pCommand )
{
	u8 nIndex;
	
	for( nIndex = 0; nIndex < nNum; nIndex++)
	{
		SendValue( pCommand[nIndex] );
	}

}



//----------------------------------------------------------------------------
static void SendValue ( u8 uValue )
{
	u32 nDDSSResult = ADI_DEV_RESULT_SUCCESS;
    /* To hold UART Transmit Holding register status */
    u32 nTHRstatus = 0;

    while (nDDSSResult == ADI_DEV_RESULT_SUCCESS)
    {
        /* IF (UART transmitter ready to accept next data?) */
        if (nTHRstatus)
        {     
            /* send the next data */
            uartOutData = uValue;
            /* send the outbound buffer */
            /* Submit Outbound buffer to UART */
            adi_dev_Write(uartDriverHandle, ADI_DEV_1D, (ADI_DEV_BUFFER *)&uartOutBuffer);
            /* exit this loop */
            break;
        }
        /* ELSE (get Transmit holding register status) */
        else
        {
            nDDSSResult = adi_dev_Control(uartDriverHandle, ADI_UART_CMD_GET_TEMT, (void *)&nTHRstatus);
        }
    }

}




//-----------------------------------------------------------------------------
void uartCallback()
{

}

//-----------------------------------------------------------------------------
void HextoASCII ( u8 nHexValue )
{ 
    SendChar(nHexValue+0x30);
}


//-----------------------------------------------------------------------------
u16 CRC16( u8 *arr_buff, u8 len)
{ 
	u16 crc=0xFFFF;
  	u8 i,j;
  	for(j=0; j<len; j++)
  	{ crc = crc ^ * arr_buff++;
    	for(i=0; i<8; i++)
    	{ if((crc & 0x0001)>0){crc=crc>>1;crc=crc^0xa001;}
       		else crc=crc>>1;
     	}
  	}
  	return(crc);
}







