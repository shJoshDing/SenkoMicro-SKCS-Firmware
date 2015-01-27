// SDP_usb.h

#ifndef _SDP_USB_H_
#define _SDP_USB_H_

void usbInit(u64 boardInitOtpWord);
				
void usbClose(void);

bool usbCheckNextUsbHeaderAvailable(void);
void usbGetUsbHeader(SDP_USB_HEADER *pUsbHeader);
				
void usbGetBulkData(void *pData, u32 numBytes, bool waitForComplete);
bool usbCheckGetDataFlag(void);
void usbWaitForGetDataFlag(void);

void usbSendBulkData(void *pData, u32 numBytes, bool waitForComplete);
bool usbCheckSendDataFlag(void);
void usbWaitForSendDataFlag(void);
void suspendDevice(void);
bool queryUsbStatus(u32 *status);

enum _USB_STATUS
{
	USB_STATUS_DISCONNECTED,	// Configured, no connection to host
	USB_STATUS_MSG_READY,		// Msg pending
	USB_STATUS_CONNECTED,		// connected, on msg pending
};

#endif
