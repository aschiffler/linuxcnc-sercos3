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
\file   CSMD_TOPOLOGY.h
\author WK
\date   01.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_TOPOLOGY.c
*/


#ifndef _CSMD_TOPOLOGY
#define _CSMD_TOPOLOGY

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif


/*! \cond PRIVATE */

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

#define CSMD_RING_DEF_PRIMARY         1U  /* Ring defect on Primary-Line    */
#define CSMD_RING_DEF_SECONDARY       2U  /* Ring defect on Secondary-Line  */

/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

SOURCE CSMD_FUNC_RET CSMD_LineBreakMngmnt
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_SearchLineDefectPoint
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_SearchLineBreakPoint
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_FUNC_RET CSMD_CheckTopology
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_SetC_DevTopology
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usDevTopology,
                                  CSMD_USHORT                usDevAddr );

SOURCE CSMD_VOID CSMD_SVCMngmnt4LineBreak
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_SetCollisionBuffer
                                ( const CSMD_INSTANCE       *prCSMD_Instance );

SOURCE CSMD_FUNC_RET CSMD_Check_Telegram_Errors
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_FUNC_RET CSMD_Evaluate_TGSR
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usFirstPort,
                                  CSMD_USHORT                usSecondPort );


#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_TOPOLOGY */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
  
------------------------------------------------------------------------------
*/
