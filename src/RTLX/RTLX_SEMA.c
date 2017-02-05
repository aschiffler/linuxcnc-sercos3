/**
 * \file      RTLX_SEMA.c
 *
 * \brief     Real-time operating system abstraction layer for Linux
 *            RT-Preempt: Semaphore handling
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
 *
 * \ingroup   RTLX
 */

//---- includes ---------------------------------------------------------------

#define SOURCE_RTLX

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
 * \fn INT RTLX_CreateSemaphore(
 *              RTLX_SEMAPHORE *pSem
 *          )
 *
 * \brief   Creates a new semaphore
 *
 * \param[in]   pSem    Pointer to RTLX_SEMAPHORE to be created
 *
 * \return
 * - 0: Success
 * - Error otherwise
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2012-10-15
 */
INT RTLX_CreateSemaphore
    (
      RTLX_SEMAPHORE *pSem,
	  char	*name
    )
{
  INT iRet = 0;

  iRet = sem_init
      (
        pSem,   // Pointer to semaphore
        0,      // Shared semaphore in common memory
        0       // Initial value
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
 * \fn INT RTLX_WaitForSemaphore(
 *              RTLX_SEMAPHORE *pSem
 *          )
 *
 * \brief   Waits for semaphore to be freed
 *
 * \param[in]   pSem    Pointer to RTLX_SEMAPHORE
 *
 * \return
 * - 0: Success
 * - Error otherwise
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2012-10-15
 */
INT RTLX_WaitForSemaphore
    (
      RTLX_SEMAPHORE *pSem
    )
{
  INT iRet = 0;

  iRet = sem_wait(pSem);

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
 * \fn INT RTLX_TrySemaphore(
 *              RTLX_SEMAPHORE *pSem
 *          )
 *
 * \brief   Checks whether a semaphore may be got without waiting. If so, it
 *          does.
 *
 * \param[in]   pSem    Pointer to RTLX_SEMAPHORE
 *
 * \return
 * - 0: Success
 * - Error otherwise
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2013-04-18
 */
INT RTLX_TrySemaphore
    (
      RTLX_SEMAPHORE *pSem
    )
{
  INT iRet = 0;

  iRet = sem_trywait(pSem);

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
 * \fn INT RTLX_PostSemaphore(
 *              RTLX_SEMAPHORE *pSem
 *          )
 *
 * \brief   Post to semaphore
 *
 * \param[in]   pSem    Pointer to RTLX_SEMAPHORE
 *
 * \return
 * - 0: Success
 * - Error otherwise
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2012-10-15
 */
INT RTLX_PostSemaphore
    (
      RTLX_SEMAPHORE *pSem
    )
{
  INT iRet = 0;

  iRet = sem_post(pSem);

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
 * \fn INT RTLX_DestroySemaphore(
 *              RTLX_SEMAPHORE *pSem
 *          )
 *
 * \brief   Destroys semaphore
 *
 * \param[in]   pSem    Pointer to RTLX_SEMAPHORE
 *
 * \return
 * - 0: Success
 * - Error otherwise
 *
 * \ingroup RTLX
 *
 * \author  GMy
 *
 * \date    2012-10-15
 */
INT RTLX_DestroySemaphore
    (
      RTLX_SEMAPHORE *pSem
    )
{
  INT iRet = 0;

  iRet = sem_destroy(pSem);

  if (iRet == 0)
  {
    return(RTOS_RET_OK);
  }
  else
  {
    return(RTOS_RET_ERROR);
  }
}

