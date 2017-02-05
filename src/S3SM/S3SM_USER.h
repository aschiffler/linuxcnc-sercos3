/**
 * \file      S3SM_USER.h
 *
 * \brief     User settings for Sercos III soft master.
 *
 * \attention Prototype status! Only for demo purposes! Not to be used in
 *            machines, only in controlled safe environments! Risk of unwanted
 *            machine movement!
 *
 * THIS SOFTWARE IS PROVIDED "AS IS"; WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY;
 * FITNESS FOR A PERTICULAR PURPOSE AND NONINFRINGEMENT. THE AUTHORS OR COPYRIGHT
 * HOLDERS SHALL NOT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE;
 * UNLESS STIPULATED BY MANDATORY LAW.
 *
 * \ingroup   S3SM
 */

// avoid multiple inclusions - open

#ifndef _S3SM_USER
#define _S3SM_USER

//---- includes ---------------------------------------------------------------

//---- defines ----------------------------------------------------------------

/**
 * \def     S3SM_VERBOSE_LEVEL
 *
 * \brief   Defines the debug level that results in the number of debug outputs
 *          in the module S3SM. A value of -1 means no output at all, 0
 *          'normal' outputs and higher values are intended for debugging.
 */
#define S3SM_VERBOSE_LEVEL              (0)

/**
 * \def     S3SM_CYCLE_TIME
 *
 * \brief   Sercos cycle time of soft master for Sercos phases CP3..CP4 in ns.
 */
#define S3SM_CYCLE_TIME                 CSMD_TSCYC_2_MS

/**
 * \def     S3SM_CYCLE_TIME_CP0_CP2
 *
 * \brief   Sercos cycle time of soft master for Sercos phases CP0..CP2 in ns.
 *          In case of SWC mode, this cycle time is only valid for CP1..CP2,
 *          and the value of the macro S3SM_CYCLE_TIME_CP0 is used for CP0.
 */
#define S3SM_CYCLE_TIME_CP0_CP2         CSMD_TSCYC_2_MS

#ifdef CSMD_SWC_EXT
/**
 * \def     S3SM_CYCLE_TIME_CP0
 *
 * \brief   Sercos cycle time of soft master with SWC extension for Sercos
 *          phases CP0 in ns.
 */
#define S3SM_CYCLE_TIME_CP0             CSMD_TSCYC_2_MS
#endif
/**
 * \def     S3SM_SVC_BUSY_TIMEOUT
 *
 * \brief   Sercos service channel busy timeout in ms. This value should be
 *          adjusted to the slowest Sercos slaves used.
 */
#define S3SM_SVC_BUSY_TIMEOUT           (1000)

/**
 * \def     S3SM_ACCEPTED_TEL_LOSSES
 *
 * \brief   Number of accepted telegram losses on master and slave, as well as
 *          for connections.
 */
#define S3SM_ACCEPTED_TEL_LOSSES        (10)

/**
 * \def     S3SM_SIII_TIMING_METHOD
 *
 * \brief   Sercos timing method (=packet order); either CSMD_METHOD_MDT_AT_IPC
 *          or CSMD_METHOD_MDT_IPC_AT
 */
#define S3SM_SIII_TIMING_METHOD         CSMD_METHOD_MDT_AT_IPC
//#define S3SM_SIII_TIMING_METHOD       CSMD_METHOD_MDT_IPC_AT

/**
 * \def     S3SM_MTU
 *
 * \brief   Maximum Ethernet transmission unit (used as CoSeMa parameter) in
 *          Bytes
 */
#define S3SM_MTU                        (1500)

/**
 * \def     S3SM_UC_BANDWIDTH
 *
 * \brief   Time interval for UC channel in ns per Sercos cycle.
 */
#define S3SM_UC_BANDWIDTH               (25*1000)

/**
 * \def     S3SM_COM_VERSION
 *
 * \brief   Sercos communication version to be signaled by soft master.
 */
#define S3SM_COM_VERSION                CSMD_COMVERSION_V1_0

/**
 * \def     S3SM_WATCHDOG_PERIOD
 *
 * \brief   Timeout period for watchdog in Sercos cycles.
 */
#define S3SM_WATCHDOG_PERIOD            (5000)

/**
 * \def     S3SM_DRIVE_OP_MODE
 *
 * \brief   Operation mode of drive in example drive callback to be stored in
 *          S-0-0032.
 */
#define S3SM_DRIVE_OP_MODE              (267)

/**
 * \def     S3SM_SOFT_MASTER_JITTER_NS
 *
 * \brief   Maximum jitter of the soft master, used for calculating the
 *          S-0-1023 of the Sercos slaves. Only used in case the generic mode
 *          (non-NIC-timed) is being used.
 */
#define S3SM_SOFT_MASTER_JITTER_NS      (45*1000)

/**
 * \def     S3SM_DETECT_SLAVE_CONFIG
 *
 * \brief   If TRUE, the detected slave configuration found in Sercos CP0 is
 *          used as configured slave configuration for the higher Sercos
 *          phases. Otherwise, the slave configuration for the higher phases
 *          has to be set manually using SIII_SetSlaveConfig().
 */
#define S3SM_DETECT_SLAVE_CONFIG		(TRUE)

/**
 * \def     S3SM_CLEAR_ERR_ON_STARTUP
 *
 * \brief   If TRUE, the errors on all slaves are automatically cleared
 *          during Sercos phase startup.
 */
#define S3SM_CLEAR_ERR_ON_STARTUP       (TRUE)

/**
 * \def     S3SM_SWITCH_BACK_DELAY
 *
 * \brief   Time in us that is waited before performing a switch-back after
 *          switching off slave power.
 */
#define S3SM_SWITCH_BACK_DELAY          (10*1000)

/**
 * \def     S3SM_CYCLE_PREPARATION_BEFORE_START
 *
 * \brief   If defined, the separate functions SIII_Cycle_Prepare() and
 *          SIII_Cycle_Start() are used rather than SIII_Cyle(). Typically,
 *          this results in improved Sercos packet transmission timing
 *          (less MST jitter).
 *
 */
#undef S3SM_CYCLE_PREPARATION_BEFORE_START

/**
 * \def		S3SM_MODULE_NAME
 *
 * \brief	Name for module
 */
#define S3SM_MODULE_NAME				"s3sm"

/**
 * \def		S3SM_MSG_PFX
 *
 * \brief	Msg prefix
 *
 */
#define S3SM_MSG_PFX					"[S3SM] "

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

#endif /* S3SM_USER_H_ */
