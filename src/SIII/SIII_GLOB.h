/**
 * \file      SIII_GLOB.h
 *
 * \brief     Sercos III soft master stack - Global header
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
 * \version 2013-02-12 (GMy): Added error handling
 * \version 2013-04-11 (GMy): Module re-arrangement
 * \version 2013-05-07 (GMy): Added support for INtime
 * \version 2013-06-06 (GMy): Added support for RTX and Kithara
 * \version 2013-06-21 (GMy): Added support for Windows desktop versions and
 *                            QNX
 * \version 2013-08-05 (GMy): Included SIII_BINCFG.c
 * \version 2013-11-04 (GMy): Bugfix (Hot-Plug called cycle-exact)
 * \version 2014-04-01 (GMy): Added support for CoSeMa 6VRS
 * \version 2014-09-12 (GMy): Improved connection handling
 * \version 2015-03-19 (GMy): Changed several static defines to dynamic
 *                            configuration variables
 * \version 2015-12-02 (GMy): New function SIII_GetSercosStatus()
 * \version 2016-10-27 (AlM): Support for CoSeMa V5 removed.
 */

// avoid multiple inclusions - open

#ifndef _SIII_GLOB_
#define _SIII_GLOB_

#undef SOURCE
#ifdef SOURCE_SIII
    #define SOURCE
#else
    #define SOURCE extern
#endif

//---- includes ---------------------------------------------------------------

#include "../SIII/SIII_USER.h"
#include "../SICE/SICE_GLOB.h"

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

// Version of Sercos soft master / Sercos IP core emulation
#define SIII_VERSION_MAJOR          (1)         /**< Major version of Sercos master stack */
#define SIII_VERSION_MINOR          (0)         /**< Minor version of Sercos master stack */
#define SIII_RELEASE                (0)         /**< Release of Sercos master stack */
#define SIII_STR_TYPE               'V'         /**< Type of version: Test / released version */

/**
 * \def SIII_ALL_DEVICES
 *
 * \brief   This macro may be used with some functions (e.g. SIII_DevicePower)
 *          if all Sercos slave devices should be accessed at once by the
 *          function.
 */
#define SIII_ALL_DEVICES                (999)

/**
 * \def SIII_SLAVE_ENABLE
 *
 * \brief   Control word value to enable Sercos slave.
 */
#define SIII_SLAVE_ENABLE               (0xE000)

/**
 * \def SIII_SLAVE_DISABLE
 *
 * \brief   Control word value to disable Sercos slave.
 */
#define SIII_SLAVE_DISABLE              (0x0000)

/**
 * \def SIII_CYCLE_TIME_ERROR
 *
 * \brief   Erroneous Sercos cycle time.
 */
#define SIII_CYCLE_TIME_ERROR           (0)


/**
 * \def SIII_SIZE_SERCOS_LIST_HEADER
 *
 * \brief   Size of the header of a Sercos list in Bytes.
 */
#define SIII_SIZE_SERCOS_LIST_HEADER    (4)

/**
 * \def SIII_MAX_SIZE_SERCOS_LIST
 *
 * \brief   Maximum net size in Bytes of a Sercos list.
 */
#define SIII_MAX_SIZE_SERCOS_LIST       (( 64 * 1024 ) - SIII_SIZE_SERCOS_LIST_HEADER)

#define SIII_MAX_SLAVES                 (CSMD_MAX_SLAVES)

#define SIII_CYCLIC_BUFFER_SIZE         (CSMD_MAX_TEL * 1500)
#define SIII_MAX_CONN_PER_SLAVE         (CSMD_MAX_CONNECTIONS)
#define SIII_MAX_CONNECTIONS            (CSMD_MAX_GLOB_CONN)

#define CSMD_EIDN   CSMD_IDN_S_0_


//---- type definitions -------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \enum SIII_FUNC_RET
 *
 * \brief   Enumeration of SIII error codes.
 */
typedef enum
{
    /* --------------------------------------------------------- */
    /* no error: states, warning codes  (error_class 0x00000nnn) */
    /* --------------------------------------------------------- */
    SIII_NO_ERROR               = (0x00000000),     /**< 0x00 Function successfully completed */
    SIII_FUNCTION_IN_PROCESS,                       /**< 0x01 Function processing still active */
    SIII_END_ERR_CLASS_00000,                       /**< End marker for error class 0x00000 nnn */

    // Between codes 0x00000001 and 0x001FFFFF, CoSeMa and SICE warnings are inherited

    /* --------------------------------------------------------- */
    /* warning codes                    (error_class 0x0021nnnn) */
    /* --------------------------------------------------------- */
    SIII_WARNING                = (0x00210000),     /**< 0x00 General: error during function execution */
    SIII_NO_EFFECT,                                 /**< 0x01 Function has no effect due to inconsistent configuration */
    SIII_END_ERR_CLASS_0021,                        /**< End marker for error class 0x00021 nnn */

    /* --------------------------------------------------------- */
    /* system error codes               (error_class 0x0022nnnn) */
    /* --------------------------------------------------------- */
    SIII_SYSTEM_ERROR           = (0x00220000),     /**< 0x00 General: error during function execution */
    SIII_MEM_ERROR,                                 /**< 0x01 Error when trying to allocate memory */
    SIII_STATE_ERROR,                               /**< 0x02 Non-existing state in function requested */
    SIII_SEMAPHORE_ERROR,                           /**< 0x03 Error with semaphore */
    SIII_BLOCKING_ERROR,                            /**< 0x04 Requested resource not available */
    SIII_PARAMETER_ERROR,                           /**< 0x05 Illegal function parameter */
    SIII_DEVICE_IDX_ERROR,                          /**< 0x06 Invalid device index */
    SIII_BUFFER_ERROR,                              /**< 0x07 Buffer to small */
    SIII_TIMEOUT_ERROR,                             /**< 0x08 A pre-defined timeout has occured */
    SIII_UCC_ERROR,                                 /**< 0x09 UCC error has occured */
    SIII_END_ERR_CLASS_0022,                        /**< End marker for error class 0x00022 nnn */

    /* --------------------------------------------------------- */
    /* Sercos error codes               (error_class 0x0023nnnn) */
    /* --------------------------------------------------------- */
    SIII_SERCOS_ERROR           = (0x00230000),     /**< 0x00 General: Sercos error */
    SIII_SERCOS_PHASE_ERROR,                        /**< 0x01 Requested Sercos phase unknown */
    SIII_SERCOS_CYCLE_TIME_INVALID,                 /**< 0x02 Invalid Sercos cycle time */
    SIII_SERCOS_SLAVE_VALID_NOT_SET,                /**< 0x03 Slave valid not set in device status */
    SIII_END_ERR_CLASS_0023,                        /**< End marker for error class 0x00023 nnn */

    /* --------------------------------------------------------- */
    /* Configuration error codes        (error_class 0x0024nnnn) */
    /* --------------------------------------------------------- */
    SIII_CONFIG_ERROR           = (0x00240000),     /**< 0x00 General: configuration error */
    SIII_NOT_ALLOWED_IN_CURRENT_CP,                 /**< 0x01 Action not allowed in current Sercos phase */
    SIII_END_ERR_CLASS_0024,                        /**< End marker for error class 0x00024 nnn */

    /* --------------------------------------------------------- */
    /* SVC error codes                  (error_class 0x0025nnnn) */
    /* --------------------------------------------------------- */
    SIII_SVC_ERROR              = (0x00250000),     /**< 0x00 General: SVC error */
    SIII_SVC_PHASE_ERROR,                           /**< 0x01 Sercos phase lower than CP2 */
    SIII_SVC_NO_CMD_PAR_ERROR,                      /**< 0x02 Trying to perform a command on a non-command parameter */
    SIII_END_ERR_CLASS_0025                         /**< End marker for error class 0x00025 nnn */

} SIII_FUNC_RET;


/**
 * \union SIII_DEVICE_INFO_STRUCT
 *
 * \brief   structure containing device information (currently FSP only)
 */
typedef struct SIII_DEVICE_INFO_STR
{
  USHORT  usFSP;         /* function specific profile (Drive, I/O) */
} SIII_DEVICE_INFO_STRUCT;

/**
 * \struct SIII_CONN_INFO_STRUCT
 *
 * \brief   structure containing internal connection information
 */
typedef struct SIII_CONN_INFO_STR
{
  USHORT  usOffset;         /* offset in local cyclic MDT/AT buffer [bytes] */
  USHORT  usLength;         /* connection length */
  USHORT  usConnIdx;        /* connection index in CoSeMa configuration structure */
} SIII_CONN_INFO_STRUCT;

/**
 * \struct SIII_PHASE_STATE_STRUCT
 *
 * \brief   Data structure for state of Sercos phase handler.
 */
typedef struct SIII_PHASE_STATE_STR
{
  UCHAR               ucPhaseState;           /**< Phase handler state */
  UCHAR               ucCsmdStateNew;         /**< Requested CoSeMa / Sercos state */
  UCHAR               ucCsmdStateCurr;        /**< Current CoSeMa / Sercos state */
  UCHAR               ucRetries;              /**< Number of retries for phase handler */
  ULONG               ulCsmdSleepCnt;         /**< No. of us to wait after last CoSeMa call */
  BOOL                boSwitchBackCP;         /**< Is being switched back from higher phase? */
  ULONG               ulPhaseHandlerWaitUs;   /**< Waiting time in us between calls of phase handler */
} SIII_PHASE_STATE_STRUCT;

/**
 * \struct SIII_CYCLIC_COMM_CTRL_STRUCT
 *
 * \brief   Control flags for cyclic Sercos communication.
 */
typedef struct SIII_CYCLIC_COMM_CTRL_STR
{
  BOOL                boCallReadAT;               /**< Should ReadAT be called cyclically?*/
  BOOL                boCallWriteMDT;             /**< Should WriteMDT be called cyclically?*/
  BOOL                boCallTxRxSoftCont;         /**< Should TxRxSoftCont be called cyclically?*/
  BOOL                boAppDataValid;             /**< Is application data valid?*/
  BOOL                boPowerOn;                  /**< Is slave power switched on?*/
  BOOL                boCyclicDataError;          /**< Has cyclic data error occured?*/
  BOOL                boHotplugCyclicPhase;       /**< Is cyclic hotplug phase active?*/
  CSMD_FUNC_RET       eCyclicCsmdError;           /**< CoSeMa error code from cyclic function*/
  BOOL                aboCycDataValid[SIII_MAX_SLAVES];
                                                  /**< Is cyclic command data from app valid?*/
} SIII_CYCLIC_COMM_CTRL_STRUCT;

/**
 * \enum SIII_SVC_ACCESS_MODE
 *
 * \brief   Enumeration of Sercos III service channel access codes.
 */
typedef enum
{
  SIII_SVC_READ               = (0x00000000),
  SIII_SVC_WRITE,
  SIII_SVC_CMD
} SIII_SVC_ACCESS_MODE;

/**
 * \struct SIII_SVC_RESULT_STRUCT
 *
 * \brief   Result structure for SVC transaction
 */
typedef struct SIII_SVC_RESULT_STR
{
  SIII_FUNC_RET       eFuncRet;                           /**< Error code of SVC transaction*/
  USHORT              ausSVCData[SIII_SVC_BUF_SIZE/2];    /**< SVC data buffer */
} SIII_SVC_RESULT_STRUCT;

/**
 * \enum SIII_PHASE
 *
 * \brief   Enumeration Sercos phases, e.g. used for Sercos phase switch.
 */
typedef enum
{
  SIII_PHASE_NRT                  = -1,       /**< Sercos NRT mode */
  SIII_PHASE_CP0                  = 0,        /**< Sercos phase CP0 */
  SIII_PHASE_CP1                  = 1,        /**< Sercos phase CP1 */
  SIII_PHASE_CP2                  = 2,        /**< Sercos phase CP2 */
  SIII_PHASE_CP3                  = 3,        /**< Sercos phase CP3 */
  SIII_PHASE_CP4                  = 4,        /**< Sercos phase CP4 */
  SIII_PHASE_CP2_AFTER_PARS_XMIT  = 10,       /**< Sercos phase CP2, including transmission
                                                   of slave parameters */
  SIII_PHASE_NEXT                 = 100,      /**< Only for switching command for single step */
  SIII_PHASE_CURR                 = 200       /**< Only for getting current Sercos cycle time */
} SIII_PHASE;

/**
 * \struct SIII_SERCOS_STATUS
 *
 * \brief   Struct that represents the Sercos network status.
 */
typedef struct
{
  SIII_PHASE          ePhase;        /**< Current Sercos phase. See definition
                                          of SIII_PHASE for semantics*/
  USHORT              usTopology;    /**< Current Sercos network topology;
                                          one of the following values:
                                          - CSMD_NO_LINK                 (0)
                                          - CSMD_TOPOLOGY_LINE_P1        (1)
                                          - CSMD_TOPOLOGY_LINE_P2        (2)
                                          - CSMD_TOPOLOGY_DOUBLE_LINE    (3)
                                          - CSMD_TOPOLOGY_RING           (4)
                                          - CSMD_TOPOLOGY_DEFECT_RING    (8)*/

} SIII_SERCOS_STATUS;

/**
 * \struct SIII_COMM_PARS_STRUCT
 *
 * \brief   Structure for Sercos III communication parameters
 */
typedef struct SIII_COMM_PARS_STR
{
#ifdef CSMD_SWC_EXT
  ULONG                   ulCycleTimeCP0;         /**< Sercos cycle time for CP0 for SWC */
#endif
  ULONG                   ulCycleTimeCP0CP2;      /**< Sercos cycle time for CP0..CP2 */
  ULONG                   ulCycleTime;            /**< Sercos cycle time for CP3..CP4 */
  USHORT                  usSVCBusyTimeout;       /**< Service channel busy timeout */
  USHORT                  usAccTelLosses;         /**< Number of accepted telegram losses */
  USHORT                  usS3TimingMethod;       /**< Sercos III timing method */
  USHORT                  usMTU;                  /**< MTU in Bytes */
  ULONG                   ulUCCBandwidth;         /**< UCC interval size in ns per Sercos cycle */
  CSMD_COM_VERSION        eComVersion;            /**< CoSeMa communication version */
  ULONG                   ulSoftMasterJitterNs;   /**< Maximum jitter of the soft master*/
  BOOL                    boDetectSlaveConfig;    /**< Found slaves are automatically configured */

  BOOL                    boClrErrOnStartup;      /**< If defined, the errors on all slaves are
                                                       automatically cleared during Sercos phase startup.*/
  ULONG                   ulSwitchBackDelay;      /**< Time in us that is waited before
                                                       performing a switch-back after switching
                                                       off slave power. */
} SIII_COMM_PARS_STRUCT;

/**
 * \brief   Function pointer for Sercos device connection configuration
 *          function
 */
typedef VOID (*FP_APP_CONN_CONFIG)
    (
      VOID*,      /**< SIII_INSTANCE_STRUCT*, casted to VOID to
                       avoid circular dependency */
      USHORT,     /**< Device index */
      USHORT*,    /**< *pusMasterConnOffset */
      USHORT      /**< usRTDataConnIdx */
    );

/**
 * \brief   Function pointer for Sercos device cyclic function
 */
typedef VOID (*FP_APP_CYCLIC)
    (
      VOID*,      /**< SIII_INSTANCE_STRUCT*, casted to VOID to
                       avoid circular dependency */
      USHORT      /**< Device index */
    );

/**
 * \brief   Function pointer for global device connection configuration
 *          function
 */
typedef VOID (*FP_APP_CONN_CONFIG_GLOB)
    (
      VOID*           /**< SIII_INSTANCE_STRUCT*, casted to VOID to
                           avoid circular dependency */
    );

/**
 * \brief   Function pointer for global cyclic function
 */
typedef VOID (*FP_APP_CYCLIC_GLOB)
    (
      VOID*           /**< SIII_INSTANCE_STRUCT*, casted to VOID to
                           avoid circular dependency */
    );

/**
 * \struct SIII_ACTIVE_IDX_LIST
 *
 * \brief   List of active connection indices
 */
typedef struct
{
  USHORT usConnIdx;
  USHORT usConfigIdx;     /* Is set, but currently not used */

} SIII_ACTIVE_IDX_LIST;

/**
 * \struct SIII_ACTIVE_PRODUCER
 *
 * \brief   List of active producers
 */
typedef struct
{
  /* Number of active producer connections */
  USHORT  usNbrActProd;
  /* List without gaps of all by the master produced connections */
  /* Note: Change size of array to maximum number of master connections! */
  SIII_ACTIVE_IDX_LIST arActIdxList[SIII_MAX_SLAVES + 1];

} SIII_ACTIVE_PRODUCER;

/**
 * \struct SIII_ACTIVE_CONSUMER
 *
 * \brief   List of active consumers
 */
typedef struct
{
  /* Number of active consumer connections */
  USHORT  usNbrActCons;
  /* List without gaps of all by the master consumed connections */
  /* Note: Change size of array to maximum number of master connections! */
  SIII_ACTIVE_IDX_LIST arActIdxList[SIII_MAX_SLAVES + 1];
} SIII_ACTIVE_CONSUMER;

/**
 * \struct SIII_INSTANCE_STRUCT
 *
 * \brief Data structure for SIII instance
 */
typedef struct
{
  // CoSeMa variables
  CSMD_INSTANCE                   rCosemaInstance;                    /**< CoSeMa instance structure */
  CSMD_SVCH_MACRO_STRUCT          arCosemaSvcMacro[SIII_MAX_SLAVES];  /**< CoSeMa service channel control structure */
  CSMD_FUNC_STATE                 rCosemaFuncState;                   /**< CoSeMa function state */
  CSMD_INIT_POINTER               rCosemaSercosInitPtr;               /**< CoSeMa Sercos initialization structure */
  CSMD_TEL_BUFFER                 rCosemaTelBuffer;                   /**< CoSeMa telegram buffer */
  USHORT*                         pusCosemaRecDevList;                /**< List of Sercos slaves discovered by CoSeMa */
#ifdef CSMD_HOTPLUG
  USHORT                          ausCosemaHPDevAddList[SIII_MAX_HP_DEV];
                                                                        /**< Hot-plug slave device list */
#endif

  // SICE variables
  SICE_INSTANCE_STRUCT            rSiceInstance;                      /**< SICE instance structure */

  // SIII control variables
  INT                             iInstanceNo;                        /**< Instance number of SIII */
  SIII_COMM_PARS_STRUCT           rS3Pars;                            /**< Sercos III parameters */
  SIII_PHASE_STATE_STRUCT         rPhaseStateStruct;                  /**< Struct to control Sercos phase state machine */
  SIII_CYCLIC_COMM_CTRL_STRUCT    rCyclicCommCtrl;                    /**< Control structure for cyclic communication */
  USHORT                          usDevCnt;                           /**< Number of detected Sercos slaves*/
  USHORT                          ausOperDevList[2*SIII_MAX_SLAVES + 4];
                                                                      /**< Detected Sercos slave device list */
  BOOL                            boSetSlaveConfig;                   /**< States whether Sercos slave configuration has already been performed */

  SIII_DEVICE_INFO_STRUCT         arDeviceInfo[SIII_MAX_SLAVES];      /**< structure containing device info */

  // Cyclic data buffer
  UCHAR                           aucCyclicMDTBuffer[SIII_CYCLIC_BUFFER_SIZE]; /**< Buffer for cyclic MDT data */
  UCHAR                           aucCyclicATBuffer[SIII_CYCLIC_BUFFER_SIZE];  /**< Buffer for cyclic AT data */
  SIII_CONN_INFO_STRUCT           arConnInfoMDT[SIII_MAX_SLAVES]
                                               [SIII_MAX_CONN_PER_SLAVE];   /**< Info struct for MDT connections (offset + length) */
  SIII_CONN_INFO_STRUCT           arConnInfoAT [SIII_MAX_SLAVES]
                                               [SIII_MAX_CONN_PER_SLAVE];   /**< Info Struct for AT connections (offset + length) */

  // SVC handling
  CSMD_SVCH_MACRO_STRUCT          rMySvcMacro;                        /**< SVC macro struct for user access */
  SIII_SVC_ACCESS_MODE            rMySVCAccessMode;                   /**< SVC access mode (see SIII_SVC_ACCESS_MODE) */
  SIII_SVC_RESULT_STRUCT          rMySVCResult;                       /**< SVC result struct for user access */
  RTOS_SEMAPHORE                  semSVCBlock;                        /**< Semaphore that is used to block SVC access when transfer already running */
  RTOS_SEMAPHORE                  semSVCStart;                        /**< Semaphore that starts SVC transfer when posted */
  RTOS_THREAD                     rSVCThread;                         /**< SVC handler thread */

  // Pointers to application-specific functions for Sercos slaves
  FP_APP_CONN_CONFIG              afpAppConnConfig[SIII_MAX_SLAVES];  /**< Device connection configuration function*/
  FP_APP_CYCLIC                   afpAppCyclic[SIII_MAX_SLAVES];      /**< Cyclic device function */

  // Pointers to application-specific functions for all slaves
  FP_APP_CONN_CONFIG_GLOB         fpAppConnConfigGlob;                /**< Global device connection configuration function*/
  FP_APP_CYCLIC_GLOB              fpAppCyclicGlob;                    /**< Global cyclic function */

  SIII_ACTIVE_PRODUCER            rSERC_Act_Prod;                     /**< Active producer connections */
  SIII_ACTIVE_CONSUMER            rSERC_Act_Cons;                     /**< Active consumer connections */

} SIII_INSTANCE_STRUCT;


/**
 * \enum SIII_PHASE_HANDLER_STATE
 *
 * \brief   States for user interface state machine
 */
typedef enum
{
  SIII_PHASE_STATE_IDLE = 0,
  SIII_PHASE_STATE_STARTED,
  SIII_PHASE_STATE_RUNNING
} SIII_PHASE_HANDLER_STATE;

/**
 * \enum SIII_CSMD_STATE
 *
 * \brief States for CoSeMa startup state machine, used in phase handler
 */
typedef enum
{
  SIII_CSMD_STATE_IDLE = 0,
  SIII_CSMD_STATE_INITIALIZE,
  SIII_CSMD_STATE_INIT_HARDWARE,
//  SIII_CSMD_STATE_INITIALIZE_2,
  SIII_CSMD_STATE_SET_COMM_PARAM,
  SIII_CSMD_STATE_SET_NRT_MODE,
  SIII_CSMD_STATE_SET_PHASE0,
  SIII_CSMD_STATE_INIT_CONFIG_STRUCT,
  SIII_CSMD_STATE_SET_PHASE1,
  SIII_CSMD_STATE_SET_PHASE2,
  SIII_CSMD_STATE_CHECK_VERSION,
  SIII_CSMD_STATE_GET_TIMING_DATA,
  SIII_CSMD_STATE_CALCULATE_TIMING,
  SIII_CSMD_STATE_TRANSMIT_TIMING,
  SIII_CSMD_STATE_SET_PHASE3,
  SIII_STATE_FILL_CONN_INFO,
  SIII_CSMD_STATE_SET_PHASE4,
  SIII_CSMD_STATE_HOTPLUG,
  SIII_CSMD_STATE_TRANS_HP2_PARA,
  SIII_CSMD_STATE_RING_RECOVERY
} SIII_CSMD_STATE;

/**
 * \enum SIII_DEV_DATA
 *
 * \brief   Index codes for Sercos slave device MDT and AT data
 */
typedef enum
{
  SIII_DEV_MDT_DATA = 0,
  SIII_DEV_AT_DATA
} SIII_DEV_DATA;


/* #defines for function specific profiles */
#define SIII_S_1302_0_1_FSP_IO            (1)
#define SIII_S_1302_0_1_FSP_DRIVE         (2)
#define SIII_S_1302_0_1_FSP_ENC           (3)


//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

// SIII_INIT.c

SOURCE SIII_FUNC_RET SIII_Init
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      INT iInstanceNo,
      SIII_COMM_PARS_STRUCT *prS3Pars
    );

SOURCE SIII_FUNC_RET SIII_Close
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

// SIII_CONF.c

SOURCE INT SIII_GetNoOfSlaves
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

SOURCE SIII_PHASE SIII_GetSercosPhase
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

SOURCE USHORT SIII_GetSercosTopology
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

SOURCE SIII_FUNC_RET SIII_CheckCycleTime
    (
      ULONG ulNewCycleTime,
      SIII_PHASE ePhase
    );

SOURCE SIII_FUNC_RET SIII_SetSlaveConfig
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT *pusOperDevList
    );

SOURCE USHORT* SIII_GetSlaveConfig
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      BOOL boDetectedSlaves
    );

SOURCE SIII_FUNC_RET SIII_SetWatchdog
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT usWDMode,
      USHORT usCycles
    );

SOURCE SIII_FUNC_RET SIII_DisableWatchdog
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

SOURCE SIII_FUNC_RET SIII_ClearDeviceCallbacks
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

SOURCE SIII_FUNC_RET SIII_SetDeviceCallback
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT usDevIdx,
      VOID* fpAppConnConfig,
      VOID* fpAppCyclic
    );

SOURCE SIII_FUNC_RET SIII_SetSercosCycleTime
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      SIII_PHASE ePhase,
      ULONG ulCycleTime
    );

SOURCE ULONG SIII_GetSercosCycleTime
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      SIII_PHASE ePhase
    );

SOURCE SIII_FUNC_RET SIII_ActivateSercosTime
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      ULONG ulSeconds,
      ULONG ulNanos
    );

SOURCE SIII_FUNC_RET SIII_ReadSercosTime
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      ULONG *pulSeconds,
      ULONG *pulNanos
    );

SOURCE SIII_FUNC_RET SIII_GetSercosStatus
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      SIII_SERCOS_STATUS   *prSercosStatus
    );

SOURCE SIII_FUNC_RET SIII_GetControlIdx
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT                usDevIdx,
      USHORT*               pusConnIdx
    );

SOURCE SIII_FUNC_RET SIII_SetSercosTimingMode
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT usNewMethod
    );

// SIII_BINCFG.c

SOURCE SIII_FUNC_RET SIII_WriteBinConnCfg
    (
        SIII_INSTANCE_STRUCT *prS3Instance,
        UCHAR*  pucBuffer,
        ULONG   ulBufSize,
        BOOL boConnNumGen,
        BOOL boSlaveInstGen
    );

SOURCE SIII_FUNC_RET SIII_ReadBinConnCfg
    (
        SIII_INSTANCE_STRUCT *prS3Instance,
        UCHAR*  pucBuffer,
        ULONG   ulBufSize,
        USHORT usCFGbinVersion,
        USHORT usAppID,
        BOOL boAppID_Pos,
        ULONG* ulReadBytes
    );

// SIII_CYCLIC.c

SOURCE SIII_FUNC_RET SIII_Cycle
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      ULONG *pulCycleTime
    );

SOURCE SIII_FUNC_RET SIII_Cycle_Prepare
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

SOURCE SIII_FUNC_RET SIII_Cycle_Start
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      ULONG *pulCycleTime
    );

SOURCE SIII_FUNC_RET SIII_DevicePower
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT usDevIdx,
      BOOL boPower
    );

SOURCE SIII_FUNC_RET SIII_GetDeviceStatus
    (
        SIII_INSTANCE_STRUCT *prS3Instance,
        USHORT  usDevIdx,
        USHORT *pusStatus
    );

SOURCE SIII_FUNC_RET SIII_GetDeviceCyclicDataPtr
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT   usDevIdx,
      SIII_DEV_DATA eMDTorAT,
      USHORT   usConnNr,
      USHORT** ppucBuffer,
      USHORT*  pusLength
    );

SOURCE SIII_FUNC_RET SIII_SetCyclicDataValid
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT usDevIdx
    );

SOURCE SIII_FUNC_RET SIII_ClearCyclicDataValid
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

// SIII_PHASE.c

SOURCE SIII_FUNC_RET SIII_PhaseSwitch
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      SIII_PHASE eTargetPhase,
      INT iRetries,
      INT iTimeOutSec
    );

// SIII_HOTPLUG.c

SOURCE SIII_FUNC_RET SIII_HotPlug
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      INT iRetries,
      INT iTimeOutSec
    );

// SIII_REDUNDANCY.c

SOURCE SIII_FUNC_RET SIII_RecoverRing
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      INT iRetries,
      INT iTimeOutSec
    );

SOURCE SIII_FUNC_RET SIII_UpdateTopology
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

// SIII_SVC.c

SOURCE SIII_FUNC_RET SIII_SVCWrite
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT  usDevIdx,
      BOOL  boIsStdPar,
      USHORT  usIdn,
      USHORT  usSI,
      USHORT  usSE,
      USHORT  usElem,
      USHORT* pusValue,
      USHORT  usLen
    );

SOURCE SIII_FUNC_RET SIII_SVCRead
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT  usDevIdx,
      BOOL  boIsStdPar,
      USHORT  usIdn,
      USHORT  usSI,
      USHORT  usSE,
      USHORT  usElem,
      USHORT* pusValue,
      USHORT  usLen
    );

SOURCE SIII_FUNC_RET SIII_SVCCmd
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT  usDevIdx,
      BOOL  boIsStdPar,
      USHORT  usIdn,
      USHORT  usSI,
      USHORT  usSE
    );

SOURCE SIII_FUNC_RET SIII_SVCClearErrors
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT  usDevIdx
    );

// SIII_UCC.c

SOURCE SIII_FUNC_RET SIII_UccHandling
    (
      SIII_INSTANCE_STRUCT    *prS3Instance
    );

#ifdef __cplusplus
}
#endif

// avoid multiple inclusions - close

#endif
