/*
 * Sercos Soft Master Core Library
 * Version: see SICE_GLOB.h
 * Copyright (C) 2012 - 2016 Bosch Rexroth AG
 *
 * ATTENTION: This file for NIC-timed transmission support functionality is not
 * part of an official release of SICE yet, but is in prototype status just for
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
 * \file      SICE_NIC_TIMED.c
 *
 * \brief     Sercos SoftMaster core: NIC-timed packet transmission support
 *
 * \attention Only for testing in safe environments, not thoroughly tested and
 *            not officially released yet!
 *
 * \ingroup   SICE_NIC_TIMED
 *
 * \author    GMy
 *
 * \date      2015-07-14
 *
 * \copyright Copyright Bosch Rexroth AG, 2012-2016
 *
 * \version 2015-11-06 (GMy): Defdb00180480: Code optimization, re-named file
 * \version 2015-11-18 (GMy): Added preliminary redundancy support for
 *                            NIC-timed transmission
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

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SICE_FUNC_RET SICE_SendTelegramsNicTimed(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   Transmit Sercos telegrams using the NIC-timed transmission mode
 *
 * \note    Function may only be called if SICE_USE_NIC_TIMED_TX is defined.
 *
 * \attention Prototype status! Not thoroughly tested yet!
 *
 * \details This function fills a data structure of the type
 *          RTOS_NIC_TIMED_PACKET_STRUCT with the packet as well as control
 *          data. Afterwards, the abstract RTOS function
 *          RTOS_TxPacketsNicTimed() is called to perform the packet
 *          transmission.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:        No error
 *          - SICE_SOCKET_ERROR:    When a problem occurred at packet
 *                                  transmission
 *          - SICE_PARAMETER_ERROR: For function parameter error
 *          - SICE_CONFIG_ERROR:    Error in event configuration
 *          - SICE_SYSTEM_ERROR:    Function must not be used in case
 *                                  SICE_USE_NIC_TIMED_TX is not defined
 *
 * \author  GMy
 *
 * \ingroup SICE_NIC_TIMED
 *
 * \date    2013-08-22
 *
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 * \version 2015-07-14 (GMy): Defdb00180480: Code optimization, added
 *                            preliminary UCC support in NIC-timed transmission
 *                            mode
 */
SICE_FUNC_RET SICE_SendTelegramsNicTimed
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
#ifdef SICE_USE_NIC_TIMED_TX
  INT                           iRet                = 0;
  INT                           iCnt                = 0;
  USHORT                        usPacketIdx         = 0;
  USHORT                        usIFG               = CSMD_HAL_TXIFG_BASE;  // Base value for IFG
  USHORT                        usReqIFG            = 0;
  RTOS_NIC_TIMED_PACKET_STRUCT  rNicTimedPacketStruct;

  SICE_VERBOSE(3, "SICE_SendTelegramsNicTimed()\n");

  if (prSiceInstance == NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  SICE_VERBOSE
      (
        2,
        "NIC-timed transmission times: MDT: %dns; AT: %dns; UCC: %dns\n",
        SICE_GetEventTime(prSiceInstance, CSMD_EVENT_START_MDT),
        SICE_GetEventTime(prSiceInstance, CSMD_EVENT_START_AT),
        SICE_GetEventTime(prSiceInstance, CSMD_EVENT_IP_CHANNEL_TX_OPEN)
      );

  // Take over inter frame gap from CoSeMa if larger than minimum value
  usReqIFG = (USHORT) prSiceInstance->prReg->ulIFG & ((ULONG) SICE_IFG_REG_MASK);
  if (usReqIFG > (USHORT) CSMD_HAL_TXIFG_BASE)
  {
    usIFG = usReqIFG;
  }

  // Fill packet transmit data structure

#ifdef SICE_REDUNDANCY
  rNicTimedPacketStruct.boRedundancy = TRUE;
#else
  rNicTimedPacketStruct.boRedundancy = FALSE;
#endif

  // MDTs
  rNicTimedPacketStruct.rMDT.ulOffsetNs =
      SICE_GetEventTime(prSiceInstance, CSMD_EVENT_START_MDT);

  rNicTimedPacketStruct.rMDT.usNum = 0;

  iCnt = 0;

  while (prSiceInstance->aprSendFrame[iCnt]->boEnable)
  {
    rNicTimedPacketStruct.rMDT.usNum++;
    rNicTimedPacketStruct.rMDT.aapucPacket[iCnt][0] =
        prSiceInstance->aprSendFrame[iCnt]->aucData;
    rNicTimedPacketStruct.rMDT.ausLen[iCnt] =
        prSiceInstance->aprSendFrame[usPacketIdx]->usLen;

    // In case of redundancy is used, transmit also packets to the other port
    if (SICE_REDUNDANCY_BOOL)
    {
      rNicTimedPacketStruct.rMDT.aapucPacket[iCnt][1] =
          prSiceInstance->aprSendFrame[iCnt + 2 * CSMD_MAX_TEL]->aucData;
    }

    iCnt++;
  }

  // ATs
  rNicTimedPacketStruct.rAT.ulOffsetNs = SICE_GetEventTime(prSiceInstance, CSMD_EVENT_START_AT);
  rNicTimedPacketStruct.rAT.usNum = 0;

  iCnt = 0;

  while (prSiceInstance->aprSendFrame[iCnt + CSMD_MAX_TEL]->boEnable)
  {
    rNicTimedPacketStruct.rAT.usNum++;
    rNicTimedPacketStruct.rAT.aapucPacket[iCnt][0] =
        prSiceInstance->aprSendFrame[iCnt + CSMD_MAX_TEL]->aucData;
    rNicTimedPacketStruct.rAT.ausLen[iCnt] =
        prSiceInstance->aprSendFrame[usPacketIdx + CSMD_MAX_TEL]->usLen;

    // In case of redundancy is used, transmit also packets to the other port
    if (SICE_REDUNDANCY_BOOL)
    {
      rNicTimedPacketStruct.rAT.aapucPacket[iCnt][1] =
          prSiceInstance->aprSendFrame[iCnt + 3* CSMD_MAX_TEL]->aucData;
    }

    iCnt++;
  }

  if  (rNicTimedPacketStruct.rMDT.ulOffsetNs == rNicTimedPacketStruct.rAT.ulOffsetNs)
  {
    SICE_VERBOSE(0, "Error in timing events\n");
    return(SICE_CONFIG_ERROR);
  }

#ifdef SICE_MEASURE_TIMING
  // De-activate AT transmission
  rNicTimedPacketStruct.rAT.usNum = 0;
#endif

  // Initialize number of UCC packets to 0
  rNicTimedPacketStruct.rUCC.usNum = 0;

#ifdef SICE_UC_CHANNEL
  {
    // UCC packets
    ULONG ulRemUCCDuration = 0;
    ULONG ulUCCStart       = 0;
    ULONG ulUCCEnd         = 0;

    // Transmit UCC packets
    if (SICE_UCC_GetTxQueueSize(prSiceInstance) > 0)
    {
      ulUCCStart = SICE_GetEventTime(prSiceInstance, CSMD_EVENT_IP_CHANNEL_TX_OPEN);
      ulUCCEnd   = SICE_GetEventTime(prSiceInstance, CSMD_EVENT_IP_CHANNEL_TX_CLOSE);
      ulRemUCCDuration = ulUCCEnd - ulUCCStart;

      rNicTimedPacketStruct.rUCC.ulOffsetNs = ulUCCStart;

      // Check whether packets to transmit are left, whether there is enough time
      // for the next packet within the remaining UCC interval as well as the
      // maximum number of UCC packets per cycle is not exceeded

      while (
              (SICE_UCC_GetTxQueueSize(prSiceInstance) > 0)           &&
              (SICE_CalcPacketDuration
                   (
                     prSiceInstance,
                     SICE_UCC_GetTxNextPacketSize(prSiceInstance)
                    ) <= ulRemUCCDuration
               )                                                      &&
               (rNicTimedPacketStruct.rUCC.usNum < RTOS_UCC_MAX_PACKETS)
            )
      {
        // Calculate new remaining time of UCC interval
        ulRemUCCDuration -= SICE_CalcPacketDuration
            (
              prSiceInstance,
              SICE_UCC_GetTxNextPacketSize(prSiceInstance)
            );

        rNicTimedPacketStruct.rUCC.ausLen[rNicTimedPacketStruct.rUCC.usNum] =
            prSiceInstance->rUCCTxBuf.usBufSize[prSiceInstance->rUCCTxBuf.ucBufPtrStart];

        // Todo: in non-redundancy mode signal error message in case port 1 is used
        rNicTimedPacketStruct.rUCC.aucPort[rNicTimedPacketStruct.rUCC.usNum] =
            prSiceInstance->rUCCTxBuf.ucBufPort[prSiceInstance->rUCCTxBuf.ucBufPtrStart];

        rNicTimedPacketStruct.rUCC.apucPacket[rNicTimedPacketStruct.rUCC.usNum] =
            prSiceInstance->rUCCTxBuf.aucBuf[prSiceInstance->rUCCTxBuf.ucBufPtrStart];

        rNicTimedPacketStruct.rUCC.usNum++;

        // ToDo: Blocking needed to avoid overwriting buffer by new packets?
        prSiceInstance->rUCCTxBuf.ucBufPtrStart =
            (prSiceInstance->rUCCTxBuf.ucBufPtrStart +1) % SICE_UCC_BUF_SIZE;

      }
    }
  }
#endif

  // Transmit packets in NIC-timed transmission mode using RTOS function
  iRet = RTOS_TxPacketsNicTimed
      (
        prSiceInstance->iInstanceNo,    // Instance number
        &rNicTimedPacketStruct,         // Packet structure
        usIFG                           // Inter-frame gap
      );

  if (iRet != RTOS_RET_OK)
  {
    SICE_VERBOSE(0, "Error transmitting packets\n");

    if (SICE_REDUNDANCY_BOOL == FALSE)
    {
      return(SICE_SOCKET_ERROR);
    }

  }
  else
  {
    SICE_VERBOSE(2, "NIC-timed packet transmission OK\n");

    (VOID)SICE_IncPacketCounter
        (
          prSiceInstance,               // SICE instance
          SICE_TX_PACKET_CNT,           // RX
          TRUE,                         // Packet is OK
          0,                            // Port number
          rNicTimedPacketStruct.rMDT.usNum + rNicTimedPacketStruct.rAT.usNum
                                        // Number of packets
        );

    if (SICE_REDUNDANCY_BOOL)
    {
      (VOID)SICE_IncPacketCounter
          (
            prSiceInstance,                             // SICE instance
            SICE_TX_PACKET_CNT,                         // RX
            TRUE,                                       // Packet is OK
            1,                                          // Port number
            rNicTimedPacketStruct.rMDT.usNum + rNicTimedPacketStruct.rAT.usNum
                                                        // Number of packets
          );
    }
  }

  return(SICE_NO_ERROR);

#else
  SICE_VERBOSE
      (
        0,
        "Error: SICE_SendTelegramsNicTimed() may only be called if "
        "SICE_USE_NIC_TIMED_TX is defined.\n"
      );
  return(SICE_SYSTEM_ERROR);
#endif
}

 /**
  * \fn SICE_FUNC_RET SICE_SendUccTelegramsNicTimedNRT(
  *              SICE_INSTANCE_STRUCT *prSiceInstance
  *          )
  *
  * \private
  *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
  *
  * \brief   Transmit UCC telegrams in NRT state in NIC-timed transmission mode
  *
  * \note    Function may only be called if SICE_USE_NIC_TIMED_TX and
  *          SICE_UC_CHANNEL are defined. It should not be called in any other
  *          Sercos phase than NRT.
  *
  * \attention Prototype status! Not thoroughly tested yet!
  *
  * \return  See definition of SICE_FUNC_RET
  *          - SICE_NO_ERROR:        No error
  *          - SICE_SOCKET_ERROR:    When a problem occurred at packet
  *                                  transmission
  *          - SICE_PARAMETER_ERROR: For function parameter error
  *          - SICE_SYSTEM_ERROR:    Function must not be used in case
  *                                  SICE_USE_NIC_TIMED_TX is not defined
  *
  * \author  GMy
  *
  * \ingroup SICE_NIC_TIMED
  *
  * \date    2015-07-16
  */
SICE_FUNC_RET SICE_SendUccTelegramsNicTimedNRT
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
#if defined (SICE_USE_NIC_TIMED_TX) && defined (SICE_UC_CHANNEL)
  INT                           iRet                = 0;
  USHORT                        usIFG               = CSMD_HAL_TXIFG_BASE;  // Base value for IFG
  RTOS_NIC_TIMED_PACKET_STRUCT  rNicTimedPacketStruct;
  ULONG                         ulRemUCCDuration    = prSiceInstance->rUCCConfig.ulUccIntNRT;

  SICE_VERBOSE(3, "SICE_SendUccTelegramsNicTimedNRT()\n");

  if (prSiceInstance == NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  SICE_VERBOSE
      (
        3,
        "UCC interval duration in NRT state: %dus\n",
        ulRemUCCDuration / 1000
      );

  // Fill packet transmit data structure

  // MDTs: none to transmit, as NRT state
  rNicTimedPacketStruct.rMDT.usNum = 0;

  // ATs: none to transmit, as NRT state
  rNicTimedPacketStruct.rAT.usNum = 0;

  // Initialize number of UCC packets to 0
  rNicTimedPacketStruct.rUCC.usNum = 0;

  // Transmit UCC packets
  if (SICE_UCC_GetTxQueueSize(prSiceInstance) > 0)
  {

    // Set UCC timing offset to 0, as in NRT state, the whole cycle time may
    // be used for UCC transmission
    rNicTimedPacketStruct.rUCC.ulOffsetNs = 0;

    // Check whether packets to transmit are left, whether there is enough time
    // for the next packet within the remaining UCC interval as well as the
    // maximum number of UCC packets per cycle is not exceeded

    while (
            (SICE_UCC_GetTxQueueSize(prSiceInstance) > 0)           &&
            (SICE_CalcPacketDuration
                 (
                   prSiceInstance,
                   SICE_UCC_GetTxNextPacketSize(prSiceInstance)
                 ) <= ulRemUCCDuration
             )                                                      &&
             (rNicTimedPacketStruct.rUCC.usNum < RTOS_UCC_MAX_PACKETS)
          )
    {

      // Calculate new remaining time of UCC interval
      ulRemUCCDuration -= SICE_CalcPacketDuration
          (
            prSiceInstance,
            SICE_UCC_GetTxNextPacketSize(prSiceInstance)
          );

      rNicTimedPacketStruct.rUCC.ausLen[rNicTimedPacketStruct.rUCC.usNum] =
          prSiceInstance->rUCCTxBuf.usBufSize[prSiceInstance->rUCCTxBuf.ucBufPtrStart];

      // Todo: in non-redundancy mode signal error message in case port 1 is used
      rNicTimedPacketStruct.rUCC.aucPort[rNicTimedPacketStruct.rUCC.usNum] =
          prSiceInstance->rUCCTxBuf.ucBufPort[prSiceInstance->rUCCTxBuf.ucBufPtrStart];

      rNicTimedPacketStruct.rUCC.apucPacket[rNicTimedPacketStruct.rUCC.usNum] =
          prSiceInstance->rUCCTxBuf.aucBuf[prSiceInstance->rUCCTxBuf.ucBufPtrStart];

      rNicTimedPacketStruct.rUCC.usNum++;

      // ToDo: Blocking needed to avoid overwriting buffer by new packets?
      prSiceInstance->rUCCTxBuf.ucBufPtrStart =
          (prSiceInstance->rUCCTxBuf.ucBufPtrStart +1) % SICE_UCC_BUF_SIZE;

    }
  }

  // Transmit packets in NIC-timed transmission mode using RTOS function
  iRet = RTOS_TxPacketsNicTimed
      (
        prSiceInstance->iInstanceNo,    // Instance number
        &rNicTimedPacketStruct,         // Packet structure
        usIFG                           // Inter-frame gap
      );

  if (iRet != RTOS_RET_OK)
  {
    SICE_VERBOSE(0, "Error transmitting packets\n");
    return(SICE_SOCKET_ERROR);
  }
  else
  {
    SICE_VERBOSE(2, "NIC-timed packet transmission OK\n");
  }
  return(SICE_NO_ERROR);

#else
  SICE_VERBOSE
      (
        0,
        "Error: SICE_SendUCCTelegramsNicTimed() may only be called if "
        "SICE_USE_NIC_TIMED_TX and SICE_UC_CHANNEL are defined.\n"
      );
  return(SICE_SYSTEM_ERROR);
#endif
}
