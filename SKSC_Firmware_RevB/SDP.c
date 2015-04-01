// SDP.c

#include "SDP_std_include.h"
#include <bfrom.h>
#include "SDP_ssl_init.h"


// optional modules - remove include of header to remove module from build
#include "SDP_adc.h"
#include "SDP_CurrentSensor.h"
#include "SDP_uart.h"

//Base Commands (to be implemented in every Blackfin App)
#define ADI_SDP_CMD_GROUP_BASE				0xCA000000
#define ADI_SDP_CMD_FLASH_LED	 			0xCA000001
#define ADI_SDP_CMD_GET_FW_VERSION			0xCA000002
#define ADI_SDP_CMD_SDRAM_PROGRAM_BOOT		0xCA000003
#define ADI_SDP_CMD_READ_ID_EEPROMS			0xCA000004
#define ADI_SDP_CMD_RESET_BOARD				0xCA000005
#define ADI_SDP_CMD_READ_MAC_ADDRESS		0xCA000006
#define ADI_SDP_CMD_STOP_STREAM             0xCA000007
#define ADI_SDP_CMD_TRIMDUT_INDEX_INCREASE	0xCA000008

#define SDRAM_BASE_ADDRESS ((u8*)0x1800000) 

// global memory
ADI_DMA_MANAGER_HANDLE ghDmaManager;
ADI_DEV_MANAGER_HANDLE ghDevManager;
SDP_GENERAL_DATA_BUF gGeneralDataBuf[SDP_NUM_GEN_DATA_BUF];
ADI_DEV_1D_BUFFER gGeneral1DBufArray[SDP_NUM_GEN_DATA_BUF];

u64 boardInitOtpWord;					   // Better option to make global than call OtpRead so many times
u8  latchValue;
//u32 numberOfTrimedDut = 0;
//u32 *pDutNumberAddress;
//#define TRIMDUT_INDEX_ADDR	0x20300000		//ADDRESS TO STORE THE TRIMED DUT NUMBER

static SDP_VERSION version = {	0x0301,         // major rev
								1,         // minor rev
								570,       // host software rev
								527,       // blackfin software rev		
								__DATE__,  // date of build
								__TIME__,  // time of build
								0xf517,         // reserved
								0 };       // flags
static void mainInit(void);
void closeMainInit(void);
static void processCommand(SDP_USB_HEADER *pUsbHeader);
static void processBaseCmd(SDP_USB_HEADER *pUsbHeader);

extern volatile bool g_bSuspendFlag;
static SDP_USB_HEADER usbHeader;
//-----------------------------------------------------------------------------
int main(void)
{
	
	unsigned long usbStatus;
	
	
	mainInit();
	
	while(1)
	{
		usbGetUsbHeader(&usbHeader);
		processCommand(&usbHeader);		
		asm("nop;");
	}
	

}

//-----------------------------------------------------------------------------
static void mainInit(void)
{
	unsigned int result;
//	u64 boardInitOtpWord;

	//Initiial GPIOs
	initialGpiosForSignalPath();


	*pPORTG_FER = 0x0000;			// To provide compatibility with previous releases
	
	// enable all async banks
	*pEBIU_AMGCTL = 0xFF;			// when ssl ebiu init not in use
	
	// read SDP board type for init from OTP
	result = bfrom_OtpRead( OTP_MAC_ADDRESS_PAGE,
	                        OTP_UPPER_HALF,
	                        &boardInitOtpWord );
	SDP_ASSERT(result)
	
	// initialise all system services, managers etc
	adiSslInit(boardInitOtpWord);
	
	usbInit(boardInitOtpWord);
	
	// enable PG0 as an output driving high (enable peripherals, no flash) or USB configured
	enablePG0();
	
	latchValue = 0x04; // USB Configured
	writeToSdpLatch(latchValue);
		
	// Once the USB have been configured 
	sdpSslUpdateSdramPll();
	
	//Initiial GPIOs
	initialGpiosForSignalPath();
	
}

//-----------------------------------------------------------------------------
void closeMainInit(void)
{
	usbClose();
	adiSslTerminate();
}

//-----------------------------------------------------------------------------
static void processCommand(SDP_USB_HEADER *pUsbHeader)
{
	switch (pUsbHeader->cmd & 0xFFFFFF00)
	{
		case ADI_SDP_CMD_GROUP_ONEWIRE:					//For one wire main
			processOneWireCmd(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_GROUP_ADC:
			processADCCmd(pUsbHeader);
			break;
			
		case ADI_SDP_CMD_GROUP_UART:
			processUartCmd(pUsbHeader);
			break;
			
			
		case ADI_SDP_CMD_GROUP_BASE:
			processBaseCmd(pUsbHeader);
			break;
			
	
		
		default:
			unknownCommand(pUsbHeader);
			break;
	}
}

//-----------------------------------------------------------------------------
static void processBaseCmd(SDP_USB_HEADER *pUsbHeader)
{
	int i;
	unsigned int result;
	
	switch (pUsbHeader->cmd)
	{
		case ADI_SDP_CMD_FLASH_LED:
			flashLed();
			break;
			
		case ADI_SDP_CMD_GET_FW_VERSION:
			#ifdef PSA_RELEASE_BUILD
				version.flags &= ~0x1;
			#else
				version.flags |= 0x1;
			#endif
			for (i=0; i<sizeof(version); i++)
			{
				gGeneralDataBuf[0].u8[i] = ((u8 *)&version)[i];
			}
			usbSendBulkData( &gGeneralDataBuf[0],
			                 pUsbHeader->upByteCount,
			                 true);
			break;
			
		case ADI_SDP_CMD_SDRAM_PROGRAM_BOOT:
			usbGetBulkData( SDRAM_BASE_ADDRESS + pUsbHeader->paramArray[0],
			                pUsbHeader->downByteCount,
			                true);
			// reboot from SDRAM if reboot flag (bit 1) is set
			if (pUsbHeader->paramArray[1] & 0x02)
			{
				closeMainInit();
							
				// disable instruction cache
				*pIMEM_CONTROL = 0x00000001;
				ssync();
				
				// Info needed for initcode booting
				*pSWRST = 0x0005;
				
				// boot
				bfrom_MemBoot(SDRAM_BASE_ADDRESS,0,0,NULL);
			}	
			break;
			
		case ADI_SDP_CMD_TRIMDUT_INDEX_INCREASE:
			//readIdEeproms();
			//numberOfTrimedDut += 1;
//			u32 *pDutNumberAddress;
/*			pDutNumberAddress = 0x20300000;
			numberOfTrimedDut = *pDutNumberAddress;
			numberOfTrimedDut += 2;
			*pDutNumberAddress = numberOfTrimedDut;
*/
			break;
			
		case ADI_SDP_CMD_RESET_BOARD:
			closeMainInit();
			// disable instruction cache
			*pIMEM_CONTROL = 0x00000001;
			ssync();
			// software reset
			*pSWRST = 0x0007;
			ssync();
			*pSWRST = 0x0000;
			ssync();
			asm("RAISE 1;");
			break;
			
		case ADI_SDP_CMD_READ_MAC_ADDRESS:
			result = bfrom_OtpRead( OTP_MAC_ADDRESS_PAGE,
			                        OTP_LOWER_HALF,
			                        &gGeneralDataBuf[0].u64[0] );
			SDP_ASSERT(result)
			usbSendBulkData( &gGeneralDataBuf[0],
			                 pUsbHeader->upByteCount,
			                 true );
			break;
			
		default:
			unknownCommand(pUsbHeader);
			break;
	}
}
