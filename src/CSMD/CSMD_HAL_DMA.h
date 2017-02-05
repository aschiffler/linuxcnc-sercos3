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
\file   CSMD_HAL_DMA.h
\author AM
\date   03.09.2007
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_HAL_DMA.c
*/


#ifndef _CSMD_HAL_DMA
#define _CSMD_HAL_DMA

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif


/*! \cond HAL_DMA */

#ifdef CSMD_PCI_MASTER

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/* DMA Control / Status Register */
#ifdef CSMD_BIG_ENDIAN
#define CSMD_HAL_DMA_STAT_BUSY      (0x00000100)    /* Bit 16: DMA Unit is busy.       */
#define CSMD_HAL_DMA_STAT_COMPLETE  (0x00000200)    /* Bit 17: DMA Transfer completed. */
#define CSMD_HAL_DMA_STAT_ERROR     (0x00000400)    /* Bit 18: DMA Error.              */
#else
#define CSMD_HAL_DMA_STAT_BUSY      (0x00010000)    /* Bit 16: DMA Unit is busy.       */
#define CSMD_HAL_DMA_STAT_COMPLETE  (0x00020000)    /* Bit 17: DMA Transfer completed. */
#define CSMD_HAL_DMA_STAT_ERROR     (0x00040000)    /* Bit 18: DMA Error.              */
#endif

/* Shift Values for DMA Register */
/* #define CSMD_HAL_DMA_SHIFT_DIR      (4)
   #define CSMD_HAL_DMA_SHIFT_START    (0)
   #define CSMD_HAL_DMA_SHIFT_SEL_REG  (8) */
#define CSMD_HAL_DMA_SHIFT_START_RX   (16)

/* DMA channel numberss for Tx- and Rx-DMA */
typedef enum CSMD_DMA_CMD_CHANNEL_NBR_EN
{
  CSMD_DMA_CHANNEL_00 = 0,
  CSMD_DMA_CHANNEL_01,
  CSMD_DMA_CHANNEL_02,
  CSMD_DMA_CHANNEL_03,
  CSMD_DMA_CHANNEL_04,
  CSMD_DMA_CHANNEL_05,
  CSMD_DMA_CHANNEL_06,
  CSMD_DMA_CHANNEL_07,
/*CSMD_DMA_CHANNEL_08,
  CSMD_DMA_CHANNEL_09,
  CSMD_DMA_CHANNEL_10,
  CSMD_DMA_CHANNEL_11,
  CSMD_DMA_CHANNEL_12,
  CSMD_DMA_CHANNEL_13,
  CSMD_DMA_CHANNEL_14,
  CSMD_DMA_CHANNEL_15,*/
  CSMD_DMA_NBR_USED_CHANNELS

} CSMD_DMA_CMD_CHANNEL_NBR;


/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

SOURCE CSMD_BOOL CSMD_HAL_CheckDMA_PCIVersion
                                ( const CSMD_HAL *prCSMD_HAL );

SOURCE CSMD_VOID CSMD_HAL_StartTxDMA
                                ( const CSMD_HAL *prCSMD_HAL,
                                  CSMD_USHORT     usChannels );

SOURCE CSMD_VOID CSMD_HAL_Enable_RXDMA
                                ( const CSMD_HAL *prCSMD_HAL,
                                  CSMD_USHORT     usChannels );

SOURCE CSMD_VOID CSMD_HAL_StartRxDMA
                                ( const CSMD_HAL *prCSMD_HAL,
                                  CSMD_USHORT     usChannels );

SOURCE CSMD_VOID CSMD_HAL_SetDMALocalAddr
                                ( const CSMD_HAL *prCSMD_HAL,
                                  CSMD_USHORT     usSelDirection,
                                  CSMD_USHORT     usSelChannelNbr,
                                  CSMD_ULONG      ulAddr );

SOURCE CSMD_VOID CSMD_HAL_SetDMAPCIAddr
                                ( const CSMD_HAL *prCSMD_HAL,
                                  CSMD_USHORT     usSelDirection,
                                  CSMD_USHORT     usSelChannelNbr,
                                  CSMD_ULONG      ulAddr );

SOURCE CSMD_VOID CSMD_HAL_SetDMACounter
                                ( const CSMD_HAL *prCSMD_HAL,
                                  CSMD_USHORT     usSelDirection,
                                  CSMD_USHORT     usSelChannelNbr,
                                  CSMD_ULONG      ulCount );

SOURCE CSMD_VOID CSMD_HAL_SetDMARDYAddr
                                ( const CSMD_HAL *prCSMD_HAL,
                                  CSMD_USHORT     usSelDirection,
                                  CSMD_USHORT     usSelChannelNbr,
                                  CSMD_ULONG      ulAddr );

SOURCE CSMD_VOID CSMD_HAL_ResetTXDMA
                                ( const CSMD_HAL *prCSMD_HAL );
                            
SOURCE CSMD_VOID CSMD_HAL_ResetRXDMA
                                ( const CSMD_HAL *prCSMD_HAL );

#ifdef __cplusplus
} // extern "C"
#endif

#endif  /* #ifdef CSMD_PCI_MASTER */

/*! \endcond */ /* HAL_DMA */

#endif /* _CSMD_HAL_DMA */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

02 Sep 2010
  - File created.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
  
------------------------------------------------------------------------------
*/
