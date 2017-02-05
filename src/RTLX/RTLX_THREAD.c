/**
 * \file      RTLX_THREAD.c
 *
 * \brief     Real-time operating system abstraction layer for Linux RT-Preempt:
 *            Thread handling
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
 */

//---- includes ---------------------------------------------------------------

#define SOURCE_RTLX

#define _GNU_SOURCE

/*lint -save -w0 */
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
/*lint -restore */

#include "../RTLX/RTLX_GLOB.h"
#include "../RTLX/RTLX_PRIV.h"
#include "../GLOB/GLOB_DEFS.h"
#include "../GLOB/GLOB_TYPE.h"

//---- defines ----------------------------------------------------------------

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn INT RTLX_CreateThread(
 *              VOID *pThreadFunc,
 *              RTLX_THREAD *pThread,
 *              VOID *pArg
 *          )
 *
 * \brief   Creates a new thread with a pointer as argument.
 *
 * \param[in]       pThreadFunc Pointer to thread function
 * \param[in,out]   pThread     Pointer to thread data structure
 * \param[in]       pArg        Pointer to argument, 'NULL' for none
 *
 * \return
 * - 0: Successful
 * - 1: Error
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2013-04-11
 *
 * \version 2013-04-19 (GMy): Change of function parameters
 */
INT RTLX_CreateThread
    (
      VOID *pThreadFunc,
      RTLX_THREAD *pThread,
	  char	*name,
      VOID *pArg
    )
{
  INT iRet = 0;

  iRet = pthread_create
      (
        pThread,        // Thread pointer
        NULL,           // Thread attributes (NULL for default)
        pThreadFunc,    // Function pointer
        pArg            // Thread arguments (NULL for none)
      );

  if (iRet == 0)
  {
    return(RTOS_RET_OK);
  }
  else
  {
    return(RTOS_RET_ERROR);
  }
}

/**
 * \fn VOID RTLX_CloseThread(
 *              VOID
 *          )
 *
 * \brief   Closes the calling thread. Additionally, RTLX_DestroyThreadData()
 *          should be called in order to avoid memory leaks on some systems.
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2012-10-16
 */
VOID RTLX_CloseThread
    (
      VOID
    )
{
  pthread_exit(NULL);
}

/**
 * \fn INT RTLX_CloseThreadRemote(
 *              RTLX_THREAD *pThread
 *          )
 *
 * \brief   Closes the thread given as parameter. This thread has to call
 *          RTLX_AllowRemoteClose() first, as otherwise on some systems
 *          remotely closing threads is not possible. Additionally,
 *          RTLX_DestroyThreadData() should be called in order to avoid memory
 *          leaks on some systems.
 *
 * \param[in]    pThread Thread to be closed
 *
 * \return
 * - 0 for success
 * - -1 for error
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2013-04-19
 */
INT RTLX_CloseThreadRemote
    (
      RTLX_THREAD *pThread
    )
{
  INT iRet = 0;

  iRet = pthread_cancel(*pThread);
  if (iRet == 0)
  {
    return(RTOS_RET_OK);
  }
  else
  {
    return(RTOS_RET_ERROR);
  }
}

/**
 * \fn VOID RTLX_AllowRemoteClose(
 *              VOID
 *          )
 *
 * \brief   Allows that the thread can be closed remotely using
 *          RTLX_CloseThreadRemote(), if needed on the present system
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2013-04-19
 */
VOID RTLX_AllowRemoteClose
    (
      VOID
    )
{
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
}

/**
 * \fn VOID RTLX_DestroyThreadData(
 *              RTLX_THREAD *pThread
 *          )
 *
 * \brief   Closes the thread data structure, if needed on current system
 *
 * \param[in]    pThread Thread data structure to be destroyed
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2013-04-19
 */
VOID RTLX_DestroyThreadData
    (
      RTLX_THREAD *pThread
    )
{
    // Nothing to do on Linux systems, as RTLX_THREAD is a static structure
}

/**
 * \fn INT RTLX_SetThreadPriority(
 *              INT iPriority
 *          )
 *
 * \brief   Sets priority of the calling thread.
 *
 * \note    The priority values for the different threads should be defined
 *          within the RTOS abstraction layer, as the meaning of values (e.g.
 *          higher for higher priority) is dependent on the RTLX.
 *
 * \param[in]   iPriority Priority
 *
 * \return
 * - 0: Success
 * - -1: Error
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2012-10-11
 */
INT RTLX_SetThreadPriority
    (
      INT iPriority
    )
{
  INT iRet = 0;
  struct sched_param rSchedParam;

  rSchedParam.sched_priority = iPriority;

  iRet = sched_setscheduler
      (
        0,              // PID, 0 for current process
        SCHED_FIFO,     // FIFO scheduling scheme
        &rSchedParam    // Scheduling parameters
      );

  if (iRet == 0)
  {
    return(RTOS_RET_OK);
  }
  else
  {
    return(RTOS_RET_ERROR);
  }
}

/**
 * \fn INT RTLX_SetThreadCoreAffinity(
 *              INT iCoreNo
 *          )
 *
 * \brief   Sets core affinity of the calling thread.
 *
 * \note    iCoreNo needs to be between 0 and number of cores - 1.
 *
 * \param[in]   iCoreNo Core number
 *
 * \return
 * - 0: Success
 * - -1: Error
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2014-10-22
 */
INT RTLX_SetThreadCoreAffinity
    (
      INT iCoreNo
    )
{
  INT iNumCores = 0;
  INT iRet = 0;

  // Retrieve number of CPU cores present
  iNumCores = sysconf(_SC_NPROCESSORS_ONLN);

  if (
      (iNumCores  >=  0)          ||
      (iNumCores  <   iNumCores)
    )
  {
    cpu_set_t rCpuSet;
    CPU_ZERO(&rCpuSet);
    CPU_SET(iCoreNo, &rCpuSet);

    iRet = pthread_setaffinity_np
        (
          pthread_self(),
          sizeof(cpu_set_t),
          &rCpuSet
        );
    if (iRet == 0)
    {
      return(RTOS_RET_OK);
    }
  }

  return(RTOS_RET_ERROR);
}

