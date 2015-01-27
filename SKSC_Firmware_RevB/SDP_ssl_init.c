// SDP_ssl_init.c

/*********************************************************************

This file contains a convenient mechanism to initialize and terminate 
all system services and the device manager.  

The application should then make one call to the function adi_ssl_Init(), 
insuring the return value from the function call returns the value 0.  
This function call initialized all services and the device manager 
according to the sizings defined in adi_ssl_init.h.

When no longer needed, the application can then one call to the function
adi_ssl_Terminate(), insuring the return value from the function call
returns the value 0.  This function call terminates all services and 
the device manager.  

The handles to the DMA and Device Manager are stored in the global 
variables ghDmaManager and ghDevManager, respectively.
These handles can be passed to subsequent adi_dev_Open() calls as
necessary.

DO NOT MODIFY ANYTHING IN THIS FILE

*********************************************************************/

// Include files
#include <services/services.h>		// system service includes
#include <drivers/adi_dev.h>        // device driver includes

#include "SDP_ssl_init.h"       // initialization sizings

/*********************************************************************

Handles

This section provides storage for handles into the services and device 
manager.  The application may use these handles into calls such as
adi_dev_Open() for opening device drivers, adi_dma_OpenChannel() for
opening DMA channels etc.  


*********************************************************************/

extern ADI_DMA_MANAGER_HANDLE ghDmaManager;   // handle to the DMA manager
extern ADI_DEV_MANAGER_HANDLE ghDevManager;   // handle to the device manager
	

/*********************************************************************

Global storage data

This section provides memory, based on the sizing defined above, for 
each of the services. 

*********************************************************************/

static  u8  InterruptServiceData        [ADI_INT_SECONDARY_MEMORY * ADI_SSL_INT_NUM_SECONDARY_HANDLERS];
static  u8  DeferredCallbackServiceData [ADI_DCB_QUEUE_SIZE * ADI_SSL_DCB_NUM_SERVERS];
static  u8  DMAServiceData              [ADI_DMA_BASE_MEMORY + (ADI_DMA_CHANNEL_MEMORY *  ADI_SSL_DMA_NUM_CHANNELS)];
static  u8  FlagServiceData             [ADI_FLAG_CALLBACK_MEMORY *  ADI_SSL_FLAG_NUM_CALLBACKS];
static  u8  SemaphoreServiceData        [ADI_SEM_SEMAPHORE_MEMORY *  ADI_SSL_SEM_NUM_SEMAPHORES];
static  u8  DevMgrData                  [ADI_DEV_BASE_MEMORY + (ADI_DEV_DEVICE_MEMORY * ADI_SSL_DEV_NUM_DEVICES)];

/*********************************************************************

	Function:		adi_ssl_Init

	Description:	Initializes the system services and device manager.


*********************************************************************/

u32 adiSslInit(u64 boardInitOtpWord)
{    
    u32 i;
    u32 Result;
 
    // EBIU and Power configuration has been removed 
    // It is already configured in the initcode file
    		
	// initialize everything but exit upon the first error
	do {
	
        // initialize the interrupt manager
        if ((Result = adi_int_Init(InterruptServiceData, sizeof(InterruptServiceData), &i, ADI_SSL_ENTER_CRITICAL)) != ADI_INT_RESULT_SUCCESS) {
    	    break;
        }
    
	    // initialize the EBIU should be done here in case of need
	    // ...
	  		
        // initialize power should be done here in case of need
        // ...
      
		
        // initialize port control
        if ((Result = adi_ports_Init(ADI_SSL_ENTER_CRITICAL)) != ADI_PORTS_RESULT_SUCCESS) {
        	break;
        }

        // initialize deferred callback
        if ((Result = adi_dcb_Init(DeferredCallbackServiceData, sizeof(DeferredCallbackServiceData), &i, ADI_SSL_ENTER_CRITICAL)) != ADI_DCB_RESULT_SUCCESS) {
       	    break;
        }

   	    // initialize the dma manager
        if ((Result = adi_dma_Init(DMAServiceData, sizeof(DMAServiceData), &i, &ghDmaManager, ADI_SSL_ENTER_CRITICAL)) != ADI_DMA_RESULT_SUCCESS) {
       		break;
   	    }
	
       	// initialize the flag manager
    	if ((Result = adi_flag_Init(FlagServiceData, sizeof(FlagServiceData), &i, ADI_SSL_ENTER_CRITICAL)) != ADI_FLAG_RESULT_SUCCESS) {
		    break;
	    }
	
   	    // initialize the timer manager
	    if ((Result = adi_tmr_Init(ADI_SSL_ENTER_CRITICAL)) != ADI_TMR_RESULT_SUCCESS) {
    		break;
    	}
	
    	// initialize the device manager
  	    if ((Result = adi_dev_Init(DevMgrData, sizeof(DevMgrData), &i, &ghDevManager, ADI_SSL_ENTER_CRITICAL)) != ADI_DEV_RESULT_SUCCESS) {
    		break;
	    }
	    
	 // WHILE (no errors or 1 pass complete)
	} while (0);
	
    return (Result);
}



/*************************************************************************

	Function:		sdpSslUpdateSdramPll

	Description:	Update SDRAM and PLL settings to maximize performance

**************************************************************************/

u32 sdpSslUpdateSdramPll(void)
{
	u32 Result;
	
	
	do
	{	
	    // initialize the EBIU should be done here in case of need
	
	    ADI_EBIU_TIMING_VALUE     twrmin       = {1,{7500, ADI_EBIU_TIMING_UNIT_PICOSEC}};   // set min TWR to 1 SCLK cycle + 7.5ns	
		ADI_EBIU_TIMING_VALUE     refresh      = {8192,{64, ADI_EBIU_TIMING_UNIT_MILLISEC}}; // set refresh period to 8192 cycles in 64ms
		ADI_EBIU_TIME             trasmin      = {44, ADI_EBIU_TIMING_UNIT_NANOSEC};         // set min TRAS to 44ns
		ADI_EBIU_TIME             trpmin       = {20, ADI_EBIU_TIMING_UNIT_NANOSEC};	     // set min TRP to 20ns
		ADI_EBIU_TIME             trcdmin      = {20, ADI_EBIU_TIMING_UNIT_NANOSEC}; 	     // set min TRCD to 20ns
		u32                       cl_threshold = 100;                                        // set cl threshold to 100 Mhz
		
		//default to 32MB SDRAM changed below depending on boardInitOtpWord
		ADI_EBIU_SDRAM_BANK_VALUE bank_size    = {0, {size: ADI_EBIU_SDRAM_BANK_32MB }};     // set bank size to 32MB
		ADI_EBIU_SDRAM_BANK_VALUE bank_width   = {0, {width: ADI_EBIU_SDRAM_BANK_COL_9BIT}}; // set column address width to 9-Bit

	    
	    // definitions for asynch memory controller commands
	   
	     ADI_EBIU_ASYNCH_CLKOUT clkout_enable = ADI_EBIU_ASYNCH_CLKOUT_ENABLE;				// clkout enable;
	    ADI_EBIU_ASYNCH_BANK_ENABLE banks_enable = ADI_EBIU_ASYNCH_BANK0_1_2_3;				// which banks to enable;
	    
	    
	    //  global control register fields 
	    
	    // bank timing parameters - specified in cycles (transition times for the four banks)
	    ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank_trans_time = {ADI_EBIU_BANK_ALL,{ADI_EBIU_ASYNCH_TT_4_CYCLES,{0,ADI_EBIU_TIMING_UNIT_NANOSEC}}};;
		
		// time between Read Enable assertion to de-assertion
	    ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank_read_access_time = {ADI_EBIU_BANK_ALL,{0xB,{0,ADI_EBIU_TIMING_UNIT_NANOSEC}}};;
	    
	   	// time between Write Enable  assertion to de-assertion 
		ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank_write_access_time = {ADI_EBIU_BANK_ALL,{7,{0,ADI_EBIU_TIMING_UNIT_NANOSEC}}};
  	
		// time from beginning of memory cycle to R/W-enable 
		ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank_setup_time = {ADI_EBIU_BANK_ALL,{ADI_EBIU_ASYNCH_ST_3_CYCLES,{0,ADI_EBIU_TIMING_UNIT_NANOSEC}}};
   		
		// time from de-assertion  to end of memory cycle 
		ADI_EBIU_ASYNCH_BANK_TIMING asynch_bank_hold_time = {ADI_EBIU_BANK_ALL,{ADI_EBIU_ASYNCH_HT_2_CYCLES,{0,ADI_EBIU_TIMING_UNIT_NANOSEC}}};
		
		// specify whether ARDY enabled (is used to insert extra wait states) 
		ADI_EBIU_ASYNCH_BANK_VALUE asynch_bank_ardy_enable = {ADI_EBIU_BANK_ALL,{ardy_enable:ADI_EBIU_ASYNCH_ARDY_DISABLE}};
    	
		// specify whether ARDY is sampled low or high 
		ADI_EBIU_ASYNCH_BANK_VALUE asynch_bank_ardy_polarity = {ADI_EBIU_BANK_ALL,{ardy_polarity:ADI_EBIU_ASYNCH_ARDY_POLARITY_LOW}};

	    
		ADI_EBIU_COMMAND_PAIR ezkit_ram[] = {	
			{ ADI_EBIU_CMD_SET_SDRAM_BANK_SIZE,     (void*)&bank_size   },
	       	{ ADI_EBIU_CMD_SET_SDRAM_BANK_COL_WIDTH,(void*)&bank_width  },
	       	{ ADI_EBIU_CMD_SET_SDRAM_CL_THRESHOLD,  (void*)cl_threshold },
	      	{ ADI_EBIU_CMD_SET_SDRAM_TRASMIN,       (void*)&trasmin     }, 
	       	{ ADI_EBIU_CMD_SET_SDRAM_TRPMIN,        (void*)&trpmin      }, 
	       	{ ADI_EBIU_CMD_SET_SDRAM_TRCDMIN,       (void*)&trcdmin     }, 
	       	{ ADI_EBIU_CMD_SET_SDRAM_TWRMIN,        (void*)&twrmin      },
	       	{ ADI_EBIU_CMD_SET_SDRAM_REFRESH,       (void*)&refresh     },
		    /* Asynch Commands memory controller commands */ 
		   	{ ADI_EBIU_CMD_SET_ASYNCH_CLKOUT_ENABLE,          (void*)&clkout_enable },
		   	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_ENABLE,            (void*)&banks_enable },                                 
		 	{ ADI_EBIU_CMD_SET_ASYNCH_BANK_TRANSITION_TIME,   (void*)&asynch_bank_trans_time  },
		    { ADI_EBIU_CMD_SET_ASYNCH_BANK_READ_ACCESS_TIME,  (void*)&asynch_bank_read_access_time  }, 
		    { ADI_EBIU_CMD_SET_ASYNCH_BANK_WRITE_ACCESS_TIME, (void*)&asynch_bank_write_access_time  },
		    { ADI_EBIU_CMD_SET_ASYNCH_BANK_SETUP_TIME,        (void*)&asynch_bank_setup_time  }, 
		    { ADI_EBIU_CMD_SET_ASYNCH_BANK_HOLD_TIME,         (void*)&asynch_bank_hold_time  },
		    { ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_ENABLE,       (void*)&asynch_bank_ardy_enable  },  
		    { ADI_EBIU_CMD_SET_ASYNCH_BANK_ARDY_POLARITY,     (void*)&asynch_bank_ardy_polarity },         	
		    { ADI_EBIU_CMD_END,                     0                   }
		};
		
		Result = adi_ebiu_Init(ezkit_ram, 0);
		if ((Result != ADI_EBIU_RESULT_SUCCESS) && (Result != ADI_EBIU_RESULT_ALREADY_INITIALIZED)) {
	    	break;
		}
	
		ADI_PWR_COMMAND_PAIR ezkit_power[] = {	
		  	{ ADI_PWR_CMD_SET_PROC_VARIANT,(void*)ADI_PWR_PROC_BF527SBBC1600  },    /*  600Mhz ADSP-BF527 */
		   	{ ADI_PWR_CMD_SET_PACKAGE,     (void*)ADI_PWR_PACKAGE_MBGA       },     /* in MBGA packaging, as on all EZ-KITS */
		   	{ ADI_PWR_CMD_SET_VDDEXT,      (void*)ADI_PWR_VDDEXT_330         },     /* external voltage supplied to the voltage regulator is 3.3V */
		   	{ ADI_PWR_CMD_SET_CLKIN,       (void*)24                         },     /* the CLKIN frequency 24 MHz */
		   	{ ADI_PWR_CMD_END,              0                                }      /* indicates end of table */
		};
		
		Result = adi_pwr_Init(ezkit_power);

		if ((Result != ADI_PWR_RESULT_SUCCESS) && (Result != ADI_PWR_RESULT_ALREADY_INITIALIZED)) {
		  	    break;
	 	}
	
	 	Result = adi_pwr_SetFreq (600,120,ADI_PWR_DF_OFF);
	 	
	} while (0);
	
	return (Result);
}




/*********************************************************************

	Function:		adi_ssl_Terminate

	Description:	Terminates the system services and device manager

*********************************************************************/

u32 adiSslTerminate(void) {

    u32 Result;
    
	// terminate everything but exit upon the first error
	do {
	
    	// terminate the device manager if needed
	    if ((Result = adi_dev_Terminate(ghDevManager)) != ADI_DEV_RESULT_SUCCESS) {
   		    break;
        }

   	    // terminate the timer manager
	    if ((Result = adi_tmr_Terminate()) != ADI_TMR_RESULT_SUCCESS) {
    		break;
    	}
	
       	// terminate the flag manager
    	if ((Result = adi_flag_Terminate()) != ADI_FLAG_RESULT_SUCCESS) {
		    break;
	    }
	
   	    // terminate the dma manager if needed
        if ((Result = adi_dma_Terminate(ghDmaManager)) != ADI_DMA_RESULT_SUCCESS) {
       		break;
   	    }

   	    // terminate the deferred callback service if needed
        if ((Result = adi_dcb_Terminate()) != ADI_DCB_RESULT_SUCCESS) {
       	    break;
        }

       	// terminate port control
        if ((Result = adi_ports_Terminate()) != ADI_PORTS_RESULT_SUCCESS) {
        	break;
        }

        // terminate power
        if ((Result = adi_pwr_Terminate()) != ADI_PWR_RESULT_SUCCESS) {
	   	    break;
	    }
    
	    // terminate the EBIU
	    if ((Result = adi_ebiu_Terminate()) != ADI_EBIU_RESULT_SUCCESS) {
    		break;
    	}
	
        // terminate the interrupt manager
        if ((Result = adi_int_Terminate()) != ADI_INT_RESULT_SUCCESS) {
    	    break;
        }
    
	 // WHILE (no errors or 1 pass complete)
	 } while (0);
	
    return (Result);
    
}



