/*
 * THE FOLLOWING FIRMWARE IS PROVIDED: (1) "AS IS" WITH NO WARRANTY; AND 
 * (2)TO ENABLE ACCESS TO CODING INFORMATION TO GUIDE AND FACILITATE CUSTOMER.
 * CONSEQUENTLY, SEMTECH SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR
 * CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 * OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 * CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 * 
 * Copyright (C) SEMTECH S.A.
 */
/*! 
 * \file       sx1276.c
 * \brief      SX1276 RF chip driver
 *
 * \version    2.0.0 
 * \date       May 6 2013
 * \author     Gregory Cristian
 *
 * Last modified by Miguel Luis on Jun 19 2013
 */
#include "platform.h"
#include "radio.h"

#if defined( USE_SX1276_RADIO )

#include "sx1276.h"

//#include "sx1276-Hal.h"
#include "hal_radio.h"
#include "sx1276-Fsk.h"
#include "sx1276-LoRa.h"
#include <stdio.h>   
/*!
 * SX1276 registers variable
 */
uint8_t SX1276Regs[0x70];

static bool LoRaOn = false;
static bool LoRaOnState = false;

/* REMARK: After radio reset the default modem is FSK */
bool LORA = false;

extern st_GDOx_Config g_GDOx_map_conf;
tRadioDriver *radio = NULL;
extern tLoRaSettings LoRaSettings;

/*****************************************************************************
 Prototype    : SX1276Init
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276Init( void )
{
  /* Initialize FSK and LoRa registers structure */
  /* 这是一个很好的初始化结构体的技巧，把数组名强制转化为指向结构体的指针 */
  #if defined(SX1276_FSK)
  SX1276 = ( tSX1276* )SX1276Regs;
  #endif

  #if defined(SX1276_LORA)
  SX1276LR = ( tSX1276LR* )SX1276Regs;
  #endif
  
  SX1276Reset( );
  /* in order to compatible FSK,change LORA to bool variable */
  if (LORA)
  {
   #if defined(SX1276_FSK)
    LoRaOn = false;
    SX1276SetLoRaOn( LoRaOn );
    /* Initialize FSK modem */
    
    SX1276FskInit( );
   #endif
  }
  else
  {
  #if defined(SX1276_LORA)
    LoRaOn = TRUE;
    SX1276SetLoRaOn( LoRaOn );
    /* Initialize LoRa modem */
    SX1276LoRaInit();
  #endif
  }
}

/*****************************************************************************
 Prototype    : hal_InitRF
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void hal_InitRF(void)
{ 
  hal_sRF_InitSPI();
  GPIO_int_Config();
  radio = RadioDriverInit();
  radio->Init();
  SX1276LoRa_Receive_Packet();
}


/****************************************************************************
 Prototype    : SX1276Reset
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276Reset( void )
{  
  uint32_t startTick;

  SX1276SetReset( RADIO_RESET_ON );

  /* Wait 1ms */
  startTick = hal_GetSystickCounter( );
  while( ( hal_GetSystickCounter( ) - startTick ) < TICK_RATE_MS( 1 ) );    

  SX1276SetReset( RADIO_RESET_OFF );

  /* Wait 6ms */
  startTick = hal_GetSystickCounter( );
  while( ( hal_GetSystickCounter( ) - startTick ) < TICK_RATE_MS( 6 ) );    
}

/*****************************************************************************
 Prototype    : SX1276SetLoRaOn
 Description  : none
 Input        : bool enable  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276SetLoRaOn( bool enable )
{
    /* if current state is the same of wanted state,return */
    if( LoRaOnState == enable )
    {
        return;
    }
    LoRaOnState = enable;
    LoRaOn = enable;

    if( LoRaOn == TRUE )
    {
        SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
        /* set to lora mode */
        SX1276LR->RegOpMode = ( SX1276LR->RegOpMode & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_ON;
        SX1276Write( REG_LR_OPMODE, SX1276LR->RegOpMode );
        
        SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );

        g_GDOx_map_conf.GDO0Config = DIO0_RxDone;
        g_GDOx_map_conf.GDO1Config = DIO1_RxTimeout;
        g_GDOx_map_conf.GDO2Config = DIO2_FhssCC;
        g_GDOx_map_conf.GDO3Config = DIO3_CadDone;
        g_GDOx_map_conf.GDO4Config = DIO4_CadDetected;
        g_GDOx_map_conf.GDO5Config = DIO5_ModeReady;
        config_GDOx_Map(g_GDOx_map_conf);
        
        SX1276ReadBuffer( REG_LR_OPMODE, SX1276Regs + 1, 0x70 - 1 );
    }
    else 
    {
        SX1276LoRaSetOpMode( RFLR_OPMODE_SLEEP );
        
        SX1276LR->RegOpMode = ( SX1276LR->RegOpMode & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_OFF;
        SX1276Write( REG_LR_OPMODE, SX1276LR->RegOpMode );
        
        SX1276LoRaSetOpMode( RFLR_OPMODE_STANDBY );
        
        SX1276ReadBuffer( REG_OPMODE, SX1276Regs + 1, 0x70 - 1 );
    }
}

/*****************************************************************************
 Prototype    : SX1276GetLoRaOn
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
bool SX1276GetLoRaOn( void )
{
    return LoRaOn;
}

/*****************************************************************************
 Prototype    : SX1276SetOpMode
 Description  : none
 Input        : uint8_t opMode  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276SetOpMode( uint8_t opMode )
{
    if( LoRaOn == false )
    {
      #if defined(SX1276_FSK)
        SX1276FskSetOpMode( opMode );
      #endif
    }
    else
    {
     #if defined(SX1276_LORA)
        SX1276LoRaSetOpMode( opMode );
     #endif
    }
}

/*****************************************************************************
 Prototype    : SX1276GetOpMode
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
uint8_t SX1276GetOpMode( void )
{
    uint8_t opMode;
    
    if( LoRaOn == false )
    {
       #if defined(SX1276_FSK)
        opMode = SX1276FskGetOpMode( );
       #endif
    }
    else
    {
      #if defined(SX1276_LORA)
        opMode = SX1276LoRaGetOpMode( );
      #endif
    }
    return opMode;
    
}

/*****************************************************************************
 Prototype    : SX1276ReadRssi
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
double SX1276ReadRssi( void )
{
  double rssi;
    if( LoRaOn == false )
    {
      #if defined(SX1276_FSK)
          rssi = SX1276FskReadRssi( );
      #endif
    }
    else
    {
      #if defined(SX1276_LORA)
        rssi = SX1276LoRaReadRssi( );
      #endif
    }
    return rssi;
}

/*****************************************************************************
 Prototype    : SX1276ReadRxGain
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
uint8_t SX1276ReadRxGain( void )
{
  uint8_t rxGain;

    if( LoRaOn == false )
    {
      #if defined(SX1276_FSK)
        rxGain = SX1276FskReadRxGain( );
      #endif
    }
    else
    {
      #if defined(SX1276_LORA)
        rxGain = SX1276LoRaReadRxGain( );
      #endif
    }
    return rxGain;
}

/*****************************************************************************
 Prototype    : SX1276GetPacketRxGain
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
uint8_t SX1276GetPacketRxGain( void )
{
  uint8_t pRxGain;
    if( LoRaOn == false )
    {
     #if defined(SX1276_FSK)
        pRxGain = SX1276FskGetPacketRxGain(  );
     #endif
    }
    else
    {
    #if defined(SX1276_LORA)
        pRxGain = SX1276LoRaGetPacketRxGain(  );
    #endif
    }
    return pRxGain;
}

/*****************************************************************************
 Prototype    : SX1276GetPacketSnr
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
int8_t SX1276GetPacketSnr( void )
{
  int8_t pSnr;
    if( LoRaOn == false )
    {
         while( 1 )
         {
             // Useless in FSK mode
             // Block program here
         }
    }
    else
    {
      #if defined(SX1276_LORA)
          pSnr = SX1276LoRaGetPacketSnr(  );
      #endif
    }
    return pSnr;
}

/*****************************************************************************
 Prototype    : SX1276GetPacketRssi
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
double SX1276GetPacketRssi( void )
{
  double pRssi;

    if( LoRaOn == false )
    {
     #if defined(SX1276_FSK)
        pRssi = SX1276FskGetPacketRssi( );
     #endif
    }
    else
    {
      #if defined(SX1276_LORA)
          pRssi = SX1276LoRaGetPacketRssi( );
      #endif
    }
    return pRssi;
    
}

/*****************************************************************************
 Prototype    : SX1276GetPacketAfc
 Description  : none
 Input        : void  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
uint32_t SX1276GetPacketAfc( void )
{
  uint32_t PAfc = 0;

    if( LoRaOn == false )
    {
     #if defined(SX1276_FSK)
        PAfc = SX1276FskGetPacketAfc(  );
     #endif
    }
    else
    {
         while( 1 )
         {
             // Useless in LoRa mode
             // Block program here
         }
    }
    return PAfc;
    
}


void SX1276StartRx( bool FreqHop )
{
  if( LoRaOn == false )
    {
     #if defined(SX1276_FSK)
        SX1276FskSetRFState( RF_STATE_RX_INIT );
     #endif
    }
    else
    {
      #if defined(SX1276_LORA)
        SX1276LoRa_Receive_Packet();
      #endif
    }
} 

/*****************************************************************************
 Prototype    : SX1276StartTx
 Description  : none
 Input        : u8 *PBuffer   
                u16 size      
                bool FreqHop  
 Output       : None
 Return Value : 
 Date         : 2014/3/15
 Author       : Barry
*****************************************************************************/
void SX1276StartTx(u8 *PBuffer,u16 size)
{
   if( LoRaOn == false )
    {
       
    }
    else
    {
      #if defined(SX1276_LORA)
       SX1276LoRa_Send_Packet( PBuffer, size );
      #endif
    }
}


uint8_t SX1276GetRFState( void )
{
  uint8_t rfState;

    if( LoRaOn == false )
    { 
      #if defined(SX1276_FSK)
        rfState = SX1276FskGetRFState( );
      #endif
    }
    else
    {
      #if defined(SX1276_LORA)
        rfState = SX1276LoRaGetRFState( );
      #endif
    }
    return rfState;
    
}

void SX1276SetRFState( uint8_t state )
{
    if( LoRaOn == false )
    {
     #if defined(SX1276_FSK)
        SX1276FskSetRFState( state );
     #endif
    }
    else
    {
     #if defined(SX1276_LORA)
        SX1276LoRaSetRFState( state );
     #endif
    }
}

#endif // USE_SX1276_RADIO
