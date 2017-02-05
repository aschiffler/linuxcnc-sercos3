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
 * \file      SICE_GLOB.h
 *
 * \brief     Global header for software emulation of Sercos SoftMaster core
 *
 * \ingroup   SICE
 *
 * \author    GMy, based on earlier work by SBe
 *
 * \copyright Copyright Bosch Rexroth AG, 2012-2016
 *
 * \version 2012-10-11 (GMy): Baseline for Sercos master IP core v3
 * \version 2012-10-31 (GMy): Updated for Sercos master IP core v4
 * \version 2013-01-11 (GMy): Added support for Windows CE
 * \version 2013-02-28 (GMy): Lint code optimization
 * \version 2013-05-07 (GMy): Added support for INtime
 * \version 2013-08-22 (GMy): Added support of NIC-timed transmission mode
 * \version 2015-05-18 (GMy): Added possibility to set offsets in emulation
 *                            memory
 * \version 2015-05-19 (GMy): Added preliminary UCC support
 * \version 2015-11-03 (GMy): Defdb00180480: Code optimization, added
 *                            initialization data structure
 */

// avoid multiple inclusions - open

#ifndef _SICE_GLOB
#define _SICE_GLOB

#undef SOURCE
#ifdef SOURCE_SICE
    #define SOURCE
#else
    #define SOURCE extern
#endif

//---- includes ---------------------------------------------------------------

/*lint -save -w0 */
#include <stdlib.h>
/*lint -restore */

#include "../SICE/SICE_USER.h"
#include "../GLOB/GLOB_TYPE.h"  /* necessary for CoSeMa 5VRS */
#include "../CSMD/CSMD_GLOB.h"
#include "../CSMD/CSMD_DIAG.h"  /* for CSMD_DRV_VERSION */

//---- defines ----------------------------------------------------------------

// Version of Sercos soft master / Sercos IP core emulation

#define SICE_VERSION_MAJOR           (1)      /**< Major version of Sercos IP core emulation */
#define SICE_STR_TYPE                'V'      /**< Type of version: Test / released version */
#define SICE_VERSION_MINOR           (0)      /**< Minor version of Sercos IP core emulation */
#define SICE_RELEASE                 (4)      /**< Release of Sercos IP core emulation */

#define SICE_IDR_SOFT_MASTER         (6)      /**< Identification of soft-master in IDR register */

// Version of emulated IP core

#if (CSMD_DRV_VERSION <=5)
  #define SICE_EMUL_IP_CORE_V        (4)      /**< Version of emulated Sercos master IP core */
  #define SICE_EMUL_IP_CORE_STR_TYPE ('V')    /**< Test version marker */
  #define SICE_EMUL_IP_CORE_R        (11)     /**< Release of emulated Sercos master IP core */
#else
  #define SICE_EMUL_IP_CORE_V        (4)      /**< Version of emulated Sercos master IP core */
  #define SICE_EMUL_IP_CORE_STR_TYPE ('V')    /**< Test version marker */
  #define SICE_EMUL_IP_CORE_R        (11)     /**< Release of emulated Sercos master IP core */
#endif

// Constants for emulated shared memory interface between Sercos IP core and CoSeMa

#define SICE_RAM_REG_SIZE            (4 *1024)               /**< Size of register memory area, twice the size
                                                                 of the area in hard IP core to also hold the
                                                                 event registers at offset 0x400*/
#define SICE_RAM_SVC_SIZE            CSMD_HAL_SVC_RAM_SIZE   /**< Size of SVC memory area*/
#define SICE_RAM_TX_SIZE             CSMD_HAL_TX_RAM_SIZE    /**< Size of TX RAM memory area*/
#define SICE_RAM_RX_SIZE             CSMD_HAL_RX_RAM_SIZE    /**< Size of RX RAM memory area*/
#define SICE_RAM_IP_TX_SIZE          CSMD_HAL_IP_TX_RAM_SIZE /**< Size of IP TX RAM memory area*/
#define SICE_RAM_IP_RX_SIZE          CSMD_HAL_IP_RX_RAM_SIZE /**< Size of IP RX RAM memory area*/

// Watchdog constants

#define SICE_WD_ALARM_NONE           (0)      /**< No watchdog alarm active */
#define SICE_WD_ALARM_SEND_EMPTY_TEL (1)      /**< Watchdog alarm: set packets to zero */
#define SICE_WD_ALARM_DISABLE_TX_TEL (2)      /**< Watchdog alarm: do not send packets */

// Ethernet constants

#define SICE_SIII_ETHER_TYPE         (0x88CD) /**< Sercos III Ethernet type */
#define SICE_ETH_FRAMEBUF_LEN        (1536)   /**< Required Ethernet packet buffer size */

#define SICE_ETH_PORT_P              (0)      /**< Primary Sercos Ethernet port */
#define SICE_ETH_PORT_S              (1)      /**< Secondary Sercos Ethernet port */
#define SICE_ETH_PORT_BOTH           (2)      /**< Both Sercos Ethernet ports */

#ifdef SICE_REDUNDANCY
    #define SICE_REDUNDANCY_VAL      (2)      /**< 2 for redundancy mode, 1 otherwise */
    #define SICE_REDUNDANCY_BOOL     TRUE     /**< TRUE for redundancy mode, FALSE otherwise */
#else
    #define SICE_REDUNDANCY_VAL      (1)      /**< 2 for redundancy mode, 1 otherwise */
    #define SICE_REDUNDANCY_BOOL     FALSE    /**< TRUE for redundancy mode, FALSE otherwise */
#endif

//---- type definitions -------------------------------------------------------

/**
 * \struct  SICE_SIII_PACKET_BUF
 *
 * \brief   Structure for Sercos III packet buffer
*/
typedef struct
{
  USHORT usLen;                               /**< Total length of Sercos packet */
  BOOL   boEnable;                            /**< Is packet enabled? */
  UCHAR  aucData[SICE_ETH_FRAMEBUF_LEN];      /**< Sercos packet data */
} SICE_SIII_PACKET_BUF;

/**
 * \struct  SICE_UCC_PACKET_BUF
 *
 * \brief   Structure for UCC packet ring buffer
*/
typedef struct
{
  UCHAR  aucBuf[SICE_UCC_BUF_SIZE][SICE_ETH_FRAMEBUF_LEN];
                                              /**< UCC buffer */
  USHORT usBufSize[SICE_UCC_BUF_SIZE];        /**< Size of packets in buffer */
  UCHAR  ucBufPort[SICE_UCC_BUF_SIZE];        /**< Port information of packets in buffer */
  UCHAR  ucBufPtrStart;                       /**< Start pointer in ring buffer */
  UCHAR  ucBufPtrEnd;                         /**< End pointer in ring buffer */
} SICE_UCC_PACKET_BUF;

/**
 * \struct  SICE_EMUL_MEM_STRUCT
 *
 * \brief   Memory structure for Sercos IP core emulation
 */
typedef struct
{
  UCHAR  aucRegister  [SICE_RAM_REG_SIZE];    /**< Register memory area */
#if (SICE_RAM_OFFSET_SVC > 0)
  UCHAR  aucOffsetSvc  [SICE_RAM_OFFSET_SVC]; /**< Unused memory area, optional */
#endif
  UCHAR  aucSvc       [SICE_RAM_SVC_SIZE];    /**< SVC memory area */
#if (SICE_RAM_OFFSET_TX > 0)
  UCHAR   aucOffsetTx  [SICE_RAM_OFFSET_TX];  /**< Unused memory area, optional */
#endif
  UCHAR  aucTxRAM     [SICE_RAM_TX_SIZE];     /**< TX memory area */
#if (SICE_RAM_OFFSET_RX > 0)
  UCHAR   aucOffsetRx  [SICE_RAM_OFFSET_RX];  /**< Unused memory area, optional */
#endif
  UCHAR  aucRxRAM     [SICE_RAM_RX_SIZE];     /**< RX memory area */
} SICE_EMUL_MEM_STRUCT;

/**
 * \struct  SICE_UCC_CONFIG_STRUCT
 *
 * \brief   Data structure for configuration of the UC channel
 */
typedef struct
{
  ULONG                       ulUccIntNRT;    /**< UCC interval duration in NRT in ns */
} SICE_UCC_CONFIG_STRUCT;

/**
 * \struct  SICE_INIT_STRUCT
 *
 * \brief   Initialization data structure for SICE instance
*/
typedef struct
{
  INT    iInstanceNo;                         /**< Instance number (index) */
} SICE_INIT_STRUCT;

/**
 * \struct  SICE_INSTANCE_STRUCT
 *
 * \brief   Data structure for instance of Sercos IP core emulation
 */
typedef struct
{
  INT                         iInstanceNo;    /**< Instance number of SICE */
  SICE_EMUL_MEM_STRUCT        rMemory;        /**< Sercos master IP core emulation memory */
  ULONG                       ulBaseCRC;      /**< Base CRC of Sercos packet */
  SICE_SIII_PACKET_BUF*       aprSendFrame[2*CSMD_MAX_TEL*SICE_REDUNDANCY_VAL];
                                              /**< Packet buffer for all Sercos packets of one cycle */
  UCHAR                       aucMyMAC[6];    /**< MAC address for Sercos*/
  UCHAR                       aucUccMAC[6];   /**< MAC address for UCC*/
  CSMD_HAL_SERCFPGA_REGISTER* prReg;          /**< Pointer to Sercos IP core register structure*/
  CSMD_HAL_SVC_RAM*           prSVC_Ram;      /**< Pointer to SVC RAM*/
  CSMD_HAL_TX_RAM*            prTX_Ram;       /**< Pointer to TX RAM*/
  CSMD_HAL_RX_RAM*            prRX_Ram;       /**< Pointer to RX RAM*/
  CSMD_HAL_SERCFPGA_DATTYP    ulTGSR1;        /**< Buffered register TGSR1*/
  CSMD_HAL_SERCFPGA_DATTYP    ulTGSR2;        /**< Buffered register TGSR2*/
  UCHAR                       ucTimingMethod; /**< Current Sercos timing method*/
  USHORT                      usNumRecogDevs; /**< Number of recognized Sercos slaves*/
  UCHAR                       ucWDAlarm;      /**< Watchdog alarm mode: Do not send packets at all */
  CSMD_EVENT*                 prEvents;       /**< TCNT event registers*/
  UCHAR                       ucCycleCnt;     /**< Current Sercos cycle counter value */
  BOOL                        boSercosTimeEn; /**< Sercos time enabled? */
  CSMD_SERCOSTIME             ulLatchedTime;  /**< Sercos time latched for transmission */
#ifdef SICE_UC_CHANNEL
  SICE_UCC_CONFIG_STRUCT      rUCCConfig;     /**< UCC configuration structure */
  SICE_UCC_PACKET_BUF         rUCCRxBuf;      /**< UCC receive ring buffer */
  SICE_UCC_PACKET_BUF         rUCCTxBuf;      /**< UCC transmit ring buffer */
#endif
} SICE_INSTANCE_STRUCT;

/**
 * \enum    SICE_FUNC_RET
 *
 * \brief   Enumeration of SICE error codes.
 */
typedef enum
{
  /* --------------------------------------------------------- */
  /* no error: states, warning codes  (error_class 0x00000nnn) */
  /* --------------------------------------------------------- */
  SICE_NO_ERROR               = (0x00000000), /**< 0x00 Function successfully completed */
  SICE_FUNCTION_IN_PROCESS,                   /**< 0x01 Function processing still active */
  SICE_CHECK_STATUS_FALSE,                    /**< 0x02 Boolean check function return false*/
  SICE_WATCHDOG_ALARM,                        /**< 0x03 Watchdog alarm */
  SICE_TIMING_METHOD_CHANGED,                 /**< 0x04 CoSeMa has changed the Sercos timing method */
  SICE_NOT_READY,                             /**< 0x06 Not ready for something */
  SICE_NO_PACKET,                             /**< 0x07 No packet available to be read */
  //SICE_INVALID_PACKET,                      /**< 0x08 SICE has received a non-Sercos packet */
  SICE_END_ERR_CLASS_00000,                   /**< End marker for error class 0x00000 nnn */
  /* --------------------------------------------------------- */
  /* system error codes               (error_class 0x00110nnn) */
  /* --------------------------------------------------------- */
  SICE_SYSTEM_ERROR           = (0x00110000), /**< 0x00 General: error during function execution */
  SICE_MEM_ERROR,                             /**< 0x01 Error when trying to allocate memory */
  SICE_SOCKET_ERROR,                          /**< 0x02 Socket error: Open, close, read or write problem*/
  SICE_BUFFER_ERROR,                          /**< 0x03 Buffer error: Invalid buffer settings */
  SICE_WATCHDOG_ERROR,                        /**< 0x04 General watchdog error */
  SICE_PARAMETER_ERROR,                       /**< 0x05 Function parameter error */
  SICE_HW_SVC_ERROR,                          /**< 0x06 Hardware SVC not supported by SICE */
  SICE_END_ERR_CLASS_00110,                   /**< End marker for error class 0x00010 nnn */
  /* --------------------------------------------------------- */
  /* Sercos error codes               (error_class 0x00120nnn) */
  /* --------------------------------------------------------- */
  SICE_SERCOS_ERROR           = (0x00120000), /**< 0x00 General: Sercos error */
  SICE_SERCOS_TIMING_MODE_ERROR,              /**< 0x01 Unknown Sercos timing mode */
  SICE_SERCOS_CYCLE_TIME_INVALID,             /**< 0x02 Illegal Sercos cycle time */
  SICE_END_ERR_CLASS_00120,                   /**< End marker for error class 0x00020 nnn */
  /* --------------------------------------------------------- */
  /* Configuration error codes        (error_class 0x00121nnn) */
  /* --------------------------------------------------------- */
  SICE_CONFIG_ERROR           = (0x00121000), /**< 0x00 General: configuration error */
  SICE_RX_DESCRIPTOR_ERROR,                   /**< 0x01 RX descriptors set by CoSeMa not consistent */
  SICE_TX_DESCRIPTOR_ERROR,                   /**< 0x02 TX descriptors set by CoSeMa not consistent */
  SICE_END_ERR_CLASS_00121,                   /**< End marker for error class 0x00021 nnn */
  /* --------------------------------------------------------- */
  /* Redundancy error codes           (error_class 0x00122nnn) */
  /* --------------------------------------------------------- */
  SICE_REDUNDANCY_ERROR       = (0x00122000), /**< 0x00 General: Sercos redundancy error */
  SICE_END_ERR_CLASS_00122,                   /**< End marker for error class 0x00022 nnn */
  /* --------------------------------------------------------- */
  /* Hot Plug error codes             (error_class 0x00123nnn) */
  /* --------------------------------------------------------- */
  SICE_HP_ERROR               = (0x00123000), /**< 0x00 General: Sercos hot-plug error */
  SICE_END_ERR_CLASS_00123,                   /**< End marker for error class 0x00023 nnn */
  /* --------------------------------------------------------- */
  /* UCC error codes                  (error_class 0x00124nnn) */
  /* --------------------------------------------------------- */
  SICE_UCC_ERROR              = (0x00124000), /**< 0x00 General: Sercos UCC error */
  SICE_END_ERR_CLASS_00124                    /**< End marker for error class 0x00024 nnn */

} SICE_FUNC_RET;

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// SICE_INIT.c

SOURCE SICE_FUNC_RET SICE_Init
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      SICE_INIT_STRUCT *prSiceInitStruct
    );

SOURCE SICE_FUNC_RET SICE_Close
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

// SICE_CYCLIC.c

SOURCE SICE_FUNC_RET SICE_Cycle
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      ULONG *pulSICECycleTime
    );

SOURCE SICE_FUNC_RET SICE_Cycle_Prepare
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    );

SOURCE SICE_FUNC_RET SICE_Cycle_Start
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      ULONG *pulSICECycleTime
    );

// SICE_UCC.c

/* ATTENTION: These functions for UCC support functionality are not part of an
 * official release of SICE yet, but are in prototype status just for
 * evaluation. They may only be used in safe environments, not in real
 * machines!*/

SOURCE SICE_FUNC_RET SICE_UCC_Cycle
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      ULONG ulUCCDuration
    );

SOURCE SICE_FUNC_RET SICE_UCC_PutPacket
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      UCHAR ucPort,
      UCHAR* pucFrame,
      USHORT usLen
    );

SOURCE SICE_FUNC_RET SICE_UCC_GetPacket
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      UCHAR* pucPort,
      UCHAR* pucFrame,
      USHORT* pusLen
    );

#ifdef __cplusplus
}
#endif

// avoid multiple inclusions - close

#endif
