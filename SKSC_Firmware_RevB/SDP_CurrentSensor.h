//SDP_OneWire.h
#include "SDP_std_include.h"


//one wire command
#define ADI_SDP_CMD_GROUP_ONEWIRE        		    0x50000000
#define ADI_SDP_CMD_ONEWIER_PW_COUNTING				0X50000001

#define ADI_SDP_CMD_ONEWIER_I2CWRITE_SINGLE			0x50000002	
#define ADI_SDP_CMD_ONEWIER_I2CREAD_SINGLE			0x50000003
#define ADI_SDP_CMD_ONEWIER_I2CWRITE_BURST			0x50000004
#define ADI_SDP_CMD_ONEWIER_I2CREAD_BURST			0x50000005
#define ADI_SDP_CMD_ONEWIER_FUSE_ON					0x50000006
#define ADI_SDP_CMD_ONEWIER_FUSE_OFF				0x50000007
#define ADI_SDP_CMD_ONEWIER_PILOT_WIDTH				0x50000008	
#define ADI_SDP_CMD_ONEWIER_UPDATE_FUSE_PW			0x50000009

#define ADI_SDP_CMD_ONEWIER_CLK_BUFFERED			0x5000000A
#define ADI_SDP_CMD_ONEWIER_DATA_BUFFERED			0x5000000B
#define ADI_SDP_CMD_ONEWIER_SET_LR					0x5000000C
#define ADI_SDP_CMD_ONEWIER_OWCI_PIN				0x5000000D

#define ADI_SDP_CMD_CURRENTSENSOR_ADC_RESET			0x50000010
#define ADI_SDP_CMD_CURRENTSENSOR_SPORTCONFIG		0x50000011
#define ADI_SDP_CMD_CURRENTSENSOR_ADC_CAPTURE		0x50000012

#define ADI_SDP_CMD_ONEWIER_FW_Version				0x5000000E
#define ADI_SDP_CMD_ONEWIER_TEST					0x5000000F

//Definition
void processOneWireCmd(SDP_USB_HEADER *pUsbHeader);

//void processOneWireCmd(void);