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
 \file   CSMD_FRCFG_TX.c
 \author WK
 \date   01.09.2010
 \brief  This File contains the private functions for the
         transmit telegram configuration of all the communication phases.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"
#include "CSMD_HAL_DMA.h"

#include "CSMD_CP_AUX.h"
#include "CSMD_PHASE.h"
#include "CSMD_HOTPLUG.h"
#include "CSMD_PRIV_SVC.h"


#define SOURCE_CSMD
#include "CSMD_FRCFG_TX.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/*! \endcond */ /* PUBLIC */


/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Configures the transmit telegram for CP0.

\ingroup module_phase
\b Description: \n
   This function configures the transmit telegram forCP0, including:
   - Clearance of descriptor index table and buffer base pointer list
   - Configuration of MDT0 data (data field and FCS field)
   - Configuration of AT0 data
   - Transmission of new descriptor index table and buffer base pointer list
     to RxRam/corresponding FPGA register
   - Initialization of MDT0 and AT0 in TxRam

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         09.02.2005
    
***************************************************************************** */
CSMD_FUNC_RET CSMD_Config_TX_Tel_P0( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_DESCR_INDEX arTxDesTable[ CSMD_HAL_TX_DESR_INDEX_NUMBER ];
  volatile CSMD_ULONG            aulTxBufBasePtr[ CSMD_HAL_TX_BASE_PTR_NBR ];   /* Tx Buffer base pointer list */
  
  CSMD_ULONG   ulTxRamOffset;      /* Current byte offset in TxRam                  */
  CSMD_USHORT  usPort_Offset;      /* Current offset [byte] in port relative buffer */
  CSMD_USHORT  usBuff0_Offset;     /* Current offset [byte] in buffer 0             */
  CSMD_USHORT  usTelegramOffset;   /* Offset [byte] in current telegram             */
  CSMD_USHORT  usIdxTableOffs;     /* Tx-Descriptor table index offset in TxRam     */
  CSMD_USHORT  usBuffOffsMDT;      /* Start MDT0 in buffer 0 */
  CSMD_USHORT  usBuffOffsAT;       /* Start AT0 in buffer 0 */
  CSMD_UCHAR  *pucTel;             /* Pointer to current Telegram */
  CSMD_USHORT  usTelI;             /* Telegram Index */
  CSMD_USHORT  usI;                /* Index */
  
  
#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    /* Prepare for DMA in CP2 */
    CSMD_HAL_ResetRXDMA( &prCSMD_Instance->rCSMD_HAL );
  }
#endif  
  
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
  {                                                
    prCSMD_Instance->rPriv.aboMDT_used[usTelI]    = FALSE;      /* MDT telegram is not used */
    prCSMD_Instance->rPriv.aboAT_used[usTelI]     = FALSE;      /* AT telegram is not used  */
    
    prCSMD_Instance->rPriv.rMDT_Length[usTelI].usTel = 0U;
    prCSMD_Instance->rPriv.rMDT_Length[usTelI].usRTD = 0U;
    
    prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel  = 0U;
    prCSMD_Instance->rPriv.rAT_Length[usTelI].usRTD  = 0U;
  }
  
  ulTxRamOffset  = 0U;    /* Byte offset in TxRam     */
  
  /* Tx-Descriptor index table offset in TxRam */
  usIdxTableOffs = (CSMD_USHORT) ulTxRamOffset;
  CSMD_HAL_SetDesIdxTableOffsTxRam( &prCSMD_Instance->rCSMD_HAL, 
                                    usIdxTableOffs );
  
  /* ------------------------------------------------------------- */
  /* Clear Tx descriptor index table                               */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_TX_DESR_INDEX_NUMBER; usI++)
  {
    arTxDesTable[usI].ulIndex = 0;
  }
  
  /* ------------------------------------------------------------- */
  /* Clear Tx Buffer base pointer list                             */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_TX_BASE_PTR_NBR; usI++)
  {
    aulTxBufBasePtr[usI] = 0U;
  }
  
  ulTxRamOffset += sizeof (arTxDesTable);     /* Reserve space for 4 MDT and 4 AT index table entries */
  
  
  /* ------------------------------------------------------------- */
  /* Configure MDT0 data                                           */
  /* ------------------------------------------------------------- */
  usTelI   = 0U;   /* Telegram index   */
  
  /* Length of MDT */
  prCSMD_Instance->rPriv.rMDT_Length[usTelI].usTel = CSMD_SERC3_MDT_DATA_LENGTH_P0;
  
  /* Descriptor Index table entry for MDT0 */
  arTxDesTable[CSMD_DES_IDX_MDT0].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
  arTxDesTable[CSMD_DES_IDX_MDT0].ulIndex |= (CSMD_ULONG) ((ulTxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
  
  usTelegramOffset = 0U;  /* Offset [byte] in current telegram */
  usBuff0_Offset   = 0U;  /* Byte offset in Tx Buffer 0 */
  usBuffOffsMDT    = usBuff0_Offset;
  
  
  /* MDT0 data field */
  CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulTxRamOffset,                /* Descriptor offset in TxRam   */
                            usBuff0_Offset,               /* Buffer offset in byte        */
                            0U,                           /* Buffer System Select         */
                            usTelegramOffset,             /* Offset after CRC in telegram */
                            CSMD_TX_DES_TYPE_RTDSP );     /* Descriptor type              */
  ulTxRamOffset += 4;
  
  
  usBuff0_Offset   += prCSMD_Instance->rPriv.rMDT_Length[usTelI].usTel;
  usTelegramOffset += prCSMD_Instance->rPriv.rMDT_Length[usTelI].usTel;
  CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulTxRamOffset,                        /* Descriptor offset in TxRam   */
                            (CSMD_USHORT)(usBuff0_Offset - 2),    /* Buffer offset in byte        */
                            0U,                                   /* Buffer System Select         */
                            (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                            CSMD_TX_DES_TYPE_RTDEP );             /* Descriptor type              */
  ulTxRamOffset += 4;
  
  
  /* MDT0 FCS field */
  CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulTxRamOffset,                /* Descriptor offset in TxRam   */
                            0U,                           /* Buffer offset (irrelevant)   */
                            0U,                           /* Buffer System Select         */
                            usTelegramOffset,             /* Offset after CRC in telegram */
                            CSMD_TX_DES_TYPE_FCSP );      /* Descriptor type              */
  ulTxRamOffset += 4;
  
  
  /* ------------------------------------------------------------- */
  /*   Configure AT0 data                                          */
  /* ------------------------------------------------------------- */
  /* Length of AT0 */
  prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel = CSMD_SERC3_AT_DATA_LENGTH_P0_V10;
  
  /* Descriptor Index table entry for AT0 */
  arTxDesTable[CSMD_DES_IDX_AT0].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
  arTxDesTable[CSMD_DES_IDX_AT0].ulIndex |= (CSMD_ULONG) ((ulTxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
  
  usTelegramOffset = 0U;  /* Offset [byte] in current telegram */
  usPort_Offset    = 0U;  /* Byte offset in Tx Buffer */
  
  
  /* Sequence counter separately adjustable for port 1 and port 2 */
  CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulTxRamOffset,                /* Descriptor offset in TxRam   */
                            usPort_Offset,                /* Buffer offset in byte        */
                            0U,                           /* Buffer System Select         */
                            usTelegramOffset,             /* Offset after CRC in telegram */
                            CSMD_TX_DES_TYPE_PRELSP );    /* Descriptor type              */
  ulTxRamOffset += 4;
  
  
  usPort_Offset    += CSMD_SEQCNT_FIELDWIDTH;
  usTelegramOffset += CSMD_SEQCNT_FIELDWIDTH;
  CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulTxRamOffset,                        /* Descriptor offset in TxRam   */
                            (CSMD_USHORT) (usPort_Offset - 2),    /* Buffer offset in byte        */
                            0U,                                   /* Buffer System Select         */
                            (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                            CSMD_TX_DES_TYPE_PRELEP );            /* Descriptor type              */
  ulTxRamOffset += 4;
  
  
  usBuffOffsAT    = usBuff0_Offset;
  
  /* Topology address field #1 to 511 */
  CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulTxRamOffset,                /* Descriptor offset in TxRam   */
                            usBuff0_Offset,               /* Buffer offset in byte        */
                            0U,                           /* Buffer System Select         */
                            usTelegramOffset,             /* Offset after CRC in telegram */
                            CSMD_TX_DES_TYPE_RTDSP );     /* Descriptor type              */
  ulTxRamOffset += 4;
  
  
  usBuff0_Offset   += (CSMD_USHORT) (prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel - CSMD_SEQCNT_FIELDWIDTH);
  usTelegramOffset += (CSMD_USHORT) (prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel - CSMD_SEQCNT_FIELDWIDTH);
  CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulTxRamOffset,                        /* Descriptor offset in TxRam   */
                            (CSMD_USHORT) (usBuff0_Offset - 2),   /* Buffer offset in byte        */
                            0U,                                   /* Buffer System Select         */
                            (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                            CSMD_TX_DES_TYPE_RTDEP );             /* Descriptor type              */
  ulTxRamOffset += 4;
  
  
  /* MDT0 FCS field */
  CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulTxRamOffset,                /* Descriptor offset in TxRam   */
                            0U,                           /* Buffer offset (irrelevant)   */
                            0U,                           /* Buffer System Select         */
                            usTelegramOffset,             /* Offset after CRC in telegram */
                            CSMD_TX_DES_TYPE_FCSP );      /* Descriptor type              */
  ulTxRamOffset += 4;
  
  
  
  /* Round up to the next divisible by 16 */
  ulTxRamOffset = ((ulTxRamOffset + 15) / 16) * 16;
  prCSMD_Instance->rPriv.ulTxRamTelOffset = ulTxRamOffset;
  
  aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_0_SYS_A] = ulTxRamOffset;
  ulTxRamOffset = ((ulTxRamOffset + usBuff0_Offset + 3) / 4) * 4;
  
  aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_1] = ulTxRamOffset;
  ulTxRamOffset = ((ulTxRamOffset + usPort_Offset + 3) / 4) * 4;
  
  aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_2] = ulTxRamOffset;
  ulTxRamOffset = ((ulTxRamOffset + usPort_Offset + 3) / 4) * 4;
  
#ifdef CSMD_DEBUG
  prCSMD_Instance->rCSMD_Debug.aulTxBufSize[CSMD_HAL_IDX_TX_BUFF_0_SYS_A] = usBuff0_Offset;
  prCSMD_Instance->rCSMD_Debug.aulTxBufSize[CSMD_HAL_IDX_TX_BUFF_PORT_1]  = usPort_Offset;
  prCSMD_Instance->rCSMD_Debug.aulTxBufSize[CSMD_HAL_IDX_TX_BUFF_PORT_2]  = usPort_Offset;
#endif
  
  /* Used FPGA TxRam memory for Sercos telegrams */
  prCSMD_Instance->rCSMD_HAL.ulTxRamInUse = (((ulTxRamOffset - 1) / CSMD_IP_RAM_SEG_SIZE) + 1) * CSMD_IP_RAM_SEG_SIZE;
  
  prCSMD_Instance->rPriv.rAT_Length[usTelI].usRTD =
    prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel;
  
  
  /* ------------------------------------------------------------- */
  /* Transmit descriptor index table into FPGA TxRam               */
  /* ------------------------------------------------------------- */
  usIdxTableOffs >>= 2;   /* Offset [Longs] in TxRam */ /*lint !e845 The left argument to operator '>>' is certain to be 0  */
  
  for (usI = 0; usI < CSMD_HAL_TX_DESR_INDEX_NUMBER; usI++)
  {
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram[usIdxTableOffs],
                        (arTxDesTable[usI].ulIndex) );
    usIdxTableOffs++;
  }
  
  /* ------------------------------------------------------------- */
  /* Transmit Tx Buffer base pointer list into FPGA register       */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_TX_BASE_PTR_NBR; usI++)
  {
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->aulTxBufBasePtr[usI], 
                        aulTxBufBasePtr[usI] );
#ifdef CSMD_DEBUG
    prCSMD_Instance->rCSMD_Debug.aulTxBufBasePtr[usI] = aulTxBufBasePtr[usI];
#endif
  }
  
  
  /* ------------------------------------------------------------- */
  /* Initialize MDT0 in Tx Ram                                     */
  /* ------------------------------------------------------------- */
  prCSMD_Instance->rPriv.pulTxRam_CP0_MDT0 =
    (CSMD_ULONG *) prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram
    + (aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_0_SYS_A] + usBuffOffsMDT) / 4;  /*lint !e845 The right argument to operator '+' is certain to be 0  */
  
#ifdef CSMD_SWC_EXT
  CSMD_Write_CP0_MDT0( prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12,
                       prCSMD_Instance->rPriv.pulTxRam_CP0_MDT0,
                       &prCSMD_Instance->rPriv.rComVersion,
                       &prCSMD_Instance->rPriv.rTimingCP12 );
#else
  CSMD_Write_CP0_MDT0( prCSMD_Instance->rPriv.rSWC_Struct.eActiveUCC_Mode_CP12,
                       prCSMD_Instance->rPriv.pulTxRam_CP0_MDT0,
                       &prCSMD_Instance->rPriv.rComVersion,
                       &prCSMD_Instance->rPriv.rTimingCP12 );
#endif
  
  /* ------------------------------------------------------------- */
  /* Initialize AT0 in Tx Ram                                      */
  /* ------------------------------------------------------------- */
  
  prCSMD_Instance->rPriv.rSlaveAvailable.usSeqCntInit  = 0x0001U;
  prCSMD_Instance->rPriv.rSlaveAvailable2.usSeqCntInit = 0x8001U;
  
  
  /* Init SeqCnt in AT0 for port 1 */
  pucTel =   (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aucTx_Ram
           + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_1];
  CSMD_HAL_WriteShort( (CSMD_VOID *)pucTel, 
                       prCSMD_Instance->rPriv.rSlaveAvailable.usSeqCntInit );
  /* Pointer to SeqCnt of port 1 */
  prCSMD_Instance->rPriv.pausSERC3_Tx_SeqCnt[CSMD_PORT_1] = (CSMD_USHORT *)((CSMD_VOID *)pucTel);
  
  /* Init SeqCnt in AT0 for port 2 */
  pucTel =   (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aucTx_Ram
           + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_2];
  CSMD_HAL_WriteShort( (CSMD_VOID *) pucTel, 
                       prCSMD_Instance->rPriv.rSlaveAvailable2.usSeqCntInit );
  /* Pointer to SeqCnt of port 2 */
  prCSMD_Instance->rPriv.pausSERC3_Tx_SeqCnt[CSMD_PORT_2] = (CSMD_USHORT *)((CSMD_VOID *)pucTel);
  
  
  /* Fill topology address fields with -1 */
  pucTel =   (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aucTx_Ram
           + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_0_SYS_A]
           + usBuffOffsAT;
  
  CSMD_Telegram_Clear( prCSMD_Instance, 
                       pucTel,
                       (CSMD_SERC3_AT_DATA_LENGTH_P0_V10 - CSMD_SEQCNT_FIELDWIDTH),
                       CSMD_PHASE0_FILL );
  
  
  prCSMD_Instance->rPriv.aboMDT_used[0] = TRUE;
  prCSMD_Instance->rPriv.usMDT_Enable = 0x0001;
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Config_TX_Tel_P0() */



/**************************************************************************/ /**
\brief Configures the transmit telegram forCP1.

\ingroup module_phase
\b Description: \n
   This function configures the transmit telegram forCP1, including:
   - Initialization of C-DEV of all operable and Hot Plug slaves
   - Clearance of descriptor index table and buffer base pointer list
   - Configuration of MDT Tx-Descriptors (SVC data, RT data and FCS field)
   - Configuration of AT Tx-Descriptors (FCS field)
   - Clearance of telegram data in TxRam for all MDT and AT
   - Setting of handshake bit in TxRam
   - Transmission of new descriptor index table and buffer base pointer list
   - Assignment of the service container according to the projected slave address list
   - Preparation of DMA for CP2  

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         09.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Config_TX_Tel_P1( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_DESCR_INDEX  arTxDesTable[ CSMD_HAL_TX_DESR_INDEX_NUMBER ];
  CSMD_ULONG        aulTxBufBasePtr[ CSMD_HAL_TX_BASE_PTR_NBR ];   /* Tx Buffer base pointer list */
  
  CSMD_ULONG        ulTxRamOffset;      /* Current byte offset in TxRam              */
  CSMD_USHORT       usBuff0_Offset;     /* Current offset [byte] in Tx buffer 0      */
  CSMD_USHORT       usBuffSVC_Offset;   /* Current offset [byte] in Tx buffer SVC    */
  CSMD_USHORT       usTelegramOffset;   /* Offset [byte] in current telegram         */
  CSMD_USHORT       usIdxTableOffs;     /* Tx-Descriptor table index offset in TxRam */
  CSMD_USHORT       usLengthSVC;        /* Length of SVC     in MDT/AT Telegrams     */
  CSMD_USHORT       usLengthRTD;        /* Length of RT data in MDT/AT Telegrams     */
  CSMD_ULONG        ulL_Offset;         /* Ram Offset in # Longs */
  
  CSMD_ULONG        ulOffsetSVC;        /* Offset for SVC data in TxRam */
  CSMD_UCHAR       *pucTel;             /* Pointer to current Telegram */
  CSMD_USHORT       usTelI;             /* Telegram Index */
  CSMD_USHORT       usI;                /* Index */
  CSMD_USHORT       usTAdd;             /* Topology address */
  CSMD_SERC3SVC    *prSVC;
  
  
#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    /* Prepare for DMA in CP2 */
    CSMD_HAL_ResetRXDMA( &prCSMD_Instance->rCSMD_HAL );
  }
#endif  
  
  /* ------------------------------------------------------------- */
  /* Initialize C-DEV of all operable and hot-plug slaves          */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    prCSMD_Instance->rPriv.ausDevControl[usI] = CSMD_C_DEV_MASTER_VALID;
  }
  
  
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
  {                                                
    prCSMD_Instance->rPriv.aboMDT_used[usTelI]  = FALSE;    /* MDT telegram is not used */
    prCSMD_Instance->rPriv.aboAT_used[usTelI]   = FALSE;    /* AT telegram is not used  */
  }
  
  ulTxRamOffset  = 0U;    /* Byte offset in TxRam     */
  
  /* Tx-Descriptor index table offset in TxRam */
  usIdxTableOffs = (CSMD_USHORT) ulTxRamOffset;
  CSMD_HAL_SetDesIdxTableOffsTxRam( &prCSMD_Instance->rCSMD_HAL, 
                                    usIdxTableOffs );
  
  /* ------------------------------------------------------------- */
  /* Clear Tx descriptor index table                               */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_TX_DESR_INDEX_NUMBER; usI++)
  {
    arTxDesTable[usI].ulIndex = 0;
  }
  
  /* ------------------------------------------------------------- */
  /* Clear Tx Buffer base pointer list                             */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_TX_BASE_PTR_NBR; usI++)
  {
    aulTxBufBasePtr[usI] = 0U;
  }
  
  ulTxRamOffset += sizeof (arTxDesTable);     /* Reserve space for 4 MDT and 4 AT index table entries */
  
  usBuff0_Offset   = 0;   /* Byte offset in Tx Buffer 0    */
  usBuffSVC_Offset = 0;   /* Byte offset in Tx Buffer SVC  */
  usLengthSVC      = CSMD_SERC3_SVC_DATA_LENGTH_P12;
  usLengthRTD      = CSMD_SERC3_RTD_DATA_LENGTH_P12;
  
  /* ------------------------------------------------------------- */
  /* Configure MDT                                                 */
  /* ------------------------------------------------------------- */
  for (usTelI = 0; usTelI < prCSMD_Instance->rPriv.usNbrTel_CP12; usTelI++)
  {
    /* Descriptor index table entry for MDT */
    arTxDesTable[usTelI].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
    arTxDesTable[usTelI].ulIndex |= (CSMD_ULONG) ((ulTxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
    
    usTelegramOffset = 0U;  /* Offset [byte] in current telegram */
    
    /* ------------------------------------------------------------- */
    /* Configure SVC Tx-Descriptors for MDT                          */
    /* ------------------------------------------------------------- */
    CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulTxRamOffset,                /* Descriptor offset in TxRam   */
                              usBuffSVC_Offset,             /* Buffer offset in byte        */
                              0U,                           /* Buffer System Select         */
                              usTelegramOffset,             /* Offset after CRC in telegram */
                              CSMD_TX_DES_TYPE_SVDSP );     /* Descriptor type              */
    ulTxRamOffset += 4;
    
    
    usBuffSVC_Offset += usLengthSVC;
    usTelegramOffset += usLengthSVC;
    CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulTxRamOffset,                        /* Descriptor offset in TxRam   */
                              (CSMD_USHORT)(usBuffSVC_Offset - 2),  /* Buffer offset in byte        */
                              0U,                                   /* Buffer System Select         */
                              (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                              CSMD_TX_DES_TYPE_SVDEP );             /* Descriptor type              */
    ulTxRamOffset += 4;
    
    
    /* ------------------------------------------------------------- */
    /* Configure RT data Tx-Descriptors for MDT                      */
    /* ------------------------------------------------------------- */
    CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulTxRamOffset,                /* Descriptor offset in TxRam   */
                              usBuff0_Offset,               /* Buffer offset in byte        */
                              0U,                           /* Buffer System Select         */
                              usTelegramOffset,             /* Offset after CRC in telegram */
                              CSMD_TX_DES_TYPE_RTDSP );     /* Descriptor type              */
    ulTxRamOffset += 4;
    
    
    usBuff0_Offset   += usLengthRTD;
    usTelegramOffset += usLengthRTD;
    CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulTxRamOffset,                        /* Descriptor offset in TxRam   */
                              (CSMD_USHORT)(usBuff0_Offset - 2),    /* Buffer offset in byte        */
                              0U,                                   /* Buffer System Select         */
                              (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                              CSMD_TX_DES_TYPE_RTDEP );             /* Descriptor type              */
    ulTxRamOffset += 4;
    
    
    /* ------------------------------------------------------------- */
    /* Configure FCS field Tx-Descriptors for MDT                    */
    /* ------------------------------------------------------------- */
    CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulTxRamOffset,                /* Descriptor offset in TxRam   */
                              0U,                           /* Buffer offset (irrelevant)   */
                              0U,                           /* Buffer System Select         */
                              usTelegramOffset,             /* Offset after CRC in telegram */
                              CSMD_TX_DES_TYPE_FCSP );      /* Descriptor type              */
    ulTxRamOffset += 4;
  }
  
  
  
  /* ------------------------------------------------------------- */
  /* Configure AT                                                  */
  /* ------------------------------------------------------------- */
  for (usTelI = 0; usTelI < prCSMD_Instance->rPriv.usNbrTel_CP12; usTelI++)
  {
    /* Descriptor Index table entry for MDT */
    arTxDesTable[usTelI + CSMD_HAL_TX_AT_DESR_IDX_OFFSET].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
    arTxDesTable[usTelI + CSMD_HAL_TX_AT_DESR_IDX_OFFSET].ulIndex |= (CSMD_ULONG) ((ulTxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
    usTelegramOffset = 0U;  /* Offset [byte] in current telegram */
    
    /* No Tx descriptor for SVC and RTD in AT */
    usTelegramOffset += (CSMD_USHORT) (usLengthSVC + usLengthRTD);
    
    /* MDT FCS field */
    /* ------------------------------------------------------------- */
    /* Configure FCS field Tx-Descriptors for AT                     */
    /* ------------------------------------------------------------- */
    CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulTxRamOffset,                /* Descriptor offset in TxRam   */
                              0U,                           /* Buffer offset (irrelevant)   */
                              0U,                           /* Buffer System Select         */
                              usTelegramOffset,             /* Offset after CRC in telegram */
                              CSMD_TX_DES_TYPE_FCSP );      /* Descriptor type              */
    ulTxRamOffset += 4;
  }
  
  
  /* Round up to the next divisible by 16 */
  ulTxRamOffset = ((ulTxRamOffset + 15) / 16) * 16;
  prCSMD_Instance->rPriv.ulTxRamTelOffset = ulTxRamOffset;
  
  /* Buffer Basepointer: SVC */
  aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_SVC] = ulTxRamOffset;
  ulTxRamOffset = ((ulTxRamOffset + usBuffSVC_Offset + 3) / 4) * 4;
  
  /* Buffer Basepointer: System A Buffer 0 */
  aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_0_SYS_A] = ulTxRamOffset;
  ulTxRamOffset = ((ulTxRamOffset + usBuff0_Offset + 3) / 4) * 4;
  
#ifdef CSMD_DEBUG
  prCSMD_Instance->rCSMD_Debug.aulTxBufSize[CSMD_HAL_IDX_TX_BUFF_0_SYS_A] = usBuff0_Offset;
  prCSMD_Instance->rCSMD_Debug.aulTxBufSize[CSMD_HAL_IDX_TX_BUFF_SVC]     = usBuffSVC_Offset;
#endif
  
  /* Used FPGA TxRam memory for Sercos telegrams */
  prCSMD_Instance->rCSMD_HAL.ulTxRamInUse = (((ulTxRamOffset - 1) / CSMD_IP_RAM_SEG_SIZE) + 1) * CSMD_IP_RAM_SEG_SIZE;
  
  ulL_Offset = aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_0_SYS_A] / 4;
  prCSMD_Instance->rPriv.pulSERC3_Tx_MDT_RTD = 
    (CSMD_ULONG *) &prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram[ulL_Offset];
  
  ulL_Offset = aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_SVC] / 4;
  prCSMD_Instance->rPriv.pulSERC3_Tx_MDT_SVC = 
    (CSMD_ULONG *) &prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram[ulL_Offset];
  
  
  /* Pointer to start of telegram Tx Ram */
  pucTel = (CSMD_UCHAR *) &prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram[ulL_Offset];
  
  /* ------------------------------------------------------------- */
  /* Clear telegram data (SVC /RTD) in Tx-Ram for all MDT and AT   */
  /* ------------------------------------------------------------- */
  CSMD_Telegram_Clear( prCSMD_Instance, 
                       pucTel,                                                  /* Pointer to TxRam */
                       (CSMD_USHORT)(  prCSMD_Instance->rCSMD_HAL.ulTxRamInUse  /* Data length      */
                                     - ulL_Offset),
                       0U                                                       /* Fill value       */
                     );
  
  
  /* ------------------------------------------------------------- */
  /* Transmit Descriptor index table into FPGA TxRam               */
  /* ------------------------------------------------------------- */
  usIdxTableOffs >>= 2;   /* Offset [Longs] in TxRam */ /*lint !e845 The left argument to operator '>>' is certain to be 0  */
  
  for (usI = 0; usI < CSMD_HAL_TX_DESR_INDEX_NUMBER; usI++)
  {
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram[usIdxTableOffs],
                        (arTxDesTable[usI].ulIndex) );
    usIdxTableOffs++;
  }
  
  /* ------------------------------------------------------------- */
  /* Transmit Tx Buffer base pointer list into FPGA register       */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_TX_BASE_PTR_NBR; usI++)
  {
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->aulTxBufBasePtr[usI], 
                        aulTxBufBasePtr[usI] );
#ifdef CSMD_DEBUG
    prCSMD_Instance->rCSMD_Debug.aulTxBufBasePtr[usI] = aulTxBufBasePtr[usI];
#endif
  }
  
  
  /* ------------------------------------------------------------- */
  /* Calculate TxRam pointer to C-DEV                              */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    usTAdd = prCSMD_Instance->rPriv.ausTopologyAddresses[usI];

    /* Pointer to C-DEV in TxRam */
    prCSMD_Instance->rPriv.pausC_Dev[CSMD_TX_BUFFER_0][usI] =
    (CSMD_USHORT *)(CSMD_VOID *)((CSMD_UCHAR *)prCSMD_Instance->rPriv.pulSERC3_Tx_MDT_RTD + (usTAdd * CSMD_C_DEV_LENGTH_CP1_2) );
  }


  /* Offset in TxRam for SVC in MDT */
  ulOffsetSVC = aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_SVC];
  
  /* ------------------------------------------------------------- */
  /* Assignment of the service container according to the          */
  /* projected slave address list.                                 */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    usTAdd = prCSMD_Instance->rPriv.ausTopologyAddresses[usI];
    
#if CSMD_MAX_HW_CONTAINER > 0
    if ((CSMD_SHORT)usI < CSMD_MAX_HW_CONTAINER )
    {
      prCSMD_Instance->rPriv.prSVContainer[usI] = 
        &prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram->rSC_S3[usI];
      
      prSVC = prCSMD_Instance->rPriv.prSVContainer[usI];
      
      CSMD_HAL_WriteShort( &prSVC->usSVCTxPointer_Control,
                           (CSMD_USHORT)(ulOffsetSVC + (usTAdd * CSMD_SVC_FIELDWIDTH)) );
    }
    /* enable emulated svc */
    else
#endif  /* #if CSMD_MAX_HW_CONTAINER > 0 */
    {
#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
      /*-------------------------------------------------------------------------- */
      /* Status and control words are pointed to the memory locations of Status    */
      /* and Control words in the RX and TX ram respectively                       */
      /*-------------------------------------------------------------------------- */
      
      prCSMD_Instance->rPriv.prSVContainer[usI] = 
        &prCSMD_Instance->rPriv.parSoftSvcContainer[usI - CSMD_MAX_HW_CONTAINER];
      
      prSVC = prCSMD_Instance->rPriv.prSVContainer[usI];
      
      prSVC->usSVCTxPointer_Control = 
        (CSMD_USHORT)(ulOffsetSVC + (usTAdd * CSMD_SVC_FIELDWIDTH));
#endif
    }
  }
  
  for (usTelI = 0; usTelI < prCSMD_Instance->rPriv.usNbrTel_CP12; usTelI++)
  {
    prCSMD_Instance->rPriv.aboMDT_used[usTelI] = TRUE;      /* MDT Telegram is used */
  }
  
  
#if (defined CSMD_PCI_MASTER) && (CSMD_MAX_HW_CONTAINER == 0)
  /* Note: Combined operation with IP-Core- and emulated-SVC causes problems with double line topology! */
  /* Prepare DMA for CP2 */
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    /* DMA for all emulated SVC.
       Set configuration of one PCI RX DMA channel
       (Source: Host ram / Destination: FPGA Tx ram) */
    CSMD_Config_DMA_Rx_Channel(
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulTxRamBase
        + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_SVC],
      (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffTxRam.aulTx_Ram
        + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_SVC],
      /* Add 1 channel for topology address 0. Add 2 byte (always guarantee a divisible by 4 length) */
      (CSMD_ULONG) ((prCSMD_Instance->rSlaveList.usNumProjSlaves + 1U) * CSMD_SVC_FIELDWIDTH + 2U),
      (CSMD_USHORT) CSMD_DMA_CHANNEL_00 );

    prCSMD_Instance->rPriv.ausRxDMA_Start[CSMD_TX_BUFFER_0] = 0;
    prCSMD_Instance->rPriv.ausRxDMA_Start[CSMD_TX_BUFFER_0] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_00);

    /* Clear the local Tx telegram buffer for all configured MDT and AT */
    (CSMD_VOID) CSMD_HAL_memset( (CSMD_VOID *)prCSMD_Instance->prTelBuffer->rLocal.rBuffTxRam.aulTx_Ram,
                                 0,
                                 prCSMD_Instance->rCSMD_HAL.ulTxRamInUse );
  }
#endif  /* #if (defined CSMD_PCI_MASTER) && (CSMD_MAX_HW_CONTAINER == 0) */
  prCSMD_Instance->rPriv.pusTxRam = (CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram;
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Config_TX_Tel_P1() */



/**************************************************************************/ /**
\brief Configures the transmit telegram for CP2.

\ingroup module_phase
\b Description: \n
   This function configures the transmit telegram for CP2, including:
   - Initialization of Busy-/Handshake-Timeout for the service channel
   - Setting of service channel control register

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         09.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Config_TX_Tel_P2( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_ULONG   ulTimeOut;
  
  
  /* ------------------------------------------------------------- */
  /* Initialize Busy-/Handshake-Timeout for SVC                    */
  /* ------------------------------------------------------------- */
  prCSMD_Instance->rPriv.usSVC_HS_Timeout   = CSMD_SERC3_HS_TIMEOUT;      /* 10 Cycles */
  
  if (prCSMD_Instance->rPriv.rHW_Init_Struct.usSVC_BusyTimeout > 2000)    /* [ms] */
  {
    ulTimeOut =                             /* in Cycles */
      (((CSMD_ULONG)prCSMD_Instance->rPriv.rHW_Init_Struct.usSVC_BusyTimeout * 100000UL) / 
      prCSMD_Instance->rPriv.ulActiveCycTime) * 10;
  }
  else
  {
    ulTimeOut =                             /* in Cycles */
      ((CSMD_ULONG)prCSMD_Instance->rPriv.rHW_Init_Struct.usSVC_BusyTimeout * 1000000UL) / 
      prCSMD_Instance->rPriv.ulActiveCycTime;
  }
  if (ulTimeOut > 65000)      /* in Cycles */
  {
    prCSMD_Instance->rPriv.usSVC_BUSY_Timeout = 65000;
  }
  else if (ulTimeOut < 1)     /* in Cycles */
  {
    prCSMD_Instance->rPriv.usSVC_BUSY_Timeout = 1;
  }
  else
  {
    prCSMD_Instance->rPriv.usSVC_BUSY_Timeout = (CSMD_USHORT) ulTimeOut;
  }
  
  /* Set Service Channel Control Register */ 
  CSMD_HAL_SetSVCTimeouts( &prCSMD_Instance->rCSMD_HAL, 
                           (CSMD_USHORT)((((CSMD_SHORT)prCSMD_Instance->rPriv.usSVC_BUSY_Timeout-2)/256)+2),
                           prCSMD_Instance->rPriv.usSVC_HS_Timeout );
  
#ifdef CSMD_HW_SVC_REDUNDANCY
  if (TRUE == prCSMD_Instance->rPriv.boHW_SVC_Redundancy)
  {
    CSMD_HAL_CtrlSVC_Redundancy( &prCSMD_Instance->rCSMD_HAL, TRUE);
  }
  else
#endif
  {
    CSMD_HAL_CtrlSVC_Redundancy( &prCSMD_Instance->rCSMD_HAL, FALSE);
    
    /* Select the port for the SVC machine trigger */
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
    {
      CSMD_HAL_SetSVCPort( &prCSMD_Instance->rCSMD_HAL, (CSMD_USHORT) CSMD_HAL_SVCCSR_TRIG_PORT2 );
    }
    else
    {
      /* trigger SVC machine from port 1 */
      CSMD_HAL_SetSVCPort( &prCSMD_Instance->rCSMD_HAL, (CSMD_USHORT) CSMD_HAL_SVCCSR_TRIG_PORT1 );
    }
    
    /* Number of Telegrams: in CP1 and CP2 */
    if (prCSMD_Instance->rPriv.usNbrTel_CP12 == 2U)
    {
      /* AT1 is the last SVC telegram, trigger svc state machine after reception of AT1 */
      CSMD_HAL_SetSVCLastAT( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_SVCCSR_TRIG_SELECT_AT1 );
    }
    else
    {
      /* AT3 is the last SVC telegram, trigger svc state machine after reception of AT3 */
      CSMD_HAL_SetSVCLastAT( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_SVCCSR_TRIG_SELECT_AT3 );
    }
  }
  
#if (defined CSMD_PCI_MASTER) && (CSMD_MAX_HW_CONTAINER == 0)
  /* Note: Combined operation with IP-Core- and emulated-SVC causes problems with double line topology! */
  /* Prepare DMA for CP2 */
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    prCSMD_Instance->rPriv.pusTxRam = (CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->prTelBuffer->rLocal.rBuffTxRam.aulTx_Ram;

    /* Set Enable_DMA and Enable_RDY_DMA for the selected channels. */
    CSMD_HAL_Enable_RXDMA( &prCSMD_Instance->rCSMD_HAL,
                           prCSMD_Instance->rPriv.ausRxDMA_Start[CSMD_TX_BUFFER_0] ); /* Only 1 channel active! */
  }
  else
#endif  /* #if (defined CSMD_PCI_MASTER) && (CSMD_MAX_HW_CONTAINER == 0) */
  {
    prCSMD_Instance->rPriv.pusTxRam = (CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram;
  }
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Config_TX_Tel_P2() */



/**************************************************************************/ /**
\brief Configures the transmit telegram forCP3.

\ingroup module_phase
\b Description: \n
   This function configures the transmit telegram forCP3, including:
   - Clearance of descriptor index table and buffer base pointer list
   - Configuration of all MDT Tx-Descriptors (HP field, SVC data, RT data and FCS field)
   - Configuration of all AT Tx-Descriptors (RT data for CC-connections and
     master produced AT connections, FCS field)
   - Clearance of telegram data in TxRam for all MDT and AT
   - Transmission of new descriptor index table and buffer base pointer list    
   - Initialization of TxRam MDT pointers
   - Initialization of TxRam AT pointers
   - Initialization of TxRam Hot Plug Pointers
   - Calculation of MDT_InternalStruct fields
   - Calculation of AT_InternalStruct fields (for master produced AT connections)
   - Calculate TxRam pointers to C-DEVs
   - Initialization of Hot Plug parameters (transmission repeat rate, timeout for
     slave scan, HP field)
   - Assignment of the service container according to the projected slave address list

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         09.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Config_TX_Tel_P3( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_DESCR_INDEX arTxDesTable[ CSMD_HAL_TX_DESR_INDEX_NUMBER ];
  CSMD_ULONG       aulTxBufBasePtr[ CSMD_HAL_TX_BASE_PTR_NBR ];   /* Tx Buffer base pointer list */
  
  CSMD_ULONG       ulTxRamOffset;     /* Current byte offset in TxRam                  */
  CSMD_USHORT      usPort_Offset;     /* Current offset [byte] in port relative buffer */
  CSMD_USHORT      usBuff0_Offset;    /* Current offset [byte] in Tx buffer 0          */
  CSMD_USHORT      usBuffSVC_Offset;  /* Current offset [byte] in Tx buffer SVC        */
  CSMD_USHORT      usTelegramOffset;  /* Offset [byte] in current telegram             */
  CSMD_USHORT      usIdxTableOffs;    /* Tx-Descriptor table index offset in TxRam     */
  CSMD_USHORT      usLengthSVC;       /* Length of SVC     in MDT/AT Telegrams         */
  CSMD_USHORT      usLengthRTD;       /* Length of RT data in MDT/AT Telegrams         */
  CSMD_ULONG       ulL_Offset;        /* Ram offset in # Longs */
  
  CSMD_UCHAR      *pucTel;            /* Pointer to current Telegram */
  CSMD_USHORT      usTelI;            /* Telegram Index */
  CSMD_USHORT      usI;               /* Index */
  CSMD_USHORT      usTempRam;
  
  CSMD_SERC3SVC   *prSVC;
  CSMD_BOOL        boContains_RTD = FALSE;
  CSMD_ULONG       ulTimeOut;
  CSMD_USHORT      usBuf;             /* Buffer index */
  CSMD_USHORT      usConnIdx;         /* Connection index */
#ifdef CSMD_MASTER_PRODUCE_IN_AT
  CSMD_USHORT      usConfigIdx;       /* Configuration Index */
#endif
  
#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    CSMD_HAL_ResetRXDMA(&prCSMD_Instance->rCSMD_HAL);
  }
#endif
  
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++)
  {
    if (   prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[usTelI] 
        && (   (prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[usTelI] < CSMD_SERC3_MIN_DATA_LENGTH) 
            || (prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[usTelI] > CSMD_SERC3_MAX_DATA_LENGTH)))
    {
      return (CSMD_FAULTY_MDT_LENGTH);
    }
    
    if (    prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[usTelI] 
        && (   (prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[usTelI] < CSMD_SERC3_MIN_DATA_LENGTH) 
            || (prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[usTelI] > CSMD_SERC3_MAX_DATA_LENGTH)))
    {
      return (CSMD_FAULTY_AT_LENGTH);
    }
  }   /* End: for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ ) */
  
  
  ulTxRamOffset  = 0U;    /* Byte offset in TxRam     */
  
  /* Tx-Descriptor index table offset in TxRam */
  usIdxTableOffs = (CSMD_USHORT) ulTxRamOffset;
  CSMD_HAL_SetDesIdxTableOffsTxRam( &prCSMD_Instance->rCSMD_HAL, 
                                    usIdxTableOffs );
  
  /* ------------------------------------------------------------- */
  /* Clear Tx descriptor index table                               */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_TX_DESR_INDEX_NUMBER; usI++)
  {
    arTxDesTable[usI].ulIndex = 0;
  }
  
  /* ------------------------------------------------------------- */
  /* Clear Tx Buffer base pointer list                             */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_TX_BASE_PTR_NBR; usI++)
  {
    aulTxBufBasePtr[usI] = 0U;
  }
  
  ulTxRamOffset += sizeof (arTxDesTable);     /* Reserve space for 4 MDT and 4 AT index table entries */
  
  usBuff0_Offset   = 0U;  /* Byte offset in Tx Buffer 0      */
  usBuffSVC_Offset = 0U;  /* Byte offset in Tx Buffer SVC    */
  usPort_Offset    = 0U;  /* Byte offset in Tx Buffer Port relative */
  
  
  /* ------------------------------------------------------------- */
  /* Configuration of all MDT telegrams                            */
  /* ------------------------------------------------------------- */
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
  {
    if (prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[usTelI])
    {
      /* Descriptor Index table entry for MDT */
      arTxDesTable[usTelI].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
      arTxDesTable[usTelI].ulIndex |= (CSMD_ULONG) ((ulTxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
      
      usTelegramOffset = 0U;  /* Offset [byte] in current telegram */
      
      if (usTelI == 0)
      {
        /* ------------------------------------------------------------- */
        /* Configure port relative Tx-Descriptor for HP field in MDT0    */
        /* ------------------------------------------------------------- */
        
        /* Hot plug data field in MDT0 separately adjustable for port 1 and port 2 */
        CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulTxRamOffset,                /* Descriptor offset in TxRam   */
                                  usPort_Offset,                /* Buffer offset in byte        */
                                  0U,                           /* Buffer System Select         */
                                  usTelegramOffset,             /* Offset after CRC in telegram */
                                  CSMD_TX_DES_TYPE_PRELSP );    /* Descriptor type              */
        ulTxRamOffset += 4;


        usPort_Offset    += prCSMD_Instance->rPriv.rMDT_Length[usTelI].usHP;
        usTelegramOffset += prCSMD_Instance->rPriv.rMDT_Length[usTelI].usHP;
        CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulTxRamOffset,                        /* Descriptor offset in TxRam   */
                                  (CSMD_USHORT) (usPort_Offset - 2),    /* Buffer offset in byte        */
                                  0U,                                   /* Buffer System Select         */
                                  (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                                  CSMD_TX_DES_TYPE_PRELEP );            /* Descriptor type              */
        ulTxRamOffset += 4;
      }
      else
      {
        /*  */
        usTelegramOffset += prCSMD_Instance->rPriv.rMDT_Length[usTelI].usHP;
      }
      
      /* No descriptor needed for Extended Function Field (Sercos Time / Sub Cycle Counter) */
      /* No space in Tx Buff0 needed. FPGA stores the Sercos time directly into the telegram stream! */
      
      usTelegramOffset += prCSMD_Instance->rPriv.rMDT_Length[usTelI].usEF;
      
      usLengthSVC = prCSMD_Instance->rPriv.rMDT_Length[usTelI].usSVC; 
      /* SVC field in telegram ? */
      if (usLengthSVC)
      {
        /* ------------------------------------------------------------- */
        /* Configure SVC Tx-Descriptors for MDT                          */
        /* ------------------------------------------------------------- */
        CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulTxRamOffset,                /* Descriptor offset in TxRam   */
                                  usBuffSVC_Offset,             /* Buffer offset in byte        */
                                  0U,                           /* Buffer System Select         */
                                  usTelegramOffset,             /* Offset after CRC in telegram */
                                  CSMD_TX_DES_TYPE_SVDSP );     /* Descriptor type              */
        ulTxRamOffset += 4;
        
        
        usBuffSVC_Offset += usLengthSVC;
        usTelegramOffset += usLengthSVC;
        CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulTxRamOffset,                        /* Descriptor offset in TxRam   */
                                  (CSMD_USHORT)(usBuffSVC_Offset - 2),  /* Buffer offset in byte        */
                                  0,                                    /* Buffer System Select         */
                                  (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                                  CSMD_TX_DES_TYPE_SVDEP );             /* Descriptor type              */
        ulTxRamOffset += 4;
      }
      
      
      usLengthRTD = prCSMD_Instance->rPriv.rMDT_Length[usTelI].usRTD;
      /* RTD field in telegram ? */
      if (usLengthRTD)
      {
        boContains_RTD = TRUE;
        /* ------------------------------------------------------------- */
        /* Configure RT data Tx-Descriptors for MDT                      */
        /* ------------------------------------------------------------- */
        CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulTxRamOffset,                /* Descriptor offset in TxRam   */
                                  usBuff0_Offset,               /* Buffer offset in byte        */
                                  0U,                           /* Buffer System Select         */
                                  usTelegramOffset,             /* Offset after CRC in telegram */
                                  CSMD_TX_DES_TYPE_RTDSP );     /* Descriptor type              */
        ulTxRamOffset += 4;
        
        
        usBuff0_Offset   += usLengthRTD;
        usTelegramOffset += usLengthRTD;
        CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulTxRamOffset,                        /* Descriptor offset in TxRam   */
                                  (CSMD_USHORT)(usBuff0_Offset - 2),    /* Buffer offset in byte        */
                                  0U,                                   /* Buffer System Select         */
                                  (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                                  CSMD_TX_DES_TYPE_RTDEP );             /* Descriptor type              */
        ulTxRamOffset += 4;
      }
      
      /* ------------------------------------------------------------- */
      /* Configure FCS field Tx-Descriptors for MDT                    */
      /* ------------------------------------------------------------- */
      CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                ulTxRamOffset,                /* Descriptor offset in TxRam   */
                                0U,                           /* Buffer offset (irrelevant)   */
                                0U,                           /* Buffer System Select         */
                                usTelegramOffset,             /* Offset after CRC in telegram */
                                CSMD_TX_DES_TYPE_FCSP );      /* Descriptor type              */
      ulTxRamOffset += 4;
      
    }   /* End: if (prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[usTelI]) */
    
  }   /* End: for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ ) */
  
  prCSMD_Instance->rPriv.usTxRamPortOffset = usPort_Offset; 
  
  /* Abfrage notwendig??? */
  if (boContains_RTD != TRUE)
    /* No telegram with RT data */
    return (CSMD_SYSTEM_ERROR);
  
  
  /* ------------------------------------------------------------- */
  /* Configuration of all AT telegrams                             */
  /* ------------------------------------------------------------- */
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
  {
    if (prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[usTelI])
    {
      /* Descriptor Index table entry for MDT */
      arTxDesTable[usTelI + CSMD_HAL_TX_AT_DESR_IDX_OFFSET].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
      arTxDesTable[usTelI + CSMD_HAL_TX_AT_DESR_IDX_OFFSET].ulIndex |= (CSMD_ULONG) ((ulTxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
      
      usTelegramOffset = 0U;  /* Offset [byte] in current telegram */
      
      /* No Tx descriptor for HP field in AT */
      usTelegramOffset += prCSMD_Instance->rPriv.rAT_Length[usTelI].usHP;
      
      /* No Tx descriptor for SVC in AT */
      usTelegramOffset += prCSMD_Instance->rPriv.rAT_Length[usTelI].usSVC;
      
      usLengthRTD = prCSMD_Instance->rPriv.rAT_Length[usTelI].usCC;
      /* CC data in this telegram? */
      if (usLengthRTD)
      {
        /* ------------------------------------------------------------- */
        /* Configure CC data Tx-Descriptor for AT                        */
        /* ------------------------------------------------------------- */
        CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulTxRamOffset,                /* Descriptor offset in TxRam   */
                                  usPort_Offset,                /* Buffer offset in byte        */
                                  0U,                           /* Buffer System Select         */
                                  usTelegramOffset,             /* Offset after CRC in telegram */
                                  CSMD_TX_DES_TYPE_PRELCCSP );  /* Descriptor type              */
        ulTxRamOffset += 4;


        usPort_Offset    += usLengthRTD;
        usTelegramOffset += usLengthRTD;
        CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulTxRamOffset,                        /* Descriptor offset in TxRam   */
                                  (CSMD_USHORT)(usPort_Offset - 2),     /* Buffer offset in byte        */
                                  0U,                                   /* Buffer System Select         */
                                  (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                                  CSMD_TX_DES_TYPE_PRELCCEP );          /* Descriptor type              */
        ulTxRamOffset += 4;
      }
      
      usLengthRTD = prCSMD_Instance->rPriv.rAT_Length[usTelI].usMProd;
      /* Field with master produced connections in telegram ? */
      if (usLengthRTD)
      {
        /* ------------------------------------------------------------- */
        /* Configure RT data (master produced connections)               */
        /* Tx-Descriptors for AT                                         */
        /* ------------------------------------------------------------- */
        CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulTxRamOffset,                /* Descriptor offset in TxRam   */
                                  usBuff0_Offset,               /* Buffer offset in byte        */
                                  0,                            /* Buffer System Select         */
                                  usTelegramOffset,             /* Offset after CRC in telegram */
                                  CSMD_TX_DES_TYPE_RTDSP );     /* Descriptor type              */
        ulTxRamOffset += 4;
        
        
        usBuff0_Offset   += usLengthRTD;
        usTelegramOffset += usLengthRTD;
        CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulTxRamOffset,                        /* Descriptor offset in TxRam   */
                                  (CSMD_USHORT)(usBuff0_Offset - 2),    /* Buffer offset in byte        */
                                  0U,                                   /* Buffer System Select         */
                                  (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                                  CSMD_TX_DES_TYPE_RTDEP );             /* Descriptor type              */
        ulTxRamOffset += 4;
      }
      
      usLengthRTD = (CSMD_USHORT)(  prCSMD_Instance->rPriv.rAT_Length[usTelI].usRTD
                                  - prCSMD_Instance->rPriv.rAT_Length[usTelI].usMProd);
      
      /* RTD field in telegram ? */
      if (usLengthRTD)
      {
        /* ------------------------------------------------------------- */
        /* No Tx-Descriptors for AT RT data (zero fill is active)        */
        /* ------------------------------------------------------------- */
        usTelegramOffset += usLengthRTD;
      }
      
      /* ------------------------------------------------------------- */
      /* Configure FCS field Tx-Descriptors for AT                     */
      /* ------------------------------------------------------------- */
      CSMD_HAL_SetTxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                ulTxRamOffset,                /* Descriptor offset in TxRam   */
                                0U,                           /* Buffer offset (irrelevant)   */
                                0U,                           /* Buffer System Select         */
                                usTelegramOffset,             /* Offset after CRC in telegram */
                                CSMD_TX_DES_TYPE_FCSP );      /* Descriptor type              */
      ulTxRamOffset += 4;
      
    }   /* End: if (prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01010[usTelI]) */
    
  }   /* End: for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ ) */
  
  
  /* Round up to the next divisible by 16 */
  ulTxRamOffset = ((ulTxRamOffset + 15) / 16) * 16;
  prCSMD_Instance->rPriv.ulTxRamTelOffset = ulTxRamOffset;
  
  /* Offset to begin of telegram data buffer */
  ulL_Offset = ulTxRamOffset >> 2;
  
  
  /* Buffer Basepointer: SVC */
  aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_SVC] = ulTxRamOffset;
  /* Round buffer offset up to divisible by 4 for proper calculation of Rx buffer base pointer */
  usBuffSVC_Offset = (CSMD_USHORT) (((usBuffSVC_Offset + 3) / 4) * 4);
  ulTxRamOffset += usBuffSVC_Offset;
  
  /* Buffer Basepointer: Port 1 relative */
  aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_1] = ulTxRamOffset;

  /* TxRamOffset for Rx Buffer Basepointer: Port 1 relative buffer (write to TxRam) */
  prCSMD_Instance->rPriv.ulTxRamOffsetPRel_P1  = ulTxRamOffset;

  ulTxRamOffset = ((ulTxRamOffset + usPort_Offset + 3) / 4) * 4;
  
  /* Buffer Basepointer: Port 2 relative */
  aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_2] = ulTxRamOffset;

  /* TxRamOffset for Rx Buffer Basepointer: Port 2 relative buffer (write to TxRam) */
  prCSMD_Instance->rPriv.ulTxRamOffsetPRel_P2  = ulTxRamOffset;

  ulTxRamOffset = ((ulTxRamOffset + usPort_Offset + 3) / 4) * 4;
  
  prCSMD_Instance->rPriv.usTxRamPRelBuffLen = usPort_Offset;
  
  /* Buffer Basepointer: System A Buffer 0 */
  aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_0_SYS_A] = ulTxRamOffset;
  /* Round buffer offset up to divisible by 4 for proper calculation of Tx buffer base pointer */
  usBuff0_Offset = (CSMD_USHORT) (((usBuff0_Offset + 3) / 4) * 4);
  ulTxRamOffset += usBuff0_Offset;
#ifdef CSMD_DEBUG
  prCSMD_Instance->rCSMD_Debug.aulTxBufSize[CSMD_HAL_IDX_TX_BUFF_0_SYS_A] = usBuff0_Offset;
#endif

#if CSMD_MAX_TX_BUFFER > CSMD_TX_DOUBLE_BUFFER
  /* Buffer Basepointer: System A Buffer 1 */
  aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_1_SYS_A] = ulTxRamOffset;
  ulTxRamOffset += usBuff0_Offset;
#ifdef CSMD_DEBUG
  prCSMD_Instance->rCSMD_Debug.aulTxBufSize[CSMD_HAL_IDX_TX_BUFF_1_SYS_A] = usBuff0_Offset;
#endif
#endif

#if CSMD_MAX_TX_BUFFER > CSMD_TX_TRIPLE_BUFFER
  /* Buffer Basepointer: System A Buffer 2 */
  aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_2_SYS_A] = ulTxRamOffset;
  ulTxRamOffset += usBuff0_Offset;
#ifdef CSMD_DEBUG
  prCSMD_Instance->rCSMD_Debug.aulTxBufSize[CSMD_HAL_IDX_TX_BUFF_2_SYS_A] = usBuff0_Offset;
#endif
#endif
  
#ifdef CSMD_DEBUG
  prCSMD_Instance->rCSMD_Debug.aulTxBufSize[CSMD_HAL_IDX_TX_BUFF_PORT_1] = usPort_Offset;
  prCSMD_Instance->rCSMD_Debug.aulTxBufSize[CSMD_HAL_IDX_TX_BUFF_PORT_2] = usPort_Offset;
  prCSMD_Instance->rCSMD_Debug.aulTxBufSize[CSMD_HAL_IDX_TX_BUFF_SVC]    = usBuffSVC_Offset;
#endif
  
  /* Used FPGA TxRam memory for Sercos telegrams */
  prCSMD_Instance->rCSMD_HAL.ulTxRamInUse = (((ulTxRamOffset - 1) / CSMD_IP_RAM_SEG_SIZE) + 1) * CSMD_IP_RAM_SEG_SIZE;
  
  if (prCSMD_Instance->rCSMD_HAL.ulTxRamInUse > CSMD_HAL_TX_RAM_SIZE)
  {
    return (CSMD_INSUFFICIENT_TX_RAM);
  }
  
  /* Pointer to start of telegram Tx Ram */
  pucTel = (CSMD_UCHAR *) &prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram[ulL_Offset];
  
  /* ------------------------------------------------------------- */
  /* Clear telegram data (SVC /RTD) in Tx-Ram for all MDT and AT   */
  /* ------------------------------------------------------------- */
  CSMD_Telegram_Clear( prCSMD_Instance, 
                       pucTel,                                                  /* Pointer to TxRam */
                       (CSMD_USHORT)(  prCSMD_Instance->rCSMD_HAL.ulTxRamInUse  /* Data length      */
                                     - ulL_Offset),
                       0U                                                       /* Fill value       */
                     );
  
  /* ------------------------------------------------------------- */
  /* Transmit Descriptor index table into FPGA TxRam               */
  /* ------------------------------------------------------------- */
  usIdxTableOffs >>= 2;   /* Offset [Longs] in TxRam */ /*lint !e845 The left argument to operator '>>' is certain to be 0  */
  
  for (usI = 0; usI < CSMD_HAL_TX_DESR_INDEX_NUMBER; usI++)
  {
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram[usIdxTableOffs],
                        (arTxDesTable[usI].ulIndex) );
    usIdxTableOffs++;
  }
  
  /* ------------------------------------------------------------- */
  /* Transmit Tx Buffer base pointer list into FPGA register       */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_TX_BASE_PTR_NBR; usI++)
  {
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->aulTxBufBasePtr[usI], 
                        aulTxBufBasePtr[usI] );
#ifdef CSMD_DEBUG
    prCSMD_Instance->rCSMD_Debug.aulTxBufBasePtr[usI] = aulTxBufBasePtr[usI];
#endif
  }
  
  
  /* ------------------------------------------------------------- */
  /* Initialize TxRam MDT pointers                                 */
  /* ------------------------------------------------------------- */
  /* Buffer Basepointer: System A Buffer 0 */
  ulTxRamOffset = aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_0_SYS_A];
  
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
  {
    if (prCSMD_Instance->rPriv.rMDT_Length[usTelI].usRTD)
    {
      prCSMD_Instance->rPriv.pusTxRam_MDT_RTData[usTelI] = 
        (CSMD_USHORT *)(CSMD_VOID *)prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram + ulTxRamOffset / 2;
      
      ulTxRamOffset += prCSMD_Instance->rPriv.rMDT_Length[usTelI].usRTD;
    }
  }
  /* ------------------------------------------------------------- */
  /* Initialize TxRam AT pointers                                  */
  /* ------------------------------------------------------------- */
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
  {
    if (prCSMD_Instance->rPriv.rAT_Length[usTelI].usRTD)
    {
      prCSMD_Instance->rPriv.pusTxRam_AT_RTData[usTelI] = 
        (CSMD_USHORT *)(CSMD_VOID *)prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram + ulTxRamOffset / 2;
      
      ulTxRamOffset += prCSMD_Instance->rPriv.rAT_Length[usTelI].usRTD;
    }
  }
  
  /* ------------------------------------------------------------- */
  /* Initialize the TxRam Hot-Plug Field pointers                  */
  /* ------------------------------------------------------------- */
  prCSMD_Instance->rPriv.rHP_P1_Struct.pulTxRam =
    (CSMD_ULONG *) &prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram[aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_1]/4];
  prCSMD_Instance->rPriv.rHP_P2_Struct.pulTxRam =
    (CSMD_ULONG *) &prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram[aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_2]/4];
  

#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    /* Clear the local Tx telegram buffer for all configured MDT and AT */
    (CSMD_VOID) CSMD_HAL_memset( (CSMD_VOID *)prCSMD_Instance->prTelBuffer->rLocal.rBuffTxRam.aulTx_Ram,
                                 0,
                                 prCSMD_Instance->rCSMD_HAL.ulTxRamInUse );

    prCSMD_Instance->rPriv.pusTxRam = (CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->prTelBuffer->rLocal.rBuffTxRam.aulTx_Ram;
  }
  else
#endif  /* #ifdef CSMD_PCI_MASTER */
  {
    prCSMD_Instance->rPriv.pusTxRam = (CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram;
  }
  
  /* ------------------------------------------------------------- */
  /* Calculate TxRam offset for MDT connections                    */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; usI++)
  {
    usConnIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI].usConnIdx;

    /* MDT connection */
    if (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usTelegramType ==
          CSMD_TELEGRAM_TYPE_MDT )
    {
      usTelI    = (CSMD_USHORT)((prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 &
                     CSMD_S_0_1050_SE3_TEL_NBR_MASK) >> CSMD_S_0_1050_SE3_TEL_NBR_SHIFT);
      /* Offset to RTD field of telegram */
      usTelegramOffset = (CSMD_USHORT)(prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 &
                           CSMD_S_0_1050_SE3_TEL_OFFSET);
      ulTxRamOffset = (CSMD_ULONG)(usTelegramOffset
                           - (  prCSMD_Instance->rPriv.rMDT_Length[usTelI].usTel
                              - prCSMD_Instance->rPriv.rMDT_Length[usTelI].usRTD));

      /* Pointer to RTD field of telegram in FPGA */
#ifdef CSMD_PCI_MASTER
      if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
      {
        pucTel = (CSMD_UCHAR *)
          (  prCSMD_Instance->prTelBuffer->rLocal.rBuffTxRam.aucTx_Ram
           + (  (CSMD_UCHAR *)prCSMD_Instance->rPriv.pusTxRam_MDT_RTData[usTelI]
              - prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aucTx_Ram)
           + ulTxRamOffset);
      }
      else
#endif
      {
        pucTel    = (CSMD_UCHAR *)prCSMD_Instance->rPriv.pusTxRam_MDT_RTData[usTelI] + ulTxRamOffset;
      }

      for (usBuf = CSMD_TX_BUFFER_0; usBuf < CSMD_MAX_TX_BUFFER; usBuf++)
      {
        prCSMD_Instance->rPriv.parConnMasterProd[usConnIdx].apusConnTxRam[usBuf] =
          (CSMD_USHORT *)(CSMD_VOID *)(pucTel + usBuf * usBuff0_Offset);
      }
    }
    // pucTel += prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5;

  }

  
#ifdef CSMD_MASTER_PRODUCE_IN_AT
  /* ---------------------------------------------------------------------- */
  /* Calculate Field of AT_INTERNAL_STRUCTs for produced master connections */
  /* ---------------------------------------------------------------------- */
  for (usI = 0; usI < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; usI++)
  {
    usConnIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI].usConnIdx;

    /* AT connection */
    if (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usTelegramType ==
          CSMD_TELEGRAM_TYPE_AT )
    {
      usConfigIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI].usConfigIdx;

      /* check if master is producer */
      if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1
           & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
      {
        usTelI    = (CSMD_USHORT)((prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 &
                       CSMD_S_0_1050_SE3_TEL_NBR_MASK) >> CSMD_S_0_1050_SE3_TEL_NBR_SHIFT);
        usTelegramOffset = (CSMD_USHORT)(prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 &
                             CSMD_S_0_1050_SE3_TEL_OFFSET);
        ulTxRamOffset = (CSMD_ULONG)(usTelegramOffset
                             - (  prCSMD_Instance->rPriv.rMDT_Length[usTelI].usTel
                                - prCSMD_Instance->rPriv.rMDT_Length[usTelI].usRTD));

        /* Pointer to RTD field of telegram in FPGA */
        pucTel    = (CSMD_UCHAR *)prCSMD_Instance->rPriv.pusTxRam_AT_RTData[usTelI] + ulTxRamOffset;

        for (usBuf = CSMD_TX_BUFFER_0; usBuf < CSMD_MAX_TX_BUFFER; usBuf++)
        {
          prCSMD_Instance->rPriv.parConnMasterProd[usConnIdx].apusConnTxRam[usBuf] =
            (CSMD_USHORT *)(CSMD_VOID *)(pucTel + usBuf * usBuff0_Offset);
        }
        pucTel += prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5;

      }
    }
  }
#endif
  

  /* ---------------------------------------------------------------------- */
  /* Calculate TxRam pointer to C-DEV                                       */
  /* ---------------------------------------------------------------------- */
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    usTelI        = (CSMD_USHORT)((prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTelegramConfig.usC_DEV_OffsetMDT_S01009 >> 12) & 0x0003);
    ulTxRamOffset = (CSMD_ULONG)(prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTelegramConfig.usC_DEV_OffsetMDT_S01009 & 0x0FFF)
      - (prCSMD_Instance->rPriv.rMDT_Length[usTelI].usTel - prCSMD_Instance->rPriv.rMDT_Length[usTelI].usRTD);
    
    for (usBuf = CSMD_TX_BUFFER_0; usBuf < CSMD_MAX_TX_BUFFER; usBuf++)
    {
      /* Pointer to device control in TxRam buffer x */
#ifdef CSMD_PCI_MASTER
      if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
      {
        prCSMD_Instance->rPriv.pausC_Dev[usBuf][usI] = (CSMD_USHORT *)(CSMD_VOID *)
          (  prCSMD_Instance->prTelBuffer->rLocal.rBuffTxRam.aucTx_Ram
           + (  (CSMD_UCHAR *) prCSMD_Instance->rPriv.pusTxRam_MDT_RTData[usTelI]
              - prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aucTx_Ram)
           + ulTxRamOffset + usBuf * usBuff0_Offset);
      }
      else
#endif
      {
        prCSMD_Instance->rPriv.pausC_Dev[usBuf][usI] = 
          (CSMD_USHORT *)(CSMD_VOID *)( (CSMD_UCHAR *)prCSMD_Instance->rPriv.pusTxRam_MDT_RTData[usTelI]
                                       + ulTxRamOffset + usBuf * usBuff0_Offset);
      }
    }
  }


#ifdef CSMD_HOTPLUG
  if (CSMD_IsHotplugSupported (prCSMD_Instance))
  {
    prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord = CSMD_HP_CNTRL_SUPPORTED | CSMD_HP_CODE_T6;
    
    /* ------------------------------------------------------------- */
    /* Initialize HP0 parameter transmission repeat rate             */
    /* ------------------------------------------------------------- */
    if (prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 <= CSMD_TSCYC_500_US)
    {
      prCSMD_Instance->rPriv.rHotPlug.usHP0_RepeatRate =
        (CSMD_USHORT) (CSMD_TSCYC_2_MS / prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 - 1);
    }
    else if (prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 >= CSMD_TSCYC_1_MS)
    {
      prCSMD_Instance->rPriv.rHotPlug.usHP0_RepeatRate = 1;
    }
    else  /* Tscyc = 750 microseconds */
    {
      prCSMD_Instance->rPriv.rHotPlug.usHP0_RepeatRate = 2;
    }

    /* ------------------------------------------------------------- */
    /* Initialize HP timeout [Sercos cycles] for slave scan          */
    /* ------------------------------------------------------------- */
    if (prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 > CSMD_TSCYC_2_MS)
    {
      prCSMD_Instance->rPriv.rHotPlug.usHP0_ScanTimeout = CSMD_HP0_SCAN_TIMEOUT;
    }
    else    /* CSMD_TSCYC_2_MS >= Tscyc >= CSMD_TSCYC_1_MS */
    {
      prCSMD_Instance->rPriv.rHotPlug.usHP0_ScanTimeout =
        (CSMD_USHORT) ((CSMD_HP0_SCAN_TIMEOUT * CSMD_TSCYC_2_MS) / prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002);
    }
  }
  else
  {
    prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord = CSMD_HP_CODE_T6;
  }
  /* ------------------------------------------------------------- */
  /* Initialize Hot-Plug fields for CP3                            */
  /* ------------------------------------------------------------- */
  prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rSelection.usWord = CSMD_HP_ADD_BRDCST_ADD;
  
  prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rInfo.ulLong    = 
    prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017;
#else
  /* ------------------------------------------------------------- */
  /* Initialize Hot-Plug fields for CP3                            */
  /* ------------------------------------------------------------- */
  prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rSelection.usWord = CSMD_HP_ADD_BRDCST_ADD;
  
  prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord = CSMD_HP_CODE_T6;
  
  prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rInfo.ulLong    = 
    prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017;
#endif
  
  /* Initialize HP field of port 2 */
  prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0 = prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0;
  
  /* Copy MDT0 HP field port 1 into Tx Ram */
  CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P1_Struct.pulTxRam,
                      *((CSMD_ULONG *)(CSMD_VOID *)&prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0) );
  CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P1_Struct.pulTxRam + 1,
                      prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rInfo.ulLong );
  
  /* Copy MDT0 HP field port 2 into Tx Ram */
  CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P2_Struct.pulTxRam,
                      *((CSMD_ULONG *)(CSMD_VOID *)&prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0) );
  CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P2_Struct.pulTxRam + 1,
                      prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0.rInfo.ulLong );
  
  
  /* ------------------------------------------------------------- */
  /* Assignment of the service container according to the          */
  /* projected slave address list.                                 */
  /* ------------------------------------------------------------- */
  /* Buffer Basepointer: SVC */
  ulTxRamOffset = aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_SVC];
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
#if CSMD_MAX_HW_CONTAINER > 0
    if ((CSMD_SHORT)usI < CSMD_MAX_HW_CONTAINER )
    {
      /* disable hardware SVC */
      CSMD_HAL_WriteShort( &prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram->uwSVC_Pointer[usI], 
                           0U );
      
      prCSMD_Instance->rPriv.prSVContainer[usI] = 
        &prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram->rSC_S3[usI];
      prSVC = prCSMD_Instance->rPriv.prSVContainer[usI];
      
      CSMD_HAL_WriteShort( &prSVC->usSVCTxPointer_Control,
                           (CSMD_USHORT)(ulTxRamOffset + (usI * CSMD_SVC_FIELDWIDTH)) );
      
      /* Set M_Busy */
      usTempRam = CSMD_HAL_ReadShort(&prSVC->rCONTROL.usWord[0]);
      CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[0], (usTempRam |= CSMD_SVC_CTRL_M_BUSY) );
      
      if (TRUE == prCSMD_Instance->rPriv.boHW_SVC_Redundancy)
      {
        /* Enable SVC, enable AT check and set pointer to SVC */
        usTempRam = (CSMD_USHORT)(  (  CSMD_SVC_POINTER_CONT_EN
                                     | CSMD_SVC_POINTER_AT_CHECK
                                     | CSMD_SVC_POINTER_AT_SELECT_AT0)
                                  | (CSMD_USHORT)(  (CSMD_UCHAR *)&prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram->rSC_S3[usI]
                                                  - (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram));
      }
      else
      {
        /* Enable SVC and set pointer to SVC */
        usTempRam = (CSMD_USHORT)(  CSMD_SVC_POINTER_CONT_EN
                                  | (CSMD_USHORT)(  (CSMD_UCHAR *)&prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram->rSC_S3[usI]
                                                  - (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram));
      }
      
      CSMD_HAL_WriteShort( &prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram->uwSVC_Pointer[usI], 
                           usTempRam );
    }
    
    /* enable emulated svc */
    else    /* if (usI < CSMD_MAX_HW_CONTAINER ) */
#endif  /* #if CSMD_MAX_HW_CONTAINER > 0 */
#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
    {
      /*-------------------------------------------------------------------------- */
      /* Status and control words are pointed to the memory locations of Status    */
      /* and Control words in the RX and TX ram respectively                       */
      /*-------------------------------------------------------------------------- */
      prCSMD_Instance->rPriv.prSVContainer[usI] = 
        &prCSMD_Instance->rPriv.parSoftSvcContainer[usI - CSMD_MAX_HW_CONTAINER];
      prSVC = prCSMD_Instance->rPriv.prSVContainer[usI];
      
      prSVC->usSVCTxPointer_Control =
        (CSMD_USHORT)(ulTxRamOffset +(usI * CSMD_SVC_FIELDWIDTH));
      
      prCSMD_Instance->rPriv.parSoftSvc[usI - CSMD_MAX_HW_CONTAINER].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      
      /* Set M_Busy and restore HS */
      prCSMD_Instance->rPriv.prSVContainer[usI]->rCONTROL.usWord[0] =
        (CSMD_USHORT)(CSMD_SVC_CTRL_M_BUSY | (prCSMD_Instance->rPriv.ausSERC3_MDT_Control_SVC[usI] & CSMD_SVC_CTRL_HANDSHAKE));

    }   /* if (usI < CSMD_MAX_HW_CONTAINER ) */
#endif
    /* Restore state of MHS bit; saved before change telegram configuration forCP3 */
    usTempRam = (CSMD_USHORT)(prCSMD_Instance->rPriv.ausSERC3_MDT_Control_SVC[usI] & CSMD_SVC_CTRL_HANDSHAKE);
    
    CSMD_HAL_WriteShort( ((CSMD_USHORT *)(CSMD_VOID *)prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram
                                           + (((CSMD_USHORT)ulTxRamOffset + (CSMD_USHORT)(usI * CSMD_SVC_FIELDWIDTH)) / 2)),
                         usTempRam );
    
#ifdef CSMD_PCI_MASTER
    if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
    {
      /* Restore state of MHS bit in the local Tx telegram buffer */
      *((CSMD_USHORT *)(CSMD_VOID *)prCSMD_Instance->prTelBuffer->rLocal.rBuffTxRam.aulTx_Ram
                                        + (((CSMD_USHORT)ulTxRamOffset + (CSMD_USHORT)(usI * CSMD_SVC_FIELDWIDTH)) / 2)) =
        usTempRam;
    }
#endif
  }   /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
  
  
  /* No emulated SVC active */
  prCSMD_Instance->usSoftSrvcCnt = 0U;
  
  prCSMD_Instance->rPriv.usSVC_HS_Timeout   = CSMD_SERC3_HS_TIMEOUT;      /* 10 Cycles */
  
  if (prCSMD_Instance->rPriv.rHW_Init_Struct.usSVC_BusyTimeout > 2000)    /* [ms] */
  {
    ulTimeOut =                             /* in cycles */
      (((CSMD_ULONG)prCSMD_Instance->rPriv.rHW_Init_Struct.usSVC_BusyTimeout * 100000UL) / 
      prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002) * 10;
  }
  else
  {
    ulTimeOut =                             /* in cycles */
      ((CSMD_ULONG)prCSMD_Instance->rPriv.rHW_Init_Struct.usSVC_BusyTimeout * 1000000UL) / 
      prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002;
  }
  if (ulTimeOut > 65000)      /* in Cycles */
  {
    prCSMD_Instance->rPriv.usSVC_BUSY_Timeout = 65000;
  }
  else if (ulTimeOut < 1)     /* in Cycles */
  {
    prCSMD_Instance->rPriv.usSVC_BUSY_Timeout = 1;
  }
  else
  {
    prCSMD_Instance->rPriv.usSVC_BUSY_Timeout = (CSMD_USHORT) ulTimeOut;
  }
  
  /* Set Service Channel Control Register */ 
  CSMD_HAL_SetSVCTimeouts( &prCSMD_Instance->rCSMD_HAL, 
                           (CSMD_USHORT)((((CSMD_SHORT)prCSMD_Instance->rPriv.usSVC_BUSY_Timeout-2)/256)+2),
                           prCSMD_Instance->rPriv.usSVC_HS_Timeout );
  
#ifdef CSMD_HW_SVC_REDUNDANCY
  if (TRUE == prCSMD_Instance->rPriv.boHW_SVC_Redundancy)
  {
    CSMD_HAL_CtrlSVC_Redundancy( &prCSMD_Instance->rCSMD_HAL, TRUE);
  }
  else
#endif
  {
    CSMD_HAL_CtrlSVC_Redundancy( &prCSMD_Instance->rCSMD_HAL, FALSE);
    
    /* Select the port for the SVC machine trigger */
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
    {
      CSMD_HAL_SetSVCPort( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_SVCCSR_TRIG_PORT2 );
    }
    else
    {
      /* trigger SVC machine from port 1 */
      CSMD_HAL_SetSVCPort( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_SVCCSR_TRIG_PORT1 );
    }
    
    {
      CSMD_SHORT sTelNbr;
      for (sTelNbr = (CSMD_MAX_TEL -1); sTelNbr >= 0; sTelNbr--)
      {
        if (prCSMD_Instance->rPriv.rAT_Length[sTelNbr].usSVC)
        {
          /* Set the last telegram with contains SVC that trigger the SVC state machine */
          CSMD_HAL_SetSVCLastAT( &prCSMD_Instance->rCSMD_HAL, (CSMD_ULONG)(CSMD_LONG)sTelNbr );
          break;
        }
      }
    }
  }
  
  
  prCSMD_Instance->rPriv.usMDT_Enable = 0x0000;
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
  {
    if (prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[usTelI])
    {
      prCSMD_Instance->rPriv.aboMDT_used[usTelI] = TRUE;  /* Telegram is used */
      prCSMD_Instance->rPriv.usMDT_Enable |= (CSMD_USHORT)(1UL << usTelI);
      
    }
  }

#ifdef CSMD_PCI_MASTER
  /* ------------------------------------------------------------- */
  /* Configure the PCI RX DMA channels                             */
  /* ------------------------------------------------------------- */
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    /* 
    */
    usBuf = CSMD_TX_BUFFER_0;
    
    /* DMA for emulated Service channels */
    if ((CSMD_SHORT)prCSMD_Instance->rSlaveList.usNumProjSlaves > CSMD_MAX_HW_CONTAINER)
    {
      /*lint -save -e845 The left/right argument to operator '+/-' is certain to be 0 */
      CSMD_Config_DMA_Rx_Channel( 
        prCSMD_Instance,
        prCSMD_Instance->rCSMD_HAL.ulTxRamBase
          + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_SVC]
          + (CSMD_MAX_HW_CONTAINER * CSMD_SVC_FIELDWIDTH),
        (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffTxRam.aulTx_Ram
          + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_SVC]
          + (CSMD_MAX_HW_CONTAINER * CSMD_SVC_FIELDWIDTH),
        (CSMD_ULONG) (  usBuffSVC_Offset
                      - (CSMD_MAX_HW_CONTAINER * CSMD_SVC_FIELDWIDTH)),
        (CSMD_USHORT) CSMD_DMA_CHANNEL_00 );
      /*lint -restore */
      prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_00);
    }
    
    /* DMA for real time data buffer 0 */
    CSMD_Config_DMA_Rx_Channel( 
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulTxRamBase
        + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_0_SYS_A],
      (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffTxRam.aulTx_Ram
        + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_0_SYS_A],
      (CSMD_ULONG) usBuff0_Offset,
      (CSMD_USHORT) CSMD_DMA_CHANNEL_01 );
    
    prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_01);
    
    usTempRam = prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf];
    
    usBuf = CSMD_TX_BUFFER_1;
    if (usBuf < CSMD_MAX_TX_BUFFER)  /*lint !e774 Boolean within 'if' always evaluates to True */
    {
      /* DMA for emulated Service channels */
      if ((CSMD_SHORT)prCSMD_Instance->rSlaveList.usNumProjSlaves > CSMD_MAX_HW_CONTAINER)
      {
        prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_00);
      }
      
      /* DMA for real time data buffer 1 */
      CSMD_Config_DMA_Rx_Channel( 
        prCSMD_Instance,
        prCSMD_Instance->rCSMD_HAL.ulTxRamBase
          + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_1_SYS_A],
        (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffTxRam.aulTx_Ram
          + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_1_SYS_A],
        (CSMD_ULONG) usBuff0_Offset,
        (CSMD_USHORT) CSMD_DMA_CHANNEL_02 );
      
      prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_02);
      
      usTempRam |= prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf];
      
    }
    
    usBuf = CSMD_TX_BUFFER_2;
    if (usBuf < CSMD_MAX_TX_BUFFER)  /*lint !e774 Boolean within 'if' always evaluates to True */
    {
      /* DMA for emulated Service channels */
      if ((CSMD_SHORT)prCSMD_Instance->rSlaveList.usNumProjSlaves > CSMD_MAX_HW_CONTAINER)
      {
        prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_00);
      }
      
      /* DMA for real time data buffer 2 */
      CSMD_Config_DMA_Rx_Channel( 
        prCSMD_Instance,
        prCSMD_Instance->rCSMD_HAL.ulTxRamBase
          + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_2_SYS_A],
        (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffTxRam.aulTx_Ram
          + aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_2_SYS_A],
        (CSMD_ULONG) usBuff0_Offset,
        (CSMD_USHORT) CSMD_DMA_CHANNEL_03 );
      
      prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_03);
      
      usTempRam |= prCSMD_Instance->rPriv.ausRxDMA_Start[usBuf];
    }
    
    /* Set Enable_DMA and Enable_RDY_DMA for the selected channels. */
    CSMD_HAL_Enable_RXDMA( &prCSMD_Instance->rCSMD_HAL,
                           usTempRam );
  }
#endif

  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Config_TX_Tel_P3() */



#ifdef CSMD_PCI_MASTER
/**************************************************************************/ /**
\brief Configure one PCI RX DMA channel.

\ingroup module_dma
\b Description;: \n
   data source is host ram \n
   data destination is FPGA Tx RAM

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   ulDstAdd_S3Ram 
              Destination address offset in FPGA Tx RAM (long aligned)
\param [in]   ulSrcAdd_Host 
              Source address in host RAM (long aligned)
\param [in]   ulLength 
              Number of bytes to be transferred (shall be divisible by 4)
\param [in]   usRxChannelNbr 
              PCI RX DMA channel number (0...15)

\return       none

\author       WK
\date         09.02.2011

***************************************************************************** */
CSMD_VOID CSMD_Config_DMA_Rx_Channel( const CSMD_INSTANCE *prCSMD_Instance,
                                      CSMD_ULONG           ulDstAdd_S3Ram,
                                      CSMD_ULONG           ulSrcAdd_Host,
                                      CSMD_ULONG           ulLength,
                                      CSMD_USHORT          usRxChannelNbr )
{
  
  CSMD_HAL_SetDMALocalAddr( &prCSMD_Instance->rCSMD_HAL,
                            CSMD_RX_DMA,
                            usRxChannelNbr,
                            ulDstAdd_S3Ram );
  
  CSMD_HAL_SetDMAPCIAddr(   &prCSMD_Instance->rCSMD_HAL,
                            CSMD_RX_DMA,
                            usRxChannelNbr,
                            ulSrcAdd_Host );
  
  CSMD_HAL_SetDMACounter(   &prCSMD_Instance->rCSMD_HAL,
                            CSMD_RX_DMA,
                            usRxChannelNbr,
                            ulLength );
  
  CSMD_HAL_SetDMARDYAddr(   &prCSMD_Instance->rCSMD_HAL,
                            CSMD_RX_DMA,
                            usRxChannelNbr,
                            (CSMD_ULONG)&prCSMD_Instance->prTelBuffer_Phys->rLocal.rDMA_RxRdy.aulFlag[usRxChannelNbr] );
  
}  /* end: CSMD_Config_DMA_Rx_Channel */
#endif


/*! \endcond */ /* PRIVATE */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
02 Dec 2013 WK
  - Defdb00165150
    CSMD_Config_TX_Tel_P3()
    Save length of port relative buffers for CC data deletion.
03 Dec 2013 WK
  - Defdb00150926
    Fixed Lint message 740 "Unusual pointer cast (incompatible indirect types)"
    for CSMD_MAX_HW_CONTAINER > 0.
09 Dec 2013 WK
  - Defdb00165521
    CSMD_Config_TX_Tel_P3()
    Fixed compiler problem for not defined CSMD_HOTLPUG:
      Calculate rHP_P2_Struct.pulTxRam independent from CSMD_HOTPLUG.
28 Jul 2014 WK
  - Defdb00172728
    CSMD_Config_TX_Tel_P3()
    Reduce use of TxRam in case of CC connections. One pair of Tx-Descriptors
    for the complete block of all CC connections.
09 Oct 2014 WK
  - Defdb00174053
    CSMD_Config_TX_Tel_P3()
    Adjust HP0 parameter transmission repeat rate.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
28 Apr 2015 WK
  - Defdb00178597
    CSMD_Config_RX_Tel_P3()
    Fixed type castings for 64-bit systems.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
14 Aug 2015 WK
  - Defdb00181153
    CSMD_Config_TX_Tel_P3()
    Restore additionally C-SVC.HS in emulated SVC container for query
    in function CSMD_IsReady_Soft_SVC_Status().
01 Oct 2015 WK
  - Defdb00182067
    Fixed compiler warnings "precision lost" respectively Lint messages
    737 "Loss of sign in promotion from int to unsigned int".
14 Oct 2015 WK
  - Fixed "precision lost" compiler warning.
19 Oct 2015 WK
  - Defdb00182406
    CSMD_Config_TX_Tel_P1()
    Enable DMA operation for SVC in CP2, if no IP-Core SVC used.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
28 Jan 2016 WK
  - Defdb00182067
    CSMD_Config_TX_Tel_P3()
    Fixed lint respectively compiler warning for defined CSMD_MASTER_PRODUCE_IN_AT.
    CSMD_Config_TX_Tel_P3()
    Fixed lint warning respectively precision lost compiler warning.

------------------------------------------------------------------------------
*/
