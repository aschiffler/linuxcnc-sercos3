/**
 * \file      SIII_INIT.c
 *
 * \brief     Sercos III soft master stack - Initialization and de-initialization
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
 * \author    GMy
 *
 * \copyright Copyright Bosch Rexroth AG, 2012-2016
 *
 * \date      2012-10-11
 *
 * \version 2012-10-11 (GMy): Baseline for CoSeMa v4
 * \version 2012-10-31 (GMy): Updated for CoSeMa v5
 * \version 2013-01-21 (GMy): SVC support added
 * \version 2013-02-05 (GMy): Hot-plug support added
 * \version 2013-02-12 (GMy): Separation of phase handler and user interface
 * \version 2013-02-26 (GMy): Separation of initialization and user interface
 * \version 2013-02-28 (GMy): Lint code optimization
 * \version 2013-04-11 (GMy): Module re-arrangement
 * \version 2013-05-07 (GMy): Added support for INtime
 * \version 2013-06-06 (GMy): Added support for RTX and Kithara
 * \version 2013-06-20 (GMy): Optimization of error handling
 * \version 2013-06-21 (GMy): Added support for Windows desktop versions and
 *                            QNX
 * \version 2013-11-04 (GMy): Bugfix (Hot-Plug called cycle-exact)
 * \version 2015-03-20 (GMy): Moved auto-power-on to S3SM
 * \version 2015-11-02 (GMy): Added support for SICE init structure
 */

//---- includes ---------------------------------------------------------------

#define SOURCE_SIII

#include "../SIII/SIII_GLOB.h"
#include "../SIII/SIII_PRIV.h"

#ifdef __qnx__
#include "../RTQX/RTQX_GLOB.h"
#include "../RTQX/RTQX_S3SM_GLOB.h"
#elif defined __unix__
#include "../RTLX/RTLX_GLOB.h"
#include "../RTLX/RTLX_S3SM_GLOB.h"
#elif defined WINCE7
#include "../RTC7/RTC7_GLOB.h"
#include "../RTC7/RTC7_S3SM_GLOB.h"
#elif defined WINCE
#include "../RTC6/RTC6_GLOB.h"
#include "../RTC6/RTC6_S3SM_GLOB.h"
#elif defined __INTIME__
#include "../RTIT/RTIT_GLOB.h"
#include "../RTIT/RTIT_S3SM_GLOB.h"
#elif defined __RTX__
#include "../RTRX/RTRX_GLOB.h"
#include "../RTRX/RTRX_S3SM_GLOB.h"
#elif defined __VXWORKS__
#include "../RTVW/RTVW_GLOB.h"
#include "../RTVW/RTVW_S3SM_GLOB.h"
#elif defined __KITHARA__
#include "../RTKT/RTKT_GLOB.h"
#include "../RTKT/RTKT_S3SM_GLOB.h"
#elif defined WIN32
#include "../RTWI/RTWI_GLOB.h"
#include "../RTWI/RTWI_S3SM_GLOB.h"
#elif defined WIN64
#include "../RTWI/RTWI_GLOB.h"
#include "../RTWI/RTWI_S3SM_GLOB.h"
#else
#error Operating system not supported by SIII!
#endif

//---- defines ----------------------------------------------------------------

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SIII_FUNC_RET SIII_Init(
 *              SIII_INSTANCE_STRUCT    *prS3Instance,
 *              INT                     iInstanceNo,
 *              SIII_COMM_PARS_STRUCT   *prS3Pars
 *          )
 *
 * \public
 *
 * \brief   This function initializes the Sercos III soft master stack
 *          instance.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       iInstanceNo     SIII module instance number. This index
 *                                  later is used to identify a certain SIII
 *                                  instance. The SICE instance inherits that
 *                                  index, too.
 * \param[in]       prS3Pars        Pointer to SIII_PARS_STRUCT containing
 *                                  Sercos parameters. The parameters in the
 *                                  data structure should be initialized before
 *                                  calling this function.
 *
 * \details This function initializes the Sercos soft master SIII instance,
 *          including the Sercos IP core emulation and the SVC handler thread.
 *          SIII_Close() should be called to close it for shutdown to avoid
 *          memory leaks.
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:        No error
 *          - SIII_SEMAPHORE_ERROR: Problem with semaphore
 *          - SIII_SYSTEM_ERROR:    General system error
 *          - SIII_PARAMETER_ERROR: Illegal function parameter
 *          - SIII_MEM_ERROR:       Memory problem
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2012-10-15
 *
 * \version 2013-02-26 (GMy): Separation of initialization from user interface
 * \version 2013-04-11 (GMy): Module re-arrangement
 * \version 2013-06-24 (GMy): Init function of RTOS added
 * \version 2013-11-04 (GMy): Bugfix (Hot-Plug called cycle-exact)
 *
 */
SIII_FUNC_RET SIII_Init
    (
      SIII_INSTANCE_STRUCT    *prS3Instance,
      INT                     iInstanceNo,
      SIII_COMM_PARS_STRUCT   *prS3Pars
    )
{
  SIII_PHASE_STATE_STRUCT         *prPhaseStateStruct;    // Shortcut pointer
  SICE_INSTANCE_STRUCT            *prSiceInstance;        // Shortcut pointer
  SIII_CYCLIC_COMM_CTRL_STRUCT    *prCyclicCommCtrl;      // Shortcut pointer
  SICE_FUNC_RET                   eSiceFuncRet;
  SIII_FUNC_RET                   eS3FuncRet;
  INT                             iRet;
  SICE_INIT_STRUCT                rSiceInit;

  SIII_VERBOSE(3, "SIII_Init()\n");

  if (
      (prS3Instance   == NULL)    ||
      (prS3Pars       == NULL)
    )
  {
    return(SIII_PARAMETER_ERROR);
  }

  // Open RTOS instance
  iRet = RTOS_Init(iInstanceNo);

  if (iRet != RTOS_RET_OK)
  {
    SIII_VERBOSE(0, "RTOS initialization error\n");
    return(SIII_SYSTEM_ERROR);
  }

  // Get shortcut pointers
  prPhaseStateStruct  = &prS3Instance->rPhaseStateStruct;
  prSiceInstance      = &prS3Instance->rSiceInstance;
  prCyclicCommCtrl    = &prS3Instance->rCyclicCommCtrl;

  // Initialize instance variables
  (VOID) memset
      (
        prS3Instance,
        (UCHAR) 0x00,
        sizeof(SIII_INSTANCE_STRUCT)
      );

  prS3Instance->iInstanceNo = iInstanceNo;

  prS3Instance->rS3Pars = *prS3Pars;

  // Create Sercos IP core emulation instance
  SIII_VERBOSE(1, "Creating Sercos IP core emulation instance ... \n");

  // Prepare SICE initialization instance
  rSiceInit.iInstanceNo = iInstanceNo;

  eSiceFuncRet = SICE_Init
      (
        prSiceInstance,
        &rSiceInit
      );

  // Check for errors, ignore warnings
  if (eSiceFuncRet > SICE_END_ERR_CLASS_00000)
  {
    SIII_VERBOSE(0, "SICE initialization error\n");
    return(SIII_SYSTEM_ERROR);
  }
  else
  {
    SIII_VERBOSE(1, "  Done.\n");
  }

  // Create SVC semaphores
  SIII_VERBOSE(1, "Creating SVC block semaphore ... \n");
  if ((RTOS_CreateSemaphore(&prS3Instance->semSVCBlock, "S.SVCBlock")) != RTOS_RET_OK)
  {
    SIII_VERBOSE(0, "  Error: SVC block semaphore could not be created!\n");
    return(SIII_SEMAPHORE_ERROR);
  }
  else
  {
    SIII_VERBOSE(1, "  Done.\n");
  }

  SIII_VERBOSE(1, "Creating SVC start semaphore ... \n");
  if ((RTOS_CreateSemaphore(&prS3Instance->semSVCStart, "S.SVCStart")) != RTOS_RET_OK)
  {
    SIII_VERBOSE(0, "  Error: SVC start semaphore could not be created !");
    return(SIII_SEMAPHORE_ERROR);
  }
  else
  {
    SIII_VERBOSE(1, "  Done.\n");
  }

  // Create SVC thread
  SIII_VERBOSE(1, "Creating SVC thread ... \n");
  iRet = RTOS_CreateThread
      (
        (VOID*) SIII_SVCThread,      // Thread function
        &prS3Instance->rSVCThread,   // Thread data structure
        "T.SVC",                     // Thread name string
        (VOID*) prS3Instance         // Function argument
      );

  if (iRet != RTOS_RET_OK)
  {
    SIII_VERBOSE(0, "  Error: SVC thread could not be created!");
    return(SIII_SYSTEM_ERROR);
  }
  else
  {
    SIII_VERBOSE(1, "  Done.\n");
  }

  (VOID)RTOS_PostSemaphore(&prS3Instance->semSVCBlock);

  // Initialize data structure for controlling cyclic Sercos communication
  prCyclicCommCtrl->boCallReadAT          = FALSE;
  prCyclicCommCtrl->boCallWriteMDT        = FALSE;
  prCyclicCommCtrl->boCallTxRxSoftCont    = FALSE;
  prCyclicCommCtrl->boAppDataValid        = FALSE;
  prCyclicCommCtrl->boPowerOn             = FALSE;
  prCyclicCommCtrl->boHotplugCyclicPhase  = FALSE;
  prCyclicCommCtrl->boCyclicDataError     = FALSE;
  prCyclicCommCtrl->eCyclicCsmdError      = CSMD_NO_ERROR;

  eS3FuncRet = SIII_ClearCyclicDataValid(prS3Instance);
  if (eS3FuncRet != SIII_NO_ERROR)
  {
    SIII_VERBOSE(0, "Error: Could not re-set cyclic data flags!");
    return(SIII_MEM_ERROR);
  }

  // Initialize function pointers for devices
  eS3FuncRet = SIII_ClearDeviceCallbacks(prS3Instance);
  if (eS3FuncRet != SIII_NO_ERROR)
  {
    SIII_VERBOSE(0, "Error: Could not re-set device callbacks!");
    return(SIII_MEM_ERROR);
  }

  // Initialize phase state structure for Sercos phase handler
  prPhaseStateStruct->ucCsmdStateNew          = (UCHAR) SIII_CSMD_STATE_IDLE;
  prPhaseStateStruct->ucCsmdStateCurr         = (UCHAR) SIII_CSMD_STATE_IDLE;
  prPhaseStateStruct->ucPhaseState            = (UCHAR) SIII_PHASE_STATE_IDLE;
  prPhaseStateStruct->ucRetries               = (UCHAR) 0;
  prPhaseStateStruct->ulCsmdSleepCnt          = (ULONG) 0;
  prPhaseStateStruct->boSwitchBackCP          = FALSE;
  prPhaseStateStruct->ulPhaseHandlerWaitUs    = (ULONG) SIII_PHASE_HANDLER_WAIT_TIME;

  prS3Instance->usDevCnt              = (USHORT) 0;
  prS3Instance->pusCosemaRecDevList   = NULL;

  // Slave configuration has to me made by calling entity outside of module
  // SIII

  if (prS3Instance->rS3Pars.boDetectSlaveConfig == FALSE)
  {
    prS3Instance->boSetSlaveConfig = FALSE;
  }

  SIII_VERBOSE
      (
        2,
        "Size of SIII instance data structure: %u Bytes\n",
        sizeof(SIII_INSTANCE_STRUCT)
      );

  return(SIII_NO_ERROR);
}

/**
 * \fn SIII_FUNC_RET SIII_Close(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \public
 *
 * \brief   Cleanup before closing Sercos III soft master
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:        No error
 *          - SIII_PARAMETER_ERROR: Illegal function parameter
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2012-10-15
 *
 * \version 2013-02-19 (GMy): Closing of SVC thread added
 * \version 2013-04-11 (GMy): Module re-arrangement
 * \version 2013-06-24 (GMy): Close function of RTOS added
 */
SIII_FUNC_RET SIII_Close
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  SICE_FUNC_RET eSiceFuncRet;

  SIII_VERBOSE(3, "SIII_Close()\n");

  if  (prS3Instance == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  // Close SVC thread and destroy thread data structure
  if (RTOS_CloseThreadRemote(&prS3Instance->rSVCThread) != RTOS_RET_OK)
  {
    SIII_VERBOSE(0, "Error closing SVC thread.\n");
  }
  RTOS_DestroyThreadData(&prS3Instance->rSVCThread);

  // Close SICE instance
  eSiceFuncRet = SICE_Close(&prS3Instance->rSiceInstance);

  if (eSiceFuncRet != SICE_NO_ERROR)
  {
    SIII_VERBOSE(0, "Error closing SICE instance.\n");
  }

  (VOID)RTOS_Close(prS3Instance->iInstanceNo);

  return((SIII_FUNC_RET) eSiceFuncRet);
}

