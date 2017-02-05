/**
 * \file      RTLX_S3SM_GLOB.h
 *
 * \brief     Global header for real-time operating system abstraction layer for
 *            Linux RT-Preempt, specific functionality for Sercos soft master
 *            (S3SM)
 *
 * \note      Some functions (specifically socket functions) are not yet capable
 *            of dealing with multiple instances in contrast to the SICE module!
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
 * \ingroup   RTLX_S3SM
 *
 */

// avoid multiple inclusions - open

#ifndef _RTLX_S3SM_GLOB
#define _RTLX_S3SM_GLOB

#undef SOURCE
#ifdef SOURCE_RTLX
    #define SOURCE
#else
    #define SOURCE extern
#endif

//---- includes ---------------------------------------------------------------

/*lint -save -w0 */
#include <semaphore.h>
/*lint -restore */

#include "../GLOB/GLOB_DEFS.h"
#include "../GLOB/GLOB_TYPE.h"
#include "../CSMD/CSMD_GLOB.h"
#include "../RTLX/RTLX_S3SM_USER.h"

//---- defines ----------------------------------------------------------------

/**
 * \def     RTOS_THREAD_PRIORITY_STD
 *
 * \brief   Defines standard process priority to be used with present OS
 */
#define RTOS_THREAD_PRIORITY_STD            (30)
#define RTOS_THREAD_PRIORITY_SVC	(20)	
/**
 * \def     RTOS_RT_PRIO
 *
 * \brief   Defines base priority for real-time thread
 */
#define RTOS_RT_PRIO                        (98)

// Macros for RTOS timing generation modes to be used for generating cyclic
// Sercos timing. The selection of these modes may be done with the macro
// RTOS_TIMING_MODE in RTxx_USER.h

/**
 * \def     RTOS_TIMING_TIMER
 *
 * \brief   In this mode, a cyclic system timer with a callback function is
 *          used for cyclic Sercos timing.
 */
#define RTOS_TIMING_TIMER                   (0)

/**
 * \def     RTOS_TIMING_REL_NANOSLEEP
 *
 * \brief   In this mode, a time-relative waiting function is used for cyclic
 *          Sercos timing.
 */
#define RTOS_TIMING_REL_NANOSLEEP           (1)

/**
 * \def     RTOS_TIMING_REL_NANOSLEEP_COMP
 *
 * \brief   In this mode, a time-relative waiting function is used for cyclic
 *          Sercos timing. The waiting the is cyclically adapted to the system
 *          clock.
 */
#define RTOS_TIMING_REL_NANOSLEEP_COMP      (2)

/**
 * \def     RTOS_TIMING_ABS_NANOSLEEP
 *
 * \brief   In this mode, a time-absolute waiting function is used for cyclic
 *          Sercos timing.
 */
#define RTOS_TIMING_ABS_NANOSLEEP           (3)

/**
 * \def     RTOS_TIMING_NIC
 *
 * \brief   In this mode, the timer integrated in the network controller is
 *          used to provide the soft master timing. This feature is only
 *          offered is some NICs
 */
#define RTOS_TIMING_NIC                     (4)

//---- type definitions -------------------------------------------------------

#define RTOS_TIMERSPEC RTLX_TIMERSPEC

/**
 * \struct  RTOS_TIMERSPEC
 *
 * \brief   Abstract type for timer data structure, to be re-defined when using
 *          a different RTOS
 */
typedef struct itimerspec RTLX_TIMERSPEC;

/**
 * \struct  RTOS_NIC_TIMED_SERCOS_PACKETS
 *
 * \brief   Structure for holding either MDT or AT packet transmission
 *          information for NIC-timed transmission mode.
 */
typedef struct
{
  USHORT usNum;                          // Number of packets
  UCHAR* aapucPacket[CSMD_MAX_TEL][2];   // Packet pointer for each port
  USHORT ausLen[CSMD_MAX_TEL];           // Length of packets in bytes
  ULONG ulOffsetNs;                      // Timing offset in ns
} RTOS_NIC_TIMED_SERCOS_PACKETS;

/**
 * \struct  RTOS_NIC_TIMED_UCC_PACKETS
 *
 * \brief   Structure for holding UCC packet transmission information for
 *          NIC-timed transmission mode.
 */
typedef struct
{
  USHORT usNum;                             // Number of packets
  UCHAR* apucPacket[RTOS_UCC_MAX_PACKETS];  // Packet pointer
  USHORT ausLen[RTOS_UCC_MAX_PACKETS];      // Length of packets in bytes
  UCHAR aucPort[RTOS_UCC_MAX_PACKETS];      // Port
  ULONG ulOffsetNs;                         // Timing offset in ns
} RTOS_NIC_TIMED_UCC_PACKETS;

/**
 * \struct  RTOS_NIC_TIMED_PACKET_STRUCT
 *
 * \brief   Data structure for packet transmission using NIC-timed transmission
 *          mode.
 */
typedef struct
{
  // Sercos MDT packets
  RTOS_NIC_TIMED_SERCOS_PACKETS rMDT;

  // Sercos AT packets
  RTOS_NIC_TIMED_SERCOS_PACKETS rAT;

  // UCC packets
  RTOS_NIC_TIMED_UCC_PACKETS rUCC;

  BOOL boRedundancy;

} RTOS_NIC_TIMED_PACKET_STRUCT;

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Socket functions (RTLX_S3SM_SOCK.c)

#define         RTOS_OpenTxSocket           RTLX_OpenTxSocket
#define         RTOS_OpenRxSocket           RTLX_OpenRxSocket
#define         RTOS_TxPacket               RTLX_TxPacket
#define         RTOS_CloseRxSocket          RTLX_CloseRxSocket
#define         RTOS_RxPacket               RTLX_RxPacket
#define         RTOS_CloseTxSocket          RTLX_CloseTxSocket

SOURCE INT RTLX_OpenTxSocket
    (
      INT iInstanceNo,
      BOOL boRedundancy,
      UCHAR* pucMAC
    );

SOURCE INT RTLX_OpenRxSocket
    (
      INT iInstanceNo,
      BOOL boRedundancy
    );

SOURCE INT RTLX_TxPacket
    (
      INT iInstanceNo,
      INT iPort,
      UCHAR* pucFrame,
      USHORT usLen,
      USHORT usIFG
    );

SOURCE VOID RTLX_CloseRxSocket
    (
      INT iInstanceNo,
      BOOL boRedundancy
    );

SOURCE INT RTLX_RxPacket
    (
      INT iInstanceNo,
      INT iPort,
      UCHAR* pucFrame,
      UCHAR** ppucFrame
    );

SOURCE VOID RTLX_CloseTxSocket
    (
      INT iInstanceNo,
      BOOL boRedundancy
    );

// Timing functions (RTLX_S3SM_TIME.c)

#define         RTOS_NanoSleepRel           RTLX_NanoSleepRel
#define         RTOS_NanoSleepAbs           RTLX_NanoSleepAbs
#define         RTOS_NanoSleep              RTLX_NanoSleep
#define         RTOS_InitTimer              RTLX_InitTimer
#define         RTOS_SetTimerTime           RTLX_SetTimerTime
#define         RTOS_CloseTimer             RTLX_CloseTimer
#define         RTOS_CyclesToTime           RTLX_CyclesToTime

SOURCE VOID RTLX_NanoSleepRel
    (
      ULONG ulNanoSec
    );

SOURCE VOID RTLX_NanoSleepAbs
    (
      ULONG ulNanoSec
    );

SOURCE VOID RTLX_NanoSleep
    (
      ULONG ulNanoSec
    );

SOURCE INT RTLX_InitTimer
    (
      INT iInstanceNo,
      ULONG ulTimeNs,
      VOID* pAlarmFunc
    );

SOURCE INT RTLX_SetTimerTime
    (
      INT iInstanceNo,
      ULONG ulTimeNs
    );

SOURCE VOID RTLX_CloseTimer
    (
      INT iInstanceNo
    );

SOURCE VOID RTLX_CyclesToTime
    (
      ULONG ulCycleTime,
      ULONG ulNoOfCycles,
      RTLX_TIMESPEC *tReqCycle
    );

// Functions for Nic-based timing transmission (RTLX_NIC_TIMED.c)

#define         RTOS_InitTimerNic          RTLX_InitTimerNic
#define         RTOS_CloseTimerNic         RTLX_CloseTimerNic
#define         RTOS_SetTimerNic           RTLX_SetTimerNic
#define         RTOS_TxPacketsNicTimed     RTLX_TxPacketsNicTimed

SOURCE INT RTLX_InitTimerNic
    (
      INT iInstanceNo,
      ULONG ulTimeNs,
      VOID* pAlarmFunc
    );

SOURCE VOID RTLX_CloseTimerNic
    (
      INT iInstanceNo
    );

SOURCE INT RTLX_SetTimerNic
    (
      INT iInstanceNo,
      ULONG ulTimeNs
    );

SOURCE INT RTLX_TxPacketsNicTimed
    (
      INT iInstanceNo,
      RTOS_NIC_TIMED_PACKET_STRUCT* prPacketStruct,
      USHORT usIFG
    );

SOURCE INT RTLX_InitNicTimedTransmission
    (
      INT iInstanceNo,
      BOOL boRedundancy,
      INT iUCCPackets,
      UCHAR* pucMAC
  );

SOURCE VOID RTLX_CloseNicTimedTransmission
    (
      INT iInstanceNo,
      BOOL boRedundancy
    );

SOURCE INT RTLX_SetNicName
    (
      INT iInstanceNo,
      CHAR* pcNicName
    );


// Functions for UCC support (RTLX_S3SM_UCC.c)

#define         RTOS_OpenUCCSocket          RTLX_OpenUCCSocket
#define         RTOS_CloseUCCSocket         RTLX_CloseUCCSocket
#define         RTOS_TxUCCPacket            RTLX_TxUCCPacket
#define         RTOS_RxUCCPacket            RTLX_RxUCCPacket

SOURCE INT RTLX_OpenUCCSocket
    (
      INT iInstanceNo,
      UCHAR* pucMAC
    );

SOURCE VOID RTLX_CloseUCCSocket
    (
      INT iInstanceNo
    );

SOURCE INT RTLX_TxUCCPacket
    (
      INT iInstanceNo,
      INT iPort,
      UCHAR* pucFrame,
      USHORT usLen
    );

SOURCE INT RTLX_RxUCCPacket
    (
      INT iInstanceNo,
      INT iPort,
      BOOL boNrtState,
      UCHAR* pucFrame
    );

#ifdef __cplusplus
}
#endif

// avoid multiple inclusions - close

#endif
