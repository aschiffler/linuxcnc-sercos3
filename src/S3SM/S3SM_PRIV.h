/**
 * \file      S3SM_PRIV.h
 *
 * \brief     Private header for Sercos III soft master
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
 *
 *
 */

// avoid multiple inclusions - open

#ifndef _S3SM_PRIV
#define _S3SM_PRIV

#undef SOURCE
#ifdef SOURCE_S3SM
    #define SOURCE
#else
    #define SOURCE extern
#endif

//---- includes ---------------------------------------------------------------

/*lint -save -w0 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*lint -restore */

#include "../GLOB/GLOB_TYPE.h"
#include "../S3SM/S3SM_USER.h"

//---- defines ----------------------------------------------------------------

/**
 * \def     S3SM_VERBOSE(_debuglevel, ...)
 *
 * \brief   Macro for debug outputs that depend on the selected verbose level.
 */
#define S3SM_VERBOSE(_debuglevel, ...)  /*lint -save -e774 -e506 */             \
                                        if (_debuglevel <= S3SM_VERBOSE_LEVEL)  \
                                        {                                       \
                                            printf(__VA_ARGS__);           \
                                        }                                       \
                                        /*lint -restore */

#define S3SM_LIST_LENGTHS( NBR_IDN )    (((CSMD_MAX_IDN_PER_CONNECTION << 16) + NBR_IDN) << 2)

#define S3SM_CONN_LEN_MDT_DEFAULT       (0)

#define S3SM_CONN_LEN_AT_DEFAULT        (0)

//---- type definitions -------------------------------------------------------

/**
 * \enum    S3SM_FUNC_RET
 * \brief   Enumeration of S3SM error codes.
 */
typedef enum S3SM_FUNC_RET_EN
{
    /* --------------------------------------------------------- */
    /* no error: states, warning codes  (error_class 0x00000nnn) */
    /* --------------------------------------------------------- */
    S3SM_NO_ERROR               = (0x00000000),     /**< 0x00 Function successfully completed */
    S3SM_FUNCTION_IN_PROCESS,                       /**< 0x01 Function processing still active */
    S3SM_END_ERR_CLASS_00000,                       /**< End marker for error class 0x00000 nnn */

    // Between code 0x00000001 and 0x002FFFFF, SIII, CoSeMa, and SICE warnings are inherited

    /* --------------------------------------------------------- */
    /* warning codes                    (error_class 0x0021nnnn) */
    /* --------------------------------------------------------- */
    S3SM_WARNING                = (0x00310000),     /**< 0x00 General: error during function execution */
    S3SM_NO_EFFECT,                                 /**< 0x01 Function has no effect due to inconsistent configuration */
    S3SM_CODE_EXIT_REQUEST,                         /**< 0x02 User Request to end program */
    S3SM_END_ERR_CLASS_0021,                        /**< End marker for error class 0x00021 nnn */

    /* --------------------------------------------------------- */
    /* system error codes               (error_class 0x0022nnnn) */
    /* --------------------------------------------------------- */
    S3SM_SYSTEM_ERROR           = (0x00320000),     /**< 0x00 General: error during function execution */
    S3SM_END_ERR_CLASS_0022,                        /**< End marker for error class 0x00022 nnn */

    /* --------------------------------------------------------- */
    /* Sercos error codes               (error_class 0x0023nnnn) */
    /* --------------------------------------------------------- */
    S3SM_SERCOS_ERROR           = (0x00330000),     /**< 0x00 General: Sercos error */
    S3SM_END_ERR_CLASS_0023,                        /**< End marker for error class 0x00023 nnn */

    /* --------------------------------------------------------- */
    /* Configuration error codes        (error_class 0x0024nnnn) */
    /* --------------------------------------------------------- */
    S3SM_CONFIG_ERROR           = (0x00340000),     /**< 0x00 General: configuration error */
    S3SM_END_ERR_CLASS_0024,                        /**< End marker for error class 0x00024 nnn */

    /* --------------------------------------------------------- */
    /* SVC error codes                  (error_class 0x0025nnnn) */
    /* --------------------------------------------------------- */
    S3SM_SVC_ERROR              = (0x00350000),     /**< 0x00 General: SVC error */
    S3SM_END_ERR_CLASS_0025                         /**< End marker for error class 0x00025 nnn */

} S3SM_FUNC_RET;

#endif
//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

