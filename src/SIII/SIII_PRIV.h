/**
 * \file      SIII_PRIV.h
 *
 * \brief     Sercos III soft master stack - Private header
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
 * \date      2012-11-05
 *
 * \version 2013-04-11 (GMy): Module re-arrangement
 * \version 2013-05-07 (GMy): Added support for INtime
 * \version 2013-06-06 (GMy): Added support for RTX and Kithara
 * \version 2013-06-21 (GMy): Added support for Windows desktop versions and
 *                            QNX
 * \version 2016-10-27 (AlM): Support for CoSeMa V5 removed.
 */

// avoid multiple inclusions - open

#ifndef _SIII_PRIV
#define _SIII_PRIV

#undef SOURCE
#ifdef SOURCE_SIII
    #define SOURCE
#else
    #define SOURCE extern
#endif

//---- includes ---------------------------------------------------------------

/*lint -save -w0 */
#include <string.h>             // for memset
#include <stdlib.h>
#include <stdio.h>
/*lint -restore */

#ifdef __qnx__
#include "../RTQX/RTQX_GLOB.h"
#elif defined __unix__
#include "../RTLX/RTLX_GLOB.h"
#elif defined WINCE7
#include "../RTC7/RTC7_GLOB.h"
#elif defined WINCE
#include "../RTC6/RTC6_GLOB.h"
#elif defined __INTIME__
#include "../RTIT/RTIT_GLOB.h"
#elif defined __RTX__
#include "../RTRX/RTRX_GLOB.h"
#elif defined __VXWORKS__
#include "../RTVW/RTVW_GLOB.h"
#elif defined __KITHARA__
#include "../RTKT/RTKT_GLOB.h"
#elif defined WIN32
#include "../RTWI/RTWI_GLOB.h"
#elif defined WIN64
#include "../RTWI/RTWI_GLOB.h"
#else
#error Operating system not supported by SIII!
#endif

//---- defines ----------------------------------------------------------------

/**
 * \def SIII_VERBOSE(_debuglevel, ...)
 *
 * \brief   Macro for debug outputs that depend on the selected verbose level.
 */
#define SIII_VERBOSE(_debuglevel, ...)  /*lint -save -e774 -e506 */             \
                                        if (_debuglevel <= SIII_VERBOSE_LEVEL)  \
                                        {                                       \
                                            /*printf("SIII: ");*/               \
                                            RTOS_PrintF(__VA_ARGS__);           \
                                        }                                       \
                                        /*lint -restore */

/**
 * \def SIII_C_CON_PROD_RDY
 *
 * \brief Producer-ready bit in C-CON
 */
#define SIII_C_CON_PROD_RDY             (0x0001)

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// SIII_INIT.c

// SIII_CONF.c
SOURCE SIII_FUNC_RET SIII_InitConnections
(
  SIII_INSTANCE_STRUCT *prS3Instance
);

SOURCE SIII_FUNC_RET SIII_BuildConnInfoList
(
  SIII_INSTANCE_STRUCT *prS3Instance
);

// SIII_SVC.c
SOURCE VOID SIII_SVCThread
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

// SIII_PHASE.c
SOURCE SIII_FUNC_RET SIII_PhaseHandler
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

// SIII_CYCLIC.c

SOURCE SIII_FUNC_RET SIII_GetConnections
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

SOURCE SIII_FUNC_RET SIII_SetConnections
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

SOURCE SIII_FUNC_RET SIII_CyclicHotplugFunc
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    );

#ifdef __cplusplus
}
#endif

#endif /* USER_PRIV_H_ */
