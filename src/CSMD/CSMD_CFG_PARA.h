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
\file   CSMD_CFG_PARA.h 
\author WK
\date   03.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_CFG_PARA.c
*/


#ifndef _CSMD_CFG_PARA
#define _CSMD_CFG_PARA

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

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

SOURCE CSMD_FUNC_RET CSMD_Transmit_SCP_Basic
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_FUNC_RET CSMD_Transmit_SCP_VarCFG
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_FUNC_RET CSMD_Transmit_SCP_Sync
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_FUNC_RET CSMD_Transmit_SCP_NRT
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_FUNC_RET CSMD_Transmit_SCP_RTB
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_FUNC_RET CSMD_ReadConfig
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

#ifndef CSMD_DISABLE_CMD_S_0_1048
/* Proceed "Activate network settings" procedure command to all used slaves */
SOURCE CSMD_FUNC_RET CSMD_Proceed_Cmd_S_0_1048
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );
#endif

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_CFG_PARA */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

03 Sep 2010
  - File created.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
  
------------------------------------------------------------------------------
*/
