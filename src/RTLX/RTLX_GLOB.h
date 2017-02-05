/**
 * \file      RTLX_GLOB.h
 *
 * \brief     Global header for real-time operating system abstraction layer for
 *            Linux RT-Preempt
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
 * \ingroup   RTLX
 *
 */

// avoid multiple inclusions - open

#ifndef _RTLX_GLOB
#define _RTLX_GLOB

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
#include "../RTLX/RTLX_USER.h"

//---- defines ----------------------------------------------------------------

#define RTOS_RET_OK     (0)
#define RTOS_RET_ERROR  (-1)

//---- type definitions -------------------------------------------------------

#define RTOS_SEMAPHORE RTLX_SEMAPHORE

/**
 * \typedef RTLX_SEMAPHORE
 *
 * \brief   Abstract type for semaphore, to be re-defined when using a
 *          different RTOS
 */
typedef sem_t RTLX_SEMAPHORE;

#define RTOS_THREAD RTLX_THREAD

/**
 * \typedef RTLX_THREAD
 *
 * \brief   Abstract type for thread, to be re-defined when using a different
 *          RTOS
 */
typedef pthread_t RTLX_THREAD;

#define RTOS_TIMESPEC RTLX_TIMESPEC

/**
 * \typedef RTOS_TIMESPEC
 *
 * \brief   Abstract type for time data structure, to be re-defined when using
 *          a different RTOS
 */
typedef struct timespec RTLX_TIMESPEC;

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Initialization functions (RTLX_INIT.c)

#define         RTOS_Init                   RTLX_Init
#define         RTOS_Close                  RTLX_Close

SOURCE INT RTLX_Init
    (
      INT iInstance
    );

SOURCE INT RTLX_Close
    (
      INT iInstance
    );

// File access functions (RTLX_FILE.c)

#define         RTOS_ReadFile               RTLX_ReadFile
#define         RTOS_WriteFile              RTLX_WriteFile
#define         RTOS_GetChar                getchar
#define         RTOS_PrintF                 printf
#define         RTOS_ScanF                  scanf

SOURCE INT RTLX_ReadFile
    (
      CHAR* pcFileName,
      UCHAR* pucBuffer,
      ULONG ulBufSize
    );

SOURCE INT RTLX_WriteFile
    (
      CHAR* pcFileName,
      UCHAR* pucBuffer,
      ULONG ulBufSize
    );

// Functions for thread handling (RTLX_THREAD.c)

#define         RTOS_CreateThread           RTLX_CreateThread
#define         RTOS_CloseThread            RTLX_CloseThread
#define         RTOS_CloseThreadRemote      RTLX_CloseThreadRemote
#define         RTOS_DestroyThreadData      RTLX_DestroyThreadData
#define         RTOS_SetThreadPriority      RTLX_SetThreadPriority
#define         RTOS_AllowRemoteClose       RTLX_AllowRemoteClose

SOURCE INT RTLX_CreateThread
    (
      VOID *pThreadFunc,
      RTLX_THREAD *pThread,
	  char 	*name,
      VOID *pArg
    );

SOURCE VOID RTLX_CloseThread
    (
      VOID
    );

SOURCE INT RTLX_CloseThreadRemote
    (
      RTLX_THREAD *pThread
    );

SOURCE VOID RTLX_DestroyThreadData
    (
      RTLX_THREAD *pThread
    );

SOURCE INT RTLX_SetThreadPriority
    (
      INT iPriority
    );

SOURCE VOID RTLX_AllowRemoteClose
    (
      VOID
    );

SOURCE INT RTLX_SetThreadCoreAffinity
    (
      INT iCoreNo
    );

// Functions for semaphore handling (RTLX_SEMA.c)

#define         RTOS_CreateSemaphore        RTLX_CreateSemaphore
#define         RTOS_WaitForSemaphore       RTLX_WaitForSemaphore
#define         RTOS_TrySemaphore           RTLX_TrySemaphore
#define         RTOS_PostSemaphore          RTLX_PostSemaphore
#define         RTOS_DestroySemaphore       RTLX_DestroySemaphore

SOURCE INT RTLX_CreateSemaphore
    (
      RTLX_SEMAPHORE *pSem,
	  char 	*name
    );

SOURCE INT RTLX_WaitForSemaphore
    (
      RTLX_SEMAPHORE *pSem
    );

SOURCE INT RTLX_TrySemaphore
    (
      RTLX_SEMAPHORE *pSem
    );

SOURCE INT RTLX_PostSemaphore
    (
      RTLX_SEMAPHORE *pSem
    );

SOURCE INT RTLX_DestroySemaphore
    (
      RTLX_SEMAPHORE *pSem
    );

// Timing functions (RTLX_TIME.c)

#define         RTOS_SimpleMicroWait        RTLX_SimpleMicroWait
#define         RTOS_GetSystemTime          RTLX_GetSystemTime
#define         RTOS_IncTime                RTLX_IncTime
#define         RTOS_WaitForSystemTime      RTLX_WaitForSystemTime
#define         RTOS_GetTimeDifference      RTLX_GetTimeDifference
#define         RTOS_GetTimeNs              RTLX_GetTimeNs

SOURCE VOID RTLX_SimpleMicroWait
    (
      ULONG ulMicroSec
    );

SOURCE VOID RTLX_GetSystemTime
    (
      RTLX_TIMESPEC * pSystemTime
    );

SOURCE VOID RTLX_IncTime
    (
      RTLX_TIMESPEC * pSystemTime,
      LONG lNanoSec
    );

SOURCE VOID RTLX_WaitForSystemTime
    (
      RTLX_TIMESPEC *pTargetTime
    );

SOURCE VOID RTLX_GetTimeDifference
    (
      RTLX_TIMESPEC *pTime1,
      RTLX_TIMESPEC *pTime2,
      RTLX_TIMESPEC *pTimeDiff
    );

SOURCE LONG RTLX_GetTimeNs
    (
      RTLX_TIMESPEC *pTime
    );

// Functions for memory management (RTLX_MEM.c)

#define         RTOS_GetPageAlignedMemSize  RTLX_GetPageAlignedMemSize
#define         RTOS_Malloc                 RTLX_Malloc
#define         RTOS_Free                   RTLX_Free
#define         RTOS_Memset                 RTLX_Memset
#define         RTOS_Memcpy                 RTLX_Memcpy

SOURCE ULONG RTLX_GetPageAlignedMemSize
    (
      ULONG lUnalignedSize
    );

SOURCE ULONG RTLX_Malloc
    (
      VOID** ppvMem,
      ULONG ulSize
    );

SOURCE ULONG RTLX_Free
    (
      VOID* pvMem
    );

SOURCE ULONG RTLX_Memset
    (
      VOID* pvMem,
      UCHAR ucValue,
      ULONG ulSize
    );

SOURCE ULONG RTLX_Memcpy
    (
      VOID* pvMemDest,
      VOID* pvMemSrc,
      ULONG ulSize
    );

#ifdef __cplusplus
}
#endif

// avoid multiple inclusions - close

#endif
