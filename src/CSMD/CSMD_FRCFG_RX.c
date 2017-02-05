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
\file   CSMD_FRCFG_RX.c
\author WK
\date   01.09.2010
 \brief  This File contains the private functions for the
         receive telegram configuration of all the communication phases.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"
#include "CSMD_HAL_DMA.h"

#include "CSMD_CP_AUX.h"
#include "CSMD_HOTPLUG.h"
#include "CSMD_PRIV_SVC.h"


#define SOURCE_CSMD
#include "CSMD_FRCFG_RX.h"

/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/*! \endcond */ /* PUBLIC */


/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Configures receive telegram for CP0.

\ingroup module_phase
\b Description: \n
   This function configures the receive telegram for CP0, including:
   - Clearance of descriptor index table and buffer base pointer list
   - Configuration for monitoring of telegram length by FPGA
   - Configuration of FCS field for Rx-descriptors for MDT
   - Configuration of AT0 data
   - Transmission of new descriptor index table and buffer base pointer list
     to RxRam/corresponding FPGA register
   - Initialization of AT0 in RxRam

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         14.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Config_RX_Tel_P0( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_DESCR_INDEX arRxDesTable[ CSMD_HAL_RX_DESR_INDEX_NUMBER ];
  CSMD_ULONG       aulRxBufBasePtr[ CSMD_HAL_RX_BASE_PTR_NBR ];   /* Rx Buffer base pointer list */
  
  CSMD_ULONG       ulRxRamOffset;     /* Current byte offset in RxRam */
  CSMD_USHORT      usPort_Offset;     /* Current offset [byte] in port buffer */
  CSMD_USHORT      usTelegramOffset;  /* Offset [byte] in current telegram */
  CSMD_USHORT      usIdxTableOffs;    /* Rx-Descriptor table index offset in RxRam */
  CSMD_UCHAR      *pucTel;            /* Pointer to current Telegram */
  CSMD_USHORT      usTelI;            /* Telegram Index */
  CSMD_USHORT      usI;               /* Index */
  CSMD_ULONG       ulL_Offset;        /* Ram Offset in # Longs */
  
  
#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    CSMD_HAL_ResetTXDMA( &prCSMD_Instance->rCSMD_HAL );
  }
#endif  
  
  ulRxRamOffset  = 0U;    /* Byte offset in RxRam     */
  
  /* Rx-Descriptor index table offset in RxRam */
  usIdxTableOffs = (CSMD_USHORT) ulRxRamOffset;
  CSMD_HAL_SetDesIdxTableOffsRxRam( &prCSMD_Instance->rCSMD_HAL, 
                                    usIdxTableOffs );
  
  /* ------------------------------------------------------------- */
  /* Clear Rx descriptor index table                               */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_RX_DESR_INDEX_NUMBER; usI++)
  {
    arRxDesTable[usI].ulIndex = 0;
  }
  
  ulRxRamOffset += sizeof (arRxDesTable);     /* Reserve space for 4 MDT and 4 AT index table entries */
  
  
  /* ---------------------------------------------------------------- */
  /* Configure received MDT0 for monitoring telegram length by FPGA.  */
  /* ---------------------------------------------------------------- */
  /* Descriptor Index table entry for MDT */
  arRxDesTable[CSMD_DES_IDX_MDT0].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
  arRxDesTable[CSMD_DES_IDX_MDT0].ulIndex |= (CSMD_ULONG) ((ulRxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
  
  usTelegramOffset = CSMD_SERC3_MDT_DATA_LENGTH_P0;  /* Offset [byte] in current telegram */
  
  /* ------------------------------------------------------------- */
  /* Configure FCS field Rx-Descriptors for MDT                    */
  /* ------------------------------------------------------------- */
  CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulRxRamOffset,                /* Descriptor offset in RxRam   */
                            0U,                           /* Buffer offset (irrelevant)   */
                            0U,                           /* Buffer System Select         */
                            usTelegramOffset,             /* Offset after CRC in telegram */
                            CSMD_RX_DES_TYPE_FCSP );      /* Descriptor type              */
  ulRxRamOffset += 4;
  
  
  /* ------------------------------------------------------------- */
  /*   Configure AT0 data                                          */
  /* ------------------------------------------------------------- */
  usTelI   = 0U;   /* Telegram index   */
  
  /* Descriptor Index table entry for AT0 */
  arRxDesTable[CSMD_DES_IDX_AT0].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
  arRxDesTable[CSMD_DES_IDX_AT0].ulIndex |= (CSMD_ULONG) ((ulRxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
  
  usTelegramOffset = 0U;  /* Offset [byte] in current telegram */
  usPort_Offset    = 0U;  /* Byte offset in Rx Buffer */
  
  
  /* MDT0 data field */
  CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulRxRamOffset,                /* Descriptor offset in RxRam   */
                            usPort_Offset,                /* Buffer offset in byte        */
                            0U,                           /* Buffer System Select         */
                            usTelegramOffset,             /* Offset after CRC in telegram */
                            CSMD_RX_DES_TYPE_RTDSP );     /* Descriptor type              */
  ulRxRamOffset += 4;
  
  
  usPort_Offset    += prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel;
  usTelegramOffset += prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel;
  CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulRxRamOffset,                        /* Descriptor offset in RxRam   */
                            (CSMD_USHORT)(usPort_Offset - 2),     /* Buffer offset in byte        */
                            0U,                                   /* Buffer System Select         */
                            (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                            CSMD_RX_DES_TYPE_RTDEP );             /* Descriptor type              */
  ulRxRamOffset += 4;
  
  
  /* MDT0 FCS field */
  CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                            ulRxRamOffset,                /* Descriptor offset in RxRam   */
                            usPort_Offset,                /* Buffer offset in byte        */
                            0U,                           /* Buffer System Select         */
                            usTelegramOffset,             /* Offset after CRC in telegram */
                            CSMD_RX_DES_TYPE_FCSP );      /* Descriptor type              */
  ulRxRamOffset += 4;
  
  
  /* Round up to the next divisible by 16 */
  ulRxRamOffset = ((ulRxRamOffset + 15) / 16) * 16;
  prCSMD_Instance->rPriv.ulRxRamTelOffset = ulRxRamOffset;
  
  /* ------------------------------------------------------------- */
  /* Initialize Rx Buffer base pointer list                        */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_RX_BASE_PTR_NBR; usI++)
  {
    aulRxBufBasePtr[usI] = ulRxRamOffset;
  }
  
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A] = ulRxRamOffset;
  ulRxRamOffset = ((ulRxRamOffset + usPort_Offset + 3) / 4) * 4;
  
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A] = ulRxRamOffset;
  ulRxRamOffset = ((ulRxRamOffset + usPort_Offset + 3) / 4) * 4;
  
#ifdef CSMD_DEBUG
  prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A] = usPort_Offset;
  
  prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A] = usPort_Offset;
#endif
  
  /* Used FPGA RxRam memory for Sercos telegrams */
  prCSMD_Instance->rCSMD_HAL.ulRxRamInUse = (((ulRxRamOffset - 1) / CSMD_IP_RAM_SEG_SIZE) + 1) * CSMD_IP_RAM_SEG_SIZE;
  
  ulL_Offset = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A] / 4;
  
  /* Pointer to start of AT port 1 in RxRam */
  prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[CSMD_RX_BUFFER_0][usTelI] = 
    (CSMD_ULONG *) &prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram[ ulL_Offset ];
  
  prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[usTelI] = 
    (CSMD_ULONG *) &prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aulRx_Ram[ ulL_Offset ];

#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    CSMD_Config_DMA_Tx_Channel(
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulRxRamBase + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A],
      (CSMD_ULONG) &prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram[ ulL_Offset ],
      (CSMD_ULONG) prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel,
      (CSMD_USHORT) CSMD_DMA_CHANNEL_00 );
  }
  prCSMD_Instance->rPriv.ausTxDMA_Start_P1[CSMD_RX_BUFFER_0] |= (0x0001 << (CSMD_ULONG) CSMD_DMA_CHANNEL_00);
#endif
  
  
  ulL_Offset = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A] / 4;
  
  /* Pointer to start of AT port 2 in RxRam */
  prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[CSMD_RX_BUFFER_0][usTelI] = 
    (CSMD_ULONG *) &prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram[ ulL_Offset ];
  
  prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[usTelI] = 
    (CSMD_ULONG *) &prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aulRx_Ram[ ulL_Offset ];

#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    CSMD_Config_DMA_Tx_Channel(
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulRxRamBase + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A],
      (CSMD_ULONG) &prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram[ ulL_Offset ],
      (CSMD_ULONG) prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel,
      (CSMD_USHORT) CSMD_DMA_CHANNEL_01 );
  }
  prCSMD_Instance->rPriv.ausTxDMA_Start_P2[CSMD_RX_BUFFER_0] |= (0x0001 << (CSMD_ULONG) CSMD_DMA_CHANNEL_01);
#endif
  
  prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usTelI] = 
    prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel;
  
  
  usTelI = 1U;
  
  prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[usTelI] = NULL;
  prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[usTelI] = NULL;
  prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usTelI] = 0U;
  
  
  /* ------------------------------------------------------------- */
  /* Transmit Descriptor index table into FPGA RxRam               */
  /* ------------------------------------------------------------- */
  usIdxTableOffs >>= 2;   /* Offset [Longs] in RxRam */ /*lint !e845 The left argument to operator '>>' is certain to be 0  */
  
  for (usI = 0; usI < CSMD_HAL_RX_DESR_INDEX_NUMBER; usI++)
  {
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram[usIdxTableOffs],
                        (arRxDesTable[usI].ulIndex) );
    usIdxTableOffs++;
  }
  
  
  /* ------------------------------------------------------------- */
  /* Transmit Rx Buffer base pointer list into FPGA register       */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_RX_BASE_PTR_NBR; usI++)
  {
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->aulRxBufBasePtr[usI], 
                        aulRxBufBasePtr[usI] );
#ifdef CSMD_DEBUG
    prCSMD_Instance->rCSMD_Debug.aulRxBufBasePtr[usI] = aulRxBufBasePtr[usI];
#endif
  }
  
  
  /* ------------------------------------------------------------- */
  /* Initialize AT0 in Rx Ram                                      */
  /* ------------------------------------------------------------- */
  
  /* Clear all allocated telegrams in FPGA Rx Ram */
  pucTel = (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram
            + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A];
  
  CSMD_Telegram_Clear( prCSMD_Instance, 
                       pucTel,
                       (CSMD_USHORT)(  prCSMD_Instance->rCSMD_HAL.ulRxRamInUse
                                     - aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A]),
                       0U );
  
  prCSMD_Instance->rPriv.aboAT_used[0] = TRUE;
  prCSMD_Instance->rPriv.usAT_Enable = 0x0001;

  CSMD_HAL_Config_Rx_Buffer_Valid( &prCSMD_Instance->rCSMD_HAL,
                                   CSMD_HAL_RX_BUFFER_SYSTEM_A, 
                                   0U,    /* Received MDTs extraneous */
                                   (CSMD_ULONG) prCSMD_Instance->rPriv.usAT_Enable );
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Config_RX_Tel_P0() */



/**************************************************************************/ /**
\brief Configures receive telegram for CP1.

\ingroup module_phase
\b Description: \n
   This function configures the receive telegram for CP1, including:
   - Clearance of descriptor index table and buffer base pointer list
   - Configuration for monitoring of telegram length by FPGA
   - Configuration of FCS field for Rx-descriptors for MDT
   - Configuration of AT Rx-descriptors (SVC data, RT data and FCS field)
   - Clearance of data in RxRam for all configured telegrams
   - transmission of new descriptor index table and buffer base pointer list
     to RxRam/corresponding FPGA register
   - Setting of pointers to device status in AT
   - Assignment of the service container according to the projected slave
     address list.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         14.02.2005
    
***************************************************************************** */
CSMD_FUNC_RET CSMD_Config_RX_Tel_P1( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_DESCR_INDEX arRxDesTable[ CSMD_HAL_RX_DESR_INDEX_NUMBER ];
  CSMD_ULONG       aulRxBufBasePtr[ CSMD_HAL_RX_BASE_PTR_NBR ];   /* Rx Buffer base pointer list */
  
  CSMD_ULONG       ulRxRamOffset;     /* Current byte offset in RxRam              */
  CSMD_USHORT      usBuff0_Offset;    /* Current offset [byte] in Rx buffer 0      */
  CSMD_USHORT      usBuffSVC_Offset;  /* Current offset [byte] in Rx buffer SVC    */
  CSMD_USHORT      usTelegramOffset;  /* Offset [byte] in current telegram         */
  CSMD_USHORT      usIdxTableOffs;    /* Rx-Descriptor table index offset in RxRam */
  CSMD_USHORT      usLengthRTD;       /* Length of RT data in MDT/AT Telegrams     */
  CSMD_USHORT      usLengthSVC;       /* Length of SVC     in MDT/AT Telegrams     */
  CSMD_ULONG       ulL_Offset;        /* Ram Offset in # Longs */
  
  CSMD_USHORT      usTelI;            /* Telegram Index */
  CSMD_UCHAR      *pucTel;            /* Pointer to current Telegram */
  CSMD_USHORT      usI;               /* Index */
  CSMD_ULONG       ulOffsetSvcRxP1;   /* Offset for SVC in RxRam port 1 */
  CSMD_ULONG       ulOffsetSvcRxP2;   /* Offset for SVC in RxRam port 2 */
  CSMD_USHORT      usTAdd;            /* Slave topology address */
  CSMD_SERC3SVC   *prSVC;
  CSMD_USHORT      usSVCOffset;       /* SVC Offset of current slave related to beginn of SVC block */


#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    CSMD_HAL_ResetTXDMA( &prCSMD_Instance->rCSMD_HAL );
  }
#endif  
  
  ulRxRamOffset  = 0U;    /* Byte offset in RxRam     */
  
  /* Rx-Descriptor index table offset in RxRam */
  usIdxTableOffs = (CSMD_USHORT) ulRxRamOffset;
  CSMD_HAL_SetDesIdxTableOffsRxRam( &prCSMD_Instance->rCSMD_HAL, 
                                    usIdxTableOffs );
  
  /* ------------------------------------------------------------- */
  /* Clear Rx descriptor index table                               */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_RX_DESR_INDEX_NUMBER; usI++)
  {
    arRxDesTable[usI].ulIndex = 0;
  }
  
  ulRxRamOffset += sizeof (arRxDesTable);     /* Reserve space for 4 MDT and 4 AT index table entries */
  
  usBuff0_Offset   = 0;   /* Byte offset in Rx Buffer 0    */
  usBuffSVC_Offset = 0;   /* Byte offset in Rx Buffer SVC  */
  usLengthSVC      = CSMD_SERC3_SVC_DATA_LENGTH_P12;
  usLengthRTD      = CSMD_SERC3_RTD_DATA_LENGTH_P12;
  
  /* ---------------------------------------------------------------- */
  /* Configure received MDT0 for monitoring telegram length by FPGA.  */
  /* ---------------------------------------------------------------- */
  for (usTelI = 0; usTelI < prCSMD_Instance->rPriv.usNbrTel_CP12; usTelI++)
  {
    /* Descriptor Index table entry for MDT */
    arRxDesTable[usTelI + CSMD_HAL_RX_MDT_DESR_IDX_OFFSET].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
    arRxDesTable[usTelI + CSMD_HAL_RX_MDT_DESR_IDX_OFFSET].ulIndex |= (CSMD_ULONG) ((ulRxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
    
    usTelegramOffset = CSMD_SERC3_DATA_LENGTH_P12;  /* Offset [byte] in current telegram */
    
    /* ------------------------------------------------------------- */
    /* Configure FCS field Rx-Descriptors for MDT                    */
    /* ------------------------------------------------------------- */
    CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulRxRamOffset,                /* Descriptor offset in RxRam   */
                              0U,                           /* Buffer offset (irrelevant)   */
                              0U,                           /* Buffer System Select         */
                              usTelegramOffset,             /* Offset after CRC in telegram */
                              CSMD_RX_DES_TYPE_FCSP );      /* Descriptor type              */
    ulRxRamOffset += 4;
  }
  
  
  /* ------------------------------------------------------------- */
  /* Configure AT                                                  */
  /* ------------------------------------------------------------- */
  for (usTelI = 0; usTelI < prCSMD_Instance->rPriv.usNbrTel_CP12; usTelI++)
  {
    /* Descriptor Index table entry for MDT */
    arRxDesTable[usTelI + CSMD_HAL_RX_AT_DESR_IDX_OFFSET].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
    arRxDesTable[usTelI + CSMD_HAL_RX_AT_DESR_IDX_OFFSET].ulIndex |= (CSMD_ULONG) ((ulRxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
    
    usTelegramOffset = 0U;  /* Offset [byte] in current telegram */
  
    
    /* ------------------------------------------------------------- */
    /* Configure SVC Rx-Descriptors for AT                           */
    /* ------------------------------------------------------------- */
    CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulRxRamOffset,                /* Descriptor offset in RxRam   */
                              usBuffSVC_Offset,             /* Buffer offset in byte        */
                              0U,                           /* Buffer System Select         */
                              usTelegramOffset,             /* Offset after CRC in telegram */
                              CSMD_RX_DES_TYPE_SVDSP );     /* Descriptor type              */
    ulRxRamOffset += 4;
    
    
    usBuffSVC_Offset += usLengthSVC;
    usTelegramOffset += usLengthSVC;
    CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulRxRamOffset,                        /* Descriptor offset in RxRam   */
                              (CSMD_USHORT)(usBuffSVC_Offset - 2),  /* Buffer offset in byte        */
                              0U,                                   /* Buffer System Select         */
                              (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                              CSMD_RX_DES_TYPE_SVDEP );             /* Descriptor type              */
    ulRxRamOffset += 4;
    
    
    /* ------------------------------------------------------------- */
    /* Configure RT data Rx-Descriptors for AT                       */
    /* ------------------------------------------------------------- */
    CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulRxRamOffset,                /* Descriptor offset in RxRam   */
                              usBuff0_Offset,               /* Buffer offset in byte        */
                              0U,                           /* Buffer System Select         */
                              usTelegramOffset,             /* Offset after CRC in telegram */
                              CSMD_RX_DES_TYPE_RTDSP );     /* Descriptor type              */
    ulRxRamOffset += 4;
    
    
    usBuff0_Offset   += usLengthRTD;
    usTelegramOffset += usLengthRTD;
    CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulRxRamOffset,                        /* Descriptor offset in RxRam   */
                              (CSMD_USHORT)(usBuff0_Offset - 2),    /* Buffer offset in byte        */
                              0U,                                   /* Buffer System Select         */
                              (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                              CSMD_RX_DES_TYPE_RTDEP );             /* Descriptor type              */
    ulRxRamOffset += 4;
    
    
    /* ------------------------------------------------------------- */
    /* Configure FCS field Rx-Descriptors for AT                     */
    /* ------------------------------------------------------------- */
    CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                              ulRxRamOffset,                /* Descriptor offset in RxRam   */
                              0U,                           /* Buffer offset (irrelevant)   */
                              0U,                           /* Buffer System Select         */
                              usTelegramOffset,             /* Offset after CRC in telegram */
                              CSMD_RX_DES_TYPE_FCSP );      /* Descriptor type              */
    ulRxRamOffset += 4;
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[usTelI] = 0;
  }
  
  if (prCSMD_Instance->rSlaveList.usNumProjSlaves == 0U)
  {
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[0] = 0U;
  }
  else if (prCSMD_Instance->rSlaveList.usNumProjSlaves < CSMD_MAX_SLAVES_PER_TEL_CP1_2)
  {
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[0] = (CSMD_USHORT)(prCSMD_Instance->rSlaveList.usNumProjSlaves - 1);
  }
  else if (prCSMD_Instance->rSlaveList.usNumProjSlaves < 2 * CSMD_MAX_SLAVES_PER_TEL_CP1_2)
  {
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[0] = CSMD_MAX_SLAVES_PER_TEL_CP1_2 - 1;
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[1] = (CSMD_USHORT)(prCSMD_Instance->rSlaveList.usNumProjSlaves - 1);
  }
#ifdef CSMD_4MDT_4AT_IN_CP1_2
  else if (prCSMD_Instance->rSlaveList.usNumProjSlaves < 3 * CSMD_MAX_SLAVES_PER_TEL_CP1_2)
  {
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[0] = CSMD_MAX_SLAVES_PER_TEL_CP1_2 - 1;
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[1] = 2 * CSMD_MAX_SLAVES_PER_TEL_CP1_2 - 1;
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[2] = (CSMD_USHORT)(prCSMD_Instance->rSlaveList.usNumProjSlaves - 1);
  }
  else /* if (prCSMD_Instance->rSlaveList.usNumProjSlaves < 3 * CSMD_MAX_SLAVES_PER_TEL_CP1_2) */
  {
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[0] = CSMD_MAX_SLAVES_PER_TEL_CP1_2 - 1;
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[1] = 2 * CSMD_MAX_SLAVES_PER_TEL_CP1_2 - 1;
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[2] = 3 * CSMD_MAX_SLAVES_PER_TEL_CP1_2 - 1;
    prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[3] = (CSMD_USHORT)(prCSMD_Instance->rSlaveList.usNumProjSlaves - 1);
  }
#endif
  
  /* Round up to the next divisible by 16 */
  ulRxRamOffset = ((ulRxRamOffset + 15) / 16) * 16;
  prCSMD_Instance->rPriv.ulRxRamTelOffset = ulRxRamOffset;
  
  /* ------------------------------------------------------------- */
  /* Initialize Rx Buffer base pointer list                        */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_RX_BASE_PTR_NBR; usI++)
  {
    aulRxBufBasePtr[usI] = ulRxRamOffset;
  }
  
  /* Buffer Basepointer: Port 1 SVC */
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_SVC] = ulRxRamOffset;
  ulRxRamOffset = ((ulRxRamOffset + usBuffSVC_Offset + 3) / 4) * 4;
  
  /* Buffer Basepointer: Port 1 System A Buffer 0 */
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A] = ulRxRamOffset;
  ulRxRamOffset = ((ulRxRamOffset + usBuff0_Offset + 3) / 4) * 4;
  
  /* Buffer Basepointer: Port 2 SVC */
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_SVC] = ulRxRamOffset;
  ulRxRamOffset = ((ulRxRamOffset + usBuffSVC_Offset + 3) / 4) * 4;
  
  /* Buffer Basepointer: Port 2 System A Buffer 0 */
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A] = ulRxRamOffset;
  ulRxRamOffset = ((ulRxRamOffset + usBuff0_Offset + 3) / 4) * 4;
  
  /* Used FPGA RxRam memory for serocs telegrams */
  prCSMD_Instance->rCSMD_HAL.ulRxRamInUse = (((ulRxRamOffset - 1) / CSMD_IP_RAM_SEG_SIZE) + 1) * CSMD_IP_RAM_SEG_SIZE;
  
#ifdef CSMD_DEBUG
  prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P1_BUFF_SVC]     = usBuffSVC_Offset;
  prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A] = usBuff0_Offset;
  
  prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P2_BUFF_SVC]     = usBuffSVC_Offset;
  prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A] = usBuff0_Offset;
#endif
  
  /* Pointer to start of telegram Tx Ram */
  pucTel = (CSMD_UCHAR *) 
      &prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram[aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_SVC] / 4];
  
  /* ------------------------------------------------------------- */
  /* Clear data (SVC /RTD) in Rx-Ram for all configured telegrams  */
  /* ------------------------------------------------------------- */
  CSMD_Telegram_Clear( prCSMD_Instance, 
                       pucTel,                                                        /* Pointer to TxRam */
                       (CSMD_USHORT)(  prCSMD_Instance->rCSMD_HAL.ulRxRamInUse        /* Data length      */
                                     - aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_SVC]),
                       0U                                                             /* Fill value       */
                     );
  
  
  /* ------------------------------------------------------------- */
  /* Transmit Descriptor index table into FPGA RxRam               */
  /* ------------------------------------------------------------- */
  usIdxTableOffs >>= 2;   /* Offset [Longs] in RxRam */ /*lint !e845 The left argument to operator '>>' is certain to be 0  */
  
  for (usI = 0; usI < CSMD_HAL_RX_DESR_INDEX_NUMBER; usI++)
  {
      CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram[usIdxTableOffs],
                          (arRxDesTable[usI].ulIndex) );
      usIdxTableOffs++;
  }
  
  /* ------------------------------------------------------------- */
  /* Transmit Rx Buffer base pointer list into FPGA register       */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_RX_BASE_PTR_NBR; usI++)
  {
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->aulRxBufBasePtr[usI], 
                        aulRxBufBasePtr[usI] );
#ifdef CSMD_DEBUG
    prCSMD_Instance->rCSMD_Debug.aulRxBufBasePtr[usI] = aulRxBufBasePtr[usI];
#endif
  }
  
  /* Prepare Segment 1 and 2 for CP1/CP2. */
  /* In CP1 only segment 1 will be used!  */
  
  usI = 0;    /* 1. Segment */
  
  /* SVC for all projected slaves of port 1 */
  ulL_Offset = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_SVC] / 4;
  
  prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[CSMD_RX_BUFFER_0][usI] = 
    (CSMD_ULONG *) &prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram[ulL_Offset];
  prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[usI] = 
    (CSMD_ULONG *) &prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aulRx_Ram[ulL_Offset];
  
  prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usI] = 
    (CSMD_USHORT) ((((prCSMD_Instance->rSlaveList.usNumProjSlaves + 1) * CSMD_SVC_FIELDWIDTH + 3) / 4) * 4);
  
#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    CSMD_Config_DMA_Tx_Channel(
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulRxRamBase + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_SVC],
      (CSMD_ULONG) &prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram[ulL_Offset],
      (CSMD_ULONG) prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usI],
      (CSMD_USHORT) CSMD_DMA_CHANNEL_00 );
  }
  prCSMD_Instance->rPriv.ausTxDMA_Start_P1[CSMD_RX_BUFFER_0] |= (0x0001 << (CSMD_ULONG) CSMD_DMA_CHANNEL_00);
#endif
    
  /* SVC for all projected slaves of port 2 */
  ulL_Offset = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_SVC] / 4;
  
  prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[CSMD_RX_BUFFER_0][usI] = 
    (CSMD_ULONG *) &prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram[ulL_Offset];
  prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[usI] = 
    (CSMD_ULONG *) &prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aulRx_Ram[ulL_Offset];

#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    CSMD_Config_DMA_Tx_Channel(
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulRxRamBase + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_SVC],
      (CSMD_ULONG) &prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram[ulL_Offset],
      (CSMD_ULONG) prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usI],
      (CSMD_USHORT) CSMD_DMA_CHANNEL_02 );
  }
  prCSMD_Instance->rPriv.ausTxDMA_Start_P2[CSMD_RX_BUFFER_0] |= (0x0001 << (CSMD_ULONG) CSMD_DMA_CHANNEL_02);
#endif
  
  prCSMD_Instance->rPriv.aboAT_used[usI] = TRUE;   /* 1. Segment is used */
  
  
  usI = 1;    /* 2. Segment */
  
  /* Device Status for all projected slaves of port 1 */
  ulL_Offset = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A] / 4;
  prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[CSMD_RX_BUFFER_0][usI] = 
    (CSMD_ULONG *) &prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram[ulL_Offset];
  prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[usI] = 
    (CSMD_ULONG *) &prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aulRx_Ram[ulL_Offset];

  prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usI] = 
    (CSMD_USHORT) ((((prCSMD_Instance->rSlaveList.usNumProjSlaves + 1) * CSMD_S_DEV_LENGTH_CP1_2 + 3) / 4) * 4);
  
#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    CSMD_Config_DMA_Tx_Channel(
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulRxRamBase + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A],
      (CSMD_ULONG) &prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram[ulL_Offset],
      (CSMD_ULONG) prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usI],
      (CSMD_USHORT) CSMD_DMA_CHANNEL_01 );
  }
  prCSMD_Instance->rPriv.ausTxDMA_Start_P1[CSMD_RX_BUFFER_0] |= (0x0001 << (CSMD_ULONG) CSMD_DMA_CHANNEL_01);
#endif
  
  /* Device Status for all projected slaves of port 2 */
  ulL_Offset = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A] / 4;
  
  prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[CSMD_RX_BUFFER_0][usI] = 
    (CSMD_ULONG *) &prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram[ulL_Offset];
  prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[usI] = 
    (CSMD_ULONG *) &prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aulRx_Ram[ulL_Offset];
  
#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    CSMD_Config_DMA_Tx_Channel(
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulRxRamBase + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A],
      (CSMD_ULONG) &prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram[ulL_Offset],
      (CSMD_ULONG) prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usI],
      (CSMD_USHORT) CSMD_DMA_CHANNEL_03 );
  }
  prCSMD_Instance->rPriv.ausTxDMA_Start_P2[CSMD_RX_BUFFER_0] |= (0x0001 << (CSMD_ULONG) CSMD_DMA_CHANNEL_03);
#endif
   
  prCSMD_Instance->rPriv.aboAT_used[usI] = FALSE;   /* 2. Segment not used ! */
  
  
  
  /* ------------------------------------------------------------- */
  /*  Configure variables (Idx 0 used for SVC / 1 used for S-DEV)  */
  /* ------------------------------------------------------------- */
  /* Number of Telegrams: 4 MDT and 4 AT in CP1 and CP2 */
  if (prCSMD_Instance->rPriv.usNbrTel_CP12 == 4U)
  {
    prCSMD_Instance->rPriv.usAT_Enable  = 0x000F;
    prCSMD_Instance->rPriv.usMDT_Enable = 0x000F;
  }
  else
  {
    prCSMD_Instance->rPriv.usAT_Enable  = 0x0003;
    prCSMD_Instance->rPriv.usMDT_Enable = 0x0003;
    
    for (usI = 2; usI < CSMD_MAX_TEL_P0_2; usI++)    /*lint !e681: Loop is not entered */
    {
      prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[usI] = NULL;
      prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[usI] = NULL;
      prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usI] = 0U;
    }
  }
  
  /* Perform buffer switch in every Sercos cycle. */
  CSMD_HAL_Config_Rx_Buffer_Valid( &prCSMD_Instance->rCSMD_HAL,
                                   CSMD_HAL_RX_BUFFER_SYSTEM_A, 
                                   0U, /* Received MDTs extraneous */
                                   0U );
  
  
  /* ------------------------------------------------------------- */
  /* Set pointer to device status in AT                            */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    usTAdd = prCSMD_Instance->rPriv.ausTopologyAddresses[usI];
    
#ifdef CSMD_PCI_MASTER
    if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
    {
      /* Pointer to device status port 1 in RxRam */
      prCSMD_Instance->rPriv.apusS_DEV[usI][CSMD_PORT_1][CSMD_RX_BUFFER_0] =
        (CSMD_USHORT *) (CSMD_VOID *)(prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[1] + usTAdd);
      
      /* Pointer to device status port 2 in RxRam */
      prCSMD_Instance->rPriv.apusS_DEV[usI][CSMD_PORT_2][CSMD_RX_BUFFER_0] =
        (CSMD_USHORT *) (CSMD_VOID *) (prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[1] + usTAdd);
    }
    else
#endif
    {
      /* Pointer to device status port 1 in RxRam */
      prCSMD_Instance->rPriv.apusS_DEV[usI][CSMD_PORT_1][CSMD_RX_BUFFER_0] =
          (CSMD_USHORT *)(CSMD_VOID *)(prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[CSMD_RX_BUFFER_0][1]
              + usTAdd);
      
      /* Pointer to device status port 2 in RxRam */
      prCSMD_Instance->rPriv.apusS_DEV[usI][CSMD_PORT_2][CSMD_RX_BUFFER_0] =
          (CSMD_USHORT *)(CSMD_VOID *)(prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[CSMD_RX_BUFFER_0][1]
              + usTAdd);
    
    /* Use the same port like the used port for SVC */
    }

  }
  
  
#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    prCSMD_Instance->rPriv.pusRxRam = (CSMD_USHORT *)prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.ausRx_Ram;
  }
  else
#endif  /* #ifdef CSMD_PCI_MASTER */
  {
    prCSMD_Instance->rPriv.pusRxRam = (CSMD_USHORT *)prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->ausRx_Ram;
  }
  
  
  /* Offset in RxRam for SVC in AT port 1 */
  ulOffsetSvcRxP1 = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_SVC];
  /* Offset in RxRam for SVC in AT port 2 */
  ulOffsetSvcRxP2 = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_SVC];
  
  /* ------------------------------------------------------------- */
  /* Assignment of the service container according to the          */
  /* projected slave address list.                                 */
  /* ------------------------------------------------------------- */
  
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    usTAdd = prCSMD_Instance->rPriv.ausTopologyAddresses[usI];
    usSVCOffset = (CSMD_USHORT) (usTAdd * CSMD_SVC_FIELDWIDTH);
    
    prCSMD_Instance->rPriv.arSVCInternalStruct[usI].usSVCRxRamPntrP1 =
      (CSMD_USHORT)(usSVCOffset + ulOffsetSvcRxP1);
    
    prCSMD_Instance->rPriv.arSVCInternalStruct[usI].usSVCRxRamPntrP2 =
      (CSMD_USHORT)(usSVCOffset + ulOffsetSvcRxP2);
    
#if CSMD_MAX_HW_CONTAINER > 0
    if ((CSMD_SHORT)usI < CSMD_MAX_HW_CONTAINER )
    {
      prCSMD_Instance->rPriv.prSVContainer[usI] = 
        &prCSMD_Instance->rCSMD_HAL.prSERC_SVC_Ram->rSC_S3[usI];
      
      prSVC = prCSMD_Instance->rPriv.prSVContainer[usI];
      
      if (TRUE == prCSMD_Instance->rPriv.boHW_SVC_Redundancy)
      {
        CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                             (CSMD_USHORT) (usSVCOffset + (CSMD_USHORT) ulOffsetSvcRxP1) );
      }
      else
      {
        if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
        {
          CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                               (CSMD_USHORT) (usSVCOffset + (CSMD_USHORT) ulOffsetSvcRxP2) );
        }
        else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
        {
          if (prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] == CSMD_PORT_1)
          {
            CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                                 (CSMD_USHORT) (usSVCOffset + (CSMD_USHORT) ulOffsetSvcRxP1) );
          }
          else
          {
            CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                                 (CSMD_USHORT) (usSVCOffset+ (CSMD_USHORT) ulOffsetSvcRxP2) );
          }
        }
        else
        {
          CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                               (CSMD_USHORT) (usSVCOffset + (CSMD_USHORT) ulOffsetSvcRxP1) );
        }
      }
    }
    /* enable emulated svc */
    else
#endif  /* #if CSMD_MAX_HW_CONTAINER > 0 */
    {
#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
      /*-------------------------------------------------------------------------- */
      /* Status and control words are pointed to the memory locations of Status    */
      /* and Control words in the RX and TX ram Respectively                       */
      /*-------------------------------------------------------------------------- */
      
      prCSMD_Instance->rPriv.prSVContainer[usI] = 
        &prCSMD_Instance->rPriv.parSoftSvcContainer[usI - CSMD_MAX_HW_CONTAINER];
      
      prSVC = prCSMD_Instance->rPriv.prSVContainer[usI];
      
      if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
      {
        prSVC->usSVCRxPointer_Status = 
          (CSMD_USHORT)(ulOffsetSvcRxP2 + (usTAdd * CSMD_SVC_FIELDWIDTH));
      }
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
      {
        if (prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] == CSMD_PORT_1)
        {
          prSVC->usSVCRxPointer_Status = (CSMD_USHORT)(usSVCOffset + ulOffsetSvcRxP1);
        }
        else
        {
          prSVC->usSVCRxPointer_Status = (CSMD_USHORT)(usSVCOffset + ulOffsetSvcRxP2);
        }
      }
      else
      {
        prSVC->usSVCRxPointer_Status = (CSMD_USHORT)(usSVCOffset + ulOffsetSvcRxP1);
      }
#endif
    }
  }
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Config_RX_Tel_P1() */



/**************************************************************************/ /**
\brief Configures receive telegram for CP2.

\ingroup module_phase
\b Description: \n
   This function configures the receive telegram for CP2, including:
   - Clearance of data in ExRam for all configured telegrams
   - Initialization of service containers

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         14.02.2005
    
 ***************************************************************************** */
CSMD_FUNC_RET CSMD_Config_RX_Tel_P2( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT    usI;             /* Index */
#if CSMD_MAX_HW_CONTAINER > 0
  CSMD_USHORT    usTempRam;
#endif
#ifdef CSMD_PCI_MASTER
  CSMD_USHORT    usTAdd;
  CSMD_USHORT   *pusLocalSVC;     /* Pointer to local ram for received SVC data */
#endif    
  CSMD_SERC3SVC *prSVC;
  
  
  /* ------------------------------------------------------------- */
  /* Clear data (SVC /RTD) in Rx-Ram for all configured telegrams  */
  /* ------------------------------------------------------------- */
  CSMD_Telegram_Clear( prCSMD_Instance, 
                       (CSMD_UCHAR *) &prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram
                                        [prCSMD_Instance->rPriv.ulRxRamTelOffset / 4],    /* Pointer to TxRam */
                       (CSMD_USHORT) (  prCSMD_Instance->rCSMD_HAL.ulRxRamInUse           /* Data length      */
                                      - prCSMD_Instance->rPriv.ulRxRamTelOffset),
                       0U                                                                 /* Fill value       */
                     );
  
  
  prCSMD_Instance->rPriv.aboAT_used[0] = TRUE;   /* 1. Segment (SVC) is used */
  prCSMD_Instance->rPriv.aboAT_used[1] = TRUE;   /* 2. Segment (RTD) is used */
  
  
  /* ------------------------------------------------------------- */
  /* Initialize service container                                  */
  /* ------------------------------------------------------------- */
#ifdef CSMD_PCI_MASTER
  pusLocalSVC = (CSMD_USHORT *)(CSMD_VOID *) (  prCSMD_Instance->prTelBuffer->rLocal.rBuffTxRam.aulTx_Ram
                                              + (  prCSMD_Instance->rPriv.pulSERC3_Tx_MDT_SVC
                                                 - prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aulTx_Ram)
                                             );
#endif
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
#if CSMD_MAX_HW_CONTAINER > 0
    if ((CSMD_SHORT)usI < CSMD_MAX_HW_CONTAINER )
    {
      prSVC = prCSMD_Instance->rPriv.prSVContainer[usI];
      
      /* M_Busy initialization */
      CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[0], (CSMD_USHORT)(CSMD_SVC_CTRL_HANDSHAKE | CSMD_SVC_CTRL_M_BUSY) );
      CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[1], CSMD_SVC_STAT_HANDSHAKE );
      CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[2], 0U );
      CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[3], 0U );
      CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[4], 0U );
      
      
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
                           usTempRam);
    }
    /* enable emulated svc */
    else
#endif  /* #if CSMD_MAX_HW_CONTAINER > 0 */
    {
      prSVC = prCSMD_Instance->rPriv.prSVContainer[usI];
      
      /* M_Busy initialization */
      prSVC->rCONTROL.usWord[0] = (CSMD_USHORT)(CSMD_SVC_CTRL_HANDSHAKE | CSMD_SVC_CTRL_M_BUSY);
      prSVC->rCONTROL.usWord[1] = CSMD_SVC_STAT_HANDSHAKE;
      prSVC->rCONTROL.usWord[2] = 0U;
      prSVC->rCONTROL.usWord[3] = 0U;
      prSVC->rCONTROL.usWord[4] = 0U;
      
#ifdef CSMD_PCI_MASTER
      /* ----------------------------------------------------------- */
      /* set handshake bit in local RAM transmit data.               */
      /* ----------------------------------------------------------- */
      if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
      {
        usTAdd = prCSMD_Instance->rPriv.ausTopologyAddresses[usI];

        
        *(pusLocalSVC + (usTAdd * CSMD_SVC_WORDSIZE)) = CSMD_SVC_CTRL_HANDSHAKE;
      }
#endif
    }
  }
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Config_RX_Tel_P2() */



/**************************************************************************/ /**
\brief Configures receive telegram for CP3.

\ingroup module_phase
\b Description: \n
   This function configures the receive telegram for CP3, including:
   - Clearance of descriptor index table and buffer base pointer list
   - Configuration for monitoring of telegram length by FPGA
   - Configuration of FCS field for Rx-descriptors for MDT
   - Configuration of all ATs (HP field, SVC data, RT data, FCS field)
   - transmission of new descriptor index table and buffer base pointer list
     to RxRam/corresponding FPGA register
   - Initialization of RxRam AT pointers
   - Calculation of ATInternalStruct fields
   - Calculation of RxRam pointers to S-DEVs
   - Assignment of the service container according to the projected slave
     address list.
   - Enabling of telegram sending

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         14.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Config_RX_Tel_P3( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_DESCR_INDEX arRxDesTable[ CSMD_HAL_RX_DESR_INDEX_NUMBER ];
  CSMD_ULONG       aulRxBufBasePtr[ CSMD_HAL_RX_BASE_PTR_NBR ];   /* Rx Buffer base pointer list */
  
  CSMD_ULONG       ulRxRamOffset;      /* Current byte offset in RxRam              */
  CSMD_USHORT      usBuff0_Offset;     /* Current offset [byte] in Rx buffer 0      */
  CSMD_USHORT      usBuffSVC_Offset;   /* Current offset [byte] in Rx buffer SVC    */
  CSMD_USHORT      usTelegramOffset;   /* Offset [byte] in current telegram         */
  CSMD_USHORT      usIdxTableOffs;     /* Rx-Descriptor table index offset in RxRam */
  CSMD_USHORT      usLengthRTD;        /* Length of RT data in MDT/AT Telegrams     */
  CSMD_USHORT      usLengthSVC;        /* Length of SVC     in MDT/AT Telegrams     */
  
  CSMD_UCHAR      *pucTel;             /* Pointer to current Telegram */
  CSMD_UCHAR      *pucTel2;            /* Pointer to current Telegram */
  CSMD_USHORT      usTelI;             /* Telegram Index */
  CSMD_USHORT      usI;                /* Index */
  CSMD_ULONG       ulOffsetRxP1;       /* Offset for RxRam port 1 */
  CSMD_ULONG       ulOffsetRxP2;       /* Offset for RxRam port 2 */
  CSMD_USHORT      usTxRamPortOffset;  /* Offset to first CC connection in Port relative Tx Buffer */
  CSMD_SERC3SVC   *prSVC;
  CSMD_USHORT      usSVCOffset;        /* SVC Offset of current slave related to begin of SVC block */
  CSMD_USHORT      usBuf;              /* Buffer index */
  CSMD_USHORT      usConnIdx;          /* connection index */
  CSMD_USHORT      usConfigIdx;        /* configuration index */
  CSMD_USHORT      usRxBufferLen[CSMD_MAX_TEL]; /* Offset in Rx buffer x */
  
  
#ifdef CSMD_PCI_MASTER
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    CSMD_HAL_ResetTXDMA( &prCSMD_Instance->rCSMD_HAL );
  }
#endif

  ulRxRamOffset  = 0U;    /* Byte offset in RxRam     */
  
  /* Rx-Descriptor index table offset in RxRam */
  usIdxTableOffs = (CSMD_USHORT) ulRxRamOffset;
  CSMD_HAL_SetDesIdxTableOffsRxRam( &prCSMD_Instance->rCSMD_HAL, 
                                    usIdxTableOffs );
  
  /* ------------------------------------------------------------- */
  /* Clear Rx descriptor index table                               */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_RX_DESR_INDEX_NUMBER; usI++)
  {
    arRxDesTable[usI].ulIndex = 0;
  }
  
  ulRxRamOffset += sizeof (arRxDesTable);     /* Reserve space for 4 MDT and 4 AT index table entries */
  
  usBuff0_Offset   = 0U;  /* Byte offset in Rx Buffer 0    */
  usBuffSVC_Offset = 0U;  /* Byte offset in Rx Buffer SVC  */
  
  
  /* ---------------------------------------------------------------- */
  /* Configure received MDT0 for monitoring telegram length by FPGA.  */
  /* ---------------------------------------------------------------- */
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
  {
    if (prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[usTelI])
    {
      /* Descriptor Index table entry for MDT */
      arRxDesTable[usTelI + CSMD_HAL_RX_MDT_DESR_IDX_OFFSET].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
      arRxDesTable[usTelI + CSMD_HAL_RX_MDT_DESR_IDX_OFFSET].ulIndex |= (CSMD_ULONG) ((ulRxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
      
      usTelegramOffset = prCSMD_Instance->rPriv.rMDT_Length[usTelI].usTel;
      
      /* ------------------------------------------------------------- */
      /* Configure FCS field Rx-Descriptors for MDT                    */
      /* ------------------------------------------------------------- */
      CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                ulRxRamOffset,                /* Descriptor offset in RxRam   */
                                0U,                           /* Buffer offset (irrelevant)   */
                                0U,                           /* Buffer System Select         */
                                usTelegramOffset,             /* Offset after CRC in telegram */
                                CSMD_RX_DES_TYPE_FCSP );      /* Descriptor type              */
      ulRxRamOffset += 4;
    }
  }
  
  usTxRamPortOffset = prCSMD_Instance->rPriv.usTxRamPortOffset;   /* Byte offset in Port relative Tx Buffer */
  
  /* ------------------------------------------------------------- */
  /* Configuration of all AT telegrams                             */
  /* ------------------------------------------------------------- */
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
  {
    usRxBufferLen[usTelI] = usBuff0_Offset; /* Offset in Rx buffer x */
    
    if (prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[usTelI])
    {
      /* Descriptor Index table entry for MDT */
      arRxDesTable[usTelI + CSMD_HAL_RX_AT_DESR_IDX_OFFSET].ulIndex |= CSMD_DESCR_INDEX_ENABLE;
      arRxDesTable[usTelI + CSMD_HAL_RX_AT_DESR_IDX_OFFSET].ulIndex |= (CSMD_ULONG) ((ulRxRamOffset/4) << CSMD_DESCR_INDEX_OFFSET_SHIFT);
      
      usTelegramOffset = 0U;  /* Offset [byte] in current telegram */
      
      if (usTelI == 0U)
      {
        /* ------------------------------------------------------------- */
        /* Configure Descriptor for Hot-Plug field in AT0                */
        /* ------------------------------------------------------------- */
        CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulRxRamOffset,                /* Descriptor offset in RxRam   */
                                  usBuff0_Offset,               /* Buffer offset in byte        */
                                  0U,                           /* Buffer System Select         */
                                  usTelegramOffset,             /* Offset after CRC in telegram */
                                  CSMD_RX_DES_TYPE_RTDSP );     /* Descriptor type              */
        ulRxRamOffset += 4;
        
        
        usBuff0_Offset   += prCSMD_Instance->rPriv.rAT_Length[usTelI].usHP;
        usTelegramOffset += prCSMD_Instance->rPriv.rAT_Length[usTelI].usHP;
        CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulRxRamOffset,                        /* Descriptor offset in RxRam   */
                                  (CSMD_USHORT) (usBuff0_Offset - 2),   /* Buffer offset in byte        */
                                  0U,                                   /* Buffer System Select         */
                                  (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                                  CSMD_RX_DES_TYPE_RTDEP );             /* Descriptor type              */
        ulRxRamOffset += 4;
      }
      else
      {
        /*  */
        usTelegramOffset += prCSMD_Instance->rPriv.rAT_Length[usTelI].usHP;
      }
      
      
      usLengthSVC = prCSMD_Instance->rPriv.rAT_Length[usTelI].usSVC;
      /* SVC field in telegram ? */
      if (usLengthSVC)
      {
        /* ------------------------------------------------------------- */
        /* Configure SVC Rx-Descriptors for AT                           */
        /* ------------------------------------------------------------- */
        CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulRxRamOffset,                /* Descriptor offset in RxRam   */
                                  usBuffSVC_Offset,             /* Buffer offset in byte        */
                                  0U,                           /* Buffer System Select         */
                                  usTelegramOffset,             /* Offset after CRC in telegram */
                                  CSMD_RX_DES_TYPE_SVDSP );     /* Descriptor type              */
        ulRxRamOffset += 4;
        
        
        usBuffSVC_Offset += usLengthSVC;
        usTelegramOffset += usLengthSVC;
        CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulRxRamOffset,                        /* Descriptor offset in RxRam   */
                                  (CSMD_USHORT)(usBuffSVC_Offset - 2),  /* Buffer offset in byte        */
                                  0U,                                   /* Buffer System Select         */
                                  (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                                  CSMD_RX_DES_TYPE_SVDEP );             /* Descriptor type              */
        ulRxRamOffset += 4;
      }
      
      /* CC data in this telegram? */
      if (prCSMD_Instance->rPriv.rAT_Length[usTelI].usCC)
      {
        for (usI = 0; usI < prCSMD_Instance->rPriv.rCC_Connections.usNbrCC_Connections; usI++)
        {
          /* -------------------------- */
          /* Consumed by the master?    */
          /* -------------------------- */
          if (prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList[usI].usMasterConsumeCC == TRUE)
          {
            if (prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList[usI].usTelNbr == usTelI)
            {
              usConnIdx        = prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList[usI].usConnIdx;
              usLengthRTD      = prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5;
              usTelegramOffset = prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList[usI].usDataOffset;
              
              /* ------------------------------------------------------------- */
              /* Configure CC connection Rx-Descriptor for AT                  */
              /* ------------------------------------------------------------- */
              CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                        ulRxRamOffset,                /* Descriptor offset in RxRam   */
                                        usTxRamPortOffset,            /* Buffer offset in byte        */
                                        0U,                           /* Buffer System Select         */
                                        usTelegramOffset,             /* Offset after CRC in telegram */
                                        CSMD_RX_DES_TYPE_PRCCDFDSP ); /* Descriptor type              */
              ulRxRamOffset += 4;
              
              
              usBuff0_Offset    += usLengthRTD; /* CC data is copied into RxRam! */
              usTxRamPortOffset += usLengthRTD;
              usTelegramOffset  += usLengthRTD;
              CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                        ulRxRamOffset,                          /* Descriptor offset in RxRam   */
                                        (CSMD_USHORT) (usTxRamPortOffset - 2),  /* Buffer offset in byte        */
                                        0U,                                     /* Buffer System Select         */
                                        (CSMD_USHORT) (usTelegramOffset - 2),   /* Offset after CRC in telegram */
                                        CSMD_RX_DES_TYPE_PRCCDFDEP );           /* Descriptor type              */
              ulRxRamOffset += 4;
            }
          }
        }

        for (usI = 0; usI < prCSMD_Instance->rPriv.rCC_Connections.usNbrCC_Connections; usI++)
        {
          /* --------------------------- */
          /* Not consumed by the master? */
          /* --------------------------- */
          if (prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList[usI].usMasterConsumeCC == FALSE)
          {
            if (prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList[usI].usTelNbr == usTelI)
            {
              usConnIdx        = prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList[usI].usConnIdx;
              usLengthRTD      = prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5;
              usTelegramOffset = prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList[usI].usDataOffset;
              
              /* ------------------------------------------------------------- */
              /* Configure CC connection Rx-Descriptor for AT                  */
              /* ------------------------------------------------------------- */
              CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                        ulRxRamOffset,                /* Descriptor offset in RxRam   */
                                        usTxRamPortOffset,            /* Buffer offset in byte        */
                                        0U,                           /* Buffer System Select         */
                                        usTelegramOffset,             /* Offset after CRC in telegram */
                                        CSMD_RX_DES_TYPE_PRDFDSP );   /* Descriptor type              */
              ulRxRamOffset += 4;
              
              
              usTxRamPortOffset += usLengthRTD;
              usTelegramOffset  += usLengthRTD;
              CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                        ulRxRamOffset,                          /* Descriptor offset in RxRam   */
                                        (CSMD_USHORT) (usTxRamPortOffset - 2),  /* Offset after CRC in telegram */
                                        0U,                                     /* Buffer System Select         */
                                        (CSMD_USHORT) (usTelegramOffset - 2),   /* Offset after CRC in telegram */
                                        CSMD_RX_DES_TYPE_PRDFDEP );             /* Descriptor type              */
              ulRxRamOffset += 4;
            }
          }
        }
      }
      
      usLengthRTD = prCSMD_Instance->rPriv.rAT_Length[usTelI].usRTD;
      /* RTD field in telegram ? */
      if (usLengthRTD)
      {
        /* ------------------------------------------------------------- */
        /* Configure RT data Rx-Descriptors for AT                       */
        /* ------------------------------------------------------------- */
        CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulRxRamOffset,                /* Descriptor offset in RxRam   */
                                  usBuff0_Offset,               /* Buffer offset in byte        */
                                  0U,                           /* Buffer System Select         */
                                  usTelegramOffset,             /* Offset after CRC in telegram */
                                  CSMD_RX_DES_TYPE_RTDSP );     /* Descriptor type              */
        ulRxRamOffset += 4;
        
        
        usBuff0_Offset   += usLengthRTD;
        usTelegramOffset += usLengthRTD;
        CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                  ulRxRamOffset,                        /* Descriptor offset in RxRam   */
                                  (CSMD_USHORT)(usBuff0_Offset - 2),    /* Buffer offset in byte        */
                                  0U,                                   /* Buffer System Select         */
                                  (CSMD_USHORT) (usTelegramOffset - 2), /* Offset after CRC in telegram */
                                  CSMD_RX_DES_TYPE_RTDEP );             /* Descriptor type              */
        ulRxRamOffset += 4;
      }
      
      /* ------------------------------------------------------------- */
      /* Configure FCS field Rx-Descriptors for AT                     */
      /* ------------------------------------------------------------- */
      CSMD_HAL_SetRxDescriptor( &prCSMD_Instance->rCSMD_HAL,
                                ulRxRamOffset,                /* Descriptor offset in RxRam   */
                                0U,                           /* Buffer offset (irrelevant)   */
                                0U,                           /* Buffer System Select         */
                                usTelegramOffset,             /* Offset after CRC in telegram */
                                CSMD_RX_DES_TYPE_FCSP );      /* Descriptor type              */
      ulRxRamOffset += 4;
      
    }   /* End: if (prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[usTelI]) */
    
  }   /* End: for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ ) */
  
  
  /* Round up to the next divisible by 16 */
  ulRxRamOffset = ((ulRxRamOffset + 15) / 16) * 16;
  prCSMD_Instance->rPriv.ulRxRamTelOffset = ulRxRamOffset;
  
  /* ------------------------------------------------------------- */
  /* Initialize Rx Buffer base pointer list                        */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_RX_BASE_PTR_NBR; usI++)
  {
    aulRxBufBasePtr[usI] = ulRxRamOffset;
  }
  
  /* Buffer Basepointer: SVC port 1 */
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_SVC] = ulRxRamOffset;
  /* Round buffer offset up to divisible by 4 for proper calculation of Rx buffer base pointer */
  usBuffSVC_Offset = (CSMD_USHORT) (((usBuffSVC_Offset + 3) / 4) * 4);
  ulRxRamOffset += usBuffSVC_Offset;
  /* Buffer Basepointer: SVC port 2 */
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_SVC] = ulRxRamOffset;
  ulRxRamOffset += usBuffSVC_Offset;
#ifdef CSMD_DEBUG
  prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P1_BUFF_SVC]     = usBuffSVC_Offset;
  prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P2_BUFF_SVC]     = usBuffSVC_Offset;
#endif
  
  /* Buffer Basepointer: System A Buffer 0 port 1 */
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A] = ulRxRamOffset;
  /* Round buffer offset up to divisible by 4 for proper calculation of Rx buffer base pointer */
  usBuff0_Offset = (CSMD_USHORT) (((usBuff0_Offset + 3) / 4) * 4);
  ulRxRamOffset += usBuff0_Offset;
  /* Buffer Basepointer: System A Buffer 0 port 2 */
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A] = ulRxRamOffset;
  ulRxRamOffset += usBuff0_Offset;
#ifdef CSMD_DEBUG
  prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A] = usBuff0_Offset;
  prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A] = usBuff0_Offset;
#endif
  
  if (prCSMD_Instance->rPriv.rHW_Init_Struct.usRxBufferMode >= CSMD_RX_DOUBLE_BUFFER)
  {
    /* Buffer Basepointer: System A Buffer 1 port 1 */
    aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_1_SYS_A] = ulRxRamOffset;
    ulRxRamOffset += usBuff0_Offset;
    /* Buffer Basepointer: System A Buffer 1 port 2 */
    aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_1_SYS_A] = ulRxRamOffset;
    ulRxRamOffset += usBuff0_Offset;
#ifdef CSMD_DEBUG
    prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P1_BUFF_1_SYS_A] = usBuff0_Offset;
    prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P2_BUFF_1_SYS_A] = usBuff0_Offset;
#endif
  }
  
  if (prCSMD_Instance->rPriv.rHW_Init_Struct.usRxBufferMode == CSMD_RX_TRIPLE_BUFFER)
  {
    /* Buffer Basepointer: System A Buffer 2 port 1 */
    aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_2_SYS_A] = ulRxRamOffset;
    ulRxRamOffset += usBuff0_Offset;
    /* Buffer Basepointer: System A Buffer 2 port 2 */
    aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_2_SYS_A] = ulRxRamOffset;
    ulRxRamOffset += usBuff0_Offset;
#ifdef CSMD_DEBUG
    prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P1_BUFF_2_SYS_A] = usBuff0_Offset;
    prCSMD_Instance->rCSMD_Debug.aulRxBufSize[CSMD_HAL_IDX_RX_P2_BUFF_2_SYS_A] = usBuff0_Offset;
#endif
  }
  
  /* Buffer Basepointer: port 1 relative Buffer (TxRam !)  */
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_PORT_WR_TX] = 
    prCSMD_Instance->rPriv.ulTxRamOffsetPRel_P2;
  
  /* Buffer Basepointer: port 2 relative Buffer (TxRam !)  */
  aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_PORT_WR_TX] = 
    prCSMD_Instance->rPriv.ulTxRamOffsetPRel_P1;
  
  
  /* Used FPGA RxRam memory for Sercos telegrams */
  prCSMD_Instance->rCSMD_HAL.ulRxRamInUse = (((ulRxRamOffset - 1) / CSMD_IP_RAM_SEG_SIZE) + 1) * CSMD_IP_RAM_SEG_SIZE;
  
  if (prCSMD_Instance->rCSMD_HAL.ulRxRamInUse > CSMD_HAL_RX_RAM_SIZE)
  {
    return (CSMD_INSUFFICIENT_RX_RAM);
  }
  
  
  /* ------------------------------------------------------------- */
  /* Transmit Descriptor index table into FPGA RxRam               */
  /* ------------------------------------------------------------- */
  usIdxTableOffs >>= 2;   /* Offset [Longs] in RxRam */ /*lint !e845 The left argument to operator '>>' is certain to be 0  */
  
  for (usI = 0; usI < CSMD_HAL_RX_DESR_INDEX_NUMBER; usI++)
  {
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aulRx_Ram[usIdxTableOffs],
                        (arRxDesTable[usI].ulIndex) );
    usIdxTableOffs++;
  }
  
  /* ------------------------------------------------------------- */
  /* Transmit Rx Buffer base pointer list into FPGA register       */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < CSMD_HAL_RX_BASE_PTR_NBR; usI++)
  {
    CSMD_HAL_WriteLong( &prCSMD_Instance->rCSMD_HAL.prSERC_Reg->aulRxBufBasePtr[usI], 
                        aulRxBufBasePtr[usI] );
#ifdef CSMD_DEBUG
    prCSMD_Instance->rCSMD_Debug.aulRxBufBasePtr[usI] = aulRxBufBasePtr[usI];
#endif
  }
  
  
  /* ------------------------------------------------------------- */
  /* Initialize RxRam AT pointers                                  */
  /* ------------------------------------------------------------- */
  
  ulOffsetRxP1 = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A];
  ulOffsetRxP2 = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A];
  
  usBuf = CSMD_RX_BUFFER_0;
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
  {
    if (prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel)
    {
      prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usTelI] = 
        (CSMD_ULONG *)(CSMD_VOID *) (prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram + ulOffsetRxP1 + usRxBufferLen[usTelI]);
      
      prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usTelI] = 
        (CSMD_ULONG *)(CSMD_VOID *) (prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram + ulOffsetRxP2 + usRxBufferLen[usTelI]);
    }
  }
  
  usBuf = CSMD_RX_BUFFER_1;
  if (usBuf < CSMD_MAX_RX_BUFFER)  /*lint !e774 Boolean within 'if' always evaluates to True */
  {
    ulOffsetRxP1 = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_1_SYS_A];
    ulOffsetRxP2 = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_1_SYS_A];
    
    for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
    {
      if (prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel)
      {
        prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usTelI] = 
          (CSMD_ULONG *)(CSMD_VOID *) (prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram + ulOffsetRxP1 + usRxBufferLen[usTelI]);
        
        prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usTelI] = 
          (CSMD_ULONG *)(CSMD_VOID *) (prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram + ulOffsetRxP2 + usRxBufferLen[usTelI]);
      }
    }
  }
  
  usBuf = CSMD_RX_BUFFER_2;
  if (usBuf < CSMD_MAX_RX_BUFFER)  /*lint !e774 Boolean within 'if' always evaluates to True */
  {
    ulOffsetRxP1 = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_2_SYS_A];
    ulOffsetRxP2 = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_2_SYS_A];
    
    for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
    {
      if (prCSMD_Instance->rPriv.rAT_Length[usTelI].usTel)
      {
        prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usTelI] = 
          (CSMD_ULONG *)(CSMD_VOID *) (prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram + ulOffsetRxP1 + usRxBufferLen[usTelI]);
        
        prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usTelI] = 
          (CSMD_ULONG *)(CSMD_VOID *) (prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram + ulOffsetRxP2 + usRxBufferLen[usTelI]);
      }
    }
  }
  
  
  /* ------------------------------------------------------------- */
  /* Calculate AT offsets                                          */
  /* ------------------------------------------------------------- */
  
  for (usI = 0; usI < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; usI++)
  {
    usConnIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI].usConnIdx;

    /* AT connection */
    if (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usTelegramType ==
          CSMD_TELEGRAM_TYPE_AT )
    {
      usConfigIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI].usConfigIdx;
      usTelI    = (CSMD_USHORT)((prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 &
                     CSMD_S_0_1050_SE3_TEL_NBR_MASK) >> CSMD_S_0_1050_SE3_TEL_NBR_SHIFT);
      usTelegramOffset = (CSMD_USHORT)(prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 &
                           CSMD_S_0_1050_SE3_TEL_OFFSET);

      /* Offset in RTD field of telegram */
      ulRxRamOffset = (CSMD_ULONG)(usTelegramOffset - prCSMD_Instance->rPriv.rAT_Length[usTelI].usSVC);
      
      /* is connection consumed by master? */
      if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1
           & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
      {
        ulRxRamOffset = (ulRxRamOffset + prCSMD_Instance->rPriv.rAT_Length[usTelI].usCC_M) - prCSMD_Instance->rPriv.rAT_Length[usTelI].usCC;
      }

      if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
      {
        /* HP field in all ATs according to Sercos specification < 1.3.0 */
        if (usTelI != 0)
        {
          ulRxRamOffset -= prCSMD_Instance->rPriv.rAT_Length[usTelI].usHP;
        }
      }

      usBuf = CSMD_RX_BUFFER_0;
#ifdef CSMD_PCI_MASTER
      if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
      {
        /* Pointer to RTD field of telegram port 1 in Mirror Ram */
        pucTel = (CSMD_UCHAR *)
          (  prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aucRx_Ram
           + (  (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usTelI]
              - prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram)
           + ulRxRamOffset);
        
        /* Pointer to RTD field of telegram port 2 in Mirror Ram */
        pucTel2 = (CSMD_UCHAR *)
          (  prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aucRx_Ram
           + (  (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usTelI]
              - prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram)
           + ulRxRamOffset);
      }
      else
#endif
      {
        /* Pointer to RTD field of telegram port 1 in FPGA */
        pucTel  = (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usTelI] + ulRxRamOffset;
        /* Pointer to RTD field of telegram port 2 in FPGA */
        pucTel2 = (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usTelI] + ulRxRamOffset;
      }

      prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].apusConnRxRam[CSMD_PORT_1][usBuf]
        = (CSMD_USHORT *) (CSMD_VOID *) pucTel;
      prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].apusConnRxRam[CSMD_PORT_2][usBuf]
        = (CSMD_USHORT *) (CSMD_VOID *) pucTel2;

      usBuf = CSMD_RX_BUFFER_1;
      if (usBuf < CSMD_MAX_RX_BUFFER)  /*lint !e774 Boolean within 'if' always evaluates to True */
      {
#ifdef CSMD_PCI_MASTER
        if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
        {
          /* Pointer to RTD field of telegram port 1 in Mirror Ram */
          pucTel = (CSMD_UCHAR *)
            (  prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aucRx_Ram
             + (  (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usTelI]
                - prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram)
             + ulRxRamOffset);

          /* Pointer to RTD field of telegram port 2 in Mirror Ram */
          pucTel2 = (CSMD_UCHAR *)
            (  prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aucRx_Ram
             + (  (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usTelI]
                - prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram)
             + ulRxRamOffset);
        }
        else
#endif
        {
          /* Pointer to RTD field of telegram port 1 in FPGA */
          pucTel  = (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usTelI] + ulRxRamOffset;
          /* Pointer to RTD field of telegram port 2 in FPGA */
          pucTel2 = (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usTelI] + ulRxRamOffset;
        }
        prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].apusConnRxRam[CSMD_PORT_1][usBuf]
          = (CSMD_USHORT *) (CSMD_VOID *) pucTel;
        prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].apusConnRxRam[CSMD_PORT_2][usBuf]
          = (CSMD_USHORT *) (CSMD_VOID *) pucTel2;


        usBuf = CSMD_RX_BUFFER_2;
        if (usBuf < CSMD_MAX_RX_BUFFER)  /*lint !e774 Boolean within 'if' always evaluates to True */
        {
#ifdef CSMD_PCI_MASTER
          if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
          {
            /* Pointer to RTD field of telegram port 1 in Mirror Ram */
            pucTel = (CSMD_UCHAR *)
              (  prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aucRx_Ram
               + (  (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usTelI]
                  - prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram)
               + ulRxRamOffset);

            /* Pointer to RTD field of telegram port 2 in Mirror Ram */
            pucTel2 = (CSMD_UCHAR *)
              (   prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aucRx_Ram
               + (  (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usTelI]
                  - prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram)
               + ulRxRamOffset);
          }
          else
#endif
          {
            /* Pointer to RTD field of telegram port 1 in FPGA */
            pucTel  = (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usTelI] + ulRxRamOffset;
            /* Pointer to RTD field of telegram port 2 in FPGA */
            pucTel2 = (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usTelI] + ulRxRamOffset;
          }
          prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].apusConnRxRam[CSMD_PORT_1][usBuf]
            = (CSMD_USHORT *) (CSMD_VOID *) pucTel;
          prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].apusConnRxRam[CSMD_PORT_2][usBuf]
            = (CSMD_USHORT *) (CSMD_VOID *) pucTel2;
        }
      } /* if (usBuf < CSMD_MAX_RX_BUFFER) */
    } /* if (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usTelegramType ==
          CSMD_TELEGRAM_TYPE_AT ) */
  } /* for (usI = 0; usI < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; usI++) */
  
  /* ------------------------------------------------------------- */
  /* Calculate RxRam pointer to S-DEV                              */
  /* ------------------------------------------------------------- */
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    usTelI        = (CSMD_USHORT)((prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTelegramConfig.usS_DEV_OffsetAT_S01011 >> 12) & 0x0003);
    
    ulRxRamOffset =
        (  (CSMD_ULONG)(prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTelegramConfig.usS_DEV_OffsetAT_S01011 & 0x0FFF)
         + prCSMD_Instance->rPriv.rAT_Length[usTelI].usCC_M)
      - (  prCSMD_Instance->rPriv.rAT_Length[usTelI].usSVC
         + prCSMD_Instance->rPriv.rAT_Length[usTelI].usCC);
    if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
    {
      /* HP field in all ATs according to Sercos specification < 1.3.0 */
      if(usTelI != 0)
      {
        ulRxRamOffset -= prCSMD_Instance->rPriv.rAT_Length[usTelI].usHP;
      }
    }
    
    for (usBuf = CSMD_RX_BUFFER_0; usBuf < CSMD_MAX_RX_BUFFER; usBuf++)
    {
#ifdef CSMD_PCI_MASTER
      if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
      {
        /* Pointer to device status in RxRam port 1 */
        prCSMD_Instance->rPriv.apusS_DEV[usI][CSMD_PORT_1][usBuf] = (CSMD_USHORT *)(CSMD_VOID *)
          (  prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aucRx_Ram
           + (  (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usTelI]
              - prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram)
           + ulRxRamOffset);

        /* Pointer to device status in RxRam port 2 */
        prCSMD_Instance->rPriv.apusS_DEV[usI][CSMD_PORT_2][usBuf] = (CSMD_USHORT *)(CSMD_VOID *)
          (  prCSMD_Instance->prTelBuffer->rLocal.rBuffRxRam.aucRx_Ram
           + (  (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usTelI]
              - prCSMD_Instance->rCSMD_HAL.prSERC_RX_Ram->aucRx_Ram)
           + ulRxRamOffset);
      }
      else
#endif
      {
        /* Pointer to device status in RxRam port 1 buffer 0 */
        prCSMD_Instance->rPriv.apusS_DEV[usI][CSMD_PORT_1][usBuf] =
          (CSMD_USHORT *)(CSMD_VOID *)( (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBuf][usTelI]
                                       + ulRxRamOffset);
        /* Pointer to device status in RxRam port 2 buffer 0 */
        prCSMD_Instance->rPriv.apusS_DEV[usI][CSMD_PORT_2][usBuf] =
          (CSMD_USHORT *)(CSMD_VOID *)( (CSMD_UCHAR *)prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBuf][usTelI]
                                       + ulRxRamOffset);
      }
    }
    
  }
  
  
  /* ------------------------------------------------------------- */
  /* Assignment of the service container according to the          */
  /* projected slave address list.                                 */
  /* ------------------------------------------------------------- */
  ulOffsetRxP1 = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_SVC];
  ulOffsetRxP2 = aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_SVC];
  
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    usSVCOffset = (CSMD_USHORT) (usI * CSMD_SVC_FIELDWIDTH);
    
    prCSMD_Instance->rPriv.arSVCInternalStruct[usI].usSVCRxRamPntrP1 =
      (CSMD_USHORT)(usSVCOffset + ulOffsetRxP1);
    
    prCSMD_Instance->rPriv.arSVCInternalStruct[usI].usSVCRxRamPntrP2 =
      (CSMD_USHORT)(usSVCOffset + ulOffsetRxP2);
    
    if ((CSMD_SHORT)usI < CSMD_MAX_HW_CONTAINER )
    {
      prSVC = prCSMD_Instance->rPriv.prSVContainer[usI];
      
      if (TRUE == prCSMD_Instance->rPriv.boHW_SVC_Redundancy)
      {
        CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                             (CSMD_USHORT)(usSVCOffset + ulOffsetRxP1) );
      }
      else
      {
        if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
        {
          CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                               (CSMD_USHORT)(usSVCOffset + ulOffsetRxP2) );
        }
        else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
        {
          if (prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] == CSMD_PORT_1)

          {
            CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                                 (CSMD_USHORT)(usSVCOffset + ulOffsetRxP1) );
          }
          else
          {
            CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                                 (CSMD_USHORT)(usSVCOffset + ulOffsetRxP2) );
          }
        }
        else
        {
          CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                               (CSMD_USHORT)(usSVCOffset + ulOffsetRxP1) );
        }
      }
    }
    
    /* enable emulated svc */
    else    /* if (usI < CSMD_MAX_HW_CONTAINER ) */
    {
      
      /*-------------------------------------------------------------------------- */
      /* Status and control words are pointed to the memory locations of Status    */
      /* and Control words in the RX and TX ram respectively                       */
      /*-------------------------------------------------------------------------- */
      prSVC = prCSMD_Instance->rPriv.prSVContainer[usI];
      
      if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
      {
        prSVC->usSVCRxPointer_Status = 
          (CSMD_USHORT)(usSVCOffset + ulOffsetRxP2);
      }
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
      {
        if (prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] == CSMD_PORT_1)

        {
          prSVC->usSVCRxPointer_Status =
            (CSMD_USHORT)(usSVCOffset + ulOffsetRxP1);
        }
        else
        {
          prSVC->usSVCRxPointer_Status = 
            (CSMD_USHORT)(usSVCOffset + ulOffsetRxP2);
        }
      }
      else
      {
        prSVC->usSVCRxPointer_Status = 
          (CSMD_USHORT)(usSVCOffset + ulOffsetRxP1);
      }
    }   /* End: if (usI < CSMD_MAX_HW_CONTAINER ) */
  }   /* End: for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++) */
  
  
  for (usTelI = 0; usTelI < prCSMD_Instance->rPriv.usNbrTel_CP12; usTelI++)
  {
    /* Delete only in CP0 to CP2 used variables */
    prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[usTelI] = NULL;
    prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[usTelI] = NULL;
    prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usTelI] = 0U;
  }
  
  
  /* ------------------------------------------------------------- */
  /* Enable telegram sending */
  /* ------------------------------------------------------------- */
  prCSMD_Instance->rPriv.usAT_Enable = 0x0000;
  for (usTelI = 0; usTelI < CSMD_MAX_TEL; usTelI++ )
  {
    if (prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[usTelI])
    {
      prCSMD_Instance->rPriv.aboAT_used[usTelI] = TRUE;           /* Telegram is used */
      prCSMD_Instance->rPriv.usAT_Enable |= (CSMD_USHORT)(1UL << usTelI);
    }
  }
  
  /* Perform buffer switch in every Sercos cycle. */
  CSMD_HAL_Config_Rx_Buffer_Valid( &prCSMD_Instance->rCSMD_HAL,
                                   CSMD_HAL_RX_BUFFER_SYSTEM_A, 
                                   0U, /* Received MDTs extraneous */
                                   0U );
  
#ifdef CSMD_PCI_MASTER
  /* ------------------------------------------------------------- */
  /* Configure the PCI TX DMA channels                             */
  /* ------------------------------------------------------------- */
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    /* 
    */
    usBuf = CSMD_RX_BUFFER_0;
    
    CSMD_Config_DMA_Tx_Channel( 
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulRxRamBase
        + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_SVC],
      (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram
        + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_SVC],
      (CSMD_ULONG) usBuffSVC_Offset,
      (CSMD_USHORT) CSMD_DMA_CHANNEL_00 );

    prCSMD_Instance->rPriv.ausTxDMA_Start_P1[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_00);

    CSMD_Config_DMA_Tx_Channel( 
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulRxRamBase
        + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_SVC],
      (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram
        + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_SVC],
      (CSMD_ULONG) usBuffSVC_Offset,
      (CSMD_USHORT) CSMD_DMA_CHANNEL_01 );

    prCSMD_Instance->rPriv.ausTxDMA_Start_P2[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_01);

    CSMD_Config_DMA_Tx_Channel( 
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulRxRamBase
        + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A],
      (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram
        + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A],
      (CSMD_ULONG) usBuff0_Offset,
      (CSMD_USHORT) CSMD_DMA_CHANNEL_02 );

    prCSMD_Instance->rPriv.ausTxDMA_Start_P1[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_02);

    CSMD_Config_DMA_Tx_Channel( 
      prCSMD_Instance,
      prCSMD_Instance->rCSMD_HAL.ulRxRamBase
        + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A],
      (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram
                + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A],
      (CSMD_ULONG) usBuff0_Offset,
      (CSMD_USHORT) CSMD_DMA_CHANNEL_03 );

    prCSMD_Instance->rPriv.ausTxDMA_Start_P2[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_03);

    usBuf = CSMD_RX_BUFFER_1;
    if (usBuf < CSMD_MAX_RX_BUFFER)  /*lint !e774 Boolean within 'if' always evaluates to True */
    {
      prCSMD_Instance->rPriv.ausTxDMA_Start_P1[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_00);
      prCSMD_Instance->rPriv.ausTxDMA_Start_P2[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_01);

      CSMD_Config_DMA_Tx_Channel( 
        prCSMD_Instance,
        prCSMD_Instance->rCSMD_HAL.ulRxRamBase
          + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_1_SYS_A],
        (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram
          + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_1_SYS_A],
        (CSMD_ULONG) usBuff0_Offset,
        (CSMD_USHORT) CSMD_DMA_CHANNEL_04 );
      
      prCSMD_Instance->rPriv.ausTxDMA_Start_P1[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_04);
      
      CSMD_Config_DMA_Tx_Channel( 
        prCSMD_Instance,
        prCSMD_Instance->rCSMD_HAL.ulRxRamBase
          + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_1_SYS_A],
        (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram
          + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_1_SYS_A],
        (CSMD_ULONG) usBuff0_Offset,
        (CSMD_USHORT) CSMD_DMA_CHANNEL_05 );
      
      prCSMD_Instance->rPriv.ausTxDMA_Start_P2[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_05);
    }
    
    usBuf = CSMD_RX_BUFFER_2;
    if (usBuf < CSMD_MAX_RX_BUFFER)  /*lint !e774 Boolean within 'if' always evaluates to True */
    {
      prCSMD_Instance->rPriv.ausTxDMA_Start_P1[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_00);
      prCSMD_Instance->rPriv.ausTxDMA_Start_P2[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_01);

      CSMD_Config_DMA_Tx_Channel( 
        prCSMD_Instance,
        prCSMD_Instance->rCSMD_HAL.ulRxRamBase
          + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_2_SYS_A],
        (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram
          + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_2_SYS_A],
        (CSMD_ULONG) usBuff0_Offset,
        (CSMD_USHORT) CSMD_DMA_CHANNEL_06 );
      
      prCSMD_Instance->rPriv.ausTxDMA_Start_P1[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_06);
      
      CSMD_Config_DMA_Tx_Channel( 
        prCSMD_Instance,
        prCSMD_Instance->rCSMD_HAL.ulRxRamBase
          + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_2_SYS_A],
        (CSMD_ULONG) prCSMD_Instance->prTelBuffer_Phys->rLocal.rBuffRxRam.aulRx_Ram
          + aulRxBufBasePtr[CSMD_HAL_IDX_RX_P2_BUFF_2_SYS_A],
        (CSMD_ULONG) usBuff0_Offset,
        (CSMD_USHORT) CSMD_DMA_CHANNEL_07 );
      
      prCSMD_Instance->rPriv.ausTxDMA_Start_P2[usBuf] |= (CSMD_USHORT)(1 << (CSMD_USHORT)CSMD_DMA_CHANNEL_07);
    }
  }
#endif
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Config_RX_Tel_P3() */



#ifdef CSMD_PCI_MASTER
/**************************************************************************/ /**
\brief Configures one PCI TX DMA channel.

\ingroup module_dma
\b Description: \n
   This function configures one PCI Tx DMA channel setting
   - DMA local address
   - DMA PCI address
   - DMA counter
   - DMA ready address

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   ulSrcAdd_S3Ram 
              Source address offset in FPGA Rx RAM (long aligned)
\param [in]   ulDstAdd_Host 
              Destination address in host RAM (long aligned)
\param [in]   ulLength 
              Number of bytes to be transferred (shall be divisible by 4)
\param [in]   usTxChannelNbr 
              PCI TX DMA channel number (0...15)

\return       none

\author       WK
\date         09.02.2011

***************************************************************************** */
CSMD_VOID CSMD_Config_DMA_Tx_Channel( const CSMD_INSTANCE *prCSMD_Instance,
                                      CSMD_ULONG           ulSrcAdd_S3Ram,
                                      CSMD_ULONG           ulDstAdd_Host,
                                      CSMD_ULONG           ulLength,
                                      CSMD_USHORT          usTxChannelNbr )
{
  
  CSMD_HAL_SetDMALocalAddr( &prCSMD_Instance->rCSMD_HAL,
                            CSMD_TX_DMA,
                            usTxChannelNbr,
                            ulSrcAdd_S3Ram );
  
  CSMD_HAL_SetDMAPCIAddr(   &prCSMD_Instance->rCSMD_HAL,
                            CSMD_TX_DMA,
                            usTxChannelNbr,
                            ulDstAdd_Host );
  
  CSMD_HAL_SetDMACounter(   &prCSMD_Instance->rCSMD_HAL,
                            CSMD_TX_DMA,
                            usTxChannelNbr,
                            ulLength );
  
  CSMD_HAL_SetDMARDYAddr(   &prCSMD_Instance->rCSMD_HAL,
                            CSMD_TX_DMA,
                            usTxChannelNbr,
                            (CSMD_ULONG)&prCSMD_Instance->prTelBuffer_Phys->rLocal.rDMA_TxRdy.aulFlag[usTxChannelNbr] );
  
}  /* end: CSMD_Config_DMA_Tx_Channel */
#endif

/*! \endcond */ /* PRIVATE */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
03 Dec 2013 WK
  - Defdb00150926
    Fixed Lint message 740 "Unusual pointer cast (incompatible indirect types)"
    for CSMD_MAX_HW_CONTAINER > 0.
13 Mar 2014 WK
  - Defdb00168199
    CSMD_Config_RX_Tel_P3()
    Malfunction of cyclic communication, if a CC connection assigned to AT1-AT3
    is consumed by the master. In this case, the pointers for all S-DEV
    and connections assigned to AT1- AT3 will be miscalculated.
17 Jun 2014 AlM
  - CSMD_Config_RX_Tel_P1(), CSMD_Config_RX_Tel_P3()
    Perform buffer switch in every Sercos cycle.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
28 Apr 2015 WK
  - Defdb00178597
    CSMD_Config_RX_Tel_P3()
    Fixed type castings for 64-bit systems.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
22 Jul 2015
  - CSMD_Config_RX_Tel_P1()
    Fixed ausLast_S_Dev_Idx[0] for ring topology with no slaves
    causing a crash in CSMD_CyclicDeviceStatus().
01 Oct 2015 WK
  - Defdb00182067
    Fixed Lint messages
    737 "Loss of sign in promotion from int to unsigned int".
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
12 Nov 2015 WK
  - Defdb00182992
    CSMD_Config_RX_Tel_P3()
    Pointer in the RxRam were incorrectly calculated for S-DEVs or Connections 
    that are configured in AT > 0.
28 Jan 2016 WK
  - Defdb00182067
    CSMD_Config_RX_Tel_P2()
    Fixed lint warning respectively precision lost compiler warning.

------------------------------------------------------------------------------
*/
