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
 * \file      SICE_USER.h
 *
 * \brief     User settings for Sercos SoftMaster core.
 *            Note: settings for CoSeMa have to be made in CSMD_USER.h.
 *
 * \ingroup   SICE
 *
 * \author    GMy
 *
 * \copyright Copyright Bosch Rexroth AG, 2013-2016
 *
 * \date      2013-03-22
 *
 * \version 2014-01-16 (GMy): New setting SICE_WAIT_RX_AFTER_TX
 * \version 2015-05-18 (GMy): New settings SICE_RAM_OFFSET_SVC,
 *                            SICE_RAM_OFFSET_TX and SICE_RAM_OFFSET_RX
 * \version 2015-05-19 (GMy): Defdb00180480: Added preliminary UCC support
 * \version 2015-11-03 (GMy): Defdb00180480: Code optimization
 */

// avoid multiple inclusions - open

#ifndef _SICE_USER
#define _SICE_USER

//---- includes ---------------------------------------------------------------

//---- defines ----------------------------------------------------------------

/**
 * \def     SICE_VERBOSE_LEVEL
 *
 * \brief   Defines the debug level that results in the number of debug outputs
 *          in the module SICE. A value of -1 means no output at all, 0
 *          'normal' outputs and higher values are intended for debugging.
 */
#define SICE_VERBOSE_LEVEL          (0)

/**
 * \def     SICE_CALL_RX_RIGHT_AFTER_TX
 *
 * \brief   If defined, Sercos packets are received right after sending them
 *          out by the soft master. This may result in easier integration into
 *          application task concept, but may cause more sensitivity to Sercos
 *          timing issues. This option only makes sense when using the
 *          high-level function SICE_Cycle() rather that SICE_Cycle_Prepare()
 *          and SICE_Cycle_Start() separately
 */
#undef SICE_CALL_RX_RIGHT_AFTER_TX

/**
 * \def     SICE_WAIT_RX_AFTER_TX
 *
 * \brief   This value defines an additional waiting time in ns between
 *          transmitting and receiving in the mode SICE_CALL_RX_RIGHT_AFTER_TX.
 */
#define SICE_WAIT_RX_AFTER_TX       (0)

/**
 * \def     SICE_LINE_BREAK_SENS
 *
 * \brief Line break sensitivity signaled to CoSeMa. Ignored by SICE.
 */
#define SICE_LINE_BREAK_SENS        (5)

/**
 * \def     SICE_OPT_UCC_CP1_2
 *
 * \brief   If defined, UCC in CP1 and CP2 is moved from default position to
 *          optimized position in order to maximize distance from RT packets
 *          (based on Sercos specification 1.3.1)
 */
#undef SICE_OPT_UCC_CP1_2

/**
 * \def     SICE_OPT_UCC_CP1_2_IGNORE_ACK
 *
 * \brief   If defined, a missing acknowledge of slave in SICE_OPT_UCC_CP1_2
 *          mode is ignored in order to support this feature also on Sercos
 *          1.3.0 slave devices.
 */
#undef SICE_OPT_UCC_CP1_2_IGNORE_ACK

/**
 * \def     SICE_WAITING_TIME_TX_MDT_AT
 *
 * \brief   This value defines an additional waiting time in ns between MDT and
 *          AT transmission for timing mode MDT-AT-UCC or the waiting time
 *          between AT and MDT for timing mode MDT-UCC-AT. This value is
 *          ignored in case the NIC-based transmission mode is being used.
 */
#define SICE_WAITING_TIME_TX_MDT_AT     (0)

/**
 * \def     SICE_USE_NIC_TIMED_TX
 *
 * \brief   If defined, the NIC-timed transmission mode is enabled to optimize
 *          timing of the soft master. This setting is only allowed if a
 *          suitable NIC is used and if RTOS_TIMING_MODE is set to RTOS_TIMING_NIC.
 *
 * \attention Only for testing in safe environments, not thoroughly tested and
 *            not officially released yet!
 */
#undef SICE_USE_NIC_TIMED_TX

/**
 * \def     SICE_REDUNDANCY
 *
 * \brief   If defined, master port redundancy is used to allow Sercos ring
 *          topology. Should only be used in SICE_USE_NIC_TIMED_TX mode.
 *
 * \attention Only for testing in safe environments, not thoroughly tested and
 *            not officially released yet!
 */
#undef SICE_REDUNDANCY

/**
 * \def     SICE_UC_CHANNEL
 *
 * \brief   If defined, SICE provides UCC support. 
 *          (For CoSeMa 5VRS it is needed to define CSMD_IP_CHANNEL)
 *
 * \attention Only for testing in safe environments, not thoroughly tested and
 *            not officially released yet!
 */
#undef SICE_UC_CHANNEL

/**
 * \def     SICE_UCC_INT_NRT
 *
 * \brief   UCC duration per cycle in NRT state in ns.
 */
#define SICE_UCC_INT_NRT                (250 * 1000)

/**
 * \def     SICE_RAM_OFFSET_SVC
 *
 * \brief   This value may be used to introduce an offset (unused memory
 *          region in bytes) between the register and the service channel
 *          memory of SICE. Default value: 0 bytes.
 */
#define SICE_RAM_OFFSET_SVC             (0)

/**
 * \def     SICE_RAM_OFFSET_TX
 *
 * \brief   This value may be used to introduce an offset (unused memory
 *          region in bytes) between service channel and transmit (tx) memory
 *          of SICE. Default value: 0 bytes.
 */
#define SICE_RAM_OFFSET_TX              (0)

/**
 * \def     SICE_RAM_OFFSET_RX
 *
 * \brief   This value may be used to introduce an offset (unused memory
 *          region in bytes) between transmit (tx) and receive (rx) memory of
 *          SICE. Default value: 0 bytes.
 */
#define SICE_RAM_OFFSET_RX              (0)

/**
 * \def     SICE_UCC_BUF_SIZE
 *
 * \brief   Buffer size (number of packets) of UCC buffer, each for transmit
 *          and receive buffer. For each packet, SICE_ETH_FRAMEBUF_LEN bytes
 *          are allocated.
 */
#define SICE_UCC_BUF_SIZE               (64)

/**
 * \def     SICE_MEASURE_TIMING
 *
 * \brief   For test only to measure packet jitter! Do not activate during
 *          Sercos operation!
 */
#undef SICE_MEASURE_TIMING

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

// avoid multiple inclusions - close

#endif
