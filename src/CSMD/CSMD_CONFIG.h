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
\file   CSMD_CONFIG.h
\author WK
\date   03.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_CONFIG.c
*/


#ifndef _CSMD_CONFIG
#define _CSMD_CONFIG

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! \cond PRIVATE */

/*-------------------------------------------------------------------------- */
/* S-0-1000, List of SCP classes & version                                   */
/*-------------------------------------------------------------------------- */
#define CSMD_MASK_SCP_TYPE          0xFF00U
#define CSMD_MASK_SCP_VERSION       0x000FU
#define CSMD_MASK_SCP_VERSION_V1    0x0001U
#define CSMD_MASK_SCP_TYPE_VER      (CSMD_MASK_SCP_TYPE | CSMD_MASK_SCP_VERSION)


/*                       | Class               | | defined   | Description                                                     */
/*                       | Code | --- |Version | | in Sercos |                                                                 */
/*                  Bits:| 15-8 | 7-4 | 3-0    | | version   |                                                                 */

/*      CSMD_SCP_TYPE_reserved_00   0x00nn */     /* V1.n.n  - not used                                                        */

/* SCP Type: basic communication profile classes                                                                               */
#define CSMD_SCP_TYPE_FIXCFG        0x0101U       /* V1.1.1  - FIX ConFiGuration of connections                                */
#define CSMD_SCP_TYPE_FIXCFG_V2     0x0102U       /* V1.3    - FIX ConFiGuration of connections                                */
#define CSMD_SCP_TYPE_FIXCFG_V3     0x0103U       /* V1.3    - FIX ConFiGuration of connections & connection stop              */
#define CSMD_SCP_TYPE_VARCFG        0x0201U       /* V1.1.1  - VARiable ConFiGuration of homogeneous connections               */
#define CSMD_SCP_TYPE_VARCFG_V2     0x0202U       /* V1.3    - VARiable ConFiGuration of homogeneous connections               */
#define CSMD_SCP_TYPE_VARCFG_V3     0x0203U       /* V1.3    - VARiable ConFiGuration of homogeneous Conn. & connection stop   */

/* SCP Type: additional communication profile classes                                                                          */
#define CSMD_SCP_TYPE_SYNC          0x0301U       /* V1.1.1  - SYNChronisation                                                 */
#define CSMD_SCP_TYPE_SYNC_V2       0x0302U       /* V1.3    - SYNChronisation tSync > tScyc using MDT Extended field          */
#define CSMD_SCP_TYPE_SYNC_V3       0x0303U       /* V1.3    - SYNChronisation tSync > tScyc using MDT Extended field          */
#define CSMD_SCP_TYPE_WD            0x0401U       /* V1.1.1  - WatchDog of connection                                          */
/*      CSMD_SCP_TYPE_DIAG          0x0501U */    /* V1.1.1  - communication DIAGnoses                                         */
#define CSMD_SCP_TYPE_RTB           0x0601U       /* V1.1.1  - configuration of Real-Time Bits                                 */
/*      CSMD_SCP_TYPE_HP            0x0701U */    /* V1.1.1  - Hot-Plug                                                        */
/*      CSMD_SCP_TYPE_SMP           0x0801U */    /* V1.1.1  - Sercos Messaging Protocol                                       */
/*      CSMD_SCP_TYPE_MUX           0x0901U */    /* V1.1.1  - MUltipleX channel (standard data container)                     */
#define CSMD_SCP_TYPE_NRT           0x0A01U       /* V1.1.1  - UC channel (ip communication)                                   */
/*      CSMD_SCP_TYPE_SIG           0x0B01U */    /* V1.1.1  - word of real time bits as producer and consumer                 */
/*      CSMD_SCP_TYPE_LISTSEG       0x0C01U */    /* V1.3    - Segmented List transfer via the svc                             */
/*      CSMD_SCP_TYPE_SIP           0x0D01U */    /* V1.3    - Support of S/IP Sercos Internet Protocol using the UC channel   */
/*      CSMD_SCP_TYPE_TFTP          0x0E01U */    /* V1.3    - Support of TFTP in the UC channel                               */
#define CSMD_SCP_TYPE_CAP           0x0F01U       /* V1.3    - Connection Capabilities                                         */
/*      CSMD_SCP_TYPE_EXTMUX        0x1001U */    /* V1.1.2  - EXTended MUltipleX channel (extended data container)            */
/*      CSMD_SCP_TYPE_RTBLISTPROD   0x1101U */    /* V1.3    - List of real-time bits as producer (status)                     */
/*      CSMD_SCP_TYPE_RTBLISTCONS   0x1201U */    /* V1.3    - List of real-time bits as consumer (control)                    */
#define CSMD_SCP_TYPE_SYSTIME       0x1301U       /* V1.3    - set Sercos Time using MDT extended field                        */
/*      CSMD_SCP_TYPE_RTBWORDPROD   0x1401U */    /* V1.3    - Word of real-time bits as producer                              */
/*      CSMD_SCP_TYPE_RTBWORDCONS   0x1501U */    /* V1.3    - Word of real-time bits as consumer                              */
/*      CSMD_SCP_TYPE_SAFETYCON     0x1601U */    /* V1.3    - CIP safety on Sercos connections                                */
/*      CSMD_SCP_TYPE_OVS_BASIC     0x1701U */    /* V1.3    - Word of real time bits as consumer                              */
#define CSMD_SCP_TYPE_NRTPC         0x1801U       /* V1.3    - UC channel (IP communication)                                   */
#define CSMD_SCP_TYPE_CYC           0x1901U       /* V1.3    - Cyclic communication                                            */
#define CSMD_SCP_TYPE_WDCON         0x1A01U       /* V1.3    - WatchDog of connection with tPcyc & data losses                 */
#define CSMD_SCP_TYPE_SWC           0x1B01U       /* V1.3.1  - Support of Industrial Ethernet protocols via UCC                */
/*      CSMD_SCP_SYNC_CN            0x1C01U */    /* V1.3.1    Support of cascaded networks (addon to SCP_Sync)                */
/*      CSMD_SCP_TYPE_BASIC         0x1D01U */    /* V1.n.n    Summary of SCP_FixCFG and SCP_VarCFG without connection configuration */
/*      CSMD_SCP_TYPE_CP_STARTUP    0x1E01U */    /* V1.n.n    Support of standardized phase start-up                          */


/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

SOURCE CSMD_FUNC_RET CSMD_Get_S1000
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_FUNC_RET CSMD_Check_Active_SCP_Classes
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_FUNC_RET CSMD_SCP_PlausibilityCheck
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSlaveIdx );

SOURCE CSMD_FUNC_RET CSMD_SCP_SetDefaultClasses
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSlaveIdx );

SOURCE CSMD_FUNC_RET CSMD_Build_SCP_BitList
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_FUNC_RET  CSMD_Check_S1036
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_CONFIG */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
18 Feb 2013 WK
  Defdb00152992 / CR-00036584 - Adjust phase switching according to spec.1.3.1
  - Added defines for SCP classes SCP_SWC and SCP_SysTime.
26 Aug 2014 WK
  - Defdb FEAT-00051252 - Generation of configuration files for the Sercos Monitor
    Shifted mask definitions for SCP configuration bitlist to CSMD_GLOB.h.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.

------------------------------------------------------------------------------
*/
