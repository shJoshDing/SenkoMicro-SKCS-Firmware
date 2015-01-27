// SDP_general_functions.h

#ifndef _SDP_GENERAL_FUNCTIONS_H_
#define _SDP_GENERAL_FUNCTIONS_H_

void writeToSdpLatch(u8 LatchData);
void flashLed(void);
void invalidTransferCounts(SDP_USB_HEADER *pUsbHeader);
void unknownCommand(SDP_USB_HEADER *pUsbHeader);

void waitMilliSec(u32 msec);
void waitHalfMilliSec(u32 hmsec);
void waitHalfMicroSec(u32 hmsec);

void enablePG0(void);
void enableUserFlash(void);
void disableUserFlash(void);
void peripheralAccessSdpRev0(bool peripherals);
void generateTimer(u32 period, u32 pulse_duration, u8 timerPin, bool polarity);
void disableTimer(u8 timerPin);


#endif
