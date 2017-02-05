/**
 * \file      RTLX_PRIV.h
 *
 * \brief     Private header for real-time operating system abstraction layer for
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
 *
 * \ingroup   RTLX
 */

// avoid multiple inclusions - open

#ifndef _RTLX_PRIV
#define _RTLX_PRIV

#undef SOURCE
#ifdef SOURCE_RTLX
    #define SOURCE
#else
    #define SOURCE extern
#endif

//---- includes ---------------------------------------------------------------

/*lint -save -w0 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
/*lint -restore */

#include "../GLOB/GLOB_DEFS.h"
#include "../GLOB/GLOB_TYPE.h"

//---- defines ----------------------------------------------------------------

/**
 * \def RTLX_VERBOSE(_debuglevel, ...)
 *
 * \brief   Macro for debug outputs that depend on the selected verbose level.
 */
#define RTLX_VERBOSE(_debuglevel, ...)  /*lint -save -e774 -e506 */             \
                                        if (_debuglevel <= RTLX_VERBOSE_LEVEL)  \
                                        {                                       \
                                            printf("RTLX: ");                   \
                                            printf(__VA_ARGS__);                \
                                        }                                       \
                                        /*lint -restore */

//---- type definitions -------------------------------------------------------

typedef enum
{
    RTLX_CONSUME    = 0,
    RTLX_NO_CONSUME
} RTLX_QUEUE_FLAG;

typedef struct QuTaskWaiting
{
  RTLX_SEMAPHORE*         ptTask_Semaphore;
  RTLX_QUEUE_FLAG         tFlag;
  struct QuTaskWaiting*   pNext_Qu_Waiting_Task;
  VOID*                   pvDataPointer;
}RTLX_TASK_QU_WAITING;

typedef struct
{
  USHORT                  usNumberQuElements;
  USHORT                  usMsgSize;
  VOID*                   pvMessagePointer;
  USHORT                  usCurrentMsgReadIdx;
  USHORT                  usCurrentMsgWriteIdx;
  RTLX_TASK_QU_WAITING*   pFirst_Qu_Waiting_Task;
}RTLX_QU_CTR;

typedef enum
{
  RTLX_CONSUME_OR = 0,
  RTLX_NO_CONSUME_OR,
  RTLX_CONSUME_AND,
  RTLX_NO_CONSUME_AND
} RTLX_EVENT_FLAG;

typedef struct EvtTaskWaiting
{
  ULONG                   ulRequested_Event;
  RTLX_SEMAPHORE*         ptTask_Semaphore;
  RTLX_EVENT_FLAG         tEventFlag;
  struct EvtTaskWaiting*  pNext_Evt_Waiting_Task;
}RTLX_TASK_EVT_WAITING;

typedef struct
{
  ULONG                   ulActive_Event;
  USHORT                  usNumberTasksWaiting;
  RTLX_TASK_EVT_WAITING*  pFirst_Evt_Waiting_Task;
}RTLX_EVT_CTR;

#define RTLX_NULL (0)
#define RTLX_OK     (0)
#define RTLX_QUEUE_FULL      (0x30100)

//---- variable declarations --------------------------------------------------

SOURCE RTOS_SEMAPHORE RTLX_Mutex;


//---- function declarations --------------------------------------------------

// avoid multiple inclusions - close

#endif /* RTLX_PRIV_H_ */
