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
\file   CSMD_INIT.h
\author WK
\date   23.01.2014
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_INIT.c
*/


#ifndef _CSMD_INIT
#define _CSMD_INIT

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif


/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! \cond PRIVATE */

/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

const CSMD_CONFIG_ERROR crCSMD_ConfigErrorInit = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

/* Reset variables for the dynamic memory allocation */
SOURCE CSMD_VOID CSMD_ResetMemAllocManagement
                                ( CSMD_INSTANCE              *prCSMD_Instance );

/* Set the required system limits */
SOURCE CSMD_FUNC_RET CSMD_SetSystemLimits
                                ( CSMD_INSTANCE              *prCSMD_Instance,
                                  CSMD_SYSTEM_LIMITS_STRUCT  *prSysLimits );

/* Get the call-back function pointers for memory allocation */
SOURCE CSMD_FUNC_RET CSMD_InitMemoryAllocCB
                                ( CSMD_INSTANCE              *prCSMD_Instance,
                                  CSMD_MEM_ALLOC_CB_STRUCT   *prMemAllocCB_Table );

/* Memory calculation/allocation for CoSeMa arrays/structures */
SOURCE CSMD_FUNC_RET CSMD_Ptr_MemoryAllocation
                                ( CSMD_INSTANCE              *prCSMD_Instance,
                                  CSMD_BOOL                   boAllocate,
                                  CSMD_LONG                  *plBytes );

SOURCE CSMD_VOID CSMD_Calc_Alloc_Mem
                                ( CSMD_BOOL                   boAllocate,
                                  CSMD_ULONG                  ulSize,
                                  CSMD_ULONG                 *pulSumSize,
                                  CSMD_VOID                  *pvBase,
                                  CSMD_VOID                 **ppvAllocMem );

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_INIT */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

23 Jan 2014
  - File created.
21 Feb 2014
 - FEAT-00049179
   Dynamic memory allocation.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.

------------------------------------------------------------------------------
*/
