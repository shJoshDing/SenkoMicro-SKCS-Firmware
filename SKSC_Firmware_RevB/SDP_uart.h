// SDP_uart.h

#ifndef _SDP_UART_H_
#define _SDP_UART_H_

#include "SDP_std_include.h"

// UART Commands
#define ADI_SDP_CMD_GROUP_UART		0x70000000
#define ADI_SDP_CMD_UART_INIT		0x70000001
#define ADI_SDP_CMD_UART_WRITE		0x70000002
#define ADI_SDP_CMD_UART_READ		0x70000003


#define ADI_SDP_CMD_UART_REMOTE			0x71
#define ADI_SDP_CMD_UART_LOCAL			0x72
#define ADI_SDP_CMD_UART_OUTPUTON		0x73
#define ADI_SDP_CMD_UART_OUTPUTOFF		0x74
#define ADI_SDP_CMD_UART_SETVOLT		0x75
#define ADI_SDP_CMD_UART_SETCURR		0x76

#define HSPYSetCommandLength			11
#define HSPYReadCommandLength			8


void processUartCmd(SDP_USB_HEADER *pUsbHeader);

#endif

