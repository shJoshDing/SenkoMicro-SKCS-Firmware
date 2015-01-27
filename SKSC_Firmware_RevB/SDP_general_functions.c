// SDP_general_functions.c

#include "SDP_std_include.h"
#include "SDP_general_functions.h"

#include "SDP_adc.h"

extern SDP_GENERAL_DATA_BUF gGeneralDataBuf[];
extern u64 boardInitOtpWord;
extern u8 latchValue;

//-----------------------------------------------------------------------------
void writeToSdpLatch(u8 latchData)
{
	peripheralAccessSdpRev0(true);

	// write to latch (bank 2)
	*((u8*)0x20200000) = latchData;
	ssync();
}


//-----------------------------------------------------------------------------
void flashLed(void)
{
	u8 i;
	
	for (i=0; i<2; i++)
	{
		latchValue = latchValue | 0x08;
		writeToSdpLatch(latchValue);
		//writeToSdpLatch(0x08); //LED on
		waitMilliSec(50);
		latchValue = latchValue & 0xF7;
		writeToSdpLatch(latchValue);
		//writeToSdpLatch(0x00); //LED off
		waitMilliSec(50);
	}

}

/*
void flashLed(void)				//Test Funtion
{
	
	SDP_USB_HEADER usbHeader;
	//usbHeader.cmd = ADI_SDP_CMD_SPORT_ADC_RESET;
	usbHeader.cmd = ADI_SDP_CMD_ADC_TRANSFER;
	//processSportCmd( &usbHeader );
	
	//usbHeader.cmd = ADI_SDP_CMD_SPORT_TRANSFER;
	processADCCmd( &usbHeader );
}
*/



//-----------------------------------------------------------------------------
void invalidTransferCounts(SDP_USB_HEADER *pUsbHeader)
{
	unknownCommand(pUsbHeader);
}

//-----------------------------------------------------------------------------
void unknownCommand(SDP_USB_HEADER *pUsbHeader)
{
	s32 i;

	// receive data if necessary
	if (pUsbHeader->downByteCount > 0)
	{
		usbGetBulkData(&gGeneralDataBuf[0], pUsbHeader->downByteCount, true);
	}

	// send zero data if any requested.
	if (pUsbHeader->upByteCount > 0)
	{
		for (i=0; i < pUsbHeader->upByteCount/4 + 1; i++)
		{
			gGeneralDataBuf[0].u32[i] = 0;
		}
		usbSendBulkData(&gGeneralDataBuf[0], pUsbHeader->upByteCount, true);
	}
}

//-----------------------------------------------------------------------------
void waitMilliSec(u32 msec)
{
	volatile u64 cur;
	volatile u64 nd;
	
	_GET_CYCLE_COUNT(cur);
	nd = cur + ((__PROCESSOR_SPEED__/1000)*msec);
	while (cur < nd)
	{
		_GET_CYCLE_COUNT(cur);
	}
}

//-----------------------------------------------------------------------------
void waitHalfMilliSec(u32 hmsec)
{
	volatile u64 cur;
	volatile u64 nd;
	
	_GET_CYCLE_COUNT(cur);
	//668 is test on oscilloscope.
	nd = cur + ((__PROCESSOR_SPEED__/668)* hmsec);
	while (cur < nd)
	{
		_GET_CYCLE_COUNT(cur);
	}
}

//------------------------------------------------------------------------------
void waitHalfMicroSec(u32 hmsec)
{
	volatile u64 cur;
	volatile u64 nd;
	
	_GET_CYCLE_COUNT(cur);
	nd = cur + ((__PROCESSOR_SPEED__/(668 * 1000))* hmsec);
	while (cur < nd)
	{
		_GET_CYCLE_COUNT(cur);
	}
}

//------------------------------------------------------------------------------
void enablePG0(void)
{
	*pPORTG_FER &= ~PIN_PG0;
	*pPORTGIO_INEN &= ~PIN_PG0;
	*pPORTGIO_SET = PIN_PG0;
	*pPORTGIO_DIR |= PIN_PG0;
}

//------------------------------------------------------------------------------
void enableUserFlash(void)
{
	*pPORTG_FER &= ~PIN_PG10;
	*pPORTGIO_INEN &= ~PIN_PG10;
	*pPORTGIO_SET = PIN_PG10;	
	*pPORTGIO_DIR |= PIN_PG10;
	
}

//------------------------------------------------------------------------------
void disableUserFlash(void)
{
	*pPORTG_FER &= ~PIN_PG10;
	*pPORTGIO_INEN &= ~PIN_PG10;
	*pPORTGIO_CLEAR = PIN_PG10;	
	*pPORTGIO_DIR |= PIN_PG10;		
}

//-------------------------------------------------------------------------------
void peripheralAccessSdpRev0(bool peripherals)
{
 
	//	PG0 has two different functions:

	//	1) Rev A and B: PG0 controls the access to flash memories or peripheral devices
	//	2) Rev 0.1 in Advanced (3 connectors board): PG0 is USB configured signal

		
	// Check Hard. Revision (1: SPI M25P32, 0: Parallel) 
	if ( ((boardInitOtpWord & 0xFFFF00000000) >> 32) == 0x0 )  // distinto
	{
		if (peripherals)
		{
			*pPORTGIO_SET = PIN_PG0;				// select peripherals (PG0 = 1)
		}
		else
		{
			*pPORTGIO_CLEAR = PIN_PG0;				// select flash (PG0 = 0)
		}
		ssync();
	}
	
}

//-------------------------------------------------------------------------------
/*void generateTimer(u32 period, u32 pulse_duration, u8 timerPin, bool polarity)
{
	// Counters using SCLK, could be changed if needed
	// pulse_duration and period are given in nanoseconds
	
	u32 pulse_counter; 
	u32 period_counter;
	
//	pulse_counter = pulse_duration*(1000/120); 		// SCLK 120MHz
//	period_counter = period*(1000/120);
	pulse_counter = pulse_duration/8.33; 
	period_counter = period/8.33;

	
	switch(timerPin)
	{
		case 0:									// timer A (PG7) Connector A
			
			*pPORTG_FER |= 0x0080;				// PG7 as a peripheral function
			*pPORTG_MUX &= ~(0x0030);			// PG7 as PWM_OUT
					
			if (polarity)
				*pTIMER3_CONFIG = 0x000D;    	// positive action pulse
			else
				*pTIMER3_CONFIG = 0x0009;    	// negative action pulse
				
			*pTIMER3_WIDTH = pulse_counter;		
			*pTIMER3_PERIOD = period_counter;	
			
			*pTIMER_ENABLE |= 0x0008;			// Timer3 enable
						
			break;
			
		case 1:									// timer B (PG5) Connector A
			
			*pPORTG_FER |= 0x0020;				// PG5 as a peripheral function
			*pPORTG_MUX &= ~(0x000C);			// PG5 as PWM_OUT
					
			if (polarity)
				*pTIMER1_CONFIG = 0x000D;    	// positive action pulse
			else
				*pTIMER1_CONFIG = 0x0009;    	// negative action pulse
				
			*pTIMER1_WIDTH = pulse_counter;		
			*pTIMER1_PERIOD = period_counter;	
			
			*pTIMER_ENABLE |= 0x0002;			// Timer1 enable
			
			break;
		
		case 2:									// timer D (PJ0): Shared across the two connectors (A and B)
			
			if (polarity)
				*pTIMER0_CONFIG = 0x000D;    	// positive action pulse
			else
				*pTIMER0_CONFIG = 0x0009;    	// negative action pulse
				
			*pTIMER0_WIDTH = pulse_counter;		
			*pTIMER0_PERIOD = period_counter;	
			
			*pTIMER_ENABLE |= 0x0001;			// Timer0 enable
		
			break;
			
		case 3:									// timer A (PG8) Connector B
		
			*pPORTG_FER |= 0x0100;				// PG8 as a peripheral function
			*pPORTG_MUX &= ~(0x0030);			// PG8 as PWM_OUT
			
			if (polarity)
				*pTIMER4_CONFIG = 0x000D;    	// positive action pulse
			else
				*pTIMER4_CONFIG = 0x0009;    	// negative action pulse
				
			*pTIMER4_WIDTH = pulse_counter;		
			*pTIMER4_PERIOD = period_counter;	
			
			*pTIMER_ENABLE |= 0x0010;			// Timer4 enable
		
			break;
			
		case 4:									// timer B (PG6) Connector B
		
			*pPORTG_FER |= 0x0040;							// PG6 as a peripheral function
			*pPORTG_MUX = (*pPORTG_MUX & 0xFFF3) | 0x0004;	// PG6 as PWM_OUT
		
			if (polarity)
				*pTIMER2_CONFIG = 0x000D;    	// positive action pulse
			else
				*pTIMER2_CONFIG = 0x0009;    	// negative action pulse
				
			*pTIMER2_WIDTH = pulse_counter;		
			*pTIMER2_PERIOD = period_counter;	
			
			*pTIMER_ENABLE |= 0x0004;			// Timer2 enable
		
			break;		
			
		default:								// No valid timer
			
			
			break;
	}

}

void disableTimer(u8 timerPin)
{
	switch(timerPin)
	{
		case 0:	
			*pTIMER_DISABLE |= 0x0008;
			break;
			
		case 1:
			*pTIMER_DISABLE |= 0x0002;
			break;
			
		case 2:
			*pTIMER_DISABLE |= 0x0001;
			break;
			
		case 3:
			*pTIMER_DISABLE |= 0x0010;
			break;
			
		case 4:
			*pTIMER_DISABLE |= 0x0004;
			break;	
			
		default:
			break;
	}
}*/
