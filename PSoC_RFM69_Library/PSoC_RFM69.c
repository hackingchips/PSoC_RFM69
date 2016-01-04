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

#define mRFM69_WaitModeReady()  while((RFM69_Register_Read(REG_IRQFLAGS1) & 0x80) == 0) {}

uint8 RFM69_GetRSSI();
uint8 RFM69_Register_Read(uint8 reg_addr);
void RFM69_Register_Write(uint8 reg_addr, uint8 reg_value);

uint8 RFM69_Start()
{
    uint16 loop;
    
    uint8 RFM69_CONFIG[][2] = {
        {REG_OPMODE, 0b00000100},			// Start in standby mode.
        {REG_DATAMODUL, 0b00000000},        // Packetmode, modulation = FSK, no shaping.  
        {REG_BITRATEMSB, 0b00110100},		// Bit rate = 2400 bauds.
        {REG_BITRATELSB, 0b00010000},
        {REG_FDEVMSB, 0b00000010},			// Frecuency deviation = 35Khz  
        {REG_FDEVLSB, 0b01000001},
        {REG_FRFMSB, 0b01101100},			// Carrier frequency = 433.749 Mhz
        {REG_FRFMID, 0b10000000},
        {REG_FRFLSB, 0b00000000},
        {REG_PALEVEL, 0b10011111},			// PA0 = on, max power.
        {REG_OCP, 0b00001111},				// OCP Enabled
        {REG_LNA, 0b10001000},				// Input impedance = 200ohm, gain = internal AGC.  
        {REG_RXBW, 0b01010010},
        {REG_DIOMAPPING1, 0b01000100},		
        {REG_PREAMBLEMSB, 0b00000000},		// Preamble = 5 bytes.   
        {REG_PREAMBLELSB, 0b00000101},
        {REG_SYNCCONFIG, 0b10010000},		// Sync word enabled, FIFO filling if sync address, size = 3(2+1), tolerated error bits = 0.
        {REG_SYNCVALUE1, 0xAA},				// Sync bytes.
        {REG_SYNCVALUE2, 0x2D},
        {REG_SYNCVALUE3, 0xD4},
        {REG_NODEADRS, NODE_ADDRESS},
        {REG_BROADCASTADRS, BROADCAST_ADDRESS},
        {REG_PACKETCONFIG1, PACKETCONFIG1_VALUE},
        {REG_PAYLOADLENGTH, 0b00000011},	// Length of payload = 3
        {REG_FIFOTHRESH, 0b10010101}, 		// Fifo threshold = 21. Tx when FIFO not empty.     
        {REG_TESTLNA, 0b00011011},			// Normal sensitivity
        {REG_TESTPA1, 0b01010101},			// Normal mode.
        {REG_TESTPA2, 0b01110000},			// Normal mode.
        {REG_TESTDAGC, 0b00110000},			// Improved margin.
        {REG_PACKETCONFIG2, 0b00010010}
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

void RFM69_SetAddressFiltering(uint8 filtering, uint8 nodeaddress, uint8 broadcastaddress)
{
    uint8 registervalue = RFM69_Register_Read(REG_PACKETCONFIG1);
    
    registervalue &= 0xF9;
    registervalue |= (filtering << 1);
    
    RFM69_Register_Write(REG_PACKETCONFIG1, registervalue);
    RFM69_Register_Write(REG_NODEADRS, nodeaddress);
    RFM69_Register_Write(REG_BROADCASTADRS, broadcastaddress);
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
    RFM69_Register_Write(REG_PACKETCONFIG2, (RFM69_Register_Read(REG_PACKETCONFIG2) & 0xFB) | 0x04);
	RFM69_SetMode(OP_MODE_TX);
    
    /* Wait until data has been sent. */
    while ((RFM69_Register_Read(REG_IRQFLAGS2) & 0x08) == 0) {};
}


int RFM69_DataPacket_RX(uint8 *buffer, uint8 *rssi) 
{
	uint8 loop, fifolength;
    
    /* Check if there is data ready in FIFO. */
    if (!(RFM69_Register_Read(REG_IRQFLAGS2) & 0x04)) return 0; // No data. Return 0.
    
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
    RFM69_Register_Write(REG_PACKETCONFIG2, (RFM69_Register_Read(REG_PACKETCONFIG2) & 0xFB) | 0x04);
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
	RFM69_Register_Write(REG_TEMP1, 0x08);
    while((RFM69_Register_Read(REG_TEMP1) & 0x04) != 0) {};

	temperature = RFM69_Register_Read(REG_TEMP2);

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
	uint8 actualmode = RFM69_Register_Read(REG_OPMODE);
	
	RFM69_SetMode(OP_MODE_STANDBY);

	/* Calculate registers value. 
		registers value = RFM69_InternalClock / bitrate.
		Internal clock of RFM69 is 32Mhz.
	*/
	regsvalue = 32000000 / bitrate;

 	RFM69_Register_Write(REG_BITRATEMSB, regsvalue >> 8);
	RFM69_Register_Write(REG_BITRATELSB, regsvalue);
	
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


