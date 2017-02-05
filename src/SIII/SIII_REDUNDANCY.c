/**
 * \file      SIII_REDUNDANCY.c
 *
 * \brief     Sercos III soft master stack - Redundancy functions
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
 * \copyright Copyright Bosch Rexroth AG, 2015-2016
 *
 * \date      2015-11-24
 *
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
 * \fn SIII_FUNC_RET SIII_RecoverRing(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              INT iRetries,
 *              INT iTimeOutSec
 *          )
 *
 * \public
 *
 * \brief   High-level blocking function to perform a Sercos ring recovery.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       iRetries        Number of retries in case of error
 * \param[in]       iTimeOutSec     Timeout in seconds. The time should be long
 *                                  enough to complete the selected number of
 *                                  retries (Typ. at least 15s per retry).
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR             for success
 *          - SIII_TIMEOUT_ERROR        the timeout has occurred before the
 *                                      target phase was reached
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2015-11-24
 *
 */
SIII_FUNC_RET SIII_RecoverRing
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      INT iRetries,
      INT iTimeOutSec
    )
{
  SIII_FUNC_RET           eRet                = SIII_NO_ERROR;
  LONG                    lTimeOutMilliSec    = iTimeOutSec * 1000;
  SIII_PHASE_STATE_STRUCT *prPhaseStateStruct;

  SIII_VERBOSE(3, "SIII_RecoverRing()\n");

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
  prPhaseStateStruct  = &prS3Instance->rPhaseStateStruct;

  if (prPhaseStateStruct->ucPhaseState == (UCHAR) SIII_PHASE_STATE_IDLE)
  {
    prPhaseStateStruct->ucRetries = (UCHAR) iRetries;

    if (prPhaseStateStruct->ucCsmdStateCurr > (UCHAR) SIII_CSMD_STATE_SET_PHASE0)
    {
      SIII_VERBOSE(0, "Starting ring recovery...\n");
      prPhaseStateStruct->ucCsmdStateNew = (UCHAR) SIII_CSMD_STATE_RING_RECOVERY;
    }
    else
    {
      SIII_VERBOSE(0, "Ring recovery is only allowed in CP1 or higher\n");
      return(SIII_SERCOS_PHASE_ERROR);
    }

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
        prPhaseStateStruct->ulPhaseHandlerWaitUs = (ULONG) SIII_PHASE_HANDLER_WAIT_TIME;
      }
      RTOS_SimpleMicroWait(prPhaseStateStruct->ulPhaseHandlerWaitUs);
      lTimeOutMilliSec -= (LONG)(prPhaseStateStruct->ulPhaseHandlerWaitUs / 1000);
    }
    while   (
          (lTimeOutMilliSec > 0)                                                    &&  // Timeout
          (eRet < SIII_END_ERR_CLASS_00000)                                         &&  // Error
          !(
            (prPhaseStateStruct->ucPhaseState == (UCHAR) SIII_PHASE_STATE_IDLE)  &&
            (eRet == SIII_NO_ERROR)
          )
        );

    if (lTimeOutMilliSec <= 0)
    {
      SIII_VERBOSE(0, "Timeout due to error during ring recovery.\n");
      eRet = SIII_TIMEOUT_ERROR;
    }
    else if (eRet > SIII_END_ERR_CLASS_00000)
    {
      SIII_VERBOSE
          (
            0,
            "Error 0x%X has occurred during ring recovery.\n",
            eRet
          );
    }
    else
    {
      SIII_VERBOSE(1, "Ring recovery done.\n");
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

/**
 * \fn SIII_FUNC_RET SIII_UpdateTopology(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \public
 *
 * \brief   Preliminary function to update Sercos network topology
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR             for success
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2015-12-08
 *
 */
SIII_FUNC_RET SIII_UpdateTopology
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  switch(prS3Instance->rCosemaInstance.usCSMD_Topology) {
    case CSMD_TOPOLOGY_LINE_P1:
      SIII_VERBOSE(0, "Sercos network topology change to line at P1\n");
      break;
    case CSMD_TOPOLOGY_LINE_P2:
      SIII_VERBOSE(0, "Sercos network topology change to line at P2\n");
      break;
    case CSMD_TOPOLOGY_BROKEN_RING:
      SIII_VERBOSE(0, "Sercos network topology change to double line resp. broken ring\n");
      break;
    case CSMD_TOPOLOGY_RING:
      SIII_VERBOSE(0, "Sercos network topology change to ring\n");
      break;
    case CSMD_TOPOLOGY_DEFECT_RING:
      SIII_VERBOSE(0, "Sercos network topology change to defect ring\n");
      break;
    default:
      SIII_VERBOSE(0, "Sercos network topology change to illegal value (0x%X)\n",
          prS3Instance->rCosemaInstance.usCSMD_Topology);
      break;
  }

  return(SIII_NO_ERROR);
}

