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
#define ADDRESS_FILTERING       1
#define NODE_ADDRESS            0x00
#define BROADCAST_ADDRESS       0x00
    
    // USE CRC.
    //      0x00 - Do not use CRC 
    //      0x01 - Use CRC.
#define CRC                     0x00
#define CRC_AUTOCLEAR           0x00
    
    
    // TRANSMISSION PACKET LENGTH MODE.
    //      00 - Fixed length.
    //      01 - Variable length.
#define PACKET_LENGTH_MODE      0x00
#define PAYLOAD_LENGTH          64          

    // SYNC
#define SYNC_SIZE               0x08    // size (max = 8)
#define SYNC_BITS_TOLERANCE     0x00 
    
#define SYNC_VALUE_1            0x55
#define SYNC_VALUE_2            0x2D
#define SYNC_VALUE_3            0x4D  
#define SYNC_VALUE_4            0x55
#define SYNC_VALUE_5            0xAA
#define SYNC_VALUE_6            0x55 
#define SYNC_VALUE_7            0xAA
#define SYNC_VALUE_8            0x55    
    
#define POWER_LEVEL             31  // Max 31. Output power = -18dBm + POWER_LEVEL   
    
    // AES Encription.
    //      0x00. No encryption.
    //      0x01. Encryption active.
    // Remember. If using address filtering. Enable it also at the transmiter side to prevent 
    // encryption of the address byte.
#define AES_ENCRYPTION          1
    
#define AES_KEY_1               0xAA
#define AES_KEY_2               0x30
#define AES_KEY_3               0x45
#define AES_KEY_4               0x56
#define AES_KEY_5               0xAF
#define AES_KEY_6               0x33
#define AES_KEY_7               0x47
#define AES_KEY_8               0x36
#define AES_KEY_9               0xA3
#define AES_KEY_10              0x61
#define AES_KEY_11              0x45
#define AES_KEY_12              0x78
#define AES_KEY_13              0xDE
#define AES_KEY_14              0xF1
#define AES_KEY_15              0x1F
#define AES_KEY_16              0x3A    
    
    // bitrate
#define BITRATE_MSB             BITRATE_MSB_1200
#define BITRATE_LSB             BITRATE_LSB_1200
    
    
    	/* Calculate registers value. 
		frequency = frequency_step * registers values.
		Frequency step = 61.03515625 Hz
	*/
    // 433.100Mhz = 7095910,4 = 
    // Frequency
#define FREQUENCY_MSB           0x6C
#define FREQUENCY_MID           0x46
#define FREQUENCY_LSB           0x66
    
    // Frequency deviation
    // Fstep = Fxosc / 2^19 = 32000000 / 524288 = 61,03515625 Hz
    // Fdev = Fstep * FREQDEV
    // 12451hz = 204
#define FREQDEV_MSB             0x00
#define FREQDEV_LSB             0xCC
    
    

#endif /* PSOC_RFM69_CONFIG_H */    

/* [] END OF FILE */
