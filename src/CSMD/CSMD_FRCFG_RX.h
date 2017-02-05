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
\file   CSMD_FRCFG_RX.h
\author WK
\date   03.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_FRCFG_RX.c
*/


#ifndef _CSMD_FRCFG_RX
#define _CSMD_FRCFG_RX

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! \cond PRIVATE */

/*---------------------------------------------------- */
/* FPGA Rx descriptor type definitions                 */
/*---------------------------------------------------- */

#define  CSMD_RX_DES_TYPE_SVDSP      0U    /* 0000b  Service channel data start posiition                */
#define  CSMD_RX_DES_TYPE_SVDEP      1U    /* 0001b  Service channel data end posiition                  */
#define  CSMD_RX_DES_TYPE_RTDSP      2U    /* 0010b  Real-time data start posiition                      */
#define  CSMD_RX_DES_TYPE_RTDEP      3U    /* 0011b  Real-time data end posiition                        */
#define  CSMD_RX_DES_TYPE_FCSP       4U    /* 0100b  FCS posiition; End of transmission                  */
/* since ip core 4v2 */
#define  CSMD_RX_DES_TYPE_PRELSP     8U    /* 1000b  Port relative start position (cross comm.), 
                                                     written to TxRAM not RxRAM!                         */
#define  CSMD_RX_DES_TYPE_PRELEP     9U    /* 1001b  Port relative end position (cross comm.), 
                                                     written to TxRAM not RxRAM!                         */
/* since ip core 4v7 */
#define  CSMD_RX_DES_TYPE_PRELCCSP  10U    /* 1010b  RTDSP Real-time data start position (cross comm.), 
                                                     copied also to TxRAM with buffer base pointer
                                                     of port relative Buffer but buffer offset of RTD!   */
#define  CSMD_RX_DES_TYPE_PRELCCEP  11U    /* 1011b  RTDSP Real-time data end position (cross comm.), 
                                                     copied also to TxRAM with buffer base pointer
                                                     of port relative Buffer but buffer offset of RTD!   */
#define  CSMD_RX_DES_TYPE_PRDFDSP   12U    /* 1100b  Like 1000b but sets C-CON Bit 11 (data field delay) */
#define  CSMD_RX_DES_TYPE_PRDFDEP   13U    /* 1101b  Like 1001b but sets C-CON Bit 11 (data field delay) */
#define  CSMD_RX_DES_TYPE_PRCCDFDSP 14U    /* 1110b  Like 1010b but sets C-CON Bit 11 (data field delay) */
#define  CSMD_RX_DES_TYPE_PRCCDFDEP 15U    /* 1111b  Like 1011b but sets C-CON Bit 11 (data field delay) */

/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

/* Configuration of receive telegram for CP0 */
SOURCE CSMD_FUNC_RET CSMD_Config_RX_Tel_P0
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Configuration of receive telegram for CP1 */
SOURCE CSMD_FUNC_RET CSMD_Config_RX_Tel_P1
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Configuration of receive telegram for CP2 */
SOURCE CSMD_FUNC_RET CSMD_Config_RX_Tel_P2
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Configuration of receive telegram for CP3 */
SOURCE CSMD_FUNC_RET CSMD_Config_RX_Tel_P3
                                ( CSMD_INSTANCE             *prCSMD_Instance );

#ifdef CSMD_PCI_MASTER
/* Set configuration of one PCI TX DMA channel
   (Source:FPGA Rx ram / Destination: Host ram) */
SOURCE CSMD_VOID CSMD_Config_DMA_Tx_Channel
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_ULONG                 ulSrcAdd_S3Ram,
                                  CSMD_ULONG                 ulDstAdd_Host,
                                  CSMD_ULONG                 ulLength,
                                  CSMD_USHORT                usTxChannelNbr );
#endif

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_FRCFG_RX */




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
