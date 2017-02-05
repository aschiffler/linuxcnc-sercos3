/*
 * Sercos Soft Master Core Library
 * Version: see SICE_GLOB.h
 * Copyright (C) 2012 - 2016 Bosch Rexroth AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS"; WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY;
 * FITNESS FOR A PERTICULAR PURPOSE AND NONINFRINGEMENT. THE AUTHORS OR COPYRIGHT
 * HOLDERS SHALL NOT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE;
 * UNLESS STIPULATED BY MANDATORY LAW.
 *
 * You may contact us at open.source@boschrexroth.de if you are interested in
 * contributing a modification to the Software.
 */

/**
 * \file      SICE_TX.c
 *
 * \brief     Sercos SoftMaster core: Packet transmission
 *
 * \ingroup   SICE
 *
 * \author    GMy, partially based on earlier work by SBe
 *
 * \date      2012-10-11
 *
 * \copyright Copyright Bosch Rexroth AG, 2012-2016
 *
 * \version 2012-10-11 (GMy): Baseline for Sercos master IP core v3
 * \version 2012-10-31 (GMy): Updated for Sercos master IP core v4
 * \version 2013-01-11 (GMy): Added support for Windows CE
 * \version 2013-01-15 (GMy): Added watchdog support
 * \version 2013-01-31 (GMy): Added Sercos timing method support
 * \version 2013-02-05 (GMy): Hot-plug support added
 * \version 2013-02-06 (GMy): Support for second buffer system added
 * \version 2013-02-28 (GMy): Lint code optimization
 * \version 2013-04-24 (GMy): Code optimization for sending and receiving
 * \version 2013-05-07 (GMy): Added support for INtime
 * \version 2013-06-06 (GMy): Added support for RTX and Kithara
 * \version 2013-06-20 (GMy): Optimization of error handling
 * \version 2013-06-21 (GMy): Added support for Windows desktop versions and
 *                            QNX
 * \version 2013-06-25 (GMy): Finalized Sercos time support, new function
 *                            SICE_GenExtField()
 * \version 2013-08-22 (GMy): Added preliminary support for NIC-timed transmission
 * \version 2014-04-01 (GMy): Added support for CoSeMa 6VRS
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 * \version 2015-01-28 (GMy): Code optimization
 * \version 2015-06-03 (GMy): Lint code optimization
 * \version 2015-07-14 (GMy): Defdb00180480: Code optimization. Moved
 *                            SendTelegramsNicTimed() to separate file.
 * \version 2015-11-03 (GMy): Defdb00180480: Code optimization
 */

//---- includes ---------------------------------------------------------------

#include "../SICE/SICE_GLOB.h"
#include "../SICE/SICE_PRIV.h"

#include "../CSMD/CSMD_HAL_PRIV.h"
#include "../CSMD/CSMD_BM_CFG.h"

#ifdef __qnx__
/*lint -save -w0 */
#include <arpa/inet.h>
/*lint -restore */
#elif defined __unix__
/*lint -save -w0 */
#include <arpa/inet.h>
/*lint -restore */
#elif defined WINCE7
/*lint -save -w0 */
#include <Winsock.h>
/*lint -restore */
#elif defined WINCE
/*lint -save -w0 */
#include <Winsock.h>
/*lint -restore */
#elif defined __INTIME__
#elif defined __RTX__
#elif defined __VXWORKS__
#elif defined __KITHARA__
#elif defined WIN32
#elif defined WIN64
#else
#error Operating system not supported by SICE!
#endif

//---- defines ----------------------------------------------------------------

// Macros for decoding Sercos master ip core tx descriptor type

/**
 * \def     SICE_TX_DESC_SVC_DATA(_DescType)
 *
 * \brief   Macro for checking whether tx descriptor type indicates SVC data
 */
#define SICE_TX_DESC_SVC_DATA(_DescType)    (   (((UCHAR)_DescType) == ((UCHAR)0x00)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x01)))

/**
 * \def     SICE_TX_DESC_RT_DATA(_DescType)
 *
 * \brief   Macro for checking whether tx descriptor type indicates RT data
 */
#define SICE_TX_DESC_RT_DATA(_DescType)     (   (((UCHAR)_DescType) == ((UCHAR)0x02)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x03)))

/**
 * \def SICE_TX_DESC_PORT_DATA(_DescType)
 *
 * \brief   Macro for checking whether tx descriptor type indicates
 *          port-specific data
 */
#define SICE_TX_DESC_PORT_DATA(_DescType)   (   (((UCHAR)_DescType) == ((UCHAR)0x08)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x09)))

/**
 * \def     SICE_TX_DESC_PORTCC_DATA(_DescType)
 *
 * \brief   Macro for checking whether tx descriptor type indicates
 *          port-specific cross communication data
 */
#define SICE_TX_DESC_PORTCC_DATA(_DescType) (   (((UCHAR)_DescType) == ((UCHAR)0x0A)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x0B)))

/**
 * \def     SICE_TX_DESC_START_DESC(_DescType)
 *
 * \brief   Macro for checking whether tx descriptor type indicates the first
 *          descriptor of descriptor pair
 */
#define SICE_TX_DESC_START_DESC(_DescType)  (   (((UCHAR)_DescType) == ((UCHAR)0x00)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x02)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x08)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x0A)))

/**
 * \def     SICE_TX_DESC_FRAME_END(_DescType)
 *
 * \brief   Macro for checking whether tx descriptor type indicates the end of
 *          the descriptor list
 */
#define SICE_TX_DESC_FRAME_END(_DescType)   (((UCHAR)_DescType) == ((UCHAR)0x04))

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SICE_FUNC_RET SICE_PrepareTelegrams(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   Prepares Sercos telegrams for transmission
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:            No error
 *          - SICE_CONFIG_ERROR:        When illegal packet numbers are used
 *          - SICE_TX_DESCRIPTOR_ERROR: When problems with TX descriptors
 *                                      occur, such as the signaling of an
 *                                      unsupported buffering mechanism
 *          - SICE_PARAMETER_ERROR:     Function parameter error
 *
 * \details The tx descriptors are interpreted. Depending on the descriptors,
 *          the telegrams to be sent are built from the different buffers with
 *          the corresponding offsets. The telegram data is stored in the array
 *          aprSendFrame of the SICE instance to be send out later.
 *
 * \author  GMy, partially based on earlier work by SBe
 *
 * \ingroup SICE
 *
 * \date    2012-10-11
 *
 * \version 2012-10-11 (GMy): Baseline for Sercos master IP core v3
 * \version 2012-10-31 (GMy): Updated for emulation of IP core v4
 * \version 2013-01-15 (GMy): Added watchdog support
 * \version 2013-01-31 (GMy): Added Sercos timing method support (not complete
 *                            yet)
 * \version 2013-02-05 (GMy): Hot-plug support added
 * \version 2013-02-06 (GMy): Support for second buffer system added
 * \version 2013-04-24 (GMy): Separated packet preparation from packet
 *                            transmission
 * \version 2013-06-25 (GMy): Finalized Sercos time support
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 */
SICE_FUNC_RET SICE_PrepareTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  UCHAR               ucPhaseAll;
  SICE_SIII_FRAME*    puSercosFrameP1         = NULL;
  SICE_SIII_FRAME*    puSercosFrameP2         = NULL;
  USHORT              usPacketIdx;
  USHORT              usIndexTableOffset;
  ULONG               ulTXDescTableOffset     = 0;
  BOOL                boTXDescTableEnabled    = FALSE;
  USHORT              usCurDescOffset         = 0;
  ULONG               ulCurDesc               = 0;
  USHORT              usFrameOffset           = 0;
  UCHAR               ucDescType              = 0;
  USHORT              usBufOffset             = 0;
  UCHAR               ucBufSel                = 0;
  USHORT              usLastFrameOffset       = 0;
  USHORT              usLastBufOffset         = 0;
  SICE_FUNC_RET       eSiceRet                = SICE_NO_ERROR;

  SICE_VERBOSE(3, "SICE_PrepareTelegrams()\n");

  if  (prSiceInstance ==  NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  ucPhaseAll = (UCHAR)(prSiceInstance->prReg->ulPHASECR & ((ULONG)SICE_PHASECR_ALL_MASK));

  // TX index table offset
  usIndexTableOffset = prSiceInstance->prReg->rDECR.rDesIdx.usOffsetTxRam;

  // For all Sercos telegrams MDT0, MDT1, ... , AT3 ...
  for (
      usPacketIdx = 0;
      usPacketIdx < 2*CSMD_MAX_TEL;
      usPacketIdx++
    )
  {
    // Check SFCR register for packet enable bit
    if  (
        SICE_CheckPacketSfcrTxEnabled
        (
          prSiceInstance,
          usPacketIdx
        )
      )
    {
      SICE_VERBOSE
          (
            3,
            "Packet type to transmit: %hu\n",
            usPacketIdx
          );

      // Use packet index as an ULONG offset for finding corresponding
      // TX descriptors
      /*lint -save -e826 */
      ulTXDescTableOffset =
          *((ULONG*)  (
                  (UCHAR *)prSiceInstance->prTX_Ram +
                  ((ULONG) usIndexTableOffset) +
                  ((ULONG) usPacketIdx) * ((ULONG) sizeof(ULONG)))
                );
      /*lint -restore */

      boTXDescTableEnabled = ulTXDescTableOffset & ((ULONG) SICE_TEL_DESC_ENABLE_MASK);
      ulTXDescTableOffset  = ulTXDescTableOffset & ((ULONG) SICE_TEL_DESC_OFFSET_MASK);

      //Is this packet type enabled in descriptor?
      if (boTXDescTableEnabled)
      {
        usCurDescOffset   = (USHORT) ulTXDescTableOffset;

        prSiceInstance->aprSendFrame[usPacketIdx]->boEnable = TRUE;

        // Pre-fill packet buffer (except for header) with zeros
        (VOID)memset
            (
              prSiceInstance->aprSendFrame[usPacketIdx]->aucData + SICE_SERC3_TEL_HEADER,
              (UCHAR)0x00,
              SICE_ETH_FRAMEBUF_LEN - SICE_SERC3_TEL_HEADER
            );

        // Pointer to already allocated frame
        puSercosFrameP1 = (SICE_SIII_FRAME *) prSiceInstance->aprSendFrame[usPacketIdx]->aucData;

        // Set Sercos type field in packet header
        puSercosFrameP1->rTel.ucSercosType = SICE_SercosTypeField
            (
              usPacketIdx,
              SICE_TEL_P_CHANNEL
            );

        // Set communication phase field in packet header
        puSercosFrameP1->rTel.ucPhase = ucPhaseAll;

        // Sercos time (and therefore extended field) activated?
#if (defined CSMD_SERCOS_TIME) || (CSMD_DRV_VERSION > 5)
        // Generate extended field in MDT0
        eSiceRet = SICE_GenExtField
            (
              prSiceInstance,       // SICE instance
              (USHORT*)&(puSercosFrameP1->rTel.aucData[SICE_TEL_EXT_FIELD_OFFSET]),
                                    // Offset of extended field in packet
              ucPhaseAll,
              usPacketIdx
            );

        if (eSiceRet != SICE_NO_ERROR)
        {
          return(SICE_SYSTEM_ERROR);
        }
#endif
        // The same for the other port - in case redundancy is enabled
        if (SICE_REDUNDANCY_BOOL)
        {
          prSiceInstance->aprSendFrame[usPacketIdx + 2*CSMD_MAX_TEL]->boEnable = TRUE;

          (VOID)memset
              (
                prSiceInstance->aprSendFrame[usPacketIdx + 2*CSMD_MAX_TEL]->aucData +
                    SICE_SERC3_TEL_HEADER,
                (UCHAR)0x00,
                SICE_ETH_FRAMEBUF_LEN - SICE_SERC3_TEL_HEADER
              );

          // Pointer to already allocated frame
          puSercosFrameP2 = (SICE_SIII_FRAME *)
              prSiceInstance->aprSendFrame[usPacketIdx + 2*CSMD_MAX_TEL]->aucData;

          // Set Sercos type field in packet header
          puSercosFrameP2->rTel.ucSercosType = SICE_SercosTypeField
              (
                usPacketIdx,
                SICE_TEL_S_CHANNEL
              );

          // Set communication phase field in packet header
          puSercosFrameP2->rTel.ucPhase = ucPhaseAll;

          // Sercos time (and therefore extended field) activated?
#if (defined CSMD_SERCOS_TIME) || (CSMD_DRV_VERSION > 5)
          // Generate extended field in MDT0
          eSiceRet = SICE_GenExtField
              (
                prSiceInstance,       // SICE instance
                (USHORT*)&(puSercosFrameP2->rTel.aucData[SICE_TEL_EXT_FIELD_OFFSET]),
                              // Offset of extended field in packet
                ucPhaseAll,
                usPacketIdx
              );

          if (eSiceRet != SICE_NO_ERROR)
          {
            return(SICE_SYSTEM_ERROR);
          }
#endif

        }

        // Browse through all TX descriptors for this packet
        do
        {
          // Retrieve TX descriptor
          /*lint -save -e826 */
          ulCurDesc =
              *(  (ULONG*)  (
                        (UCHAR *)prSiceInstance->prTX_Ram +
                        ((ULONG) usCurDescOffset)
                      )
              );
          /*lint -restore */

          // Move descriptor pointer
          usCurDescOffset += (USHORT) sizeof(ULONG);

          // Decode descriptor
          usFrameOffset = (USHORT) ((ulCurDesc & ((ULONG) CSMD_HAL_DES_MASK_TEL_OFFS))
                            >> ((ULONG) CSMD_HAL_DES_SHIFT_TEL_OFFS));
          ucDescType    = (UCHAR) ((ulCurDesc & ((ULONG) CSMD_HAL_DES_MASK_TYPE))
                            >> ((ULONG) CSMD_HAL_DES_SHIFT_TYPE));
          usBufOffset   = (USHORT) (ulCurDesc  & ((ULONG) CSMD_HAL_DES_MASK_BUFF_OFFS));
          ucBufSel      = (UCHAR) ((ulCurDesc & ((ULONG) CSMD_HAL_DES_MASK_BUFF_SYS_SEL))
                            >> ((ULONG) CSMD_HAL_DES_SHIFT_BUFF_SYS_SEL));

          // Is it first descriptor of descriptor pair?
          if (SICE_TX_DESC_START_DESC(ucDescType))
          {
            // Store start offsets for frame and buffer
            usLastFrameOffset = usFrameOffset;
            usLastBufOffset   = usBufOffset;
          }
          // Second descriptor of descriptor pair
          else if (!SICE_TX_DESC_FRAME_END(ucDescType))
          {
            // Copy data to frame buffer(s)
            eSiceRet = SICE_CopyTxData
                (
                  prSiceInstance,     // Pointer to SICE instance
                  puSercosFrameP1,    // Frame buffer primary port
                  puSercosFrameP2,    // Frame buffer secondary port
                  ucDescType,         // descriptor type
                  ucBufSel,           // Buffer selector
                  usLastBufOffset,    // Buffer offset of last descriptor
                  usLastFrameOffset,  // Frame offset of last descriptor
                  usFrameOffset       // Frame offset of current descriptor
                );
            if (eSiceRet != SICE_NO_ERROR)
            {
              return(eSiceRet);
            }
          }

          if (SICE_TX_DESC_FRAME_END(ucDescType))
          {
            // Calculate total frame length
            prSiceInstance->aprSendFrame[usPacketIdx]->usLen =
                usFrameOffset + ((USHORT) SICE_SERC3_TEL_HEADER);

            if (SICE_REDUNDANCY_BOOL)
            {
              // Calculate total frame length
              prSiceInstance->aprSendFrame[usPacketIdx + 2* CSMD_MAX_TEL]->usLen =
                  usFrameOffset + ((USHORT) SICE_SERC3_TEL_HEADER);
            }

#ifdef CSMD_HW_WATCHDOG
            // In case of watchdog alarm and set packets to zero
            // mode, do it.
            if (prSiceInstance->ucWDAlarm == (UCHAR) SICE_WD_ALARM_SEND_EMPTY_TEL)
            {
              // Replace telegram content by zeros
              (VOID)memset
                  (
                    prSiceInstance->aprSendFrame[usPacketIdx]->aucData +
                        SICE_SERC3_TEL_HEADER,
                    (UCHAR) 0x00,
                    prSiceInstance->aprSendFrame[usPacketIdx]->usLen -
                        SICE_SERC3_TEL_HEADER
                  );
            }
#endif

            // Calculate CRC of dynamic part of header and use
            // pre-calculated base CRC
            // \todo Check if it works with big endian. htonl for conversion?
            puSercosFrameP1->rTel.ulCRC = SICE_CRC32Calc
                (
                  &puSercosFrameP1->aucRaw[ SICE_TEL_LENGTH_HDR_FOR_CRC - SICE_TEL_LENGTH_DYN_HDR_FOR_CRC],
                  SICE_TEL_LENGTH_DYN_HDR_FOR_CRC,
                  prSiceInstance->ulBaseCRC
                );

            if (SICE_REDUNDANCY_BOOL)
            {
              // Calculate CRC of dynamic part of header and use
              // pre-calculated base CRC
              // \todo Check if it works with big endian. htonl for conversion?
              puSercosFrameP2->rTel.ulCRC = SICE_CRC32Calc
                  (
                    &puSercosFrameP2->aucRaw[SICE_TEL_LENGTH_HDR_FOR_CRC - SICE_TEL_LENGTH_DYN_HDR_FOR_CRC],
                    SICE_TEL_LENGTH_DYN_HDR_FOR_CRC,
                    prSiceInstance->ulBaseCRC
                  );
            }

          } // if boIsLastFrameDesc
        } while (!SICE_TX_DESC_FRAME_END(ucDescType));
        // while more descriptors for this packet
      } // if packet enabled in descriptor
      else
      {
        SICE_VERBOSE
            (
              0,
              "Error: Sercos packet #%hu enabled in SFCR "
              "register, but not in TX descriptor.\n",
              usPacketIdx
            );

        prSiceInstance->aprSendFrame[usPacketIdx]->boEnable = FALSE;
        return(SICE_TX_DESCRIPTOR_ERROR);
      }
    } // if packet enabled in SFCR
    else
    {
      prSiceInstance->aprSendFrame[usPacketIdx]->boEnable = FALSE;
      if (SICE_REDUNDANCY_BOOL)
      {
        prSiceInstance->aprSendFrame[usPacketIdx + 2* CSMD_MAX_TEL]->boEnable = FALSE;
      }
    }
  } // for all packets MDT0 .. AT3

  return(SICE_NO_ERROR);
}

 /**
 * \fn BOOL SICE_CheckPacketSfcrTxEnabled(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              USHORT usPacketIdx
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 * \param[in]       usPacketIdx     Packet type CSMD_DES_IDX_*
 *
 * \brief   Checks whether a packet type is enabled for transmission in the
 *          Sercos III frame control register
 *
 * \return
 *      - TRUE, in case the packet is enabled in SFCR
 *      - FALSE, otherwise
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2015-01-28
 */
 BOOL SICE_CheckPacketSfcrTxEnabled
  (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      USHORT usPacketIdx
  )
 {

  ULONG ulSFCREnableMask  = 0;
  BOOL boSFCREnable     = FALSE;

  SICE_VERBOSE(3, "SICE_CheckPacketSfcrTxEnabled()\n");

  if  (prSiceInstance ==  NULL)
  {
    return(boSFCREnable);
  }

  // Find SFCR bit mask for packet index
  /*lint -save -e527 const! */
  switch (usPacketIdx)
  {
    case CSMD_DES_IDX_MDT0:
      ulSFCREnableMask = (ULONG) CSMD_HAL_SFCR_ENABLE_MDT0;
      break;
    case CSMD_DES_IDX_MDT1:
      ulSFCREnableMask = (ULONG) CSMD_HAL_SFCR_ENABLE_MDT1;
      break;
    case CSMD_DES_IDX_MDT2:
      ulSFCREnableMask = (ULONG) CSMD_HAL_SFCR_ENABLE_MDT2;
      break;
    case CSMD_DES_IDX_MDT3:
      ulSFCREnableMask = (ULONG) CSMD_HAL_SFCR_ENABLE_MDT3;
      break;
    case CSMD_DES_IDX_AT0:
      ulSFCREnableMask = (ULONG) CSMD_HAL_SFCR_ENABLE_AT0;
      break;
    case CSMD_DES_IDX_AT1:
      ulSFCREnableMask = (ULONG) CSMD_HAL_SFCR_ENABLE_AT1;
      break;
    case CSMD_DES_IDX_AT2:
      ulSFCREnableMask = (ULONG) CSMD_HAL_SFCR_ENABLE_AT2;
      break;
    case CSMD_DES_IDX_AT3:
      ulSFCREnableMask = (ULONG) CSMD_HAL_SFCR_ENABLE_AT3;
      break;
    default:
      return(FALSE);
      break;
  }
  /*lint -restore const! */

  boSFCREnable = (
            prSiceInstance->prReg->ulSFCR     &
            (((ULONG) 1) << ulSFCREnableMask)
          )   != ((ULONG) 0);

  return(boSFCREnable);
}

 /**
 * \fn UCHAR SICE_SercosTypeField(
 *              USHORT usPacketIdx,
 *              UCHAR ucChannel
 *          )
 *
 * \private
 *
 * \param[in]       usPacketIdx     Packet type CSMD_DES_IDX_*
 * \param[in]       ucChannel       SICE_TEL_P_CHANNEL or SICE_TEL_S_CHANNEL
 *
 * \brief   Returns the encoded Sercos type field of the Sercos telegram header
 *
 * \return  Encoded Sercos type field
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2015-01-28
 */
UCHAR SICE_SercosTypeField
    (
      USHORT usPacketIdx,
      UCHAR ucChannel
    )
{
  UCHAR ucSercosType;

  SICE_VERBOSE(3, "SICE_SercosTypeField()\n");

  ucSercosType =
      ((UCHAR) SICE_TEL_CYCLE_CNT)                              |   // Cycle counter
      (ucChannel)                                               |   // Channel
      ((UCHAR) (usPacketIdx & ((USHORT) SICE_TEL_NO_MASK)))     |   // Packet index
      ((UCHAR)(
            (
              (usPacketIdx & ((USHORT) SICE_TEL_DESC_IDX_AT))
              >> ((USHORT) SICE_TEL_DESC_IDX_SHIFT)
            )
            << ((USHORT) SICE_TEL_TYPE_SHIFT)
          )                                                         // MDT / AT flag
      );

  return(ucSercosType);
}

/**
 * \fn SICE_FUNC_RET SICE_CopyTxData(
 *          SICE_INSTANCE_STRUCT*   prSiceInstance,
 *          SICE_SIII_FRAME*        puSercosFrameP1,
 *          SICE_SIII_FRAME*        puSercosFrameP2,
 *          UCHAR                   ucDescType,
 *          UCHAR                   ucBufSel,
 *          USHORT                  usLastBufOffset,
 *          USHORT                  usLastFrameOffset,
 *          USHORT                  usFrameOffset
 *  )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance      Pointer to Sercos SoftMaster core
 *                                      instance
 * \param[in,out]   puSercosFrameP1     Pointer to frame buffer for primary
 *                                      port
 * \param[in,out]   puSercosFrameP2     Pointer to frame buffer for secondary
 *                                      port
 * \param[in]       ucDescType          Descriptor type
 * \param[in]       ucBufSel            Buffer type
 * \param[in]       usLastBufOffset     Offset in buffer buffer of last
 *                                      descriptor
 * \param[in]       usLastFrameOffset   Offset in frame buffer of last
 *                                      descriptor
 * \param[in]       usFrameOffset       Offset in buffer of current descriptor
 *
 * \brief   Copies the data to be transmitted from the TX/RX RAM to the
 *          packet buffer.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:            No error
 *          - SICE_SOCKET_ERROR:        When a problem occurred at packet
 *                                      transmission
 *          - SICE_PARAMETER_ERROR:     For function parameter error
 *          - SICE_TX_DESCRIPTOR_ERROR: When problems with TX descriptors
 *                                      occur, such as the signaling of an
 *                                      unsupported buffering mechanism
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2015-01-28
 */
SICE_FUNC_RET SICE_CopyTxData
    (
      SICE_INSTANCE_STRUCT*   prSiceInstance,
      SICE_SIII_FRAME*        puSercosFrameP1,
      SICE_SIII_FRAME*        puSercosFrameP2,
      UCHAR                   ucDescType,
      UCHAR                   ucBufSel,
      USHORT                  usLastBufOffset,
      USHORT                  usLastFrameOffset,
      USHORT                  usFrameOffset
    )
{
  USHORT  usBufSysOffset = 0;
  USHORT  usLen;

  usLen = (usFrameOffset - usLastFrameOffset) + ((USHORT)sizeof(USHORT));
                                                      //USHORT alignment

  // Real-time data?
  if (SICE_TX_DESC_RT_DATA(ucDescType))
  {
    switch (ucBufSel)
    {
      case 0:
        usBufSysOffset = ((USHORT) CSMD_HAL_IDX_TX_BUFF_0_SYS_A);
        break;

      case 1:
        usBufSysOffset = ((USHORT) CSMD_HAL_IDX_TX_BUFF_0_SYS_B);
        break;

      default:
        SICE_VERBOSE
            (
              0,
              "Error in TX descriptor: Selected buffer "
              "system %c is not supported.",
              ucBufSel
            );
        return(SICE_TX_DESCRIPTOR_ERROR);
        /*lint -save -e527 */
        break;
        /*lint -restore */
    }
    (VOID)memcpy
        (
          &puSercosFrameP1->rTel.aucData[usLastFrameOffset],
          (UCHAR *)prSiceInstance->prTX_Ram +
            prSiceInstance->prReg->aulTxBufBasePtr[usBufSysOffset] +
            usLastBufOffset,
          usLen
        );

    if (SICE_REDUNDANCY_BOOL)
    {
      (VOID)memcpy
          (
            &puSercosFrameP2->rTel.aucData[usLastFrameOffset],
            (UCHAR *)prSiceInstance->prTX_Ram +
              prSiceInstance->prReg->aulTxBufBasePtr[usBufSysOffset] +
              usLastBufOffset,
            usLen
          );
    }
  }
  // Service channel data?
  else if (SICE_TX_DESC_SVC_DATA(ucDescType))
  {
    (VOID)memcpy
        (
          &puSercosFrameP1->rTel.aucData[usLastFrameOffset],
          (UCHAR *)prSiceInstance->prTX_Ram +
            prSiceInstance->prReg->aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_SVC] +
            usLastBufOffset,
          usLen
        );

    if (SICE_REDUNDANCY_BOOL)
    {
      (VOID)memcpy
          (
            &puSercosFrameP2->rTel.aucData[usLastFrameOffset],
            (UCHAR *)prSiceInstance->prTX_Ram +
              prSiceInstance->prReg->aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_SVC] +
              usLastBufOffset,
            usLen
          );
    }
  }
  // Port-specific data?
  else if (SICE_TX_DESC_PORT_DATA(ucDescType))
  {
    (VOID)memcpy
        (
          &puSercosFrameP1->rTel.aucData[usLastFrameOffset],
          (UCHAR *)prSiceInstance->prTX_Ram +
            prSiceInstance->prReg->aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_1] +
            usLastBufOffset,
          usLen
        );

    if (SICE_REDUNDANCY_BOOL)
    {
      (VOID)memcpy
          (
            &puSercosFrameP2->rTel.aucData[usLastFrameOffset],
            (UCHAR *)prSiceInstance->prTX_Ram +
              prSiceInstance->prReg->aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_2] +
              usLastBufOffset,
            usLen
          );
    }
  }
  // CC data?
  else if (SICE_TX_DESC_PORTCC_DATA(ucDescType))
  {
    // \todo fill with zeros after AT error
    // \todo set datafield delay bit (only needed for double line)
    //    Something like:
    //    prSiceInstance->prReg->aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_1] +
    //      usLastBufOffset |= 1<<2;
    (VOID)memcpy
        (
          &puSercosFrameP1->rTel.aucData[usLastFrameOffset],
          (UCHAR *)prSiceInstance->prTX_Ram +
            prSiceInstance->prReg->aulTxBufBasePtr[CSMD_HAL_IDX_TX_BUFF_PORT_1] +
            usLastBufOffset,
          usLen
        );
  }
  else
  {
    SICE_VERBOSE(0, "Error: Unknown TX descriptor\n");
    return(SICE_TX_DESCRIPTOR_ERROR);
  }

  return(SICE_NO_ERROR);
}

/**
 * \fn SICE_FUNC_RET SICE_SendMDTTelegrams(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   Transmits Sercos MDT telegrams
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:            No error
 *          - SICE_SOCKET_ERROR:        When a problem occurred at packet
 *                                      transmission
 *          - SICE_PARAMETER_ERROR:     For function parameter error
 *
 * \note    When not using NIC-timed packet transmission, the exact timing of
 *          the packets within the Sercos cycle is not based on event structure
 *          provided by CoSeMa. The reason is that this is very difficult to
 *          achieve with some real-time operating systems that do not allow
 *          such a small granularity and accuracy of timing events. However,
 *          typically that does not cause problems with Sercos slave devices.
 *
 * \author  GMy, partially based on earlier work by SBe
 *
 * \ingroup SICE
 *
 * \date    2012-10-11
 *
 * \version 2012-10-11 (GMy): Baseline for Sercos master IP core v3
 * \version 2012-10-31 (GMy): Updated for emulation of IP core v4
 * \version 2013-04-24 (GMy): Separated packet preparation from packet
 *                            transmission and separated MDT and AT transmission
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 */
SICE_FUNC_RET SICE_SendMDTTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  INT           iRet        = 0;
  USHORT        usPacketIdx;
  USHORT        usIFG       = CSMD_HAL_TXIFG_BASE;
  USHORT        usReqIFG;
  INT           iPort       = 0;

  SICE_VERBOSE(3, "SICE_SendMDTTelegrams()\n");

  if  (prSiceInstance ==  NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  // Take over inter frame gap from CoSeMa if larger than minimum value
  usReqIFG = (USHORT) prSiceInstance->prReg->ulIFG & ((ULONG) SICE_IFG_REG_MASK);
  if (usReqIFG > (USHORT) CSMD_HAL_TXIFG_BASE)
  {
    usIFG = usReqIFG;
  }

  // For all Sercos MDT telegrams MDT0 .. MDT3
  for (
      usPacketIdx = 0;
      usPacketIdx < CSMD_MAX_TEL;
      usPacketIdx++
    )
  {
    // For all ports - depending on redundancy setting
    for (
        iPort = 0;
        iPort < SICE_REDUNDANCY_VAL;
        iPort++
      )
    {
      if (prSiceInstance->aprSendFrame[usPacketIdx + iPort * 2 * CSMD_MAX_TEL]->boEnable)
      {
        SICE_VERBOSE
            (
              2,
              "Sending MDT%d telegram on port %d ...\n",
              usPacketIdx,
              iPort
            );

        // Send packet
        iRet = RTOS_TxPacket
            (
              prSiceInstance->iInstanceNo,            // SICE instance
              iPort,                                  // Port index
              prSiceInstance->aprSendFrame[usPacketIdx + iPort*2*CSMD_MAX_TEL]->aucData,
                                                      // data pointer
              prSiceInstance->aprSendFrame[usPacketIdx + iPort*2*CSMD_MAX_TEL]->usLen,
                                                      // packet length
              usIFG                                   // inter-frame gap
            );

        if (iRet < 0)
        {
          SICE_VERBOSE
              (
                0,
                "Error: Send MDT%d telegram on port %d failed.\n",
                usPacketIdx,
                iPort
              );
          return(SICE_SOCKET_ERROR);
        }
        else
        {
          // Increase counter register for successfully
          // transmitted packets
          (VOID)SICE_IncPacketCounter
              (
                prSiceInstance,     // SICE instance
                SICE_TX_PACKET_CNT, // TX
                TRUE,               // Packet is OK
                iPort,              // Port number
                1                   // Single packet
              );

          SICE_VERBOSE(2, "Packet transmission OK\n");
        }
      } // if packet enabled
    } // for all ports
  } // for all Sercos MDT telegrams MDT0 .. MDT3

  return(SICE_NO_ERROR);
}

/**
 * \fn SICE_FUNC_RET SICE_SendATTelegrams(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   Transmit Sercos AT telegrams
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:        No error
 *          - SICE_SOCKET_ERROR:    When a problem occurred at packet
 *                                  transmission
 *          - SICE_PARAMETER_ERROR: For function parameter error
 *
 * \note    When not using NIC-timed packet transmission, the exact timing of
 *          the packets within the Sercos cycle is not based on event structure
 *          provided by CoSeMa. The reason is that this is very difficult to
 *          achieve with some real-time operating systems that do not allow
 *          such a small granularity and accuracy of timing events. However,
 *          typically that does not cause problems with Sercos slave devices.
 *
 * \author  GMy, partially based on earlier work by SBe
 *
 * \ingroup SICE
 *
 * \date    2012-10-11
 *
 * \version 2012-10-11 (GMy): Baseline for Sercos master IP core v3
 * \version 2012-10-31 (GMy): Updated for emulation of IP core v4
 * \version 2013-04-24 (GMy): Separated packet preparation from packet
 *                            transmission and separated MDT and AT
 *                            transmission
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 */
SICE_FUNC_RET SICE_SendATTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  INT    iRet        = 0;
  USHORT usPacketIdx;
  USHORT usIFG       = CSMD_HAL_TXIFG_BASE;
  USHORT usReqIFG;
  INT    iPort       = 0;

  SICE_VERBOSE(3, "SICE_SendATTelegrams()\n");

  if  (prSiceInstance == NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

#ifndef SICE_MEASURE_TIMING

  // Take over inter frame gap from CoSeMa if larger than minimum value
  usReqIFG = (USHORT) prSiceInstance->prReg->ulIFG & ((ULONG) SICE_IFG_REG_MASK);
  if (usReqIFG > (USHORT) CSMD_HAL_TXIFG_BASE)
  {
    usIFG = usReqIFG;
  }

  // For all Sercos AT telegrams AT0 .. AT3
  for (
      usPacketIdx = CSMD_MAX_TEL;
      usPacketIdx < 2*CSMD_MAX_TEL;
      usPacketIdx++
    )
  {
    // For all ports - depending on redundancy setting
    for (
        iPort =0;
        iPort < SICE_REDUNDANCY_VAL;
        iPort++
      )
    {
      if (prSiceInstance->aprSendFrame[usPacketIdx + iPort*2*CSMD_MAX_TEL]->boEnable)
      {
        SICE_VERBOSE
            (
              2,
              "Sending AT%u telegram on port %i...\n",
              usPacketIdx - CSMD_MAX_TEL,
              iPort
            );

        // Send packet.
        iRet = RTOS_TxPacket
            (
              prSiceInstance->iInstanceNo,                    // SICE instance
              iPort,                                          // Port index
              prSiceInstance->aprSendFrame[usPacketIdx + iPort*2*CSMD_MAX_TEL]->aucData,
                                                              // data pointer
              prSiceInstance->aprSendFrame[usPacketIdx + iPort*2*CSMD_MAX_TEL]->usLen,
                                                              // packet length
              usIFG                                           // inter-frame gap
            );

        if (iRet < 0)
        {
          SICE_VERBOSE
              (
                0,
                "Error: Send AT%u telegram on port %i failed.\n",
                usPacketIdx - CSMD_MAX_TEL,
                iPort
              );
          return(SICE_SOCKET_ERROR);
        }
        else
        {
          // Increase counter register for successfully
          // transmitted packets
          (VOID)SICE_IncPacketCounter
              (
                prSiceInstance,     // SICE instance
                SICE_TX_PACKET_CNT, // TX
                TRUE,               // Packet is OK
                iPort,              // Port number
                1                   // Single packet
              );

          SICE_VERBOSE(2, "Packet transmission OK\n");
        }
      } // if packet enabled
    } // For all ports
  } // For all Sercos telegrams AT0 .. AT3

#endif

  return(SICE_NO_ERROR);
}

/**
 * \fn SICE_FUNC_RET SICE_CheckPreCondsSend(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   This function checks whether the pre-conditions for sending Sercos
 *          packets are met.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:            Pre-conditions are met
 *          - SICE_CHECK_STATUS_FALSE:  Pre-Conditions are not (or only
 *                                      partially) met
 *          - SICE_PARAMETER_ERROR:     For function parameter error
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2013-04-22
 *
 * \version 2013-04-22 (GMy): Moved logic to separate function
 * \version 2013-08-02 (GMy): Bugfix: Problem with Sercos phase bit mask
 */
SICE_FUNC_RET SICE_CheckPreCondsSend
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  UCHAR ucPhase;

  if (prSiceInstance   ==  NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  // Retrieve current Sercos phase
  ucPhase = (UCHAR)(prSiceInstance->prReg->ulPHASECR & ((ULONG) CSMD_HAL_PHASECR_PHASE_MASK));

  /*lint -save -e568 -e685 const! */
  if (
      (ucPhase >= ((UCHAR) CSMD_SERC_PHASE_0))                                &&
                                        // if in CP0 or higher (currently always true)
      (prSiceInstance->ucWDAlarm != (UCHAR) SICE_WD_ALARM_DISABLE_TX_TEL)     &&
                                        // Packet transmission not disabled by watchdog?
      (prSiceInstance->prReg->ulTCSR & (((ULONG)1) << ((ULONG) CSMD_HAL_TCSR_ET0))) &&
                                        // if main system timer running

#if (CSMD_DRV_VERSION <=5)
      (prSiceInstance->prReg->ulDFCSR & (((ULONG)1) << ((ULONG) CSMD_HAL_DFCSR_TX_ENABLE)))
                                                // if packet transmission is enabled
#else
            (prSiceInstance->prReg->rDFCSR.ulLong & (((ULONG)1)
                  << ((ULONG) CSMD_HAL_DFCSR_TX_ENABLE)))
                                                // if packet transmission is enabled
#endif
    )
  /*lint -restore const! */
  {
    return(SICE_NO_ERROR);
  }
  else
  {
    return(SICE_CHECK_STATUS_FALSE);
  }
}

/**
 * \fn SICE_FUNC_RET SICE_GenExtField(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              USHORT *pusExtField,
                UCHAR ucPhaseAll,
                USHORT usPacketIdx
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 * \param[out]      pusExtField     Pointer to extended field to be written
 * \param[in]       ucPhaseAll      Sercos packet phase field
 * \param[in]       usPacketIdx     Packet index
 *
 * \brief   This function generates the extended function field
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:            Success
 *          - SICE_SYSTEM_ERROR:        Internal inconsistency
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2013-06-25
 *
 * \version 2014-05-21 (GMy): Moved check for telegram and phase action to
 *                            function
 */
SICE_FUNC_RET SICE_GenExtField
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      USHORT *pusExtField,
      UCHAR ucPhaseAll,
      USHORT usPacketIdx
    )
{
  INT iSercosTimePhase = 0;

  SICE_VERBOSE(2, "SICE_GenExtField()\n");

  // Only in MDT0 during phases CP3 and CP4
  if  (
      (
        (
          (
            (ucPhaseAll & ((UCHAR) CSMD_HAL_PHASECR_PHASE_MASK))
            == ((UCHAR) CSMD_SERC_PHASE_3)
          )                                                           &&
                                                        // in CP3
          !(ucPhaseAll & SICE_TEL_CP_SWITCH_MASK)
        )                                                               ||
                                                        // but not in transition from CP2
        (
          (ucPhaseAll & ((UCHAR) CSMD_HAL_PHASECR_PHASE_MASK))
          == ((UCHAR) CSMD_SERC_PHASE_4)
        )
      )                                                                   &&
                                                        // or in CP4
      (usPacketIdx == CSMD_DES_IDX_MDT0)          // and only for MDT0
    )
  {

    // SICE has to write pre-calculated time in extended field after
    // hot-plug field

    // Is Sercos time activated in TCSR register?
    if  (
        (prSiceInstance->prReg->ulTCSR & (((ULONG)1) << CSMD_HAL_TCSR_EST))
        == ((ULONG) 0)
      )
    {
      // Sercos time is not activated
      prSiceInstance->boSercosTimeEn  = FALSE;

  #if (CSMD_DRV_VERSION <= 5)
      // First word of extended function field (C-Time)
      pusExtField[0] = (USHORT)
        (
          (prSiceInstance->prReg->rSCCCMDT.rSCCNT.usSccCount & ((USHORT)0x3FFF))    |   // TSRef counter
          (USHORT) (
            prSiceInstance->prReg->ulTCSR                       &
            ((ULONG)1 << CSMD_HAL_TCSR_NEW_ST)
          )                                                           |   // Toggle bit
          (((USHORT) 0) << 14)                                            // Time fragment invalid
        );
  #else
      // First word of extended function field (C-Time)
      pusExtField[0] = (USHORT)
        (
          (
            prSiceInstance->prReg->rSCCMDT.rSCCNT.usScCount &
            ((USHORT)0x3FFF)
          )                                                           |   // TSRef counter
          (USHORT) (
            prSiceInstance->prReg->ulTCSR                       &
            ((ULONG)1 << CSMD_HAL_TCSR_SYS_TIME_TOGGLE)
          )                                                           |   // Toggle bit
          (((USHORT) 0) << 14)                                            // Time fragment invalid
        );
  #endif
      // Second word of extended function field (Time)
      pusExtField[1] = (USHORT) 0;
    }
    else
    {
      // Sercos time is activated

      // Latch Sercos time for transmission at cycle count 0
      if  (prSiceInstance->ucCycleCnt == (UCHAR)0)
      {
        prSiceInstance->ulLatchedTime.ulSeconds = prSiceInstance->prReg->ulSTSECP;
        prSiceInstance->ulLatchedTime.ulNanos   = prSiceInstance->prReg->ulSTNSP;
        prSiceInstance->boSercosTimeEn          = TRUE;
      }

      if  (prSiceInstance->boSercosTimeEn)
      {
        // Only change Sercos time transmission phase every second cycle,
        // so each value is transmitted twice in a row
        iSercosTimePhase = prSiceInstance->ucCycleCnt / 2;

  #if (CSMD_DRV_VERSION <=5)
        // First word of extended function field (C-Time)
        pusExtField[0] = (USHORT)
          (
            (prSiceInstance->prReg->rSCCCMDT.rSCCNT.usSccCount &((USHORT) 0x3FFF))      |   // TSRef counter
            (USHORT)(
              prSiceInstance->prReg->ulTCSR                       &
              ((ULONG)1 << CSMD_HAL_TCSR_NEW_ST)
            )                                                           |   // Toggle bit
            ((USHORT)1 << 14)                                               // Time fragment valid
          );
  #else
        // First word of extended function field (C-Time)
        pusExtField[0] = (USHORT)
          (
            (prSiceInstance->prReg->rSCCMDT.rSCCNT.usScCount &((USHORT) 0x3FFF))      |   // TSRef counter
            (USHORT)(
              prSiceInstance->prReg->ulTCSR                       &
              ((ULONG)1 << CSMD_HAL_TCSR_SYS_TIME_TOGGLE)
            )                                                           |   // Toggle bit
            ((USHORT)1 << 14)                                               // Time fragment valid
          );
  #endif
        // Set second word of extended field (time) multiplexed time value.
        // First high byte of seconds, then low byte, then high byte of
        // nanos, then low byte.
        // \todo OK for big endian?
        /*lint -save -e527 const! */
        switch(iSercosTimePhase)
        {
          case 0:
            pusExtField[1] =
                (USHORT)((prSiceInstance->ulLatchedTime.ulSeconds & (ULONG)0xFFFF0000) >> 16);
            break;
          case 1:
            pusExtField[1] =
                (USHORT)(prSiceInstance->ulLatchedTime.ulSeconds & (ULONG)0x0000FFFF);
            break;
          case 2:
            pusExtField[1] =
                (USHORT)((prSiceInstance->ulLatchedTime.ulNanos & (ULONG)0xFFFF0000) >> 16);
            break;
          case 3:
            pusExtField[1] =
                (USHORT)(prSiceInstance->ulLatchedTime.ulNanos & (ULONG)0x0000FFFF);
            break;
          default:
            return(SICE_SYSTEM_ERROR);
            break;
        }
        /*lint -restore const! */
      }
      else
      {
        // Wait until next time the cycle counter is 0 to latch Sercos time value.

  #if (CSMD_DRV_VERSION <=5)
        // First word of extended function field (C-Time)
        pusExtField[0] = (USHORT)
          (
            (prSiceInstance->prReg->rSCCCMDT.rSCCNT.usSccCount & ((USHORT)0x3FFF))     |   // TSRef counter
            (USHORT)(
              prSiceInstance->prReg->ulTCSR                       &
              ((ULONG)1 << CSMD_HAL_TCSR_NEW_ST)
            )                                                           |   // Toggle bit
            ((USHORT)0 << 14)                                               // Time fragment invalid
          );
  #else
        // First word of extended function field (C-Time)
        pusExtField[0] = (USHORT)
          (
            (prSiceInstance->prReg->rSCCMDT.rSCCNT.usScCount &((USHORT)0x3FFF))       |   // TSRef counter
            (USHORT)(
              prSiceInstance->prReg->ulTCSR                       &
              ((ULONG)1 << CSMD_HAL_TCSR_SYS_TIME_TOGGLE)
            )                                                           |   // Toggle bit
            ((USHORT)0 << 14)                                               // Time fragment invalid
          );
  #endif

        // Second word of extended function field (Time)
        pusExtField[1] = (USHORT) 0;
      }
    }
  }
  return(SICE_NO_ERROR);
}
