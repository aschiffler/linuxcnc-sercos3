/*
 * Sercos Soft Master Core Library
 * Version: see SICE_GLOB.h
 * Copyright (C) 2012 - 2016 Bosch Rexroth AG
 *
 * ATTENTION: This file for UCC support functionality is not part of an
 * official release of SICE yet, but is in prototype status just for
 * evaluation. It may only be used in safe environments, not in real machines!
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
 * \file      SICE_UCC.c
 *
 * \brief     Sercos SoftMaster core: UC channel handling
 *
 * \note      Prototype status!
 *
 * \attention Only for testing in safe environments, not thoroughly tested and
 *            not officially released yet!
 *
 * \ingroup   SICE_UCC
 *
 * \author    GMy
 *
 * \date      2013-02-24
 *
 * \copyright Copyright Bosch Rexroth AG, 2013-2016
 *
 * \version 2013-02-24 (GMy): Baseline version
 * \version 2013-02-28 (GMy): Lint code optimization
 * \version 2015-05-19 (GMy): Preliminary implementation of UCC
 * \version 2015-11-03 (GMy): Defdb00180480: Code optimization
 */

//---- includes ---------------------------------------------------------------

#include "../SICE/SICE_GLOB.h"
#include "../SICE/SICE_PRIV.h"


//---- defines ----------------------------------------------------------------

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SICE_FUNC_RET SICE_UCC_Init(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              SICE_UCC_CONFIG_STRUCT *prUCCConfig,
 *              UCHAR* pucMAC
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 * \param[in,out]   prUCCConfig     Pointer UCC configuration struct
 * \param[out]      pucMAC          MAC address
 *
 * \brief   Initializes UC channel. This function is called by SICE_Init().
 *
 * \attention   Not tested yet!
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:        No error
 *          - SICE_PARAMETER_ERROR: For function parameter error
 *          - SICE_CONFIG_ERROR:    UCC not enabled
 *          - SICE_SOCKET_ERROR:    Problem with socket
 *
 * \author  GMy
 *
 * \ingroup SICE_UCC
 *
 * \date    2015-05-19
 */
SICE_FUNC_RET SICE_UCC_Init
  (
    SICE_INSTANCE_STRUCT *prSiceInstance,
    SICE_UCC_CONFIG_STRUCT *prUCCConfig,
    UCHAR* pucMAC
  )
{
#ifdef SICE_UC_CHANNEL

  ULONG ulRet = 0;

  SICE_VERBOSE(3, "SICE_InitUCC()\n");

  if (
      (prSiceInstance == NULL)    ||
      (prUCCConfig == NULL)       ||
      (pucMAC == NULL)
    )
  {
    return(SICE_PARAMETER_ERROR);
  }

  // Initialize UCC packet buffers
  prSiceInstance->rUCCRxBuf.ucBufPtrStart = 0;
  prSiceInstance->rUCCRxBuf.ucBufPtrEnd   = 0;
  prSiceInstance->rUCCTxBuf.ucBufPtrStart = 0;
  prSiceInstance->rUCCTxBuf.ucBufPtrEnd   = 0;

  // IP transmit stack (IPTXS)
  prSiceInstance->prReg->ulIPTXS1 = (ULONG) 0;
  prSiceInstance->prReg->ulIPTXS2 = (ULONG) 0;

  // IP buffers in RX RAM (IPRRS)
  prSiceInstance->prReg->ulIPRRS1 = (ULONG) 0;
  prSiceInstance->prReg->ulIPRRS2 = (ULONG) 0;

  // IP receive stack (IPRXS)
  prSiceInstance->prReg->ulIPRXS1 = (ULONG) 0;
  prSiceInstance->prReg->ulIPRXS2 = (ULONG) 0;

  // Maximum transmit unit MTU (IPLASTFL)
  prSiceInstance->prReg->ulIPLASTFL = (ULONG) 0;

  // Frame and error counter registers
  prSiceInstance->prReg->rIPFCSERR.ulErrCnt   = (ULONG) 0;
  prSiceInstance->prReg->rIPFRXOK.ulErrCnt    = (ULONG) 0;
  prSiceInstance->prReg->rIPFTXOK.ulErrCnt    = (ULONG) 0;
  prSiceInstance->prReg->rIPALGNERR.ulErrCnt  = (ULONG) 0;
  prSiceInstance->prReg->rIPDISRXB.ulErrCnt   = (ULONG) 0;
  prSiceInstance->prReg->rIPCHVIOL.ulErrCnt   = (ULONG) 0;
  prSiceInstance->prReg->rIPSERCERR.ulErrCnt  = (ULONG) 0;
  prSiceInstance->prReg->rIPCSR1.ulIPCSR      = (ULONG) 0;
  prSiceInstance->prReg->rIPCSR2.ulIPCSR      = (ULONG) 0;
  prSiceInstance->prReg->rIPDISRXB.ulErrCnt   = (ULONG) 0;
  prSiceInstance->prReg->rIPDISCOLB.ulErrCnt  = (ULONG) 0;

  ulRet = RTOS_OpenUCCSocket
      (
        prSiceInstance->iInstanceNo,
        pucMAC
      );

  if (ulRet == RTOS_RET_OK)
  {
    return(SICE_NO_ERROR);
  }
  else
  {
    return(SICE_SOCKET_ERROR);
  }

#else

  SICE_VERBOSE(3, "Warning: SICE_InitUCC() called, but UCC disabled\n");
  return (SICE_CONFIG_ERROR);

#endif

}

/**
 * \fn SICE_FUNC_RET SICE_UCC_Close(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   Close UC channel. This function is called by SICE_Close().
 *
 * \attention   Not tested yet!
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:        No error
 *          - SICE_PARAMETER_ERROR: For function parameter error
 *          - SICE_CONFIG_ERROR:    UCC not enabled
 *
 * \details This function is non-blocking.
 *
 * \author  GMy
 *
 * \ingroup SICE_UCC
 *
 * \date    2015-05-19
 */
SICE_FUNC_RET SICE_UCC_Close
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
#ifdef SICE_UC_CHANNEL

  SICE_VERBOSE(3, "SICE_CloseUCC()\n");

  if (prSiceInstance == NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  RTOS_CloseUCCSocket
      (
        prSiceInstance->iInstanceNo
      );

  return(SICE_NO_ERROR);

#else

  SICE_VERBOSE(3, "Warning: SICE_CloseUCC() called, but UCC disabled\n");
  return(SICE_CONFIG_ERROR);

#endif

}

/**
 * \fn SICE_FUNC_RET SICE_UCC_Reset(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   Resets UC channel. This function is called by SICE_SoftReset().
 *
 * \attention   Not tested yet!
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:        No error
 *          - SICE_PARAMETER_ERROR: For function parameter error
 *          - SICE_CONFIG_ERROR:    UCC not enabled
 *
 * \author  GMy
 *
 * \ingroup SICE_UCC
 *
 * \date    2015-06-12
 */
SICE_FUNC_RET SICE_UCC_Reset
  (
    SICE_INSTANCE_STRUCT *prSiceInstance
  )
{
#ifdef SICE_UC_CHANNEL

  SICE_VERBOSE(3, "SICE_ResetUCC()\n");

  if (prSiceInstance == NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  // IP status and control registers (IPCSR1..2)
  prSiceInstance->prReg->rIPCSR1.ulIPCSR  = (ULONG) 0;
  prSiceInstance->prReg->rIPCSR2.ulIPCSR  = (ULONG) 0;

  // MAC address register (MAC1)
  // Different behavior compared to Sercos master IP core, as initialization
  // value here is 0.
  prSiceInstance->prReg->rMAC1.ul.AddressH =
      (((ULONG) prSiceInstance->aucUccMAC[5]) << 0)  +
      (((ULONG) prSiceInstance->aucUccMAC[4]) << 8)  +
      (((ULONG) prSiceInstance->aucUccMAC[3]) << 16) +
      (((ULONG) prSiceInstance->aucUccMAC[2]) << 24);

  prSiceInstance->prReg->rMAC1.ul.AddressL =
      (((ULONG) prSiceInstance->aucUccMAC[1]) << 0)  +
      (((ULONG) prSiceInstance->aucUccMAC[0]) << 8);

  // IP transmit stack (IPTXS)
  prSiceInstance->prReg->ulIPTXS1 = (ULONG) 0;
  prSiceInstance->prReg->ulIPTXS2 = (ULONG) 0;

  // IP buffers in RX RAM (IPRRS)
  prSiceInstance->prReg->ulIPRRS1 = (ULONG) 0;
  prSiceInstance->prReg->ulIPRRS2 = (ULONG) 0;

  // IP receive stack (IPRXS)
  prSiceInstance->prReg->ulIPRXS1 = (ULONG) 0;
  prSiceInstance->prReg->ulIPRXS2 = (ULONG) 0;

  return(SICE_NO_ERROR);

#else

  SICE_VERBOSE(3, "Warning: SICE_ResetUCC() called, but UCC disabled\n");
  return (SICE_CONFIG_ERROR);

#endif

}

/**
 * \fn SICE_FUNC_RET SICE_UCC_Cycle(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              ULONG ulUCCDuration
 *          )
 *
 * \public
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 * \param[in]       ulUCCDuration   Duration of UCC interval in ns (net value
 *                                  with timing jitter taken into account)
 *
 * \brief   Starts cyclic UC channel handling: transmission and reception of
 *          UCC packets.
 *
 * \note    This function shall NOT be called in case the NIC is used to
 *          create the Sercos packet timing! In that case, the function
 *          SICE_SendTelegramsNicTimed() takes care of transmission, and
 *          SICE_UCC_ReceiveTelegrams() should be used for reception.
 *
 * \attention   Not tested yet!
 *
 * \details This function should to called cyclically at the beginning of each
 *          UCC interval. Taking care of the timing both regarding the point of
 *          time when this function is called as well as regarding the
 *          calculation of ulUCCDuration is up to the calling entity.
 *          This function is non-blocking.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:     No error
 *          - SICE_CONFIG_ERROR: UCC not enabled
 *          - SICE_BUFFER_ERROR: Ring buffer overflow
 *          - SICE_SOCKET_ERROR: Problems with UCC socket
 *
 * \author  GMy
 *
 * \ingroup SICE_UCC
 *
 * \date    2015-05-19
 *
 * \version 2015-07-14 (GMy): Code optimization, separation to transmit and
 *                            receive functions
 */
SICE_FUNC_RET SICE_UCC_Cycle
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      ULONG ulUCCDuration
    )
{
#ifdef SICE_UC_CHANNEL

  SICE_FUNC_RET eSiceRet = SICE_NO_ERROR;

  SICE_VERBOSE(3, "SICE_StartUCC()\n");

  eSiceRet = SICE_UCC_SendTelegrams
      (
        prSiceInstance,
        ulUCCDuration
      );

  if (eSiceRet != SICE_NO_ERROR)
  {
    return (eSiceRet);
  }

  eSiceRet = SICE_UCC_ReceiveTelegrams
      (
        prSiceInstance,
        FALSE
      );
  return (eSiceRet);

#else

  SICE_VERBOSE(3, "Warning: SICE_StartUCC() called, but UCC disabled\n");
  return (SICE_CONFIG_ERROR);

#endif

}

/**
 * \fn SICE_FUNC_RET SICE_UCC_SendTelegrams(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              ULONG ulUCCDuration
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 * \param[in]       ulUCCDuration   Duration of UCC interval in ns (net value
 *                                  with timing jitter taken into account)
 *
 * \brief   Transmits UCC telegrams. This function should only be called by
 *          SICE_UCC_Cycle().
 *
 * \note    This function shall NOT be called in case the NIC is used to
 *          create the Sercos packet timing! In that case, the function
 *          SICE_SendTelegramsNicTimed() takes care of that.
 *
 * \attention   Not tested yet!
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:        No error
 *          - SICE_CONFIG_ERROR:    UCC not enabled
 *          - SICE_SOCKET_ERROR:    Problems with UCC socket
 *          - SICE_PARAMETER_ERROR: For function parameter error
 *
 * \author  GMy
 *
 * \ingroup SICE_UCC
 *
 * \date    2015-07-14
 */
SICE_FUNC_RET SICE_UCC_SendTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      ULONG ulUCCDuration
    )
{
#ifdef SICE_UC_CHANNEL

  ULONG  ulRemUCCDuration = ulUCCDuration;
  INT    iRet             = 0;
  INT    iCnt             = 0;

  SICE_VERBOSE(3, "SICE_UCC_SendTelegrams()\n");

  if (prSiceInstance == NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  // Transmit UCC packets
  if (SICE_UCC_GetTxQueueSize(prSiceInstance) > 0)
  {
    // Check whether packets to transmit are left, whether there is enough time
    // for the next packet within the remaining UCC interval as well as the
    // maximum number of UCC packets per cycle is not exceeded
    while (
            (SICE_UCC_GetTxQueueSize(prSiceInstance) > 0)          &&
            (SICE_CalcPacketDuration
                 (
                   prSiceInstance,
                   SICE_UCC_GetTxNextPacketSize(prSiceInstance)
                  ) <= ulRemUCCDuration
             )                                                      &&
             (iCnt < RTOS_UCC_MAX_PACKETS)
          )
    {
      iCnt++;

      // Calculate new remaining time of UCC interval
      ulRemUCCDuration -= SICE_CalcPacketDuration
          (
            prSiceInstance,
            SICE_UCC_GetTxNextPacketSize(prSiceInstance)
          );

      // Send packet via RTOS function

      iRet = RTOS_TxUCCPacket
          (
            prSiceInstance->iInstanceNo,
            prSiceInstance->rUCCTxBuf.ucBufPort[prSiceInstance->rUCCTxBuf.ucBufPtrStart],
            prSiceInstance->rUCCTxBuf.aucBuf[prSiceInstance->rUCCTxBuf.ucBufPtrStart],
            prSiceInstance->rUCCTxBuf.usBufSize[prSiceInstance->rUCCTxBuf.ucBufPtrStart]
          );
      if (iRet < 0)
      {
        SICE_VERBOSE
            (
              0,
              "Error: Transmitting UCC data on port %d failed.\n",
              prSiceInstance->rUCCTxBuf.ucBufPort[prSiceInstance->rUCCTxBuf.ucBufPtrStart]
            );
        return(SICE_SOCKET_ERROR);
      }
      else
      {
        // Successfully transmitted the packet
        prSiceInstance->rUCCTxBuf.ucBufPtrStart = (prSiceInstance->rUCCTxBuf.ucBufPtrStart +1) % SICE_UCC_BUF_SIZE;
        SICE_VERBOSE(1, "Successfully transmitted UCC packet\n");
      }
    }
  }

  return(SICE_NO_ERROR);

#else

  SICE_VERBOSE(3, "Warning: SICE_UCC_SendTelegrams() called, but UCC disabled\n");
  return (SICE_CONFIG_ERROR);

#endif
}

/**
 * \fn SICE_FUNC_RET SICE_UCC_ReceiveTelegrams(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              BOOL boNrtState
 *          )
 *
 * \public
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 * \param[in]       boNrtState      TRUE for NRT state, false otherwise
 *
 * \brief   Receive UCC telegrams. This function should only be called by
 *          SICE_UCC_Cycle(), or in case of NIC-timed mode, by
 *          SICE_Cycle_Start()
 *
 * \attention   Not tested yet!
 *
 * \details In case more packets are received than the ring buffer can hold,
 *          the oldest packets are overwritten (= dropped).
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:        No error
 *          - SICE_CONFIG_ERROR:    UCC not enabled
 *          - SICE_BUFFER_ERROR:    Ring buffer overflow
 *          - SICE_SOCKET_ERROR:    Problems with UCC socket
 *          - SICE_PARAMETER_ERROR: For function parameter error
 *
 * \author  GMy
 *
 * \ingroup SICE_UCC
 *
 * \date    2015-07-14
 */
SICE_FUNC_RET SICE_UCC_ReceiveTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      BOOL boNrtState
    )
{
#ifdef SICE_UC_CHANNEL

  INT iPort = 0;
  INT iRet  = 1;

  SICE_VERBOSE(3, "SICE_UCC_ReceiveTelegrams()\n");

  if (prSiceInstance == NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  for (
      iPort = 0;
      iPort < 2;
      iPort++
    )
  {
    do
    {
      iRet = RTOS_RxUCCPacket
          (
            prSiceInstance->iInstanceNo,
            iPort,
            boNrtState,
            prSiceInstance->rUCCRxBuf.aucBuf[prSiceInstance->rUCCRxBuf.ucBufPtrEnd]
          );
      if (iRet < 0)
      {
        SICE_VERBOSE
            (
              0,
              "Error: Receiving UCC data from port %d failed.\n",
              iPort
            );
        return(SICE_SOCKET_ERROR);
      }
      else if (iRet > 0)
      {
        // Received UCC packet
        prSiceInstance->rUCCRxBuf.ucBufPort[prSiceInstance->rUCCRxBuf.ucBufPtrEnd] = iPort;
        prSiceInstance->rUCCRxBuf.usBufSize[prSiceInstance->rUCCRxBuf.ucBufPtrEnd] = iRet;

        // Increase ring buffer pointer
        prSiceInstance->rUCCRxBuf.ucBufPtrEnd =
            (prSiceInstance->rUCCRxBuf.ucBufPtrEnd + 1) % SICE_UCC_BUF_SIZE;

        // Ring buffer overflow?
        if (prSiceInstance->rUCCRxBuf.ucBufPtrEnd == prSiceInstance->rUCCRxBuf.ucBufPtrStart)
        {
          SICE_VERBOSE(0, "Error: UCC RX Buffer overflow!");

          // Overwrite (= drop) oldest packet
          prSiceInstance->rUCCRxBuf.ucBufPtrStart =
              (prSiceInstance->rUCCRxBuf.ucBufPtrStart + 1) % SICE_UCC_BUF_SIZE;

          (VOID)SICE_IncPacketCounter
              (
                prSiceInstance,           // SICE instance structure
                SICE_RX_UCC_DISC,         // Discarded receive packet
                FALSE,                    // Error
                iPort,                    // Port
                1                         // One packet
              );

          return(SICE_BUFFER_ERROR);
        }
      }                  // if packet received
    } while (iRet > 0);  // while more packets received
  }                      // for all ports

  return(SICE_NO_ERROR);
#else

  SICE_VERBOSE(3, "Warning: SICE_StartUCC() called, but UCC disabled\n");
  return (SICE_CONFIG_ERROR);

#endif
}

/**
 * \fn SICE_FUNC_RET SICE_UCC_PutPacket(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              UCHAR ucPort,
 *              UCHAR* pucFrame,
 *              USHORT usLen
 *          )
 *
 * \public
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 * \param[in]       ucPort          Port index (SICE_ETH_PORT_P,
 *                                  SICE_ETH_PORT_S, or SICE_ETH_PORT_BOTH)
 * \param[in]       pucFrame        Pointer to packet buffer
 * \param[in]       usLen           Length of packet
 *
 * \brief   Writes UCC packet to ring buffer. The packet transmission is
 *          performed by the cyclic function SICE_UCC_Cycle(). The function is
 *          non-blocking.
 *
 * \attention   Not tested yet!
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:        No error
 *          - SICE_SYSTEM_ERROR:    Writing packet not possible, e.g. due to
 *                                  buffer overrun
 *          - SICE_CONFIG_ERROR:    UCC not enabled
 *          - SICE_PARAMETER_ERROR: For function parameter error
 *
 * \details This function is non-blocking. In case more packets are received
 *          than the ring buffer can hold, the oldest packets are overwritten
 *          (= dropped).
 *
 * \author  GMy
 *
 * \ingroup SICE_UCC
 *
 * \date    2015-05-19
 */
SICE_FUNC_RET SICE_UCC_PutPacket
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      UCHAR ucPort,
      UCHAR* pucFrame,
      USHORT usLen
    )
{
#ifdef SICE_UC_CHANNEL

  SICE_VERBOSE(3, "SICE_PutUCCPacket()\n");

  if (
      (prSiceInstance == NULL)  ||
      (pucFrame == NULL)        ||
      (usLen == 0)
    )
  {
    return(SICE_PARAMETER_ERROR);
  }

  // Copy packet data to buffer
  memcpy
      (
        prSiceInstance->rUCCTxBuf.aucBuf[prSiceInstance->rUCCTxBuf.ucBufPtrEnd],
        pucFrame,
        usLen
      );

  prSiceInstance->rUCCTxBuf.ucBufPort[prSiceInstance->rUCCTxBuf.ucBufPtrEnd] = ucPort;

  prSiceInstance->rUCCTxBuf.usBufSize[prSiceInstance->rUCCTxBuf.ucBufPtrEnd] = usLen;

  // Increase ring buffer pointer
  prSiceInstance->rUCCTxBuf.ucBufPtrEnd =
      (prSiceInstance->rUCCTxBuf.ucBufPtrEnd + 1) % SICE_UCC_BUF_SIZE;

  // Ring buffer overflow?
  if (prSiceInstance->rUCCTxBuf.ucBufPtrEnd == prSiceInstance->rUCCTxBuf.ucBufPtrStart)
  {
    SICE_VERBOSE(0, "Error: UCC TX Buffer overflow!");

    // Overwrite (= drop) oldest packet
    prSiceInstance->rUCCTxBuf.ucBufPtrStart =
        (prSiceInstance->rUCCTxBuf.ucBufPtrStart + 1) % SICE_UCC_BUF_SIZE;

    return(SICE_BUFFER_ERROR);
  }

  return(SICE_NO_ERROR);

#else

  SICE_VERBOSE(3, "Warning: SICE_PutUCCPacket() called, but UCC disabled\n");
  return (SICE_CONFIG_ERROR);

#endif
}

/**
 * \fn SICE_FUNC_RET SICE_UCC_GetPacket(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              UCHAR* pucPort,
 *              UCHAR* pucFrame,
 *              USHORT* pusLen
 *          )
 *
 * \public
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 * \param[out]      pucPort         Port index (SICE_ETH_PORT_P or
 *                                  SICE_ETH_PORT_S)
 * \param[out]      pucFrame        Pointer to packet buffer. Needs to be able
 *                                  to hold entire Ethernet frame
 * \param[out]      pusLen          Length of packet
 *
 * \brief   Reads single UCC packet from ring buffer, if at least one is
 *          present. The function is non-blocking.
 *
 * \attention   Not tested yet!
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:        No error
 *          - SICE_NO_PACKET:       No packet in buffer
 *          - SICE_SYSTEM_ERROR:    Reading packet not possible
 *          - SICE_CONFIG_ERROR:    UCC not enabled
 *          - SICE_PARAMETER_ERROR: For function parameter error
 *
 * \details This function is non-blocking.
 *
 * \author  GMy
 *
 * \ingroup SICE_UCC
 *
 * \date    2015-05-19
 */
SICE_FUNC_RET SICE_UCC_GetPacket
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      UCHAR* pucPort,
      UCHAR* pucFrame,
      USHORT* pusLen
    )
{
#ifdef SICE_UC_CHANNEL

  SICE_VERBOSE(3, "SICE_GetUCCPacket()\n");

  if (
      (prSiceInstance == NULL)  ||
      (pucFrame == NULL)        ||
      (pucPort == NULL)         ||
      (pusLen == NULL)
    )
  {
    return(SICE_PARAMETER_ERROR);
  }

  if (prSiceInstance->rUCCRxBuf.ucBufPtrStart != prSiceInstance->rUCCRxBuf.ucBufPtrEnd)
  {
    // Found packet(s) in ring buffer, so copy first one
    memcpy
        (
          pucFrame,
          prSiceInstance->rUCCRxBuf.aucBuf[prSiceInstance->rUCCRxBuf.ucBufPtrStart],
          prSiceInstance->rUCCRxBuf.usBufSize[prSiceInstance->rUCCRxBuf.ucBufPtrStart]
        );
    *pusLen = prSiceInstance->rUCCRxBuf.usBufSize[prSiceInstance->rUCCRxBuf.ucBufPtrStart];
    *pucPort = prSiceInstance->rUCCRxBuf.ucBufPort[prSiceInstance->rUCCRxBuf.ucBufPtrStart];

    // Increase ring buffer pointer
    prSiceInstance->rUCCRxBuf.ucBufPtrStart = (prSiceInstance->rUCCRxBuf.ucBufPtrStart + 1) % SICE_UCC_BUF_SIZE;

    return(SICE_NO_ERROR);
  }
  else
  {
    return(SICE_NO_PACKET);
  }

#else

  SICE_VERBOSE(3, "Warning: SICE_GetUCCPacket() called, but UCC disabled\n");
  return (SICE_CONFIG_ERROR);

#endif
}

/**
 * \fn ULONG SICE_UCC_GetTxQueueSize(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   Returns current size of UCC transmit queue (number of packets)
 *
 * \return  - Current UCC transmit queue size (number of packets).
 *          - 0 for error or empty queue
 *
 * \author  GMy
 *
 * \ingroup SICE_UCC
 *
 * \date    2015-05-19
 */
ULONG SICE_UCC_GetTxQueueSize
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
#ifdef SICE_UC_CHANNEL
  if (prSiceInstance == NULL)
  {
    return(0);
  }

  if (prSiceInstance->rUCCTxBuf.ucBufPtrEnd >= prSiceInstance->rUCCTxBuf.ucBufPtrStart)
  {
    return(prSiceInstance->rUCCTxBuf.ucBufPtrEnd - prSiceInstance->rUCCTxBuf.ucBufPtrStart);
  }
  else
  {
    return(prSiceInstance->rUCCTxBuf.ucBufPtrEnd + SICE_UCC_BUF_SIZE - prSiceInstance->rUCCTxBuf.ucBufPtrStart);
  }
#else

  SICE_VERBOSE(3, "Warning: SICE_GetUCCTxQueueSize() called, but UCC disabled\n");
  return (0);

#endif
}

/**
 * \fn ULONG SICE_UCC_GetTxNextPacketSize(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   Returns size of next packet to send.
 *
 * \return  Size of next packet to send (Bytes). 0 in case of no packet in
 *          queue or error.
 *
 * \author  GMy
 *
 * \ingroup SICE_UCC
 *
 * \date    2015-05-19
 */
ULONG SICE_UCC_GetTxNextPacketSize
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
#ifdef SICE_UC_CHANNEL
  ULONG ulPacketSize = 0;

  if (prSiceInstance == NULL)
  {
    return(0);
  }

  if (SICE_UCC_GetTxQueueSize(prSiceInstance) == 0)
  {
    return(0);
  }
  else
  {
    ulPacketSize = prSiceInstance->rUCCTxBuf.usBufSize[prSiceInstance->rUCCTxBuf.ucBufPtrStart];
    return(ulPacketSize);
  }

#else

  SICE_VERBOSE(3, "Warning: SICE_GetUCCTxNextPacketSize() called, but UCC disabled\n");
  return (0);

#endif
}
