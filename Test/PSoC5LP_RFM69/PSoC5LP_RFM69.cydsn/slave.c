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
#include "main.h"
#include "slave.h"
#include "..\..\..\PSoC_RFM69_Library\PSoC_RFM69.h"

/* ************************************************************************* */

void Config_ForSlave()
{
    /* ----------------------------------------- */ 
    /* Set slave address. At runtime.            */
    
    #ifdef COMPILE_FOR_SLAVE_1
            RFM69_SetAddressFiltering(01, 100, 0);
    #endif
    
    #ifdef COMPILE_FOR_SLAVE_2
            RFM69_SetAddressFiltering(01, 200, 0);
    #endif

    /* ----------------------------------------- */
        
	RFM69_SetMode(OP_MODE_RX);
    TerminalSend_Slave();    
}

/* ************************************************************************* */

void Loop_Slave()
{
	uint8 loop;
    uint8 rssi;
    uint8 frame_len;
    
    /* Testing with RFM69 interrupts? */
    
    #ifdef TEST_USING_INTERRUPTS
        
        if (rfrxirqflag == 1)
        {
            frame_len = RFM69_DataPacket_RX(rfdatabytes, &rssi);
            
            UART_PutString("Using RFM Ints...");
 
    		/* Print bytes 3 to frame_len to verify data integrity. */
    		for (loop = 3; loop < frame_len; loop++)
    		{
    			UART_PutChar(rfdatabytes[loop]);
    			UART_PutChar(',');
    			if ((loop % 16) == 0) UART_PutChar('\n');
    		}
    		UART_PutChar('\n');
    		
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
                    
                    #ifdef COMPILE_FOR_SLAVE_1
                        rfdatabytes[0] = 100;
                    #endif
                    
                    #ifdef COMPILE_FOR_SLAVE_2
                        rfdatabytes[0] = 200;
                    #endif
                    
                    rfdatabytes[1] = rssi;
                    rfdatabytes[2] = RFM69_GetTemperature();

                    RFM69_DataPacket_TX(rfdatabytes, 64);

                    /* Return to RX state. */
                    RFM69_SetMode(OP_MODE_RX);

                }; break;
            }
    		
    		TerminalSend_Slave();
            
            rfrxirqflag = 0;
        }
        
    #else
    
        frame_len = RFM69_DataPacket_RX(rfdatabytes, &rssi);
            
        if (frame_len != 0)
        {
            UART_PutString("Polling...");
            
    		/* Print bytes 3 to frame_len to verify data integrity. */
    		for (loop = 3; loop < frame_len; loop++)
    		{
    			UART_PutChar(rfdatabytes[loop]);
    			UART_PutChar(',');
    			if ((loop % 16) == 0) UART_PutChar('\n');
    		}
    		UART_PutChar('\n');
    		
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
                    
                    #ifdef COMPILE_FOR_SLAVE_1
                        rfdatabytes[0] = 100;
                    #endif
                    
                    #ifdef COMPILE_FOR_SLAVE_2
                        rfdatabytes[0] = 200;
                    #endif
                    
                    rfdatabytes[1] = rssi;
                    rfdatabytes[2] = RFM69_GetTemperature();

                    RFM69_DataPacket_TX(rfdatabytes, 64);

                    /* Return to RX state. */
                    RFM69_SetMode(OP_MODE_RX);

                }; break;
            }
    		
    		TerminalSend_Slave();
        }   
    
    #endif
}

/* ************************************************************************* */

void TerminalSend_Slave()
{
    
    /* ----------------------------------------- */    
    #ifdef COMPILE_FOR_SLAVE_1
        UART_PutString("\nSLAVE_1... Waiting...\n");
    #endif    

    #ifdef COMPILE_FOR_SLAVE_2
        UART_PutString("SLAVE_2... Waiting...\n");     
    #endif      
    /* ----------------------------------------- */

}

/* [] END OF FILE */
