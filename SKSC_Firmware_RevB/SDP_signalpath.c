#include "SDP_std_include.h"
//#include "SDP_adc.h"
#include "SDP_signalpath.h"
#include <string.h>

//#define DEBUG

bool bInitGpiosForSignalPath = false;
bool bPowerVolt = false;



//-----------------------------------------------------------------------------
void processSignalPathCmd(SDP_USB_HEADER *pUsbHeader)
{
	switch (pUsbHeader->cmd)
	{			
		case ADI_SDP_CMD_SIGNALPATH_SET:
			setSignalPath(pUsbHeader->numParam);
			break;
			
		case ADI_SDP_CMD_SIGNALPATH_INIT:
			initialGpiosForSignalPath();
			//processSignalPathCmd(0xFF);
			break;
			
		case ADI_SDP_CMD_SIGNALPATH_GROUP:
			multiSiteGroupSelect(pUsbHeader->numParam);
			break;
			
		case ADI_SDP_CMD_SIGNALPATH_SOCKET:
			multiSiteSocketSelect(pUsbHeader->numParam);
			break;
			
			
		default:
			unknownCommand(pUsbHeader);
			break;
	}
}



//-----------------------------------------------------------------------------
void setSignalPath( u32 pathID )
{
	if( !bInitGpiosForSignalPath )
	{
		initialGpiosForSignalPath();
		
		#ifdef DEBUG
		flashLed();
		#endif
	}
	
	//flashLed();
	
	switch ( pathID )
	{
		case SP_VOUT_WITH_CAP:				//0X61
			*pPORTHIO_CLEAR = GPIO_5;
			ssync();
			ssync();
			#ifdef DEBUG
			flashLed();
			#endif
			//adcVoutCap( true );
			break;
		
		case SP_VOUT_WITHOUT_CAP:			//0X62
			*pPORTHIO_SET = GPIO_5;
			ssync();
			ssync();
			//adcVoutCap( false );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_VREF_WITH_CAP:				//0X63
			*pPORTHIO_CLEAR = GPIO_5;
			ssync();
			ssync();
			//adcVrefCap( true );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_VREF_WITHOUT_CAP:			//0X64
			*pPORTHIO_SET = GPIO_5;
			ssync();
			ssync();
			//adcVrefCap( false );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_VIN_TO_VOUT:				//0X65
			//flashLed();
			*pPORTHIO_CLEAR = ( GPIO_2|GPIO_4);
			ssync();
			ssync();
			*pPORTHIO_SET = ( GPIO_3 );
			ssync();
			ssync();
			//*pPORTHIO_CLEAR |= RELAY_2;
			//adcSetVinToVout( true );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_VIN_TO_VREF:				//0X66
			//*pPORTHIO_CLEAR = ( GPIO_3|GPIO_2);
			//ssync();
			//ssync();
			*pPORTHIO_SET = ( GPIO_2 | GPIO_3 | GPIO_4 );
			//flashLed();
			//*pPORTHIO_CLEAR = RELAY_2;
			ssync();
			ssync();
			//*pPORTHIO_SET = RELAY_1;
			//ssync();
			//ssync();
			//adcSetVinToVout( false );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_CONFIG_TO_VOUT:			//0X67
			*pPORTHIO_CLEAR = ( GPIO_3 );
			ssync();
			ssync();
			//*pPORTHIO_SET = ( GPIO_2|GPIO_3 | GPIO_4 );
			//*pPORTHIO_CLEAR = RELAY_1;

			//*pPORTHIO_SET = RELAY_2;
			//ssync();
			//ssync();
			//adcSetConfigToVout( true );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_CONFIG_TO_VREF:			//0X68
			//*pPORTHIO_CLEAR = ( GPIO_2|GPIO_3|GPIO_11 );
			//*pPORTHIO_SET = (RELAY_1|RELAY_2);
			//*pPORTHIO_SET |= RELAY_2;
			//adcSetConfigToVout( false );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
		
		case SP_VDD_FROM_EXT:				//0x69
			*pPORTHIO_SET = GPIO_8;
			ssync();
			ssync();
			//*pPORTHIO_CLEAR = GPIO_7;
			bPowerVolt = true;
			//*pPORTHIO_SET = RELAY_5;
			//adcSetVddFromExt( true );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
		
		case SP_VDD_FROM_5V:
			*pPORTHIO_SET = GPIO_7;
			ssync();
			ssync();
			*pPORTHIO_CLEAR = ( GPIO_6 | GPIO_8);				//0x6A
			bPowerVolt = false;
			ssync();
			ssync();
			//*pPORTHIO_CLEAR = RELAY_5;
			//adcSetVddFromExt( false );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_VDD_POWER_ON:	
			if( bPowerVolt == true )
			{
				*pPORTHIO_SET = GPIO_8;
				ssync();
				//flashLed();
				ssync();
				//*pPORTHIO_CLEAR = GPIO_7 | GPIO_9;
			}
			else if( bPowerVolt == false )
			{
				*pPORTHIO_SET = GPIO_7;
				ssync();
				ssync();
				*pPORTHIO_CLEAR = GPIO_6 | GPIO_8;	
				ssync();
				ssync();	
			}
		
			//*pPORTHIO_SET = RELAY_POWERON;
			//adcSetVddFromExt( false );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_VDD_POWER_OFF:					//0x6C
			*pPORTHIO_CLEAR = GPIO_7 | GPIO_8;
			//*pPORTHIO_SET = GPIO_7;
			ssync();
			ssync();
			//*pPORTHIO_SET = GPIO_8 | GPIO_7 | GPIO_9 ;
			//adcSetVddFromExt( false );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_MODULE_510OUT:
			//*pPORTHIO_SET = GPIO_2 | GPIO_3 | GPIO_11;
			//*pPORTHIO_CLEAR = MODULE_15V_OUT;
			//adcSetVddFromExt( false );
			#ifdef DEBUG
			flashLed();
			#endif
			break;			
			
		case SP_MODULE_AMPOUT:
			//*pPORTHIO_CLEAR = ( GPIO_3|GPIO_11|GPIO_4);
			ssync();
			ssync();
			//*pPORTHIO_SET = ( GPIO_2 );
			//*pPORTHIO_SET = MODULE_15V_OUT;
			//adcSetVddFromExt( false );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
						
		
		case SP_VIN_TO_VCS:
			*pPORTHIO_CLEAR = ( GPIO_4);
			ssync();
			ssync();
			*pPORTHIO_SET = ( GPIO_2 | GPIO_3 );
			ssync();
			ssync();
			//*pPORTHIO_SET = MODULE_15V_OUT;
			//adcSetVddFromExt( false );
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_SET_CURRENT_SENCE:
			//*pPORTHIO_CLEAR = GPIO_1;
			ssync();
			#ifdef DEBUG
			flashLed();
			#endif
			break;	
			
		case SP_BYPASS_CURRENT_SENCE:
			//*pPORTHIO_SET = GPIO_1;
			ssync();
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_VIN_TO_510OUT:
			//*pPORTHIO_CLEAR = GPIO_3;
			ssync();
			ssync();
			//*pPORTHIO_SET = GPIO_2 | GPIO_4 | GPIO_11;
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_VIN_TO_MOUT:
			*pPORTHIO_CLEAR = GPIO_2;
			ssync();
			ssync();
			*pPORTHIO_SET = GPIO_3 | GPIO_4;
			ssync();
			ssync();
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_CONFIG_TO_510OUT:
			//*pPORTHIO_SET = GPIO_2 | GPIO_3 | GPIO_4 | GPIO_11;
			ssync();
			ssync();
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_TRIM_RESULT_PASS:
			*pPORTHIO_SET = GPIO_14;
			ssync();
			ssync();
			#ifdef DEBUG
			flashLed();
			#endif
			break;
			
		case SP_TRIM_RESULT_FAIL:
			*pPORTHIO_CLEAR = GPIO_14;
			ssync();
			ssync();
			#ifdef DEBUG
			flashLed();
			#endif
			break;
					
			
			
		default:
			break;
	} 

}



//-----------------------------------------------------------------------------
void initialGpiosForSignalPath(void)
{
	//portH
	*pPORTH_FER &= ~(GPIO_1|GPIO_2|GPIO_3|GPIO_4|GPIO_5|GPIO_6|GPIO_7|GPIO_8|GPIO_9|GPIO_10|GPIO_13|GPIO_14|GPIO_15|GPIO_16);
	*pPORTHIO_INEN &= ~(GPIO_1|GPIO_2|GPIO_3|GPIO_4|GPIO_5|GPIO_6|GPIO_7|GPIO_8|GPIO_9|GPIO_10|GPIO_13|GPIO_14|GPIO_15|GPIO_16);
	*pPORTHIO_CLEAR |= GPIO_1|GPIO_2|GPIO_3|GPIO_4|GPIO_5|GPIO_6|GPIO_7|GPIO_8|GPIO_9|GPIO_10|GPIO_13|GPIO_14|GPIO_15|GPIO_16;
	//*pPORTHIO_SET |= GPIO_2|GPIO_3|GPIO_4|GPIO_7|GPIO_8|GPIO_9|GPIO_10|GPIO_15;	
	*pPORTHIO_DIR |= (GPIO_1|GPIO_2|GPIO_3|GPIO_4|GPIO_5|GPIO_6|GPIO_7|GPIO_8|GPIO_9|GPIO_10|GPIO_13|GPIO_14|GPIO_15|GPIO_16);
	
	//portF
	*pPORTF_FER &= ~(GPIO_11|GPIO_12|GPIO_17|GPIO_18|GPIO_19|GPIO_20|GPIO_21|GPIO_22|GPIO_23|GPIO_25|GPIO_26|GPIO_27);
	*pPORTFIO_INEN &= ~(GPIO_11|GPIO_12|GPIO_17|GPIO_18|GPIO_19|GPIO_20|GPIO_21|GPIO_22|GPIO_23|GPIO_25|GPIO_26|GPIO_27);
	*pPORTFIO_CLEAR |= GPIO_11|GPIO_12|GPIO_17|GPIO_18|GPIO_19|GPIO_20|GPIO_21|GPIO_22|GPIO_23|GPIO_25|GPIO_26|GPIO_27;
	//*pPORTFIO_SET |= GPIO_2|GPIO_3|GPIO_4|GPIO_7|GPIO_8|GPIO_9|GPIO_10|GPIO_15;	
	*pPORTFIO_DIR |= (GPIO_11|GPIO_12|GPIO_17|GPIO_18|GPIO_19|GPIO_20|GPIO_21|GPIO_22|GPIO_23|GPIO_25|GPIO_26|GPIO_27);
	
	//portG
	*pPORTG_FER &= ~(GPIO_24|GPIO_28);
	*pPORTGIO_INEN &= ~(GPIO_24|GPIO_28);
	*pPORTGIO_CLEAR |= GPIO_24|GPIO_28;
	//*pPORTGIO_SET |= GPIO_24|GPIO_28;	
	*pPORTGIO_DIR |= (GPIO_24|GPIO_28);
	
	
	bInitGpiosForSignalPath = true;
	
}


//-----------------------------------------------------------------------------
void multiSiteGroupSelect( u32 groupID )
{
	switch ( groupID)
	{
		case SP_MULTISITTE_GROUP_A:
			*pPORTHIO_CLEAR = GPIO_10;
			ssync();
			ssync();
			break;
			
		case SP_MULTISITTE_GROUP_B:
			*pPORTHIO_SET = GPIO_10;
			ssync();
			ssync();
			break;
	}
}



//-----------------------------------------------------------------------------
void multiSiteSocketSelect( u32 socketID )
{
	switch ( socketID )
	{
		case 0:
			*pPORTFIO_SET = GPIO_17;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_18 | GPIO_19 | GPIO_22 | GPIO_21;
			ssync();
			ssync();
			break;
			
		case 1:
			*pPORTFIO_SET = GPIO_17 | GPIO_19;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_18 | GPIO_22 | GPIO_21;
			ssync();
			ssync();
			break;
			
		case 2:
			*pPORTFIO_SET = GPIO_17 | GPIO_22;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_18 | GPIO_19 | GPIO_21;
			ssync();
			ssync();
			break;
		
		case 3:
			*pPORTFIO_SET = GPIO_17 | GPIO_19 | GPIO_22;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_18  | GPIO_21;
			ssync();
			ssync();
			break;
			
		case 4:
			*pPORTFIO_SET = GPIO_17 | GPIO_21;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_18 | GPIO_19 | GPIO_22;
			ssync();
			ssync();
			break;
			
		case 5:
			*pPORTFIO_SET = GPIO_17 | GPIO_19 | GPIO_21;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_18 | GPIO_22;
			ssync();
			ssync();
			break;
		
		case 6:
			*pPORTFIO_SET = GPIO_17 | GPIO_22 | GPIO_21;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_18 | GPIO_19;
			ssync();
			ssync();
			break;
			
		case 7:
			*pPORTFIO_SET = GPIO_17 | GPIO_19 | GPIO_22 | GPIO_21;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_18 ;
			ssync();
			ssync();
			break;
			
		case 8:
			*pPORTFIO_SET = GPIO_18;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_17 | GPIO_19 | GPIO_22 | GPIO_21;
			ssync();
			ssync();
			break;
			
		case 9:
			*pPORTFIO_SET = GPIO_18 | GPIO_19;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_17 | GPIO_22 | GPIO_21;
			ssync();
			ssync();
			break;
			
		case 10:
			*pPORTFIO_SET = GPIO_18 | GPIO_22;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_17 | GPIO_19 | GPIO_21;
			ssync();
			ssync();
			break;
		
		case 11:
			*pPORTFIO_SET = GPIO_18 | GPIO_19 | GPIO_22;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_17  | GPIO_21;
			ssync();
			ssync();
			break;
			
		case 12:
			*pPORTFIO_SET = GPIO_18 | GPIO_21;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_17 | GPIO_19 | GPIO_22;
			ssync();
			ssync();
			break;
			
		case 13:
			*pPORTFIO_SET = GPIO_18 | GPIO_19 | GPIO_21;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_17 | GPIO_22;
			ssync();
			ssync();
			break;
		
		case 14:
			*pPORTFIO_SET = GPIO_18 | GPIO_22 | GPIO_21;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_17 | GPIO_19;
			ssync();
			ssync();
			break;
			
		case 15:
			*pPORTFIO_SET = GPIO_18 | GPIO_19 | GPIO_22 | GPIO_21;
			ssync();
			ssync();
			*pPORTFIO_CLEAR = GPIO_17 ;
			ssync();
			ssync();
			break;
	}
}

