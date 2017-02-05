/**
 * \file      SIII_CONF.c
 *
 * \brief     Sercos III soft master stack - High-level configuration and
 *            status functions
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
 * \copyright Copyright Bosch Rexroth AG, 2013-2016
 *
 * \author    GMy
 *
 * \date      2013-03-05
 *
 * \version 2013-03-27 (GMy): New functions SIII_SetSlaveConfig() and
 *                            SIII_GetSlaveConfig()
 * \version 2013-04-11 (GMy): Module re-arrangement and several new high-level
 *                            interface functions
 * \version 2013-05-07 (GMy): Added support for INtime
 * \version 2013-06-06 (GMy): Added support for RTX and Kithara
 * \version 2013-06-13 (GMy): Added SWC support
 * \version 2013-06-20 (GMy): Optimization of error handling
 * \version 2013-06-21 (GMy): Added support for Windows desktop versions and
 *                            QNX
 * \version 2013-06-25 (GMy): New functions SIII_ActivateSercosTime() and
 *                            SIII_ReadSercosTime()
 * \version 2014-04-01 (GMy): Added support for CoSeMa 6VRS
 * \version 2014-09-12 (GMy): Added function SIII_InitConnections() for
 *                            improved connection handling
 * \version 2015-12-02 (GMy): New function SIII_GetSercosStatus()
 * \version 2016-10-27 (AlM): Support for CoSeMa V5 removed.
 * \version 2016-11-30 (GMy): New function SIII_SetSercosTimingMode()
 */

//---- includes ---------------------------------------------------------------

#include "../CSMD/CSMD_USER.h"
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

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn INT SIII_GetNoOfSlaves(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \public
 *
 * \brief   This function returns the number of detected Sercos slaves.
 *
 * \note    The function shall not be called before being in Sercos phase CP0.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return
 * - In Sercos CP0 or later:    Number of of detected slaves
 * - In NRT mode:               -1
 * - Illegal SIII instance:     -2
 *
 * \ingroup SIII
 *
 * \author GMy
 *
 * \date 2013-02-05
 *
 */
/*lint -save -e818 const! */
INT SIII_GetNoOfSlaves
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  SIII_VERBOSE(3, "SIII_GetNoOfSlaves()\n");

  if (prS3Instance == NULL)
  {
    return(-2);
  }
  else if (SIII_GetSercosPhase(prS3Instance) >= SIII_PHASE_CP0)
  {
    return((INT) prS3Instance->usDevCnt);
  }
  else
  {
    return(-1);
  }
}
/*lint -restore const! */

/**
 * \fn SIII_PHASE SIII_GetSercosPhase(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \public
 *
 * \brief   Returns the current Sercos phase.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  Current Sercos phase (see definition of SIII_PHASE)
 *
 * \ingroup SIII
 *
 * \author GMy
 *
 * \date 2013-02-05
 *
 */
/*lint -save -e818 const! */
SIII_PHASE SIII_GetSercosPhase
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  SIII_VERBOSE(3, "SIII_GetSercosPhase()\n");

  return((SIII_PHASE) prS3Instance->rCosemaInstance.sCSMD_Phase);
}
/*lint -restore const! */

/**
 * \fn SIII_PHASE SIII_GetSercosTopology(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \public
 *
 * \brief   Returns the current Sercos topology.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  Current Sercos topology from CoSeMa instance
 *
 * \ingroup SIII
 *
 * \author AlM
 *
 * \date 2016-08-02
 *
 */
/*lint -save -e818 const! */
USHORT SIII_GetSercosTopology
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  SIII_VERBOSE(3, "SIII_GetSercosTopology()\n");

  return(prS3Instance->rCosemaInstance.usCSMD_Topology);
}
/*lint -restore const! */

/**
 * \fn SIII_FUNC_RET SIII_CheckCycleTime(
 *              ULONG ulNewCycleTime,
 *              SIII_PHASE ePhase
 *          )
 *
 * \public
 *
 * \brief   Checks whether the cycle time is valid for a certain Sercos phase
 *          according to the Sercos specification.
 *
 * \param[in]   ulNewCycleTime  Cycle time in ns
 * \param[in]   ePhase          Designates the Sercos phase, see definition of
 *                              SIII_PHASE
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR                     for valid cycle time
 *          - SIII_SERCOS_CYCLE_TIME_INVALID    for illegal cycle time
 *
 * \ingroup SIII
 *
 * \author  WK, GMy
 *
 * \date    2012-11-13
 *
 * \version 2013-04-13 (GMy): Added support for CP0..CP2
 * \version 2013-06-13 (GMy): Added SWC support
 * \version 2013-06-19 (GMy): Bugfix: Logic for blocking cycle times <1ms
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_CheckCycleTime
    (
      ULONG ulNewCycleTime,
      SIII_PHASE ePhase
    )
{
  SIII_VERBOSE(3, "SIII_CheckCycleTime()\n");

  // Is CP value valid?
  if (
      (ePhase != SIII_PHASE_CP0)  &&
      (ePhase != SIII_PHASE_CP1)  &&
      (ePhase != SIII_PHASE_CP2)  &&
      (ePhase != SIII_PHASE_CP3)  &&
      (ePhase != SIII_PHASE_CP4)
    )
  {
    return(SIII_SERCOS_PHASE_ERROR);
  }

  // Cycle times < 1ms are only allowed in CP3 and CP4
  if (
      (
        (ePhase == SIII_PHASE_CP0) ||
        (ePhase == SIII_PHASE_CP1) ||
        (ePhase == SIII_PHASE_CP2)
      )                                &&
      (ulNewCycleTime < ((ULONG) CSMD_TSCYC_1_MS))
    )
  {
    return(SIII_SERCOS_CYCLE_TIME_INVALID);
  }

  if (ulNewCycleTime >= ((ULONG) CSMD_TSCYC_250_US))
  {
    // Cycle time must be multiple of 250us and not greater than maximum
    // cycle time
    if  (
        ((ulNewCycleTime % ((ULONG) CSMD_TSCYC_250_US)) != 0)   ||
        (ulNewCycleTime > ((ULONG) CSMD_TSCYC_MAX))
      )
    {
      return(SIII_SERCOS_CYCLE_TIME_INVALID);
    }
    else
    {
      return(SIII_NO_ERROR);
    }
  }
  else
  {
    if  (
        (ulNewCycleTime != ((ULONG) CSMD_TSCYC_MIN))      &&
        (ulNewCycleTime != ((ULONG) CSMD_TSCYC_62_5_US))  &&
        (ulNewCycleTime != ((ULONG) CSMD_TSCYC_125_US))
      )
    {
      return(SIII_SERCOS_CYCLE_TIME_INVALID);
    }
    else
    {
      return(SIII_NO_ERROR);
    }
  }
}
/*lint -restore const! */

/**
 * \fn SIII_FUNC_RET SIII_SetSlaveConfig(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT *pusOperDevList
 *          )
 *
 * \public
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       pusOperDevList  Pointer to Sercos list of Sercos slaves
 *                                  - [0]:  Actual length in bytes (number of
 *                                          slaves * 2)
 *                                  - [1]:  Maximum length (in bytes), can be
 *                                          set to same value as [0]
 *                                  - [2]:  Sercos address of first slave
 *                                  - [3]:  Sercos address of second slave,
 *                                          etc.
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:        Success
 *          - SIII_PARAMETER_ERROR: Erroneous slave device list or other
 *                                  function parameter error
 *          - SIII_NO_EFFECT:       In case of the detected slaves during
 *                                  startup are used and the Sercos slave
 *                                  device list is not taken over
 *
 * \brief   This function sets the Sercos slave configuration.
 *
 * \note    The function has to be called before switching to Sercos phase CP1.
 *          If called in a higher Sercos phase, the configuration is taken over
 *          but does not become active until the next Sercos startup to phase
 *          CP1.
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    26.03.2013
 *
 * \version 2015-05-21 (GMy): Bugfix: Logical problem with return value
 *                            SIII_NO_EFFECT
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_SetSlaveConfig
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT *pusOperDevList
    )
{
  INT iDevCnt = 0;
  INT iCnt    = 0;

  SIII_VERBOSE(3, "SIII_SetSlaveConfig()\n");

  if (prS3Instance == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  if(!prS3Instance->rS3Pars.boDetectSlaveConfig)
  {
    // Actual length of list larger than maximum?
    if (pusOperDevList[0] > pusOperDevList[1])
    {
      return(SIII_PARAMETER_ERROR);
    }

    // Check number of slaves
    iDevCnt = (INT)(pusOperDevList[0] / sizeof(USHORT));
    if (iDevCnt > SIII_MAX_SLAVES)
    {
      return(SIII_PARAMETER_ERROR);
    }

    for (   
        iCnt = 0;
        iCnt < iDevCnt + 2; // Number of slaves + actual
                            // length and max. length fields
        iCnt++
      )
    {
      prS3Instance->ausOperDevList[iCnt] = pusOperDevList[iCnt];
    }

    prS3Instance->boSetSlaveConfig = TRUE;
    return(SIII_NO_ERROR);
  }
  else
  {
    SIII_VERBOSE
        (
          1,
          "Configured slave list overridden by detected slave list "
          "due to use detected slave configuration mode\n"
        );

    return(SIII_NO_EFFECT);
  }
}
/*lint -restore const! */

/**
 * \fn USHORT* SIII_GetSlaveConfig(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              BOOL boDetectedSlaves
 *          )
 *
 * \public
 *
 * \param[in,out]   prS3Instance        Pointer to SIII instance structure
 * \param[in]       boDetectedSlaves
 *                                      - TRUE: Get list of detected slaves
 *                                      - FALSE: Get list of configured slaves
 *
 * \return
 *  - In case of success: Pointer to list of Sercos slaves (NULL before
 *  CP0)
 *      - [0]: Actual length in bytes (number of slaves * 2)
 *      - [1]: Maximum length (in bytes), can be ignored
 *      - [2]: Sercos address of first slave
 *      - [3]: Sercos address of second slave, etc.
 *  - In case of error: NULL
 *
 * \brief   This function retrieves the slave configuration. It shall be called
 *          not before switching to Sercos phase CP0. The parameter
 *          boDetectedSlaves has only an effect if the use of the detected
 *          slave configuration is not used, otherwise the detected slaves are
 *          identical to the configured ones.
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    26.03.2013
 *
 */
/*lint -save -e818 const! */
USHORT* SIII_GetSlaveConfig
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      BOOL boDetectedSlaves
    )
{
  SIII_VERBOSE(3, "SIII_GetSlaveConfig()\n");


  if (prS3Instance == NULL)
  {
    return(NULL);
  }
  if (boDetectedSlaves)
  {
    return(prS3Instance->pusCosemaRecDevList);
  }
  else
  {
    if (prS3Instance->rS3Pars.boDetectSlaveConfig)
    {
      return(prS3Instance->ausOperDevList);
    }
    else
    {
      // in case of using the detected slave configuration, the
      // recognized device list is identical to the configured slave
      // list.
      return(prS3Instance->pusCosemaRecDevList);
    }
  }
}
/*lint -restore const! */

#ifdef CSMD_HW_WATCHDOG
/**
 * \fn SIII_FUNC_RET SIII_SetWatchdog(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT usWDMode,
 *              USHORT usCycles
 *          )
 *
 * \public
 *
 * \brief   This function enables the SICE watchdog.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       usWDMode        Watchdog alarm reaction mode
 *                                  - 0 (CSMD_HAL_WD_TO_SEND_EMPTY_TEL):
 *                                      Send empty telegrams
 *                                  - 1 (CSMD_HAL_WD_TO_DISABLE_TX_TEL):
 *                                      Stop sending telegrams
 * \param[in]       usCycles        Watchdog reload value (number of Sercos
 *                                  cycles)
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:        For success
 *          - SIII_PARAMETER_ERROR: For unknown watchdog mode or other function
 *                                  parameter error
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-11-04
 *
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_SetWatchdog
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT usWDMode,
      USHORT usCycles
    )
{
  SIII_VERBOSE(3, "SIII_SetWatchdog()\n");

  if (prS3Instance == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  if (
      (usWDMode != CSMD_HAL_WD_TO_SEND_EMPTY_TEL) &&
      (usWDMode != CSMD_HAL_WD_TO_DISABLE_TX_TEL)
    )
  {
    SIII_VERBOSE
        (
          0,
          "Unknown watchdog mode %d.\n",
          usWDMode
        );
    return(SIII_PARAMETER_ERROR);
  }

  prS3Instance->rCosemaInstance.usWD_Mode = usWDMode;

  CSMD_Watchdog_Configure
      (
        &prS3Instance->rCosemaInstance,
        usCycles
      );
  CSMD_Watchdog_Control
      (
        &prS3Instance->rCosemaInstance,
        TRUE
      );
  return(SIII_NO_ERROR);
}
/*lint -restore const! */
#endif

#ifdef CSMD_HW_WATCHDOG
/**
 * \fn SIII_FUNC_RET SIII_DisableWatchdog(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          );
 *
 * \public
 *
 * \brief   This function disables the SICE watchdog.
 *
 * \param[in,out]   prS3Instance Pointer to SIII instance structure
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:        For success
 *          - SIII_PARAMETER_ERROR: For function parameter error
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-11-04
 *
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_DisableWatchdog
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  SIII_VERBOSE(3, "SIII_DisableWatchdog()\n");

  if (prS3Instance == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  CSMD_Watchdog_Control
      (
        &prS3Instance->rCosemaInstance,
        FALSE
      );
  return(SIII_NO_ERROR);
}
/*lint -restore const! */
#endif

/**
 * \fn SIII_FUNC_RET SIII_ClearDeviceCallbacks(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          );
 *
 * \public
 *
 * \brief   This function clears all configured Sercos slave device callback
 *          functions.
 *
 * \param[in,out]   prS3Instance Pointer to SIII instance structure
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:        For success
 *          - SIII_PARAMETER_ERROR: For function parameter error
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-11-04
 *
 * \version 2013-08-05 (GMy): Added support for device-independent (global)
 *                            callbacks
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_ClearDeviceCallbacks
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  INT iCnt;

  SIII_VERBOSE(3, "SIII_ClearDeviceCallbacks()\n");

  if (prS3Instance == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  prS3Instance->fpAppConnConfigGlob = NULL;
  prS3Instance->fpAppCyclicGlob     = NULL;

  for (
      iCnt = 0;
      iCnt < SIII_MAX_SLAVES;
      iCnt++
    )
  {
    prS3Instance->afpAppConnConfig[iCnt] = NULL;
    prS3Instance->afpAppCyclic[iCnt]     = NULL;
  }

  return(SIII_NO_ERROR);
}
/*lint -restore const! */

/**
 * \fn SIII_FUNC_RET SIII_SetDeviceCallback(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT usDevIdx,
 *              VOID* fpAppConnConfig,
 *              VOID* fpAppCyclic
 *          )
 *
 * \public
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       usDevIdx        Slave device index
 * \param[in]       fpAppConnConfig Function pointer to connection
 *                                  configuration callback function. This
 *                                  callback is called by the phase handler
 *                                  after reaching Sercos CP2.
 * \param[in]       fpAppCyclic     Function pointer to cyclic callback
 *                                  function that is called in CP4 every Sercos
 *                                  cycle. This function has to be
 *                                  non-blocking.
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR         For success
 *          - SIII_DEVICE_IDX_ERROR For invalid device index
 *          - SIII_PARAMETER_ERROR  For other function parameter error
 *
 * \brief   This function sets the given callback functions for the slave
 *          device given by the slave index. The pointers of function callbacks
 *          that are not used should be set to 'NULL'.
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-11-04
 *
 * \version 2013-06-19 (GMy): Check for valid device index
 * \version 2013-08-05 (GMy): Added support for device-independent (global)
 *                            callbacks
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_SetDeviceCallback
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT usDevIdx,
      VOID* fpAppConnConfig,
      VOID* fpAppCyclic
    )
{
  SIII_VERBOSE(3, "SIII_SetDeviceCallback()\n");

  if (prS3Instance == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  if (usDevIdx == (USHORT) SIII_ALL_DEVICES)
  {
    prS3Instance->fpAppConnConfigGlob = (FP_APP_CONN_CONFIG_GLOB) fpAppConnConfig;
    prS3Instance->fpAppCyclicGlob = (FP_APP_CYCLIC_GLOB) fpAppCyclic;

    return(SIII_NO_ERROR);
  }
  else if (usDevIdx >= (USHORT) SIII_MAX_SLAVES)
  {
    return(SIII_DEVICE_IDX_ERROR);
  }
  else
  {
    prS3Instance->afpAppConnConfig[usDevIdx] =
        (FP_APP_CONN_CONFIG) fpAppConnConfig;
    prS3Instance->afpAppCyclic[usDevIdx] =
        (FP_APP_CYCLIC) fpAppCyclic;

    return(SIII_NO_ERROR);
  }
}
/*lint -restore const! */

/**
 * \fn SIII_FUNC_RET SIII_SetSercosCycleTime(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              SIII_PHASE ePhase,
 *              ULONG ulCycleTime
 *          )
 *
 * \public
 *
 * \brief   This function sets the given Sercos cycle time either for CP0..CP2
 *          or for CP4, depending on boCP3CP4. In case the requested cycle time
 *          is valid for the Sercos phases in question, it is taken over at the
 *          next switch to this Sercos phase.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       ePhase          Designates the Sercos phase, see definition
 *                                  of SIII_PHASE
 * \param[in]       ulCycleTime     New cycle time in ns
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR                     For valid cycle time
 *          - SIII_SERCOS_CYCLE_TIME_INVALID    For illegal cycle time
 *          - SIII_SERCOS_PHASE_ERROR           For illegal value of ePhase
 *          - SIII_PARAMETER_ERROR              For function parameter error
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-11-04
 *
 * \version 2013-06-13 (GMy): Added SWC support
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_SetSercosCycleTime
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      SIII_PHASE ePhase,
      ULONG ulCycleTime
    )
{
  SIII_FUNC_RET eS3FuncRet;

  SIII_VERBOSE(3, "SIII_SetSercosCycleTime()\n");

  if (prS3Instance == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  eS3FuncRet = SIII_CheckCycleTime
      (
        ulCycleTime,
        ePhase
      );

  if (eS3FuncRet == SIII_NO_ERROR)
  {
#ifdef CSMD_SWC_EXT
    if (ePhase == SIII_PHASE_CP0)
    {
      prS3Instance->rS3Pars.ulCycleTimeCP0 = ulCycleTime;
    }
    else if (
          (ePhase == SIII_PHASE_CP1)  ||
          (ePhase == SIII_PHASE_CP2)
        )
    {
      prS3Instance->rS3Pars.ulCycleTimeCP0CP2 = ulCycleTime;
    }
#else
    if (
        (ePhase == SIII_PHASE_CP0)  ||
        (ePhase == SIII_PHASE_CP1)  ||
        (ePhase == SIII_PHASE_CP2)
      )
    {
      prS3Instance->rS3Pars.ulCycleTimeCP0CP2 = ulCycleTime;
    }
#endif
    else if (
          (ePhase == SIII_PHASE_CP3)  ||
          (ePhase == SIII_PHASE_CP4)
        )
    {
      prS3Instance->rS3Pars.ulCycleTime = ulCycleTime;
    }
    else
    {
      return(SIII_SERCOS_PHASE_ERROR);
    }
  }
  return(eS3FuncRet);
}
/*lint -restore const! */

/**
 * \fn ULONG SIII_GetSercosCycleTime(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              SIII_PHASE ePhase
 *          )
 *
 * \public
 *
 * \brief   This function returns the current Sercos cycle time either for
 *          the selected Sercos phase.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       ePhase          Designates the Sercos phase, see definition
 *                                  of SIII_PHASE
 *
 * \return  -   Current Sercos cycle time in ns set for the selected Sercos
 *              phase, depending on ePhase. In case ePhase is SIII_PHASE_CURR,
 *              the cycle time of the currently active phase is returned.
 *          -   SIII_CYCLE_TIME_ERROR for illegal
 *              value of ePhase.
 *          -   0 in case of parameter error
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-04-11
 *
 * \version 2013-06-13 (GMy): Added SWC support
 * \version 2013-06-03 (GMy): Added option SIII_PHASE_CURR
 */
/*lint -save -e818 const! */
ULONG SIII_GetSercosCycleTime
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      SIII_PHASE ePhase
    )
{
  SIII_PHASE eMyPhase = SIII_PHASE_NRT;

  SIII_VERBOSE(3, "SIII_GetSercosCycleTime()\n");

  if (prS3Instance == NULL)
  {
    return(0);
  }

  if (ePhase == SIII_PHASE_CURR)
  {
    eMyPhase = SIII_GetSercosPhase(prS3Instance);
  }
  else
  {
    eMyPhase = ePhase;
  }


#ifdef CSMD_SWC_EXT
  if (eMyPhase == SIII_PHASE_CP0)
  {
    return(prS3Instance->rS3Pars.ulCycleTimeCP0);
  }
  else if (
        (eMyPhase == SIII_PHASE_CP1)  ||
        (eMyPhase == SIII_PHASE_CP2)
      )
  {
    return(prS3Instance->rS3Pars.ulCycleTimeCP0CP2);
  }
#else
  if (
      (eMyPhase == SIII_PHASE_CP0)  ||
      (eMyPhase == SIII_PHASE_CP1)  ||
      (eMyPhase == SIII_PHASE_CP2)
    )
  {
    return(prS3Instance->rS3Pars.ulCycleTimeCP0CP2);
  }
#endif
  else if (
        (eMyPhase == SIII_PHASE_CP3)  ||
        (eMyPhase == SIII_PHASE_CP4)
      )
  {
    return(prS3Instance->rS3Pars.ulCycleTime);
  }
  else
  {
    return(SIII_CYCLE_TIME_ERROR);
  }
}
/*lint -restore const! */

/**
 * \fn SIII_FUNC_RET SIII_SetSercosTimingMode(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT usNewMethod
 *          )
 *
 * \public
 *
 * \brief   This function sets the Sercos timing mode for CP3/CP4.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       usNewMethod     New timing mode
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR                     For valid cycle time
 *          - SIII_NOT_ALLOWED_IN_CURRENT_CP    For call of function in phase
 *                                              in CP3 or CP4
 *          - SIII_PARAMETER_ERROR              For function parameter error
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2016-11-30
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_SetSercosTimingMode
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT usNewMethod
    )
{
  SIII_VERBOSE(3, "SIII_SetSercosTimingMode()\n");

  if (prS3Instance == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  if (SIII_GetSercosPhase(prS3Instance) > SIII_PHASE_CP2)
  {
    return(SIII_NOT_ALLOWED_IN_CURRENT_CP);
  }
  else
  {
    if (
         (usNewMethod != CSMD_METHOD_MDT_AT_IPC)  &&
         (usNewMethod != CSMD_METHOD_MDT_IPC_AT)  &&
         (usNewMethod != CSMD_METHOD_AT_CYC_END)
       )
    {
      // Illegal timing mode
      return(SIII_PARAMETER_ERROR);
    }
    else
    {
      prS3Instance->rS3Pars.usS3TimingMethod = usNewMethod;
    }
  }

  return(SIII_NO_ERROR);
}
/*lint -restore const! */

/**
 * \fn SIII_FUNC_RET SIII_ActivateSercosTime(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              ULONG ulSeconds,
 *              ULONG ulNanos
 *          )
 *
 * \public
 *
 * \brief   This function activates the Sercos time. After activation, the
 *          Sercos time is counted up by SICE and the time is transmitted in
 *          the extended field of MDT0 in CP3 / CP4.
 *
 * \note    The function should only be called in CP3 or CP4.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       ulSeconds       Seconds value to set Sercos time to
 * \param[in]       ulNanos         Nano seconds value to set Sercos time to
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:                    Success
 *          - SIII_PARAMETER_ERROR:             Function parameter error
 *          - SIII_NOT_ALLOWED_IN_CURRENT_CP:   Not in CP3 or CP4
 *          - Otherwise inherited CSMD_FUNC_RET value
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-06-25
 *
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_ActivateSercosTime
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      ULONG ulSeconds,
      ULONG ulNanos
    )
{
  CSMD_FUNC_RET   eCosemaFuncRet;
  CSMD_SERCOSTIME rMySercosTime;

  SIII_VERBOSE(3, "SIII_ActivateSercosTime()\n");

  if (prS3Instance == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  if  (
      (SIII_GetSercosPhase(prS3Instance) != SIII_PHASE_CP3)   &&
      (SIII_GetSercosPhase(prS3Instance) != SIII_PHASE_CP4)
    )
  {
    return(SIII_NOT_ALLOWED_IN_CURRENT_CP);
  }

  rMySercosTime.ulNanos   = ulNanos;
  rMySercosTime.ulSeconds = ulSeconds;

  eCosemaFuncRet = CSMD_New_Sercos_Time
      (
        &prS3Instance->rCosemaInstance,
        rMySercosTime
      );

  return((SIII_FUNC_RET) eCosemaFuncRet);
}
/*lint -restore const! */

/**
 * \fn SIII_FUNC_RET SIII_ReadSercosTime(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              ULONG *pulSeconds,
 *              ULONG *pulNanos
 *          )
 *
 * \public
 *
 * \brief   This function reads the Sercos time from SICE.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       pulSeconds      Read seconds value
 * \param[in]       pulNanos        Read nano seconds value
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:                    For success
 *          - SIII_PARAMETER_ERROR:             For function parameter error
 *          - SIII_NOT_ALLOWED_IN_CURRENT_CP:   Not in CP3 or CP4
 *          - Otherwise inherited CSMD_FUNC_RET value
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-06-25
 *
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_ReadSercosTime
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      ULONG *pulSeconds,
      ULONG *pulNanos
    )
{
  CSMD_FUNC_RET   eCosemaFuncRet;
  CSMD_SERCOSTIME rSercosTime;

  SIII_VERBOSE(3, "SIII_ReadSercosTime()\n");

  if (
      (prS3Instance   == NULL) ||
      (pulSeconds   == NULL)   ||
      (pulNanos     == NULL)
    )
  {
    return(SIII_PARAMETER_ERROR);
  }

  if  (
      (SIII_GetSercosPhase(prS3Instance) != SIII_PHASE_CP3)   &&
      (SIII_GetSercosPhase(prS3Instance) != SIII_PHASE_CP4)
    )
  {
    return(SIII_NOT_ALLOWED_IN_CURRENT_CP);
  }

  eCosemaFuncRet = CSMD_Get_Sercos_Time
      (
        &prS3Instance->rCosemaInstance,
        &rSercosTime
      );

  SIII_VERBOSE
      (
        2,
        "SICE time: %lu:%lu\n",
        rSercosTime.ulSeconds,
        rSercosTime.ulNanos
      );

  if (eCosemaFuncRet == CSMD_NO_ERROR)
  {
    *pulSeconds = rSercosTime.ulSeconds;
    *pulNanos   = rSercosTime.ulNanos;
  }

  return((SIII_FUNC_RET) eCosemaFuncRet);
}
/*lint -restore const! */

/**
 * \fn SIII_FUNC_RET SIII_InitConnections(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \public
 *
 * \brief   This function initializes the lists of consumer and producer
 *          connections.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  Always SIII_NO_ERROR
 *
 * \ingroup SIII
 *
 * \author  GMy, based on code snippet by WK
 *
 * \date    2014-09-12
 *
 */
SIII_FUNC_RET SIII_InitConnections
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  USHORT usIdx;
  USHORT usActIdx;
  CSMD_CONN_IDX_STRUCT *parConnIdxList;

  parConnIdxList = prS3Instance->rCosemaInstance.rConfiguration.rMasterCfg.parConnIdxList;

  /* Generate list without gaps for all by the master produced connections */
  for (
      usIdx = 0, usActIdx = 0;
      usIdx < prS3Instance->rCosemaInstance.rPriv.rSystemLimits.usMaxConnMaster;
      usIdx++
    )
  {
    if (
        (   parConnIdxList[usIdx].usConnIdx <
              prS3Instance->rCosemaInstance.rPriv.rSystemLimits.usMaxGlobConn
        )                                         &&
        (
          parConnIdxList[usIdx].usConfigIdx <
              prS3Instance->rCosemaInstance.rPriv.rSystemLimits.usMaxGlobConfig
        )                                         &&
        (
          (
            prS3Instance->rCosemaInstance.rConfiguration.parConfiguration[parConnIdxList[usIdx].usConfigIdx].usS_0_1050_SE1
              & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK
          ) == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER
        )
      )
    {
      prS3Instance->rSERC_Act_Prod.arActIdxList[usActIdx].usConnIdx =
          parConnIdxList[usIdx].usConnIdx;
      prS3Instance->rSERC_Act_Prod.arActIdxList[usActIdx].usConfigIdx =
          parConnIdxList[usIdx].usConfigIdx;
      usActIdx++;
    }
  }
  /* Number of connections produced by the master */
  prS3Instance->rSERC_Act_Prod.usNbrActProd = usActIdx;

  /* Generate list without gaps for all by the master consumed connections */
  for (
      usIdx = 0, usActIdx = 0;
      usIdx < prS3Instance->rCosemaInstance.rPriv.rSystemLimits.usMaxConnMaster;
      usIdx++
    )
  {
    if  (
        (
          parConnIdxList[usIdx].usConnIdx <
              prS3Instance->rCosemaInstance.rPriv.rSystemLimits.usMaxGlobConn
        )                                         &&
        (
          parConnIdxList[usIdx].usConfigIdx <
          prS3Instance->rCosemaInstance.rPriv.rSystemLimits.usMaxGlobConfig
        )                                         &&
        (
            (
              prS3Instance->rCosemaInstance.rConfiguration.parConfiguration[parConnIdxList[usIdx].usConfigIdx].usS_0_1050_SE1
              & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK
            ) == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER
          )
      )
    {
      prS3Instance->rSERC_Act_Cons.arActIdxList[usActIdx].usConnIdx   = parConnIdxList[usIdx].usConnIdx;
      prS3Instance->rSERC_Act_Cons.arActIdxList[usActIdx].usConfigIdx = parConnIdxList[usIdx].usConfigIdx;
      usActIdx++;
    }
  }
  /* Number of connections produced by the master */
  prS3Instance->rSERC_Act_Cons.usNbrActCons = usActIdx;

  /* initialize connection info structs */
  (VOID) memset
      (
          prS3Instance->arConnInfoMDT,
          0xFF,
          SIII_MAX_SLAVES * SIII_MAX_CONN_PER_SLAVE * sizeof(SIII_CONN_INFO_STRUCT)
      );

  (VOID) memset
      (
          prS3Instance->arConnInfoAT,
          0xFF,
          SIII_MAX_SLAVES * SIII_MAX_CONN_PER_SLAVE * sizeof(SIII_CONN_INFO_STRUCT)
      );

  return(SIII_NO_ERROR);
}


/**
 * \fn SIII_FUNC_RET SIII_BuildConnInfoList(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \public
 *
 * \brief   This function fills the structure arConnInfo which contains length
 *          and local buffer offset for all configured connections. Furthermore,
 *          the function specific profile of all slaves is read and stored.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  Always SIII_NO_ERROR
 *
 * \ingroup SIII
 *
 * \author  AlM
 *
 * \date    2016-10-17
 *
 */
SIII_FUNC_RET SIII_BuildConnInfoList
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  USHORT usI;
  USHORT usK;
  USHORT usSlaveIdx;             /* slave index in list of projected slaves */
  USHORT usConnIdx;              /* CoSeMa connection index */
  USHORT usConfigIdx;            /* CoSeMa configuration index */
  USHORT usOffsetMDT = 0;
  USHORT usOffsetAT = 0;
  USHORT usConnCountMDT;          /* counter for connection instance in arConnInfoMDT */
  USHORT usConnCountAT;           /* counter for connection instance in arConnInfoAT */
  SIII_FUNC_RET eS3Ret;

  CSMD_CONFIG_STRUCT* prConfig = &prS3Instance->rCosemaInstance.rConfiguration;

  /* parse list of projected slaves */
  for (usSlaveIdx = 0; usSlaveIdx < prS3Instance->rCosemaInstance.rSlaveList.usNumProjSlaves; usSlaveIdx++)
  {
    // Read S-0-1302.0.1 and store result in S3Instance.arDeviceInfo
    eS3Ret = SIII_SVCRead
        (
          prS3Instance,     // SIII instance
          usSlaveIdx,       // Device index
          TRUE,             // Standard or specific parameter?
          1302,             // IDN
          0,                // Structural instance
          1,                // Structural element
          (USHORT) 7,       // Element 7: Operational data
          prS3Instance->rMySVCResult.ausSVCData,
                            // Data pointer
          (USHORT)SIII_SVC_BUF_SIZE
                            // Buffer size
        );

    if (eS3Ret != SIII_NO_ERROR)
    {
      SIII_VERBOSE
          (
            0,
            "Error #%X during SVC access.\n",
            (INT)eS3Ret
          );

      /* set profile to drive if an error occurred reading S-0-1302.0.1 */
      prS3Instance->arDeviceInfo[usSlaveIdx].usFSP = SIII_S_1302_0_1_FSP_DRIVE;
    }
    else
    {
      /* read high word of S-0-1302.0.1 */
      prS3Instance->arDeviceInfo[usSlaveIdx].usFSP = prS3Instance->rMySVCResult.ausSVCData[1];
    }

    /* initialize counter variables */
    usConnCountMDT = 0;
    usConnCountAT = 0;

    /* parse list of connections configured for the slave */
    for (usI = 0; usI < SIII_MAX_CONN_PER_SLAVE; usI++)
    {
      /* check if connection with current index is activated */
      usConfigIdx = prConfig->parSlave_Config[usSlaveIdx].arConnIdxList[usI].usConfigIdx;

      if (usConfigIdx < CSMD_MAX_GLOB_CONFIG)
      {
        if (prConfig->parConfiguration[usConfigIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE)
        {
          usConnIdx = prConfig->parSlave_Config[usSlaveIdx].arConnIdxList[usI].usConnIdx;

          if (usConnIdx < CSMD_MAX_GLOB_CONN)
          {
            /* distinguish between MDT and AT */
            if (prConfig->parConnection[usConnIdx].usS_0_1050_SE3 & CSMD_S_0_1050_SE3_TELTYPE_MDT)
            {
              /* MDT connection */
              prS3Instance->arConnInfoMDT[usSlaveIdx][usConnCountMDT].usOffset = usOffsetMDT;
              prS3Instance->arConnInfoMDT[usSlaveIdx][usConnCountMDT].usLength = prConfig->parConnection[usConnIdx].usS_0_1050_SE5;
              prS3Instance->arConnInfoMDT[usSlaveIdx][usConnCountMDT].usConnIdx = usConnIdx;
              usOffsetMDT += prS3Instance->arConnInfoMDT[usSlaveIdx][usConnCountMDT].usLength;
              usConnCountMDT++;
            }
            else
            {
              /* AT connection */
              /* check if master is consumer of the connection */
              for (usK = 0; usK < prConfig->prMaster_Config->usNbrOfConnections; usK++)
              {
                /* check if connection index appears in list of connections with master involved */
                if (prConfig->prMaster_Config->parConnIdxList[usK].usConnIdx == usConnIdx)
                {
                  prS3Instance->arConnInfoAT[usSlaveIdx][usConnCountAT].usOffset = usOffsetAT;
                  prS3Instance->arConnInfoAT[usSlaveIdx][usConnCountAT].usLength = prConfig->parConnection[usConnIdx].usS_0_1050_SE5;
                  prS3Instance->arConnInfoAT[usSlaveIdx][usConnCountAT].usConnIdx = usConnIdx;
                  usOffsetAT += prS3Instance->arConnInfoAT[usSlaveIdx][usConnCountAT].usLength;
                  usConnCountAT++;
                  break;
                }
              }
            }
          } /* if (usConnIdx <= CSMD_MAX_GLOB_CONN) */
        } /* if (prConfig->parConfiguration[usConfigIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE) */
      } /* if (usConfigIdx > CSMD_MAX_GLOB_CONFIG) */
    } /* for (usI = 0; usI < SIII_MAX_CONN_PER_SLAVE; usI++) */
  } /* for (usSlaveIdx = 0; usSlaveIdx < prS3Instance->rCosemaInstance.rSlaveList.usNumProjSlaves; usSlaveIdx++) */

  return(SIII_NO_ERROR);
} /* SIII_FUNC_RET SIII_BuildConnInfoList() */


/**
 * \fn SIII_FUNC_RET SIII_GetSercosStatus(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              SIII_SERCOS_STATUS *prSercosStatus
 *          )
 *
 * \public
 *
 * \brief   This function retrieves the Sercos network status.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[out]      prSercosStatus  Retrieved Sercos network status. For
 *                                  the semantics, please see definition of
 *                                  SIII_SERCOS_STATUS.
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:                    For success
 *          - SIII_PARAMETER_ERROR:             For function parameter error
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2015-12-02
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_GetSercosStatus
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      SIII_SERCOS_STATUS *prSercosStatus
    )
{
  SIII_VERBOSE(3, "SIII_GetSercosStatus()\n");

  if (
      (prS3Instance   == NULL) ||
      (prSercosStatus == NULL)
    )
  {
    return(SIII_PARAMETER_ERROR);
  }

  prSercosStatus->ePhase     = (SIII_PHASE) prS3Instance->rCosemaInstance.sCSMD_Phase;
  prSercosStatus->usTopology = prS3Instance->rCosemaInstance.usCSMD_Topology;

  return(SIII_NO_ERROR);
}
/*lint -restore const! */


/**
 * \fn SIII_FUNC_RET   SIII_GetControlIdx(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT                usDevIdx,
 *              USHORT*               pusConnIdx
 *          )
 *
 * \public
 *
 * \brief   This function returns the index of the MDT connection
 *          which contains the drive control word S-0-0134 (refers to arConnInfoMDT).
 *          For I/Os, this function currently always returns index 0
 *
 * \param[in]   prS3Instance    Pointer to SIII instance structure
 * \param[in]   usDevIdx        Device index
 * \param[out]  pusConnIdx      Output pointer to connection index
 *                                (refers to arConnInfoMDT[usDexIdx][INDEX])
 *
 * \return  SIII_NO_EFFECT\n
 *          SIII_PARAMETER_ERROR\n
 *          SIII_NO_ERROR
 *
 * \ingroup SIII
 *
 * \author  AlM
 *
 * \date    2016-10-25
 *
 * \version 2016-11-09 (AlM): Fixed index search for I/Os
 */
/*lint -save -e818 const! */
SIII_FUNC_RET SIII_GetControlIdx
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT                usDevIdx,
      USHORT*               pusConnIdx
    )
{
  USHORT usI, usK, usL;
  USHORT usConfigIdx;
  USHORT usConnIdx;
  ULONG* pulIdnList;
  USHORT usElements;
  BOOL   boFound = FALSE;

  CSMD_CONFIG_STRUCT*  prConfig = &prS3Instance->rCosemaInstance.rConfiguration;

  SIII_VERBOSE(3, "SIII_GetControlIdx()\n");

  if (prS3Instance   == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  if (prS3Instance->arDeviceInfo[usDevIdx].usFSP == SIII_S_1302_0_1_FSP_DRIVE)
  {
    /* scan all connections configured for the slave */
    for (usI = 0; (usI < prConfig->parSlaveConfig[usDevIdx].usNbrOfConnections) && (boFound == FALSE); usI++)
    {
      usConfigIdx = prConfig->parSlaveConfig[usDevIdx].arConnIdxList[usI].usConfigIdx;

      /* check if configuration exists */
      if (usConfigIdx < CSMD_MAX_GLOB_CONFIG)
      {
        pulIdnList = (ULONG*)prConfig->parConfiguration[usConfigIdx].ulS_0_1050_SE6;
        usElements = *((USHORT*)pulIdnList) / 4;

        if (usElements > 1)
        {
          usElements = 1; /* only first element in configuration list is checked, so S-0-0134
                             is expected to be configured as first IDN in connection */
        }
        pulIdnList++;

        /* parse idn configuration list of the found configuration index for S-0-0134 */
        for (usK = 0; (usK < usElements) && (boFound == FALSE); usK++)
        {
          if (*pulIdnList == CSMD_EIDN(134, 0, 0))
          {
            usConnIdx = prConfig->parSlaveConfig[usDevIdx].arConnIdxList[usI].usConnIdx;

            for (usL = 0; usL < prConfig->parSlaveConfig[usDevIdx].usNbrOfConnections; usL++)
            {
              /* return index of connection in arConnInfoMDT[usDevIdx][INDEX] */
              if (prS3Instance->arConnInfoMDT[usDevIdx][usL].usConnIdx == usConnIdx)
              {
                *pusConnIdx = usL;
                boFound = TRUE;
                break;
              }
            }
            if (boFound == FALSE)
            {
              return (SIII_PARAMETER_ERROR);
            }
          }
          pulIdnList++;
        }
      } /* if (usConfigIdx <= CSMD_MAX_GLOB_CONFIG) */
    } /* for (usI = 0; (usI < prConfig->parSlaveConfig[usDevIdx].usNbrOfConnections) && (boFound == FALSE); usI++) */
  } /* if (prS3Instance->arDeviceInfo[usDevIdx].usFSP == SIII_S_1302_0_1_FSP_DRIVE) */
  else
  {
    /* for I/Os, always select first connection of slave in MDT */
    *pusConnIdx = 0;
    boFound = TRUE;
  }

  if (boFound == TRUE)
  {
    return(SIII_NO_ERROR);
  }
  else
  {
    return (SIII_NO_EFFECT);
  }
}
/*lint -restore const! */

