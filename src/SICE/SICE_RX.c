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
 * \file      SICE_RX.c
 *
 * \brief     Sercos SoftMaster core: Packet reception functions
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
 * \version 2014-04-01 (GMy): Added support for CoSeMa 6VRS
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 * \version 2014-05-22 (GMy): New function SICE_CalcNoOfSlaves()
 * \version 2015-06-03 (GMy): Lint code optimization
 * \version 2015-11-03 (GMy): Defdb00180480: Code optimization
 */

//---- includes ---------------------------------------------------------------

#include "../SICE/SICE_GLOB.h"
#include "../SICE/SICE_PRIV.h"

#include "../CSMD/CSMD_HAL_PRIV.h"

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
/*lint -save -w0 */
#include <sys/endian.h>
/*lint -restore */
#elif defined __RTX__
#elif defined __VXWORKS__
#elif defined __KITHARA__
#elif defined WIN32
#elif defined WIN64
#else
#error Operating system not supported by SICE!
#endif

//---- defines ----------------------------------------------------------------

/**
 * \def     SICE_RX_DESC_SVC_DATA(_DescType)
 *
 * \brief   Macro for checking whether rx descriptor type indicates SVC data
 */
#define SICE_RX_DESC_SVC_DATA(_DescType)    (   (((UCHAR)_DescType) == ((UCHAR)0x00)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x01)))

/**
 * \def     SICE_RX_DESC_RT_DATA(_DescType)
 *
 * \brief   Macro for checking whether rx descriptor type indicates RT data
 */
#define SICE_RX_DESC_RT_DATA(_DescType)     (   (((UCHAR)_DescType) == ((UCHAR)0x02)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x03)))

/**
 * \def     SICE_RX_DESC_PORTCC_DATA(_DescType)
 *
 * \brief   Macro for checking whether rx descriptor type indicates
 *          port-specific cross communication data
 */
#define SICE_RX_DESC_PORTCC_DATA(_DescType) (   (((UCHAR)_DescType) == ((UCHAR)0x08)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x09)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x0C)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x0D)))

/**
 * \def     SICE_RX_DESC_RTCC_DATA(_DescType)
 *
 * \brief   Macro for checking whether rx descriptor type indicates real-time
 *          cross communication data
 */
#define SICE_RX_DESC_RTCC_DATA(_DescType)   (   (((UCHAR)_DescType) == ((UCHAR)0x0A)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x0B)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x0E)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x0F)))

/**
 * \def     SICE_RX_DESC_START_DESC(_DescType)
 *
 * \brief   Macro for checking whether rx descriptor type indicates the first
 *          descriptor of descriptor pair
 */
#define SICE_RX_DESC_START_DESC(_DescType)  (   (((UCHAR)_DescType) == ((UCHAR)0x00)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x02)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x08)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x0A)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x0C)) || \
                                                (((UCHAR)_DescType) == ((UCHAR)0x0E)))

/**
 * \def     SICE_RX_DESC_FRAME_END(_DescType)
 *
 * \brief   Macro for checking whether receive descriptor type indicates the
 *          end of the descriptor list
 */
#define SICE_RX_DESC_FRAME_END(_DescType)       (((UCHAR)_DescType) == ((UCHAR)0x04))

/**
 * \def     SICE_RX_DESC_DATA_DELAY(_DescType)
 *
 * \brief   Macro for checking whether rx descriptor type indicates the data
 *          field delay bit
 */
#define SICE_RX_DESC_DATA_DELAY(_DescType)      ((((UCHAR)_DescType) & ((UCHAR)0x0C)) == \
                                                ((UCHAR)0x0C))

/**
 * \def     SICE_TGSR(_Port)
 *
 * \brief   Macro for accessing the TGSR register depending on the port number
 */
#define SICE_TGSR(_Port)                        *(&prSiceInstance->ulTGSR1 + _Port *    \
                                                (&prSiceInstance->ulTGSR2 -             \
                                                &prSiceInstance->ulTGSR1))


//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SICE_FUNC_RET SICE_ReceiveTelegrams(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   This function receives Sercos telegrams.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:            No error
 *          - SICE_RX_DESCRIPTOR_ERROR: Problem with RX descriptors has
 *                                      occurred, such as the signaling of an
 *                                      unsupported buffering mechanism
 *          - SICE_SOCKET_ERROR:        When a problem occurred with packet
 *                                      reception
 *          - SICE_PARAMETER_ERROR:     For function parameter error
 *
 * \details This functions reads all packets present in the receive buffer. If
 *          the preconditions (such as RX enable) are met, the TGSR registers
 *          are set depending on the type of the received packet. Only ATs are
 *          processed further, MDTs are discarded. The packets are copied to
 *          the RX RAM or other buffers according to the settings in the RX
 *          descriptors.
 *
 * \note    The function is non-blocking.
 *
 * \author  GMy, partially based on earlier work by SBe
 *
 * \ingroup SICE
 *
 * \date 2012-10-11
 *
 * \version 2012-10-11 (GMy): Baseline for Sercos master IP core v3
 * \version 2012-10-31 (GMy): Updated for emulation of IP core v4
 * \version 2013-01-10 (GMy): Optimized buffer management
 * \version 2013-02-05 (GMy): Hot-plug support added
 * \version 2013-02-06 (GMy): Support for second buffer system added
 * \version 2013-08-02 (GMy): Bugfix: Problem with Sercos phase bit mask
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 */
SICE_FUNC_RET SICE_ReceiveTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  SICE_SIII_PACKET_BUF  rReceiveFrame;
  SICE_SIII_PACKET_BUF* prReceiveFrame      = NULL;
  SICE_SIII_FRAME*      puSercosFrame       = NULL;
  UCHAR*                pucPacketBuf        = NULL;
  UCHAR                 ucPhase             = 0;
  UCHAR                 ucTelSercosType     = 0;
  INT                   iRet                = 0;
  UCHAR                 ucPacketNoIndex     = 0;
  USHORT                usIndexTableOffset  = 0;
  ULONG                 ulRXDescTableOffset = 0;
  ULONG                 ulCurDesc           = 0;
  USHORT                usCurDescOffset     = 0;
  USHORT                usFrameOffset       = 0;
  UCHAR                 ucDescType          = 0;
  USHORT                usBufOffset         = 0;
  UCHAR                 ucBufSel            = 0;
  USHORT                usLastFrameOffset   = 0;
  USHORT                usLastBufOffset     = 0;
  USHORT                usLen               = 0;
  ULONG                 ulTmpCRC            = 0;
  USHORT                usBufSysOffset      = 0;
  INT                   iPort;
  BOOL                  boIsPortP           = TRUE;

  SICE_VERBOSE(3, "SICE_ReceiveTelegrams()\n");

  if  (prSiceInstance   ==  NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  // Do for all ports (1 in case without redundancy, 2 in case of redundancy)
  // being used
  for (
      iPort = 0;
      iPort < SICE_REDUNDANCY_VAL;
      iPort++
    )
  {
    // Attempt to receive frame (non-blocking)
    iRet = RTOS_RxPacket
        (
          prSiceInstance->iInstanceNo,  // SICE instance
          iPort,                        // Port index
          rReceiveFrame.aucData,        // provided data buffer
          &pucPacketBuf                 // pointer to foreign data buffer
        );
    if (iRet < 0)
    {
      SICE_VERBOSE
          (
            0,
            "Error: Receiving data from port %d failed.\n",
            iPort
          );

      return(SICE_SOCKET_ERROR);
    }

    // Do while more frames are in receive buffer
    while (iRet > 0)
    {
      SICE_VERBOSE(2, "Received a telegram.\n");

      // Get pointer to received frame
      prReceiveFrame = &rReceiveFrame;

      // Get length of received frame
      rReceiveFrame.usLen = (USHORT) iRet;

      // Retrieve current Sercos phase
      ucPhase = (UCHAR)(prSiceInstance->prReg->ulPHASECR
              & ((ULONG) CSMD_HAL_PHASECR_PHASE_MASK));

      // Get frame pointer
      // Does receive function use own or provided packet buffer?
      if (pucPacketBuf != NULL)
      {
        // Own buffer from RTOS abstraction layer is used
        /*lint -save -e826 */
        puSercosFrame = (SICE_SIII_FRAME*)pucPacketBuf;
        /*lint -restore */
      }
      else
      {
        // Provided buffer is used
        /*lint -save -e826 */
        puSercosFrame = (SICE_SIII_FRAME*)prReceiveFrame->aucData;
        /*lint -restore */
      }

      // Check if received packet is a Sercos frame. Normally, already
      // the RTOS abstraction layer should filter for the Sercos ether
      // type, so that should not happen, unless the filter is switched off
      // to allow UCC operation
      if (puSercosFrame->rTel.usPortID !=
          (USHORT) htons((USHORT) SICE_SIII_ETHER_TYPE))
      {
        SICE_VERBOSE(1, "Warning: Received non-Sercos packet\n");

#ifdef SICE_UC_CHANNEL
        SICE_VERBOSE(1, "Writing UC packet to queue.\n");

        // Copy packet data to buffer
        memcpy
            (
              prSiceInstance->rUCCRxBuf.aucBuf[prSiceInstance->rUCCRxBuf.ucBufPtrEnd],
              puSercosFrame->aucRaw,
              usLen
            );

        //Store frame size
        prSiceInstance->rUCCRxBuf.usBufSize[prSiceInstance->rUCCRxBuf.ucBufPtrEnd] = usLen;

        // Store port index
        prSiceInstance->rUCCTxBuf.ucBufPort[prSiceInstance->rUCCTxBuf.ucBufPtrEnd] = (UCHAR)iPort;

        // Increase ring buffer pointer
        prSiceInstance->rUCCRxBuf.ucBufPtrEnd =
            (prSiceInstance->rUCCRxBuf.ucBufPtrEnd + 1) % SICE_UCC_BUF_SIZE;

        // Ring buffer overflow?
        if (prSiceInstance->rUCCRxBuf.ucBufPtrEnd == prSiceInstance->rUCCRxBuf.ucBufPtrStart)
        {
          SICE_VERBOSE(0, "Error: UCC RX Buffer overflow!");

          // Overwrite oldest packet
          prSiceInstance->rUCCRxBuf.ucBufPtrStart =
              (prSiceInstance->rUCCRxBuf.ucBufPtrStart + 1) % SICE_UCC_BUF_SIZE;

          return(SICE_BUFFER_ERROR);
        }

#endif

      }
      else
      {
        // Calculate CRC of received frame
        // \todo check if it works with big endian. htonl for conversion?
        ulTmpCRC = SICE_CRC32Calc
            (
              puSercosFrame->aucRaw,        // Data pointer
              SICE_TEL_LENGTH_HDR_FOR_CRC,  // Header length
              ((ULONG) 0)                   // Base CRC, not used here
            );

        // Is CRC of received frame correct?
        if (ulTmpCRC == puSercosFrame->rTel.ulCRC)
        {
          (VOID)SICE_IncPacketCounter
              (
                prSiceInstance,     // SICE instance
                SICE_RX_PACKET_CNT, // RX
                TRUE,               // Packet is OK
                iPort,              // Port number
                1                   // Single packet
              );

          // Un-mask Sercos telegram type
          ucTelSercosType =
              puSercosFrame->rTel.ucSercosType &
              (
                ((UCHAR) SICE_TEL_NO_MASK)      |   // Telegram number
                ((UCHAR) SICE_TEL_TYPE_MASK)    |   // MDT or AT
                ((UCHAR) SICE_TEL_CHANNEL_MASK)     // S/P channel
              );

          if ((ucTelSercosType & ((UCHAR)SICE_TEL_CHANNEL_MASK)) == (UCHAR)0)
          {
            boIsPortP = TRUE;
          }
          else
          {
            boIsPortP = FALSE;
          }

          // Check type of Sercos telegram
          switch (ucTelSercosType & ~((UCHAR) SICE_TEL_CHANNEL_MASK))
          {
            // MDT0
            case (SICE_TEL_TYPE_MDT + CSMD_TELEGRAM_NBR_0):
              // Signal packet reception in TGSR: MDT0 and valid MST
              SICE_TGSR(iPort) |= (ULONG) CSMD_HAL_TGSR_MDT0 | (ULONG) CSMD_HAL_TGSR_MST_VALID;

              //if ((ucTelSercosType & ((UCHAR) SICE_TEL_CHANNEL_MASK)) != (UCHAR) 0)
              if (boIsPortP == FALSE)
              {
                SICE_TGSR(iPort) |= (ULONG) CSMD_HAL_TGSR_SEC_TEL;
              }
              ucPacketNoIndex    =  (UCHAR) CSMD_DES_IDX_MDT0;

              SICE_VERBOSE
                  (
                    3,
                    "Received packet type: MDT0 (P/S: %d) on Port %d\n",
                    boIsPortP,
                    iPort
                  );
              break;

            // MDT1
            case (SICE_TEL_TYPE_MDT + CSMD_TELEGRAM_NBR_1):
              // Signal packet reception in TGSR
              SICE_TGSR(iPort) |= (ULONG) CSMD_HAL_TGSR_MDT1;
              ucPacketNoIndex  =  (UCHAR) CSMD_DES_IDX_MDT1;

              SICE_VERBOSE
                  (
                    3,
                    "Received packet type: MDT1 (P/S: %d) on Port %d\n",
                    boIsPortP,
                    iPort
                  );
              break;

            // MDT2
            case (SICE_TEL_TYPE_MDT + CSMD_TELEGRAM_NBR_2):
              // Signal packet reception in TGSR
              SICE_TGSR(iPort) |= (ULONG) CSMD_HAL_TGSR_MDT2;
              ucPacketNoIndex  =  (UCHAR) CSMD_DES_IDX_MDT2;
              SICE_VERBOSE
                  (
                    3,
                    "Received packet type: MDT2 (P/S: %d) on Port %d\n",
                    boIsPortP,
                    iPort
                  );
              break;

            // MDT3
            case (SICE_TEL_TYPE_MDT + CSMD_TELEGRAM_NBR_3):
              // Signal packet reception in TGSR
              SICE_TGSR(iPort) |= (ULONG) CSMD_HAL_TGSR_MDT3;
              ucPacketNoIndex  =  (UCHAR) CSMD_DES_IDX_MDT3;
              SICE_VERBOSE
                  (
                    3,
                    "Received packet type: MDT3 (P/S: %d) on Port %d\n",
                    boIsPortP,
                    iPort
                  );
              break;

            // AT0
            case (SICE_TEL_TYPE_AT + CSMD_TELEGRAM_NBR_0):
              // Signal packet reception in TGSR
              SICE_TGSR(iPort) |= (ULONG) CSMD_HAL_TGSR_AT0;
              ucPacketNoIndex  =  (UCHAR) CSMD_DES_IDX_AT0;
              SICE_VERBOSE
                  (
                    3,
                    "Received packet type: AT0 (P/S: %d) on Port %d\n",
                    boIsPortP,
                    iPort
                  );

              // In CP0, evaluate sequence counter value
              if (ucPhase == ((UCHAR) CSMD_SERC_PHASE_0))
              {
                // Set sequence counter register (SEQCNT)
                // \todo OK for big endian?
                if (iPort == 0)
                {
                  prSiceInstance->prReg->ulSEQCNT =
                      (prSiceInstance->prReg->ulSEQCNT & (ULONG) 0xFFFF0000)    |
                      (ULONG)((USHORT)*((USHORT *)puSercosFrame->rTel.aucData));
                }
                else
                {
                  prSiceInstance->prReg->ulSEQCNT =
                      (prSiceInstance->prReg->ulSEQCNT & (ULONG) 0x0000FFFF)    |
                      (((ULONG)((USHORT)*((USHORT *)puSercosFrame->rTel.aucData))) << 16);
                }

                // Store number of recognized slaves.
                prSiceInstance->usNumRecogDevs = (USHORT) SICE_CalcNoOfSlaves(prSiceInstance);
              }
              break;

            // AT1
            case (SICE_TEL_TYPE_AT + CSMD_TELEGRAM_NBR_1):
              // Signal packet reception in TGSR
              SICE_TGSR(iPort) |= (ULONG) CSMD_HAL_TGSR_AT1;
              ucPacketNoIndex  =  (UCHAR) CSMD_DES_IDX_AT1;
              SICE_VERBOSE
                  (
                    3,
                    "Received packet type: AT1 (P/S: %d) on Port %d\n",
                    boIsPortP,
                    iPort
                  );
              break;

            // AT2
            case (SICE_TEL_TYPE_AT + CSMD_TELEGRAM_NBR_2):
              // Signal packet reception in TGSR
              SICE_TGSR(iPort) |= (ULONG) CSMD_HAL_TGSR_AT2;
              ucPacketNoIndex  =  (UCHAR) CSMD_DES_IDX_AT2;
              SICE_VERBOSE
                  (
                    3,
                    "Received packet type: AT2 (P/S: %d) on Port %d\n",
                    boIsPortP,
                    iPort
                  );
              break;

            // AT3
            case (SICE_TEL_TYPE_AT + CSMD_TELEGRAM_NBR_3):
              // Signal packet reception in TGSR
              SICE_TGSR(iPort) |= (ULONG) CSMD_HAL_TGSR_AT3;
              ucPacketNoIndex     =  (UCHAR) CSMD_DES_IDX_AT3;
              SICE_VERBOSE
                  (
                    3,
                    "Received packet type: AT3 (P/S: %d) on Port %d\n",
                    boIsPortP,
                    iPort
                  );
              break;

            default:
              SICE_VERBOSE
                  (
                    0,
                    "Warning: Received illegal Sercos packet type\n"
                  );
              break;
          }

          // Only copy data from AT frames
          if ((puSercosFrame->rTel.ucSercosType & ((UCHAR) SICE_TEL_TYPE_MASK))
              == ((UCHAR) SICE_TEL_TYPE_AT))
          {
            // Find corresponding descriptors for receive frame

            // RX index table offset
            usIndexTableOffset  =
                prSiceInstance->prReg->rDECR.rDesIdx.usOffsetRxRam;

            /*lint -save -e826 */
            ulRXDescTableOffset =
                *(
                  (ULONG*)(
                        (UCHAR *)prSiceInstance->prRX_Ram
                        + ((ULONG) usIndexTableOffset)
                        + ((ULONG) ucPacketNoIndex) * ((ULONG) sizeof(ULONG))
                      )
                );
            /*lint -restore */

            // Descriptor enable bit, currently ignored
            // (the same in current Sercos master 'hard' ip core)
            // byRXDescTableEnabled = ulTXDescTableOffset & SIII_TEL_DESC_ENABLE_MASK;

            // De-mask
            ulRXDescTableOffset =
                ulRXDescTableOffset & ((ULONG) SICE_TEL_DESC_OFFSET_MASK);

            usCurDescOffset = (USHORT) ulRXDescTableOffset;

            // Browse through all descriptors for this packet
            do
            {
              // Get RX descriptor
              /*lint -save -e826 */
              ulCurDesc =
                  *(
                    (ULONG*)((UCHAR *)prSiceInstance->prRX_Ram +
                          (ULONG) usCurDescOffset)
                  );
              /*lint -restore */

              usCurDescOffset += (USHORT) sizeof(ULONG);

              usFrameOffset = (USHORT) ((ulCurDesc & ((ULONG) CSMD_HAL_DES_MASK_TEL_OFFS))
                        >> CSMD_HAL_DES_SHIFT_TEL_OFFS);
              ucDescType    = (UCHAR) ((ulCurDesc & ((ULONG) CSMD_HAL_DES_MASK_TYPE))
                        >> CSMD_HAL_DES_SHIFT_TYPE);
              usBufOffset   = (USHORT) (ulCurDesc & ((ULONG) CSMD_HAL_DES_MASK_BUFF_OFFS));
              ucBufSel      = (UCHAR) ((ulCurDesc & ((ULONG) CSMD_HAL_DES_MASK_BUFF_SYS_SEL))
                        >> CSMD_HAL_DES_SHIFT_BUFF_SYS_SEL);

              // Is it first descriptor of descriptor pair?
              if (SICE_RX_DESC_START_DESC(ucDescType))
              {
                // Store start offsets for frame and buffer
                usLastFrameOffset = usFrameOffset;
                usLastBufOffset   = usBufOffset;
              }
              // Second descriptor of descriptor pair
              else
              {
                // Calculate length of data to be copied
                usLen = (usFrameOffset - usLastFrameOffset) +
                    ((USHORT) sizeof(USHORT));     // USHORT alignment

                // Real-time data
                if (SICE_RX_DESC_RT_DATA(ucDescType))
                {
                  switch (ucBufSel)
                  {
                    case 0:
                      if (iPort == 0)
                      {
                        usBufSysOffset = (USHORT) CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A;
                      }
                      else
                      {
                        usBufSysOffset = (USHORT) CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A;
                      }
                      break;
                    case 1:
                      if (iPort == 0)
                      {
                        usBufSysOffset = (USHORT) CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_B;
                      }
                      else
                      {
                        usBufSysOffset = (USHORT) CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_B;
                      }
                      break;
                    default:
                      SICE_VERBOSE
                          (
                            0,
                            "Error in RX descriptor: Selected buffer "
                            "system %c is not supported.",
                            ucBufSel
                          );

                      return(SICE_RX_DESCRIPTOR_ERROR);
                      /*lint -save -e527 */
                      break;
                      /*lint -restore */
                  }
                  (VOID)memcpy
                      (
                        (UCHAR *)prSiceInstance->prRX_Ram +
                          prSiceInstance->prReg->aulRxBufBasePtr[usBufSysOffset] +
                          usLastBufOffset,
                        &puSercosFrame->rTel.aucData[usLastFrameOffset],
                        usLen
                      );
                }
                // SVC data
                else if (SICE_RX_DESC_SVC_DATA(ucDescType))
                {
                  if (iPort == 0)
                  {
                    usBufSysOffset = CSMD_HAL_IDX_RX_P1_BUFF_SVC;
                  }
                  else
                  {
                    usBufSysOffset = CSMD_HAL_IDX_RX_P2_BUFF_SVC;
                  }
                  (VOID)memcpy
                      (
                        (UCHAR *)prSiceInstance->prRX_Ram +
                          prSiceInstance->prReg->aulRxBufBasePtr[usBufSysOffset] +
                          usLastBufOffset,
                        &puSercosFrame->rTel.aucData[usLastFrameOffset],
                        usLen
                      );
                }
                // Port-specific CC data
                else if (SICE_RX_DESC_PORTCC_DATA(ucDescType))
                {
                  // Written to TX RAM, not RX RAM
                  // \todo send out again on other port in case of double line
                  (VOID)memcpy
                      (
                        (UCHAR *)prSiceInstance->prTX_Ram +
                          prSiceInstance->prReg->aulRxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_PORT_WR_TX] +
                          usLastBufOffset,
                        &puSercosFrame->rTel.aucData[usLastFrameOffset],
                        usLen
                      );

                  if (SICE_RX_DESC_DATA_DELAY(ucDescType))
                  {
                    // \todo set data field delay accordingly
                    // Currently not needed, as only single line supported
                  }
                }
                // Real-time CC data
                else if (SICE_RX_DESC_RTCC_DATA(ucDescType))
                {
                  // Written to TX RAM with buffer base pointer of port specific buffer,
                  // but buffer offset of RTD
                  // \todo send out again on other port in case of double line
                  switch (ucBufSel)
                  {
                    case 0:
                      usBufSysOffset = (USHORT) CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A;
                      break;
                    case 1:
                      usBufSysOffset = (USHORT) CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_B;
                      break;
                    default:
                      SICE_VERBOSE
                          (
                            0,
                            "Error in RX descriptor: Selected buffer "
                            "system %d is not supported.",
                            usBufSysOffset
                          );

                      return(SICE_RX_DESCRIPTOR_ERROR);
                      /*lint -save -e527 */
                      break;
                      /*lint -restore */
                  }

                  (VOID)memcpy
                      (
                        (UCHAR *)prSiceInstance->prRX_Ram +
                          prSiceInstance->prReg->aulRxBufBasePtr[usBufSysOffset] +
                          usLastBufOffset,
                        &puSercosFrame->rTel.aucData[usLastFrameOffset],
                        usLen
                      );

                  (VOID)memcpy
                      (
                        (UCHAR *)prSiceInstance->prTX_Ram +
                          prSiceInstance->prReg->aulTxBufBasePtr[CSMD_HAL_IDX_RX_P1_BUFF_PORT_WR_TX] +
                          usLastBufOffset,
                        &puSercosFrame->rTel.aucData[usLastFrameOffset],
                        usLen
                      );

                  if (SICE_RX_DESC_DATA_DELAY(ucDescType))
                  {
                    // \todo set data field delay accordingly
                    // Currently not needed, as only single line supported
                  }
                }
                else if (!SICE_RX_DESC_FRAME_END(ucDescType))
                {
                  SICE_VERBOSE
                      (
                        0,
                        "Error: Unknown RX descriptor type\n"
                      );
                  return(SICE_RX_DESCRIPTOR_ERROR);
                }
              }

              // Packet done
              if (SICE_RX_DESC_FRAME_END(ucDescType))
              {
                /*
                TODO
                set only if rxbuftr & tgsr == rxbuftr (+ masken),
                i.e. all configured telegrams were received
                */
                // Signal new data flag
                if (iPort == 0)
                {
                  prSiceInstance->prReg->ulRXBUFCSR_A |=
                      (ULONG) CSMD_HAL_RXBUFCSR_NEW_DATA_P1;
                }
                else
                {
                  prSiceInstance->prReg->ulRXBUFCSR_A |=
                      (ULONG) CSMD_HAL_RXBUFCSR_NEW_DATA_P2;
                }
                SICE_VERBOSE(2, "Packet copied to RX RAM\n");
              }
            } while (!SICE_RX_DESC_FRAME_END(ucDescType));
          }// if AT frame
        }
        else
        {
          SICE_VERBOSE(0, "Warning: Received Sercos packet with broken CRC\n");

          // Increase error counter
          (VOID)SICE_IncPacketCounter
              (
                prSiceInstance,     // SICE instance
                SICE_RX_PACKET_CNT, // RX
                FALSE,              // Packet is erroneus
                iPort,              // Port number
                1                   // Single packet
              );

        } // check frame CRC
      } // if received frame is Sercos packet

      // Receive following frame (non-blocking)
      iRet = RTOS_RxPacket
          (
            prSiceInstance->iInstanceNo,  // SICE instance
            iPort,                        // Port index
            rReceiveFrame.aucData,        // provided data buffer
            &pucPacketBuf                 // pointer to foreign data buffer
          );

      if (iRet < 0)
      {
        SICE_VERBOSE
            (
              0,
              "Error: Receiving data from port %d failed.\n",
              iPort
            );

        return(SICE_SOCKET_ERROR);
      }
    } // While (iRet > 0)
  } // For all ports

  prSiceInstance->prReg->ulTGSR1 = prSiceInstance->ulTGSR1;
  prSiceInstance->prReg->ulTGSR2 = prSiceInstance->ulTGSR2;

  return(SICE_NO_ERROR);
}

/**
 * \fn SICE_FUNC_RET SICE_CheckPreCondsReceive(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   This function checks whether the pre-conditions for receiving
 *          Sercos packets are met.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:            Pre-conditions are met
 *          - SICE_CHECK_STATUS_FALSE:  Pre-Conditions are not (or only
 *                                      partially) met
 *          - SICE_PARAMETER_ERROR:     For function parameter error
 *
 * \author GMy
 *
 * \ingroup SICE
 *
 * \date 2013-04-22
 *
 * \version 2013-04-22 (GMy): Moved logic to separate function
 * \version 2013-08-02 (GMy): Bugfix: Problem with Sercos phase bit mask
 */
SICE_FUNC_RET SICE_CheckPreCondsReceive
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  UCHAR ucPhase;

  SICE_VERBOSE(3, "SICE_CheckPreCondsReceive()\n");

  if  (prSiceInstance ==  NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  // Retrieve current Sercos phase
  ucPhase = (UCHAR)   (
              prSiceInstance->prReg->ulPHASECR       &
              ((ULONG) CSMD_HAL_PHASECR_PHASE_MASK)
            );

#if (CSMD_DRV_VERSION <= 5)
  /*lint -save -e568 -e685 const! */
  if  (
      (ucPhase >= ((UCHAR)CSMD_SERC_PHASE_0))                     &&
                        // if in CP0 or higher (currently always TRUE)
      (prSiceInstance->prReg->ulTCSR                          &
          (((ULONG) 1) << ((ULONG) CSMD_HAL_TCSR_ET0)))           &&
                        // if main system timer running
      (prSiceInstance->prReg->ulDFCSR                         &
          (((ULONG) 1) << ((ULONG) CSMD_HAL_DFCSR_RX_ENABLE)))
    )                     // if packet reception is enabled
  /*lint -restore const! */
  {
    return(SICE_NO_ERROR);
  }
  else
  {
    return(SICE_CHECK_STATUS_FALSE);
  }
#else
  if  (
      (ucPhase >= ((UCHAR)CSMD_SERC_PHASE_0))                     &&
                        // if in CP0 or higher (currently always TRUE)
      (prSiceInstance->prReg->ulTCSR                           &
          (((ULONG) 1) << ((ULONG) CSMD_HAL_TCSR_ET0)))           &&
                        // if main system timer running
      (prSiceInstance->prReg->rDFCSR.ulLong                    &
          (((ULONG) 1) << ((ULONG) CSMD_HAL_DFCSR_RX_ENABLE)))
    )                     // if packet reception is enabled

  {
    return(SICE_NO_ERROR);
  }
  else
  {
    return(SICE_CHECK_STATUS_FALSE);
  }
#endif
}

/**
 * \fn INT SICE_CalcNoOfSlaves(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   This function calculates the number of present Sercos slaves based
 *          on the sequence counters received in Sercos CP0
 *
 * \note    Currently only works for ring and single line topology. The
 *          function shall only be called in Sercos phase CP0.
 *
 * \return  -1 for Error
 *          No of Slaves otherwise
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2013-05-22
 *
 * \version 2015-07-27 (GMy): Defdb00180781: Wrong number of slaves calculation
 *                            in redundancy mode
 */
INT SICE_CalcNoOfSlaves
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  INT iNumSlaves = 0;

  if (SICE_REDUNDANCY_BOOL)
  {
    // Check whether ring topology or not
    if ((prSiceInstance->prReg->ulSEQCNT & 0x00008000UL) != 0UL)
    {
      // Ring topology
      iNumSlaves += (USHORT)((prSiceInstance->prReg->ulSEQCNT & 0x00007FFFUL) - 1UL);
    }
    else
    {
      // Line or double line topology
      iNumSlaves += (USHORT)((prSiceInstance->prReg->ulSEQCNT & 0x00007FFFUL) - 1UL);
      iNumSlaves += (USHORT)(((prSiceInstance->prReg->ulSEQCNT & 0x7FFF0000UL) >> 16) - 1UL);
    }
  }
  else
  {
    // Line topology
    iNumSlaves += (USHORT)((prSiceInstance->prReg->ulSEQCNT & 0x0000FFFFUL) / 2UL);
  }
  return(iNumSlaves);
}

