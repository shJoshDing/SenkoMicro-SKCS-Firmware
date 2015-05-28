#ifndef _SDP_SIGNALPATH_H_
#define _SDP_SIGNALPATH_H_

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

//PORTH
#define GPIO_1						0x0002
#define GPIO_2						0x0004
#define GPIO_3						0x0008
#define GPIO_4						0x0001
#define GPIO_5						0x0020
#define GPIO_6						0x0040
#define GPIO_7						0x0080
#define GPIO_8						0x0400
#define GPIO_9						0x1000
#define GPIO_10						0x4000
#define GPIO_13						0x8000
#define GPIO_14						0x2000
#define GPIO_15						0x0800
#define GPIO_16						0x0100

//PORTF
#define GPIO_11						0x0200
#define GPIO_12						0x1000
#define GPIO_17						0x0080
#define GPIO_18						0x0040
#define GPIO_19						0x0008
#define GPIO_20						0x0004
#define GPIO_21						0x8000
#define GPIO_22						0x4000
#define GPIO_23						0x2000
#define GPIO_25						0x0800
#define GPIO_26						0x0400
#define GPIO_27						0x0100

//PORTG
#define GPIO_24						0x4000
#define GPIO_28						0x8000





void initialGpiosForSignalPath(void);
void processSignalPathCmd( u32 pathID );


#endif

