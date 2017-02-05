/**
 * \file      SIII_PHASE.c
 *
 * \brief     Sercos III soft master stack - Phase handler and related 
 *            functions
 *
 * THIS SOFTWARE IS PROVIDED "AS IS"; WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY;
 * FITNESS FOR A PERTICULAR PURPOSE AND NONINFRINGEMENT. THE AUTHORS OR COPYRIGHT
 * HOLDERS SHALL NOT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE;
 * UNLESS STIPULATED BY MANDATORY LAW.
 *
 * \ingroup   SIII
 *
 * \author    GMy, partially based on earlier work by SBe
 *
 * \copyright Copyright Bosch Rexroth AG, 2012-2016
 *
 * \date      2012-10-11
 *
 * \version 2012-10-11 (GMy): Baseline for CoSeMa v4
 * \version 2012-10-31 (GMy): Updated for CoSeMa v5
 * \version 2012-11-05 (GMy): Added slave application support
 * \version 2013-01-21 (GMy): SVC support added
 * \version 2013-02-05 (GMy): Hot-plug support added
 * \version 2013-02-12 (GMy): Separation of phase handler and user interface
 * \version 2013-02-28 (GMy): Lint code optimization
 * \version 2013-04-11 (GMy): Module re-arrangement
 * \version 2013-05-07 (GMy): Added support for INtime
 * \version 2013-05-22 (GMy): Bugfix in SIII_PhaseHandler().
 * \version 2013-05-27 (GMy): Adaptations for CoSeMa SWC extensions in
 *                            SIII_PhaseHandler().
 * \version 2013-06-06 (GMy): Added support for RTX and Kithara
 * \version 2013-06-20 (GMy): Optimization of error handling
 * \version 2013-06-21 (GMy): Added support for Windows desktop versions and
 *                            QNX
 * \version 2013-11-04 (GMy): Bugfix (Hot-Plug called cycle-exact)
 * \version 2014-04-01 (GMy): Added support for CoSeMa 6VRS
 * \version 2014-09-12 (GMy): Improved connection handling
 * \version 2015-03-19 (GMy): Modifications due to dynamic handling of
 *                            several former static SIII settings
 * \version 2015-11-24 (GMy): Added support for Sercos ring recovery
 * \version 2016-10-27 (AlM): Support for CoSeMa V5 removed.
 */

//---- includes ---------------------------------------------------------------

#include "../SIII/SIII_GLOB.h"
#include "../SIII/SIII_PRIV.h"

#ifdef __qnx__
#include "../RTQX/RTQX_GLOB.h"
#elif defined __unix__
#include "../RTLX/RTLX_GLOB.h"
#elif defined WINCE7
#include "../RTC7/RTC7_GLOB.h"
#elif defined WINCE
#include "../RTC6/RTC6_GLOB.h"
#elif defined __INTIME__
#include "../RTIT/RTIT_GLOB.h"
#elif defined __RTX__
#include "../RTRX/RTRX_GLOB.h"
#elif defined __VXWORKS__
#include "../RTVW/RTVW_GLOB.h"
#include "../RTVW/RTVW_S3SM_GLOB.h"
#elif defined __KITHARA__
#include "../RTKT/RTKT_GLOB.h"
#elif defined WIN32
#include "../RTWI/RTWI_GLOB.h"
#elif defined WIN64
#include "../RTWI/RTWI_GLOB.h"
#else
#error Operating system not supported by SIII!
#endif

//---- defines ----------------------------------------------------------------

// Helper macros
#define SIII_ULONG_NS_TO_DOUBLE_MS(_UlongNs)        (((DOUBLE)_UlongNs) /       \
                                                    ((DOUBLE) 1000.0*1000.0))


//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SIII_FUNC_RET SIII_PhaseHandler(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \private
 *
 * \brief   Sercos phase handler, shall be called cyclically every
 *          SIII_PHASE_HANDLER_WAIT_TIME ns.
 *
 * \note    SIII_PHASE_HANDLER_WAIT_TIME shall not be smaller than the Sercos
 *          cycle time!
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \details This cyclic function controls the progression through the Sercos
 *          communication phases as well as hot-plug of Sercos
 *          devices. It also initializes the CoSeMa instance. The function
 *          shall be called cyclically every SIII_INT_WAIT_TIME us during
 *          active commands and may be controlled by the calling entity using
 *          the phase state structure SIII_PHASE_STATE_STRUCT that is included
 *          in the SIII_INSTANCE_STRUCT, specifically by setting the
 *          ucCsmdStateNew field accordingly. The function pointer array
 *          afpAppConnConfig within the SIII instance structure is used to call
 *          application-specific Sercos III connection configuration functions
 *          during the Sercos phase startup.
 *
 *          This function is private. Applications should not call this
 *          function directly and neither change the SIII_PHASE_STATE_STRUCT
 *          directly, but use SIII_PhaseSwitch() instead. This function sets
 *          SIII_PHASE_STATE_STRUCT accordingly and calls SIII_PhaseHandler()
 *          as needed.
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:                    Success
 *          - SIII_SERCOS_PHASE_ERROR:          When an illegal phase switch is
 *                                              attempted
 *          - Inherited CSMD_FUNC_RET value:    When a CoSeMa error occurred
 *
 * \ingroup SIII
 *
 * \author  GMy, partially based on earlier work by SBe
 *
 * \date    2012-10-15
 *
 * \version 2012-10-11 (GMy): Baseline for CoSeMa v4
 * \version 2012-10-31 (GMy): Updated for CoSeMa v5
 * \version 2012-11-05 (GMy): Added slave application support
 * \version 2013-01-21 (GMy): SVC support added
 * \version 2013-02-05 (GMy): Hot-plug support added
 * \version 2013-02-12 (GMy): Separation of phase handler and user interface
 * \version 2013-03-04 (GMy): Added retry functionality for Sercos startup
 * \version 2013-04-25 (GMy): Optimized switch-back to CP0 to avoid potential
 *                            slave error
 * \version 2013-05-22 (GMy): Bugfix. Corrected initialization of length fields
 *                            in MDT and AT management structs
 * \version 2013-05-27 (GMy): Adaptations for CoSeMa SWC extensions
 * \version 2013-06-12 (GMy): Added slave error clear on startup
 * \version 2013-07-03 (GMy): Change to ensure that CSMD calling interval is
 *                            never smaller than Sercos cycle time
 * \version 2013-08-05 (GMy): Added support for device-independent (global)
 *                            callbacks
 * \version 2013-11-04 (GMy): Bugfix (Hot-Plug called cycle-exact)
 * \version 2014-04-01 (GMy): Added support for CoSeMa 6VRS
 * \version 2015-01-15 (GMy): Use of new define SIII_SOFT_MASTER_JITTER_NS
 * \version 2015-07-09 (GMy): Code optimization
 * \version 2015-11-24 (GMy): Added support for Sercos ring recovery
 */
SIII_FUNC_RET SIII_PhaseHandler
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  CSMD_FUNC_RET           eCosemaFuncRet        = CSMD_NO_ERROR;
                                                            // CoSeMa function return value
  INT                     iCnt                  = 0;        // Counter variable
  USHORT                  usMasterConnOffset    = 0;        // Master connection configuration offset
  SIII_FUNC_RET           eErrorCode            = SIII_NO_ERROR;
                                                            // Error code that occurred during function
  SIII_PHASE_STATE_STRUCT *prPhaseStateStruct;      // Shortcut pointer
  SICE_INSTANCE_STRUCT    *prSiceInstance;          // Shortcut pointer
  CSMD_FUNC_STATE         *prCosemaFuncState;       // Shortcut pointer
  CSMD_INSTANCE           *prCosemaInstance;        // Shortcut pointer
  CSMD_HW_INIT_STRUCT     *prCosemaHWInitStruct;    // Shortcut pointer

  SIII_VERBOSE(3, "SIII_PhaseHandler()\n");

  // Get shortcut pointers
  prPhaseStateStruct    = &prS3Instance->rPhaseStateStruct;
  prSiceInstance        = &prS3Instance->rSiceInstance;
  prCosemaFuncState     = &prS3Instance->rCosemaFuncState;
  prCosemaInstance      = &prS3Instance->rCosemaInstance;
  prCosemaHWInitStruct  = &prS3Instance->rCosemaInstance.rHW_Settings;

  // If state handler is in idle mode, check requested Sercos / CoSeMa state
  // and react accordingly
  if (
      (prPhaseStateStruct->ucPhaseState == (UCHAR) SIII_PHASE_STATE_IDLE)
     )
  {
    // 'Normal' Sercos phase progression requested
    if (
        (prPhaseStateStruct->ucCsmdStateNew > prPhaseStateStruct->ucCsmdStateCurr)    &&
        (prPhaseStateStruct->ucCsmdStateNew <= (UCHAR) SIII_CSMD_STATE_SET_PHASE4)
      )
    {
      prPhaseStateStruct->ucCsmdStateCurr++;
      prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_STARTED;
    }
    // Hotplug only (without TRANS_HP2_PARA) requested
    else if
      (
        (prPhaseStateStruct->ucCsmdStateNew == (UCHAR) SIII_CSMD_STATE_HOTPLUG)        &&
        (prPhaseStateStruct->ucCsmdStateCurr == (UCHAR) SIII_CSMD_STATE_SET_PHASE4)
      )
    {
      prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_HOTPLUG;
      prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_STARTED;
    }
    // Hotplug with TRANS_HP2_PARA requested
    else if
      (
        (prPhaseStateStruct->ucCsmdStateNew == (UCHAR) SIII_CSMD_STATE_TRANS_HP2_PARA)   &&
          (
            (prPhaseStateStruct->ucCsmdStateCurr == (UCHAR) SIII_CSMD_STATE_SET_PHASE4)  ||
            (prPhaseStateStruct->ucCsmdStateCurr == (UCHAR) SIII_CSMD_STATE_HOTPLUG)
          )
      )
    {
      prPhaseStateStruct->ucCsmdStateCurr++;
      prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_STARTED;
    }
    // Ring recovery requested
    else if
      (
        (prPhaseStateStruct->ucCsmdStateNew == (UCHAR) SIII_CSMD_STATE_RING_RECOVERY)
      )
    {
      prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_RING_RECOVERY;
      prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_STARTED;
    }

    // Phase switch-back requested
    else if (prPhaseStateStruct->ucCsmdStateNew <= prPhaseStateStruct->ucCsmdStateCurr) // <= by AlM Switch to CP0
    {
      if (prPhaseStateStruct->ucCsmdStateNew == (UCHAR) SIII_CSMD_STATE_SET_PHASE0)
      {
        // Switch slave output enable off
        (VOID)SIII_DevicePower
              (
                prS3Instance,
                SIII_ALL_DEVICES,
                FALSE
              );
        SIII_VERBOSE(0, "Slave output switched off\n");

        SIII_VERBOSE
             (
               1,
               "Waiting %dms for device power-down\n",
               prS3Instance->rS3Pars.ulSwitchBackDelay / 1000
             );

        RTOS_SimpleMicroWait(prS3Instance->rS3Pars.ulSwitchBackDelay);

        prPhaseStateStruct->boSwitchBackCP = TRUE;
        prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_PHASE0;
        prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_STARTED;
      }
      else if
        (
          (prPhaseStateStruct->ucCsmdStateNew == (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE)   &&
          (prPhaseStateStruct->ucCsmdStateCurr == (UCHAR) SIII_CSMD_STATE_SET_PHASE0)
        )
      {
        prPhaseStateStruct->boSwitchBackCP = TRUE;
        prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
        prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_STARTED;
      }
      else
      {
        return(SIII_STATE_ERROR);
      }
    }
    else
    {
      return(SIII_STATE_ERROR);
    }
  }

  // Sercos / CoSeMa phase state machine
  switch (prPhaseStateStruct->ucPhaseState)
  {
    case SIII_PHASE_STATE_IDLE:
      eErrorCode = SIII_NO_ERROR;
      break;

    case SIII_PHASE_STATE_STARTED:
      // CoSeMa API functions without state machine
      switch (prPhaseStateStruct->ucCsmdStateCurr)
      {
        case SIII_CSMD_STATE_INITIALIZE:

          // Initialize CoSeMa instance
          (VOID) memset
              (
                prCosemaInstance,
                (UCHAR) 0x00,
                sizeof(CSMD_INSTANCE)
              );

          {
            CSMD_INIT_POINTER *prSercosInitPtr = &prS3Instance->rCosemaSercosInitPtr;
            prSercosInitPtr->pvSERCOS_RX_Ram = (VOID*) (prSiceInstance->rMemory.aucRxRAM);
            prSercosInitPtr->pvSERCOS_TX_Ram = (VOID*) (prSiceInstance->rMemory.aucTxRAM);
            prSercosInitPtr->pvSERCOS_SVC_Ram = (VOID*) (prSiceInstance->rMemory.aucSvc);
            prSercosInitPtr->pvSERCOS_Register = (VOID*) (prSiceInstance->rMemory.aucRegister);

            eCosemaFuncRet = CSMD_Initialize
              (
                prCosemaInstance,   // CoSeMa instance
                prSercosInitPtr     // Sercos soft master RAM image
              );
          }
          SIII_VERBOSE
              (
                0,
                "CSMD_Initialize - return value = 0x%x\n",
                eCosemaFuncRet
              );

          if (eCosemaFuncRet != CSMD_NO_ERROR)
          {
            // Stop automatic startup when error occurs
            eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
            prPhaseStateStruct->ucCsmdStateNew  = (UCHAR) SIII_CSMD_STATE_IDLE;
            prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_IDLE;
          }
          else
          {
            prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_INIT_HARDWARE;
          }
          break;

        case SIII_CSMD_STATE_INIT_HARDWARE:

          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;
          break;

        case SIII_CSMD_STATE_SET_COMM_PARAM:

          prCosemaHWInitStruct->usSVC_BusyTimeout = prS3Instance->rS3Pars.usSVCBusyTimeout;
          prCosemaHWInitStruct->usRxBufferMode = (USHORT) CSMD_RX_SINGLE_BUFFER;
          prCosemaHWInitStruct->usTxBufferMode = (USHORT) CSMD_TX_SINGLE_BUFFER;
          prCosemaHWInitStruct->usSVC_Valid_TOut_CP1 = (USHORT) SIII_TIMEOUT_SVC_VALID_CP1;

          prCosemaHWInitStruct->ulCycleTime_CP0 = prS3Instance->rS3Pars.ulCycleTimeCP0CP2;
          prCosemaHWInitStruct->ulCycleTime_CP12 = prS3Instance->rS3Pars.ulCycleTimeCP0CP2;
          prCosemaHWInitStruct->ulUCC_Width = prS3Instance->rS3Pars.ulUCCBandwidth;
          prCosemaHWInitStruct->usIP_MTU_P34 = prS3Instance->rS3Pars.usMTU;

#ifdef CSMD_SWC_EXT
            prCosemaHWInitStruct->boFastCPSwitch = FALSE;
            prCosemaHWInitStruct->ulCycleTime_CP0 = prS3Instance->rS3Pars.ulCycleTimeCP0;
  #ifdef SICE_OPT_UCC_CP1_2
            prCosemaHWInitStruct->eUCC_Mode_CP12 = CSMD_UCC_MODE_CP12_1_VAR;
  #else
            prCosemaHWInitStruct->eUCC_Mode_CP12 = CSMD_UCC_MODE_CP12_FIX;
            //prCosemaHWInitStruct->eUCC_Mode_CP12 =
            //    CSMD_UCC_MODE_CP12_1;
  #endif
#endif

#ifdef CSMD_4MDT_4AT_IN_CP1_2
          prCosemaHWInitStruct->boFourMDT_AT_CP12   = FALSE;
#endif

#ifdef CSMD_HOTPLUG
          // Only for hotplug enabled in CoSeMa: enable it at startup.
          prCosemaHWInitStruct->boHotPlug = TRUE;
#endif

          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;
          break;

        case SIII_CSMD_STATE_SET_NRT_MODE:

          // Switch back flags for cyclic CoSeMa calls
          prS3Instance->rCyclicCommCtrl.boAppDataValid     = FALSE;
          prS3Instance->rCyclicCommCtrl.boCallWriteMDT     = FALSE;
          prS3Instance->rCyclicCommCtrl.boCallReadAT       = FALSE;
          prS3Instance->rCyclicCommCtrl.boCallTxRxSoftCont = FALSE;

          prPhaseStateStruct->boSwitchBackCP               = FALSE;

          eCosemaFuncRet = CSMD_Set_NRT_State
              (
                prCosemaInstance
              );

          SIII_VERBOSE
              (
                0,
                "CSMD_Set_NRT_Mode - return value = 0x%x\n",
                eCosemaFuncRet
              );

          if (eCosemaFuncRet != CSMD_NO_ERROR)
          {
            // Stop automatic startup when error occurs
            eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
            prPhaseStateStruct->ucCsmdStateNew  =
                (UCHAR) (SIII_CSMD_STATE_SET_NRT_MODE - 1);
            prPhaseStateStruct->ucCsmdStateCurr =
                (UCHAR) (SIII_CSMD_STATE_SET_NRT_MODE - 1);
          }

          prPhaseStateStruct->ucPhaseState  =
              (UCHAR) SIII_PHASE_STATE_IDLE;
          break;

        case SIII_CSMD_STATE_SET_PHASE0:

          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;
          prCosemaInstance->rConfiguration.rMasterCfg.usMaxNbrTelErr =
              prS3Instance->rS3Pars.usAccTelLosses;

#ifdef CSMD_SWC_EXT
          SIII_VERBOSE
              (
                0,
                "Setting Sercos cycle time to %gms\n",
                SIII_ULONG_NS_TO_DOUBLE_MS(prCosemaHWInitStruct->ulCycleTime_CP0)
              );
#else
          SIII_VERBOSE
              (
                0,
                "Setting Sercos cycle time to %gms\n",
                SIII_ULONG_NS_TO_DOUBLE_MS(prCosemaHWInitStruct->ulCycleTime_CP0)
              );
#endif
          /*
          // If switching back from higher phase
          if (prPhaseStateStruct->boSwitchBackCP)
          {
            // Switch back flags for cyclic CoSeMa calls
            prS3Instance->rCyclicCommCtrl.boCallTxRxSoftCont = FALSE;
            prS3Instance->rCyclicCommCtrl.boAppDataValid     = FALSE;
          }
          */
          break;

        case SIII_CSMD_STATE_INIT_CONFIG_STRUCT:

          eCosemaFuncRet = CSMD_Init_Config_Struct(prCosemaInstance);

          SIII_VERBOSE
              (
                0,
                "CSMD_Init_Config_Struct - return value = 0x%x\n",
                eCosemaFuncRet
              );

          if (eCosemaFuncRet != CSMD_NO_ERROR)
          {
            // Stop automatic startup when error occurs
            eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
            prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
            prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
            break; // AlM
          }
          else
          {
            // Start calling CSMD_ReadAT every cycle
            prS3Instance->rCyclicCommCtrl.boCallReadAT = TRUE;
          }
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;
          break;

        case SIII_CSMD_STATE_SET_PHASE1:

          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;
#ifdef CSMD_SWC_EXT
          SIII_VERBOSE
              (
                0,
                "Setting Sercos cycle time to %gms\n",
                SIII_ULONG_NS_TO_DOUBLE_MS(prCosemaHWInitStruct->ulCycleTime_CP0)
              );
#endif
          break;

        case SIII_CSMD_STATE_SET_PHASE2:

          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;
          break;

        case SIII_CSMD_STATE_CHECK_VERSION:

          // Initialize SVCH_MACRO_STRUCT
          (VOID) memset
              (
                &prS3Instance->arCosemaSvcMacro[0],
                (UCHAR) 0x00,
                sizeof(prS3Instance->arCosemaSvcMacro)
              );

          if (prS3Instance->rS3Pars.boClrErrOnStartup)
          {
            // Clear errors on all slaves
            SIII_VERBOSE(0, "Clearing errors on all slaves\n");

            (VOID)SIII_SVCClearErrors
                (
                  prS3Instance,
                  SIII_ALL_DEVICES
                );
          }

          // Call application-specific connection configuration via function pointer

          if (prS3Instance->fpAppConnConfigGlob != NULL)
          {
            prS3Instance->fpAppConnConfigGlob(prS3Instance);
          }

          usMasterConnOffset = (USHORT) 0;

          for (
              iCnt = 0;
              iCnt < SIII_MAX_SLAVES;
              iCnt++
            )
          {
            if (prS3Instance->afpAppConnConfig[iCnt] != NULL)
            {
              prS3Instance->afpAppConnConfig[iCnt]
                  (
                    prS3Instance,        // SIII instance
                    (USHORT) iCnt,       // Device index
                    &usMasterConnOffset, // Master connection number offset
                    (USHORT) iCnt        // RT data connection offset
                  );
            }
          }

          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;
          break;

        case SIII_CSMD_STATE_GET_TIMING_DATA:

          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;
          break;

        case SIII_CSMD_STATE_CALCULATE_TIMING:

          // Configure communication parameters
          prCosemaInstance->rConfiguration.rComTiming.ulCommCycleTime_S01002 =
              prS3Instance->rS3Pars.ulCycleTime;
          prCosemaInstance->rConfiguration.rMasterCfg.ulJitter_Master =
              prS3Instance->rS3Pars.ulSoftMasterJitterNs;
          prCosemaInstance->rConfiguration.rComTiming.ulMinTimeStartAT =
              (ULONG) SIII_MIN_TIME_START_AT;

          eCosemaFuncRet = CSMD_CalculateTiming
              (
                prCosemaInstance,                         // CoSeMa instance
                prS3Instance->rS3Pars.usS3TimingMethod    // Sercos timing method
              );

          SIII_VERBOSE
              (
                1,
                "Activating Sercos timing mode #%d...\n",
                //prS3Instance->rS3Pars.usS3TimingMethod
                prS3Instance->rCosemaInstance.rPriv.usTimingMethod
              );

          SIII_VERBOSE
              (
                0,
                "CSMD_CalculateTiming - state = %i, "
                "return value = 0x%x\n",
                prCosemaFuncState->usActState,
                eCosemaFuncRet
              );

          if (eCosemaFuncRet > CSMD_END_ERR_CLASS_00000)
          {
            // Stop automatic startup when error occurs
            eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
            prPhaseStateStruct->ucCsmdStateNew  =
                (UCHAR) (SIII_CSMD_STATE_CALCULATE_TIMING - 1);
            prPhaseStateStruct->ucCsmdStateCurr =
                (UCHAR) (SIII_CSMD_STATE_CALCULATE_TIMING - 1);
          }

          prPhaseStateStruct->ucPhaseState =
              (UCHAR) SIII_PHASE_STATE_IDLE;
          break;

        case SIII_CSMD_STATE_TRANSMIT_TIMING:

          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;

          for(
              iCnt = 0;
              iCnt < prCosemaInstance->rSlaveList.usNumProjSlaves;
              iCnt++
            )
          {
            prCosemaInstance->rConfiguration.parSlaveConfig[iCnt].rTiming.usMaxNbrTelErr_S1003 =
                prS3Instance->rS3Pars.usAccTelLosses;
          }
          break;

        case SIII_CSMD_STATE_SET_PHASE3:

          (VOID)SIII_InitConnections(prS3Instance);

          (VOID) memset
              (
                prS3Instance->aucCyclicMDTBuffer,
                (UCHAR) 0x00,
                SIII_CYCLIC_BUFFER_SIZE
              );

          (VOID) memset
              (
                prS3Instance->aucCyclicATBuffer,
                (UCHAR) 0x00,
                SIII_CYCLIC_BUFFER_SIZE
              );

          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;

          SIII_VERBOSE
              (
                0,
                "Setting Sercos cycle time to %gms\n",
                SIII_ULONG_NS_TO_DOUBLE_MS(prS3Instance->rS3Pars.ulCycleTime)
              );
#ifdef SIII_INC_PRIO_SETPHASE3
          (VOID)RTOS_SetThreadPriority(RTOS_THREAD_PRIORITY_SIII_SETPHASE3);
#endif

          break;

        case SIII_STATE_FILL_CONN_INFO:

          eErrorCode = SIII_BuildConnInfoList
              (
                prS3Instance
              );

          /* eErrorCode is not checked yet, always returns SIII_NO_ERROR */
          SIII_VERBOSE
              (
                0,
                "SIII_BuildConnInfoList, "
                "return value = 0x%x\n",
                eErrorCode
              );

          prPhaseStateStruct->ucPhaseState =
              (UCHAR) SIII_PHASE_STATE_IDLE;
          break;

        case SIII_CSMD_STATE_SET_PHASE4:

          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;
          break;
#ifdef CSMD_HOTPLUG
        case SIII_CSMD_STATE_HOTPLUG:

          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;
          SIII_VERBOSE(0, "Starting hotplug procedure ...\n");
          prS3Instance->rCyclicCommCtrl.eCyclicCsmdError = CSMD_NO_ERROR;
          prS3Instance->rCyclicCommCtrl.boHotplugCyclicPhase = TRUE;
          break;

        case SIII_CSMD_STATE_TRANS_HP2_PARA:

          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;
          break;

#endif
        case SIII_CSMD_STATE_RING_RECOVERY:
          prCosemaFuncState->ulSleepTime = (ULONG) 0;
          prCosemaFuncState->usActState = (USHORT) CSMD_FUNCTION_1ST_ENTRY;
          prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
          prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_RUNNING;
          break;

        default:
          break;

      }
      break;
    case SIII_PHASE_STATE_RUNNING:
      // CoSeMa API functions with internal state machine
      switch(prPhaseStateStruct->ucCsmdStateCurr)
      {
        case SIII_CSMD_STATE_INITIALIZE:
          break;

        case SIII_CSMD_STATE_INIT_HARDWARE:

          prPhaseStateStruct->ulCsmdSleepCnt +=
              prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

          if (prPhaseStateStruct->ulCsmdSleepCnt >= prCosemaFuncState->ulSleepTime)
          {

            prCosemaHWInitStruct->ulCycleTime_CP0 =
                prS3Instance->rS3Pars.ulCycleTimeCP0CP2;
            prCosemaHWInitStruct->ulCycleTime_CP12 =
                prS3Instance->rS3Pars.ulCycleTimeCP0CP2;
            prCosemaHWInitStruct->ulUCC_Width =
                prS3Instance->rS3Pars.ulUCCBandwidth;

            eCosemaFuncRet = CSMD_InitHardware
                (
                  prCosemaInstance,     // CoSeMa instance
                  prCosemaFuncState     // CoSeMa state machine state
                );

            prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;

            if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
            {
              SIII_VERBOSE
                  (
                    0,
                    "CSMD_InitHardware - state = %i, "
                    "return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );

              if (eCosemaFuncRet != CSMD_NO_ERROR)
              {
                // Stop automatic startup when error occurs
                eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
                prPhaseStateStruct->ucCsmdStateNew  =
                    (UCHAR) (SIII_CSMD_STATE_INIT_HARDWARE - 1);
                prPhaseStateStruct->ucCsmdStateCurr =
                    (UCHAR) (SIII_CSMD_STATE_INIT_HARDWARE - 1);
              }

              prPhaseStateStruct->ucPhaseState =
                  (UCHAR) SIII_PHASE_STATE_IDLE;
            }
            else
            {
              SIII_VERBOSE
                  (
                    1,
                    "CSMD_InitHardware - state = %i, "
                    "return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );
            }
          }
          break;

        case SIII_CSMD_STATE_SET_COMM_PARAM:
          break;

        case SIII_CSMD_STATE_SET_NRT_MODE:
          break;

        case SIII_CSMD_STATE_SET_PHASE0:

          prPhaseStateStruct->ulCsmdSleepCnt +=
              prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

          // Initialize telegram buffer
          prCosemaInstance->prTelBuffer = &prS3Instance->rCosemaTelBuffer;

          if (prPhaseStateStruct->ulCsmdSleepCnt >= prCosemaFuncState->ulSleepTime)
          {
            eCosemaFuncRet = CSMD_SetPhase0
                (
                  prCosemaInstance,                     // CoSeMa instance
                  prCosemaFuncState,                    // CoSeMa state machine state
                  &prS3Instance->pusCosemaRecDevList,   // List of recognized slaves
                  prS3Instance->rS3Pars.eComVersion     // Requested COM version
                );
            prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;

#ifndef SICE_MEASURE_TIMING
            if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
#else
            if (
                (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS) ||
                (prCosemaFuncState->usActState == 6)
              )
#endif
            {
              // Switch back flags for cyclic CoSeMa calls
              prS3Instance->rCyclicCommCtrl.boAppDataValid = FALSE;
              prS3Instance->rCyclicCommCtrl.boCallWriteMDT = TRUE;

              prPhaseStateStruct->boSwitchBackCP = FALSE;

              SIII_VERBOSE
                  (
                    0,
                    "CSMD_SetPhase0 - state = %i, "
                    "return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );

#ifdef SICE_MEASURE_TIMING
              SIII_VERBOSE(0, "***** Stopping phase transition for jitter measurements *****\n");
#endif

#ifdef SICE_OPT_UCC_CP1_2_IGNORE_ACK
              if (
                  (eCosemaFuncRet != CSMD_NO_ERROR)           &&
                  (eCosemaFuncRet != CSMD_CP0_COM_VER_CHECK)
                )
                        // Ignore potentially missing ack of slaves to
                        // shifting UC position in CP1 and CP2
#else
              if (eCosemaFuncRet != CSMD_NO_ERROR)
#endif
              {
                if (prPhaseStateStruct->ucRetries > (UCHAR) 0)
                {
                  SIII_VERBOSE
                      (
                        0,
                        "Error in Sercos startup, %d retries left...\n",
                        prPhaseStateStruct->ucRetries
                      );
                  prPhaseStateStruct->ucRetries--;
                  prPhaseStateStruct->ucCsmdStateCurr =
                      ((UCHAR) SIII_CSMD_STATE_SET_PHASE0) - ((UCHAR) 1);
                }
                else
                {
                  // Stop automatic startup when error occurs
                  eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
                  prPhaseStateStruct->ucCsmdStateNew =
                      (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
                  prPhaseStateStruct->ucCsmdStateCurr =
                      (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
                }
              }
              else
              {
#ifdef SICE_OPT_UCC_CP1_2_IGNORE_ACK
                if (eCosemaFuncRet == CSMD_CP0_COM_VER_CHECK)
                {
                  SIII_VERBOSE(0, "CSMD_CP0_COM_VER_CHECK "
                      "(0x3C) ignored.\n")
                }
#endif

                // Switch back flags for cyclic CoSeMa calls
                prS3Instance->rCyclicCommCtrl.boCallTxRxSoftCont = FALSE;
                prS3Instance->rCyclicCommCtrl.boAppDataValid     = FALSE;

                // If any slaves found
                if (prS3Instance->pusCosemaRecDevList != NULL)
                {
                  // Get number of found slaves
                  prS3Instance->usDevCnt =
                      prS3Instance->pusCosemaRecDevList[0] / sizeof(USHORT);
                                                          // Byte length to word length

                  // Print addresses of found devices
                  SIII_VERBOSE
                      (
                        0,
                        "Found %i slave(s):\n",
                        prS3Instance->usDevCnt
                      );
                  for (
                      iCnt = 0;
                      iCnt < prS3Instance->usDevCnt;
                      iCnt++
                    )
                  {
                    SIII_VERBOSE
                        (
                          0,
                          "- Device #%i (Sercos address: %i)",
                          iCnt,
                          *(prS3Instance->pusCosemaRecDevList+2+iCnt)
                        );

                    if (iCnt != prS3Instance->usDevCnt-1)
                    {
                      SIII_VERBOSE(0, ",\n");
                    }
                    else
                    {
                      SIII_VERBOSE(0, "\n");
                    }
                  }
                }
              }
              prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;
            }
            else  // Function still in processing
            {
              SIII_VERBOSE
                  (
                    1,
                    "CSMD_SetPhase0 - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );
            }
          }

          break;

        case SIII_CSMD_STATE_INIT_CONFIG_STRUCT:
          break;

        case SIII_CSMD_STATE_SET_PHASE1:

          prPhaseStateStruct->ulCsmdSleepCnt +=
              prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

          if (
              prPhaseStateStruct->ulCsmdSleepCnt >=
                  prCosemaFuncState->ulSleepTime
            )
          {
            if (prS3Instance->rS3Pars.boDetectSlaveConfig)
            {

              // Use discovered slaves for configuration
              eCosemaFuncRet  = CSMD_SetPhase1
                  (
                    prCosemaInstance,                   // CoSeMa instance
                    prCosemaFuncState,                  // CoSeMa state machine state
                    prS3Instance->pusCosemaRecDevList   // Slave list
                  );
            }
            else
            {
              // Use manually configured slave for configuration

              if (prS3Instance->boSetSlaveConfig == FALSE)
              {
                SIII_VERBOSE(0, "Error: No Sercos slave configuration.\n")
                prPhaseStateStruct->ucCsmdStateNew  =
                    (UCHAR) (SIII_CSMD_STATE_SET_PHASE1 - 1);
                prPhaseStateStruct->ucCsmdStateCurr =
                    (UCHAR) (SIII_CSMD_STATE_SET_PHASE1 - 1);
                return(SIII_CONFIG_ERROR);
              }

              prS3Instance->pusCosemaRecDevList = prS3Instance->ausOperDevList;

              eCosemaFuncRet  = CSMD_SetPhase1
                  (
                    prCosemaInstance,             // CoSeMa instance
                    prCosemaFuncState,            // CoSeMa state machine state
                    prS3Instance->ausOperDevList  // Slave list
                  );
            }

            prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
            if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
            {
              if (prS3Instance->rS3Pars.boDetectSlaveConfig)
              {
                SIII_VERBOSE(0, "Using detected slave configuration.\n");
              }
              else
              {
                SIII_VERBOSE(0, "Using pre-configured slave configuration:\n");
                for (
                    iCnt = 0;
                    iCnt < (prS3Instance->ausOperDevList[0]/2);
                    iCnt ++
                  )
                {
                  SIII_VERBOSE
                      (
                        0,
                        "- Device #%i (Sercos address: %i)",
                        iCnt,
                        (INT) prS3Instance->ausOperDevList[iCnt+2]
                      );
                  if (iCnt != (prS3Instance->ausOperDevList[0]/2)-1)
                  {
                    SIII_VERBOSE(0, ",\n");
                  }
                  else
                  {
                    SIII_VERBOSE(0, "\n");
                  }
                }
              }

              SIII_VERBOSE
                  (
                    0,
                    "CSMD_SetPhase1 - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );

              prPhaseStateStruct->ucPhaseState =
                  (UCHAR) SIII_PHASE_STATE_IDLE;

              if (eCosemaFuncRet != CSMD_NO_ERROR)
              {
                if (prPhaseStateStruct->ucRetries > (UCHAR) 0)
                {
                  SIII_VERBOSE
                      (
                        0,
                        "Error in Sercos startup, %d retries left...\n",
                        prPhaseStateStruct->ucRetries
                      );
                  prPhaseStateStruct->ucRetries--;
                  prPhaseStateStruct->ucCsmdStateCurr =
                      ((UCHAR) SIII_CSMD_STATE_SET_PHASE0) - ((UCHAR) 1);
                }
                else
                {
                  // Stop automatic startup when error occurs
                  eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
                  prPhaseStateStruct->ucCsmdStateNew  =
                      (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
                  prPhaseStateStruct->ucCsmdStateCurr =
                      (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
                }
              }
              else
              {
                // Start calling CSMD_WriteMDT() every cycle
                prS3Instance->rCyclicCommCtrl.boCallWriteMDT = TRUE;
              }
            }
            else
            {
              SIII_VERBOSE
                  (
                    1,
                    "CSMD_SetPhase1 - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );
            }
          }

          break;

        case SIII_CSMD_STATE_SET_PHASE2:

          prPhaseStateStruct->ulCsmdSleepCnt +=
              prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

          if  (
              prPhaseStateStruct->ulCsmdSleepCnt >= prCosemaFuncState->ulSleepTime
            )
          {
            eCosemaFuncRet  = CSMD_SetPhase2
                (
                  prCosemaInstance,   // CoSeMa instance
                  prCosemaFuncState   // CoSeMa state machine state
                );
            prPhaseStateStruct->ulCsmdSleepCnt  = (ULONG) 0;

            if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
            {
              SIII_VERBOSE
                  (
                    0,
                    "CSMD_SetPhase2 - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );

              prPhaseStateStruct->ucPhaseState =
                  (UCHAR) SIII_PHASE_STATE_IDLE;

              if (eCosemaFuncRet != CSMD_NO_ERROR)
              {
                if (prPhaseStateStruct->ucRetries > (UCHAR) 0)
                {
                  SIII_VERBOSE
                      (
                        0,
                        "Error in Sercos startup, %d retries left...\n",
                        prPhaseStateStruct->ucRetries
                      );
                  prPhaseStateStruct->ucRetries--;
                  prPhaseStateStruct->ucCsmdStateCurr =
                      (UCHAR) (SIII_CSMD_STATE_SET_PHASE0 - 1);
                }
                else
                {
                  // Stop automatic startup when error occurs
                  eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
                  prPhaseStateStruct->ucCsmdStateNew  = (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
                  prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
                }
              }
              else
              {
                // Start calling CSMD_TxRxSoftCont every cycle
                prS3Instance->rCyclicCommCtrl.boCallTxRxSoftCont = TRUE;
              }
            }
            else
            {
              SIII_VERBOSE
                  (
                    1,
                    "CSMD_SetPhase2 - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );
            }
          }
          break;

        case SIII_CSMD_STATE_CHECK_VERSION:

          prPhaseStateStruct->ulCsmdSleepCnt +=
              prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

          if (prPhaseStateStruct->ulCsmdSleepCnt >= prCosemaFuncState->ulSleepTime)
          {
            eCosemaFuncRet  = CSMD_CheckVersion
                (
                  prCosemaInstance,                   // CoSeMa instance
                  prCosemaFuncState,                  // CoSeMa state machine state
                  &prS3Instance->arCosemaSvcMacro[0]  // SVC macro structure
                );
            prPhaseStateStruct->ulCsmdSleepCnt  = (ULONG) 0;

            if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
            {
              SIII_VERBOSE
                  (
                    0,
                    "CSMD_CheckVersion - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );

              if (eCosemaFuncRet != CSMD_NO_ERROR)
              {
                if (prPhaseStateStruct->ucRetries > (UCHAR) 0)
                {
                  SIII_VERBOSE
                      (
                        0,
                        "Error in Sercos startup, %d retries left...\n",
                        prPhaseStateStruct->ucRetries
                      );
                  prPhaseStateStruct->ucRetries--;
                  prPhaseStateStruct->ucCsmdStateCurr =
                      (UCHAR) (SIII_CSMD_STATE_SET_PHASE0 - 1);
                }
                else
                {
                  // Stop automatic startup when error occurs
                  eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
                  prPhaseStateStruct->ucCsmdStateNew =
                      (UCHAR) (SIII_CSMD_STATE_CHECK_VERSION - 1);
                  prPhaseStateStruct->ucCsmdStateCurr =
                      (UCHAR) (SIII_CSMD_STATE_CHECK_VERSION - 1);
                }
              }

              prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;
            }
            else
            {
              SIII_VERBOSE
                  (
                    1,
                    "CSMD_CheckVersion - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );
            }

          }
          break;

        case SIII_CSMD_STATE_GET_TIMING_DATA:

          prPhaseStateStruct->ulCsmdSleepCnt +=
              prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

          if  (
              prPhaseStateStruct->ulCsmdSleepCnt >= prCosemaFuncState->ulSleepTime
            )
          {
            eCosemaFuncRet = CSMD_GetTimingData
                (
                  prCosemaInstance,                   // CoSeMa instance
                  prCosemaFuncState,                  // CoSeMa state machine state
                  &prS3Instance->arCosemaSvcMacro[0]  // SVC macro structure
                );
            prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
            if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
            {
              SIII_VERBOSE
                  (
                    0,
                    "CSMD_GetTimingData - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );

              if (eCosemaFuncRet != CSMD_NO_ERROR)
              {
                if (prPhaseStateStruct->ucRetries > (UCHAR) 0)
                {
                  SIII_VERBOSE
                      (
                        0,
                        "Error in Sercos startup, %d retries left...\n",
                        prPhaseStateStruct->ucRetries
                      );
                  prPhaseStateStruct->ucRetries--;
                  prPhaseStateStruct->ucCsmdStateCurr =
                      (UCHAR) (SIII_CSMD_STATE_SET_PHASE0 - 1);
                }
                else
                {
                  // Stop automatic startup when error occurs
                  eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
                  prPhaseStateStruct->ucCsmdStateNew  =
                      (UCHAR) (SIII_CSMD_STATE_GET_TIMING_DATA - 1);
                  prPhaseStateStruct->ucCsmdStateCurr =
                      (UCHAR) (SIII_CSMD_STATE_GET_TIMING_DATA - 1);
                }
              }
              prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;
            }
            else
            {
              SIII_VERBOSE
                  (
                    1,
                    "CSMD_GetTimingData - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );
            }

          }
          break;

        case SIII_CSMD_STATE_CALCULATE_TIMING:
          break;

        case SIII_CSMD_STATE_TRANSMIT_TIMING:

          prPhaseStateStruct->ulCsmdSleepCnt +=
              prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

          if (prPhaseStateStruct->ulCsmdSleepCnt >= prCosemaFuncState->ulSleepTime)
          {
            eCosemaFuncRet = CSMD_TransmitTiming
                (
                  prCosemaInstance,                   // CoSeMa instance
                  prCosemaFuncState,                  // CoSeMa state machine state
                  &prS3Instance->arCosemaSvcMacro[0]  // SVC macro structure
                );
            prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
            if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
            {
              SIII_VERBOSE
                  (
                    0,
                    "CSMD_TransmitTiming - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );

              if (eCosemaFuncRet != CSMD_NO_ERROR)
              {
                if (prPhaseStateStruct->ucRetries > (UCHAR) 0)
                {
                  SIII_VERBOSE
                      (
                        0,
                        "Error in Sercos startup, %d retries left...\n",
                        prPhaseStateStruct->ucRetries
                      );
                  prPhaseStateStruct->ucRetries--;
                  prPhaseStateStruct->ucCsmdStateCurr =
                      (UCHAR) (SIII_CSMD_STATE_SET_PHASE0 - 1);
                }
                else
                {
                  // Stop automatic startup when error occurs
                  eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
                  prPhaseStateStruct->ucCsmdStateNew  =
                      (UCHAR) (SIII_CSMD_STATE_TRANSMIT_TIMING - 1);
                  prPhaseStateStruct->ucCsmdStateCurr =
                      (UCHAR) (SIII_CSMD_STATE_TRANSMIT_TIMING - 1);
                }
              }
              prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;
            }
            else
            {
              SIII_VERBOSE
                  (
                    1,
                    "CSMD_TransmitTiming - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );
            }
          }
          break;

        case SIII_CSMD_STATE_SET_PHASE3:

          prPhaseStateStruct->ulCsmdSleepCnt +=
              prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

          if (
              prPhaseStateStruct->ulCsmdSleepCnt >= prCosemaFuncState->ulSleepTime
            )
          {
            eCosemaFuncRet = CSMD_SetPhase3
                (
                  prCosemaInstance,                   // CoSeMa instance
                  prCosemaFuncState,                  // CoSeMa state machine state
                  &prS3Instance->arCosemaSvcMacro[0]  // SVC macro structure
                );

            prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;

            if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
            {

#ifdef SIII_INC_PRIO_SETPHASE3
          (VOID)RTOS_SetThreadPriority(RTOS_THREAD_PRIORITY_USER);
#endif

              SIII_VERBOSE
                  (
                    0,
                    "CSMD_SetPhase3 - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );

              if (eCosemaFuncRet != CSMD_NO_ERROR)
              {
                if (prPhaseStateStruct->ucRetries > (UCHAR) 0)
                {
                  SIII_VERBOSE
                      (
                        0,
                        "Error in Sercos startup, %d retries left...\n",
                        prPhaseStateStruct->ucRetries
                      );
                  prPhaseStateStruct->ucRetries--;
                  prPhaseStateStruct->ucCsmdStateCurr =
                      (UCHAR) (SIII_CSMD_STATE_SET_PHASE0 - 1);
                }
                else
                {
                  // Stop automatic startup when error occurs
                  eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
                  prPhaseStateStruct->ucCsmdStateNew  = (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
                  prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
                }
              }
              prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;
            }
            else
            {
              SIII_VERBOSE
                  (
                    1,
                    "CSMD_SetPhase3 - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );
            }
          }
          break;

        case SIII_STATE_FILL_CONN_INFO:
          break;

        case SIII_CSMD_STATE_SET_PHASE4:

          prPhaseStateStruct->ulCsmdSleepCnt +=
              prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

          if (prPhaseStateStruct->ulCsmdSleepCnt >= prCosemaFuncState->ulSleepTime)
          {
            eCosemaFuncRet = CSMD_SetPhase4
                (
                  prCosemaInstance,                   // CoSeMa instance
                  prCosemaFuncState,                  // CoSeMa state machine state
                  &prS3Instance->arCosemaSvcMacro[0]  // SVC macro structure
                );
            prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
            if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
            {
              SIII_VERBOSE
                  (
                    0,
                    "CSMD_SetPhase4 - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );

              prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;
              prS3Instance->rCyclicCommCtrl.boAppDataValid = TRUE;

              if (eCosemaFuncRet != CSMD_NO_ERROR)
              {
                if (prPhaseStateStruct->ucRetries > (UCHAR) 0)
                {
                  SIII_VERBOSE
                      (
                        0,
                        "Error in Sercos startup, %d retries left...\n",
                        prPhaseStateStruct->ucRetries
                      );
                  prPhaseStateStruct->ucRetries--;
                  prPhaseStateStruct->ucCsmdStateCurr =
                      ((UCHAR) SIII_CSMD_STATE_SET_PHASE0) - ((UCHAR) 1);
                }
                else
                {
                  eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
                  prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
                  prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
                }
              }
            }
            else
            {
              SIII_VERBOSE
                  (
                    1,
                    "CSMD_SetPhase4 - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );
            }
          }
          break;
#ifdef CSMD_HOTPLUG
        case SIII_CSMD_STATE_HOTPLUG:

          if  (
              (prS3Instance->rCyclicCommCtrl.boHotplugCyclicPhase == FALSE)   &&
              (prS3Instance->rCyclicCommCtrl.eCyclicCsmdError == CSMD_FUNCTION_IN_PROCESS)
            )
          {
            prPhaseStateStruct->ulCsmdSleepCnt +=
                prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

            if (prPhaseStateStruct->ulCsmdSleepCnt >= prCosemaFuncState->ulSleepTime)
            {
              eCosemaFuncRet = CSMD_HotPlug
                  (
                    prCosemaInstance,                     // CoSeMa state machine state
                    prCosemaFuncState,                    // CoSeMa state machine state
                    prS3Instance->ausCosemaHPDevAddList,  // Hotplug slave list
                    FALSE                                 // Start, not cancel
                  );
              prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
              if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
              {
                SIII_VERBOSE
                    (
                      0,
                      "CSMD_Hotplug - state = %i, return value = 0x%x\n",
                      prCosemaFuncState->usActState,
                      eCosemaFuncRet
                    );

                prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;
                // prS3Instance->rCyclicCommCtrl.boAppDataValid =
                //    TRUE;
                if (eCosemaFuncRet != CSMD_NO_ERROR)
                {
                  // Fall back to CP4
                  prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;
                  prS3Instance->rCyclicCommCtrl.boAppDataValid = TRUE;
                  prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_PHASE4;
                  prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_PHASE4;
                  eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
                }
              }
              else
              {
                SIII_VERBOSE
                    (
                      1,
                      "CSMD_Hotplug - state = %i, return value = 0x%x\n",
                      prCosemaFuncState->usActState,
                      eCosemaFuncRet
                    );
              }
            }
          }
          else if
              (
                (prS3Instance->rCyclicCommCtrl.eCyclicCsmdError != CSMD_FUNCTION_IN_PROCESS)
                &&
                (prS3Instance->rCyclicCommCtrl.eCyclicCsmdError != CSMD_NO_ERROR)  // Hotplug error
              )
          {
            SIII_VERBOSE
                (
                  1,
                  "CSMD_Hotplug - state = %i, return value = 0x%x\n",
                  prCosemaFuncState->usActState,
                  prS3Instance->rCyclicCommCtrl.eCyclicCsmdError
                );
          }
          break;

        case SIII_CSMD_STATE_TRANS_HP2_PARA:

          prPhaseStateStruct->ulCsmdSleepCnt +=
              prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

          if (prPhaseStateStruct->ulCsmdSleepCnt >= prCosemaFuncState->ulSleepTime)
          {
            eCosemaFuncRet = CSMD_TransHP2Para
                (
                  prCosemaInstance,                   // CoSeMa instance
                  prCosemaFuncState,                  // CoSeMa state machine state
                  &prS3Instance->arCosemaSvcMacro[0], // SVC macro structure
                  FALSE                               // Start, not cancel
                );
            prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
            if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
            {
              SIII_VERBOSE
                  (
                    0,
                    "CSMD_TransHP2Para - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );

              if (prCosemaInstance->rExtendedDiag.usNbrSlaves != 0)
              {
                SIII_VERBOSE
                    (
                      0,
                      "Error in CSMD_TransHP2Para.\nrExtendedDiag:\n"
                      "No of slave errors: %d;\n"
                      "ulIDN: 0x%lX;\n"
                      "Error Slave 0: %X;\n"
                      "Error Slave 1: %X;\n"
                      "SVC Error Slave 0: %hX;\n"
                      "SVC Error Slave 1: %hX;\n",
                      prCosemaInstance->rExtendedDiag.usNbrSlaves,
                      prCosemaInstance->rExtendedDiag.ulIDN,
                      prCosemaInstance->rExtendedDiag.aeSlaveError[0],
                      prCosemaInstance->rExtendedDiag.aeSlaveError[1],
                      prCosemaInstance->parSvchMngmtData[0].usSvchError,
                      prCosemaInstance->parSvchMngmtData[1].usSvchError
                    );
              }

              prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;
              prS3Instance->rCyclicCommCtrl.boAppDataValid = TRUE;
              prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_PHASE4;
              prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_PHASE4;
              if (eCosemaFuncRet != CSMD_NO_ERROR)
              {
                eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
              }
            }
            else
            {
              SIII_VERBOSE
                  (
                    1,
                    "CSMD_TransHP2Para - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );
            }
          }
          break;

#endif
        case SIII_CSMD_STATE_RING_RECOVERY:

          prPhaseStateStruct->ulCsmdSleepCnt +=
              prPhaseStateStruct->ulPhaseHandlerWaitUs / (ULONG) 1000;

          if (prPhaseStateStruct->ulCsmdSleepCnt >= prCosemaFuncState->ulSleepTime)
          {
            eCosemaFuncRet = CSMD_RecoverRingTopology
                (
                  prCosemaInstance,                   // CoSeMa instance
                  prCosemaFuncState,                  // CoSeMa state machine state
                  &prS3Instance->arCosemaSvcMacro[0]  // SVC macro structure
                );
            prPhaseStateStruct->ulCsmdSleepCnt = (ULONG) 0;
            if (eCosemaFuncRet != CSMD_FUNCTION_IN_PROCESS)
            {
              SIII_VERBOSE
                  (
                    0,
                    "CSMD_RecoverRingTopology - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );

              if (eCosemaFuncRet != CSMD_RECOVER_RING_OK)
              {
                if (prPhaseStateStruct->ucRetries > (UCHAR) 0)
                {
                  SIII_VERBOSE
                      (
                        0,
                        "Error in Sercos ring topology recovery, %d retries left...\n",
                        prPhaseStateStruct->ucRetries
                      );
                  prPhaseStateStruct->ucRetries--;
                }
                else
                {
                  // Stop automatic startup when error occurs
                  eErrorCode = (SIII_FUNC_RET) eCosemaFuncRet;
                }
              }
              prPhaseStateStruct->ucPhaseState = (UCHAR) SIII_PHASE_STATE_IDLE;

              switch (SIII_GetSercosPhase(prS3Instance))
              {
                case SIII_PHASE_CP1:
                  prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_PHASE1;
                  prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_PHASE1;
                  break;
                case SIII_PHASE_CP2:
                  prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_PHASE2;
                  prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_PHASE2;
                  break;
                case SIII_PHASE_CP3:
                  prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_PHASE3;
                  prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_PHASE3;
                  break;
                case SIII_PHASE_CP4:
                  prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_PHASE4;
                  prPhaseStateStruct->ucCsmdStateCurr = (UCHAR) SIII_CSMD_STATE_SET_PHASE4;
                  break;
                default:
                  // Here should something happen !!!
                  break;
              }
            }
            else
            {
              SIII_VERBOSE
                  (
                    1,
                    "CSMD_RecoverRingTopology - state = %i, return value = 0x%x\n",
                    prCosemaFuncState->usActState,
                    eCosemaFuncRet
                  );
            }
          }
          break;

        default:
          SIII_VERBOSE(0, "Unknown Sercos startup state.\n");
          eErrorCode = SIII_STATE_ERROR;
          break;

      } // end: switch (prPhaseStateStruct->ucCsmdStateCurr)
      break;

    default:
      SIII_VERBOSE(0, "Unknown phase handler state.\n");
      eErrorCode = SIII_STATE_ERROR;
      break;

  } // end: switch (prPhaseStateStruct->ucPhaseState)

  if (
      (prPhaseStateStruct->ucPhaseState != (UCHAR) SIII_PHASE_STATE_IDLE)  &&
      (eErrorCode == SIII_NO_ERROR)
    )
  {
    return(SIII_FUNCTION_IN_PROCESS);
  }
  else
  {
    return(eErrorCode);
  }
}

/**
 * \fn SIII_FUNC_RET SIII_PhaseSwitch(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              SIII_PHASE eTargetPhase,
 *              INT iRetries,
 *              INT iTimeOutSec
 *          )
 *
 * \public
 *
 * \brief   High-level blocking function to perform a Sercos phase switch.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       eTargetPhase    Target Sercos phase, see definition of
 *                                  SIII_PHASE
 * \param[in]       iRetries        Number of retries in case of error
 * \param[in]       iTimeOutSec     Timeout in seconds. The time should be long
 *                                  enough to complete the selected number of
 *                                  retries (Typ. at least 15s per retry).
 *
 * \details The function should be used instead of running the Sercos phase
 *          handler SIII_PhaseHandler() in a persistent thread. In case the
 *          phase handler is already active, a SIII_BLOCKING_ERROR is returned.
 *          When a phase switch is attempted that is illegal according to the
 *          Sercos III specification, SIII_SERCOS_PHASE_ERROR is returned.
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR             for success
 *          - SIII_BLOCKING_ERROR       when phase handler is already in use
 *          - SIII_PARAMETER_ERROR      for illegal function parameter
 *          - SIII_SERCOS_PHASE_ERROR   when an illegal phase switch is
 *                                      attempted
 *          - SIII_TIMEOUT_ERROR        the timeout has occurred before the
 *                                      target phase was reached
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-03-08
 *
 * \version 2013-06-06 (GMy): Added phase switching option
 *                            SIII_PHASE_CP2_AFTER_PARS_XMIT
 * \version 2013-07-03 (GMy): Change to ensure that CSMD calling interval is
 *                            never smaller than Sercos cycle time
 */
SIII_FUNC_RET SIII_PhaseSwitch
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      SIII_PHASE eTargetPhase,
      INT iRetries,
      INT iTimeOutSec
    )
{
  SIII_FUNC_RET           eRet                = SIII_NO_ERROR;
  LONG                    lTimeOutMilliSec    = iTimeOutSec * 1000;
  SIII_PHASE_STATE_STRUCT *prPhaseStateStruct;

  SIII_VERBOSE(3, "SIII_PhaseSwitch()\n");

  // Check function parameters
  if (
      (prS3Instance   ==  NULL)   ||
      (iRetries       <   0)      ||
      (iTimeOutSec    <   0)
    )
  {
    return(SIII_PARAMETER_ERROR);
  }

  // Get shortcut pointers
  prPhaseStateStruct = &prS3Instance->rPhaseStateStruct;

  if (prPhaseStateStruct->ucPhaseState == (UCHAR) SIII_PHASE_STATE_IDLE)
  {
    prPhaseStateStruct->ucRetries = (UCHAR) iRetries;

    switch (eTargetPhase)
    {
      case SIII_PHASE_NRT:
        if  (
            (prPhaseStateStruct->ucCsmdStateCurr == (UCHAR) SIII_CSMD_STATE_SET_PHASE0)   ||
            (prPhaseStateStruct->ucCsmdStateCurr < (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE)
          )
        {
          SIII_VERBOSE(0, "Switching to NRT.\n");
          prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_NRT_MODE;
        }
        else
        {
          SIII_VERBOSE
              (
                0,
                "Switching back to NRT is only allowed when in CP0.\n"
              );
          return(SIII_SERCOS_PHASE_ERROR);
        }
        break;
      case SIII_PHASE_CP0:
        SIII_VERBOSE(0, "Switching to CP0.\n");
        prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_PHASE0;
        break;
      case SIII_PHASE_CP1:
        if (prPhaseStateStruct->ucCsmdStateCurr < (UCHAR) SIII_CSMD_STATE_SET_PHASE1)
        {
          SIII_VERBOSE(0, "Switching to CP1.\n");
          prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_PHASE1;
        }
        else
        {
          SIII_VERBOSE
              (
                0,
                "Switching back from higher phase only allowed to CP0.\n"
              );
          return(SIII_SERCOS_PHASE_ERROR);
        }
        break;
      case SIII_PHASE_CP2:
        if (prPhaseStateStruct->ucCsmdStateCurr < (UCHAR) SIII_CSMD_STATE_SET_PHASE2)
        {
          SIII_VERBOSE(0, "Switching to CP2.\n");
          prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_PHASE2;
        }
        else
        {
          SIII_VERBOSE
              (
                0,
                "Switching back from higher phase only allowed to CP0.\n"
              );
          return(SIII_SERCOS_PHASE_ERROR);
        }
        break;
      case SIII_PHASE_CP3:
        if (prPhaseStateStruct->ucCsmdStateCurr < (UCHAR) SIII_CSMD_STATE_SET_PHASE3)
        {
          SIII_VERBOSE(0, "Switching to CP3.\n");
          prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_PHASE3;
        }
        else
        {
          SIII_VERBOSE
              (
                0,
                "Switching back from higher phase only allowed to CP0.\n"
              );
          return(SIII_SERCOS_PHASE_ERROR);
        }
        break;
      case SIII_PHASE_CP4:
        if (prPhaseStateStruct->ucCsmdStateCurr < (UCHAR) SIII_CSMD_STATE_SET_PHASE4)
        {
          SIII_VERBOSE(0, "Switching to CP4.\n");
          prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_SET_PHASE4;
        }
        else
        {
          SIII_VERBOSE
              (
                0,
                "Switching back from higher phase only allowed to CP0.\n"
              );
          return(SIII_SERCOS_PHASE_ERROR);
        }
        break;
      case SIII_PHASE_CP2_AFTER_PARS_XMIT:
        if (prPhaseStateStruct->ucCsmdStateCurr < (UCHAR) SIII_CSMD_STATE_TRANSMIT_TIMING)
        {
          SIII_VERBOSE
              (
                0,
                "Switching to CP2 (after transmission of slave parameters).\n"
              );
          prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_TRANSMIT_TIMING;
        }
        else
        {
          SIII_VERBOSE
              (
                0,
                "Switching back from higher phase only allowed to CP0.\n"
              );
          return(SIII_SERCOS_PHASE_ERROR);
        }
        break;
      case SIII_PHASE_NEXT:
        if (prPhaseStateStruct->ucCsmdStateCurr < (UCHAR) SIII_CSMD_STATE_SET_PHASE4)
        {
          SIII_VERBOSE(0, "Switching to next CoSeMa phase.\n");
          prPhaseStateStruct->ucCsmdStateNew = prPhaseStateStruct->ucCsmdStateCurr + 1;
        }
        else
        {
          SIII_VERBOSE(0, "Already in phase 4.\n");
          return(SIII_SERCOS_PHASE_ERROR);
        }
        break;
      default:
        return(SIII_SERCOS_PHASE_ERROR);
        break;
    }
/*
    // Set own thread priority
    if (RTOS_SetThreadPriority(RTOS_THREAD_PRIORITY_USER) != RTOS_RET_OK)
    {
      SIII_VERBOSE(0, "Error: Failed to set priority of thread.\n");
      return(SIII_SYSTEM_ERROR);
    }
*/
    do
    {

      // Call Sercos phase handler
      eRet = SIII_PhaseHandler(prS3Instance);

      if (eRet > SIII_END_ERR_CLASS_00000)
      {
        SIII_VERBOSE
            (
              0,
              "Error in Sercos phase handler, code: 0x%x\n",
              (INT) eRet
            );
      }


      // Take care that waiting time of phase handler is at least Sercos
      // cycle time
      if (SIII_PHASE_HANDLER_WAIT_TIME <
          SIII_GetSercosCycleTime(prS3Instance, SIII_PHASE_CURR) / 1000)
      {
        prPhaseStateStruct->ulPhaseHandlerWaitUs =
            SIII_GetSercosCycleTime(prS3Instance, SIII_PHASE_CURR) / 1000;
      }
      else
      {
        prPhaseStateStruct->ulPhaseHandlerWaitUs =
            (ULONG)SIII_PHASE_HANDLER_WAIT_TIME;
      }
      RTOS_SimpleMicroWait(prPhaseStateStruct->ulPhaseHandlerWaitUs);
      lTimeOutMilliSec -= (LONG)(prPhaseStateStruct->ulPhaseHandlerWaitUs / 1000);
    }
    while (
        (lTimeOutMilliSec > 0)                                      &&    // Timeout
        (eRet < SIII_END_ERR_CLASS_00000)                           &&    // Error
        !(
          (prPhaseStateStruct->ucPhaseState == (UCHAR) SIII_PHASE_STATE_IDLE)   &&
          (eRet == SIII_NO_ERROR)                                 &&
          (
            (eTargetPhase == SIII_PHASE_NEXT)                   ||
            (
              (eTargetPhase == SIII_PHASE_CP2_AFTER_PARS_XMIT)                  &&
              (prPhaseStateStruct->ucCsmdStateCurr ==
                  SIII_CSMD_STATE_TRANSMIT_TIMING)
            )                                                   ||
            (SIII_GetSercosPhase(prS3Instance) == eTargetPhase)
          )                       // Phase switch successful
        )
      );

    if (lTimeOutMilliSec <= 0)
    {
      SIII_VERBOSE(0, "Timeout due to error during phase switching.\n");
      eRet = SIII_TIMEOUT_ERROR;
    }
    else if (eRet > SIII_END_ERR_CLASS_00000)
    {
      SIII_VERBOSE
          (
            0,
            "Error 0x%X has occurred during phase switch.\n",
            eRet
          );
    }
    else
    {
      SIII_VERBOSE(1, "Phase startup done.\n");
      eRet = SIII_NO_ERROR;
    }
  }
  else
  {
    // Phase handler currently active, no phase switch possible
    SIII_VERBOSE(0, "Phase handler currently active, no operation possible.\n");
    eRet = SIII_BLOCKING_ERROR;
  }

  return(eRet);
}

