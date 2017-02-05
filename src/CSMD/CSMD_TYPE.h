/*
CoSeMa V6.1 - Common Sercos Master function library
Copyright (c) 2004 - 2015  Bosch Rexroth AG

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THIS SOFTWARE IS PROVIDED "AS IS"; WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY;
FITNESS FOR A PERTICULAR PURPOSE AND NONINFRINGEMENT. THE AUTHORS OR COPYRIGHT
HOLDERS SHALL NOT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE;
UNLESS STIPULATED BY MANDATORY LAW.

You may contact us at open.source@boschrexroth.de
if you are interested in contributing a modification to the Software.

30 April 2015

============================================================================ */

/*!
 \file   CSMD_TYPE.h
 \author WK
 \date   06.03.2008
 \brief  This File contains the global type definitions for CoSeMa.
*/
/*!    
    \brief Agreement of types for the target microprocessor, which should \n
        be used in 'C' as well as in C++ Sources.

    \b Hint \b to the \b usage of the \b types:
       In the CoSeMa sources only the types which are defined in "CSMD_TYPE.h"
       should be used.

       - The (problematic) type CSMD_INT were supplied only for the cases,
         with which a variable in the respective machine variable  is needed.
       - The type CSMD_CHAR used to represent characters from the 8-Bit-ASCII-Set.

       - The types CSMD_SCHAR respectively CSMD_UCHAR are used to represent
         8-Bit signed/unsigned values.
       - The types CSMD_SHORT respectively CSMD_USHORT are used to represent
         16-Bit signed/unsigned values.
       - The types CSMD_LONG respectively CSMD_ULONG are used to represent
         32-Bit signed/unsigned values.

       - The type CSMD_BOOL serves to represent a boolean variable with the
         values TRUE or FALSE.

       - The type CSMD_UINT_PTR designates an unsigned integer type with the
         property that any valid pointer to void can be converted to this type,
         then converted back to pointer to void, and the result will compare
         equal to the original pointer.
        
    \b Note: 
        Is the internal representation of a variable of importance 
        (number of bytes, scope), never use CSMD_INT.
        In the case of doubt don't use CSMD_INT.
        
    \b Usage: 
        The header CSMD_TYPE.h should be included in all 
        applications that use CoSeMa functions.

*/


#ifndef _CSMD_TYPE
#define _CSMD_TYPE


#ifndef FALSE
  #define FALSE (0)
#endif

#ifndef TRUE
  #define TRUE  (1)
#endif


/*---------------- Declaration: Global Types --------------------------------*/

#if(0)
/*      C99-Standard
        Integer-Type        CoSeMa type               Prefix      Remark                  */
typedef void                CSMD_VOID;      /*<! prefix:    v     \n void                 */
typedef int32_t             CSMD_INT;       /*!< prefix:    n     \n Target dependent     */
typedef int8_t              CSMD_CHAR;      /*<! prefix:    c     \n ASCII character      */

typedef int8_t              CSMD_SCHAR;     /*!  prefix:    sc    \n  8-Bit with sign     */
typedef uint8_t             CSMD_UCHAR;     /*<! prefix:    uc    \n  8-Bit without sign  */
typedef int16_t             CSMD_SHORT;     /*<! prefix:    s     \n 16-Bit with sign     */
typedef uint16_t            CSMD_USHORT;    /*<! prefix:    us    \n 16-Bit without sign  */
typedef int32_t             CSMD_LONG;      /*<! prefix:    l     32-Bit with sign        */
typedef uint32_t            CSMD_ULONG;     /*<! prefix:    ul    32-Bit without sign     */

typedef int32_t             CSMD_BOOL;      /*<! prefix:    bo    \n boolean              */
typedef uintptr_t           CSMD_UINT_PTR;  /*<! prefix:          \n unsigned integer type
                                                          capable of holding any void ptr */
#endif


/*      Original type       CoSeMa type               Prefix      Remark                  */
typedef void                CSMD_VOID;      /*<! prefix:    v     \n void                 */
typedef int                 CSMD_INT;       /*!< prefix:    n     \n Target dependent     */
typedef char                CSMD_CHAR;      /*<! prefix:    c     \n ASCII character      */

typedef signed char         CSMD_SCHAR;     /*!  prefix:    sc    \n  8-Bit with sign     */
typedef unsigned char       CSMD_UCHAR;     /*<! prefix:    uc    \n  8-Bit without sign  */
typedef short               CSMD_SHORT;     /*<! prefix:    s     \n 16-Bit with sign     */
typedef unsigned short      CSMD_USHORT;    /*<! prefix:    us    \n 16-Bit without sign  */
typedef long                CSMD_LONG;      /*<! prefix:    l     \n 32-Bit with sign     */
typedef unsigned long       CSMD_ULONG;     /*<! prefix:    ul    \n 32-Bit without sign  */

typedef int                 CSMD_BOOL;      /*<! prefix:    bo    \n boolean              */
typedef unsigned long       CSMD_UINT_PTR;  /*<! prefix:          \n unsigned integer type
                                                          capable of holding any void ptr */



/*
** More prefixes for special types
** -------------------------------------------------------------------------
**
** Enumeration       (enum):        e
** Structure         (struct):      r   ( = record )
** Union             (union):       u
** Constant variable (const):       c
**
*/


/*
** Additional prefixes with pointers and vectors
** -------------------------------------------------------------------------
** Vector :                         a   ( = array )
** Pointer:                         p   ( = pointer )
**
** Function pointer:               fp   ( = pointer to a function )
*/

#endif /* #ifndef _CSMD_TYPE */




/*
--------------------------------------------------------------------------------
  Modification history
--------------------------------------------------------------------------------

06 Mar 2008
  - Create from GLOB_TYPE.h and GLOB_DEFS.h.
17 Jan 2014 WK
  - Added type INT.
  - Revised doxygen documentation.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
23 Jul 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
    Added type CSMD_UINT_PTR.

--------------------------------------------------------------------------------
*/
