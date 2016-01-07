/*******************************************************************************
* File Name: RFM69.c
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

#include <project.h>

#include "PSoC_RFM69.h"
#include "PSoC_RFM69_Config.h"

#define mRFM69_WaitModeReady()  while((RFM69_Register_Read(REG_IRQFLAGS_1) & 0x80) == 0) {}

uint8 RFM69_GetRSSI();
uint8 RFM69_Register_Read(uint8 reg_addr);
void RFM69_Register_Write(uint8 reg_addr, uint8 reg_value);

uint8 RFM69_Start()
{
    uint16 loop;
    
    uint8 RFM69_CONFIG[][2] = {
            /* Start in standby mode. */
        {REG_OPMODE, 0b00000100},
            /* Packed mode, modulation = FSK, no shaping. */
        {REG_DATAMODUL, 0b00000000},
            /* Bitrate set in config. */
        {REG_BITRATEMSB, BITRATE_MSB}, // BITRATE_MSB},	
        {REG_BITRATELSB, BITRATE_LSB}, //BITRATE_LSB},
            /* Frequency deviation fixed in configuration. */
        {REG_FDEVMSB, FREQDEV_MSB},  
        {REG_FDEVLSB, FREQDEV_LSB},
            /* Frequency carrier fixed in configuration. */
        {REG_FRFMSB, FREQUENCY_MSB},
        {REG_FRFMID, FREQUENCY_MID},
        {REG_FRFLSB, FREQUENCY_LSB},
            /* PA = ON. */
        {REG_PALEVEL, 0b10000000 | POWER_LEVEL},
        {REG_OCP, 0b00001111},				// OCP Enabled
        {REG_LNA, 0b10001000},				// Input impedance = 200ohm, gain = internal AGC.  
        {REG_RXBW, 0b01010010},
        {REG_DIOMAPPING_1, 0b01000100},		
            /* Preamble set to 5 bytes. */
        {REG_PREAMBLEMSB, 0b00000000},  
        {REG_PREAMBLELSB, 0b00000101},
            /* Sync. word generation and detection allways enabled. FifoFillCondition = if SyncAddress interrupt ocurs. */
        {REG_SYNCCONFIG, 0b10000000 | ((SYNC_SIZE - 1) << 3) | SYNC_BITS_TOLERANCE},
        {REG_SYNCVALUE_1, SYNC_VALUE_1},
        {REG_SYNCVALUE_2, SYNC_VALUE_2},
        {REG_SYNCVALUE_3, SYNC_VALUE_3},
        {REG_SYNCVALUE_4, SYNC_VALUE_4},
        {REG_SYNCVALUE_5, SYNC_VALUE_5},
        {REG_SYNCVALUE_6, SYNC_VALUE_6},
        {REG_SYNCVALUE_7, SYNC_VALUE_7},
        {REG_SYNCVALUE_8, SYNC_VALUE_8},       
            /* Node address, broadcast address, packetconfig1 and payload length set in config file. */
        {REG_NODEADRS, NODE_ADDRESS},
        {REG_BROADCASTADRS, BROADCAST_ADDRESS},
            /* Packet config 1. Depends on configuration values. */
        {REG_PACKETCONFIG_1, (ADDRESS_FILTERING << 1) | (CRC_AUTOCLEAR << 3) | (CRC << 4) | (PACKET_LENGTH_MODE << 7)},
            /* Payload length. Depends on configuration values. */
        {REG_PAYLOADLENGTH, PAYLOAD_LENGTH},
            /* Start TX when there is at least one byte in FIFO. FIFO threshold set to PAYLOAD_LENGTH - 1. */
        {REG_FIFOTHRESH, 0b10000000 | (PAYLOAD_LENGTH - 1)},  
        {REG_TESTLNA, 0b00011011},			// Normal sensitivity
        {REG_TESTPA_1, 0b01010101},			// Normal mode.
        {REG_TESTPA_2, 0b01110000},			// Normal mode.
        {REG_TESTDAGC, 0b00110000},			// Improved margin.
            /* InterPacketRxDelay. AutoRxRestartOn = 1 = Rx automatically restarted after InterPacketRxDelay. No AES encryption. */
        {REG_PACKETCONFIG_2, 0b00010010 | AES_ENCRYPTION},
            /* AES Keys. */
        {REG_AESKEY_1, AES_KEY_1},
        {REG_AESKEY_2, AES_KEY_2},
        {REG_AESKEY_3, AES_KEY_3},
        {REG_AESKEY_4, AES_KEY_4},
        {REG_AESKEY_5, AES_KEY_5},
        {REG_AESKEY_6, AES_KEY_6},
        {REG_AESKEY_7, AES_KEY_7},
        {REG_AESKEY_8, AES_KEY_8},
        {REG_AESKEY_9, AES_KEY_9},
        {REG_AESKEY_10, AES_KEY_10},
        {REG_AESKEY_11, AES_KEY_11},
        {REG_AESKEY_12, AES_KEY_12},
        {REG_AESKEY_13, AES_KEY_13},
        {REG_AESKEY_14, AES_KEY_14},
        {REG_AESKEY_15, AES_KEY_15},
        {REG_AESKEY_16, AES_KEY_16}
    };
    
    if (!RFM69_CheckPresence()) return 0;
   
    /* Configure the module. */
	for (loop = 0; loop < (sizeof(RFM69_CONFIG) / 2); loop++)    //RFM69_CONFIG[loop][0] != 255; loop++) 
    {
	    RFM69_Register_Write(RFM69_CONFIG[loop][0], RFM69_CONFIG[loop][1]);
	}
    
    /* Start module in standby mode. */
    RFM69_SetMode(OP_MODE_STANDBY);
    
    return 1;
}

uint8 RFM69_CheckPresence() 
{
    /*  Backup AES registry. */
    int aeskey1 = RFM69_Register_Read(0x3E);;

	RFM69_Register_Write(0x3E,0x55);
    if (RFM69_Register_Read(0x3E) != 0x55) return 0;
    
	RFM69_Register_Write(0x3E,0xAA);
    if (RFM69_Register_Read(0x3E) != 0xAA) return 0;
    
	/* Restore previous value. */
	RFM69_Register_Write(0x3E, aeskey1);

	return 1;
}

uint8 RFM69_SetAddressFiltering(uint8 addressfiltering, uint8 nodeaddress, uint8 broadcastaddress)
{
    uint8 registervalue;
    
    if (addressfiltering > 2) return 0;
    
    registervalue = RFM69_Register_Read(REG_PACKETCONFIG_1);
    registervalue = (registervalue & 0xF9) | (addressfiltering << 1);
    
    RFM69_Register_Write(REG_PACKETCONFIG_1, registervalue);
    
    if (addressfiltering != 0)
    {
        RFM69_Register_Write(REG_NODEADRS, nodeaddress);
        RFM69_Register_Write(REG_BROADCASTADRS, broadcastaddress);
    }
    
    return 1;
}

void RFM69_SetPayloadLength(uint8 plength)
{
    RFM69_Register_Write(REG_PAYLOADLENGTH, plength);
}

uint8 RFM69_SetSync(uint8 syncsize, uint8 syncbitstolerance, uint8 *syncvalue)
{
    uint8 loop;
    
    if ((syncsize > 8) || (syncbitstolerance > 7)) return 0;
    
    RFM69_Register_Write(REG_SYNCCONFIG, 0b10000000 | ((syncsize - 1) << 3) | syncbitstolerance);
    
    for (loop = REG_SYNCVALUE_1; loop < (REG_SYNCVALUE_8 + 1); loop++)
    {
        RFM69_Register_Write(loop, *syncvalue);
        syncvalue++;
    }
    
    return 1;
}

uint8 RFM69_Encryption(uint8 setunset, uint8 *aeskey)
{
    uint8 loop;
    
    if (setunset > 1) return 0;
    
        /* Set standby mode. */
	RFM69_SetMode(OP_MODE_STANDBY);
            
    if (setunset == 1)
    {
        for (loop = REG_AESKEY_1; loop < (REG_AESKEY_16 + 1); loop++)
        {
            RFM69_Register_Write(loop, *aeskey);
            aeskey++;
        }
    }

    RFM69_Register_Write(REG_PACKETCONFIG_2, (RFM69_Register_Read(REG_PACKETCONFIG_2) & 0xFE) | setunset);
    
    return 1;
}

void RFM69_SetMode(uint8 mode) 
{
	RFM69_Register_Write(REG_OPMODE, mode);
    mRFM69_WaitModeReady();
}

void RFM69_DataPacket_TX(uint8 *buf, int len) 
{
    /* Set standby mode before writting to FIFO. */
	RFM69_SetMode(OP_MODE_STANDBY);             

    /* Send data to RFM69 FIFO. */
    SPI_ss0_m_Write(0);
    CyDelayUs(SS_DELAY);

    SPI_SpiUartClearTxBuffer();
    SPI_SpiUartWriteTxData(REG_FIFO | 0x80);
    mSPI_WAIT_TXDONE();
    
    SPI_SpiUartClearTxBuffer();
    SPI_SpiUartPutArray(buf, len);
    mSPI_WAIT_TXDONE();

    CyDelayUs(SS_DELAY);
    SPI_ss0_m_Write(1);

	/* Set TX mode. */
    RFM69_Register_Write(REG_PACKETCONFIG_2, (RFM69_Register_Read(REG_PACKETCONFIG_2) & 0xFB) | 0x04);
	RFM69_SetMode(OP_MODE_TX);
    
    /* Wait until data has been sent. */
    while ((RFM69_Register_Read(REG_IRQFLAGS_2) & 0x08) == 0) {};
}


int RFM69_DataPacket_RX(uint8 *buffer, uint8 *rssi) 
{
	uint8 loop, fifolength;
    
    /* Check if there is data ready in FIFO. */
    if (!(RFM69_Register_Read(REG_IRQFLAGS_2) & 0x04)) return 0; // No data. Return 0.
    
    /* Set standby mode. */
    RFM69_SetMode(OP_MODE_STANDBY);
    
    /* Get length of data in FIFO. */
    fifolength = RFM69_Register_Read(REG_PAYLOADLENGTH);

    /* Read data from FIFO. */
    SPI_ss0_m_Write(0);
    CyDelayUs(SS_DELAY);
    
    SPI_SpiUartClearTxBuffer();
    SPI_SpiUartWriteTxData(REG_FIFO);
    mSPI_WAIT_TXDONE();
    
    SPI_SpiUartClearRxBuffer();
    
    for (loop = 0; loop < fifolength; loop++)
    {
        SPI_SpiUartWriteTxData(0x00);
        mSPI_WAIT_TXDONE();
        
    	buffer[loop] = SPI_SpiUartReadRxData();
    }

    CyDelayUs(SS_DELAY);
    SPI_ss0_m_Write(1);
    
    /* Set RX mode. */
    RFM69_Register_Write(REG_PACKETCONFIG_2, (RFM69_Register_Read(REG_PACKETCONFIG_2) & 0xFB) | 0x04);
    RFM69_SetMode(OP_MODE_RX);
        
    /* If pointer to rssi != 0, then read RSSI. */
    if (rssi != 0) *rssi = RFM69_GetRSSI();    

    return fifolength;
}

uint8 RFM69_GetRSSI() 
{
    /* Start RSSI measurement. */
	RFM69_Register_Write(REG_RSSICONFIG, 0x01); 
    
    /* Wait until measurement has finished. */
    while ((RFM69_Register_Read(REG_RSSICONFIG) & 0x02) == 0) {};
    
    return RFM69_Register_Read(REG_RSSIVALUE);
}

uint8 RFM69_GetTemperature() 
{
    uint8 temperature;
    
	/* Get current operation mode. */
	uint8 actualmode = RFM69_Register_Read(REG_OPMODE);

	/* Set standby mode to read temperature. */
	RFM69_SetMode(OP_MODE_STANDBY);
    
    /* Start temperature measurement and wait until finish. */
	RFM69_Register_Write(REG_TEMP_1, 0x08);
    while((RFM69_Register_Read(REG_TEMP_1) & 0x04) != 0) {};

	temperature = RFM69_Register_Read(REG_TEMP_2);

	/* Restore previous mode. */
    RFM69_SetMode(actualmode);

	return temperature;
}

uint8 RFM69_Register_Read(uint8 reg_addr) 
{
    uint8 reg_value;
    
    SPI_ss0_m_Write(0);
    CyDelayUs(SS_DELAY);
    
    SPI_SpiUartClearTxBuffer();
    SPI_SpiUartClearRxBuffer();
    SPI_SpiUartWriteTxData(reg_addr);
    mSPI_WAIT_TXDONE();
    
    SPI_SpiUartClearTxBuffer();
    SPI_SpiUartClearRxBuffer();
    SPI_SpiUartWriteTxData(0xFF);
    mSPI_WAIT_TXDONE();    
    
	reg_value = SPI_SpiUartReadRxData();

    CyDelayUs(SS_DELAY);
    SPI_ss0_m_Write(1);
    
	return reg_value;
}

void RFM69_Register_Write(uint8 reg_addr, uint8 reg_value) 
{
    SPI_ss0_m_Write(0);
    CyDelayUs(SS_DELAY);
    
    SPI_SpiUartClearTxBuffer();
    SPI_SpiUartWriteTxData(reg_addr | 0x80); 
    SPI_SpiUartWriteTxData(reg_value);
    mSPI_WAIT_TXDONE();

    CyDelayUs(SS_DELAY);
    SPI_ss0_m_Write(1);
}



void RFM69_SetBitrate(uint16 bitrate)
{
	uint16 regsvalue;

	/* Calculate registers value. 
		registers value = RFM69_InternalClock / bitrate.
		Internal clock of RFM69 is 32Mhz.
	*/
	regsvalue = 32000000 / bitrate;
    RFM69_SetBitrateCls(regsvalue >> 8, regsvalue);
}

void RFM69_SetBitrateCls(uint8 msb, uint8 lsb)
{
    uint8 actualmode = RFM69_Register_Read(REG_OPMODE);
	
	RFM69_SetMode(OP_MODE_STANDBY);

 	RFM69_Register_Write(REG_BITRATEMSB, msb);
	RFM69_Register_Write(REG_BITRATELSB, lsb);
	
	/* Restore previous mode. */
	RFM69_SetMode(actualmode);
}

void RFM69_SetFrequency(uint32 frequency)
{
	uint32 regsvalue;
	uint8 actualmode = RFM69_Register_Read(REG_OPMODE);
	
	RFM69_SetMode(OP_MODE_STANDBY);

	/* Calculate registers value. 
		frequency = frequency_step * registers values.
		Frequency step = 61.03515625 Hz
	*/
  	regsvalue = frequency / 61.03515625f;

  	RFM69_Register_Write(REG_FRFMSB, regsvalue >> 16);
  	RFM69_Register_Write(REG_FRFMID, regsvalue >> 8);
  	RFM69_Register_Write(REG_FRFLSB, regsvalue);

	/* Restore previous mode. */
	RFM69_SetMode(actualmode);
}

void RFM69_SetFrequencyDeviation(uint16 frequency)
{
	uint16 regsvalue;
	uint8 actualmode = RFM69_Register_Read(REG_OPMODE);
	
	RFM69_SetMode(OP_MODE_STANDBY);
	
	/* Calculate registers value. 
		frequency_dev = frequency_step * registers values.
		Frequency step = 61.03515625 Hz
	*/
  	regsvalue = frequency / 61.03515625f;	

	RFM69_Register_Write(REG_FDEVMSB, regsvalue >> 8);
  	RFM69_Register_Write(REG_FDEVLSB, regsvalue);

	/* Restore previous mode. */
	RFM69_SetMode(actualmode);
}

void RFM69_SetPower(uint8 power)
{
	if (power > 31) power = 31;
	
	RFM69_Register_Write(REG_PALEVEL, (RFM69_Register_Read(REG_PALEVEL) & 0xF0) | power); 
}


