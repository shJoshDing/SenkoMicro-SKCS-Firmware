// SDP_std_include.h

#ifndef _SDP_STD_INCLUDE_H_
#define _SDP_STD_INCLUDE_H_

#include <services/services.h>     // system service includes
#include <drivers/adi_dev.h>       // device manager includes

//#define ADP_ASSERT_ENABLE

// pll settings - core clock and system clock (CSEL fixed at 0)
#define MSEL_VALUE 25
#define SSEL_VALUE 5
#define SDP_CRYSTAL_HZ 24000000
#define CORE_CLK_HZ ((SDP_CRYSTAL_HZ)*(MSEL_VALUE))
#define SYSTEM_CLK_HZ ((CORE_CLK_HZ)/(SSEL_VALUE))

// general data buffer size
#define SDP_DATA_BUF_SIZE 65536
#define SDP_NUM_GEN_DATA_BUF 64

// other constants

#define OTP_MAC_ADDRESS_PAGE 0xDF

#define CONNECTOR_A 0
#define CONNECTOR_B 1

#define CORE_TIMER_ENABLE_BITS 0x00000003

#define PIN_PG0 0x0001
#define PIN_PG10 0x0400

// macros

#ifdef ADP_ASSERT_ENABLE
	#define SDP_ASSERT(var) if(var)asm("emuexcpt;");
#else
	#define SDP_ASSERT(var)
#endif

//u8 u8[SDP_DATA_BUF_SIZE];

typedef union _SDP_GENERAL_DATA_BUF
{
	s8 s8[SDP_DATA_BUF_SIZE];
	u8 u8[SDP_DATA_BUF_SIZE];
	s16 s16[SDP_DATA_BUF_SIZE/2];
	u16 u16[SDP_DATA_BUF_SIZE/2];
	s32 s32[SDP_DATA_BUF_SIZE/4];
	u32 u32[SDP_DATA_BUF_SIZE/4];
	s64 s64[SDP_DATA_BUF_SIZE/8];
	u64 u64[SDP_DATA_BUF_SIZE/8];
} SDP_GENERAL_DATA_BUF, *pSDP_GENERAL_DATA_BUF;

typedef struct _SDP_USB_HEADER
{
	u32 cmd;              // command to execute
	u32 downByteCount;    // number of bytes in next transfer down
	u32 upByteCount;      // number of bytes expected in next transfer up
	u32 numParam;         // number of valid parameters in u32ParamArray
	u32 paramArray[124];  // Parameter array
} SDP_USB_HEADER, *pSDP_USB_HEADER;

typedef struct _SDP_VERSION
{
	u16 majorRev;
	u16 minorRev;
	u16 hostRev;
	u16 blackfinRev;
	u8 dateInfo[12];
	u8 timeInfo[8];
	u16 rebootSource;
	u16 flags;
} SDP_VERSION, *pSDP_VERSION;

typedef struct _GUID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
} GUID, *pGUID;

#include "SDP_general_functions.h"
#include "SDP_usb.h"

void closeMainInit(void);

#endif
