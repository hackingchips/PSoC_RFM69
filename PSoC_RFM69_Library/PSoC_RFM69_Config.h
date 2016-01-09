/*******************************************************************************
* File Name: PSoC_RFM69_Config.h
*
* Version: ??? (no version yet, it works but in development).
*
* Description:
*  PSoC library to control RFM69 radio modules.
*  This is the configuration file for the library.
*  This are values used at start up to program RFM69 registers.
*  Some of them can be changed at run time using some functions.
*
* Note:
*   - Support only for RFM69 module (not H, high power module).
*   - Only FSK modulation.
*   - Fixed length packet.
*   - No interrupt support.
*   - PSOC4 and PSCO4M supported.
*
*   TODO's:
*       - add support for H module.
*       - add support for variable length packet.
*       - add support for interrupts.
*       - add support for hardware reset.
*       - add support for PSOC5LP.
*
********************************************************************************
* Copyright (c) 2015 - 2016 Jesús Rodríguez Cacabelos
* This library is dual licensed under the MIT and GPL licenses.
* http:
*******************************************************************************/

#if !defined(PSOC_RFM69_CONFIG_H)
#define PSOC_RFM69_CONFIG_H
    
#include "PSoC_RFM69.h"

/*******************************************************************************
*   SPI bus configuration.
*******************************************************************************/
// Define SS bus pin delay in uS. ** will be documented. **
#define SS_DELAY        100

#define mSPI_WAIT_TXDONE()      while(0u == (SPI_GetMasterInterruptSource() & SPI_INTR_MASTER_SPI_DONE)) {} \
                                SPI_ClearMasterInterruptSource(SPI_INTR_MASTER_SPI_DONE);

/*******************************************************************************
*   RFM69 configuration.
*******************************************************************************/    
    
/* *** Packet length mode *** 
    (do not change this. For future use. At this time, only fixed length is supported.)
    
    PACKET_LENGTH_MODE = 0. Fixed length.
    PACKET_LENGTH_MODE = 1. Variable length.
*/    
    
#define PACKET_LENGTH_MODE      0
    
/* *** Payload length. *** 
    
    PAYLOAD_LENGTH  = length of the packed used in communications.
    
    This value can be changed at run time, but this doesn't mean it is the same
    as the variable length packet mode.
    Read datasheet to be sure about differences between fixed length and variable length
    packet modes and waht is the limit of packet length depending if you are using AES 
    encryption and/or address filtering.
*/     
    
#define PAYLOAD_LENGTH          64     
    
/* *** Syncronization bytes ***
    
    SYNC_SIZE               How many bytes used for syncronization (maximun 8).
    SYNC_BITS_TOLERANCE     Number of tolerated bit errors in Sync word.
    
    SYNC_VALUE_1 - SYNC_VALUE_1     Syncronization bytes. 
*/     

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
    
/* *** Carrier frequency. ***
    
    Fxosc = 32Mhz in RFM69 module.
    Frequency_Step = Fxosc / 2^19 = 32000000 / 524288 = 61,03515625 Hz  (datasheet).
    Carrier_Frequency = Frequency_Step * (FREQUENCY_MSB - FREQUENCY_MID - FREQUENCY_LSB)
    Example: 433.100Mhz = 0x6C4666
*/     
#define FREQUENCY_MSB           0x6C
#define FREQUENCY_MID           0x46
#define FREQUENCY_LSB           0x66

/* *** Frequency deviation. ***
    
    Frequency_Deviation = Frequency_Step * (FREQDEV_MSB - FREQDEV_LSB)
    Example: 12451hz = 0x00CC
*/    
#define FREQDEV_MSB             0x00
#define FREQDEV_LSB             0xCC    
    
/* *** Bitrate. ***
*/ 
#define BITRATE_MSB             BITRATE_MSB_1200
#define BITRATE_LSB             BITRATE_LSB_1200   
    
/* *** TX output power. ***
    
    POWER_LEVEL     Power level (minimun 0, maximun 31).
    
    Output power in dBm = -18dBm + POWER_LEVEL. 
*/       
    
#define POWER_LEVEL             31  // Max 31. Output power = -18dBm + POWER_LEVEL     
    
/* *** CRC ***
    
    CRC = 0. Disable CRC check/calculation in RX/TX
    CRC = 1. Enable CRC check/calculation in RX/TX
    
    CRC_AUTOCLEAR = 0. If CRC check fails. Clear FIFO and start new packet reception. No PayloadReady. 
    CRC_AUTOCLEAR = 1. If CRC check fails. Do no clear FIFO. PayloadReady.
*/     
    
#define CRC                     0
#define CRC_AUTOCLEAR           0    
    
/* *** Address filtering ***
    
    ADDRESS_FILTERING = 0x00. Do not use address filtering.
    ADDRESS_FILTERING = 0x01. Use address filtering.
    ADDRESS_FILTERING = 0x10. Use address filtering and broadcast filtering.
    
    NODE_ADDRESS        node address.
    BROADCAST_ADDRESS   broadcast address.
*/     
    
#define ADDRESS_FILTERING       0x00
#define NODE_ADDRESS            0x00
#define BROADCAST_ADDRESS       0x00
   
/* *** AES Encryption. ***
    
    AES_ENCRYPTION = 0. Disable AES encryption.
    AES_ENCRYPTION = 1. Enable AES encryption.
    
    AES_KEY_1 - AES_KEY_16  Encryption key.
    
    ¡¡¡ Remember !!!
    If using AES encryption and address filtering, enable encryption at both sides to prevent 
    encryption of the address byte.
*/      

#define AES_ENCRYPTION          0
    
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

#endif /* PSOC_RFM69_CONFIG_H */    

/* [] END OF FILE */
