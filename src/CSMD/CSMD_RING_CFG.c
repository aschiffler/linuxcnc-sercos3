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

\file   CSMD_RING_CFG.c
\author WK
\date   01.09.2010
\brief  This File contains the public API functions 
        which handles the ring topology.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
#include "CSMD_CALC.h"
#endif
#include "CSMD_CP_AUX.h"
#include "CSMD_DIAG.h"
#include "CSMD_PHASE.h"
#include "CSMD_PRIV_SVC.h"
#include "CSMD_TOPOLOGY.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */


/* ------------------------------------------------------------------

      |-----------------------------------------------------------|
      | Recognized telegram on | Recognized topology              |
      |   port 1   |  port 2   |                                  |
      |------------|-----------|----------------------------------|
      |     P      |     -     | Line on port 1                   |
      |     -      |     S     | Line in port 2                   |
      |     P      |     S     | Broken ring                      |
      |            |           |                                  |
      |     S      |     P     | Ring                             |
      |     S      |     S     | Ring  (P channel defective)      |
      |     P      |     P     | Ring  (S channel defective)      |
      |     -      |     -     | Ring not closed                  |
      |------------|-----------|----------------------------------|

   ------------------------------------------------------------------ */


/**************************************************************************/ /**
\brief Initiates the recovery of the ring.

\ingroup module_redundancy
\b Description: \n
   This function turns any topology into a ring topology if the physical link
   is established properly. For this purpose, the topology must be physically closed
   to form a ring. Due to the topology change, the ring delay time must be newly
   measured. If the delay time is higher that the previous one, the delay time
   is, it is transmitted to the slaves and the command S-0-1024 is executed.

<B>Call Environment:</B> \n
   When this function is called, the current topology must not be a ring.
   If the CSMD_RecoverRingTopology() function is not to complete the transmission of
   the parameter S-0-1015 Ring delay and the command S-0-1024 automatically, the
   function can be terminated prematurely by querying the state
   CSMD_FUNCTION_NEW_RINGDELAY.\n
   The call-up should be performed from a task.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures
\param [in]   prFuncState
              Pointer to management data for this function

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_WRONG_TOPOLOGY \n
              \ref CSMD_RECOVER_RING_ERROR \n
              \ref CSMD_RING_RECOVERY_ABORTED \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_RECOVER_RING_OK \n

\author       KP
\date         06.02.2008 
 
***************************************************************************** */
CSMD_FUNC_RET CSMD_RecoverRingTopology( CSMD_INSTANCE          *prCSMD_Instance,
                                        CSMD_FUNC_STATE        *prFuncState,
                                        CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_USHORT usI;
  CSMD_USHORT usFinished;
  CSMD_USHORT usTempC_DevWord;
  CSMD_USHORT usTempS_DevWord;
  CSMD_USHORT usBuf;
  CSMD_USHORT usSlaveIdx_P1 = 0;  /* index of last slave on port 1 */
  CSMD_USHORT usSlaveIdx_P2 = 0;  /* index of last slave on port 2 */
  
  /* flags which signalize that the topology HS of concerned slaves has been processed yet */
  CSMD_BOOL   boFF_ReadyP1 = FALSE;
  CSMD_BOOL   boFF_ReadyP2 = FALSE;
  
  CSMD_FUNC_RET    eFuncRet = CSMD_FUNCTION_IN_PROCESS;
  CSMD_RING_DELAY *prRingDelay = &prCSMD_Instance->rPriv.rRingDelay;
  
  /* usSlaveIdx_P1 must not be used with defect ring on primary line topology */
  if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect != CSMD_RING_DEF_PRIMARY)
  {
    usSlaveIdx_P1 = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[prCSMD_Instance->usSercAddrLastSlaveP1];
  }
  /* usSlaveIdx_P2 must not be used with defect ring on secondary line topology */
  if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect != CSMD_RING_DEF_SECONDARY)
  {
    usSlaveIdx_P2 = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[prCSMD_Instance->usSercAddrLastSlaveP2];
  }

  switch (prFuncState->usActState)
  {
  /* --------------------------------------------------------- */
  /* Step Entry : Initialize recover ring                      */
  /* --------------------------------------------------------- */
  case CSMD_FUNCTION_1ST_ENTRY:
    
    /* Wait 1 Sercos cycle */
    prFuncState->ulSleepTime = 
      prCSMD_Instance->rPriv.ulActiveCycTime / 1000000U;
    if (prFuncState->ulSleepTime == 0)
    {
      prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    }
    
    if (prCSMD_Instance->sCSMD_Phase < CSMD_SERC_PHASE_1)
    {
      eFuncRet = CSMD_WRONG_PHASE;
    }
    else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
    {
      eFuncRet = CSMD_WRONG_TOPOLOGY;
    }
    prCSMD_Instance->rPriv.lStableTimer = CSMD_NBR_STABLE_TOPOLOGY;   /* The topology shall be stable for 100 Sercos cycles */
    prFuncState->usActState     = CSMD_FUNCTION_STEP_1;
    break;
    
    
  /* ---------------------------------------------------------------------*/
  /* Step 1 : Check S-DEV of last slaves for stable topology status       */
  /* ---------------------------------------------------------------------*/
  case CSMD_FUNCTION_STEP_1:
    
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING)
    {
      if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_SECONDARY)
      {
        usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1];
        /* Device status of slave at the defect point */
        usTempS_DevWord = CSMD_END_CONV_S( *prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx_P1][CSMD_PORT_1][usBuf] );
        
        if (!(   (usTempS_DevWord & (CSMD_S_DEV_TOPOLOGY_STS_MASK | CSMD_S_DEV_STAT_INACT_MASK))
              == (CSMD_S_DEV_CURRENT_LB_FW_P_TEL | CSMD_S_DEV_STAT_INACT_S_TEL) ) )
        {
          eFuncRet = CSMD_RECOVER_RING_ERROR; /* no S-Telegram on inactive port of last slave connected to port 1 */
        }
      }
      else /* RingDefect == CSMD_RING_DEF_PRIMARY */
      {
        usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2];
        /* Device status of slave at the defect point */
        usTempS_DevWord = CSMD_END_CONV_S( *prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx_P2][CSMD_PORT_2][usBuf]);
        
        if (!(   (usTempS_DevWord & (CSMD_S_DEV_TOPOLOGY_STS_MASK | CSMD_S_DEV_STAT_INACT_MASK))
              == (CSMD_S_DEV_CURRENT_LB_FW_S_TEL | CSMD_S_DEV_STAT_INACT_P_TEL) ) )
        {
          eFuncRet = CSMD_RECOVER_RING_ERROR; /* no P-Telegram on inactive port of last slave connected to port 2 */
        }
      }
    }
    else
    {
      if (prCSMD_Instance->rPriv.boP1_active)
      {
        usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1];
        /* Device status of last slave on port 1 */
        usTempS_DevWord = CSMD_END_CONV_S(
          *prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx_P1][CSMD_PORT_1][usBuf] );
        
        /* primary telegram on Port 1 */
        if (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP1)
        {
          /* last slave on Port 1 is Loopback & Forward P-Tel */
          if ( !( (usTempS_DevWord & CSMD_S_DEV_STAT_INACT_MASK) == CSMD_S_DEV_STAT_INACT_S_TEL) )
          {
            eFuncRet = CSMD_RECOVER_RING_ERROR; /* no S-Telegram on inactive port of last slave connected to port 1 */
          }
        }
        else
        {
          eFuncRet = CSMD_RECOVER_RING_ERROR; /* no P-Telegram received on master port 1 */
        }
      }
      
      if (prCSMD_Instance->rPriv.boP2_active)
      {
        usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2];
        /* Device status of last slave on port 2 */
        usTempS_DevWord = CSMD_END_CONV_S(
          *prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx_P2][CSMD_PORT_2][usBuf] );
        
        /* secondary telegram on Port 2 ? */
        if (TRUE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP2)
        {
          /* last slave on Port 2 is LoopBack & Forward S-Tel */
          if (! ( (usTempS_DevWord & CSMD_S_DEV_STAT_INACT_MASK) == CSMD_S_DEV_STAT_INACT_P_TEL)  )
          {
            eFuncRet = CSMD_RECOVER_RING_ERROR; /* no P-Telegram on inactive port of last slave connected to port 2*/
          }
        }
        else
        {
          eFuncRet = CSMD_RECOVER_RING_ERROR; /* no S-Telegram received on master port 2 */
        }
      }
    }
    
    if (eFuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      if (prCSMD_Instance->rPriv.lStableTimer < 0)
      {
        prFuncState->usActState = CSMD_FUNCTION_STEP_2;
      }
      else
      {
        prCSMD_Instance->rPriv.lStableTimer--;
        break;
      }
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
  /* --------------------------------------------------------------------- */
  /* Step 2 : Set C-DEV topology to FF and toggle topology bit             */
  /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_2:
    
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING)
    {
      /* Toggle topology HandShake bit and set topology to FF both ports for slave at the defect point */
      if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_SECONDARY)
      {
        CSMD_SetC_DevTopology( prCSMD_Instance,
                               CSMD_C_DEV_CMD_TOPOLOGY_FF_BOTH,
                               usSlaveIdx_P1 );
      }
      else
      {
        CSMD_SetC_DevTopology( prCSMD_Instance,
                               CSMD_C_DEV_CMD_TOPOLOGY_FF_BOTH,
                               usSlaveIdx_P2 );
      }
    }
    else
    {
      /* if topology is CSMD_TOPOLOGY_LINE_P1 or CSMD_TOPOLOGY_BROKEN_RING then toggle topology HandShake bit
         and set topology to FF for last slave on Port 1 */
      if (prCSMD_Instance->rPriv.boP1_active)
      {
        /* Set last slave topology on P1 to Fast-Forward */
        CSMD_SetC_DevTopology( prCSMD_Instance, 
                               CSMD_C_DEV_CMD_TOPOLOGY_FF_BOTH, 
                               usSlaveIdx_P1 );
      }
      
      /* if topology is CSMD_TOPOLOGY_LINE_P2 or CSMD_TOPOLOGY_BROKEN_RING then toggle topology HandShake bit
         and set FF Topology for last slave on Port 2 */
      if (prCSMD_Instance->rPriv.boP2_active)
      {
        /* Set last slave topology on P2 to Fast-Forward */
        CSMD_SetC_DevTopology( prCSMD_Instance, 
                               CSMD_C_DEV_CMD_TOPOLOGY_FF_BOTH, 
                               usSlaveIdx_P2 );
      }
    }
    
   /* number of cycles to be checked whether the commanded slaves have toggled their topology HS bit */
    prCSMD_Instance->rPriv.lStableTimer = 11;

    prFuncState->ulSleepTime = 
      (CSMD_ULONG) ((12 * prCSMD_Instance->rPriv.ulActiveCycTime)/1000000);
    if (prFuncState->ulSleepTime < 4)
    {
      prFuncState->ulSleepTime = 4;
    }
    prFuncState->usActState = CSMD_FUNCTION_STEP_3;
    break;
    
    
  /* --------------------------------------------------------------------- */
  /* Step 3 : Check if topology toggle bit in S-DEV toggles                */
  /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_3:
    
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING)
    {
      if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_SECONDARY)
      {
        usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1];
        usTempC_DevWord = prCSMD_Instance->rPriv.ausDevControl[usSlaveIdx_P1];

        /* Device status of slave at the defect point */
        usTempS_DevWord = CSMD_END_CONV_S( *prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx_P1][CSMD_PORT_1][usBuf] );
        
        /* check if topology and handshake are the same */
        if (   (usTempS_DevWord & CSMD_MASK_TOPO_TOGGLE) 
            != (usTempC_DevWord & CSMD_MASK_TOPO_TOGGLE))
        {
          prCSMD_Instance->rPriv.lStableTimer--;
          
          /* the commanded slave has not toggled its topology HS bit for too many cycles */
          if (prCSMD_Instance->rPriv.lStableTimer < 0)
          {
            /* Set topology of slave at the defect point to Loopback-Forward P-Telegram */
            CSMD_SetC_DevTopology( prCSMD_Instance,
                                   CSMD_C_DEV_CMD_LOOPB_FW_P_TEL,
                                   usSlaveIdx_P1 );
          
             prFuncState->usActState = CSMD_FUNCTION_STEP_6; /* topology change failed on port 1 */
          }
        }
        else
        {
          prFuncState->ulSleepTime = 
          (CSMD_ULONG) ((4 * prCSMD_Instance->rPriv.ulActiveCycTime)/1000000);
          if (prFuncState->ulSleepTime < 4)
          {
            prFuncState->ulSleepTime = 4;
          }

          prRingDelay->usCount1    = 0;
          prRingDelay->usCount2    = 0;
          prRingDelay->ulSumRD1    = 0;
          prRingDelay->ulSumRD2    = 0;
          prRingDelay->ulMinDelay1 = 0xFFFFFFFF;
          prRingDelay->ulMaxDelay1 = 0;
          prRingDelay->ulMinDelay2 = 0xFFFFFFFF;
          prRingDelay->ulMaxDelay2 = 0;

          prCSMD_Instance->rPriv.lStableTimer = CSMD_NBR_STABLE_TOPOLOGY;  /* The topology shall be stable for 100 Sercos cycles */
          prFuncState->usActState = CSMD_FUNCTION_STEP_4;
        }
      }
      else /* RingDefect == CSMD_RING_DEF_PRIMARY */
      {
        usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2];
        usTempC_DevWord = prCSMD_Instance->rPriv.ausDevControl[usSlaveIdx_P2];

        /* Device status of slave at the defect point */
        usTempS_DevWord = CSMD_END_CONV_S( *prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx_P2][CSMD_PORT_2][usBuf] );
        
        /* check if topology and handshake are the same */
        if (   (usTempS_DevWord & CSMD_MASK_TOPO_TOGGLE) 
            != (usTempC_DevWord & CSMD_MASK_TOPO_TOGGLE))
        {
          prCSMD_Instance->rPriv.lStableTimer--;
          
          /* the commanded slave has not toggled its topology HS bit for 12 cycles */
          if (prCSMD_Instance->rPriv.lStableTimer < 0)
          {
            /* Set topology of slave at the defect point to Loopback-Forward S-Telegram */
            CSMD_SetC_DevTopology( prCSMD_Instance,
                                   CSMD_C_DEV_CMD_LOOPB_FW_S_TEL,
                                   usSlaveIdx_P2 );
            
            prFuncState->usActState = CSMD_FUNCTION_STEP_6; /* topology change failed on port 2 */
          }
        }
        else
        {
          prFuncState->ulSleepTime = 
          (CSMD_ULONG) ((4 * prCSMD_Instance->rPriv.ulActiveCycTime)/1000000);
          if (prFuncState->ulSleepTime < 4)
          {
            prFuncState->ulSleepTime = 4;
          }

          prRingDelay->usCount1    = 0;
          prRingDelay->usCount2    = 0;
          prRingDelay->ulSumRD1    = 0;
          prRingDelay->ulSumRD2    = 0;
          prRingDelay->ulMinDelay1 = 0xFFFFFFFF;
          prRingDelay->ulMaxDelay1 = 0;
          prRingDelay->ulMinDelay2 = 0xFFFFFFFF;
          prRingDelay->ulMaxDelay2 = 0;

          prCSMD_Instance->rPriv.lStableTimer = CSMD_NBR_STABLE_TOPOLOGY;  /* The topology shall be stable for 100 Sercos cycles */
          prFuncState->usActState = CSMD_FUNCTION_STEP_4;
        }
      }
    }
    else  /* if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING) */
    {
      if (prCSMD_Instance->rPriv.boP1_active)
      {
      /* check if topology handshake bit toggle */
        usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1];
        usTempC_DevWord = prCSMD_Instance->rPriv.ausDevControl[usSlaveIdx_P1];
        usTempS_DevWord = CSMD_END_CONV_S(
          *prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx_P1][CSMD_PORT_1][usBuf] );
        
        /* (one of) the commanded slave(s) has not toggled its topology HS bit for too many cycles */
        if (prCSMD_Instance->rPriv.lStableTimer < 0)
        {
          /* topology change failed, set the last slave connected to port 1 to LB */
          CSMD_SetC_DevTopology( prCSMD_Instance, 
                                 CSMD_C_DEV_CMD_LOOPB_FW_P_TEL, 
                                 usSlaveIdx_P1 );
          
          prFuncState->usActState = CSMD_FUNCTION_STEP_6; /* topology change failed on port 1 */
        }
        /* has the topology of last slave connected to port 1 changed to FF? */
        else if (   (usTempS_DevWord & CSMD_MASK_TOPO_TOGGLE) 
                 != (usTempC_DevWord & CSMD_MASK_TOPO_TOGGLE))
        {
          prCSMD_Instance->rPriv.lStableTimer--;
        }
        else
        {
          boFF_ReadyP1 = TRUE;
        }
      }
      else /* do not require any feedback from port 1 if it is not active */
      {
        boFF_ReadyP1 = TRUE;
      }

      if (prCSMD_Instance->rPriv.boP2_active)
      {
        usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2];
        usTempC_DevWord = prCSMD_Instance->rPriv.ausDevControl[usSlaveIdx_P2];
        usTempS_DevWord = CSMD_END_CONV_S(
          *prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx_P2][CSMD_PORT_2][usBuf] );
        
        /* the commanded slave has not toggled its topology HS bit for too many cycles */
        if (prCSMD_Instance->rPriv.lStableTimer < 0)
        {
          /* topology change failed, set the last slave connected to port 2 to LB */
          CSMD_SetC_DevTopology( prCSMD_Instance, 
                                 CSMD_C_DEV_CMD_LOOPB_FW_S_TEL, 
                                 usSlaveIdx_P2 );
          
          prFuncState->usActState = CSMD_FUNCTION_STEP_6; /* topology change failed on port 2 */
        }
        /* has the topology of last slave connected to port 2 changed to FF? */
        else if (   (usTempS_DevWord & CSMD_MASK_TOPO_TOGGLE) 
                 != (usTempC_DevWord & CSMD_MASK_TOPO_TOGGLE))
        {
          prCSMD_Instance->rPriv.lStableTimer--;
        }
        else
        {
          boFF_ReadyP2 = TRUE;
        }
      }
      else /* do not require any feedback from port 2 if it is not active */
      {
        boFF_ReadyP2 = TRUE;
      }
      
      /* all commanded slaves have toggled their topology HS bit */
      if ((boFF_ReadyP1 && boFF_ReadyP2) || (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING))
      {
        prFuncState->ulSleepTime = 
          (CSMD_ULONG) ((4 * prCSMD_Instance->rPriv.ulActiveCycTime)/1000000);
        if (prFuncState->ulSleepTime < 4)
        {
          prFuncState->ulSleepTime = 4;
        }
        
        prRingDelay->usCount1    = 0;
        prRingDelay->usCount2    = 0;
        prRingDelay->ulSumRD1    = 0;
        prRingDelay->ulSumRD2    = 0;
        prRingDelay->ulMinDelay1 = 0xFFFFFFFF;
        prRingDelay->ulMaxDelay1 = 0;
        prRingDelay->ulMinDelay2 = 0xFFFFFFFF;
        prRingDelay->ulMaxDelay2 = 0;
        
        /* Initiate sending of t6/t7 via MDT HP field */
        prCSMD_Instance->rPriv.usTimerHP_CP3 = 0;
        
        prCSMD_Instance->rPriv.lStableTimer = CSMD_NBR_STABLE_TOPOLOGY;  /* The topology shall be stable for 100 Sercos cycles */
        prFuncState->usActState = CSMD_FUNCTION_STEP_4;
      }
    } /* end: if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING) */
    break;
    
  /* --------------------------------------------------------------------- */
  /* Step 4 : Ring recovered, measure ring delay                           */
  /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_4:
    
    /* secondary telegram on port 1 and primary telegram on port 2 */
    if (prCSMD_Instance->usCSMD_Topology != CSMD_TOPOLOGY_RING)
    {
      /* Physical topology has changed again. Abort ring recovery process. */
      prFuncState->usActState = CSMD_FUNCTION_STEP_10;
    }
    else
    {
      /* Measure ring delay on port 1 and port 2 */
      CSMD_MeasureRingdelay( prCSMD_Instance );
      prCSMD_Instance->rPriv.lStableTimer--;
      
      if (prCSMD_Instance->rPriv.lStableTimer < 0)
      {
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
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
        prFuncState->ulSleepTime = 0;
        prFuncState->usActState  = CSMD_FUNCTION_STEP_5;  /* list stable, go to next state */
      }
      else
      {
        /* continue check list, wait one Sercos cycle */
        prFuncState->ulSleepTime = prCSMD_Instance->rPriv.ulActiveCycTime / 1000000U;
        if (prFuncState->ulSleepTime == 0)
        {
          prFuncState->ulSleepTime = 1;
        }
      }
    }
    break;
    
  /* --------------------------------------------------------------------- */
  /* Step 5 : Ring recover, determine S-0-1015 ring delay                  */
  /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_5:
    
    if (prCSMD_Instance->usCSMD_Topology != CSMD_TOPOLOGY_RING)
    {
      /* Physical topology has changed again. Abort ring recovery process. */
      prFuncState->usActState = CSMD_FUNCTION_STEP_10;
    }
    else
    {
#if !defined CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
      /* Determination of S-0-1015 "Ring delay" */
      (CSMD_VOID)CSMD_Determination_Ringdelay( prCSMD_Instance,
                                               CSMD_CALC_RD_MODE_RECOVER_RING_ACTIVE,
                                               &prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015,
                                               &prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015_P2 );
#endif
      if (prCSMD_Instance->sCSMD_Phase >= CSMD_SERC_PHASE_3)
      {
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
        /* Ring delay is not needed in CP1 to CP2 ! */

        /* Determination of S-0-1015 "Ring delay" */
        eFuncRet = CSMD_Determination_Ringdelay( prCSMD_Instance,
                                                 CSMD_CALC_RD_MODE_RECOVER_RING_ACTIVE,
                                                 &prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015,
                                                 &prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015_P2 );
        if (eFuncRet != CSMD_NO_ERROR)
        {
          prFuncState->usActState = CSMD_FUNCTION_STEP_7;
        }
        else
#endif
        {
          prCSMD_Instance->rExtendedDiag.ulIDN = CSMD_IDN_S_0_1015;
          prFuncState->ulSleepTime = 0;
          /* Proceed transfer of S-0-1015 and execution of command S-0-1024 */
          prFuncState->usActState  = CSMD_FUNCTION_NEW_RINGDELAY;

          /* continue with next step */
          for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
          {
            if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == CSMD_SLAVE_ACTIVE)
            {
              parSvcMacro[usI].usState = CSMD_START_REQUEST;
            }
          }
        }
      }
      else /* skip SVC actions if communication phase < CP3 */
      {
        prFuncState->usActState = CSMD_FUNCTION_STEP_9;
      }
    }
    break;
    
  /* --------------------------------------------------------------------- */
  /* Step 6 : Ring recovery failed, old topology was recovered.            */
  /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_6:
    
    prCSMD_Instance->rPriv.lStableTimer = 0;                  /* Set Timer */
    prFuncState->ulSleepTime = 0;
    eFuncRet = CSMD_RECOVER_RING_ERROR;
    break;
    
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
  /* --------------------------------------------------------------------- */
  /* Step 7 : Ring recovery failed, sum of tNetwork > tSyncDelay           */
  /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_7:

    prFuncState->ulSleepTime = 0;
    break;
#endif

  /* --------------------------------------------------------------------- */
  /* Step 7 : Send new ring delay S-1015 to all slaves.                    */
  /* --------------------------------------------------------------------- */
  case CSMD_FUNCTION_NEW_RINGDELAY:
    
    usFinished = 1U;
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == CSMD_SLAVE_ACTIVE)
      {
        if (CSMD_SCP_SYNC & prCSMD_Instance->rPriv.aulSCP_Config[usI]) 
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015;
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
        } /* End: if (CSMD_SCP_SYNC & prCSMD_Instance->rPriv.aulSCP_Config[usI]) */
      }
    }
    
    eFuncRet = CSMD_FUNCTION_IN_PROCESS;
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        eFuncRet = prCSMD_Instance->rExtendedDiag.aeSlaveError[0];
      }
      
      /* secondary telegram on port 1 and primary telegram on port 2 */
      if (prCSMD_Instance->usCSMD_Topology != CSMD_TOPOLOGY_RING)     
      {
        /* Physical topology has changed again. Abort ring recovery process. */
        prFuncState->usActState = CSMD_FUNCTION_STEP_10;
        break;
      }
      
      /* SYNC delay measuring procedure command */
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_8;
      
      prCSMD_Instance->rPriv.eRequiredState = CSMD_SLAVE_ACTIVE;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState = CSMD_FUNCTION_1ST_ENTRY;
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
    }
    else 
    {
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_NEW_RINGDELAY;
      break;
    }
    break;
    
  /* ----------------------------------------------------------------------------------------- */
  /* Step 8 : start of delay measuring with command S-1024 for determine the new SYNCCNT-P/S   */
  /* ----------------------------------------------------------------------------------------- */
  case CSMD_FUNCTION_STEP_8:
    
    /* activation S-0-1024 */
    eFuncRet = CSMD_Proceed_Cmd_S_0_1024( prCSMD_Instance,
                                          &prCSMD_Instance->rPriv.rInternalFuncState,
                                          parSvcMacro );
    
    if (eFuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      break;
    }
    /*lint -fallthrough */
    
  /* ------------------------------------------------------------------------------------------ */
  /* Step 9: successful end of ring recovery, restore all relevant settings                     */
  /* ------------------------------------------------------------------------------------------ */
  case CSMD_FUNCTION_STEP_9:
    
    prFuncState->ulSleepTime = 0;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    
    if (! (eFuncRet > CSMD_FUNCTION_IN_PROCESS) ) /* no error */
    {
      eFuncRet = CSMD_RECOVER_RING_OK;
    }
    break;
    
  /* ------------------------------------------------------------------------------------------ */
  /* Step 10 : Topology has changed during ring recovery process. Function aborted.             */
  /* ------------------------------------------------------------------------------------------ */
  case CSMD_FUNCTION_STEP_10:
    
    prFuncState->ulSleepTime = 0;
    eFuncRet = CSMD_RING_RECOVERY_ABORTED;
    break;
    
  default:
    eFuncRet = CSMD_ILLEGAL_CASE;
    break;
    
  } /* end switch */
  
  return eFuncRet;
  
} /* end: CSMD_RecoverRingTopology() */



/**************************************************************************/ /**
\brief Opens the Sercos ring between two nodes.

\ingroup module_redundancy
\b Description: \n
   This function opens the ring between two slaves or between a master port and
   the directly connected slave. After the function has been executed without
   errors, the ring may be physically opened.\n
   The Sercos addresses of the slaves located at the end of the line(s) are entered into the
   variables usSercAddrLastSlaveP1 and/or usSercAddrLastSlaveP2 in the CoSeMa
   structure. Cross-Communication between slaves that are located in different
   lines after having opened the ring is maintained by CoSeMa-internal copying functions.

<B>Call Environment:</B> \n
   When this function is called, the current topology must be a ring.\n
   The call-up should be performed from a task.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usSlaveAddr1
              0-511, Sercos slave addresses or master port identification\n
              0 = open at Master-Port\n
              1-511 = Sercos slave addresses
\param [in]   usSlaveAddr2
              0-511, Sercos slave addresses or master port identification\n
              0 = open at Master-Port\n
              1-511 = Sercos slave addresses\n
\param [in]   prFuncState
              Pointer to management data for this function
   
\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_WRONG_TOPOLOGY \n
              \ref CSMD_OPEN_RING_ERROR \n
              \ref CSMD_OPEN_RING_INVALID_ADDR \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_OPEN_RING_OK \n
                                   
\author       KP
 
\date         08.01.2008 
 
***************************************************************************** */
CSMD_FUNC_RET CSMD_OpenRing( CSMD_INSTANCE   *prCSMD_Instance,
                             CSMD_FUNC_STATE *prFuncState,
                             CSMD_USHORT      usSlaveAddr1,
                             CSMD_USHORT      usSlaveAddr2 )
{
  
  CSMD_USHORT  usTempS_DevWord;
  CSMD_USHORT  usTempC_DevWord;
  CSMD_USHORT  usI;
  CSMD_USHORT  usSlaveIdx_P = 0xFFFF; /* index of slave to be commanded loopback and forward of P-telegrams */
  CSMD_USHORT  usSlaveIdx_S = 0xFFFF; /* index of slave to be commanded loopback and forward of S-telegrams */
  
  CSMD_FUNC_RET   eFuncRet = CSMD_FUNCTION_IN_PROCESS;
  
  /* Wrong topology to execute OpenRing() */
  if (   !(prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
      && (prFuncState->usActState == CSMD_FUNCTION_1ST_ENTRY) )
  {
    prFuncState->ulSleepTime = 0UL;
    eFuncRet = (CSMD_WRONG_TOPOLOGY);
  }
  /* Wrong communication phase to execute OpenRing() */
  else if (prCSMD_Instance->sCSMD_Phase < CSMD_SERC_PHASE_1)
  {
    prFuncState->ulSleepTime = 0UL;
    eFuncRet = (CSMD_WRONG_PHASE);
  }
  else
  {
    switch (prFuncState->usActState)
    {
      /* ------------------------------------------------------------------*/
      /* Step Entry : Initialize open ring                                 */
      /* ------------------------------------------------------------------*/
      
    case CSMD_FUNCTION_1ST_ENTRY:
      
      prFuncState->ulSleepTime = 0UL;
      
      /* check if both addresses are greater than 511 or equal */
      if ((usSlaveAddr1 > CSMD_MAX_SLAVE_ADD) || (usSlaveAddr2 > CSMD_MAX_SLAVE_ADD) || (usSlaveAddr1 == usSlaveAddr2))
      {
        eFuncRet = (CSMD_OPEN_RING_INVALID_ADDR);
        break;
      }
      
      /* if one address is 0, check if the slave with the other address is directly connected to a master port */
      if (usSlaveAddr1 == 0U)
      {
        /* slave is directly connected to master port 1 */
        if (prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[0] == usSlaveAddr2)
        {
          usSlaveIdx_S = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr2];
        }
        /* slave is directly connected to master port 2 */
        else if (prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[0] == usSlaveAddr2)
        {
          usSlaveIdx_P = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr2];
        }
        /* slave is not connected to any master port */
        else
        {
          eFuncRet = (CSMD_OPEN_RING_INVALID_ADDR);
          break;
        }
      }
      else if (usSlaveAddr2 == 0U)
      {
        /* slave is directly connected to master port 1 */
        if (prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[0] == usSlaveAddr1)
        {
          usSlaveIdx_S = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr1];
        }
        /* slave is directly connected to master port 2 */
        else if (prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[0] == usSlaveAddr1)
        {
          usSlaveIdx_P = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr1];
        }
        /* slave is not connected to any master port */
        else
        {
          eFuncRet = (CSMD_OPEN_RING_INVALID_ADDR);
          break;
        }
      }
      /* two slaves are selected, check if they are directly connected to each other */
      else
      {
        /* search for one slave in the port topology list for port 1 */
        for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
        {
          if (usSlaveAddr1 == prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI])
          {
            break;
          }
        }

        if (prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI + 1] == usSlaveAddr2)
        {
          usSlaveIdx_P = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr1];
          usSlaveIdx_S = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr2];
        }
        /* prevent negative array index if one of the two selected
         * slaves is directly connected to master port */
        else if (usI > 0)
        {
          if (prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI - 1] == usSlaveAddr2)
          {
            usSlaveIdx_P = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr2];
            usSlaveIdx_S = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr1];
          }
        }
        else
        {
          eFuncRet = (CSMD_OPEN_RING_INVALID_ADDR);
          break;
        }
      }

      prFuncState->usActState = CSMD_FUNCTION_STEP_1;
      /*lint -fallthrough */
      
      /* ---------------------------------------------------------------------*/
      /* Step 1 : Set new topology and toggle topology bit                    */
      /* ---------------------------------------------------------------------*/
    case CSMD_FUNCTION_STEP_1:
      
      if (usSlaveIdx_P != 0xFFFF)
      {
        /* Set last slave's topology on P1 to Loopback-Forward of P-Telegrams */
          CSMD_SetC_DevTopology( prCSMD_Instance,
                                 CSMD_C_DEV_CMD_LOOPB_FW_P_TEL,
                                 usSlaveIdx_P );
      }
      if (usSlaveIdx_S != 0xFFFF)
      {
        /* Set last slave's topology on P2 to Loopback-Forward of S-Telegrams */
          CSMD_SetC_DevTopology( prCSMD_Instance,
                                 CSMD_C_DEV_CMD_LOOPB_FW_S_TEL,
                                 usSlaveIdx_S );
      }
      
      prFuncState->ulSleepTime = (CSMD_ULONG) ((8 * prCSMD_Instance->rPriv.ulActiveCycTime)/1000000);
      if (prFuncState->ulSleepTime < 4)
      {
        prFuncState->ulSleepTime = 4;
      }
      prFuncState->usActState = CSMD_FUNCTION_STEP_2;
      break;
      
      /* ------------------------------------------------------------------*/
      /* Step 2 : Check if topology change has been performed              */
      /* ------------------------------------------------------------------*/
    case CSMD_FUNCTION_STEP_2:
      
      if (usSlaveAddr1 > 0)
      {
        usTempS_DevWord = prCSMD_Instance->arDevStatus[prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr1]].usS_Dev;
        usTempC_DevWord = prCSMD_Instance->rPriv.ausDevControl[
                            prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr1]];

        if ((usTempS_DevWord & CSMD_MASK_TOPO_TOGGLE) == (usTempC_DevWord & CSMD_MASK_TOPO_TOGGLE))
        {
          prFuncState->ulSleepTime = 0;
          prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
          /* topology change at slave was successful */
          eFuncRet = CSMD_OPEN_RING_OK;
        }
        else
        {
          /* slave has not acknowledged the topology change, command fast forward */
          CSMD_SetC_DevTopology( prCSMD_Instance,
                                 CSMD_C_DEV_CMD_TOPOLOGY_FF_BOTH,
                                 prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr1] );
          eFuncRet = CSMD_OPEN_RING_ERROR;
        }
      }

      if (usSlaveAddr2 > 0)
      {
        usTempS_DevWord = prCSMD_Instance->arDevStatus[prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr2]].usS_Dev;
        usTempC_DevWord = prCSMD_Instance->rPriv.ausDevControl[
                            prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr2]];

        if ((usTempS_DevWord & CSMD_MASK_TOPO_TOGGLE) == (usTempC_DevWord & CSMD_MASK_TOPO_TOGGLE))
        {
          prFuncState->ulSleepTime = 0;
          prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
          /* topology change at slave was successful */
          eFuncRet = CSMD_OPEN_RING_OK;
        }
        else
        {
          /* slave has not acknowledged the topology change, command fast forward */
          CSMD_SetC_DevTopology( prCSMD_Instance,
                                 CSMD_C_DEV_CMD_TOPOLOGY_FF_BOTH,
                                 prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr2] );
          eFuncRet = CSMD_OPEN_RING_ERROR;
        }
      }
      break;
      
    default:
      eFuncRet = (CSMD_ILLEGAL_CASE);
      break;
      
    } /* end switch */
  }
  return (eFuncRet);           /* Function processing still active */
  
} /* end: CSMD_OpenRing() */

/*! \endcond */ /* PUBLIC */



/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/*! \endcond */ /* PRIVATE */


/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
03 Mar 2015 WK
  - Defdb00179554
    CSMD_RecoverRingTopology()
    Send new S-0-1015 unconditional to all slaves as of CP3.
    Execute command S-0-1024 at all slaves as of CP3.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
12 Nov 2015 AlM
  - Defdb00182904
    Fixed potential array-out-of-bounds problem in CSMD_RecoverRingTopology()
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
 11 Jul 2016 WK
 - Defdb00188354
   CSMD_RecoverRingTopology()
   Fixed division by zero during tNetwork calculation.
  
------------------------------------------------------------------------------
*/
