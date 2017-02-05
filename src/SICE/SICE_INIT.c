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
 * \file      SICE_INIT.c
 *
 * \brief     Sercos SoftMaster core: Functions for initialization,
 *            de-initialization and reset of SICE.
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
 * \version 2013-02-05 (GMy): Hot-plug support added
 * \version 2013-02-14 (GMy): Ring delay measurement emulation
 * \version 2013-02-28 (GMy): Lint code optimization
 * \version 2013-03-28 (GMy): Code modularization
 * \version 2013-05-07 (GMy): Added support for INtime
 * \version 2013-06-06 (GMy): Added support for RTX and Kithara
 * \version 2013-06-20 (GMy): Optimization of error handling
 * \version 2013-06-21 (GMy): Added support for Windows desktop versions and QNX
 * \version 2014-04-01 (GMy): Added support for CoSeMa 6VRS
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 * \version 2015-11-03 (GMy): Defdb00180480: Code optimization, added
 *                            initialization data structure
 */

//---- includes ---------------------------------------------------------------

#define SOURCE_SICE

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
/*lint -save -w0 */
#include <netinet/in.h>
/*lint -restore */
#elif defined __KITHARA__
#elif defined WIN32
#elif defined WIN64
/*lint -save -w0 */
#include <Winsock2.h>
/*lint -restore */
#else
#error Operating system not supported by SICE!
#endif

//---- defines ----------------------------------------------------------------

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SICE_FUNC_RET SICE_Init(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              SICE_INIT_STRUCT *prSiceInit
 *          )
 *
 * \public
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos IP core emulation
 *                                  instance
 * \param[in]       prSiceInit      Initialization data structure
 *
 * \brief   This function initializes the Sercos IP core emulation.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:        No error
 *          - SICE_SOCKET_ERROR:    When a problem occurred when opening
 *                                  Ethernet transmit and receive sockets
 *          - SICE_MEM_ERROR:       For problems with memory allocation
 *          - SICE_PARAMETER_ERROR: For function parameter error
 *          - SICE_UCC_ERROR:       Error during UCC initialization
 *
 * \details This function initializes the Sercos SoftMaster core.
 *          Transmit and receive sockets are opened. Memory for data structures
 *          is allocated. The CRC table is set up, and the base CRC calculated.
 *          This is done in order to reduce calculation load during cyclic run
 *          of the IP core emulation. Finally, SICE_SoftReset() is called,
 *          mainly in order to re-set the emulated ip core registers.
 *
 * \author  GMy, partially based on earlier work by SBe
 *
 * \ingroup SICE
 *
 * \date 2012-10-11
 *
 * \version 2012-10-11 (GMy): Baseline for Sercos master IP core v3
 * \version 2012-10-31 (GMy): Updated for Sercos master IP core v4
 * \version 2013-08-08 (GMy): Support of MAC address register added, optimized
 *                            initialization
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 * \version 2015-06-11 (GMy): Added preliminary UCC support
 * \version 2015-11-03 (GMy): Defdb00180480: Code optimization, added
 *                            initialization data structure
 * \version 2015-11-03 (GMy): Defdb00180480: Code optimization
 */
SICE_FUNC_RET SICE_Init
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      SICE_INIT_STRUCT *prSiceInit
    )
{
  INT           iCnt;
  INT           iRet;
  SICE_FUNC_RET eSiceRet;

  SICE_VERBOSE(3, "SICE_Init()\n");

  if  (prSiceInstance ==  NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  prSiceInstance->iInstanceNo = prSiceInit->iInstanceNo;

  SICE_VERBOSE(1, "Obtaining transmit socket ...\n");

  iRet = RTOS_OpenTxSocket
      (
        prSiceInstance->iInstanceNo,    // Instance number
        SICE_REDUNDANCY_BOOL,           // Is redundancy used?
        prSiceInstance->aucMyMAC        // Buffer for MAC address
      );

  if (iRet != RTOS_RET_OK)
  {
    SICE_VERBOSE(0, "Error: Could not open Ethernet transmit socket!\n");
    return(SICE_SOCKET_ERROR);
  }
  else
  {
    SICE_VERBOSE
        (
          1,
          "  Done. MAC address: %x:%x:%x:%x:%x:%x\n",
          prSiceInstance->aucMyMAC[0],
          prSiceInstance->aucMyMAC[1],
          prSiceInstance->aucMyMAC[2],
          prSiceInstance->aucMyMAC[3],
          prSiceInstance->aucMyMAC[4],
          prSiceInstance->aucMyMAC[5]
        );
  }

  SICE_VERBOSE(1, "Obtaining receive socket ...\n");
  iRet = RTOS_OpenRxSocket
      (
        prSiceInstance->iInstanceNo,
        SICE_REDUNDANCY_BOOL
      );
  if (iRet != RTOS_RET_OK)
  {
    SICE_VERBOSE(0, "Error: Could not open Ethernet receive socket!\n");
    return(SICE_SOCKET_ERROR);
  }
  else
  {
    SICE_VERBOSE(1, "  Done.\n");
  }

  // Allocate Sercos frames
  for (
      iCnt = 0;
      iCnt < (2*CSMD_MAX_TEL*SICE_REDUNDANCY_VAL);
      iCnt++
    )
  {
    prSiceInstance->aprSendFrame[iCnt] =
        (SICE_SIII_PACKET_BUF*) malloc(sizeof (SICE_SIII_PACKET_BUF));

    if (prSiceInstance->aprSendFrame[iCnt] == NULL)
    {
      return(SICE_MEM_ERROR);
    }
  }

  // Perform soft reset on Sercos IP core
  eSiceRet = SICE_SoftReset(prSiceInstance);
  if (eSiceRet != SICE_NO_ERROR)
  {
    return(eSiceRet);
  }

#ifdef SICE_UC_CHANNEL

  SICE_VERBOSE(1, "Initializing UCC ...\n");

  prSiceInstance->rUCCConfig.ulUccIntNRT = SICE_UCC_INT_NRT;
  prSiceInstance->prReg->ulIFG           = CSMD_HAL_TXIFG_BASE;

      eSiceRet = SICE_UCC_Init
      (
        prSiceInstance,                  // SICE instance structure
        &(prSiceInstance->rUCCConfig),   // UCC configuration structure
        prSiceInstance->aucUccMAC        // Buffer for MAC address
      );
  if (eSiceRet != RTOS_RET_OK)
  {
    SICE_VERBOSE(0, "Error: Could not open UCC!\n");
    return(SICE_UCC_ERROR);
  }
  else
  {
    SICE_VERBOSE(1, "  Done.\n");
  }

#endif
  
  // Prepare CRC32 calculation
  (VOID)SICE_CRC32BuildTable();

  SICE_VERBOSE
      (
        1,
        "Size of SICE instance data structure: %u Bytes\n",
        sizeof(SICE_INSTANCE_STRUCT)
      );


  return(SICE_NO_ERROR);
}

/**
 * \fn SICE_FUNC_RET SICE_SoftReset(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \public
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   Performs soft reset of Sercos SoftMaster core
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:        No error
 *
 * \details The SICE instance variables are set back to default values and the
 *          ip core register values are re-set to their re-set values.
 *
 * \author  GMy, partially based on earlier work by SBe
 *
 * \ingroup SICE
 *
 * \date 2012-10-11
 *
 * \version 2012-10-11 (GMy): Baseline for Sercos master IP core v3
 * \version 2012-10-31 (GMy): Updated for emulation of IP core v4
 * \version 2013-01-31 (GMy): Added Sercos timing method support
 * \version 2013-08-08 (GMy): Support of MAC address register added, optimized
 *                            initialization
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 */
SICE_FUNC_RET SICE_SoftReset
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  INT               iCnt;
  SICE_SIII_FRAME*  puSercosFrame = NULL;

  SICE_VERBOSE(1, "SICE_SoftReset()\n");

  // Initialize memory
  (VOID)memset
      (
        &prSiceInstance->rMemory,
        (UCHAR)0x00,
        sizeof(SICE_EMUL_MEM_STRUCT)
      );

  // Initialize SICE instance variables
  prSiceInstance->ucTimingMethod          = (UCHAR) CSMD_METHOD_MDT_AT_IPC;   // default value
  prSiceInstance->usNumRecogDevs          = (USHORT) 0;
  prSiceInstance->ucWDAlarm               = (UCHAR) SICE_WD_ALARM_NONE;
  prSiceInstance->ucCycleCnt              = (UCHAR) 0;
  prSiceInstance->ulLatchedTime.ulSeconds = (ULONG) 0;
  prSiceInstance->ulLatchedTime.ulNanos   = (ULONG) 0;
  prSiceInstance->boSercosTimeEn          = FALSE;

  // Initialize Sercos frames
  for (
      iCnt = 0;
      iCnt < (2*CSMD_MAX_TEL*SICE_REDUNDANCY_VAL);
      iCnt++
    )
  {
    // De-activate frame
    prSiceInstance->aprSendFrame[iCnt]->boEnable = FALSE;

    // Get frame pointer and fill frame with constant header values
    puSercosFrame = (SICE_SIII_FRAME *)prSiceInstance->aprSendFrame[iCnt]->aucData;

    // Set destination MAC address to broadcast according to Sercos
    // specification
    puSercosFrame->rTel.aucDestMAC[0] = (UCHAR) 0xFF;
    puSercosFrame->rTel.aucDestMAC[1] = (UCHAR) 0xFF;
    puSercosFrame->rTel.aucDestMAC[2] = (UCHAR) 0xFF;
    puSercosFrame->rTel.aucDestMAC[3] = (UCHAR) 0xFF;
    puSercosFrame->rTel.aucDestMAC[4] = (UCHAR) 0xFF;
    puSercosFrame->rTel.aucDestMAC[5] = (UCHAR) 0xFF;

    // Set source MAC address
    puSercosFrame->rTel.aucSrcMAC[0] = prSiceInstance->aucMyMAC[0];
    puSercosFrame->rTel.aucSrcMAC[1] = prSiceInstance->aucMyMAC[1];
    puSercosFrame->rTel.aucSrcMAC[2] = prSiceInstance->aucMyMAC[2];
    puSercosFrame->rTel.aucSrcMAC[3] = prSiceInstance->aucMyMAC[3];
    puSercosFrame->rTel.aucSrcMAC[4] = prSiceInstance->aucMyMAC[4];
    puSercosFrame->rTel.aucSrcMAC[5] = prSiceInstance->aucMyMAC[5];

    // Set Ethernet type
    puSercosFrame->rTel.usPortID = (USHORT) htons((USHORT) SICE_SIII_ETHER_TYPE);
  }

  // Calculate base checksum to reduce processing performance needed later
  // \todo OK for big endian?
  prSiceInstance->ulBaseCRC = SICE_CRC32Calc
      (
        puSercosFrame->aucRaw,              // Data pointer
        SICE_TEL_LENGTH_HDR_FOR_CRC - SICE_TEL_LENGTH_DYN_HDR_FOR_CRC,
                                            // Static part of Sercos header
        (ULONG) 0x00000000                  // Base CRC
      );

  // Get pointers to Sercos IP core memory segments to reduce typing overhead
  prSiceInstance->prReg =
      (CSMD_HAL_SERCFPGA_REGISTER*) prSiceInstance->rMemory.aucRegister;
  prSiceInstance->prSVC_Ram =
      (CSMD_HAL_SVC_RAM*) prSiceInstance->rMemory.aucSvc;
  prSiceInstance->prTX_Ram =
      (CSMD_HAL_TX_RAM*) prSiceInstance->rMemory.aucTxRAM;
  prSiceInstance->prRX_Ram =
      (CSMD_HAL_RX_RAM*) prSiceInstance->rMemory.aucRxRAM;

  // Initialize Sercos IP core registers

  // Identification register (IDR)
  prSiceInstance->prReg->ulIDR =
      ((ULONG) CSMD_HAL_FPGA_IDR_SIII_IDENT) << ((ULONG) CSMD_HAL_FPGA_IDR_IDENT_SHIFT);
  prSiceInstance->prReg->ulIDR |=
      ((ULONG) SICE_IDR_SOFT_MASTER) << ((ULONG) CSMD_HAL_FPGA_IDR_TYPE_SHIFT);
  prSiceInstance->prReg->ulIDR |=
      ((ULONG) SICE_EMUL_IP_CORE_VER) << ((ULONG) CSMD_HAL_FPGA_IDR_RELEASE_SHIFT);

  SICE_VERBOSE
      (
        1,
        "Signaled SICE version in IDR register: 0x%X\n",
        prSiceInstance->prReg->ulIDR
      );

  // Global control/status/feature register (GCSFR)
  prSiceInstance->prReg->ulGCSFR =
      ((ULONG) SICE_LINE_BREAK_SENS) << ((ULONG) CSMD_HAL_GCSFR_SHIFT_LINE_BR_SENS);

  // Address segment control registers (ASCR0..1), obsolete
  prSiceInstance->prReg->ulASCR0 = (ULONG) 0;
  prSiceInstance->prReg->ulASCR1 = (ULONG) 0;

  // Interrupt enable registers (IER0..1)
  prSiceInstance->prReg->ulIER0 = (ULONG) 0;
  prSiceInstance->prReg->ulIER1 = (ULONG) 0;

  // Interrupt multiplex registers (IMR0..1)
  prSiceInstance->prReg->ulIMR0 = (ULONG) 0;
  prSiceInstance->prReg->ulIMR1 = (ULONG) 0;

  // Interrupt reset/status registers (IRR0..1 / ISR0..1)
  // Overlapped read/write access
  prSiceInstance->prReg->ulIRR0 = (ULONG) 0;
  prSiceInstance->prReg->ulIRR1 = (ULONG) 0;

  // Frame control register (SFCR)
  prSiceInstance->prReg->ulSFCR = (ULONG) 0;

  // Timing control/status register (TCSR)
  prSiceInstance->prReg->ulTCSR = (ULONG) 0;

  // System timer readback register (STRBR)
  prSiceInstance->prReg->ulSTRBR = (ULONG) 0;
  // \todo put system time value into register

  // Sync delay register (TCYCSTART)
  prSiceInstance->prReg->ulTCYCSTART = (ULONG) 0;

  // Event configuration registers
  prSiceInstance->prReg->ulMTDRL = (ULONG) 0;
  prSiceInstance->prReg->ulMTDRU = (ULONG) 0;
  prSiceInstance->prReg->ulMTDSR = (ULONG) 0;

  // Time measure registers (TMR1..2)
  prSiceInstance->prReg->ulRDLY1 = (ULONG) 0;
  prSiceInstance->prReg->ulRDLY2 = (ULONG) 0;

  // TCNT cycletime register (TCNTCYCR)
  prSiceInstance->prReg->ulTCNTCYCR = (ULONG) 0;

  // System time registers; nano seconds (STNS), seconds (STSEC)
  prSiceInstance->prReg->ulSTNS   = (ULONG) 0;
  prSiceInstance->prReg->ulSTSEC  = (ULONG) 0;

  // System time registers; pre-calculated nano (STNSP)
  // pre-calculated seconds (STSECP)
  prSiceInstance->prReg->ulSTNSP  = (ULONG) 0;
  prSiceInstance->prReg->ulSTSECP = (ULONG) 0;

  // Subcycle counters (SCCAB, SCCMDT)
  prSiceInstance->prReg->rSCCAB.ulSCCSR = (ULONG) 0;

#if (CSMD_DRV_VERSION <=5)
  prSiceInstance->prReg->rSCCCMDT.ulSCCNT = (ULONG) 0;

  // Data flow control/status register (DFCSR)
  prSiceInstance->prReg->ulDFCSR =
      (ULONG) CSMD_HAL_DFCSR_TOPOLOGY_NRT_LINE_MODE;
#else
  prSiceInstance->prReg->rSCCMDT.ulSCCNT = (ULONG) 0;

  // Data flow control/status register (DFCSR)
  prSiceInstance->prReg->rDFCSR.ulLong =
      (ULONG) CSMD_HAL_DFCSR_TOPOLOGY_UC_LINE_MODE;
#endif

  // Descriptor Control Register
  prSiceInstance->prReg->rDECR.ulDesIdxTableOffsets = (ULONG) 0;

  // Sequence counter register (SEQCNT)
  prSiceInstance->prReg->ulSEQCNT = (ULONG) 0;

  // Telegram status registers (TGSR1..2), overlapped read/write access
  prSiceInstance->prReg->ulTGSR1 = (ULONG) 0;
  prSiceInstance->prReg->ulTGSR2 = (ULONG) 0;
  prSiceInstance->ulTGSR1 = (ULONG) 0;
  prSiceInstance->ulTGSR2 = (ULONG) 0;

  // Phase control register (PHASECR)
  prSiceInstance->prReg->ulPHASECR = (ULONG) 0;

  // Inter-frame-gap register (IFG)
  prSiceInstance->prReg->ulIFG = (ULONG) 0;

  // RX/TX buffer control/status register (TXBUFCSR, RXBUFCSR)
  // Different from the Sercos master IP core, buffer count is initialized
  // with 0 (single buffer system in soft master vs. default triple buffer
  // in master ip core)
  prSiceInstance->prReg->ulTXBUFCSR_A = (ULONG) 0;
  prSiceInstance->prReg->ulTXBUFCSR_B = (ULONG) 0;
  prSiceInstance->prReg->ulRXBUFCSR_A = (ULONG) 0;
  prSiceInstance->prReg->ulRXBUFCSR_B = (ULONG) 0;

  // RX buffer telegram valid registers (RXBUFTV)
  prSiceInstance->prReg->ulRXBUFTV_A = (ULONG) 0;
  prSiceInstance->prReg->ulRXBUFTV_B = (ULONG) 0;

  // RX buffer telegram requirements registers (RXBUFTR)
  prSiceInstance->prReg->ulRXBUFTR_A = (ULONG) 0;
  prSiceInstance->prReg->ulRXBUFTR_B = (ULONG) 0;

  // SVC control/status register (SVCCSR)
  prSiceInstance->prReg->ulSVCCSR = (ULONG) 0;

  // Watchdog control & status register (WDCSR) and watchdog counter (WDCNT).
  // In contrast to the Sercos master IP core, the control word of
  // WDCSR is not initialized with 0x88CD / SIII_ETHER_TYPE /
  // CSMD_HAL_WD_MAGIC_PATTERN, but with 0 in order to recognize when
  // the application writes via CoSeMa 0x88CD to it. This behavior is
  // compatible with CoSeMa.
  prSiceInstance->prReg->rWDCSR.ulWDCSR           = (ULONG) 0;
  prSiceInstance->prReg->rWDCNT.rCounter.usActual = (USHORT) 0;
  prSiceInstance->prReg->rWDCNT.rCounter.usReset  = (USHORT) 0;

  // Get pointer to event list that is mapped to memory interface
  // (different from 'hard' IP core, enabled by CoSeMa define
  // CSMD_SOFT_MASTER)
  /*lint -save -e513 -e826 const! */
  prSiceInstance->prEvents =
      (CSMD_EVENT*)   (
                ((UCHAR*)prSiceInstance->prReg) +
                CSMD_HAL_SOFT_MASTER_REG_EVENT_OFFSET
              );
  /*lint -restore const! */

  // Initialize event list
  for (
      iCnt = 0;
      iCnt < CSMD_TIMER_EVENT_NUMBER;
      iCnt++
    )
  {
    prSiceInstance->prEvents[iCnt].ulTime         = (ULONG) 0;
    prSiceInstance->prEvents[iCnt].usSubCycCnt    = (USHORT) 0;
    prSiceInstance->prEvents[iCnt].usSubCycCntSel = (USHORT) 0;
    prSiceInstance->prEvents[iCnt].usType         = (USHORT) 0;
  }

  // Initialize UC channel
#ifdef SICE_UC_CHANNEL

  (VOID)SICE_UCC_Reset(prSiceInstance);

#endif

  // Maximum transmit unit MTU (IPLASTFL)
  prSiceInstance->prReg->ulIPLASTFL = (ULONG) 0;

  // Frame and error counter registers
  prSiceInstance->prReg->rIPFCSERR.ulErrCnt  = (ULONG) 0;
  prSiceInstance->prReg->rIPFRXOK.ulErrCnt   = (ULONG) 0;
  prSiceInstance->prReg->rIPFTXOK.ulErrCnt   = (ULONG) 0;
  prSiceInstance->prReg->rIPALGNERR.ulErrCnt = (ULONG) 0;
  prSiceInstance->prReg->rIPDISRXB.ulErrCnt  = (ULONG) 0;
  prSiceInstance->prReg->rIPCHVIOL.ulErrCnt  = (ULONG) 0;
  prSiceInstance->prReg->rIPSERCERR.ulErrCnt = (ULONG) 0;

  // MII control/status register
  prSiceInstance->prReg->rMIICSR.ulLong = (ULONG) 0;

  // Debug output and control register
  prSiceInstance->prReg->ulDBGOCR = (ULONG) 0;

  return(SICE_NO_ERROR);
}

/**
 * \fn SICE_FUNC_RET SICE_Close(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \public
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   Cleanup for closing the Sercos SoftMaster core instance
 *
 * \details This functions frees allocated memory and closes the receive and
 *          transmit sockets.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:    No error
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2012-10-15
 *
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 */
SICE_FUNC_RET SICE_Close
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  INT iCnt;

  SICE_VERBOSE(3, "SICE_Close()\n");

  //Close UC channel
#ifdef SICE_UC_CHANNEL

  SICE_UCC_Close
      (
        prSiceInstance                   // SICE instance structure
      );

#endif

  // Close sockets
  RTOS_CloseRxSocket
      (
        prSiceInstance->iInstanceNo,
        SICE_REDUNDANCY_BOOL
      );
  RTOS_CloseTxSocket
      (
        prSiceInstance->iInstanceNo,
        SICE_REDUNDANCY_BOOL
      );

  // Free allocated frame buffers
  for (
      iCnt = 0;
      iCnt < (2*CSMD_MAX_TEL*SICE_REDUNDANCY_VAL);
      iCnt++
    )
  {
    free(prSiceInstance->aprSendFrame[iCnt]);
  }

return(SICE_NO_ERROR);
}
