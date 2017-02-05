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
\file   CSMD_INIT.c
\author WK
\date   13.01.2005
\brief  This file contains public API functions which are initializing the API.
*/

/*---- Includes: -------------------------------------------------------------*/

#define SOURCE_CSMD
#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"
#include "CSMD_INIT.h"
#undef SOURCE_CSMD


#include "CSMD_HAL_DMA.h"
#include "CSMD_CP_AUX.h"
#include "CSMD_PHASEDEV.h"  /* only for CSMD_GetHW_Settings() */
#include "CSMD_PRIV_SVC.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */


/**************************************************************************/ /**
\brief Initializes the pointers to the RAM areas of the Sercos controller.

\ingroup func_init
\b Description: \n
   This function performs the basic initialization for the operation of
   the CoSeMa driver.
   - Initializes base pointers to Sercos IP-Core memory areas
   - Resets the memory allocation variables

   Note: Loading of the operating firmware of the Sercos controller is not the
   task of CoSeMa but must have been completed by the application before this 
   function was called.

<B>Call Environment:</B> \n
   The CSMD_Initialize function can be called at any time before any other access to
   CoSeMa driver functions. The call-up may only be performed once as with another
   call-up, all settings will be reset to the initial values.\n
   This function should be called from a task.

\param [in]   prCSMD_Instance
                Pointer to memory range allocated for the variables of the
                CoSeMa instance
\param [in]   prSERCOS
                Pointer to structure with pointers to virtual addresses
                of FPGA resources
 
\return       \ref CSMD_INVALID_SYSTEM_LIMITS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         09.08.2004

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_Initialize( CSMD_INSTANCE     *prCSMD_Instance,
                               CSMD_INIT_POINTER *prSERCOS )
{
  CSMD_FUNC_RET eFuncRet;
  
  /* virtual addresses */
  prCSMD_Instance->rCSMD_HAL.prSERC_Reg       = (CSMD_HAL_SERCFPGA_REGISTER *) prSERCOS->pvSERCOS_Register;
  prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram   = (CSMD_HAL_SVC_RAM *) prSERCOS->pvSERCOS_SVC_Ram;
  prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram    = (CSMD_HAL_TX_RAM *) prSERCOS->pvSERCOS_TX_Ram;
  prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram    = (CSMD_HAL_RX_RAM *) prSERCOS->pvSERCOS_RX_Ram;
#ifdef CSMD_PCI_MASTER
  prCSMD_Instance->rCSMD_HAL.prSERC_DMA_Reg   = (CSMD_HAL_SERCFPGA_DMA_REGISTER *) prSERCOS->pvSERCOS_DMA_Register;
#endif
  
  prCSMD_Instance->rCSMD_HAL.ulSvcRamBase = (CSMD_ULONG)(  (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram
                                                         - (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_Reg);
  prCSMD_Instance->rCSMD_HAL.ulTxRamBase  = (CSMD_ULONG)(  (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram
                                                         - (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_Reg);
  prCSMD_Instance->rCSMD_HAL.ulRxRamBase  = (CSMD_ULONG)(  (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram
                                                         - (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_Reg);

  prCSMD_Instance->rCSMD_HAL.ulSizeOfTxRam    = CSMD_HAL_TX_RAM_SIZE;
  prCSMD_Instance->rCSMD_HAL.ulSizeOfRxRam    = CSMD_HAL_RX_RAM_SIZE;
  
#ifdef CSMD_SOFT_MASTER
  /* Sercos soft-master is used */
  prCSMD_Instance->rPriv.boSoftMaster = TRUE;
#else
  /* Sercos master IP-Core is used */
  prCSMD_Instance->rPriv.boSoftMaster = FALSE;
#endif

  prCSMD_Instance->rPriv.rMemAlloc.ulAllocatedMemory    = 0UL;

  CSMD_ResetMemAllocManagement(prCSMD_Instance);

  if (CSMD_NO_ERROR != (eFuncRet = CSMD_SetSystemLimits(prCSMD_Instance, NULL)))
  {
    return (eFuncRet);
  }
  
  return (eFuncRet);
  
} /* end: CSMD_Initialize */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Initializes the system limits and calculate the required memory for
       CoSeMa structures.

\ingroup func_init
\b Description: \n
   This function
   - Check and take the required system limits
     (max. nbr. of operable slaves, connections, configurations etc.)
   - Initialize the pointer to the memory allocation call-back functions.
   - Calculate the required memory space for the CoSeMa structures.


<B>Call Environment:</B> \n
   The
   This function should be called from a task.

\param [in]   prCSMD_Instance
                Pointer to memory range allocated for the variables of the
                CoSeMa instance.
\param [in]   prMemAllocCB_Table
                Structure with call-back function pointers
\param [in]   prSysLimits
                Structure with system limitation variables
\param [out]  plBytes
                Required memory size for the CoSeMa structures.

\return       \ref CSMD_INVALID_SYSTEM_LIMITS \n
              \ref CSMD_MEMORY_ALLOCATION_FAILED \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         03.02.2014

***************************************************************************** */
CSMD_FUNC_RET CSMD_InitSystemLimits( CSMD_INSTANCE             *prCSMD_Instance,
                                     CSMD_MEM_ALLOC_CB_STRUCT  *prMemAllocCB_Table,
                                     CSMD_SYSTEM_LIMITS_STRUCT *prSysLimits,
                                     CSMD_LONG                 *plBytes )
{

  CSMD_FUNC_RET eFuncRet;

  /* Check and take the required system limits (max. nbr. of operable slaves, connections, configurations etc. ) */
  eFuncRet = CSMD_SetSystemLimits(prCSMD_Instance, prSysLimits);
  if (eFuncRet == CSMD_NO_ERROR)
  {
    /* Initialize the pointer to the memory allocation call-back functions */
    eFuncRet = CSMD_InitMemoryAllocCB(prCSMD_Instance, prMemAllocCB_Table);
    if (eFuncRet == CSMD_NO_ERROR)
    {
      /* Calculate the required memory space for the CoSeMa structures. */
      eFuncRet = CSMD_Ptr_MemoryAllocation( prCSMD_Instance,
                                            FALSE,
                                            plBytes );
    }
  }
  return (eFuncRet);

} /* end: CSMD_InitSystemLimits */



/**************************************************************************/ /**
\brief Initializes pointers to the allocated memory for the CoSeMa structures.

\ingroup func_init
\b Description: \n
   This function distributes the allocated memory to the CoSeMa structures and
   calculate corresponding pointers

<B>Call Environment:</B> \n
   This
   This function should be called from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]   plBytes
                Required memory size for the CoSeMa structures.

\return       \ref CSMD_MEMORY_ALLOCATION_FAILED \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         03.02.2014

***************************************************************************** */
CSMD_FUNC_RET CSMD_InitMemory( CSMD_INSTANCE *prCSMD_Instance,
                               CSMD_LONG     *plBytes )
{
  CSMD_FUNC_RET  eFuncRet;
  CSMD_VOID     *pvBase;

  if (prCSMD_Instance->rPriv.rMemAlloc.rCB_FuncTable.fpCSMD_set_mem_ptr == NULL)
  {
    /* Function pointer was not initialized by the master */
    eFuncRet = CSMD_MEMORY_ALLOCATION_FAILED;
  }
  else
  {
    /* Allocate system memory and pointer */
    pvBase = prCSMD_Instance->rPriv.rMemAlloc.rCB_FuncTable.fpCSMD_set_mem_ptr( (CSMD_INT)*plBytes );

    /* Guarantee long alignment of the base pointer */
    prCSMD_Instance->rPriv.rMemAlloc.pulBase = (CSMD_ULONG *) (((CSMD_UINT_PTR)pvBase + 3U) & ~3U);

    /* Distribute the memory to the CoSeMa structures */
    eFuncRet = CSMD_Ptr_MemoryAllocation( prCSMD_Instance,
                                          TRUE,
                                          plBytes );
  }
  return (eFuncRet);

} /* end: CSMD_InitMemory */



/**************************************************************************/ /**
\brief Performs a software reset of the Sercos controller.

\ingroup func_init
\b Description: \n
   This function
   - initializes the Sercos controller as master with the transferred settings.
     The hardware-specific settings for Sercos are realized in sub-functions.
   - Resets the configuration structure of the CoSeMa instance
   - Set default values for hardware specific settings in \ref CSMD_HW_INIT_STRUCT.

<B>Call Environment:</B> \n
   This function can be called at any time before any other access to the 
   Sercos controller.\n
   This function should be called from a task.\n
   The function comprises a state machine (see above).

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prFuncState
              Pointer to management data for this function

\return       \ref CSMD_FPGA_IDENT_VERSION \n
              \ref CSMD_INVALID_SERCOS_CYCLE_TIME \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         13.01.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_InitHardware( CSMD_INSTANCE   *prCSMD_Instance,
                                 CSMD_FUNC_STATE *prFuncState )
{
  
  CSMD_BOOL     boSoftwareReset;
  CSMD_BOOL     boPHY_Reset;
  CSMD_FUNC_RET eFuncRet = CSMD_FUNCTION_IN_PROCESS;  /* Function processing still active */
  

  switch (prFuncState->usActState)
  {
    
    case CSMD_FUNCTION_1ST_ENTRY:

      prFuncState->ulSleepTime = CSMD_WAIT_2MS;
      prCSMD_Instance->usCSMD_Topology            = CSMD_NO_LINK;  /* flag for topology */
      prCSMD_Instance->usRequired_Topology        = CSMD_NO_LINK;  /* no specific topology required */
#ifdef CSMD_HW_WATCHDOG
      CSMD_HAL_WatchdogSet( &prCSMD_Instance->rCSMD_HAL, FALSE );
#endif
      prCSMD_Instance->rCSMD_HAL.boSoftMaster = prCSMD_Instance->rPriv.boSoftMaster;

      prCSMD_Instance->rPriv.rMemAlloc.ulCSMD_Instance_Size = (CSMD_ULONG)sizeof (CSMD_INSTANCE);

      /* ------------------------------------------------------------------------ */
      /* Step 1 :: Initialization of all pointers for accessing Sercos FPGA       */
      /* ------------------------------------------------------------------------ */

      /* ------------------------------------------------------------------------ */
      /* Step 2 :: Check Version */
      /* ------------------------------------------------------------------------ */
      if (!CSMD_HAL_CheckVersion(&prCSMD_Instance->rCSMD_HAL))
      {
        eFuncRet = CSMD_FPGA_IDENT_VERSION;           /* FPGA Version not supported */
      }
      else
      {
#ifdef CSMD_PCI_MASTER
        prCSMD_Instance->prTelBuffer_Phys = NULL;       /* Pointer to local RAM for Sercos telegrams (physical address) */
#endif
        prCSMD_Instance->prTelBuffer      = NULL;       /* Pointer to local RAM for Sercos telegrams */

#ifndef CSMD_STATIC_MEM_ALLOC
        /* Check, if pointer to memory for CoSeMa structures is initialized */
        if (NULL == prCSMD_Instance->rPriv.rMemAlloc.pulBase)
        {
          eFuncRet = CSMD_MEMORY_ALLOCATION_FAILED;
        }
        else
#endif
        {
          /* ------------------------------------------------------------------------ */
          /* Step 3 :: Software-Reset master function */
          /* ------------------------------------------------------------------------ */
          CSMD_HAL_SoftReset( &prCSMD_Instance->rCSMD_HAL,
                              TRUE,   /* SoftwareReset */
                              TRUE ); /* PHY_Reset     */

          prCSMD_Instance->rPriv.lOutTimer = CSMD_TIMEOUT_FPGA_RESET;

          prFuncState->ulSleepTime = CSMD_WAIT_100MS;
          prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
        }
      }
      break;


    case CSMD_FUNCTION_STEP_1:

      CSMD_HAL_StatusSoftReset( &prCSMD_Instance->rCSMD_HAL,
                                &boSoftwareReset,
                                &boPHY_Reset );

      if ((FALSE == boSoftwareReset) && (FALSE == boPHY_Reset))
      {
        prFuncState->ulSleepTime = CSMD_WAIT_2MS;
        prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
      }
      else
      {
        prCSMD_Instance->rPriv.lOutTimer -= (CSMD_LONG)prFuncState->ulSleepTime;
        if (prCSMD_Instance->rPriv.lOutTimer <= 0)
        {
          /* Timeout */
          prFuncState->usActState = CSMD_FUNCTION_STEP_2;
        }
      }
      break;


    case CSMD_FUNCTION_STEP_2:

      /* ------------------------------------------------------------------------ */
      /* Step 3 :: initialize FPGA registers */
      /* ------------------------------------------------------------------------ */

      /* Clear image of IP-Core registers */
      (CSMD_VOID) CSMD_HAL_memset( (CSMD_VOID *)&prCSMD_Instance->rCSMD_HAL.rRegCopy,
                                   0,
                                   sizeof(prCSMD_Instance->rCSMD_HAL.rRegCopy) );

      CSMD_HAL_IntControl( &prCSMD_Instance->rCSMD_HAL,
                           0,
                           TRUE,
                           FALSE,
                           0 );

      CSMD_HAL_IntControl( &prCSMD_Instance->rCSMD_HAL,
                           0,
                           FALSE,
                           TRUE,
                           0 );

      CSMD_HAL_DisableSVCMachine( &prCSMD_Instance->rCSMD_HAL );

      CSMD_ClearInternalSVCInterrupts( prCSMD_Instance );

      CSMD_HAL_SetComMode( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_UC_RING_MODE );
      CSMD_HAL_CtrlDescriptorUnit( &prCSMD_Instance->rCSMD_HAL, FALSE, FALSE );

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

      CSMD_HAL_ClearTelegramStatusP1( &prCSMD_Instance->rCSMD_HAL, (CSMD_USHORT)0xFFFF );
      CSMD_HAL_ClearTelegramStatusP2( &prCSMD_Instance->rCSMD_HAL, (CSMD_USHORT)0xFFFF );

      CSMD_HAL_SetLineBreakSensitivity( &prCSMD_Instance->rCSMD_HAL, (CSMD_USHORT) 5 );

      /* Clear UC channel callback function pointer */
      prCSMD_Instance->rPriv.rCbFuncTable.RxTxRamAlloc    = NULL;
      prCSMD_Instance->rPriv.rCbFuncTable.S3Event         = NULL;
      prCSMD_Instance->rPriv.rCbFuncTable.S3EventFromISR  = NULL;

#ifdef CSMD_ACTIVATE_AUTONEGOTIATION
      /* Disable 10BASE-T capability for port 1 */
      CSMD_HAL_WriteMiiPhy( &prCSMD_Instance->rCSMD_HAL,
                            (CSMD_USHORT)PHY_PORT1,
                            (CSMD_USHORT)CSMD_PHYAN,
                            (CSMD_USHORT)CSMD_PHYAN_DIS_10MBIT );

      /* Disable 10BASE-T capability for port 2 */
      CSMD_HAL_WriteMiiPhy( &prCSMD_Instance->rCSMD_HAL,
                            (CSMD_USHORT)PHY_PORT2,
                            (CSMD_USHORT)CSMD_PHYAN,
                            (CSMD_USHORT)CSMD_PHYAN_DIS_10MBIT );


      /* Enable auto negotiation mode for port 1 */
      CSMD_HAL_WriteMiiPhy( &prCSMD_Instance->rCSMD_HAL,
                            (CSMD_USHORT)PHY_PORT1,
                            (CSMD_USHORT)CSMD_PHYCR,
                            (CSMD_USHORT)(CSMD_PHYCR_SP100 | CSMD_PHYCR_FD | CSMD_PHYCR_ANENAB | CSMD_PHYCR_ANRESTART) );


      /* Enable auto negotiation mode for port 2 */
      CSMD_HAL_WriteMiiPhy( &prCSMD_Instance->rCSMD_HAL,
                            (CSMD_USHORT)PHY_PORT2,
                            (CSMD_USHORT)CSMD_PHYCR,
                            (CSMD_USHORT)(CSMD_PHYCR_SP100 | CSMD_PHYCR_FD | CSMD_PHYCR_ANENAB | CSMD_PHYCR_ANRESTART) );

#endif  /* #ifdef CSMD_ACTIVATE_AUTONEGOTIATION */

      prCSMD_Instance->rPriv.lOutTimer = CSMD_TIMEOUT_PHY_RESET;
      prFuncState->ulSleepTime = CSMD_WAIT_100MS;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_3;
      break;


    case CSMD_FUNCTION_STEP_3:

      if (CSMD_HAL_GetLinkStatus( &prCSMD_Instance->rCSMD_HAL ))
      {
        /* Link at port 1 or/and port 2 */
        prFuncState->ulSleepTime = CSMD_WAIT_2MS;
        prFuncState->usActState  = CSMD_FUNCTION_STEP_4;
      }
      else
      {
        prCSMD_Instance->rPriv.lOutTimer -= (CSMD_LONG)prFuncState->ulSleepTime;
        if (prCSMD_Instance->rPriv.lOutTimer <= 0)
        {
          /* Timeout */
          prFuncState->usActState = CSMD_FUNCTION_STEP_4;
        }
      }
      break;


    case CSMD_FUNCTION_STEP_4:
      /* ------------------------------------------------------------------------ */
      /* Step 4 :: take hardware specific settings */
      /* ------------------------------------------------------------------------ */

      /* Default initialization of communication version field
          - Address allocation (Sercos version >= 1.1.1)
          - 2 MDTs and 2 ATs  (up to 255 slaves) in CP1/CP2
          - No transmission of communication parameters
          - Transmission of MST interrupted during switch */
      prCSMD_Instance->rPriv.rComVersion.ulLong =   CSMD_COMMVER_ADD_ALLOC
                                                  | CSMD_COMMVER_CP12_2MDT_2AT;

      /* Set the hardware specific settings to default values */
      (CSMD_VOID)CSMD_GetHW_Settings( prCSMD_Instance, TRUE );

      /* Disable CYC_CLC Function */
      (CSMD_VOID)CSMD_PrepareCYCCLK( prCSMD_Instance,
                                     FALSE,           /* Timing master mode   */
                                     FALSE,           /* Enable input CYC_CLK */
                                     FALSE,           /* Positive edge        */
                                     0 );

      /* Initialize configurable FPGA events (clear event time, state "not configured" */
      (CSMD_VOID) CSMD_HAL_memset( &prCSMD_Instance->rPriv.rCSMD_Event,
                                   0,
                                   sizeof(prCSMD_Instance->rPriv.rCSMD_Event) );

#ifdef CSMD_FAST_STARTUP
      {
        CSMD_INT  nLoop;
        for (nLoop = 0; nLoop < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; nLoop++)
        {
          prCSMD_Instance->rConfiguration.parSlaveConfig[nLoop].usSettings = 0;
        }
      }
#endif
      /* Resets the configuration structure of the CoSeMa instance */
      eFuncRet = CSMD_Init_Config_Struct( prCSMD_Instance );

#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
      prCSMD_Instance->rConfiguration.rComTiming.ulCalc_DelaySlave  = CSMD_DELAY_SLAVE_DELAY;
      prCSMD_Instance->rConfiguration.rComTiming.lCompDelay         = CSMD_DELAY_COMPONENTS_DELAY;
#else
      prCSMD_Instance->rConfiguration.rComTiming.ulCalc_DelayMaster = CSMD_DELAY_MASTER_DELAY;
      prCSMD_Instance->rConfiguration.rComTiming.ulCalc_DelaySlave  = CSMD_DELAY_SLAVE_DELAY;
      prCSMD_Instance->rConfiguration.rComTiming.ulCalc_DelayComp   = CSMD_DELAY_COMPONENTS_DELAY;
#endif
      /* Default value for max number of telegram failure */
      prCSMD_Instance->rConfiguration.rMasterCfg.usMaxNbrTelErr = CSMD_NBR_ALLOWED_TEL_ERROR;

      prCSMD_Instance->rConfig_Error = crCSMD_ConfigErrorInit;

      prCSMD_Instance->sCSMD_Phase = CSMD_SERC_PHASE_NRT_STATE;
      prFuncState->usActState      = CSMD_FUNCTION_FINISHED;

      /* eFuncRet = CSMD_NO_ERROR; */
      break;

    default:
      eFuncRet = CSMD_ILLEGAL_CASE;
      break;

  } /* End: switch (prFuncState->usActState) */
  
  return (eFuncRet);
  
} /* end: CSMD_InitHardware */



/**************************************************************************/ /**
\brief Sets the Sercos controller to initial values for CP0.

\ingroup func_init
\b Description: \n
   This function resets the Sercos controller (re-)initializing topology and
   communication phase data as well as FPGA registers for phase, interrupt control,
   communication mode, descriptor unit and Tx and Rx buffers.

<B>Call Environment:</B> \n
   This function should be called from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         09.08.2004

***************************************************************************** */
CSMD_FUNC_RET CSMD_ResetHardware( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT   usI;
  
  
  /* copying of cyclic Sercos data / ring monitoring is disabled */
  prCSMD_Instance->rPriv.eMonitoringMode     = CSMD_MONITORING_OFF;
  prCSMD_Instance->rPriv.boP1_active         = FALSE;
  prCSMD_Instance->rPriv.boP2_active         = FALSE;
  
  for (usI = 0; usI < CSMD_MAX_TEL; usI++)
  {
    prCSMD_Instance->rPriv.aboMDT_used[usI] = FALSE;    /* MDT telegram is not used */
    prCSMD_Instance->rPriv.aboAT_used[usI]  = FALSE;    /* AT telegram is not used */
  }
  
  prCSMD_Instance->usCSMD_Topology    = CSMD_NO_LINK;        /* flag for topology */
  prCSMD_Instance->sCSMD_Phase        = CSMD_SERC_PHASE_NRT_STATE;
  
  CSMD_HAL_SetPhase( &prCSMD_Instance->rCSMD_HAL,
                     CSMD_CPS_CURRENT_CP,
                     CSMD_SERC_PHASE_0 );
  
  CSMD_HAL_IntControl( &prCSMD_Instance->rCSMD_HAL, 
                       0,
                       TRUE,
                       FALSE,
                       0 );
  
  CSMD_HAL_IntControl( &prCSMD_Instance->rCSMD_HAL, 
                       0,
                       FALSE,
                       TRUE,
                       0 );
  
  CSMD_HAL_SetComMode(&prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_UC_RING_MODE);
  
  /* Descriptor unit disabled */
  /* No reason to delete Rx-Tx-Descriptors or events! */
  CSMD_HAL_CtrlDescriptorUnit(&prCSMD_Instance->rCSMD_HAL, FALSE, FALSE);
  
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
  
  /* Timer Off / Timing Master Mode / no CYC_CLK / no CON_CLK */
  CSMD_HAL_Initialize(&prCSMD_Instance->rCSMD_HAL);
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_ResetHardware */



/**************************************************************************/ /**
\brief Resets the configuration structure of the CoSeMa instance.

\ingroup func_init
\b Description: \n
   This function sets the configuration structure rConfiguration of the
   CoSeMa instance back to a defined state, including:
   - The indices of all lists arConnIdxList[] (master and slaves) are set to 0xFFFF
   - The structure parConnection[] is deleted (Exception: element usConnectionName[]
     is filled with spaces)
   - The structure arConfiguration[] is deleted (Exception: element usS_0_1050_SE7
     is initialized with 0xFFFF)
   - The structure arReadTimeBit[] is deleted.
   - The element usNbrOfConnections is set to zero for master configuration and
     all slave configurations

<B>Call Environment:</B> \n
   This function can be called at any time in CP0 to CP2.\n
   The call-up should be performed by a task

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_NO_ERROR \n
 
\author       WK
\date         09.12.2010

***************************************************************************** */
CSMD_FUNC_RET CSMD_Init_Config_Struct( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT usI;
  CSMD_ULONG  ulSize;

  CSMD_CONFIG_STRUCT *prCfg;
  
  
  /* Check current communication phase */
  if (prCSMD_Instance->sCSMD_Phase > CSMD_SERC_PHASE_2)
  {
    return (CSMD_WRONG_PHASE);
  }
  
  prCfg = &prCSMD_Instance->rConfiguration;
  
  /* Set default value 0xFFFF for all indices in master connection index list. */
  ulSize = sizeof( CSMD_CONN_IDX_STRUCT ) * prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster;
  
  (CSMD_VOID) CSMD_HAL_memset( prCfg->rMasterCfg.parConnIdxList,
                               0xFF,
                               ulSize );

  prCfg->rMasterCfg.usNbrOfConnections = 0;
  
#ifdef CSMD_CONFIG_PARSER
  prCfg->rMasterCfg.usFirstConfigParamIndex = 0xFFFF;
#endif
  /* Set default value 0xFFFF for all indexes in connection index list for all possible slaves. */
  ulSize = sizeof( prCfg->parSlaveConfig[0].arConnIdxList );
  
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
  {
    (CSMD_VOID) CSMD_HAL_memset( prCfg->parSlaveConfig[usI].arConnIdxList,
                                 0xFF,
                                 ulSize );
    
    prCfg->parSlaveConfig[usI].usNbrOfConnections = 0;
#ifdef CSMD_CONFIG_PARSER
    prCfg->parSlaveConfig[usI].usFirstConfigParamIndex = 0xFFFF;
#endif
  }
  
  /* Clear all connnections */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn; usI++)
  {
    prCfg->parConnection[usI].usTelegramType   = 0;
    prCfg->parConnection[usI].usS_0_1050_SE2   = 0;
    prCfg->parConnection[usI].usS_0_1050_SE3   = 0;
    prCfg->parConnection[usI].usS_0_1050_SE5   = 0;
    prCfg->parConnection[usI].ulS_0_1050_SE10  = 0;
    prCfg->parConnection[usI].usS_0_1050_SE11  = 0;
    prCfg->parConnection[usI].usApplicationID  = 0;
    (CSMD_VOID) CSMD_HAL_memset( prCfg->parConnection[usI].ucConnectionName,
                                 ' ',
                                 CSMD_CONN_NAME_LENGTH );
    prCfg->parConnection[usI].pvConnInfPtr     = NULL;
  }
  
  /* Clear all configurations */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig; usI++)
  {
    (CSMD_VOID) CSMD_HAL_memset( &prCfg->parConfiguration[usI],
                                 0,
                                 sizeof (CSMD_CONFIGURATION) );

    /* Set default value 0xFFFF for all S-0-1050.x.07 Assigned connection capability */
    prCfg->parConfiguration[usI].usS_0_1050_SE7 = 0xFFFF;
  }
  
  /* Clear all real time bit configurations */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig; usI++)
  {
    (CSMD_VOID) CSMD_HAL_memset( &prCfg->parRealTimeBit[usI],
                                 0,
                                 sizeof (CSMD_REALTIME_BIT) );
  }
  
#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
  /* Clear all references to parameter configuration lists */
  (CSMD_VOID) CSMD_HAL_memset( prCfg->parSlaveParamConfig,
                               0xFF,
                               sizeof(CSMD_SLAVE_PARAMCONFIG) * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams );

  /* Clear all parameter configuration lists */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList; usI++)
  {
    prCfg->parConfigParamsList[usI].usApplicationID = 0;
    
    (CSMD_VOID) CSMD_HAL_memset( &prCfg->parConfigParamsList[usI].ausParamTableIndex[0],
                                 0xFF,
                                 CSMD_MAX_PARAMS_IN_CONFIG_LIST * sizeof (CSMD_USHORT) );
  }
  
  /* Clear all  configuration parameters */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter; usI++)
  {
    /* Set all data to 0 */
    (CSMD_VOID) CSMD_HAL_memset( &prCfg->parConfigParam[usI],
                                 0,
                                 sizeof(CSMD_CONFIGURATION_PARAMETER) );
  }
#endif
  
#ifdef CSMD_CONFIG_PARSER
  /* Set the pointers initially to the CoSeMa structures */
  prCfg->prMaster_Config = &prCfg->rMasterCfg;
  prCfg->parSlave_Config = prCfg->parSlaveConfig;
  
  /* Clear the temporary used index list */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
  {
    prCSMD_Instance->rSlaveList.ausParserTempIdxList[usI] = 0U;
  }

  /* Clear the temporary used address list */
  for (usI = 0; usI < (CSMD_USHORT)(prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves + 2); usI++)
  {
    prCSMD_Instance->rSlaveList.ausParserTempAddList[usI]   = 0U;
  }
#endif
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Init_Config_Struct */



#ifdef CSMD_SOFT_MASTER
/**************************************************************************/ /**
\brief Selects the used IP-Core.

\ingroup func_init
\b Description: \n
  This access function selects the IP-Core to be used. \n

\note  The presetting (definition of \ref CSMD_SOFT_MASTER) will be
       taken over in the function CSMD_Initialize().\n
       Changes are possible before calling the function CSMD_InitHardware().

<B>Call Environment:</B> \n
   This function should be called from a task.

\param [in]   prCSMD_Instance
                Pointer to memory range allocated for the variables of the
                CoSeMa instance.
\param [in]   boSoftMaster
                TRUE &nbsp;: Sercos soft master is used instead of hard master IP-Core.\n
                FALSE:       Sercos hard master IP-Core SERCON100M is used.\n

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         11.02.2015

***************************************************************************** */
CSMD_FUNC_RET CSMD_UseSoftMaster( CSMD_INSTANCE *prCSMD_Instance,
                                  CSMD_BOOL      boSoftMaster )
{
  if (boSoftMaster == TRUE)
  {
    prCSMD_Instance->rPriv.boSoftMaster = TRUE;
  }
  else
  {
    prCSMD_Instance->rPriv.boSoftMaster = FALSE;
  }
  return (CSMD_NO_ERROR);

} /* End: CSMD_UseSoftMaster() */
#endif

/*! \endcond */ /* PUBLIC */



/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief  Reset memory allocation variables

\ingroup func_init
\b Description: \n
   - Reset call-back function pointers
   - Reset pointers to the CoSeMa structures
   - Reset base pointer to allocated system memory

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
                Pointer to memory range allocated for the variables of the
                CoSeMa instance

\return       none

\author       WK
\date         29.01.2014

***************************************************************************** */
CSMD_VOID CSMD_ResetMemAllocManagement( CSMD_INSTANCE *prCSMD_Instance )
{

  prCSMD_Instance->rPriv.rMemAlloc.rCB_FuncTable.fpCSMD_set_mem_ptr = NULL;
/*prCSMD_Instance->rPriv.rMemAlloc.rCB_FuncTable                    = NULL;*/
/*prCSMD_Instance->rPriv.rMemAlloc.rCB_FuncTable                    = NULL;*/

#ifndef CSMD_STATIC_MEM_ALLOC
  prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList = NULL;
  prCSMD_Instance->rConfiguration.parSlaveConfig            = NULL;
  prCSMD_Instance->rConfiguration.parConnection             = NULL;
  prCSMD_Instance->rConfiguration.parConfiguration          = NULL;
  prCSMD_Instance->rConfiguration.parRealTimeBit            = NULL;
  #if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
  prCSMD_Instance->rConfiguration.parSlaveParamConfig       = NULL;
  prCSMD_Instance->rConfiguration.parConfigParamsList       = NULL;
  prCSMD_Instance->rConfiguration.parConfigParam            = NULL;
  #endif

  prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList     = NULL;
  prCSMD_Instance->rPriv.parRdWrBuffer                      = NULL;
  prCSMD_Instance->rPriv.parConnMasterProd                  = NULL;
  prCSMD_Instance->rPriv.parConnSlaveProd                   = NULL;

  #if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
  prCSMD_Instance->rPriv.parSoftSvcContainer                = NULL;
  prCSMD_Instance->rPriv.parSoftSvc                         = NULL;
  #endif
  prCSMD_Instance->parSvchMngmtData                         = NULL;
  #ifdef CSMD_CONFIG_PARSER
  prCSMD_Instance->rPriv.parSlaveInst                       = NULL;
  prCSMD_Instance->rPriv.paucSlaveSetupManipulated          = NULL;
  prCSMD_Instance->rPriv.rUsedCfgs.paucConnNbrUsed          = NULL;
  prCSMD_Instance->rPriv.rUsedCfgs.paucConnUsed             = NULL;
  prCSMD_Instance->rPriv.rUsedCfgs.paucConfUsed             = NULL;
  prCSMD_Instance->rPriv.rUsedCfgs.paucRTBtUsed             = NULL;
  prCSMD_Instance->rPriv.rUsedCfgs.paucSetupParamsListUsed  = NULL;
  prCSMD_Instance->rPriv.rUsedCfgs.paucSetupParamsUsed      = NULL;
  #endif
#endif
  prCSMD_Instance->rPriv.rMemAlloc.pulBase                  = NULL;

} /* end: CSMD_ResetMemAllocManagement */



/**************************************************************************/ /**
\brief Set the required system limits

\ingroup func_init
\b Description: \n
   This function checks and assumes the required system limits.
   - prSysLimits == NULL \n
     Take the default (maximum) values from CSMD_USER.h
   - otherwise \n
     Checks and assumes the required system limits.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
                Pointer to memory range allocated for the variables of the
                CoSeMa instance
\param [in]   prSysLimits
                Structure with system limitation variables

\return       CSMD_INVALID_SYSTEM_LIMITS \n
              CSMD_NO_ERROR \n

\author       WK
\date         29.01.2014

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_SetSystemLimits( CSMD_INSTANCE             *prCSMD_Instance,
                                    CSMD_SYSTEM_LIMITS_STRUCT *prSysLimits )
{
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;

  if (prSysLimits == NULL)
  {
    prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster        = CSMD_MAX_CONNECTIONS_MASTER;
    prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves            = CSMD_MAX_SLAVES;
    prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn          = CSMD_MAX_GLOB_CONN;
    prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig        = CSMD_MAX_GLOB_CONFIG;
    prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig       = CSMD_MAX_RT_BIT_CONFIG;
#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
    prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams = CSMD_MAX_SLAVE_CONFIGPARAMS;
    prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList  = CSMD_MAX_CONFIGPARAMS_LIST;
    prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter   = CSMD_MAX_CONFIG_PARAMETER;
#endif
    prCSMD_Instance->rPriv.rSystemLimits.usMaxProdConnCC        = CSMD_MAX_PROD_CONN_CC;
  }
#ifndef CSMD_STATIC_MEM_ALLOC
  else
  {
    if (   (prSysLimits->usMaxConnMaster        > CSMD_MAX_CONNECTIONS_MASTER)
        || (prSysLimits->usMaxConnMaster        < 2)
        || (prSysLimits->usMaxSlaves            > CSMD_MAX_SLAVES)
        || (prSysLimits->usMaxSlaves            < 1)
        || (prSysLimits->usMaxGlobConn          > CSMD_MAX_GLOB_CONN)
        || (prSysLimits->usMaxGlobConn          < 2)
        || (prSysLimits->usMaxGlobConfig        > CSMD_MAX_GLOB_CONFIG)
        || (prSysLimits->usMaxGlobConfig        < 4)
        || (prSysLimits->usMaxRtBitConfig       > CSMD_MAX_RT_BIT_CONFIG)
        || (prSysLimits->usMaxRtBitConfig       < 1)
#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
        || (prSysLimits->usMaxSlaveConfigParams > CSMD_MAX_SLAVE_CONFIGPARAMS)
        || (prSysLimits->usMaxSlaveConfigParams < 1)
        || (prSysLimits->usMaxConfigParamsList  > CSMD_MAX_CONFIGPARAMS_LIST)
        || (prSysLimits->usMaxConfigParamsList  < 1)
        || (prSysLimits->usMaxConfigParameter   > CSMD_MAX_CONFIG_PARAMETER)
        || (prSysLimits->usMaxConfigParameter   < 1)
#endif
        || (prSysLimits->usMaxProdConnCC        > CSMD_MAX_PROD_CONN_CC)
        || (prSysLimits->usMaxProdConnCC        < 1)
        )
    {
      eFuncRet = CSMD_INVALID_SYSTEM_LIMITS;
    }
    else
    {
      prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster        = prSysLimits->usMaxConnMaster;
      prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves            = prSysLimits->usMaxSlaves;
      prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn          = prSysLimits->usMaxGlobConn;
      prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig        = prSysLimits->usMaxGlobConfig;
      prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig       = prSysLimits->usMaxRtBitConfig;
#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
      prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams = prSysLimits->usMaxSlaveConfigParams;
      prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList  = prSysLimits->usMaxConfigParamsList;
      prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter   = prSysLimits->usMaxConfigParameter;
#endif
      prCSMD_Instance->rPriv.rSystemLimits.usMaxProdConnCC        = prSysLimits->usMaxProdConnCC;
    }
  }
#endif

  return (eFuncRet);

} /* end: CSMD_SetSystemLimits */
/*lint -restore const! */



/**************************************************************************/ /**
\brief  Get the call-back function pointers for memory allocation.

\ingroup func_init
\b Description: \n

<B>Call Environment:</B> \n

   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
                Pointer to memory range allocated for the variables of the
                CoSeMa instance
\param [in]   prMemAllocCB_Table
                Structure with call-back function pointers:
                - fpCSMD_set_mem_ptr
                  Get pointer to the required memory for CoSeMa structures.
                - fpCSMD_malloc (not realized)
                - fpCSMD_free   (not realized)

\return       \ref CSMD_MEMORY_ALLOCATION_FAILED \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         30.01.2014

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_InitMemoryAllocCB( CSMD_INSTANCE            *prCSMD_Instance,
                                      CSMD_MEM_ALLOC_CB_STRUCT *prMemAllocCB_Table )
{
  if (prMemAllocCB_Table->fpCSMD_set_mem_ptr == NULL)
  {
    return (CSMD_MEMORY_ALLOCATION_FAILED);
  }
  else
  {
    prCSMD_Instance->rPriv.rMemAlloc.rCB_FuncTable.fpCSMD_set_mem_ptr = prMemAllocCB_Table->fpCSMD_set_mem_ptr;
    /*prCSMD_Instance->rPriv.rMemAlloc.rCB_FuncTable.fpCSMD_malloc      = prMemAllocCB_Table->fpCSMD_malloc;*/
    /*prCSMD_Instance->rPriv.rMemAlloc.rCB_FuncTable.fpCSMD_free        = prMemAllocCB_Table->fpCSMD_free;*/
    return (CSMD_NO_ERROR);
  }

} /* end: CSMD_InitMemoryAllocCB */
/*lint -restore const! */



/**************************************************************************/ /**
\brief  Memory calculation/allocation for CoSeMa arrays/structures.

\ingroup func_init
\b Description: \n
   This function
    - Compare configuration variables with limits defined in csmd_user.h
    - Allocate memory from master-stack for all dynamic allocated memory blocks
      of CoSeMa.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance.
\param [in]   boAllocate
              - FALSE \n
                Calculate the required memory space for the CoSeMa array/structures.
              - TRUE \n
                Distributes the allocated memory to the CoSeMa arrays/structures
                and calculate corresponding pointers.
\param [out]  plBytes

\return       \ref CSMD_MEMORY_ALLOCATION_FAILED \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         29.01.2014

***************************************************************************** */
CSMD_FUNC_RET CSMD_Ptr_MemoryAllocation( CSMD_INSTANCE *prCSMD_Instance,
                                         CSMD_BOOL      boAllocate,
                                         CSMD_LONG     *plBytes )
{

  CSMD_ULONG  ulSumSize = 0;
  CSMD_ULONG *pulBase   = prCSMD_Instance->rPriv.rMemAlloc.pulBase;

  if (boAllocate == TRUE)
  {
    /* Check, if pointer to memory for CoSeMa structures is initialized */
    if (NULL == pulBase)
    {
      return (CSMD_MEMORY_ALLOCATION_FAILED);
    }
  }

#ifndef CSMD_STATIC_MEM_ALLOC
  /* Initialize pointer to List of master connections in the master configuration */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_CONN_IDX_STRUCT) * prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList );

  /* Initialize pointer to list of slave configurations */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_SLAVE_CONFIGURATION) * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rConfiguration.parSlaveConfig );

  /* Initialize pointer to list of connections */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_CONNECTION) * prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rConfiguration.parConnection );

  /* Initialize pointer to list of configurations */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_CONFIGURATION) * prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rConfiguration.parConfiguration );

  /* Initialize pointer to list of real-time bit configurations */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_REALTIME_BIT) * prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rConfiguration.parRealTimeBit );


  #if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
  /* Initialize pointer to References to parameters list */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_SLAVE_PARAMCONFIG) * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rConfiguration.parSlaveParamConfig );

  /* Initialize pointer to List with references to parameters */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_CONFIGPARAMS_LIST) * prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rConfiguration.parConfigParamsList );

  /* Initialize pointer to List with references to parameters */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_CONFIGURATION_PARAMETER) * prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rConfiguration.parConfigParam );
  #endif

  /* list of CC connections needed for descriptor assignment */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_CC_CONN_LIST) * prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList );

  /* Temporary data buffer for service channel macro function calls */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_RD_WR_BUFFER) * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.parRdWrBuffer );

  /* Initialize pointer to internal structure for master-produced connections */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_CONN_MASTERPROD) * prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.parConnMasterProd );

  /* Initialize pointer to internal structure for slave-produced connections */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_CONN_SLAVEPROD) * prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster,  /* !!! CSMD_MAX_CONNECTIONS_MASTER */
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.parConnSlaveProd );

  #if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
  /* Initialize pointer to list soft service container */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_SERC3SVC) * (prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves - CSMD_MAX_HW_CONTAINER),
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.parSoftSvcContainer );

  /* Initialize pointer to list Soft SVC variable structure */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_SOFT_SVC_STRUCT) * (prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves - CSMD_MAX_HW_CONTAINER),
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.parSoftSvc );
  #endif  /* #if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER */

  /* Initialize pointer to list of service channel management structures */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_SVCH_MNGMT_STRUCT) * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->parSvchMngmtData );

  #ifdef CSMD_CONFIG_PARSER
  /* todo description */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_SLAVE_INST_MANIPULATED) * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.parSlaveInst );

  /* todo description */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.paucSlaveSetupManipulated );

  /* todo description */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.rUsedCfgs.paucConnNbrUsed );

  /* todo description */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.rUsedCfgs.paucConnUsed );

  /* todo description */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.rUsedCfgs.paucConfUsed );

  /* todo description */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.rUsedCfgs.paucRTBtUsed );

  /* todo description */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.rUsedCfgs.paucSetupParamsListUsed );

  /* todo description */
  CSMD_Calc_Alloc_Mem( boAllocate,
                       sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter,
                       &ulSumSize,
                       (CSMD_VOID *)pulBase,
                       (CSMD_VOID *)&prCSMD_Instance->rPriv.rUsedCfgs.paucSetupParamsUsed );
  #endif

  /* Add space to guarantee long alignment of the pointer to the allocated memory. */
  ulSumSize += sizeof(CSMD_ULONG);

#endif  /* #ifndef CSMD_STATIC_MEM_ALLOC */

  /* Memory space needed for CoSeMa structures */
  *plBytes = (CSMD_LONG)ulSumSize;
  prCSMD_Instance->rPriv.rMemAlloc.ulAllocatedMemory = ulSumSize;


  return (CSMD_NO_ERROR);

} /* end: CSMD_Ptr_MemoryAllocation */



/**************************************************************************/ /**
\brief  Memory allocation for one CoSeMa array/structure.

\ingroup func_init
\b Description: \n
   This function

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]     boAllocate
                - FALSE \n
                  Calculate the required memory space for the CoSeMa array/structures.
                - TRUE \n
                  Distributes the allocated memory to the CoSeMa arrays/structures
                  and calculate corresponding pointers.
\param [in]     ulSize
                  Size of the memory block
\param [in,out] pulSumSize
                  Summed size of the memory
\param [in]     pvBase
                  Begin of allocated memory for CoSeMa
param [in,out]  ppvAllocMem
                  Calculated begin of allocated memory for the requested block
\return         none

\author       WK
\date         10.02.2014

***************************************************************************** */
CSMD_VOID CSMD_Calc_Alloc_Mem( CSMD_BOOL    boAllocate,
                               CSMD_ULONG   ulSize,
                               CSMD_ULONG  *pulSumSize,
                               CSMD_VOID   *pvBase,
                               CSMD_VOID  **ppvAllocMem )
{
  if (boAllocate == TRUE)
  {
    /* Calculate pointer */
    *ppvAllocMem = (CSMD_VOID *)((CSMD_UCHAR *)pvBase + *pulSumSize);
  }
  /* Calculate sum of memory (offset to next block) */
  *pulSumSize += (ulSize + 3U) & ~3U;

} /* end: CSMD_Calc_Alloc_Mem */


/*! \endcond */ /* PRIVATE */




/*
--------------------------------------------------------------------------------
  Modification history
--------------------------------------------------------------------------------

01 Sep 2010
  - Refactoring of CoSeMa files.
06 Feb 2014 WK
  WP-00044851 - Miscellaneous code clean-up for CoSeMa 6.1
  - CSMD_InitHardware()
    Removed partially initialization of rConfiguration, since the pointer
    to memory of CoSeMa configuration structures are unknown at this time.
  - CSMD_InitHardware()
    Initialize rConfiguration by calling the function CSMD_Init_Config_Struct().
05 Aug 2014 AlM
  Dynamic memory assignment:
  - Removed pointer calculation
    rOffsetList members
      parProdConnMDT
      parProdConnAT
      parProdCC_ConnAT
      parProdConnAT_Master
  - Added memory assignment:
    parCC_ConnList
13 Nov 2014 WK
 - Defdb00000000
   CSMD_Initialize(): Removed test code for defined CSMD_DEBUG.
24 Nov 2014 WK
  - CSMD_InitHardware()
    Added missing set to default values of rPriv.rHW_Init_Struct.
11 Feb 2015 WK
  - Defdb00176768
    CSMD_Initialize():
      Set CSMD_PRIV.boSoftmaster depending on define CSMD_SOFT_MASTER.
    CSMD_InitHardware():
      Accept CSMD_PRIV.boSoftmaster in the structure CSMD_HAL.
    Added access function CSMD_UseSoftMaster().
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 Feb 2015 WK
  - CSMD_Ptr_MemoryAllocation()
    Fixed size of memory needed for
      parSlaveInst
      paucSlaveSetupManipulated
      paucCSMD_ConnNbrUsed
      paucCSMD_ConnUsed
      paucCSMD_ConfUsed
      paucCSMD_RTBtUsed
      paucCSMD_SetupParamsListUsed
      paucCSMD_SetupParamsUsed
28 Apr 2015 WK
  - Defdb00178597
    CSMD_Initialize()
    Added initialization of FPGA base offset for TxRam, RxRam and SVC Ram.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
23 Jul 2015 WK
  - Defdb00178597
    CSMD_InitMemory(), CSMD_Calc_Alloc_Mem():
    Fixed type castings for 64-bit systems.
04 Aug 2015 WK
  - Defdb00182038
    CSMD_Ptr_MemoryAllocation()
    Excessive memory consumption caused by incorrect initialization
    of CSMD_SLAVE_INST_MANIPULATED *parSlaveInst.
14 Oct 2015 WK
  - Defdb00182283
    CSMD_InitHardware()
    Removed CSMD_HAL_Initialize(). This functionality is achieved by the
    soft reset of the IP core.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
08 Mar 2016 WK
  - Defdb00185518
    Encapsulation of function CSMD_UseSoftMaster() with CSMD_SOFT_MASTER.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
14 Jul 2016 WK
 - FEAT-00051878 - Support for Fast Startup
   CSMD_Init_Config_Struct():
   Deleting parSlaveConfig[].usSettings moved to CSMD_InitHardware()
   and CSMD_SetPhase2().
27 Oct 2016
  - Defdb00182067
    Adjust include of CSMD_HAL_PRIV.h for SoftMaster.
  
--------------------------------------------------------------------------------
*/
