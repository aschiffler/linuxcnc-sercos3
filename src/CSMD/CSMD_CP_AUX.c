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
\file   CSMD_CP_AUX.c
\author WK
\date   01.09.2010
\brief  This File contains the public API and private auxiliary functions.

*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#include "CSMD_DIAG.h"

#define SOURCE_CSMD
#include "CSMD_CP_AUX.h"
#include "CSMD_TOPOLOGY.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */


/**************************************************************************/ /**
\brief Triggers the TCNT cycle counter once (software trigger).

\ingroup func_timing
\b Description: \n
   This function is an API interface for a single trigger of TCNT register.

<B>Call Environment:</B> \n
   This function can be called from a task if the master is in timing slave mode.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       none

\author       WK
\date         12.05.2010

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_VOID CSMD_Retrigger_System_Timer( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_HAL_Retrigger_TCNT( &prCSMD_Instance->rCSMD_HAL );
  
} /* end: CSMD_Retrigger_System_Timer() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Checks, if system timer is running.

\ingroup func_timing
\b Description: \n
   This function is an API interface for checking if the TCNT timer is
   currently running.

<B>Call Environment:</B> \n
   This function can be called from either an interrupt or a task.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       TRUE - System timer TCNT is running\n
              FALSE - System timer TCNT is not running

\author       WK
\date         12.05.2010

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_BOOL CSMD_IsRunning_System_Timer( CSMD_INSTANCE *prCSMD_Instance )
{
  
  return CSMD_HAL_IsRunning_TCNT( &prCSMD_Instance->rCSMD_HAL );
  
} /* end: CSMD_IsRunning_System_Timer() */
/*lint -restore const! */



#ifdef CSMD_SYNC_DPLL
/**************************************************************************/ /**
\brief Controls the DPLL for external sychronisation

\ingroup sync_ext_slave
\b Description: \n
   This function controls the PLL for re-adjusting the main timer TCNT
   when synchronizing using an external clock master.
   The parameters of the PLL are hard-coded in the Sercos III master IP core
   and cannot be changed by the user.

<B>Call Environment:</B> \n
   It is proposed to call this function cyclically using an interrupt or task 
   every n * tscyc (n>0) with boRunPLL = TRUE. 
   - With activation by boRunPLL = TRUE the function returns sequentially the 
     status CSMD_PLL_INIT, CSMD_PLL_ACTIVE, and CSMD_PLL_LOCKED.
     If the status changes from CSMD_PLL_LOCKED to CSMD_PLL_SYNC_ERROR, the PLL
     is automatically re-started.
   - The PLL may be switched off with boRunPLL = FALSE. The de-activation is 
     finalized when the function returns with the status CSMD_PLL_INIT.
     Afterwards, the function does not need to be called any more.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   boRunPLL
              PLL activation
              - TRUE  Activate the PLL
              - FALSE Deactivate the PLL

\return       \ref CSMD_PLL_STATE
              Current state of the PLL: 
              - \ref CSMD_PLL_NO_INFO        PLL returns no status information
              - \ref CSMD_PLL_INIT           PLL is in init state (not active)
              - \ref CSMD_PLL_ACTIVE         PLL is active (set after start sync command is executed)
              - \ref CSMD_PLL_LOCKED         PLL is locked to the master
              - \ref CSMD_PLL_SYNC_ERROR     PLL is outside of the synchronization window (not locked)

              - \ref CSMD_PLL_CMD_ACTIVE     Command for starting/stoping the PLL is active

\author       WK
\date         16.11.2011

***************************************************************************** */
CSMD_PLL_STATE CSMD_PLL_Control( CSMD_INSTANCE *prCSMD_Instance, 
                                 CSMD_BOOL      boRunPLL )
{
  
  CSMD_ULONG          ulState;
  CSMD_PLL_STATE      ePLL_State;
#ifdef CSMD_DEBUG_DPLL
  CSMD_PLL_DBG_INFO  *prDbg_PLL;
  
  
  prDbg_PLL = &prCSMD_Instance->rCSMD_Dbg_PLL;
#endif
  if (prCSMD_Instance->sCSMD_Phase >= CSMD_SERC_PHASE_3)
  {
#ifdef CSMD_DEBUG_DPLL
    ulState = prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulPLLCSR;
#else
    ulState = CSMD_HAL_Read_PLL_Status( &prCSMD_Instance->rCSMD_HAL );
#endif  /*  #ifdef CSMD_DEBUG_DPLL */
    
    /* Evaluation of PLL status */
    if (ulState & CSMD_HAL_PLL_CMD_ACTIVE) 
    {
      /* Command for starting/stoping the PLL is active */
      ePLL_State = CSMD_PLL_CMD_ACTIVE;
    }
    else if (ulState & CSMD_HAL_PLL_LOCKED)
    {
      /* PLL is locked to the master */
      ePLL_State = CSMD_PLL_LOCKED;
    }
    else if (ulState & CSMD_HAL_PLL_INIT)
    {
      /* PLL is in init state */
      ePLL_State = CSMD_PLL_INIT;
    }
    else if (ulState & CSMD_HAL_PLL_SYNC_ERROR)
    {
      /* PLL is outside of the synchronization window */
      ePLL_State = CSMD_PLL_SYNC_ERROR;
    }
    else if (ulState & CSMD_HAL_PLL_ACTIVE)
    {
      /* PLL is active (set after start sync command is executed) */
      ePLL_State = CSMD_PLL_ACTIVE;
    }
    else
    {
      /* No status information from PLL available */
      ePLL_State = CSMD_PLL_NO_INFO;
    }
    
#ifdef CSMD_DEBUG_DPLL
    prCSMD_Instance->rCSMD_Dbg_PLL.ulCycCnt++;
    if (ulState != prDbg_PLL->rPLL_Stat.ulStatus[prDbg_PLL->ulCurrIdx])
    {
      if (prDbg_PLL->ulCurrIdx < (CSMD_RPLL_STAT_MAX_ENTRYS - 1))
      {
        prDbg_PLL->ulCurrIdx++;
        prDbg_PLL->rPLL_Stat.ulStatus[prDbg_PLL->ulCurrIdx] = ulState;
        prDbg_PLL->rSPLL_State.eState[prDbg_PLL->ulCurrIdx] = ePLL_State;
        prDbg_PLL->rCyc_Cnt.ulCycCnt[prDbg_PLL->ulCurrIdx]  = prDbg_PLL->ulCycCnt;
      }
    }
#endif  /*  #ifdef CSMD_DEBUG_DPLL */
    
    if (boRunPLL == TRUE)
    {
      if (ulState & CSMD_HAL_PLL_LOCKED)
      {
        if (prCSMD_Instance->rPriv.boPLL_Mode == FALSE)
        {
          CSMD_HAL_Enable_PLL( &prCSMD_Instance->rCSMD_HAL, TRUE );
          prCSMD_Instance->rPriv.boPLL_Mode = TRUE;
        }
      }
      else if (ulState & CSMD_HAL_PLL_INIT)
      {
        CSMD_HAL_Write_PLL_Control(  &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_PLL_START_SYNC );
      }
      else if (ulState & CSMD_HAL_PLL_SYNC_ERROR)
      {
        if (prCSMD_Instance->rPriv.boPLL_Mode == TRUE)
        {
          CSMD_HAL_Write_PLL_Control(  &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_PLL_STOP_SYNC );
          CSMD_HAL_Enable_PLL( &prCSMD_Instance->rCSMD_HAL, FALSE );
          prCSMD_Instance->rPriv.boPLL_Mode = FALSE;
        }
      }
    }
    else
    {
      if (prCSMD_Instance->rPriv.boPLL_Mode == TRUE)
      {
        CSMD_HAL_Write_PLL_Control(  &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_PLL_STOP_SYNC );
        CSMD_HAL_Enable_PLL( &prCSMD_Instance->rCSMD_HAL, FALSE );
        prCSMD_Instance->rPriv.boPLL_Mode = FALSE;
      }
    }
  }
  else
  {
#ifdef CSMD_DEBUG_DPLL
    if (prDbg_PLL->ulCycCnt)
    {
      /* Clear the PLL debug structure */
      (CSMD_VOID) CSMD_HAL_memset( prCSMD_Instance->rCSMD_Dbg_PLL,
                                   0,
                                   sizeof(prCSMD_Instance->rCSMD_Dbg_PLL) );
    }
#endif  /*  #ifdef CSMD_DEBUG_DPLL */

    /* No status information from PLL available */
    ePLL_State = CSMD_PLL_NO_INFO;
  }
  
  return  (ePLL_State);
  
} /* End: CSMD_PLL_Control() */
#endif  /* #ifdef CSMD_SYNC_DPLL */

/*! \endcond */ /* PUBLIC */


/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Measures the ring delay on one or both ports.

\ingroup func_timing
\b Description: \n
   This function measures the ring delay on one or both ports and determines
   the respective minimum and maximum values.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       none

\author       WK
\date         03.12.2007

***************************************************************************** */
CSMD_VOID  CSMD_MeasureRingdelay( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_RING_DELAY  *prRingDelay = &prCSMD_Instance->rPriv.rRingDelay;
  CSMD_ULONG        ulDelay;
  CSMD_BOOL         boMeasurePort1 = FALSE;
  CSMD_BOOL         boMeasurePort2 = FALSE;

  switch (prCSMD_Instance->usCSMD_Topology)
  {
    case CSMD_TOPOLOGY_LINE_P1:
      boMeasurePort1 = TRUE;
      break;

    case CSMD_TOPOLOGY_LINE_P2:
      boMeasurePort2 = TRUE;
      break;

    case CSMD_TOPOLOGY_BROKEN_RING:
    case CSMD_TOPOLOGY_RING:
      boMeasurePort1 = TRUE;
      boMeasurePort2 = TRUE;
      break;

    case CSMD_TOPOLOGY_DEFECT_RING:
      if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_PRIMARY)
      {
        boMeasurePort1 = TRUE;
      }
      else /* if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_SECONDARY) */
      {
        boMeasurePort2 = TRUE;
      }
      break;

    default:
      break;
  }
  
  if (TRUE == boMeasurePort1)
  {
    /* ring delay time measuring */
    if (prRingDelay->usCount1 < CSMD_NBR_OF_RD_MEASUREMENTS)
    {
      ulDelay = CSMD_HAL_GetRingRuntimeMeasure( &prCSMD_Instance->rCSMD_HAL, 1U );
      if (ulDelay)
      {
        /* Ring delay = Measured delay relating to tScyc. */
        ulDelay -= prCSMD_Instance->rPriv.ulOffsetTNCT_SERCCycle;
#ifdef CSMD_DEBUG
        prRingDelay->ulBuff1[prRingDelay->usCount1] = ulDelay;
#endif
        prRingDelay->ulSumRD1 += ulDelay;
        
        if (ulDelay > prRingDelay->ulMaxDelay1)
        {
          prRingDelay->ulMaxDelay1 = ulDelay;
        }
        else if (ulDelay < prRingDelay->ulMinDelay1)
        {
          prRingDelay->ulMinDelay1 = ulDelay;
        }
        
        prRingDelay->usCount1++;
      }
    }
  }
  
  if (TRUE == boMeasurePort2)
  {
    /* ring delay time measuring */
    if (prRingDelay->usCount2 < CSMD_NBR_OF_RD_MEASUREMENTS)
    {
      ulDelay = CSMD_HAL_GetRingRuntimeMeasure( &prCSMD_Instance->rCSMD_HAL, 2U );
      if (ulDelay)
      {
        /* Ring delay = Measured delay relating to tScyc. */
        ulDelay -= prCSMD_Instance->rPriv.ulOffsetTNCT_SERCCycle;
#ifdef CSMD_DEBUG
        prRingDelay->ulBuff2[prRingDelay->usCount2] = ulDelay;
#endif 
        prRingDelay->ulSumRD2 += ulDelay;
        
        if (ulDelay > prRingDelay->ulMaxDelay2)
          prRingDelay->ulMaxDelay2 = ulDelay;
        else if (ulDelay < prRingDelay->ulMinDelay2)
          prRingDelay->ulMinDelay2 = ulDelay;
        
        prRingDelay->usCount2++;
      }
    }
  }
  
} /* end: CSMD_MeasureRingdelay() */



#if !defined CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
/**************************************************************************/ /**
\brief Builds parameter S-0-1015, "Ring delay".
  
\ingroup func_timing
\b Description: \n
   Builds ring delay depending on measured ring delays on both ports,
   topology, number of recognized and number of possible hot-plug slaves.
   S-0-1015 is transmitted to the slaves, but not furthermore used
   for timing calculations.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   eRD_Mode
              Mode for ring delay calculation:
              - CSMD_CALC_RD_MODE_NORMAL \n
                  Ring delay calculation during a communication phase switch
              - CSMD_CALC_RD_MODE_RECOVER_RING_ACTIVE \n
                  Ring delay calculation during a ring recovery
              - CSMD_CALC_RD_MODE_HOTPLUG_ACTIVE \n
                  Ring delay calculation during Hot-plug a slave
\param [out]  pulS_0_1015_P1
              Pointer for ring delay for parameter S-0-1015 
\param [out]  pulS_0_1015_P2
              Pointer for ring delay for parameter S-0-1015 for slave on port 2
              in case of broken ring topology.
 
\return       \ref CSMD_NO_ERROR \n
 
\author       WK
\date         13.10.2010

***************************************************************************** */
CSMD_FUNC_RET CSMD_Determination_Ringdelay( CSMD_INSTANCE     *prCSMD_Instance,
                                            CSMD_RD_CALC_MODE  eRD_Mode,
                                            CSMD_ULONG        *pulS_0_1015_P1,
                                            CSMD_ULONG        *pulS_0_1015_P2 )
{
  
  CSMD_ULONG       ulSlaveDelayHP;  /* Slave delay of all hot-plug slaves. */
  CSMD_ULONG       ulSlaveJitter;   /* Sum of slave jitter                 */
  CSMD_ULONG       ulExtraDelay;    /* additional delay at the ring delay  */
  CSMD_USHORT      usI;
  CSMD_RING_DELAY *prRingDelay = &prCSMD_Instance->rPriv.rRingDelay;
  
  
  /* ----------------------------------------------------------------
    Build S-0-1015 Ringdelay                                         
        
        Averaged ringdelay port 1 > Averaged ringdelay port 2:
        
        #<-------------- S-0-1015 (port 2) -------------->#
        #                                                 #
        #                   #  Extradelay + Extraoffset   #
        #<- AvRingdelay 2 ->#<-------------+------------->#
        #                   #              #
        #<------------ TSref ------------->#
        #                         #        #        #
        #<---- AvRingdelay 1 ---->#<-------+------->#
        #                         #    Extradelay   #
        #                                           #
        #<----------- S-0-1015 (port 1) ----------->#
        
          TSref = Averaged ringdelay port 1 + Extradelay / 2
        
        
        Averaged ringdelay port 2 > Averaged ringdelay port 1:
        
        #<-------------- S-0-1015 (port 1) -------------->#
        #                                                 #
        #                   #  Extradelay + Extraoffset   #
        #<- AvRingdelay 1 ->#<-------------+------------->#
        #                   #              #
        #<------------ TSref ------------->#
        #                         #        #        #
        #<---- AvRingdelay 2 ---->#<-------+------->#
        #                         #    Extradelay   #
        #                                           #
        #<----------- S-0-1015 (port 2) ----------->#
        
          TSref = Averaged ringdelay port 2 + Extradelay / 2
        
    S-0-1015 (port 1) = TSref + (TSref - Averaged ringdelay port 1)
    S-0-1015 (port 2) = TSref + (TSref - Averaged ringdelay port 2)


     ---------------------------------------------------------------- */

#ifdef CSMD_HOTPLUG
  if (eRD_Mode == CSMD_CALC_RD_MODE_HOTPLUG_ACTIVE)
  {
    /* ============================================================= */
    /* After hotplug a slave:                                        */
    /* Determine S-0-1015 for the port on which the hot-plug         */
    /* was performed.                                                */
    if (prCSMD_Instance->rPriv.rHotPlug.prHP_ActPort == &prCSMD_Instance->rPriv.rHP_P1_Struct)
    {
      /* Average value of Ring Delay port 1 */
      prRingDelay->ulAverageRD_P1 =
        prRingDelay->ulSumRD1 / CSMD_NBR_OF_RD_MEASUREMENTS;
      /* Hot-plug was executed at port 1 */
      *pulS_0_1015_P1 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P1;
      /* Ringdelay of port 2 will remain unaffected */
      *pulS_0_1015_P2 = prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015_P2;
    }
    else
    {
      /* Average value of Ring Delay port 2 */
      prRingDelay->ulAverageRD_P2 =
        prRingDelay->ulSumRD2 / CSMD_NBR_OF_RD_MEASUREMENTS;
      /* Hot-plug was executed at port 2 */
      *pulS_0_1015_P2 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P2;
      /* Ringdelay of port 1 will remain unaffected */
      *pulS_0_1015_P1 = prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015;
    }
  }
  
  else  /* Ring delay determination for communication phase switching or ring recovery */
#endif  /* #ifdef CSMD_HOTPLUG */
  {
    /* Average value of Ring Delay port 1 */
    prRingDelay->ulAverageRD_P1 = 
      prRingDelay->ulSumRD1 / CSMD_NBR_OF_RD_MEASUREMENTS;
    /* Average value of Ring Delay port 2 */
    prRingDelay->ulAverageRD_P2 = 
      prRingDelay->ulSumRD2 / CSMD_NBR_OF_RD_MEASUREMENTS;
    
    if (eRD_Mode == CSMD_CALC_RD_MODE_RECOVER_RING_ACTIVE)
    {
      /* ============================================================= */
      /* Active ring recovery, Ring is closed!                         */
      /* ============================================================= */
      *pulS_0_1015_P1 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P1;
      *pulS_0_1015_P2 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P2;
    }
    
    else  /* eRD_Mode == CSMD_CALC_RD_MODE_NORMAL */
    {
      /* Calculate part of extra delay caused by slave jitter of projected slaves */
      for (usI=0, ulSlaveJitter = 0; usI<prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        ulSlaveJitter +=
          prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTiming.usSlaveJitter_S1037;
      }
      ulSlaveJitter *= 2;
      prRingDelay->ulSlaveJitter = ulSlaveJitter;
      
      /* Calculate part of extra delay for all possible hot-plug slaves */
      if (prCSMD_Instance->rSlaveList.usNumRecogSlaves >= prCSMD_Instance->rSlaveList.usNumProjSlaves)
      {
        ulSlaveDelayHP = 0;
      }
      else
      {
        /* #hot-plug slaves = #projected slaves - #recognized slaves */
        if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
        {
          ulSlaveDelayHP =   (  prCSMD_Instance->rSlaveList.usNumProjSlaves
                              - prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb)
                           * prCSMD_Instance->rConfiguration.rComTiming.ulCalc_DelaySlave;
        }
        else
        {
          ulSlaveDelayHP =   (  prCSMD_Instance->rSlaveList.usNumProjSlaves
                              - (  prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb
                                 + prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb))
                           * prCSMD_Instance->rConfiguration.rComTiming.ulCalc_DelaySlave;
        }
        ulSlaveDelayHP *= 2;
      }
      
      ulExtraDelay = (ulSlaveJitter + ulSlaveDelayHP) * 2;
      prRingDelay->ulExtraDelay = ulExtraDelay;
      
      /* ======================================================================== */
      if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
      {
        if (prCSMD_Instance->rSlaveList.usNumRecogSlaves)
        {
          if (prRingDelay->ulAverageRD_P1 >= prRingDelay->ulAverageRD_P2)
          {
            /* Note: Add 1 to the recognized slaves and remove addition of CSMD_DELAY_MASTER_DELAY
                     to prevent a division by zero. */
            prRingDelay->ulTSref =   ((prRingDelay->ulAverageRD_P1 ) / (prCSMD_Instance->rSlaveList.usNumRecogSlaves + 1))
                                   * (prCSMD_Instance->rSlaveList.usNumRecogSlaves * 2 )
                                   + ulExtraDelay / 2;
          }
          else
          {
            prRingDelay->ulTSref =   ((prRingDelay->ulAverageRD_P2 ) / (prCSMD_Instance->rSlaveList.usNumRecogSlaves + 1))
                                   * (prCSMD_Instance->rSlaveList.usNumRecogSlaves * 2 )
                                   + ulExtraDelay / 2;
          }
        }
        else
        {
          if (prRingDelay->ulAverageRD_P1 >= prRingDelay->ulAverageRD_P2)
          {
            prRingDelay->ulTSref = prRingDelay->ulAverageRD_P1 + ulExtraDelay / 2;
          }
          else
          {
            prRingDelay->ulTSref = prRingDelay->ulAverageRD_P2 + ulExtraDelay / 2;
          }
        }
        *pulS_0_1015_P1 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P1;
        *pulS_0_1015_P2 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P2;
      }
      
      /* ======================================================================== */
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
      {
        prRingDelay->ulTSref = prRingDelay->ulAverageRD_P1 + ulExtraDelay / 2;
        
        *pulS_0_1015_P1 = prRingDelay->ulAverageRD_P1 + ulExtraDelay;
        *pulS_0_1015_P2 = 0;
      }
      
      /* ======================================================================== */
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
      {
        prRingDelay->ulTSref = prRingDelay->ulAverageRD_P2 + ulExtraDelay / 2;
        
        *pulS_0_1015_P1 = 0;
        *pulS_0_1015_P2 = prRingDelay->ulAverageRD_P2 + ulExtraDelay;
      }
      
      /* ======================================================================== */
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
      {
        prRingDelay->ulTSref = prRingDelay->ulAverageRD_P1 + prRingDelay->ulAverageRD_P2 + ulExtraDelay / 2;

        *pulS_0_1015_P1 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P1;
        *pulS_0_1015_P2 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P2;
      }
      
      /* ======================================================================== */
      else
      {
        /* Should not be here ! */
        /*! >todo - How to handle unknown topology respectively defect ring ?! */
        if (prRingDelay->ulAverageRD_P1 > prRingDelay->ulAverageRD_P2)
        {
          prRingDelay->ulTSref = prRingDelay->ulAverageRD_P1 + ulExtraDelay / 2;
        }
        else
        {
          prRingDelay->ulTSref = prRingDelay->ulAverageRD_P2 + ulExtraDelay / 2;
        }
        
        *pulS_0_1015_P1 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P1;
        *pulS_0_1015_P2 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P2;
      }
    }
  }
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Determination_Ringdelay() */



/**************************************************************************/ /**
\brief Calculates the ring delay independent of measured delay.
  
\ingroup func_timing
\b Description: \n
   Build Ring delay independent of measured ring delay in order to achieve
   a stable ring delay value in every communication phase progression process if slave
   configuration has not changed since last run-up.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
 
\return       none
 
\author       AM
\date         15.04.2010
  
***************************************************************************** */
CSMD_VOID  CSMD_Calculate_RingDelay( CSMD_INSTANCE *prCSMD_Instance )

{
  
  CSMD_ULONG  ulSlaveJitter = 0;  /* Sum of slave jitter */
  CSMD_USHORT usI;
  
  for (usI=0; usI<prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    ulSlaveJitter +=
      prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTiming.usSlaveJitter_S1037;
  }
  ulSlaveJitter *= 2;
  
  prCSMD_Instance->rConfiguration.rComTiming.ulCalc_RingDelay =
    (prCSMD_Instance->rSlaveList.usNumProjSlaves * 2 + 1)
     * prCSMD_Instance->rConfiguration.rComTiming.ulCalc_DelaySlave
      + ulSlaveJitter
      + prCSMD_Instance->rConfiguration.rComTiming.ulCalc_DelayMaster
      + prCSMD_Instance->rConfiguration.rComTiming.ulCalc_DelayComp;
  
} /* end: CSMD_Calculate_RingDelay() */
#endif   /* #if !defined CSMD_STABLE_TSREF */



/**************************************************************************/ /**
\brief Sets the Sercos Communication Version Field (in MDT0-MST of CP0).

\ingroup module_phase
\b Description: \n
   No further description.
  
<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
    - Preempt remote address allocation
    - Set number of telegrams for CP1/CP2
    - Forwarding srecos telegrams at last slave in line
    - Fast CP switch
    - Transmit communication parameters for CP1/CP2

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   eComVersion
                CSMD_COMVERSION_V1_0 
                for address allocation (Sercos version >= 1.1.1).

\return       TRUE - for successful change

\author       bk
\date         17.10.2006
    
***************************************************************************** */
CSMD_BOOL CSMD_SetComVersion( CSMD_INSTANCE     *prCSMD_Instance,
                              CSMD_COM_VERSION   eComVersion )
{
  
  prCSMD_Instance->rPriv.rComVersion.ulLong = 0;
  
  /* In future versions may check setting for allowed states */
  if (eComVersion == CSMD_COMVERSION_V1_0)
  {
    prCSMD_Instance->rPriv.rComVersion.ulLong |= CSMD_COMMVER_ADD_ALLOC;
  }
  
#ifdef  CSMD_4MDT_4AT_IN_CP1_2
  /* Number of Telegrams: 4 MDT and 4 AT in CP1 and CP2 */
  if (prCSMD_Instance->rPriv.rHW_Init_Struct.boFourMDT_AT_CP12 == TRUE)
  {
    prCSMD_Instance->rPriv.rComVersion.ulLong |= CSMD_COMMVER_CP12_4MDT_4AT;
    prCSMD_Instance->rPriv.usNbrTel_CP12       = 4;
  }
  else
#endif
  {
    prCSMD_Instance->rPriv.rComVersion.ulLong |= CSMD_COMMVER_CP12_2MDT_2AT;
    prCSMD_Instance->rPriv.usNbrTel_CP12       = 2;
  }
  
#ifdef CSMD_SWC_EXT
  /* Switch off Sercos III telegrams */
  if (prCSMD_Instance->rPriv.rHW_Init_Struct.boTelOffLastSlave == TRUE)
  {
    prCSMD_Instance->rPriv.rComVersion.ulLong |= CSMD_COMMVER_LAST_FORW_OFF;
  }
  
  /* Fast communication phase switch */
  if (prCSMD_Instance->rPriv.rHW_Init_Struct.boFastCPSwitch == TRUE)
  {
    prCSMD_Instance->rPriv.rComVersion.ulLong |= CSMD_COMMVER_FAST_CP_SWITCH;
  }
  
  /* Transmission of communication parameters in CP0 */
  if (   (prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_1)
      || (prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_2)
      || (prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_1_VAR))
  {
    prCSMD_Instance->rPriv.rComVersion.ulLong |= CSMD_COMMVER_COMM_PAR_CP0;
  }
#endif  /* #ifdef CSMD_SWC_EXT */
  
  return TRUE;
  
} /* End: CSMD_SetComVersion */



/**************************************************************************/ /**
\brief Provides a pointer to the service channel in Tx Ram of a slave
       inside the MDT in CP1 and CP2.
                          
\ingroup module_phase
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to allocated memory range for the variables of the
              CoSeMa instance 
\param [in]   usSlaveIndex
              Index of the Sercos III slave

\return       \ref CSMD_SVC_CONTROL_UNION Pointer to service channel

\author       bk
\date         17.10.2006
    
***************************************************************************** */
CSMD_SVC_CONTROL_UNION * CSMD_GetMdtServiceChannel( CSMD_INSTANCE *prCSMD_Instance,
                                                    CSMD_USHORT    usSlaveIndex )
{
  
  CSMD_USHORT              usMdtIndex;
  CSMD_USHORT              usSvcIndex;
  CSMD_USHORT              usTAdd;
  CSMD_SVC_CONTROL_UNION  *prSVC       = NULL;
  
  usTAdd = prCSMD_Instance->rPriv.ausTopologyAddresses[usSlaveIndex];
  if (usTAdd)
  {
    /* get MDT telegram index and position inside telegram */
    CSMD_GetTelegramPosition( usTAdd, &usMdtIndex, &usSvcIndex );
    
    prSVC = (CSMD_SVC_CONTROL_UNION *)(CSMD_VOID *) (prCSMD_Instance->rPriv.pulSERC3_Tx_MDT_SVC) + CSMD_SVC_WORDSIZE * usSvcIndex;
  }
  else
  {
    CSMD_RuntimeWarning( prCSMD_Instance, usSlaveIndex,
                         "CSMD_GetMdtServiceChannel: called with invalid slave index" );
  }
  
  return prSVC;
  
} /* End: CSMD_GetMdtServiceChannel */



/**************************************************************************/ /**
\brief Provides a pointer to the service channel of a slave
       inside the AT (usage only for CP1).

\ingroup module_phase
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to allocated memory range for the variables of the
              CoSeMa instance 
\param [in]   usPort
              Port index
\param [in]   usTAdd
              Topology address of slave

\return       \ref CSMD_SVC_FIELD_AT_STRUCT Pointer to service channel

\author       bk
\date         17.10.2006

***************************************************************************** */
CSMD_SVC_FIELD_AT_STRUCT * CSMD_GetAtServiceChannel( const CSMD_INSTANCE *prCSMD_Instance,
                                                     CSMD_USHORT          usPort,
                                                     CSMD_USHORT          usTAdd )
{
  
  CSMD_UCHAR   *pucSvc = NULL;
  CSMD_USHORT   usAtIndex;
  CSMD_USHORT   usSvcIndex;
  
  /* get AT telegram index and position inside telegram */
  CSMD_GetTelegramPosition( usTAdd, &usAtIndex, &usSvcIndex );
  
  /* create base pointer to start of all service */
  if (usPort == CSMD_PORT_2)
  {
    pucSvc = (CSMD_UCHAR *)(CSMD_VOID *)
      prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[0];
  }
  else
  {
    pucSvc = (CSMD_UCHAR *)(CSMD_VOID *)
      prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[0];
  }
  
  /* add 6 byte offset for all leading service channels */
  pucSvc = pucSvc + (CSMD_SVC_BYTESIZE * usSvcIndex);
  
  return (CSMD_SVC_FIELD_AT_STRUCT *)(CSMD_VOID *)pucSvc;

} /* of CSMD_GetAtServiceChannel */



/**************************************************************************/ /**
\brief Provides MDT telegram index and position
       inside the telegram in CP1 or CP2. 

\ingroup module_phase
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   usTAdd
              Topology address of the selected slave
\param [out]  pusTelegramIndex
              Pointer to MDT index
\param [out]  pusPositionIndex
              Pointer to position index 

\return       none

\author       bk
\date         17.10.2006
    
***************************************************************************** */
CSMD_VOID CSMD_GetTelegramPosition( CSMD_USHORT  usTAdd,
                                    CSMD_USHORT *pusTelegramIndex,
                                    CSMD_USHORT *pusPositionIndex )
{
  
  *pusTelegramIndex = 0;
  *pusPositionIndex = usTAdd;
  
} /* of CSMD_GetTelegramPosition */



/**************************************************************************/ /**
\brief Saves the SVC Control word from Tx Ram.

\ingroup module_phase
\b Description: \n
   This function saves the service channel control word from TxRam in order to
   restore the master handshake bit properly at start of CP3.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to allocated memory range for the variables of the
              CoSeMa instance

\return       none

\author       WK
\date         02.04.2007
    
***************************************************************************** */
CSMD_VOID  CSMD_SaveSVCControlMDTPhase2( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT              usI;
  CSMD_SVC_CONTROL_UNION  *prSvcCtrl;  /* pointer to service channel control word */
  
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    prSvcCtrl = CSMD_GetMdtServiceChannel(prCSMD_Instance, usI);
    
    if (prSvcCtrl != NULL)
      prCSMD_Instance->rPriv.ausSERC3_MDT_Control_SVC[usI] = 
        CSMD_HAL_ReadShort( &prSvcCtrl->usWord );
  }
  
} /* end: CSMD_SaveSVCControlMDTPhase2() */



/**************************************************************************/ /**
\brief Checks validity of Sercos cycle time in CP1 and CP2.

\ingroup module_phase
\b Description:
   The validity of Sercos cycle time in CP1 and CP2 depends on "Number of
   MDTs and ATs in CP1 and CP2" in the communication version field (MDT0 in CP0).
   With 2MDT/2AT the min. cycle time is 1ms, otherwise 2ms.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to allocated memory range for the variables of the
              CoSeMa instance

\param [in]   ulCycleTimeCP12
              Currently chosen cycle time in CP0-CP2

\return       \ref CSMD_INVALID_SERCOS_CYCLE_TIME \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         07.07.2008

***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckCycleTime_CP12( const CSMD_INSTANCE *prCSMD_Instance,
                                        CSMD_ULONG           ulCycleTimeCP12 )
{
  
  CSMD_ULONG  ulMinCycTimeCP12;
  CSMD_BOOL   boFourMDT_AT_CP12;

#ifdef CSMD_4MDT_4AT_IN_CP1_2
    /* Check for valid Sercos cycle time for CP1 and CP2 */
  boFourMDT_AT_CP12 = prCSMD_Instance->rPriv.rHW_Init_Struct.boFourMDT_AT_CP12;
#else
  boFourMDT_AT_CP12 = FALSE;
#endif
  
  if (boFourMDT_AT_CP12 == TRUE)
  {
    ulMinCycTimeCP12 = CSMD_TSCYC_2_MS;
  }
  else
  {
    ulMinCycTimeCP12 = CSMD_TSCYC_1_MS;
  }
  
  /* Check for valid Sercos cycle time for CP0 */
  if (   (ulCycleTimeCP12 < ulMinCycTimeCP12)
      || (ulCycleTimeCP12 % CSMD_TSCYC_250_US)
      || (ulCycleTimeCP12 > CSMD_TSCYC_MAX) )
  {
    return (CSMD_INVALID_SERCOS_CYCLE_TIME);
  }
  else
  {
    return (CSMD_NO_ERROR);
  }
  
} /* end: CSMD_CheckCycleTime_CP12() */



/**************************************************************************/ /**
\brief Sets the CYCCLK related registers.

\ingroup module_phase
\b Description: \n
   This function sets the register TCYCSTART (0x3C) and the CYCCLK polarity,
   CYCCLK enable and the Timing Master/Slave bits in the TCSR register (0x38).
  
<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to allocated memory range for the variables of the
              CoSeMa instance

\return       none

\author       WK
\date         05.09.2005

***************************************************************************** */
CSMD_VOID CSMD_SetCYCCLK( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_HAL_ConfigCYCCLK( &prCSMD_Instance->rCSMD_HAL, 
                         prCSMD_Instance->rPriv.rCycClk.boActivate,
                         prCSMD_Instance->rPriv.rCycClk.boEnableInput,
                         prCSMD_Instance->rPriv.rCycClk.boPolarity,
                         prCSMD_Instance->rPriv.rCycClk.ulStartDelay );
  
} /* end: CSMD_SetCYCCLK() */



/**************************************************************************/ /**
\brief Clears telegram data in FPGA TxRam or RxRam.

\ingroup module_phase
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to allocated memory range for the variables of the
              CoSeMa instance
\param [in]   pucTelegram
              Pointer to telegram memory
\param [in]   usTelLen
              Telegram length in Bytes
\param [in]   usFillData
              Data used for memory initialization

\return       none

\author       WK
\date         23.05.2005
    
***************************************************************************** */
/*lint -save -e550 Symbol pDummy not accessed */
/*lint -save -e438 Last value assigned to variable not used */
CSMD_VOID CSMD_Telegram_Clear( CSMD_INSTANCE *prCSMD_Instance,
                               CSMD_UCHAR    *pucTelegram,
                               CSMD_USHORT    usTelLen,
                               CSMD_USHORT    usFillData )
{
  
  CSMD_USHORT   *pusTelDestRam  = (CSMD_USHORT *)(CSMD_VOID *)pucTelegram;
  CSMD_USHORT    usNbrWords     = (CSMD_USHORT)(usTelLen/2);
  CSMD_ULONG     ulFillData     = ((CSMD_ULONG)usFillData << 16) + (CSMD_ULONG)usFillData;
  CSMD_USHORT    usNbrLongs;        /* Nbr of Longs for Long Clear */
  CSMD_ULONG    *pulTelDest;        /* Pointer to Long for Long Clear */
  CSMD_INSTANCE *pDummy;
  
  pDummy = prCSMD_Instance;   /* Dummy access to prevent compiler warnings; is optimized out */
#ifdef CSMD_DEBUG
  prCSMD_Instance->rCSMD_Debug.pusTelDbg = pusTelDestRam;
  prCSMD_Instance->rCSMD_Debug.usLenDbg  = usTelLen;
#endif
  
  if ((CSMD_UINT_PTR)pusTelDestRam & 2U)
  {
    /* Word alignment: at first clear one Word */
    CSMD_HAL_WriteShort( pusTelDestRam, usFillData );
    pusTelDestRam++;
    usNbrWords--;
  }
  
  /* Long alignment: clear long by long */
  usNbrLongs = (CSMD_USHORT)(usNbrWords / 2);
  pulTelDest = (CSMD_ULONG *)(CSMD_VOID *)pusTelDestRam;
  
  while (usNbrLongs)
  {
    CSMD_HAL_WriteLong( pulTelDest, ulFillData );
    pulTelDest++;
    usNbrLongs--;
  }
  
  /* Rest of one Word? */
  if (usNbrWords & (CSMD_USHORT)0x0001)
  {
    /* clear last Word */
    CSMD_HAL_WriteShort( ((CSMD_USHORT *)(CSMD_VOID *)pulTelDest), usFillData );
  }
  
} /* end: CSMD_Telegram_Clear() */
/*lint -restore */



/**************************************************************************/ /**

\brief Writes the communication version and parameters into the CP0 MDT0.

\ingroup module_phase
\b Description: \n
   This function 

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   eActiveUCC_Mode_CP12
                Preselected by application:
                - CSMD_UCC_MODE_CP12_FIX
                - CSMD_UCC_MODE_CP12_1
                - CSMD_UCC_MODE_CP12_2
                - CSMD_UCC_MODE_CP12_1_VAR
\param [in]   pulTxRam_CP0_MDT0
                Pointer to memory the memory location of the CP0 MDT0
                telegram data in the FPGA TxRam.
\param [in]   prComVersion
                Pointer to the Communication version structure.
\param [in]   prTimingCP12
                Pointer to the calculated timing data for CP1/CP2.

\return       none

\author       WK
\date         16.07.2012

***************************************************************************** */
CSMD_VOID CSMD_Write_CP0_MDT0( CSMD_UCC_MODE_ENUM             eActiveUCC_Mode_CP12,
                               CSMD_ULONG                    *pulTxRam_CP0_MDT0,
                               const CSMD_COM_VER_FIELD      *prComVersion,
                               const CSMD_TIMING_CP12_STRUCT *prTimingCP12 )
{
  
  CSMD_ULONG  *pulTxRam = pulTxRam_CP0_MDT0;
  CSMD_USHORT  usI;
  
  /* Init communication version field in MDT0 */
  CSMD_HAL_WriteLong( pulTxRam++, prComVersion->ulLong );
  
  if (eActiveUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_FIX)
  {
    /* Clear MDT0 padding data */
    for (usI = 1; usI < (CSMD_SERC3_MDT_DATA_LENGTH_P0 / 4); usI++)
    {
      CSMD_HAL_WriteLong( pulTxRam++, 0 );
    }
  }
  else
  {
    CSMD_HAL_WriteLong( pulTxRam++, prTimingCP12->ulStartAT_t1 );
  
    CSMD_HAL_WriteLong( pulTxRam++, prTimingCP12->ulOpenUCC_t6 );
  
    CSMD_HAL_WriteLong( pulTxRam++, prTimingCP12->ulCloseUCC_t7 );
  
    /* Clear MDT0 padding data */
    for (usI = 4; usI < (CSMD_SERC3_MDT_DATA_LENGTH_P0 / 4); usI++)
    {
      CSMD_HAL_WriteLong( pulTxRam++, 0 );
    }
  }

} /* end: CSMD_Write_CP0_MDT0() */

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
  - Defdb00177704
    CSMD_Determination_Ringdelay()
    Fixed calculation of TSref (Extra delay too small).
    After Hot-plug, always calculate a new ring delay.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
24 Jul 2015 WK
  - Defdb00178597
    CSMD_Telegram_Clear():
    Fixed type castings for 64-bit systems.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
  
------------------------------------------------------------------------------
*/

