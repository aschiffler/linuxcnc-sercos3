/**
 * \file      GLOB_TYPE.h
 *
 * \brief     Header that defines global data types, to be used with Sercos soft
 *            master
 *
 * THIS SOFTWARE IS PROVIDED "AS IS"; WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY;
 * FITNESS FOR A PERTICULAR PURPOSE AND NONINFRINGEMENT. THE AUTHORS OR COPYRIGHT
 * HOLDERS SHALL NOT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE;
 * UNLESS STIPULATED BY MANDATORY LAW.
 *
 * \ingroup   GLOB
 *
 * \author    GMy, based on GLOB_TYPE.h from CoSeMa
 *
 * \copyright Copyright Bosch Rexroth AG, 2013-2016
 *
 * \date      29.04.2013
 *
 * \version 2016-02-10 (WK, GMy): Solved compatibility issues with INtime
 */

#ifndef _GLOB_TYPE
#define _GLOB_TYPE

#include "GLOB_DEFS.h"

#ifdef __cplusplus
extern "C"
{
#endif /* #ifdef __cplusplus */


//---- includes ---------------------------------------------------------------

//---- defines ----------------------------------------------------------------

//---- type definitions -------------------------------------------------------

/*#define VOID                void*/
#ifndef VOID
typedef void                VOID;           /* v */
#endif
typedef void                CSMD_VOID;
typedef char                CHAR;           /* c */
typedef int                 INT;            /* n */     /* Target dependent */
typedef int                 CSMD_INT;

typedef unsigned char       UCHAR;          /* uc */    /* 8-bit-bit pattern */
typedef signed char         SCHAR;          /* sc */    /* 8-bit-bit pattern */
typedef char                STR;            /* str */   /* 8-bit-bit pattern */
typedef char                CSMD_CHAR;
typedef signed char         CSMD_SCHAR;
typedef unsigned char       CSMD_UCHAR;
/*typedef UCHAR             BYTE;*/         /* b */     /* 8-bit-bit pattern */

typedef short               SHORT;          /* s */     /* 16-Bit with sign */
typedef unsigned short      USHORT;         /* us */    /* 16-Bit without sg. */
typedef short               CSMD_SHORT;
typedef unsigned short      CSMD_USHORT;

#ifdef __unix__
typedef int                 LONG;           /* s */     /* 32-Bit with sign */
typedef unsigned int        ULONG;
typedef int                 CSMD_LONG;
typedef unsigned int        CSMD_ULONG;
typedef unsigned long       CSMD_UINT_PTR;
#else
typedef long                LONG;           /* s */     /* 32-Bit with sign */
typedef unsigned long       ULONG;          /* us */    /* 32-Bit without sg. */
typedef long                CSMD_LONG;
typedef unsigned long       CSMD_ULONG;
typedef unsigned int        CSMD_UINT_PTR;
#endif
typedef unsigned long long  ULONGLONG;      /* ull */   /* 64-Bit without sg. */
typedef long long           LONGLONG;       /* ll */    /* 64-Bit with sign */
/*typedef unsigned int      UINT;*/         /* ui */    /* Target dependent */

typedef float               FLOAT;          /* f */     /* 32-Bit-IEEE-float */
typedef double              DOUBLE;         /* d */     /* 64-Bit-IEEE-float */

#ifndef __INTIME__
    #ifndef BOOL
    typedef int             BOOL;           /* bo */    /* Target dependent */
    typedef int             CSMD_BOOL;
    #endif
#else
    #ifndef BOOL
    typedef UCHAR           BOOL;           /* bo */    /* Target dependent */
    #endif
    typedef UCHAR           CSMD_BOOL;
#endif




          /*!< prefix:    n     \n Target dependent     */
          /*<! prefix:    c     \n ASCII character      */

          /*!  prefix:    sc    \n  8-Bit with sign     */
          /*<! prefix:    uc    \n  8-Bit without sign  */
          /*<! prefix:    s     \n 16-Bit with sign     */
          /*<! prefix:    us    \n 16-Bit without sign  */
          /*<! prefix:    l     \n 32-Bit with sign     */
          /*<! prefix:    ul    \n 32-Bit without sign  */

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

#ifdef __cplusplus
}   /* extern "C" */
#endif /* #ifdef __cplusplus */

#endif /* #ifndef _GLOB_TYPE */
