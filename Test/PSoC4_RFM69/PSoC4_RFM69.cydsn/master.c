/*******************************************************************************
* PSoC_RFM69
*
* Description:
*  Testing software for PSoC RFM69 library.
*
********************************************************************************
* Copyright (c) 2015 - 2016 Jesús Rodríguez Cacabelos
* This library is dual licensed under the MIT and GPL licenses.
* http:
*******************************************************************************/

#include <project.h>
#include <stdlib.h>
#include "main.h"
#include "master.h"
#include "..\..\..\PSoC_RFM69_Library\PSoC_RFM69.h"

uint8 master_state = 0;                 // = 0, waiting serial port action. 
                                        // = 1, waiting response from slave.

/* ************************************************************************* */

void Config_ForMaster()
{
        /* ¡¡¡ Remember !!!
           Master is configured without address checking. This way, it can receive data from
           any slave. Unless using encryption. */
    TerminalSend_Master();
}

/* ************************************************************************* */

void Loop_Master()
{
    if (master_state == 0) Loop_Master_WaitingSPort();
    else if (master_state == 1) Loop_Master_WaitingSlaveResponse();
}

/* ************************************************************************* */

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
                // If using encryption, enable address filtering to avoid 
                // encryption of the byte that contains the address, or the slave will not
                // respond. (more info in RFM datasheet)
                #ifdef TEST_WITH_ENCRYPTION
                    RFM69_SetAddressFiltering(1, 100, 0);
                #endif
                
                rfdatabytes[0] = 100; 
                rfdatabytes[1] = 1;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;
            
            case '2':
            {
                // If using encryption, enable address filtering to avoid 
                // encryption of the byte that contains the address, or the slave will not
                // respond. (more info in RFM datasheet)
                #ifdef TEST_WITH_ENCRYPTION
                    RFM69_SetAddressFiltering(1, 100, 0);
                #endif
                
                rfdatabytes[0] = 100;
                rfdatabytes[1] = 2;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;
            
            case '3':
            {
                // If using encryption, enable address filtering to avoid 
                // encryption of the byte that contains the address, or the slave will not
                // respond. (more info in RFM datasheet)
                #ifdef TEST_WITH_ENCRYPTION
                    RFM69_SetAddressFiltering(1, 100, 0);
                #endif
                
                rfdatabytes[0] = 100;
                rfdatabytes[1] = 3;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;
            
            case '4':
            {
                // If using encryption, enable address filtering to avoid 
                // encryption of the byte that contains the address, or the slave will not
                // respond. (more info in RFM datasheet)
                #ifdef TEST_WITH_ENCRYPTION
                    RFM69_SetAddressFiltering(1, 200, 0);
                #endif
                
                rfdatabytes[0] = 200;
                rfdatabytes[1] = 1;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;
            
            case '5':
            {
                // If using encryption, enable address filtering to avoid 
                // encryption of the byte that contains the address, or the slave will not
                // respond. (more info in RFM datasheet)
                #ifdef TEST_WITH_ENCRYPTION
                    RFM69_SetAddressFiltering(1, 200, 0);
                #endif
                
                rfdatabytes[0] = 200;
                rfdatabytes[1] = 2;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;
            
            case '6':
            {
                // If using encryption, enable address filtering to avoid 
                // encryption of the byte that contains the address, or the slave will not
                // respond. (more info in RFM datasheet)
                #ifdef TEST_WITH_ENCRYPTION
                    RFM69_SetAddressFiltering(1, 200, 0);
                #endif
                
                rfdatabytes[0] = 200;
                rfdatabytes[1] = 3;
                RFM69_DataPacket_TX(rfdatabytes, 64);
            }; break;    
            
            case 'a':
            case 'A':
            {
                // If using encryption, enable address filtering to avoid 
                // encryption of the byte that contains the address, or the slave will not
                // respond. (more info in RFM datasheet)
                #ifdef TEST_WITH_ENCRYPTION
                    RFM69_SetAddressFiltering(1, 100, 0);
                #endif
                
                rfdatabytes[0] = 100;
                rfdatabytes[1] = 0xAB;
                RFM69_DataPacket_TX(rfdatabytes, 64);
                
                /* Enter in RX state until response from slave is received or timeout. 
                   Â¡Â¡Â¡ Remember !!!
                   At Master we donÂ´t use address filtering, so we can received data from
                   different slaves. */
                UART_UartPutString("Waiting data from Slave 1...");
                RFM69_SetMode(OP_MODE_RX);
                timercnt = 1000;            // set timout = 1000ms = 1s.
                master_state = 1;
            }; break;
            
            case 'b':
            case 'B':
            {
                // If using encryption, enable address filtering to avoid 
                // encryption of the byte that contains the address, or the slave will not
                // respond. (more info in RFM datasheet)
                #ifdef TEST_WITH_ENCRYPTION
                    RFM69_SetAddressFiltering(1, 200, 0);
                #endif
                
                rfdatabytes[0] = 200;
                rfdatabytes[1] = 0xAB;
                RFM69_DataPacket_TX(rfdatabytes, 64);
                
                /* Enter in RX state until response from slave is received or timeout. 
                   Â¡Â¡Â¡ Remember !!!
                   At Master we donÂ´t use address filtering, so we can received data from
                   different slaves. */
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

/* ************************************************************************* */

void Loop_Master_WaitingSlaveResponse()
{
    uint8 frame_len = 0;
    char itoastr[5];
    uint8 rssi;
    
    /* Testing with interrupts? */
    #ifdef TEST_USING_INTERRUPTS
        
        if (rfrxirqflag == 1)
        {
            frame_len = RFM69_DataPacket_RX(rfdatabytes, &rssi);

            UART_UartPutString("OK...(using ints)\n");

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
            
            rfrxirqflag = 0;
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
        
    #else
    
        // Waiting a packet from a slave containing
        // Slave address + RSSI at slave when master data received + temperature.
        frame_len = RFM69_DataPacket_RX(rfdatabytes, &rssi);

        if (frame_len != 0)
        {
            UART_UartPutString("OK...(polling rfm module)\n");

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
    
    #endif
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

/* [] END OF FILE */
