/**
 * \file      RTLX_TIME.c
 *
 * \brief     Real-time operating system abstraction layer for Linux
 *            RT-Preempt: Time functions
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
 *
 */

//---- includes ---------------------------------------------------------------

#define SOURCE_RTLX

/*lint -save -w0 */
#include <unistd.h>
#include <signal.h>
#include <time.h>   // needs rt-library; Project Explorer: Properties ->
                    // C/C++ Build -> GCC C++ Linker -> Libraries
#include <sys/time.h>
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
 * \fn VOID RTLX_SimpleMicroWait(
 *              ULONG ulMicroSec
 *          )
 *
 * \brief   Sleep for roughly a number of micro seconds
 *
 * \param[in]   ulMicroSec  Number of micro seconds to sleep
 *
 * \return  None.
 *
 * \ingroup RTLX
 *
 */
VOID RTLX_SimpleMicroWait
    (
      ULONG ulMicroSec
    )
{
  (VOID)usleep (ulMicroSec);
}

/**
 * \fn VOID RTLX_GetSystemTime(RTLX_TIMESPEC *tpSystem)
 *
 * \brief   Reads system time
 *
 * \param[out]  tpSystem    Pointer to structure for system time
 *
 * \return  None
 *
 * \ingroup RTLX
 *
 */
VOID RTLX_GetSystemTime(RTLX_TIMESPEC *pSystemTime)
{
  (VOID) clock_gettime
      (
        CLOCK_MONOTONIC,
        pSystemTime
      );
}

/**
 * \fn VOID RTLX_IncTime(
 *              RTLX_TIMESPEC * pSystemTime,
 *              LONG lNanoSec
 *          )
 *
 * \brief   Increases time structure by offset in ns.
 *
 * \param[in,out]   pSystemTime Pointer to structure for system time
 * \param[in]       lNanoSec    Number of nanoseconds,
 *                              abs(lNanoSec)<=1000*1000*1000
 *
 * \return  None
 *
 * \ingroup RTLX
 *
 */
VOID RTLX_IncTime
    (
      RTLX_TIMESPEC * pSystemTime,
      LONG lNanoSec
    )
{
  if ((((LONG)pSystemTime->tv_nsec) + lNanoSec) < 0)
  {
    pSystemTime->tv_nsec += 1000*1000*1000 + lNanoSec;
    pSystemTime->tv_sec--;
  }
  else
  {
    pSystemTime->tv_nsec += lNanoSec;
    if (pSystemTime->tv_nsec > 1000*1000*1000)
    {
      pSystemTime->tv_nsec -= 1000*1000*1000;
      pSystemTime->tv_sec++;
    }
  }
}

/**
 * \fn VOID RTLX_WaitForSystemTime(
 *              RTLX_TIMESPEC *pTargetTime
 *          )
 *
 * \brief   Waits until a certain system time
 *
 * \param[in]   pTargetTime Point of system time to wait for
 *
 * \ingroup RTLX
 *
 */
VOID RTLX_WaitForSystemTime
    (
      RTLX_TIMESPEC *pTargetTime
    )
{
  RTLX_TIMESPEC tSystemTime;
  BOOL boDone = FALSE;

  do
  {
    (VOID)clock_gettime(CLOCK_MONOTONIC, &tSystemTime);

    if (tSystemTime.tv_sec > pTargetTime->tv_sec)
    {
      boDone = TRUE;
    }
    else if (tSystemTime.tv_sec < pTargetTime->tv_sec)
    {
      boDone = FALSE;
    }
    else if (tSystemTime.tv_nsec > pTargetTime->tv_nsec)
    {
      boDone = TRUE;
    }

  } while (!boDone);
}

/**
 * \fn VOID RTLX_GetTimeDifference(
 *              RTLX_TIMESPEC *pTime1,
 *              RTLX_TIMESPEC *pTime2,
 *              RTLX_TIMESPEC *pTimeDiff
 *          )
 *
 * \brief   Returns difference of pTime1 and pTime2 in ns.
 *
 * \param[in]   pTime1      Time 1
 * \param[in]   pTime2      Time 2
 * \param[out]  pTimeDiff   Time difference (Time1 - Time2)
 *
 * \return  None
 *
 * \ingroup RTLX
 *
 */
VOID RTLX_GetTimeDifference
    (
      RTLX_TIMESPEC *pTime1,
      RTLX_TIMESPEC *pTime2,
      RTLX_TIMESPEC *pTimeDiff
    )
{
  if (pTime1->tv_nsec >= pTime2->tv_nsec)
  {
    pTimeDiff->tv_nsec  = pTime1->tv_nsec - pTime2->tv_nsec;
    pTimeDiff->tv_sec   = pTime1->tv_sec  - pTime2->tv_sec;
  }
  else
  {
    // Take care of 'carry'
    pTimeDiff->tv_nsec  = pTime1->tv_nsec + 1000*1000*1000 - pTime2->tv_nsec;
    pTimeDiff->tv_sec   = pTime1->tv_sec - 1 - pTime2->tv_sec;
  }
}

/**
 * \fn LONG RTLX_GetTimeNs(
 *              RTLX_TIMESPEC *pTime
 *          )
 *
 * \brief   Returns the input time in ns (modulo seconds).
 *
 * \param[in]   pTime   Input for time
 *
 * \return  Value in ns
 *
 * \ingroup RTLX
 *
 */
LONG RTLX_GetTimeNs
    (
      RTLX_TIMESPEC *pTime
    )
{
  if (pTime->tv_sec >= 0)
  {
    return(pTime->tv_nsec);
  }
  else
  {
    return(((LONG)(-1000*1000*1000)) + ((LONG)pTime->tv_nsec));
  }
}

/**
 * \fn LONG RTLX_GetTimeS(
 *              RTLX_TIMESPEC *pTime
 *          )
 *
 * \param[in]   pTime   Input for time
 *
 * \brief   Returns the input time in seconds.
 *
 * \return  Value in s
 *
 * \ingroup RTLX
 *
 */
LONG RTLX_GetTimeS
    (
      RTLX_TIMESPEC *pTime
    )
{
  return(pTime->tv_sec);
}



