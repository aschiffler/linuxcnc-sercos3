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
\file   CSMD_CONFIG.c
\author WK
\date   01.09.2010
\brief  This File contains the public API functions for
        the slave configuration.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#include "CSMD_CONFIG.h"
#include "CSMD_DIAG.h"
#include "CSMD_PRIV_SVC.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/**************************************************************************/ /**
\brief Performs a check of the SCP classes.

\ingroup func_config
\b Description: \n
    Before calling this function, the following pre-conditions have to be met:
    - For all hot-plug slaves, the supported SCP classes need to be stored to
      the Sercos lists ausSCPClasses[ ].
    - The Sercos lists of the active SCP classes ausActiveSCPClasses[ ] need to be
      configured for the coressponding slaves, if required by the master.
    
    This function reads the SCP classes supported by the slaves
    (S-0-1000 "List of SCP classes & version") and stores the information in 
    ausSCPClasses[ ]. \n
    The Sercos lists ausSCPClasses[ ] and ausActiveSCPClasses[ ] are checked for consistency:\n
    It is checked whether the rules for S-0-1000.0.1 are met:
    - Only version from either SCP class FixCFG or VarCFG may be signalled. Otherwise,
      \ref CSMD_BASIC_SCP_TYPE_MISMATCH is returned.
    - Check, whether all lower versions of an SCP class are present in the list S-0-1000.\n

    The active SCP classes ausActiveSCPClasses[ ] are stored to S-0-1000.0.1 
    "List of active SCP classes & version" in the slaves.

    The SCP classes are interpreted by CoSeMa as follows:

    Slave supports\n S-0-1000.1.1 | ausActiveSCPClasses[ ] | CoSeMa make use of \n the following SCP classes
    ----------------------------- | ---------------------- | -----------------------------------------------
    Yes                           | is empty list          | All classes of version 1 supported \n by CoSeMa in ausSCPClasses[ ]
    Yes                           | list cotains classes   | ausActiveSCPClasses [ ]
    No                            | is empty list          | ausSCPClasses[ ]
    No                            | list contain classes   | Error message #CSMD_S_0_1000_0_1_NOT_SUPPORTED
    
    The following SCP classes are supported:
    
   - <B>Basic SCP classes:</B>
     - SCP_FixCFG       => Fix configuration of (two) connections
     - SCP_FixCFG_0x02  => Fix configuration of (two) connections
     - SCP_FixCFG_0x03  => Fix configuration of (two) connections & connection stop
     \n
     - SCP_VarCFG       => Variable configuration of (with up to 255) homogeneous connections
     - SCP_VarCFG_0x02  => Variable configuration of (with up to 255) homogeneous connections
     - SCP_VarCFG_0x03  => Variable configuration of (with up to 255) homogeneous connections & connection stop
   
   - <B>Additional SCP classes:</B>
     - SCP_Sync         --> Synchronization
     - SCP_Sync_0x02    => Synchronization + IFG
     - SCP_Sync_0x03    => Synchronization tSync > tScyc using MDT Extended field
     - SCP_WD           => Watch dog of connection
     - SCP_RTB          => Configuration of real-time bits
     - SCP_NRT          => each UC communication parameter activated immediately
     - SCP_Cap          => Connection Capabilities
     - SCP_SysTime      => set Sercos Time using MDT extended field
     - SCP_NRTPC        => UC communication parameters activated with procedure command
     - SCP_Cyc          => cyclic communication
     - SCP_WDCon        => Watch-dog of connection with tPcyc & data losses
     - SCP_SWC          => Support of Industrial Ethernet protocols via UCC
    
<B>Call Environment:</B> \n
   This function shall be called after CP2 is reached and before call of CSMD_GetTimingData().\n
   The call-up should be performed from a task.\n
   This function comprises a state machine.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_SERCOS_VERSION_MISMATCH \n
              \ref CSMD_SERCOS_LIST_TOO_LONG \n
              \ref CSMD_S_0_1000_0_1_NOT_SUPPORTED \n
              \ref CSMD_ACT_SCP_CLASS_NOT_SUPPORTED \n
              \ref CSMD_ACT_SCP_MULTIPLE_VERSIONS \n
              \ref CSMD_BASIC_SCP_TYPE_MISMATCH \n
              \n
              \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         02.08.2007

***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckVersion( CSMD_INSTANCE          *prCSMD_Instance,
                                 CSMD_FUNC_STATE        *prFuncState,
                                 CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_USHORT   usI;
  CSMD_FUNC_RET eFuncRet;
  
  
  /* --------------------------------------------------------- */
  /* Step 1 :: Check current communication phase               */
  /* --------------------------------------------------------- */
  if (prCSMD_Instance->sCSMD_Phase < CSMD_SERC_PHASE_2)
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
      
      eFuncRet = CSMD_Get_S1000( prCSMD_Instance,
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
      
      /* Checks the plausibility of the SCP classes. */
      eFuncRet = CSMD_Check_Active_SCP_Classes( prCSMD_Instance,
                                                &prCSMD_Instance->rPriv.rInternalFuncState,
                                                parSvcMacro );
      
      if (eFuncRet == CSMD_NO_ERROR)
      {
        /* Evaluate SCP type list and build bitlist */
        eFuncRet = CSMD_Build_SCP_BitList( prCSMD_Instance );
        
        if (eFuncRet != CSMD_NO_ERROR)
        {
          break;
        }
        
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
      
      /* Checks whether all slaves with SCP_sync support the parameter S-0-1036 */
      eFuncRet = CSMD_Check_S1036( prCSMD_Instance,
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
      
      /* Set flag to 'Slave SCP configurations has been checked' */
      prCSMD_Instance->rPriv.boSCP_Checked = TRUE;
      
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
      {
        CSMD_INT     nSlaves;           /* Loop counter projected slaves */

        /* Determine maximum slave jitter of all slaves. */
        prCSMD_Instance->rPriv.usMaxSlaveJitter = 0;
        for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++)
        {
          if (  prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].rTiming.usSlaveJitter_S1037
              > prCSMD_Instance->rPriv.usMaxSlaveJitter)
          {
            prCSMD_Instance->rPriv.usMaxSlaveJitter =
              prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].rTiming.usSlaveJitter_S1037;
          }
        }
      }
#endif
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
      eFuncRet = CSMD_NO_ERROR;
      break;
      
    default:
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usCancelActTrans = 1U;
          (CSMD_VOID)CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
        }
      }
      eFuncRet = CSMD_ILLEGAL_CASE;
      break;
      
  }
  return (eFuncRet);
  
}   /* End: CSMD_CheckVersion() */



/**************************************************************************/ /**
\brief Prepares the timing slave mode.


\ingroup sync_external
\b Description: \n
   This function prepares the timing slave mode for activation in CSMD_SetPhase0()
   to CSMD_SetPhase4().\n The CYC_CLK input may also be activated or deactivated
   at any later time by means of the CSMD_Enable_CYCCLK_Input() function.\n\n
   Note: In CSMD_InitHardware(), the timing master mode is set by default.

   - Timing master mode: \n
     The input CYC_CLK is ignored. The counter TCNT is not synchronized by external input.

   - Timing slave mode: \n
     The counter TCNT is synchronized by the external input CYC_CLK. \n
     The synchronization of TCNT may be delayed. The internal delay between
     synchronization of TCNT and start of Sercos cycle (End MDT0-MST) is not compensated!

<B>Call Environment:</B> \n
   For an activation in CP3, this function must be called in CP2 before
   the function CSMD_CalculateTiming()!\n
   The function can be called from a task.
   
\param [in]   prCSMD_Instance
                Pointer to memory range allocated for the variables of the
                CoSeMa instance
\param [in]   boActivate
                0 deactivate CYC_CLK (timing master mode)\n
                1 activate CYC_CLK (timing slave mode)
\param [in]   boEnableInput 
                0 disable of input CYC_CLK\n
                1 enable of input CYC_CLK
\param [in]   boPolarity    
                0 positive edge of CYC_CLK\n
                1 negative edge of CYC_CLK
\param [in]   ulStartDelay
                Delay between CYC_CLK input and start of main counter TCNT [ns]. \n
                This value is not relevant for the Timing master mode!
  
\return       \ref CSMD_NO_ERROR \n
    
\author       WK
\date         05.09.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_PrepareCYCCLK ( CSMD_INSTANCE *prCSMD_Instance,
                                   CSMD_BOOL      boActivate,
                                   CSMD_BOOL      boEnableInput,
                                   CSMD_BOOL      boPolarity,
                                   CSMD_ULONG     ulStartDelay )
{
  
  prCSMD_Instance->rPriv.rCycClk.boActivate    = boActivate;
  
  prCSMD_Instance->rPriv.rCycClk.boEnableInput = boEnableInput;
  
  prCSMD_Instance->rPriv.rCycClk.boPolarity    = boPolarity;
  
  prCSMD_Instance->rPriv.rCycClk.ulStartDelay  = ulStartDelay;
  
  /*! >todo Check value of Start-delay depending on communication phase ? */
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_PrepareCYCCLK() */



/**************************************************************************/ /**
\brief Prepares the timing slave mode.

\ingroup sync_external
\b Description: \n
   This function prepares the timing slave mode for activation in CSMD_SetPhase0()
   to CSMD_SetPhase4().\n The CYC_CLK input may also be activated or deactivated
   at any later time by means of the CSMD_Enable_CYCCLK_Input() function.\n\n
   Note: In CSMD_InitHardware(), the timing master mode is set by default.

   - Timing master mode: \n
     The input CYC_CLK is ignored. The counter TCNT is not synchronized by external input.

   - Timing slave mode: \n
     The counter TCNT is synchronized by the external input CYC_CLK. \n
     The synchronization of TCNT may be delayed. The internal delay between
     synchronization of TCNT and start of Sercos cycle (End MDT0-MST) is taken into account.

<B>Call Environment:</B> \n
   For an activation in CP3, this function must be called in CP2 before
   the function CSMD_CalculateTiming()!\n
   The function can be called from a task.

\param [in]   prCSMD_Instance
                Pointer to memory range allocated for the variables of the
                CoSeMa instance
\param [in]   boActivate
                0 deactivate CYC_CLK (timing master mode)\n
                1 activate CYC_CLK (timing slave mode)
\param [in]   boEnableInput
                0 disable of input CYC_CLK\n
                1 enable of input CYC_CLK
\param [in]   boPolarity
                0 positive edge of CYC_CLK\n
                1 negative edge of CYC_CLK
\param [in]   ulDelaySercosCycle
                Delay between CYC_CLK input and start of Sercos cycle [ns]. \n
                The minimum value is communication phase dependent:
                - CP0...CP2:\n
                  52460 ns (Fixed master jitter + MST delay = 50000 + 2460)
                - CP3 / CP4:\n
                  CSMD_MASTER_CONFIGURATION.ulJitter_Master + 2460 ns
                .
                The value of ulDelaySercosCycle is only related to the Sercos cycle,
                if it does not fall below the minimum value. \n
                \n
                This value is not relevant for the Timing master mode!

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         19.11.2014

***************************************************************************** */
CSMD_FUNC_RET CSMD_PrepareCYCCLK_2 ( CSMD_INSTANCE *prCSMD_Instance,
                                     CSMD_BOOL      boActivate,
                                     CSMD_BOOL      boEnableInput,
                                     CSMD_BOOL      boPolarity,
                                     CSMD_ULONG     ulDelaySercosCycle )
{

  if (ulDelaySercosCycle > prCSMD_Instance->rPriv.ulOffsetTNCT_SERCCycle)
  {
    prCSMD_Instance->rPriv.rCycClk.ulStartDelay  =
        ulDelaySercosCycle - prCSMD_Instance->rPriv.ulOffsetTNCT_SERCCycle;
  }
  else
  {
    prCSMD_Instance->rPriv.rCycClk.ulStartDelay  = 0UL;
  }
  prCSMD_Instance->rPriv.rCycClk.boActivate    = boActivate;
  prCSMD_Instance->rPriv.rCycClk.boEnableInput = boEnableInput;
  prCSMD_Instance->rPriv.rCycClk.boPolarity    = boPolarity;
  return (CSMD_NO_ERROR);

} /* end: CSMD_PrepareCYCCLK_2() */



/**************************************************************************/ /**
\brief CSMD_Enable_CYCCLK_Input() enables or disables the CYC_CLK input in
       TCSR register (0x38).

\ingroup sync_ext_slave
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This function may be called from either an interrupt or from a task.
   
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   boEnable
              0 disable of input CYC_CLK\n
              1 enable  of input CYC_CLK

\return       none

\author       WK
\date         05.09.2005

***************************************************************************** */
CSMD_VOID CSMD_Enable_CYCCLK_Input ( CSMD_INSTANCE *prCSMD_Instance,
                                     CSMD_BOOL      boEnable )
{

  CSMD_HAL_CtrlCYCCLKInput (&prCSMD_Instance->rCSMD_HAL, boEnable);

} /* end: CSMD_Enable_CYCCLK_Input() */



/**************************************************************************/ /**
\brief Configures the CON_CLK output in TCSR register (0x38).

\ingroup sync_ext_master
\b Description: \n
   This function enables or disables the CON_CLK output. The times for 
   setting and clearing the output must have been determined by CSMD_EventControl().

<B>Call Environment:</B> \n
   This function may be called from either an interupt or a task.

  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   boActivate
              0 deactivate CON_CLK function\n
              1 activate CON_CLK function
\param [in]   boEnableDriver
              0 disable output driver\n
              1 enable output driver
\param [in]   boPolarity
              0 CON_CLK signal active high (ONE)\n
              1 CON_CLK signal active low (ZERO)

\return       none

\author       WK
\date         05.09.2005

***************************************************************************** */
CSMD_VOID CSMD_SetCONCLK ( CSMD_INSTANCE *prCSMD_Instance,
                           CSMD_BOOL      boActivate,
                           CSMD_BOOL      boEnableDriver,
                           CSMD_BOOL      boPolarity )
{
  
  CSMD_HAL_ConfigCONCLK (&prCSMD_Instance->rCSMD_HAL, boActivate, boEnableDriver, boPolarity);

} /* end: CSMD_SetCONCLK() */



/**************************************************************************/ /**
\brief Configures and activates the DIV_CLK functionality.

\ingroup module_sync
\b Description: \n
   This function configures and activates the DIV_CLK output. Via the DIV_CLK
   output, the control application can be coupled to the Sercos communication
   timing. The chosen mode determines whether the Sercos cycle is to be duplicated
   or divided.\n
   The DIV_CLK signal can trigger an interrupt. The control application is
   affected by the interrupt control functions.

<B>Call Environment:</B> \n
   This function may be called from either an interrupt or a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   boActivate 
              0 deactivate DIV_CLK function\n
              1 activate DIV_CLK function
\param [in]   boMode     
              0 DIV_CLK becomes active n-times within one comm. cycle.\n
              1 DIV_CLK becomes active once after n comm cycles.
\param [in]   boPolarity 
              0 DIV_CLK signal is active on positive edge.\n
              1 DIV_CLK signal is active on negative edge.
\param [in]   boOutpDisable  
              0 DIV_CLK output is enabled.\n
              1 DIV_CLK output is disabled (tristated).
\param [in]   usNbrPulses 
              Number of DIV_CLK pulses 'n' within one comm. cycle (max 256)
\param [in]   ulPulseDistance
              DIV_CLK pulse distance [ns]
\param [in]   ulStartDelay
              Time at whitch the first pulse of DIV_CLK occurs [ns]

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         16.10.2006

***************************************************************************** */
CSMD_FUNC_RET CSMD_ConfigDIVCLK ( CSMD_INSTANCE *prCSMD_Instance,
                                  CSMD_BOOL      boActivate,
                                  CSMD_BOOL      boMode,
                                  CSMD_BOOL      boPolarity,
                                  CSMD_BOOL      boOutpDisable,
                                  CSMD_USHORT    usNbrPulses,
                                  CSMD_ULONG     ulPulseDistance,
                                  CSMD_ULONG     ulStartDelay )
{
  
  return (CSMD_FUNC_RET) CSMD_HAL_ConfigDIVCLK( &prCSMD_Instance->rCSMD_HAL, 
                                                boActivate, 
                                                boMode, 
                                                boPolarity,
                                                boOutpDisable,
                                                usNbrPulses, 
                                                ulPulseDistance, 
                                                ulStartDelay );
  
} /* end: CSMD_ConfigDIVCLK() */



/**************************************************************************/ /**
\brief Prepares the event timers 0 to 3 that can be triggered
       by the TCNT timer and the CON_CLK hardware signal.

\ingroup sync_external
\b Description: \n
   The events prepared with this function will be activated during the phase start-up in
   CSMD_SetPhase0(), CSMD_SetPhase1(), and CSMD_SetPhase3().
   The time and activation state of the event stay constant when switching phases.
   When switching to CP3, changes have to be performed after calling CSMD_CalculateTiming()!\n
\n
   In CSMD_InitHardware(), all events that can be activated by means of this function, are deactivated.\n
   The time given for the event is related to the Sercos cycle. The time must not be larger
   than the maximum time that is defined in 'MaxTime'.\n
   Triggering of an interrupt for the timer events must be activated via the interrupt control
   functions (CSMD_IntControl, CSMD_CheckInt).
\note \n
   The buffer change events (IDs CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A and CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A) \n
   may in some cases (mult-buffer switching, event-controlled DMA) be activated by CoSeMa with default values.
   These default values may be changed before switching to the next communication phase.
   
<B>Call Environment:</B> \n
   The function can be called from a task.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usEventID
              Event identifier:\n
              \ref CSMD_EVENT_ID_TIMER_0 \n
              \ref CSMD_EVENT_ID_TIMER_1 \n
              \ref CSMD_EVENT_ID_TIMER_2 \n
              \ref CSMD_EVENT_ID_TIMER_3 \n
              \ref CSMD_EVENT_ID_CONCLK_SET \n
              \ref CSMD_EVENT_ID_CONCLK_RESET \n
              \ref CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A \n
              \ref CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A \n

\param [in]   boActivate
              TRUE - Event is activated by the next communication phase switch routine call.\n
              FALSE - Event is deactivated by the next communication phase switch routine call.
\param [in]   ulTime
              Time for this Event [ns] relating to tScyc
  
\return       \ref CSMD_WRONG_EVENT_ID \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         29.08.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_EventControl( CSMD_INSTANCE *prCSMD_Instance,
                                 CSMD_USHORT    usEventID,
                                 CSMD_BOOL      boActivate,
                                 CSMD_ULONG     ulTime )
{
  
  if (usEventID < CSMD_NBR_EVENT_ID)
  {
    if (boActivate == TRUE)
    {
      prCSMD_Instance->rPriv.rCSMD_Event.aulTime[usEventID]       = ulTime;
      prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[usEventID] = TRUE;
    }
    else
    {
      prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[usEventID] = FALSE;
    }
    return (CSMD_NO_ERROR);
  }
  else
  {
    return (CSMD_WRONG_EVENT_ID);
  }
  
} /* end: CSMD_EventControl() */



/**************************************************************************/ /**
\brief Read the current Event time for the selected configurable Event.

\ingroup sync_external
\b Description: \n
              Read the current configured time for the selected Event.

<B>Call Environment:</B> \n
     The function can be called from a task.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\param [in]   usEventID
              Event identifier:\n
              CSMD_EVENT_ID_TIMER_0 \n
              CSMD_EVENT_ID_TIMER_1 \n
              CSMD_EVENT_ID_TIMER_2 \n
              CSMD_EVENT_ID_TIMER_3 \n
              CSMD_EVENT_ID_CONCLK_SET \n
              CSMD_EVENT_ID_CONCLK_RESET \n
              CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A \n
              CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A \n

\param [in]   pulTime
              Pointer to current configured (not teh active) Time for 
              this Event [ns] relating to tScyc.
  
\return       \ref CSMD_WRONG_EVENT_ID \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         01.12.2011

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET  CSMD_GetEventTime( CSMD_INSTANCE  *prCSMD_Instance,
                                  CSMD_ULONG     *pulTime,
                                  CSMD_USHORT     usEventID )
{
  
  if (usEventID < CSMD_NBR_EVENT_ID)
  {
    *pulTime = prCSMD_Instance->rPriv.rCSMD_Event.aulTime[usEventID];
    return (CSMD_NO_ERROR);
  }
  else
  {
    *pulTime = 0xFFFFFFFF;
    return (CSMD_WRONG_EVENT_ID);
  }
  
} /* end: CSMD_GetEventTime() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Assigns interrupt sources to hardware output INT0 or INT1
       and enable interrupts.

\ingroup sync_internal
\b Description: \n
   This function executes the following interrupt control processes:
   - Activation or deactivation of interrupt sources writing the corresponding
     bits in Interrupt Enable Register 0. Therefore the events causing an
     interrupt can be determined.
   - Assignment of an interrupt source to one of the two hardware outputs
     INT0 or INT1 writing the corresponding bit in Interrupt Multiplex
     Register 0.

  | Interrupt\n Number | Interrupt\n Reset\n Register 0 | Name | Event              |
  | :-- | :----- | :---------------- | :------------------------------------------- |
  |  0  | Bit  0 | Int_TINT0         | Event TINT[0], created by timer/counter TCNT |
  |  1  | Bit  1 | Int_TINT1         | Event TINT[1], created by timer/counter TCNT |
  |  2  | Bit  2 | Int_TINT2         | Event TINT[2], created by timer/counter TCNT |
  |  3  | Bit  3 | Int_TINT3         | Event TINT[3], created by timer/counter TCNT |
  |  4  | Bit  4 | Int_TINTMAX       | Event TMAX, created by timer/counter TCNT    |
  | ... |        |                   |                                              |
  |  7  | Bit  7 | Int_DivClk        | Interrupt from DivClk unit                   |
  |  8  | Bit  8 | Int_IPIntPort1    | Event IP port 1                              |
  |  9  | Bit  9 | Int_IPIntPort2    | Event IP port 2                              |
  | ... |        |                   |                                              |
  | 12  | Bit 12 | Int_RxBufReqPort1 | Internal receive buffer change for port 1    |
  | 13  | Bit 13 | Int_RxBufReqPort2 | Internal receive buffer change for port 2    |

<B>Call Environment:</B> \n
   This function should not be called from an interrupt. The function may be
   called at any time if the desired sources for an interrupt have changed.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   ulIntEnable
              Enable interrupt source by setting corresponding bit to 1
\param [in]   ulIntOutputMask
              Assigns interrupt source to hardware output INT0 by setting the
              correspondig bit to zero.\n
              Assigns to INT1 by setting the corresponding bit to one.

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         15.09.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_IntControl( CSMD_INSTANCE *prCSMD_Instance,
                               CSMD_ULONG     ulIntEnable,
                               CSMD_ULONG     ulIntOutputMask )
{
  /* Assign to hardware output INT0/INT1 */
  /* Reset disabled interrupts */
  /* Enable interrupt Source */
  CSMD_HAL_IntControl( &prCSMD_Instance->rCSMD_HAL, 
                       ulIntEnable, 
                       TRUE,
                       FALSE,
                       ulIntOutputMask ); 
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_IntControl() */



/**************************************************************************/ /**
\brief Reads the current value from the TCNT counter (System time readback register).

\ingroup sync_internal
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This function may be called from either an interrupt or a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [out]  pulSysTimerReadbackValue
              Pointer to current value of FPGA system counter [ns]

\return       none
  
\author       WK
\date         29.08.2005

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_VOID CSMD_Get_TCNT( CSMD_INSTANCE *prCSMD_Instance,
                         CSMD_ULONG    *pulSysTimerReadbackValue )
{

  *pulSysTimerReadbackValue = CSMD_HAL_GetSystemTimer(&prCSMD_Instance->rCSMD_HAL); 

} /* end: CSMD_Get_TCNT() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Returns the current time of the Sercos cycle relating to the end of MST.

\ingroup sync_internal
\b Description: \n
   The current time value returned by this function refers to the end of MST.

<B>Call Environment:</B> \n
   This function may be called either from an interrupt or a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [out]  pulCurrentTimeInCycle 
              Output pointer for return value which holds the current time in
              Sercos cycle [ns].

\return       none

\author       WK
\date         02.10.2009

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_VOID CSMD_Get_TCNT_Relative( CSMD_INSTANCE *prCSMD_Instance,
                                  CSMD_ULONG    *pulCurrentTimeInCycle )
{
  
  *pulCurrentTimeInCycle = CSMD_HAL_GetSystemTimer(&prCSMD_Instance->rCSMD_HAL) -
                           /* Delay between TCNT start and End of MDT0_MST */  
                           prCSMD_Instance->rPriv.ulOffsetTNCT_SERCCycle;

} /* end: CSMD_Get_TCNT_Relative() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Returns the Synchronization reference time (TSref).

\ingroup func_timing_aux
\b Description: \n
   This access function returns the Synchronization reference time (TSref)
   calculated in CP1.\n
   CSMD_WRONG_PHASE is returned, if the current phase is lower than CP1.

<B>Call Environment:</B> \n
   This function may be called either from an interrupt or a task.

\param [in]   prCSMD_Instance
                Pointer to memory range allocated for the variables of the
                CoSeMa instance
\param [out]  pulTSref
                Synchronization reference time (TSref) [ns]

\return       #CSMD_WRONG_PHASE \n
              #CSMD_NO_ERROR

\author       WK
\date         16.01.2015

***************************************************************************** */
CSMD_FUNC_RET CSMD_Get_TSref( const CSMD_INSTANCE *prCSMD_Instance,
                              CSMD_ULONG          *pulTSref )
{
  if (prCSMD_Instance->sCSMD_Phase < CSMD_SERC_PHASE_1)
  {
    return (CSMD_WRONG_PHASE);
  }
  else
  {
    *pulTSref = prCSMD_Instance->rPriv.rRingDelay.ulTSref;
    return (CSMD_NO_ERROR);
  }
} /* end: CSMD_Get_TSref() */



#ifdef CSMD_HW_WATCHDOG
/**************************************************************************/ /**
\brief Triggers the hardware watchdog counter, if active.

\ingroup func_watchdog
\b Description: \n
   This function triggers the hardware watchdog if the watchdog is activated. 
   The actual watchdog counter is set to the initial watchdog counter value. 

<B>Call Environment:</B> \n
   This function must be called cyclically if the watchdog is active, so it has
   to be called from an interrupt.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       none

\author       AM
\date         08.07.2010

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_VOID CSMD_Watchdog_Trigger( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_HAL_WatchdogTrigger( &prCSMD_Instance->rCSMD_HAL );
  
} /* end: CSMD_Watchdog_Trigger() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Enables or disables the hardware watchdog. 

\ingroup func_watchdog
\b Description: \n
   This function can be called to either activate or deactivate the watchdog.
   Deactivation of watchdog may take place even if watchdog action has been 
   processed yet.\n The behavior at watchdog timeout 
   (Insert zeros into the telegrams / Stop sending telegrams) is adjusted 
   depending on the consumed slave connections in CSMD_CalculateTiming().

<B>Call Environment:</B> \n
   This function should be called from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   boControl
              TRUE - activates the watchdog
              FALSE - deactivates the watchdog

\return       none

\author       AM
\date         08.07.2010

***************************************************************************** */
CSMD_VOID CSMD_Watchdog_Control( CSMD_INSTANCE *prCSMD_Instance,
                                 CSMD_BOOL      boControl )
{
  
  CSMD_HAL_WatchdogMode( &prCSMD_Instance->rCSMD_HAL,
                         prCSMD_Instance->usWD_Mode );

  CSMD_HAL_WatchdogSet( &prCSMD_Instance->rCSMD_HAL,
                        boControl );
  
} /* end: CSMD_Watchdog_Control() */



/**************************************************************************/ /**
\brief Configures the number of cycles required for watchdog action. 

\ingroup func_watchdog
\b Description: \n
   The number of required triggerless cycles for watchdog action is configured
   by transmitting the chosen value to this function.
     
<B>Call Environment:</B>
   This function should be called either from an interrupt or from a task.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usCycles
              number of triggerless cycles 

\return       none

\author       AM
\date         08.07.2010

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_VOID CSMD_Watchdog_Configure( CSMD_INSTANCE *prCSMD_Instance,
                                   CSMD_USHORT    usCycles )
{
  
  CSMD_HAL_WatchdogConfigure( &prCSMD_Instance->rCSMD_HAL,
                              usCycles );
  
} /* end: CSMD_Watchdog_Configure() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Returns the current watchdog status.

\ingroup func_watchdog
\b Description: \n
   This function provides a structure containing the current value of watchdog
   counter register WDCNT as well as the two status bits (16 and 17) of the watchdog
   control and status register WDCSR.

<B>Call Environment:</B> \n
   This function can be called either from an interrupt or from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [out]  prWdStatus
              Output pointer for return value, holding structure with current
              watchdog status

\return       none

\author       AM
\date         08.07.2010

***************************************************************************** */
CSMD_VOID CSMD_Watchdog_Status( CSMD_INSTANCE *prCSMD_Instance,
                                CSMD_WDSTATUS *prWdStatus )
{
  
  CSMD_HAL_WatchdogStatus( &prCSMD_Instance->rCSMD_HAL,
                           (CSMD_HAL_WDSTATUS *) (CSMD_VOID *)prWdStatus );
  
} /* end: CSMD_Watchdog_Status() */

#endif  /* #ifdef CSMD_HW_WATCHDOG */

/*! \endcond */ /* PUBLIC */


/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Read S-0-1000 "List of SCP classes & version" of all slaves.

\ingroup func_config
\b Description: \n
This function reads S-0-1000 "List of SCP classes & version" of all
operable slaves.

<B>Call Environment:</B> \n
  This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function 
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_SERCOS_VERSION_MISMATCH \n
              \ref CSMD_SERCOS_LIST_TOO_LONG \n
              \n
              \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         18.11.2008

***************************************************************************** */
CSMD_FUNC_RET  CSMD_Get_S1000( CSMD_INSTANCE          *prCSMD_Instance,
                               CSMD_FUNC_STATE        *prFuncState,
                               CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_USHORT    usI;
  CSMD_USHORT    usFinished;
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  CSMD_SLAVE_CONFIGURATION *parSlaveConfig;
  
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    prFuncState->ulSleepTime = 0UL;
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prCSMD_Instance->rExtendedDiag.ulIDN       = CSMD_IDN_S_0_1000;
    
    if (prCSMD_Instance->rSlaveList.usNumProjSlaves == 0)
    {
      prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
      
      return (CSMD_NO_ERROR);
    }
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    
    prFuncState->usActState = CSMD_FUNCTION_STEP_1;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_1:
    
    usFinished = 1U;
    
    /* Check length of Parameter S-0-1000 "List of SCP classes & version" for all slaves */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
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
              parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1000;
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
      }
    } /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
    
    if (usFinished)
    {
      /* check current list length */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          if (parSvcMacro[usI].usState == CSMD_DATA_VALID)
          {
            /* is list, check current length */
            if (  prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData[0]   /* current list length received  */
                > (CSMD_MAX_ENTRIES_S_0_1000 * 2))                      /* maximum list length available */
            {
              usFinished = 0U;
              eFuncRet = CSMD_SERCOS_LIST_TOO_LONG;
              
              CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
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
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      
      /* continue with next step */
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    }
    
    else 
    {
      if (eFuncRet == CSMD_SERCOS_LIST_TOO_LONG)
      {
        return (eFuncRet);
      }
      
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_2:
    
    usFinished = 1U;
    
    /* Points to configuration of first slave */
    parSlaveConfig = &prCSMD_Instance->rConfiguration.parSlaveConfig[0];
    
    /* Read Parameter S-0-1000 "List of SCP classes & version" for all slaves */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
               || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
        {
          /* check if MBUSY is set, if it is not set do nothing */
          if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
          {
            if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
            {
              parSvcMacro[usI].pusAct_Data            = &parSlaveConfig[usI].ausSCPClasses[0];
              parSvcMacro[usI].usSlaveIdx             = usI;
              parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
              parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1000;
              parSvcMacro[usI].usIsList               = CSMD_ELEMENT_IS_LIST;
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

    prFuncState->usActState = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);
    
    
  default:
    return (CSMD_ILLEGAL_CASE);
    
  }   /* End: switch (prFuncState->usActState) */
  
  return (CSMD_FUNCTION_IN_PROCESS);
  
} /* end: CSMD_Get_S1000() */



/**************************************************************************/ /**
\brief Checks the plausibility of the SCP classes.

\ingroup func_config
\b Description: \n

  \todo description
  This function checks 
  - the availability of the IDN S-0-1000.0.1
  - the plausibility between the available SCP classes (ausSCPClasses[]) \n
    and the active SCP classes (ausActiveSCPClasses[]).

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function 
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_S_0_1000_0_1_NOT_SUPPORTED \n
              \ref CSMD_SERCOS_LIST_TOO_LONG \n
              \ref CSMD_ACT_SCP_CLASS_NOT_SUPPORTED \n
              \ref CSMD_ACT_SCP_MULTIPLE_VERSIONS \n
              \n
              \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         21.02.2012

***************************************************************************** */
CSMD_FUNC_RET  CSMD_Check_Active_SCP_Classes( CSMD_INSTANCE          *prCSMD_Instance,
                                              CSMD_FUNC_STATE        *prFuncState,
                                              CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{

  CSMD_USHORT    usI;
  CSMD_USHORT    usFinished;
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  CSMD_SLAVE_CONFIGURATION *parSlaveConfig;
  
  
  switch (prFuncState->usActState)
  {
    
    case CSMD_FUNCTION_1ST_ENTRY:
      
      prFuncState->ulSleepTime = 0UL;
      prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
      prCSMD_Instance->rExtendedDiag.ulIDN       = CSMD_IDN_S_0_1000_0_1;
    
      if (prCSMD_Instance->rSlaveList.usNumProjSlaves == 0)
      {
        prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
      
        return (CSMD_NO_ERROR);
      }
    
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          prCSMD_Instance->rPriv.ausActConnection[usI] = 0;    /* Used as flag 'IDN available' */
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
    
      prFuncState->usActState = CSMD_FUNCTION_STEP_1;
      /*lint -fallthrough */
      
      
    case CSMD_FUNCTION_STEP_1:
      
      usFinished = 1U;
      
      /* Points to configuration of first slave */
      parSlaveConfig = &prCSMD_Instance->rConfiguration.parSlaveConfig[0];
      
      /* Check the availability of Parameter S-0-1000.0.1 "List of active SCP classes & version" for all slaves */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
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
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE3_ATTRIBUTE;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1000_0_1;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_NO_LIST;
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
                  /* */
                  eFuncRet = CSMD_NO_ERROR;
                }
                else
                {
                  CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                }
              }
              else if (parSvcMacro[usI].usState == CSMD_DATA_VALID)
              {
                /* Set flag to 'IDN available' */
                prCSMD_Instance->rPriv.ausActConnection[usI] = 1;
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
        prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
      }
      else
      {
        break;
      }
      /*lint -fallthrough */
      
      
    case CSMD_FUNCTION_STEP_2:
      
      /* Points to configuration of first slave */
      parSlaveConfig = &prCSMD_Instance->rConfiguration.parSlaveConfig[0];
      
      /* Check SCP classes */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          /* Is S-0-1000.0.1 available? */
          if (prCSMD_Instance->rPriv.ausActConnection[usI] == 1)
          {
            /* Active SCP classes filled by the application? */
            if (parSlaveConfig[usI].ausActiveSCPClasses[0] == 0)
            {
              /* Set all relevant Classes with Version 1 from S-0-1000 ! */
              eFuncRet = CSMD_SCP_SetDefaultClasses( prCSMD_Instance, usI );
            }
            /*lint -e{838} Previously assigned value to variable 'eFuncRet' has not been used */
            eFuncRet = CSMD_SCP_PlausibilityCheck( prCSMD_Instance, usI );
            
            parSvcMacro[usI].usState = CSMD_START_REQUEST;
          }
          else
          {
            /* IDN not there! Active SCP classes filled by the application? */
            if (parSlaveConfig[usI].ausActiveSCPClasses[0])
            {
              if (prCSMD_Instance->rExtendedDiag.usNbrSlaves < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
              {
                prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves]    = usI;
                prCSMD_Instance->rExtendedDiag.aeSlaveError[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = CSMD_S_0_1000_0_1_NOT_SUPPORTED;
                prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
              }
            }
          }
        }
        else
        {
          /* Hot-Plug slave! */
          /* Active SCP classes filled by the application? */
          if (parSlaveConfig[usI].ausActiveSCPClasses[0])
          {
            eFuncRet = CSMD_SCP_PlausibilityCheck( prCSMD_Instance, usI );
          }
        }
      }
      
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
      }
      /* continue with next step */
      prFuncState->ulSleepTime = 0U;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_3;
      /*lint -fallthrough */
      
      
    case CSMD_FUNCTION_STEP_3:
      
      usFinished = 1U;
      
      /* Points to configuration of first slave */
      parSlaveConfig = &prCSMD_Instance->rConfiguration.parSlaveConfig[0];
      
      /* Write Parameter S-0-1000.0.1 "List of active SCP classes & version" to all slaves */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          if ( !(   (parSvcMacro[usI].usState == CSMD_DATA_VALID)
                 || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
          {
            /* check if MBUSY is set, if it is not set do nothing */
            if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
            {
              if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
              {
                parSvcMacro[usI].pusAct_Data            = &parSlaveConfig[usI].ausActiveSCPClasses[0];
                parSvcMacro[usI].usSlaveIdx             = usI;
                parSvcMacro[usI].usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
                parSvcMacro[usI].ulIdent_Nbr            = CSMD_IDN_S_0_1000_0_1;
                parSvcMacro[usI].usIsList               = CSMD_ELEMENT_IS_LIST;
                parSvcMacro[usI].usLength               = 0U;
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
        prFuncState->usActState  = CSMD_FUNCTION_STEP_4;
      }
      else
      {
        break;
      }
      /*lint -fallthrough */
      
      
    case CSMD_FUNCTION_STEP_4:
      
      prFuncState->usActState = CSMD_FUNCTION_FINISHED;
      return (CSMD_NO_ERROR);
      
      
    default:
      return (CSMD_ILLEGAL_CASE);
      
  } /* End: switch (prFuncState->usActState) */
  
  return (CSMD_FUNCTION_IN_PROCESS);
  
} /* end: CSMD_Check_Active_SCP_Classes() */



/**************************************************************************/ /**
\brief Checks the plausibility of the given active SCP classes.

\ingroup func_config
\b Description: \n

This function checks if 
- the limit for the current list length of ausActiveSCPClasses[].
- all active SCP classes are elemets in S-0-1000.
- all active SCP classes are present in only one version. 

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usSlaveIdx
              Slave Index of the selected slave.

\return       \ref CSMD_SERCOS_LIST_TOO_LONG \n
              \ref CSMD_ACT_SCP_CLASS_NOT_SUPPORTED \n
              \ref CSMD_ACT_SCP_MULTIPLE_VERSIONS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         21.02.2012

***************************************************************************** */
CSMD_FUNC_RET CSMD_SCP_PlausibilityCheck( CSMD_INSTANCE *prCSMD_Instance,
                                          CSMD_USHORT    usSlaveIdx )
{
  
  CSMD_USHORT    usJ, usK;
  CSMD_USHORT    usNbrClasses;     /* Number of elemnts in S-0-1000     */
  CSMD_USHORT    usNbrActClasses;  /* Number of elemnts in S-0-1000.0.1 */
  CSMD_USHORT    usActClass;
  CSMD_BOOL      boFound;
  CSMD_USHORT   *pausSCP_List;
  CSMD_USHORT   *pausActSCP_List;
  
  
  prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
  prCSMD_Instance->rExtendedDiag.ulIDN       = CSMD_IDN_S_0_1000_0_1;
  
  /* Check the plausibility of S-0-1000.0.1 */
  usNbrActClasses = (CSMD_USHORT)(prCSMD_Instance->rConfiguration.parSlaveConfig[usSlaveIdx].ausActiveSCPClasses[0] / 2U);

  if (usNbrActClasses > CSMD_MAX_ENTRIES_S_0_1000)
  {
    prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves]    = usSlaveIdx;
    prCSMD_Instance->rExtendedDiag.aeSlaveError[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = CSMD_SERCOS_LIST_TOO_LONG;
    prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
  }
  else
  {
    pausActSCP_List = &prCSMD_Instance->rConfiguration.parSlaveConfig[usSlaveIdx].ausActiveSCPClasses[2];
  
    usNbrClasses    = (CSMD_USHORT)(prCSMD_Instance->rConfiguration.parSlaveConfig[usSlaveIdx].ausSCPClasses[0] / 2U);
    pausSCP_List    = &prCSMD_Instance->rConfiguration.parSlaveConfig[usSlaveIdx].ausSCPClasses[2];
  
    /* 1. Check if active classes are members of S-0-1000 */
    for (usJ = 0; usJ < usNbrActClasses; usJ++)
    {
      usActClass = pausActSCP_List[usJ];
      boFound = FALSE;
    
      for (usK = 0; usK < usNbrClasses; usK++)
      {
        if (usActClass == pausSCP_List[usK])
        {
          /* Found active SCP class in S-0-1000 */
          boFound = TRUE;
        }
      }
      if (boFound == FALSE)
      {
        if (prCSMD_Instance->rExtendedDiag.usNbrSlaves < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
        {
          prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves]    = usSlaveIdx;
          prCSMD_Instance->rExtendedDiag.aeSlaveError[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = CSMD_ACT_SCP_CLASS_NOT_SUPPORTED;
          prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
        }
      }
    }
  
    /* 2. Check, if only one version of a class in S-0-1000.0.1 is present */
    for (usJ = 0; usJ < usNbrActClasses; usJ++)
    {
      usActClass = (CSMD_USHORT)(pausActSCP_List[usJ] & CSMD_MASK_SCP_TYPE);
      boFound = FALSE;
    
      for (usK = (CSMD_USHORT)(usJ + 1); usK < usNbrActClasses; usK++)
      {
        if (usActClass == (pausActSCP_List[usK] & CSMD_MASK_SCP_TYPE))
        {
          /* Found second entry with the same SCP class */
          boFound = TRUE;
        }
      }
      if (boFound == TRUE)
      {
        if (prCSMD_Instance->rExtendedDiag.usNbrSlaves < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
        {
          prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves]    = usSlaveIdx;
          prCSMD_Instance->rExtendedDiag.aeSlaveError[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = CSMD_ACT_SCP_MULTIPLE_VERSIONS;
          prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
        }
      }
    }
  }
  
  if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
  {
    return (prCSMD_Instance->rExtendedDiag.aeSlaveError[0]);
  }
  else
  {
    return (CSMD_NO_ERROR);
  }
  
} /* end: CSMD_SCP_PlausibilityCheck() */



/**************************************************************************/ /**
\brief Sets as default all active SCP classes with version 1.

\ingroup func_config
\b Description: \n
  This function takes all SCP classes with version 1 from S-0-1000 and stores
  them to the structure ausActiveSCPClasses[].
.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usSlaveIdx
              Slave Index of the selected slave.

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         21.02.2012

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_SCP_SetDefaultClasses( CSMD_INSTANCE *prCSMD_Instance,
                                          CSMD_USHORT    usSlaveIdx )
{
  
  CSMD_USHORT  usK;
  CSMD_USHORT  usNbrClasses;     /* Number of elemnts in S-0-1000     */
  CSMD_USHORT  usNbrActClasses;  /* Number of elemnts in S-0-1000.0.1 */
  CSMD_USHORT *pausSCP_List;
  CSMD_USHORT *pausActSCP_List;
  
  
  /* Prepare default configuration for S-0-1000.0.1 */
  usNbrActClasses = 0;
  pausActSCP_List = &prCSMD_Instance->rConfiguration.parSlaveConfig[usSlaveIdx].ausActiveSCPClasses[2];
  
  usNbrClasses    = (CSMD_USHORT)(prCSMD_Instance->rConfiguration.parSlaveConfig[usSlaveIdx].ausSCPClasses[0] / 2U);
  pausSCP_List    = &prCSMD_Instance->rConfiguration.parSlaveConfig[usSlaveIdx].ausSCPClasses[2];
  
  for (usK = 0; usK < usNbrClasses; usK++)
  {
    if ((pausSCP_List[usK] & CSMD_MASK_SCP_VERSION) == CSMD_MASK_SCP_VERSION_V1)
    {
      pausActSCP_List[usNbrActClasses++] = pausSCP_List[usK];
    }
  }
  /* Set current and maximun list length */
  prCSMD_Instance->rConfiguration.parSlaveConfig[usSlaveIdx].ausActiveSCPClasses[0] = (CSMD_USHORT) (usNbrActClasses * 2);
  prCSMD_Instance->rConfiguration.parSlaveConfig[usSlaveIdx].ausActiveSCPClasses[1] = (CSMD_MAX_ENTRIES_S_0_1000 * 2);
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_SCP_SetDefaultClasses() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Evaluates the SCP type & version list and builds a class bit list.

\ingroup func_config
\b Description: \n
   Sets the bits representing the supported SCP classes in the SCP configuration
   mask of the respective slave.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_SERCOS_LIST_TOO_LONG \n
              \ref CSMD_ACT_SCP_CLASS_NOT_SUPPORTED \n
              \ref CSMD_ACT_SCP_MULTIPLE_VERSIONS \n
              \ref CSMD_BASIC_SCP_TYPE_MISMATCH \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         15.09.2008

***************************************************************************** */
CSMD_FUNC_RET CSMD_Build_SCP_BitList( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT                 usI;
  CSMD_USHORT                 usJ;
  CSMD_USHORT                 usNbrSCP_Elements;
  CSMD_USHORT                *pausSCP_List;
  CSMD_ULONG                 *paulSCP_ConfigMask;
  CSMD_SLAVE_CONFIGURATION   *parSlaveConfig;
  CSMD_SLAVE_ACTIVITY_STATUS  eStatus;
  CSMD_FUNC_RET               eFuncRet = CSMD_NO_ERROR;
  
  
  /* Slave SCP configurations already checked? */
  if (prCSMD_Instance->rPriv.boSCP_Checked == FALSE)
  {
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prCSMD_Instance->rExtendedDiag.ulIDN       = CSMD_IDN_S_0_1000;
  
    /* Points to configuration of first slave */
    parSlaveConfig = &prCSMD_Instance->rConfiguration.parSlaveConfig[0];

    /* Points to configuration bitlist of first slave */
    paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];
  
    eStatus = (CSMD_SLAVE_ACTIVITY_STATUS) (prCSMD_Instance->sCSMD_Phase / 2);

    /* Evaluate SCP type list and build bitlist */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == eStatus )
      {
        if (parSlaveConfig[usI].ausActiveSCPClasses[0])
        {
          eFuncRet = CSMD_SCP_PlausibilityCheck( prCSMD_Instance, usI );
          
          /* Build bitlist from active SCP classes (S-0-1000.0.1). */
          usNbrSCP_Elements = (CSMD_USHORT)(parSlaveConfig[usI].ausActiveSCPClasses[0] / 2);
          pausSCP_List      = &parSlaveConfig[usI].ausActiveSCPClasses[2];
        }
        else
        {
          /* Build bitlist from available SCP classes (S-0-1000). */
          usNbrSCP_Elements = (CSMD_USHORT)(parSlaveConfig[usI].ausSCPClasses[0] / 2);
          pausSCP_List      = &parSlaveConfig[usI].ausSCPClasses[2];
        }
        
        paulSCP_ConfigMask[usI] = 0;
        
        for (usJ = 0; usJ < usNbrSCP_Elements; usJ++)
        {
          /* Evaluate list element */
          switch (pausSCP_List[usJ] & CSMD_MASK_SCP_TYPE_VER)
          {
            /* SCP Type: Basic Communication Profiles */
            case CSMD_SCP_TYPE_FIXCFG:    /* FIXed ConFiGuration                    */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_FIXCFG;
              break;
            case CSMD_SCP_TYPE_VARCFG:    /* VARiable ConFiGuration                 */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_VARCFG;
              break;
            case CSMD_SCP_TYPE_FIXCFG_V2: /* FIXed ConFiGuration    V2              */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_FIXCFG_V2;
              break;
            case CSMD_SCP_TYPE_VARCFG_V2: /* VARiable ConFiGuration V2              */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_VARCFG_V2;
              break;
            case CSMD_SCP_TYPE_FIXCFG_V3: /* FIXed ConFiGuration    V3              */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_FIXCFG_V3;
              break;
            case CSMD_SCP_TYPE_VARCFG_V3: /* VARiable ConFiGuration V3              */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_VARCFG_V3;
              break;
          
            /* SCP Type: Additive function groups */
            case CSMD_SCP_TYPE_SYNC:      /* SYNChronization                        */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_SYNC;
              break;
            case CSMD_SCP_TYPE_SYNC_V2:   /* SYNChronization (tSync > tScyc)        */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_SYNC_V2;
              break;
            case CSMD_SCP_TYPE_SYNC_V3:   /* SYNChronization (tSync > tScyc)        */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_SYNC_V3;
              break;
            case CSMD_SCP_TYPE_WD:        /* WatchDog                               */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_WD;
              break;
            case CSMD_SCP_TYPE_RTB:       /* RealTimeBits                           */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_RTB;
              break;
            case CSMD_SCP_TYPE_NRT:       /* unified communication channel (UCC)    */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_NRT;
              break;
            case CSMD_SCP_TYPE_CAP:       /* Connection Capabilities                */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_CAP;
              break;
            case CSMD_SCP_TYPE_SYSTIME:   /* set Sercos Time using MDT extended field */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_SYSTIME;
              break;
            case CSMD_SCP_TYPE_NRTPC:     /* unified communication channel & IP settings */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_NRTPC;
              break;
            case CSMD_SCP_TYPE_CYC:       /* Cyclic communication                   */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_CYC;
              break;
            case CSMD_SCP_TYPE_WDCON:     /* WatchDog (with timeout & data losses)  */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_WDCON;
              break;
            case CSMD_SCP_TYPE_SWC:     /* Support of Industrial Ethernet protocols via UCC */
              paulSCP_ConfigMask[usI] |= CSMD_SCP_SWC;
              break;
          
            /* Unknown or not used SCP type / version */
            default:
              break;
          }
        } /* end: for (usJ = 0; ... */
      
        if (   /* Only one basic SCP class type is allowed */
               (   (paulSCP_ConfigMask[usI] & CSMD_SCP_FIXCFG_ALL)
                && (paulSCP_ConfigMask[usI] & CSMD_SCP_VARCFG_ALL))
               /* At least one basic SCP class shall be exist */
            || !(paulSCP_ConfigMask[usI] & CSMD_SCP_BASIC))
        {
          if (prCSMD_Instance->rExtendedDiag.usNbrSlaves < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
          {
            prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves]    = usI;
            prCSMD_Instance->rExtendedDiag.aeSlaveError[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = CSMD_BASIC_SCP_TYPE_MISMATCH;
            prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
          }
        }
      }
    }
    if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
    {
      eFuncRet = prCSMD_Instance->rExtendedDiag.aeSlaveError[0];
    }
  }
  return (eFuncRet);
  
} /* end: CSMD_Build_SCP_BitList() */



/**************************************************************************/ /**
\brief Checks whether all slaves with SCP_sync support the parameter S-0-1036.

\ingroup func_config
\b Description: \n
   If not all slaves with SCP_Sync support the IDN S-0-1036,
   the flag prCSMD_Instance->boIFG_V1_3 will be set to FALSE.

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
              \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         30.08.2011

***************************************************************************** */
CSMD_FUNC_RET  CSMD_Check_S1036( CSMD_INSTANCE          *prCSMD_Instance,
                                 CSMD_FUNC_STATE        *prFuncState,
                                 CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_USHORT    usI;
  CSMD_USHORT    usFinished;
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  CSMD_ULONG    *paulSCP_ConfigMask;

  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    prFuncState->ulSleepTime = 0UL;
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prCSMD_Instance->rExtendedDiag.ulIDN       = CSMD_IDN_S_0_1036;
    
    if (prCSMD_Instance->rSlaveList.usNumProjSlaves == 0)
    {
      prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
      
      return (CSMD_NO_ERROR);
    }
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    
    prFuncState->usActState = CSMD_FUNCTION_STEP_1;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_1:
    
    usFinished = 1U;
    /* Points to configuration bitlist of first slave */
    paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];

    /* For all slaves with SCP_Sync:                */
    /* Check if S-0-1036 "Inter Frame Gap" is there.*/
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
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
              eFuncRet = CSMD_ReadSVCH( prCSMD_Instance, &parSvcMacro[usI], NULL );
            
              if ( !(   (eFuncRet == CSMD_NO_ERROR) 
                     || (eFuncRet == CSMD_INTERNAL_REQUEST_PENDING)) )
              {
                if (   (eFuncRet == CSMD_SVC_ERROR_MESSAGE) 
                    && (parSvcMacro[usI].usSvchError == (CSMD_USHORT) CSMD_SVC_ID_NOT_THERE) )
                {
                  /* Not all slaves with SCP_Sync supports S-0-1036 */
                  prCSMD_Instance->boIFG_V1_3 = FALSE;
                  eFuncRet = CSMD_NO_ERROR;
                }
                else
                {
                  CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
                }
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
      prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_2:
    
    prFuncState->usActState = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);
    
    
  default:
    return (CSMD_ILLEGAL_CASE);
    
  }   /* End: switch (prFuncState->usActState) */
  
  return (CSMD_FUNCTION_IN_PROCESS);
  
} /* end: CSMD_Check_S1036() */

/*! \endcond */ /* PRIVATE */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
19 Nov 2014 WK
  - Defdb00174985
    Added function CSMD_PrepareCYCCLK_2(). The delay is related
    to the Sercos cycle.
16 Jan 2015 WK
  - Defdb00175997
    Added function CSMD_Get_TSref().
22 Jan 2015 WK
  - Convert simple HTML tables into "markdown" extra syntax to make it
    more readable in the source code.
  - Fixed description of CSMD_PrepareCYCCLK() and CSMD_PrepareCYCCLK_2().
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

------------------------------------------------------------------------------
*/
