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
\file   CSMD_GET_TIME.c
\author WK
\date   01.09.2010
\brief  This File contains the public API functions and private functions
        which handles the telgram timing.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#include "CSMD_CONFIG.h"
#include "CSMD_DIAG.h"
#include "CSMD_PRIV_SVC.h"

#define SOURCE_CSMD
#include "CSMD_GET_TIME.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/**************************************************************************/ /**
\brief Reads all parameters necessary for timing and telegram calculation
       from all slaves.

\ingroup func_timing
\b Description: \n
   The following actions are performed for every slave and every connection:

- <B>Evaluation of the parameter S-0-1000 'List of SCP classes & version'</B><BR>
     stored in the variable ausSCPClasses[] of the configuration structure <BR>
     CSMD_SLAVE_CONFIGURATION determines the further processes.

- <B>Evaluation of the configuration type in \ref S_0_1050_x_01 "usS_0_1050_SE1"</B>

- <B>Determination of the maximum number of possible connections of the slave by:</B>
  - reading the list length of S-0-1051 'Image of connection setups' for slaves with the basic class SCP_VarCFG
  - setting to 2 connections for slaves with the basic class SCP_FixCFG

- <B>Write to slaves with the class SCP_Cap</B>
  - S-0-1050.x.7 Assigned connection capability

- <B>Write to slaves with the classes SCP_VarCFG, SCP_Sync, SCP_WD or SCP_WDCon:</B>
  - S-0-1050.x.1 Connection setup

- <B>Write to slaves with the baisc class SCP_VarCFG:</B>
  - S-0-1050.x.6 Configuration List<BR>
    for slaves with 'type of configuration' = 00 (Bit 5-4 in S-0-1050.x.1)
  - S-0-0015 Telegram type<BR>
    for slaves with 'type of configuration' = 10 (Bit 5-4 in S-0-1050.x.1)

- <B>Write the CP2/CP3 slave configuration parameters for all slaves</B><BR>
     (optionally specified by application)

- <B>Read from slaves with the basic classes SCP_FixCFG or SCP_VarCFG:</B>
  - S-0-1050.x.5 Current length of connection

- <B>Read from slaves with the class combination SCP_Sync and SCP_Cap:</B>
  - S-0-1060.x.7 Maximum processing time

- <B>Read from slaves with the classes SCP_Sync or SCP_Cyc:</B>
  - S-0-1005 Maximum Producer processing Time (t5)
  - S-0-1006 AT0 transmission starting time (t1) (read minimum input value)

- <B>Read from slaves with the basic classes SCP_VarCfg_0x02 or SCP_FixCfg_0x02:</B>
  - S-0-1037 Slave Jitter

- <B>Read from slaves with the classes SCP_Sync_0x02 or SCP_Cyc:</B>
  - S-0-1047 Maximum Consumer Processing Time (t11)


<B>Call Environment:</B> \n
   This function can only be called inCP2. If called in another
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

\return       
              \ref CSMD_WRONG_PHASE \n
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
              \ref CSMD_TOO_MANY_CONNECTIONS \n
              \ref CSMD_CONNECTION_LENGTH_0 \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         11.05.2007

***************************************************************************** */
CSMD_FUNC_RET CSMD_GetTimingData( CSMD_INSTANCE          *prCSMD_Instance,
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
    
    eFuncRet = CSMD_WriteSetup( prCSMD_Instance,
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
      return (eFuncRet);
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_2:
    
    eFuncRet = CSMD_ReadSetup( prCSMD_Instance,
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
      return (eFuncRet);
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_3:
    
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);
    
    
  default:
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usCancelActTrans = 1U;
        (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
      }
    }
    return (CSMD_ILLEGAL_CASE);
    
  }
  /* return (CSMD_FUNCTION_IN_PROCESS); */
  
}   /* End: CSMD_GetTimingData() */

/*! \endcond */ /* PUBLIC */


/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Writes parameters to slaves during CSMD_GetTimingData().

\ingroup func_config
\b Description: \n
   This function reads from all slaves the maximum length of the parameter
   - S-0-1051 Image of Connection Setups
   and puts the value into usMaxNbrOfConnections. It is used to determine the 
   number of connections each slave supports

   This function writes the following parameters to all slaves which support
   the corresponding SCP class:
   - S-0-1050.x.7 Connection Capability
   - S-0-1050.x.1 Connection Setup
   - S-0-1050.x.6 Configuration List
   - S-0-0015     Preferred Telegram

   The function sends also additional configuration parameters to the slaves,
   where configured.

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
              \ref CSMD_TOO_MANY_CONNECTIONS \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         16.06.2008

***************************************************************************** */
CSMD_FUNC_RET CSMD_WriteSetup( CSMD_INSTANCE          *prCSMD_Instance,
                               CSMD_FUNC_STATE        *prFuncState,
                               CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_ULONG                    *paulSCP_ConfigMask;
  CSMD_SLAVE_CONFIGURATION      *parSlaveConfig;
  CSMD_CONFIGURATION            *parConfigs;
  CSMD_CONN_IDX_STRUCT          *parIdxList;     /* Pointer to Connection index list" of selected slave */
#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
  CSMD_SLAVE_PARAMCONFIG        *parSlaveParaCfg;
  CSMD_CONFIGPARAMS_LIST        *parConfParamsList;
  CSMD_CONFIGURATION_PARAMETER  *parConfParam;
#endif
  CSMD_USHORT  usI;              /* Slave index      */
  CSMD_USHORT  usFinished;
  CSMD_USHORT  usCIL_Idx;        /* Index in "Connection index list" in master- or slave- configuration */
  CSMD_USHORT  usConfigIdx;      /* Index in "Configurations list" */
  CSMD_USHORT  usConnIdx;        /* Index in "Connections list"    */
  
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  
  
  /* Points to slave configuration of first slave */
  parSlaveConfig    = &prCSMD_Instance->rConfiguration.parSlaveConfig[0];
  parConfigs        = &prCSMD_Instance->rConfiguration.parConfiguration[0];
#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
  parSlaveParaCfg   = &prCSMD_Instance->rConfiguration.parSlaveParamConfig[0];
  parConfParamsList = &prCSMD_Instance->rConfiguration.parConfigParamsList[0];
  parConfParam      = &prCSMD_Instance->rConfiguration.parConfigParam[0];
#endif
  /* Points to configuration bitlist of first slave */
  paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];
  
  switch (prFuncState->usActState)
  {
  case CSMD_FUNCTION_1ST_ENTRY:
    
    prFuncState->ulSleepTime = 0UL;
    /* Evaluate SCP type list and build bitlist */
    if (CSMD_NO_ERROR != (eFuncRet = CSMD_Build_SCP_BitList( prCSMD_Instance )))
      return (eFuncRet);
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
      
#ifdef CSMD_BIG_ENDIAN
      /* Initialize the auxiliary parameter data structure */
      prCSMD_Instance->rPriv.aucAuxConfParam[usI][0] = 0;
#endif
    }
    
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0U;
    prCSMD_Instance->rExtendedDiag.ulIDN       = CSMD_IDN_S_0_( 1050, 0, 1 );
    prFuncState->usActState = CSMD_FUNCTION_STEP_1;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_1:
    
    usFinished = 1U;
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (  parSlaveConfig[usI].usNbrOfConnections
            > CSMD_MAX_CONNECTIONS)
        {
          return (CSMD_TOO_MANY_CONNECTIONS);
        }
        
        parIdxList = &prCSMD_Instance->rConfiguration.parSlaveConfig[usI].arConnIdxList[0];
        
        prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
        
        for (usCIL_Idx = 0; usCIL_Idx < CSMD_MAX_CONNECTIONS; usCIL_Idx++)
        {
          if (parIdxList[usCIL_Idx].usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
          {
            if (paulSCP_ConfigMask[usI] & CSMD_SCP_VARCFG)
            {
              usConfigIdx = parIdxList[usCIL_Idx].usConfigIdx;
              if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
              {
                if (parConfigs[ usConfigIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE)
                {
                  if (   (parConfigs[ usConfigIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_TYPE)
                      == CSMD_S_0_1050_SE1_RESERVED_TYPE)
                  {
                    usFinished = 0U;
                    eFuncRet = CSMD_INVALID_CONNECTION;
                    prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_( 1050, usCIL_Idx, 1 );
                    prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = usI;
                    prCSMD_Instance->rExtendedDiag.aeSlaveError[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = eFuncRet;
                    prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
                    if (prCSMD_Instance->rExtendedDiag.usNbrSlaves >= prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
                      return (CSMD_SYSTEM_ERROR);
                  }
                }
                else
                {
                  /* Connection is not active */
                }
              }
              else
              {
                /* No Configuration */
              }
            }
            else if (paulSCP_ConfigMask[usI] & CSMD_SCP_FIXCFG)
            {
              if (usCIL_Idx >= CSMD_NBR_CONNECTIONS_FIX_CFG)
              {
                usFinished = 0U;
                eFuncRet = CSMD_WRONG_NBR_OF_CONNECTIONS;
                prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_( 1050, usCIL_Idx, 1 );
                prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = usI;
                prCSMD_Instance->rExtendedDiag.aeSlaveError[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = eFuncRet;
                prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
                if (prCSMD_Instance->rExtendedDiag.usNbrSlaves >= prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
                  return (CSMD_SYSTEM_ERROR);
              }
              
            } /* end: if (paulSCP_ConfigMask[usI] & CSMD_SCP_VARCFG) */
            
          } /* end: if (parIdxList[usCIL_Idx].usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn) */
          else
          {
            /* No Connection */
          }   /* end: if (parIdxList[usCIL_Idx].usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn) */
          
        } /* End: for (usCIL_Idx = 0; usCIL_Idx < CSMD_MAX_CONNECTIONS; usCIL_Idx++) */
        
        if (usFinished == 0)
        {
          break;   /* continue with next slave */
        }
      } /* End: if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) ) */
      
    } /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
    {
      eFuncRet = prCSMD_Instance->rExtendedDiag.aeSlaveError[0];
      return (eFuncRet);
    }
    
    prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1051;
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_2:
    
    usFinished = 1U;
    
    /* Get listlength of parameter S-0-1051 "Image of connection setups" to determine the maximum number of possible connections */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_VARCFG)
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
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1051;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_RD_LIST_LENGTH;
                parSvcMacro[usI].usLength               = 0U;
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
                if (   (eFuncRet == CSMD_SVC_ERROR_MESSAGE) 
                    && (parSvcMacro[usI].usSvchError == (CSMD_USHORT) CSMD_SVC_ID_NOT_THERE))
                {
                  eFuncRet = CSMD_SERCOS_VERSION_MISMATCH;
                }
                
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
        } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_VARCFG) */
      }
    } /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      
      /* check current list length */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        CSMD_USHORT  usK;
        
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          if (paulSCP_ConfigMask[usI] & CSMD_SCP_VARCFG)
          {
            /* check current length */
            if (  prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0]    /* current list length received      */
                > (CSMD_IDN_MAX_NBR_SI * 2))                            /* maximum nbr. of structure indexes */
            {
              eFuncRet = CSMD_SYSTEM_ERROR;
              
              CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
            }
            else
            {
              parSlaveConfig[usI].usMaxNbrOfConnections =
                (CSMD_USHORT) (prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] / 2);
              
              if (  parSlaveConfig[usI].usNbrOfConnections
                  > parSlaveConfig[usI].usMaxNbrOfConnections)
              {
                /* More connections configured than supported by the slave */
                eFuncRet = CSMD_TOO_MANY_CONNECTIONS;
                
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
            
            /* Check if connections are configured in higher instance than the slaves supports */
            for (usK = parSlaveConfig[usI].usMaxNbrOfConnections; usK < CSMD_MAX_CONNECTIONS; usK++)
            {
              if (parSlaveConfig[usI].arConnIdxList[usK].usConfigIdx != 0xFFFF)
              {
                /* More connections configured than supported by the slave */
                eFuncRet = CSMD_TOO_MANY_CONNECTIONS;
                
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
            }
          }
          else
          {
            parSlaveConfig[usI].usMaxNbrOfConnections = 
              CSMD_NBR_CONNECTIONS_FIX_CFG;
          } /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_VARCFG) */
        }
      }

      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          /* Initialize for next function step */
          prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
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
    
    usFinished = 1U;
    
    /* SCP_Cap: Send S-0-1050.x.7 "Assigned connection capability" for all slaves and all connections x */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & CSMD_SCP_CAP)
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
                usConfigIdx = parSlaveConfig[usI].arConnIdxList[ prCSMD_Instance->rPriv.ausActConnection[usI] ].usConfigIdx;
                if (   (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
                    && (parConfigs[usConfigIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE) )
                {
                  /* check if MBUSY is set, if it is not set do nothing */
                  if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
                  {
                    if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
                    {
                      prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] = parConfigs[usConfigIdx].usS_0_1050_SE7;
                      parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                      parSvcMacro[usI].usSlaveIdx             = usI;
                      parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                      parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1050, prCSMD_Instance->rPriv.ausActConnection[usI], 7 );
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
              else
              {
                /* select next connection */
                prCSMD_Instance->rPriv.ausActConnection[usI]++;
              }
            }
          }
        }
      }
    }   /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      /* continue with next step */
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
    
    /* SCP_VarCFG: Send S-0-1050.x.1 "Connection setup" for all slaves and all connections x */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#ifdef CSMD_FAST_STARTUP
        if (   (paulSCP_ConfigMask[usI] & (CSMD_SCP_VARCFG | CSMD_SCP_SYNC | CSMD_SCP_WD | CSMD_SCP_WDCON))
            && !(parSlaveConfig[usI].usSettings & CSMD_SLAVECONFIG_UNCHANGED))
#else
        if (paulSCP_ConfigMask[usI] & (CSMD_SCP_VARCFG | CSMD_SCP_SYNC | CSMD_SCP_WD | CSMD_SCP_WDCON))
#endif
        {
          if (parSvcMacro[usI].usState != CSMD_REQUEST_ERROR)
          {
            if (prCSMD_Instance->rPriv.ausActConnection[usI] < parSlaveConfig[usI].usMaxNbrOfConnections)
            {
              /* still connections for this slave to check? */
              if (prCSMD_Instance->rPriv.ausActConnection[usI] < CSMD_MAX_CONNECTIONS)
              {
                usConnIdx = parSlaveConfig[usI].arConnIdxList[prCSMD_Instance->rPriv.ausActConnection[usI]].usConnIdx;
                if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
                {
                  usConfigIdx = parSlaveConfig[usI].arConnIdxList[prCSMD_Instance->rPriv.ausActConnection[usI]].usConfigIdx;
                  if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
                  {
                    parSvcMacro[usI].pusAct_Data = &parConfigs[usConfigIdx].usS_0_1050_SE1;
                  }
                  else
                  {
                    /* No Configuration: Disable connection */
                    prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0] = 0U;
                    parSvcMacro[usI].pusAct_Data = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                  }
                }
                else
                {
                  /* No Connection: Disable connection */
                  prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0] = 0U;
                  parSvcMacro[usI].pusAct_Data = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                }
              }
              else
              {
                /* Connection is not accessible by the master, then disable */
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0] = 0U;
                parSvcMacro[usI].pusAct_Data = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
              }
              
              /* check if MBUSY is set, if it is not set do nothing */
              if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
              {
                if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
                {
                  parSvcMacro[usI].usSlaveIdx             = usI;
                  parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                  parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1050, prCSMD_Instance->rPriv.ausActConnection[usI], 1 );
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
                      && (parSvcMacro[usI].usSvchError == (CSMD_USHORT) CSMD_SVC_ID_NOT_THERE))
                  {
                    /* select next connection */
                    prCSMD_Instance->rPriv.ausActConnection[usI]++;
                    
                    /* start request for next connection */
                    parSvcMacro[usI].usState = CSMD_START_REQUEST;
                    
                    eFuncRet = CSMD_NO_ERROR;
                  }
                  else
                  {
                    prCSMD_Instance->rExtendedDiag.ulIDN = parSvcMacro[usI].ulIdent_Nbr;
                    CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                  }
                }
                
                if (parSvcMacro[usI].usState == CSMD_DATA_VALID) /* if ready write next configuration */
                {
                  /* select next connection */
                  prCSMD_Instance->rPriv.ausActConnection[usI]++;
                  
                  /* start request for next connection */
                  parSvcMacro[usI].usState = CSMD_START_REQUEST;
                }
              }
              
              if (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)
              {
                 CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
              }
              else
              {
                if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                    && (prCSMD_Instance->rPriv.ausActConnection[usI] >= parSlaveConfig[usI].usMaxNbrOfConnections) )
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
    }   /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      /* continue with next step */
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
    
    /* SCP_VarCFG: Send S-0-1050.x.6 "Configurationlist" / S-0-0015 "Telegram type" for all slaves and all connections x */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
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
                usConfigIdx = parSlaveConfig[usI].arConnIdxList[ prCSMD_Instance->rPriv.ausActConnection[usI] ].usConfigIdx;
                if (   (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
                    && (parConfigs[usConfigIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE) )
                {
                  if (CSMD_S_0_1050_SE1_CONTAINER != (parConfigs[usConfigIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_TYPE))
                  {
                    /* check if MBUSY is set, if it is not set do nothing */
                    if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
                    {
                      if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
                      {
                        if (  (parConfigs[usConfigIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_TYPE)
                            == CSMD_S_0_1050_SE1_IDN_LIST)
                        {
                          /* Configuration type: IDN list (SE6 relevant) */
                          parSvcMacro[usI].pusAct_Data      = (CSMD_USHORT *)(CSMD_VOID *)parConfigs[usConfigIdx].ulS_0_1050_SE6;
                          parSvcMacro[usI].usIsList         = CSMD_ELEMENT_IS_LIST;
                          /* current length of the list plus 4 for length info */
                          parSvcMacro[usI].usLength         = (CSMD_USHORT) (parConfigs[usConfigIdx].ulS_0_1050_SE6[0] + (CSMD_USHORT)4);
                          parSvcMacro[usI].ulIdent_Nbr      = CSMD_IDN_S_0_( 1050, prCSMD_Instance->rPriv.ausActConnection[usI], 6 ); /*  */
                        }
                        else if (  (parConfigs[usConfigIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_TYPE)
                                 == CSMD_S_0_1050_SE1_FSP_DRIVE)
                        {
                          /* Configuration type: Tel.type par. (S-0-0015 relevant) */
                          parSvcMacro[usI].pusAct_Data      = &parConfigs[usConfigIdx].usTelgramTypeS00015;
                          parSvcMacro[usI].usIsList         = CSMD_ELEMENT_NO_LIST;
                          parSvcMacro[usI].usLength         = 2U;
                          parSvcMacro[usI].ulIdent_Nbr      = CSMD_IDN_S_0_0015; /*  */
                        }

                        parSvcMacro[usI].usSlaveIdx             = usI;
                        parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
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
                }
                else
                {
                  /* select next connection */
                  prCSMD_Instance->rPriv.ausActConnection[usI]++;
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
            }   /* End: if (prCSMD_Instance->rPriv.ausActConnection[usI] < parSlaveConfig[usI].usMaxNbrOfConnections) */
          }   /* End: if (parSvcMacro[usI].usState != CSMD_REQUEST_ERROR) */
        }   /* End: if ( ( paulSCP_ConfigMask[usI] & (CSMD_SCP_VARCFG | CSMD_SCP_VARCFG_V2) )...) */
      }   /* End: if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) ) */
    }   /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          /* Initialize for next function step */
#ifdef CSMD_CONFIG_PARSER
          prCSMD_Instance->rPriv.ausActConnection[usI] = parSlaveConfig[usI].usFirstConfigParamIndex;
#endif
          prCSMD_Instance->rPriv.ausActParam[usI] = 0;
          
          /* start request for next step */
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      prFuncState->usActState  = CSMD_FUNCTION_STEP_6;
#else
      prFuncState->usActState  = CSMD_FUNCTION_STEP_7;
#endif
      /* continue with next step */
      prFuncState->ulSleepTime = 0U;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_6:
#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
    usFinished = 1U;
    
    /* Send the slave configuration parameters for all slaves */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
#ifdef CSMD_FAST_STARTUP
      if (   (   prCSMD_Instance->rSlaveList.aeSlaveActive[usI]
              == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2))
          && !(parSlaveConfig[usI].usSettings & CSMD_SLAVE_SETUP_UNCHANGED))
#else
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
#endif
      {
        if (parSvcMacro[usI].usState != CSMD_REQUEST_ERROR)
        {
          /* Misuse of ausActConnection, to save memory */
          if (prCSMD_Instance->rPriv.ausActConnection[usI] < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams)
          {
            /* Get the setup parameter list index */
            CSMD_USHORT  usParamListIdx = parSlaveParaCfg[prCSMD_Instance->rPriv.ausActConnection[usI]].usConfigParamsList_Index;
            
            /* Does a configuration entry exist? */
            if (usParamListIdx >= prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList)
            {
              /* No entry -> set to next config param list */
              prCSMD_Instance->rPriv.ausActConnection[usI] = parSlaveParaCfg[prCSMD_Instance->rPriv.ausActConnection[usI]].usNextIndex;
              
              continue;
            }
            
            /* Write the parameter */
            
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                CSMD_USHORT  usParamIdx = parConfParamsList[usParamListIdx].ausParamTableIndex[prCSMD_Instance->rPriv.ausActParam[usI]];
                
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = prCSMD_Instance->rConfiguration.parConfigParam[usParamIdx].ulIDN;
#ifndef CSMD_BIG_ENDIAN
                parSvcMacro[usI].pusAct_Data            = (CSMD_USHORT *)(CSMD_VOID *)(&parConfParam[usParamIdx].aucParamData[0]);
#else
                parSvcMacro[usI].pusAct_Data            = (CSMD_USHORT *)(CSMD_VOID *)(&prCSMD_Instance->rPriv.aucAuxConfParam[usI][0]);
#endif
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_UNKNOWN_LENGTH;
                parSvcMacro[usI].usLength               = 0U;
                parSvcMacro[usI].ulAttribute            = 0U;
                parSvcMacro[usI].usCancelActTrans       = 0U;
                parSvcMacro[usI].usPriority             = 1U;
                parSvcMacro[usI].usOtherRequestCanceled = 0U;
                parSvcMacro[usI].usSvchError            = 0U;
                parSvcMacro[usI].usInternalReq          = 1U;
              }
              eFuncRet = CSMD_WriteSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
              
              if (!(   (eFuncRet == CSMD_NO_ERROR) 
                    || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)))
              {
                /* Set the current IDN */
                prCSMD_Instance->rExtendedDiag.ulIDN  = parSvcMacro[usI].ulIdent_Nbr;
                CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
              }
              
#ifdef CSMD_BIG_ENDIAN
              if (parSvcMacro[usI].usState == CSMD_ATTRIBUTE_VALID) /* if valid, evaluate and turn the data */
              {
                CSMD_USHORT  usParamIdx = parConfParamsList[usParamListIdx].ausParamTableIndex[prCSMD_Instance->rPriv.ausActParam[usI]];
                
                CSMD_UCHAR  *pucSourcePtr = &parConfParam[usParamIdx].aucParamData[0];
                CSMD_UCHAR  *pucDestPtr = (CSMD_UCHAR *)(CSMD_VOID *)&prCSMD_Instance->rPriv.aucAuxConfParam[usI][0];
                CSMD_USHORT  usK = 0;
                
                /* Is it variable length? */
                if (parSvcMacro[usI].ulAttribute & CSMD_SERC_VAR_LEN)
                {
                  /* Turn the first two shorts containing the length information */
                  *((CSMD_USHORT *)(CSMD_VOID *)(pucDestPtr)) = CSMD_END_CONV_S(*((CSMD_USHORT *)(CSMD_VOID *)(pucSourcePtr)));
                  *((CSMD_USHORT *)(CSMD_VOID *)(pucDestPtr + 2)) = CSMD_END_CONV_S(*((CSMD_USHORT *)(CSMD_VOID *)(pucSourcePtr + 2)));
                  
                  /* Adjust the data pointer to the position behind length information */
                  usK = 4;
                  
                  /* Special handling for 1 byte; only for variable length */
                  if ((parSvcMacro[usI].ulAttribute & CSMD_SERC_LEN) == CSMD_SERC_VAR_BYTE_LEN)
                  {
                    /* no turning necessary for 1 byte data */
                    for (; usK < parConfParam[usParamIdx].usDataLength; usK++)
                    {
                      *(pucDestPtr + usK) = *(pucSourcePtr + usK);
                    }
                  }
                }
                
                if ((parSvcMacro[usI].ulAttribute & CSMD_SERC_LEN_WO_LISTINFO) == CSMD_SERC_WORD_LEN)
                {
                  /* swap 2 byte data */
                  for (; usK < parConfParam[usParamIdx].usDataLength; usK += 2)
                  {
                    *((CSMD_USHORT *)(CSMD_UCHAR *)(pucDestPtr + usK)) =
                      CSMD_END_CONV_S(*((CSMD_USHORT *)(CSMD_UCHAR *)(pucSourcePtr + usK)));
                  }
                }
                else if ((parSvcMacro[usI].ulAttribute & CSMD_SERC_LEN_WO_LISTINFO) == CSMD_SERC_LONG_LEN)
                {
                  /* swap 4 byte data */
                  for (; usK < parConfParam[usParamIdx].usDataLength; usK += 4)
                  {
                    *((CSMD_ULONG *)(CSMD_UCHAR *)(pucDestPtr + usK)) =
                      CSMD_END_CONV_L(*((CSMD_ULONG *)(CSMD_UCHAR *)(pucSourcePtr + usK)));
                  }
                }
                else if ((parSvcMacro[usI].ulAttribute & CSMD_SERC_LEN_WO_LISTINFO) == CSMD_SERC_DOUBLE_LEN)
                {
                  /* swap 8 byte data */
                  for (; usK < parConfParam[usParamIdx].usDataLength; usK += 8)
                  {
                    *((CSMD_ULONG *)(CSMD_UCHAR *)(pucDestPtr + usK)) =
                      CSMD_END_CONV_L(*((CSMD_ULONG *)(CSMD_UCHAR *)(pucSourcePtr + usK + 4)));
                    *((CSMD_ULONG *)(CSMD_UCHAR *)(pucDestPtr + usK + 4)) =
                      CSMD_END_CONV_L(*((CSMD_ULONG *)(CSMD_UCHAR *)(pucSourcePtr + usK)));
                  }
                }
              }
#endif
              
              if (parSvcMacro[usI].usState == CSMD_DATA_VALID) /* if ready write next configuration */
              {
                /* select next parameter */
                prCSMD_Instance->rPriv.ausActParam[usI]++;
                
                /* Is this parameter list finished? */
                if (parConfParamsList[usParamListIdx].ausParamTableIndex[prCSMD_Instance->rPriv.ausActParam[usI]] == 0xFFFF)
                {
                  /* select next parameter list */
                  prCSMD_Instance->rPriv.ausActConnection[usI] = 
                    parSlaveParaCfg[prCSMD_Instance->rPriv.ausActConnection[usI]].usNextIndex;
                  
                  prCSMD_Instance->rPriv.ausActParam[usI] = 0;
                }
                
                /* start request for next connection */
                parSvcMacro[usI].usState = CSMD_START_REQUEST;
              }
            }
            
            if (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)
            {
              CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
            }
            else
            {
              if (   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                  && (prCSMD_Instance->rPriv.ausActConnection[usI] >= parSlaveConfig[usI].usMaxNbrOfConnections) )
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
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      
      /* continue with next step */
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_7;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
#endif  /* #if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS */

    
  case CSMD_FUNCTION_STEP_7:
    
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);

    
  default:
    
    /* in case of an error, cancel all SVC-transmission and return */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usCancelActTrans = 1U;
        (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
      }
    }
    return (CSMD_ILLEGAL_CASE);
    
  }
  
  return (CSMD_FUNCTION_IN_PROCESS);
  
} /* End: CSMD_WriteSetup() */



/**************************************************************************/ /**
\brief Reads the parameters necessary for timing calculation from the slaves.

\ingroup func_config
\b Description: \n
   This function reads the following parameters from all slaves which support
   the corresponding SCP class:
   - S-0-1050.x.5 Connection Length
   Either
   - S-0-1060.x.7 Maximum Processing Time
   or
   - S-0-1005 Maximum Producer processing Time (t5)
   - S-0-1006 AT0 transmission starting time (t1) (minimum value)
   - S-0-1037 Slave Jitter
   - S-0-1047 Maximum Consumer Processing Time (t11)

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
              \ref CSMD_CONNECTION_LENGTH_0 \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         16.06.2008

***************************************************************************** */
CSMD_FUNC_RET CSMD_ReadSetup( CSMD_INSTANCE          *prCSMD_Instance,
                              CSMD_FUNC_STATE        *prFuncState,
                              CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_ULONG                *paulSCP_ConfigMask;
  CSMD_SLAVE_CONFIGURATION  *parSlaveConfig;
  CSMD_CONNECTION           *parConnect;
  CSMD_CONFIGURATION        *parConfig;
  
  CSMD_USHORT  usI;              /* Slave index      */
  CSMD_USHORT  usFinished;
  CSMD_USHORT  usConnIdx;        /* Index in "Connections list"    */
  CSMD_USHORT  usConfIdx;        /* Index in "Configurations list"    */
  
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  
  
  /* Points to slave configuration of first slave */
  parSlaveConfig = &prCSMD_Instance->rConfiguration.parSlaveConfig[0];
  parConnect     = &prCSMD_Instance->rConfiguration.parConnection[0];
  parConfig      = &prCSMD_Instance->rConfiguration.parConfiguration[0];
  
  /* Points to configuration bitlist of first slave */
  paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];
  
  switch (prFuncState->usActState)
  {
  case CSMD_FUNCTION_1ST_ENTRY:
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_1:
    
    usFinished = 1U;
    
    /* SCP_Basic: Read Parameter S-0-1050.x.5  "Current length of connection" for all slaves and all connections x */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
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
                  parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                  parSvcMacro[usI].usSlaveIdx             = usI;
                  parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1050, prCSMD_Instance->rPriv.ausActConnection[usI], 5 );
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
                
                eFuncRet = CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
                
                if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                       || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
                {
                  prCSMD_Instance->rExtendedDiag.ulIDN = parSvcMacro[usI].ulIdent_Nbr;
                  CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                }
                
                if (parSvcMacro[usI].usState == CSMD_DATA_VALID) /* if ready read next connection length */
                {
                  if (parConnect[usConnIdx].usS_0_1050_SE5 == 0)
                  {
                    /* Connection length is not configured */
                    if (prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] == 0)
                    {
                      /* The read connection length is equal 0 */
                      eFuncRet = CSMD_CONNECTION_LENGTH_0;

                      prCSMD_Instance->rExtendedDiag.ulIDN = parSvcMacro[usI].ulIdent_Nbr;
                      CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );

                      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves >= prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
                      {
                        CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );

                        return (CSMD_SYSTEM_ERROR);
                      }
                    }
                    else
                    {
                      /* Take the read length of the connection into the configuration structure */
                      parConnect[usConnIdx].usS_0_1050_SE5 = (CSMD_USHORT)CSMD_RND_UP_2( prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0] );
                    }
                  }
                  else if (   parConnect[usConnIdx].usS_0_1050_SE5
                           != prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0])
                  {
                    /* The configured connection length is not equal to the read length */
                    eFuncRet = CSMD_CONNECTION_DATALENGTH_INCONSISTENT;
                    
                    prCSMD_Instance->rExtendedDiag.ulIDN = parSvcMacro[usI].ulIdent_Nbr;
                    CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                    
                    if (prCSMD_Instance->rExtendedDiag.usNbrSlaves >= prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
                    {
                      CSMD_ClearInternal_SVCInterrupt( prCSMD_Instance, usI );
                      
                      return (CSMD_SYSTEM_ERROR);
                    }
                  }
                  
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
    } /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          prCSMD_Instance->rPriv.ausActConnection[usI] = 0;
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
          
          /* Set the processing time to 0 */
          parSlaveConfig[usI].rTiming.ulMinFdbkProcTime_S01005 = 0;
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
    
    /* For all slaves with the class combination SCP_Sync and SCP_Cap: */
    /* Read Parameter S-0-1060.x.7  "Maximum processing time" for all produced connections x */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if ((paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC | CSMD_SCP_CAP)) == (CSMD_SCP_SYNC | CSMD_SCP_CAP))
        {
          if (parSvcMacro[usI].usState != CSMD_REQUEST_ERROR)
          {
            /* still connections for this slave to check? */
            if (   (prCSMD_Instance->rPriv.ausActConnection[usI] < parSlaveConfig[usI].usMaxNbrOfConnections)
                && (prCSMD_Instance->rPriv.ausActConnection[usI] < CSMD_MAX_CONNECTIONS))
            {
              usConfIdx = parSlaveConfig[usI].arConnIdxList[prCSMD_Instance->rPriv.ausActConnection[usI]].usConfigIdx;
              
              /* Is this connection used ? */
              if (usConfIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
              {
                /* Is the slave the producer */
                if ((parConfig[usConfIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE_PRODUCER) == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
                {
                  /* Is the capability used ? */
                  if (parConfig[usConfIdx].usS_0_1050_SE7 != 0xFFFF)
                  {
                    /* check if MBUSY is set, if it is not set do nothing */
                    if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
                    {
                      if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
                      {
                        parSvcMacro[usI].pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
                        parSvcMacro[usI].usSlaveIdx             = usI;
                        parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_( 1060, parConfig[usConfIdx].usS_0_1050_SE7, 7 );
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
                      
                      eFuncRet = CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
                      
                      if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                             || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
                      {
                        prCSMD_Instance->rExtendedDiag.ulIDN = parSvcMacro[usI].ulIdent_Nbr;
                        CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                      }
                      
                      if (parSvcMacro[usI].usState == CSMD_DATA_VALID) /* if ready read next maximum processing time */
                      {
                        if (parSlaveConfig[usI].rTiming.ulMinFdbkProcTime_S01005 < prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0])
                        {
                          /* Take the current length of the connection into the configuration structure */
                          parSlaveConfig[usI].rTiming.ulMinFdbkProcTime_S01005 = prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0];
                        }
                        
                        /* select next connection */
                        prCSMD_Instance->rPriv.ausActConnection[usI]++;
                        
                        /* start request for next connection */
                        parSvcMacro[usI].usState = CSMD_START_REQUEST;
                      }
                    }
                  }
                  else
                  {
                    /* Capability is not used, get the maximum processing time later from S-0-1005 */
                    
                    /* Set the marker to a finished state */
                    prCSMD_Instance->rPriv.ausActConnection[usI] = parSlaveConfig[usI].usMaxNbrOfConnections;
                    
                    /* Set the time back to 0 */
                    parSlaveConfig[usI].rTiming.ulMinFdbkProcTime_S01005 = 0;
                  }
                }
                else
                {
                  /* select next connection */
                  prCSMD_Instance->rPriv.ausActConnection[usI]++;
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
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      /* continue with next step */
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1005;
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
    
    /* For all slaves with the classes SCP_Sync or SCP_Cyc: */
    /* Read parameter S-0-1005 "Maximum Producer processing Time (t5)"   */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if (( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) ) && (parSlaveConfig[usI].rTiming.ulMinFdbkProcTime_S01005 == 0))
      {
        if (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC | CSMD_SCP_CYC)) 
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
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1005;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 4U;
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
            
            if (parSvcMacro[usI].usState == CSMD_DATA_VALID)
            {
              /* Transfer the current length of the connection into the configuration structure */
              parSlaveConfig[usI].rTiming.ulMinFdbkProcTime_S01005 =
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0];
            }
          }
        }   /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC) */   
        else
        {
          parSlaveConfig[usI].rTiming.ulMinFdbkProcTime_S01005 = 0U;
          continue;
        }
        
        if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
               || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
        {
          usFinished = 0U;
        }
      }
      
    } /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rConfiguration.rComTiming.ulMinTimeStartAT = 0U;
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1006;
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
    
    /* For all slaves with the classes SCP_Sync or SCP_Cyc: */
    /* Read minimum input value of Parameter S-0-1006 "AT0 transmission starting time (t1)" */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC | CSMD_SCP_CYC)) 
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
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE5_MIN_VALUE;
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
              eFuncRet = CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
              
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                if (   (eFuncRet == CSMD_SVC_ERROR_MESSAGE) 
                    && (parSvcMacro[usI].usSvchError == (CSMD_USHORT) CSMD_SVC_NO_MIN))
                {
                  /* No minimum input value for this IDN */
                  /* Set value for later comparison */
                  prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0] = 0;
                  
                  /* Suspend  error message */
                  eFuncRet = CSMD_NO_ERROR;
                }
                else
                {
                  CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                }
              }
            }
          }
          
          if (parSvcMacro[usI].usState == CSMD_DATA_VALID)
          {
            if (  prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0]
                > prCSMD_Instance->rConfiguration.rComTiming.ulMinTimeStartAT)
            {
              prCSMD_Instance->rConfiguration.rComTiming.ulMinTimeStartAT = 
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0];
            }
          }
        }   /* End: if (paulSCP_ConfigMask[usI] & CSMD_SCP_SYNC) */
        else
        {
          continue;
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
    } /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1037;
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

    /* For all slaves with the basic classes SCP_VarCFG_0x02 or SCP_FixCFG_0x02: */
    /* read S-0-1037 Slave Jitter */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & (CSMD_SCP_FIXCFG_V2 | CSMD_SCP_VARCFG_V2))
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
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1037;
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
            }
            
            if (parSvcMacro[usI].usState == CSMD_DATA_VALID)
            {
              prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTiming.usSlaveJitter_S1037
                = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0];
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
        }   /* End: if (  (paulSCP_ConfigMask[usI] &
                          (CSMD_SCP_FIXCFG_V2 | CSMD_SCP_VARCFG_V2)) ) */   
        else
        {
          continue;
        }
      }
    } /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1047;
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

    /* For all slaves with the classes SCP_Sync_0x02 or SCP_Cyc: */
    /* read S-0-1047 Maximum Consumer Processing Time (t11) */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC_V2 | CSMD_SCP_CYC))
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
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1047;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
                parSvcMacro[usI].usLength               = 4U;
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
            }
            
            if (parSvcMacro[usI].usState == CSMD_DATA_VALID)
            {
              prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTiming.ulMaxConsActTimeT11_S01047 = 
                prCSMD_Instance->rPriv.parRdWrBuffer[usI].aulData[0];
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
        }   /* End: if (paulSCP_ConfigMask[usI] & (CSMD_SCP_SYNC_V2 | CSMD_SCP_CYC)) */
        else
        {
          continue;
        }
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
      prFuncState->usActState  = CSMD_FUNCTION_STEP_7;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_7:
    
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);
    
    
  default:
    
    /* in case of an error, cancel all SVC-transmission and return */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usCancelActTrans = 1U;
        (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
      }
    }
    return (CSMD_ILLEGAL_CASE);
    
  }
  
  return (CSMD_FUNCTION_IN_PROCESS);
  
} /* End: CSMD_ReadSetup() */


/*! \endcond */ /* PRIVATE */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
04 Nov 2013 WK
  Defdb00161830
  CSMD_WriteSetup(), CSMD_FUNCTION_STEP_1
  Fixed handling of structure instance of erroneous parameters for 
  extended diagnosis.
04 Mar 2014 WK
  Defdb00155249
  CSMD_WriteSetup()
  Possible deadlock if a configured connection is not an 
  active connection (S-0-1050.x.1).^
26 Aug 2014 WK
  CSMD_ReadSetup()
  Fixed reading of ulMaxConsActTimeT11_S01047.
31 Oct 2014 WK
  CSMD_ReadSetup()
  Rounding up read usS_0_1050_SE5 to a divisible by 2 value.
24 Feb 2015 WK
  - Defdb00177060
    CSMD_ReadSetup()
    Check, if the read connection length is greater than 2 bytes.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
14 Jul 2016 WK
 - FEAT-00051878 - Support for Fast Startup
   CSMD_WriteSetup()
   Added Fast-Startup support for slave setup data from binary configuration.
  
------------------------------------------------------------------------------
*/
