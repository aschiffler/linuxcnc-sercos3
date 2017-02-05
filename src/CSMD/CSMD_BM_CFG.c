/*
CoSeMa V6.1 - Common Sercos Master function library
Copyright (c) 2004 - 2016  Bosch Rexroth AG

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THIS SOFTWARE IS PROVIDED "AS IS"; WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY;
FITNESS FOR A PERTICULAR PURPOSE AND NONINFRINGEMENT. THE AUTHORS OR COPYRIGHT
HOLDERS SHALL NOT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE;
UNLESS STIPULATED BY MANDATORY LAW.

You may contact us at open.source@boschrexroth.de
if you are interested in contributing a modification to the Software.

30 April 2015

============================================================================ */

/*!
\file   CSMD_BM_CFG.c
\author WK
\date   01.09.2010
\brief  This File contains the private functions for the bus master configuration.

*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"
#include "CSMD_HAL_DMA.h"

#include "CSMD_BM_CFG.h"
#include "CSMD_CALC.h"
#include "CSMD_PRIV_SVC.h"
#include "CSMD_TOPOLOGY.h"

/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/*! \endcond */ /* PUBLIC */


/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Sets all registers needed for preparing CP0.

\ingroup module_phase
\b Description: \n
   This function performs a (re-)initialization of several basic settings
   and default values, including:
   - Topology (no link)
   - clearance of pointers to all buffers
   - Inter Frame Gap (default value)
   - Single buffer mode for Tx and Rx buffer
   - Event time values (minimum, maximum, reload)
   - Calculation of timer events for CP0.
   - Check for IP-Core service channel redundancy.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_EVENT_TIME_MAX_LIMIT \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         04.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Set_Register_P0( CSMD_INSTANCE *prCSMD_Instance )    
{
  
  CSMD_USHORT   usI;
  CSMD_ULONG    ulReload;           /* Value for Event Reload [ns]                */
  CSMD_ULONG    ulDelay;            /* Delay between FPGA- and Sercos-Timing [ns] */
  CSMD_ULONG    ulEventOffset;      /* Value of forbidden range for events [ns]   */
/*CSMD_ULONG    ulMinEventTime;*/   /* Limit for minimum event time [ns]          */
  CSMD_ULONG    ulMaxEventTime;     /* Limit for maximum event time [ns]          */
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;
  
  CSMD_EVENT    arTimerEvent[ CSMD_TIMER_EVENT_NUMBER ];
  CSMD_EVENT    arPortsEvent[ CSMD_PORTS_EVENT_NUMBER ];
  CSMD_USHORT   usIdx;              /* Event Index */
  CSMD_USHORT   usBuf;
  CSMD_ULONG    ulTime;
  
  
  for (usI = 0; usI < CSMD_MAX_TEL; usI++ )
  {                                                
    prCSMD_Instance->rPriv.aboMDT_used[usI]   = FALSE;      /* MDT telegram is not used */
    prCSMD_Instance->rPriv.aboAT_used[usI]    = FALSE;      /* AT telegram is not used  */
    
    for (usBuf = 0; usBuf < CSMD_MAX_RX_BUFFER; usBuf++)
    {
      /* Clear pointer to all buffers.
         Note: Multiple buffering is used from CP3. */
      prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usI] = NULL;
      prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usI] = NULL;
#ifdef CSMD_PCI_MASTER
      prCSMD_Instance->rPriv.ausTxDMA_Start_P1[usBuf] = 0;
      prCSMD_Instance->rPriv.ausTxDMA_Start_P2[usBuf] = 0;
#endif
    }
#ifdef CSMD_PCI_MASTER
    for (usBuf = 0; usBuf < CSMD_MAX_TX_BUFFER; usBuf++)
    {
      prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf]    = 0;
    }
#endif
  }
  
  prCSMD_Instance->rPriv.boP1_active = TRUE;         /* flag port 1 active */
  prCSMD_Instance->rPriv.boP2_active = TRUE;         /* flag port 2 active */
  prCSMD_Instance->usCSMD_Topology         = CSMD_NO_LINK; /* flag for topology */  
  
  prCSMD_Instance->usEnableTel = (CSMD_USHORT)0;          /* Not ready to enable CYC_CLK */
  
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
  {
    prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] = CSMD_PORT_1;   /* Clear flags for SVC */
  }
  
  /* register default values */
  /* prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulIER0 = 0x00000000L; */
  /* prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulIMR0 = 0x00000000L; */
  
  CSMD_HAL_IntControl( &prCSMD_Instance->rCSMD_HAL, 
                       0U,
                       FALSE,
                       TRUE,
                       0U );
  
#ifdef CSMD_SYNC_DPLL
  /* Command Stop_Sync to the PLL control register. */
  CSMD_HAL_Write_PLL_Control(  &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_PLL_STOP_SYNC );
  /* Diasable the PLL by disble the ConClkPLLEn bit in the TCSR. */
  CSMD_HAL_Enable_PLL( &prCSMD_Instance->rCSMD_HAL, FALSE );
  
  /* PLL for external synchronisation is not active. */
  prCSMD_Instance->rPriv.boPLL_Mode = FALSE;
#endif
  
  /* Don't change TCSR FPGA register because timer is already off ! / 
     keep the last setting of CYC_CLK, CON_CLK and DIV_CLK */
  
  /* Set all buffer systems to single buffering mode */
  CSMD_HAL_Configure_Tx_Buffer( &prCSMD_Instance->rCSMD_HAL, 
                                CSMD_HAL_TX_BUFFER_SYSTEM_A, 
                                CSMD_HAL_SINGLE_BUFFER_SYSTEM );

  CSMD_HAL_Configure_Tx_Buffer( &prCSMD_Instance->rCSMD_HAL, 
                                CSMD_HAL_TX_BUFFER_SYSTEM_B, 
                                CSMD_HAL_SINGLE_BUFFER_SYSTEM );

  CSMD_HAL_Configure_Rx_Buffer( &prCSMD_Instance->rCSMD_HAL, 
                                CSMD_HAL_RX_BUFFER_SYSTEM_A, 
                                CSMD_HAL_SINGLE_BUFFER_SYSTEM );

  CSMD_HAL_Configure_Rx_Buffer( &prCSMD_Instance->rCSMD_HAL, 
                                CSMD_HAL_RX_BUFFER_SYSTEM_B, 
                                CSMD_HAL_SINGLE_BUFFER_SYSTEM );
  
  
  /* Default IFG is sufficient. Only 1 AT and 1 MDT is used. */
  prCSMD_Instance->rPriv.ulInterFrameGap = CSMD_IFG_DEFAULT;
  CSMD_HAL_SetInterFrameGap( &prCSMD_Instance->rCSMD_HAL, 
                             prCSMD_Instance->rPriv.ulInterFrameGap );
  
  /* Set the MTU, so FPGA can check if a transmission is possible after last transmit event. */
  CSMD_HAL_Set_MTU( &prCSMD_Instance->rCSMD_HAL, CSMD_ETHERNET_MTU_MAX );
  
  /* Timing mode */
  if (prCSMD_Instance->rPriv.rCycClk.boActivate == TRUE)
  {
    /* Timing slave mode */
    ulEventOffset = CSMD_JITTER_MASTER_CP0_2;
  }
  else
  {
    /* Timing Master Mode */
    ulEventOffset = 0U;
  }
  
  /* Offset plus time displacement between FPGA- and Sercos-Timing */
  ulDelay = ulEventOffset + CSMD_MST_DELAY;
  prCSMD_Instance->rPriv.ulOffsetTNCT_SERCCycle = ulDelay;
  
  /* Limit for minimum event time */
  /*  ulMinEventTime = ulEventOffset + (CSMD_ULONG) (CSMD_FPGA_MDT_START_TIME + 100); */
  prCSMD_Instance->ulMinTime = 0U;
  
  /* Limit for maximum event time */
  ulMaxEventTime =   prCSMD_Instance->rPriv.ulActiveCycTime 
                   - ulEventOffset;
  prCSMD_Instance->ulMaxTime = ulMaxEventTime - ulDelay;
  
  /* Value for Event Reload */
  ulReload =   prCSMD_Instance->rPriv.ulActiveCycTime 
             + ulEventOffset;
  
  
  /* -------------------------------------------------------------
      Calculate Timer Events for CP0
     ------------------------------------------------------------- */
  
  usIdx = 0;
  
  /* Event_TINT[0] (Timer interrupt TINT0) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_0])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_0];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_0;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_TINT[1] (Timer interrupt TINT1) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_1])
  {
    ulTime  = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_1];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_1;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_TINT[2] (Timer interrupt TINT2) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_2])
  {
    ulTime  = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_2];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_2;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_TINT[3] (Timer interrupt TINT3) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_3])
  {
    ulTime  = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_3];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_3;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_Sync_Port_Set (Set the output signal CON_CLK) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_CONCLK_SET])
  {
    ulTime  = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_CONCLK_SET];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_SYNC_SET;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_Sync_Port_Reset (Reset the output signal CON_CLK) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_CONCLK_RESET])
  {
    ulTime  = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_CONCLK_RESET];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_SYNC_RESET;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_Start_MDT (Start the transmission of MDT's.) */
  arTimerEvent[usIdx].ulTime         = CSMD_FPGA_MDT_START_TIME + ulEventOffset;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_START_MDT;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_Start_AT (Start the transmission of AT's) */
  arTimerEvent[usIdx].ulTime         = CSMD_FPGA_AT_START_TIME_CP0 + ulDelay;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_START_AT;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_IP_Channel_Transmit_Open */
  arTimerEvent[usIdx].ulTime         = CSMD_FPGA_IP_WINDOW_OPEN_CP0 + CSMD_FPGA_DELAY_IP_WIN_OPEN + ulDelay;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_IP_CHANNEL_TX_OPEN;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;


  /* Event_IP_Channel_Last_Transmit */
  arTimerEvent[usIdx].ulTime         = CSMD_FPGA_IP_LAST_TRASMISS_CP0 + ulDelay;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_IP_CHANNEL_TX_LAST_TRANS;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;


  /* Event_IP_Channel_Transmit_Close */
  arTimerEvent[usIdx].ulTime         = CSMD_FPGA_IP_WINDOW_CLOSE_CP0 + ulDelay;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_IP_CHANNEL_TX_CLOSE;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_Tx_Buffer_Request_Buffer_System_A */
#if defined CSMD_PCI_MASTER && defined CSMD_PCI_MASTER_EVENT_DMA
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A])
    {
      prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A] = FALSE;
    }
    else
    {
      prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A] =
        CSMD_FPGA_TX_BUF_CHNG_REQ_CP0;
    }
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A];
    
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulEventOffset;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TX_BUFREQ_BUFSYS_A;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
#endif
  
  
  /* Event_Reload */
  arTimerEvent[usIdx].ulTime         = ulReload;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_RELOAD;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Reload_Value */
  arTimerEvent[usIdx].ulTime         = ulEventOffset;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_RELOAD_VALUE;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  /* Sort and Program Timer Events */
  eFuncRet = CSMD_Transmit_Timer_Events( prCSMD_Instance,
                                         arTimerEvent,
                                         usIdx );
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  
  /* -------------------------------------------------------------
      Calculate Port related Timer Events for CP0
     ------------------------------------------------------------- */
  
  usIdx = 0;
  
  /* Event_AT_Window_Open (Monitoring window for AT0 Sercos header is opened) */
  arPortsEvent[usIdx].ulTime         = CSMD_FPGA_AT_START_TIME_CP0 - CSMD_JITTER_MASTER_CP0_2;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_AT_WINDOW_OPEN;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_AT_Window_Close (Monitoring window for AT0 Sercos header is closed) */
  arPortsEvent[usIdx].ulTime         = CSMD_FPGA_AT_START_TIME_CP0 + CSMD_JITTER_MASTER_CP0_2;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_AT_WINDOW_CLOSE;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_IP_Channel_Receive_Open (Open IP receive window) */
  arPortsEvent[usIdx].ulTime         = CSMD_FPGA_IP_WINDOW_OPEN_CP0;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_IP_CHANNEL_RX_OPEN;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;


  /* Event_IP_Channel_Receive_Close (Close IP receive window)*/
  arPortsEvent[usIdx].ulTime         = CSMD_FPGA_IP_WINDOW_CLOSE_CP0;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_IP_CHANNEL_RX_CLOSE;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_MST_Window_Open (Monitoring window for MST is opened) */
  arPortsEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.ulActiveCycTime - CSMD_FPGA_MST_WINDOW_P0_2;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_MST_WINDOW_OPEN;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_MST_Window_Close resp. Reload (Monitoring window for MST is closed) */
  arPortsEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.ulActiveCycTime + CSMD_FPGA_MST_WINDOW_P0_2;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_MST_WINDOW_CLOSE;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_Rx_Buffer_Request_Buffer_System_A */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A])
  {
    prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A] = FALSE;
  }
  else
  {
    /* Time for Rx Buffer Change Event */
    prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A] =
        (  CSMD_FPGA_AT_START_TIME_CP0 
         + CSMD_MEDIA_LAYER_OVERHEAD)
      + (  CSMD_SERC3_AT_DATA_LENGTH_P0_V10
         + prCSMD_Instance->rPriv.ulInterFrameGap) * CSMD_BYTE_TIME;
  }
  ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A];
  
  if (ulTime > prCSMD_Instance->ulMaxTime)
    eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
  arPortsEvent[usIdx].ulTime         = ulTime;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_RX_BUFREQ_BUFSYS_A;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Reload_Value */
  arPortsEvent[usIdx].ulTime         = CSMD_FPGA_MST_WINDOW_P0_2;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_RELOAD_VALUE;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  
  /* Sort and Program Port Timer Events */
  eFuncRet = CSMD_Transmit_Port_Events( prCSMD_Instance,
                                        arPortsEvent,
                                        usIdx );
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  
  /* Set PLL cycle time (also necessary for FPGA activity LED control / Line break sensitivity) */
  CSMD_HAL_SetPLLCycleTime( &prCSMD_Instance->rCSMD_HAL,
                            prCSMD_Instance->rPriv.ulActiveCycTime );
  
#ifdef CSMD_HW_SVC_REDUNDANCY
  prCSMD_Instance->rPriv.boHW_SVC_Redundancy = TRUE;
#else
  prCSMD_Instance->rPriv.boHW_SVC_Redundancy = FALSE;
#endif

  return (eFuncRet);
  
} /* end: CSMD_Set_Register_P0() */



/**************************************************************************/ /**
\brief Sets all registers needed for preparing CP1.

\ingroup module_phase
\b Description: \n
   This function configures the following settings appropriate for CP1, including:
   - Initialization of flags relevant for redundancy
   - Clearance of pointers to all buffers
   - Inter Frame Gap (default value)
   - Event time values (minimum, maximum, reload)
   - Calculation of timer events for CP1 and CP2
   - Determination of last slave in each line for non-ring-topology

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_EVENT_TIME_MAX_LIMIT \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         04.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Set_Register_P1( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT   usI;
  CSMD_ULONG    ulReload;           /* Value for Event Reload [ns]                */
  CSMD_ULONG    ulDelay;            /* Delay between FPGA- and Sercos-Timing [ns] */
  CSMD_ULONG    ulEventOffset;      /* Value of forbidden range for events [ns]   */
/*CSMD_ULONG    ulMinEventTime;*/   /* Limit for minimum event time [ns]          */
  CSMD_ULONG    ulMaxEventTime;     /* Limit for maximum event time [ns]          */
  
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;
  
  CSMD_EVENT    arTimerEvent[ CSMD_TIMER_EVENT_NUMBER ];
  CSMD_EVENT    arPortsEvent[ CSMD_PORTS_EVENT_NUMBER ];
  CSMD_USHORT   usIdx;              /* Event Index */
  CSMD_USHORT   usBuf;
  CSMD_ULONG    ulTime;
  
  CSMD_USHORT   usSlaveIdx;
  
  prCSMD_Instance->rPriv.usTelErrCnt = 0;  /* Clear counter for successive telegram errors */
  
  for (usI = 0; usI < CSMD_MAX_TEL; usI++ )
  {                                                
    prCSMD_Instance->rPriv.aboMDT_used[usI]  = FALSE;           /* MDT telegram is not used */
    prCSMD_Instance->rPriv.aboAT_used[usI]   = FALSE;           /* AT telegram is not used  */

    for (usBuf = 0; usBuf < CSMD_MAX_RX_BUFFER; usBuf++)
    {
      /* Clear pointer to all buffers.
         Note: Multiple buffering is used from CP3. */
      prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usI] = NULL;
      prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usI] = NULL;
#ifdef CSMD_PCI_MASTER
      prCSMD_Instance->rPriv.ausTxDMA_Start_P1[usBuf] = 0;
      prCSMD_Instance->rPriv.ausTxDMA_Start_P2[usBuf] = 0;
#endif
    }
#ifdef CSMD_PCI_MASTER
    for (usBuf = 0; usBuf < CSMD_MAX_TX_BUFFER; usBuf++)
    {
      prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf]    = 0;
    }
#endif
  }
  
  
  /* register default values */
  /* prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulIER0 = 0x00000000L; */
  /* prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulIMR0 = 0x00000000L; */
  
  CSMD_HAL_IntControl( &prCSMD_Instance->rCSMD_HAL, 
                       0U,
                       FALSE,
                       TRUE,
                       0U );
  
  
  /* Don't change TCSR FPGA register because timer is already off ! / 
     keep the last setting of CYC_CLK, CON_CLK and DIV_CLK */
  
  
  /* --------------------------------------------------------- */
  /* Specify IFG value                                         */
  /* --------------------------------------------------------- */
  prCSMD_Instance->rPriv.ulInterFrameGap = CSMD_IFG_DEFAULT;
  CSMD_HAL_SetInterFrameGap( &prCSMD_Instance->rCSMD_HAL, 
                             prCSMD_Instance->rPriv.ulInterFrameGap );
  
  
  /* Timing mode */
  if (prCSMD_Instance->rPriv.rCycClk.boActivate == TRUE)
  {
    /* Timing slave mode */
    ulEventOffset = CSMD_JITTER_MASTER_CP0_2;
  }
  else
  {
    /* Timing Master Mode */
    ulEventOffset = 0U;
  }
  
  /* Offset plus time displacement between FPGA- and Sercos-Timing */
  ulDelay = ulEventOffset + CSMD_MST_DELAY;
  prCSMD_Instance->rPriv.ulOffsetTNCT_SERCCycle = ulDelay;
  
  /* Limit for minimum event time */
  /*  ulMinEventTime = ulEventOffset + (CSMD_ULONG) (CSMD_FPGA_MDT_START_TIME + 100); */
  prCSMD_Instance->ulMinTime = 0U;
  
  /* Limit for maximum event time */
  ulMaxEventTime =   prCSMD_Instance->rPriv.ulActiveCycTime 
                   - ulEventOffset;
  prCSMD_Instance->ulMaxTime = ulMaxEventTime - ulDelay;
  
  /* Value for Event Reload */
  ulReload =   prCSMD_Instance->rPriv.ulActiveCycTime 
             + ulEventOffset;
  
  
  /* -------------------------------------------------------------
      Calculate Timer Events for CP1 and CP2
     ------------------------------------------------------------- */
  
  usIdx = 0;
  
  /* Event_TINT[0] (Timer interrupt TINT0) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_0])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_0];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_0;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_TINT[1] (Timer interrupt TINT1) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_1])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_1];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_1;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_TINT[2] (Timer interrupt TINT2) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_2])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_2];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_2;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_TINT[3] (Timer interrupt TINT3) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_3])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_3];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_3;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_Sync_Port_Set (Set the output signal CON_CLK) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_CONCLK_SET])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_CONCLK_SET];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_SYNC_SET;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_Sync_Port_Reset (Reset the output signal CON_CLK) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_CONCLK_RESET])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_CONCLK_RESET];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_SYNC_RESET;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_Start_MDT (Start the transmission of MDT's.) */
  arTimerEvent[usIdx].ulTime         = CSMD_FPGA_MDT_START_TIME + ulEventOffset;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_START_MDT;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_Start_AT (Start the transmission of AT's) */
  arTimerEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1 + ulDelay;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_START_AT;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_IP_Channel_Transmit_Open */
  arTimerEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.rTimingCP12.ulOpenUCC_t6 + ulDelay + CSMD_FPGA_DELAY_IP_WIN_OPEN;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_IP_CHANNEL_TX_OPEN;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;


  /* Event_IP_Channel_Last_Transmit */
  arTimerEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.rTimingCP12.ulUCC_Latest + ulDelay;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_IP_CHANNEL_TX_LAST_TRANS;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;


  /* Event_IP_Channel_Transmit_Close */
  arTimerEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.rTimingCP12.ulCloseUCC_t7 + ulDelay;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_IP_CHANNEL_TX_CLOSE;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_Tx_Buffer_Request_Buffer_System_A */
#if defined CSMD_PCI_MASTER && defined CSMD_PCI_MASTER_EVENT_DMA
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A])
    {
      prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A] = FALSE;
    }
    else
    {
      prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A] =
        prCSMD_Instance->rPriv.rTimingCP12.ulTxBufReqA;
    }
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A];
    
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulEventOffset;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TX_BUFREQ_BUFSYS_A;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
#endif
  
  
  /* Event_Reload */
  arTimerEvent[usIdx].ulTime         = ulReload;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_RELOAD;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Reload_Value */
  arTimerEvent[usIdx].ulTime         = ulEventOffset;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_RELOAD_VALUE;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  /* Sort and program Timer Events */
  eFuncRet = CSMD_Transmit_Timer_Events( prCSMD_Instance,
                                         arTimerEvent,
                                         usIdx );
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  
  /* -------------------------------------------------------------
      Calculate Port related Timer Events for CP1 and CP2
     ------------------------------------------------------------- */
  
  usIdx = 0;
  
  /* Event_AT_Window_Open (Monitoring window for AT0 Sercos header is opened) */
  arPortsEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1 - CSMD_JITTER_MASTER_CP0_2;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_AT_WINDOW_OPEN;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_AT_Window_Close (Monitoring window for AT0 Sercos header is closed) */
  arPortsEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1 + CSMD_JITTER_MASTER_CP0_2;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_AT_WINDOW_CLOSE;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
    /* Event_IP_Channel_Receive_Open (Open IP receive window) */
    arPortsEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.rTimingCP12.ulOpenUCC_t6 + CSMD_FPGA_DELAY_IP_WIN_OPEN;
    arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_IP_CHANNEL_RX_OPEN;
    arPortsEvent[usIdx].usSubCycCnt    = 0;
    arPortsEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
    
    
    /* Event_IP_Channel_Receive_Close (Close IP receive window)*/
    arPortsEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.rTimingCP12.ulCloseUCC_t7;
    arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_IP_CHANNEL_RX_CLOSE;
    arPortsEvent[usIdx].usSubCycCnt    = 0;
    arPortsEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  
  
  /* Event_MST_Window_Open (Monitoring window for MST is opened) */
  arPortsEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.ulActiveCycTime - CSMD_FPGA_MST_WINDOW_P0_2;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_MST_WINDOW_OPEN;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_MST_Window_Close resp. Reload (Monitoring window for MST is closed) */
  arPortsEvent[usIdx].ulTime         = prCSMD_Instance->rPriv.ulActiveCycTime + CSMD_FPGA_MST_WINDOW_P0_2;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_MST_WINDOW_CLOSE;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_Rx_Buffer_Request_Buffer_System_A */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A])
  {
    prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A] = FALSE;
  }
  else
  {
    /* Time for Rx Buffer Change Event */
    prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A] =
      prCSMD_Instance->rPriv.rTimingCP12.ulRxBufReqA;
  }
  ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A];
  
  if (ulTime > prCSMD_Instance->ulMaxTime)
    eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
  arPortsEvent[usIdx].ulTime         = ulTime;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_RX_BUFREQ_BUFSYS_A;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  if (TRUE == prCSMD_Instance->rPriv.boHW_SVC_Redundancy)
  {
    /* Event_Trigger service channel processor          */
    /* Take same time like buffer change event + 100 ns */
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A] + 100;
  
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arPortsEvent[usIdx].ulTime         = ulTime;
    arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_START_SVC;
    arPortsEvent[usIdx].usSubCycCnt    = 0;
    arPortsEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Reload_Value */
  arPortsEvent[usIdx].ulTime         = CSMD_FPGA_MST_WINDOW_P0_2;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_RELOAD_VALUE;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  
  /* Sort and Program Port Timer Events */
  eFuncRet = CSMD_Transmit_Port_Events( prCSMD_Instance,
                                        arPortsEvent,
                                        usIdx );
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  
  /* Set PLL cycle time (also necessary for FPGA activity LED control / Line break sensitivity) */
  CSMD_HAL_SetPLLCycleTime( &prCSMD_Instance->rCSMD_HAL,
                            prCSMD_Instance->rPriv.ulActiveCycTime );
  

  /* At least one Sercos address was detected (ausRecogSlaveAddList) more than once. */
  if (TRUE == prCSMD_Instance->rPriv.boMultipleSAddress)
  {
    /* build private list of topology addresses by slave index */
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)  /* interrupted ring / two lines */
    {
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
      {
        prCSMD_Instance->rPriv.ausTopologyAddresses[usI] = (CSMD_USHORT)(usI + 1);

        /* determine preferred master port for slaves connected to port 1 */
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] = CSMD_PORT_1;
      }
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
      {
        prCSMD_Instance->rPriv.ausTopologyAddresses[prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb + usI] =
          (CSMD_USHORT)(prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb + usI + 1);

        /* determine preferred master port for slaves connected to port 2 */
        prCSMD_Instance->rPriv.ausPrefPortBySlave[prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb + usI] = CSMD_PORT_2;
      }
    }

    else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
    {
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
      {
        prCSMD_Instance->rPriv.ausTopologyAddresses[usI] = (CSMD_USHORT)(usI + 1);
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] = CSMD_PORT_1;
      }
    }

    else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
    {
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
      {
        prCSMD_Instance->rPriv.ausTopologyAddresses[usI] = (CSMD_USHORT)(usI + 1);
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] = CSMD_PORT_2;
      }
    }

    else  /* ring or defect ring */
    {
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
      {
        prCSMD_Instance->rPriv.ausTopologyAddresses[usI] = (CSMD_USHORT)(usI + 1);

        if (   (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING)
            && (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_SECONDARY) )
        {
          prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] = CSMD_PORT_2;
        }
        else
        {
          prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] = CSMD_PORT_1;
        }
      }
    }
  }
  else

  {
    /* build private list of topology addresses by slave index */
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)  /* interrupted ring / two lines */
    {
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
      {
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI]];
        prCSMD_Instance->rPriv.ausTopologyAddresses[usSlaveIdx] = (CSMD_USHORT)(usI + 1);

        /* determine preferred master port for slaves connected to port 1 */
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usSlaveIdx] = CSMD_PORT_1;
      }
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
      {
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI]];
        prCSMD_Instance->rPriv.ausTopologyAddresses[usSlaveIdx] = (CSMD_USHORT)(prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb + usI + 1);

        /* determine preferred master port for slaves connected to port 2 */
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usSlaveIdx] = CSMD_PORT_2;
      }
    }

    else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
    {
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
      {
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI]];
        prCSMD_Instance->rPriv.ausTopologyAddresses[usSlaveIdx] = (CSMD_USHORT)(usI + 1);
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usSlaveIdx] = CSMD_PORT_1;
      }
    }

    else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
    {
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
      {
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI]];
        prCSMD_Instance->rPriv.ausTopologyAddresses[usSlaveIdx] = (CSMD_USHORT)(usI + 1);
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usSlaveIdx] = CSMD_PORT_2;
      }
    }

    else  /* ring or defect ring */
    {
      /* ring topology */
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
      {
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI]];
        prCSMD_Instance->rPriv.ausTopologyAddresses[usSlaveIdx] = (CSMD_USHORT)(usI + 1);

        if (   (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING)
            && (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_SECONDARY) )
        {
          prCSMD_Instance->rPriv.ausPrefPortBySlave[usSlaveIdx] = CSMD_PORT_2;
        }
        else
        {
          prCSMD_Instance->rPriv.ausPrefPortBySlave[usSlaveIdx] = CSMD_PORT_1;
        }
      }
    }
  }
  
  /* set default value for number of allowed consecutive slave valid misses */
  prCSMD_Instance->rConfiguration.rComTiming.usAllowed_Slave_Valid_Miss = CSMD_DEFAULT_SLAVE_VALID_MISS;

  return (eFuncRet);
  
} /* end: CSMD_Set_Register_P1 */



/**************************************************************************/ /**
\brief Sets all registers needed for preparing CP3.

\ingroup module_phase
\b Description: \n
   This function configures the following settings appropriate for CP3, including:
   - Clearance of pointers to all buffers
   - Selection of Tx and Rx buffer systems
   - Inter Frame Gap (calculated value)
   - Event time values (minimum, maximum, reload)
   - Calculation of timer events for CP3 and CP4
   - activation sending of Sercos time in MDT0

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_EVENT_TIME_MAX_LIMIT \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         04.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Set_Register_P3( CSMD_INSTANCE *prCSMD_Instance )    
{
  
  CSMD_USHORT   usI;
  CSMD_ULONG    ulReload;           /* Value for Event Reload [ns]                  */
  CSMD_ULONG    ulDelay;            /* Delay between FPGA- and Sercos-Timing [ns]   */
  CSMD_ULONG    ulEventOffset;      /* Value of forbidden range for events [ns]     */
/*CSMD_ULONG    ulMinEventTime;*/   /* Limit for minimum event time [ns]            */
  CSMD_ULONG    ulMaxEventTime;     /* Limit for maximum event time [ns]            */
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;
  
  CSMD_EVENT    arTimerEvent[ CSMD_TIMER_EVENT_NUMBER ];
  CSMD_EVENT    arPortsEvent[ CSMD_PORTS_EVENT_NUMBER ];
  CSMD_USHORT   usIdx;              /* Event Index */
  CSMD_USHORT   usBuf;
  CSMD_ULONG    ulTime;
  
  
  for (usI = 0; usI < CSMD_MAX_TEL; usI++ )
  {                                                
    prCSMD_Instance->rPriv.aboMDT_used[usI]  = FALSE;           /* MDT telegram is not used */
    prCSMD_Instance->rPriv.aboAT_used[usI]   = FALSE;           /* AT telegram is not used  */
    
    for (usBuf = 0; usBuf < CSMD_MAX_RX_BUFFER; usBuf++)
    {
      /* Clear pointer to all buffers. */
      prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usI] = NULL;
      prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usI] = NULL;
#ifdef CSMD_PCI_MASTER
      prCSMD_Instance->rPriv.ausTxDMA_Start_P1[usBuf] = 0;
      prCSMD_Instance->rPriv.ausTxDMA_Start_P2[usBuf] = 0;
#endif
    }
#ifdef CSMD_PCI_MASTER
    for (usBuf = 0; usBuf < CSMD_MAX_TX_BUFFER; usBuf++)
    {
      prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf]    = 0;
    }
#endif
  }
  
  
  /* register default values */
  /* prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulIER0 = 0x00000000L; */
  /* prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulIMR0 = 0x00000000L; */
  
  CSMD_HAL_IntControl( &prCSMD_Instance->rCSMD_HAL, 
                       0U,
                       FALSE,
                       TRUE,
                       0U );
  
  
  /* Select TxRam buffering system */
  CSMD_HAL_Configure_Tx_Buffer( &prCSMD_Instance->rCSMD_HAL, 
                                CSMD_HAL_TX_BUFFER_SYSTEM_A, 
                                (CSMD_ULONG) prCSMD_Instance->rPriv.rHW_Init_Struct.usTxBufferMode );
  
  /* Select RxRam buffering system */
  CSMD_HAL_Configure_Rx_Buffer( &prCSMD_Instance->rCSMD_HAL, 
                                CSMD_HAL_RX_BUFFER_SYSTEM_A, 
                                (CSMD_ULONG) prCSMD_Instance->rPriv.rHW_Init_Struct.usRxBufferMode );
  
  /* Don't change TCSR fpga register because timer is already off ! / 
     keep the last setting of CYC_CLK, CON_CLK and DIV_CLK */
  
  CSMD_HAL_SetInterFrameGap( &prCSMD_Instance->rCSMD_HAL, 
                             prCSMD_Instance->rPriv.ulInterFrameGap );
  
  /* Set the MTU, so FPGA can check if a transmission is possible after last transmit event. */
  CSMD_HAL_Set_MTU( &prCSMD_Instance->rCSMD_HAL, 
                    ((CSMD_ULONG) prCSMD_Instance->rPriv.usRequested_MTU) );
  
  /* Timing mode */
  if (prCSMD_Instance->rPriv.rCycClk.boActivate == TRUE)
  {
    /* Timing slave mode */
    ulEventOffset = prCSMD_Instance->rConfiguration.rMasterCfg.ulJitter_Master;
  }
  else
  {
    /* Timing Master Mode */
    ulEventOffset = 0U;
  }
  
  /* Offset plus time displacement between FPGA- and Sercos-Timing */
  ulDelay = ulEventOffset + CSMD_MST_DELAY;
  prCSMD_Instance->rPriv.ulOffsetTNCT_SERCCycle = ulDelay;
  
  /* Limit for minimum event time */
  /*  ulMinEventTime = ulEventOffset + (CSMD_ULONG) (CSMD_FPGA_MDT_START_TIME + 100); */
  prCSMD_Instance->ulMinTime = 0U;
  
  /* Limit for maximum event time */
  ulMaxEventTime =   prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 
                   - ulEventOffset;
  prCSMD_Instance->ulMaxTime = ulMaxEventTime - ulDelay;
  
  /* Value for Event Reload */
  ulReload =   prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 
             + ulEventOffset;
  
  
  /* -------------------------------------------------------------
      Calculate Timer Events for CP3 and CP4
     ------------------------------------------------------------- */
  
  usIdx = 0;
  
  /* Event_TINT[0] (Timer interrupt TINT0) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_0])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_0];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_0;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_TINT[1] (Timer interrupt TINT1) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_1])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_1];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_1;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_TINT[2] (Timer interrupt TINT2) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_2])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_2];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_2;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_TINT[3] (Timer interrupt TINT3) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TIMER_3])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TIMER_3];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TINT_3;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_Sync_Port_Set (Set the output signal CON_CLK) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_CONCLK_SET])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_CONCLK_SET];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_SYNC_SET;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_Sync_Port_Reset (Reset the output signal CON_CLK) */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_CONCLK_RESET])
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_CONCLK_RESET];
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_SYNC_RESET;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_Start_MDT (Start the transmission of MDT's.) */
  arTimerEvent[usIdx].ulTime         = CSMD_FPGA_MDT_START_TIME + ulEventOffset;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_START_MDT;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_Start_AT (Start the transmission of AT's) */
  arTimerEvent[usIdx].ulTime         = prCSMD_Instance->rConfiguration.rComTiming.ulATTxStartTimeT1_S01006 + ulDelay;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_START_AT;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* UC channel bandwidth [ns] */
  if (prCSMD_Instance->rPriv.ulUCC_Width)
  {
    /* Event_IP_Channel_Transmit_Open */
    arTimerEvent[usIdx].ulTime         = prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017 + CSMD_FPGA_DELAY_IP_WIN_OPEN
      + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_IP_CHANNEL_TX_OPEN;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
    
    
    /* Event_IP_Channel_Last_Transmit */
    arTimerEvent[usIdx].ulTime         = prCSMD_Instance->rConfiguration.rUC_Channel.ulLatestTransmissionUC 
      + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_IP_CHANNEL_TX_LAST_TRANS;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
    
    
    /* Event_IP_Channel_Transmit_Close */
    arTimerEvent[usIdx].ulTime         = prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 
      + ulDelay;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_IP_CHANNEL_TX_CLOSE;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_Tx_Buffer_Request_Buffer_System_A */
  /* Set default state to "not configured" */
  prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A] = FALSE;
#if defined CSMD_PCI_MASTER && defined CSMD_PCI_MASTER_EVENT_DMA
  if (   (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
      || (prCSMD_Instance->rPriv.rHW_Init_Struct.usTxBufferMode >= CSMD_TX_DOUBLE_BUFFER))
#else
  if (prCSMD_Instance->rPriv.rHW_Init_Struct.usTxBufferMode >= CSMD_TX_DOUBLE_BUFFER)
#endif
  {
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A];

    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arTimerEvent[usIdx].ulTime         = ulTime + ulEventOffset;
    arTimerEvent[usIdx].usType         = CSMD_EVENT_TX_BUFREQ_BUFSYS_A;
    arTimerEvent[usIdx].usSubCycCnt    = 0;
    arTimerEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Event_Reload */
  arTimerEvent[usIdx].ulTime         = ulReload;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_RELOAD;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Reload_Value */
  arTimerEvent[usIdx].ulTime         = ulEventOffset;
  arTimerEvent[usIdx].usType         = CSMD_EVENT_RELOAD_VALUE;
  arTimerEvent[usIdx].usSubCycCnt    = 0;
  arTimerEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  /* Sort and Program Timer Events */
  eFuncRet = CSMD_Transmit_Timer_Events( prCSMD_Instance,
                                         arTimerEvent,
                                         usIdx );
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  
  /* -------------------------------------------------------------
      Calculate Port related Timer Events for CP3 and CP4
     ------------------------------------------------------------- */
  
  usIdx = 0;
  /*
      |                                            AT_Win_Open|       |Close
      |                                                       |       |
      |               -+--------------------------------------+---+---+--
      |- tNetwork P1 ->|                                          |
      |                TMST[1]                                    |
      |                                                           |
  +-- MDT0 --+                            +------- AT0 ------+    |
  |MST|--------------- t1 --------------->|header|           |    |
     -+-----------------------------------+------+-----------+--  |
      |                                          |- tNetwork P1 ->|
      |                                          |
      |                                          |---- tNetwork P2 ---->|
      |                      TMST[2]                                    |
      |---- tNetwork P2 ---->|                                          |
      |                     -+--------------------------------------+---+---+--
      |                                                             |       |
      |                                                  AT_Win_Open|       |Close
   */
  
  /* Event_AT_Window_Open (Monitoring window for AT0 Sercos header is opened) */
  ulTime = ( (prCSMD_Instance->rConfiguration.rComTiming.ulATTxStartTimeT1_S01006 + CSMD_TIME_SIII_HEADER + 100)
            - prCSMD_Instance->rConfiguration.rComTiming.ulSyncJitter_S01023/2);
  if (ulTime > prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002) /* Prevents underflow */
    ulTime = CSMD_TIME_SIII_HEADER;
  arPortsEvent[usIdx].ulTime         = ulTime;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_AT_WINDOW_OPEN;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_AT_Window_Close (Monitoring window for AT0 Sercos header is closed) */
  ulTime = ( (prCSMD_Instance->rConfiguration.rComTiming.ulATTxStartTimeT1_S01006 + CSMD_TIME_SIII_HEADER + 100)
            + prCSMD_Instance->rConfiguration.rComTiming.ulSyncJitter_S01023/2);
  arPortsEvent[usIdx].ulTime         = ulTime;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_AT_WINDOW_CLOSE;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* UC channel bandwidth [ns] */
  if (prCSMD_Instance->rPriv.ulUCC_Width)
  {
    /* Event_IP_Channel_Receive_Open (Open IP receive window) */
    arPortsEvent[usIdx].ulTime         = prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017;
    arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_IP_CHANNEL_RX_OPEN;
    arPortsEvent[usIdx].usSubCycCnt    = 0;
    arPortsEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
    
    
    /* Event_IP_Channel_Receive_Close (Close IP receive window)*/
    arPortsEvent[usIdx].ulTime         = prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017;
    arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_IP_CHANNEL_RX_CLOSE;
    arPortsEvent[usIdx].usSubCycCnt    = 0;
    arPortsEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  /*
      Start_MST                                  Start_MST
      |                                          |
     -+------------------------------------------+------
      |
      |                                           MST_Win_Open|_______|Close
      |- tNetwork P1 ->|                                      |       |
      |               -+--------------------------------------+---+---+---
      |                |                                          |
      |                TMST[1]                                    TMST[1]
      |
      |                                                MST_Win_Open|_______|Close
      |--- tNetwork P2 ---->|                                      |       |
                           -+--------------------------------------+---+---+--
                            |                                          |
                            TMST[2]                                    TMST[2]
   */
  
  /* Event_MST_Window_Open (Monitoring window for MST is opened) */
  ulTime = prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 
    - (prCSMD_Instance->rConfiguration.rComTiming.ulSyncJitter_S01023/2);
  arPortsEvent[usIdx].ulTime         = ulTime;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_MST_WINDOW_OPEN;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_MST_Window_Close (Monitoring window for MST is closed) */
  /* Note:  The MST Window Close event has to be at lest 200ppm
   *        greater than the cycle time to compensate runtime
   *        differences of the oscillators. */
  ulTime =   prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002
           + ((prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 * 200) / 1000000)
           + (prCSMD_Instance->rConfiguration.rComTiming.ulSyncJitter_S01023/2);
  arPortsEvent[usIdx].ulTime         = ulTime;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_MST_WINDOW_CLOSE;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  /* Event_Rx_Buffer_Request_Buffer_System_A */
  if (TRUE == prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A])
  {
    prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A] = FALSE;
  }
  ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A];
  
  if (ulTime > prCSMD_Instance->ulMaxTime)
    eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
  arPortsEvent[usIdx].ulTime         = ulTime;
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_RX_BUFREQ_BUFSYS_A;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  
  if (TRUE == prCSMD_Instance->rPriv.boHW_SVC_Redundancy)
  {
    /* Event_Trigger service channel processor          */
    /* Take same time like buffer change event + 100 ns */
    ulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A] + 100;
  
    if (ulTime > prCSMD_Instance->ulMaxTime)
      eFuncRet = CSMD_EVENT_TIME_MAX_LIMIT;
    arPortsEvent[usIdx].ulTime         = ulTime;
    arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_START_SVC;
    arPortsEvent[usIdx].usSubCycCnt    = 0;
    arPortsEvent[usIdx].usSubCycCntSel = 0;
    usIdx++;
  }
  
  
  /* Reload_Value */
  arPortsEvent[usIdx].ulTime         = (prCSMD_Instance->rConfiguration.rComTiming.ulSyncJitter_S01023/2);
  arPortsEvent[usIdx].usType         = CSMD_PORT_EVENT_RELOAD_VALUE;
  arPortsEvent[usIdx].usSubCycCnt    = 0;
  arPortsEvent[usIdx].usSubCycCntSel = 0;
  usIdx++;
  
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  
  /* Sort and Program Port Timer Events */
  eFuncRet = CSMD_Transmit_Port_Events( prCSMD_Instance,
                                        arPortsEvent,
                                        usIdx );
  if (eFuncRet != CSMD_NO_ERROR)
    return (eFuncRet);
  
  
  /* Enable telegram insertion of Sercos time */
  CSMD_HAL_Enable_SERCOS_Time( &prCSMD_Instance->rCSMD_HAL, TRUE );
  
  /* Set PLL cycle time (also necessary for FPGA activity LED control / Line break sensitivity) */
  CSMD_HAL_SetPLLCycleTime( &prCSMD_Instance->rCSMD_HAL,
                            prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 );
  
  return (eFuncRet);
  
} /* end: CSMD_Set_Register_P3() */



/**************************************************************************/ /**
\brief  Sorts timer events depending on time values and transmits
        the sorted list into FPGA.
 
\ingroup module_phase
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
   
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   parPortEvent
              Pointer to timer event list. Last entry must be the reload value!
\param [in]   usNumber
              Number of programmed timer events (including reload event and reload value)

\return       \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n
              
\author       WK
\date         09.02.2008

***************************************************************************** */
CSMD_FUNC_RET  CSMD_Transmit_Timer_Events( CSMD_INSTANCE *prCSMD_Instance, 
                                           CSMD_EVENT    *parTimerEvent,
                                           CSMD_USHORT    usNumber )
{
  
  CSMD_USHORT  usIdx;
  CSMD_SHORT   sJ;
  CSMD_USHORT  usTemp;
  CSMD_USHORT  ausIndex[ CSMD_TIMER_EVENT_NUMBER ];
  CSMD_EVENT  *prTimerEvent;
  
  if (usNumber > CSMD_TIMER_EVENT_NUMBER)
  {
    return (CSMD_SYSTEM_ERROR);
  }
  
  for (usIdx = 0; usIdx < CSMD_TIMER_EVENT_NUMBER; usIdx++ )
  {
    ausIndex[usIdx] = usIdx;
  }
  
  /* Indirect sorting (sorting the index) */
  for (usIdx = 0; usIdx < usNumber - 2; usIdx++)
  {
    for (sJ = (CSMD_SHORT)(usNumber - 2); sJ > (CSMD_SHORT)usIdx; sJ-- )
    {
      if (parTimerEvent[ausIndex[sJ-1]].ulTime > parTimerEvent[ausIndex[sJ]].ulTime)
      {
        usTemp         = ausIndex[sJ-1];
        ausIndex[sJ-1] = ausIndex[sJ];
        ausIndex[sJ]   = usTemp;
      }
    }
  }
  
#ifdef CSMD_DEBUG
  for (usIdx = 0; usIdx < usNumber; usIdx++)
  {
    /* Sorted Event table */
    prCSMD_Instance->rCSMD_Debug.arEvTimerSorted[ usIdx ] = parTimerEvent [ausIndex[usIdx]];
  }
#endif
  
  /* Check for Event_Reload and Reload_Value are last in sorted table */
  if (   (usNumber < 2)
      || (parTimerEvent[ ausIndex[usNumber - 2] ].usType != CSMD_EVENT_RELOAD)
      || (parTimerEvent[ ausIndex[usNumber - 1] ].usType != CSMD_EVENT_RELOAD_VALUE))
  {
    return (CSMD_SYSTEM_ERROR);
  }
  
  /* Write sorted Event Table */
  for (usIdx = 0; usIdx < usNumber; usIdx++)
  {
    /* Get sorted index */
    prTimerEvent = &parTimerEvent[ausIndex[usIdx]];
    
    CSMD_HAL_SetTimerEvent( &prCSMD_Instance->rCSMD_HAL,
                            usIdx,
                            prTimerEvent->ulTime,
                            prTimerEvent->usType,
                            prTimerEvent->usSubCycCnt,
                            prTimerEvent->usSubCycCntSel );
  }
  
  /* Clear unused Events */
  for ( ; usIdx < CSMD_TIMER_EVENT_NUMBER; usIdx++)
  {
    CSMD_HAL_SetTimerEvent( &prCSMD_Instance->rCSMD_HAL,
                            usIdx,
                            0U,
                            CSMD_NO_EVENT,
                            0U,
                            0U );
  }
  
#ifdef CSMD_DEBUG
  for (usIdx = 0; usIdx < CSMD_TIMER_EVENT_NUMBER; usIdx++)
  {
    /* Read back from FPGA */
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulMTDSR, 
                        (CSMD_ULONG) usIdx );
    
    prCSMD_Instance->rCSMD_Debug.aulEvTimerFPGA[usIdx][0] = 
      CSMD_HAL_ReadLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulMTDRL );
    prCSMD_Instance->rCSMD_Debug.aulEvTimerFPGA[usIdx][1] = 
      CSMD_HAL_ReadLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulMTDRU );
  }
#endif
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Transmit_Timer_Events() */



/**************************************************************************/ /**
\brief  Sorts port related timer events depending on time values and
        transmits the sorted list into FPGA.
 
\ingroup module_phase
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
   
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   parPortEvent
              pointer to port timer event list. Last entry must be the reload value!
\param [in]   usNumber
              Number of programmed timer events (including reload event and reload value)

\return       \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n
              
\author       WK
\date         09.02.2008

***************************************************************************** */
CSMD_FUNC_RET  CSMD_Transmit_Port_Events( CSMD_INSTANCE *prCSMD_Instance, 
                                          CSMD_EVENT    *parPortEvent,
                                          CSMD_USHORT    usNumber )
{
  
  CSMD_USHORT  usIdx;
  CSMD_SHORT   sJ;
  CSMD_USHORT  usTemp;
  CSMD_USHORT  ausIndex[ CSMD_PORTS_EVENT_NUMBER ];
  CSMD_EVENT  *prPortEvent;
  
  if (usNumber > CSMD_PORTS_EVENT_NUMBER)
  {
    return (CSMD_SYSTEM_ERROR);
  }
  
  for (usIdx = 0; usIdx < CSMD_PORTS_EVENT_NUMBER; usIdx++ )
  {
    ausIndex[usIdx] = usIdx;
  }
  
  /* Indirect sorting (sorting the index) */
  for (usIdx = 0; usIdx < usNumber - 2; usIdx++)
  {
    for (sJ = (CSMD_SHORT)(usNumber - 2); sJ > (CSMD_SHORT)usIdx; sJ-- )
    {
      if (parPortEvent[ausIndex[sJ-1]].ulTime > parPortEvent[ausIndex[sJ]].ulTime)
      {
        usTemp         = ausIndex[sJ-1];
        ausIndex[sJ-1] = ausIndex[sJ];
        ausIndex[sJ]   = usTemp;
      }
    }
  }
  
#ifdef CSMD_DEBUG
  for (usIdx = 0; usIdx < usNumber; usIdx++)
  {
    /* Sorted Event table */
    prCSMD_Instance->rCSMD_Debug.arEvPortsSorted[ usIdx ] = parPortEvent [ausIndex[usIdx]];
  }
#endif
  
  /* Check for Event_Reload and Reload_Value are last in sorted table */
  if (   (usNumber < 2)
      || (parPortEvent[ ausIndex[usNumber - 2] ].usType != CSMD_PORT_EVENT_MST_WINDOW_CLOSE)    /* equates to Reload Event */
      || (parPortEvent[ ausIndex[usNumber - 1] ].usType != CSMD_PORT_EVENT_RELOAD_VALUE))
  {
    return (CSMD_SYSTEM_ERROR);
  }
  
  /* Write sorted Event Table */
  for (usIdx = 0; usIdx < usNumber; usIdx++)
  {
    /* Get sorted index */
    prPortEvent = &parPortEvent[ausIndex[usIdx]];
    
    CSMD_HAL_SetPortEvent( &prCSMD_Instance->rCSMD_HAL,
                           usIdx,
                           prPortEvent->ulTime,
                           prPortEvent->usType,
                           prPortEvent->usSubCycCnt,
                           prPortEvent->usSubCycCntSel );
  }
  
  /* Clear unused Events */
  for ( ; usIdx < CSMD_PORTS_EVENT_NUMBER; usIdx++)
  {
    CSMD_HAL_SetPortEvent( &prCSMD_Instance->rCSMD_HAL,
                           usIdx,
                           0U,
                           CSMD_NO_EVENT,
                           0U,
                           0U );
  }
  
#ifdef CSMD_DEBUG
  for (usIdx = 0; usIdx < CSMD_PORTS_EVENT_NUMBER; usIdx++)
  {
    /* Read back from FPGA */
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulPTDSR, 
                        (CSMD_ULONG) usIdx );
    
    prCSMD_Instance->rCSMD_Debug.aulEvPortsFPGA[usIdx][0] = 
      CSMD_HAL_ReadLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulPTDRL );
    prCSMD_Instance->rCSMD_Debug.aulEvPortsFPGA[usIdx][1] = 
      CSMD_HAL_ReadLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulPTDRU );
  }
#endif
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Transmit_Port_Events() */


/*! \endcond */ /* PRIVATE */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
27 Nov 2013 WK
  Defdb00000000
  - CSMD_Set_Register_P0(), CSMD_Set_Register_P1()
    Set the IP events unconditional. 
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
29 May 2015 WK
  - CSMD_Set_Register_P1()
    In case of multiple recognized addresses the lists ausTopologyAddresses[]
    and ausPrefPortBySlave[] were not build.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
15 Mar 2016 WK
  Defdb00185943
  CSMD_Set_Register_P3()
  - Event AT window open/close times fixed.
    (Correction negligible in CP0 to CP2.)
  - Adjust Event MST window close referred to IP-Core documentation.
  
------------------------------------------------------------------------------
*/
