/*******************************************************************************
* File Name: main.c  
* 
*
* Description:
*  Software application for testing and demo purpouses during development
*  of the PSoC_RFM69 library.
*  For PSOC5LP
*
* Note:
*
********************************************************************************
* Copyright (c) 2015 JesÃºs RodrÃ­guez Cacabelos
* This library is dual licensed under the MIT and GPL licenses.
* http:
*******************************************************************************/

#include <project.h>
#include <stdlib.h>
#include "..\..\..\PSoC_RFM69_Library\PSoC_RFM69.h"

#include "main.h"
#include "master.h"
#include "slave.h"



    /* Data buffer for transmision/reception to/from RFM69 module. */
uint8 rfdatabytes[] = { // 64 bytes.
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',    
};  

    /* Encrypt key. */
uint8 encryptionkey[] = { // 16 bytes.
    0xAB, 0xE4, 0x65, 0x32, 0xBB, 0xCC, 0x13, 0x21,
    0x75, 0x91, 0x49, 0x21, 0xFE, 0xFF, 0xAA, 0x55 
};    

    /* Interrupts flag. */
volatile uint8 rfrxirqflag = 0;

/* ************************************************************************* */
/* *** Function prototypes. *** */

CY_ISR_PROTO(SysTick_Isr);              // interrupt prototype.
CY_ISR_PROTO(RFM69_IsrHandler);

/* ************************************************************************* */

CY_ISR(SysTickIsrHandler)
{
    // decrease timer counter if it is greater than 0.
    if (timercnt > 0) timercnt--;
}

CY_ISR(RFM69_IsrHandler)
{
    /* Clear pending Interrupt */
    RFM_isr_ClearPending();
    //RFM_INT_ClearInterrupt();
    
    if (RFM69_GetIRQFlags() & IRQ_RX) rfrxirqflag = 1;
}


/* ************************************************************************* */

int main()
{
    uint8 loop;  
    	/* *** Start SPI bus. *** */
    SPI_Start();    
    SPI_SS_Write(1);  
    /* *** Turn off leds at startup. *** */
    Led_Red_Write(1);
    Led_Green_Write(1);
    Led_Blue_Write(1);
    
    /* *** Start serial port. *** */
    UART_Start();
    UART_PutString("\nPSoC RFM69 Test... PSoC 5LP...\n");
    
     
    
    /* *** Blink Red Led two times before starting RFM69 module. * ***/
    Led_Red_Write(0); CyDelay(250); Led_Red_Write(1); CyDelay(250);
    Led_Red_Write(0); CyDelay(250); Led_Red_Write(1); CyDelay(250);
    
    /* *** Start RFM69 module. Configure it. If the module is not found, program gets locked. 
           And red led gets turned on. If can find RFM module, then turn on green led. *** */
    UART_PutString("Looking for RFM69 module... ");
    
	if (RFM69_Start() != 1) 
    { 
        UART_PutString("FAILED\n\n");
        Led_Red_Write(0);
        while (1) {}; 
        
    }
    else 
    {
        UART_PutString("OK\n\n");
        Led_Green_Write(1);   
    }
    
    /* If testing with interrupts. But disabled until entering RX mode. */
    #ifdef TEST_USING_INTERRUPTS
        
        RFM_isr_StartEx(RFM69_IsrHandler);
        
    #endif    
    
    /* Start SysTick.
       When compiled for "MASTER" this is used to control timeout while in reception state. */
    CySysTickStart();   // interrupt every 1ms.
    
    /* Find unused callback slot. */
    for (loop = 0; loop < CY_SYS_SYST_NUM_OF_CALLBACKS; ++loop)
    {
        if (CySysTickGetCallback(loop) == NULL)
        {
            /* Set callback */
            CySysTickSetCallback(loop, SysTickIsrHandler);
            break;
        }
    }
    
    /* ----------------------------------------- */
    /* Configuration depending on node type.     */
    #ifdef COMPILE_FOR_MASTER
        Config_ForMaster();
    #endif

    #if defined COMPILE_FOR_SLAVE_1 || defined COMPILE_FOR_SLAVE_2
        Config_ForSlave();
    #endif
    /* ----------------------------------------- */    
    
    /* Uncomment next line and adjust params if you want to change bitrate at runtime. */
    RFM69_SetBitrateCls(BITRATE_MSB_9600, BITRATE_LSB_9600);
    
    /* If testing using encryption, change encryption mode and key at runtime. */
    #ifdef TEST_WITH_ENCRYPTION
        RFM69_Encryption(1, encryptionkey);
    #endif
   
    CyGlobalIntEnable; /* Enable global interrupts. */

    for(;;)
    {
        
        /* ----------------------------------------- */ 
        /* Call main loop depending on node type.    */
        #ifdef COMPILE_FOR_MASTER
                Loop_Master();
        #endif

        #if defined COMPILE_FOR_SLAVE_1 || defined COMPILE_FOR_SLAVE_2
                Loop_Slave();
        #endif
        /* ----------------------------------------- */

    }
}

