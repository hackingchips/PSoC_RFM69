/*******************************************************************************
* File Name: main.c  
* 
*
* Description:
*  Software application for testing and demo purpouses during development
*  of the PSoC_RFM69 library.
*  For PSOC 4 and PSOC 4M.
*
* Note:
*
********************************************************************************
* Copyright (c) 2015 Jesús Rodríguez Cacabelos
* This library is dual licensed under the MIT and GPL licenses.
* http:
*******************************************************************************/

#include <project.h>
#include <stdlib.h>
#include "..\..\..\PSoC_RFM69_Library\PSoC_RFM69.h"

/* *** Uncomment one line. *** */

    /* For testing. Master doesn´t have an address assigned so it can receive data from all slaves.
       For slaves, edit node address in config header file. Put address 100 for slave 1 and 200 for 
       slave 2. 
    */
#define MASTER
//#define SLAVE_1
//#define SLAVE_2

/* ************************************************************************* */

volatile int16 timercnt = 0;            // used for timming.

uint8 master_state = 0;                 // = 0, waiting serial port action. 
                                        // = 1, waiting response from slave.

uint8 rfdatabytes[] = { // 64 bytes.
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',    
};

/* ************************************************************************* */
/* *** Function prototypes. *** */
void Config_ForMaster();
void Config_ForSlave();
void Loop_Master();
void Loop_Master_WaitingSPort();
void Loop_Master_WaitingSlaveResponse();
void Loop_Slave();
void TerminalSend_Master();
void TerminalSend_Slave();
CY_ISR_PROTO(SysTick_Isr);              // interrupt prototype.

/* ************************************************************************* */

void SysTickIsrHandler(void)
{
    // decrease timer counter if it is greater than 0.
    if (timercnt > 0) timercnt--;
}

/* ************************************************************************* */

int main()
{
    uint8 loop;
    
    /* *** Turn off leds at startup. *** */
    Led_Red_Write(1);
    Led_Green_Write(1);
    Led_Blue_Write(1);
    
    /* *** Start serial port. *** */
    UART_Start();
    UART_UartPutString("\nPSoC RFM69 Test... PSoC 4/4M...\n");
    
	/* *** Start SPI bus. *** */
    SPI_Start();    
    
    (*(reg32 *)SPI_ss0_m__0__HSIOM) = 
        ((*(reg32 *)SPI_ss0_m__0__HSIOM) & (uint32)~SPI_ss0_m__0__HSIOM_MASK) | (uint32)(SPI_HSIOM_GPIO_SEL << SPI_ss0_m__0__HSIOM_SHIFT);
    SPI_ss0_m_Write(1);       
    
    /* *** Blink Red Led two times before starting RFM69 module. * ***/
    Led_Red_Write(0); CyDelay(250); Led_Red_Write(1); CyDelay(250);
    Led_Red_Write(0); CyDelay(250); Led_Red_Write(1); CyDelay(250);
    
    /* *** Start RFM69 module. Configure it. If the module is not found, program gets locked. 
           And red led gets turned on. If can find RFM module, then turn on green led. *** */
    UART_UartPutString("Looking for RFM69 module... ");
    
	if (RFM69_Start() != 1) 
    { 
        UART_UartPutString("FAILED\n\n");
        Led_Red_Write(0);
        //while (1) {}; 
        
    }
    else 
    {
        UART_UartPutString("OK\n\n");
        Led_Green_Write(0);   
    }
    
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
    
#ifdef MASTER
    Config_ForMaster();
#endif

#ifdef SLAVE_1
    Config_ForSlave();
#endif   

#ifdef SLAVE_2
    Config_ForSlave();
#endif   
   
    CyGlobalIntEnable; /* Enable global interrupts. */

    for(;;)
    {
        
#ifdef MASTER
        Loop_Master();
#endif

#ifdef SLAVE_1
        Loop_Slave();
#endif

#ifdef SLAVE_1
        Loop_Slave();
#endif

    }
}

/* ************************************************************************* */

void Config_ForMaster()
{
    RFM69_SetBitrateCls(BITRATE_MSB_9600, BITRATE_LSB_9600);
    TerminalSend_Master();
}

/* ************************************************************************* */

void Config_ForSlave()
{
    RFM69_SetMode(OP_MODE_RX);  
    
    TerminalSend_Slave();  
}

/* ************************************************************************* */

void Loop_Master()
{
    if (master_state == 0) Loop_Master_WaitingSPort();
    else if (master_state == 1) Loop_Master_WaitingSlaveResponse();
}

void Loop_Master_WaitingSPort()
{
    char itoastr[5];
    uint8 tmpbyte;
        
    uint8 uartchar = UART_UartGetChar();
            
    if (uartchar != 0)
    {
        UART_UartPutChar(uartchar);
        UART_UartPutChar('\n');
        
        // Remember, when using address filtering, the node address have to be the first byte
        // of the payload.
        // Slave 1 is node address 100.
        // Slave 2 is node address 200.
        
        switch (uartchar)
        {

            case '1':
            {
                rfdatabytes[0] = 100; 
                rfdatabytes[1] = 1;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;
            
            case '2':
            {
                rfdatabytes[0] = 100;
                rfdatabytes[1] = 2;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;
            
            case '3':
            {
                rfdatabytes[0] = 100;
                rfdatabytes[1] = 3;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;
            
            case '4':
            {
                rfdatabytes[0] = 200;
                rfdatabytes[1] = 1;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;
            
            case '5':
            {
                rfdatabytes[0] = 200;
                rfdatabytes[1] = 2;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;
            
            case '6':
            {
                rfdatabytes[0] = 200;
                rfdatabytes[1] = 3;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;    
            
            case 'a':
            case 'A':
            {
                rfdatabytes[0] = 100;
                rfdatabytes[1] = 0xAB;
                RFM69_DataPacket_TX(rfdatabytes, 64);
                
                /* Enter in RX state until response from slave is received or timeout. 
                   ¡¡¡ Remember !!!
                   At Master we don´t use address filtering, so we can received data from
                   different slaves. */
                UART_UartPutString("Waiting data from Slave 1...");
                RFM69_SetMode(OP_MODE_RX);
                timercnt = 1000;            // set timout = 1000ms = 1s.
                master_state = 1;
            }; break;
            
            case 'b':
            case 'B':
            {
                rfdatabytes[0] = 200;
                rfdatabytes[1] = 0xAB;
                RFM69_DataPacket_TX(rfdatabytes, 64);
                
                /* Enter in RX state until response from slave is received or timeout. 
                   ¡¡¡ Remember !!!
                   At Master we don´t use address filtering, so we can received data from
                   different slaves. 
                   We have to activate address filtering at the master if we use AES
                   encryption to prevent encryption of the address byte.
                */
                UART_UartPutString("Waiting data from Slave 2...");
                RFM69_SetMode(OP_MODE_RX);
                timercnt = 1000;            // set timout = 1000ms = 1s.
                master_state = 1;                        
            }; break;      
            
            case '9':
            {
                tmpbyte = RFM69_GetTemperature();
                itoa(tmpbyte, itoastr, 10);
                UART_UartPutString("Temperature: register raw value = ");
                UART_UartPutString(itoastr);
                UART_UartPutChar('\n');
            }; break;                
                
            case '?': 
            {
                TerminalSend_Master();
            }; break;

        }
        
        if (master_state == 0) UART_UartPutString("Select test :> ");
    }
}

void Loop_Master_WaitingSlaveResponse()
{
    char itoastr[5];
    uint8 rssi;
    
    // Waiting a packet from a slave containing
    // Slave address + RSSI at slave when master data received + temperature.
    uint8 frame_len = RFM69_DataPacket_RX(rfdatabytes, &rssi);

    if (frame_len != 0)
    {
        UART_UartPutString("OK...\n");

        UART_UartPutString("Slave: ");
        itoa(rfdatabytes[0], itoastr, 10);
        UART_UartPutString(itoastr);
        UART_UartPutString(" RSSI: ");
        itoa(rfdatabytes[1], itoastr, 10);
        UART_UartPutString(itoastr);
        UART_UartPutString(" Temperature: ");
        itoa(rfdatabytes[2], itoastr, 10);
        UART_UartPutString(itoastr);
        UART_UartPutString("\nSelect test :> ");
        
        // change to TX state.
        master_state = 0;
    }
    else
    {
        if (timercnt == 0) // timed out?
        {
            // change to TX state.
            master_state = 0; 
            UART_SpiUartClearRxBuffer();    // discard received data while in rx mode.
            UART_UartPutString("TimedOut...\n");
            UART_UartPutString("Select test :> ");
        }
    }     
}

/* ************************************************************************* */

void Loop_Slave()
{
    uint8 loop;
    uint8 rssi;
    uint8 frame_len = RFM69_DataPacket_RX(rfdatabytes, &rssi);
        
    if (frame_len != 0)
    {
		/* Print bytes 3 to frame_len to verify data integrity. */
		for (loop = 3; loop < frame_len; loop++)
		{
			UART_UartPutChar(rfdatabytes[loop]);
			UART_UartPutChar(',');
			if ((loop % 16) == 0) UART_UartPutChar('\n');
		}
		UART_UartPutChar('\n');
        
        // Remember!!! When using address filtering, address is not stripped from received data.
        // Address is stored in FIFO, and it is the first byte in the FIFO.
        switch (rfdatabytes[1])
        {
            case 1: Led_Red_Write(~Led_Red_Read()); break;
            case 2: Led_Green_Write(~Led_Green_Read()); break;
            case 3: Led_Blue_Write(~Led_Blue_Read()); break;
            
            case 0xAB: // Master asking for data.
            {
                /* Set RFM69 module in standby mode to discard new data while
                   processing. */
                RFM69_SetMode(OP_MODE_STANDBY);
                
                /* Will send 3 bytes. Own node address + RSSI + temperature */
                rfdatabytes[0] = 100;   //
                rfdatabytes[1] = rssi;
                rfdatabytes[2] = RFM69_GetTemperature();

                RFM69_DataPacket_TX(rfdatabytes, 64);

                /* Return to RX state. */
                RFM69_SetMode(OP_MODE_RX);

            }; break;
        }
    }   
}

/* ************************************************************************* */

void TerminalSend_Master()
{
    UART_UartPutString("\nPSoC RFM69 Test... PSoC 4/4M... MASTER\n\n");
    UART_UartPutString("Options:\n");    
    UART_UartPutString("\t1 - Slave 1: Toggle red led.\n");
    UART_UartPutString("\t2 - Slave 1: Toggle green led.\n");
    UART_UartPutString("\t3 - Slave 1: Toggle blue led.\n");
    UART_UartPutString("\t4 - Slave 2: Toggle red led.\n");
    UART_UartPutString("\t5 - Slave 2: Toggle green led.\n");
    UART_UartPutString("\t6 - Slave 2: Toggle blue led.\n");
    UART_UartPutString("\tA - Slave 1: Read from...\n");
    UART_UartPutString("\tB - Slave 2: Read from...\n");
    UART_UartPutString("\t9 - Read this (master) temperature.\n");    
    UART_UartPutString("\t? - This menu.\n\n");   
    UART_UartPutString("\nSelect test :> ");    
}

/* ************************************************************************* */

void TerminalSend_Slave()
{
#ifdef SLAVE_1
    UART_UartPutString("SLAVE_1... Waiting...\n");
#endif    

#ifdef SLAVE_2
    UART_UartPutString("SLAVE_2... Waiting...\n");     
#endif      
}

