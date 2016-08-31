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

#if !defined(MAIN_H)
#define MAIN_H
    
#include <cytypes.h>    
    
    /* PSoC5LP Dev Kit only have one user led. */
#define Led_Red_Write(value)        User_Led_Write(value)
#define Led_Green_Write(value)      User_Led_Write(value)
#define Led_Blue_Write(value)       User_Led_Write(value)
#define Led_Red_Read(value)         User_Led_Read(value)
#define Led_Green_Read(value)       User_Led_Read(value)
#define Led_Blue_Read(value)        User_Led_Read(value)    
    
/* ************************************************************************* */
/* *** Uncomment one line. *** */

//#define COMPILE_FOR_MASTER
#define COMPILE_FOR_SLAVE_1
//#define COMPILE_FOR_SLAVE_2  
    
/* ************************************************************************* */
/* *** If you want. Uncomment line. *** */
/* *** Testing with interrupts, if selected; is done only while in reception 
       mode.*/
    
//#define TEST_WITH_ENCRYPTION
//#define TEST_USING_INTERRUPTS    
    
/* ************************************************************************* */    
    
    /* RFM interrupts flag. */
extern volatile uint8 rfrxirqflag;
    
    /* Used for timing. */
volatile int16 timercnt;            // used for timming.
    
    /* Data buffer for transmision/reception to/from RFM69 module. */
extern uint8 rfdatabytes[];    


    
#endif  /* MAIN_H */
