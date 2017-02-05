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
\file   CSMD_CALC.h
\author WK
\date   03.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_CALC.c
*/


#ifndef _CSMD_CALC
#define _CSMD_CALC

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! \cond PRIVATE */

/*---------------------------------------------------- */
/* Sercos Ethernet time constants [nano seconds]       */
/*---------------------------------------------------- */
#define CSMD_IPC_MIN_TIME           (125 * 1000)                      /* min. UC channel bandwidth [ns] */
#define CSMD_BIT_TIME               10UL                              /* 10 ns. at 100Mbit/s data rate */
#define CSMD_BYTE_TIME              (8UL * CSMD_BIT_TIME)             /* 80 ns. at 100Mbit/s data rate */

#define CSMD_PREAMBLE_AND_SFD       8UL                               /* Preamble + StartFrameDelimiter        (7+1 Byte) */
#define CSMD_DEST_SRC_ADDR          12UL                              /* Destination and source MAC addresses  (6+6 Byte) */
#define CSMD_ETHERTYPE              2UL                               /* Ether type                              (2 Byte) */
#define CSMD_MST                    6UL                               /* Sercos III header                     (2+4 Byte) */
#define CSMD_FCS                    4UL                               /* Frame check sequence (32-bit CRC)       (4 Byte) */

/* The telegram header beginning of the preamble to the end of MST (TTref)   (28 byte) */
#define CSMD_SIII_HEADER            (  CSMD_PREAMBLE_AND_SFD \
                                     + CSMD_DEST_SRC_ADDR    \
                                     + CSMD_ETHERTYPE        \
                                     + CSMD_MST)


/* tTH [ns] (Delay time from beginning of the preamble to the end of Sercos III header = 2240 ns) */
#define CSMD_TIME_SIII_HEADER       (CSMD_SIII_HEADER * CSMD_BYTE_TIME)

/* Sercos telegram overhead consisting of tTH + FrameCheckSequence (32 Byte) = 2560 ns plus variable InterFrameGap */
#define CSMD_MEDIA_LAYER_OVERHEAD   (  (CSMD_SIII_HEADER + CSMD_FCS)\
                                     * CSMD_BYTE_TIME)

#define CSMD_FPGA_LOGIC_DELAY       (220)                             /* FPGA Logic Delay [ns] */
#define CSMD_FPGA_DELAY_IP_WIN_OPEN (200)                             /* IP-Core delay for event CSMD_EVENT_IP_CHANNEL_TX_OPEN */

#define CSMD_DELAY_MDT              (  CSMD_FPGA_LOGIC_DELAY          /* Delay between TCNT event "Start MDT" and End of MDT0_MST */\
                                     + CSMD_TIME_SIII_HEADER)         /* (220 + 28*80 = 220 + 2240 = 2460 ns              */

#define CSMD_MST_DELAY              (  CSMD_FPGA_MDT_START_TIME       /* Delay between TCNT start.and End of MDT0_MST     */\
                                     + CSMD_DELAY_MDT)                /* 100 ns + (2240 ns + 220 ns) */



#define CSMD_ETHERNET_II_HEADER     (  CSMD_DEST_SRC_ADDR             /* DstAdd + SrcAdd + Ether Type           (14 Byte) */\
                                     + CSMD_ETHERTYPE)

#define CSMD_MIN_IFG                12UL                              /* Minimum required Inter Frame Gap       (12 Byte) */

#define CSMD_IP_MAC_LAYER_OVERHEAD  (  (  CSMD_PREAMBLE_AND_SFD       /*    8 */\
                                        + CSMD_ETHERNET_II_HEADER     /* + 14 */\
                                        + CSMD_FCS                    /* +  4 */\
                                        + CSMD_MIN_IFG)               /* + 12 = 38 Byte */\
                                     * CSMD_BYTE_TIME)

#define CSMD_ETHERNET_II_TELEGRAM   (  CSMD_IP_MAC_LAYER_OVERHEAD \
                                     + (  CSMD_ETHERNET_MTU_MAX   \
                                        * CSMD_BYTE_TIME))


#define CSMD_FPGA_HW_SVC_PROC_TIME              (400)   /* Processing time of fpga svc state machine for 1 svc [ns] */


/*---------------------------------------------------- */
/*    lookup table for Inter Frame Gap calculation     */
/*---------------------------------------------------- */
SOURCE const CSMD_USHORT ausSqrt_IFG[512] 
#ifdef SOURCE_CSMD
= {
    0,  1414,  2000,  2449,  2828,  3162,  3464,  3742,  4000,  4243,  4472,  4690,  4899,  5099,  5292,  5477,
 5657,  5831,  6000,  6164,  6325,  6481,  6633,  6782,  6928,  7071,  7211,  7348,  7483,  7616,  7746,  7874,
 8000,  8124,  8246,  8367,  8485,  8602,  8718,  8832,  8944,  9055,  9165,  9274,  9381,  9487,  9592,  9695,
 9798,  9899, 10000, 10100, 10198, 10296, 10392, 10488, 10583, 10677, 10770, 10863, 10954, 11045, 11136, 11225,
11314, 11402, 11489, 11576, 11662, 11747, 11832, 11916, 12000, 12083, 12166, 12247, 12329, 12410, 12490, 12570,
12649, 12728, 12806, 12884, 12961, 13038, 13115, 13191, 13266, 13342, 13416, 13491, 13565, 13638, 13711, 13784,
13856, 13928, 14000, 14071, 14142, 14213, 14283, 14353, 14422, 14491, 14560, 14629, 14697, 14765, 14832, 14900,
14967, 15033, 15100, 15166, 15232, 15297, 15362, 15427, 15492, 15556, 15620, 15684, 15748, 15811, 15875, 15937,
16000, 16062, 16125, 16186, 16248, 16310, 16371, 16432, 16492, 16553, 16613, 16673, 16733, 16793, 16852, 16912,
16971, 17029, 17088, 17146, 17205, 17263, 17321, 17378, 17436, 17493, 17550, 17607, 17664, 17720, 17776, 17833,
17889, 17944, 18000, 18055, 18111, 18166, 18221, 18276, 18330, 18385, 18439, 18493, 18547, 18601, 18655, 18708,
18762, 18815, 18868, 18921, 18974, 19026, 19079, 19131, 19183, 19235, 19287, 19339, 19391, 19442, 19494, 19545,
19596, 19647, 19698, 19748, 19799, 19849, 19900, 19950, 20000, 20050, 20100, 20149, 20199, 20248, 20298, 20347,
20396, 20445, 20494, 20543, 20591, 20640, 20688, 20736, 20785, 20833, 20881, 20928, 20976, 21024, 21071, 21119,
21166, 21213, 21260, 21307, 21354, 21401, 21448, 21494, 21541, 21587, 21633, 21679, 21726, 21772, 21817, 21863,
21909, 21954, 22000, 22045, 22091, 22136, 22181, 22226, 22271, 22316, 22361, 22405, 22450, 22494, 22539, 22583,
22627, 22672, 22716, 22760, 22804, 22847, 22891, 22935, 22978, 23022, 23065, 23108, 23152, 23195, 23238, 23281,
23324, 23367, 23409, 23452, 23495, 23537, 23580, 23622, 23664, 23707, 23749, 23791, 23833, 23875, 23917, 23958,
24000, 24042, 24083, 24125, 24166, 24207, 24249, 24290, 24331, 24372, 24413, 24454, 24495, 24536, 24576, 24617, 
24658, 24698, 24739, 24779, 24819, 24860, 24900, 24940, 24980, 25020, 25060, 25100, 25140, 25179, 25219, 25259,
25298, 25338, 25377, 25417, 25456, 25495, 25534, 25573, 25612, 25652, 25690, 25729, 25768, 25807, 25846, 25884,
25923, 25962, 26000, 26038, 26077, 26115, 26153, 26192, 26230, 26268, 26306, 26344, 26382, 26420, 26458, 26495,
26533, 26571, 26608, 26646, 26683, 26721, 26758, 26796, 26833, 26870, 26907, 26944, 26981, 27019, 27055, 27092,
27129, 27166, 27203, 27240, 27276, 27313, 27350, 27386, 27423, 27459, 27495, 27532, 27568, 27604, 27641, 27677,
27713, 27749, 27785, 27821, 27857, 27893, 27928, 27964, 28000, 28036, 28071, 28107, 28142, 28178, 28213, 28249,
28284, 28320, 28355, 28390, 28425, 28460, 28496, 28531, 28566, 28601, 28636, 28671, 28705, 28740, 28775, 28810,
28844, 28879, 28914, 28948, 28983, 29017, 29052, 29086, 29120, 29155, 29189, 29223, 29257, 29292, 29326, 29360,
29394, 29428, 29462, 29496, 29530, 29563, 29597, 29631, 29665, 29698, 29732, 29766, 29799, 29833, 29866, 29900,
29933, 29967, 30000, 30033, 30067, 30100, 30133, 30166, 30199, 30232, 30265, 30299, 30332, 30364, 30397, 30430,
30463, 30496, 30529, 30561, 30594, 30627, 30659, 30692, 30725, 30757, 30790, 30822, 30854, 30887, 30919, 30952,
30984, 31016, 31048, 31081, 31113, 31145, 31177, 31209, 31241, 31273, 31305, 31337, 31369, 31401, 31432, 31464,
31496, 31528, 31559, 31591, 31623, 31654, 31686, 31718, 31749, 31780, 31812, 31843, 31875, 31906, 31937, 31969
}
#endif
; 

/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

SOURCE CSMD_FUNC_RET CSMD_CheckConfiguration
                                ( CSMD_INSTANCE             *prCSMD_Instance );

#ifdef CSMD_HW_WATCHDOG
SOURCE CSMD_FUNC_RET CSMD_Determine_Watchdog_Mode
                                ( CSMD_INSTANCE             *prCSMD_Instance );
#endif

SOURCE CSMD_FUNC_RET CSMD_CalculateTelegramAssignment
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_FUNC_RET CSMD_CalculateTimingMethod
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usTimingMethod );

/* sort list of connections which are either produced or consumed by the master in order to create an access list without gaps */
SOURCE CSMD_VOID CSMD_Sort_Master_Connections
                                ( CSMD_INSTANCE             *prCSMD_Instance);

/* initialize private connection structures for connection state machine and determine producer index of all slaveproduced connections */
SOURCE CSMD_VOID CSMD_Init_Priv_Conn_Structs
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* build lists for determination whether a connection has to be produced in a certain cycle */
SOURCE CSMD_VOID CSMD_Calculate_Producer_Cycles
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* build list of different producer cycle times in ascending order */
SOURCE CSMD_FUNC_RET CSMD_Build_Producer_Cycle_Times_List
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_Calc_Max_TSref
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_USHORT               *pusTSrefMax );

#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
/* Determination of S-0-1015 "Ring delay" */
SOURCE CSMD_FUNC_RET CSMD_Determination_Ringdelay
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_RD_CALC_MODE          eRD_Mode,
                                  CSMD_ULONG                *pulS_0_1015_P1,
                                  CSMD_ULONG                *pulS_0_1015_P2 );

/* Calculation of stable ring delay */
SOURCE CSMD_VOID CSMD_Calculate_RingDelay
                                ( CSMD_INSTANCE             *prCSMD_Instance );
#endif

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_CALC */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
26 Nov 2013 WK
  Defdb00000000
  - Added prototypes of function CSMD_CalculateTelegramAssignment() and
    CSMD_CalculateTimingMethod().
05 Aug 2014 AlM
  - Removed prototype of function CSMD_BuildConnectionList()
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
23 Jul 2015 WK
  - Adjust value of CSMD_FPGA_LOGIC_DELAY.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
  
------------------------------------------------------------------------------
*/
