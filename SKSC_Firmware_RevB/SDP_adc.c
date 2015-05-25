// SDP_sport.c

#include "SDP_std_include.h"
#include "SDP_adc.h"
#include "SDP_signalpath.h"
#include <string.h>
//#include "SDP_general_functions.h"


#define DEBUG

#define MAX_SAMPLES_NUMBER_CAPTURE 	2048
#define SPI_CLK_PIN 				PF5
#define SPI_SS_PIN 					PF1
#define SPI_MISO_PIN 				PF0
#define SPI_CONVST_PIN 				PG5
//#define RELAY_1						0x0002
//#define RELAY_2						0x0004
//#define RELAY_3						0x0008
//#define RELAY_4						0x0040
//#define RELAY_5						0x0020
//#define RELAY_POWERON				0x0400
//#define MODULE_15V_OUT				0x1000





//bool bPowerVolt = false;
bool bAdcReset = false;
bool bInitGpiosForADC = false;
//bool bInitGpiosForSignalPath = false;


u16 gAdcDataBuffer[MAX_SAMPLES_NUMBER_CAPTURE];






#ifdef DEBUG
u16 testArray[0xFFFF];
#endif
//memset(testArray, 0xAA, sizeof(testArray));

//-----------------------------------------------------------------------------
void processADCCmd(SDP_USB_HEADER *pUsbHeader)
{
	switch (pUsbHeader->cmd)
	{
		case ADI_SDP_CMD_ADC_TRANSFER:
			//captureAdcData( pUsbHeader->downByteCount, pUsbHeader->upByteCount);
			captureAdcData( pUsbHeader->downByteCount, pUsbHeader->upByteCount);
			break; 
			
		case ADI_SDP_CMD_ADC_RESET:
			//sportStreamFrom(pUsbHeader->paramArray);
			adcReset();
			break;
			
		case ADI_SDP_CMD_ADC_SIGNAL_PATH:
			//sportStreamFrom(pUsbHeader->paramArray);
			processSignalPathCmd(pUsbHeader->numParam);
			break;
			
		case ADI_SDP_CMD_ADC_SIGPATH_INIT:
			//initialGpiosForSignalPath();
			processSignalPathCmd(0xFF);
			break;
			
			
		default:
			unknownCommand(pUsbHeader);
			break;
	}
}

//-----------------------------------------------------------------------------
static void captureAdcData(u32 adcConvstFreq, u32 sampleNumbers )
{
	u16 i;
	u8 j = 0;
	
	if( sampleNumbers > MAX_SAMPLES_NUMBER_CAPTURE)
	{
		sampleNumbers = MAX_SAMPLES_NUMBER_CAPTURE;
	}

	for( i =0; i<sampleNumbers; i++ )
	{
		gAdcDataBuffer[i] = signalSampleAdc( adcConvstFreq );
		#ifdef DEBUG
		testArray[i]=i;
		#endif
	}
	
	j = sampleNumbers/256;						//calc usb pacakge numbers
	for( i = 0; i<j ; i++ )
	{
		usbSendBulkData( (gAdcDataBuffer + 256*i),512,false);
		waitMilliSec(50);
		//flashLed();
	}
	
	if( sampleNumbers%256 > 0)					//not a multiple of 512, +1, last pacakge size is not 512
	{
		usbSendBulkData( (gAdcDataBuffer + 256*j),2*(sampleNumbers - 256*j),false);
	}
}


		                 
		                 
//-----------------------------------------------------------------------------
static u16 signalSampleAdc( u32 adcConvstFreq )
{
	u16 j = 0;
	u8 i;
	u8 delayCounter;
	u16 readValue = 0;
	bool cpha = true;
	bool cpol = false;
	u32 clkCount;
	
	//clkCount = 600000000/adcConvstFreq;
	clkCount = 600;						// adcConvstFreq = 1MHz
	
	*pTCNTL &= ~TMREN;
	ssync();
	*pTSCALE = 0x0;
	*pTPERIOD = clkCount;
	*pTCNTL = 0x01;
	*pTCOUNT = 300;
	ssync();	

	
	if( !bAdcReset )
	{
		adcReset();
		//bAdcReset = true;
	}
	
	delayCounter = 10;					//200ns
	*pPORTGIO_CLEAR = SPI_CONVST_PIN;	//start of t7
	while(delayCounter--){}
	*pPORTGIO_SET = SPI_CONVST_PIN;		 //end of t7
	
	delayCounter = 20;					//200ns			
	while(delayCounter--){}
	*pPORTFIO_CLEAR = SPI_SS_PIN;
	
	delayCounter = 5;					//200ns			
	while(delayCounter--){}

	for(i = 0; i<12; i++)
	{
		readValue <<= 1;
		*pPORTFIO_SET = SPI_CLK_PIN;
		//*pTCNTL &= ~TMREN;
		//ssync();
		*pTCOUNT = 300;
		ssync();
		*pTCNTL |= TMREN;
		ssync();
		if (*pPORTFIO & SPI_MISO_PIN)
			readValue |= 0x0001;
		while ( *pTCOUNT > 26 ); 
		*pPORTFIO_CLEAR = SPI_CLK_PIN;
		//*pTCNTL &= ~TMREN;
		//ssync();
		*pTCOUNT = 300;
		ssync();
		*pTCNTL |= TMREN;
		ssync();
		while ( *pTCOUNT > 26 );
		
	}

	*pPORTFIO_SET = SPI_SS_PIN;
	
	return readValue;			
}


//-----------------------------------------------------------------------------
static void adcReset(void)
{
	u8 i;
	u8 delayCounter;
	
	if( !bInitGpiosForADC )
	{
		initialGPIOsforADC();
		//bInitGpiosForADC = true;
	}
		
	delayCounter = 10;						//200ns
	//delayCounter = 20;					//400ns
	*pPORTGIO_CLEAR = SPI_CONVST_PIN;		//start of t7
	while(delayCounter--){}
	*pPORTGIO_SET = SPI_CONVST_PIN;		  	//end of t7
	
	delayCounter = 20;
	while(delayCounter--){}
	*pPORTFIO_CLEAR = SPI_SS_PIN;
	ssync();
	
	for(i=0; i<4; i++)
	{
		*pPORTFIO_CLEAR = SPI_CLK_PIN;
		delayCounter = 25;					//500ns
		while(delayCounter--){}
		*pPORTFIO_SET = SPI_CLK_PIN;
		delayCounter = 25;					//500ns
		while(delayCounter--){}
	}
	
	*pPORTFIO_SET = SPI_SS_PIN;	
	*pPORTFIO_CLEAR = SPI_CLK_PIN;
	
	//bAdcReset = true;
	
}


//-----------------------------------------------------------------------------
static void initialGPIOsforADC(void)
{
	//PG7 = CONVST	
	*pPORTG_FER &= ~(SPI_CONVST_PIN);
	*pPORTGIO_INEN &= ~SPI_CONVST_PIN;
	*pPORTGIO_SET |= SPI_CONVST_PIN;
	*pPORTGIO_DIR |= SPI_CONVST_PIN;
	
	//PF1 & PF4 = CS
	*pPORTF_FER &= ~(SPI_SS_PIN);
	*pPORTFIO_INEN &= ~SPI_SS_PIN;
	*pPORTFIO_SET |= SPI_SS_PIN;
	*pPORTFIO_DIR |= SPI_SS_PIN;
	
	*pPORTF_FER &= ~(PF4);
	*pPORTFIO_INEN |= PF4;
	*pPORTFIO_DIR &= ~PF4;
	
	//PF2 & PF5 = CLK
	*pPORTF_FER &= ~(SPI_CLK_PIN);
	*pPORTFIO_INEN &= ~SPI_CLK_PIN;
	*pPORTFIO_SET |= SPI_CLK_PIN;
	*pPORTFIO_DIR |= SPI_CLK_PIN;
	
	*pPORTF_FER &= ~(PF2);
	*pPORTFIO_INEN |= PF2;
	*pPORTFIO_DIR &= ~PF2;
	
	//PF0 = SD	
	*pPORTF_FER &= ~(SPI_MISO_PIN);
	*pPORTFIO_INEN |= SPI_MISO_PIN;
	*pPORTFIO_DIR &= ~SPI_MISO_PIN;

}





