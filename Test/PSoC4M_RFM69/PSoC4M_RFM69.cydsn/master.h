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

#if !defined(MASTER_H)
#define MASTER_H

void Config_ForMaster();
void Loop_Master();
void Loop_Master_WaitingSPort();
void Loop_Master_WaitingSlaveResponse();
void TerminalSend_Master();
    
#endif  /* MASTER_H */
