/*
CoSeMa V6.1 - Common Sercos Master function library
Copyright (c) 2004 - 2016  Bosch Rexroth AG

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
\file   CSMD_PHASE.h
\author WK
\date   03.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_PHASE.c
*/


#ifndef _CSMD_PHASE
#define _CSMD_PHASE

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! \cond PRIVATE */

#define CSMD_PHASE0_FILL                    0xFFFFU

#define CSMD_JITTER_AT_START                   (80)   /* Jitter of start sending AT relating to start sending MDT */


#define CSMD_CPS_MAX_TIMEOUT                (200UL)

#ifdef CSMD_SWC_EXT
#define CSMD_CPS_SLEEP_TIME                 ( 10UL)
#else
#define CSMD_CPS_SLEEP_TIME                 CSMD_CPS_MAX_TIMEOUT
#endif

#define CSMD_CPS_TIMEOUT_CNT                (CSMD_CPS_MAX_TIMEOUT / CSMD_CPS_SLEEP_TIME)


/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

/* Checks whether AT's still remaining at CP0 */
SOURCE CSMD_FUNC_RET CSMD_CheckPhase0
                                ( CSMD_INSTANCE             *prCSMD_Instance, 
                                  CSMD_BOOL                  boCheckProgrammed );

/* Calculate Timing for the UC channel in CP1 and CP2 */
SOURCE CSMD_VOID CSMD_Calculate_Timing_CP1_CP2
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Check the the S-DEV.SlaveValid of all recognized slaves at start of CP1 */
SOURCE CSMD_FUNC_RET CSMD_Check_S_DEV_Start_CP1
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Check the the S-SVC.SlaveValid of all recognized slaves at start of CP1 */
SOURCE CSMD_FUNC_RET CSMD_Check_S_SVC_Start_CP1
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Check whether all required SVC RT valid bits are enabled or disabled. */
SOURCE CSMD_FUNC_RET CSMD_CheckPhase1
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boShouldSend );

/* Check whether all required device status RT valid bits are enabled or disabled. */
SOURCE CSMD_FUNC_RET CSMD_CheckPhase2
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boShouldSend );

/* Check if all drives are switched off */
SOURCE CSMD_FUNC_RET CSMD_Finish_Phase_Check
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SHORT                 sPhase );

/* Start the new communication phase */
SOURCE CSMD_FUNC_RET CSMD_Start_New_Phase_Prepare
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SHORT                 sPhase );

/* Check if all drives are switched ON */
SOURCE CSMD_FUNC_RET CSMD_Start_New_Phase_Check
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SHORT                 sPhase );

#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
/* Check received SVC status of emulated service channels. */
SOURCE CSMD_BOOL CSMD_IsReady_Soft_SVC_Status
                                ( const CSMD_INSTANCE       *prCSMD_Instance );

/* Initialize SVC status of emulated service channels. */
SOURCE CSMD_VOID CSMD_Init_Soft_SVC
                                ( const CSMD_INSTANCE       *prCSMD_Instance );
#endif  /* #if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER */

/* Check whether all required device status RT valid bits are enabled or disabled. */
SOURCE CSMD_FUNC_RET CSMD_CheckPhase34
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boShouldSend );

SOURCE CSMD_FUNC_RET CSMD_CP_TransitionCheck
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro,
                                  CSMD_ULONG                 ulIdentNbr );

/* Proceed SYNC delay measuring procedure command to all used slaves */
SOURCE CSMD_FUNC_RET CSMD_Proceed_Cmd_S_0_1024
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_PHASE */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

03 Sep 2010
  - File created.
13 Nov 2014 WK
 - Defdb00000000
   Removed test code for defined CSMD_DEBUG.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
14 Aug 2015 WK
  - Defdb00181153
    Added prototype of function CSMD_IsReady_Soft_SVC_Status().
28 Jan 2016 WK
  - Defdb00182067
    CSMD_Check_S_DEV_Start_CP1(): Static declaration removed.
18 Apr 2016 WK
  - Defdb00182067
    CSMD_Init_Soft_SVC(), CSMD_IsReady_Soft_SVC_Status()
    Fixed possible Lint warnings 661.

------------------------------------------------------------------------------
*/
