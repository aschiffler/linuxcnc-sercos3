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
 \file   CSMD_CFG_PARA.c
 \author WK
 \date   01.09.2010
 \brief  This File contains the public API functions and private functions for
         the transmission of the slave configuration.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#include "CSMD_CONFIG.h"
#include "CSMD_DIAG.h"
#include "CSMD_PRIV_SVC.h"

#define SOURCE_CSMD
#include "CSMD_CFG_PARA.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/**************************************************************************/ /**
\brief Writes the parameters for the determined telegram structure
       and timing to the available slaves.

\ingroup func_timing
\b Description: \n
   This function transmits the following parameters to all slaves:

- <B>Write to slaves with the basic classes SCP_FixCFG or SCP_VarCFG:</B>
  - S-0-1002 Communication Cycle time (tScyc)
  - S-0-1009 Device Control (C-DEV) Offset in MDT
  - S-0-1010 Lengths of MDTs
  - S-0-1011 Device Status (S-DEV) Offset in AT
  - S-0-1012 Lengths of ATs
  - S-0-1013 SVC offset in MDT
  - S-0-1014 SVC offset in AT
  - S-0-1003 Allowed MST losses in CP3&CP4
  - S-0-1050.x.3 Connection: Telegram Assignment
  - S-0-1017 UC transmission time

- <B>Write to slaves with the basic class SCP_VarCFG:</B>
  - S-0-1050.x.2 Connection: Connection Number

- <B>Write to slaves with the class SCP_Sync or SCP_Cyc:</B>
  - S-0-1006 AT0 transmission starting time (t1)

- <B>Write to slaves with the class SCP_Sync:</B>
  - S-0-1007 Synchronization time (tSync)
  - S-0-1008 Command value valid time (t3)
  - S-0-1015 Ring delay
  - S-0-1023 SYNC jitter

- <B>Write to slaves with the classes SCP_Sync or SCP_WDCon:</B>
  - S-0-1050.x.11 Connection: Allowed Data Losses

- <B>Write to slaves with the classes SCP_Sync, SCP_WD or SCP_WDCon:</B>
  - S-0-1050.x.10 Connection: Producer Cycle Time

- <B>Write to slaves with the class SCP_Sync_0x02:</B>
  - S-0-1036 Inter Frame Gap

- <B>Write to slaves with the class SCP_Sync_0x03:</B>
  - S-0-1061 Maximum TSref-Counter

- <B>Write to slaves with the classes SCP_Sync_0x03, SCP_SysTime or SCP_SWC:</B>
  - S-0-1032 Communication control

- <B>Write to slaves with the classes SCP_NRT or SCP_NRTPC:</B>
  - S-0-1027.0.1 Requested MTU

- <B>Write to slaves with the class SCP_NRTPC:</B>
  - S-0-1048 Activate network settings

- <B>Write to slaves with the class SCP_RTB:</B>
  - S-0-1050.x.20 Connection: IDN Allocation of real-time bit
  - S-0-1050.x.21 Connection: Bit allocation of real-time bit

- <B>Read from slaves with the classes SCP_NRT or SCP_NRTPC:</B>
  - S-0-1027.0.2 Effective MTU
  
<B>Call Environment:</B> \n
   This function can only be called in CP2. If called in another
   communication phase, an error will be generated.\n
   The call-up should be performed from a task.\n
   This function comprises a state machine.
  

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures
  
\return       \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_WRONG_ELEMENT_NBR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_NO_ERROR \n
  
\author       RA
\date         20.04.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_TransmitTiming( CSMD_INSTANCE          *prCSMD_Instance,
                                   CSMD_FUNC_STATE        *prFuncState,
                                   CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_USHORT    usI;
  CSMD_FUNC_RET  eFuncRet;
  
  
  /* --------------------------------------------------------- */
  /* Step 1 :: Check current communication phase               */
  /* --------------------------------------------------------- */
  if (prCSMD_Instance->sCSMD_Phase != CSMD_SERC_PHASE_2)
  {
    prFuncState->ulSleepTime = 0UL;
    return (CSMD_WRONG_PHASE);
  }
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    /* state machine for internal function call */
    prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0U;
    prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
    
    /* state machine for this function */
    prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_1:
    
    eFuncRet = CSMD_Transmit_SCP_Basic( prCSMD_Instance,
                                        &prCSMD_Instance->rPriv.rInternalFuncState,
                                        parSvcMacro );
    
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0U;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    }
    else 
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_2:
    
    eFuncRet = CSMD_Transmit_SCP_VarCFG( prCSMD_Instance,
                                         &prCSMD_Instance->rPriv.rInternalFuncState,
                                         parSvcMacro );
    
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0U;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_3;
    }
    else 
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_3:
    
    eFuncRet = CSMD_Transmit_SCP_Sync( prCSMD_Instance,
                                       &prCSMD_Instance->rPriv.rInternalFuncState,
                                       parSvcMacro );
    
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0U;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_4;
    }
    else 
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_4:
    
    eFuncRet = CSMD_Transmit_SCP_NRT( prCSMD_Instance,
                                      &prCSMD_Instance->rPriv.rInternalFuncState,
                                      parSvcMacro );
    
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0U;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_5;
    }
    else 
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_5:
#ifndef CSMD_DISABLE_CMD_S_0_1048
    eFuncRet = CSMD_Proceed_Cmd_S_0_1048( prCSMD_Instance,
                                          &prCSMD_Instance->rPriv.rInternalFuncState,
                                          parSvcMacro );
    
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0U;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;

      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_6;
    }
    else 
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      break;
    }
#endif
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_6:
    
    eFuncRet = CSMD_Transmit_SCP_RTB( prCSMD_Instance,
                                      &prCSMD_Instance->rPriv.rInternalFuncState,
                                      parSvcMacro );
    
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0U;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_7;
    }
    else 
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_7:
    
    eFuncRet = CSMD_ReadConfig( prCSMD_Instance,
                                &prCSMD_Instance->rPriv.rInternalFuncState,
                                parSvcMacro );
    
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0U;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_8;
    }
    else 
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      break;
    }
    /*lint -fallthrough */
  

  case CSMD_FUNCTION_STEP_8:
    
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    eFuncRet = CSMD_NO_ERROR;
    break;
    
    
  default:
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == CSMD_SLAVE_ACTIVE)
      {
        parSvcMacro[usI].usCancelActTrans = 1U;
        (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
      }
    }
    eFuncRet = CSMD_ILLEGAL_CASE;
    break;
    
  }
  
  return (eFuncRet);
  
}   /* End: CSMD_TransmitTiming() */

/*! \endcond */ /* PUBLIC */


/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Transmits all parameters related to the SCP class "basic", this means
       SCP_FixCFG or SCP_VarCFG.

\ingroup module_phase
\b Description: \n
   This function transmits the parameters related to the basic classification
   SCP_Basic to all slaves which support this SCP class.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_WRONG_ELEMENT_NBR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         20.08.2007

***************************************************************************** */
CSMD_FUNC_RET CSMD_Transmit_SCP_Basic( CSMD_INSTANCE          *prCSMD_Instance,
                                       CSMD_FUNC_STATE        *prFuncState,
                                       CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_ULONG                *paulSCP_ConfigMask;
  CSMD_SLAVE_CONFIGURATION  *parSlaveConfig;
  CSMD_CONNECTION           *parConnect;

  CSMD_USHORT  usI;              /* Slave index      */
  CSMD_USHORT  usFinished;
  CSMD_USHORT  usConnIdx;        /* Index in "Connections list"    */
  
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  
  
  /* Points to slave configuration of first slave */
  parSlaveConfig = &prCSMD_Instance->rConfiguration.parSlaveConfig[0];
  parConnect     = &prCSMD_Instance->rConfiguration.parConnection[0];
  
  /* Points to configuration bitlist of first slave */
  paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1002;
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
    /*lint -fallthrough */
    
    
    /* --------------------------------------------------------------------- */
    /* Communication parameters                                              */
    /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_1:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write Parameter S-0-1002 "Communication Cycle time (tScyc)" to all slaves */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */ 
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1002;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 4U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }                
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
              
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        }  /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC) */ 
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1009;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
    /* --------------------------------------------------------------------- */
    /* Telegram configuration parameters                                     */
    /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_2:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write Parameter S-0-1009 "Device Control offset in MDT" to all slaves */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = &prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTelegramConfig.usC_DEV_OffsetMDT_S01009;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1009;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 2U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }                
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        }  /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC) */ 
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1010;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_3;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_3:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write Parameter S-0-1010 "Lengths of MDTs" to all slaves */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] = (CSMD_USHORT)(2 * CSMD_MAX_TEL);
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[1] = (CSMD_USHORT)(2 * CSMD_MAX_TEL);
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[2] = prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[0];
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[3] = prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[1];
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[4] = prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[2];
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[5] = prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[3];
              
                parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1010;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_IS_LIST;
                parSvcMacro[usI].usLength               = (CSMD_USHORT) (2 * CSMD_MAX_TEL + 4);
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }                
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        }  /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC) */ 
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1011;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_4;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_4:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write Parameter S-0-1011 "Device Status offset in AT" to all slaves */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = &prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTelegramConfig.usS_DEV_OffsetAT_S01011;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1011;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 2U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }                
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        }  /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC) */ 
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1012;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_5;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_5:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write Parameter S-0-1012 "Lengths of ATs" to all slaves */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] = 2 * CSMD_MAX_TEL;
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[1] = 2 * CSMD_MAX_TEL;
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[2] = prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[0];
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[3] = prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[1];
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[4] = prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[2];
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[5] = prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[3];
              
                parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1012;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_IS_LIST;
                parSvcMacro[usI].usLength               = (CSMD_USHORT) (2 * CSMD_MAX_TEL + 4);
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }                
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
          
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        }  /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC) */ 
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1013;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_6;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_6:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write Parameter S-0-1013 "SVC offset in MDT" to all slaves */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = &prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTelegramConfig.usSvcOffsetMDT_S01013;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1013;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 2U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }                
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
          
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        }  /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC) */ 
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1014;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_7;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_7:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write Parameter S-0-1014 "SVC offset in AT" to all slaves */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = &prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTelegramConfig.usSvcOffsetAT_S01014;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1014;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 2U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }                
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        }  /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC) */ 
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1003;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_8;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
    /* --------------------------------------------------------------------- */
    /* Sercos bus errors and diagnostics parameters                          */
    /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_8:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write Parameter S-0-1003 "Allowed MST losses in CP3/CP4" to all slaves */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0] =
                  (CSMD_ULONG) prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTiming.usMaxNbrTelErr_S1003;
                parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1003;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 4U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }                
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
          
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        }  /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC) */ 
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_9;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
    /* --------------------------------------------------------------------- */
    /* Connection parameters                                                 */
    /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_9:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write Parameter S-0-1050.x.3 "Telegram Assignment" for all slaves and all connections x */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
#endif
        {
          if (parSvcMacro[usI].usState != CSMD_REQUEST_ERROR)
          {
            /* still connections for this slave to check? */
            if (   (prCSMD_Instance->rPriv.ausActConnection[usI] < parSlaveConfig[usI].usMaxNbrOfConnections)
                && (prCSMD_Instance->rPriv.ausActConnection[usI] < CSMD_MAX_CONNECTIONS))
            {
              usConnIdx = parSlaveConfig[usI].arConnIdxList[prCSMD_Instance->rPriv.ausActConnection[usI]].usConnIdx;
              if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
              {
                /* check if MBUSY is set, if it is not set do nothing */
                if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
                {
                  if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
                  {
                    prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] = parConnect[usConnIdx].usS_0_1050_SE3;
                    parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                    parSvcMacro[usI].usSlaveIdx             = usI;
                    parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                    parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1050, prCSMD_Instance->rPriv.ausActConnection[usI], 3 );
                    parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                    parSvcMacro[usI].usLength               = 2U;
                    parSvcMacro[usI].ulAttribute            = 0U;
                    parSvcMacro[usI].usCancelActTrans       = 0U;
                    parSvcMacro[usI].usPriority             = 1U;
                    parSvcMacro[usI].usOtherRequestCanceled = 0U;
                    parSvcMacro[usI].usSvchError            = 0U;
                    parSvcMacro[usI].usInternalReq          = 1U;
                  }
                  eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
                  
                  if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                         || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
                  {
                    prCSMD_Instance->rExtendedDiag.ulIDN = parSvcMacro[usI].ulIdent_Nbr;
                    CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                  }
                  
                  if (parSvcMacro[usI].usState == CSMD_DATA_VALID) /* if ready write next Configuration */
                  {
                    /* select next connection */
                    prCSMD_Instance->rPriv.ausActConnection[usI]++;
                    
                    /* start request for next connection */
                    parSvcMacro[usI].usState = CSMD_START_REQUEST;
                  }
                }
              }
              else
              {
                /* select next connection */
                prCSMD_Instance->rPriv.ausActConnection[usI]++;
              }
              
              if (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)
              {
                 CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
              }
              else
              {
                if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                    && (   (prCSMD_Instance->rPriv.ausActConnection[usI] >= parSlaveConfig[usI].usMaxNbrOfConnections)
                        || (prCSMD_Instance->rPriv.ausActConnection[usI] >= CSMD_MAX_CONNECTIONS)) )
                {
                  CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
                }
                else
                {
                  usFinished = 0U;
                }
              }
            } /* if (prCSMD_Instance->rPriv.ausActConnection[usI] < parSlaveConfig[usI].usMaxNbrOfConnections) */
          } /* if (parSvcMacro[usI].usState != CSMD_REQUEST_ERROR) */
        } /* End: if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC) ... */
      }  /* End: if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == ... */
    } /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      
      /* continue with next step */
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_10;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_10:
    
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);
    
    
  default:
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usCancelActTrans = 1U;
        (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
      }
    }
    return (CSMD_ILLEGAL_CASE);
    
  }
  
  return (CSMD_FUNCTION_IN_PROCESS);
  
}   /* End: CSMD_Transmit_SCP_Basic() */



/**************************************************************************/ /**
\brief Transmits all parameters related to the class SCP_VarCFG.

\ingroup module_phase
\b Description: \n
   This function transmits the parameters related to the basic classification
   SCP_VarCFG to all slaves which support this SCP class.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_WRONG_ELEMENT_NBR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         20.08.2007
   
***************************************************************************** */
CSMD_FUNC_RET CSMD_Transmit_SCP_VarCFG( CSMD_INSTANCE          *prCSMD_Instance,
                                        CSMD_FUNC_STATE        *prFuncState,
                                        CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_ULONG                *paulSCP_ConfigMask;
  CSMD_SLAVE_CONFIGURATION  *parSlaveConfig;
  CSMD_CONNECTION           *parConnect;

  CSMD_USHORT  usI;              /* Slave index      */
  CSMD_USHORT  usFinished;
  CSMD_USHORT  usConnIdx;        /* Index in "Connections list"    */
  
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  
  
  /* Points to slave configuration of first slave */
  parSlaveConfig = &prCSMD_Instance->rConfiguration.parSlaveConfig[0];
  parConnect     = &prCSMD_Instance->rConfiguration.parConnection[0];

  /* Points to configuration bitlist of first slave */
  paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
    /*lint -fallthrough */
    
    
    /* --------------------------------------------------------------------- */
    /* Connection parameters                                                 */
    /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_1:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write Parameter S-0-1050.x.2 "Connection Number" for all slaves and all connections x */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_VARCFG)
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_VARCFG)
#endif
        {
          if (parSvcMacro[usI].usState != CSMD_REQUEST_ERROR)
          {
            /* still connections for this slave to check? */
            if (   (prCSMD_Instance->rPriv.ausActConnection[usI] < parSlaveConfig[usI].usMaxNbrOfConnections)
                && (prCSMD_Instance->rPriv.ausActConnection[usI] < CSMD_MAX_CONNECTIONS))
            {
              usConnIdx = parSlaveConfig[usI].arConnIdxList[prCSMD_Instance->rPriv.ausActConnection[usI]].usConnIdx;
              if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
              {
                /* check if MBUSY is set, if it is not set do nothing */
                if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
                {
                  if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
                  {
                    prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] = parConnect[usConnIdx].usS_0_1050_SE2;
                    parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                    parSvcMacro[usI].usSlaveIdx             = usI;
                    parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1050, prCSMD_Instance->rPriv.ausActConnection[usI], 2 );
                    parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                    parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                    parSvcMacro[usI].usLength               = 2U;
                    parSvcMacro[usI].ulAttribute            = 0U;
                    parSvcMacro[usI].usCancelActTrans       = 0U;
                    parSvcMacro[usI].usPriority             = 1U;
                    parSvcMacro[usI].usOtherRequestCanceled = 0U;
                    parSvcMacro[usI].usSvchError            = 0U;
                    parSvcMacro[usI].usInternalReq          = 1U;
                  }
                  eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
                  
                  if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                         || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
                  {
                    prCSMD_Instance->rExtendedDiag.ulIDN = parSvcMacro[usI].ulIdent_Nbr;
                    CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                  }
                  
                  if (parSvcMacro[usI].usState == CSMD_DATA_VALID) /* if ready write next configuration */
                  {
                    /* select next connection */
                    prCSMD_Instance->rPriv.ausActConnection[usI]++;
                    
                    /* start request for next connection */
                    parSvcMacro[usI].usState = CSMD_START_REQUEST;
                  }
                }
              }
              else
              {
                /* select next connection */
                prCSMD_Instance->rPriv.ausActConnection[usI]++;
              }
              
              if (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)
              {
                 CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
              }
              else
              {
                if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                    && (   (prCSMD_Instance->rPriv.ausActConnection[usI] >= parSlaveConfig[usI].usMaxNbrOfConnections)
                        || (prCSMD_Instance->rPriv.ausActConnection[usI] >= CSMD_MAX_CONNECTIONS)) )
                {
                  CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
                }
                else
                {
                  usFinished = 0U;
                }
              }
            }
          }
        }
      }
    } /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      /* continue with next step */
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
    /* --------------------------------------------------------------------- */
    /* Communication parameters                                              */
    /* --------------------------------------------------------------------- */
    
    /* --------------------------------------------------------------------- */
    /* Telegram configuration parameters                                     */
    /* --------------------------------------------------------------------- */
    
    /* --------------------------------------------------------------------- */
    /* Sercos bus errors and diagnostics parameters                          */
    /* --------------------------------------------------------------------- */
    
    
  case CSMD_FUNCTION_STEP_2:
    
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);
    
    
  default:
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usCancelActTrans = 1U;
        (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
      }
    }
    return (CSMD_ILLEGAL_CASE);
    
  }
  
  return (CSMD_FUNCTION_IN_PROCESS);
  
}   /* End: CSMD_Transmit_SCP_VarCFG() */



/**************************************************************************/ /**
\brief Transmits all parameters related to the class SCP_Sync.

\ingroup module_phase
\b Description: \n
   This function transmits the parameters related to the additive function group
   SCP_Sync to all slaves which support this SCP class.


<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_WRONG_ELEMENT_NBR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         20.08.2007

***************************************************************************** */
CSMD_FUNC_RET CSMD_Transmit_SCP_Sync( CSMD_INSTANCE          *prCSMD_Instance,
                                      CSMD_FUNC_STATE        *prFuncState,
                                      CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_ULONG                *paulSCP_ConfigMask;
  CSMD_SLAVE_CONFIGURATION  *parSlaveConfig;
  CSMD_CONNECTION           *parConnect;

  CSMD_USHORT    usI;         /* Slave index      */
  CSMD_USHORT    usFinished;
  CSMD_USHORT    usConnIdx;   /* Index in "Connections list"    */
  
  CSMD_USHORT    usNumSlaves = prCSMD_Instance->rSlaveList.usNumProjSlaves;
  CSMD_FUNC_RET  eFuncRet    = CSMD_NO_ERROR;
  
  
  /* Points to slave configuration of first slave */
  parSlaveConfig = &prCSMD_Instance->rConfiguration.parSlaveConfig[0];
  parConnect     = &prCSMD_Instance->rConfiguration.parConnection[0];

  /* Points to configuration bitlist of first slave */
  paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    for (usI = 0; usI < usNumSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1006;
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
    /*lint -fallthrough */
    
    
    /* --------------------------------------------------------------------- */
    /* Communication parameters                                              */
    /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_1:
    
    usFinished = 1U;
    
    /* SCP_Sync/SCP_Cyc: Write Parameter S-0-1006 "AT transmission starting time (t1)" to all slaves. */
    for (usI = 0; usI < usNumSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC | CSMD_SCP_CYC))
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC | CSMD_SCP_CYC))
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->rConfiguration.rComTiming.ulATTxStartTimeT1_S01006;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1006;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 4U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
              
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < usNumSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1007;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_2:
    
    usFinished = 1U;
    
    /* SCP_Sync: Write parameter S-0-1007 "Synchronization time (tSync)" to all slaves. */
    for (usI = 0; usI < usNumSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC)
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC)
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->rConfiguration.rComTiming.ulSynchronizationTime_S01007;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1007;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 4U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < usNumSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1008;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_3;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_3:
    
    usFinished = 1U;
    
    /* SCP_Sync: Write parameter S-0-1008 "Command value valid time (t3)" to all slaves. */
    for (usI = 0; usI < usNumSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC) 
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->rConfiguration.rComTiming.ulCmdValidTimeT3_S01008;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1008;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 4U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
              
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < usNumSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1032;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_4;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_4:
    
    usFinished = 1U;
    {
      CSMD_ULONG ulCommunicationControl = 0;

      ulCommunicationControl |= CSMD_S_0_1032_EF_MDT0;  /* Master has configured the extended field for transmission of time and TSref-counter */

      if (CSMD_HAL_IsSoftMaster( &prCSMD_Instance->rCSMD_HAL ) == TRUE)
      {
        ulCommunicationControl |= CSMD_S_0_1032_MQUAL_GRADE;  /* Master quality grade: MST jitter > 1 microsecond */
      }
      /* SCP_Sync_V3/SCP_sysTime/SCP_SWC: Write Parameter S-0-1032 "Communication control" to all slaves */
      for (usI = 0; usI < usNumSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          if (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC_V3 | CSMD_SCP_SYSTIME | CSMD_SCP_SWC))
          {
            if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                   || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
            {
              /* check if MBUSY is set, if it is not set do nothing */
              if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
              {
                if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
                {
                  prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0] = ulCommunicationControl;
                  if (prCSMD_Instance->rPriv.rSWC_Struct.boNoForwardLastSlave)
                  {
                    if (   (prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[usI+2] == prCSMD_Instance->usSercAddrLastSlaveP1)
                        || (prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[usI+2] == prCSMD_Instance->usSercAddrLastSlaveP2))
                    {
                      prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0] |= CSMD_S_0_1032_LB_NO_FORWARD;
                    }
                  }
                  parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                  parSvcMacro[usI].usSlaveIdx             = usI;
                  parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                  parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1032;
                  parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                  parSvcMacro[usI].usLength               = 4U;
                  parSvcMacro[usI].ulAttribute            = 0U;
                  parSvcMacro[usI].usCancelActTrans       = 0U;
                  parSvcMacro[usI].usPriority             = 1U;
                  parSvcMacro[usI].usOtherRequestCanceled = 0U;
                  parSvcMacro[usI].usSvchError            = 0U;
                  parSvcMacro[usI].usInternalReq          = 1U;
                }
                eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );

                if ( !(   (eFuncRet == CSMD_NO_ERROR)
                       || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
                {
                  CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                }
              }

              if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                  || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
              {
                CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
              }
              else
              {
                usFinished = 0U;
              }
            }
          } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC_V3) */
        }
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < usNumSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1015;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_5;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_5:
    
    usFinished = 1U;
    
    /* SCP_Sync: Write parameter S-0-1015 "Ring delay" to all slaves. */
    for (usI = 0; usI < usNumSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC) 
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                /* Transmit ringdelay port specific */
                if ( prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] == CSMD_PORT_1 )
                {
                  parSvcMacro[usI].pusAct_Data = 
                    (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015;
                }
                else
                {
                  parSvcMacro[usI].pusAct_Data = 
                    (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015_P2;
                }
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1015;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 4U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < usNumSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1023;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_6;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_6:
    
    usFinished = 1U;
    
    /* SCP_Sync: Write parameter S-0-1023 "SYNC jitter" to all slaves. */
    for (usI = 0; usI < usNumSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC)
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC)
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->rConfiguration.rComTiming.ulSyncJitter_S01023;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1023;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 4U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
              
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < usNumSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_7;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  /* --------------------------------------------------------------------- */
  /* Connection parameters                                                 */
  /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_7:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write parameter S-0-1050.x.10 "Producer Cycle Time" for all slaves and all connections x */
    for (usI = 0; usI < usNumSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC | CSMD_SCP_WD | CSMD_SCP_WDCON))
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC | CSMD_SCP_WD | CSMD_SCP_WDCON))
#endif
        {
          if (parSvcMacro[usI].usState != CSMD_REQUEST_ERROR)
          {
            /* still connections for this slave to check? */
            if (   (prCSMD_Instance->rPriv.ausActConnection[usI] < parSlaveConfig[usI].usMaxNbrOfConnections)
                && (prCSMD_Instance->rPriv.ausActConnection[usI] < CSMD_MAX_CONNECTIONS))
            {
              usConnIdx = parSlaveConfig[usI].arConnIdxList[prCSMD_Instance->rPriv.ausActConnection[usI]].usConnIdx;
              if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
              {
                /* check if MBUSY is set, if it is not set do nothing */
                if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
                {
                  if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
                  {
                    if (parConnect[usConnIdx].ulS_0_1050_SE10 != 0)
                    {
                      prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0] =
                        parConnect[usConnIdx].ulS_0_1050_SE10;
                    }
                    else
                    {
                      /* If tPcyc is unknown (value = 0, use tScyc instead.*/
                      prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0] =
                        prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002;
                    }
                    parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                    parSvcMacro[usI].usSlaveIdx             = usI;
                    parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1050, prCSMD_Instance->rPriv.ausActConnection[usI], 10 );
                    parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                    parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                    parSvcMacro[usI].usLength               = 4U;
                    parSvcMacro[usI].ulAttribute            = 0U;
                    parSvcMacro[usI].usCancelActTrans       = 0U;
                    parSvcMacro[usI].usPriority             = 1U;
                    parSvcMacro[usI].usOtherRequestCanceled = 0U;
                    parSvcMacro[usI].usSvchError            = 0U;
                    parSvcMacro[usI].usInternalReq          = 1U;
                  }
                  eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
              
                  if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                         || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
                  {
                    prCSMD_Instance->rExtendedDiag.ulIDN = parSvcMacro[usI].ulIdent_Nbr;
                    CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                  }
                  
                  if (parSvcMacro[usI].usState == CSMD_DATA_VALID) /* if ready write next Configuration */
                  {
                    /* select next connection */
                    prCSMD_Instance->rPriv.ausActConnection[usI]++;
                    
                    /* start request for next connection */
                    parSvcMacro[usI].usState = CSMD_START_REQUEST;
                  }
                }
              }
              else
              {
                /* select next connection */
                prCSMD_Instance->rPriv.ausActConnection[usI]++;
              }
              
              if (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)
              {
                 CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
              }
              else
              {
                if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                    && (   (prCSMD_Instance->rPriv.ausActConnection[usI] >= parSlaveConfig[usI].usMaxNbrOfConnections)
                        || (prCSMD_Instance->rPriv.ausActConnection[usI] >= CSMD_MAX_CONNECTIONS)) )
                {
                  CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
                }
                else
                {
                  usFinished = 0U;
                }
              }
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC | CSMD_SCP_WD)) */
      }
    } /* End: for (usI = 0; usI < usNumSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      for (usI = 0; usI < usNumSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      /* continue with next step */
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_8;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_8:
    
    usFinished = 1U;
    
    /* SCP_Sync: Write Parameter S-0-1050.x.11 "Allowed Data Losses" for all slaves and all connections x */
    for (usI = 0; usI < usNumSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC | CSMD_SCP_WDCON))
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC | CSMD_SCP_WDCON))
#endif
        {
          if (parSvcMacro[usI].usState != CSMD_REQUEST_ERROR)
          {
            /* still connections for this slave to check? */
            if (   (prCSMD_Instance->rPriv.ausActConnection[usI] < parSlaveConfig[usI].usMaxNbrOfConnections)
                && (prCSMD_Instance->rPriv.ausActConnection[usI] < CSMD_MAX_CONNECTIONS))
            {
              usConnIdx = parSlaveConfig[usI].arConnIdxList[prCSMD_Instance->rPriv.ausActConnection[usI]].usConnIdx;
              if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
              {
                /* check if MBUSY is set, if it is not set do nothing */
                if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
                {
                  if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
                  {
                    prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] = parConnect[usConnIdx].usS_0_1050_SE11;
                    parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                    parSvcMacro[usI].usSlaveIdx             = usI;
                    parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1050, prCSMD_Instance->rPriv.ausActConnection[usI], 11 );
                    parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                    parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                    parSvcMacro[usI].usLength               = 2U;
                    parSvcMacro[usI].ulAttribute            = 0U;
                    parSvcMacro[usI].usCancelActTrans       = 0U;
                    parSvcMacro[usI].usPriority             = 1U;
                    parSvcMacro[usI].usOtherRequestCanceled = 0U;
                    parSvcMacro[usI].usSvchError            = 0U;
                    parSvcMacro[usI].usInternalReq          = 1U;
                  }
                  eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
                  
                  if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                         || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
                  {
                    prCSMD_Instance->rExtendedDiag.ulIDN = parSvcMacro[usI].ulIdent_Nbr;
                    CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                  }
                  
                  if (parSvcMacro[usI].usState == CSMD_DATA_VALID) /* if ready write next configuration */
                  {
                    /* select next connection */
                    prCSMD_Instance->rPriv.ausActConnection[usI]++;
                    
                    /* start request for next connection */
                    parSvcMacro[usI].usState = CSMD_START_REQUEST;
                  }
                }
              }
              else
              {
                /* select next connection */
                prCSMD_Instance->rPriv.ausActConnection[usI]++;
              }
              
              if (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)
              {
                 CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
              }
              else
              {
                if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                    && (   (prCSMD_Instance->rPriv.ausActConnection[usI] >= parSlaveConfig[usI].usMaxNbrOfConnections)
                        || (prCSMD_Instance->rPriv.ausActConnection[usI] >= CSMD_MAX_CONNECTIONS)) )
                {
                  CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
                }
                else
                {
                  usFinished = 0U;
                }
              }
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC) */
      }
    } /* End: for (usI = 0; usI < usNumSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      for (usI = 0; usI < usNumSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      /* continue with next step */
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1036;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_9;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_9:
    
    usFinished = 1U;
    
    /* SCP_Sync: Write Parameter S-0-1036 "Inter Frame Gap" to all slaves */
    for (usI = 0; usI < usNumSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
               || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
        {
          /* check if MBUSY is set, if it is not set do nothing */
          if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
          {
            if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
            {
              prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] = (CSMD_USHORT)prCSMD_Instance->rPriv.ulInterFrameGap;
              
              parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
              parSvcMacro[usI].usSlaveIdx             = usI;
              parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
              parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1036;
              parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
              parSvcMacro[usI].usLength               = 2U;
              parSvcMacro[usI].ulAttribute            = 0U;
              parSvcMacro[usI].usCancelActTrans       = 0U;
              parSvcMacro[usI].usPriority             = 1U;
              parSvcMacro[usI].usOtherRequestCanceled = 0U;
              parSvcMacro[usI].usSvchError            = 0U;
              parSvcMacro[usI].usInternalReq          = 1U;
            }
            eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
            if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                   || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
            {
              if (   (eFuncRet == CSMD_SVC_ERROR_MESSAGE) 
                  && (parSvcMacro[usI].usSvchError == (CSMD_USHORT) CSMD_SVC_ID_NOT_THERE)
                  && !(paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC_V2) )
              {
                eFuncRet = CSMD_NO_ERROR;
              }
              else
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
          }
          
          if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
              || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
          {
            CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
          }
          else
          {
            usFinished = 0U;
          }
        }
      }  /* End: if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == ... */
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < usNumSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1061;
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_10;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_10:
    
    usFinished = 1U;
    
    /* SCP_Sync_V3: Write Parameter S-0-1061 "Maximum TSref-Counter" to all slaves */
    for (usI = 0; usI < usNumSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC_V3)
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC_V3)
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->rConfiguration.rComTiming.usMaxTSRefCount_S1061;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1061;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 2U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
              
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC_V3) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < usNumSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_11;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
    /* --------------------------------------------------------------------- */
    /* Telegram configuration parameters                                     */
    /* --------------------------------------------------------------------- */
    
    /* --------------------------------------------------------------------- */
    /* Sercos bus errors and diagnostics parameters                          */
    /* --------------------------------------------------------------------- */
    
    
  case CSMD_FUNCTION_STEP_11:
    
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);
    
    
  default:
    
    for (usI = 0; usI < usNumSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usCancelActTrans = 1U;
        (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
      }
    }
    return (CSMD_ILLEGAL_CASE);
    
  }
  
  return (CSMD_FUNCTION_IN_PROCESS);
  
}   /* End: CSMD_Transmit_SCP_Sync() */



/**************************************************************************/ /**
\brief Transmits all parameters related to the classes SCP_NRT or SCP_NRTPC.

\ingroup module_phase
\b Description: \n
   This function transmits the parameters related to the additive function group
   SCP_NRT to all slaves which support this SCP class.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_WRONG_ELEMENT_NBR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         20.08.2007

***************************************************************************** */
CSMD_FUNC_RET CSMD_Transmit_SCP_NRT( CSMD_INSTANCE          *prCSMD_Instance,
                                     CSMD_FUNC_STATE        *prFuncState,
                                     CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_USHORT     usI;
  CSMD_USHORT     usFinished;
  CSMD_FUNC_RET   eFuncRet;
  CSMD_ULONG     *paulSCP_ConfigMask;
  
  
  /* Points to configuration bitlist of first slave */
  paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1017;
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
    /*lint -fallthrough */
    
    
  /* ------------------------------------------------------------------------ */
  /* UC channel parameters                                                    */
  /* ------------------------------------------------------------------------ */
  case CSMD_FUNCTION_STEP_1:
    
    usFinished = 1U;
    
    /* SCP_Basic: Write parameter S-0-1017 "UC transmission time" for all slaves. */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
            && !(prCSMD_Instance->rConfiguration.parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC)   /* SCP_FixCFG or SCP_VarCFG */
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] = 0x0008;
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[1] = 0x0008;
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[1] = prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017;
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[2] = prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017;
              
                parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1017;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_IS_LIST;
                parSvcMacro[usI].usLength               = 12U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }                
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prFuncState->ulSleepTime             = 0U;
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_( 1027, 0, 1 );

      /* UC channel configured? */
      if (prCSMD_Instance->rPriv.ulUCC_Width)
      {
        prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
      }
      else
      {
        prFuncState->usActState  = CSMD_FUNCTION_STEP_3;
        break;
      }
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_2:
    
    usFinished = 1U;
    
    /* SCP_NRT: Write parameter S-0-1027.0.1 "Requested MTU" for all slaves. */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & (CSMD_SCP_NRT | CSMD_SCP_NRTPC))
            && !(prCSMD_Instance->rConfiguration.parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & (CSMD_SCP_NRT | CSMD_SCP_NRTPC))
#endif
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] = prCSMD_Instance->rPriv.usRequested_MTU;
                
                parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1027, 0, 1);
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 2U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }                
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & (CSMD_SCP_NRT | CSMD_SCP_NRTPC)) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_3;
    }
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_3:
    
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);
    
    
  default:
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usCancelActTrans = 1U;
        (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
      }
    }
    return (CSMD_ILLEGAL_CASE);
    
  }
  
  return (CSMD_FUNCTION_IN_PROCESS);
  
}   /* End: CSMD_Transmit_SCP_NRT() */



/**************************************************************************/ /**
\brief Transmits all parameters related to the class SCP_RTB.

\ingroup module_phase
\b Description: \n
   This function transmits the parameters related to the additive function group
   SCP_RTB to all slaves which support this SCP class.


<B>Call Enrivonment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_WRONG_ELEMENT_NBR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         12.01.2011

***************************************************************************** */
CSMD_FUNC_RET CSMD_Transmit_SCP_RTB( CSMD_INSTANCE          *prCSMD_Instance,
                                     CSMD_FUNC_STATE        *prFuncState,
                                     CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_ULONG                *paulSCP_ConfigMask;
  CSMD_SLAVE_CONFIGURATION  *parSlaveConfig;
  CSMD_REALTIME_BIT         *parRTBits;

  CSMD_USHORT  usI;              /* Slave index      */
  CSMD_USHORT  usFinished;
  CSMD_USHORT  usConnIdx;        /* Index in "Connections list"    */
  CSMD_USHORT  usRtbIdx;         /* Index in "RTB configuration list" */
  
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  
  
  /* Points to slave configuration of first slave */
  parSlaveConfig = &prCSMD_Instance->rConfiguration.parSlaveConfig[0];
  parRTBits      = &prCSMD_Instance->rConfiguration.parRealTimeBit[0];
  
  /* Points to configuration bitlist of first slave */
  paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
    /*lint -fallthrough */
    
    
  /* ------------------------------------------------------------------------ */
  /* Real time bit configuration parameters                                   */
  /* ------------------------------------------------------------------------ */
  case CSMD_FUNCTION_STEP_1:
    
    usFinished = 1U;
    
    /* SCP_RTB: Write parameter S-0-1050.x.20 "IDN Allocation of real-time bit" for all slaves and all connections x */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_RTB)
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (CSMD_SCP_RTB & paulSCP_ConfigMask[usI])
#endif
        {
          if (parSvcMacro[usI].usState != CSMD_REQUEST_ERROR)
          {
            /* still connections for this slave to check? */
            if (   (prCSMD_Instance->rPriv.ausActConnection[usI] < parSlaveConfig[usI].usMaxNbrOfConnections)
                && (prCSMD_Instance->rPriv.ausActConnection[usI] < CSMD_MAX_CONNECTIONS))
            {
              usConnIdx = parSlaveConfig[usI].arConnIdxList[prCSMD_Instance->rPriv.ausActConnection[usI]].usConnIdx;
              if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
              {
                /* check if MBUSY is set, if it is not set do nothing */
                if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
                {
                  if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
                  {
                    CSMD_USHORT usLength;
                    
                    usRtbIdx = parSlaveConfig[usI].arConnIdxList[prCSMD_Instance->rPriv.ausActConnection[usI]].usRTBitsIdx;
                    if (usRtbIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig)
                    {
                      usLength = (CSMD_USHORT)parRTBits[usRtbIdx].ulS_0_1050_SE20[0]; /* Get current list length */
                      if (usLength < 4)
                      {
                        /* Empty list */
                        usLength = 0;
                      }
                      else if (usLength < 8)
                      {
                        /* List contains 1 element --> transmit 2 elements */
                        usLength = 8;
                        prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[1] = parRTBits[usRtbIdx].ulS_0_1050_SE20[1];
                        prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[2] = CSMD_IDN_S_0_0000;
                      }
                      else
                      {
                        /* List contains 2 elements */
                        usLength = 8;
                        prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[1] = parRTBits[usRtbIdx].ulS_0_1050_SE20[1];
                        prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[2] = parRTBits[usRtbIdx].ulS_0_1050_SE20[2];
                      }
                    }
                    else
                    {
                      /* RTB not configured, write empty list */
                      usLength = 0;
                    }
                    prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] = usLength;
                    prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[1] = usLength;
                    
                    parSvcMacro[usI].usLength               = (CSMD_USHORT) (usLength + 4); /* length inclusive current/maximum length */
                    parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                    parSvcMacro[usI].usSlaveIdx             = usI;
                    parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1050, prCSMD_Instance->rPriv.ausActConnection[usI], 20 );
                    parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                    parSvcMacro[usI].usIsList               = CSMD_ELEMENT_IS_LIST;
                    parSvcMacro[usI].ulAttribute            = 0U;
                    parSvcMacro[usI].usCancelActTrans       = 0U;
                    parSvcMacro[usI].usPriority             = 1U;
                    parSvcMacro[usI].usOtherRequestCanceled = 0U;
                    parSvcMacro[usI].usSvchError            = 0U;
                    parSvcMacro[usI].usInternalReq          = 1U;
                  }
                  eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
              
                  if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                         || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
                  {
                    prCSMD_Instance->rExtendedDiag.ulIDN = parSvcMacro[usI].ulIdent_Nbr;
                    CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                  }
                  
                  if (parSvcMacro[usI].usState == CSMD_DATA_VALID) /* if ready write next Configuration */
                  {
                    /* select next connection */
                    prCSMD_Instance->rPriv.ausActConnection[usI]++;
                    
                    /* start request for next connection */
                    parSvcMacro[usI].usState = CSMD_START_REQUEST;
                  }
                }
              }
              else
              {
                /* select next connection */
                prCSMD_Instance->rPriv.ausActConnection[usI]++;
              }
              
              if (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)
              {
                 CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
              }
              else
              {
                if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                    && (   (prCSMD_Instance->rPriv.ausActConnection[usI] >= parSlaveConfig[usI].usMaxNbrOfConnections)
                        || (prCSMD_Instance->rPriv.ausActConnection[usI] >= CSMD_MAX_CONNECTIONS)) )
                {
                  CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
                }
                else
                {
                  usFinished = 0U;
                }
              }
            }
          }
        } /* End: if (CSMD_SCP_RTB & paulSCP_ConfigMask[usI]) */
      }
    } /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      /* continue with next step */
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_2:
    
    usFinished = 1U;
    
    /* SCP_RTB: Write Parameter S-0-1050.x.21 "Bit allocation of real-time bit" for all slaves and all connections x */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & CSMD_SCP_RTB)
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (CSMD_SCP_RTB & paulSCP_ConfigMask[usI])
#endif
        {
          if (parSvcMacro[usI].usState != CSMD_REQUEST_ERROR)
          {
            /* still connections for this slave to check? */
            if (   (prCSMD_Instance->rPriv.ausActConnection[usI] < parSlaveConfig[usI].usMaxNbrOfConnections)
                && (prCSMD_Instance->rPriv.ausActConnection[usI] < CSMD_MAX_CONNECTIONS))
            {
              usConnIdx = parSlaveConfig[usI].arConnIdxList[prCSMD_Instance->rPriv.ausActConnection[usI]].usConnIdx;
              if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
              {
                /* check if MBUSY is set, if it is not set do nothing */
                if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
                {
                  if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
                  {
                    CSMD_USHORT usLength;
                    
                    usRtbIdx = parSlaveConfig[usI].arConnIdxList[prCSMD_Instance->rPriv.ausActConnection[usI]].usRTBitsIdx;
                    if (usRtbIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig)
                    {
                      usLength = parRTBits[usRtbIdx].usS_0_1050_SE21[0]; /* Get current list length */
                      if (usLength < 2)
                      {
                        /* Empty list */
                        usLength = 0;
                      }
                      else if (usLength < 4)
                      {
                        /* List contains 1 element --> transmit 2 elements */
                        usLength = 4;
                        prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[2] = parRTBits[usRtbIdx].usS_0_1050_SE21[2];
                        prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[3] = 0;
                      }
                      else
                      {
                        /* List contains 2 elements */
                        usLength = 4;
                        prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[2] = parRTBits[usRtbIdx].usS_0_1050_SE21[2];
                        prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[3] = parRTBits[usRtbIdx].usS_0_1050_SE21[3];
                      }
                    }
                    else
                    {
                      /* RTB not configured, write empty list */
                      usLength = 0;
                    }
                    prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] = usLength;
                    prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[1] = usLength;
                    
                    parSvcMacro[usI].usLength               = (CSMD_USHORT) (usLength + 4); /* length inclusive current/maximum length */
                    parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                    parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1050, prCSMD_Instance->rPriv.ausActConnection[usI], 21 );
                    parSvcMacro[usI].usSlaveIdx             = usI;
                    parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                    parSvcMacro[usI].usIsList               = CSMD_ELEMENT_IS_LIST;
                    parSvcMacro[usI].ulAttribute            = 0U;
                    parSvcMacro[usI].usCancelActTrans       = 0U;
                    parSvcMacro[usI].usPriority             = 1U;
                    parSvcMacro[usI].usOtherRequestCanceled = 0U;
                    parSvcMacro[usI].usSvchError            = 0U;
                    parSvcMacro[usI].usInternalReq          = 1U;
                  }
                  eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
              
                  if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                         || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
                  {
                    prCSMD_Instance->rExtendedDiag.ulIDN = parSvcMacro[usI].ulIdent_Nbr;
                    CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                  }
                  
                  if (parSvcMacro[usI].usState == CSMD_DATA_VALID) /* if ready write next Configuration */
                  {
                    /* select next connection */
                    prCSMD_Instance->rPriv.ausActConnection[usI]++;
                    
                    /* start request for next connection */
                    parSvcMacro[usI].usState = CSMD_START_REQUEST;
                  }
                }
              }
              else
              {
                /* select next connection */
                prCSMD_Instance->rPriv.ausActConnection[usI]++;
              }
              
              if (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)
              {
                 CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
              }
              else
              {
                if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                    && (   (prCSMD_Instance->rPriv.ausActConnection[usI] >= parSlaveConfig[usI].usMaxNbrOfConnections)
                        || (prCSMD_Instance->rPriv.ausActConnection[usI] >= CSMD_MAX_CONNECTIONS)) )
                {
                  CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
                }
                else
                {
                  usFinished = 0U;
                }
              }
            }
          }
        } /* End: if (CSMD_SCP_RTB & paulSCP_ConfigMask[usI]) */
      }
    } /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      
      /* continue with next step */
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_3;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_3:
    
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);
    
    
  default:
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usCancelActTrans = 1U;
        (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
      }
    }
    return (CSMD_ILLEGAL_CASE);
    
  }
  
  return (CSMD_FUNCTION_IN_PROCESS);
  
}   /* End: CSMD_Transmit_SCP_RTB() */



#ifndef CSMD_DISABLE_CMD_S_0_1048
/**************************************************************************/ /**
\brief Process "Activate network settings" procedure command
       for all slaves with class SCP_NRTPC.

\ingroup module_phase
\b Description: \n
   This function performs a full execution of command S-0-1048 "Activate 
   network settings procedure command" processing all steps of the 
   command state machine.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance                          
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_S_0_1048_CMD_ERROR \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         14.12.2011

***************************************************************************** */
CSMD_FUNC_RET CSMD_Proceed_Cmd_S_0_1048( CSMD_INSTANCE          *prCSMD_Instance,
                                         CSMD_FUNC_STATE        *prFuncState,
                                         CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_ULONG             *paulSCP_ConfigMask;
  CSMD_USHORT             usI;
  CSMD_USHORT             usFinished;
  CSMD_SVCH_MACRO_STRUCT *pSvcMacro;
  CSMD_FUNC_RET           eFuncRet = CSMD_NO_ERROR;
  
  
  /* Points to configuration bitlist of first slave */
  paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    prFuncState->ulSleepTime = 0UL;
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prCSMD_Instance->rExtendedDiag.ulIDN       = CSMD_IDN_S_0_1048;
    
    if (prCSMD_Instance->rSlaveList.usNumProjSlaves == 0)
    {
      prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
      
      return (CSMD_NO_ERROR);
    }
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    
    prFuncState->usActState  = CSMD_FUNCTION_CLR_CMD;
    /* continue with next case ! */
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_CLR_CMD:
    
    usFinished = 1U;
    
    /* ------------------------------------------------------------------------ */
    /* Clear procedure command                                                  */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) 
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_CMD_CLEARED)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                pSvcMacro                         = &parSvcMacro[usI];
                pSvcMacro->pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                pSvcMacro->usSlaveIdx             = usI;
                pSvcMacro->usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                pSvcMacro->ulIdent_Nbr            = CSMD_IDN_S_0_1048;
                pSvcMacro->usIsList               = CSMD_ELEMENT_NO_LIST;
                pSvcMacro->usLength               = 2U;
                pSvcMacro->ulAttribute            = CSMD_ATTR_GENERIC_PROC_CMD;
                pSvcMacro->usCancelActTrans       = 0U;
                pSvcMacro->usPriority             = 1U;
                pSvcMacro->usOtherRequestCanceled = 0U;
                pSvcMacro->usSvchError            = 0U;
                pSvcMacro->usInternalReq          = 1U;
              }
              eFuncRet = CSMD_ClearCommand( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_CMD_CLEARED)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (CSMD_S_0_1048_CMD_ERROR);
      }
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      /* continue with next case */
      /* prFuncState->ulSleepTime = CSMD_WAIT_10MS; */
      prFuncState->usActState = CSMD_FUNCTION_SET_CMD;
    }
    
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_SET_CMD:
    
    usFinished = 1U;
    
    /* ------------------------------------------------------------------------ */
    /* Set procedure command                                                    */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) 
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_CMD_ACTIVE)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                pSvcMacro                         = &parSvcMacro[usI];
                pSvcMacro->pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                pSvcMacro->usSlaveIdx             = usI;
                pSvcMacro->usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                pSvcMacro->ulIdent_Nbr            = CSMD_IDN_S_0_1048;
                pSvcMacro->usIsList               = CSMD_ELEMENT_NO_LIST;
                pSvcMacro->usLength               = 2U;
                pSvcMacro->ulAttribute            = CSMD_ATTR_GENERIC_PROC_CMD;
                pSvcMacro->usCancelActTrans       = 0U;
                pSvcMacro->usPriority             = 1U;
                pSvcMacro->usOtherRequestCanceled = 0U;
                pSvcMacro->usSvchError            = 0U;
                pSvcMacro->usInternalReq          = 1U;
              }
              eFuncRet = CSMD_SetCommand( prCSMD_Instance, &parSvcMacro[usI], NULL );
              
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                /* Ignore "CMD already active", caused by multi-slave-devices */
                if ( !(   (eFuncRet == CSMD_SVC_ERROR_MESSAGE)
                       && (parSvcMacro[usI].usSvchError == (CSMD_USHORT)CSMD_SVC_COM_ACTIVE_NOW)) )
                {
                  CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                }
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_CMD_ACTIVE)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (CSMD_S_0_1048_CMD_ERROR);
      }
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      /* continue with next case */
      /* prFuncState->ulSleepTime = CSMD_WAIT_10MS; */
      prFuncState->usActState = CSMD_FUNCTION_SET_CHECK;
    }
    
    else 
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_SET_CHECK:
    
    usFinished = 1U;
    
    /* ------------------------------------------------------------------------ */
    /* Check if procedure command properly executed                             */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) 
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_CMD_STATUS_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                pSvcMacro                         = &parSvcMacro[usI];
                pSvcMacro->pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                pSvcMacro->usSlaveIdx             = usI;
                pSvcMacro->usElem                 = CSMD_SVC_DBE1_IDN;
                pSvcMacro->ulIdent_Nbr            = CSMD_IDN_S_0_1048;
                pSvcMacro->usIsList               = CSMD_ELEMENT_NO_LIST;
                pSvcMacro->usLength               = 4U;
                pSvcMacro->ulAttribute            = CSMD_ATTR_GENERIC_PROC_CMD;
                pSvcMacro->usCancelActTrans       = 0U;
                pSvcMacro->usPriority             = 1U;
                pSvcMacro->usOtherRequestCanceled = 0U;
                pSvcMacro->usSvchError            = 0U;
                pSvcMacro->usInternalReq          = 1U;
              }
              eFuncRet = CSMD_ReadCmdStatus( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_CMD_STATUS_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) */
      }
    }
    
    if (usFinished)
    {
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {    
          if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) 
          {
            if (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)
              continue;
            
            if (parSvcMacro[usI].usState == CSMD_CMD_STATUS_VALID)
            {
              /* Procedure command executing (0x7) */
              if (parSvcMacro[usI].pusAct_Data[0] == CSMD_CMD_RUNNING)
              {
                usFinished = 0U;
                /* read command state again */
                parSvcMacro[usI].usState = CSMD_START_REQUEST;
              }
            }
          } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) */
        }
      }
    }
    
    if (usFinished)
    {
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
            (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) 
          {
            /* Procedure command not properly executed (0x3) */
            if (parSvcMacro[usI].pusAct_Data[0] != CSMD_CMD_FINISHED)
            {
              eFuncRet = CSMD_S_0_1048_CMD_ERROR;
              
              CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
            }
            parSvcMacro[usI].usState = CSMD_START_REQUEST;
          } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) */
        }
      }
      prFuncState->usActState = CSMD_FUNCTION_CLR_CMD_AGAIN;
    }
    else
    {
      /* read command state again */
      prFuncState->usActState = CSMD_FUNCTION_SET_CHECK;
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_CLR_CMD_AGAIN:
    
    usFinished = 1U;
    
    /* ------------------------------------------------------------------------ */
    /* Clear procedure command again                                            */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) 
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_CMD_CLEARED)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                pSvcMacro                         = &parSvcMacro[usI];
                pSvcMacro->pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                pSvcMacro->usSlaveIdx             = usI;
                pSvcMacro->usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                pSvcMacro->ulIdent_Nbr            = CSMD_IDN_S_0_1048;
                pSvcMacro->usIsList               = CSMD_ELEMENT_NO_LIST;
                pSvcMacro->usLength               = 2U;
                pSvcMacro->ulAttribute            = CSMD_ATTR_GENERIC_PROC_CMD;
                pSvcMacro->usCancelActTrans       = 0U;
                pSvcMacro->usPriority             = 1U;
                pSvcMacro->usOtherRequestCanceled = 0U;
                pSvcMacro->usSvchError            = 0U;
                pSvcMacro->usInternalReq          = 1U;
              }
              eFuncRet = CSMD_ClearCommand( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if (   (eFuncRet != CSMD_NO_ERROR) 
                  && (eFuncRet != CSMD_INTERNAL_REQUEST_PENDING))
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            if (   (parSvcMacro[usI].usState == CSMD_CMD_CLEARED)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_NRTPC) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (CSMD_S_0_1048_CMD_ERROR);
      }
      
      /* continue with next case */
      /* prFuncState->ulSleepTime = CSMD_WAIT_10MS; */
      prFuncState->usActState = CSMD_FUNCTION_FINISHED; 
    }
    break;
    
    
  case CSMD_FUNCTION_FINISHED:
    
    /* --------------------------------------------------------- */
    /* Command processing is finished.                           */
    /* --------------------------------------------------------- */
    prFuncState->ulSleepTime = 0U;
    
    return (CSMD_NO_ERROR);
    
    
  default:
    return (CSMD_ILLEGAL_CASE);
    
  }   /* End: switch (prFuncState->usActState) */
  
  return (CSMD_FUNCTION_IN_PROCESS);           /* Function processing still active */
  
} /* end: CSMD_Proceed_Cmd_S_0_1048() */
#endif /* #ifndef CSMD_DISABLE_CMD_S_0_1048 */



/**************************************************************************/ /**
\brief Reads parameters from slaves during CSMD_TransmitTiming().

\ingroup module_phase
\b Description: \n
   This function reads the parameter S-0-0127 from all slaves which support
   the classes SCP_NRT or SCP_NRTPC during CSMD_TransmitTiming().

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_WARNING_MTU_MISMATCH \n
              \ref CSMD_SVCH_INUSE \n
              \ref CSMD_SERCOS_VERSION_MISMATCH \n
              \ref CSMD_BASIC_SCP_TYPE_MISMATCH \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_WRONG_ELEMENT_NBR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_NO_ERROR \n

\author       AlM
\date         07.12.2010

***************************************************************************** */
CSMD_FUNC_RET CSMD_ReadConfig( CSMD_INSTANCE          *prCSMD_Instance,
                               CSMD_FUNC_STATE        *prFuncState,
                               CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{

  CSMD_USHORT    usI;
  CSMD_USHORT    usFinished;
  CSMD_ULONG    *paulSCP_ConfigMask;
  CSMD_FUNC_RET  eFuncRet;


  /* Points to configuration bitlist of first slave */
  paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];


  switch (prFuncState->usActState)
  {

  case CSMD_FUNCTION_1ST_ENTRY:

    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prCSMD_Instance->rExtendedDiag.ulIDN       = CSMD_IDN_S_0_( 1027, 0, 2 );
    prFuncState->ulSleepTime = 0U;

    /* UC channel configured? */
    if (prCSMD_Instance->rPriv.ulUCC_Width)
    {
      prFuncState->usActState = CSMD_FUNCTION_STEP_1;
    }
    else
    {
      prFuncState->usActState = CSMD_FUNCTION_STEP_2;
      break;
    }
    /*lint -fallthrough */


  case CSMD_FUNCTION_STEP_1:

    usFinished = 1U;

    /* For all slaves with the classes SCP_NRT or SCP_NRTPC: */
    /* Read parameter S-0-1027.0.2 "Effective MTU" */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & (CSMD_SCP_NRT | CSMD_SCP_NRTPC))
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1027, 0, 2);
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 2U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }
              eFuncRet = CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );

              if ( !(   (eFuncRet == CSMD_NO_ERROR)
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }   /* End: if (CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY) */

            if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR) )
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              usFinished = 0U;
            }
          }     /* End: if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)...*/
        }       /* End: if ( paulSCP_ConfigMask[usI] & ( CSMD_SCP_NRT | CSMD_SCP_NRTPC) ) */
      }         /* End: if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == ... */
    }           /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */

    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      else
      {
        eFuncRet = CSMD_FUNCTION_IN_PROCESS;
        for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
        {
          if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
              (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
          {
            if ( paulSCP_ConfigMask[usI] & (CSMD_SCP_NRT | CSMD_SCP_NRTPC) )
            {
              if (   prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] /* Effective MTU */
                  != prCSMD_Instance->rPriv.usRequested_MTU)
              {
                if (prCSMD_Instance->rPriv.usRequested_MTU == CSMD_ETHERNET_MTU_MAX)
                {
                  eFuncRet = CSMD_WARNING_MTU_MISMATCH;
                }
                else
                {
                  eFuncRet = CSMD_INVALID_MTU;
                  CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                }
              }
            }
          }
        }
        if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
        {
          return (eFuncRet);
        }
      }

      /* continue with next step */
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */


  case CSMD_FUNCTION_STEP_2:

    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);


  default:

    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usCancelActTrans = 1U;
        (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
      }
    }
    return (CSMD_ILLEGAL_CASE);

  }

  return (CSMD_FUNCTION_IN_PROCESS);

} /* End: CSMD_ReadConfig() */


/*! \endcond */ /* PRIVATE */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
20 Nov 2013 WK
  - Defdb00165158
    CSMD_ReadConfig()
    Fixed compiler warning for not defined CSMD_NRT_CHANNEL.
29 Nov 2013 WK
  - Defdb00165278
    CSMD_Transmit_SCP_Sync()
    Fixed transmission of S-0-1036 and S-0-1050.x.11 for Big Endian systems.
  - Defdb00150926
    CSMD_TransmitTiming()
    Fixed Lint Warning: 616: control flows into case/default
    in case of defined CSMD_DISABLE_CMD_S_0_1048.
11 Feb 2015 WK
  - Defdb00176768
    CSMD_Transmit_SCP_Sync(): Adjustments regarding soft master.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
17 Nov 2015 WK
  - Defdb00183182
    CSMD_Proceed_Cmd_S_0_1048()
    Ignore SVC error 7010 "Command is already active" caused by slaves
    in a multi-slave-device.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
  
------------------------------------------------------------------------------
*/
