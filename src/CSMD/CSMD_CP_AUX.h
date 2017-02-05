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
\file   CSMD_CP_AUX.h
\author WK
\date   03.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_CP_AUX.c
*/




#ifndef _CSMD_CP_AUX
#define _CSMD_CP_AUX

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! \cond PRIVATE */

/*---------------------------------------------------- */
/* Communication Version Field   (for MDT0 in CP0)     */
/*---------------------------------------------------- */
#define CSMD_COMMVER_LAST_FORW_OFF    0x00400000U   /* Bit    22:  1 = loopback without forward of last slave in line */
#define CSMD_COMMVER_FAST_CP_SWITCH   0x00200000U   /* Bit    21:  1 = fast communication phase witch                 */
#define CSMD_COMMVER_COMM_PAR_CP0     0x00100000U   /* Bit    20:  1 = transmission of t1, t6 and t7 in MDT0 of CP0   */
#define CSMD_COMMVER_CP12_2MDT_2AT    0x00000000U   /* Bit 17-16: 00 = 2 MDTs and 2 ATs  (up to 255 slaves)           */
#define CSMD_COMMVER_CP12_4MDT_4AT    0x00010000U   /* Bit 17-16: 01 = 4 MDTs and 4 ATs  (up to 511 slaves)           */
#define CSMD_COMMVER_ADD_ALLOC        0x00000001U   /* Bit     0:  1 = address allocation (Sercos version >= 1.1.1)   */


#define  CSMD_SERC3_HS_TIMEOUT      ( 10)       /*!< handshake timeout [Sercos cycles] */


/*---- Declaration private Types: --------------------------------------------*/

#ifdef CSMD_DEBUG
/* Note: maybe that this structure must be declared via define in reverse     */
/*       order for other compilers !!!                                        */
typedef struct CSMD_SVC_STATUSBITS_STR        /* Service Channel Status       */
{                                             /*                              */
  CSMD_USHORT    bfBit4to15    :12;           /*!< Bit 4-15: undefined        */
  CSMD_USHORT    btValid       : 1;           /*!< Bit 3:    SVC-Valid        */
  CSMD_USHORT    btError       : 1;           /*!< Bit 2:    Error            */
  CSMD_USHORT    btBusy        : 1;           /*!< Bit 1:    BUSY             */
  CSMD_USHORT    btHandshake   : 1;           /*!< Bit 0:    Ackn. Handshake  */
  
} CSMD_SVC_STATUSBITS;


/* Note: maybe that this structure must be declared via define in reverse     */
/*       order for other compilers !!!                                        */
typedef struct CSMD_SVC_CONTROLBITS_STR       /* Service Channel Control      */
{                                             /*                              */
  CSMD_USHORT    bfBit3to15    :10;           /*!< Bit 6-15: undefined        */
  CSMD_USHORT    btElement     : 3;           /*!< Bit 3-5:  Element          */
  CSMD_USHORT    btLastTran    : 1;           /*!< Bit 2:    Last transmission*/
  CSMD_USHORT    btWrite       : 1;           /*!< Bit 1:    Write operation  */
  CSMD_USHORT    btHandshake   : 1;           /*!< Bit 0:    Master Handshake */
  
} CSMD_SVC_CONTROLBITS;
#endif


#ifdef CSMD_PACK_PRAGMA
#pragma pack(push,2)
#endif

union CSMD_SVC_CONTROL_UN
{                                             /* Service Channel Control as   */
#ifdef CSMD_DEBUG
  CSMD_SVC_CONTROLBITS        rBit;           /*!<   - as bit structure       */
#endif
  CSMD_USHORT                 usWord;         /*!<   - as word                */

} /*lint !e659  (Nothing follows '}' on line terminating struct/union/enum definition) */
#ifdef CSMD_PACK_ATTRIBUTE
__attribute__((packed))
#endif
;
#ifdef CSMD_PACK_PRAGMA
#pragma pack(pop)
#endif
typedef union  CSMD_SVC_CONTROL_UN  CSMD_SVC_CONTROL_UNION;


#ifdef CSMD_PACK_PRAGMA
#pragma pack(push,2)
#endif

struct CSMD_SVC_FIELD_AT_STR                    /* AT-Service-Channel           */
{                                               /* description                  */
  union                                         /*                              */
  {                                             /* Service Channel Status as    */
#ifdef CSMD_DEBUG
    CSMD_SVC_STATUSBITS       rBit;           /*!<   - as bit structure       */ 
#endif
    CSMD_USHORT               usWord;         /*!<   - as word                */
  } rSvcStatus;
  
  CSMD_USHORT                 ausSvcData[2];  /*!< - Service Channel Data     */

} /*lint !e659  (Nothing follows '}' on line terminating struct/union/enum definition) */
#ifdef CSMD_PACK_ATTRIBUTE
__attribute__((packed))
#endif
;
#ifdef CSMD_PACK_PRAGMA
#pragma pack(pop)
#endif
typedef struct CSMD_SVC_FIELD_AT_STR  CSMD_SVC_FIELD_AT_STRUCT;


/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

/* Set Sercos communication version */
SOURCE CSMD_BOOL CSMD_SetComVersion
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_COM_VERSION           eComVersion );

/* Check Sercos cycle time for CP1 and CP2 */
SOURCE CSMD_FUNC_RET CSMD_CheckCycleTime_CP12
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_ULONG                 ulCycleTimeCP12 );

/* Get pointer to service channel of a slave inside the AT */
SOURCE CSMD_SVC_FIELD_AT_STRUCT * CSMD_GetAtServiceChannel
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_USHORT                usPort,
                                  CSMD_USHORT                usTadd );

/* Get pointer to service channel of a slave inside the MDT */
SOURCE CSMD_SVC_CONTROL_UNION * CSMD_GetMdtServiceChannel
                                ( CSMD_INSTANCE             *prCSMD_Instance, 
                                  CSMD_USHORT                usSlaveIndex );

/* Get MDT telegram index and position inside telegram */
SOURCE CSMD_VOID CSMD_GetTelegramPosition
                                ( CSMD_USHORT                usTAdd,
                                  CSMD_USHORT               *pusTelegramIndex,
                                  CSMD_USHORT               *pusPositionIndex );

/* To restore MHS bit properly on start CP3 */
SOURCE CSMD_VOID CSMD_SaveSVCControlMDTPhase2
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Prepare CYC_CLK for initialization in CSMD_SetPhaseX() */
SOURCE CSMD_VOID CSMD_SetCYCCLK ( CSMD_INSTANCE             *prCSMD_Instance );

/* Measure ringdelay on port 1 and/or Port 2 */
SOURCE CSMD_VOID CSMD_MeasureRingdelay
                                ( CSMD_INSTANCE             *prCSMD_Instance );

#if !defined CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
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

/* Clear telegram data */
SOURCE CSMD_VOID CSMD_Telegram_Clear
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_UCHAR                *pucTelegram,
                                  CSMD_USHORT                usTelLen,
                                  CSMD_USHORT                usFillData );

SOURCE CSMD_VOID CSMD_Write_CP0_MDT0
                                ( CSMD_UCC_MODE_ENUM             eActiveUCC_Mode_CP12,
                                  CSMD_ULONG                    *pulTxRam_CP0_MDT0,
                                  const CSMD_COM_VER_FIELD      *PrComVersion,
                                  const CSMD_TIMING_CP12_STRUCT *prTimingCP12 );

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_CP_AUX */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup

------------------------------------------------------------------------------
*/
