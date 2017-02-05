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
\file   CSMD_FRCFG_TX.h
\author WK
\date   03.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_FRCFG_TX.c
*/


#ifndef _CSMD_FRCFG_TX
#define _CSMD_FRCFG_TX

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! \cond PRIVATE */

/*---------------------------------------------------- */
/* FPGA Tx descriptor type definitions                 */
/*---------------------------------------------------- */

#define CSMD_TX_DES_TYPE_SVDSP       0U    /* 0000b  Service channel data start posiition        */
#define CSMD_TX_DES_TYPE_SVDEP       1U    /* 0001b  Service channel data end posiition          */
#define CSMD_TX_DES_TYPE_RTDSP       2U    /* 0010b  Real-time data start posiition              */
#define CSMD_TX_DES_TYPE_RTDEP       3U    /* 0011b  Real-time data end posiition                */
#define CSMD_TX_DES_TYPE_FCSP        4U    /* 0100b  FCS posiition; End of transmission          */
#define CSMD_TX_DES_TYPE_PRELSP      8U    /* 1000b  Port relative start posiition               */
#define CSMD_TX_DES_TYPE_PRELEP      9U    /* 1001b  Port relative end posiition                 */
/* since ip core 4v5 */
#define CSMD_TX_DES_TYPE_PRELCCSP   10U    /* 1010b  Port relative start position (cross comm.), 
                                                     filled with zeros after according AT error  */
#define CSMD_TX_DES_TYPE_PRELCCEP   11U    /* 1011b  Port relative end position (cross comm.),
                                                     filled with zeros after according AT error  */



#define CSMD_SEQCNT_FIELDWIDTH       2U    /* Size of Sequence Counter field in AT0 [byte] */

/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

/* Configuration of transmit telegram forCP0 */
SOURCE CSMD_FUNC_RET CSMD_Config_TX_Tel_P0
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Configuration of transmit telegram forCP1 */
SOURCE CSMD_FUNC_RET CSMD_Config_TX_Tel_P1
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Configuration of transmit telegram forCP2 */
SOURCE CSMD_FUNC_RET CSMD_Config_TX_Tel_P2
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Configuration of transmit telegram forCP3 */
SOURCE CSMD_FUNC_RET CSMD_Config_TX_Tel_P3
                                ( CSMD_INSTANCE             *prCSMD_Instance );

#ifdef CSMD_PCI_MASTER
/* Set configuration of one PCI RX DMA channel 
   (Source:Host ram / Destination: FPGA Tx ram) */
SOURCE CSMD_VOID CSMD_Config_DMA_Rx_Channel
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_ULONG                 ulDstAdd_S3Ram,
                                  CSMD_ULONG                 ulSrcAdd_Host,
                                  CSMD_ULONG                 ulLength,
                                  CSMD_USHORT                usRxChannelNbr );
#endif

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_FRCFG_TX */




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
