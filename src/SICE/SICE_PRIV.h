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
 * \file      SICE_PRIV.h
 *
 * \brief     Private header for Sercos SoftMaster core
 *
 * \ingroup   SICE
 *
 * \author    GMy
 *
 * \copyright Copyright Bosch Rexroth AG, 2012-2016
 *
 * \date      14.11.2012
 *
 * \version 2013-01-11 (GMy): Added support for Windows CE
 * \version 2013-02-28 (GMy): Lint code optimization
 * \version 2013-09-03 (GMy): Added preliminary support of NIC-timed
 *                            transmission mode
 * \version 2015-05-19 (GMy): Added preliminary support of UCC
 * \version 2015-11-03 (GMy): Defdb00180480: Code optimization
 */

// avoid multiple inclusions - open

#ifndef _SICE_PRIV
#define _SICE_PRIV

#undef SOURCE
#ifdef SOURCE_SICE
    #define SOURCE
#else
    #define SOURCE extern
#endif

//---- includes ---------------------------------------------------------------

/*lint -save -w0 */
#include <string.h>             // for memset
#include <stdio.h>
/*lint -restore */

#include "../SICE/SICE_USER.h"

#ifdef __qnx__
#include "../RTQX/RTQX_GLOB.h"
#include "../RTQX/RTQX_S3SM_GLOB.h"
#elif defined __unix__
#include "../RTLX/RTLX_GLOB.h"
#include "../RTLX/RTLX_S3SM_GLOB.h"
#elif defined WINCE7
#include "../RTC7/RTC7_GLOB.h"
#include "../RTC7/RTC7_S3SM_GLOB.h"
#elif defined WINCE
#include "../RTC6/RTC6_GLOB.h"
#include "../RTC6/RTC6_S3SM_GLOB.h"
#elif defined __INTIME__
#include "../RTIT/RTIT_GLOB.h"
#include "../RTIT/RTIT_S3SM_GLOB.h"
#elif defined __RTX__
#include "../RTRX/RTRX_GLOB.h"
#include "../RTRX/RTRX_S3SM_GLOB.h"
#elif defined __VXWORKS__
#include "../RTVW/RTVW_GLOB.h"
#include "../RTVW/RTVW_S3SM_GLOB.h"
#elif defined __KITHARA__
#include "../RTKT/RTKT_GLOB.h"
#include "../RTKT/RTKT_S3SM_GLOB.h"
#elif defined WIN32
#include "../RTWI/RTWI_GLOB.h"
#include "../RTWI/RTWI_S3SM_GLOB.h"
#elif defined WIN64
#include "../RTWI/RTWI_GLOB.h"
#include "../RTWI/RTWI_S3SM_GLOB.h"
#else
#error Operating system not supported by SICE!
#endif

//---- defines ----------------------------------------------------------------

/**
\def    SICE_EMUL_IP_CORE_VER

\brief  Defines the encoded version of emulated IP core to be handed to CoSeMa.
*/
#if SICE_STR_TYPE=='V'
    #define SICE_EMUL_IP_CORE_VER   ((SICE_EMUL_IP_CORE_V << CSMD_HAL_FPGA_IDR_VERSION_SHIFT) + \
                                     (SICE_EMUL_IP_CORE_R << CSMD_HAL_FPGA_IDR_RELEASE_SHIFT) + \
                                     (0                   << CSMD_HAL_FPGA_IDR_TEST_SHIFT))
#else
    #define SICE_EMUL_IP_CORE_VER   ((SICE_EMUL_IP_CORE_V << CSMD_HAL_FPGA_IDR_VERSION_SHIFT) + \
                                     (SICE_EMUL_IP_CORE_R << CSMD_HAL_FPGA_IDR_RELEASE_SHIFT) + \
                                     (1                   << CSMD_HAL_FPGA_IDR_TEST_SHIFT))
#endif

/**
 * \def     SICE_VERBOSE(_debuglevel, ...)
 *
 * \brief   Macro for debug outputs that depend on the selected verbose level.
 */
#define SICE_VERBOSE(_debuglevel, ...)  /*lint -save -e774 -e506 */             \
                                        if (_debuglevel <= SICE_VERBOSE_LEVEL)  \
                                        {                                       \
                                            RTOS_PrintF("SICE: ");              \
                                            RTOS_PrintF(__VA_ARGS__);           \
                                        }                                       \
                                        /*lint -restore */

#define SICE_RX_PACKET_CNT  (0)
#define SICE_TX_PACKET_CNT  (1)
#define SICE_RX_UCC_CNT     (2)
#define SICE_TX_UCC_CNT     (3)
#define SICE_RX_UCC_DISC    (4)
#define SICE_TX_UCC_DISC    (5)


// Check for configuration inconsistencies

#ifndef CSMD_SOFT_MASTER
#error SICE_PRIV.h: Soft master mode in CoSeMa is not enabled               \
    (CSMD_SOFT_MASTER). This may also be an indicator for an unsupported    \
    CoSeMa version (has to be at least 5V3.11).
#endif

#if ((CSMD_MAX_TX_BUFFER > 1) || (CSMD_MAX_RX_BUFFER > 1)) //only single buffering supported
#error SICE_PRIV.h: Sercos master IP core emulation only supports single \
    buffering (CSMD_MAX_TX_BUFFER, CSMD_MAX_RX_BUFFER).
#endif

#if (CSMD_MAX_HW_CONTAINER != 0) // Only soft SVCs supported
#error SICE_PRIV.h: Sercos master IP core emulation only supports soft SVCs \
    (CSMD_MAX_HW_CONTAINER).
#endif

#ifdef CSMD_BIG_ENDIAN
#error SICE_PRIV.h: Big endian on Sercos master IP core emulation not tested \
    yet (CSMD_BIG_ENDIAN).
#endif

#if (defined CSMD_PCI_MASTER) || (defined CSMD_PCI_MASTER_EVENT_DMA)
#error SICE_PRIV.h: Using DMA with soft master does not make sense \
    (CSMD_PCI_MASTER, CSMD_PCI_MASTER_EVENT_DMA).
#endif

#ifdef SICE_OPT_UCC_CP1_2
#ifndef CSMD_SWC_EXT
#error SICE_PRIV.h: SWC (CSMD_SWC_EXT) has to be enabled to support setting \
    SICE_OPT_UCC_CP1_2.
#endif
#if (CSMD_DRV_VERSION <=5)
#ifndef CSMD_IP_CHANNEL
#error SICE_PRIV.h: UCC (CSMD_IP_CHANNEL) has to be enabled to support \
    setting SICE_OPT_UCC_CP1_2.
#endif
#endif
#endif

#ifdef SICE_UC_CHANNEL
#if (CSMD_DRV_VERSION <=5)
    #ifndef CSMD_IP_CHANNEL
        #error SICE_PRIV.h: (CSMD_IP_CHANNEL) has to be enabled to use SICE_UC_CHANNEL
    #endif
#endif
#endif

#ifdef SICE_USE_NIC_TIMED_TX
    #if (RTOS_TIMING_MODE != RTOS_TIMING_NIC)
    #error SICE_PRIV.h: For SICE_USE_NIC_TIMED_TX, the RTOS timing mode   \
            RTOS_TIMING_NIC and RTOS_USE_NIC_TIMED_TX have to be used.
    #endif
    #ifndef RTOS_USE_NIC_TIMED_TX
    #error SICE_PRIV.h: For SICE_USE_NIC_TIMED_TX, the RTOS timing mode   \
            RTOS_TIMING_NIC and RTOS_USE_NIC_TIMED_TX have to be used.
    #endif
#endif

// Constants for Sercos header

#define SICE_TEL_CP_MASK            (0x0F)              /**< Bit in phase field to signal communication phase */
#define SICE_TEL_CP_SWITCH_MASK     (0x80)              /**< Bit in phase field to signal CP switch */

#define SICE_TEL_NO_MASK            (0x03)              /**< Field in Sercos type field to signal telegram number */

#define SICE_TEL_TYPE_MASK          (0x40)              /**< Telegram type bit in Sercos type field */
#define SICE_TEL_TYPE_MDT           (0x00)              /**< Telegram type for MDT in Sercos type field */
#define SICE_TEL_TYPE_AT            (0x40)              /**< Telegram type for AT in Sercos type field */
#define SICE_TEL_TYPE_SHIFT         (0x06)              /**< Shift offset for telegram type in Sercos type field */

#define SICE_TEL_CHANNEL_MASK       (0x80)              /**< Telegram channel mask */
#define SICE_TEL_P_CHANNEL          (0x00)              /**< Telegram channel for primary port */
#define SICE_TEL_S_CHANNEL          (0x80)              /**< Telegram channel for secondary port */

#define SICE_TEL_CYCLE_CNT          (0x20)              /**< Bit in Sercos type field for enabling cycle counter */

#define SICE_TEL_LENGTH_HDR_FOR_CRC (0x10)              /**< Length of Sercos packet header for CRC calculation */
#define SICE_TEL_LENGTH_DYN_HDR_FOR_CRC (0x02)          /**< Length of 'dynamic' part of Sercos packet header for
                                                             CRC calculation (Sercos type and phase fields) */

#define SICE_TEL_EXT_FIELD_OFFSET   (0x08)              /**< Offset of extended field in Sercos packet payload */

// Constants for Sercos master IP Core that are not included in CoSeMa

#define SICE_PHASECR_CYCLE_COUNT_MOD    (8)             /**< Bit mask for cycle counter flag in Sercos type field */
#define SICE_PHASECR_CYCLE_COUNT_SHIFT  (4)             /**< Shift mask for cycle counter */
#define SICE_PHASECR_ALL_MASK       (0x000000FF)        /**< Mask for phase including transition bit and cycle counter */

#define SICE_SVCCSR_ENABLE_MASK     (0x00000001)        /**< Mask for HW SVC enable bit in SVCCSR register */

#define SICE_TEL_DESC_IDX_MDT       (0x00)              /**< Telegram type for MDT in telegram lists, e.g. IP core descriptors */
#define SICE_TEL_DESC_IDX_AT        (0x04)              /**< Telegram type for AT in telegram lists, e.g. IP core descriptors */
#define SICE_TEL_DESC_IDX_SHIFT     (0x02)              /**< Telegram type shift in telegram lists, e.g. IP core descriptors */
#define SICE_TEL_DESC_OFFSET_MASK   (0x00003FFC)        /**< Mask for RX/TX descriptor offsets */
#define SICE_TEL_DESC_ENABLE_MASK   (0x00000001)        /**< Mask for enabling RX/TX descriptors for the current packet */

#define SICE_IFG_REG_MASK           (0x000003FF)        /**< Mask for IFG in IP core IFG register */

#define SICE_STRBR_LSW_MASK         (0x0000FFFF)        /**< Mask for TCNT LSW in STRBR register */
#define SICE_STRBR_HSW_MASK         (0x03FF0000)        /**< Mask for TCNT LSW in STRBR register */

#define SICE_CRC_POLY               (0xEDB88320)        /**< Generator polynom of CRC32 calculation */
#define SICE_CRC_TABLE_SIZE         (256)               /**< Size of pre-calculated CRC32 table */

#define SICE_SERC3_TEL_HEADER       (20)                /**< Size of Sercos 3 telegram header */
#define SICE_SERC3_MAX_DATA_LENGTH  (1494)              /**< Maximum size of Sercos 3 telegram data length */

#define SICE_SERC3_MAC_ADR_SIZE     (6)                 /**< Length of Ethernet MAC address */

//---- type definitions -------------------------------------------------------

/**
 * \union   SICE_SIII_FRAME
 *
 * \brief   Union for Sercos III frame to access the data both raw and as
 *          structured fields.
*/
typedef union
{
  struct
  {
    UCHAR   aucDestMAC[SICE_SERC3_MAC_ADR_SIZE];        /**< Destination MAC address */
    UCHAR   aucSrcMAC[SICE_SERC3_MAC_ADR_SIZE];         /**< Source MAC address */
    USHORT  usPortID;                                   /**< ID of Ethernet port */
    UCHAR   ucSercosType;                               /**< Sercos type field */
    UCHAR   ucPhase;                                    /**< Sercos phase field */
    ULONG   ulCRC;                                      /**< Sercos header CRC */
    UCHAR   aucData[SICE_SERC3_MAX_DATA_LENGTH];        /**< Packet payload data */
  } rTel;                                               /**< Structured field data */
  UCHAR aucRaw[SICE_SERC3_MAX_DATA_LENGTH + SICE_SERC3_TEL_HEADER];
                                                        /**< Raw packet data */
} SICE_SIII_FRAME;

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// SICE_INIT.c

SOURCE SICE_FUNC_RET SICE_SoftReset
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

// SICE_SIII.c

SOURCE SICE_FUNC_RET SICE_CRC32BuildTable
    (
      VOID
    );

SOURCE ULONG SICE_CRC32Calc
    (
      UCHAR *pucBuffer,
      INT iSize,
      ULONG ulStartCRC
    );

SOURCE SICE_FUNC_RET SICE_CheckCycleTime
    (
      ULONG ulNewCycleTime,
      UCHAR ucPhase
    );

// SICE_TX.c

SOURCE SICE_FUNC_RET SICE_PrepareTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE BOOL SICE_CheckPacketSfcrTxEnabled
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      USHORT usPacketIdx
    );

SOURCE UCHAR SICE_SercosTypeField
    (
      USHORT usPacketIdx,
      UCHAR ucChannel
    );

SOURCE SICE_FUNC_RET SICE_CopyTxData
    (
      SICE_INSTANCE_STRUCT* prSiceInstance,
      SICE_SIII_FRAME* puSercosFrameP1,
      SICE_SIII_FRAME* puSercosFrameP2,
      UCHAR ucDescType,
      UCHAR ucBufSel,
      USHORT usLastBufOffset,
      USHORT usLastFrameOffset,
      USHORT usFrameOffset
    );


SOURCE SICE_FUNC_RET SICE_SendMDTTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE SICE_FUNC_RET SICE_SendATTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE SICE_FUNC_RET SICE_CheckPreCondsSend
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE SICE_FUNC_RET SICE_GenExtField
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      USHORT *pusExtField,
      UCHAR ucPhaseAll,
      USHORT usPacketIdx
    );

SOURCE SICE_FUNC_RET SICE_SendTelegramsNicTimed
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE SICE_FUNC_RET SICE_SendUccTelegramsNicTimedNRT
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

// SICE_RX.c

SOURCE SICE_FUNC_RET SICE_ReceiveTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE SICE_FUNC_RET SICE_CheckPreCondsReceive
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE INT SICE_CalcNoOfSlaves
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

// SICE_UTIL.c

SOURCE ULONG SICE_GetEventTime
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      USHORT usEventType
    );

SOURCE SICE_FUNC_RET SICE_CheckTimingMethod
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE SICE_FUNC_RET SICE_CalcRingDelay
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

#ifdef CSMD_HW_WATCHDOG
SOURCE SICE_FUNC_RET SICE_UpdateWatchdogStatus
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );
#endif

SOURCE SICE_FUNC_RET SICE_IncPacketCounter
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      INT iRXorTX,
      BOOL boIsOk,
      INT iPort,
      INT iVal
    );

SOURCE ULONG SICE_CalcPacketDuration
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      USHORT usPacketLen
    );

// SICE_UCC.c

SOURCE SICE_FUNC_RET SICE_UCC_Init
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      SICE_UCC_CONFIG_STRUCT *prUCCConfig,
      UCHAR* pucMAC
    );

SOURCE SICE_FUNC_RET SICE_UCC_Close
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE SICE_FUNC_RET SICE_UCC_SendTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      ULONG ulUCCDuration
    );

SOURCE SICE_FUNC_RET SICE_UCC_ReceiveTelegrams
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      BOOL boNrtState
    );

SOURCE SICE_FUNC_RET SICE_UCC_Reset
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE ULONG SICE_UCC_GetTxQueueSize
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE ULONG SICE_UCC_GetTxNextPacketSize
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

#ifdef __cplusplus
}
#endif

// avoid multiple inclusions - close

#endif
