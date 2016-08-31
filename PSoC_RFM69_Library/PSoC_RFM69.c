/*******************************************************************************
* File Name: PSoC_RFM69.c
*
* Version: ??? (no version yet, it works but in development). 
*
* Description:
*  PSoC library to control RFM69 radio modules.
*
* Note:
*   - Support only for RFM69 module (not H, high power module).
*   - Only FSK modulation.
*   - Fixed length packet.
*   - PSOC4 and PSOC4M supported.
*
*   TODO's:
*       - add support for H module.
*       - add support for variable length packet.
*       - add support for PSOC5LP.
*
********************************************************************************
* Copyright (c) 2015 - 2016 Jesús Rodríguez Cacabelos
* This library is dual licensed under the MIT and GPL licenses.
* http:
*******************************************************************************/

#include <project.h>

#include "PSoC_RFM69.h"
#include "PSoC_RFM69_Config.h"

#if (CY_PSOC4) 
    /* Macro name : mSPI_WAIT_TXDONE. (PSOC4/4M)
       Description: Wait until SPI Tx have finished.
    */  
    #define mSPI_WAIT_TXDONE()      while(0u == (SPI_GetMasterInterruptSource() & SPI_INTR_MASTER_SPI_DONE)) {} \
                                    SPI_ClearMasterInterruptSource(SPI_INTR_MASTER_SPI_DONE);
#endif

#if (CY_PSOC5LP)
    /* Macro name : mSPI_WAIT_TXDONE. (PSOC%LP)
       Description: Wait until SPI Tx have finished.
    */                                 
    #define mSPI_WAIT_TXDONE()      while (SPI_GetTxBufferSize())    
#endif    

/* Macro name : mRFM69_WaitModeReady.
   Description: Wait until RFM69 module operation mode have changed.
*/   
#define mRFM69_WaitModeReady()  while((RFM69_Register_Read(REG_IRQFLAGS_1) & 0x80) == 0) {}

/*******************************************************************************
*   Function prototypes.
*******************************************************************************/
void RFM69_Set_HMode(uint8 mode);
uint8 RFM69_GetRSSI();
uint8 RFM69_Register_Read(uint8 reg_addr);
void RFM69_Register_Write(uint8 reg_addr, uint8 reg_value);

/*******************************************************************************
* Function Name: RFM69_Start
********************************************************************************
*
* Summary:
*  Initializacion of the RFM69 module internal registers. Some registers have
*  fixed values. Other registers depend on values fixed in configuration
*  header file.
*
* Parameters:
*  none
*
* Return:
*  1 - If initialization sucessful.
*  0 - If can not connect to the RFM69 module.
*
*******************************************************************************/
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
            /* Over current protection enabled. */
        {REG_OCP, 0b00001111},
            /* Input impedance = 200ohm, gain = internal AGC. */
        {REG_LNA, 0b10001000},
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
    
    /* If using hardware reset, do a reset. */
#ifdef mmRESET_PIN
    RFM69_HardwareReset();
#endif    
    
    /* Try to connect to RFM69 module. */
    if (!RFM69_CheckPresence()) return 0;
   
    /* Configure the module. */
	for (loop = 0; loop < (sizeof(RFM69_CONFIG) / 2); loop++)
    {
	    RFM69_Register_Write(RFM69_CONFIG[loop][0], RFM69_CONFIG[loop][1]);
	}
    
    /* Start module in standby mode. */
    RFM69_SetMode(OP_MODE_STANDBY);
    
    return 1;
}

/*******************************************************************************
* Function Name: RFM69_CheckPresence
********************************************************************************
*
* Summary:
*  Look if RFM69 is connected to SPI bus.
*
* Parameters:
*  none
*
* Return:
*  1 - If RFM69 module was found in SPI bus..
*  0 - If can not connect to the RFM69 module.
*
* Note:
*  I took the idea of checking module pressence in the bus from the library of
*  Joe Desbonnet
*  https://github.com/jdesbonnet/RFM69_LPC812_firmware.git
*
*******************************************************************************/
uint8 RFM69_CheckPresence() 
{
    /*  Backup AES registry. */
    int aeskey1 = RFM69_Register_Read(0x3E);;

    /* Write some values to some AES registries and try to reread them. */
	RFM69_Register_Write(0x3E,0x55);
    if (RFM69_Register_Read(0x3E) != 0x55) return 0;
    
	RFM69_Register_Write(0x3E,0xAA);
    if (RFM69_Register_Read(0x3E) != 0xAA) return 0;
    
	/* Restore previous value. */
	RFM69_Register_Write(0x3E, aeskey1);

	return 1;
}

/*******************************************************************************
* Function Name: RFM69_SetMode
********************************************************************************
*
* Summary:
*  Change operating mode of the RFM69 module.
*
* Parameters:
*  mode:        new operating mode.
*
* Return:
*  none
*
*******************************************************************************/
void RFM69_SetMode(uint8 mode) 
{
    /* If operation mode = RX, then set DIO0 for interrupt when PayloadReady.
       If operation mode = TX, then set DIO0 for interrupt when PacketSent. 
    */
    if (mode == OP_MODE_RX)
    {
        #ifdef RFM69_HVARIANT
            RFM69_Set_HMode(0);
        #endif
        
        RFM69_Register_Write(REG_DIOMAPPING_1, 0x40);
    }
    else if (mode == OP_MODE_TX)
    {
        #ifdef RFM69_HVARIANT
            RFM69_Set_HMode(1);
        #endif        
        
        RFM69_Register_Write(REG_DIOMAPPING_1, 0x00);
    }
    
	RFM69_Register_Write(REG_OPMODE, mode);
    mRFM69_WaitModeReady();
}

/*******************************************************************************
* Function Name: RFM69_Set_H_Mode
********************************************************************************
*
* Summary:
*  For high power variant of RFM69 modules.
*  Changes the mode of the module from low power to high power or viceversa.
*
* Parameters:
*  mode:        new operating mode.
*
* Return:
*  none
*
*******************************************************************************/
void RFM69_Set_HMode(uint8 mode)
{
    uint8 regval = RFM69_Register_Read(REG_PALEVEL);
    
    if (mode == 1)      // H mode for H modules
    {
        RFM69_Register_Write(REG_OCP, 0x0F);
        
        regval = (regval & 0x1F) | 0x60;    // turn off PA0, turn on PA1 & PA2
        RFM69_Register_Write(REG_PALEVEL, regval);
        
        RFM69_Register_Write(REG_TESTPA_1, 0x5D);
        RFM69_Register_Write(REG_TESTPA_2, 0x7C);
    }
    else if (mode == 0) // normal modules and RX mode.
    {
        //RFM69_Register_Write(REG_OCP, 0x10);
        
        regval = (regval & 0x1F) | 0x80;    // turn off PA1 & PA2, turn on PA0
        RFM69_Register_Write(REG_PALEVEL, regval);
                
        RFM69_Register_Write(REG_TESTPA_1, 0x55);
        RFM69_Register_Write(REG_TESTPA_2, 0x70);
    }    
}

/*******************************************************************************
* Function Name: RFM69_SetPayloadLength
********************************************************************************
*
* Summary:
*  Change the length of payload.
*
* Parameters:
*  plength:     new payload length.
*
* Return:
*  none
*
*******************************************************************************/
void RFM69_SetPayloadLength(uint8 plength)
{
    RFM69_Register_Write(REG_PAYLOADLENGTH, plength);
}

/*******************************************************************************
* Function Name: RFM69_SetSync
********************************************************************************
*
* Summary:
*  Set how many bytes will be used as syncronization bytes and what is the 
*  error tolerance in bits when checking syncronization.
*
* Parameters:
*  syncsize:            number of syncronizatino bytes used (max 8 bytes).
*  syncbitstolerance:   erro tolerance in bits (max 7 bits).
*  syncvalue:           pointer to uint8 array with bytes values used as
*                       syncronization bytes. Length of the array have to be
*                       same as 'synsize' value.
*
* Return:
*  1 - If succesful.
*  2 - If 'synsize' > 8 or 'synbitstolerance' > 7
*
*******************************************************************************/
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

/*******************************************************************************
* Function Name: RFM69_SetFrequency
********************************************************************************
*
* Summary:
*  Set value of carrier frequency.
*
* Parameters:
*  frequency:           value of carrier frequency in hz.
*
* Return:
*  none
*
*******************************************************************************/
void RFM69_SetFrequency(uint32 frequency)
{
	uint32 regsvalue;
	uint8 actualmode = RFM69_Register_Read(REG_OPMODE);
	
    /* Set standby mode. */
	RFM69_SetMode(OP_MODE_STANDBY);

  	regsvalue = frequency / 61.03515625f;

  	RFM69_Register_Write(REG_FRFMSB, regsvalue >> 16);
  	RFM69_Register_Write(REG_FRFMID, regsvalue >> 8);
  	RFM69_Register_Write(REG_FRFLSB, regsvalue);

	/* Restore previous mode. */
	RFM69_SetMode(actualmode);
}

/*******************************************************************************
* Function Name: RFM69_SetFrequencyDeviation
********************************************************************************
*
* Summary:
*  Set deviation frequency value.
*
* Parameters:
*  frequency:           value of frequency in hz.
*
* Return:
*  none
*
*******************************************************************************/
void RFM69_SetFrequencyDeviation(uint16 frequency)
{
	uint16 regsvalue;
	uint8 actualmode = RFM69_Register_Read(REG_OPMODE);
	
    /* Set standby mode. */
	RFM69_SetMode(OP_MODE_STANDBY);
	
  	regsvalue = frequency / 61.03515625f;	

	RFM69_Register_Write(REG_FDEVMSB, regsvalue >> 8);
  	RFM69_Register_Write(REG_FDEVLSB, regsvalue);

	/* Restore previous mode. */
	RFM69_SetMode(actualmode);
}

/*******************************************************************************
* Function Name: RFM69_SetBitrate
********************************************************************************
*
* Summary:
*  Set communication bit rate.
*
* Parameters:
*  bitrate:     bitrate in bits per second.
*
* Return:
*  none
*
*******************************************************************************/
void RFM69_SetBitrate(uint16 bitrate)
{
	uint16 regsvalue;

	regsvalue = 32000000 / bitrate;
    RFM69_SetBitrateCls(regsvalue >> 8, regsvalue);
}

/*******************************************************************************
* Function Name: RFM69_SetBitrateCls
********************************************************************************
*
* Summary:
*  Set communication bit rate.
*  This function takes a previously calculated 16 bits value to be written
*  directly into RFM module registers.
*  Usually used to set communication rate using calculated standard values in
*  'PSoC_RFM69.h' file.
*
* Parameters:
*  msb:     more significant byte.
*  lsb:     less significant byte.
*
* Return:
*  none
*
*******************************************************************************/
void RFM69_SetBitrateCls(uint8 msb, uint8 lsb)
{
    uint8 actualmode = RFM69_Register_Read(REG_OPMODE);
	
    /* Set standby mode. */
	RFM69_SetMode(OP_MODE_STANDBY);

 	RFM69_Register_Write(REG_BITRATEMSB, msb);
	RFM69_Register_Write(REG_BITRATELSB, lsb);
	
	/* Restore previous mode. */
	RFM69_SetMode(actualmode);
}

/*******************************************************************************
* Function Name: RFM69_SetPower
********************************************************************************
*
* Summary:
*  Set transmission output power.
*
* Parameters:
*  power:   power value (max 31)
*
* Return:
*  none
*
*******************************************************************************/
void RFM69_SetPower(uint8 power)
{
	if (power > 31) power = 31;
	
	RFM69_Register_Write(REG_PALEVEL, (RFM69_Register_Read(REG_PALEVEL) & 0xF0) | power); 
}

/*******************************************************************************
* Function Name: RFM69_SetAddressFiltering
********************************************************************************
*
* Summary:
*  Enable/disable address filtering.
*
* Parameters:
*  addressfiltering:    00 - disable address filtering.
*                       01 - enable node address filtering.
*                       10 - enable node & broadcast address filtering.
*  nodeaddress:         new node address.
*  broadcastaddress:    new broadcast address.
*
* Return:
*  1 - If sucessful.
*  0 - If 'addressfiltering' have an inproper value.
*
*******************************************************************************/
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

/*******************************************************************************
* Function Name: RFM69_Encryption
********************************************************************************
*
* Summary:
*  Enable/disable AES encryption.
*
* Parameters:
*  enable:      0 - disable encryption.
*               1 - enable encryption.
*  aeskey:      pointer to uint8 array, 16 bytes length; with new encryption
*               key.
*
* Return:
*  1 - If sucessful.
*  0 - If 'enable' > 1.
*
*******************************************************************************/
uint8 RFM69_Encryption(uint8 enable, uint8 *aeskey)
{
    uint8 loop;
    uint8 actualmode;
    
    if (enable > 1) return 0;
    
    actualmode = RFM69_Register_Read(REG_OPMODE);
    
    /* Set standby mode. */
	RFM69_SetMode(OP_MODE_STANDBY);
 
    if (enable == 1)
    {
        for (loop = REG_AESKEY_1; loop < (REG_AESKEY_16 + 1); loop++)
        {
            RFM69_Register_Write(loop, *aeskey);
            aeskey++;
        }
    }

    RFM69_Register_Write(REG_PACKETCONFIG_2, (RFM69_Register_Read(REG_PACKETCONFIG_2) & 0xFE) | enable);
    
    /* Restore previous mode. */
	RFM69_SetMode(actualmode);
    
    return 1;
}

/*******************************************************************************
* Function Name: RFM69_DataPacket_TX
********************************************************************************
*
* Summary:
*  Transmit a packet of data.
*
* Parameters:
*  buf:     pointer to byte array with data to be sent.
*  len:     length of data to be sent.
*
* Return:
*  none
*
*******************************************************************************/
void RFM69_DataPacket_TX(uint8 *buf, int len) 
{
    /* Set standby mode before writting to FIFO. */
	RFM69_SetMode(OP_MODE_STANDBY);             

    /* Send data to RFM69 FIFO. */
    mmSPI_SS_Write(0);
    CyDelayUs(SS_DELAY);

    mmSPI_SpiUartClearTxBuffer();
    mmSPI_SpiUartWriteTxData(REG_FIFO | 0x80);
    mSPI_WAIT_TXDONE();
    
    mmSPI_SpiUartClearTxBuffer();
    mmSPI_SpiUartPutArray(buf, len);
    mSPI_WAIT_TXDONE();

    CyDelayUs(SS_DELAY);
    mmSPI_SS_Write(1);

	/* Set TX mode. */
    RFM69_Register_Write(REG_PACKETCONFIG_2, (RFM69_Register_Read(REG_PACKETCONFIG_2) & 0xFB) | 0x04);
	RFM69_SetMode(OP_MODE_TX);
    
    /* Wait until data has been sent. */
    while ((RFM69_Register_Read(REG_IRQFLAGS_2) & 0x08) == 0) {};
}

/*******************************************************************************
* Function Name: RFM69_DataPacket_RX
********************************************************************************
*
* Summary:
*  Try to read a received data packet.
*
* Parameters:
*  buffer:      pointer to byte array where received data is returned.
*  rssi:        pointer to 'uint8'.
*               If 'rssi' != 0, at return it will have value of RSSI; 
*               if = 0, RSSI is not readed.
*
* Return:
*  Length of received data or 0 if there si no data ready.
*
*******************************************************************************/
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
    mmSPI_SS_Write(0);
    CyDelayUs(SS_DELAY);
    
    mmSPI_SpiUartClearTxBuffer();
    mmSPI_SpiUartWriteTxData(REG_FIFO);
    mSPI_WAIT_TXDONE();
    
    mmSPI_SpiUartClearRxBuffer();
    
    for (loop = 0; loop < fifolength; loop++)
    {
        mmSPI_SpiUartWriteTxData(0x00);
        mSPI_WAIT_TXDONE();
        
    	buffer[loop] = mmSPI_SpiUartReadRxData();
    }

    CyDelayUs(SS_DELAY);
    mmSPI_SS_Write(1);
    
    /* Set RX mode. */
    RFM69_Register_Write(REG_PACKETCONFIG_2, (RFM69_Register_Read(REG_PACKETCONFIG_2) & 0xFB) | 0x04);
    RFM69_SetMode(OP_MODE_RX);
        
    /* If pointer to rssi != 0, then read RSSI. */
    if (rssi != 0) *rssi = RFM69_GetRSSI();    

    return fifolength;
}

/*******************************************************************************
* Function Name: RFM69_GetIRQFlags
********************************************************************************
*
* Summary:
*  Return IRQ flags.
*
* Parameters:
*  none
*
* Return:
*  byte with flags:
*   bit 0 - PayloadReady.
*   bit 1 - PacketSent.
*
*******************************************************************************/
uint8 RFM69_GetIRQFlags()
{
    uint8 dd;
    dd = RFM69_Register_Read(REG_IRQFLAGS_2);
    dd = dd >> 2;
    dd = dd & 0x03;
    
    return dd;
}

/*******************************************************************************
* Function Name: RFM69_GetTemperature
********************************************************************************
*
* Summary:
*  Read RFM module internal sensor temperature.
*
* Parameters:
*  none
*
* Return:
*  Temperature, register raw value.
*
*******************************************************************************/
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

/*******************************************************************************
* Function Name: RFM69_GetRSSI
********************************************************************************
*
* Summary:
*  Read RSSI value.
*
* Parameters:
*  none
*
* Return:
*  RSSI value, register raw value.
*
*******************************************************************************/
uint8 RFM69_GetRSSI() 
{
    /* Start RSSI measurement. */
	RFM69_Register_Write(REG_RSSICONFIG, 0x01); 
    
    /* Wait until measurement has finished. */
    while ((RFM69_Register_Read(REG_RSSICONFIG) & 0x02) == 0) {};
    
    return RFM69_Register_Read(REG_RSSIVALUE);
}

/*******************************************************************************
* Function Name: RFM69_Register_Read
********************************************************************************
*
* Summary:
*  Read value of a RFM module register.
*
* Parameters:
*  reg_addr:        register address.
*
* Return:
*  Register value.
*
*******************************************************************************/
uint8 RFM69_Register_Read(uint8 reg_addr) 
{
    uint8 reg_value;
    
    mmSPI_SS_Write(0);
    CyDelayUs(SS_DELAY);
    
    mmSPI_SpiUartClearTxBuffer();
    mmSPI_SpiUartClearRxBuffer();
    mmSPI_SpiUartWriteTxData(reg_addr);
    mSPI_WAIT_TXDONE();
    
    mmSPI_SpiUartClearTxBuffer();
    mmSPI_SpiUartClearRxBuffer();
    mmSPI_SpiUartWriteTxData(0xFF);
    mSPI_WAIT_TXDONE();    
    
	reg_value = mmSPI_SpiUartReadRxData();

    CyDelayUs(SS_DELAY);
    mmSPI_SS_Write(1);
    
	return reg_value;
}

/*******************************************************************************
* Function Name: RFM69_Register_Write
********************************************************************************
*
* Summary:
*  Write value to a RFM module register.
*
* Parameters:
*  reg_addr:        register address.
*  reg_value:       new register value.
*
* Return:
*  none.
*
*******************************************************************************/
void RFM69_Register_Write(uint8 reg_addr, uint8 reg_value) 
{
    mmSPI_SS_Write(0);
    CyDelayUs(SS_DELAY);
    
    mmSPI_SpiUartClearTxBuffer();
    mmSPI_SpiUartWriteTxData(reg_addr | 0x80); 
    mmSPI_SpiUartWriteTxData(reg_value);
    mSPI_WAIT_TXDONE();

    CyDelayUs(SS_DELAY);
    mmSPI_SS_Write(1);
}

/*******************************************************************************
* Function Name: RFM69_HardwareReset
********************************************************************************
*
* Summary:
*  Perform a hardware reset to RFM69 module
*
* Parameters:
*  none.
*
* Return:
*  none.
*
*******************************************************************************/

#ifdef mmRESET_PIN

void RFM69_HardwareReset()
{
    mmRESET_PIN(1);     // Reset pin to high level.
    CyDelay(1) ;        // Wait 1ms (datasheet says 100us)
    mmRESET_PIN(0);     // Reset pin to low state.
    CyDelay(10);        // Wait 10ms (datasheet says 5ms)
}

#endif






