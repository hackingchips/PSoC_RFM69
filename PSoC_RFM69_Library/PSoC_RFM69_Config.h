/*******************************************************************************
* File Name: RFM69_Config.h
* 
*
* Description:
*  PSoC library to control RFM69 radio modules.
*
* Note:
*
********************************************************************************
* Copyright (c) 2015 Jesús Rodríguez Cacabelos
* This library is dual licensed under the MIT and GPL licenses.
* http:
*******************************************************************************/

#if !defined(PSOC_RFM69_CONFIG_H)
#define PSOC_RFM69_CONFIG_H
    
#include "PSoC_RFM69.h"


// Define SS bus pin delay in uS.
#define SS_DELAY        100

#define mSPI_WAIT_TXDONE()      while(0u == (SPI_GetMasterInterruptSource() & SPI_INTR_MASTER_SPI_DONE)) {} \
                                SPI_ClearMasterInterruptSource(SPI_INTR_MASTER_SPI_DONE);

    // ADDRESS_FILTERING:
    //      00 - Do not use address filtering.
    //      01 - Use address filtering.
    //      10 - Use address filtering and broadcast filtering.
#define ADDRESS_FILTERING       0x00
#define NODE_ADDRESS            0x00
#define BROADCAST_ADDRESS       0x00

    // Fixed length, DC-free = none, Crc = 0ff, address filtering = none.
#define PACKETCONFIG1_VALUE     (ADDRESS_FILTERING << 1)

#endif /* PSOC_RFM69_CONFIG_H */    

/* [] END OF FILE */
