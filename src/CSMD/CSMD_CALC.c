/*
CoSeMa V6.1 - Common Sercos Master function library
Copyright (c) 2004 - 2016  Bosch Rexroth AG

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THIS SOFTWARE IS PROVIDED "AS IS"; WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY;
FITNESS FOR A PERTICULAR PURPOSE AND NONINFRINGEMENT. THE AUTHORS OR COPYRIGHT
HOLDERS SHALL NOT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE;
UNLESS STIPULATED BY MANDATORY LAW.

You may contact us at open.source@boschrexroth.de
if you are interested in contributing a modification to the Software.

30 April 2015

============================================================================ */

/*!
\file   CSMD_CALC.c
\author WK
\date   01.09.2010
\brief  This File contains the public API functions and private function for
        phase switching and transmission of the slave configuration.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#include "CSMD_BM_CFG.h"
#include "CSMD_CYCLIC.h"
#include "CSMD_PHASEDEV.h"  /* only for CSMD_GetHW_Settings() */

#define SOURCE_CSMD
#include "CSMD_CALC.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */


/**************************************************************************/ /**
\brief Calculates the telegram assignment and timing.

\ingroup func_timing
\b Description: \n
   This function
   - performs a plausibility check for the configuration of all master and
     slave connections. In case of misconfigurations, error messages are created.
   - determines the necessary mode for the IP-Core watchdog functionality.
     If at least one slave consumer connection with type "non-cyclic type 2"
     is found, the watchdog mode "Stop sendig telegrams" is selected.\n
     Otherwise the watchdog mode "Insert zeros in telegram data" is selected.
   - evaluates the configuration structure for all connections of master and
     slaves. For telegram length calculation, only the produced connections are
     considered.
     These actions provide a basis for telegram construction and Sercos timing.
   - optionally evaluates the configuration structure of connections which are
     produced by the master into the AT.
   - determines the producer cycles for every connection relative to values
     of TSref counter
   - provides three different options for time slot calculation.
     - Method 1: MDT / AT / UCC
     - Method 2: MDT / UCC / AT
     - Method 3: MDT / UCC / AT with AT at the very end of the Sercos cycle
\todo translation
       The width of the UC channel is taken from CSMD_HW_INIT_STRUCT.ulUCC_Width.
       - if ulUCC_Width >= 125000 then take this length
       - if ulUCC_Width = 0 then length = 0 (no UC channel)
       - else calculate length (maximum available)
\ end todo translation

   - calculates the memory needed for the PCI-Busmaster functionality.

   The CSMD_CalculateTiming() function analyzes the connection configuration
   of all master and slave connections entered in the CSMD_CONFIGURATION structure.
   On this basis, the structure of the MDT and AT telegrams is determined.

   General telegram parameters:
     - S-0-1010 Lengths of MDT
     - S-0-1012 Lengths of AT
   Slave-specific telegram parameters:
     - S-0-1009 Device Control(C-DEV) offset in MDT
     - S-0-1011 Device Status (S-DEV) offset in AT
     - S-0-1013 SVC offset in MDT
     - S-0-1014 SVC offset in AT

   Afterwards, the Sercos timing will be determined using the communication cycle
   time (tscyc) specified in the CSMD_COMMON_TIMING structure together with the
   required timing method and the UC channel width.

   General timing parameters:
     - S-0-1006 AT0 Transmission starting time (t1)
     - S-0-1007 Synchronization time (tSync)
     - S-0-1008 Command value valid time MDT (t3)
     - S-0-1017 UCC transmission time (t6/t7)
   Slave-specific timing parameter:
     - S-0-1050.si.SE3 Telegram assignment

   The telegrams are configured as follows:<BR>
   MDT:
   - Hot Plug Field
   - Extended Function Field (Sercos time)
   - SVC of all projected slaves in projection order
   - C-DEV of all slaves in projection order before connection data of all slaves in projection order

   AT:
   - Hot Plug Field
   - SVC of all slaves in projection order
   - CC-Connections
   - Connections produced by the master into AT
   - S-DEV of all slaves in projection order before connection data of all slaves in projection order

<B>Call Environment:</B> \n
   This function may only be called in CP2. If called in another communication phase, it will generate an error.\n
   This function should be called from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]   usTimingMethod
              Position of the UC channel:\n
              - 1 (CSMD_METHOD_MDT_AT_IPC): method 1, after last AT
              - 2 (CSMD_METHOD_MDT_IPC_AT): method 2, between last MDT and first AT
              - 3 (CSMD_METHOD_AT_CYC_END): as method 2 with AT at the end of Sercos-Cycle

\return       \ref CSMD_WARNING_IFG_MISMATCH \n
              \ref CSMD_WARN_RECALCULATED_MTU \n
              \ref CSMD_WRONG_PHASE \n
              \ref CSMD_INVALID_MASTER_JITTER \n
              \ref CSMD_MAX_T_NETWORK_GT_T_SYNCDELAY \n
              \ref CSMD_TOO_MANY_MASTER_CONNECTIONS \n
              \ref CSMD_TOO_MANY_SLAVE_CONNECTIONS \n
              \ref CSMD_NO_UNIQUE_CON_NBR \n
              \ref CSMD_MASTER_PROD_CONN_IN_AT \n
              \ref CSMD_CONNECTION_NOT_CONSUMED \n
              \ref CSMD_WRONG_CONNECTION_INDEX \n
              \ref CSMD_WRONG_CONFIGURATION_INDEX \n
              \ref CSMD_WRONG_RTBIT_CONFIGURATION_INDEX \n
              \ref CSMD_WRONG_SCP_CAP_CONFIGURATION \n
              \ref CSMD_MASTER_CONSUME_IN_MDT \n
              \ref CSMD_SLAVE_PRODUCE_IN_MDT \n
              \ref CSMD_PROD_CYC_TIME_INVALID \n
              \ref CSMD_CONNECTION_WRONG_TEL_TYPE \n
              \ref CSMD_CONN_MULTIPLE_PRODUCED \n
              \ref CSMD_CONNECTION_NOT_PRODUCED \n
              \ref CSMD_CONFIGURATION_NOT_CONFIGURED \n
              \ref CSMD_INVALID_ETHERNET_MTU \n
              \ref CSMD_TOO_MANY_OPER_SLAVES \n
              \ref CSMD_INVALID_SERCOS_CYCLE_TIME \n
              \ref CSMD_TEL_NBR_MDT_SVC \n
              \ref CSMD_TEL_NBR_MDT_RTD \n
              \ref CSMD_TEL_NBR_AT_SVC \n
              \ref CSMD_TEL_NBR_AT_RTD \n
              \ref CSMD_ILLEGAL_TIMING_METHOD \n
              \ref CSMD_CYCTEL_LEN_GT_TSCYC \n
              \ref CSMD_TEL_LEN_GT_TSCYC \n
              \ref CSMD_CONNECTION_LENGTH_0 \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         13.04.2005

 todo         - Add input parameter for timing mode (master/slave).
                Problem: Der mit CSMD_PrepareCYCCLK() vorzugegebene
                Startdelay berechnet die Applikation mit Hilfe des in
                CalculateTiming() berechneten Wertes fuer tSync.

***************************************************************************** */
CSMD_FUNC_RET CSMD_CalculateTiming( CSMD_INSTANCE *prCSMD_Instance,
                                    CSMD_USHORT    usTimingMethod )
{

  CSMD_FUNC_RET   eFuncRet;
  CSMD_FUNC_RET   eFuncRetTemp;

  /* --------------------------------------------------------- */
  /* Step 1 :: Check current communication phase               */
  /* --------------------------------------------------------- */
  if (prCSMD_Instance->sCSMD_Phase != CSMD_SERC_PHASE_2)
  {
    return (CSMD_WRONG_PHASE);
  }
  if (prCSMD_Instance->rSlaveList.usNumProjSlaves > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
  {
    return (CSMD_TOO_MANY_OPER_SLAVES);    /* Too many projected slaves */
  }

  /* Take the hardware specific settings */
  if (CSMD_NO_ERROR != (eFuncRet = CSMD_GetHW_Settings( prCSMD_Instance, FALSE )) )
  {
    return (eFuncRet);
  }

  prCSMD_Instance->rPriv.ulUCC_Width = prCSMD_Instance->rPriv.rHW_Init_Struct.ulUCC_Width;

  /* Check Configuration of all Connections */
  if (CSMD_NO_ERROR != (eFuncRet = CSMD_CheckConfiguration( prCSMD_Instance )) )
  {
    return (eFuncRet);
  }

  /* build list of different producer cycle times in ascending order */
  if (CSMD_NO_ERROR != (eFuncRet = CSMD_Build_Producer_Cycle_Times_List (prCSMD_Instance)) )
  {
    return (eFuncRet);
  }

  /* Sort list of connections which are either produced or consumed by the master in order
     to create an access list without gaps and then sort the created list by producer cycle
     time in ascending order */
  CSMD_Sort_Master_Connections (prCSMD_Instance);

#ifdef CSMD_HW_WATCHDOG
  /* Determine the necessary mode for the IP-Core watchdog functionality */
  if (CSMD_NO_ERROR != (eFuncRet = CSMD_Determine_Watchdog_Mode( prCSMD_Instance )) )
  {
    return (eFuncRet);
  }
#endif

  /* Check for valid Sercos cycle time */
  if (prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 >= CSMD_TSCYC_250_US)
  {
    if (   (prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 % CSMD_TSCYC_250_US)
        || (prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 > CSMD_TSCYC_MAX) )
    {
      return (CSMD_INVALID_SERCOS_CYCLE_TIME);
    }
  }
  else
  {
    if (   (prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 != CSMD_TSCYC_MIN)
        && (prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 != CSMD_TSCYC_62_5_US)
        && (prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 != CSMD_TSCYC_125_US) )
    {
      return (CSMD_INVALID_SERCOS_CYCLE_TIME);
    }
  }

  /* Check for valid MDT send jitter (Sync_jitter) */
  if (   (prCSMD_Instance->rConfiguration.rMasterCfg.ulJitter_Master == 0)
      || (  prCSMD_Instance->rConfiguration.rMasterCfg.ulJitter_Master
          > (prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 / 8)))
  {
    return (CSMD_INVALID_MASTER_JITTER);
  }

  if (CSMD_END_ERR_CLASS_00000 < (eFuncRetTemp = CSMD_CalculateTelegramAssignment( prCSMD_Instance )) )
  {
    return (eFuncRetTemp);
  }

  /* carry over Timing method MDT|AT|UCC or MDT|UCC|AT */
  prCSMD_Instance->rPriv.usTimingMethod = usTimingMethod;

  if (CSMD_END_ERR_CLASS_00000 < (eFuncRet = CSMD_CalculateTimingMethod( prCSMD_Instance,
                                                                         usTimingMethod  )) )
  {
    return (eFuncRet);
  }

  /* initialize private connection structures for connection state machine and determine producer index
     of all slaveproduced connections */
  CSMD_Init_Priv_Conn_Structs (prCSMD_Instance);

  /* build lists for determination whether a connection has to be produced in a certain cycle */
  CSMD_Calculate_Producer_Cycles (prCSMD_Instance);

  if (eFuncRet == CSMD_NO_ERROR)
  {
    return (eFuncRetTemp);
  }
  else
  {
    return (eFuncRet);
  }
} /* end: CSMD_CalculateTiming() */



#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
/**************************************************************************/ /**
\brief Returns tSyncDelay (TSref) and maximum measured tNetwork.

\ingroup func_timing_aux
\b Description: \n
   This function calculate the Synchronization reference time (TSref)
   and the maximum of measured tNetwork at current topology
   according to communication specification as of version 1.3.2.\n
   
   \todo Translation
   TSref muss immer größer sein, als die maximal auftretende Laufzeit.
   Ein wesentlich zu großer Wert für TSref kann durch Anpassung des vorzeichen-
   behafteten Components-Delay (lCompDelay) korrigiert werden.
   Ebenfalls ist eine Änderung des Slave-Port-Delay (ulCalc_DelaySlave)
   möglich.
   
   CSMD_WRONG_PHASE is returned, if the current phase is lower than CP2.

<B>Call Environment:</B> \n
   - This function may be called either from an interrupt or a task
   - This function should be called after CSMD_CheckVersion()

\param [in]   prCSMD_Instance
                Pointer to memory range allocated for the variables of the
                CoSeMa instance
\param [out]  plTSref
                Synchronization reference time (TSref) [ns]
\param [out]  plTNetworkMax
                Maximum of measured tNetwork [ns]

\return       #CSMD_WRONG_PHASE \n
              #CSMD_NO_ERROR

\author       WK
\date         14.06.2016

***************************************************************************** */
CSMD_FUNC_RET CSMD_Check_TSref( CSMD_INSTANCE *prCSMD_Instance,
                                CSMD_LONG     *plTSref,
                                CSMD_LONG     *plTNetworkMax )
{
  CSMD_ULONG ulIFG;

  if (prCSMD_Instance->sCSMD_Phase < CSMD_SERC_PHASE_2)
  {
    return (CSMD_WRONG_PHASE);
  }
  else
  {
    ulIFG = (CSMD_ULONG) ( (  27 * prCSMD_Instance->rPriv.usMaxSlaveJitter
                            * ausSqrt_IFG[prCSMD_Instance->rSlaveList.usNumProjSlaves])
                          / 640000 + 12 + 1);
     /* 1 octet is added in order to round up the value */

    /* Not all slaves with SCP_Sync supports S-0-1036 and calculated IFG > 37 ? */
    if (   (prCSMD_Instance->boIFG_V1_3 == FALSE)
        && (ulIFG > CSMD_IFG_DEFAULT_CP3_CP4))
    {
      prCSMD_Instance->rPriv.ulInterFrameGap = CSMD_IFG_DEFAULT_CP3_CP4;
    }
    else
    {
      prCSMD_Instance->rPriv.ulInterFrameGap = ulIFG;
    }

    /* Calculate stable TSref = calculated tNetwork_max + IFG_jitter/2 + components delay */
    *plTSref =  (CSMD_LONG)(  2U * (  prCSMD_Instance->rSlaveList.usNumProjSlaves
                                    * prCSMD_Instance->rConfiguration.rComTiming.ulCalc_DelaySlave)
                            + (prCSMD_Instance->rPriv.ulInterFrameGap * CSMD_BYTE_TIME) / 2U)
               + prCSMD_Instance->rConfiguration.rComTiming.lCompDelay;

    /* Calculate maximum tNetwork for single line topology */
    *plTNetworkMax = (CSMD_LONG)prCSMD_Instance->rPriv.rRingDelay.rValid.ulMaxTNetwork;

    return (CSMD_NO_ERROR);
  }
} /* end: CSMD_Check_TSref() */
#endif

/*! \endcond */ /* PUBLIC */


/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief  Calculates the telegram assignment.

\ingroup func_timing
\b Description: \n

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       \ref CSMD_WARNING_IFG_MISMATCH \n
              \ref CSMD_TEL_NBR_MDT_SVC \n
              \ref CSMD_TEL_NBR_MDT_RTD \n
              \ref CSMD_TEL_NBR_AT_SVC \n
              \ref CSMD_TEL_NBR_AT_RTD \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         26.11.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_CalculateTelegramAssignment( CSMD_INSTANCE *prCSMD_Instance )
{

  CSMD_INT     nSlaves;           /* Loop counter projected slaves */
  CSMD_INT     nSConn;            /* Loop counter slave connections */
  CSMD_INT     nMConn;            /* Loop counter master connections */
  CSMD_USHORT  usK, usL;          /* count variables */
  CSMD_USHORT  usConnIdx;         /* Connection index */
  CSMD_USHORT  usConfigIdx;       /* Configuration index */
  CSMD_USHORT  usTelNbr;          /* current calculated telegram number */
  CSMD_USHORT  usTelPointer;      /* pointer [byte] to current telegram pos. */
  CSMD_USHORT  usConnLength;      /* connection length in bytes */
  CSMD_BOOL    boFound;

  CSMD_USHORT  usListIndex;
  CSMD_CC_CONN_LIST  *prCC_Conn;  /* help pointer for abbreviation of private structure */

  /* --------------------------------------------------------- */
  /* Clear telegram length info                                */
  /* --------------------------------------------------------- */
  (CSMD_VOID) CSMD_HAL_memset( prCSMD_Instance->rPriv.rMDT_Length,
                               0,
                               sizeof (prCSMD_Instance->rPriv.rMDT_Length) );

  (CSMD_VOID) CSMD_HAL_memset( prCSMD_Instance->rPriv.rAT_Length,
                               0,
                               sizeof (prCSMD_Instance->rPriv.rAT_Length) );


  /* ------------------------------------------------------------------------ */
  /* Step 1:  Calculate MDT telegrams:                                        */
  /*            IDN/S-0-1013       SVC offset in MDT                          */
  /*            IDN/S-0-1009       Device Control (C-DEV) offset in MDT       */
  /*            IDN/S-0-1050.x.03  Telegram Assignment                        */
  /*            IDN/S-0-1010       Lengths of MDTs                            */
  /* ------------------------------------------------------------------------ */

  usTelNbr     = 0;                                   /* first telegram */
  /* Hot-Plug field in MDT0 */
  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usHP = CSMD_HOTPLUG_FIELDWIDTH;

  /* Extended Function field only in MDT0 */
  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usEF = CSMD_EXT_FUNCT_FIELDWIDTH;
  if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
  {
    /* SVC start at position 8 + 6 * n */
    prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usEF += 2U;
  }

  /* pointer to start SVC in current MDT telegram */
  usTelPointer = (CSMD_USHORT) (  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usHP
                                + prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usEF);


  /* ------------------------------------------------------------------------ */
  /* Calculate position of SVC (MDT) for all projected slaves.                */
  /* ------------------------------------------------------------------------ */
  for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++)
  {
    /* Does SVC fit into current telegram? */
    if (  (usTelPointer + CSMD_SVC_FIELDWIDTH)
        > CSMD_SERC3_MAX_DATA_LENGTH)
    {
      /* Data length of telegram */
      prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usTel = usTelPointer;
      /* SVC length of telegram */
      prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usSVC =
        (CSMD_USHORT) (usTelPointer - (  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usHP
                                       + prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usEF));

      /* use next MDT telegram for complete SVC of the slave */
      usTelNbr++;
      if (usTelNbr >= CSMD_MAX_TEL)
      {
        return (CSMD_TEL_NBR_MDT_SVC);
      }
      if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
      {
        /* HP field in all MDTs according to Sercos specification < 1.3.0 */
        prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usHP = CSMD_HOTPLUG_FIELDWIDTH;
      }
      /* Pointer to SVC in next telegram */
      usTelPointer = (CSMD_USHORT) (  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usHP
                                    + prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usEF);
    }

    /* SCP_Basic: S-0-1013 "SVC offset in MDT" */
    prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].rTelegramConfig.usSvcOffsetMDT_S01013 =
      (CSMD_USHORT)(usTelPointer | (CSMD_USHORT)(usTelNbr << 12));

    /* Offset to next SVC */
    usTelPointer += CSMD_SVC_FIELDWIDTH;
  }
  /* SVC length of telegram */
  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usSVC =
    (CSMD_USHORT) (usTelPointer - (  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usHP
                                   + prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usEF));


  /* ------------------------------------------------------------------------ */
  /* For all projected slaves:                                                */
  /* Calculate C-DEV position                                                 */
  /* and position of all consumed by the master produced connections.         */
  /* ------------------------------------------------------------------------ */
  for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++)
  {
    /* Does C-DEV fit into current telegram? */
    if (  (usTelPointer + CSMD_C_DEV_LENGTH)
        > CSMD_SERC3_MAX_DATA_LENGTH)
    {
      /* Data length of telegram */
      prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usTel = usTelPointer;

      /* use next telegram for this C-DEV */
      usTelNbr++;
      if (usTelNbr >= CSMD_MAX_TEL)
      {
        return (CSMD_TEL_NBR_MDT_RTD);
      }
      if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
      {
        /* HP field in all MDTs according to Sercos specification < 1.3.0 */
        prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usHP = CSMD_HOTPLUG_FIELDWIDTH;
      }
      /* Pointer to RTD in next telegram */
      usTelPointer = (CSMD_USHORT) (  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usHP
                                    + prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usEF);
    }

    /* SCP_Basic: S-0-1009 "Device Control offset in MDT" */
    prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].rTelegramConfig.usC_DEV_OffsetMDT_S01009 =
      (CSMD_USHORT)(usTelPointer | (CSMD_USHORT)(usTelNbr << 12));

    /* Offset to next C-DEV */
    usTelPointer += CSMD_C_DEV_LENGTH;
  }

  /* ------------------------------------------------------------------------ */
  /* Calculate connection offset for all MDT connections                      */
  /* ------------------------------------------------------------------------ */
  for (nMConn = 0; nMConn < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; nMConn++)
  {
    usConnIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nMConn].usConnIdx;

    /* check if connection is configured into MDT => produced by master */
    if (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usTelegramType == CSMD_TELEGRAM_TYPE_MDT)
    {
      usConnLength = prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5;

      /* check if connection fits into current telegram? */
      if ((usTelPointer + usConnLength) > CSMD_SERC3_MAX_DATA_LENGTH)
      {
        if (usTelPointer < CSMD_SERC3_MIN_DATA_LENGTH)
        {
          /* Set minimum data length */
          usTelPointer = CSMD_SERC3_MIN_DATA_LENGTH;
        }
        /* adjust data length of telegram */
        prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usTel = usTelPointer;

        /* use next telegram for this connection */
        usTelNbr++;
        if (usTelNbr >= CSMD_MAX_TEL)
        {
          return (CSMD_TEL_NBR_MDT_RTD);
        }
        if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
        {
          /* HP field in all MDTs according to Sercos specification < 1.3.0 */
          prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usHP = CSMD_HOTPLUG_FIELDWIDTH;
        }
        /* Pointer to RTD in next telegram */
        usTelPointer = (CSMD_USHORT) (  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usHP
                                      + prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usEF);
      }

      /* SCP_Basic: S-0-1050.x.3 Telegram Assignment (MDT) */
      prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 =
        (CSMD_USHORT)( CSMD_S_0_1050_SE3_TEL_NBR(usTelNbr) | CSMD_S_0_1050_SE3_TELTYPE_MDT | usTelPointer);

      usTelPointer += usConnLength;
    }
  } /* for (nMConn = 0; nMConn < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; nMConn++) */


  /* --------------------------------------------------------- */
  /* Calculate length of last MDT telegram                     */
  /* --------------------------------------------------------- */
  if (usTelPointer < CSMD_SERC3_MIN_DATA_LENGTH)
  {
    /* Set minimum data length */
    usTelPointer = CSMD_SERC3_MIN_DATA_LENGTH;
  }
  /* Data length of telegram */
  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usTel = usTelPointer;

  for (usTelNbr = 0; usTelNbr < CSMD_MAX_TEL; usTelNbr++)
  {
    /* SCP_Basic: S-0-1010, Lengths of MDTs */
    prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[usTelNbr] =
      prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usTel;

    /* Length of MDT RT Data */
    prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usRTD = (CSMD_USHORT)
      (prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usTel
      - (  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usHP
         + prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usEF
         + prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usSVC));
  }


  /* ------------------------------------------------------------------------ */
  /* Step 2:  Calculate AT telegrams:                                         */
  /*            IDN/S-0-1014       SVC offset in AT                           */
  /*            IDN/S-0-1011       Device Status (S-DEV) offset in AT         */
  /*            IDN/S-0-1050.x.03  Telegram Assignment                        */
  /*            IDN/S-0-1012       Lengths of ATs                             */
  /* ------------------------------------------------------------------------ */

  usTelNbr     = 0;                                   /* first telegram */
  /* Hot-Plug field in AT0 */
  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP  = CSMD_HOTPLUG_FIELDWIDTH;
  /* pointer to start SVC in current AT telegram */
  usTelPointer = prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP;

  /* ------------------------------------------------------------------------ */
  /* Calculate position of SVC (AT) for all projected slaves.                 */
  /* ------------------------------------------------------------------------ */
  for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++)
  {
    /* Does SVC fit into current telegram? */
    if (  (usTelPointer + CSMD_SVC_FIELDWIDTH)
        > CSMD_SERC3_MAX_DATA_LENGTH)
    {
      /* Data length of telegram */
      prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usTel = usTelPointer;

      /* SVC length of telegram */
      prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usSVC =
        (CSMD_USHORT) (usTelPointer - prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP);

      /* use next AT telegram for complete SVC of the slave */
      usTelNbr++;
      if (usTelNbr >= CSMD_MAX_TEL)
      {
        return (CSMD_TEL_NBR_AT_SVC);
      }
      if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
      {
        /* HP field in all ATs according to Sercos specification < 1.3.0 */
        prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP = CSMD_HOTPLUG_FIELDWIDTH;
      }
      /* Pointer to SVC in next telegram */
      usTelPointer = prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP;
    }

    /* SCP_Basic: S-0-1014 "SVC offset in AT" */
    prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].rTelegramConfig.usSvcOffsetAT_S01014 =
      (CSMD_USHORT)(usTelPointer | (CSMD_USHORT)(usTelNbr << 12));

    /* Offset to next SVC */
    usTelPointer += CSMD_SVC_FIELDWIDTH;
  }

  /* SVC length of telegram */
  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usSVC =
    (CSMD_USHORT) (usTelPointer - prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP);


  /* ------------------------------------------------------------------------ */
  /* Calculate position of all produced slave CC connections (AT)             */
  /* ------------------------------------------------------------------------ */

  /* At first configure all CC connections which are consumed by the master! */
  for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++)
  {
    for (nSConn = 0; nSConn < CSMD_MAX_CONNECTIONS; nSConn++)
    {
      usConnIdx = prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].arConnIdxList[nSConn].usConnIdx;
      if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
      {
        /* AT connection */
        if (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usTelegramType == CSMD_TELEGRAM_TYPE_AT)
        {
          usConfigIdx = prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].arConnIdxList[nSConn].usConfigIdx;
          if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
          {
            /* slave consumes from AT => CC connection or master produces in AT */
            if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1
                 & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
            {
              /* search CC connection in master configuration */
              for (nMConn = 0; nMConn < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; nMConn++)
              {
                if (prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nMConn].usConnIdx == usConnIdx)
                {
                  /* check if master is consumer of the connection */
                  usConfigIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nMConn].usConfigIdx;
                  if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
                  {
                    if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1
                         & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
                    {
                      usConnLength = prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5;

                      /* Does connection fit into current telegram? */
                      if ( (usTelPointer + usConnLength) > CSMD_SERC3_MAX_DATA_LENGTH)
                      {
                        if (usTelPointer < CSMD_SERC3_MIN_DATA_LENGTH)
                        {
                          /* Set minimum data length */
                          usTelPointer = CSMD_SERC3_MIN_DATA_LENGTH;
                        }
                        /* Data length of telegram */
                        prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usTel = usTelPointer;

                        /* Length of CC connections in telegram */
                        prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usCC_M =
                          (CSMD_USHORT) (  usTelPointer
                                         - (  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP
                                            + prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usSVC));

                        prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usCC =
                          prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usCC_M;

                        /* use next telegram for this connection */
                        usTelNbr++;
                        if (usTelNbr >= CSMD_MAX_TEL)
                        {
                          return (CSMD_TEL_NBR_AT_RTD);
                        }
                        if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
                        {
                          /* HP field in all ATs according to Sercos specification < 1.3.0 */
                          prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP = CSMD_HOTPLUG_FIELDWIDTH;
                        }
                        /* Pointer to RTD in next telegram */
                        usTelPointer = prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP;
                      }

                      /* SCP_Basic: S-0-1050.x.3 Telegram Assignment (AT) */
                      prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 =
                        (CSMD_USHORT)( CSMD_S_0_1050_SE3_TEL_NBR(usTelNbr) | usTelPointer);

                      /* store connection in private list of CC connections */
                      usListIndex =  prCSMD_Instance->rPriv.rCC_Connections.usNbrCC_Connections;
                      prCC_Conn   = &prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList[usListIndex];

                      prCC_Conn->usConnIdx = usConnIdx;
                      prCC_Conn->usDataOffset = usTelPointer;
                      prCC_Conn->usTelNbr = usTelNbr;
                      prCC_Conn->usMasterConsumeCC = TRUE;

                      prCSMD_Instance->rPriv.rCC_Connections.usNbrCC_Connections++;

                      usTelPointer += usConnLength;

                    } /* if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1 ... */
                  } /* if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig) */
                } /* if (prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nMConn].usConnIdx == usConnIdx) */
              } /* for (nMConn = 0; nMConn < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; nMConn++) */
            } /* if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1 ... */
          } /* if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig) */
        } /* if (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usTelegramType == CSMD_TELEGRAM_TYPE_AT) */
      } /* if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn) */
    } /* for (nSConn = 0; nSConn < CSMD_MAX_CONNECTIONS; nSConn++) */
  } /* for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++) */

  /* Length of CC connections in telegram */
  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usCC_M =
    (CSMD_USHORT) (  usTelPointer
                   - (  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP
                      + prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usSVC));


  /* Secondly configure CC connections that are not consumed by the master! */
  for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++)
  {
    for (nSConn = 0; nSConn < CSMD_MAX_CONNECTIONS; nSConn++)
    {
      usConnIdx = prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].arConnIdxList[nSConn].usConnIdx;
      if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
      {
        /* AT connection */
        if (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usTelegramType == CSMD_TELEGRAM_TYPE_AT)
        {
          usConfigIdx = prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].arConnIdxList[nSConn].usConfigIdx;
          if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
          {
            /* slave consumes from AT => CC connection or master produces in AT */
            if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1
                 & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
            {
              boFound = FALSE;

              /* check if any slave is configured as producer of the connection */
              for (usK = 0; (usK < prCSMD_Instance->rSlaveList.usNumProjSlaves && boFound == FALSE); usK++)
              {
                for (usL = 0; usL < CSMD_MAX_CONNECTIONS; usL++)
                {
                  if (usConnIdx == prCSMD_Instance->rConfiguration.parSlaveConfig[usK].arConnIdxList[usL].usConnIdx)
                  {
                    /* connection found, check if slave is configured as producer */
                    usConfigIdx = prCSMD_Instance->rConfiguration.parSlaveConfig[usK].arConnIdxList[usL].usConfigIdx;
                    if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
                    {
                      if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1
                           & CSMD_S_0_1050_SE1_ACTIVE_PRODUCER) == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
                      {
                        /* Check if telegram assignment for this connection has been configured yet.
                         * If not, the master does not consume the connection (configured above). */
                        if ( !(prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3) )
                        {
                          boFound = TRUE;   /* loop exit flag */
                          usConnLength = prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5;

                          /* Does connection fit into current telegram? */
                          if ( (usTelPointer + usConnLength) > CSMD_SERC3_MAX_DATA_LENGTH)
                          {
                            if (usTelPointer < CSMD_SERC3_MIN_DATA_LENGTH)
                            {
                              /* Set minimum data length */
                              usTelPointer = CSMD_SERC3_MIN_DATA_LENGTH;
                            }
                            /* Data length of telegram */
                            prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usTel = usTelPointer;

                            /* Length of CC connections in telegram */
                            prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usCC =
                              (CSMD_USHORT) (  usTelPointer
                                             - (  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP
                                                + prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usSVC));

                            /* use next telegram for this connection */
                            usTelNbr++;
                            if (usTelNbr >= CSMD_MAX_TEL)
                            {
                              return (CSMD_TEL_NBR_AT_RTD);
                            }
                            if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
                            {
                              /* HP field in all ATs according to Sercos specification < 1.3.0 */
                              prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP = CSMD_HOTPLUG_FIELDWIDTH;
                            }
                            /* Pointer to RTD in next telegram */
                            usTelPointer = prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP;
                          }

                          /* SCP_Basic: S-0-1050.x.3 Telegram Assignment (AT) */
                          prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 =
                            (CSMD_USHORT)( CSMD_S_0_1050_SE3_TEL_NBR(usTelNbr) | usTelPointer);

                          /* store connection in private list of CC connections */
                          usListIndex =  prCSMD_Instance->rPriv.rCC_Connections.usNbrCC_Connections;
                          prCC_Conn   = &prCSMD_Instance->rPriv.rCC_Connections.parCC_ConnList[usListIndex];

                          prCC_Conn->usConnIdx = usConnIdx;
                          prCC_Conn->usDataOffset = usTelPointer;
                          prCC_Conn->usTelNbr = usTelNbr;
                          prCC_Conn->usMasterConsumeCC = FALSE;

                          prCSMD_Instance->rPriv.rCC_Connections.usNbrCC_Connections++;

                          usTelPointer += usConnLength;

                        } /* if ( !(prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3) ) */
                      } /* if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1 ... */
                    } /* if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig) */
                  } /* if (usConnIdx == prCSMD_Instance->rConfiguration.parSlaveConfig[usK].arConnIdxList[usL].usConnIdx) */
                } /* for (usL = 0; usL < CSMD_MAX_CONNECTIONS; usL++) */
              } /* for (usK = 0; (usK < prCSMD_Instance->rSlaveList.usNumProjSlaves && boFound == FALSE); usK++) */
            } /* if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1 ... */
          } /* if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig) */
        } /* if (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usTelegramType == CSMD_TELEGRAM_TYPE_AT) */
      } /* if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn) */
    } /* for (nSConn = 0; nSConn < CSMD_MAX_CONNECTIONS; nSConn++) */
  } /* for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++) */

  /* Length of CC connections in telegram */
  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usCC =
    (CSMD_USHORT) (  usTelPointer
                   - (  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP
                      + prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usSVC));

#ifdef CSMD_MASTER_PRODUCE_IN_AT
  /* ------------------------------------------------------------------------ */
  /* Calculate position of all produced master connections (AT !)             */
  /* ------------------------------------------------------------------------ */
  for (nMConn = 0; nMConn < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; nMConn++)
  {
    usConfigIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nMConn].usConfigIdx;
    if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
    {
      /* check if connection is produced by the master */
      if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1
           & CSMD_S_0_1050_SE1_ACTIVE_PRODUCER) == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
      {
        usConnIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nMConn].usConnIdx;
        if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
        {
          /* check if connection is configured into AT */
          if (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usTelegramType == CSMD_TELEGRAM_TYPE_AT)
          {
            usConnLength = prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5;

            /* Does connection fit into current telegram? */
            if ( (usTelPointer + usConnLength) > CSMD_SERC3_MAX_DATA_LENGTH)
            {
              if (usTelPointer < CSMD_SERC3_MIN_DATA_LENGTH)
              {
                /* Set minimum data length */
                usTelPointer = CSMD_SERC3_MIN_DATA_LENGTH;
              }
              /* Data length of telegram */
              prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usTel = usTelPointer;

              /* use next telegram for this connection */
              usTelNbr++;
              if (usTelNbr >= CSMD_MAX_TEL)
              {
                return (CSMD_TEL_NBR_AT_RTD);
              }
              if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
              {
                /* HP field in all ATs according to Sercos specification < 1.3.0 */
                prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP = CSMD_HOTPLUG_FIELDWIDTH;
              }
              /* Pointer to RTD in next telegram */
              usTelPointer = prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP;
            }

            /* SCP_Basic: S-0-1050.x.3 Telegram Assignment (AT) */
            prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 =
              (CSMD_USHORT)( CSMD_S_0_1050_SE3_TEL_NBR(usTelNbr) | usTelPointer);

            usTelPointer += usConnLength;

          } /* if (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usTelegramType == CSMD_TELEGRAM_TYPE_AT) */
        } /* if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn) */
      } /* if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1 ... */
    } /* if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig) */
  } /* for (nMConn = 0; nMConn < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; nMConn++) */

  /* Length of master produced connections in AT (part of RTD!) */
  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usMProd =
    (CSMD_USHORT) (  usTelPointer
                   - (  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP
                      + prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usSVC
                      + prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usCC));
#endif


  /* ------------------------------------------------------------------------ */
  /* For all projected slaves: calculate S-DEV position                       */
  /* ------------------------------------------------------------------------ */
  for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++)
  {
    /* Does S-DEV fit into current telegram? */
    if (  (usTelPointer + CSMD_S_DEV_LENGTH)
        > CSMD_SERC3_MAX_DATA_LENGTH)
    {
      /* Data length of telegram */
      prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usTel = usTelPointer;

      /* insert slave index of last slave fitting into telegram
       * into telegram assignment list of device status */
      prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[usTelNbr] = (CSMD_USHORT)(nSlaves - 1);

      /* use next telegram for S-DEV of the slave */
      usTelNbr++;
      if (usTelNbr >= CSMD_MAX_TEL)
      {
        return (CSMD_TEL_NBR_AT_RTD);
      }
      if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
      {
        /* HP field in all ATs according to Sercos specification < 1.3.0 */
        prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP = CSMD_HOTPLUG_FIELDWIDTH;
      }
      /* Pointer to RTD in next telegram */
      usTelPointer = prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP;
    }

    /* SCP_Basic: S-0-1011 "Device Status offset in AT" */
    prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].rTelegramConfig.usS_DEV_OffsetAT_S01011 =
      (CSMD_USHORT)(usTelPointer | (CSMD_USHORT)(usTelNbr << 12));

    /* Offset to next connection */
    usTelPointer += CSMD_S_DEV_LENGTH;
  }

  /* ------------------------------------------------------------------------------ */
  /* Calculate connection offset for all slave-produced AT connections (without CC) */
  /* ------------------------------------------------------------------------------ */
  for (nMConn = 0; nMConn < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; nMConn++)
  {
    usConfigIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nMConn].usConfigIdx;
    if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
    {
      /* check if connection is consumed by the master */
      if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1
           & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
      {
        usConnIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nMConn].usConnIdx;
        if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
        {
          /* exclude CC-Connections by checking if connection has been assigned yet */
          if ( !(prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3) )
          {
            usConnLength = prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5;

            /* Does connection fit into current telegram? */
            if ( (usTelPointer + usConnLength) > CSMD_SERC3_MAX_DATA_LENGTH )
            {
              if (usTelPointer < CSMD_SERC3_MIN_DATA_LENGTH)
              {
                /* Set minimum data length */
                usTelPointer = CSMD_SERC3_MIN_DATA_LENGTH;
              }
              /* Data length of telegram */
              prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usTel = usTelPointer;

              /* use next telegram for this connection */
              usTelNbr++;
              if (usTelNbr >= CSMD_MAX_TEL)
              {
                return (CSMD_TEL_NBR_AT_RTD);
              }
              if (prCSMD_Instance->rPriv.rHW_Init_Struct.boHP_Field_All_Tel == TRUE)
              {
                /* HP field in all ATs according to Sercos specification < 1.3.0 */
                prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP = CSMD_HOTPLUG_FIELDWIDTH;
              }
              /* Pointer to RTD in next telegram */
              usTelPointer = prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP;
            }

            /* SCP_Basic: S-0-1050.x.3 Telegram Assignment (AT) */
            prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 =
              (CSMD_USHORT)( CSMD_S_0_1050_SE3_TEL_NBR(usTelNbr) | usTelPointer);

            usTelPointer += usConnLength;

          } /* if ( !(prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3) ) */
        } /* if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn) */
      } /* if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1 ... */
    } /* if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig) */
  } /* for (nMConn = 0; nMConn < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; nMConn++) */

  /* --------------------------------------------------------- */
  /* Calculate AT length of last AT telegram                   */
  /* --------------------------------------------------------- */
  if (usTelPointer < CSMD_SERC3_MIN_DATA_LENGTH)
  {
    /* Set minimum data length */
    usTelPointer = CSMD_SERC3_MIN_DATA_LENGTH;
  }
  /* Data length of telegram */
  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usTel = usTelPointer;

  for (usTelNbr = 0; usTelNbr < CSMD_MAX_TEL; usTelNbr++)
  {
    /* SCP_Basic: S-0-1012, Lengths of ATs */
    prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[usTelNbr] =
      prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usTel;

    /* Length of AT RT Data */
    prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usRTD = (CSMD_USHORT)
      (prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usTel
      - (  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usHP
         + prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usSVC
         + prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usCC));
  }



#ifdef CSMD_PCI_MASTER
  prCSMD_Instance->usTX_DMA_Datalength = 0;
  prCSMD_Instance->usRX_DMA_Datalength = 0;

  for (usTelNbr = 0; usTelNbr < CSMD_MAX_TEL; usTelNbr++)
  {
    prCSMD_Instance->usTX_DMA_Datalength += (CSMD_USHORT)
      (  prCSMD_Instance->rPriv.rMDT_Length[usTelNbr].usRTD
       + prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usTel);
    prCSMD_Instance->usRX_DMA_Datalength += (CSMD_USHORT)
      (  prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usRTD
       + prCSMD_Instance->rPriv.rAT_Length[usTelNbr].usTel);
  }
#endif


  /* --------------------------------------------------------- */
  /* Specify IFG value                                         */
  /* --------------------------------------------------------- */
#if !defined CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
  /* Determine maximum slave jitter of all slaves. */
  prCSMD_Instance->rPriv.usMaxSlaveJitter = 0;
  for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++)
  {
    if (  prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].rTiming.usSlaveJitter_S1037
        > prCSMD_Instance->rPriv.usMaxSlaveJitter)
    {
      prCSMD_Instance->rPriv.usMaxSlaveJitter =
        prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].rTiming.usSlaveJitter_S1037;
    }
  }
#endif

  /* -------------------- Calculate the Inter Frame Gap --------------------- */
  /*                                                                          */
  /*                                                                          */
  /*        27 * S_0_1037_max * sqrt( 2 * N ) [us]     1 octet                */
  /*  IFG = ---------------------------------------- * --------- + 12 octets  */
  /*                   8000                            0,08 us                */
  /*                                                                          */
  /*                                                                          */
  /* ------------------------------------------------------------------------ */
  {
    CSMD_ULONG ulIFG;  /* Calculated Inter Frame Gap */

    ulIFG = (CSMD_ULONG) ( (  27 * prCSMD_Instance->rPriv.usMaxSlaveJitter
                            * ausSqrt_IFG[prCSMD_Instance->rSlaveList.usNumProjSlaves])
                          / 640000 + 12 + 1);
     /* 1 octet is added in order to round up the value */

    /* Not all slaves with SCP_Sync supports S-0-1036 and calculated IFG > 37 ? */
    if (   (prCSMD_Instance->boIFG_V1_3 == FALSE)
        && (ulIFG > CSMD_IFG_DEFAULT_CP3_CP4))
    {
      prCSMD_Instance->rPriv.ulInterFrameGap = CSMD_IFG_DEFAULT_CP3_CP4;
    }
    else
    {
      prCSMD_Instance->rPriv.ulInterFrameGap = ulIFG;
    }
    prCSMD_Instance->rConfiguration.rComTiming.usInterFrameGap_S1036 =
      (CSMD_USHORT)prCSMD_Instance->rPriv.ulInterFrameGap;

    /* Not all slaves with SCP_Sync supports S-0-1036 and calculated IFG > 37 ? */
    if (ulIFG > prCSMD_Instance->rPriv.ulInterFrameGap)
    {
      return (CSMD_WARNING_IFG_MISMATCH);
    }
  }
  return (CSMD_NO_ERROR);

} /* end: CSMD_CalculateTelegramAssignment() */



/**************************************************************************/ /**
\brief  Calculates the telegram timing

\ingroup func_timing
\b Description: \n

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]   usTimingMethod
              Position of the UC channel:\n
              - 1 (CSMD_METHOD_MDT_AT_IPC): method 1, after last AT
              - 2 (CSMD_METHOD_MDT_IPC_AT): method 2, between last MDT and first AT
              - 3 (CSMD_METHOD_AT_CYC_END): as method 2 with AT at the end of Sercos-Cycle
\param [in]   usIPChannelBandwidth
              Width of UC channel in microseconds
              - if >= 125 then length = usIPChannelBandwidth
              - if = 0 then length = 0 (no UC channel)
              - else calculate length (maximum)

\return       \ref CSMD_WARN_RECALCULATED_MTU \n
              \ref CSMD_MAX_T_NETWORK_GT_T_SYNCDELAY \n
              \ref CSMD_INVALID_ETHERNET_MTU \n
              \ref CSMD_ILLEGAL_TIMING_METHOD \n
              \ref CSMD_CYCTEL_LEN_GT_TSCYC \n
              \ref CSMD_TEL_LEN_GT_TSCYC \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         26.11.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_CalculateTimingMethod( CSMD_INSTANCE *prCSMD_Instance,
                                          CSMD_USHORT    usTimingMethod )
{

  CSMD_USHORT usI;                /* count variables */
  CSMD_USHORT usTelNbr;           /* current calculated telegram number */
  CSMD_ULONG  ulTelTimer;         /* current telegram time [ns] based on end of first MST */
  CSMD_ULONG  ulTimeMDT;          /* duration of all MDT incl. media layer overhead [ns] */
  CSMD_ULONG  ulTimeAT;           /* duration of all AT  incl. media layer overhead [ns] */
  CSMD_ULONG  ulT5_S01005;        /* Maximum Producer processing Time (t5) [ns] */
  CSMD_ULONG  ulStart_AT;         /* Start time of AT telegrams */
  CSMD_ULONG  ulTemp;
  CSMD_ULONG  ulSYNC_Jitter;
  CSMD_ULONG  ulT1_Jitter;
  CSMD_ULONG  ulT1_Min;
  CSMD_ULONG  ulT1_Max;
  CSMD_ULONG  ulT6_Min;
  CSMD_ULONG  ulT7_Max;
  CSMD_ULONG  ulUCC_Width;        /* Time width of the UC channel [ns]                   */
  CSMD_ULONG  ulDelay;            /* Delay between FPGA- and Sercos-Timing [ns]          */
  CSMD_ULONG  ulEventOffset;      /* Value of forbidden range for events [ns]            */
/*CSMD_ULONG  ulMinEventTime;*/   /* Limit for minimum event time [ns]                   */
  CSMD_ULONG  ulMaxEventTime;     /* Limit for maximum event time [ns]                   */
  CSMD_ULONG  ulProcTimeHW_SVC;   /* Processing duration of all active hardware SVC [ns] */


  /* --------------------------------------------------------- */
  /* Step 3 :: Calculate Timings                               */
  /* --------------------------------------------------------- */


  /* --------------------------------------------------------- */
  /* Calculate duration of all MDT                             */
  /* --------------------------------------------------------- */
  ulTelTimer = 0U;

  for (usTelNbr = 0; usTelNbr < CSMD_MAX_TEL; usTelNbr++)
  {
    if (prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[usTelNbr])
    {
      /* Add Time for MDT (data field + media layer overhead) */
      ulTelTimer +=
        (  CSMD_MEDIA_LAYER_OVERHEAD
         + CSMD_BYTE_TIME *
        (  (CSMD_ULONG)prCSMD_Instance->rConfiguration.rComTiming.usMDT_Length_S01010[usTelNbr]
         + prCSMD_Instance->rPriv.ulInterFrameGap) );
    }
  }
  ulTimeMDT = ulTelTimer;     /* duration of all MDT incl. media layer overheads */


  /* --------------------------------------------------------- */
  /* Calculate duration of all AT                              */
  /* --------------------------------------------------------- */
  ulTelTimer = 0U;

  for (usTelNbr = 0; usTelNbr < CSMD_MAX_TEL; usTelNbr++)
  {
    if (prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[usTelNbr])
    {
      /* Add Time for AT (data field + media layer overhead) */
      ulTelTimer +=
        (  CSMD_MEDIA_LAYER_OVERHEAD
         + CSMD_BYTE_TIME *
        (  (CSMD_ULONG)prCSMD_Instance->rConfiguration.rComTiming.usAT_Length_S01012[usTelNbr]
         + prCSMD_Instance->rPriv.ulInterFrameGap) );
    }
  }
  ulTimeAT = ulTelTimer;      /* duration of all AT incl. media layer overheads */


  /* --------------------------------------------------------- */
  /* Step 4 :: Timing depending on Timing Method               */
  /* --------------------------------------------------------- */
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
  {
    CSMD_FUNC_RET eFuncRet;
    /* Determination of S-0-1015 "Ring delay" */
    eFuncRet = CSMD_Determination_Ringdelay( prCSMD_Instance,
                                             CSMD_CALC_RD_MODE_NORMAL,
                                             &prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015,
                                             &prCSMD_Instance->rConfiguration.rComTiming.ulRingDelay_S01015_P2 );
    if (eFuncRet != CSMD_NO_ERROR)
    {
      return (eFuncRet);
    }
  }
  CSMD_Calculate_RingDelay( prCSMD_Instance );            /* calculation of ring delay independent of measured delay */
  /* In CSMD_Calculate_RingDelay() wird ulCalc_RingDelay = ulTSref gesetzt. */
#else
  /* The calculated stable ring delay shall be greater than the
   * maximum network delay represented by TSref. */
  if (  prCSMD_Instance->rPriv.rRingDelay.ulTSref
      > prCSMD_Instance->rConfiguration.rComTiming.ulCalc_RingDelay )
  {
    prCSMD_Instance->rConfiguration.rComTiming.ulCalc_RingDelay =
      prCSMD_Instance->rPriv.rRingDelay.ulTSref;
  }
#endif
  
  /* Timing mode */
  if (prCSMD_Instance->rPriv.rCycClk.boActivate == TRUE)
  {
    /* Timing slave mode */
    ulEventOffset = prCSMD_Instance->rConfiguration.rMasterCfg.ulJitter_Master;
  }
  else
  {
    /* Timing Master Mode */
    ulEventOffset = 0U;
  }

  /* Offset plus time displacement between FPGA- and Sercos-Timing */
  ulDelay = ulEventOffset + CSMD_MST_DELAY;
  prCSMD_Instance->rPriv.ulOffsetTNCT_SERCCycle = ulDelay;

  /* Limit for minimum event time */
  /*  ulMinEventTime = ulEventOffset + (CSMD_ULONG) (CSMD_FPGA_MDT_START_TIME + 100); */
  prCSMD_Instance->ulMinTime = 0U;

  /* Limit for maximum event time */
  ulMaxEventTime =   prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002
    - ulEventOffset;
  prCSMD_Instance->ulMaxTime =   ulMaxEventTime
    - (ulEventOffset + CSMD_MST_DELAY );


  /* SCP Sync:    S-0-1023, SYNC jitter */
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
  /* S-0-1023 = J_Sync = (J_MST * J_IFG) / 2 */
  prCSMD_Instance->rConfiguration.rComTiming.ulSyncJitter_S01023 =
    (  prCSMD_Instance->rConfiguration.rMasterCfg.ulJitter_Master
     + prCSMD_Instance->rPriv.ulInterFrameGap * CSMD_BYTE_TIME) / 2;
#else
  ulTemp = 0U;
  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    ulTemp += prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTiming.usSlaveJitter_S1037;
  }
  prCSMD_Instance->rConfiguration.rComTiming.ulSyncJitter_S01023 =
    prCSMD_Instance->rConfiguration.rMasterCfg.ulJitter_Master + 2U * ulTemp;
#endif

  ulSYNC_Jitter = prCSMD_Instance->rConfiguration.rComTiming.ulSyncJitter_S01023;
  ulT1_Jitter = ulSYNC_Jitter;

  /* Minimum time of t1 (t1min) */
  ulT1_Min = ulTimeMDT + ulT1_Jitter;

  if (ulT1_Min < prCSMD_Instance->rConfiguration.rComTiming.ulMinTimeStartAT)
    ulT1_Min = prCSMD_Instance->rConfiguration.rComTiming.ulMinTimeStartAT;

  /* Processing duration of all active hardware SVC [ns] (for SERCON100M: max 32 * 400ns) */
  if (prCSMD_Instance->rSlaveList.usNumProjSlaves > CSMD_MAX_HW_CONTAINER)
  {
    ulProcTimeHW_SVC = CSMD_MAX_HW_CONTAINER * CSMD_FPGA_HW_SVC_PROC_TIME;
  }
  else
  {
    ulProcTimeHW_SVC = (CSMD_ULONG)prCSMD_Instance->rSlaveList.usNumProjSlaves * CSMD_FPGA_HW_SVC_PROC_TIME;
  }

  /* Maximum time of t1 (t1max) */
  /* Guarantee end of hardware svc state machine before start sending of MDT */
  if ((ulT1_Jitter + ulDelay) > ulProcTimeHW_SVC)
  {
    ulT1_Max =
      prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002
      - (ulTimeAT + ulT1_Jitter + ulDelay + prCSMD_Instance->rConfiguration.rComTiming.ulCalc_RingDelay);
  }
  else
  {
    ulT1_Max =
      prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002
      - (ulTimeAT + ulProcTimeHW_SVC + prCSMD_Instance->rConfiguration.rComTiming.ulCalc_RingDelay);
  }

  ulUCC_Width = prCSMD_Instance->rPriv.ulUCC_Width;


  /* ----------------------------------------------------------- */
  /*                                                             */
  /*  Timing method 1             (MDT / AT / UCC)               */
  /*                                                             */
  /* ----------------------------------------------------------- */
  if (usTimingMethod == CSMD_METHOD_MDT_AT_IPC)
  {
    ulT6_Min =   ulT1_Min
               + ulT1_Jitter
               + ulTimeAT;

    ulT7_Max =   prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002
               - (  ulSYNC_Jitter
                  + CSMD_TIME_SIII_HEADER
                  + ulDelay );

    if ((ulT6_Min + ulUCC_Width) > ulT7_Max)
    {
      return (CSMD_TEL_LEN_GT_TSCYC);
    }

    /* SCP Sync:    S-0-1006, AT transmission starting time (t1) */
    ulStart_AT = ulT1_Min;


    /* ----------------------------------------------------------- */
    /*  0                                                   tscyc  */
    /*  |+--------+ +--------+          +----------+          |    */
    /*  ||  MDTs  | |   ATs  |          |UC channel|          |    */
    /* -++--------+-+--------+----------+----------+----------+--- */

    if (ulUCC_Width >= CSMD_IPC_MIN_TIME)
    {
      /* S-0-1017, Begin of the UC channel (t6) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017 =
        ulT6_Min
        + (ulT7_Max - ulT6_Min) / 2
        - ulUCC_Width / 2;

      /* S-0-1017, End of the UC channel (t7) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 =
        prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017
        + ulUCC_Width;
    }


    /* ----------------------------------------------------------- */
    /*  0                                                   tscyc  */
    /*  |+--------+ +--------+ +-----------------------------+|    */
    /*  ||  MDTs  | |   ATs  | |         UC channel          ||    */
    /* -++--------+-+--------+-+-----------------------------++--- */

    else if (ulUCC_Width > 0U)      /* Calculate UC channel bandwidth */
    {
      /* S-0-1017, Begin of the UC channel (t6) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017 =
        ulT6_Min;

      /* S-0-1017, End of the UC channel (t7) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 =
        ulT7_Max;

      ulUCC_Width = ulT7_Max - ulT6_Min;
      prCSMD_Instance->rPriv.ulUCC_Width = ulUCC_Width;
    }


    /* ----------------------------------------------------------- */
    /*  0                                                   tscyc  */
    /*  |+--------+ +--------+                                |    */
    /*  ||  MDTs  | |   ATs  |                                |    */
    /* -++--------+-+--------+--------------------------------+--- */

    else
    {
      /* S-0-1017, Begin of the UC channel (t6) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017 =
        0U;

      /* S-0-1017, End of the UC channel (t7) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 =
        ulT6_Min
        + (ulT7_Max - ulT6_Min) / 2;
    }
  }


  /* ----------------------------------------------------------- */
  /*                                                             */
  /*  Timing method 2             (MDT / UCC / AT)               */
  /*                                                             */
  /* ----------------------------------------------------------- */
  else if (usTimingMethod == CSMD_METHOD_MDT_IPC_AT)
  {
    ulT6_Min =
      ulTimeMDT
      + ulSYNC_Jitter
      - CSMD_TIME_SIII_HEADER;

    ulT7_Max =
      ulT1_Max
      - (  ulT1_Jitter
         + CSMD_TIME_SIII_HEADER );

    if ((ulT6_Min + ulUCC_Width) > ulT7_Max)
    {
      return (CSMD_TEL_LEN_GT_TSCYC);
    }


    /* ----------------------------------------------------------- */
    /*  0                                                   tscyc  */
    /*  |+--------+ +----------+ +---------+                  |    */
    /*  ||  MDTs  | |UC channel| |   ATs   |                  |    */
    /* -++--------+-+----------+-+---------+------------------+--- */

    if (ulUCC_Width >= CSMD_IPC_MIN_TIME)
    {
      /* SCP Sync:    S-0-1006, AT transmission starting time (t1) */
      ulStart_AT =   ulT6_Min
        + ulUCC_Width
        + ulT1_Jitter
        + CSMD_TIME_SIII_HEADER
        + prCSMD_Instance->rConfiguration.rComTiming.ulCalc_RingDelay;

      if (ulStart_AT < prCSMD_Instance->rConfiguration.rComTiming.ulMinTimeStartAT)
        ulStart_AT = prCSMD_Instance->rConfiguration.rComTiming.ulMinTimeStartAT;

      /* S-0-1017, Begin of the UC channel (t6) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017 =
        ulT6_Min;

      /* S-0-1017, End of the UC channel (t7) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 =
        ulT6_Min + ulUCC_Width;
    }


    /* ----------------------------------------------------------- */
    /*  0                                                   tscyc  */
    /*  |+--------+ +-----------------------------+ +--------+|    */
    /*  ||  MDTs  | |         UC channel          | |   ATs  ||    */
    /* -++--------+-+-----------------------------+-+--------++--- */

    else if (ulUCC_Width > 0U)      /* Calculate UC channel bandwidth */
    {
      /* SCP Sync:    S-0-1006, AT transmission starting time (t1) */
      ulStart_AT = ulT1_Max;

      /* S-0-1017, Begin of the UC channel (t6) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017 =
        ulT6_Min;

      /* S-0-1017, End of the UC channel (t7) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 =
        ulT7_Max;

      ulUCC_Width = ulT7_Max - ulT6_Min;
      prCSMD_Instance->rPriv.ulUCC_Width = ulUCC_Width;
    }


    /* ----------------------------------------------------------- */
    /*  0                                                   tscyc  */
    /*  |+--------+ +--------+                                |    */
    /*  ||  MDTs  | |   ATs  |                                |    */
    /* -++--------+-+--------+--------------------------------+--- */

    else
    {
      /* SCP Sync:    S-0-1006, AT transmission starting time (t1) */
      ulStart_AT = ulT1_Min;

      /* S-0-1017, Begin of the UC channel (t6) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017 =
        0U;

      /* S-0-1017, End of the UC channel (t7) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 =
        ulT6_Min
        + (ulT7_Max - ulT6_Min) / 2;
    }
  }


  /* ----------------------------------------------------------- */
  /*                                                             */
  /*  Timing method 3    (method 2 with AT at the end of cycle)  */
  /*                                                             */
  /* ----------------------------------------------------------- */
  else if (usTimingMethod == CSMD_METHOD_AT_CYC_END)
  {
    ulT6_Min =
      ulTimeMDT
      - CSMD_TIME_SIII_HEADER;

    ulT7_Max =
      ulT1_Max
      - (  ulT1_Jitter
         + CSMD_TIME_SIII_HEADER );

    if ((ulT6_Min + ulUCC_Width) > ulT7_Max)
    {
      return (CSMD_TEL_LEN_GT_TSCYC);
    }

    /* SCP Sync:    S-0-1006, AT transmission starting time (t1) */
    ulStart_AT = ulT1_Max;


    /* ----------------------------------------------------------- */
    /*  0                                                   tscyc  */
    /*  |+--------+          +----------+          +---------+|    */
    /*  ||  MDTs  |          |UC channel|          |   ATs   ||    */
    /* -++--------+----------+----------+----------+---------++--- */

    if (ulUCC_Width >= CSMD_IPC_MIN_TIME)
    {
      /* S-0-1017, Begin of the UC channel (t6) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017 =
        ulT6_Min
        + (ulT7_Max - ulT6_Min) / 2
        - ulUCC_Width / 2;

      /* S-0-1017, End of the UC channel (t7) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 =
        prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017
        + ulUCC_Width;
    }


    /* ----------------------------------------------------------- */
    /*  0                                                   tscyc  */
    /*  |+--------+ +-----------------------------+ +--------+|    */
    /*  ||  MDTs  | |         UC channel          | |   ATs  ||    */
    /* -++--------+-+-----------------------------+-+--------++--- */

    else if (ulUCC_Width > 0U)      /* Calculate UC channel bandwidth */
    {
      /* S-0-1017, Begin of the UC channel (t6) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017 =
        ulT6_Min;

      /* S-0-1017, End of the UC channel (t7) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 =
        ulT7_Max;

      ulUCC_Width = ulT7_Max - ulT6_Min;
      prCSMD_Instance->rPriv.ulUCC_Width = ulUCC_Width;
    }


    /* ----------------------------------------------------------- */
    /*  0                                                   tscyc  */
    /*  |+--------+                                 +--------+|    */
    /*  ||  MDTs  |                                 |   ATs  ||    */
    /* -++--------+------------------------------ --+--------++--- */

    else
    {
      /* S-0-1017, Begin of the UC channel (t6) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017 =
        0U;

      /* S-0-1017, End of the UC channel (t7) */
      prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 =
        ulT6_Min
        + (ulT7_Max - ulT6_Min) / 2;
    }
  }

  else
  {
    return (CSMD_ILLEGAL_TIMING_METHOD);
  }



  /* Duration of all MDTs + ATs + Gaps */
  ulTelTimer =
    ulTimeMDT + ulTimeAT;

  if (ulTelTimer > prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002)
  {
    return (CSMD_CYCTEL_LEN_GT_TSCYC);
  }

  prCSMD_Instance->rPriv.usRequested_MTU =
    prCSMD_Instance->rPriv.rHW_Init_Struct.usIP_MTU_P34;

  if (ulUCC_Width > 0)
  {
    /* Duration of all MDTs + ATs + IPC + Gaps */
    if (  (prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 - prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017)
        > (prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 - ulTelTimer))
    {
      return (CSMD_TEL_LEN_GT_TSCYC);
    }

    /* Maximum length [ns] of an Ethernet frame depending of the MTU */
    ulTemp = CSMD_IP_MAC_LAYER_OVERHEAD +
             (CSMD_ULONG) ( (CSMD_ULONG)prCSMD_Instance->rPriv.usRequested_MTU *
                            CSMD_BYTE_TIME);

    if (   (prCSMD_Instance->rPriv.usRequested_MTU < CSMD_ETHERNET_MTU_MIN)
        || (prCSMD_Instance->rPriv.usRequested_MTU > CSMD_ETHERNET_MTU_MAX) )
    {
      return (CSMD_INVALID_ETHERNET_MTU);
    }

    if (ulUCC_Width < ulTemp)
    {
      /* calculate new MTU */
      prCSMD_Instance->rPriv.usRequested_MTU =
        (CSMD_USHORT) ((ulUCC_Width - CSMD_IP_MAC_LAYER_OVERHEAD) / CSMD_BYTE_TIME);

      /* define maximum length [ns] of an Ethernet frame again */
      ulTemp = CSMD_IP_MAC_LAYER_OVERHEAD +
               (CSMD_ULONG) ( (CSMD_ULONG)prCSMD_Instance->rPriv.usRequested_MTU *
                              CSMD_BYTE_TIME);
    }

    /* Latest transmission start time in the UC channel */
    prCSMD_Instance->rConfiguration.rUC_Channel.ulLatestTransmissionUC =
      prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017 - ulTemp;
  }



  /* Get max. Value of S-0-1005, Maximum Producer processing Time (t5) */
  ulT5_S01005 = 0U;

  for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
  {
    if (prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTiming.ulMinFdbkProcTime_S01005 > ulT5_S01005)
    {
      ulT5_S01005 = prCSMD_Instance->rConfiguration.parSlaveConfig[usI].rTiming.ulMinFdbkProcTime_S01005;
    }
  }


  /* --------------------------------------------------------- */
  /*                                                           */
  /* --------------------------------------------------------- */

  /* SCP Sync:    S-0-1006, AT transmission starting time (t1) */
  prCSMD_Instance->rConfiguration.rComTiming.ulATTxStartTimeT1_S01006 =
    ulStart_AT;


  /* SCP_Sync:    S-0-1008 "Command value valid time (t3)" */
  prCSMD_Instance->rConfiguration.rComTiming.ulCmdValidTimeT3_S01008 =
    ulTimeMDT
    + prCSMD_Instance->rConfiguration.rComTiming.ulCalc_RingDelay
    - CSMD_TIME_SIII_HEADER;


  /* SCP_Sync:    S-0-1007 Synchronization time (tSync) */
  {
    CSMD_LONG  lTSync;
    lTSync =   (CSMD_LONG)prCSMD_Instance->rConfiguration.rComTiming.ulATTxStartTimeT1_S01006
             - (CSMD_LONG)(prCSMD_Instance->rPriv.rRingDelay.ulTSref + ulT5_S01005);
    while (lTSync < 0)
    {
      lTSync += (CSMD_LONG)prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002;
    }
    prCSMD_Instance->rConfiguration.rComTiming.ulSynchronizationTime_S01007 = (CSMD_ULONG)lTSync;
  }


  /* Time for interrupt to read AT */
  prCSMD_Instance->rConfiguration.rComTiming.ulEndofAT =
    ulStart_AT
    + ulTimeAT
    + prCSMD_Instance->rConfiguration.rComTiming.ulCalc_RingDelay;

  if (   prCSMD_Instance->rConfiguration.rComTiming.ulEndofAT
      >= prCSMD_Instance->ulMaxTime)
  {
    /* Next Sercos cycle */
    prCSMD_Instance->rConfiguration.rComTiming.ulEndofAT =
      prCSMD_Instance->ulMinTime;
  }


  /* Event_Rx_Buffer_Request_Buffer_System_A */
  /* Set default time and state to "not configured" */
  prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A]       = ulStart_AT + ulTimeAT;
  prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A] = FALSE;


  /* Event_Tx_Buffer_Request_Buffer_System_A (immediate behind start MDT event) */
  /* Set default time and state to "not configured" */
  prCSMD_Instance->rPriv.rCSMD_Event.aboConfigured[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A] = FALSE;
#if defined CSMD_PCI_MASTER && defined CSMD_PCI_MASTER_EVENT_DMA
  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A] =
        prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002
      - CSMD_DMA_TXRAM_COPYTIME_CP34_DEF;
  }
  else
#endif
  {
    if (prCSMD_Instance->rPriv.rHW_Init_Struct.usTxBufferMode >= CSMD_TX_DOUBLE_BUFFER)
    {
      prCSMD_Instance->rPriv.rCSMD_Event.aulTime[CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A] =
        CSMD_FPGA_MDT_START_TIME + 100UL;
    }
  }

  /*

      Regeln fuer t3, tSync, t9:

        tmax < S1002 - S1023    (Tscyc - SyncJitter)

  */

  /* Warning, if given MTU is recalculated due to shortened UCC bandwidth */
  if (   prCSMD_Instance->rPriv.usRequested_MTU
      != prCSMD_Instance->rPriv.rHW_Init_Struct.usIP_MTU_P34)
  {
    return (CSMD_WARN_RECALCULATED_MTU);
  }

  return (CSMD_NO_ERROR);

} /* end: CSMD_CalculateTimingMethod() */



/**************************************************************************/ /**
\brief Checks the connection configuration.
 
\ingroup func_config
\b Description:
    - Checks indices in master configuration
    - Checks indices in configuration of all slaves
    - Checks if connection numbers are unique
    - Checks if consumed connections are produced once
    - Checks if produced connections are consumed
    - Checks if producer cycle times are valid
    - Checks if slaves produce in AT telegram only

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_TOO_MANY_MASTER_CONNECTIONS \n
              \ref CSMD_TOO_MANY_SLAVE_CONNECTIONS \n
              \ref CSMD_WRONG_CONNECTION_INDEX \n
              \ref CSMD_CONFIGURATION_NOT_CONFIGURED \n
              \ref CSMD_WRONG_CONFIGURATION_INDEX \n
              \ref CSMD_WRONG_RTBIT_CONFIGURATION_INDEX \n
              \ref CSMD_WRONG_SCP_CAP_CONFIGURATION \n
              \ref CSMD_NO_UNIQUE_CON_NBR \n
              \ref CSMD_CONN_MULTIPLE_PRODUCED \n
              \ref CSMD_MASTER_CONSUME_IN_MDT \n
              \ref CSMD_MASTER_PROD_CONN_IN_AT \n
              \ref CSMD_CONNECTION_WRONG_TEL_TYPE \n
              \ref CSMD_SLAVE_PRODUCE_IN_MDT \n
              \ref CSMD_CONNECTION_NOT_CONSUMED \n
              \ref CSMD_CONNECTION_NOT_PRODUCED \n
              \ref CSMD_CONNECTION_LENGTH_0 \n
              \ref CSMD_PROD_CYC_TIME_INVALID \n
              \ref CSMD_NO_ERROR \n
      
\author  WK
\date    15.03.2011

***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckConfiguration( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_INT    nSlaves;          /* Loop counter projected slaves */
  CSMD_USHORT usJ;              /* count variable */
  CSMD_USHORT usCIL_Idx;        /* Index in "Connection index list" in master- or slave- configuration */
  CSMD_USHORT usConnIdx;        /* Index in "Connections list"    */
  CSMD_USHORT usConfigIdx;      /* Index in "Configurations list" */
  CSMD_USHORT usRTCfgIdx;       /* Index in "Realtime Bit Configurations list" */
  
  /* temp variables */
  CSMD_USHORT usNumCon;         /* Number of connections of a slave configuration */
  CSMD_USHORT usConNbrMaster;   /* Number of configured master connections */
  CSMD_USHORT usTelType;        /* Telegram type (MDT / AT) */
  CSMD_ULONG  ultpcyc;          /* Producer Cycle Time    */
  CSMD_BOOL   boProd;           /* Producer for selcted connection was found */
  CSMD_BOOL   boCons;           /* Consumer for selcted connection was found */
  
  CSMD_CONN_IDX_STRUCT *parIdxListM;    /* Pointer to connection-index-list of master */
  CSMD_CONN_IDX_STRUCT *parIdxList;     /* Pointer to connection-index-list of selected slave */
  CSMD_CONNECTION      *parConnections; /* Pointer to connection-list */
  CSMD_CONFIGURATION   *parConfigs;     /* Pointer to configuration-list */

  CSMD_USHORT usConNbrCount = 0;             /* Counter for ausConNbr[] */
  CSMD_USHORT ausConNbr[CSMD_MAX_GLOB_CONN]; /* Connection numbers of used produced connections */
  
  const CSMD_CONFIG_ERROR crCSMD_ConfigErrorInit = { 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF };

  
  parIdxListM    = &prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[0];
  parConnections = &prCSMD_Instance->rConfiguration.parConnection[0];
  parConfigs     = &prCSMD_Instance->rConfiguration.parConfiguration[0];
  prCSMD_Instance->rConfig_Error = crCSMD_ConfigErrorInit;
  
  
#if(0)  
// Test: Defdb00155249  ConnIdx & ConfiIdx sind gültig, 1050.x.SE1 = 0 --> Verbindung soll nicht geprüft wreden!
  /* ------------------------------------------------------------------------ */
  /* Check index list of master configuration                                 */
  /* ------------------------------------------------------------------------ */
  usConNbrMaster = prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster;
  if (prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections > prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster)
  {
    return (CSMD_TOO_MANY_MASTER_CONNECTIONS);
  }
  
  for ( usCIL_Idx = 0; usCIL_Idx < usConNbrMaster; usCIL_Idx++ )
  {
    usConnIdx   = parIdxListM[usCIL_Idx].usConnIdx;
    usConfigIdx = parIdxListM[usCIL_Idx].usConfigIdx;
    usRTCfgIdx  = parIdxListM[usCIL_Idx].usRTBitsIdx;
    
    prCSMD_Instance->rConfig_Error.usSlaveIndex    = prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves;
    prCSMD_Instance->rConfig_Error.usConnectionIdx = usCIL_Idx;
    
    if (   (usConnIdx   == 0xFFFF)
        && (usConfigIdx == 0xFFFF))
    {
      /* No connection configured */
    }
    else
    {
      if (usConnIdx >= prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
      {
        return (CSMD_WRONG_CONNECTION_INDEX);
      }
      
      /* Telegram type not MDT or AT? */
      if (   (parConnections[usConnIdx].usTelegramType != CSMD_TELEGRAM_TYPE_MDT)
          && (parConnections[usConnIdx].usTelegramType != CSMD_TELEGRAM_TYPE_AT))
      {
        return (CSMD_CONNECTION_WRONG_TEL_TYPE);
      }
      
      if (usConfigIdx >= prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
      {
        return (CSMD_WRONG_CONFIGURATION_INDEX);
      }
      
      if (   (usRTCfgIdx >= prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig)
          && (usRTCfgIdx != 0xFFFF))
      {
        return (CSMD_WRONG_RTBIT_CONFIGURATION_INDEX);
      }
    }
  }
  
  
  /* ------------------------------------------------------------------------ */
  /* Check index list of all slave configurations                             */
  /* ------------------------------------------------------------------------ */
  for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++)
  {
    parIdxList  = &prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].arConnIdxList[0];
    usNumCon    = CSMD_MAX_CONNECTIONS;
    prCSMD_Instance->rConfig_Error.usSlaveIndex = nSlaves;
    
    if (  prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].usNbrOfConnections
        > CSMD_MAX_CONNECTIONS)
    {
      return (CSMD_TOO_MANY_SLAVE_CONNECTIONS);
    }
    
    /* Over all connections of the slave */
    for (usCIL_Idx = 0; usCIL_Idx < usNumCon; usCIL_Idx++)
    {
      usConnIdx   = parIdxList[usCIL_Idx].usConnIdx;
      usConfigIdx = parIdxList[usCIL_Idx].usConfigIdx;
      usRTCfgIdx  = parIdxList[usCIL_Idx].usRTBitsIdx;
      prCSMD_Instance->rConfig_Error.usConnectionIdx  = usCIL_Idx;
      
      if (   (usConnIdx   == 0xFFFF)
          && (usConfigIdx == 0xFFFF))
      {
        /* No connection configured */
      }
      else
      {
        if (usConnIdx >= prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
        {
          return (CSMD_WRONG_CONNECTION_INDEX);
        }
        
        /* Telegram type not MDT or AT? */
        if (   (parConnections[usConnIdx].usTelegramType != CSMD_TELEGRAM_TYPE_MDT)
            && (parConnections[usConnIdx].usTelegramType != CSMD_TELEGRAM_TYPE_AT))
        {
          return (CSMD_CONNECTION_WRONG_TEL_TYPE);
        }
        
        if (usConfigIdx >= prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
        {
          return (CSMD_WRONG_CONFIGURATION_INDEX);
        }
        
        if (   (usRTCfgIdx >= prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig)
            && (usRTCfgIdx != 0xFFFF))
        {
          return (CSMD_WRONG_RTBIT_CONFIGURATION_INDEX);
        }
      }
    }
  }


#else

  /* ------------------------------------------------------------------------ */
  /* Check index list of master configuration                                 */
  /* ------------------------------------------------------------------------ */
  usConNbrMaster = prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster;
  if (prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections > prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster)
  {
    return (CSMD_TOO_MANY_MASTER_CONNECTIONS);
  }
  
  for ( usCIL_Idx = 0; usCIL_Idx < usConNbrMaster; usCIL_Idx++ )
  {
    usConnIdx   = parIdxListM[usCIL_Idx].usConnIdx;
    usConfigIdx = parIdxListM[usCIL_Idx].usConfigIdx;
    usRTCfgIdx  = parIdxListM[usCIL_Idx].usRTBitsIdx;
    
    prCSMD_Instance->rConfig_Error.usSlaveIndex    = prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves;
    prCSMD_Instance->rConfig_Error.usConnectionIdx = usCIL_Idx;
    
    if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
    {
      /* Telegram type not MDT or AT? */
      if (   (parConnections[usConnIdx].usTelegramType != CSMD_TELEGRAM_TYPE_MDT)
          && (parConnections[usConnIdx].usTelegramType != CSMD_TELEGRAM_TYPE_AT))
      {
        return (CSMD_CONNECTION_WRONG_TEL_TYPE);
      }
      
      if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
      {
        /* Used connection? */
        if (!(parConfigs[usConfigIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE))
        {
          return (CSMD_CONFIGURATION_NOT_CONFIGURED);
        }
      }
      else
      {
        return (CSMD_WRONG_CONFIGURATION_INDEX);
      }
      
      if (   (usRTCfgIdx >= prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig)
          && (usRTCfgIdx != 0xFFFF))
      {
        return (CSMD_WRONG_RTBIT_CONFIGURATION_INDEX);
      }
    }
    else if (usConnIdx != 0xFFFF)
    {
      return (CSMD_WRONG_CONNECTION_INDEX);
    }
  }
  
  
  /* ------------------------------------------------------------------------ */
  /* Check index list of all slave configurations                             */
  /* ------------------------------------------------------------------------ */
  for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++)
  {
    parIdxList  = &prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].arConnIdxList[0];
    usNumCon    = CSMD_MAX_CONNECTIONS;
    prCSMD_Instance->rConfig_Error.usSlaveIndex = (CSMD_USHORT)nSlaves;
    
    if (  prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].usNbrOfConnections
        > CSMD_MAX_CONNECTIONS)
    {
      return (CSMD_TOO_MANY_SLAVE_CONNECTIONS);
    }
    
    /* Over all connections of the slave */
    for (usCIL_Idx = 0; usCIL_Idx < usNumCon; usCIL_Idx++)
    {
      usConnIdx   = parIdxList[usCIL_Idx].usConnIdx;
      usConfigIdx = parIdxList[usCIL_Idx].usConfigIdx;
      usRTCfgIdx  = parIdxList[usCIL_Idx].usRTBitsIdx;
      prCSMD_Instance->rConfig_Error.usConnectionIdx = usCIL_Idx;
      
      if (usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
      {
        /* Telegram type not MDT or AT? */
        if (   (parConnections[usConnIdx].usTelegramType != CSMD_TELEGRAM_TYPE_MDT)
            && (parConnections[usConnIdx].usTelegramType != CSMD_TELEGRAM_TYPE_AT))
        {
          return (CSMD_CONNECTION_WRONG_TEL_TYPE);
        }
        
        if (usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
        {
          /* Used connection? */
          if (!(parConfigs[usConfigIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE))
          {
            return (CSMD_CONFIGURATION_NOT_CONFIGURED);
          }
        }
        else
        {
          return (CSMD_WRONG_CONFIGURATION_INDEX);
        }
        
        if (   (usRTCfgIdx >= prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig)
            && (usRTCfgIdx != 0xFFFF))
        {
          return (CSMD_WRONG_RTBIT_CONFIGURATION_INDEX);
        }
      }
      else if (usConnIdx != 0xFFFF)
      {
        return (CSMD_WRONG_CONNECTION_INDEX);
      }
    }
  }
#endif
  
  
  /* ------------------------------------------------------------------------ */
  /* Check assignment to the connection list                                  */
  /* ------------------------------------------------------------------------ */
  for (usConnIdx = 0; usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn; usConnIdx++)
  {
    prCSMD_Instance->rConfig_Error = crCSMD_ConfigErrorInit;
    boProd    = FALSE;
    boCons    = FALSE;
    usTelType = parConnections[usConnIdx].usTelegramType;
    
    /* Master connections */
    for ( usCIL_Idx = 0; usCIL_Idx < usConNbrMaster; usCIL_Idx++ )
    {
      if (parIdxListM[usCIL_Idx].usConnIdx == usConnIdx)
      {
        usConfigIdx = parIdxListM[usCIL_Idx].usConfigIdx;
        
        if (usTelType == CSMD_TELEGRAM_TYPE_MDT)
        {
          if (   (parConfigs[ usConfigIdx ].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) 
              == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
          {
            if (boProd == FALSE)
            {
              boProd = TRUE;
              /* Produced connection found */
              prCSMD_Instance->rConfig_Error.usSlaveIndex    = prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves;
              prCSMD_Instance->rConfig_Error.usConnectionIdx = usCIL_Idx;
            }
            else
            {
              return (CSMD_CONN_MULTIPLE_PRODUCED);
            }
          }
          else if (   (parConfigs[ usConfigIdx ].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) 
                   == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
          {
            return (CSMD_MASTER_CONSUME_IN_MDT);
          }
        } /* end: if (usTelType == CSMD_TELEGRAM_TYPE_MDT) */ 
        
        else if (usTelType == CSMD_TELEGRAM_TYPE_AT)
        {
          if (   (parConfigs[ usConfigIdx ].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) 
              == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
          {
#ifdef CSMD_MASTER_PRODUCE_IN_AT
            if (boProd == FALSE)
            {
              boProd = TRUE;
              /* Produced connection found */
              prCSMD_Instance->rConfig_Error.usSlaveIndex    = prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves;
              prCSMD_Instance->rConfig_Error.usConnectionIdx = usCIL_Idx;
            }
            else
            {
              return (CSMD_CONN_MULTIPLE_PRODUCED);
            }
#else
            return (CSMD_MASTER_PROD_CONN_IN_AT);
#endif
          }
          else if (   (parConfigs[ usConfigIdx ].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) 
                   == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
          {
            boCons = TRUE;
            /* Consumed connection found */
            prCSMD_Instance->rConfig_Error.usSlaveIndex2    = prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves;
            prCSMD_Instance->rConfig_Error.usConnectionIdx2 = usCIL_Idx;
          }
        } /* end: else if (usTelType == CSMD_TELEGRAM_TYPE_AT) */
        
        else
        {
          return (CSMD_CONNECTION_WRONG_TEL_TYPE);
        }
      } /* end: if (parIdxListM[usCIL_Idx].usConnIdx == usConnIdx) */
    }   /* end: for ( usCIL_Idx = 0; usCIL_Idx < usConNbrMaster; usCIL_Idx++ ) */
    
    
    /* Slave connections */
    usNumCon = CSMD_MAX_CONNECTIONS;
    for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++)
    {
      parIdxList  = &prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].arConnIdxList[0];
      
      for ( usCIL_Idx = 0; usCIL_Idx < usNumCon; usCIL_Idx++ )
      {
        if (parIdxList[usCIL_Idx].usConnIdx == usConnIdx)
        {
          usConfigIdx = parIdxList[usCIL_Idx].usConfigIdx;

          /* Check if a connection capability is configured and the */
          /* slave does not support SCP_CAP                         */
          if (   (parConfigs[ usConfigIdx ].usS_0_1050_SE7 != 0xFFFF)
              && (!(prCSMD_Instance->rPriv.aulSCP_Config[nSlaves] & CSMD_SCP_CAP)))
          {
            /* Error: Capability is configured but slave doesn't support this */
            return (CSMD_WRONG_SCP_CAP_CONFIGURATION);
          }
          
          if (usTelType == CSMD_TELEGRAM_TYPE_MDT)
          {
            if (   (parConfigs[ usConfigIdx ].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) 
                == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
            {
              return (CSMD_SLAVE_PRODUCE_IN_MDT);
            }
            else if (   (parConfigs[ usConfigIdx ].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) 
                     == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
            {
              boCons = TRUE;
              /* Consumed connection found */
              prCSMD_Instance->rConfig_Error.usSlaveIndex2    = (CSMD_USHORT)nSlaves;
              prCSMD_Instance->rConfig_Error.usConnectionIdx2 = usCIL_Idx;
            }
          } /* end: if (usTelType == CSMD_TELEGRAM_TYPE_MDT) */ 
          
          else if (usTelType == CSMD_TELEGRAM_TYPE_AT)
          {
            if (   (parConfigs[ usConfigIdx ].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) 
                == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
            {
              if (boProd == FALSE)
              {
                boProd = TRUE;
                /* Produced connection found */
                prCSMD_Instance->rConfig_Error.usSlaveIndex    = (CSMD_USHORT)nSlaves;
                prCSMD_Instance->rConfig_Error.usConnectionIdx = usCIL_Idx;
              }
              else
              {
                return (CSMD_CONN_MULTIPLE_PRODUCED);
              }
            }
            else if (   (parConfigs[ usConfigIdx ].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) 
                     == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
            { 
              boCons = TRUE;
              /* Produced connection found */
              prCSMD_Instance->rConfig_Error.usSlaveIndex2    = (CSMD_USHORT)nSlaves;
              prCSMD_Instance->rConfig_Error.usConnectionIdx2 = usCIL_Idx;
            }
          } /* end: else if (usTelType == CSMD_TELEGRAM_TYPE_AT) */
          
          else
          {
            return (CSMD_CONNECTION_WRONG_TEL_TYPE);
          }
        } /* end: if (parIdxList[usCIL_Idx].usConnIdx == usConnIdx) */
      }   /* end: for ( usCIL_Idx = 0; usCIL_Idx < usNumCon; usCIL_Idx++ ) */
    }     /* end: for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++) */
    
    if ((boProd == TRUE) && (boCons == FALSE))
    {
      return (CSMD_CONNECTION_NOT_CONSUMED);
    }
    
    if ((boProd == FALSE) && (boCons == TRUE))
    {
      return (CSMD_CONNECTION_NOT_PRODUCED);
    }
    
    /* Used connection */
    if (boProd == TRUE)
    {
      /* Check length of the connection */
      if (parConnections[usConnIdx].usS_0_1050_SE5 == 0)
      {
        return (CSMD_CONNECTION_LENGTH_0);
      }

      /* check producer cycle time */
      ultpcyc = parConnections[usConnIdx].ulS_0_1050_SE10;
             /* tpcyc < Sercos cycle time */
      if (   (   (ultpcyc < prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002)
              && (ultpcyc != 0))
             /* tpcyc is no integer multiple of the Sercos cycle time */
          || (ultpcyc % prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002) )
      {
        return (CSMD_PROD_CYC_TIME_INVALID);
      }
      
      /* Check for unique connection numbers */
      for (usJ = 0; usJ < usConNbrCount; usJ++)
      {
        if (ausConNbr[usJ] == parConnections[usConnIdx].usS_0_1050_SE2) /*lint !e530 Symbol 'ausConNbr' not initialized */
        {
          return (CSMD_NO_UNIQUE_CON_NBR);
        }
      }
      /* Take connection number into the list */
      ausConNbr[usConNbrCount] = parConnections[usConnIdx].usS_0_1050_SE2;
      usConNbrCount++;
    }
  } /* end: for (nSlaves = 0; nSlaves < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn; nSlaves++) */
  
  return (CSMD_NO_ERROR);
  
} /* End: CSMD_CheckConfiguration() */



#ifdef CSMD_HW_WATCHDOG
/**************************************************************************/ /**
\brief Determine the necessary mode for the IP-Core watchdog functionality. 
 
\ingroup func_config
\b Description:
   Checks all active consumer slave connection of all slaves.
   If at least one connection with type "non-cyclic type 2" is found,
   the watchdog mode "Stop sending telegrams" shall be selected.\n
   Otherwise the watchdog mode "Insert zeros in telegram data" is selected.
   This selection automatically is active if the IP-Core watchdog 
   functionality is used by the application.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n
      
\author  WK
\date    04.01.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_Determine_Watchdog_Mode( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_INT    nSlaves;                /* Loop counter projected slaves */
  CSMD_USHORT usCIL_Idx;              /* Index into "Connection index list" in slave configuration */
  CSMD_USHORT usConnectionSetup;      /* S-0-1050.x.01 */
  CSMD_BOOL   boStopSending;          /* watchdog mode "Stop sendig telegrams" */
  
  CSMD_CONN_IDX_STRUCT *parIdxList; /* Pointer to connection-index-list of selected slave */
  CSMD_CONFIGURATION   *parConfigs; /* Pointer to configuration-list */
  
  
  parConfigs    = &prCSMD_Instance->rConfiguration.parConfiguration[0];
  boStopSending = FALSE;  /* default watchdog mode is "Insert zeros in telegram data" */
  
  /* Over all slaves */
  for (nSlaves = 0; ((nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves) && (boStopSending == FALSE)); nSlaves++)
  {
    /* Is connection setup parameter used by the slave? */
    if (prCSMD_Instance->rPriv.aulSCP_Config[nSlaves] & (  CSMD_SCP_VARCFG
                                                         | CSMD_SCP_SYNC
                                                         | CSMD_SCP_WD
                                                         | CSMD_SCP_WDCON))
    {
      /* Check connection setup */
      parIdxList = &prCSMD_Instance->rConfiguration.parSlaveConfig[nSlaves].arConnIdxList[0];
      
      /* Over all configurable slave connections */
      for ( usCIL_Idx = 0; usCIL_Idx < CSMD_MAX_CONNECTIONS; usCIL_Idx++ )
      {
        if (parIdxList[usCIL_Idx].usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
        {
          usConnectionSetup = parConfigs[ parIdxList[usCIL_Idx].usConfigIdx ].usS_0_1050_SE1;
          
          /* Active consumed connection in MDT or AT ? */
          if (   (usConnectionSetup & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) 
              == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
          {
            /* Type of connection "non-cyclic type 2" ? */
            if (   (usConnectionSetup & CSMD_S_0_1050_SE1_MONITOR) 
                == CSMD_S_0_1050_SE1_ASYNC)
            {
              boStopSending = TRUE;
              break;
            }
          }
        } /* end: if (parIdxList[usCIL_Idx].usConfigIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig) */
      }   /* end: for ( usCIL_Idx = 0; usCIL_Idx < usNumCon; usCIL_Idx++ ) */
    }     /* end: if (prCSMD_Instance->rPriv.aulSCP_Config[nSlaves] & (  CSMD_SCP_VARCFG ... */
    else
    {
      /* No connection setup parameter in use --> seems to be an asynchronous slave */
      boStopSending = TRUE;
    }
  } /* end: for (nSlaves = 0; nSlaves < prCSMD_Instance->rSlaveList.usNumProjSlaves; nSlaves++) */
  
  if (TRUE == boStopSending)
  {
    prCSMD_Instance->usWD_Mode = CSMD_WD_TO_DISABLE_TX_TEL;
  }
  else
  {
    prCSMD_Instance->usWD_Mode = CSMD_WD_TO_SEND_EMPTY_TEL;
  }
  
  return (CSMD_NO_ERROR);
  
} /* End: CSMD_Determine_Watchdog_Mode() */
#endif  /* #ifdef CSMD_HW_WATCHDOG */



/**************************************************************************/ /**
\brief CSMD_Sort_Master_Connections() sorts the connections in the master configuration.

\ingroup module_calc
\b Description: \n
   This function searches for gaps in the master configuration list and creates a
   list with continuous indices by exchanging elements in the master configuration
   structure. Furthermore, the number of connections with the master involved as either
   producer or consumer is set in this function.
   Afterwards, all connections found in the master configuration are bubble sorted by
   producer cycle time in ascending order.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]       prCSMD_Instance
                  Pointer to memory range allocated for the variables of the
                  CoSeMa instance

\return       none

\author       AlM
\date         04.02.2014

***************************************************************************** */
CSMD_VOID CSMD_Sort_Master_Connections (CSMD_INSTANCE  *prCSMD_Instance)
{
  CSMD_USHORT  usI;                 /* ascending index in master configuration */
  CSMD_USHORT  usK = (CSMD_USHORT)(prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster - 1U);  /* descending index in master configuration */
  CSMD_USHORT  usConnIdx;
  CSMD_ULONG   ulProdCycTime;
  CSMD_BOOL    boSwap;              /* flag needed for bubble sort */
  CSMD_CONN_IDX_STRUCT rTempConn;   /* temporary structure for exchange mechanism of bubble sort */
  CSMD_USHORT  usNbrConn = 0;

  /* search while descending index is greater than ascending index */
  for (usI = 0; usI <= usK; usI++)
  {
    /* check if the ascending index represents a gap in connection configuration */
    if (   prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI].usConnIdx
        >= prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
    {
      for (; usK > usI; usK--)
      {
        if (  prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usK].usConnIdx
            < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
        {
          /* exchange the structures of the found indices */
          prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI] =
              prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usK];

          prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usK].usConfigIdx = 0xFFFF;
          prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usK].usConnIdx   = 0xFFFF;
          prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usK].usRTBitsIdx = 0xFFFF;

          usNbrConn++;
          break;
        }
      }
    }
    else
    {
      usNbrConn++;
    }
  }

  /* set number of connections which are either produced or consumed by the master */
  prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections = usNbrConn;

  /* bubble sort list of connections with the master involved by producer cycle time in ascending order */
  if (prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections > 0)
  {
    do
    {
      boSwap = FALSE;
      for (usI = 0; usI < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections - 1; usI++)
      {
        usConnIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI].usConnIdx;
        ulProdCycTime = prCSMD_Instance->rConfiguration.parConnection[usConnIdx].ulS_0_1050_SE10;

        /* check if producer cycle time of the adjacent connection in master configuration is shorter */
        if (ulProdCycTime > prCSMD_Instance->rConfiguration.parConnection[
              prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI + 1].usConnIdx].ulS_0_1050_SE10)
        {
          /* store connection in temporary structure */
          rTempConn = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI];

          /* exchange the two connection structures */
          prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI] =
            prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI + 1];

          prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI + 1] = rTempConn;
          boSwap = TRUE;
        }
      }
    } while (boSwap == TRUE);
  }

}  /* end: CSMD_Sort_Master_Connections() */


/**************************************************************************/ /**
\brief Initializes the CoSeMa-private connection structures and related variables.

\ingroup module_calc
\b Description: \n
   This function initializes the CoSeMa-private connection structures for both
   master-produced and slave-produced connections. Furthermore, this function
   looks up the producer index of slave-produced connections in the public connection
   configuration structure and inserts it into the respective private structure.
   The arrays containing the different producer cycle times and producer determination
   by current value of TSref counter are also initialized in this function.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]       prCSMD_Instance
                  Pointer to memory range allocated for the variables of the
                  CoSeMa instance

\return       none

\author       AlM
\date         27.01.2014

***************************************************************************** */
CSMD_VOID CSMD_Init_Priv_Conn_Structs( CSMD_INSTANCE *prCSMD_Instance )
{
  /* -------------------------------------------------------
   * counter variables used for finding a connection's     -
   * producer index in connection configuration structure  -
   * ------------------------------------------------------- */
  CSMD_USHORT  usI;
  CSMD_USHORT  usConnIdx;
  CSMD_USHORT  usConfigIdx;
  CSMD_USHORT  usSlaveIdx;
  CSMD_USHORT  usSlaveConn;
  /*-------------------------------------------------------- */
  CSMD_BOOL boFound;      /* flag for loop cancellation */

  /* initialize private structure for master-produced connections */
  (CSMD_VOID) CSMD_HAL_memset( &prCSMD_Instance->rPriv.parConnMasterProd[0],
                               0,
                               prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster * sizeof(CSMD_CONN_MASTERPROD) );

  /* initialize private structure for slave-produced connections */
  (CSMD_VOID) CSMD_HAL_memset( &prCSMD_Instance->rPriv.parConnSlaveProd[0],
                               0,
                               prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster * sizeof(CSMD_CONN_SLAVEPROD) );

  for (usI = 0; usI < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; usI++)
  {
    usConnIdx   = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI].usConnIdx;
    usConfigIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI].usConfigIdx;

    /* check if connection is produced by the master or a slave */
    if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1
         & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
    {
      /* connection is produced by the master */
      prCSMD_Instance->rPriv.parConnMasterProd[usConnIdx].eState = CSMD_PROD_STATE_PREPARE;
    }
    else
    {
      /* connection is produced by a slave */
      prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].eState = CSMD_CONS_STATE_PREPARE;

      /* search for the connection's producer index in connection configuration structure */
      boFound = FALSE;

      for (usSlaveIdx = 0; (usSlaveIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves) && (boFound == FALSE); usSlaveIdx++)
      {
        /* determine slave index the connection is configured to */
        for (usSlaveConn = 0; (usSlaveConn < CSMD_MAX_CONNECTIONS) && (boFound == FALSE); usSlaveConn++)
        {
          if (prCSMD_Instance->rConfiguration.parSlaveConfig[usSlaveIdx].arConnIdxList[usSlaveConn].usConnIdx == usConnIdx)
          {
            prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].usProdIdx = usSlaveIdx;
            boFound = TRUE;
          }
        }
      }
    }

    /* initialize connection production bit list, is set in CSMD_Build_Producer_Times_List() */
    prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].usProduced = 0;
  }

  /* delete unused producer cycle times from index list */
  for (usI = prCSMD_Instance->rPriv.usNumProdCycTimes; usI < CSMD_MAX_CYC_TIMES; usI++)
  {
    prCSMD_Instance->rPriv.aulProdCycTimes[usI] = 0;
  }
  /* delete values of producer determination with index higher that maximum TSref counter */
  for (usI = prCSMD_Instance->rConfiguration.rComTiming.usMaxTSRefCount_S1061; usI < CSMD_MAX_TSREF; usI++)
  {
    prCSMD_Instance->rPriv.ausTSrefList[usI] = 0;
  }

}  /* end: CSMD_Init_Priv_Conn_Structs() */


/**************************************************************************/ /**
\brief CSMD_Calculate_Producer_Cycles() determines the production cycles for every connection.

\ingroup module_calc
\b Description: \n
   This function sets one bit in the bit list for every connection which is associated
   to a certain producer cycle time in aulProdCycTimes[]. Furthermore, another bit list is
   created containing the 'active' producer cycle times related to the value of TSref counter.
   All values calculated in this function use the same code, one bit representing a certain
   producer cycle time, while bit 0 always represents Sercos cycle time.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       none

\author       AlM
\date         09.12.2013

***************************************************************************** */
CSMD_VOID CSMD_Calculate_Producer_Cycles( CSMD_INSTANCE  *prCSMD_Instance )
{
  CSMD_USHORT  usConnIdx;
  CSMD_USHORT  usConfigIdx;
  CSMD_USHORT  usConnSetup;
  CSMD_USHORT  usI;
  CSMD_USHORT  usK;
  CSMD_USHORT  usTSrefMax;  /* least common multiple of all producer cycle times */

  /* --------------------------------------------------- */
  /* build production bit list of connections            */
  /* --------------------------------------------------- */
  for (usI = 0; usI < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; usI++)
  {
    usConnIdx   = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI].usConnIdx;
    usConfigIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[usI].usConfigIdx;
    usConnSetup = prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1;

    /* check if producer cycle time is equal to Sercos cycle time */
    if (   (   prCSMD_Instance->rConfiguration.parConnection[usConnIdx].ulS_0_1050_SE10
            == prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002)
        || (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].ulS_0_1050_SE10 == 0)
        || ((usConnSetup & CSMD_S_0_1050_SE1_MONITOR) == CSMD_S_0_1050_SE1_ASYNC) )
    {
      /* connection has to be produced in every Sercos cycle */
      if ((usConnSetup & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
      {
        prCSMD_Instance->rPriv.parConnMasterProd[usConnIdx].usProduced = 1;
      }
      else
      {
        prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].usProduced = 1;
      }
    }
    else
    {
      for (usK = 1; usK < prCSMD_Instance->rPriv.usNumProdCycTimes; usK++)
      {
        /* find producer cycle time of this connection in the list of producer cycle
         * times in ascending order */
        if (   prCSMD_Instance->rConfiguration.parConnection[usConnIdx].ulS_0_1050_SE10
            == prCSMD_Instance->rPriv.aulProdCycTimes[usK])
        {
          if ((usConnSetup & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
          {
            /* set producing time determination variable */
            prCSMD_Instance->rPriv.parConnMasterProd[usConnIdx].usProduced = (CSMD_USHORT)((1UL << usK));
          }
          else
          {
            prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].usProduced = (CSMD_USHORT)((1UL << usK));
          }
        }
      }
    }
  } /* for (usI = 0; usI < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; usI++) */

  /* ------------------------------------------------------------------------------- */
  /* determine maximum value for TSref counter (LCM of all producer cycle times - 1) */
  /* ------------------------------------------------------------------------------- */
  CSMD_Calc_Max_TSref( prCSMD_Instance, &usTSrefMax );

  /* decrement maximum value of TSref counter (starts counting at zero), store it in CoSeMa structure
   * and write the value to IP-Core */
  prCSMD_Instance->rConfiguration.rComTiming.usMaxTSRefCount_S1061 = usTSrefMax;
  CSMD_HAL_SetTSrefMax (&prCSMD_Instance->rCSMD_HAL, usTSrefMax);

  /* --------------------------------------------------------- */
  /* build production bit list related to TSref counter value  */
  /* --------------------------------------------------------- */
  usI = 0;
  do
  {
    for (usK = 0; usK < prCSMD_Instance->rPriv.usNumProdCycTimes; usK++)
    {
      if (usI % (prCSMD_Instance->rPriv.aulProdCycTimes[usK] /
          prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002) == 0)
      {
        prCSMD_Instance->rPriv.ausTSrefList[usI] |= (CSMD_USHORT)((1UL << usK));
      }
    }
    usI++;
  } while (usI <= prCSMD_Instance->rConfiguration.rComTiming.usMaxTSRefCount_S1061);

}  /* end: CSMD_Calculate_Producer_Cycles */


/**************************************************************************/ /**
\brief CSMD_Build_Producer_Cycle_Times_List() build a list of all configured
       producer cycle times in ascending order.

\ingroup module_calc
\b Description: \n
   This function build a list of all configured producer cycle times in ascending order.
   The Sercos cycle time is always the first element in this list, even if neither
   connection is produced in every Sercos cycle.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       \ref CSMD_TOO_MANY_PRODUCER_CYCLETIMES \n
              \ref CSMD_NO_ERROR \n

\author       AlM
\date         09.12.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_Build_Producer_Cycle_Times_List( CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_USHORT   usConnIdx;
  CSMD_USHORT   usNumTimes = 1; /* number of different producer cycle times */
  CSMD_ULONG    ulProdCycTime;  /* temporary variable for producer cycle time */
  CSMD_USHORT   usI;
  CSMD_BOOL     boUnique;
  CSMD_BOOL     boSwap;
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;

  /* set Sercos cycle time as shortest producer cycle time */
  prCSMD_Instance->rPriv.aulProdCycTimes[0] = prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002;

  /* check all producer cycle times of connections */
  for (usConnIdx = 0; usConnIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usConnIdx++)
  {
    /* check if maximum number of different producer cycle times has been exceeded */
    if (usNumTimes > CSMD_MAX_CYC_TIMES)
    {
      return (CSMD_TOO_MANY_PRODUCER_CYCLETIMES);
    }

    ulProdCycTime = prCSMD_Instance->rConfiguration.parConnection[usConnIdx].ulS_0_1050_SE10;

    /* skip if connection is not configured or producer cycle time equals Sercos cycle time */
    if (ulProdCycTime > 0)
    {
      /* check if the producer cycle time is unique */
      boUnique = TRUE;
      for (usI = 0; usI < usNumTimes; usI++)
      {
        if (ulProdCycTime == prCSMD_Instance->rPriv.aulProdCycTimes[usI])
        {
          boUnique = FALSE;
          break;
        }
      }
      if (boUnique)
      {
        /* if producer cycle time is unique so far, insert it in list of producer
         * cycle times and increment number of different producer cycle times */
        prCSMD_Instance->rPriv.aulProdCycTimes[usNumTimes] = ulProdCycTime;
        usNumTimes++;
      }
    }
  }

  /* write number of different producer cycle times to CoSeMa private structure */
  prCSMD_Instance->rPriv.usNumProdCycTimes = usNumTimes;

  /* bubble sort list of producer cycle times in ascending order */
  do
  {
    boSwap = FALSE;
    for (usI = 0; usI < usNumTimes - 1; usI++)
    {
      if (prCSMD_Instance->rPriv.aulProdCycTimes[usI] > prCSMD_Instance->rPriv.aulProdCycTimes [usI + 1])
      {
        ulProdCycTime = prCSMD_Instance->rPriv.aulProdCycTimes[usI + 1]; /* ulProdCycTime is re-used here! */
        prCSMD_Instance->rPriv.aulProdCycTimes[usI + 1] = prCSMD_Instance->rPriv.aulProdCycTimes[usI];
        prCSMD_Instance->rPriv.aulProdCycTimes[usI] = ulProdCycTime;
        boSwap = TRUE;
      }
    }
  } while (boSwap == TRUE);

  return (eFuncRet);

}  /* end: CSMD_Build_Producer_Cycle_Times_List */


/**************************************************************************/ /**
\brief CSMD_Calc_Max_TSref() calculates the least common multiple of all producer cycle times.

\ingroup module_calc

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [out]  pusTSrefMax
              Output pointer to calculated value of TSrefMax

\return       none

\author       AlM
\date         20.03.2014

***************************************************************************** */
CSMD_VOID CSMD_Calc_Max_TSref( const CSMD_INSTANCE *prCSMD_Instance,
                               CSMD_USHORT         *pusTSrefMax )
{
  CSMD_USHORT  usLCM = 0;
  CSMD_USHORT  usCycTimeIdx;
  CSMD_BOOL    boFinished = FALSE;

  while (!boFinished)
  {
    boFinished = TRUE;

    /* add shortest producer cycle time related to Sercos cycle time */
    usLCM += (CSMD_USHORT)(  prCSMD_Instance->rPriv.aulProdCycTimes[0]
                           / prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002);

    /* check if sum is divisible by all configured producer cycle times */
    for (usCycTimeIdx = (CSMD_USHORT)(prCSMD_Instance->rPriv.usNumProdCycTimes - 1);
         usCycTimeIdx > 0; usCycTimeIdx--)
    {
      boFinished = boFinished && (usLCM %
                      (  prCSMD_Instance->rPriv.aulProdCycTimes[usCycTimeIdx]
                       / prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 ) == 0);
    }
  }

  /* return decremented LCM because TSref counter starts with zero */
  *pusTSrefMax = (CSMD_USHORT)(usLCM - 1);

}


#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
/**************************************************************************/ /**
\brief Builds parameter S-0-1015, "Ring delay".

\ingroup func_timing
\b Description: \n
   Builds ring delay depending on measured ring delays on both ports,
   topology, number of recognized and number of possible hot-plug slaves.
   S-0-1015 is transmitted to the slaves, but not furthermore used
   for timing calculations.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
   - This function shall be called after IFG calculation for CP3/CP4.
   - In ring healing this function shall not be called before CP3.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]   eRD_Mode
              Mode for ring delay calculation:
              - CSMD_CALC_RD_MODE_NORMAL \n
                  Ring delay calculation during a phase switch CP0
              - CSMD_CALC_RD_MODE_RECOVER_RING_ACTIVE \n
                  Ring delay calculation during a ring recovery
              - CSMD_CALC_RD_MODE_HOTPLUG_ACTIVE \n
                  Ring delay calculation during Hot-plug a slave
\param [out]  pulS_0_1015_P1
              Pointer for ring delay for parameter S-0-1015
\param [out]  pulS_0_1015_P2
              Pointer for ring delay for parameter S-0-1015 for slave on port 2
              in case of broken ring topology.

\return       \ref CSMD_NO_ERROR \n
              \ref CSMD_MAX_T_NETWORK_GT_T_SYNCDELAY

\author       WK
\date         13.10.2010

***************************************************************************** */
CSMD_FUNC_RET CSMD_Determination_Ringdelay( CSMD_INSTANCE     *prCSMD_Instance,
                                            CSMD_RD_CALC_MODE  eRD_Mode,
                                            CSMD_ULONG        *pulS_0_1015_P1,
                                            CSMD_ULONG        *pulS_0_1015_P2 )
{

  CSMD_RING_DELAY *prRingDelay = &prCSMD_Instance->rPriv.rRingDelay;

  /* ----------------------------------------------------------------
    Build S-0-1015 Ring delay

        tNetwork = average value of the measured network delay

        tNetwork port 1 > tNetwork port 2:

        #<----------- S-0-1015 (port 1) ----------->#
        #                                           #
        #                         #  Extra delay P1 #
        #<----- tNetwork P1 ----->#<-------+------->#
        #                         #        #        #
        #                                TSref
        #<---------- tSyncDelay ---------->#
        #                                  #
        #                   #              #              #
        #<-- tNetwork P2 -->#<-------------+------------->#
        #                   #        Extra delay P2       #
        #                                                 #
        #<-------------- S-0-1015 (port 2) -------------->#



        tNetwork port 2 > tNetwork port 1:

        #<-------------- S-0-1015 (port 1) -------------->#
        #                                                 #
        #                   #        Extra delay P1       #
        #<-- tNetwork P1 -->#<-------------+------------->#
        #                   #              #              #
        #                                TSref
        #<---------- tSyncDelay ---------->#
        #                         #        #        #
        #<----- tNetwork P2 ----->#<-------+------->#
        #                         #  Extra delay P2 #
        #                                           #
        #<----------- S-0-1015 (port 2) ----------->#

    TSref = tNetwork calculated + tComp

    S-0-1015 (port 1) = TSref + (TSref - tNetwork P1)
    S-0-1015 (port 2) = TSref + (TSref - tNetwork P2)

     ---------------------------------------------------------------- */

  /* ============================================================= */
  /* Calculate TSref                                               */
  /* ============================================================= */
  if (eRD_Mode == CSMD_CALC_RD_MODE_NORMAL)   /* that is CP < 3 */
  {
    CSMD_LONG lTSref;
    /* Calculate stable TSref = calculated tNetwork_max + IFG_jitter/2 + components delay */
    lTSref =   (CSMD_LONG)(  2U * (  prCSMD_Instance->rSlaveList.usNumProjSlaves
                                   * prCSMD_Instance->rConfiguration.rComTiming.ulCalc_DelaySlave)
                           + (prCSMD_Instance->rPriv.ulInterFrameGap * CSMD_BYTE_TIME) / 2U)
             + prCSMD_Instance->rConfiguration.rComTiming.lCompDelay;

    prRingDelay->ulTSref = (CSMD_ULONG)lTSref;
    if ((CSMD_LONG)prRingDelay->rValid.ulMaxTNetwork > lTSref)
    {
      return (CSMD_MAX_T_NETWORK_GT_T_SYNCDELAY);
    }
  }

  /* ============================================================= */
  /* Calculate Ring delay                                          */
  /* ============================================================= */
  if (prRingDelay->rValid.ulMaxTNetwork > prRingDelay->ulTSref)
  {
    return (CSMD_MAX_T_NETWORK_GT_T_SYNCDELAY);
  }
  else
  {
    /* ======================================================================== */
    if (prRingDelay->rValid.usTopology == CSMD_TOPOLOGY_RING)
    {
      /* Take the lower value of tNetwork to get the greatest value of Ring delay! */
      if (prRingDelay->ulAverageRD_P1 >= prRingDelay->ulAverageRD_P2)
      {
        *pulS_0_1015_P1 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P2;
      }
      else
      {
        *pulS_0_1015_P1 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P1;
      }
      *pulS_0_1015_P2 = *pulS_0_1015_P1;
    }
    /* ======================================================================== */
    else if (prRingDelay->rValid.usTopology == CSMD_TOPOLOGY_LINE_P1)
    {
      *pulS_0_1015_P1 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P1;
      *pulS_0_1015_P2 = 0;
    }

    /* ======================================================================== */
    else if (prRingDelay->rValid.usTopology == CSMD_TOPOLOGY_LINE_P2)
    {
      *pulS_0_1015_P1 = 0;
      *pulS_0_1015_P2 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P2;
    }

    /* ======================================================================== */
    else if (prRingDelay->rValid.usTopology == CSMD_TOPOLOGY_BROKEN_RING)
    {
      *pulS_0_1015_P1 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P1;
      *pulS_0_1015_P2 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P2;
    }

    /* ======================================================================== */
    else
    {
      /* Should not be here ! */
      /*! >todo - How to handle unknown topology respectively defect ring ?! */
      *pulS_0_1015_P1 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P1;
      *pulS_0_1015_P2 = 2 * prRingDelay->ulTSref - prRingDelay->ulAverageRD_P2;
    }

    return (CSMD_NO_ERROR);
  }
} /* end: CSMD_Determination_Ringdelay() */



/**************************************************************************/ /**
\brief Calculates the ring delay independent of measured delay.

\ingroup func_timing
\b Description: \n
   Build Ring delay independent from measured tNetwork in order to achieve
   a stable value in every phase progression process if slave
   configuration has not changed since last run-up.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
   - This function shall not be called before TSref was calculated!

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       none

\author       AM
\date         15.04.2010

***************************************************************************** */
CSMD_VOID  CSMD_Calculate_RingDelay( CSMD_INSTANCE *prCSMD_Instance )

{
  /* Calculate stable tNetwork for timing calculations */
  prCSMD_Instance->rConfiguration.rComTiming.ulCalc_RingDelay =
      prCSMD_Instance->rPriv.rRingDelay.ulTSref;

} /* end: CSMD_Calculate_RingDelay() */
#endif  /* #ifdef CSMD_STABLE_TSREF */

/*! \endcond */ /* PRIVATE */



/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
26 Nov 2013 WK
  Defdb00000000
  CSMD_CalculateTiming()
  - Shifted calculation of timing method and telegram assignment in
    two sub-routines CSMD_CalculateTelegramAssignment() and
    CSMD_CalculateTimingMethod().
04 Dec 2013 WK
  - HP field only in MDT0 and AT0.
  - EF field size reduced from 6 to 4 bytes.
13 Mar 2014 WK
  - Defdb00168199
    CSMD_BuildConnectionList()
    In some cases the arProdCC_ConnAT[].usDevIdx is determined wrong.
05 Aug 2014 AlM
  - Removed function CSMD_BuildConnectionList()
07 Jan 2015 WK
  - Defdb00175903
    CSMD_CalculateTiming()
    Miscalculation of rAT_Length[].usCC, if a telegram contains only by the
    master consumed CC connections but no other real-time connections.
03 Feb 2015 WK
  - Defdb00000000
    CSMD_CalculateTelegramAssignment()
    CSMD_COMMON_TIMING.usInterFrameGap_S1036 has not been set.
24 Feb 2015 WK
  - Defdb00177060
    CSMD_CheckConfiguration()
    Check, if the length of a produced connection is greater than 2 bytes.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
26 Mar 2015 WK
  - Defdb00178103
    CSMD_CalculateTiming()
    Fixed calculation of ulSynchronizationTime_S01007
    (Reference is TSref, not TMref)
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
01 Sep 2015 WK
  - Defdb00181092
    CSMD_Calculate_Producer_Cycles()
    Produced or consumed "non cyclic type 2" connections shall be evaluated
    every Sercos cycle.
03 Nov 2015 WK
  - Defdb00182862
    CSMD_CalculateTiming()
    When warnings the function has been terminated with an error.
05 Nov 2015 WK
  - Defdb00182857
    CSMD_CalculateTelegramAssignment()
    HotPlug field configuration depending on boHP_Field_All_Tel setting.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
30 Nov 2015 WK
  - Defdb00183423
    CSMD_Sort_Master_Connections()
    Incorrect calculation of rMasterCfg.usNbrOfConnections when all
    connections are configured in rMasterCfg.parConnIdxList[].
02 Dec 2015 WK
   - Defdb00183570
     CSMD_CalculateTelegramAssignment()
     EF-field 6 bytes extended depending on boHP_Field_All_Tel for slaves
     awaiting an SVC offset of 8 + 6 * n.
19 Jan 2016 WK
  - Defdb00184409
    CSMD_CalculateTiming()
    The stable ring delay for timing calculation was too large.
03 May 2016 WK
  - Defdb00186838
    CSMD_Calculate_Producer_Cycles()
    By defining a Producer Cycle Time of 0, no entry is made in the list of
    Connection Production Times. As a result the master fails to access this connection
    in CP4 with CSMD_CONNECTION_NOT_CONFIGURED.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
01 Jul 2016 WK
  - CSMD_Check_TSref()
    Adjustment for doxygen documentaion.
  
------------------------------------------------------------------------------
*/
