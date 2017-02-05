/**
 * \file      SIII_HOTPLUG.c
 *
 * \brief     Sercos III soft master stack - Hotplug functions
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
 * \copyright Copyright Bosch Rexroth AG, 2013-2016
 *
 * \date      2013-04-11
 *
 * \version 2013-06-20 (GMy): Optimization of error handling
 * \version 2016-10-27 (AlM): Support for CoSeMa V5 removed.
 */

//---- includes ---------------------------------------------------------------

#include "../SIII/SIII_GLOB.h"
#include "../SIII/SIII_PRIV.h"

//---- defines ----------------------------------------------------------------

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SIII_FUNC_RET SIII_HotPlug(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              INT iRetries,
 *              INT iTimeOutSec
 *          )
 *
 * \public
 *
 * \brief   High-level function (blocking) to perform hot-plug procedure.
 *
 * \details The function shall only be called in Sercos phase CP4. In order
 *          that hotplug works, the manual slave configuration needs to be used
 *          rather than using the detected slaves. The more, the hotplug device
 *          needs to be configured before switching to CP1 using
 *          SIII_SetSlaveConfig(). The connection configuration for this device
 *          has to be done before switching to CP3.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       iRetries        Number of retries in case of error
 * \param[in]       iTimeOutSec     Timeout in seconds. The time should be long
 *                                  enough to complete the selected number of
 *                                  retries in case of errors (typ. at least 5s
 *                                  per retry).
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR             for success
 *          - SIII_BLOCKING_ERROR,      when phase handler is already in use
 *          - SIII_SERCOS_PHASE_ERROR,  when not in CP4
 *          - SIII_PARAMETER_ERROR      for illegal function parameter
 *          - SIII_TIMEOUT_ERROR        the timeout has occurred before the
 *                                      hotplug process was completed
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-04-11
 *
 * \version 2013-07-03 (GMy): Change to ensure that CSMD calling interval is
 *                            never smaller than Sercos cycle time
 */
SIII_FUNC_RET SIII_HotPlug
    (
        SIII_INSTANCE_STRUCT *prS3Instance,
        INT iRetries,
        INT iTimeOutSec
    )
{
  SIII_FUNC_RET               eRet                = SIII_NO_ERROR;
  LONG                        lTimeOutMilliSec    = (LONG)iTimeOutSec * 1000;
  SIII_PHASE_STATE_STRUCT     *prPhaseStateStruct;
  SICE_INSTANCE_STRUCT        *prSiceInstance;

  SIII_VERBOSE(3, "SIII_HotPlug()\n");

  if (
      (prS3Instance   ==  NULL)   ||
      (iRetries       <   0)      ||
      (iTimeOutSec    <   0)
    )
  {
    return(SIII_PARAMETER_ERROR);
  }

  // Get shortcut pointers
  prPhaseStateStruct  = &prS3Instance->rPhaseStateStruct;
  prSiceInstance      = &prS3Instance->rSiceInstance;

  if (prPhaseStateStruct->ucPhaseState == (UCHAR) SIII_PHASE_STATE_IDLE)
  {
    prPhaseStateStruct->ucRetries = (UCHAR) iRetries;

    if (prPhaseStateStruct->ucCsmdStateCurr == (UCHAR) SIII_CSMD_STATE_SET_PHASE4)
    {
      SIII_VERBOSE(0, "Starting hot-plug.\n");
      prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_TRANS_HP2_PARA;
    }
    else
    {
      SIII_VERBOSE(0, "Hot-plug is only allowed when in CP4\n");
      return(SIII_SERCOS_PHASE_ERROR);
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
            (ULONG) SIII_PHASE_HANDLER_WAIT_TIME;
      }
      RTOS_SimpleMicroWait(prPhaseStateStruct->ulPhaseHandlerWaitUs);
      lTimeOutMilliSec -= (LONG)(prPhaseStateStruct->ulPhaseHandlerWaitUs / 1000);
    }
    while   (
          (lTimeOutMilliSec > 0)                                                    &&  // Timeout
          (eRet < SIII_END_ERR_CLASS_00000)                                         &&  // Error
          !(
            (prPhaseStateStruct->ucPhaseState == (UCHAR) SIII_PHASE_STATE_IDLE)  &&
            (eRet == SIII_NO_ERROR)                                              &&
            (prPhaseStateStruct->ucCsmdStateCurr == (UCHAR) SIII_CSMD_STATE_SET_PHASE4)
          )                                           // Phase switch successful
        );

    if (lTimeOutMilliSec <= 0)
    {
      SIII_VERBOSE(0, "Timeout due to error during hotplug.\n");
      eRet = SIII_TIMEOUT_ERROR;
    }
    else if (eRet > SIII_END_ERR_CLASS_00000)
    {
      SIII_VERBOSE
          (
            0,
            "Error 0x%X has occurred during hotplug.\n",
            eRet
          );
    }
    else
    {
      SIII_VERBOSE(1, "Hot-plug done.\n");
      eRet = SIII_NO_ERROR;
    }
  }
  else
  {
    // Phase handler currently active, no operation possible.
    SIII_VERBOSE(0, "Phase handler currently active, no operation possible.\n");
    eRet = SIII_BLOCKING_ERROR;
  }

  return(eRet);
}
