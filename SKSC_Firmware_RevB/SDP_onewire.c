//SDP_OneWire.c

#include "SDP_onewire.h"
//#include "SDP_timer.h"
#include <stdio.h>
#include <cdefBF527.h>
#include <ccblkfn.h>
#include <string.h>
//#include <sys\exception.h>
#include <bfrom.h>
#include "SDP_std_include.h"
#include "SDP_general_functions.h"
#include <services/services.h>          /* system service includes      */ 


u32 PulseCounter = 0;
#define FW_version 0x31

//Params & functions definition ---------------------------------------------------------------
//void processOneWireCmd(SDP_USB_HEADER *pUsbHeader);
//void PulseWidthCounting(SDP_USB_HEADER *pUsbHeader);

void ConfigPulseWidthCounting(void);

EX_INTERRUPT_HANDLER(TIMER_CAP_ISR);				//Timer Capture Interrupt Handle
EX_INTERRUPT_HANDLER(COUNTER_DT_ISR);				//Timer Capture Interrupt Handle
EX_INTERRUPT_HANDLER(CNT_ISR);						//Counter Inter handle

//void ConfigPulsePWMoutput(void);
//void UpdatePulsePWMcounter(u32 uWidth, u32 uPeriod );

void Init_Timer7Interrupts(void);
void Init_CoreTimer_Interrupt_Fuse(void);
void EnableTimer7(void);
//void Init_Counter(void);

void ConfigGpCounter( void );

// Core Timer -----------------------------
void ConfigCoreTimer( u32 scale, bool auto_reload );

void CoreTimerDisable(void);

void CoreTiemrEnable(u32 count);

void Init_CoreTimer_Interrupt(void);

EX_INTERRUPT_HANDLER(CORE_TIMER_ISR);
EX_INTERRUPT_HANDLER(CORE_TIMER_ISR_FUSE);
//-----------------------------------------

// GPIO -----------------------------------
#define ControlPin PG12
void ConfigPG_Output(u16 pinDefine);
void ConfigPG_Input(u16 pinDefine);
void TogglePG_Output(u16 pinDefine);

void ConfigPH_Output(u16 pinDefine);
void TogglePH_Output(u16 pinDefine);

//Command Functions
void PulseWidthCounting(SDP_USB_HEADER *pUsbHeader);
void I2CWriteSingle(SDP_USB_HEADER *pUsbHeader);
void I2CReadSingle(SDP_USB_HEADER *pUsbHeader);
void I2CWriteBurst(SDP_USB_HEADER *pUsbHeader);
void I2CReadBurst(SDP_USB_HEADER *pUsbHeader);
void FuseOn(SDP_USB_HEADER *pUsbHeader);
void FuseOff(SDP_USB_HEADER *pUsbHeader);
void SetPilotWidth(SDP_USB_HEADER *pUsbHeader);
void UpdateFusePulseWidth(SDP_USB_HEADER *pUsbHeader);
void GetFirmWareVersion(SDP_USB_HEADER *pUsbHeader);
void Test(SDP_USB_HEADER *pUsbHeader);

void SetClkBuffered(SDP_USB_HEADER *pUsbHeader);
void SetDataBuffered(SDP_USB_HEADER *pUsbHeader);
void SetLr(SDP_USB_HEADER *pUsbHeader);
void SetOwciPin(SDP_USB_HEADER *pUsbHeader);



// Device R/W Operation -------------------
void Start_Op(u32 pcount);
void Stop_Op(u32 pcount);
void Device_Addr(u32 pcount1, u32 pcount2, u32 addr, bool read);
void Register_Addr(u32 pcount1, u32 pcount2, u32 addr);

void Write_Reg_Data(u32 pcount1, u32 pcount2, u32 data);
void Write_Single(u32 pcount1, u32 pcount2, u32 dev_addr, u32 reg_addr, u32 reg_data);
void Write_Burst(u32 pcount1, u32 pcount2, u32 dev_addr, u32 reg_addr, u32 reg_data[], u32 data_num);

void SetLevelShifterDirection(bool rw);
//void InitialGpioForLevelShifter(u16 LSpin);

//u32 Read_Reg_Data_CoreTimer(u32 pcount_half);
u8 Read_Reg_Data_ExTimer(void);
u32 DistinguishOneZero(u32 count);
//u32 Read_Single(u32 pcount1, u32 pcount2, u32 dev_addr, u32 reg_addr);
u8 Read_Single(u32 pcount1, u32 pcount2, u32 dev_addr, u32 reg_addr);
void Read_Burst(u32 pcount1, u32 pcount2, u32 dev_addr, u32 reg_addr, u8 reg_data[], u32 data_num);

void Delay_BeforeStart(void);
bool TriggerByPH5(void);


//Parameters Definition
bool b_Interrupted = true;
bool b_ExTimerInter = false;
u32 u32_codeTimeCount = 10;				// compensate the time elapse fot the "needless" c code
u32 scale_CoreTimer = 0;

u32 u32_period_fuse = 135;
u32 u32_duration_count_fuse = 50000;
u32 u32_pilotCount_half = 1180;
u32 u32_PilotCount_1x = 2360;			//default value: 4us. this value = realvalue - u32_codeTimeCount.(Should adjust with scope)
u32 u32_PilotCount_3x = 7080;			// 3 pilots to write 1
u32 u32_PilotCount_9x = 21240;			// 9 pilots to write stop signal.
u32 u32_Boundary_OneZero = 720;			// used to judge the pilot width readback. about 1.5 pilots, but it use the system clock, so should divide by 5. 

u32 u32_mask = 0x80;
u32 u32_temp = 0xFFFFFFFF;
u32 u32_array_temp[8] = {0};
u8 u8_ReadBitNum = 8;

u8 sendBuf_OneWire[512];


//---------------------------------------------------
//Test Buffer
//u32 ttt = 0;

//---------------------------------------------------

//---------------------------------------------------------------------------------------------

// Functions ----------------------------------------------------------------------------------
void processOneWireCmd(SDP_USB_HEADER *pUsbHeader)
{	
	switch(pUsbHeader->cmd)
	{		
		case ADI_SDP_CMD_ONEWIER_PW_COUNTING:
			PulseWidthCounting(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_I2CWRITE_SINGLE:
			I2CWriteSingle(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_I2CREAD_SINGLE:
			I2CReadSingle(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_I2CWRITE_BURST:
			I2CWriteBurst(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_I2CREAD_BURST:
			I2CReadBurst(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_FUSE_ON:
			//flashLed();
			FuseOn(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_FUSE_OFF:
			//flashLed();
			FuseOff(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_PILOT_WIDTH:
			SetPilotWidth(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_UPDATE_FUSE_PW:
			UpdateFusePulseWidth(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_FW_Version:
			GetFirmWareVersion(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_TEST:
			Test(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_CLK_BUFFERED:
			SetClkBuffered(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_DATA_BUFFERED:
			SetDataBuffered(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_SET_LR:
			SetLr(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_ONEWIER_OWCI_PIN:
			SetOwciPin(pUsbHeader);
			break;
			
		default:
			break;
	}

}

//-------------------------------------- Funcitons ------------------------------------------------------------------
void PulseWidthCounting(SDP_USB_HEADER *pUsbHeader)
{
	//todo
}

void I2CWriteSingle(SDP_USB_HEADER *pUsbHeader)
{
	//flashLed();
	Write_Single(u32_PilotCount_1x, u32_PilotCount_3x, pUsbHeader->downByteCount, pUsbHeader->paramArray[0], pUsbHeader->paramArray[1]);	
}

void I2CReadSingle(SDP_USB_HEADER *pUsbHeader)
{
	u8 reg_data = Read_Single(u32_PilotCount_1x, u32_PilotCount_3x, pUsbHeader->downByteCount, pUsbHeader->paramArray[0]);
	
	memset(sendBuf_OneWire, 0, sizeof(sendBuf_OneWire));
	//8*sendBuf_OneWire = reg_data;
	sendBuf_OneWire[0] = reg_data;
	
	usbSendBulkData( sendBuf_OneWire,
		                 512,
		                 false);		
}

void I2CWriteBurst(SDP_USB_HEADER *pUsbHeader)
{
	u32 write_num = pUsbHeader->upByteCount;
	u32 data[write_num];
	while(write_num--)
	{
		data[write_num] = pUsbHeader->paramArray[write_num + 1];
	}
	Write_Burst(u32_PilotCount_1x, u32_PilotCount_3x, pUsbHeader->downByteCount, pUsbHeader->paramArray[0], data, pUsbHeader->upByteCount);
}

void I2CReadBurst(SDP_USB_HEADER *pUsbHeader)
{
	u32 read_num = pUsbHeader->upByteCount;
	u8 data[read_num];
	
	Read_Burst(u32_PilotCount_1x, u32_PilotCount_3x, pUsbHeader->downByteCount, pUsbHeader->paramArray[0], data, pUsbHeader->upByteCount);
	
	memset(sendBuf_OneWire, 0, sizeof(sendBuf_OneWire));
	while(read_num--)
	{
		//8*(sendBuf_OneWire + read_num * 4) = data[read_num];
		sendBuf_OneWire[0 + read_num * 4] = data[read_num];
	}	
	
	//(u32)*sendBuf_OneWire = reg_data;
	usbSendBulkData( &sendBuf_OneWire[0],
		                 512,
		                 false);
}

void FuseOn(SDP_USB_HEADER *pUsbHeader)
{
	ConfigPG_Output(ControlPin);
	//u32_duration_count_fuse = pUsbHeader->upByteCount;
	u32_duration_count_fuse = 256*2;				//256 pulse cycles
	//Config Core Timer	
	Init_CoreTimer_Interrupt_Fuse();
	*pTCNTL |= (TINT | TMPWR|TAUTORLD);			//TCNTL bit0, bit3 = 1 (Can be enabled by TMREN, Has Interrupt)

	//For test
	//*pTCNTL |= TAUTORLD;
		
	*pTSCALE = 0;							// cclk/(scale + 1) = 600M/(scale + 1) (ex: 600M/5 = 120M)
	//*pTPERIOD = (pUsbHeader->downByteCount)*1250;			//1ns GUI = 2us FW
	//*pTCOUNT = (pUsbHeader->downByteCount)*1250;
	*pTPERIOD = (pUsbHeader->downByteCount)*156;			//1ns GUI = 0.25us FW
	*pTCOUNT = (pUsbHeader->downByteCount)*156;
	*pTCNTL |= TMREN;
	ssync();
}

void FuseOff(SDP_USB_HEADER *pUsbHeader)
{
	*pTCNTL = 0x00; //(TINT | TMPWR|TAUTORLD);	
	*pTPERIOD = 0;
	//*pTCNTL &= ~TMREN;				// disable core timer
	ssync();
	//flashLed();
}

void SetPilotWidth(SDP_USB_HEADER *pUsbHeader)
{
	u32_PilotCount_1x = pUsbHeader->downByteCount;			//this value = realvalue - u32_codeTimeCount.(The time spend on other codes used for the enalbe core time)
	u32_PilotCount_3x = u32_PilotCount_1x * 3;				// 3 pilots to write 1
	u32_PilotCount_9x = u32_PilotCount_1x * 9;				// 9 pilots to write stop signal.
	// used to judge the pilot width readback. about 1.5 pilots / 5. 
	//because it use system clock not core clock. system clock = 120M, core clock = 600M 
	u32_Boundary_OneZero = pUsbHeader->upByteCount;			
}

void UpdateFusePulseWidth(SDP_USB_HEADER *pUsbHeader)
{
	
}

void GetFirmWareVersion(SDP_USB_HEADER *pUsbHeader)
{
	memset(sendBuf_OneWire, 0, sizeof(sendBuf_OneWire));
	sendBuf_OneWire[0] = FW_version;
	usbSendBulkData( &sendBuf_OneWire[0],
		                 512,
		                 false);
}

void Test(SDP_USB_HEADER *pUsbHeader)
{
	memset(sendBuf_OneWire, 0, sizeof(sendBuf_OneWire));
	sendBuf_OneWire[0] = (u8)u32_Boundary_OneZero;
	sendBuf_OneWire[1] = (u8)(u32_Boundary_OneZero >>8);
	sendBuf_OneWire[2] = (u8)(u32_Boundary_OneZero >> 16);
	sendBuf_OneWire[3] = (u8)(u32_Boundary_OneZero >> 24);
	usbSendBulkData( &sendBuf_OneWire[0],
		                 512,
		                 false);
}





//--------------------------------------- Timer CAP -----------------------------------------------------------------
//void PulseWidthCounting(SDP_USB_HEADER *pUsbHeader)
void ConfigPulseWidthCounting(void)
{
	*pPORTG_FER |= PG11;					// PG11 as a peripheral function
	*pPORTG_MUX &= ~(0x0C00);				// PG11 as TMR7
	
	*pTIMER7_CONFIG = 0x0016;    			// TMR input, Interrupt enable, count width, Postive width, WDTH_CAP mode
	
    //enableTimer(0x0080);

	//disableTimer(0x0080);

}

//--------------------------------------------------------------------------//
// Function:	Init_Interrupts												//
//																			//
// Description:	Initialize Interrupt for Timer7 							//
//--------------------------------------------------------------------------//
void Init_Timer7Interrupts(void)
{
	// Set Timer7 interrupt priority to 5 = IVG12 

	//---*pSIC_IAR4 = 0x5fffffff;
	*pSIC_IAR4 = 0x55555555;

	// assign ISRs to interrupt vectors
	// Timer7 ISR -> IVG 12
	register_handler(ik_ivg12, TIMER_CAP_ISR);	

	// enable Timer 7 interrupt
	*pSIC_IMASK1 |= 0x00000080;
}

void EnableTimer7(void)
{
	//enableTimer(0x0080);
}

EX_INTERRUPT_HANDLER(TIMER_CAP_ISR)
{
	b_ExTimerInter = true;
	// confirm interrupt handling
	PulseCounter = *pTIMER7_WIDTH;
	
	//*pTIMER7_CONFIG = 0x00;
	//*pSIC_IMASK1 &= ~(0x00000080);
	*pTIMER_DISABLE &= 0x0080;				//Disable Timer 7
	*pTIMER_STATUS &= (~TRUN0);	
	
	//CoreTimerDisable();						//ding
	
	ssync();
	// disable Timer 7 interrupt
	//*pSIC_IMASK1 &= ~(0x00000080);
	//disableTimer(0x0080);
	//*pTIMER_DISABLE &= 0x0080;			//Disable Timer 7
	//flashLed();
	
	//u8_ReadBitNum
	//TogglePH_Output(PH5);
}


//------------------------------------------- GP Counter ------------------------------------------------------
//config GP Counter as a timeout for OWCI read no response situation
void ConfigGpCounter( void )
{
	*pCNT_CONFIG = 0x0050;		//set to direction timer mode
	
	*pSIC_IAR3 = 0x44444422;	//Set interrupt priority to 4 = IVG11
	*pCNT_COUNTER = u32_PilotCount_9x;

	// assign ISRs to interrupt vectors
	// Timer7 ISR -> IVG 12
	register_handler(ik_ivg11, COUNTER_DT_ISR);	

	// enable UPcount interrupt 
	*pCNT_IMASK |= 0x0002;
}

EX_INTERRUPT_HANDLER(COUNTER_DT_ISR)
{
	b_ExTimerInter = true;
	// confirm interrupt handling
	PulseCounter = 0x00;
	
	*pTIMER_DISABLE &= 0x0080;				//Disable Timer 7
	*pTIMER_STATUS &= (~TRUN0);	
	
	*pCNT_CONFIG &= (~0x0001);				//Disable GP Counter
	*pCNT_COUNTER = u32_PilotCount_9x;
	
	//TTT[ttt] = ttt+0x10;
	//ttt++;
	
	ssync();
}







//------------------------------------- Core Timer ------------------------------------------------------------
void ConfigCoreTimer(u32 scale, bool auto_reload)
{
	*pTCNTL |= (TINT | TMPWR);			//TCNTL bit0, bit3 = 1 (Can be enabled by TMREN, Has Interrupt)
	if(auto_reload)
	{
		*pTCNTL |= TAUTORLD;			//enable auto reload)
	}
	else
	{
		*pTCNTL &= ~TAUTORLD;			//TCNTL bit2 = 0 (Disable auto reload)
	}

	//For test
	//*pTCNTL |= TAUTORLD;
		
	*pTSCALE = scale;					// cclk/(scale + 1) = 600M/(scale + 1) (ex: 600M/5 = 120M)
	
}

void CoreTimerDisable(void)
{
	*pTCNTL &= ~TMREN;				// TCNTL bit1 = 1 (enable)
}

inline void CoreTiemrEnable(u32 count)
{
	*pTCOUNT = count;				//Set count value
	//*pTPERIOD = count;
	
	//*pTCNTL |= TMREN;				// TCNTL bit1 = 0 (disable)
}

void Init_CoreTimer_Interrupt(void)
{
	register_handler(ik_timer, CORE_TIMER_ISR);
	*pIMASK |= 0x00000020;
}

void Init_CoreTimer_Interrupt_Fuse(void)
{
	register_handler(ik_timer, CORE_TIMER_ISR_FUSE);
	*pIMASK |= 0x00000020;
	
}

EX_INTERRUPT_HANDLER(CORE_TIMER_ISR)
{
	//flashLed();
	//TogglePG_Output(ControlPin);
	//*pPORTGIO_TOGGLE = ControlPin;
	b_Interrupted = true;
	
	//ttt++;
	// Stop read operation, return timeout	
	*pTIMER_DISABLE &= 0x0080;				//Disable Timer 7
	*pTIMER_STATUS &= (~TRUN0);
	
	PulseCounter = 0x00;	
	b_ExTimerInter = true;
	
	*pTCOUNT = u32_PilotCount_9x;
	//*pTSCALE = 0;
	//flashLed();
}

EX_INTERRUPT_HANDLER(CORE_TIMER_ISR_FUSE)
{
	//TogglePG_Output(ControlPin);
	*pPORTGIO_TOGGLE = ControlPin;
	if(!(u32_duration_count_fuse--))
	{
		*pTCNTL &= ~TMREN;				// disable core timer
		*pPORTGIO_CLEAR = ControlPin;		//make sure it start from low
		ssync();
	}
}

//--------------------------------------Counter -----------------------------------------------------------------
//--------------------------------------------------------------------------//
// Function:	Init_Counter												//
//																			//
// Description:	Initialize Counter as Timed Direction Mode 					//
//--------------------------------------------------------------------------//



//------------------------------------ GPIO Control --------------------------------------------------------------
void ConfigPG_Output(u16 pinDefine)
{
	*pPORTG_FER &= ~pinDefine;				//Set as GPIO	
	*pPORTGIO_INEN &= ~pinDefine;			//Input Buffer Disabled	
	*pPORTGIO_CLEAR = pinDefine;
	*pPORTGIO_DIR |= pinDefine;				//Output Direction
	//todo -> Set, Clear or Toggle this Pin
}

void ConfigPG_Input(u16 pinDefine)
{
	*pPORTG_FER &= ~pinDefine;				//Set as GPIO	
	*pPORTGIO_INEN |= pinDefine;			//Input Buffer Enabled	
	//*pPORTGIO_CLEAR = pinDefine;
	*pPORTGIO_DIR &= ~pinDefine;			//Input Direction	
}

void TogglePG_Output(u16 pinDefine)
{
	//ConfigPG_Output(pinDefine);
	*pPORTGIO_TOGGLE = pinDefine;
}


//----------------------------------- Device R/W ------------------------------------------------------------------
//------- pcount:1 pilot ---------
void Start_Op(u32 pcount)
{
	//Config the output pin with a low level
	ConfigPG_Output(ControlPin);
	
	//Config Core Timer
	ConfigCoreTimer(scale_CoreTimer,false);
	Init_CoreTimer_Interrupt();	
	
	//Generate pulse sequence, 1 pilot for low then 1 pilot high
	//Delay_BeforeStart();
	*pPORTGIO_CLEAR = ControlPin;		//make sure it start from low
	b_Interrupted = false;
	*pTCOUNT = pcount;				//Set count value
	*pTCNTL |= TMREN;				//enable core timer
	//ssync();						//sync enable
	while(!b_Interrupted){}			//wait counter done	
	//ssync();						//Sync disable
	b_Interrupted = false;
	*pTCOUNT = pcount;				//Set count value	
	*pTCNTL |= TMREN;				// enable core timer
	//ssync();		
	*pPORTGIO_SET = ControlPin;
	while(!b_Interrupted){}			//wait counter done	
	//ssync();
	*pPORTGIO_CLEAR = ControlPin;		//make sure it start from low
}

//------- pcount:9 pilot ---------
void Stop_Op(u32 pcount)
{
	*pPORTGIO_CLEAR = ControlPin;		//make sure it start from low
	
	b_Interrupted = false;
	*pTCOUNT = pcount;					//Set count value
	*pTCNTL |= TMREN;					// enable core timer
	//ssync();
	while(!b_Interrupted){}
	//ssync();
}

void Device_Addr(u32 pcount1, u32 pcount2, u32 addr, bool read)
{
	u8 num_addr = 7;
	
	addr <<= 1;
	bool flag = addr & u32_mask;
	//*pPORTGIO_CLEAR = ControlPin;		//make sure it start from low
	while(num_addr--)
	{
		if(flag)					//write 1
		{
			addr <<= 1;							//Get the next bit value
			flag = addr & u32_mask;
			// 1 pilot low
			b_Interrupted = false;
			*pTCOUNT = pcount1;					//Set count value
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();
			while(!b_Interrupted){}				//wait counter done
			//ssync();
			//*pPORTGIO_SET = ControlPin;		//set to high; change to set after the second time enalbe operation
			
			// 2 pilot high
			b_Interrupted = false;
			*pTCOUNT = pcount2;					//Set count value
			*pPORTGIO_SET = ControlPin;			//set to high
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();						
			while(!b_Interrupted){}				//wait counter done	
			//ssync();		
			*pPORTGIO_CLEAR = ControlPin;		//set to low
			
						
		}
		else							//write 0
		{
			// 2 pilot low
			b_Interrupted = false;
			*pTCOUNT = pcount2;					//Set count value
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();
			addr <<= 1;							//Get the next bit value
			flag = addr & u32_mask;
			while(!b_Interrupted){}				//wait counter done
			//ssync();
			//*pPORTGIO_SET = ControlPin;		//set to high; change to set after the second time enalbe operation
			
			// 1 pilot high
			b_Interrupted = false;
			*pTCOUNT = pcount1;					//Set count value
			*pPORTGIO_SET = ControlPin;			//set to high
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();			
			while(!b_Interrupted){}				//wait counter done	
			//ssync();		
			*pPORTGIO_CLEAR = ControlPin;			//set to low	
		}	
	}
	
	//------------------ write R/W flag ------------------------------------
	if(read)					//write 1
	{
		// 1 pilot low
		b_Interrupted = false;
		*pTCOUNT = pcount1;					//Set count value
		*pTCNTL |= TMREN;					// enable core timer
		//ssync();
		while(!b_Interrupted){}				//wait counter done
		//ssync();
		//*pPORTGIO_SET = ControlPin;		//set to high; change to set after the second time enalbe operation
		
		// 2 pilot high
		b_Interrupted = false;
		*pTCOUNT = pcount2;					//Set count value
		*pPORTGIO_SET = ControlPin;			//set to high
		*pTCNTL |= TMREN;					// enable core timer
		//ssync();		
		while(!b_Interrupted){}				//wait counter done	
		//ssync();	
		*pPORTGIO_CLEAR = ControlPin;			//set to low
		
		b_Interrupted = false;
		*pTCOUNT = pcount1;					//Set count value
		*pTCNTL |= TMREN;					// enable core timer
		//ssync();			
		while(!b_Interrupted){}				//wait counter done			
	}
	else							//write 0
	{
		// 2 pilot low
		b_Interrupted = false;
		*pTCOUNT = pcount2;					//Set count value
		*pTCNTL |= TMREN;					// enable core timer
		//ssync();
		while(!b_Interrupted){}				//wait counter done
		//ssync();
		//*pPORTGIO_SET = ControlPin;		//set to high; change to set after the second time enalbe operation
		
		// 1 pilot high
		b_Interrupted = false;
		*pTCOUNT = pcount1;					//Set count value
		*pPORTGIO_SET = ControlPin;			//set to high
		*pTCNTL |= TMREN;					// enable core timer
		//ssync();
		while(!b_Interrupted){}				//wait counter done
		//ssync();
		*pPORTGIO_CLEAR = ControlPin;			//set to low
		
		b_Interrupted = false;
		*pTCOUNT = pcount1;					//Set count value
		*pTCNTL |= TMREN;					// enable core timer
		//ssync();			
		while(!b_Interrupted){}				//wait counter done	
	}	
}

void Register_Addr(u32 pcount1, u32 pcount2, u32 addr)
{
	u8 num_addr = 8;
	
	bool flag = addr & u32_mask;
	//*pPORTGIO_CLEAR = ControlPin;		//make sure it start from low
	while(num_addr--)
	{
		if(flag)					//write 1
		{
			addr <<= 1;							//Get the next bit value
			flag = addr & u32_mask;
			// 1 pilot low
			b_Interrupted = false;
			*pTCOUNT = pcount1;					//Set count value
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();
			while(!b_Interrupted){}				//wait counter done
			//ssync();
			//*pPORTGIO_SET = ControlPin;		//set to high; change to set after the second time enalbe operation
			
			// 2 pilot high
			b_Interrupted = false;
			*pTCOUNT = pcount2;					//Set count value
			*pPORTGIO_SET = ControlPin;			//set to high
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();			
			while(!b_Interrupted){}				//wait counter done	
			//ssync();		
			*pPORTGIO_CLEAR = ControlPin;			//set to low	
			
			b_Interrupted = false;
			*pTCOUNT = pcount1;					//Set count value
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();			
			while(!b_Interrupted){}				//wait counter done			
		}
		else							//write 0
		{
			addr <<= 1;							//Get the next bit value
			flag = addr & u32_mask;
			// 2 pilot low
			b_Interrupted = false;
			*pTCOUNT = pcount2;					//Set count value
			*pTCNTL |= TMREN;					// enable core timer	
			//ssync();		
			while(!b_Interrupted){}				//wait counter done
			//ssync();
			//*pPORTGIO_SET = ControlPin;		//set to high; change to set after the second time enalbe operation
			
			// 1 pilot high
			b_Interrupted = false;
			*pTCOUNT = pcount1;					//Set count value
			*pPORTGIO_SET = ControlPin;			//set to high
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();			
			while(!b_Interrupted){}				//wait counter done		
			//ssync();	
			*pPORTGIO_CLEAR = ControlPin;			//set to low
			
			b_Interrupted = false;
			*pTCOUNT = pcount1;					//Set count value
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();			
			while(!b_Interrupted){}				//wait counter done		
		}	
	}	
}

void Write_Reg_Data(u32 pcount1, u32 pcount2, u32 data)
{
	u8 num_addr = 8;
	
	bool flag = data & u32_mask;
	//*pPORTGIO_CLEAR = ControlPin;		//make sure it start from low
	while(num_addr--)
	{
		if(flag)					//write 1
		{
			data <<= 1;							//Get the next bit value
			flag = data & u32_mask;
			// 1 pilot low
			b_Interrupted = false;
			*pTCOUNT = pcount1;					//Set count value
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();
			while(!b_Interrupted){}				//wait counter done
			//ssync();
			//*pPORTGIO_SET = ControlPin;		//set to high; change to set after the second time enalbe operation
			
			// 2 pilot high
			b_Interrupted = false;
			*pTCOUNT = pcount2;					//Set count value
			*pPORTGIO_SET = ControlPin;			//set to high
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();			
			while(!b_Interrupted){}				//wait counter done		
			//ssync();	
			*pPORTGIO_CLEAR = ControlPin;			//set to low			
		}
		else							//write 0
		{
			data <<= 1;							//Get the next bit value
			flag = data & u32_mask;
			// 2 pilot low
			b_Interrupted = false;
			*pTCOUNT = pcount2;					//Set count value
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();			
			while(!b_Interrupted){}				//wait counter done
			//ssync();
			//*pPORTGIO_SET = ControlPin;		//set to high; change to set after the second time enalbe operation
			
			// 1 pilot high
			b_Interrupted = false;
			*pTCOUNT = pcount1;					//Set count value
			*pPORTGIO_SET = ControlPin;			//set to high
			*pTCNTL |= TMREN;					// enable core timer
			//ssync();			
			while(!b_Interrupted){}				//wait counter done	
			//ssync();		
			*pPORTGIO_CLEAR = ControlPin;			//set to low	
		}	
	}	
	
}

void Write_Single(u32 pcount1, u32 pcount2, u32 dev_addr, u32 reg_addr, u32 reg_data)
{
	//Set levelshifter direction
	//ConfigPH_Output(PH0);
	//*pPORTHIO_SET |= PH0;		//write operation, signal from SDP to DUT
	
	
	Start_Op(pcount1);
	Device_Addr(pcount1, pcount2, dev_addr, false);
	Register_Addr(pcount1, pcount2, reg_addr);
	Write_Reg_Data(pcount1, pcount2, reg_data);
	
	Stop_Op(u32_PilotCount_9x);
}

void Write_Burst(u32 pcount1, u32 pcount2, u32 dev_addr, u32 reg_addr, u32 reg_data[], u32 data_num)
{
	//Set levelshifter direction
	//ConfigPH_Output(PH0);
	//*pPORTHIO_SET |= PH0;		//write operation, signal from SDP to DUT
	
	Start_Op(pcount1);
	Device_Addr(pcount1, pcount2, dev_addr, false);
	
	u32 loop_num = data_num;
	Register_Addr(pcount1, pcount2, reg_addr);
	while(loop_num)
	{
		Write_Reg_Data(pcount1, pcount2, reg_data[data_num - loop_num--]);
	}
	
	Stop_Op(u32_PilotCount_9x);
}



u8 Read_Reg_Data_ExTimer(void)		
{
	u8 reg_data = 0;
	
	u8 num_bits = 8;
	u8_ReadBitNum = 8;
	ConfigPG_Input(ControlPin);		//Change the PG port to a Input port
	//enableTimer(0x0080);
	//ConfigPG_Input(PG11);	
	Init_Timer7Interrupts();
	ConfigPulseWidthCounting();	
	//ConfigGpCounter();
	//enableTimer(0x0080);
	
	//*pPORTHIO_CLEAR = PH0;		//Read operation, signal from DUT to SDP
	//ssync();
	
	//while(num_bits--)
	while( num_bits )
	{
		//TogglePH_Output(PH1);
		b_ExTimerInter = false;
		reg_data <<= 1;		
		PulseCounter = 0;
		//enableTimer(0x0080);
		
		//config timeout timer
		ConfigCoreTimer( scale_CoreTimer ,false);				//Scale coretimer to SCLK; no autoload
		Init_CoreTimer_Interrupt();
		*pTCOUNT = (u32_PilotCount_9x);			//Set count value
		
		//*pSIC_IMASK1 = 0x00000080;
		*pTIMER_ENABLE |= 0x0080;		//EnableTimer7();
		ssync();
		
		

		while(!b_ExTimerInter)
		{
			//*pCNT_CONFIG |= (0x0001);				//Enable GP Counter
			//Config Core Timer
			
			//start timeout timer
			*pTCNTL |= TMREN;
			
		}
		
		//Delay_BeforeStart();
		//*pTIMER7_CONFIG = 0x00;
		//*pSIC_IMASK1 &= ~(0x00000080);
		
		//distinguish 0 or 1
		//printf("pulse counter Value is %x\n",PulseCounter);
		//if( PulseCounter > 0x20 )
		{
			num_bits--;
		}
		
		u32_array_temp[num_bits] = PulseCounter;
		if(PulseCounter > u32_Boundary_OneZero)
		{
			reg_data |= 0x01;
		}
		
		//TTT[num_bits] = reg_data;
		
		
	}
	
	//disableTimer(0x0080);
	//*pSIC_IMASK1 = 0x00000000;
	b_ExTimerInter = false;
	*pTCOUNT = (u32_PilotCount_9x/2);			//Set count value	
	//*pSIC_IMASK1 = 0x00000080;
	*pTIMER_ENABLE |= 0x0080;		//EnableTimer7();
	ssync();
	while(!b_ExTimerInter)
	{}
	
	
	ConfigPG_Output(ControlPin);
	
	return reg_data;
}

u32 DistinguishOneZero(u32 count)
{
	if(count < u32_Boundary_OneZero)
		return 0;
	else
		return 1;
}

u8 Read_Single(u32 pcount1, u32 pcount2, u32 dev_addr, u32 reg_addr)
{
	u8 reg_data = 0;
	
	//Set levelshifter direction
	//ConfigPH_Output(PH0);
	//*pPORTHIO_SET |= PH0;		//write operation, signal from SDP to DUT
	ssync();
	
	Start_Op(pcount1);
	Device_Addr(pcount1, pcount2, dev_addr, true);
	Register_Addr(pcount1, pcount2, reg_addr);
	
	//u32 i = 1;
	//while(i--){}
	
	//*pPORTHIO_CLEAR |= PH0;		//Read operation, signal from DUT to SDP
	
	u32 i = 500;
	while(i--){}
	
	//*pPORTHIO_SET |= PH0;
	
	ssync();
	reg_data = Read_Reg_Data_ExTimer();
	
	i = 1;
	while(i--){}
	
	//*pPORTHIO_SET |= PH0;		//After read, write operation, signal from SDP to DUT
	ssync();
	Stop_Op(u32_PilotCount_9x);	
	
	//*pPORTHIO_CLEAR |= PH0;
	ssync();	
	return reg_data;
}


void Read_Burst(u32 pcount1, u32 pcount2, u32 dev_addr, u32 reg_addr, u8 reg_data[], u32 data_num)
{	
	//Set levelshifter direction
	//ConfigPH_Output(PH0);
	//*pPORTHIO_SET |= PH0;		//write operation, signal from SDP to DUT
	
	Start_Op(pcount1);
	Device_Addr(pcount1, pcount2, dev_addr, true);
	Register_Addr(pcount1, pcount2, reg_addr);
	
	//*pPORTHIO_CLEAR |= PH0;		//Read operation, signal from DUT to SDP
	reg_data[0] = Read_Reg_Data_ExTimer();
	
	u32 loop_num = --data_num;
	while(loop_num--)
	{
		//*pPORTHIO_SET |= PH0;		//write operation, signal from SDP to DUT
		Device_Addr(pcount1, pcount2, dev_addr, true);
		//*pPORTHIO_CLEAR |= PH0;		//Read operation, signal from DUT to SDP
		reg_data[data_num - loop_num] = Read_Reg_Data_ExTimer();
	}
	
	//*pPORTHIO_SET |= PH0;		//write operation, signal from SDP to DUT
	Stop_Op(u32_PilotCount_9x);	
}

void Delay_BeforeStart(void)
{
    u16 cnt = 50;
    u16 i = 0;
    for (i=0;i<cnt;i++);
}

bool TriggerByPH5(void)
{
	*pPORTH_FER &= ~PH5;				//Set as GPIO	
	*pPORTHIO_INEN |= PH5;				//Input Buffer Enable	
	//*pPORTHIO_CLEAR = PH5;
	*pPORTHIO_DIR &= ~PH5;				//input Direction
	
	//Delay_BeforeStart();
	//Delay_BeforeStart();
	if(*pPORTHIO & PH5)	
		return true;
	else
		return false;
}
void ConfigPH_Output(u16 pinDefine)
{
	*pPORTH_FER &= ~pinDefine;				//Set as GPIO	
	*pPORTHIO_INEN &= ~pinDefine;			//Input Buffer Disabled	
	*pPORTHIO_SET |= pinDefine;
	*pPORTHIO_DIR |= pinDefine;				//Output Direction
	//todo -> Set, Clear or Toggle this Pin
}


void TogglePH_Output(u16 pinDefine)
{
	//ConfigPG_Output(pinDefine);
	*pPORTHIO_TOGGLE = pinDefine;
}

void SetClkBuffered(SDP_USB_HEADER *pUsbHeader)
{
	if( pUsbHeader->upByteCount == 0)
	{
		*pPORTHIO_CLEAR = PH3;		//BYPASS CLK BUFFER
	}
	else
	{
		*pPORTHIO_SET = PH3;		//BUFFER CLK
	}
}

void SetDataBuffered(SDP_USB_HEADER *pUsbHeader)
{
	if( pUsbHeader->upByteCount == 0)
	{
		*pPORTHIO_CLEAR = PH2;		//BYPASS DATA BUFFER
	}
	else
	{
		*pPORTHIO_SET = PH2;		//BUFFER DATA
	}
}

void SetLr(SDP_USB_HEADER *pUsbHeader)
{
	*pPORTHIO_CLEAR = PH1;		//SET OWCI TO config
	
	if( pUsbHeader->upByteCount == 0)
	{
		*pPORTHIO_CLEAR = PH6;		//SET LR TO LOW
	}
	else
	{
		*pPORTHIO_SET = PH6;		//SET LR TO HIGH
	}
}

void SetOwciPin(SDP_USB_HEADER *pUsbHeader)
{
	if( pUsbHeader->upByteCount == 0)
	{
		*pPORTHIO_SET = PH1;		//SET LR AS OWCI PIN
	}
	else
	{
		*pPORTHIO_CLEAR = PH1;		//SET CONFIG AS OWCI PIN
	}
}








































