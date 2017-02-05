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
\file   CSMD_HOTPLUG.c
\author WK
\date   01.09.2010
\brief  This File contains the public API functions and private functions
        for the Hot-Plug feature.
*/

/*---- Includes: -------------------------------------------------------------*/

#ifdef CSMD_DEBUG_HP
#include <stdio.h>
#endif

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#include "CSMD_CONFIG.h"
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
#include "CSMD_CALC.h"
#endif
#include "CSMD_CP_AUX.h"
#include "CSMD_GET_TIME.h"
#include "CSMD_CFG_PARA.h"
#include "CSMD_DIAG.h"
#include "CSMD_PHASE.h"
#include "CSMD_PRIV_SVC.h"
#include "CSMD_HOTPLUG.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

#ifdef CSMD_HOTPLUG

/**************************************************************************/ /**
\brief Performs the Hot Plug process for a Sercos device.

\ingroup module_hotplug
\b Description: \n
   This function executes the Hot Plug phase progression process from HP0 to HP2
   including service channel activation. During this function, no CoSeMa-internal
   SVC accesses are carried out. After the function has finished successfully,
   the returned Sercos list contains the Sercos addresses of all slaves of the
   Hot Plug device.

<B>Call Environment:</B> \n
   This function must be called once per Sercos cycle in CP4. It has to be
   called until either it returns an error message or it has finished
   successfully.\n
   A premature cancellation of the function is possible setting the transfer
   parameter boCancel to TRUE. In this case the original topology is
   recovered.\n
   This function comprises an internal state machine.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function 
\param [in]   pusHPSlaveAddList
              Pointer to Sercos list that contains the Sercos addresses
              of all slaves of the Hot-Plug device.
              The Sercos list contains a maximum of 16 slaves.
\param [in]   boCancel
              If set to TRUE, cancel Hot-Plug operation and recover the previous topology.
              
\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_HP_NOT_SUPPORTED \n
              \ref CSMD_HP_NOT_WITH_CLOSED_RING \n
              \ref CSMD_HP_NO_HOTPLUG_SLAVE \n
              \ref CSMD_HP_WRONG_TOPOLOGY \n
              \ref CSMD_HP_TOPOLOGY_CHANGE \n
              \ref CSMD_HP_PHASE_0_TIMEOUT \n
              \ref CSMD_HP_SLAVE_SCAN_TIMEOUT \n
              \ref CSMD_HP_NO_SLAVE_FOUND \n
              \ref CSMD_HP_PHASE_0_FAILED \n
              \ref CSMD_HP_SLAVE_RECOGNIZED_IN_CP0 \n
              \ref CSMD_HP_SLAVE_IS_NOT_PROJECTED \n
              \ref CSMD_HP_PHASE_1_TIMEOUT \n
              \ref CSMD_HP_SWITCH_TO_SVC_FAILED \n
              \ref CSMD_HP_SWITCH_TO_SVC_TIMEOUT \n
              \ref CSMD_HP_OPERATION_ABORTED \n
              \ref CSMD_HP_DOUBLE_SLAVE_ADDRESSES \n
              \ref CSMD_HP_ILLEGAL_SLAVE_ADDRESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n
 
\author       wk
\date         2008-04-25
  

***************************************************************************** */
CSMD_FUNC_RET CSMD_HotPlug( CSMD_INSTANCE   *prCSMD_Instance,
                            CSMD_FUNC_STATE *prFuncState,
                            CSMD_USHORT     *pusHPSlaveAddList,
                            CSMD_BOOL        boCancel )
{

  CSMD_FUNC_RET      eFuncRet  = CSMD_FUNCTION_IN_PROCESS;  /* Function processing still active */
  CSMD_HOT_PLUG_AUX *prHotPlug = &prCSMD_Instance->rPriv.rHotPlug;
  CSMD_USHORT        usSAdd;      /* Sercos address   */
  CSMD_USHORT        usSlaveIdx;  /* Slave index      */
  CSMD_USHORT        usS_Dev;     /* S-DEV            */
  CSMD_USHORT        usI;         /* Loop counter     */
  CSMD_USHORT        usSvcOffset;
  
  
  /* Hot-Plug canceled */
  if (boCancel == TRUE)
  {
    prFuncState->ulSleepTime = 0;
    
    /* Cancel function only if not finished */
    if (prFuncState->usActState < CSMD_FUNCTION_STEP_9)
    {
      /* Disable Hot-Plug */
      if (prFuncState->usActState < CSMD_FUNCTION_STEP_2)
        prFuncState->usActState  = CSMD_FUNCTION_STEP_11;
      else
        prFuncState->usActState  = CSMD_FUNCTION_STEP_10;

      prHotPlug->usHP_Phase = CSMD_NON_HP_MODE;
#ifdef CSMD_DEBUG_HP
      printf ("\nC2C: Hot-Plug operation aborted in Step %d", prFuncState->usActState);
#endif
    }
    prHotPlug->eHP_FuncRet = CSMD_HP_OPERATION_ABORTED;
  }
  
  switch (prFuncState->usActState)
  {
  case CSMD_FUNCTION_1ST_ENTRY: /* CSMD_FUNCSTATE_HP0_ENTRY */
    
    /* Initialize slave address Sercos list */
    prHotPlug->ausHP_SlaveAddList[0] = 0;                              /* current length of Sercos list */
    prHotPlug->ausHP_SlaveAddList[1] = (CSMD_HP_ADD_MAX_SLAVES * 2);   /* maximum length */
    
    *pusHPSlaveAddList     = prHotPlug->ausHP_SlaveAddList[0];
    *(pusHPSlaveAddList+1) = prHotPlug->ausHP_SlaveAddList[1];
    
    prFuncState->ulSleepTime = 0;
    
    if (!CSMD_IsHotplugSupported (prCSMD_Instance))
    {
      eFuncRet = CSMD_HP_NOT_SUPPORTED;
      break;
    }
    
    /* --------------------------------------------------------- */
    /* Check whether current communication phase is CP4.         */
    /* --------------------------------------------------------- */
    if (prCSMD_Instance->sCSMD_Phase != CSMD_SERC_PHASE_4)
    {
      eFuncRet = CSMD_WRONG_PHASE;
      break;
    }
    
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
    {
      eFuncRet = CSMD_HP_NOT_WITH_CLOSED_RING;
      break;
    }
    
    /* Set Hot Plug active flag which prevents overwriting of DFCSR by CSMD_SetCollisionBuffer() during Hot Plug */
    prHotPlug->boHotPlugActive = TRUE;
    
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
    {
      CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_RING_MODE);
    }
    
    /* --------------------------------------------------------- */
    /* Check topology of last slave in line.                     */
    /* --------------------------------------------------------- */
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
    {
      usSAdd   = prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[ prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb - 1];
      usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList [ usSAdd ];
      
      /* status of last slave in line */
      usS_Dev = prCSMD_Instance->arDevStatus[ usSlaveIdx ].usS_Dev;
      usS_Dev &= CSMD_S_DEV_STAT_INACT_MASK;
      
      if (usS_Dev == CSMD_S_DEV_STAT_INACT_LINK)
      {
        /* Link at last slave on P-Channel */
        /* Start Hot Plug mechanism */
        prHotPlug->prHP_ActPort = &prCSMD_Instance->rPriv.rHP_P1_Struct;
      }
      else if (usS_Dev == CSMD_S_DEV_STAT_INACT_NO_LINK)
      {
        /* No link at last slave on inactive port on P-Channel --> check inactive S-Channel */
        if (   (CSMD_HAL_GetLinkStatus(&prCSMD_Instance->rCSMD_HAL) & 0x2)
            && (prCSMD_Instance->rPriv.rRedundancy.boPriTelP2 == FALSE)
            && (prCSMD_Instance->rPriv.rRedundancy.boSecTelP2 == FALSE))
        {
          /* Link, but no Sercos telegram on S-Channel received */
          /* Set topology to "Broken ring" */
          CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P12_MODE);
          /* Set index for master */
          usSlaveIdx = 0xFFFF;
          /* Start Hot Plug mechanism */
          prHotPlug->prHP_ActPort = &prCSMD_Instance->rPriv.rHP_P2_Struct;
        }
        else
        {
          /* Sercos telegram on inactive S-Channel */
          prHotPlug->usHP_Phase = CSMD_NON_HP_MODE;
          eFuncRet = CSMD_HP_NO_HOTPLUG_SLAVE;
          break;
        }
      }
      else
      {
        /* Sercos P-telegram or  S-telegram at last slave on inactive port on P-Channel */
        prHotPlug->usHP_Phase = CSMD_NON_HP_MODE;
        eFuncRet = CSMD_HP_NO_HOTPLUG_SLAVE;
        break;
      }
      prHotPlug->usHP_IdxLastSlave = usSlaveIdx;    /* Last slave on P-Channel or 0xFFFF */
    }
    else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
    {
      usSAdd   = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[ prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb - 1];
      usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList [ usSAdd ];
      
      /* status of last slave in line */
      usS_Dev = prCSMD_Instance->arDevStatus[ usSlaveIdx ].usS_Dev;
      usS_Dev &= CSMD_S_DEV_STAT_INACT_MASK;
      
      if (usS_Dev == CSMD_S_DEV_STAT_INACT_LINK)
      {
        /* Link at last slave on S-Channel */
        /* Start Hot Plug mechanism */
        prHotPlug->prHP_ActPort = &prCSMD_Instance->rPriv.rHP_P2_Struct;
      }
      else if (usS_Dev == CSMD_S_DEV_STAT_INACT_NO_LINK)
      {
        /* No link at last slave on inactive port on S-Channel --> check inactive P-Channel */
        if (   (CSMD_HAL_GetLinkStatus(&prCSMD_Instance->rCSMD_HAL) & 0x1)
            && (prCSMD_Instance->rPriv.rRedundancy.boPriTelP1 == FALSE)
            && (prCSMD_Instance->rPriv.rRedundancy.boSecTelP1 == FALSE))
        {
          /* Link, but no Sercos telegram on P-Channel received */
          /* Set topology to "Broken ring" */
          CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P12_MODE);
          /* Set index for Master */
          usSlaveIdx = 0xFFFF;
          /* Start Hot Plug mechanism */
          prHotPlug->prHP_ActPort = &prCSMD_Instance->rPriv.rHP_P1_Struct;
        }
        else
        {
          /* Sercos telegram on inactive P-Channel */
          prHotPlug->usHP_Phase = CSMD_NON_HP_MODE;
          eFuncRet = CSMD_HP_NO_HOTPLUG_SLAVE;
          break;
        }
      }
      else
      {
        /* Sercos P-telegram or  S-telegram at last slave on inactive port on S-Channel */
        prHotPlug->usHP_Phase = CSMD_NON_HP_MODE;
        eFuncRet = CSMD_HP_NO_HOTPLUG_SLAVE;
        break;
      }
      prHotPlug->usHP_IdxLastSlave = usSlaveIdx;    /* Last slave on S-Channel or 0xFFFF */
    }
    
    else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
    {
      /* Get last slave on P-Channel */
      usSAdd   = prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[ prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb - 1];
      usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList [ usSAdd ];
      
      /* status of last slave in line on port 1 */
      usS_Dev = prCSMD_Instance->arDevStatus[ usSlaveIdx ].usS_Dev;
      usS_Dev &= CSMD_S_DEV_STAT_INACT_MASK;
      
      if (usS_Dev == CSMD_S_DEV_STAT_INACT_LINK)
      {
        prHotPlug->usHP_IdxLastSlave = usSlaveIdx;    /* Last slave on P-Channel */
        
        /* Link at last slave on P-Channel */
        /* Start Hot Plug mechanism on port 1 */
        prHotPlug->prHP_ActPort = &prCSMD_Instance->rPriv.rHP_P1_Struct;
      }
      else
      {
        /* Get last slave on S-Channel */
        usSAdd   = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[ prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb - 1];
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList [ usSAdd ];
        
        /* status of last slave in line on port 2 */
        usS_Dev = prCSMD_Instance->arDevStatus[ usSlaveIdx ].usS_Dev;
        usS_Dev &= CSMD_S_DEV_STAT_INACT_MASK;
        
        if (usS_Dev == CSMD_S_DEV_STAT_INACT_LINK)
        {
          prHotPlug->usHP_IdxLastSlave = usSlaveIdx;    /* Last slave on S-Channel */
          
          /* Link at last slave on S-Channel */
          /* Start Hot Plug mechanism on port 2 */
          prHotPlug->prHP_ActPort = &prCSMD_Instance->rPriv.rHP_P2_Struct;
        }
        else
        {
          /* No link or Sercos Link at last slave on inactive port off P-Channel and S-Channel */
          prHotPlug->usHP_Phase = CSMD_NON_HP_MODE;
          eFuncRet = CSMD_HP_NO_HOTPLUG_SLAVE;
          break;
        }
      }
    }
    else
    {
      /* Not supported topology (Ring) */
      prHotPlug->usHP_Phase = CSMD_NON_HP_MODE;
      eFuncRet = CSMD_HP_WRONG_TOPOLOGY;
      break;
    }
    
    prHotPlug->usHP_Phase = CSMD_HP_PHASE_0;
    
    /* Prepare for HP0 parameter transmission */
    prHotPlug->usHP_Topology   = prCSMD_Instance->usCSMD_Topology;
    prHotPlug->usHP0_ParamCnt  = 0;
    prHotPlug->usHP0_RepeatCnt = 0;
    prHotPlug->usHP_Timeout    = 0;
    prHotPlug->boHP0_CheckLink = FALSE;
    
    /* Set coding of HP0 parameter to "no parameter" and enable Hot-Plug */
    prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED 
                                                              | CSMD_HP_CNTRL_ENABLED
                                                              | CSMD_HP_CODE_NO_DATA );
    
    /* Set slave index and broadcast address for HP0 parameter transmission */
    prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord =
      (CSMD_HP_ADD_SLAVE_IDX_0 | CSMD_HP_ADD_BRDCST_ADD);
    
    prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong = 0;
    prFuncState->ulSleepTime = 
      prCSMD_Instance->rPriv.ulActiveCycTime / 1000000U;
    if (prFuncState->ulSleepTime == 0)
    {
      prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    }
    prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
    
    break;
    
    
  case CSMD_FUNCTION_STEP_1:  /* CSMD_FUNCSTATE_HP0_DO_TX_PARAM */
    /* --------------------------------------------------------- */
    /* Hot Plug Phase 0: Transmit HP0 parameters                 */
    /* --------------------------------------------------------- */
    /* -------------------------------------------------------------------- */
    /* 1.1: not yet reached timeout for transmission of HP0-parameter       */
    /* -------------------------------------------------------------------- */
    if (prHotPlug->usHP_Timeout < CSMD_HP0_PAR_SEND_CYCLES)
    {
      /* Check Sercos link on inactive port */
      if (TRUE == CSMD_HP_Is_Sercos_Link( prCSMD_Instance ))
      {
        /* Hot-plug not at master port and topology has changed? */
        if (   (prHotPlug->usHP_IdxLastSlave != 0xFFFF)
            && (prHotPlug->usHP_Topology != prCSMD_Instance->usCSMD_Topology))
        {
          /* Topology has changed */
          eFuncRet = CSMD_HP_TOPOLOGY_CHANGE;
        }
        else
        {
          /* Set coding of HP0 parameter to "no parameter" */
          prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED 
                                                                    | CSMD_HP_CNTRL_ENABLED
                                                                    | CSMD_HP_CODE_NO_DATA );
          /* Initialize HP-Info */
          prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong = 0;

          /* HP0 parameter transmission successful */
          prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
        }
      }
      else
      {
        CSMD_Send_HP0_Parameter( prCSMD_Instance );
      }
    }
    /* -------------------------------------------------------------------- */
    /* 1.2: reached timeout for transmission of HP0-parameter               */
    /* -------------------------------------------------------------------- */
    else if (prHotPlug->usHP_Timeout == CSMD_HP0_PAR_SEND_CYCLES)
    {
      if (prHotPlug->prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
      {
        /* No Sercos telegram on inactive port at last slave in Line port 1 */
        if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
        {
          /* Topology is broken ring. No hot-plug slave detected at port 1.
             If link at port 2 start hot-plug at port 2 */

          /* Get last slave on S-Channel */
          usSAdd   = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[ prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb - 1];
          usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList [ usSAdd ];

          /* status of last slave in line on port 2 */
          usS_Dev = prCSMD_Instance->arDevStatus[ usSlaveIdx ].usS_Dev;

          if ((usS_Dev & CSMD_S_DEV_STAT_INACT_MASK) == CSMD_S_DEV_STAT_INACT_LINK)
          {
            /* Set coding of HP0 parameter to "no parameter" and disable hot-plug at port 1 */
            prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED
                                                                      | CSMD_HP_CODE_NO_DATA );

            /* Initialize HP-Selection and HP-Info at port 1 */
            prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord = 0;
            prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong      = 0;

            /* Link at last slave on S-Channel */
            prHotPlug->usHP_IdxLastSlave = usSlaveIdx;    /* Last slave on S-Channel */
          }
          else
          {
            /* No link or Sercos Link at last slave on inactive port off P-Channel and S-Channel */
            eFuncRet = CSMD_HP_NO_HOTPLUG_SLAVE;
          }
        }
        else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
        {
          /* No link at last slave on inactive port on P-Channel --> check inactive S-Channel */
          if (   (CSMD_HAL_GetLinkStatus(&prCSMD_Instance->rCSMD_HAL) & 0x2)
              && (prCSMD_Instance->rPriv.rRedundancy.boPriTelP2 == FALSE)
              && (prCSMD_Instance->rPriv.rRedundancy.boSecTelP2 == FALSE))
          {
            /* Link, but no Sercos telegram on S-Channel received */
            /* Set topology to "Broken ring" */
            CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P12_MODE);

            /* Set coding of HP0 parameter to "no parameter" and disable hot-plug at port 1 */
            prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED
                                                                      | CSMD_HP_CODE_NO_DATA );

            /* Initialize HP-Selection and HP-Info at port 1 */
            prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord = 0;
            prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong    = 0;

            /* Set index for master */
            prHotPlug->usHP_IdxLastSlave = 0xFFFF;
          }
          else
          {
            /* No link or sercos Link at last slave on inactive port off P-Channel and S-Channel */
            eFuncRet = CSMD_HP_NO_HOTPLUG_SLAVE;
          }
        }
        else
        {
          eFuncRet = CSMD_HP_PHASE_0_TIMEOUT;
        }
      }
      else
      {
        /* HP Phase 0 timeout (no Sercos telegram at last slave on inactive port) */
        eFuncRet = CSMD_HP_PHASE_0_TIMEOUT;
#ifdef CSMD_DEBUG_HP
        printf ("\nC2C: 1 CSMD_HP_PHASE_0_TIMEOUT");
#endif
      }
      prHotPlug->usHP_Timeout++;
    }
    /* -------------------------------------------------------------------- */
    /* 1.3: Wait 10 Sercos cycles before Hot-Plug is enabled on port 2      */
    /* -------------------------------------------------------------------- */
    else if (prHotPlug->usHP_Timeout <= (CSMD_HP0_PAR_SEND_CYCLES + CSMD_PORT_CHANGE_WAIT_CYCLES))
    {
      prHotPlug->usHP_Timeout++;
    }
    /* -------------------------------------------------------------------- */
    /* 1.4: Start Hot-Plug mechanism on port 2                              */
    /* -------------------------------------------------------------------- */
    else
    {
      /* Prepare for HP0 parameter transmission */
      prHotPlug->usHP0_ParamCnt  = 0;
      prHotPlug->usHP0_RepeatCnt = 0;
      prHotPlug->usHP_Timeout    = 0;

      prHotPlug->prHP_ActPort = &prCSMD_Instance->rPriv.rHP_P2_Struct;

      /* Set coding of HP0 parameter to "no parameter" and enable Hot-Plug at port 2 */
      prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED
                                                                | CSMD_HP_CNTRL_ENABLED
                                                                | CSMD_HP_CODE_NO_DATA );

      /* Set slave index and broadcast address for HP0 parameter transmission at port 2*/
      prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord =
        (CSMD_HP_ADD_SLAVE_IDX_0 | CSMD_HP_ADD_BRDCST_ADD);
      prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong = 0;
    }

    if (eFuncRet != CSMD_FUNCTION_IN_PROCESS)
    {
      /* Disable Hot-Plug */
      prHotPlug->usHP_Phase = CSMD_NON_HP_MODE;

      /* Set coding of HP0 parameter to "no parameter" and disable hot-plug  */
      prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED
                                                                | CSMD_HP_CODE_NO_DATA );

      /* Initialize HP-Selection and HP-Info */
      prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord = 0;
      prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong      = 0;
    }
    break;


  case CSMD_FUNCTION_STEP_2:  /* CSMD_FUNCSTATE_HP0_DO_SCAN_SLAVES */
#ifdef CSMD_DEBUG_HP
    printf ("\nC2C: 3 Scan timeout %d",prHotPlug->usHP_Timeout);
#endif

    /* --------------------------------------------------------- */
    /* Hot Plug Phase 0: Prepare for scanning of Hot plug slaves */
    /* --------------------------------------------------------- */
    if (prHotPlug->usHP_IdxLastSlave == 0xFFFF)
    {
      /* Hot-Plug at inactive master port */
      prFuncState->usActState  = CSMD_FUNCTION_STEP_4;
    }
    else
    {
      /* Activate fast forward on both ports at last slave */
      if (  prCSMD_Instance->arDevStatus[ prHotPlug->usHP_IdxLastSlave ].usS_Dev & CSMD_S_DEV_TOPOLOGY_HS)
      {
        /* Command Topology: fast forward on both ports */
        prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] = (CSMD_USHORT)
          ((  prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ]
            & ~(CSMD_C_DEV_CMD_TOPOLOGY_MASK |CSMD_C_DEV_TOPOLOGY_HS)) | CSMD_C_DEV_CMD_TOPOLOGY_FF_BOTH);
      }
      else
      {
        /* Command Topology: fast forward on both ports */
        prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] = (CSMD_USHORT)
          ((  prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ]
            & ~CSMD_C_DEV_CMD_TOPOLOGY_MASK) | (CSMD_C_DEV_CMD_TOPOLOGY_FF_BOTH | CSMD_C_DEV_TOPOLOGY_HS));
      }
      prFuncState->usActState  = CSMD_FUNCTION_STEP_3;
    }

    /* Initialize slave index and slave address for slave scan */
    prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord =
      (CSMD_USHORT) (CSMD_HP_ADD_SLAVE_IDX_0 | CSMD_HP_ADD_DEFAULT_SADD);

    /* Init slave index for address scan */
    prHotPlug->usHP_ScanIdx = 0;

    prHotPlug->usHP0_RepeatCnt = 0;

    prFuncState->ulSleepTime = 0;
    break;


  case CSMD_FUNCTION_STEP_3:  /* CSMD_FUNCSTATE_HP0_EXIT */
    /* --------------------------------------------------------- */
    /* Hot Plug Phase 0: Check S-DEV topology status             */
    /* --------------------------------------------------------- */
    if (   (prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] & (CSMD_C_DEV_TOPOLOGY_HS | CSMD_C_DEV_CMD_TOPOLOGY_MASK))
        != (prCSMD_Instance->arDevStatus[ prHotPlug->usHP_IdxLastSlave ].usS_Dev & (CSMD_S_DEV_TOPOLOGY_HS | CSMD_S_DEV_TOPOLOGY_STS_MASK)) )
    {
      /* Topology HS in device control and device status are not the same */
      if (prHotPlug->usHP0_RepeatCnt < prHotPlug->usHP0_ScanTimeout)
      {
        prHotPlug->usHP0_RepeatCnt++;
      }
      else
      {
        /* Timeout during topology HS */
#ifdef CSMD_DEBUG_HP
        printf ("\nC2C: 1 CSMD_HP_SLAVE_SCAN_TIMEOUT");
#endif
        prHotPlug->eHP_FuncRet = CSMD_HP_SLAVE_SCAN_TIMEOUT;
        /* Disable Hot-Plug */
        prFuncState->usActState  = CSMD_FUNCTION_STEP_10;
      }
    }
    else
    {
#ifdef CSMD_DEBUG_HP
      printf ("\nC2C: 1 Scan timeout %d",prHotPlug->usHP0_RepeatCnt);
#endif
      prHotPlug->usHP0_RepeatCnt = 0;
      prHotPlug->usHP_Phase      = CSMD_HP_PHASE_1;
      prFuncState->usActState    = CSMD_FUNCTION_STEP_4;
    }
    prFuncState->ulSleepTime = 0;
    break;


  case CSMD_FUNCTION_STEP_4:  /* CSMD_FUNCSTATE_HP1_ENTRY */
    /* --------------------------------------------------------- */
    /* Hot Plug Phase 1: Scan Hot plug slaves                    */
    /* --------------------------------------------------------- */
    if ((  prHotPlug->prHP_ActPort->rHpField_AT0.rSelection.usWord
         ^ prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord) & CSMD_HP_ADD_SLAVE_IDX_MASK)
    {
      /* Slave Index of HP control and HP status are not the same */
      if (prHotPlug->usHP0_RepeatCnt < prHotPlug->usHP0_ScanTimeout)
      {
        prHotPlug->usHP0_RepeatCnt++;
      }
      else
      {
        /* Timeout during slave scanning */
#ifdef CSMD_DEBUG_HP
        printf ("\nC2C: 2 CSMD_HP_SLAVE_SCAN_TIMEOUT");
#endif
        prHotPlug->eHP_FuncRet = CSMD_HP_SLAVE_SCAN_TIMEOUT;
        /* Disable Hot-Plug */
        prFuncState->usActState = CSMD_FUNCTION_STEP_10;
      }
    }
    else
    {
      if (   (prHotPlug->prHP_ActPort->rHpField_AT0.rStatus.usWord & (CSMD_HP_STAT_ERROR | CSMD_HP_STAT_ACKN_ERR_MASK))
          != CSMD_HP_ACKN_SLAVE_SCAN_OK)
      {
        /* No acknowledgment */
        if (prHotPlug->usHP0_RepeatCnt < prHotPlug->usHP0_ScanTimeout)
        {
          prHotPlug->usHP0_RepeatCnt++;
        }
        else
        {
          /* Timeout during slave scanning */
#ifdef CSMD_DEBUG_HP
          printf ("\nC2C: 3 CSMD_HP_SLAVE_SCAN_TIMEOUT");
#endif
          prHotPlug->eHP_FuncRet = CSMD_HP_SLAVE_SCAN_TIMEOUT;
          /* Disable Hot-Plug */
          prFuncState->usActState = CSMD_FUNCTION_STEP_10;
        }
        break;
      }

      /* Same slave Index in HP control and HP status */
      usSAdd = (CSMD_USHORT)(prHotPlug->prHP_ActPort->rHpField_AT0.rSelection.usWord & CSMD_HP_ADD_SERCOS_ADD_MASK);

      if (usSAdd < CSMD_MAX_SLAVE_ADD)
      {
        /* Store valid Sercos address of found slave */
        prHotPlug->ausHP_SlaveAddList[ prHotPlug->usHP_ScanIdx + 2 ] = usSAdd;
        prHotPlug->ausHP_SlaveAddList[0] += 2;  /* adjust current list length */

        prHotPlug->usHP_ScanIdx++;
        if (prHotPlug->usHP_ScanIdx < CSMD_HP_ADD_MAX_SLAVES)
        {
          /* scan next slave */
          /* Increment slave index */
          prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord += CSMD_HP_ADD_SLAVE_IDX_OFFSET;
          prHotPlug->usHP0_RepeatCnt = 0;
        }
        else
        {
          /* Scan complete: Found maximum nbr of slaves */
          prFuncState->ulSleepTime = 0;
          prFuncState->usActState  = CSMD_FUNCTION_STEP_5;
        }
      }
      else if (usSAdd == CSMD_HP_ADD_NOT_EXIST) /* End of scanning */
      {
        if (prHotPlug->usHP_ScanIdx)
        {
          /* Scan complete: At least one valid slave address detected */
          prFuncState->ulSleepTime = 0;
          prFuncState->usActState  = CSMD_FUNCTION_STEP_5;
        }
        else
        {
          /* No slave found at scan */
          prHotPlug->eHP_FuncRet  = CSMD_HP_NO_SLAVE_FOUND;
          /* Disable Hot-Plug */
          prFuncState->usActState = CSMD_FUNCTION_STEP_10;
        }
      }
      else
      {
        /* Wrong Sercos address. */
        prHotPlug->eHP_FuncRet  = CSMD_HP_PHASE_0_FAILED;
        /* Disable Hot-Plug */
        prFuncState->usActState = CSMD_FUNCTION_STEP_10;
      }
    }

    if (prFuncState->usActState == CSMD_FUNCTION_STEP_5)
    {
      if (prHotPlug->usHP_NbrSlaves > CSMD_HP_ADD_MAX_SLAVES)
      {
        /* Too many Hot-Plug slaves */
        prHotPlug->eHP_FuncRet  = CSMD_HP_PHASE_0_FAILED;
        /* Disable Hot-Plug */
        prFuncState->usActState = CSMD_FUNCTION_STEP_10;
      }
      else
      {
        prHotPlug->usHP_NbrSlaves = prHotPlug->usHP_ScanIdx;
      }
    }
    break;


  case CSMD_FUNCTION_STEP_5:  /* CSMD_FUNCSTATE_HP1_DO_CHECK_ADDR */
#ifdef CSMD_DEBUG_HP
    printf ("\nC2C: 2 Scan timeout %d",prHotPlug->usHP0_RepeatCnt);
#endif
    /* --------------------------------------------------------- */
    /* Hot Plug Phase 1: Check addresses of scanned HP slaves    */
    /* --------------------------------------------------------- */
    {
      CSMD_USHORT *pausAddList = &prHotPlug->ausHP_SlaveAddList[2];
      CSMD_USHORT  usJ;          /* Loop counter */

      for (usI = 0; usI < prHotPlug->usHP_NbrSlaves; usI++)
      {
        usSAdd = pausAddList[usI];
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList [ usSAdd ];

        if ((usSAdd < CSMD_HP_ADD_MIN_SADD) || (usSAdd > CSMD_HP_ADD_MAX_SADD))
        {
          /* Illegal Sercos Address of HP slave */
          prHotPlug->eHP_FuncRet = CSMD_HP_ILLEGAL_SLAVE_ADDRESS;
          /* Disable Hot-Plug */
          prFuncState->usActState = CSMD_FUNCTION_STEP_10;
        }
        else if (usSAdd != prCSMD_Instance->rSlaveList.ausProjSlaveAddList[usSlaveIdx + 2])
        {
          /* HP slave is not projected */
          prHotPlug->eHP_FuncRet = CSMD_HP_SLAVE_IS_NOT_PROJECTED;
          /* Disable Hot-Plug */
          prFuncState->usActState = CSMD_FUNCTION_STEP_10;
        }
        else if (prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] == CSMD_SLAVE_ACTIVE)
        {
          /* Slave with this address already found in CP0; wrong HP slave address! */
          prHotPlug->eHP_FuncRet = CSMD_HP_SLAVE_RECOGNIZED_IN_CP0;
          /* Disable Hot-Plug */
          prFuncState->usActState = CSMD_FUNCTION_STEP_10;
        }
        else
        {
          /* Check for double addresses in scanned HP slave address list */
          for (usJ = (CSMD_USHORT)(usI+1); usJ < prHotPlug->usHP_NbrSlaves; usJ++)
          {
            if (usSAdd == pausAddList[usJ])
            {
              /* Two slaves with identical Sercos address were found */
              prHotPlug->eHP_FuncRet = CSMD_HP_DOUBLE_SLAVE_ADDRESSES;
              /* Disable Hot-Plug */
              prFuncState->usActState = CSMD_FUNCTION_STEP_10;
              break;
            }
          }
        }
        if (prFuncState->usActState == CSMD_FUNCTION_STEP_10)
        {
          break;
        }
      } /* for (usI = 0; usI < prCSMD_Instance->rPriv.usHP_ScanIdx; usI++) */
    }

    if (prFuncState->usActState == CSMD_FUNCTION_STEP_5)
    {
      /* Slave Index = 0,  Sercos address = 0 */
      prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord = 0;

      /* Init slave index for transmit of HP1 parameters */
      prHotPlug->usHP_ScanIdx   = 0;
      prHotPlug->usHP0_ParamCnt = 0;
      prHotPlug->usHP_Timeout   = 0;

      prFuncState->usActState = CSMD_FUNCTION_STEP_6;
    }
    break;


  case CSMD_FUNCTION_STEP_6:  /* CSMD_FUNCSTATE_HP1_DO_TX_PARAM */
    /* --------------------------------------------------------- */
    /* Hot Plug Phase 1: Transmit HP1 parameters                 */
    /* --------------------------------------------------------- */
    if (prHotPlug->usHP_ScanIdx < prHotPlug->usHP_NbrSlaves)
    {
      switch (prHotPlug->usHP0_ParamCnt)
      {
        /* ------------------------------------ */
        /* Transmit S-0-1013, SVC offset in MDT */
        /* ------------------------------------ */
        case 0:
          /* 1.Cycle: Start transmission */
          if (prHotPlug->usHP_Timeout == 0)
          {
            CSMD_USHORT  usHP_Selection;
            /* Set MDT-HP.Slave_index */
            usHP_Selection = (CSMD_USHORT)(prHotPlug->usHP_ScanIdx << CSMD_HP_ADD_SLAVE_IDX_SHIFT);
            /* Set MDT-HP.ADR */
            usHP_Selection |= prHotPlug->ausHP_SlaveAddList[ prHotPlug->usHP_ScanIdx + 2 ];
            prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord = usHP_Selection;

            /* S-0-1013, SVC offset in MDT */
            prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
              (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_MDT_SVC_OFFS);

            usSAdd = prHotPlug->ausHP_SlaveAddList[ prHotPlug->usHP_ScanIdx + 2 ];
            usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList [ usSAdd ];

            prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
              prCSMD_Instance->rConfiguration.parSlaveConfig[ usSlaveIdx ].rTelegramConfig.usSvcOffsetMDT_S01013;

            prHotPlug->usHP_Timeout++;
          }
          /* 2. to 10. Cycle: Check acknowledge of slave */
          else if (prHotPlug->usHP_Timeout <  prCSMD_Instance->rPriv.usSVC_BUSY_Timeout)
          {
            if (  prHotPlug->prHP_ActPort->rHpField_AT0.rStatus.usWord
                & CSMD_HP_STAT_ERROR)
            {
              /* HP1 parameter reception error */
            }
            else if (   (prHotPlug->prHP_ActPort->rHpField_AT0.rStatus.usWord & CSMD_HP_STAT_ACKN_ERR_MASK)
                     != CSMD_HP_STAT_CODE_MDT_SVC_OFFS)
            {
              /* Unexpected HP1 acknowledgment code */
            }
            else if (    prHotPlug->prHP_ActPort->rHpField_AT0.rSelection.usWord
                     !=  prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord)
            {
              /* Slave Index and Sercos address of HP control and HP status are not the same */
            }
            else
            {
              /* HP1 parameter transmission without error */
              /* Transmit next parameter */
              prHotPlug->usHP0_ParamCnt++;
              prHotPlug->usHP_Timeout = 0;
              break;
            }
            prHotPlug->usHP_Timeout++;
          }
          else    /* Timeout */
          {
            /* HP1 parameter transmission failed */
            /* Timeout, no positive acknowledge of slave */

            /* HP Phase 1 timeout (parameter transmission failed; no positive acknowledge of slave */
            prHotPlug->eHP_FuncRet = CSMD_HP_PHASE_1_TIMEOUT;
            /* Disable Hot-Plug */
            prFuncState->usActState  = CSMD_FUNCTION_STEP_10;
          }
          break;

        /* ------------------------------------ */
        /* Transmit S-0-1014, SVC offset in AT  */
        /* ------------------------------------ */
        case 1:
          /* 1.Cycle: Start transmission */
          if (prHotPlug->usHP_Timeout == 0)
          {
            CSMD_USHORT  usHP_Selection;
            /* Set MDT-HP.Slave_index */
            usHP_Selection = (CSMD_USHORT)(prHotPlug->usHP_ScanIdx << CSMD_HP_ADD_SLAVE_IDX_SHIFT);
            /* Set MDT-HP.ADR */
            usHP_Selection |= prHotPlug->ausHP_SlaveAddList[ prHotPlug->usHP_ScanIdx + 2 ];
            prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord = usHP_Selection;

            /* S-0-1014, SVC offset in AT */
            prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
              (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_AT_SVC_OFFS);

            usSAdd = prHotPlug->ausHP_SlaveAddList[ prHotPlug->usHP_ScanIdx + 2 ];
            usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList [ usSAdd ];

            prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
              prCSMD_Instance->rConfiguration.parSlaveConfig[ usSlaveIdx ].rTelegramConfig.usSvcOffsetAT_S01014;

            prHotPlug->usHP_Timeout++;
          }
          /* 2. to 10. Cycle: Check acknowledge of slave */
          else if (prHotPlug->usHP_Timeout <  prCSMD_Instance->rPriv.usSVC_BUSY_Timeout)
          {
            if (  prHotPlug->prHP_ActPort->rHpField_AT0.rStatus.usWord
                & CSMD_HP_STAT_ERROR)
            {
              /* HP1 parameter reception error */
            }
            else if (   (prHotPlug->prHP_ActPort->rHpField_AT0.rStatus.usWord & CSMD_HP_STAT_ACKN_ERR_MASK)
                     != CSMD_HP_STAT_CODE_AT_SVC_OFFS)
            {
              /* Unexpected HP1 acknowledgment code */
            }
            else if (    prHotPlug->prHP_ActPort->rHpField_AT0.rSelection.usWord
                     !=  prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord)
            {
              /* Slave Index and Sercos address of HP control and HP status are not the same */
            }
            else
            {
              /* HP1 parameter transmission without error */
              /* Transmit next parameter */
              prHotPlug->usHP0_ParamCnt++;
              prHotPlug->usHP_Timeout = 0;
              break;
            }
            prHotPlug->usHP_Timeout++;
          }
          else    /* Timeout */
          {
            /* HP1 parameter transmission failed */
            /* Timeout, no positive acknowledge of slave */

            /* HP Phase 1 Timeout (parameter transmission failed; no positive acknowledge of slave */
            prHotPlug->eHP_FuncRet = CSMD_HP_PHASE_1_TIMEOUT;
            /* Disable Hot-Plug */
            prFuncState->usActState  = CSMD_FUNCTION_STEP_10;
          }
          break;

        /* ---------------------------------------- */
        /* Actual Slave: HP1 parameters transmitted */
        /* ---------------------------------------- */
        case 2:
          prHotPlug->usHP0_ParamCnt = 0;
          /* Select next slave */
          prHotPlug->usHP_ScanIdx++;
          break;

        default:
          /* CoSeMa internal error */
          CSMD_RuntimeError( prCSMD_Instance, 0,
            "CSMD_HotPlug(): Illegal case in CSMD_FUNCTION_STEP_6" );
          eFuncRet = CSMD_ILLEGAL_CASE;
          break;

      } /* End: switch (prHotPlug->usHP0_ParamCnt) */

    }
    else    /* if (prHotPlug->usHP_ScanIdx < prHotPlug->usHP_NbrSlaves) */
    {
      /* Set MHS in C-SVC for each slave  */
      for (usI = 0; usI < prHotPlug->usHP_NbrSlaves; usI++)
      {
        usSAdd = prHotPlug->ausHP_SlaveAddList[ usI + 2 ];
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList [ usSAdd ];

        /* Assign SVC status pointer to the active port for first life signs of the Hot-Plug slaves */
        if (   (prHotPlug->prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
            || (   (TRUE == prCSMD_Instance->rPriv.boHW_SVC_Redundancy)
                && ((CSMD_SHORT)usSlaveIdx < CSMD_MAX_HW_CONTAINER)))
        {
          /* Assign SVC status to Port 1 */
          /* Note: In case of activated IP-Core SVC redundancy, the SVC status pointer in the
                   IP-Core SVC-Ram shall be assigned to port 1. The service channel located in
                   Rx-Ram is still related to port 1 respectively port 2 ! */
          CSMD_HAL_WriteShort( &prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx]->usSVCRxPointer_Status,
                               prCSMD_Instance->rPriv.arSVCInternalStruct[usSlaveIdx].usSVCRxRamPntrP1);
        }
        else
        {
          /* Assign SVC status to Port 2 */
          CSMD_HAL_WriteShort( &prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx]->usSVCRxPointer_Status,
                               prCSMD_Instance->rPriv.arSVCInternalStruct[usSlaveIdx].usSVCRxRamPntrP2);
        }

        if ((CSMD_SHORT)usSlaveIdx < CSMD_MAX_HW_CONTAINER)
        {
          /* Set MHS bit in the SVC container */
          CSMD_HAL_WriteShort(  &prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx]->rCONTROL.usWord[0], 
                                CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx]->rCONTROL.usWord[0] ) 
                              | CSMD_SVC_CTRL_HANDSHAKE
                             );

          /* Get offset of SVC in TxRam */
          usSvcOffset = CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx]->usSVCTxPointer_Control );
          /* Set C-SVC MHS bit in TxRam */
          CSMD_HAL_WriteShort( ((CSMD_USHORT *)(CSMD_VOID *)prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram + (usSvcOffset / 2)),
                               CSMD_SVC_CTRL_HANDSHAKE );
        }
        else
        {
          /* Set MHS bit in the SVC container */
          prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx]->rCONTROL.usWord[0] |= CSMD_SVC_CTRL_HANDSHAKE;
          /* Set AHS bit in the SVC container */
          prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx]->rCONTROL.usWord[1] |= CSMD_SVC_STAT_HANDSHAKE;

          /* Get offset of SVC in TxRam */
          usSvcOffset = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx]->usSVCTxPointer_Control;
#ifdef CSMD_PCI_MASTER
          if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
          {
            /* Set C-SVC MHS bit in TxRam in the local Tx telegram buffer */
            *((CSMD_USHORT *)(CSMD_VOID *)prCSMD_Instance->prTelBuffer->rLocal.rBuffTxRam.aulTx_Ram + (usSvcOffset / 2)) =
              CSMD_SVC_CTRL_HANDSHAKE;
          }
          else
#endif
          {
            /* Set C-SVC MHS bit in TxRam */
            CSMD_HAL_WriteShort( ((CSMD_USHORT *)(CSMD_VOID *)prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram + (usSvcOffset / 2)),
                                 CSMD_SVC_CTRL_HANDSHAKE );
          }
        }
      }

      /* Activate transmission via SVC / HP0 coding = no parameter */
      prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED
                                                                | CSMD_HP_CNTRL_ENABLED
                                                                | CSMD_HP_CODE_NO_DATA
                                                                | CSMD_HP_CNTRL_SVC_ACTIVE );

      prHotPlug->prHP_ActPort->rHpField_MDT0.rSelection.usWord = 0;
      prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong = 0U;

      prHotPlug->usHP_Timeout = 0;
      prFuncState->usActState = CSMD_FUNCTION_STEP_7;
    }

    break;
    
    
  case CSMD_FUNCTION_STEP_7:  /* CSMD_FUNCSTATE_HP1_DO_CHECK_SVC */
    /* --------------------------------------------------------- */
    /* Hot Plug Phase 1: Check for proper activation of SVC      */
    /* --------------------------------------------------------- */
    if (prHotPlug->usHP_Timeout < prCSMD_Instance->rPriv.usSVC_BUSY_Timeout)
    {
      if (  prHotPlug->prHP_ActPort->rHpField_AT0.rStatus.usWord 
          & CSMD_HP_STAT_ERROR)
      {
        /* Error switching to SVC (Error code = CSMD_HP_ERR_SWITCH_TO_SVC ) */

        /* HP Phase 1: Switch to Service Channel Communication failed */
        prHotPlug->eHP_FuncRet = CSMD_HP_SWITCH_TO_SVC_FAILED;
        /* Disable Hot-Plug */
        prFuncState->usActState  = CSMD_FUNCTION_STEP_10;
      }

      /* Check AHS and SVC-valid in S-SVC for each slave  */
      for (usI = 0; usI < prHotPlug->usHP_NbrSlaves; usI++)
      {
        usSAdd = prHotPlug->ausHP_SlaveAddList[ usI + 2 ];
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList [ usSAdd ];

        if (prCSMD_Instance->rPriv.rHotPlug.prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
        {
          usSvcOffset = prCSMD_Instance->rPriv.arSVCInternalStruct[usSlaveIdx].usSVCRxRamPntrP1;
        }
        else
        {
          usSvcOffset = prCSMD_Instance->rPriv.arSVCInternalStruct[usSlaveIdx].usSVCRxRamPntrP2;
        }
        /* Check Service Channel Status */
        if (   (  CSMD_HAL_ReadShort( (CSMD_VOID *)((CSMD_UCHAR *)(prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram) + usSvcOffset) )
                & (CSMD_SVC_STAT_VALID | CSMD_SVC_STAT_HANDSHAKE))
            != (CSMD_SVC_STAT_VALID | CSMD_SVC_STAT_HANDSHAKE) )
        {
          prHotPlug->usHP_Timeout++;
          break;
        }
      }

      if (usI == prHotPlug->usHP_NbrSlaves)
      {
        /* SVC of all hot plugged slaves ready */
        prFuncState->usActState  = CSMD_FUNCTION_STEP_8;
      }
    }
    else
    {
      /* Timeout */

      /* HP Phase 1: Timeout during switch to Service Channel communication */
      prHotPlug->eHP_FuncRet = CSMD_HP_SWITCH_TO_SVC_TIMEOUT;
      /* Disable Hot-Plug */
      prFuncState->usActState  = CSMD_FUNCTION_STEP_10;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_8:  /* CSMD_FUNCSTATE_HP1_DO_ACTIVATE */
    /* --------------------------------------------------------- */
    /* Hot Plug Phase 2: Adjust lists with new slaves            */
    /* --------------------------------------------------------- */

    /* Copy slave address Sercos list */
    for (usI = 0; usI < prHotPlug->usHP_NbrSlaves + 2; usI++)
    {
      /* Return Sercos list with Sercos addresses of the Hot-Plug slaves */
      *(pusHPSlaveAddList+usI) = prHotPlug->ausHP_SlaveAddList[usI];
    }

    for (usI = 2; usI < prHotPlug->usHP_NbrSlaves + 2; usI++)
    {
      usSAdd   =  *(pusHPSlaveAddList+usI);
      usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[ usSAdd ];

      /* 1. Set activity status to hot plug for SVC transmissions */
      prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] = CSMD_SLAVE_HP_IN_PROCESS;

      /* 2.  */
      if (prHotPlug->prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
      {
        /* Hot Plug slave(s) on port 1 */
        prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb] =
          usSAdd;
        prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb++;

        /* assign hot plug slave(s) preferably to port 1 */
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usSlaveIdx] = CSMD_PORT_1;
      }
      else
      {
        /* Hot Plug slave(s) on port 2 */
        prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb] =
          usSAdd;
        prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb++;

        /* assign hot plug slave(s) preferably to port 2 */
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usSlaveIdx] = CSMD_PORT_2;
      }

      if ((CSMD_SHORT)usSlaveIdx >= CSMD_MAX_HW_CONTAINER)
      {
        /* Copy the status word of the slave from RX ram to the local status word in the svc container */
        prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx]->rCONTROL.usWord[1] = 
          CSMD_HAL_ReadShort( prCSMD_Instance->rPriv.pusRxRam + CSMD_END_CONV_S(prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx]->usSVCRxPointer_Status)/2 );
      }
      
      /* Initialize C-DEV */
      prCSMD_Instance->rPriv.ausDevControl[usSlaveIdx] = CSMD_C_DEV_MASTER_VALID;
    }
    
    /* 6. Set variables for redundancy feature */
    if (prHotPlug->prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
    {
      /* Slave address of Slave at the end of Line on Port1 */
      prCSMD_Instance->usSercAddrLastSlaveP1 =
        prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[ prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb -1 ];
    }
    else
    {
      /* Slave address of Slave at the end of Line on Port2 */
      prCSMD_Instance->usSercAddrLastSlaveP2 =
        prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[ prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb -1 ];
    }
    
    if (prHotPlug->usHP_IdxLastSlave == 0xFFFF)
    {
      /* Hot-Plug at inactive master port */
      prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_BROKEN_RING;
    }

    prFuncState->usActState  = CSMD_FUNCTION_STEP_9;
    break;


  case CSMD_FUNCTION_STEP_9:  /* CSMD_FUNCSTATE_HP1_EXIT */
    /* --------------------------------------------------------- */
    /* Hot Plug Phase 2: Ready                                   */
    /* --------------------------------------------------------- */

      /* SVC reconfigured */
      prHotPlug->usHP_Phase = CSMD_HP_PHASE_2;
      eFuncRet = CSMD_NO_ERROR;
    break;
    
    
  case CSMD_FUNCTION_STEP_10: /* CSMD_FUNCSTATE_HP__ENTRY_ERROR */
    /* --------------------------------------------------------- */
    /* Perform end of Hot Plug because of error condition.       */
    /* eHP_FuncRet must be set with corresponding                */
    /* function error code!                                      */
    /* --------------------------------------------------------- */

    if (prHotPlug->usHP_IdxLastSlave == 0xFFFF)
    {
      /* Hot-Plug at inactive master port */
      if (prHotPlug->prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
      {
        /* Set topology to "Single line on port 2 " */
        CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P2_MODE);
      }
      else
      {
        /* Set topology to "Single line on port 1 " */
        CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P1_MODE);
      }
    }
    else
    {
      /* status of last slave in line */
      usS_Dev = prCSMD_Instance->arDevStatus[ prHotPlug->usHP_IdxLastSlave ].usS_Dev;
      usS_Dev &= CSMD_S_DEV_TOPOLOGY_STS_MASK;
      
      if (usS_Dev == CSMD_S_DEV_CURRENT_LB_FW_P_TEL)
      {
        prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] = (CSMD_USHORT)
          (  (prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] & ~CSMD_C_DEV_CMD_TOPOLOGY_MASK)
           | CSMD_C_DEV_CMD_LOOPB_FW_P_TEL);
      }
      else if (usS_Dev == CSMD_S_DEV_CURRENT_LB_FW_S_TEL)
      {
        prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] = (CSMD_USHORT)
          (  (prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] & ~CSMD_C_DEV_CMD_TOPOLOGY_MASK)
           | CSMD_C_DEV_CMD_LOOPB_FW_S_TEL);
      }
      else    /* Fast-Forward on both ports */
      {
        /* Is Hot-Plug realized on port 1? */
        if (prHotPlug->prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
        {
          prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] = (CSMD_USHORT)
            (  (prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] & ~CSMD_C_DEV_CMD_TOPOLOGY_MASK)
             | CSMD_C_DEV_CMD_LOOPB_FW_P_TEL);
        }
        else
        {
          prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] = (CSMD_USHORT)
            (  (prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] & ~CSMD_C_DEV_CMD_TOPOLOGY_MASK)
             | CSMD_C_DEV_CMD_LOOPB_FW_S_TEL);
        }
      }

      /* Toggle topology change command bit */
      prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] ^= CSMD_C_DEV_TOPOLOGY_HS;
    }

    prFuncState->ulSleepTime =
       prCSMD_Instance->rPriv.ulActiveCycTime / 1000000U;
    if (prFuncState->ulSleepTime == 0)
    {
      prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    }
    prFuncState->usActState = CSMD_FUNCTION_STEP_11;
    prHotPlug->usHP_Timeout = 0;
    break;


  case CSMD_FUNCTION_STEP_11: /* CSMD_FUNCSTATE_HP__EXIT_ERROR */
    if (prHotPlug->usHP_IdxLastSlave != 0xFFFF)
    {
      if (prHotPlug->usHP_Timeout++ < 10)
      {
        if (   (prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] &  CSMD_C_DEV_TOPOLOGY_HS)
            != (prCSMD_Instance->arDevStatus[ prHotPlug->usHP_IdxLastSlave ].usS_Dev &  CSMD_S_DEV_TOPOLOGY_HS) )
        {
          /* Handshake bits in C-DEV and S-DEV of last slave in line not equal */
          break;
        }
      }
      else
      {
        /* Switch to loopback & forward at the last slave in line is failed. */
        prHotPlug->eHP_FuncRet = CSMD_HP_LAST_SLAVE_LB_RESTORE_TIMEOUT;
      }
    }

    /* Perform end of hot plug on both ports */
    /* Set coding of HP0 parameter to "no parameter" and disable Hot-Plug */
    prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED
                                                                          | CSMD_HP_CODE_NO_DATA );
    prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED
                                                                          | CSMD_HP_CODE_NO_DATA );
    eFuncRet = prHotPlug->eHP_FuncRet;
    break;


  default:
    /* CoSeMa internal error */
    CSMD_RuntimeError( prCSMD_Instance, 0,
                       "CSMD_HotPlug(): Illegal case" );
    eFuncRet = CSMD_ILLEGAL_CASE;
    break;

  }   /* End: switch (prFuncState->usActState) */

  if (eFuncRet != CSMD_FUNCTION_IN_PROCESS)
  {
    prHotPlug->boHotPlugActive = FALSE;
  }

  return  (eFuncRet);

} /* End: CSMD_HotPlug() */



/**************************************************************************/ /**
\brief Transmits communication and timing parameters to
       all the slaves of the Hot Plug device via the service channel.

\ingroup module_hotplug
\b Description: \n
   The following processes are executed for each slave:
   - reading and evaluation of S-0-1000 (see CSMD_CheckVersion() )
   - reading and writing of connection configuration (see CSMD_GetTimingData() )
   - writing of telegram and timing parameters (see CSMD_TransmitTiming() )
   - execution of commands S-0-1024, S-0-0127 and S-0-0128

   After the parameters have been transmitted successfully, the commands S-0-1024,
   S-0-0127 and S-0-0128 are executed successively. If no error occurs during this
   process, the Hot Plug device is in CP4.\n
   A premature cancellation of this function is possible setting the transfer
   parameter boCancel to TRUE. In this case the original topology is recovered.

<B>Call Environment:</B> \n
   This function may only be called in CP4. Furthermore, the Hot Plug device
   must remain in Hot Plug phase 2.\n
   The call-up should be performed.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures
\param [in]   boCancel
              If set to TRUE, cancel HP phase 2 parameter transmission and recover
              previous topology

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_INTERNAL_REQUEST_PENDING \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_HP_NOT_SUPPORTED \n
              \ref CSMD_HP_NOT_WITH_CLOSED_RING \n
              \ref CSMD_HP_WRONG_PHASE \n
              \ref CSMD_HP_OPERATION_ABORTED \n
              \ref CSMD_S_0_1024_CMD_ERROR \n
              \ref CSMD_CP3_TRANS_CHECK_CMD_ERROR \n
              \ref CSMD_CP4_TRANS_CHECK_CMD_ERROR \n
              \ref CSMD_SERCOS_VERSION_MISMATCH \n
              \ref CSMD_BASIC_SCP_TYPE_MISMATCH \n
              \ref CSMD_TOO_MANY_CONNECTIONS \n
              \ref CSMD_INVALID_CONNECTION \n
              \ref CSMD_INVALID_MTU \n
              \ref CSMD_CONNECTION_LENGTH_0 \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n

\author       wk
\date         2008-04-25

***************************************************************************** */
CSMD_FUNC_RET CSMD_TransHP2Para( CSMD_INSTANCE          *prCSMD_Instance,
                                 CSMD_FUNC_STATE        *prFuncState,
                                 CSMD_SVCH_MACRO_STRUCT *parSvcMacro,
                                 CSMD_BOOL               boCancel )
{
  
  CSMD_FUNC_RET      eFuncRet  = CSMD_FUNCTION_IN_PROCESS;  /* Function processing still active */
  CSMD_HOT_PLUG_AUX *prHotPlug = &prCSMD_Instance->rPriv.rHotPlug;
  CSMD_USHORT        usI;
  CSMD_USHORT        usSlaveIdx;
  CSMD_USHORT        usS_Dev;     /* S-DEV */
  
  
  /* --------------------------------------------------------- */
  /* Check whether current communication phase is CP4.         */
  /* --------------------------------------------------------- */
  if (prCSMD_Instance->sCSMD_Phase != CSMD_SERC_PHASE_4)
  {
    prFuncState->ulSleepTime = 0;
    return (CSMD_WRONG_PHASE);
  }
  
  /* Hot Plug canceled */
  if (boCancel == TRUE)
  {
    prFuncState->ulSleepTime = 0;
    
    if (!CSMD_IsHotplugSupported (prCSMD_Instance))
    {
      return (CSMD_HP_NOT_SUPPORTED);
    }
    
    /* Function already in state cancel */
    if (prFuncState->usActState < CSMD_FUNCTION_STEP_13) 
    {
      if (prHotPlug->usHP_Phase != CSMD_HP_PHASE_2)
      {
        return (CSMD_HP_WRONG_PHASE);
      }
      
      /* CP3 transition check procedure command was not yet executed */
      prFuncState->usActState = CSMD_FUNCTION_STEP_16;
      
      prHotPlug->usHP_Phase = CSMD_NON_HP_MODE;
#ifdef CSMD_DEBUG_HP
      printf ("\nC2C: Hot-Plug phase 2 parameter transmission aborted in Step %d", prFuncState->usActState);
#endif
      prHotPlug->eHP_FuncRet = CSMD_HP_OPERATION_ABORTED;
    }
  }
  
  
  switch (prFuncState->usActState)
  {
  case CSMD_FUNCTION_1ST_ENTRY:
    
    prFuncState->ulSleepTime = 0;
    
    if (!CSMD_IsHotplugSupported (prCSMD_Instance))
    {
      eFuncRet = CSMD_HP_NOT_SUPPORTED;
      break;
    }
    
    /* --------------------------------------------------------- */
    /* Check whether hot plug phase 2 is reached                 */
    /* --------------------------------------------------------- */
    if (prHotPlug->usHP_Phase != CSMD_HP_PHASE_2)
    {
      eFuncRet = CSMD_HP_WRONG_PHASE;
      break;
    }
    
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
    {
      eFuncRet = CSMD_HP_NOT_WITH_CLOSED_RING;
      break;
    }
    
    /* state machine for internal function call */
    prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
    prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
    
    /* state machine for this function */
    prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
    /*lint -fallthrough */
    
    break;
    
    
  case CSMD_FUNCTION_STEP_1:
    
    prHotPlug->eHP_FuncRet = CSMD_Get_S1000( prCSMD_Instance,
                                             &prCSMD_Instance->rPriv.rInternalFuncState,
                                             parSvcMacro );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_2:
    
    prHotPlug->eHP_FuncRet = CSMD_Check_Active_SCP_Classes( prCSMD_Instance,
                                                            &prCSMD_Instance->rPriv.rInternalFuncState,
                                                            parSvcMacro );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* Evaluate SCP type list and build bitlist */
      prHotPlug->eHP_FuncRet = CSMD_Build_SCP_BitList( prCSMD_Instance );
      
      if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
      {
        /* continue with next step */
        /* state machine for internal function call */
        prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
        prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
        /* state machine for this function */
        prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
        prFuncState->usActState  = CSMD_FUNCTION_STEP_3;
      }
      else
      {
        /* Perform end of Hot-Plug because of error condition */
        prFuncState->ulSleepTime = 0;
        prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
      }
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_15;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_3:
    
    prHotPlug->eHP_FuncRet = CSMD_WriteSetup( prCSMD_Instance,
                                              &prCSMD_Instance->rPriv.rInternalFuncState,
                                              parSvcMacro );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_4;
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_4:
    
    prHotPlug->eHP_FuncRet = CSMD_ReadSetup( prCSMD_Instance,
                                             &prCSMD_Instance->rPriv.rInternalFuncState,
                                             parSvcMacro );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_5;
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_5:
    
    prHotPlug->eHP_FuncRet = CSMD_Transmit_SCP_Basic( prCSMD_Instance,
                                                      &prCSMD_Instance->rPriv.rInternalFuncState,
                                                      parSvcMacro );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_6;
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_6:
    
    prHotPlug->eHP_FuncRet = CSMD_Transmit_SCP_VarCFG( prCSMD_Instance,
                                                       &prCSMD_Instance->rPriv.rInternalFuncState,
                                                       parSvcMacro );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* Measure ring delay */
      prCSMD_Instance->rPriv.rRingDelay.usCount1 = 0;
      prCSMD_Instance->rPriv.rRingDelay.usCount2 = 0;
      prCSMD_Instance->rPriv.rRingDelay.ulSumRD1 = 0;
      prCSMD_Instance->rPriv.rRingDelay.ulSumRD2 = 0;
#ifdef CSMD_DEBUG
      for (usI = 0; usI < CSMD_NBR_OF_RD_MEASUREMENTS; usI++)
      {
        prCSMD_Instance->rPriv.rRingDelay.ulBuff1[usI] = 0;
        prCSMD_Instance->rPriv.rRingDelay.ulBuff2[usI] = 0;
      }
#endif
      prCSMD_Instance->rPriv.rRingDelay.ulMinDelay1 = 0xFFFFFFFF;
      prCSMD_Instance->rPriv.rRingDelay.ulMaxDelay1 = 0;
      prCSMD_Instance->rPriv.rRingDelay.ulMinDelay2 = 0xFFFFFFFF;
      prCSMD_Instance->rPriv.rRingDelay.ulMaxDelay2 = 0;

      /* continue with next step */
      /* state machine for this function */
      prFuncState->ulSleepTime =
        prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 / 1000000U;
      if (prFuncState->ulSleepTime == 0)
      {
        prFuncState->ulSleepTime = 1U;
      }
      prFuncState->usActState  = CSMD_FUNCTION_STEP_7;
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_7:
    
    if (   (prCSMD_Instance->rPriv.rRingDelay.usCount1 < CSMD_NBR_OF_RD_MEASUREMENTS)
        && (prCSMD_Instance->rPriv.rRingDelay.usCount2 < CSMD_NBR_OF_RD_MEASUREMENTS) )
    {
      /* Measure ringdelay on port 1 and port 2 */
      CSMD_MeasureRingdelay( prCSMD_Instance );
    }
    else
    {
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
        CSMD_RING_DELAY *prRingDelay = &prCSMD_Instance->rPriv.rRingDelay;

        if (prRingDelay->usCount1)
        {
          prRingDelay->ulAverageRD_P1 = prRingDelay->ulSumRD1 / prRingDelay->usCount1;  /* Average value of tNetwork port 1 */
        }
        if (prRingDelay->usCount2)
        {
          prRingDelay->ulAverageRD_P2 = prRingDelay->ulSumRD2 / prRingDelay->usCount2;  /* Average value of tNetwork port 2 */
        }
        /* Save values of valid measuring of tNetwork */
        prRingDelay->rValid.ulTNetwork_P1 = prRingDelay->ulAverageRD_P1;       /* Average value of tNetwork port 1   */
        prRingDelay->rValid.ulTNetwork_P2 = prRingDelay->ulAverageRD_P2;       /* Average value of tNetwork port 2   */
        prRingDelay->rValid.usTopology    = prCSMD_Instance->usCSMD_Topology;  /* Topology during tNetwork measuring */
        prRingDelay->rValid.ulMaxTNetwork = prRingDelay->ulMaxDelay1 + prRingDelay->ulMaxDelay2;  /* Sum of tNetwork_max form both ports*/
#endif
      /* Determination of S-0-1015 "Ring delay" for the new line */
      prHotPlug->eHP_FuncRet = CSMD_Determination_Ringdelay( prCSMD_Instance,
                                                             CSMD_CALC_RD_MODE_HOTPLUG_ACTIVE,
                                                             &prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015,
                                                             &prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015_P2 );
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
      if (prCSMD_Instance->rPriv.rHotPlug.eHP_FuncRet != CSMD_NO_ERROR)
      {
        /* Perform end of Hot-Plug because of error condition */
        prFuncState->ulSleepTime = 0;
        prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
      }
      else
#endif
      {
        /* continue with next step */
        /* state machine for internal function call */
        prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
        prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;

        /* state machine for this function */
        prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
        prFuncState->usActState  = CSMD_FUNCTION_STEP_8;
      }
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_8:
    
    prHotPlug->eHP_FuncRet = CSMD_Transmit_SCP_Sync( prCSMD_Instance,
                                                     &prCSMD_Instance->rPriv.rInternalFuncState,
                                                     parSvcMacro );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_9;
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_9:
    
    prHotPlug->eHP_FuncRet = CSMD_Transmit_SCP_NRT( prCSMD_Instance,
                                                    &prCSMD_Instance->rPriv.rInternalFuncState,
                                                    parSvcMacro );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
#ifndef CSMD_DISABLE_CMD_S_0_1048
      /* UC channel configured? */
      if (prCSMD_Instance->rPriv.ulUCC_Width)
      {
        prFuncState->usActState  = CSMD_FUNCTION_STEP_10;
      }
      else
#endif
      {
        prFuncState->usActState  = CSMD_FUNCTION_STEP_11;
      }
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
    
    
#ifndef CSMD_DISABLE_CMD_S_0_1048
  case CSMD_FUNCTION_STEP_10:
    
    prHotPlug->eHP_FuncRet = CSMD_Proceed_Cmd_S_0_1048( prCSMD_Instance,
                                                        &prCSMD_Instance->rPriv.rInternalFuncState,
                                                        parSvcMacro );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_11;
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
#endif
    
    
  case CSMD_FUNCTION_STEP_11:
    
    prHotPlug->eHP_FuncRet = CSMD_Transmit_SCP_RTB( prCSMD_Instance,
                                                    &prCSMD_Instance->rPriv.rInternalFuncState,
                                                    parSvcMacro );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      prCSMD_Instance->rPriv.eRequiredState = CSMD_SLAVE_HP_IN_PROCESS;
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_12;
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_12:
    
    prHotPlug->eHP_FuncRet = CSMD_Proceed_Cmd_S_0_1024( prCSMD_Instance,
                                                        &prCSMD_Instance->rPriv.rInternalFuncState,
                                                        parSvcMacro );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_13;
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_13:
    
    prHotPlug->eHP_FuncRet = CSMD_CP_TransitionCheck( prCSMD_Instance,
                                                      &prCSMD_Instance->rPriv.rInternalFuncState,
                                                      parSvcMacro,
                                                      CSMD_IDN_S_0_0127 );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_CP4_TRANS_CHECK;
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
    
    
  case CSMD_FUNCTION_CP4_TRANS_CHECK:
    
    prHotPlug->eHP_FuncRet = CSMD_CP_TransitionCheck( prCSMD_Instance,
                                                      &prCSMD_Instance->rPriv.rInternalFuncState,
                                                      parSvcMacro,
                                                      CSMD_IDN_S_0_0128 );
    
    if (prHotPlug->eHP_FuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (prHotPlug->eHP_FuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      for (usI = 0; usI < prHotPlug->usHP_NbrSlaves; usI++)
      {
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[prHotPlug->ausHP_SlaveAddList[usI + 2]];
        prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] = CSMD_SLAVE_ACTIVE;
      }
      
      /* state machine for this function */
      prHotPlug->usHP_Timeout  = 0;
      prFuncState->ulSleepTime = CSMD_WAIT_4MS;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_15;
    }
    else 
    {
      /* Perform end of Hot-Plug because of error condition */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_15:
    /* --------------------------------------------------------- */
    /* Hot-Plug successfully completed                           */
    /* --------------------------------------------------------- */
    {
      CSMD_USHORT usSAdd;
      CSMD_USHORT usValidCnt = 0;

      /* Check, if all slaves of the HP device have set the S-DEV.Valid bit */
      for (usI = 0; usI < prHotPlug->usHP_NbrSlaves; usI++)
      {
        usSAdd = prHotPlug->ausHP_SlaveAddList[usI+2];
        if (   (usSAdd >= CSMD_MIN_SLAVE_ADD)
            && (usSAdd <= CSMD_MAX_SLAVE_ADD))
        {
          usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[ usSAdd ];
          if (prCSMD_Instance->arDevStatus[usSlaveIdx].usS_Dev & CSMD_S_DEV_SLAVE_VALID)
          {
            usValidCnt++;
          }
        }
      }
      prHotPlug->usHP_Timeout += CSMD_WAIT_4MS;

      if (prHotPlug->usHP_Timeout > CSMD_WAIT_200MS)
      {
        /* Timeout: Not all slaves have set the S-DEV.Valid */
        prHotPlug->eHP_FuncRet = CSMD_HP_SWITCH_TO_CP4_FAILED;

        /* Perform end of Hot-Plug because of error condition */
        prFuncState->usActState  = CSMD_FUNCTION_STEP_16;
      }
      else if (usValidCnt == prHotPlug->usHP_NbrSlaves)
      {
        /* Set coding of HP0 parameter to "no parameter" and disable Hot-Plug */
        prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED
                                                                              | CSMD_HP_CODE_NO_DATA );
        prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED
                                                                              | CSMD_HP_CODE_NO_DATA );
        prFuncState->usActState = CSMD_FUNCTION_FINISHED;
        eFuncRet = CSMD_NO_ERROR;
      }
      /* else check S-DEV.Valid again */
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_16:
    /* --------------------------------------------------------- */
    /* Perform end of Hot Plug because of error condition.       */
    /* eHP_FuncRet must be set with corresponding                */
    /* function error code!                                      */
    /* --------------------------------------------------------- */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] ==
          (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
        if ((CSMD_SHORT)usI >= CSMD_MAX_HW_CONTAINER)
        {
          if (prCSMD_Instance->rPriv.parSoftSvc[usI - CSMD_MAX_HW_CONTAINER].usInUse == CSMD_SVC_CONTAINER_IN_USE)
          {
            parSvcMacro[usI].usCancelActTrans = 1U;
            (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
          }
        }
        else
#endif
        {
          parSvcMacro[usI].usCancelActTrans = 1U;
          (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
        }
      }
    }
    
    if (prHotPlug->usHP_IdxLastSlave == 0xFFFF)
    {
      /* Hot-Plug at inactive master port */
      if (prHotPlug->prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
      {
        /* Set topology to "Single line on port 2 " */
        CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P2_MODE);
      }
      else
      {
        /* Set topology to "Single line on port 1 " */
        CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P1_MODE);
      }
    }
    else  /* if (prHotPlug->usHP_IdxLastSlave == 0xFFFF) */
    {
      /* status of last slave in line */
      usS_Dev = prCSMD_Instance->arDevStatus[ prHotPlug->usHP_IdxLastSlave ].usS_Dev;
      usS_Dev &= CSMD_S_DEV_TOPOLOGY_STS_MASK;
      
      if (usS_Dev == CSMD_S_DEV_CURRENT_LB_FW_P_TEL)
      {
        prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] = (CSMD_USHORT)
          (  (prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] & ~CSMD_C_DEV_CMD_TOPOLOGY_MASK)
           | CSMD_C_DEV_CMD_LOOPB_FW_P_TEL);
      }
      else if (usS_Dev == CSMD_S_DEV_CURRENT_LB_FW_S_TEL)
      {
        prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] = (CSMD_USHORT)
          (  (prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] & ~CSMD_C_DEV_CMD_TOPOLOGY_MASK)
           | CSMD_C_DEV_CMD_LOOPB_FW_S_TEL);
      }
      else    /* Fast-Forward on both ports */
      {
        /* Is Hot-Plug realized on port 1? */
        if (prHotPlug->prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
        {
          prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] = (CSMD_USHORT)
            (  (prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] & ~CSMD_C_DEV_CMD_TOPOLOGY_MASK)
             | CSMD_C_DEV_CMD_LOOPB_FW_P_TEL);
        }
        else
        {
          prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] = (CSMD_USHORT)
            (  (prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] & ~CSMD_C_DEV_CMD_TOPOLOGY_MASK)
             | CSMD_C_DEV_CMD_LOOPB_FW_S_TEL);
        }
      }
      /* Toggle topology change command bit */
      prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] ^= CSMD_C_DEV_TOPOLOGY_HS;
      
    } /* if (prHotPlug->usHP_IdxLastSlave == 0xFFFF) */
    
    /* set activity status for hot plug slaves to inactive */
    for (usI = 0; usI < prHotPlug->usHP_NbrSlaves; usI++)
    {
      usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[prHotPlug->ausHP_SlaveAddList[usI + 2]];
      prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] = CSMD_SLAVE_INACTIVE;
    }
    
    prHotPlug->usHP_Timeout = 0;
    prFuncState->ulSleepTime =
      prCSMD_Instance->rPriv.ulActiveCycTime / 1000000U;
    if (prFuncState->ulSleepTime == 0)
    {
      prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    }
    prFuncState->usActState  = CSMD_FUNCTION_STEP_17;
    break;
    
    
  case CSMD_FUNCTION_STEP_17:
    
    if (prHotPlug->usHP_IdxLastSlave != 0xFFFF)
    {
      if (prHotPlug->usHP_Timeout++ < 10)
      {
        if (   (prCSMD_Instance->rPriv.ausDevControl[ prHotPlug->usHP_IdxLastSlave ] &  CSMD_C_DEV_TOPOLOGY_HS)
            != (prCSMD_Instance->arDevStatus[ prHotPlug->usHP_IdxLastSlave ].usS_Dev &  CSMD_S_DEV_TOPOLOGY_HS) )
        {
          /* Handshake bits in C-DEV and S-DEV of last slave in line not equal */
          break;
        }
      }
      else
      {
        /* Switch to loopback & forward at the last slave in line is failed. */
        prHotPlug->eHP_FuncRet = CSMD_HP_LAST_SLAVE_LB_RESTORE_TIMEOUT;
      }
    }
    
    /* Perform end of Hot-Plug on both ports */
    /* Set coding of HP0 parameter to "no parameter" and disable Hot-Plug */
    prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED 
                                                                          | CSMD_HP_CODE_NO_DATA );
    prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0.rControl.usWord = (  CSMD_HP_CNTRL_SUPPORTED 
                                                                          | CSMD_HP_CODE_NO_DATA );
    eFuncRet = prHotPlug->eHP_FuncRet;
    break;
    
    
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

    /* set activity status for hot plug slaves to inactive */
    for (usI = 0; usI < prHotPlug->usHP_NbrSlaves; usI++)
    {
      usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList
          [prHotPlug->ausHP_SlaveAddList[usI + 2]];
      prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] = CSMD_SLAVE_INACTIVE;
    }

    /* CoSeMa internal error */
    CSMD_RuntimeError( prCSMD_Instance, 0,
                       "CSMD_TransHP2Para(): Illegal case in switch statement" );
    eFuncRet = CSMD_ILLEGAL_CASE;
    break;
    
  }   /* End: switch (prFuncState->usActState) */
  
  return  (eFuncRet);
  
} /* End: CSMD_TransHP2Para() */

/*! \endcond */ /* PUBLIC */



/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */


/**************************************************************************/ /**
\brief Reads information whether Hot-Plug is supported by the master

\ingroup module_hotplug
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       TRUE if master support Hot-Plug, otherwise FALSE

\author       WK
\date         13.03.2008
    
***************************************************************************** */
CSMD_BOOL CSMD_IsHotplugSupported( const CSMD_INSTANCE *prCSMD_Instance )
{
  return (prCSMD_Instance->rPriv.rHW_Init_Struct.boHotPlug == TRUE);
}



/**************************************************************************/ /**
\brief Check for Sercos link on inactive Port.

\ingroup module_hotplug
\b Description: \n
   This function checks whether Sercos telegrams are received on inactive Port.
   The check is activated after the first transferring all HP0 parameters.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       TRUE  Sercos link on inactive port
              FALSE No Sercos link recognized

\author       WK
\date         30.05.2016

*****************************************************************************/
CSMD_BOOL CSMD_HP_Is_Sercos_Link( CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_HOT_PLUG_AUX *prHotPlug  = &prCSMD_Instance->rPriv.rHotPlug;
  CSMD_BOOL          boFinished = FALSE;
  CSMD_USHORT        usS_Dev;

  if (TRUE == prHotPlug->boHP0_CheckLink)
  {
    /* Hot-plug at master port */
    if (prHotPlug->usHP_IdxLastSlave == 0xFFFF)
    {
      /* Status of inactive master port */
      if (prHotPlug->prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
      {
        /* Check for Sercos telegram at inactive master port 1 */
        if (   (TRUE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP1)
            && (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP1) )
        {
          /* Sercos P-telegram at inactive master port (P-Channel) */
          boFinished = TRUE;
        }
      }
      else
      {
        /* Check for Sercos telegram at inactive master port 2 */
        if (   (TRUE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP2)
            && (TRUE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP2) )
        {
          /* Sercos S-telegram at inactive master port (S-Channel) */
          boFinished = TRUE;
        }
      }
    }
    else  /* if (prHotPlug->usHP_IdxLastSlave == 0xFFFF) */
    {
      /* status of last slave in line */
      usS_Dev = prCSMD_Instance->arDevStatus[ prHotPlug->usHP_IdxLastSlave ].usS_Dev;
      usS_Dev &= CSMD_S_DEV_STAT_INACT_MASK;

      /* Check for Sercos telegram at last slave on inactive port */
      if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
      {
        if (usS_Dev == CSMD_S_DEV_STAT_INACT_P_TEL)
        {
          /* Sercos P-telegram at last slave on inactive port on P-Channel */
          boFinished = TRUE;
        }
      }
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
      {
        if (usS_Dev == CSMD_S_DEV_STAT_INACT_S_TEL)
        {
          /* Sercos S-telegram at last slave on inactive port on S-Channel */
          boFinished = TRUE;
        }
      }
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
      {
        if (prHotPlug->prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
        {
          if (usS_Dev == CSMD_S_DEV_STAT_INACT_P_TEL)
          {
            /* Sercos P-telegram at last slave on inactive port on P-Channel */
            boFinished = TRUE;
          }
        }
        else
        {
          if (usS_Dev == CSMD_S_DEV_STAT_INACT_S_TEL)
          {
            /* Sercos S-telegram at last slave on inactive port on S-Channel */
            boFinished = TRUE;
          }
        }
      }
    } /* if (prHotPlug->usHP_IdxLastSlave == 0xFFFF) */
  } /* if (TRUE == prHotPlug->boHP0_CheckLink) */

  return (boFinished);

} /* End: CSMD_HP_Is_Sercos_Link() */



/**************************************************************************/ /**
\brief Sends the HP0 parameters.

\ingroup module_hotplug
\b Description: \n
   This function transmit the HP0 parameters at the selected port.
   A transmission cycle consists of the transfer of all HP0 parameters.
   Each HP0 parameter is transmitted for 2 Sercos cycles.
   After the first transmission cycle, the check for Sercos link on
   inactive Sercos port is enabled.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       TRUE  Sercos link on inactive port
              FALSE No Sercos link recognized

\author       WK
\date         30.05.2016

*****************************************************************************/
CSMD_VOID CSMD_Send_HP0_Parameter( CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_HOT_PLUG_AUX *prHotPlug = &prCSMD_Instance->rPriv.rHotPlug;

  switch (prHotPlug->usHP0_ParamCnt)
  {
    case 0:
      /* ----------------------------------------------- */
      /* S-0-1002, Communication Cycle time (tScyc)      */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_TS_CYC);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002;

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 1:
      /* ----------------------------------------------- */
      /* S-0-1010, Lengths of MDTs (MDT0)                */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_MDT0_LENGTH);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[0];

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 2:
      /* ----------------------------------------------- */
      /* S-0-1010, Lengths of MDTs (MDT1)                */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_MDT1_LENGTH);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[1];

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 3:
      /* ----------------------------------------------- */
      /* S-0-1010, Lengths of MDTs (MDT2)                */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_MDT2_LENGTH);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[2];

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 4:
      /* ----------------------------------------------- */
      /* S-0-1010, Lengths of MDTs (MDT3)                */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_MDT3_LENGTH);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[3];

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 5:
      /* ----------------------------------------------- */
      /* S-0-1012, Lengths of ATs (AT0)                  */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_AT0_LENGTH);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[0];

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 6:
      /* ----------------------------------------------- */
      /* S-0-1012, Lengths of ATs (AT1)                  */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_AT1_LENGTH);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[1];

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 7:
      /* ----------------------------------------------- */
      /* S-0-1012, Lengths of ATs (AT2)                  */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_AT2_LENGTH);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[2];

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 8:
      /* ----------------------------------------------- */
      /* S-0-1012, Lengths of ATs (AT3)                  */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_AT3_LENGTH);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[3];

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 9:
      /* ----------------------------------------------- */
      /* S-0-1017[0], Begin of the UC channel (t6)       */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_T6);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017;

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 10:
      /* ----------------------------------------------- */
      /* S-0-1017[1], End of the UC channel (t7)         */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_T7);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017;

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 11:
      /* ----------------------------------------------- */
      /* S-0-1027.0.1 Requested MTU                      */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_REQ_MTU);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rPriv.usRequested_MTU;

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    case 12:
      /* ----------------------------------------------- */
      /* Communication Version Field   (for MDT0 in CP0) */
      /* ----------------------------------------------- */
      if (prHotPlug->usHP0_RepeatCnt == 0)
      {
        prHotPlug->prHP_ActPort->rHpField_MDT0.rControl.usWord =
          (CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CNTRL_ENABLED | CSMD_HP_CODE_COMM_VERSION);

        prHotPlug->prHP_ActPort->rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rPriv.rComVersion.ulLong;

        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else if (prHotPlug->usHP0_RepeatCnt <= prHotPlug->usHP0_RepeatRate)
      {
        prHotPlug->usHP0_RepeatCnt++;
        break;
      }
      else
      {
        prHotPlug->usHP0_RepeatCnt = 0;
        prHotPlug->usHP0_ParamCnt++;
      }
      /*lint -fallthrough */

    default:
      /* ------------------------------------------------- */
      /* All HP0 parameters transmitted. Check sercos link */
      /* ------------------------------------------------- */
      /* One set of HP0 parameters transfered */
      prHotPlug->usHP_Timeout++;
      /* Transmit all HP0 parameters once more */
      prHotPlug->usHP0_ParamCnt = 0;
      /* Activate check for Sercos link on inactive port */
      prHotPlug->boHP0_CheckLink = TRUE;
      break;

  }   /* End of: switch (prCSMD_Instance->rPriv.usHP0_ParamCnt) */

} /* End: CSMD_Send_HP0_Parameter() */


#endif  /* End: #ifdef CSMD_HOTPLUG */

/*! \endcond */ /* PRIVATE */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
20 Nov 2013 WK
  - Defdb00165158
    CSMD_HotPlug()
    The HP0 parameter "Requested MTU" was not transmitted for not defined
    CSMD_NRT_CHANNEL.
  - Defdb00165280
    CSMD_HotPlug(), CSMD_HotUnPlug()
    Fixed access to IP-Core SVC channel for Big Endian systems.
09 Dec 2013 AlM
  - Defdb00164530
    CSMD_HotUnplug()
    Skip function for ring or defect ring topology.
09 Jan 2014 WK
  - Defdb00165281
    CSMD_TransHP2Para()
    Check, if all slaves of the HP device have set the S-DEV.Valid
    before the C-HP.Enable has been cleared.
18 Mar 2014 WK
  - Defdb00144624
    CSMD_HotPlug()
    The HP phase 0 parameter block was transmitted only one time,
    if hot plug was initiated at the end of a line.
02 Sep 2014 Wk
  - Defdb00173984
    CSMD_HotPlug()
    HP1: Check, if found slaves are projected doesn't work.
09 Oct 2014 WK
  Defdb00174053
  - CSMD_HotPlug()
    - HP0: Wait 1 cycle after last parameter before check finishing
           HP0 parameter transmission.
    - End of HP0: Additionally to the Topology-HS, compare
                  C-DEV.Topology_control with S-DEV.Topology_status.
    - HP1: Added check for double addresses in scanned HP slave address list.
           (error code CSMD_HP_DOUBLE_SLAVE_ADDRESSES)
    - HP1: Added check for valid slave address.
           (error code CSMD_HP_ILLEGAL_SLAVE_ADDRESS)
    - HP1: Change timeout for parameter transmission from 10 Sercos cycles
           to SVC busy timeout.
    - HP1: Set the Slave_index in MDT-HP.Selection during parameter
           transmission for the current slave in a Multi-slave-device
           according to Sercos Spec. 1.3.2.
    - End of HP1: Set C-HP.HP_coding to "no HP parameter".
                  Set MDT-HP.Info to 0.
                  Set MDT-HP.Selection to 0.
    - End of HP1: Change timeout for acknowledge of SVC activation
                  from 40 Sercos cycles to SVC busy timeout.
    - HP2: C-DEV.Master-valid has not been set.
    - Set usHP_Phase = CSMD_HP_PHASE_1 at begin scanning slaves.
  - CSMD_TransHP2Para()
    Rename case CSMD_FUNCTION_STEP_14 to CSMD_FUNCTION_CP4_TRANS_CHECK.
23 Oct 2014 WK
  - Defdb00173986
    CSMD_HotPlug()  CSMD_FUNCTION_STEP_1
    - With broken ring topology:
      Hot-plug at line port 2 was not activated, if a link
      at end of line port 1 is detected.
    - With topology line port 1:
      Hot-plug at master port 2 was not activated, if a link
      at end of line port 1 is detected.
    - After Port1.C-HP.Enable=0 wait 10 Sercos cycles, before setting
      Port2.C-HP.Enable=1.
12 Nov 2014 WK
  - Defdb00174745
    CSMD_HotPlug()
    The fist access to the SVC after successful completion of this
    function results with CSMD_HANDSHAKE_TIMEOUT, if hot-plug is
    executed on port 2 and redundancy for hardware SVC is activated.
11 Feb 2015 WK
  - Defdb00150926
    Fixed brackets in expression in line no. 1411 to prevent
    warnings with some compilers.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
03 Mar 2015 WK
  - Defdb00179554
    CSMD_TransHP2Para()
    Ring delay calculation independent from topology at HP start-up.
06 Mar 2015 WK
  - Defdb00177334
    CSMD_TransHP2Para(), CSMD_FUNCTION_STEP_16
    Possible locked SVC after active canceling the function.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
01 Oct 2015 WK
  - Defdb00177334
    CSMD_TransHP2Para(), CSMD_FUNCTION_STEP_16
    Active SVC transfers with hardware SVC may not have been canceled.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
18 Apr 2016 WK
  - Defdb00182067
    CSMD_TransHP2Para()
    Fixed possible Lint warning 661.
31 May 2016 WK
  - Defdb00187475
    HP0: Shifted transmission of HP0 parameters in the function
         CSMD_Send_HP0_Parameter().
    HP0: Check for Sercos link in each Sercos cycle. The activation takes
         place after the first HP0 transmission cycle.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
 11 Jul 2016 WK
 - Defdb00188354
   CSMD_TransHP2Para()
   Fixed division by zero during tNetwork calculation.
  
------------------------------------------------------------------------------
*/
