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
 * \file      SICE_CYCLIC.c
 *
 * \brief     Sercos SoftMaster core: Cyclic functions
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
 * \version 2013-02-28 (GMy): Lint code optimization
 * \version 2013-03-28 (GMy): Code modularization
 * \version 2013-04-22 (GMy): Code modularization part 2
 * \version 2013-05-07 (GMy): Added support for INtime
 * \version 2013-06-06 (GMy): Added support for RTX and Kithara
 * \version 2013-06-21 (GMy): Added support for Windows desktop versions and
 *                            QNX
 * \version 2013-06-25 (GMy): Finalized Sercos time support
 * \version 2013-08-21 (GMy): Added preliminary support for NIC-timed packet
 *                            transmission mode
 * \version 2014-01-16 (GMy): New setting SICE_WAIT_RX_AFTER_TX taken into
 *                            account
 * \version 2014-04-01 (GMy): Added support for CoSeMa 6VRS
 * \version 2014-05-15 (GMy): Added preliminary redundancy support
 * \version 2014-12-04 (GMy): Bugfix in CoSeMa 5V3 compatibility
 * \version 2015-04-09 (GMy): Separation of SICE_Cycle() to
 *                            SICE_Cycle_Prepare() and SICE_Cycle_Start()
 * \version 2015-05-19 (GMy): Removed call of UCC in SICE_Cycle_Start()
 * \version 2015-07-14 (GMy): Defdb00180480: Code optimization, added
 *                            preliminary UCC support in NIC-timed transmission
 *                            mode
 * \version 2015-11-03 (GMy): Defdb00180480: Code optimization
 */

//---- includes ---------------------------------------------------------------

#include "../SICE/SICE_GLOB.h"
#include "../SICE/SICE_PRIV.h"

#include "../CSMD/CSMD_HAL_PRIV.h"

//---- defines ----------------------------------------------------------------

// CoSeMa 5VRS and CoSeMa 6VRS compatibility
#if (CSMD_DRV_VERSION <=5)
    #define SICE_DFCSR      prSiceInstance->prReg->ulDFCSR

    /*--------------------------------------------------------- */
    /* Rx Buffer Telegram Valid Register A/B (204h/214h)        */
    /*--------------------------------------------------------- */
    #define CSMD_HAL_RXBUFTV_MDT_SHIFT  (0)                     /* Bit shifts for required MDT's                     */
    #define CSMD_HAL_RXBUFTV_MDT_MASK   ((ULONG) 0x0000000F)    /* Bit  3- 0: Mask for required MDT's                */
    #define CSMD_HAL_RXBUFTV_AT_SHIFT   (8)                     /* Bit shifts for required AT's                      */
    #define CSMD_HAL_RXBUFTV_AT_MASK    ((ULONG) 0x00000F00)    /* Bit 11- 8: Mask for required AT's                 */
    #define CSMD_HAL_RXBUFTV_P2_SHIFT   (16)                    /* Bit shift for port 2 telegrams                    */

#else
    #define SICE_DFCSR      prSiceInstance->prReg->rDFCSR.ulLong
#endif

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SICE_FUNC_RET SICE_Cycle(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              ULONG *pulSICECycleTime
 *          )
 *
 * \public
 *
 * \param[in,out]   prSiceInstance      Pointer to Sercos SoftMaster core
 *                                      instance
 * \param[out]      pulSICECycleTime    Current cycle time in ns. It shall be
 *                                      used by the calling entity to update
 *                                      the calling interval of SICE_Cycle()
 *                                      accordingly.
 *
 * \brief   This function performs a single Sercos SoftMaster core cycle.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:                    No error
 *          - SICE_SYSTEM_ERROR:                Problems with SICE reset
 *          - SICE_WATCHDOG_ERROR:              Watchdog error
 *          - SICE_BUFFER_ERROR:                Illegal buffer settings
 *          - SICE_PARAMETER_ERROR:             Function parameter error
 *          - SICE_HW_SVC_ERROR:                Hardware SVC is requested by
 *                                              CoSeMa
 *
 * \details This function performs a single emulation cycle and is to be called
 *          at the beginning of each Sercos cycle. As the timing of this
 *          function call determines the timing of the whole soft master, care
 *          should be used when designing the entity providing this timing.
 *
 *          Instead of this function, the sub-functions SICE_Cycle_Prepare()
 *          and SICE_Cycle_Start() may be called directly in order to optimize
 *          the timing of the soft master.
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2012-10-11
 *
 * \version 2012-10-11 (GMy): Baseline for Sercos master IP core v3
 * \version 2012-10-31 (GMy): Updated for Sercos master IP core v4
 * \version 2013-01-15 (GMy): Added watchdog support
 * \version 2013-01-31 (GMy): Added Sercos timing method support
 * \version 2013-02-14 (GMy): Ring delay measurement emulation
 * \version 2013-02-23 (GMy): Improved structure for sending and receiving
 * \version 2013-06-20 (GMy): Optimization of error handling
 * \version 2013-06-25 (GMy): Finalized Sercos time support
 * \version 2013-08-02 (GMy): Added check for cycle time value and hardware SVC
 *                            usage
 * \version 2013-08-07 (GMy): Bugfix: Problem with reset signal fixed
 * \version 2013-08-21 (GMy): Added preliminary support for NIC-timed
 *                            transmission mode
 * \version 2014-01-16 (GMy): New setting SICE_WAIT_RX_AFTER_TX taken into
 *                            account
 * \version 2014-04-03 (GMy): Bugfix: Corrected handling of ScCount register
 * \version 2014-05-19 (GMy): Added support of RXBUFTV register
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 * \version 2015-04-04 (GMy): Separation into preparation and start function
 */
SICE_FUNC_RET SICE_Cycle
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      ULONG *pulSICECycleTime
    )
{
  SICE_FUNC_RET   eSiceFuncRet;

  SICE_VERBOSE(3, "SICE_Cycle()\n");

  eSiceFuncRet = SICE_Cycle_Prepare
      (
        prSiceInstance
      );

  if (eSiceFuncRet == SICE_NO_ERROR)
  {
    eSiceFuncRet = SICE_Cycle_Start
        (
          prSiceInstance,
          pulSICECycleTime
        );
  }

  if (eSiceFuncRet != SICE_NO_ERROR)
  {
    return(eSiceFuncRet);
  }

  return(SICE_NO_ERROR);
}

/**
 * \fn SICE_FUNC_RET SICE_Cycle_Prepare(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \public
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   This function performs prepares the Sercos packets to be sent
 *          at the beginning of the next Sercos cycle and updates the
 *          registers of the memory mapped interface.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:                    No error
 *          - SICE_SYSTEM_ERROR:                Problems with SICE reset
 *          - SICE_WATCHDOG_ERROR:              Watchdog error
 *          - SICE_BUFFER_ERROR:                Illegal buffer settings
 *          - SICE_PARAMETER_ERROR:             Function parameter error
 *
 * \details If a soft reset is requested, it is performed. PHY reset signals
 *          are ignored. If enabled, the watchdog status is updated. The
 *          emulated IP core registers are updated. Afterwards, the packets are
 *          prepared for transmission. Transmission itself is done in the
 *          separate function SICE_Cycle_Start().
 *
 * \author  GMy, partially based on earlier work by SBe
 *
 * \ingroup SICE
 *
 * \date    2015-04-09
 *
 * \version 2015-04-09 (GMy): Created from SICE_Cycle()
 */
SICE_FUNC_RET SICE_Cycle_Prepare
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  SICE_FUNC_RET eSiceFuncRet = SICE_NO_ERROR;

  SICE_VERBOSE(3, "SICE_Cycle_Prepare()\n");

  if (prSiceInstance ==  NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  // Global control/status/feature register (GCSFR)

  // Check for PHY reset command
  if (
      (prSiceInstance->prReg->ulGCSFR & (((ULONG) 1) << CSMD_HAL_GCSFR_PHY_RESET))
      != (ULONG) 0
    )
  {
    SICE_VERBOSE(1, "PHY reset\n");

    // Nothing to do, as there is no PHY. Just clear the flag.
    prSiceInstance->prReg->ulGCSFR &= ~(((ULONG) 1) << CSMD_HAL_GCSFR_PHY_RESET);
  }

  // Check for soft reset command
  if (
      (prSiceInstance->prReg->ulGCSFR & (((ULONG) 1) << CSMD_HAL_GCSFR_SOFT_RESET))
      != (ULONG) 0
    )
  {
    SICE_VERBOSE(1, "Soft reset\n");

    // Perform soft reset of soft master
    eSiceFuncRet = SICE_SoftReset(prSiceInstance);

    if (eSiceFuncRet != SICE_NO_ERROR)
    {
      return(SICE_SYSTEM_ERROR);
    }

    // Clear soft reset flag
    prSiceInstance->prReg->ulGCSFR &= ~(((ULONG) 1) << CSMD_HAL_GCSFR_SOFT_RESET);
  }
  else
  {
    // 'Hardware' watchdog activated by CoSeMa #define switch?
#ifdef CSMD_HW_WATCHDOG

    eSiceFuncRet = SICE_UpdateWatchdogStatus(prSiceInstance);

    if (eSiceFuncRet != SICE_NO_ERROR)
    {
      SICE_VERBOSE(0, "Watchdog error!\n");
      return(SICE_WATCHDOG_ERROR);
    }
#endif

    // Initialize TGSRs
    prSiceInstance->ulTGSR1 = 0;
    prSiceInstance->ulTGSR2 = 0;
    prSiceInstance->prReg->ulTGSR1 = 0;
    prSiceInstance->prReg->ulTGSR2 = 0;

    // Timer registers

    // Is ET3 / EST for Sercos system time enabled?
    if (
        (prSiceInstance->prReg->ulTCSR & (((ULONG)1) << CSMD_HAL_TCSR_EST))
        != ((ULONG) 0)
      )
    {
      // System timer registers
      prSiceInstance->prReg->ulSTNS += prSiceInstance->prReg->ulTCNTCYCR;
      if (prSiceInstance->prReg->ulSTNS >= ((ULONG) 1000*1000*1000))
      {
        // Overflow
        prSiceInstance->prReg->ulSTNS -= (ULONG) 1000*1000*1000;
        prSiceInstance->prReg->ulSTSEC++;
      }

      SICE_VERBOSE
          (
            2,
            "SICE time: %lu:%lu\n",
            prSiceInstance->prReg->ulSTSEC,
            prSiceInstance->prReg->ulSTNS
          );

      // Pre-calculated system timer registers
      prSiceInstance->prReg->ulSTNSP += prSiceInstance->prReg->ulTCNTCYCR;
      if (prSiceInstance->prReg->ulSTNSP >= ((ULONG) 1000*1000*1000))
      {
        // Overflow
        prSiceInstance->prReg->ulSTNSP -= (ULONG) 1000*1000*1000;
        prSiceInstance->prReg->ulSTSECP++;
      }
    }

    // System timer readback register (STRBR)
    // \todo put time value into register
    prSiceInstance->prReg->ulSTRBR = (ULONG) 0;

    // Interrupt reset/status registers (IRR0..1 / ISR0..1)
    // Overlapped read/write access
    // Interrupts currently not used
    prSiceInstance->prReg->ulIRR0 = (ULONG) 0;
    prSiceInstance->prReg->ulIRR1 = (ULONG) 0;

    // Phase Control Register (PHASECR)
    // De-mask cycle counter
    prSiceInstance->ucCycleCnt =
        (UCHAR) (
              (prSiceInstance->prReg->ulPHASECR & ((ULONG) CSMD_HAL_PHASECR_CYCLE_COUNT))
              >> ((ULONG) SICE_PHASECR_CYCLE_COUNT_SHIFT)
            );

    // Increment cycle counter
    prSiceInstance->ucCycleCnt = (UCHAR)((prSiceInstance->ucCycleCnt + 1) %
        ((UCHAR) SICE_PHASECR_CYCLE_COUNT_MOD));

    // Write back cycle counter
    prSiceInstance->prReg->ulPHASECR =
        (prSiceInstance->prReg->ulPHASECR & ~((ULONG) CSMD_HAL_PHASECR_CYCLE_COUNT)) +
        (((ULONG) prSiceInstance->ucCycleCnt) << ((ULONG) SICE_PHASECR_CYCLE_COUNT_SHIFT));

    // Timing control and status register (TCSR)
    // Set TCNT running status flag to ET0 control flag as this bit may be
    // used by CoSeMa e.g. to detect whether the TCNT may not be running
    // due to a missing external trigger signal
    prSiceInstance->prReg->ulTCSR |=
        (prSiceInstance->prReg->ulTCSR & (((ULONG)1) << ((ULONG) CSMD_HAL_TCSR_ET0)))
        << ((ULONG) CSMD_HAL_TCSR_TCNT_RUNNING);

    // Subcycle counters (SCCA, SCCB, SCCMDT)

    prSiceInstance->prReg->rSCCAB.rSCC.ucSccValueBufA++;

    if (prSiceInstance->prReg->rSCCAB.rSCC.ucSccDifBufA != (UCHAR) 0)
    {
      prSiceInstance->prReg->rSCCAB.rSCC.ucSccValueBufA %=
            prSiceInstance->prReg->rSCCAB.rSCC.ucSccDifBufA;
    }

    prSiceInstance->prReg->rSCCAB.rSCC.ucSccValueBufB++;

    if (prSiceInstance->prReg->rSCCAB.rSCC.ucSccDifBufB != (UCHAR) 0)
    {
      prSiceInstance->prReg->rSCCAB.rSCC.ucSccValueBufB %=
          prSiceInstance->prReg->rSCCAB.rSCC.ucSccDifBufB;
    }

#if (CSMD_DRV_VERSION <=5)

    if (prSiceInstance->prReg->rSCCCMDT.rSCCNT.usSccCount != (USHORT) 0)
    {
      prSiceInstance->prReg->rSCCCMDT.rSCCNT.usSccReadBack++;
      prSiceInstance->prReg->rSCCCMDT.rSCCNT.usSccReadBack %=
          prSiceInstance->prReg->rSCCCMDT.rSCCNT.usSccCount;
    }
    else
    {
      prSiceInstance->prReg->rSCCCMDT.rSCCNT.usSccReadBack = (USHORT) 0;
    }
#else
    if (prSiceInstance->prReg->rSCCMDT.rSCCNT.usScCount != (USHORT) 0)
    {
      prSiceInstance->prReg->rSCCMDT.rSCCNT.usSccReadBack++;

      prSiceInstance->prReg->rSCCMDT.rSCCNT.usSccReadBack %=
          prSiceInstance->prReg->rSCCMDT.rSCCNT.usScCount;
    }
    else
    {
      prSiceInstance->prReg->rSCCMDT.rSCCNT.usSccReadBack = (USHORT) 0;
    }
#endif
    // Ring delay registers (ulRDLY1..2)
    (VOID)SICE_CalcRingDelay(prSiceInstance);

    // Data flow control/status register (DFCSR)
    SICE_DFCSR  &=  ~((ULONG) 1 << (ULONG) CSMD_HAL_DFCSR_LINE_STS_P1);
                                // Line status P1: no error

    SICE_DFCSR  &=  ~((ULONG) 1 << (ULONG) CSMD_HAL_DFCSR_LINE_STS_DELAY_P1);
                                // Delayed line status P1: no error

    SICE_DFCSR  |=  ((ULONG) 1  << (ULONG) CSMD_HAL_DFCSR_LINK_P1);
                                // Link on P1

#ifndef SICE_REDUNDANCY

    SICE_DFCSR  |=  ((ULONG) 1  << (ULONG) CSMD_HAL_DFCSR_LINE_STS_P2);
                                // Line status P2: error

    SICE_DFCSR  |=  ((ULONG) 1  << (ULONG) CSMD_HAL_DFCSR_LINE_STS_DELAY_P2);
                                // Delayed line status P2: error

    SICE_DFCSR  &=  ~((ULONG) 1 << (ULONG) CSMD_HAL_DFCSR_LINK_P2);
                                // No link on P2

#else

    SICE_DFCSR  &=  ~((ULONG) 1 << (ULONG) CSMD_HAL_DFCSR_LINE_STS_P2);
                                // Line status P2: no error

    SICE_DFCSR  &=  ~((ULONG) 1 << (ULONG) CSMD_HAL_DFCSR_LINE_STS_DELAY_P2);
                                // Delayed line status P2: no error

    SICE_DFCSR  |=  ((ULONG) 1 << (ULONG) CSMD_HAL_DFCSR_LINK_P2);
                                // Link on P2

#endif

    // Sequence counter register (SEQCNT).
    // in CP0 overwritten by receive function later
    prSiceInstance->prReg->ulSEQCNT = (ULONG) 0;

    // RX buffer control/status register
    // Currently, only buffer A with single buffering is supported
    if (
        (prSiceInstance->prReg->ulRXBUFCSR_A & ((ULONG) CSMD_HAL_RXBUFCSR_COUNT_MASK))
        != ((ULONG) CSMD_HAL_SINGLE_BUFFER_SYSTEM)
      )
    {
      SICE_VERBOSE
          (
            0,
            "Error: Only receive buffer system A with single "
            "buffering currently is supported by SICE.\n"
          );

      return(SICE_BUFFER_ERROR);
    }

    prSiceInstance->prReg->ulRXBUFCSR_A &= ~((ULONG) CSMD_HAL_RXBUFCSR_NEW_DATA_P1);

    prSiceInstance->prReg->ulRXBUFCSR_A &= ~((ULONG) CSMD_HAL_RXBUFCSR_NEW_DATA_P2);

    if (
        (prSiceInstance->prReg->ulRXBUFCSR_B & ((ULONG) CSMD_HAL_RXBUFCSR_COUNT_MASK))
        != ((ULONG) CSMD_HAL_SINGLE_BUFFER_SYSTEM)
      )
    {
      SICE_VERBOSE
          (
            0,
            "Error: Only receive buffer system A with single "
            "buffering currently is supported by SICE.\n"
          );
      return(SICE_BUFFER_ERROR);
    }
    prSiceInstance->prReg->ulRXBUFCSR_B &= ~((ULONG) 0xFFFF0000);

    // TX buffer control/status register
    // Currently, only single buffering is supported
    if (
        (prSiceInstance->prReg->ulTXBUFCSR_A & ((ULONG) CSMD_HAL_TXBUFCSR_COUNT_MASK))
        != ((ULONG) CSMD_HAL_SINGLE_BUFFER_SYSTEM)
      )
    {
      SICE_VERBOSE
          (
            0,
            "Error: Only single buffering currently is supported "
            "by SICE.\n"
          );

      return(SICE_BUFFER_ERROR);
    }

    if (
        (prSiceInstance->prReg->ulTXBUFCSR_B & ((ULONG) CSMD_HAL_TXBUFCSR_COUNT_MASK))
        != ((ULONG) CSMD_HAL_SINGLE_BUFFER_SYSTEM)
      )
    {
      SICE_VERBOSE
          (
            0,
            "Error: Only single buffering currently is supported "
            "by SICE.\n"
          );

      return(SICE_BUFFER_ERROR);
    }

    // Check whether Sercos timing mode commanded by CoSeMa via event table
    // has changed

    eSiceFuncRet = SICE_CheckTimingMethod(prSiceInstance);

    if (eSiceFuncRet == SICE_TIMING_METHOD_CHANGED)
    {
      if (prSiceInstance->ucTimingMethod == ((UCHAR) CSMD_METHOD_MDT_AT_IPC))
      {
        SICE_VERBOSE(1, "Sercos timing mode was set to mode MDT-AT-UCC.\n");
      }
      else
      {
        SICE_VERBOSE(1, "Sercos timing mode was set to mode MDT-UCC-AT.\n");
      }
    }

  #ifndef SICE_CALL_RX_RIGHT_AFTER_TX

    if (SICE_CheckPreCondsReceive(prSiceInstance) == SICE_NO_ERROR)
    {
      // Receive Sercos telegrams
      eSiceFuncRet = SICE_ReceiveTelegrams(prSiceInstance);

      if (eSiceFuncRet != SICE_NO_ERROR)
      {
        return(eSiceFuncRet);
      }
    }
  #endif

    if (SICE_CheckPreCondsSend(prSiceInstance) == SICE_NO_ERROR)
    {
      // Prepare Sercos telegrams
      eSiceFuncRet = SICE_PrepareTelegrams(prSiceInstance);

      if (eSiceFuncRet != SICE_NO_ERROR)
      {
        return(eSiceFuncRet);
      }

      return(SICE_NO_ERROR);
    }
  }

  return(SICE_NO_ERROR);
}

/**
 * \fn SICE_FUNC_RET SICE_Cycle_Start(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              ULONG *pulSICECycleTime
 *          )
 *
 * \public
 *
 * \param[in,out]   prSiceInstance      Pointer to Sercos SoftMaster core
 *                                      instance
 * \param[out]      pulSICECycleTime    Current cycle time in ns. It shall be
 *                                      used by the calling entity to update
 *                                      the calling interval of SICE_Cycle()
 *                                      accordingly. The value may be 0 during
 *                                      initialization.
 *
 * \brief   This function starts the Sercos cycle by transmitting the Sercos
 *          telegrams.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:                    No error
 *          - SICE_PARAMETER_ERROR:             Function parameter error
 *          - SICE_HW_SVC_ERROR:                Hardware SVC is requested by
 *                                              CoSeMa
 *
 * \details This function performs a single emulation cycle and is to be called
 *          at the beginning of each Sercos cycle. As the timing of this
 *          function call determines the timing of the whole soft master, care
 *          should be used when designing the entity providing this timing.
 *
 *          It is required that SICE_Cycle_Prepare() is called before.
 *
 * \author  GMy, partially based on earlier work by SBe
 *
 * \ingroup SICE
 *
 * \date    2015-04-09
 *
 * \version 2015-04-09 (GMy): Created from SICE_Cycle()
 * \version 2015-07-14 (GMy): Defdb00180480: Code optimization, added
 *                            preliminary UCC support in NIC-timed transmission
 *                            mode
 */
SICE_FUNC_RET SICE_Cycle_Start
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      ULONG *pulSICECycleTime
    )
{
  SICE_FUNC_RET eSiceFuncRet = SICE_NO_ERROR;

  SICE_VERBOSE(3, "SICE_Cycle_Start()\n");

  if (
      (prSiceInstance   ==  NULL)   ||
      (pulSICECycleTime ==  NULL)
    )
  {
    return(SICE_PARAMETER_ERROR);
  }

  if (SICE_CheckPreCondsSend(prSiceInstance) == SICE_NO_ERROR)
  {

#ifdef SICE_USE_NIC_TIMED_TX

    // Transmit Sercos (and optionally UCC) telegrams using NIC-timed
    // transmission mode
    eSiceFuncRet = SICE_SendTelegramsNicTimed(prSiceInstance);

    if (eSiceFuncRet != SICE_NO_ERROR)
    {
      return(eSiceFuncRet);
    }

  #ifdef SICE_UC_CHANNEL

    // Call UCC packet reception. For non-NIC-timed mode, the cyclic function
    // SICE_UCC_Cycle() should be used instead which also takes care of UCC
    // packet transmission

    eSiceFuncRet = SICE_UCC_ReceiveTelegrams
        (
          prSiceInstance,
          FALSE
        );

    if (eSiceFuncRet != SICE_NO_ERROR)
    {
      return(eSiceFuncRet);
    }

  #endif

#else

    if (prSiceInstance->ucTimingMethod == ((UCHAR) CSMD_METHOD_MDT_AT_IPC))
    {
      // Transmit Sercos MDT telegrams
      eSiceFuncRet = SICE_SendMDTTelegrams(prSiceInstance);

      if (eSiceFuncRet != SICE_NO_ERROR)
      {
        return(eSiceFuncRet);
      }

  #if ((SICE_WAITING_TIME_TX_MDT_AT != 0) && (RTOS_TIMING_MODE != RTOS_TIMING_NIC))
      RTOS_NanoSleepRel(SICE_WAITING_TIME_TX_MDT_AT);
  #endif

      // Transmit Sercos AT telegrams
      eSiceFuncRet = SICE_SendATTelegrams(prSiceInstance);

      if (eSiceFuncRet != SICE_NO_ERROR)
      {
        return(eSiceFuncRet);
      }
    }
    else if (prSiceInstance->ucTimingMethod == ((UCHAR) CSMD_METHOD_MDT_IPC_AT))
    {
      // Transmit Sercos AT telegrams
      eSiceFuncRet = SICE_SendATTelegrams(prSiceInstance);

      if (eSiceFuncRet != SICE_NO_ERROR)
      {
        return(eSiceFuncRet);
      }

      // In case NIC-timed transmission is not used and SICE_WAIT_TX_MDT_AT is
      // enabled, wait accordingly
  #if ((SICE_WAIT_TX_MDT_AT != 0) && (RTOS_TIMING_MODE != RTOS_TIMING_NIC))
      RTOS_NanoSleepRel(SICE_WAIT_TX_MDT_AT);
  #endif

      // Transmit Sercos MDT telegrams
      eSiceFuncRet = SICE_SendMDTTelegrams(prSiceInstance);

      if (eSiceFuncRet != SICE_NO_ERROR)
      {
        return(eSiceFuncRet);
      }
    }
    else
    {
      SICE_VERBOSE(0, "Error: Unknown Sercos timing method.\n");
      return(SICE_SERCOS_TIMING_MODE_ERROR);
    }

#endif    // SICE_USE_NIC_TIMED_TX

#ifdef SICE_CALL_RX_RIGHT_AFTER_TX

    // Take additional waiting time SICE_WAIT_RX_AFTER_TX into account if
    // defined in SICE_USER.h
    if (SICE_WAIT_RX_AFTER_TX > 0)
    {
      RTOS_NanoSleepRel(SICE_WAIT_RX_AFTER_TX);
    }

    if (SICE_CheckPreCondsReceive(prSiceInstance) == SICE_NO_ERROR)
    {
      // Receive Sercos telegrams
      eSiceFuncRet = SICE_ReceiveTelegrams(prSiceInstance);

      if (eSiceFuncRet != SICE_NO_ERROR)
      {
        return(eSiceFuncRet);
      }
    }
#endif

    prSiceInstance->prReg->ulTGSR1 = (ULONG) prSiceInstance->ulTGSR1;
    prSiceInstance->prReg->ulTGSR2 = (ULONG) prSiceInstance->ulTGSR2;

    /*
    // Telegram status registers (TGSR1..2), overlapped read/write access
    if (prSiceInstance->prReg->ulTGSR1 == (ULONG) 0xFFFFFFFF)
    {
      // Reset telegram status
      prSiceInstance->ulTGSR1 = (ULONG) 0;
      prSiceInstance->prReg->ulTGSR1 = prSiceInstance->ulTGSR1;
    }
    else
    {
      // Take over telegram status
      prSiceInstance->prReg->ulTGSR1 = prSiceInstance->ulTGSR1;
    }
    if (prSiceInstance->prReg->ulTGSR2 == (ULONG) 0xFFFFFFFF)
    {
      // Reset telegram status
      prSiceInstance->ulTGSR2 = (ULONG) 0;
      prSiceInstance->prReg->ulTGSR2 = prSiceInstance->ulTGSR2;
    }
    else
    {
      // Take over telegram status
      prSiceInstance->prReg->ulTGSR2 = (ULONG) prSiceInstance->ulTGSR2;
    }*/

    SICE_VERBOSE
        (
          1,
          "TGSR: 0x%X 0x%X\n",
          prSiceInstance->prReg->ulTGSR1,
          prSiceInstance->prReg->ulTGSR2
        );

    // Take over TGSR telegram status to RXBUFTV
    prSiceInstance->prReg->ulRXBUFTV_A =
        (
          (
            (
              (prSiceInstance->prReg->ulRXBUFTV_A & ~CSMD_HAL_RXBUFTV_MDT_MASK)
              & ~CSMD_HAL_RXBUFTV_AT_MASK
            )
#if (CSMD_DRV_VERSION <=5)
            & ~(CSMD_HAL_RXBUFTV_MDT_MASK << CSMD_HAL_RXBUFTV_P2_SHIFT)
#else
            & ~(CSMD_HAL_RXBUFTV_MDT_MASK << CSMD_HAL_RXBUFTV_PORT_SHIFT)
#endif
          )
#if (CSMD_DRV_VERSION <=5)
          & ~(CSMD_HAL_RXBUFTV_AT_MASK << CSMD_HAL_RXBUFTV_P2_SHIFT)
#else
          & ~(CSMD_HAL_RXBUFTV_AT_MASK << CSMD_HAL_RXBUFTV_PORT_SHIFT)
#endif
        )                                       |
                          // Clear telegram flags
        (
          (prSiceInstance->prReg->ulTGSR1 & CSMD_HAL_TGSR_ALL_MDT)
            << (CSMD_HAL_RXBUFTV_MDT_SHIFT - 0)
        )                                       |
                          // Take over MDT telegram flags for P1
        (
          (prSiceInstance->prReg->ulTGSR1 & CSMD_HAL_TGSR_ALL_AT)
            << (CSMD_HAL_RXBUFTV_AT_SHIFT - 4)
        )                                       |
                          // Take over AT telegram flags for P1
        (
#if (CSMD_DRV_VERSION <=5)
          (prSiceInstance->prReg->ulTGSR2 & CSMD_HAL_TGSR_ALL_MDT)
            << (CSMD_HAL_RXBUFTV_P2_SHIFT + CSMD_HAL_RXBUFTV_MDT_SHIFT - 0)
#else
          (prSiceInstance->prReg->ulTGSR2 & CSMD_HAL_TGSR_ALL_MDT)
            << (CSMD_HAL_RXBUFTV_PORT_SHIFT + CSMD_HAL_RXBUFTV_MDT_SHIFT - 0)
#endif
        )                                       |
                          // Take over MDT telegram flags for P2
        (
#if (CSMD_DRV_VERSION <=5)
          (prSiceInstance->prReg->ulTGSR2 & CSMD_HAL_TGSR_ALL_AT)
            << (CSMD_HAL_RXBUFTV_P2_SHIFT + CSMD_HAL_RXBUFTV_AT_SHIFT - 4)
#else
          (prSiceInstance->prReg->ulTGSR2 & CSMD_HAL_TGSR_ALL_AT)
            << (CSMD_HAL_RXBUFTV_PORT_SHIFT + CSMD_HAL_RXBUFTV_AT_SHIFT - 4)
#endif
        );
                          // Take over AT telegram flags for P2

    // Check whether CoSeMa tries to use hardware SVC
    if (
        (prSiceInstance->prReg->ulSVCCSR & ((ULONG) SICE_SVCCSR_ENABLE_MASK))
        != ((ULONG) 0)
      )
    {
      // Hardware SVC is not supported by SICE
      return(SICE_HW_SVC_ERROR);
    }
  } // if SICE_CheckPreCondsSend()

  else // NRT state
  {

#ifdef SICE_USE_NIC_TIMED_TX
  #ifdef SICE_UC_CHANNEL

  // Transmit UCC telegrams using NIC-timed transmission mode
  eSiceFuncRet = SICE_SendUccTelegramsNicTimedNRT(prSiceInstance);

  if (eSiceFuncRet != SICE_NO_ERROR)
  {
    return(eSiceFuncRet);
  }

  // Call UCC packet reception. For non-NIC-timed mode, the cyclic function
  // SICE_UCC_Cycle() should be used instead which also takes care of UCC
  // packet transmission

  eSiceFuncRet = SICE_UCC_ReceiveTelegrams
      (
        prSiceInstance,
        TRUE
      );

  if (eSiceFuncRet != SICE_NO_ERROR)
  {
    return(eSiceFuncRet);
  }

  #endif
#endif

  }

  // Return current cycle time after checking whether it is valid.
  // The register is set by CoSeMa accordingly. The entity calling
  // SICE_Cycle() shall use this value to adapt the calling interval of
  // SICE_Cycle() accordingly.

  eSiceFuncRet = SICE_CheckCycleTime
      (
        prSiceInstance->prReg->ulTCNTCYCR,
        (UCHAR)(prSiceInstance->prReg->ulPHASECR & ((ULONG)CSMD_HAL_PHASECR_PHASE_MASK))
      );

  if (eSiceFuncRet != SICE_NO_ERROR)
  {
    return(eSiceFuncRet);
  }

  *pulSICECycleTime = prSiceInstance->prReg->ulTCNTCYCR;

  // 'Hardware' watchdog activated by CoSeMa #define switch?
#ifdef CSMD_HW_WATCHDOG
  if (
#if (CSMD_DRV_VERSION <=5)
      (prSiceInstance->prReg->rWDCSR.rShort.usStatusWord & ((USHORT) CSMD_HAL_WD_ACTIVE))      &&
      (prSiceInstance->prReg->rWDCSR.rShort.usStatusWord & ((USHORT) CSMD_HAL_WD_ALARM))
#else
      (prSiceInstance->prReg->rWDCSR.rShort.usControlStatus & ((USHORT) CSMD_HAL_WD_ACTIVE))   &&
      (prSiceInstance->prReg->rWDCSR.rShort.usControlStatus & ((USHORT) CSMD_HAL_WD_ALARM))
#endif
    )
  {
    return(SICE_WATCHDOG_ALARM);
  }
#endif

  return(SICE_NO_ERROR);
}
