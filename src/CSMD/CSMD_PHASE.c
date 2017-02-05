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
\file   CSMD_PHASE.c
\author WK
\date   01.09.2010
\brief  This file contains the public API functions and private functions
        for the communication state machine.

>todo   separate functions set and check (???)
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#include "CSMD_BM_CFG.h"
#include "CSMD_CALC.h"      /* wg CSMD_MEDIA_LAYER_OVERHEAD */ 
#include "CSMD_CP_AUX.h"
#include "CSMD_DIAG.h"
#include "CSMD_FRCFG_TX.h"
#include "CSMD_FRCFG_RX.h"
#include "CSMD_PHASEDEV.h"
#include "CSMD_PRIV_SVC.h"
#include "CSMD_TOPOLOGY.h"

#define SOURCE_CSMD
#include "CSMD_PHASE.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/**************************************************************************/ /**
\brief Switches the Sercos master to Non-Real-Time Mode.

\ingroup func_phase
\b Description: \n
   This function switches the Sercos master to NRT state. The communication
   with Sercos telegrams is deactivated.

<B>Call Environment:</B> \n
   This function can be called directly after the basic initialization of
   the Sercos controller or after CSMD_SetPhase0().\n
   The function can be called from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         24.10.2005
                          
***************************************************************************** */
CSMD_FUNC_RET CSMD_Set_NRT_State( CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_USHORT   usI;
  CSMD_ULONG    ulIntEnable;
  CSMD_ULONG    ulIntOutputMask;
  CSMD_ULONG    ulDFCSR;
  
  
  /* --------------------------------------------------------- */
  /* Step 1 :: Check current communication phase               */
  /* --------------------------------------------------------- */
  if (prCSMD_Instance->sCSMD_Phase > CSMD_SERC_PHASE_0 )
  {
    return (CSMD_WRONG_PHASE);
  }
  
  if (*prCSMD_Instance->rPriv.rCbFuncTable.S3Event != NULL)
  {
    /* Notification to the IP-Driver: End driver shall terminate receive/transmit non Sercos telegrams */
    prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_STOP_COMMUNICATION;

    (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.S3Event)
      ( prCSMD_Instance->rPriv.pvCB_Info,             /* distinguish the instance */
        &prCSMD_Instance->rPriv.rSercEvent );         /* Sercos event information */
  }
  
  /* copying of cyclic Sercos data / ring monitoring is disabled */
  prCSMD_Instance->rPriv.eMonitoringMode    = CSMD_MONITORING_OFF;
  prCSMD_Instance->rPriv.boP1_active        = FALSE;
  prCSMD_Instance->rPriv.boP2_active        = FALSE;
  
  for (usI = 0; usI < CSMD_MAX_TEL; usI++ )
  {                                                
    prCSMD_Instance->rPriv.aboMDT_used[usI] = FALSE;    /* MDT telegram is not used */
    prCSMD_Instance->rPriv.aboAT_used[usI]  = FALSE;    /* AT telegram is not used */
  }
  
  prCSMD_Instance->usCSMD_Topology               = CSMD_NO_LINK;   /* flag for topology */  
  prCSMD_Instance->sCSMD_Phase                   = CSMD_SERC_PHASE_NRT_STATE;
  
  /* Used FPGA TxRam memory for Sercos telegrams */
  prCSMD_Instance->rCSMD_HAL.ulTxRamInUse = 0;
  /* Used FPGA RxRam memory for Sercos telegrams */
  prCSMD_Instance->rCSMD_HAL.ulRxRamInUse = 0;
  
  CSMD_HAL_SetPhase( &prCSMD_Instance->rCSMD_HAL,
                     CSMD_CPS_CURRENT_CP,
                     CSMD_SERC_PHASE_0 );
  
  
  CSMD_HAL_GetIntControl( &prCSMD_Instance->rCSMD_HAL,
                          &ulIntEnable,
                          TRUE,
                          FALSE,
                          &ulIntOutputMask );
  
  /* Block event interrupts other than UC channel interrupts */
  CSMD_HAL_IntControl( &prCSMD_Instance->rCSMD_HAL, 
                       ulIntEnable & (CSMD_ULONG)CSMD_INT_IPINT,
                       TRUE,
                       FALSE,
                       ulIntOutputMask );
  
  CSMD_HAL_IntControl( &prCSMD_Instance->rCSMD_HAL, 
                       0,
                       FALSE,
                       TRUE,
                       0 );
   
  
  /* Descriptor unit disabled */
  /* No reason to delete Rx-Tx-Descriptors or events! */
  CSMD_HAL_CtrlDescriptorUnit(&prCSMD_Instance->rCSMD_HAL, FALSE, FALSE);
  
  /* Timer Off / Timing Master Mode / no CYC_CLK / no CON_CLK */
  CSMD_HAL_Initialize(&prCSMD_Instance->rCSMD_HAL);
  
  
  {
    ulDFCSR = CSMD_HAL_GetComMode(&prCSMD_Instance->rCSMD_HAL);
    
    /* set non-realtime mode */
    if (   (ulDFCSR == CSMD_HAL_DFCSR_TOPOLOGY_RT_RING_MODE)
        || (ulDFCSR == CSMD_HAL_DFCSR_TOPOLOGY_UC_RING_MODE))
    {
      CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_UC_RING_MODE);
    }
    else
    {
      CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_UC_LINE_MODE);
    }
  }
  
  if (*prCSMD_Instance->rPriv.rCbFuncTable.RxTxRamAlloc != NULL)
  {
    /* Notification to IP-Driver for changing in Sercos Ram allocation */
    (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.RxTxRamAlloc)
      ( prCSMD_Instance->rPriv.pvCB_Info,           /* distinguish the instance */
        prCSMD_Instance->rCSMD_HAL.ulTxRamInUse,    /* Tx-Ram Start Address for IP frames */
        prCSMD_Instance->rCSMD_HAL.ulSizeOfTxRam,   /* Size in Tx-Ram for IP frames */
        prCSMD_Instance->rCSMD_HAL.ulRxRamInUse,    /* Rx-Ram Start Address for IP frames */
        prCSMD_Instance->rCSMD_HAL.ulSizeOfRxRam ); /* Size in Rx-Ram for IP frames */
  }
  
  if (*prCSMD_Instance->rPriv.rCbFuncTable.S3Event != NULL)
  {
    /* Notification to the IP-Driver: End driver shall activate receive/transmit non Sercos telegrams */
    prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_START_COMMUNICATION;

    (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.S3Event)
      ( prCSMD_Instance->rPriv.pvCB_Info,             /* distinguish the instance */
        &prCSMD_Instance->rPriv.rSercEvent );         /* Sercos event information */
  }
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Set_NRT_State() */


/**************************************************************************/ /**
\brief Enables or disables forwarding of UC telegrams if master is in UC state.

\ingroup func_UC
\b Description: \n
   This function modifies the collision buffer setting in UC mode. It is possible
   to either enable collision buffers for both master ports, to enable collision
   buffer of one master port only, or to disable them both. As soon as one master
   ports is required to forward non-sercos telegrams, collision buffer in DFCSR
   register is enabled. Port-specific collision buffers are controlled by the
   respective IPCSR registers.
   This function enables the application to control the master collision buffer
   in UC mode before Sercos communication has started.

<B>Call Environment:</B> \n
   This function can only be called if the master is in UC mode.\n
   The function can be called from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   boEnablePort1
              TRUE - Enable collision buffer for port 1
              FALSE - Disable collision buffer for port 1

\param [in]   boEnablePort2
              TRUE - Enable collision buffer for port 2
              FALSE - Disable collision buffer for port 2

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_NO_ERROR \n

\author       AlM
\date         25.06.2014

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_Control_UC( CSMD_INSTANCE *prCSMD_Instance,
                               CSMD_BOOL      boEnablePort1,
                               CSMD_BOOL      boEnablePort2 )
{
  CSMD_FUNC_RET eFuncRet;
  CSMD_ULONG    ulIPCSR1;
  CSMD_ULONG    ulIPCSR2;

  /* exit function if master is not in UC mode */
  if (CSMD_SERC_PHASE_NRT_STATE == prCSMD_Instance->sCSMD_Phase)
  {
    /* Set communication mode in DFCSR register depending on control bits for both master ports.
     * If any master port shall forward non-sercos telegrams, collision buffer is enabled */
    if (boEnablePort1 || boEnablePort2)
    {
      CSMD_HAL_SetComMode( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_UC_LINE_MODE);
    }
    else
    {
      CSMD_HAL_SetComMode( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_UC_RING_MODE);
    }

    /* Modify collision buffer in IPCSR register for both master ports depending on control bits. */
    ulIPCSR1 = CSMD_HAL_ReadShort( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->rIPCSR1.us.IPCR );
    ulIPCSR2 = CSMD_HAL_ReadShort( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->rIPCSR2.us.IPCR );
    if (boEnablePort1)
    {
      ulIPCSR1 &= ~CSMD_HAL_IPCR_COL_BUF_DISABLE;
    }
    else
    {
      ulIPCSR1 |= CSMD_HAL_IPCR_COL_BUF_DISABLE;
    }
    if (boEnablePort2)
    {
      ulIPCSR2 &= ~CSMD_HAL_IPCR_COL_BUF_DISABLE;
    }
    else
    {
      ulIPCSR2 |= CSMD_HAL_IPCR_COL_BUF_DISABLE;
    }
    CSMD_HAL_WriteShort( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->rIPCSR1.us.IPCR, ulIPCSR1 );
    CSMD_HAL_WriteShort( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->rIPCSR2.us.IPCR, ulIPCSR2 );

    eFuncRet = CSMD_NO_ERROR;
  }
  else
  {
    eFuncRet = CSMD_WRONG_PHASE;
  }
  return (eFuncRet);

} /* end: CSMD_Control_UC() */
/*lint -restore const! */


/**************************************************************************/ /**
\brief This function performs a communication phase switch to CP0.

\ingroup func_phase
\b Description: \n
   This function performs a communication phase switch to CP0, including:
   - configuration of the Sercos controller for CP0
   - check whether the ring is closed
   - determination of the ring delay times
   - validation of a stable Sercos topology by monitoring the AT content
     over 100 query cycles
   - check whether all detected slaves support the signaled requested functions
   
   The function provides a Sercos list containing the Sercos addresses of the
   slaves found in the ring. The order of the list corresponds to the topology
   (In a broken ring, the slaves recognized at port 2 are added to the slaves of
   port 1 in an inverted form. This is done in order to maintain the order after
   the interruption is canceled.) The topology addresses are generated out of
   this list in ascending order starting from 1. If a Sercos address is recognized
   more than once, this function returns the error message 
   CSMD_ERROR_DOUBLE_RECOGNIZED_ADDRESS. In this case, a communication phase switch to CP2
   is possible in order to then assign the concerned slaves a unique address
   via the service channel. The access index to the service channel is then 
   determined by the list ausRecogSlaveAddList[]. When all slaves have a unique
   Sercos address, a new phase start-up is only possible via CP0.\n\n
   
   After call-up of the function, the usCSMD_Topology variable in the rCSMD_Instance
   structure specifies the found topology of the Sercos connection as follows:\n
   - 0x0000 = CSMD_NO_LINK\n
   - 0x0001 = CSMD_TOPOLOGY_LINE_P1\n
   - 0x0002 = CSMD_TOPOLOGY_LINE_P2\n
   - 0x0003 = CSMD_TOPOLOGY_BROKEN_RING\n
   - 0x0004 = CSMD_TOPOLOGY_RING\n
   - 0x0008 = CSMD_TOPOLOGY_DEFECT_RING\n

   The default topology can be defined in the variable usRequired_Topology in 
   the structure rCSMD_Instance:\n
   If a ring topology is broken at Port 1, there is a 'Line Port 2' topology detected
   during phase start-up. The detected slaves are listed in ausRecogSlaveAddList[ ] 
   in reverse order compared to topology 'Ring'.\n
   If this behavior is not intended, the default topology 'ring' can be
   disapproved in usRequired_Topology. In this case, when topology 'Line Port 2'
   is detected, the list is assembled in the same way as in case of topology 'Ring'.
   This behavior is deactivated by default!

   <B>The following requested functions are supported:</B>
   - Timing variant in CP1/CP2
     - Method according to Sercos specification 1.3.1 or higher\n
       - Method 1 (MDT/AT/UCC) or \n Method 2 (MDT/UCC/AT)\n
         Configuration of a UCC with maximum width.\n
       - Method 1 with variable  UCC\n
         Configuration of a centered UCC with given width in CSMD_HW_INIT_STRUCT ulUCC_Width.\n

       The feature is activated, when all detected slaves support this feature. 
       If not, the function CSMD_SetPhase0() is terminated with error message CSMD_CP0_COM_VER_CHECK,
       if no other error occurs.
       In order to reach CP0, CSMD_SetPhase0() has to be called again using an adapted configuration of requested functions.

     - Fixed Timing\n
       Fixed UCC according to Sercos specification 1.3.0 or lower
     . 
     The timing variant in <B>CSMD_HW_INIT_STRUCT.eUCC_Mode_CP12</B> has to be configured before calling
     CSMD_SetPhase0().

   - CP switch Mode
     - Fast CP switch according to Sercos specification 1.3.1 or higher\n
       Reduction of the transmission break (maximum 20ms) when switching phases. The feature is
       activated, when all detected slaves support this feature. 
       If not, the function CSMD_SetPhase0() is terminated with error message CSMD_CP0_COM_VER_CHECK,
       if no other error occurs.
       In order to reach CP0, CSMD_SetPhase0() has to be called again using an adapted configuration of requested functions.
       
     - CP switch mode according to Sercos specification 1.3.0 or lower\n
       Transmission break of ca. 120 ms when switching phases
     .
     The CP switch mode in <B>CSMD_HW_INIT_STRUCT.boFastCPSwitch</B> has to be configured before calling
     CSMD_SetPhase0().

   - Forwarding of Sercos telegrams from CP1
     - No forwarding according to Sercos specification 1.3.1 or higher\n
       Stop sending of Sercos telegrams at last slave in line topology \n
       to support the connection of Industrial Ethernet Devices (e.g. EtherNet/IP).
       The feature is activated, when all detected slaves support this feature. 
       If not, the function CSMD_SetPhase0() is terminated with error message CSMD_CP0_COM_VER_CHECK,
       if no other error occurs.
       In order to reach CP0, CSMD_SetPhase0() has to be called again using an adapted configuration of requested functions.

     - Forwarding according to Sercos specification 1.3.0 or lower\n
       The last slave in line topology forwards Sercos telegrams in all Sercos CP.
     .
     The forwarding mode in <B>CSMD_HW_INIT_STRUCT.boTelOffLastSlave</B> has to be configured before calling
     CSMD_SetPhase0().
       
(reference to \ref init_str_sec1)

  <B>Call Environment:</B> \n
   Before the call-up of the CSMD_SetPhase0() function, the basic initialization of
   the Sercos controller must have been completed.\n
   The call-up should be performed from a task.\n
   The function comprises a state machine.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [out]  pusRecognizedSlaveList[]
              Output pointer to Sercos list with recognized slaves
\param [in]   eComVersion
              Desired Com-Version defined through enum CSMD_COM_VERSION --> no longer in use! 
        
\return       \ref CSMD_INVALID_SERCOS_CYCLE_TIME \n
              \ref CSMD_ERROR_TIMEOUT_P0 \n
              \ref CSMD_NO_COMMUNICATION_P0 \n
              \ref CSMD_NO_RAM_MIRROR_ALLOCATED \n
              \ref CSMD_ERROR_PHASE_CHANGE_CHECK \n
              \ref CSMD_ILLEGAL_SLAVE_ADDRESS \n
              \ref CSMD_ERROR_DOUBLE_RECOGNIZED_ADDRESS \n
              \ref CSMD_LOOP_NOT_CLOSED \n
              \ref CSMD_INCONSISTENT_RING_ADDRESSES \n
              \ref CSMD_CP0_COM_VER_CHECK \n
              \ref CSMD_NO_STABLE_TOPOLOGY_IN_CP0 \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_EVENT_TIME_MAX_LIMIT \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n
                          
\author       WK
\date         01.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_SetPhase0( CSMD_INSTANCE     *prCSMD_Instance,
                              CSMD_FUNC_STATE   *prFuncState,
                              CSMD_USHORT      **pusRecognizedSlaveList,
                              CSMD_COM_VERSION   eComVersion )
{
  
  CSMD_FUNC_RET eFuncRet = CSMD_FUNCTION_IN_PROCESS;    /* Function processing still active */
  CSMD_USHORT   usI;
  CSMD_ULONG    ulDFCSR = 0;
  
  
  *pusRecognizedSlaveList = NULL;
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_COPY_ONLY;

    prFuncState->ulSleepTime = 0UL;
    CSMD_RuntimeError( prCSMD_Instance, 0, "" );
    
    /* Reset FPGA counters for frames with FCS errors or are misaligned */
    CSMD_ResetSercosErrorCounter( prCSMD_Instance );
    
#ifdef CSMD_DEBUG
    (CSMD_VOID) CSMD_HAL_memset( &prCSMD_Instance->rCSMD_Debug,
                                 0,
                                 sizeof (prCSMD_Instance->rCSMD_Debug) );
#endif
    
    /* Take the hardware specific settings */
    if (CSMD_NO_ERROR != (eFuncRet = CSMD_GetHW_Settings( prCSMD_Instance, FALSE )) )
    {
      return (eFuncRet);
    }
    else
    {
      prCSMD_Instance->rPriv.ulUCC_Width = prCSMD_Instance->rPriv.rHW_Init_Struct.ulUCC_Width;
      eFuncRet = CSMD_FUNCTION_IN_PROCESS;
    }

#ifdef CSMD_PCI_MASTER
    /* Assume DMA mode for CP0 to CP4 ! */
    if (prCSMD_Instance->rPriv.rHW_Init_Struct.boPciMode == TRUE)
    {
      prCSMD_Instance->rPriv.boDMA_IsActive = TRUE;
    }
    else
    {
      prCSMD_Instance->rPriv.boDMA_IsActive = FALSE;
    }
    if (   (prCSMD_Instance->prTelBuffer == NULL)
        || (   (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
            && (prCSMD_Instance->prTelBuffer_Phys == NULL)))
#else
    if (prCSMD_Instance->prTelBuffer == NULL)
#endif
    {
      return (CSMD_NO_RAM_MIRROR_ALLOCATED);
    }
    
    /* ------------------------------------------------------------- */
    /* Clear C-DEV "Master valid" of all configurable slaves         */
    /* ------------------------------------------------------------- */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
    {
      prCSMD_Instance->rPriv.ausDevControl[usI] &= (CSMD_USHORT)~CSMD_C_DEV_MASTER_VALID;
    }
    
#ifdef CSMD_SWC_EXT
    {
      CSMD_FUNC_RET eRet;
      /* Check for valid Sercos cycle time for CP1 and CP2 */
      eRet = CSMD_CheckCycleTime_CP12( prCSMD_Instance,
                                       prCSMD_Instance->rPriv.rHW_Init_Struct.ulCycleTime_CP12 );

      if (eRet != CSMD_NO_ERROR)
      {
        return (eRet);
      }
    }
#endif
    /* Get Sercos cycle time for CP0 */
    prCSMD_Instance->rPriv.rTimingCP12.ulCycleTime_CP0 =
      prCSMD_Instance->rPriv.rHW_Init_Struct.ulCycleTime_CP0;
    
    /* Check for valid Sercos cycle time for CP0 */
    if (   (prCSMD_Instance->rPriv.rTimingCP12.ulCycleTime_CP0 < CSMD_TSCYC_1_MS)
        || (prCSMD_Instance->rPriv.rTimingCP12.ulCycleTime_CP0 % CSMD_TSCYC_250_US)
        || (prCSMD_Instance->rPriv.rTimingCP12.ulCycleTime_CP0 > CSMD_TSCYC_MAX) )
    {
      return (CSMD_INVALID_SERCOS_CYCLE_TIME);
    }
    
    /* Wait at least 1 Sercos cycle to set the C-DEV's */
    prFuncState->ulSleepTime = (CSMD_ULONG)((prCSMD_Instance->rPriv.ulActiveCycTime + 1000000)/1000000);
    prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
    break;
    
    
  case CSMD_FUNCTION_STEP_1:
    
    /* --------------------------------------------------------- */
    /* Step 4 :: Finish Sercos current communication phase       */
    /* --------------------------------------------------------- */
    if (prCSMD_Instance->sCSMD_Phase != CSMD_SERC_PHASE_NRT_STATE)
    {
      /* Entry from CP1 till CP4 */
      CSMD_HAL_SetPhase( &prCSMD_Instance->rCSMD_HAL,
                         CSMD_CPS_NEW_CP,
                         CSMD_SERC_PHASE_0 );
      
      prCSMD_Instance->sCSMD_Phase = CSMD_SERC_PHASE_NRT_STATE;
    }
    else
    {
      /* Entry from NRT state */
      CSMD_HAL_SetPhase( &prCSMD_Instance->rCSMD_HAL,
                         CSMD_CPS_CURRENT_CP,
                         CSMD_SERC_PHASE_0 );
      
      /* No fast switch to CP0 after entry from NRT state or CoSeMa initialization */
      prCSMD_Instance->rPriv.rSWC_Struct.boFastCPSwitchActive = FALSE;
    }
    
#ifdef CSMD_HOTPLUG
    prCSMD_Instance->rPriv.rHotPlug.usHP_Phase      = CSMD_NON_HP_MODE;
    prCSMD_Instance->rPriv.rHotPlug.boHotPlugActive = FALSE;
#endif
    /* Initialize timer for HP transmission of t6/t7 in CP3 */
    prCSMD_Instance->rPriv.usTimerHP_CP3 = 0xFFFF;
    
    
    /* Delete MDT send jitter (Sync_jitter) */
    prCSMD_Instance->rConfiguration.rMasterCfg.ulJitter_Master = 0U;
    
    /* Initialize jitter of Sercos interface with default value */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
    {
      prCSMD_Instance->rConfiguration.parSlaveConfig[usI].ausActiveSCPClasses[0] = 0;
      prCSMD_Instance->rConfiguration.parSlaveConfig[usI].ausActiveSCPClasses[1] = (CSMD_MAX_ENTRIES_S_0_1000 * 2);

      prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTiming.usSlaveJitter_S1037 =
        (CSMD_USHORT) CSMD_JITTER_SLAVE;
    }
    
    /* Set flag to 'Slave SCP configurations not checked' */
    prCSMD_Instance->rPriv.boSCP_Checked = FALSE;
    
#ifdef CSMD_SWC_EXT
    /* The settings for the requested functions (communication version field) 
       are taken from the CSMD_HW_INIT_STRUCT elements.                       */
#else
    /* Use default settings */
    prCSMD_Instance->rPriv.rSWC_Struct.boNoForwardLastSlave = FALSE;
    prCSMD_Instance->rPriv.rSWC_Struct.boFastCPSwitchActive = FALSE;
    prCSMD_Instance->rPriv.rSWC_Struct.eActiveUCC_Mode_CP12 = CSMD_UCC_MODE_CP12_FIX;
#endif
    
    /* Always remote address allocation ! */
    eComVersion = CSMD_COMVERSION_V1_0;
    /* apply address mode */
    (CSMD_VOID) CSMD_SetComVersion( prCSMD_Instance, eComVersion );
    
    /* Calculate Timing for the UC channel in CP1 and CP2 */
    CSMD_Calculate_Timing_CP1_CP2( prCSMD_Instance );
    
    /* Timeout slave check = 200 ms */
    prCSMD_Instance->rPriv.lOutTimer = 1;
    
    prFuncState->ulSleepTime = CSMD_CPS_MAX_TIMEOUT;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    break;
    
    
  case CSMD_FUNCTION_STEP_2:
    /* --------------------------------------------------------- */
    /* Step 5 :: Finish Sercos current communication phase       */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Finish_Phase_Check( prCSMD_Instance, 
                                        prCSMD_Instance->sCSMD_Phase );
    
    prCSMD_Instance->rPriv.lOutTimer--;
    
    if (   (eFuncRet == CSMD_NO_ERROR)
        || (prCSMD_Instance->rPriv.lOutTimer <= 0))
    {
      /* Continue if check successful or after timeout */
      /* copying of cyclic Sercos data / ring monitoring is disabled */
      prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_OFF;
      
      prCSMD_Instance->usEnableTel = 0U;          /* Not ready to enable CYC_CLK */
      /* Disable Central Timers */
      CSMD_HAL_CtrlSystemCounter( &prCSMD_Instance->rCSMD_HAL, 
                                  FALSE,
                                  FALSE,
                                  FALSE );
      
      /* wait for current telegram sent before reconfigure descriptor unit */
      prFuncState->ulSleepTime = 
        (prCSMD_Instance->rPriv.ulActiveCycTime + 1000000 - 1) / 1000000;
      prFuncState->usActState = CSMD_FUNCTION_CHG_CLK_POSSIBLE;
    }
    eFuncRet = CSMD_FUNCTION_IN_PROCESS;
    
    break;
    
    
  case CSMD_FUNCTION_CHG_CLK_POSSIBLE:
    /* ------------------------------------------------------------ */
    /*   Step 1 ::  Setting for CP0                                 */
    /* ------------------------------------------------------------ */
    if (*prCSMD_Instance->rPriv.rCbFuncTable.S3Event != NULL)
    {
      /* Notification to the IP-Driver: End driver shall terminate receive/transmit non Sercos telegrams */
      prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_STOP_COMMUNICATION;
    
      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.S3Event)( prCSMD_Instance->rPriv.pvCB_Info,
                                                                 &prCSMD_Instance->rPriv.rSercEvent );
    }

    /* switch off Sercos transmission for both ports */
    CSMD_HAL_CtrlDescriptorUnit( &prCSMD_Instance->rCSMD_HAL, FALSE, FALSE );
    
    ulDFCSR = CSMD_HAL_GetComMode(&prCSMD_Instance->rCSMD_HAL);
    
    /* set realtime mode */
    if (   (ulDFCSR == CSMD_HAL_DFCSR_TOPOLOGY_RT_RING_MODE) 
        || (ulDFCSR == CSMD_HAL_DFCSR_TOPOLOGY_UC_RING_MODE))
    {
      CSMD_HAL_SetComMode( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_RING_MODE );
    }
    else
    {
      CSMD_HAL_SetComMode( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P12_MODE );
    }
    
    /* Disable IP-Core SVC state machine */
    CSMD_HAL_CtrlSVCMachine( &prCSMD_Instance->rCSMD_HAL, FALSE );
  
    /* Disable all FPGA SVC */
    for (usI = 0; usI < CSMD_NUMBER_OF_SVC; usI++)
    {
      CSMD_HAL_WriteShort( &prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram->uwSVC_Pointer[usI], 
                           0U );
    }
  
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
    {
      /* Clear Service channel management structures for atomic function call */
      (CSMD_VOID)CSMD_ResetSVCH( prCSMD_Instance, usI, NULL );
      /* Reset S-DEV.SlaveValid error counter */
      prCSMD_Instance->arDevStatus[usI].usS_Dev = 0;
      prCSMD_Instance->arDevStatus[usI].usMiss = 0;
    }
    
    prCSMD_Instance->rPriv.ulActiveCycTime = 
      prCSMD_Instance->rPriv.rTimingCP12.ulCycleTime_CP0;
    
    eFuncRet = CSMD_Set_Register_P0( prCSMD_Instance );     /* Set register preparing CP0 */
    
    if (eFuncRet != CSMD_NO_ERROR)
    {
      return (eFuncRet);
    }
    
    prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_3;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_3:
    /* ---------------------------------------------------------------- */
    /* Step 2: Configuration of transmit telegram for CP0               */
    /* ---------------------------------------------------------------- */
    eFuncRet = CSMD_Config_TX_Tel_P0( prCSMD_Instance );
    if (eFuncRet != CSMD_NO_ERROR) 
    {
      break;
    }
    else
    {
      eFuncRet = CSMD_FUNCTION_IN_PROCESS;
    }
    prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_4;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_4:
    /* ---------------------------------------------------------------- */
    /* Step 3: Configuration of AT for CP0                              */
    /* ---------------------------------------------------------------- */
    eFuncRet = CSMD_Config_RX_Tel_P0( prCSMD_Instance );
    if (eFuncRet != CSMD_NO_ERROR) 
    {
      break;
    }
    else
    {
      eFuncRet = CSMD_FUNCTION_IN_PROCESS;
    }
    /* FPGA Debug Outputs: Pin1 = Port1 MST;  Pin2 = Port 2 MST */
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulDBGOCR,
                        (CSMD_ULONG)((CSMD_DBGOCR_PORT2_MST << 8) + CSMD_DBGOCR_PORT1_MST) );
    
    if (*prCSMD_Instance->rPriv.rCbFuncTable.RxTxRamAlloc != NULL)
    {
      /* Notification to IP-Driver for changing in Sercos Ram allocation */
      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.RxTxRamAlloc)
        ( prCSMD_Instance->rPriv.pvCB_Info,                  /* distinguish the instance */
          prCSMD_Instance->rCSMD_HAL.ulTxRamInUse,           /* Tx-Ram Start Address for IP frames */
          prCSMD_Instance->rCSMD_HAL.ulSizeOfTxRam,          /* Size in Tx-Ram for IP frames */
          prCSMD_Instance->rCSMD_HAL.ulRxRamInUse,           /* Rx-Ram Start Address for IP frames */
          prCSMD_Instance->rCSMD_HAL.ulSizeOfRxRam );        /* Size in Rx-Ram for IP frames */
    }
    
#ifdef CSMD_SWC_EXT
    if (prCSMD_Instance->rPriv.rSWC_Struct.boFastCPSwitchActive == TRUE)
    {
      /* Normally dont't wait */
      /* For sercans Testmaster there shall be a break between step CSMD_FUNCTION_CHG_CLK_POSSIBLE and CSMD_FUNCTION_STEP_5. */
      prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    }
    else
#endif
    {
      /* wait at least 120ms before reconfigure slaves. */
      prFuncState->ulSleepTime  = (CSMD_ULONG) (CSMD_WAIT_120MS - prCSMD_Instance->rPriv.ulActiveCycTime/1000000);
    }
    prFuncState->usActState = CSMD_FUNCTION_STEP_5;
    break;
    
    
  case CSMD_FUNCTION_STEP_5:
    /* --------------------------------------------------------- */
    /* Step 8 :: Start of communication phase CP0                */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Start_New_Phase_Prepare( prCSMD_Instance,
                                             CSMD_SERC_PHASE_0 );
    if (eFuncRet == CSMD_NO_ERROR)
    {
      eFuncRet = CSMD_FUNCTION_IN_PROCESS;
    }
    /* copying of cyclic Sercos data is enabled, but slave valid is not checked */
    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_COPY_ONLY;
    
    if (*prCSMD_Instance->rPriv.rCbFuncTable.S3Event != NULL)
    {
      /* Notification to the IP-Driver: End driver shall activate receive/transmit non Sercos telegrams */
      prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_START_COMMUNICATION;
    
      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.S3Event)
        ( prCSMD_Instance->rPriv.pvCB_Info,             /* distinguish the instance */
          &prCSMD_Instance->rPriv.rSercEvent );         /* Sercos event information */
    }
    
#ifdef CSMD_SWC_EXT
    if (prCSMD_Instance->rPriv.rSWC_Struct.boFastCPSwitchActive == TRUE)
    {
      /* Dont't wait */
      prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    }
    else
#endif
    {
      prFuncState->ulSleepTime = CSMD_WAIT_200MS;
    }
    prFuncState->usActState = CSMD_FUNCTION_STEP_6;
    break;
    
    
  case CSMD_FUNCTION_STEP_6:
    /* ------------------------------------------------------------- */
    /* Step 4 :: Deleting the slave lists before scan                */
    /* ------------------------------------------------------------- */
    CSMD_DeleteSlaveList( prCSMD_Instance );
    
    /* Init for CSMD_Detect_Available_Slaves() */
    prCSMD_Instance->rPriv.rInternalFuncState.usActState = CSMD_FUNCTION_1ST_ENTRY;
    
    prFuncState->ulSleepTime = 0;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_7;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_7:
    /* ----------------------------------------------------------------- */
    /* Step 5: Scan for all available slaves                             */
    /*         and build recognized slave list                           */
    /* ----------------------------------------------------------------- */
    eFuncRet = CSMD_Detect_Available_Slaves( prCSMD_Instance );
    
    if (eFuncRet == CSMD_FUNCTION_IN_PROCESS)
    {
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    }
    else if (eFuncRet == CSMD_NO_ERROR)
    {
      prFuncState->ulSleepTime = CSMD_WAIT_10MS;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_8;
      eFuncRet = CSMD_FUNCTION_IN_PROCESS;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_8:
    /* ------------------------------------------------------------- */
    /* Step                                                          */
    /* ------------------------------------------------------------- */
    {
      CSMD_FUNC_RET eSlaveAckErr;
#ifdef CSMD_SWC_EXT
      eSlaveAckErr = CSMD_Check_Slave_Acknowledgment( prCSMD_Instance );
      if (eSlaveAckErr == CSMD_NO_ERROR) 
      {
        /* The specified mode is supported by all slaves */
        prCSMD_Instance->rPriv.rSWC_Struct.boNoForwardLastSlave =
          prCSMD_Instance->rPriv.rHW_Init_Struct.boTelOffLastSlave;
        prCSMD_Instance->rPriv.rSWC_Struct.boFastCPSwitchActive =
          prCSMD_Instance->rPriv.rHW_Init_Struct.boFastCPSwitch;
        prCSMD_Instance->rPriv.rSWC_Struct.eActiveUCC_Mode_CP12 =
          prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12;
      }
#else
      eSlaveAckErr = CSMD_NO_ERROR;
#endif  /* #ifdef CSMD_SWC_EXT */
      
      /* ---------------------------------------------------------------- */
      /* Build Sercos list with Sercos addresses of all recognized slave  */
      /* in topological order.                                            */
      /* ---------------------------------------------------------------- */
      eFuncRet = CSMD_Build_Recog_Slave_AddList( prCSMD_Instance );
      if (eFuncRet != CSMD_NO_ERROR) 
      {
        break;
      }
      
      /* ----------------------------------------------------------------
         - Check, if the current Topology conform with the recognized
           topology in usCSMD_Topology.
         - Check, if the current sequence counters conform with
           the number of recognized slave.
         ---------------------------------------------------------------- */
      if ( CSMD_NO_ERROR != (eFuncRet = CSMD_Check_Recognized_Topology_CP0( prCSMD_Instance )) )
      {
          return (eFuncRet);
      }

      /* return pointer to list */
      *pusRecognizedSlaveList = prCSMD_Instance->rSlaveList.ausRecogSlaveAddList;
      
      eFuncRet = CSMD_Check_Slave_Addresses( prCSMD_Instance );
      
      if (eSlaveAckErr != CSMD_NO_ERROR)
      {
        /* Prefer error message from slave acknowledgment check */
        eFuncRet = eSlaveAckErr;
      }
    }
    prFuncState->ulSleepTime = 0;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    break;
    
    
  default:
    eFuncRet = CSMD_ILLEGAL_CASE;
    break;    
    
  }   /* End: switch (prFuncState->usActState) */
  
  
  if (   (prCSMD_Instance->rPriv.ulUCC_Width)
      && (eFuncRet != CSMD_FUNCTION_IN_PROCESS))
  {
    if (   (prCSMD_Instance->rPriv.boP1_active == FALSE)
        && (prCSMD_Instance->rPriv.boP2_active == FALSE))
    {
      /* switch off Sercos transmission for both port */
      CSMD_HAL_CtrlDescriptorUnit(&prCSMD_Instance->rCSMD_HAL, FALSE, FALSE);
      
      /* set non realtime mode */
      if (   (ulDFCSR == CSMD_HAL_DFCSR_TOPOLOGY_RT_RING_MODE)
          || (ulDFCSR == CSMD_HAL_DFCSR_TOPOLOGY_UC_RING_MODE))
      {
        CSMD_HAL_SetComMode( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_UC_RING_MODE );
      }
      else
      {
        CSMD_HAL_SetComMode( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_UC_LINE_MODE );
      }
    }
    else
    {
      switch (prCSMD_Instance->usCSMD_Topology)
      {
      case CSMD_TOPOLOGY_LINE_P1:
        CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P1_MODE);
        break;
        
      case CSMD_TOPOLOGY_LINE_P2:
        CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P2_MODE);
        break;
        
      case CSMD_TOPOLOGY_BROKEN_RING:
        CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P12_MODE);
        break;
        
      default:
        /* Default value: CSMD_TOPOLOGY_RING */
        CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_RING_MODE);
        break;
      }
    }
  }
  
  return (eFuncRet);
  
} /* end: CSMD_SetPhase0() */



/**************************************************************************/ /**
\brief This function performs a communication phase switch to CP1.

\ingroup func_phase
\b Description: \n
   This function performs a communication phase switch to CP1, including:
   - configuration of the Sercos controller for CP1
   - initiation of CP1
   - initialization of the service channels
   - scanning of all slaves recognized in the ring
   - determination of ring run time

<B>Call Environment:</B> \n
   Before the call-up of the CSMD_SetPhase1 function, CP0 must have been
   achieved successfully.\n
   The call-up should be performed from a task.\n
   The function comprises a state machine.\n

   This function must be provided with a Sercos list containing the Sercos
   addresses of all recognized slaves in the ring. Regarding the possibility
   of projected hot plug slaves, there are two different cases determining
   the operations done in this function:\n\n

   - Only actually existing slaves are to be configured:\n\n
     In this case, the list of recognized slaves of the CSMD_SetPhase1()
     function can be directly transmitted as list of the projected slaves.\n\n
   - Only slaves that cannot be found during the start-up are to be configured:\n\n
     In addition to the actually existing slaves, the transferred list of 
     projected slaves may also contain more slaves (Hot Plug slaves).
     In order to be able to calculate the telegram length and the timing,
     CoSeMa must know the configuration of these Hot Plug slaves before
     CSMD_CalculateTiming() is called. If this information is missing, this
     function will generate an error.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   pusProjectedSlaveList
              Pointer to a Sercos list with Sercos addresses of the projected slaves.
              The Recognized slave list must be a subset in the projected slave list.

\return       \ref CSMD_WARNING_SAME_PHASE \n
              \ref CSMD_INVALID_SERCOS_CYCLE_TIME \n
              \ref CSMD_WRONG_PHASE \n
              \ref CSMD_WRONG_PROJECTED_SLAVE_LIST \n
              \ref CSMD_ILLEGAL_SLAVE_ADDRESS \n
              \ref CSMD_ERROR_DOUBLE_ADDRESS \n
              \ref CSMD_PROJ_SLAVES_NOT_ONE_TO_ONE \n
              \ref CSMD_ERROR_PHASE_CHANGE_CHECK \n
              \ref CSMD_ERROR_PHASE_CHANGE_START \n
              \ref CSMD_EVENT_TIME_MAX_LIMIT \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         01.02.2005

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_SetPhase1( CSMD_INSTANCE   *prCSMD_Instance,
                              CSMD_FUNC_STATE *prFuncState,
                              CSMD_USHORT     *pusProjectedSlaveList )
{
  
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    /* --------------------------------------------------------- */
    /* Step 1 :: Check current communication phase               */
    /* --------------------------------------------------------- */
    prFuncState->ulSleepTime = CSMD_WAIT_2MS;
    
    if (prCSMD_Instance->sCSMD_Phase == CSMD_SERC_PHASE_1)
    {
      prFuncState->usActState = CSMD_FUNCTION_STEP_1;
      return (CSMD_WARNING_SAME_PHASE);
    }
    else if (prCSMD_Instance->sCSMD_Phase != CSMD_SERC_PHASE_0)
    {
      return (CSMD_WRONG_PHASE);
    }
#ifdef CSMD_DEBUG
    (CSMD_VOID) CSMD_HAL_memset( &prCSMD_Instance->rCSMD_Debug,
                                 0,
                                 sizeof (prCSMD_Instance->rCSMD_Debug) );
#endif
    /* Take the hardware specific settings */
    if (CSMD_NO_ERROR != (eFuncRet = CSMD_GetHW_Settings( prCSMD_Instance, FALSE )) )
    {
      return (eFuncRet);
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_1:
#ifdef CSMD_SWC_EXT
    if (prCSMD_Instance->rPriv.rSWC_Struct.eActiveUCC_Mode_CP12 != CSMD_UCC_MODE_CP12_FIX)
    {
      /* No change in cycle time possible; Sercos cycle time was determined in CP0 */
      prCSMD_Instance->rPriv.ulActiveCycTime = 
        prCSMD_Instance->rPriv.rTimingCP12.ulCycleTime_CP1_2;
    }
    else
#endif
    {
      prCSMD_Instance->rPriv.ulActiveCycTime =
        prCSMD_Instance->rPriv.rHW_Init_Struct.ulCycleTime_CP12;
    }
    
    /* Check for valid Sercos cycle time for CP0 to CP2 */
    eFuncRet = CSMD_CheckCycleTime_CP12( prCSMD_Instance,
                                         prCSMD_Instance->rPriv.ulActiveCycTime );

    if (eFuncRet != CSMD_NO_ERROR)
    { 
      return (eFuncRet);
    }
    
    /* ----------------------------------------------------------------- */
    /* Compare the list of existing slaves with the list                 */
    /* of defined slaves.                                                */
    /* ----------------------------------------------------------------- */
    eFuncRet = CSMD_ScanOperDrives( prCSMD_Instance,
                                    pusProjectedSlaveList );
    
    if (eFuncRet != CSMD_NO_ERROR)
    { 
      return (eFuncRet);
    }
    
    /* Set communication phase to change mode and new communication phase */
    CSMD_HAL_SetPhase( &prCSMD_Instance->rCSMD_HAL,
                       CSMD_CPS_NEW_CP,
                       CSMD_SERC_PHASE_1 );
    
    /* Timeout slave check = 20 * 10 ms */
    prCSMD_Instance->rPriv.lOutTimer = CSMD_CPS_TIMEOUT_CNT;
    prFuncState->ulSleepTime         = CSMD_CPS_SLEEP_TIME;
    
    prFuncState->usActState          = CSMD_FUNCTION_STEP_3;
    break;
    
    
  case CSMD_FUNCTION_STEP_3:
    /* --------------------------------------------------------- */
    /* Step 2: Checks if all slaves stops inserting data         */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Finish_Phase_Check( prCSMD_Instance, 
                                        CSMD_SERC_PHASE_0 );
    
    prCSMD_Instance->rPriv.lOutTimer--;
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* copying of cyclic Sercos data / ring monitoring is disabled */
      prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_OFF;
      prCSMD_Instance->usEnableTel = 0U;          /* Not ready to enable CYC_CLK */
      /* Disable Central Timers */
      CSMD_HAL_CtrlSystemCounter( &prCSMD_Instance->rCSMD_HAL, 
                                  FALSE,
                                  FALSE,
                                  FALSE );
      
      /* wait for current telegram sent before reconfigure descriptor unit */
      prFuncState->ulSleepTime = 
        prCSMD_Instance->rPriv.ulActiveCycTime / 1000000U;
      if (prFuncState->ulSleepTime == 0)
      {
        prFuncState->ulSleepTime = CSMD_WAIT_1MS;
      }
      
      prFuncState->usActState = CSMD_FUNCTION_STEP_4;
    }
    else if (prCSMD_Instance->rPriv.lOutTimer <= 0)
    {
      return (eFuncRet);
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_4:
    if (*prCSMD_Instance->rPriv.rCbFuncTable.S3Event != NULL)
    {
      /* Notification to the IP-Driver: End driver shall terminate receive/transmit non Sercos telegrams */
      prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_STOP_COMMUNICATION;
    
      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.S3Event)( prCSMD_Instance->rPriv.pvCB_Info,
                                                            &prCSMD_Instance->rPriv.rSercEvent );
    }

    /* switch off Sercos transmission for both ports */
    CSMD_HAL_CtrlDescriptorUnit( &prCSMD_Instance->rCSMD_HAL, FALSE, FALSE );
    
    eFuncRet = CSMD_Set_Register_P1( prCSMD_Instance );     /* Set register preparing CP1 */
    
    if (eFuncRet != CSMD_NO_ERROR)
    {
      return (eFuncRet);
    }
    
    /* --------------------------------------------------------- */
    /* Step 6 :: Configuration of transmit telegrams for CP1     */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Config_TX_Tel_P1( prCSMD_Instance );
    if (eFuncRet != CSMD_NO_ERROR) 
      return (eFuncRet);
    
    prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_5;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_5:
    /* --------------------------------------------------------- */
    /* Step 7 :: Configuration of Rx memory                      */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Config_RX_Tel_P1( prCSMD_Instance );
    if (eFuncRet != CSMD_NO_ERROR) 
      return (eFuncRet);
    
    if (*prCSMD_Instance->rPriv.rCbFuncTable.RxTxRamAlloc != NULL)
    {
      /* Notification to IP-Driver for changing in Sercos Ram allocation */
      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.RxTxRamAlloc)
        ( prCSMD_Instance->rPriv.pvCB_Info,                  /* distinguish the instance */
          prCSMD_Instance->rCSMD_HAL.ulTxRamInUse,           /* Tx-Ram start address for IP frames */
          prCSMD_Instance->rCSMD_HAL.ulSizeOfTxRam,          /* Size in Tx-Ram for IP frames */
          prCSMD_Instance->rCSMD_HAL.ulRxRamInUse,           /* Rx-Ram start address for IP frames */
          prCSMD_Instance->rCSMD_HAL.ulSizeOfRxRam );        /* Size in Rx-Ram for IP frames */
    }
    
    prFuncState->usActState = CSMD_FUNCTION_STEP_6;
#ifdef CSMD_SWC_EXT
    if (prCSMD_Instance->rPriv.rSWC_Struct.boFastCPSwitchActive == TRUE)
    {
      /* Dont't wait */
      /* SWC wk 30.07.2012 Sercans: There shall be a break between step CSMD_FUNCTION_CHG_CLK_POSSIBLE and CSMD_FUNCTION_STEP_5. */
      prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    }
    else
#endif
    {
      /* wait at least 120ms before reconfigure slaves. */
      prFuncState->ulSleepTime  = (CSMD_ULONG) (CSMD_WAIT_120MS - prCSMD_Instance->rPriv.ulActiveCycTime/1000000);
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_6:
    /* --------------------------------------------------------- */
    /* Step 6 :: Start of communication phase CP1                */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Start_New_Phase_Prepare( prCSMD_Instance,
                                             CSMD_SERC_PHASE_1 );
    
    /* copying of cyclic Sercos data is enabled, but slave valid is not checked */
    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_COPY_ONLY;
    
    if (*prCSMD_Instance->rPriv.rCbFuncTable.S3Event != NULL)
    {
      /* Notification to the IP-Driver: End driver shall activate receive/transmit non Sercos telegrams */
      prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_START_COMMUNICATION;
    
      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.S3Event)
        ( prCSMD_Instance->rPriv.pvCB_Info,             /* distinguish the instance */
          &prCSMD_Instance->rPriv.rSercEvent );         /* Sercos event information */
    }
    
    if (eFuncRet != CSMD_NO_ERROR) 
    {
      return (eFuncRet);
    }
    
    prFuncState->ulSleepTime         = CSMD_WAIT_10MS;
    /* Timeout for S-DEV.SlaveValid at switch to CP1 = 20 * 10 ms.*/
    prCSMD_Instance->rPriv.lOutTimer = CSMD_CPS_TIMEOUT_CNT;
    prFuncState->ulSleepTime         = CSMD_CPS_SLEEP_TIME;
    
    prFuncState->usActState          = CSMD_FUNCTION_STEP_7;
    break;
    
    
  case CSMD_FUNCTION_STEP_7:
    /* --------------------------------------------------------- */
    /* Step 7: Check S-DEV.SlaveValid of all recognized slaves   */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Check_S_DEV_Start_CP1( prCSMD_Instance );
    
    prCSMD_Instance->rPriv.lOutTimer--;
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* Timeout for S-SVC.Valid check */
      prCSMD_Instance->rPriv.lOutTimer = 
        prCSMD_Instance->rPriv.rHW_Init_Struct.usSVC_Valid_TOut_CP1 / prFuncState->ulSleepTime;
      if (prCSMD_Instance->rPriv.lOutTimer <= 0)
      {
        /* At least one step */
        prCSMD_Instance->rPriv.lOutTimer = 1;
      }
      
      prFuncState->usActState = CSMD_FUNCTION_STEP_8;
    }
    else if (prCSMD_Instance->rPriv.lOutTimer <= 0)
    {
      return (eFuncRet);
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_8:
    /* --------------------------------------------------------- */
    /* Step 8: Check S_SVC.Valid of all recognized slaves        */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Check_S_SVC_Start_CP1( prCSMD_Instance );
    
    prCSMD_Instance->rPriv.lOutTimer--;
    if (   (eFuncRet == CSMD_NO_ERROR)
        || (prCSMD_Instance->rPriv.lOutTimer <= 0))
    {
      /* Continue if check successful or after timeout */
      CSMD_USHORT usSlaveIdx;
      CSMD_SVC_CONTROL_UNION * prSvcCtrl;  /* pointer to service channel control word */
      
      for (usSlaveIdx = 0; usSlaveIdx < prCSMD_Instance->rSlaveList.usNumProjSlaves; usSlaveIdx++)
      {
        /* ----------------------------------------------------------- */
        /* set handshake bit in Tx Ram.                                */ 
        /* note: pointer prSvcCtrl is only available for programmed    */
        /*       slaves                                                */
        /* ----------------------------------------------------------- */
        prSvcCtrl = CSMD_GetMdtServiceChannel(prCSMD_Instance, usSlaveIdx);
        if (prSvcCtrl != NULL)
          /* Set complete word is ok, because of telegram is initialized with 0 */
          CSMD_HAL_WriteShort( &prSvcCtrl->usWord, CSMD_SVC_CTRL_HANDSHAKE );
      }
      
      /* Timeout slave check = 20 * 10 ms */
      prCSMD_Instance->rPriv.lOutTimer = CSMD_CPS_TIMEOUT_CNT;
      prFuncState->ulSleepTime         = CSMD_CPS_SLEEP_TIME;
      
      prFuncState->usActState          = CSMD_FUNCTION_STEP_9;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_9:
    /* --------------------------------------------------------- */
    /* Step 7: Check for all recognized slaves whether the bits  */
    /*         S_SVC.Valid and S_SVC.AHS are set.                */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Start_New_Phase_Check( prCSMD_Instance,
                                           CSMD_SERC_PHASE_1 );
    
    prCSMD_Instance->rPriv.lOutTimer--;
    if (eFuncRet == CSMD_NO_ERROR)
    {
      prFuncState->ulSleepTime = CSMD_WAIT_2MS;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_10;
    }
    else if (prCSMD_Instance->rPriv.lOutTimer <= 0)
    {
      return (eFuncRet);
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_10:
    /* --------------------------------------------------------- */
    /* Step 8 ::  */
    /* --------------------------------------------------------- */
    
    prFuncState->ulSleepTime = (CSMD_ULONG) ((4 * prCSMD_Instance->rPriv.ulActiveCycTime)/1000000);
    prFuncState->usActState  = CSMD_FUNCTION_STEP_11;
    break;
    
    
  case CSMD_FUNCTION_STEP_11:
#if !defined CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
    /* Determination of S-0-1015 "Ring delay" */
    eFuncRet = CSMD_Determination_Ringdelay( prCSMD_Instance,
                                             CSMD_CALC_RD_MODE_NORMAL,
                                             &prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015,
                                             &prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015_P2 );

    if (eFuncRet != CSMD_NO_ERROR)
    {
      return (eFuncRet);
    }
    
    CSMD_Calculate_RingDelay( prCSMD_Instance );            /* calculation of ring delay independent of measured delay */
#endif
    
    prFuncState->ulSleepTime = 0U;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    
    /* enable cyclic handling with slave valid check */
    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_FULL;

    return (CSMD_NO_ERROR);
    
    
  default:
    return (CSMD_ILLEGAL_CASE);
    
  }   /* End: switch (prFuncState->usActState) */
  
  
  return (CSMD_FUNCTION_IN_PROCESS);           /* function processing still active */
  
} /* end: CSMD_SetPhase1() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief This function performs a communication phase switch to CP2.

\ingroup func_phase
\b Description: \n
   This function performs a communication phase switch to CP2, including:
   - configuration of the Sercos controller for CP2
   - service channel activation for all slaves in the ring
   - setting the identification bit to FALSE for all slaves
   - The element usSettings is set to zero for all slave configurations
   - initiation of CP2

   In CP2, the application is able communicate with slaves via the service
   channel in order to reconfigure them if they have identical Sercos addresses.
   The application can read out characteristics from the slaves that guarantee
   an unambiguous identification. This way it is able to allocate a unique
   Sercos address to every slave via the service channel.

<B>Call Environment:</B> \n
   Before the call-up of this function, CP1 must have been achieved successfully.\n
   The call-up should be performed from a task.\n
   The function comprises a state machine.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function

\return       \ref CSMD_WARNING_SAME_PHASE \n
              \ref CSMD_WRONG_PHASE \n
              \ref CSMD_ERROR_PHASE_CHANGE_CHECK \n
              \ref CSMD_ERROR_PHASE_CHANGE_START \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         01.02.2005
    
***************************************************************************** */
CSMD_FUNC_RET CSMD_SetPhase2( CSMD_INSTANCE   *prCSMD_Instance,
                              CSMD_FUNC_STATE *prFuncState )
{
  
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT   usI;
  
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    /* --------------------------------------------------------- */
    /* Step 1 :: Check current communication phase               */
    /* --------------------------------------------------------- */
    prFuncState->ulSleepTime = CSMD_WAIT_2MS;
    
    if (prCSMD_Instance->sCSMD_Phase == CSMD_SERC_PHASE_2)
    {
      prFuncState->usActState = CSMD_FUNCTION_STEP_1;
      
      return (CSMD_WARNING_SAME_PHASE);
    }
    else if (prCSMD_Instance->sCSMD_Phase != CSMD_SERC_PHASE_1)
    {
      return (CSMD_WRONG_PHASE);
    }
    
    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_COPY_ONLY;

    /* Initialize structure for extended diagnostics */
    (CSMD_VOID) CSMD_HAL_memset( &prCSMD_Instance->rExtendedDiag,
                                 0,
                                 sizeof(prCSMD_Instance->rExtendedDiag) );

    prCSMD_Instance->rPriv.rCC_Connections.usNbrCC_Connections = 0;
    /* clear private structure of CC connections */
    (CSMD_VOID) CSMD_HAL_memset( prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList,
                                 0,
                                 (sizeof( CSMD_CC_CONN_LIST ) * prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn) );

    /* Take the hardware specific settings */
    if (CSMD_NO_ERROR != (eFuncRet = CSMD_GetHW_Settings( prCSMD_Instance, FALSE )) )
    {
      return (eFuncRet);
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_1:
    /* --------------------------------------------------------- */
    /* Step 2 :: Finish communication phase CP1                  */
    /* --------------------------------------------------------- */
    
    /* Set communication phase to change mode and new communication phase */
    CSMD_HAL_SetPhase( &prCSMD_Instance->rCSMD_HAL,
                       CSMD_CPS_NEW_CP,
                       CSMD_SERC_PHASE_2 );
    
    /* Timeout slave check = 20 * 10 ms */
    prCSMD_Instance->rPriv.lOutTimer = CSMD_CPS_TIMEOUT_CNT;
    prFuncState->ulSleepTime         = CSMD_CPS_SLEEP_TIME;
    
    prFuncState->usActState          = CSMD_FUNCTION_STEP_2;
    break;
    
    
  case CSMD_FUNCTION_STEP_2:
    /* --------------------------------------------------------- */
    /* Step 2: Checks if all slaves stops inserting data         */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Finish_Phase_Check( prCSMD_Instance, 
                                        CSMD_SERC_PHASE_1 );
    
    prCSMD_Instance->rPriv.lOutTimer--;
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* copying of cyclic Sercos data / ring monitoring is disabled */
      prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_OFF;
      prCSMD_Instance->usEnableTel = 0U;          /* Not ready to enable CYC_CLK */
      /* Disable Central Timers */
      CSMD_HAL_CtrlSystemCounter( &prCSMD_Instance->rCSMD_HAL, 
                                  FALSE,
                                  FALSE,
                                  FALSE );
      
      /* wait for current telegram sent before reconfigure descriptor unit */
      prFuncState->ulSleepTime = 
        prCSMD_Instance->rPriv.ulActiveCycTime / 1000000U;
      if (prFuncState->ulSleepTime == 0)
      {
        prFuncState->ulSleepTime = CSMD_WAIT_1MS;
      }
      
      prFuncState->usActState = CSMD_FUNCTION_STEP_3;
    }
    else if (prCSMD_Instance->rPriv.lOutTimer <= 0)
    {
      return (eFuncRet);
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_3:
    /* --------------------------------------------------------- */
    /* Step 4 :: Configuration of transmit telegrams for CP2     */
    /* --------------------------------------------------------- */
    if (*prCSMD_Instance->rPriv.rCbFuncTable.S3Event != NULL)
    {
      /* Notification to the IP-Driver: End driver shall terminate receive/transmit non Sercos telegrams */
      prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_STOP_COMMUNICATION;
    
      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.S3Event)( prCSMD_Instance->rPriv.pvCB_Info,
                                                                 &prCSMD_Instance->rPriv.rSercEvent );
    }

    /* switch off Sercos transmission for both ports */
    CSMD_HAL_CtrlDescriptorUnit( &prCSMD_Instance->rCSMD_HAL, FALSE, FALSE );
    
    eFuncRet = CSMD_Config_TX_Tel_P2( prCSMD_Instance );
    if (eFuncRet != CSMD_NO_ERROR) 
      return (eFuncRet);
    
    prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_4;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_4:
    /* --------------------------------------------------------- */
    /* Step 5 :: Configuration of receive telegrams for CP2      */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Config_RX_Tel_P2( prCSMD_Instance );
    if (eFuncRet != CSMD_NO_ERROR) 
      return (eFuncRet);
    
    /* set C-DEV bit 14 "Topology HS" for all slaves to zero in order to match 
       S-DEV behaviour (S-DEV bit 14 is set to zero at communication phase switch) */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      prCSMD_Instance->rPriv.ausDevControl[usI] &= (CSMD_USHORT)~CSMD_C_DEV_TOPOLOGY_HS;
    }
    
    /* Enable SVC operation */
    CSMD_HAL_CtrlSVCMachine(&prCSMD_Instance->rCSMD_HAL, TRUE);
    
    prFuncState->usActState = CSMD_FUNCTION_STEP_5;
    
#ifdef CSMD_SWC_EXT
    if (prCSMD_Instance->rPriv.rSWC_Struct.boFastCPSwitchActive == TRUE)
    {
      /* Dont't wait */
      /* SWC wk 30.07.2012 Sercans: There shall be a break between step CSMD_FUNCTION_CHG_CLK_POSSIBLE and CSMD_FUNCTION_STEP_5. */
      prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    }
    else
#endif
    {
      /* wait at least 120ms before reconfigure slaves. */
      prFuncState->ulSleepTime  = (CSMD_ULONG) (CSMD_WAIT_120MS - prCSMD_Instance->rPriv.ulActiveCycTime/1000000);
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_5:
    /* --------------------------------------------------------- */
    /* Step 5 :: Start of communication phase CP2                */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Start_New_Phase_Prepare( prCSMD_Instance,
                                             CSMD_SERC_PHASE_2 );
    
    /* copying of cyclic Sercos data is enabled, but slave valid is not checked */
    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_COPY_ONLY;
    
    if (*prCSMD_Instance->rPriv.rCbFuncTable.S3Event != NULL)
    {
      /* Notification to the IP-Driver: End driver shall activate receive/transmit non Sercos telegrams */
      prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_START_COMMUNICATION;
    
      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.S3Event)
        ( prCSMD_Instance->rPriv.pvCB_Info,             /* distinguish the instance */
          &prCSMD_Instance->rPriv.rSercEvent );         /* Sercos event information */
    }
    
    if (eFuncRet != CSMD_NO_ERROR) 
    {
      return (eFuncRet);
    }
    
    /* Timeout slave check = 20 * 10 ms */
    prCSMD_Instance->rPriv.lOutTimer = CSMD_CPS_TIMEOUT_CNT;
    prFuncState->ulSleepTime         = CSMD_CPS_SLEEP_TIME;
    
    prFuncState->usActState          = CSMD_FUNCTION_STEP_6;
    break;
    
    
  case CSMD_FUNCTION_STEP_6:
    /* --------------------------------------------------------- */
    /* Step 6 :: Start of communication phase CP2                */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Start_New_Phase_Check( prCSMD_Instance,
                                           CSMD_SERC_PHASE_2 );
    
    prCSMD_Instance->rPriv.lOutTimer--;
    if (eFuncRet == CSMD_NO_ERROR)
    {
#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
      if (   CSMD_IsReady_Soft_SVC_Status( prCSMD_Instance )
          || (prCSMD_Instance->rPriv.lOutTimer <= 0) )
      {
        CSMD_Init_Soft_SVC( prCSMD_Instance );

        prFuncState->ulSleepTime = CSMD_WAIT_2MS;
        prFuncState->usActState  = CSMD_FUNCTION_STEP_7;
      }
#else
      prFuncState->ulSleepTime = CSMD_WAIT_2MS;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_7;
#endif
    }
    else if (prCSMD_Instance->rPriv.lOutTimer <= 0)
    {
      return (eFuncRet);
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_7:
    /* --------------------------------------------------------- */
    /* Step 7 ::                                                 */
    /* --------------------------------------------------------- */
    prCSMD_Instance->rConfiguration.rComTiming.ulMinTimeStartAT = (CSMD_ULONG) CSMD_MIN_TIME_START_AT;
    
    /* By default: All slaves with SCP_Sync supports S-0-1036 */
    prCSMD_Instance->boIFG_V1_3 = TRUE;

    /* Initialize bit list of SCP classes */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      prCSMD_Instance->rConfiguration.parSlaveConfig[usI].ausSCPClasses[0] = 0;  /* Initialize as empty list */
      prCSMD_Instance->rConfiguration.parSlaveConfig[usI].ausSCPClasses[1] = CSMD_MAX_ENTRIES_S_0_1000 * 2;
    }
#ifdef CSMD_FAST_STARTUP
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
    {
      prCSMD_Instance->rConfiguration.parSlaveConfig[usI].usSettings = 0;
    }
#endif

    prFuncState->ulSleepTime = 0;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    
    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_FULL;

    return (CSMD_NO_ERROR);
    
    
  default:
    return (CSMD_ILLEGAL_CASE);
    
  }   /* End: switch (prFuncState->usActState) */
  
  return (CSMD_FUNCTION_IN_PROCESS);           /* function processing still active */
  
} /* end: CSMD_SetPhase2() */



/**************************************************************************/ /**
\brief This function performs a communication phase switch to CP3.

\ingroup func_phase
\b Description: \n
   This function performs a communication phase switch to CP3, including:
   - deletion, start and reset of command S-0-1024 in all slaves
   - deletion, start and reset of command S-0-0127 in all slaves
   - configuration of the Sercos controller for CP3
   - initiation of CP3

<B>Call Environment:</B> \n
   Before the call-up of this function, CP2 must have been achieved successfully.
   The timing and telegram calculations must have been completed and transmitted
   to the slaves so that the correct settings are available in the slaves.\n
   If the PCI bus master DMA functionality is activated, memory must be allocated for
   one MDT and AT telegram buffer each before this function is called. For this purpose,
   the rCSMD_DMAMirror structure must be provided with two pointers the allocated
   memory range. The size of the two required telegram buffers has been determined in the
   CSMD_CalculateTiming() function in advance and has been stored in this structure.\n\n
   If this function is not to complete the S-0-1024, "SYNC delay measuring procedure command"
   automatically, it can be caused to do so by calling it
   with the CSMD_FUNCTION_CP3_TRANS_CHECK state. It is then directly started with
   the execution of the Communication Phase 3 Transition Check command.\n\n
   If this function is not to process the switching command automatically,
   it is possible to select another entry point calling the function. For this purpose,
   CSMD_FUNCTION_CHANGE_PHASE has to be transmitted to the function as state. In
   this case, the pure switching feature may be processed without having to execute
   the adequate command itself.\n
   The call-up of this function should be performed from a task.\n
   The function comprises a state machine.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_WARNING_SAME_PHASE \n
              \ref CSMD_WARN_TOO_FEW_TX_RAM_FOR_UCC \n
              \ref CSMD_WARN_TOO_FEW_RX_RAM_FOR_UCC \n
              \ref CSMD_WRONG_PHASE \n
              \ref CSMD_ILLEGAL_SLAVE_ADDRESS \n
              \ref CSMD_FAULTY_MDT_LENGTH \n
              \ref CSMD_FAULTY_AT_LENGTH \n
              \ref CSMD_INSUFFICIENT_TX_RAM \n
              \ref CSMD_INSUFFICIENT_RX_RAM \n
              \ref CSMD_ERROR_PHASE_CHANGE_CHECK \n
              \ref CSMD_ERROR_PHASE_CHANGE_START \n
              \ref CSMD_CP3_TRANS_CHECK_CMD_ERROR \n
              \ref CSMD_NO_UNIQUE_RECOGNIZED_ADDRESSES \n
              \ref CSMD_S_0_1024_CMD_ERROR \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_EVENT_TIME_MAX_LIMIT \n
              \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_INTERNAL_REQUEST_PENDING \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         01.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_SetPhase3( CSMD_INSTANCE          *prCSMD_Instance,
                              CSMD_FUNC_STATE        *prFuncState,
                              CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_USHORT   usI;
  CSMD_USHORT   usSAdd;
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;
  
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    /* --------------------------------------------------------- */
    /* Step 1 :: Check current communication phase               */
    /* --------------------------------------------------------- */
    prFuncState->ulSleepTime = CSMD_WAIT_2MS;
    
    if (prCSMD_Instance->sCSMD_Phase == CSMD_SERC_PHASE_3)
    {
      prFuncState->usActState = CSMD_FUNCTION_STEP_1;
      
      return (CSMD_WARNING_SAME_PHASE);
    }
    else if (prCSMD_Instance->sCSMD_Phase != CSMD_SERC_PHASE_2)
    {
      return (CSMD_WRONG_PHASE);
    }

    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_COPY_ONLY;

#ifdef CSMD_DEBUG
    (CSMD_VOID) CSMD_HAL_memset( &prCSMD_Instance->rCSMD_Debug,
                                 0,
                                 sizeof (prCSMD_Instance->rCSMD_Debug) );
#endif
    /* Take the hardware specific settings */
    if (CSMD_NO_ERROR != (eFuncRet = CSMD_GetHW_Settings( prCSMD_Instance, FALSE )) )
    {
      return (eFuncRet);
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_1:
    if (TRUE == prCSMD_Instance->rPriv.boMultipleSAddress)
    {
      return (CSMD_NO_UNIQUE_RECOGNIZED_ADDRESSES);
    }
    
    /* Check for invalid Sercos addresses in the projected slave address list. */
    for (usI = 2; usI < (prCSMD_Instance->rSlaveList.usNumProjSlaves + 2); usI++)
    {
      usSAdd = prCSMD_Instance->rSlaveList.ausProjSlaveAddList[usI];
      
      if (   (usSAdd < CSMD_MIN_SLAVE_ADD)
          || (usSAdd > CSMD_MAX_SLAVE_ADD))
      {
        CSMD_RuntimeError( prCSMD_Instance, usSAdd, 
                           "CSMD_Check_Slave_Addresses: Projected Sercos slave address <min or >max address." );
        return (CSMD_ILLEGAL_SLAVE_ADDRESS);
      }
    }
    
    prCSMD_Instance->rPriv.eRequiredState = CSMD_SLAVE_ACTIVE;
    /* state machine for internal function call */
    prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
    prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
    
    /* state machine for this function */
    prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_2:
    
    eFuncRet = CSMD_Proceed_Cmd_S_0_1024( prCSMD_Instance,
                                          &prCSMD_Instance->rPriv.rInternalFuncState,
                                          parSvcMacro );
    
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_CP3_TRANS_CHECK;
    }
    else 
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      return (eFuncRet);
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_CP3_TRANS_CHECK:
    
    eFuncRet = CSMD_CP_TransitionCheck( prCSMD_Instance,
                                        &prCSMD_Instance->rPriv.rInternalFuncState,
                                        parSvcMacro,
                                        CSMD_IDN_S_0_0127 );
    
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_CHANGE_PHASE;
      break;  /* Must be a 'break' for Test-Master */
    }
    else 
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      return (eFuncRet);
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_CHANGE_PHASE:
    /* --------------------------------------------------------- */
    /* Step 2 :: Finish communication phase CP2                  */
    /* --------------------------------------------------------- */
    
    /* Set communication phase to change mode and new communication phase */
    CSMD_HAL_SetPhase( &prCSMD_Instance->rCSMD_HAL,
                       CSMD_CPS_NEW_CP,
                       CSMD_SERC_PHASE_3 );
    
    /* Timeout slave check = 20 * 10 ms */
    prCSMD_Instance->rPriv.lOutTimer = CSMD_CPS_TIMEOUT_CNT;
    prFuncState->ulSleepTime         = CSMD_CPS_SLEEP_TIME;
    
    prFuncState->usActState          = CSMD_FUNCTION_STEP_3;
    break;  /* Must be a 'break' for Test-Master */
    
    
  case CSMD_FUNCTION_STEP_3:
    /* --------------------------------------------------------- */
    /* Step 3 :: Finish communication phase CP2                  */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Finish_Phase_Check( prCSMD_Instance, 
                                        CSMD_SERC_PHASE_2 );
    
    prCSMD_Instance->rPriv.lOutTimer--;
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* copying of cyclic Sercos data / ring monitoring is disabled */
      prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_OFF;
      prCSMD_Instance->usEnableTel = 0U;          /* Not ready to enable CYC_CLK */
      /* Disable Central Timers */
      CSMD_HAL_CtrlSystemCounter( &prCSMD_Instance->rCSMD_HAL, 
                                  FALSE,
                                  FALSE,
                                  FALSE );
      
      /* wait for current telegram sent before reconfigure descriptor unit */
      prFuncState->ulSleepTime = 
        prCSMD_Instance->rPriv.ulActiveCycTime / 1000000U;
      if (prFuncState->ulSleepTime == 0)
      {
        prFuncState->ulSleepTime = CSMD_WAIT_1MS;
      }
      
      prFuncState->usActState = CSMD_FUNCTION_CHG_CLK_POSSIBLE;
    }
    else if (prCSMD_Instance->rPriv.lOutTimer <= 0)
    {
      return (eFuncRet);
    }
    break;  /* Must be a 'break' for Test-Master */
    
    
  case CSMD_FUNCTION_CHG_CLK_POSSIBLE:
    /* ------------------------------------------------------------ */
    /*   Step 4 ::  Setting for CP3                                 */
    /* ------------------------------------------------------------ */
    if (*prCSMD_Instance->rPriv.rCbFuncTable.S3Event != NULL)
    {
      /* Notification to the IP-Driver: End driver shall terminate receive/transmit non Sercos telegrams */
      prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_STOP_COMMUNICATION;
    
      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.S3Event)( prCSMD_Instance->rPriv.pvCB_Info,
                                                            &prCSMD_Instance->rPriv.rSercEvent );
    }

    /* switch off Sercos transmission for both ports */
    CSMD_HAL_CtrlDescriptorUnit( &prCSMD_Instance->rCSMD_HAL, FALSE, FALSE );
    
    CSMD_SaveSVCControlMDTPhase2( prCSMD_Instance );        /* to restore MHS bit properly on start CP3 */
    
    /* is no longer used (20.01.2015)    prCSMD_Instance->usAT_ErrorCounter2 = 0U; */       /* Reset AT error counter 2, port 1 */
    
    eFuncRet = CSMD_Set_Register_P3( prCSMD_Instance );     /* Set Register preparing CP3 */
    
    if (eFuncRet != CSMD_NO_ERROR)
    {
      return (eFuncRet);
    }
    
    prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_5;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_5:
    /* --------------------------------------------------------- */
    /* Step 6 :: Configuration of transmit telegrams for CP3     */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Config_TX_Tel_P3( prCSMD_Instance );
    if (eFuncRet != CSMD_NO_ERROR) 
      return (eFuncRet);
    
    prFuncState->ulSleepTime = CSMD_WAIT_1MS;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_6;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_6:
    /* --------------------------------------------------------- */
    /* Step 6 :: Configuration of Rx telegrams                   */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Config_RX_Tel_P3( prCSMD_Instance );
    if (eFuncRet != CSMD_NO_ERROR) 
      return (eFuncRet);
    
    /* set C-DEV bit 14 "Topology HS" for all slaves to zero in order to match 
       S-DEV behaviour (S-DEV bit 14 is set to zero at communication phase switch) */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      prCSMD_Instance->rPriv.ausDevControl[usI] &= (CSMD_USHORT)~CSMD_C_DEV_TOPOLOGY_HS;
    }
    
    /* Enable SVC operation */
    CSMD_HAL_CtrlSVCMachine(&prCSMD_Instance->rCSMD_HAL, TRUE);
    
    if (*prCSMD_Instance->rPriv.rCbFuncTable.RxTxRamAlloc != NULL)
    {
      /* Notification to IP-Driver for changing in Sercos Ram allocation */
      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.RxTxRamAlloc)
        ( prCSMD_Instance->rPriv.pvCB_Info,                  /* distinguish the instance */
          prCSMD_Instance->rCSMD_HAL.ulTxRamInUse,           /* Tx-Ram Start Address for IP frames */
          prCSMD_Instance->rCSMD_HAL.ulSizeOfTxRam,          /* Size in Tx-Ram for IP frames */
          prCSMD_Instance->rCSMD_HAL.ulRxRamInUse,           /* Rx-Ram Start Address for IP frames */
          prCSMD_Instance->rCSMD_HAL.ulSizeOfRxRam );        /* Size in Rx-Ram for IP frames */
    }
    
    prCSMD_Instance->rPriv.ulActiveCycTime = 
      prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002;
    prFuncState->usActState = CSMD_FUNCTION_STEP_7;

#ifdef CSMD_SWC_EXT
    if (prCSMD_Instance->rPriv.rSWC_Struct.boFastCPSwitchActive == TRUE)
    {
      /* Dont't wait */
      prFuncState->ulSleepTime = CSMD_WAIT_1MS; /* Must be a 'break' for Test-Master */
    }
    else
#endif
    {
      /* wait at least 120ms before reconfigure slaves. */
      prFuncState->ulSleepTime  = (CSMD_ULONG) (CSMD_WAIT_120MS - prCSMD_Instance->rPriv.ulActiveCycTime/1000000);
    }
    break;  /* Must be a 'break' for Test-Master */
    
    
  case CSMD_FUNCTION_STEP_7:
    /* --------------------------------------------------------- */
    /* Step 5 :: Start of communication phase CP3                */
    /* --------------------------------------------------------- */
    /* Initialize Timer for HP transmission of t6/t7 in CP3 */ 
    prCSMD_Instance->rPriv.usTimerHP_CP3 = 0;
    
    eFuncRet = CSMD_Start_New_Phase_Prepare( prCSMD_Instance,
                                             CSMD_SERC_PHASE_3 );
    
    /* copying of cyclic Sercos data is enabled, but slave valid is not checked */
    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_COPY_ONLY;
    
    if (*prCSMD_Instance->rPriv.rCbFuncTable.S3Event != NULL)
    {
      /* Notification to the IP-Driver: End driver shall activate receive/transmit non Sercos telegrams */
      prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_START_COMMUNICATION;
    
      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.S3Event)
        ( prCSMD_Instance->rPriv.pvCB_Info,             /* distinguish the instance */
          &prCSMD_Instance->rPriv.rSercEvent );         /* Sercos event information */
    }
    
    if (eFuncRet != CSMD_NO_ERROR) 
    {
      return (eFuncRet);
    }
    
    /* Timeout slave check = 20 * 10 ms */
    prCSMD_Instance->rPriv.lOutTimer = CSMD_CPS_TIMEOUT_CNT;
    prFuncState->ulSleepTime         = CSMD_CPS_SLEEP_TIME;
    
    prFuncState->usActState          = CSMD_FUNCTION_STEP_8;
    break;  /* Must be a 'break' for Test-Master */
    
    
  case CSMD_FUNCTION_STEP_8:
    /* --------------------------------------------------------- */
    /* Step 6 :: Start of communication phase CP3                */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Start_New_Phase_Check( prCSMD_Instance,
                                           CSMD_SERC_PHASE_3 );
    
    prCSMD_Instance->rPriv.lOutTimer--;
    if (eFuncRet == CSMD_NO_ERROR)
    {
#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
      if (   CSMD_IsReady_Soft_SVC_Status( prCSMD_Instance )
          || (prCSMD_Instance->rPriv.lOutTimer <= 0) )
      {
        CSMD_Init_Soft_SVC( prCSMD_Instance );

        prFuncState->ulSleepTime = CSMD_WAIT_2MS;
        prFuncState->usActState  = CSMD_FUNCTION_STEP_9;
      }
#else
      prFuncState->ulSleepTime = CSMD_WAIT_2MS;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_9;
#endif
    }
    else if (prCSMD_Instance->rPriv.lOutTimer <= 0)
    {
      return (eFuncRet);
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_9:
    /* --------------------------------------------------------- */
    /* Step 9 ::                                                 */
    /* --------------------------------------------------------- */
    
    /* Set flag to 'Slave SCP configurations not checked' */
    prCSMD_Instance->rPriv.boSCP_Checked = FALSE;
    
    prFuncState->ulSleepTime = 0;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    
    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_FULL;

#ifdef CSMD_SWC_EXT
    if (prCSMD_Instance->rCSMD_HAL.ulTxRamInUse > (prCSMD_Instance->rCSMD_HAL.ulSizeOfTxRam - CSMD_IP_TX_RAM_SIZE_CP0_CP2))
    {
      return (CSMD_WARN_TOO_FEW_TX_RAM_FOR_UCC);
    }
    else if (prCSMD_Instance->rCSMD_HAL.ulRxRamInUse > (prCSMD_Instance->rCSMD_HAL.ulSizeOfRxRam - CSMD_IP_RX_RAM_SIZE_CP0_CP2))
    {
      return (CSMD_WARN_TOO_FEW_RX_RAM_FOR_UCC);
    }
    else
#endif
    {
      return (CSMD_NO_ERROR);
    }
    
    
  default:
    return (CSMD_ILLEGAL_CASE);
    
  }   /* End: switch (prFuncState->usActState) */
  
  return (CSMD_FUNCTION_IN_PROCESS);           /* function processing still active */
  
} /* end: CSMD_SetPhase3() */



/**************************************************************************/ /**
\brief This function performs a communication phase switch to CP4

\ingroup func_phase
\b Description: \n
   This function performs a communication phase switch to CP4, including:
   - deletion, start and reset of command S-0-0128 in all slaves
   - configuration of the Sercos controller for CP4
   - initiation of CP4
  
<B>Call Environment:</B> \n
   Before the call-up of this function, CP3 must have been achieved successfully.\n\n
   If the CSMD_SetPhase4 function is not to process the switching command automatically, it is
   possible to select another entry point calling the function. For this purpose,
   CSMD_FUNCTION_CHANGE_PHASE has to be transmitted to the function as state. In
   this case, the pure switching feature may be activated without having to execute
   the adequate command itself.\n\n
   During the switch-over to CP4, transmission of the telegrams is not deactivated!\n\n
   The call-up of this function should be performed from a task.\n
   The function comprises a state machine.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_WARNING_SAME_PHASE \n
              \ref CSMD_WRONG_PHASE \n
              \ref CSMD_ERROR_PHASE_CHANGE_CHECK \n
              \ref CSMD_ERROR_PHASE_CHANGE_START \n
              \ref CSMD_CP4_TRANS_CHECK_CMD_ERROR \n
              \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_INTERNAL_REQUEST_PENDING \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n
             
\author       WK
\date         01.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_SetPhase4( CSMD_INSTANCE          *prCSMD_Instance,
                              CSMD_FUNC_STATE        *prFuncState,
                              CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_USHORT    usI;
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    /* --------------------------------------------------------- */
    /* Step 1 :: Check current communication phase               */
    /* --------------------------------------------------------- */
    prFuncState->ulSleepTime = CSMD_WAIT_2MS;
    
    if (prCSMD_Instance->sCSMD_Phase == CSMD_SERC_PHASE_4)
    {
      prFuncState->usActState = CSMD_FUNCTION_STEP_1;
      
      return (CSMD_WARNING_SAME_PHASE);
    }
    else if (prCSMD_Instance->sCSMD_Phase != CSMD_SERC_PHASE_3)
    {
      return (CSMD_WRONG_PHASE);
    }

    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_COPY_ONLY;

    /* Take the hardware specific settings */
    if (CSMD_NO_ERROR != (eFuncRet = CSMD_GetHW_Settings( prCSMD_Instance, FALSE )) )
    {
      return (eFuncRet);
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_1:
    /* state machine for internal function call */
    prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
    prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
    
    /* state machine for this function */
    prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_STEP_2:
    
    eFuncRet = CSMD_CP_TransitionCheck( prCSMD_Instance,
                                        &prCSMD_Instance->rPriv.rInternalFuncState,
                                        parSvcMacro,
                                        CSMD_IDN_S_0_0128 );
    
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* continue with next step */
      /* state machine for internal function call */
      prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime = 0;
      prCSMD_Instance->rPriv.rInternalFuncState.usActState  = CSMD_FUNCTION_1ST_ENTRY;
      
      /* state machine for this function */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      prFuncState->usActState  = CSMD_FUNCTION_CHANGE_PHASE;
      break;  /* Must be a 'break' for Test-Master */
    }
    else 
    {
      /* current step is not ready */
      prFuncState->ulSleepTime = prCSMD_Instance->rPriv.rInternalFuncState.ulSleepTime;
      return (eFuncRet);
    }
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_CHANGE_PHASE:
    /* --------------------------------------------------------- */
    /* Step 2 :: Finish communication phase CP3                  */
    /* --------------------------------------------------------- */
    
    /* Set communication phase to change mode and new communication phase */
    CSMD_HAL_SetPhase( &prCSMD_Instance->rCSMD_HAL,
                       CSMD_CPS_NEW_CP,
                       CSMD_SERC_PHASE_4 );
    
    /* Timeout slave check = 20 * 10 ms */
    prCSMD_Instance->rPriv.lOutTimer = CSMD_CPS_TIMEOUT_CNT;
    prFuncState->ulSleepTime         = CSMD_CPS_SLEEP_TIME;
    
    prFuncState->usActState          = CSMD_FUNCTION_STEP_3;
    break;  /* Must be a 'break' for Test-Master */
    
    
  case CSMD_FUNCTION_STEP_3:
    /* --------------------------------------------------------- */
    /* Step 3 :: Finish communication phase CP3                  */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Finish_Phase_Check( prCSMD_Instance, 
                                        CSMD_SERC_PHASE_3 );
    
    prCSMD_Instance->rPriv.lOutTimer--;
    if (eFuncRet == CSMD_NO_ERROR)
    {
      prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_OFF;

      /* set C-DEV bit 14 "Topology HS" for all slaves to zero in order to match 
         S-DEV behaviour (S-DEV bit 14 is set to zero at communication phase switch) */
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        prCSMD_Instance->rPriv.ausDevControl[usI] &= (CSMD_USHORT)~CSMD_C_DEV_TOPOLOGY_HS;
      }
      
      /* CC connections configured? */
      if (prCSMD_Instance->rPriv.rCC_Connections.usNbrCC_Connections)
      {
        /* Clear CC connection data in TxRam once at CP=CP4 & CPS=1 */
        prCSMD_Instance->rPriv.boClear_Tx_CC_Data = TRUE;
        /* Wait at least 3 Sercos cycles */
        prFuncState->ulSleepTime = (CSMD_ULONG)(( 3 * prCSMD_Instance->rPriv.ulActiveCycTime + 999999)/1000000);
      }
      else
      {
        prFuncState->ulSleepTime = CSMD_WAIT_1MS;
      }
      prFuncState->usActState  = CSMD_FUNCTION_STEP_4;
    }
    else if (prCSMD_Instance->rPriv.lOutTimer <= 0)
    {
      return (eFuncRet);
    }
    break;  /* Must be a 'break' for Test-Master */
    
    
  case CSMD_FUNCTION_STEP_4:
    /* --------------------------------------------------------- */
    /* Step 4 :: Start of communication phase CP4                */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Start_New_Phase_Prepare( prCSMD_Instance,
                                             CSMD_SERC_PHASE_4 );
    
    if (eFuncRet != CSMD_NO_ERROR) 
    {
      return (eFuncRet);
    }
    
    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_COPY_ONLY;

    /* Timeout slave check = 20 * 10 ms */
    prCSMD_Instance->rPriv.lOutTimer = CSMD_CPS_TIMEOUT_CNT;
    prFuncState->ulSleepTime         = CSMD_CPS_SLEEP_TIME;
    
    prFuncState->usActState          = CSMD_FUNCTION_STEP_5;
    break;  /* Must be a 'break' for Test-Master */
    
    
  case CSMD_FUNCTION_STEP_5:
    /* --------------------------------------------------------- */
    /* Step 5 :: Start of communication phase CP4                */
    /* --------------------------------------------------------- */
    eFuncRet = CSMD_Start_New_Phase_Check( prCSMD_Instance,
                                           CSMD_SERC_PHASE_4 );
    
    prCSMD_Instance->rPriv.lOutTimer--;
    if (eFuncRet == CSMD_NO_ERROR)
    {
      prFuncState->ulSleepTime = CSMD_WAIT_2MS;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_6;
    }
    else if (prCSMD_Instance->rPriv.lOutTimer <= 0)
    {
      return (eFuncRet);
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_6:
    /* --------------------------------------------------------- */
    /* Step 6 :: configure cyclic Rx buffer switch               */
    /* --------------------------------------------------------- */

    /* Perform Rx buffer switch in every Sercos cycle independent from received telegrams
     * ==> NewData always has to be true */
    CSMD_HAL_Config_Rx_Buffer_Valid( &prCSMD_Instance->rCSMD_HAL,
                                     CSMD_HAL_RX_BUFFER_SYSTEM_A,
                                     0, /* Received MDTs extraneous */
                                     0 );

      prFuncState->usActState = CSMD_FUNCTION_STEP_7;
    /*lint -fallthrough */

  case CSMD_FUNCTION_STEP_7:
    /* --------------------------------------------------------- */
    /* Step 7 :: Finish step                                     */
    /* --------------------------------------------------------- */
    
    prFuncState->ulSleepTime = 0;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    prCSMD_Instance->rPriv.eMonitoringMode = CSMD_MONITORING_FULL;
    
    return (CSMD_NO_ERROR);
    
    
  default:
    return (CSMD_ILLEGAL_CASE);
    
  }   /* End: switch (prFuncState->usActState) */
  
  return (CSMD_FUNCTION_IN_PROCESS);           /* function processing still active */
  
} /* end: CSMD_SetPhase4() */

/*! \endcond */ /* PUBLIC */



/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Checks whether there are still ATs remaining in CP0 format.

\ingroup module_phase
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   boCheckProgrammed
              TRUE - only check programmed slaves\n
              FALSE  - check maximum number of slaves

\return       \ref CSMD_ERROR_PHASE_CHANGE_CHECK \n
              \ref CSMD_NO_ERROR \n

\date         17.10.2006
\author       WK

***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckPhase0( CSMD_INSTANCE *prCSMD_Instance,
                                CSMD_BOOL      boCheckProgrammed )
{
  
  CSMD_USHORT  usIndexStart;
  CSMD_USHORT  usCompareValue;
  CSMD_USHORT *pusPort;
  CSMD_USHORT  usTopologyIndex;
  CSMD_BOOL    boCheckSlave  = TRUE;
  
  
  /* --------------------------------------------------------------------------
      Make sure that no more AT's are running. Using remote addressing 
      this can be detected by checking all words behind SeqCnt against 
      0xffff. Using previous address mode the whole telegram content 
      should be 0x0.
     ----------------------------------------------------------------------- */
  
  usIndexStart   = 1U;                  /* offset inside AT */
  usCompareValue = CSMD_PHASE0_FILL;
  
  
  prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
  prCSMD_Instance->rExtendedDiag.ulIDN       = 0;
  
  /* --------------------------------------------------------------------------
      check port 1
     ----------------------------------------------------------------------- */
  if (prCSMD_Instance->rPriv.boP1_active)
  {
    pusPort = (CSMD_USHORT *)(CSMD_VOID *)prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[0] + usIndexStart;
    
    for (usTopologyIndex = 0;
         usTopologyIndex < (CSMD_USHORT)(CSMD_NBR_SCAN_SLAVES - 1);
         usTopologyIndex++)
    {
      if (boCheckProgrammed)
          boCheckSlave = (usTopologyIndex < prCSMD_Instance->rSlaveList.usNumRecogSlaves);
      
      if (boCheckSlave && (*pusPort != usCompareValue))
      {
        prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = usTopologyIndex;
        prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
      }
      pusPort++;
    } 
  }
  
  /* --------------------------------------------------------------------------
      check port 2
     ----------------------------------------------------------------------- */
  if (prCSMD_Instance->rPriv.boP2_active)
  {
    pusPort = (CSMD_USHORT *)(CSMD_VOID *)prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[0] + usIndexStart;
    
    for (usTopologyIndex = 0;
         usTopologyIndex < (CSMD_USHORT)(CSMD_NBR_SCAN_SLAVES - 1);
         usTopologyIndex++)
    {
      if (boCheckProgrammed)
          boCheckSlave = (usTopologyIndex < prCSMD_Instance->rSlaveList.usNumRecogSlaves);
      
      if (boCheckSlave && (*pusPort != usCompareValue))
      {
        prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = usTopologyIndex;
        prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
      }
      pusPort++;
    } 
  }
  
  if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
  {
    /* Any slave reports a problem */
    return (CSMD_ERROR_PHASE_CHANGE_CHECK);
  }
  else
  {
    return (CSMD_NO_ERROR);
  }

} /* of CSMD_CheckPhase0 */



/**************************************************************************/ /**
\brief Calculate the timing for the UC channel in CP1 and CP2.

\ingroup func_timing
\todo description
\b Description: \n
   This function configures the UC channel depending on
   - Number of telegrams (2MDT & 2AT or 4MDT & 4AT)
   - UC channel mode
     - Fix
     - Method 1
     - Method 2
     - Method 1 with UCC time width

\note For the UC channel mode "Method 1 with UCC time width" the
      UCC with is taken from CSMD_HW_INIT_STRUCT.ulUCC_Width.

   In the timing calculation tTH (time for Sercos header of MDT0) can be ignored.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       none

\author       WK
\date         21.06.2012
    
***************************************************************************** */
CSMD_VOID CSMD_Calculate_Timing_CP1_CP2( CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_ULONG ulAT_BlockTime;

  ulAT_BlockTime =
      prCSMD_Instance->rPriv.usNbrTel_CP12
    * (  CSMD_MEDIA_LAYER_OVERHEAD  
       + CSMD_BYTE_TIME * (CSMD_SERC3_DATA_LENGTH_P12 + CSMD_IFG_DEFAULT));
  
  prCSMD_Instance->rPriv.rTimingCP12.ulCycleTime_CP1_2 = 
    prCSMD_Instance->rPriv.rHW_Init_Struct.ulCycleTime_CP12;
  
  /* fixed UC channel according to previous specification    */
#ifdef CSMD_SWC_EXT
  if (   (prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_FIX)
      || (prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12 >= CSMD_UCC_MODE_VARIANTS))
#endif
  {
    if (prCSMD_Instance->rPriv.usNbrTel_CP12 == 2)
    {
      prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1  = CSMD_FPGA_AT_START_TIME_CP12;
      prCSMD_Instance->rPriv.rTimingCP12.ulOpenUCC_t6  = CSMD_FPGA_IP_WINDOW_OPEN_CP12;
      prCSMD_Instance->rPriv.rTimingCP12.ulCloseUCC_t7 = CSMD_FPGA_IP_WINDOW_CLOSE_CP12;
      prCSMD_Instance->rPriv.rTimingCP12.ulUCC_Latest  = CSMD_FPGA_IP_LAST_TRANSMISS_CP12;
      prCSMD_Instance->rPriv.rTimingCP12.ulTxBufReqA   = CSMD_FPGA_TX_BUF_CHNG_REQ_CP12;
    }
    else
    {
      prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1  = CSMD_FPGA_AT_START_TIME_CP12_4TEL;
      prCSMD_Instance->rPriv.rTimingCP12.ulOpenUCC_t6  = CSMD_FPGA_IP_WIN_OPEN_CP12_4TEL;
      prCSMD_Instance->rPriv.rTimingCP12.ulCloseUCC_t7 = CSMD_FPGA_IP_WIN_CLOSE_CP12_4TEL;
      prCSMD_Instance->rPriv.rTimingCP12.ulUCC_Latest  = CSMD_FPGA_IP_LAST_TRANS_CP12_4TEL;
      prCSMD_Instance->rPriv.rTimingCP12.ulTxBufReqA   = CSMD_FPGA_TX_BUF_CHNG_REQ_CP12_4TEL;
    }
    prCSMD_Instance->rPriv.rTimingCP12.ulRxBufReqA     = prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1 + ulAT_BlockTime;
  }
#ifdef CSMD_SWC_EXT
  else
  {
    CSMD_ULONG ulMDT_BlockTime;
    
    /* rounded up to the nearest 10000 ns */
    ulAT_BlockTime  = ulAT_BlockTime + 10000 - (ulAT_BlockTime % 10000);
    ulMDT_BlockTime = ulAT_BlockTime;
    
    /* UC channel with maximum width (Method 1 --> MDT / AT / UCC) */
    if (prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_1)
    {
      prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1 =
          ulMDT_BlockTime + CSMD_JITTER_MASTER_CP0_2;
      prCSMD_Instance->rPriv.rTimingCP12.ulOpenUCC_t6 =
          prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1 
        + ulAT_BlockTime + CSMD_JITTER_MASTER_CP0_2;
      prCSMD_Instance->rPriv.rTimingCP12.ulCloseUCC_t7 =
          prCSMD_Instance->rPriv.rHW_Init_Struct.ulCycleTime_CP12
        - CSMD_JITTER_MASTER_CP0_2;
    }
  
    /* UC channel with maximum width (Method 2 --> MDT / UCC / AT) */
    else if (prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_2)
    {
      prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1 =
          (prCSMD_Instance->rPriv.rHW_Init_Struct.ulCycleTime_CP12 - ulAT_BlockTime)
        - CSMD_JITTER_MASTER_CP0_2;
      prCSMD_Instance->rPriv.rTimingCP12.ulOpenUCC_t6 =
          ulMDT_BlockTime + CSMD_JITTER_MASTER_CP0_2;
      prCSMD_Instance->rPriv.rTimingCP12.ulCloseUCC_t7 =
          prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1 
        - CSMD_JITTER_MASTER_CP0_2;
    }
    /* Centered UC channel with given width (Method 1 --> MDT / AT / UCC) */
    else  /* if (prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_1_VAR) */
    {
      /*
                           |<--------------- t_UCC_max --------------->|
           +-----+ +-----+ |             #################             |
           | MDT | | AT  | |<-- t_gap -->#<--IPchanTim-->#             |
        ---+-----+-+-----+-+-------------#-------+-------#-------------+-+-
           |       |       |             |       |       |             | |
           0       t1      t6_min        t6             t7       t7_max   t_Scyc
      */
      CSMD_ULONG ulT6_Min, ulT7_Max, ulGap;
      prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1 =
          ulMDT_BlockTime + CSMD_JITTER_MASTER_CP0_2;
      
      ulT6_Min =   prCSMD_Instance->rPriv.rTimingCP12.ulStartAT_t1 
                 + ulAT_BlockTime + CSMD_JITTER_MASTER_CP0_2;
      
      ulT7_Max =   prCSMD_Instance->rPriv.rHW_Init_Struct.ulCycleTime_CP12
                 - CSMD_JITTER_MASTER_CP0_2;
      
      if (  prCSMD_Instance->rPriv.ulUCC_Width
          < ulT7_Max - ulT6_Min)
      {
        ulGap = ((ulT7_Max - ulT6_Min) - prCSMD_Instance->rPriv.ulUCC_Width) / 2;
        /* Calculate t6/t7 for the centered UCC */
        ulT6_Min += ulGap;
        ulT7_Max -= ulGap;
      }
      prCSMD_Instance->rPriv.rTimingCP12.ulOpenUCC_t6  = ulT6_Min;
      prCSMD_Instance->rPriv.rTimingCP12.ulCloseUCC_t7 = ulT7_Max;
    }
    
    prCSMD_Instance->rPriv.rTimingCP12.ulUCC_Latest = 
        prCSMD_Instance->rPriv.rTimingCP12.ulCloseUCC_t7
      - CSMD_MAX_TIME_ETHERNET_FRAME;
    prCSMD_Instance->rPriv.rTimingCP12.ulTxBufReqA =
        prCSMD_Instance->rPriv.rTimingCP12.ulCloseUCC_t7
      - 10000;
    prCSMD_Instance->rPriv.rTimingCP12.ulRxBufReqA = 
        prCSMD_Instance->rPriv.rTimingCP12.ulOpenUCC_t6
      + 10000;
  }
#endif
  
} /* end: CSMD_Calculate_Timing_CP1_CP2() */



/**************************************************************************/ /**
\brief This function checks the the S-DEV.SlaveValid of all recognized
       slaves.

\ingroup module_phase
\b Description: \n
   This function checks whether the S-DEV.SlaveValid of all recognized
   slaves are enabled. 
   Before starting this function, the C-DEV.MasterValid shall be enabled
   and the C-SVC shall be cleared.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_ERROR_PHASE_CHANGE_START \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         21.02.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_Check_S_DEV_Start_CP1( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_BOOL    boFound;
  CSMD_USHORT  usSlaveIdx;
  CSMD_USHORT  usS_Dev;
  CSMD_USHORT  usMaxSlaveNmb;
  
  
  /* Check all projected slaves */
  prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
  prCSMD_Instance->rExtendedDiag.ulIDN       = 0;
  
  for (usSlaveIdx = 0, usMaxSlaveNmb = prCSMD_Instance->rSlaveList.usNumProjSlaves;
       usSlaveIdx < usMaxSlaveNmb; 
       usSlaveIdx++)
  {
    /* Is active slave ? */
    if (prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] == CSMD_SLAVE_ACTIVE)
    {
      boFound = FALSE;
      
      if (prCSMD_Instance->rPriv.boP1_active)      /* check port 1 */
      {
        usS_Dev = CSMD_END_CONV_S( *(prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx][CSMD_PORT_1][CSMD_RX_BUFFER_0]) );
        if (usS_Dev & CSMD_S_DEV_SLAVE_VALID)
        {
          /* slave is ready to activate the SVC */
          boFound = TRUE;
        }
      }
      
      if (prCSMD_Instance->rPriv.boP2_active)      /* check port 2 */
      {
        usS_Dev = CSMD_END_CONV_S( *(prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx][CSMD_PORT_2][CSMD_RX_BUFFER_0]) );
        if (usS_Dev & CSMD_S_DEV_SLAVE_VALID)
        {
          /* slave is ready to activate the SVC */
          boFound = TRUE;
        }
      }
      
      if (FALSE == boFound)
      {
        prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = usSlaveIdx;
        prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
      }
      
    } /* if (usTAdd) */
  } /* for (usSlaveIdx = 0; usSlaveIdx < usMaxSlaveNmb; usSlaveIdx++) */
  
  if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
  {
    /* Any slave reports a problem */
    return (CSMD_ERROR_PHASE_CHANGE_START);
  }
  else
  {
    return (CSMD_NO_ERROR);
  }
  
} /* end: CSMD_Check_S_DEV_Start_CP1 */



/**************************************************************************/ /**
\brief This function checks the the S-SVC.SlaveValid of all recognized
       slaves.

\ingroup module_phase
\b Description: \n
   This function checks whether the S-SVC.SlaveValid of all recognized
   slaves are enabled. 

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_ERROR_PHASE_CHANGE_START \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         21.02.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_Check_S_SVC_Start_CP1( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_BOOL    boFound;
  CSMD_USHORT  usSlaveIdx;
  CSMD_USHORT  usS_Svc;
  CSMD_USHORT  usMaxSlaveNmb;
  
  
  /* Check all projected slaves */
  prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
  prCSMD_Instance->rExtendedDiag.ulIDN       = 0;
  
  for (usSlaveIdx = 0, usMaxSlaveNmb = prCSMD_Instance->rSlaveList.usNumProjSlaves;
       usSlaveIdx < usMaxSlaveNmb; 
       usSlaveIdx++)
  {
    /* active slave ? */
    if (prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] == CSMD_SLAVE_ACTIVE)
    {
      boFound = FALSE;
      
      if (prCSMD_Instance->rPriv.boP1_active)      /* check port 1 */
      {
        usS_Svc = CSMD_HAL_ReadShort( (  prCSMD_Instance->rPriv.pusRxRam 
                                       + prCSMD_Instance->rPriv.arSVCInternalStruct[usSlaveIdx].usSVCRxRamPntrP1/2) );
        
        if (usS_Svc & CSMD_SVC_STAT_VALID) 
        {
          /* slave ready to respond to C_SVC.MHS */
          boFound = TRUE;
        }
      }
      
      if (prCSMD_Instance->rPriv.boP2_active)      /* check port 2 */
      {
        usS_Svc = CSMD_HAL_ReadShort( (  prCSMD_Instance->rPriv.pusRxRam 
                                       + prCSMD_Instance->rPriv.arSVCInternalStruct[usSlaveIdx].usSVCRxRamPntrP2/2) );
        
        if (usS_Svc & CSMD_SVC_STAT_VALID) 
        {
          /* slave ready to respond to C_SVC.MHS */
          boFound = TRUE;
        }
      }
      
      if (FALSE == boFound)
      {
        prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = usSlaveIdx;
        prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
      }
      
    } /* if (usTAdd) */
  } /* for (usSlaveIdx = 0; usSlaveIdx < usMaxSlaveNmb; usSlaveIdx++) */
  
  if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
  {
    /* Any slave reports a problem */
    return (CSMD_ERROR_PHASE_CHANGE_START);
  }
  else
  {
    return (CSMD_NO_ERROR);
  }
  
} /* end: CSMD_Check_S_SVC_Start_CP1 */



/**************************************************************************/ /**
\brief Checks the status of all required SVC valid bits. are
       enabled or disabled depending on the transfer parameter.

\ingroup module_phase
\b Description: \n
   Depending on the transfer parameter, this function checks whether the
   required SVC valid bits (SVC valid and AHS) are both either enabled or disabled.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   boShouldSend
              Service channel should be sending

\return       \ref CSMD_ERROR_PHASE_CHANGE_CHECK \n
              \ref CSMD_ERROR_PHASE_CHANGE_START \n
              \ref CSMD_NO_ERROR \n

\author       bk
\date         17.10.2006

***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckPhase1( CSMD_INSTANCE *prCSMD_Instance,
                                CSMD_BOOL      boShouldSend )
{
  
  CSMD_USHORT  usI;
  CSMD_USHORT  usTAdd;
  CSMD_USHORT  usMaxSlaveNmb;
  CSMD_BOOL    boFound;
  CSMD_USHORT  usSVC_Status;
  
  
  /* Check all projected slaves */
  usMaxSlaveNmb = prCSMD_Instance->rSlaveList.usNumProjSlaves;
  prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
  prCSMD_Instance->rExtendedDiag.ulIDN       = 0;

  for (usI = 0; usI < usMaxSlaveNmb; usI++)
  {
    usTAdd = prCSMD_Instance->rPriv.ausTopologyAddresses[usI];
    
    /* active slave ? */
    if (usTAdd)
    {
      boFound = FALSE;
      
      if (prCSMD_Instance->rPriv.boP1_active)      /* check port 1 */
      {
        usSVC_Status = CSMD_END_CONV_S( CSMD_GetAtServiceChannel(prCSMD_Instance, CSMD_PORT_1 ,usTAdd)->rSvcStatus.usWord );
        if ((usSVC_Status & (CSMD_SVC_STAT_VALID | CSMD_SVC_STAT_HANDSHAKE)) 
                         == (CSMD_SVC_STAT_VALID | CSMD_SVC_STAT_HANDSHAKE))
        {
          /* slave is sending  */
          boFound = TRUE;
        }
      }
      
      if (prCSMD_Instance->rPriv.boP2_active)      /* check port 2 */
      {
        usSVC_Status = CSMD_END_CONV_S( CSMD_GetAtServiceChannel(prCSMD_Instance, CSMD_PORT_2 ,usTAdd)->rSvcStatus.usWord );
        if ((usSVC_Status & (CSMD_SVC_STAT_VALID | CSMD_SVC_STAT_HANDSHAKE)) 
                         == (CSMD_SVC_STAT_VALID | CSMD_SVC_STAT_HANDSHAKE))
        {
          /* slave is sending  */
          boFound = TRUE;
        }
      }
      
      if (boShouldSend ^ boFound)
      {
        prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = usI;
        prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
      }
      
    } /* if (usTAdd) */
  } /* for (usI = 0; usI < usMaxSlaveNmb; usI++) */
  
  if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
  {
    /* Any slave reports a problem */
    if (boShouldSend)
    {
      return (CSMD_ERROR_PHASE_CHANGE_START);
    }
    else
    {
      return (CSMD_ERROR_PHASE_CHANGE_CHECK);
    }
  }
  else
  {
    return (CSMD_NO_ERROR);
  }
  
} /* of CSMD_CheckPhase1 */



/**************************************************************************/ /**
\brief Checks if all slaves in the ring produce any
       feedback in their device status word.

\ingroup module_phase
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   boShouldSend
              Service channel should be sending
                          
\return       \ref CSMD_ERROR_PHASE_CHANGE_CHECK \n
              \ref CSMD_ERROR_PHASE_CHANGE_START \n
              \ref CSMD_NO_ERROR \n

\author       bk
\date         17.10.2006
    
***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckPhase2( CSMD_INSTANCE *prCSMD_Instance,
                                CSMD_BOOL      boShouldSend )
{
  
  CSMD_USHORT usI;
  CSMD_BOOL   boFound;
  
  
  prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
  prCSMD_Instance->rExtendedDiag.ulIDN       = 0;
  
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    /* active slave ? */
    if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == CSMD_SLAVE_ACTIVE)
    {
      boFound = FALSE;
      
      if (prCSMD_Instance->arDevStatus[usI].usS_Dev )
      {
        /* slave is sending  */
        boFound = TRUE;
      }
      
      if (boShouldSend ^ boFound)
      {
        prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = usI;
        prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
      }
    }
    else
    {
      /* No check of hot-plug slaves */
      boFound = TRUE;
    }
  }
  
  if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
  {
    /* Any slave reports a problem */
    if (boShouldSend)
    {
      return (CSMD_ERROR_PHASE_CHANGE_START);
    }
    else
    {
      return (CSMD_ERROR_PHASE_CHANGE_CHECK);
    }
  }
  else
  {
    return (CSMD_NO_ERROR);
  }

} /* of CSMD_CheckPhase2 */



/**************************************************************************/ /**
\brief Checks if the slave valid bit in the device status
       word of all slaves in the ring is enabled.

\ingroup module_phase
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   boShouldSend
              Service channel should be sending
                         
\return       \ref CSMD_ERROR_PHASE_CHANGE_CHECK \n
              \ref CSMD_ERROR_PHASE_CHANGE_START \n
              \ref CSMD_NO_ERROR \n
            
\author       WK
\date         06.02.2007

***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckPhase34( CSMD_INSTANCE  *prCSMD_Instance,
                                 CSMD_BOOL       boShouldSend )
{
  
  CSMD_USHORT  usI;
  CSMD_BOOL    boFound;
  CSMD_USHORT  usDeviceStatus;
  
  
  prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
  prCSMD_Instance->rExtendedDiag.ulIDN       = 0;
  
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    /* active slave ? */
    if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == CSMD_SLAVE_ACTIVE)
    {
      boFound = FALSE;
      
      /* Get pointer to S_DEV on active port */    
      usDeviceStatus = prCSMD_Instance->arDevStatus[usI].usS_Dev;
      
      if (usDeviceStatus & CSMD_S_DEV_SLAVE_VALID)        /* Slave is sending  */
      {
        /* slave is sending  */
        boFound = TRUE;
      }
      
      if (boShouldSend ^ boFound)
      {
        prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = usI;
        prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
      }
    }
    else
    {
      /* No check of hot-plug slaves */
      boFound = TRUE;
    }
  }
  
  if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
  {
    /* Any slave reports a problem */
    if (boShouldSend)
    {
      return (CSMD_ERROR_PHASE_CHANGE_START);
    }
    else
    {
      return (CSMD_ERROR_PHASE_CHANGE_CHECK);
    }
  }
  else
  {
    return (CSMD_NO_ERROR);
  }

} /* of CSMD_CheckPhase34 */



/**************************************************************************/ /**
\brief Checks if all slaves stops inserting data into the AT telegram.

\ingroup module_phase
\b Description: \n
   This function cancels the communication phase switching process,
   if an error occurred in any of the functions:
   - CSMD_CheckPhase0()
   - CSMD_CheckPhase1()
   - CSMD_CheckPhase2()
   - CSMD_CheckPhase34()
   
   If an error occurred, this function returns the respective error code.
   If any slave did not stop sending data, the function is finished returning
   CSMD_ERROR_PHASE_CHANGE_CHECK while all erroneous slaves are recorded in the
   extended diagnosis structure.<BR>
   If no error occurred, this function deactivates Sercos communication for both
   ports and disables SVC operation as well as the central timers (TCNT, TCNT[1]
   and TCNT[2]).<BR>
   CSMD_Finish_Phase_Check() performs the last step before stopping communication.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   sPhase
              Communication phase (Sercos phase) to be finished

\return       \ref CSMD_ERROR_PHASE_CHANGE_CHECK \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         25.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Finish_Phase_Check( CSMD_INSTANCE *prCSMD_Instance,
                                       CSMD_SHORT     sPhase )
{
  
  CSMD_FUNC_RET eReturn = CSMD_NO_ERROR;
  
  
  switch (sPhase)                         /* check the sending of telegrams */
  {
    
  case CSMD_SERC_PHASE_0:
    /* check inserting of Sercos address in AT0 of recognized slaves */
    
    eReturn = CSMD_CheckPhase0( prCSMD_Instance, FALSE );
    if (eReturn != CSMD_NO_ERROR)
      return eReturn;
    break;
    
    
  case CSMD_SERC_PHASE_1:  
    /* check SVC status bits Valid and AHS */
    
    eReturn = CSMD_CheckPhase1( prCSMD_Instance, FALSE );
    if (eReturn != CSMD_NO_ERROR)
      return eReturn;
    /* Disable SVC operation */
    CSMD_HAL_CtrlSVCMachine( &prCSMD_Instance->rCSMD_HAL, FALSE );
    break;
    
    
  case CSMD_SERC_PHASE_2:
    /* check device status */
    
    eReturn = CSMD_CheckPhase2( prCSMD_Instance, FALSE );
    if (eReturn != CSMD_NO_ERROR)
      return eReturn;
    /* Disable SVC operation */
    CSMD_HAL_CtrlSVCMachine( &prCSMD_Instance->rCSMD_HAL, FALSE );
    break;
    
    
  case CSMD_SERC_PHASE_3:
  case CSMD_SERC_PHASE_4:
    /* check device status valid bit */
    
    eReturn = CSMD_CheckPhase34( prCSMD_Instance, FALSE );
    if (eReturn != CSMD_NO_ERROR)
      return eReturn;
    /* Dont't disable SVC operation if switching to CP4 (not necessary) or CP0 (done in SetPhase0) */
    break;
    
    
    /* This case is reached with CSMD_SetPhase0() because sPhase is set to -1. */
  default:
    break;
    
  }   /* End: switch (sPhase) */
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Finish_Phase_Check() */



/**************************************************************************/ /**
\brief Starts the new communication phase.

\ingroup module_phase
\b Description: \n
   This function prepares the new communication phase (re-)starting Sercos
   communication and enabling the central timers TCNT, TCNT[1] and TCNT[2].

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   sPhase
              New communication phase

\return       \ref CSMD_NO_ERROR \n

\author       wk
\date         26.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Start_New_Phase_Prepare( CSMD_INSTANCE *prCSMD_Instance,
                                            CSMD_SHORT     sPhase )
{
  
  /* Set communication phase to normal mode */
  /* set new communication phase */
  CSMD_HAL_SetPhase( &prCSMD_Instance->rCSMD_HAL, (CSMD_USHORT) 0, (CSMD_USHORT) sPhase );
  
  
  prCSMD_Instance->sCSMD_Phase = sPhase;
  
  /* no structure changes to CP4 */
  if (sPhase != CSMD_SERC_PHASE_4)
  {
    /* --------------------------------------------------------------------- */
    /* Enable telegram sending */
    /* --------------------------------------------------------------------- */
    if (prCSMD_Instance->rPriv.ulUCC_Width)
    {
      if (   (prCSMD_Instance->rPriv.boP1_active == TRUE)
          || (prCSMD_Instance->rPriv.boP2_active == TRUE))
      {
        /* switch on Sercos transmission for both ports */
        CSMD_HAL_CtrlDescriptorUnit( &prCSMD_Instance->rCSMD_HAL, TRUE, TRUE );
      }
    }
    else    
    {
      /* switch on Sercos transmission for both ports */
      CSMD_HAL_CtrlDescriptorUnit( &prCSMD_Instance->rCSMD_HAL, TRUE, TRUE );
      
    }
    
    CSMD_SetCYCCLK (prCSMD_Instance);                   /* Sets the TCYCSTART and TCSR register */
    
    CSMD_HAL_EnableTelegrams( &prCSMD_Instance->rCSMD_HAL,
                              (CSMD_ULONG)prCSMD_Instance->rPriv.usMDT_Enable,
                              (CSMD_ULONG)prCSMD_Instance->rPriv.usAT_Enable );
    
    /* Enable Central Timers: TCNT, TCNT(1), TCNT(2) */
    CSMD_HAL_CtrlSystemCounter( &prCSMD_Instance->rCSMD_HAL, 
                                TRUE,
                                TRUE,
                                TRUE );
    
    if (sPhase > CSMD_SERC_PHASE_2)
    {
      prCSMD_Instance->usEnableTel = (CSMD_USHORT)1;  /* Ready to enable CYC_CLK */
    }
  }
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Start_New_Phase_Prepare() */



/**************************************************************************/ /**
\brief Cancels the communication phase switching process due to incorrect
       behavior of any slave in the ring.

\ingroup module_phase
\b Description: \n
   This function cancels the communication phase switching process,
   if an error occurred in any of the functions:
   - CSMD_CheckPhase0()
   - CSMD_CheckPhase1()
   - CSMD_CheckPhase2()
   - CSMD_CheckPhase34()

   If an error occurred, this function returns the respective error message.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   sPhase
              New communication phase.

\return       \ref CSMD_ERROR_PHASE_CHANGE_START \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         26.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Start_New_Phase_Check( CSMD_INSTANCE *prCSMD_Instance,
                                          CSMD_SHORT     sPhase )
{
  
  CSMD_FUNC_RET   eReturn;
  
  
  switch (sPhase)                         /* check the sending of telegrams */
  {
    
  case CSMD_SERC_PHASE_0:
    
    eReturn = CSMD_CheckPhase0( prCSMD_Instance, TRUE );
    if (eReturn != CSMD_NO_ERROR)
      return eReturn;
    break;
    
    
  case CSMD_SERC_PHASE_1:  
    /* check SVC status bits Valid and AHS */
    
    eReturn = CSMD_CheckPhase1( prCSMD_Instance, TRUE );
    if (eReturn != CSMD_NO_ERROR)
      return eReturn;
    break;
    
    
  case CSMD_SERC_PHASE_2:
    /* check device status */
    
    eReturn = CSMD_CheckPhase2( prCSMD_Instance, TRUE );
    if (eReturn != CSMD_NO_ERROR)
      return eReturn;
    break;
    
    
  case CSMD_SERC_PHASE_3:
  case CSMD_SERC_PHASE_4:
    /* check device status valid bit */
    
    eReturn = CSMD_CheckPhase34( prCSMD_Instance, TRUE );
    if (eReturn != CSMD_NO_ERROR)
      return eReturn;
    break;
    
  default:
    return (CSMD_WRONG_PHASE);
    
  }   /* End: switch (sPhase) */
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Start_New_Phase_Check() */



#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
/**************************************************************************/ /**
\brief Check, if the emulated service channel is ready to operate.

\ingroup module_phase
\b Description: \n
   This function checks, if the emulated slave service channel are ready to
   operate. The received SVC.Status.Valid shall be set and the
   received SVC.Status.HS shall be equal to SVC.Control.HS:

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       TRUE  Service channel ready to operate.
              FALSE Service channel is not ready to operate.

\author       WK
\date         14.08.2015

*****************************************************************************/
CSMD_BOOL CSMD_IsReady_Soft_SVC_Status( const CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_USHORT usI;
  CSMD_USHORT usRdStatWrd;
  CSMD_BOOL   boSvcOK = TRUE;

  for (usI = CSMD_MAX_HW_CONTAINER; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == CSMD_SLAVE_ACTIVE)
    {
      usRdStatWrd = CSMD_HAL_ReadShort( prCSMD_Instance->rPriv.pusRxRam + prCSMD_Instance->rPriv.prSVContainer[usI]->usSVCRxPointer_Status/2 );

      /* SVC.Status.Valid == 1 and SVC.Status.HS == SVC.Control.HS ? */
      if (   !(usRdStatWrd & CSMD_SVC_STAT_VALID)
          || ((usRdStatWrd ^ prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0]) & CSMD_SVC_CTRL_HANDSHAKE))
      {
        /* Slaves SVC is not ready to operate */
        boSvcOK = FALSE;
        break;
      }
    }
  }
  return (boSvcOK);

} /* end: CSMD_IsReady_Soft_SVC_Status() */



/**************************************************************************/ /**
\brief Initializes emulated service channels.

\ingroup module_phase
\b Description: \n
   This function initializes the service status word in the local SVC container 
   for emulated SVC with the received status from rx ram.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       none

\author       WK
\date         31.03.2011

*****************************************************************************/
CSMD_VOID CSMD_Init_Soft_SVC( const CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT  usI;
  
  /* Initialize status word in emulated SVC container to prevent SVC error            */
  /* at first access to SVC, if SVC status Error bit is set in the received telegram. */
  for (usI = CSMD_MAX_HW_CONTAINER; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == CSMD_SLAVE_ACTIVE)
    {
      /* Get pointer to service container in local ram */
      /* Copy the status word of the requested drive in RX ram to the local status word in svc container */
      prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[1] =
        CSMD_HAL_ReadShort( prCSMD_Instance->rPriv.pusRxRam + prCSMD_Instance->rPriv.prSVContainer[usI]->usSVCRxPointer_Status/2 );
    }
  }
  
} /* end: CSMD_Init_Soft_SVC() */
#endif  /* #if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER */



/**************************************************************************/ /**
\brief Executes "SYNC delay measuring procedure command" for all slaves
       with class SCP_Sync.

\ingroup module_phase
\b Description: \n
   This function performs a full execution of command S-0-1024 "SYNC delay 
   measuring procedure command" processing all steps of the command state
   machine.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance                          
\param [in]   prFuncState
              Pointer to management data for this function
\param [in]   parSvcMacro
              Pointer to array with SVC macro structures

\return       \ref CSMD_S_0_1024_CMD_ERROR \n
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
\date         19.06.2008

***************************************************************************** */
CSMD_FUNC_RET CSMD_Proceed_Cmd_S_0_1024( CSMD_INSTANCE          *prCSMD_Instance,
                                         CSMD_FUNC_STATE        *prFuncState,
                                         CSMD_SVCH_MACRO_STRUCT *parSvcMacro )
{
  
  CSMD_USHORT             usI;
  CSMD_SVCH_MACRO_STRUCT *pSvcMacro;
  CSMD_FUNC_RET           eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT             usFinished;
  CSMD_ULONG             *paulSCP_ConfigMask;
  
  
  /* Points to configuration bitlist of first slave */
  paulSCP_ConfigMask = &prCSMD_Instance->rPriv.aulSCP_Config[0];
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prCSMD_Instance->rExtendedDiag.ulIDN       = CSMD_IDN_S_0_1024;
    
    if (prCSMD_Instance->rSlaveList.usNumProjSlaves == 0)
    {
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
      
      return (CSMD_NO_ERROR);
    }
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == prCSMD_Instance->rPriv.eRequiredState)
      {
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    
    prFuncState->ulSleepTime = 0;
    prFuncState->usActState  = CSMD_FUNCTION_CLR_CMD;
    /* continue with next case ! */
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_CLR_CMD:
    
    usFinished = 1U;
    
    /* ------------------------------------------------------------------------ */
    /* Clear SYNC delay measuring procedure command                             */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == prCSMD_Instance->rPriv.eRequiredState)
      {
        if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) 
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
                pSvcMacro->ulIdent_Nbr            = CSMD_IDN_S_0_1024;
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
        } /* End: if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (CSMD_S_0_1024_CMD_ERROR);
      }
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == prCSMD_Instance->rPriv.eRequiredState)
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
    /* Set   SYNC delay measuring procedure command                             */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == prCSMD_Instance->rPriv.eRequiredState)
      {
        if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) 
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
                pSvcMacro->ulIdent_Nbr            = CSMD_IDN_S_0_1024;
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
        } /* End: if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (CSMD_S_0_1024_CMD_ERROR);
      }
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == prCSMD_Instance->rPriv.eRequiredState)
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
    /* Check if SYNC delay measuring procedure command properly executed        */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == prCSMD_Instance->rPriv.eRequiredState)
      {
        if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) 
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
                pSvcMacro->ulIdent_Nbr            = CSMD_IDN_S_0_1024;
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
        } /* End: if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) */
      }
    }
    
    if (usFinished)
    {
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == prCSMD_Instance->rPriv.eRequiredState)
        {    
          if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) 
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
          } /* if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) */
        }
      }
    }
    
    if (usFinished)
    {
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == prCSMD_Instance->rPriv.eRequiredState)
        {
          if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) 
          {
            /* Procedure command not properly executed (0x3) */
            if (parSvcMacro[usI].pusAct_Data[0] != CSMD_CMD_FINISHED)
            {
              eFuncRet = CSMD_S_0_1024_CMD_ERROR;
              
              CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
            }
            parSvcMacro[usI].usState = CSMD_START_REQUEST;
          } /* End: if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) */
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
    /* Clear SYNC delay measuring procedure command again                       */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == prCSMD_Instance->rPriv.eRequiredState)
      {
        if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) 
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
                pSvcMacro->ulIdent_Nbr            = CSMD_IDN_S_0_1024;
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
        } /* End: if (CSMD_SCP_SYNC & paulSCP_ConfigMask[usI]) */
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        return (CSMD_S_0_1024_CMD_ERROR);
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
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    
    return (CSMD_NO_ERROR);
    
    
  default:
    return (CSMD_ILLEGAL_CASE);
    
  }   /* End: switch (prFuncState->usActState) */
  
  return (CSMD_FUNCTION_IN_PROCESS);           /* Function processing still active */
  
} /* end: CSMD_Proceed_Cmd_S_0_1024() */



/**************************************************************************/ /**
\brief Proceeds the specified transmission check command for all
       operable slaves.

\ingroup module_phase
\b Description: \n
   This function performs a full execution of one of the communication phase
   transition commands S-0-0127 or S-0-0128 processing all steps of the
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
\param [in]   ulIdentNbr
              Ident number of the desired procedure command:
              - CSMD_IDN_S_0_0127:  CP3 transition check
              - CSMD_IDN_S_0_0128:  CP4 transition check

\return       \ref CSMD_CP3_TRANS_CHECK_CMD_ERROR \n
              \ref CSMD_CP4_TRANS_CHECK_CMD_ERROR \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         16.06.2008

***************************************************************************** */
CSMD_FUNC_RET CSMD_CP_TransitionCheck( CSMD_INSTANCE          *prCSMD_Instance,
                                       CSMD_FUNC_STATE        *prFuncState,
                                       CSMD_SVCH_MACRO_STRUCT *parSvcMacro,
                                       CSMD_ULONG              ulIdentNbr )
{
  
  CSMD_USHORT             usI;
  CSMD_SVCH_MACRO_STRUCT *pSvcMacro;
  CSMD_FUNC_RET           eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT             usFinished;
  
  
  switch (prFuncState->usActState)
  {
    
  case CSMD_FUNCTION_1ST_ENTRY:
    
    prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
    prCSMD_Instance->rExtendedDiag.ulIDN       = ulIdentNbr;
    
    if (prCSMD_Instance->rSlaveList.usNumProjSlaves == 0)
    {
      prFuncState->ulSleepTime = 0UL;
      prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
      
      return (CSMD_NO_ERROR);
    }
    
    if (!(   (ulIdentNbr == CSMD_IDN_S_0_0127)
          || (ulIdentNbr == CSMD_IDN_S_0_0128)))
    {
      /* CoSeMa internal fault */
      return (CSMD_SYSTEM_ERROR);
    }
    
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        parSvcMacro[usI].usState = CSMD_START_REQUEST;
      }
    }
    
    prFuncState->ulSleepTime = 0;
    prFuncState->usActState  = CSMD_FUNCTION_CLR_CMD;
    /* continue with next case ! */
    /*lint -fallthrough */
    
    
  case CSMD_FUNCTION_CLR_CMD:
    
    usFinished = 1U;
    
    /* ------------------------------------------------------------------------ */
    /* Prepare Clear Command                                                    */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
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
              pSvcMacro->ulIdent_Nbr            = ulIdentNbr;
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
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        if (ulIdentNbr == CSMD_IDN_S_0_0127)
          return (CSMD_CP3_TRANS_CHECK_CMD_ERROR);
        else
          return (CSMD_CP4_TRANS_CHECK_CMD_ERROR);
      }
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
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
    /* Execute command  on all Oper Drives                                      */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
      {
        if ( !(   (parSvcMacro[usI].usState == CSMD_CMD_ACTIVE)
               || (parSvcMacro[usI].usState == CSMD_REQUEST_ERROR)) )
        {
          /* check if MBUSY is set, if it is not set do nothing */
          if (CSMD_HAL_ReadShort( &prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY)
          {
            if (parSvcMacro[usI].usState == CSMD_START_REQUEST)
            {
              pSvcMacro = &parSvcMacro[usI];
              pSvcMacro->pusAct_Data            = prCSMD_Instance->rPriv.parRdWrBuffer[usI].ausData;
              pSvcMacro->usSlaveIdx             = usI;
              pSvcMacro->usElem                 = CSMD_SVC_DBE7_OPERATION_DATA;
              pSvcMacro->ulIdent_Nbr            = ulIdentNbr;
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
              CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
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
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        if (ulIdentNbr == CSMD_IDN_S_0_0127)
          return (CSMD_CP3_TRANS_CHECK_CMD_ERROR);
        else
          return (CSMD_CP4_TRANS_CHECK_CMD_ERROR);
      }
      
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
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
    /* Check, if set command properly executed                                  */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
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
              pSvcMacro->ulIdent_Nbr            = ulIdentNbr;
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
      } 
    }
    
    if (usFinished)
    {
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
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
        }
      }
    }
    
    if (usFinished)
    {
      for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
      {
        if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
        {
          /* Procedure command not properly executed (0x3) */
          if (parSvcMacro[usI].pusAct_Data[0] != CSMD_CMD_FINISHED)
          {
            if (ulIdentNbr == CSMD_IDN_S_0_0127)
              eFuncRet = CSMD_CP3_TRANS_CHECK_CMD_ERROR;
            else
              eFuncRet = CSMD_CP4_TRANS_CHECK_CMD_ERROR;
            
            CSMD_Record_SVC_Error( prCSMD_Instance, usI, eFuncRet );
          }
          parSvcMacro[usI].usState = CSMD_START_REQUEST;
        }
      }
      prFuncState->usActState = CSMD_FUNCTION_CLR_CMD_AGAIN;
      
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        if (ulIdentNbr == CSMD_IDN_S_0_0127)
          return (CSMD_CP3_TRANS_CHECK_CMD_ERROR);
        else
          return (CSMD_CP4_TRANS_CHECK_CMD_ERROR);
      }
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
    /* Prepare clear command                                                    */
    /* ------------------------------------------------------------------------ */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      if ( prCSMD_Instance->rSlaveList.aeSlaveActive[usI] == (CSMD_SLAVE_ACTIVITY_STATUS)(prCSMD_Instance->sCSMD_Phase / 2) )
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
              pSvcMacro->ulIdent_Nbr            = ulIdentNbr;
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
      }
    }
    
    if (usFinished)
    {
      if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
      {
        if (ulIdentNbr == CSMD_IDN_S_0_0127)
          return (CSMD_CP3_TRANS_CHECK_CMD_ERROR);
        else
          return (CSMD_CP4_TRANS_CHECK_CMD_ERROR);
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
    prFuncState->ulSleepTime = 0;
    
    return (CSMD_NO_ERROR);
    
    
  default:
    return (CSMD_ILLEGAL_CASE);
    
  }   /* End: switch (prFuncState->usActState) */
  
  return (CSMD_FUNCTION_IN_PROCESS);           /* function processing still active */
  
} /* end: CSMD_CP_TransitionCheck() */

/*! \endcond */ /* PRIVATE */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
02 Dec 2013 WK
  - Defdb00165150
    CSMD_SetPhase4()
    Set flag boClear_Tx_CC_Data to clear CC data in TxRam during
    transition to CP4 and wait at least 3 Sercos cycles.
12 Jan 2014 WK
  - Defdb00151992
    CSMD_SetPhase0()
    Set sCSMD_Phase = CSMD_SERC_PHASE_NRT_STATE in case of entry from CP > 0.
    Replace slave check with waiting time of 200ms. (Status prior to 5.3V010)
17 Mar 2014 WK
  - Defdb00168074
    CSMD_SetPhase2()
    Initialize arSlaveConfig[usI].ausSCPClasses[0] as empty list
    (current length 0).
25 Jun 2014 AlM
  - Defdb00171440
  - Added function CSMD_Control_UC().
13 Nov 2014 WK
 - Defdb00000000
   CSMD_Set_NRT_State(): Removed test code for defined CSMD_DEBUG.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
02 Apr 2015 WK
  - Defdb00177706
    CSMD_Control_NRT_Comm()
    Remove activation of IP-RxEnable and IP-TxEnable.
08 Apr 2015 WK
  - Defdb00000000
    CSMD_Proceed_Cmd_S_0_1024()
    Fixed query of active slaves for ring recovery in CP4.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
14 Aug 2015 WK
  - Defdb00181153
    CSMD_SetPhase3():
      Initialize status in emulated SVC container with received SVC status from AT.
    CSMD_SetPhase2(), CSMD_SetPhase3():
      Check received SVC status before initialization of emulated SVC container.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
17 Nov 2015 WK
  - Defdb00183182
    CSMD_Proceed_Cmd_S_0_1024()
    Ignore SVC error 7010 "Command is already active" caused by slaves
    in a multi-slave-device.
28 Jan 2016 WK
  - Defdb00182067
    CSMD_Check_S_DEV_Start_CP1(): Static definition removed.
01 Feb 2016 WK
  - Defdb00184402
    CSMD_SetPhase2()
    Fixed initialization of rCC_Connections structure. Pointer parCC_ConnList
    was erroneously cleared.
18 Apr 2016 WK
  - Defdb00182067
    CSMD_Init_Soft_SVC(), CSMD_IsReady_Soft_SVC_Status()
    Fixed possible Lint warning 661.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
14 Jul 2016 WK
 - FEAT-00051878 - Support for Fast Startup
   CSMD_SetPhase2():
   Insert deletion of parSlaveConfig[].usSettings.
  
------------------------------------------------------------------------------
*/
