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
\file   CSMD_BM_CFG.h
\author WK
\date   03.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_BM_CFG.c
*/




#ifndef _CSMD_BM_CFG
#define _CSMD_BM_CFG

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! \cond PRIVATE */

#define CSMD_JITTER_MASTER_CP0_2            (50U * 1000U)   /* Master Jitter for Timing Slave Mode in CP0 to CP2 */
#define CSMD_FPGA_MST_WINDOW_P0_2           (50U * 1000U)   /* MST window time width */


/*---------------------------------------------------- */
/* FPGA timer event types                              */
/*---------------------------------------------------- */
#define CSMD_NO_EVENT                            0U
#define CSMD_EVENT_TINT_0                        1U
#define CSMD_EVENT_TINT_1                        2U
#define CSMD_EVENT_TINT_2                        3U
#define CSMD_EVENT_TINT_3                        4U
#define CSMD_EVENT_SYNC_SET                      5U
#define CSMD_EVENT_SYNC_RESET                    6U
#define CSMD_EVENT_START_MDT                     7U
#define CSMD_EVENT_START_AT                      8U
#define CSMD_EVENT_IP_CHANNEL_TX_OPEN            9U
#define CSMD_EVENT_IP_CHANNEL_TX_LAST_TRANS     10U
#define CSMD_EVENT_IP_CHANNEL_TX_CLOSE          11U
#define CSMD_EVENT_RELOAD                       12U
#define CSMD_EVENT_RELOAD_VALUE                 13U
#define CSMD_EVENT_TX_BUFREQ_BUFSYS_A           14U
#define CSMD_EVENT_TX_BUFREQ_BUFSYS_B           15U
/*---------------------------------------------------- */
/* FPGA port timer event types                         */
/*---------------------------------------------------- */
#define CSMD_NO_PORT_EVENT                       0U
#define CSMD_PORT_EVENT_IP_CHANNEL_RX_OPEN       1U
#define CSMD_PORT_EVENT_2_reserved               2U
#define CSMD_PORT_EVENT_IP_CHANNEL_RX_CLOSE      3U
#define CSMD_PORT_EVENT_AT_WINDOW_OPEN           4U
#define CSMD_PORT_EVENT_AT_WINDOW_CLOSE          5U
#define CSMD_PORT_EVENT_RX_BUFREQ_BUFSYS_A       6U
#define CSMD_PORT_EVENT_RX_BUFREQ_BUFSYS_B       7U
#define CSMD_PORT_EVENT_8_reserved               8U
#define CSMD_PORT_EVENT_9_reserved               9U
#define CSMD_PORT_EVENT_MST_WINDOW_OPEN         10U
#define CSMD_PORT_EVENT_MST_WINDOW_CLOSE        11U
#define CSMD_PORT_EVENT_RELOAD_VALUE            12U
#define CSMD_PORT_EVENT_START_SVC               13U   /* Trigger service channel processor */

/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

/* Set Register preparing CP0 */
SOURCE CSMD_FUNC_RET CSMD_Set_Register_P0
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Set Register preparing CP1 */
SOURCE CSMD_FUNC_RET CSMD_Set_Register_P1
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Set Register preparing CP3 */
SOURCE CSMD_FUNC_RET CSMD_Set_Register_P3
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_FUNC_RET CSMD_Transmit_Timer_Events
                                ( CSMD_INSTANCE             *prCSMD_Instance, 
                                  CSMD_EVENT                *parTimerEvent,
                                  CSMD_USHORT                usNumber );

SOURCE CSMD_FUNC_RET CSMD_Transmit_Port_Events
                                ( CSMD_INSTANCE             *prCSMD_Instance, 
                                  CSMD_EVENT                *parPortEvent,
                                  CSMD_USHORT                usNumber );

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_BM_CFG */




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
