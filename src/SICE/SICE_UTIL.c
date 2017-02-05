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
 * \file      SICE_UTIL.c
 *
 * \brief     Sercos SoftMaster core: Utility functions
 *
 * \ingroup   SICE
 *
 * \date      2013-01-15
 *
 * \author    GMy
 *
 * \copyright Copyright Bosch Rexroth AG, 2013-2016
 *
 * \version 2013-01-15 (GMy): Added watchdog support
 * \version 2013-01-31 (GMy): Added Sercos timing method support
 * \version 2013-02-14 (GMy): Ring delay measurement emulation
 * \version 2013-04-22 (GMy): Code modularization
 * \version 2013-06-20 (GMy): Optimization of error handling
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 * \version 2014-05-22 (GMy): New function SICE_IncPacketCounter()
 * \version 2015-05-19 (GMy): New function SICE_CalcPacketDuration()
 * \version 2015-06-03 (GMy): Lint code optimization
 */

//---- includes ---------------------------------------------------------------

#include "../SICE/SICE_GLOB.h"
#include "../SICE/SICE_PRIV.h"

#include "../CSMD/CSMD_BM_CFG.h"
#include "../CSMD/CSMD_CALC.h"

//---- defines ----------------------------------------------------------------

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn ULONG SICE_GetEventTime(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              USHORT usEventType
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 * \param[in]       usEventType     Event type to return time of
 *
 * \brief   This function returns the time of the given event.
 *
 * \return
 * - 0: Invalid Event
 * - Event time in ns otherwise
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2013-03-28
 */
ULONG SICE_GetEventTime
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      USHORT usEventType
    )
{
  INT iCnt;

  SICE_VERBOSE(3, "SICE_GetEventTime()\n");

  if (prSiceInstance ==  NULL)
  {
    return(0);
  }

  iCnt = 0;
  do
  {
    if (prSiceInstance->prEvents[iCnt].usType == usEventType)
    {
      // Corresponding event found
      return(prSiceInstance->prEvents[iCnt].ulTime);
    }
    iCnt++;
  }
  while (iCnt < CSMD_TIMER_EVENT_NUMBER);

  // Event not found in list
  return(0);
}

/**
 * \fn SICE_FUNC_RET SICE_CheckTimingMethod(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   This function checks whether CoSeMa has changed the Sercos timing
 *          method by use of event table.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:                No change of timing method
 *          - SICE_TIMING_METHOD_CHANGED:   Timing method has been changed by
 *                                          CoSeMa
 *          - SICE_PARAMETER_ERROR:         For function parameter error
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2013-03-28
 *
 * \version 2013-08-22 (GMy): Use of SICE_GetEventTime()
 */
SICE_FUNC_RET SICE_CheckTimingMethod
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  UCHAR ucTimingMethodNew = 0;
  ULONG ulATTime;
  ULONG ulUCCTime;

  SICE_VERBOSE(3, "SICE_CheckTimingMethod()\n");

  if  (prSiceInstance == NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  // Find out AT and UCC start event time
  ulATTime = SICE_GetEventTime
      (
        prSiceInstance,
        CSMD_EVENT_START_AT
      );

  ulUCCTime = SICE_GetEventTime
      (
        prSiceInstance,
        CSMD_EVENT_IP_CHANNEL_TX_OPEN
      );

  // Is AT scheduled after UCC?
  // This criterion is used to determine whether Sercos telegram timing
  // method 1 or 2 is used.
  // In case one of the events has not been signaled, the default is to
  // use the timing mode CSMD_METHOD_MDT_AT_IPC
  if (
      (ulATTime   !=  (ULONG) 0)    &&
      (ulUCCTime  !=  (ULONG) 0)    &&
      (ulATTime   >   ulUCCTime)
    )
  {
    ucTimingMethodNew = (UCHAR) CSMD_METHOD_MDT_IPC_AT;
  }
  else
  {
    ucTimingMethodNew = (UCHAR) CSMD_METHOD_MDT_AT_IPC;
  }

  if (ucTimingMethodNew != prSiceInstance->ucTimingMethod)
  {
    prSiceInstance->ucTimingMethod = ucTimingMethodNew;
    return(SICE_TIMING_METHOD_CHANGED);
  }
  else
  {
    return(SICE_NO_ERROR);
  }
}

#ifdef CSMD_HW_WATCHDOG
/**
 * \fn SICE_FUNC_RET SICE_UpdateWatchdogStatus(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   This function updates the status of the emulated hardware watchdog
 *          of 'hard' Sercos IP core.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:            Success
 *          - SICE_PARAMETER_ERROR:     For function parameter error
 *
 * \details The function checks whether CoSeMa has triggered or disabled the
 *          watchdog and reacts accordingly. In case the watchdog is active,
 *          the watchdog counter is counted down. If the actual counter value
 *          is zero, the watchdog alarm is signaled by the variable ucWDAlarm
 *          in the SICE instance.
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2013-03-28
 *
 * \version 2013-04-11 (GMy): Optimization of signaling
 *
 */
SICE_FUNC_RET SICE_UpdateWatchdogStatus
    (
      SICE_INSTANCE_STRUCT *prSiceInstance
    )
{
  SICE_VERBOSE(3, "SICE_UpdateWatchdogStatus()\n");

  if (prSiceInstance == NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

#if (CSMD_DRV_VERSION <=5)
  // Check for watchdog trigger
  // (magic pattern written by CoSeMa to WDCSR)
  if (prSiceInstance->prReg->rWDCSR.rShort.usControlWord == (USHORT) CSMD_HAL_WD_MAGIC_PATTERN)
  {
    SICE_VERBOSE
        (
          1,
          "Watchdog triggered. Timeout: %d\n",
          prSiceInstance->prReg->rWDCNT.rCounter.usReset
        );

    prSiceInstance->prReg->rWDCSR.rShort.usStatusWord |= (USHORT) CSMD_HAL_WD_ACTIVE;

    prSiceInstance->prReg->rWDCNT.rCounter.usActual =
        prSiceInstance->prReg->rWDCNT.rCounter.usReset;

    // In contrast to the Sercos master IP core, the control word of
    // WDCSR is not initialized with 0x88CD / SIII_ETHER_TYPE /
    // CSMD_HAL_WD_MAGIC_PATTERN, but with 0 in order to recognize when
    // the application writes 0x88CD to it via CoSeMa. This behavior is
    // compatible with CoSeMa.
    prSiceInstance->prReg->rWDCSR.rShort.usControlWord  = (USHORT) 0;
  }
  // Check for watchdog disable command (inverted magic pattern)
  else if (prSiceInstance->prReg->rWDCSR.rShort.usControlWord ==
      ((USHORT) ~CSMD_HAL_WD_MAGIC_PATTERN))
  {
    SICE_VERBOSE(0, "Watchdog disabled\n");

    prSiceInstance->prReg->rWDCSR.rShort.usStatusWord &= ((USHORT) ~CSMD_HAL_WD_ACTIVE);
    prSiceInstance->prReg->rWDCSR.rShort.usStatusWord &= ((USHORT) ~CSMD_HAL_WD_ALARM);
    prSiceInstance->prReg->rWDCNT.rCounter.usActual = (USHORT) 0;
    prSiceInstance->ucWDAlarm = (UCHAR) SICE_WD_ALARM_DISABLE_TX_TEL;

    // In contrast to the Sercos master IP core, the control word of
    // WDCSR is not initialized with 0x88CD / SIII_ETHER_TYPE /
    // CSMD_HAL_WD_MAGIC_PATTERN, but with 0 in order to recognize when
    // the application writes via CoSeMa 0x88CD to it. This behavior is
    // compatible with CoSeMa.
    prSiceInstance->prReg->rWDCSR.rShort.usControlWord = (USHORT) 0;
  }
  // Is watchdog currently active?
  else if (prSiceInstance->prReg->rWDCSR.rShort.usStatusWord & (USHORT) CSMD_HAL_WD_ACTIVE)
  {
    // Watchdog counter at zero?
    if (prSiceInstance->prReg->rWDCNT.rCounter.usActual == (USHORT) 0)
    {
      // Signal watchdog alarm, if not already active
      if (!(prSiceInstance->prReg->rWDCSR.rShort.usStatusWord & (USHORT) CSMD_HAL_WD_ALARM))
      {
        prSiceInstance->prReg->rWDCSR.rShort.usStatusWord |=
            (USHORT) CSMD_HAL_WD_ALARM;

        // Check alarm mode selected by CoSeMa
        if (prSiceInstance->prReg->rWDCSR.rShort.usStatusWord &
              (USHORT) CSMD_HAL_WDCSR_MODE_DIS_TEL)
        {
          // Do not send packets alarm mode
          SICE_VERBOSE
              (
                0,
                "Watchdog alarm: Stopping to send telegrams.\n"
              );

          prSiceInstance->ucWDAlarm = (UCHAR) SICE_WD_ALARM_DISABLE_TX_TEL;
        }
        else
        {
          // Replace telegram content by zeros alarm mode
          SICE_VERBOSE
              (
                0,
                "Watchdog alarm: Replacing telegram content by zeros.\n"
              );

          prSiceInstance->ucWDAlarm = (UCHAR) SICE_WD_ALARM_SEND_EMPTY_TEL;
        }
      }
    }
    else
    {
      // Count down watchdog value counter
      prSiceInstance->prReg->rWDCNT.rCounter.usActual--;
      SICE_VERBOSE
          (
            2,
            "Watchdog counting down. Value: %d\n",
            prSiceInstance->prReg->rWDCNT.rCounter.usActual
          );
    }
  }
#else
  // Check for watchdog trigger
  // (magic pattern written by CoSeMa to WDCSR)
  if (prSiceInstance->prReg->rWDCSR.rShort.usMagicPattern == (USHORT) CSMD_HAL_WD_MAGIC_PATTERN)
  {
    SICE_VERBOSE
        (
          1,
          "Watchdog triggered. Timeout: %d\n",
          prSiceInstance->prReg->rWDCNT.rCounter.usReset
        );

    prSiceInstance->prReg->rWDCSR.rShort.usControlStatus |= (USHORT) CSMD_HAL_WD_ACTIVE;

    prSiceInstance->prReg->rWDCNT.rCounter.usActual =
        prSiceInstance->prReg->rWDCNT.rCounter.usReset;

    // In contrast to the Sercos master IP core, the control word of
    // WDCSR is not initialized with 0x88CD / SIII_ETHER_TYPE /
    // CSMD_HAL_WD_MAGIC_PATTERN, but with 0 in order to recognize when
    // the application writes via CoSeMa 0x88CD to it. This behavior is
    // compatible with CoSeMa.
    prSiceInstance->prReg->rWDCSR.rShort.usMagicPattern = (USHORT) 0;
  }
  // Check for watchdog disable command (inverted magic pattern)
  else if (prSiceInstance->prReg->rWDCSR.rShort.usMagicPattern ==
      ((USHORT) ~CSMD_HAL_WD_MAGIC_PATTERN))
  {
    SICE_VERBOSE(0, "Watchdog disabled\n");

    prSiceInstance->prReg->rWDCSR.rShort.usControlStatus &= ((USHORT) ~CSMD_HAL_WD_ACTIVE);
    prSiceInstance->prReg->rWDCSR.rShort.usControlStatus &= ((USHORT) ~CSMD_HAL_WD_ALARM);
    prSiceInstance->prReg->rWDCNT.rCounter.usActual = (USHORT) 0;
    prSiceInstance->ucWDAlarm = (UCHAR) SICE_WD_ALARM_DISABLE_TX_TEL;

    // In contrast to the Sercos master IP core, the control word of
    // WDCSR is not initialized with 0x88CD / SIII_ETHER_TYPE /
    // CSMD_HAL_WD_MAGIC_PATTERN, but with 0 in order to recognize when
    // the application writes via CoSeMa 0x88CD to it. This behavior is
    // compatible with CoSeMa.
    prSiceInstance->prReg->rWDCSR.rShort.usMagicPattern = (USHORT) 0;
  }
  // Is watchdog currently active?
  else if (prSiceInstance->prReg->rWDCSR.rShort.usControlStatus &
      (USHORT) CSMD_HAL_WD_ACTIVE)
  {
    // Watchdog counter at zero?
    if (prSiceInstance->prReg->rWDCNT.rCounter.usActual == (USHORT) 0)
    {
      // Signal watchdog alarm, if not already active
      if (!(prSiceInstance->prReg->rWDCSR.rShort.usControlStatus &
          (USHORT) CSMD_HAL_WD_ALARM))
      {
        prSiceInstance->prReg->rWDCSR.rShort.usControlStatus |= (USHORT) CSMD_HAL_WD_ALARM;

        // Check alarm mode selected by CoSeMa
        if (prSiceInstance->prReg->rWDCSR.rShort.usControlStatus &
              (USHORT) CSMD_HAL_WDCSR_MODE_DIS_TEL)
        {
          // Do not send packets alarm mode
          SICE_VERBOSE(
              0,
              "Watchdog alarm: Stopping to send telegrams.\n");

          prSiceInstance->ucWDAlarm = (UCHAR) SICE_WD_ALARM_DISABLE_TX_TEL;
        }
        else
        {
          // Replace telegram content by zeros in alarm mode
          SICE_VERBOSE
              (
                0,
                "Watchdog alarm: Replacing telegram content by zeros.\n"
              );

          prSiceInstance->ucWDAlarm = (UCHAR) SICE_WD_ALARM_SEND_EMPTY_TEL;
        }
      }
    }
    else
    {
      // Count down watchdog value counter
      prSiceInstance->prReg->rWDCNT.rCounter.usActual--;
      SICE_VERBOSE
          (
            2,
            "Watchdog counting down. Value: %d\n",
            prSiceInstance->prReg->rWDCNT.rCounter.usActual
          );
    }
  }
#endif

  return(SICE_NO_ERROR);
}
#endif

/**
 * \fn SICE_FUNC_RET SICE_CalcRingDelay(
 *              SICE_INSTANCE_STRUCT *prSiceInstance
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 *
 * \brief   This function calculates the Sercos ring delay as an emulation to
 *          the measured value.
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:            Success
 *          - SICE_PARAMETER_ERROR:     For function parameter error
 *
 * \note    It is very difficult to provide a stable measurement the Sercos
 *          ring delay on a soft master, as typically the telegram jitter of
 *          the soft master is larger than the ring delay. Therefore, a
 *          calculated value is used here.
 *
 * \details For the master, MST delay and master delay defined in CSMD_USER.h
 *          are taken into account. For the slaves, for each Sercos port (two
 *          times the slave devices minus 1 in the line, one time for each
 *          slave device in ring topology), the slave delay from CSMD_USER.h is
 *          used in the calculation.
 *
 * \note    As a fixed single line topology is assumed, the ring delay for the
 *          second port is always set to 0.
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2013-04-22
 *
 * \version 2013-04-22 (GMy): Moved calculation to separate function
 * \version 2014-05-21 (GMy): Added preliminary redundancy support
 */
SICE_FUNC_RET SICE_CalcRingDelay(SICE_INSTANCE_STRUCT *prSiceInstance)
{
  SICE_VERBOSE(3, "SICE_CalcRingDelay()\n");

  if  (prSiceInstance == NULL)
  {
    return(SICE_PARAMETER_ERROR);
  }

  // Case 1: No redundancy (single line topology)
#ifndef SICE_REDUNDANCY
  // For port 1, calculated from number of recognized slaves and other
  // parameters.
  // For port 2 always 0.
  // Hot-plug slaves, etc. are taken into account separately by CoSeMa.

  prSiceInstance->prReg->ulRDLY1 =
      ((ULONG) CSMD_MST_DELAY)                +   // Offset MDT start to MST, including MDT
                                                  // timer offset and logic delay in IP core
      ((ULONG) CSMD_DELAY_MASTER_DELAY)       +
      (
        ((ULONG) CSMD_DELAY_SLAVE_DELAY)    *
        ((((ULONG) prSiceInstance->usNumRecogDevs) * (ULONG) 2) - (ULONG) 1)
      );

  prSiceInstance->prReg->ulRDLY2 = (ULONG) 0; // Port 2: always non-connected

  // Case 2: With redundancy (ring topology)
#else

  // Ring topology For port 1 and 2, calculated from number of recognized
  // slaves and other parameters.

  prSiceInstance->prReg->ulRDLY1 =
      ((ULONG) CSMD_MST_DELAY)                +   // Offset MDT start to MST, including MDT
                                                  // timer offset and logic delay in IP core
      ((ULONG) CSMD_DELAY_MASTER_DELAY)       +
      (((ULONG) CSMD_DELAY_SLAVE_DELAY) * ((ULONG) prSiceInstance->usNumRecogDevs));

  prSiceInstance->prReg->ulRDLY2 = prSiceInstance->prReg->ulRDLY1;    // Port 2: the same as port 1

#endif

  SICE_VERBOSE
      (
        2,
        "Ring delay set. P1: %luns, P2: %luns\n",
        prSiceInstance->prReg->ulRDLY1,
        prSiceInstance->prReg->ulRDLY2
      );

  return(SICE_NO_ERROR);
}

/**
 * \fn SICE_FUNC_RET SICE_IncPacketCounter(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              INT iRXorTX,
 *              BOOL boIsOk,
 *              INT iPort,
 *              INT iVal
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 * \param[in]       iRXorTX         SICE_RX_PACKET_CNT for RX,
 *                                  SICE_TX_PACKET_CNT for TX,
 *                                  SICE_RX_UCC_CNT for UCC RX,
 *                                  SICE_TX_UCC_CNT for UCC TX,
 *                                  SICE_RX_UCC_DISC for discarded UCC RX,
 *                                  SICE_TX_UCC_DISC for discarded UCC TX
 * \param[in]       boIsOk          Was transmission OK?
 * \param[in]       iPort           Port index, either SICE_ETH_PORT_P or
 *                                  SICE_ETH_PORT_S
 * \param[in]       iVal            Number of packets / increments
 *
 * \brief   This function increases the corresponding packet counter register
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR:            Success
 *          - SICE_PARAMETER_ERROR:     For function parameter error
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2013-05-22
 *
 * \version 2015-07-17 (GMy): Defdb00180480: Added UCC support, corrected
 *                            error counter registers
 */
SICE_FUNC_RET SICE_IncPacketCounter
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      INT iRXorTX,
      BOOL boIsOk,
      INT iPort,
      INT iVal
    )
{
  SICE_VERBOSE(3, "SICE_IncPacketCounter()\n");

  if (prSiceInstance == NULL)
  {
    return (SICE_PARAMETER_ERROR);
  }

  switch (iRXorTX)
  {
    case SICE_RX_PACKET_CNT:
      if (boIsOk)
      {
        if (iPort == SICE_ETH_PORT_P)
        {
          // no counter?
        }
        else if (iPort == SICE_ETH_PORT_S)
        {
          // no counter?
        }
        else
        {
          return(SICE_PARAMETER_ERROR);
        }
      }
      else
      {
        if (iPort == SICE_ETH_PORT_P)
        {
          // no counter?
        }
        else if (iPort == SICE_ETH_PORT_S)
        {
          // no counter?
        }
        else
        {
          return(SICE_PARAMETER_ERROR);
        }
      }
      break;

    case SICE_TX_PACKET_CNT:
      if (boIsOk)
      {
        if (iPort == SICE_ETH_PORT_P)
        {
          // no counter?
        }
        else
        {
          // no counter?
        }
      }
      else
      {
        if (iPort == SICE_ETH_PORT_P)
        {
          // no counter?
        }
        else
        {
          // no counter?
        }
      }
      return(SICE_NO_ERROR);
      break;

#ifdef SICE_UC_CHANNEL

    case SICE_RX_UCC_CNT:
      if (boIsOk)
      {
        if (iPort == SICE_ETH_PORT_P)
        {
          // Increase register for signaling received error-free frames
          if (prSiceInstance->prReg->rIPFRXOK.rErrCnt.usPort1 != ((USHORT) 0xFFFF))
            //saturation
          {
            prSiceInstance->prReg->rIPFRXOK.rErrCnt.usPort1 += (USHORT)iVal;
          }
        }
        else if (iPort == SICE_ETH_PORT_S)
        {
          // Increase register for signaling received error-free frames
          if (prSiceInstance->prReg->rIPFRXOK.rErrCnt.usPort2 != ((USHORT) 0xFFFF))
            //saturation
          {
            prSiceInstance->prReg->rIPFRXOK.rErrCnt.usPort2 += (USHORT)iVal;
          }
        }
      }
        else
        {
          if (iPort == SICE_ETH_PORT_P)
          {
            // no counter?
          }
          else
          {
            // no counter?
          }
        }
      break;

    case SICE_TX_UCC_CNT:
      if (boIsOk)
      {
        if (iPort == SICE_ETH_PORT_P)
        {
          // Increase register for signaling received error-free IP frames
          if (prSiceInstance->prReg->rIPFTXOK.rErrCnt.usPort1 != ((USHORT) 0xFFFF))
            //saturation
          {
            prSiceInstance->prReg->rIPFTXOK.rErrCnt.usPort1 += (USHORT)iVal;
          }
        }
        else
        {
          // Increase register for signaling received error-free IP frames
          if (prSiceInstance->prReg->rIPFTXOK.rErrCnt.usPort2 != ((USHORT) 0xFFFF))
            //saturation
          {
            prSiceInstance->prReg->rIPFTXOK.rErrCnt.usPort2 += (USHORT)iVal;
          }
        }
      }
      else
      {
        if (iPort == SICE_ETH_PORT_P)
        {
          // no counter?
        }
        else
        {
          // no counter?
        }
      }
      break;

    case SICE_RX_UCC_DISC:
      // Increase register for signaling discarded receive packets
      if (iPort == SICE_ETH_PORT_P)
      {
        if (prSiceInstance->prReg->rIPDISRXB.rErrCnt.usPort1 != ((USHORT) 0xFFFF))
        //saturation
        {
          prSiceInstance->prReg->rIPDISRXB.rErrCnt.usPort1 += (USHORT)iVal;
        }
      }
      else
      {
        if (prSiceInstance->prReg->rIPDISRXB.rErrCnt.usPort2 != ((USHORT) 0xFFFF))
        //saturation
        {
          prSiceInstance->prReg->rIPDISRXB.rErrCnt.usPort2 += (USHORT)iVal;
        }
      }
      break;

    case SICE_TX_UCC_DISC:
      // Increase register for signaling discarded transmit packets
      if (iPort == SICE_ETH_PORT_P)
      {
        if (prSiceInstance->prReg->rIPDISCOLB.rErrCnt.usPort1 != ((USHORT) 0xFFFF))
        //saturation
        {
          prSiceInstance->prReg->rIPDISCOLB.rErrCnt.usPort1 += (USHORT)iVal;
        }
      }
      else
      {
        if (prSiceInstance->prReg->rIPDISCOLB.rErrCnt.usPort2 != ((USHORT) 0xFFFF))
        //saturation
        {
          prSiceInstance->prReg->rIPDISCOLB.rErrCnt.usPort2 += (USHORT)iVal;
        }
      }
      break;
#endif
    default:
      return(SICE_PARAMETER_ERROR);
      break;
  }
  return(SICE_NO_ERROR);
}

/**
 * \fn ULONG SICE_CalcPacketDuration(
 *              SICE_INSTANCE_STRUCT *prSiceInstance,
 *              USHORT usPacketLen
 *          )
 *
 * \private
 *
 * \param[in,out]   prSiceInstance  Pointer to Sercos SoftMaster core
 *                                  instance
 * \param[in]       usPacketLen     Length of packet in bytes
 *
 * \brief   This function calculates and returns the time needed for packet
 *          transmission including inter frame gap, but not including possible
 *          timing jitter
 *
 * \return
 * - Packet transmission duration in ns (including inter frame gap)
 *
 * \author  GMy
 *
 * \ingroup SICE
 *
 * \date    2015-05-19
 *
 */
ULONG SICE_CalcPacketDuration
    (
      SICE_INSTANCE_STRUCT *prSiceInstance,
      USHORT usPacketLen
    )
{
  ULONG ulPacketDuration;

  SICE_VERBOSE(3, "SICE_CalcPacketDuration()\n");

  ulPacketDuration = (usPacketLen + prSiceInstance->prReg->ulIFG) * CSMD_BYTE_TIME;

  SICE_VERBOSE
      (
        3,
        "Packet length: %u, Duration: %u\n",
        usPacketLen,
        ulPacketDuration
      );

  return (ulPacketDuration);
}
