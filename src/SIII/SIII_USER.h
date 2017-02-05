/**
 * \file      SIII_USER.h
 *
 * \brief     Sercos III soft master stack - User settings. Additionally,
 *            settings for SICE and CoSeMa have to be made in SICE_USER.h and
 *            CSMD_USER.h.
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
 * \date      2013-11-04
 *
 * \version 2013-07-03 (GMy): Simplified phase handler timing settings by
 *                            merging several values.
 * \version 2015-01-15 (GMy): Replacement for CSMD_USER.h define:
 *                            SIII_SOFT_MASTER_JITTER_NS
 * \version 2015-03-19 (GMy): Changed several static defines to dynamic
 *                            configuration variables
 * \version 2016-07-12 (GMy): Changed SIII_NO_CONN_BYTES to 128 to
 *                            harmonize with Sercans.
 */

// avoid multiple inclusions - open

#ifndef _SIII_USER_
#define _SIII_USER_

//---- includes ---------------------------------------------------------------

//---- defines ----------------------------------------------------------------

/**
 * \def     SIII_VERBOSE_LEVEL
 *
 * \brief   Defines the debug level that results in the number of debug outputs
 *          in the module SIII.
 */
#define SIII_VERBOSE_LEVEL              (0)

/**
 * \def     SIII_INC_PRIO_SETPHASE3
 *
 * \brief   If defined, the priority of the phase handler thread is increased
 *          shortly when switching to CP3 using CSMD_SetPhase3(), as this is
 *          the most critical point of the non-cyclic CoSeMa functions.
 *          However, this typically is not needed. If the define is set,
 *          USER_INT_WAIT_TIME should be set to the Sercos cycle time for
 *          CP3/CP4, or lower, in order to ensure that CSMD_SetPhase3() is
 *          called often enough during that time interval.
 */
#undef SIII_INC_PRIO_SETPHASE3

/**
 * \def     SIII_MAX_HP_DEV
 *
 * \brief   Maximum number of hotplug slaves
 */
#define SIII_MAX_HP_DEV                 (18)

/**
 * \def     SIII_SVC_BUF_SIZE
 *
 * \brief   Size [Bytes] of user SVC data buffer
 */
#define SIII_SVC_BUF_SIZE               (65534)   // Capability to hold a list with maximum??? length.

/**
 * \def     SIII_NO_CONN_BYTES
 *
 * \brief   Size [Bytes] of user data buffer allocated per connection
 */
#define SIII_NO_CONN_BYTES              (128)

/**
 * \def     SIII_TIMEOUT_SVC_VALID_CP1
 *
 * \brief   Timeout for SVC valid flag signaled by slave in CP1 in ms.
 */
#define SIII_TIMEOUT_SVC_VALID_CP1      (200)

/**
 * \def     SIII_MIN_TIME_START_AT
 *
 * \brief   Earliest point of time in ns to start AT transmission. Increasing
 *          it may improve jitter tolerance of the slaves, however, it
 *          indirectly may increase dead time and limits the Sercos cycle time.
 *
 * \note    May cause various CoSeMa errors and slave errors at Sercos phase
 *          startup if value is not possible for present setup!
 */
#define SIII_MIN_TIME_START_AT          (0)

/**
 * \def     SIII_PHASE_HANDLER_WAIT_TIME
 *
 * \brief   Call interval of Sercos phase handler in us. It shall not be lower
 *          than 1000 (1ms). In case the value is lower than the Sercos cycle
 *          time, it will automatically be increased by the phase handler to
 *          the cycle time duration.
 */
#define SIII_PHASE_HANDLER_WAIT_TIME    (5000)

/**
 * \def     SIII_SVC_WAIT_TIME
 *
 * \brief   Call interval of service channel functions in us. One-way data
 *          transfer needs to be completed within this time frame, therefore
 *          this value is critical for large list parameters.
 */
#define SIII_SVC_WAIT_TIME              (100*1000)

/**
 * \def     SIII_SVC_CMD_WAIT_TIME
 *
 * \brief   Time in us that is waited before checking for SVC command
 *          completion.
 */
#define SIII_SVC_CMD_WAIT_TIME          (100*1000)

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

// avoid multiple inclusions - close

#endif
