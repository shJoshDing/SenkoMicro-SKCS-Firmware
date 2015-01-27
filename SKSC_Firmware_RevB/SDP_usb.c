// SDP_usb.c

#include "SDP_std_include.h"
#include <bfrom.h>
#include <drivers/usb/usb_core/adi_usb_core.h>
#include <drivers/usb/controller/otg/adi/hdrc/adi_usb_hdrc.h>
#include <drivers/usb/class/peripheral/vendor_specific/adi/bulkadi/adi_usb_bulkadi.h>
#include "SDP_usb.h"
#include <stdio.h>

#define VR_FREQ       		0x3000
#define VR_USB_WAKEUP 		0x0200
#define VR_CTL_DEFAULT    	0x7070

extern ADI_DMA_MANAGER_HANDLE ghDmaManager;
extern ADI_DEV_MANAGER_HANDLE ghDevManager;

static ADI_DEV_PDD_HANDLE usbDevHandle;
static ADI_DEV_DEVICE_HANDLE usbBulkHandle;
static ADI_DEV_1D_BUFFER getBulk1DBuff;
static ADI_DEV_1D_BUFFER sendBulk1DBuff;
static bool usbConfiguredFlag;
volatile bool g_bSuspendFlag = FALSE;			/* tells us if we got suspend event */
static bool usbHeaderRequested;
static SDP_USB_HEADER nextUsbHeader;

int g_nUSBState = USB_STATUS_DISCONNECTED;


static void usbCallback(void *AppHandle, u32 Event, void *pArg);

//-----------------------------------------------------------------------------
void usbInit(u64 boardInitOtpWord)
{
	u32 result;
	s16 i;
	u64 macAddress;
	char serialNumberString[13];
	u8 serialNumIndex;
	PDEVICE_DESCRIPTOR pDevDesc;
	ADI_ENUM_ENDPOINT_INFO enumEpInfo;
	ADI_USB_APP_EP_INFO usbAppEpInfo[2];
	
	PCONFIG_OBJECT pConfigO;
	
	usbConfiguredFlag = false;
	
	// Initialize USB Core
	adi_usb_CoreInit((void*)&result);

	// Open USB Controller driver
	result = adi_dev_Open(	
		ghDevManager,                    // DevMgr handle
		&ADI_USBDRC_Entrypoint,          // pdd entry point
		0,                               // device instance
		(void*)1,                        // client handle callback identifier
		&usbDevHandle,                   // device handle
		ADI_DEV_DIRECTION_BIDIRECTIONAL, // data direction for this device
		ghDmaManager,                    // handle to DmaMgr for this device
		NULL,                            // handle to deferred callback service
		usbCallback );                   // client's callback function
	SDP_ASSERT(result)

	// Open the vendor specific Bulk USB class driver
	result = adi_dev_Open(
		ghDevManager,                    // DevMgr handle 
		&ADI_USB_VSBulk_Entrypoint,      // pdd entry point 
		0,                               // device instance 
		(void*)1,                        // client handle callback identifier 
		&usbBulkHandle,                  // DevMgr handle for this device 
		ADI_DEV_DIRECTION_BIDIRECTIONAL, // data direction for this device 
		ghDmaManager,                    // handle to DmaMgr for this device 
		NULL,                            // handle to deferred callback service
		usbCallback );                   // client's callback function
	SDP_ASSERT(result)

	// get a pointer to the device descriptor which is maintained 
	// by the usb core 
	pDevDesc = adi_usb_GetDeviceDescriptor();
	SDP_ASSERT(pDevDesc == NULL)
	
	// device descriptor for this device
	//pDevDesc->wIdVendor = 0x0456;
	pDevDesc->wIdVendor = 0x064B;

	// User firmware running on the SDP board B626
	//pDevDesc->wIdProduct = 0xB626;
	pDevDesc->wIdProduct = 0x1208;

	// read MAC address from OTP
/*	bfrom_OtpRead( OTP_MAC_ADDRESS_PAGE,
	               OTP_LOWER_HALF,
	               &macAddress );
	ssync();

	// convert MAC address to a string
	serialNumberString[12] = '\0';
	for (i=11; i>=0; i--)
	{
		serialNumberString[i] = ((char)(macAddress)) & 0x0F;
		if (serialNumberString[i] > 9)
			serialNumberString[i] += 'A'-10;
		else
			serialNumberString[i] += '0';
		macAddress >>= 4;
	}
	
	// insert MAC address string as the USB serial number
	serialNumIndex = adi_usb_CreateString(serialNumberString);
	if ( (serialNumIndex > 0) && (serialNumIndex < USB_MAX_STRINGS) )
		pDevDesc->bISerialNumber = serialNumIndex;
	SDP_ASSERT(!((serialNumIndex > 0) && (serialNumIndex < USB_MAX_STRINGS)))
*/
	// configure the controller handle
	result = adi_dev_Control( usbBulkHandle,
	                          ADI_USB_CMD_CLASS_SET_CONTROLLER_HANDLE,
	                          (void*)usbDevHandle );
	SDP_ASSERT(result)

	// configure the class
	result = adi_dev_Control( usbBulkHandle,
	                          ADI_USB_CMD_CLASS_CONFIGURE,
	                          (void*)0 );
	SDP_ASSERT(result)

	// Usb internal drivers detect three different object (the same values)
	// Considering device id = 1, we get access to object_id[0]	
	adi_usb_GetObjectFromID(1,USB_CONFIGURATION_OBJECT_TYPE,(void**)&pConfigO);
	pConfigO->pConfigDesc->bMaxPower = 0x7D;	// set to 250 mA
	
	// configure the controller mode
	result = adi_dev_Control( usbBulkHandle,
	                          ADI_DEV_CMD_SET_DATAFLOW_METHOD,
	                          (void*)ADI_DEV_MODE_CHAINED );
	SDP_ASSERT(result)

	// enable data flow
	result = adi_dev_Control( usbBulkHandle,
	                          ADI_DEV_CMD_SET_DATAFLOW,
	                          (void*)TRUE );
	SDP_ASSERT(result)

	// enable USB connection with host
	result = adi_dev_Control( usbDevHandle,
	                          ADI_USB_CMD_ENABLE_USB,
	                          (void*)0 );
	SDP_ASSERT(result)

	// wait here until the USB is configured (and plugged in!)
	while (!usbConfiguredFlag);
	
	// Get the endpoint information
	enumEpInfo.pUsbAppEpInfo = &usbAppEpInfo[0];
	enumEpInfo.dwEpTotalEntries 
		= sizeof(usbAppEpInfo)/sizeof(ADI_USB_APP_EP_INFO);
	enumEpInfo.dwEpProcessedEntries = 0;

	result = adi_dev_Control( usbBulkHandle,
	                          ADI_USB_CMD_CLASS_ENUMERATE_ENDPOINTS,
	                          (void*)&enumEpInfo );
	SDP_ASSERT(result)

	// put endpoint IDs in 1D buffer parameters
	if(usbAppEpInfo[0].eDir == USB_EP_IN)
	{
		sendBulk1DBuff.Reserved[BUFFER_RSVD_EP_ADDRESS] 
			= usbAppEpInfo[0].dwEndpointID;
			
		getBulk1DBuff.Reserved[BUFFER_RSVD_EP_ADDRESS] 
			= usbAppEpInfo[1].dwEndpointID;
	}
	else
	{
		getBulk1DBuff.Reserved[BUFFER_RSVD_EP_ADDRESS] 
			= usbAppEpInfo[0].dwEndpointID;
			
		sendBulk1DBuff.Reserved[BUFFER_RSVD_EP_ADDRESS] 
			= usbAppEpInfo[1].dwEndpointID;
	}
	
	// set up the rest of the parameters in bulk 1D buffers
	getBulk1DBuff.ElementWidth = 1;
	getBulk1DBuff.CallbackParameter = &getBulk1DBuff;
	getBulk1DBuff.pNext = NULL;
	getBulk1DBuff.pAdditionalInfo = NULL;
	
	sendBulk1DBuff.ElementWidth = 1;
	sendBulk1DBuff.CallbackParameter = &sendBulk1DBuff;
	sendBulk1DBuff.pNext = NULL;
	sendBulk1DBuff.pAdditionalInfo = NULL;
	
	usbHeaderRequested = false;
}

//-----------------------------------------------------------------------------
void usbClose(void)
{
	adi_dev_Close(usbDevHandle);	
	adi_dev_Close(usbBulkHandle);
}

//-----------------------------------------------------------------------------
bool usbCheckNextUsbHeaderAvailable(void)
{
	if (!usbHeaderRequested)
	{
		usbGetBulkData(&nextUsbHeader, sizeof(nextUsbHeader), false);
		usbHeaderRequested = true;
	}
	return usbCheckGetDataFlag();
}

//-----------------------------------------------------------------------------
void usbGetUsbHeader(SDP_USB_HEADER *pUsbHeader)
{
	u16 i;
	if (usbHeaderRequested)
	{
		while (!usbCheckGetDataFlag());
		pUsbHeader->cmd = nextUsbHeader.cmd;
		pUsbHeader->downByteCount = nextUsbHeader.downByteCount;
		pUsbHeader->upByteCount = nextUsbHeader.upByteCount;
		pUsbHeader->numParam = nextUsbHeader.numParam;
		for (i=0; i<124; i++)
		{
			pUsbHeader->paramArray[i] = nextUsbHeader.paramArray[i];
		}
	}
	else
	{
		usbGetBulkData(pUsbHeader, sizeof(pUsbHeader), true);
	}
	usbHeaderRequested = false;
}

//-----------------------------------------------------------------------------
void usbGetBulkData(void *pData, u32 numBytes, bool waitForComplete)
{
	u32 result;
	
	// Round numBytes up to nearest multiple of 512
	if (numBytes % 512)
		numBytes = ((numBytes / 512) + 1) * 512;
	
	getBulk1DBuff.Data = pData;
	getBulk1DBuff.ElementCount = numBytes;
	getBulk1DBuff.ProcessedFlag = FALSE;
	getBulk1DBuff.ProcessedElementCount = 0;

	result = adi_dev_Read( usbBulkHandle,
	                       ADI_DEV_1D,
	                       (ADI_DEV_BUFFER *)&getBulk1DBuff );
	SDP_ASSERT(result)

	if (waitForComplete)
		usbWaitForGetDataFlag();	
		
	// probably bad place to go	
//	if ( g_bSuspendFlag )
//		suspendDevice();		
	
}

//-----------------------------------------------------------------------------
bool usbCheckGetDataFlag(void)
{
	return getBulk1DBuff.ProcessedFlag;
}

//-----------------------------------------------------------------------------
void usbWaitForGetDataFlag(void)
{
	while (!getBulk1DBuff.ProcessedFlag && usbConfiguredFlag);
	SDP_ASSERT(!usbConfiguredFlag)
	
}

//-----------------------------------------------------------------------------
void usbSendBulkData(void *pData, u32 numBytes, bool waitForComplete)
{
	u32 result;
	
	// Round numBytes up to nearest multiple of 512
	if (numBytes % 512)
		numBytes = ((numBytes / 512) + 1) * 512;
	
	sendBulk1DBuff.Data = pData;
	sendBulk1DBuff.ElementCount = numBytes;
	sendBulk1DBuff.ProcessedFlag = FALSE;
	sendBulk1DBuff.ProcessedElementCount = 0;

	result = adi_dev_Write( usbBulkHandle,
	                        ADI_DEV_1D,
	                        (ADI_DEV_BUFFER *)&sendBulk1DBuff );
 	//printf("adi_dev_write result %x", result);
	SDP_ASSERT(result)

	if (waitForComplete)
		usbWaitForSendDataFlag();
}

//-----------------------------------------------------------------------------
bool usbCheckSendDataFlag(void)
{
	return sendBulk1DBuff.ProcessedFlag;
}

//-----------------------------------------------------------------------------
void usbWaitForSendDataFlag(void)
{
	while (!sendBulk1DBuff.ProcessedFlag && usbConfiguredFlag);
	SDP_ASSERT(!usbConfiguredFlag)
}

//-----------------------------------------------------------------------------
void suspendDevice(void)
{	
	unsigned int new_value = 0; 
	unsigned int IMASK_reg;
	
	// SDP values
	*pPORTG_FER &= 0xFFFE;
	*pPORTGIO_DIR |= 0x1;
	*pPORTGIO_CLEAR = PIN_PG0;
	
	/* configure the class */
	adi_dev_Control(usbBulkHandle, ADI_USB_CMD_HIBERNATE, (void*)0);
       
    *pVR_CTL = VR_CTL_DEFAULT & ( ~VR_FREQ ) & ( ~CLKBUFOE );
    *pVR_CTL = *pVR_CTL | VR_USB_WAKEUP;     /* enable WAKEUP on USB */
	
    ssync();
    
    IMASK_reg = cli();
    idle();  
}


//-----------------------------------------------------------------------------
static void usbCallback(void *AppHandle, u32 Event, void *pArg)
{
	// AppHandle will be handle which was passed into the adi_dev_Open() call
	switch (Event)
	{
		// (mick's endpoint 0 work is in SVNv179 back)
		
		case ADI_USB_EVENT_DATA_TX:
			// Transmit complete event
			// (pArg holds the address of the data buffer processed)
			break;
		
		case ADI_USB_EVENT_DATA_RX:
			// Receive complete event
			// (pArg holds the address of the data buffer processed)
			break;

		case ADI_USB_EVENT_SET_CONFIG:
			// Host called set configuration
			// (pArg holds the configuration number)
			if (0x1 == (u32)pArg)
			{
				usbConfiguredFlag = true;	
				g_nUSBState = USB_STATUS_MSG_READY;
			}
			else
			{
				// USB is disconnected 
				g_nUSBState = USB_STATUS_DISCONNECTED;
				usbConfiguredFlag = false;	
			}			
			break;

		case ADI_USB_EVENT_ROOT_PORT_RESET:
			// Reset signaling detected (pArg is NULL for this event)
			usbConfiguredFlag = false;
			g_nUSBState = USB_STATUS_DISCONNECTED;
			break;

		case ADI_USB_EVENT_VBUS_FALSE:
			// VBUS is below threshold (pArg is NULL for this event)
			usbConfiguredFlag = false;
			g_nUSBState = USB_STATUS_DISCONNECTED;
			break;

		case ADI_USB_EVENT_RESUME:
			// Resume event (pArg is NULL for this event)
			break;

		case ADI_USB_EVENT_SUSPEND:
			// Suspend event, if cable is unplugged this event gets trigged.
			// (pArg is NULL for this event)
			g_bSuspendFlag = true;	
			break;

		case ADI_USB_EVENT_DISCONNECT:
	    	// Disconnet event (pArg is NULL for this event)
			usbConfiguredFlag = false;
			g_nUSBState = USB_STATUS_DISCONNECTED;
			break;
	}
}


//-----------------------------------------------------------------------------
//	Function:		QueryUsbStatus
//	Description:	Return status of USB
bool queryUsbStatus(u32 *status)
{
	*status = g_nUSBState;
	return true;
}




