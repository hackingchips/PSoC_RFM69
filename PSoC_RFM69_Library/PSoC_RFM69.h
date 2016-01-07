/*******************************************************************************
* File Name: RFM69.h
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
    
#if !defined(PSOC_RFM69_H)
#define PSOC_RFM69_H
    
#include <cytypes.h>    

#define REG_FIFO            0x00
#define REG_OPMODE          0x01                       
#define REG_DATAMODUL       0x02
#define REG_BITRATEMSB      0x03
#define REG_BITRATELSB      0x04
#define REG_FDEVMSB         0x05
#define REG_FDEVLSB         0x06
#define REG_FRFMSB          0x07
#define REG_FRFMID          0x08
#define REG_FRFLSB          0x09
#define REG_OSC_1           0x0A
#define REG_AFCCTRL         0x0B
#define REG_LISTEN_1        0x0D
#define REG_LISTEN_2        0x0E
#define REG_LISTEN_3        0x0F
#define REG_VERSION         0x10
#define REG_PALEVEL         0x11
#define REG_PARAMP          0x12
#define REG_OCP             0x13
#define REG_LNA             0x18
#define REG_RXBW            0x19
#define REG_AFCBW           0x1A
#define REG_RSSICONFIG      0x23
#define REG_RSSIVALUE       0x24
#define REG_DIOMAPPING_1    0x25
#define REG_IRQFLAGS_1      0x27
#define REG_IRQFLAGS_2      0x28
#define REG_RSSITHRESH      0x29
#define REG_RXTIMEOUT_1     0x2A
#define REG_RXTIMEOUT_2     0x2B
#define REG_PREAMBLEMSB		0x2C
#define REG_PREAMBLELSB		0x2D
#define REG_SYNCCONFIG      0x2E
#define REG_SYNCVALUE_1     0x2F
#define REG_SYNCVALUE_2     0x30
#define REG_SYNCVALUE_3     0x31
#define REG_SYNCVALUE_4     0x32
#define REG_SYNCVALUE_5     0x33
#define REG_SYNCVALUE_6     0x34
#define REG_SYNCVALUE_7     0x35
#define REG_SYNCVALUE_8     0x36
#define REG_PACKETCONFIG_1  0x37
#define REG_PAYLOADLENGTH   0x38
#define REG_NODEADRS        0x39
#define REG_BROADCASTADRS   0x3A
#define REG_FIFOTHRESH      0x3C
#define REG_PACKETCONFIG_2  0x3D
#define REG_AESKEY_1        0x3E
#define REG_AESKEY_2        0x3F
#define REG_AESKEY_3        0x40
#define REG_AESKEY_4        0x41
#define REG_AESKEY_5        0x42
#define REG_AESKEY_6        0x43
#define REG_AESKEY_7        0x44
#define REG_AESKEY_8        0x45
#define REG_AESKEY_9        0x46
#define REG_AESKEY_10       0x47
#define REG_AESKEY_11       0x48
#define REG_AESKEY_12       0x49
#define REG_AESKEY_13       0x4A
#define REG_AESKEY_14       0x4B
#define REG_AESKEY_15       0x4C
#define REG_AESKEY_16       0x4D    
#define REG_TEMP_1          0x4E
#define REG_TEMP_2          0x4F  
#define REG_TESTLNA			0x58
#define	REG_TESTPA_1        0x5A
#define REG_TESTPA_2	    0x5C
#define REG_TESTDAGC        0x6F  
#define REG_TESTAFC			0x71
    
#define OP_MODE_SLEEP       (0x00 << 2)
#define OP_MODE_STANDBY     (0x01 << 2)
#define OP_MODE_FS          (0x02 << 2)
#define OP_MODE_TX          (0x03 << 2)
#define OP_MODE_RX          (0x04 << 2)
    
#define BITRATE_MSB_1200            0x68
#define BITRATE_LSB_1200            0x2B
#define BITRATE_MSB_2400            0x34
#define BITRATE_LSB_2400            0x15
#define BITRATE_MSB_4800            0x1A
#define BITRATE_LSB_4800            0x0B
#define BITRATE_MSB_9600            0x0D
#define BITRATE_LSB_9600            0x05
#define BITRATE_MSB_19200           0x06
#define BITRATE_LSB_19200           0x83
#define BITRATE_MSB_38400           0x03
#define BITRATE_LSB_38400           0x41
#define BITRATE_MSB_57600           0x02
#define BITRATE_LSB_57600           0x2C
#define BITRATE_MSB_115200          0x01
#define BITRATE_LSB_115200          0x16    
    
uint8 RFM69_Start();
uint8 RFM69_CheckPresence();   
uint8 RFM69_Encryption(uint8 setunset, uint8 *aeskey);
void RFM69_SetMode(uint8 mode);
uint8 RFM69_SetAddressFiltering(uint8 filtering, uint8 nodeaddress, uint8 broadcastaddress);
void RFM69_SetPayloadLength(uint8 plength);
uint8 RFM69_SetSync(uint8 syncsize, uint8 syncbitstolerance, uint8 *syncvalue);
void RFM69_DataPacket_TX(uint8 *buf, int len);  
int RFM69_DataPacket_RX(uint8 *buffer, uint8 *rssi);
uint8 RFM69_GetTemperature();
void RFM69_SetBitrate(uint16 bitrate);
void RFM69_SetBitrateCls(uint8 msb, uint8 lsb);
void RFM69_SetFrequency(uint32 frequency);
void RFM69_SetFrequencyDeviation(uint16 frequency);
void RFM69_SetPower(uint8 power);

#endif  /* PSOC_RFM69_H */    
