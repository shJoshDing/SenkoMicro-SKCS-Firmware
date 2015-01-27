// SDP_sport.h

#ifndef _SDP_ADC_H_
#define _SDP_ADC_H_

// SPORT Commands
#define ADI_SDP_CMD_GROUP_ADC             	0x60000000
#define ADI_SDP_CMD_ADC_TRANSFER          	0x6000000A
#define ADI_SDP_CMD_ADC_SIGNAL_PATH			0x6000000B
#define ADI_SDP_CMD_ADC_RESET				0x6000000C
#define ADI_SDP_CMD_ADC_SIGPATH_INIT		0x6000000D


#define ADC_VOUT_WITH_CAP				0x61
#define ADC_VOUT_WITHOUT_CAP			0x62
#define ADC_VREF_WITH_CAP				0x63
#define ADC_VREF_WITHOUT_CAP			0x64
#define ADC_VIN_TO_VOUT					0x65
#define ADC_VIN_TO_VREF					0x66
#define ADC_CONFIG_TO_VOUT				0x67
#define ADC_CONFIG_TO_VREF				0x68
#define ADC_VDD_FROM_EXT				0x69
#define ADC_VDD_FROM_5V					0x6A
#define ADC_VDD_POWER_ON				0x6B
#define ADC_VDD_POWER_OFF				0x6C
#define ADC_MODULE_510OUT				0x6D
#define ADC_MODULE_AMPOUT				0x6E
#define ADC_VIN_TO_VCS					0x6F
#define ADC_SET_CURRENT_SENCE			0x70
#define ADC_BYPASS_CURRENT_SENCE		0x71

#define ADC_VIN_TO_510OUT				0x72
#define ADC_VIN_TO_MOUT					0x73
#define ADC_CONFIG_TO_510OUT			0x74
#define ADC_CONFIG_TO_MOUT				0x75
#define ADC_CONFIG_TO_VCS				0x76

#define ADC_TRIM_RESULT_PASS			0x77
#define ADC_TRIM_RESULT_FAIL			0x78


#define GPIO_1						0x0002
#define GPIO_2						0x0004
#define GPIO_3						0x0008
#define GPIO_4						0x0001
#define GPIO_5						0x0020
//#define GPIO_6						0x0040
#define GPIO_7						0x0080
#define GPIO_8						0x0400
#define GPIO_9						0x1000
#define GPIO_10						0x4000
#define GPIO_11						0x0040
#define GPIO_15						0x0800



void processADCCmd(SDP_USB_HEADER *pUsbHeader);
static void captureAdcData(u32 adcConvstFreq, u32 sampleNumbers );
static u16 signalSampleAdc( u32 adcConvstFreq );                                   
static void adcReset(void);
static void initialGPIOsforADC(void);
void initialGpiosForSignalPath(void);
static void adcSignalPath( u32 pathID );

#endif
