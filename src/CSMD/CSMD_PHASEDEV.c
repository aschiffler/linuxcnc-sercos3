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
\file   CSMD_PHASEDEV.c
\author WK
\date   01.09.2010
\brief  This File contains the private functions 
        for the telegram configuration and communication phase switching.

*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"
#include "CSMD_HAL_DMA.h"

#include "CSMD_CP_AUX.h"
#include "CSMD_DIAG.h"
#include "CSMD_TOPOLOGY.h"

#define SOURCE_CSMD
#include "CSMD_PHASEDEV.h"

/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/*! \endcond */ /* PUBLIC */


/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Take the hardware-specific settings.

\ingroup func_init
\b Description: \n

   \todo translation \n
      If an element in pvHW_Init_Struct equals 0, the corresponding default value will be taken over.
   Die aktuellen (Hardware)Einstellungen werden bei jedem Phasenwechsel aus der
   oeffentlichen Struktur rHW_Settings uebernommen.\n
   Bei Elementen mit unpassenden Werten, wird deren Defaultwert uebrenommen.

   Die Struktur rHW_Settings wird in CSMD_InitHardware() mit Defaultwerten
   initialisiert.

<B>Call Environment:</B> \n
  This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]   boSetDefault: \n
              - TRUE  Get default settings.
              - FALSE Get the settings from CSMD_INSTANCE.rHW_Init_Struct

\return  \ref CSMD_FPGA_IDENT_VERSION \n
         \ref CSMD_SYSTEM_ERROR \n
         \ref CSMD_NO_ERROR \n

\author       WK
\date         27.11.2013

*******************************************************************************/
CSMD_FUNC_RET CSMD_GetHW_Settings( CSMD_INSTANCE  *prCSMD_Instance,
                                   CSMD_BOOL       boSetDefault )
{

  CSMD_HW_INIT_STRUCT  *prHW_StrGlob = &prCSMD_Instance->rHW_Settings;
  CSMD_HW_INIT_STRUCT  *prHW_StrPriv = &prCSMD_Instance->rPriv.rHW_Init_Struct;


  if (boSetDefault == TRUE)
  {
    /* Communication cycle time for CP0 [ns] */
    prHW_StrGlob->ulCycleTime_CP0 = CSMD_TSCYC_1_MS;
    /* Sercos cycle time for CP1 and CP2 [ns] */
    prHW_StrGlob->ulCycleTime_CP12 = CSMD_FPGA_SERCOS_CYCLE_TIME_P0_2;
    /* UC Channel bandwidth [ns] */
    prHW_StrGlob->ulUCC_Width = 0UL;
    /* IP Maximum Transmission Unit in CP3 to CP4 [Byte] */
    prHW_StrGlob->usIP_MTU_P34 = CSMD_ETHERNET_MTU_MAX;
#ifdef CSMD_HOTPLUG
    /* Hot-Plug activation */
    prHW_StrGlob->boHotPlug = FALSE;
#endif
    /* Get Tx buffer system mode for real time data */
    prHW_StrGlob->usTxBufferMode = CSMD_TX_SINGLE_BUFFER;
    /* Get Rx buffer system mode for real time data */
    prHW_StrGlob->usRxBufferMode = CSMD_RX_SINGLE_BUFFER;
#ifdef CSMD_PCI_MASTER
    prHW_StrGlob->boPciMode = FALSE;
#endif
#ifdef CSMD_4MDT_4AT_IN_CP1_2
    prHW_StrGlob->boFourMDT_AT_CP12 = FALSE;
#endif
    /* HotPlug field in all telegrams */
    prHW_StrGlob->boHP_Field_All_Tel = TRUE;
    /* SVC Busy Timeout [ms] */
    prHW_StrGlob->usSVC_BusyTimeout = CSMD_SVC_BUSY_TIEMOUT_DEFAULT;
    /* Timeout for S-SVC.Valid at switch to CP1 [ms].*/
    prHW_StrGlob->usSVC_Valid_TOut_CP1 = CSMD_WAIT_200MS;
#ifdef CSMD_SWC_EXT
    /* Mode for UC channel in CP1/CP2 */
    prHW_StrGlob->eUCC_Mode_CP12 = CSMD_UCC_MODE_CP12_FIX;
    /* Communication phase switch mode */
    prHW_StrGlob->boFastCPSwitch = FALSE;
    /* Switch off Sercos III telegrams */
    prHW_StrGlob->boTelOffLastSlave = FALSE;
#endif
    *prHW_StrPriv = *prHW_StrGlob;
  }
  else
  {
    /* Communication cycle time for CP0 [ns] */
    prHW_StrPriv->ulCycleTime_CP0 = prHW_StrGlob->ulCycleTime_CP0;
    /* Sercos cycle time for CP1 and CP2 [ns] */
    prHW_StrPriv->ulCycleTime_CP12 = prHW_StrGlob->ulCycleTime_CP12;
    /* UC Channel bandwidth [ns] */
    prHW_StrPriv->ulUCC_Width = prHW_StrGlob->ulUCC_Width;
    /* IP Maximum Transmission Unit in CP3 to CP4 [Byte] */
    prHW_StrPriv->usIP_MTU_P34 = prHW_StrGlob->usIP_MTU_P34;
#ifdef CSMD_HOTPLUG
    /* Hot-Plug activation */
    if (prHW_StrGlob->boHotPlug == TRUE)
    {
      prHW_StrPriv->boHotPlug = TRUE;
    }
    else
    {
      prHW_StrPriv->boHotPlug = FALSE;
    }
#endif
    /* Get Tx buffer system mode for real time data */
    if (prHW_StrGlob->usTxBufferMode < CSMD_MAX_TX_BUFFER)
    {
      prHW_StrPriv->usTxBufferMode = prHW_StrGlob->usTxBufferMode;
    }
    else
    {
      prHW_StrPriv->usTxBufferMode = CSMD_TX_SINGLE_BUFFER;
    }
    /* Get Rx buffer system mode for real time data */
    if (prHW_StrGlob->usRxBufferMode < CSMD_MAX_RX_BUFFER)
    {
      prHW_StrPriv->usRxBufferMode = prHW_StrGlob->usRxBufferMode;
    }
    else
    {
      prHW_StrPriv->usRxBufferMode = CSMD_RX_SINGLE_BUFFER;
    }
#ifdef CSMD_PCI_MASTER
    if (prHW_StrGlob->boPciMode == TRUE)
    {
      prHW_StrPriv->boPciMode = TRUE;

      if (prCSMD_Instance->rCSMD_HAL.prSERC_DMA_Reg == NULL)
      {
        /*! >todo correct error message */
        return (CSMD_SYSTEM_ERROR);
      }
      /* Check PCI Busmaster version */
      if (!CSMD_HAL_CheckDMA_PCIVersion( &prCSMD_Instance->rCSMD_HAL ))
      {
        return (CSMD_FPGA_IDENT_VERSION);
      }
    }
    else
    {
      prHW_StrPriv->boPciMode = FALSE;
    }
#endif
#ifdef CSMD_4MDT_4AT_IN_CP1_2
    if (prHW_StrGlob->boFourMDT_AT_CP12 == TRUE)
    {
      /* Number of Telegrams: 4 MDT and 4 AT in CP1 and CP2 */
      prHW_StrPriv->boFourMDT_AT_CP12 = TRUE;
    }
    else
    {
      /* Number of Telegrams: 2 MDT and 2 AT in CP1 and CP2 */
      prHW_StrPriv->boFourMDT_AT_CP12 = FALSE;
    }
#endif
    if (prHW_StrGlob->boHP_Field_All_Tel == TRUE)
    {
      /* HotPlug field in all telegrams */
      prHW_StrPriv->boHP_Field_All_Tel = TRUE;
    }
    else
    {
      /* HotPlug field in MDT= and AT0 only */
      prHW_StrPriv->boHP_Field_All_Tel = FALSE;
    }
    /* SVC Busy Timeout [ms] */
    prHW_StrPriv->usSVC_BusyTimeout = prHW_StrGlob->usSVC_BusyTimeout;
    /* Timeout for S-SVC.Valid at switch to CP1 [ms].*/
    prHW_StrPriv->usSVC_Valid_TOut_CP1 = prHW_StrGlob->usSVC_Valid_TOut_CP1;
#ifdef CSMD_SWC_EXT
    /* Mode for UC channel in CP1/CP2 */
    if (prHW_StrGlob->eUCC_Mode_CP12 < CSMD_UCC_MODE_VARIANTS)
    {
      prHW_StrPriv->eUCC_Mode_CP12 = prHW_StrGlob->eUCC_Mode_CP12;
    }
    else
    {
      /* Fixed UC channel in CP1/CP2 */
      prHW_StrPriv->eUCC_Mode_CP12 = CSMD_UCC_MODE_CP12_FIX;
    }
    /* Communication phase switch mode */
    if (prHW_StrGlob->boFastCPSwitch == TRUE)
    {
      /* Fast communication phase switch */
      prHW_StrPriv->boFastCPSwitch = TRUE;
    }
    else
    {
      /* No fast communication phase switch */
      prHW_StrPriv->boFastCPSwitch = FALSE;
    }
    /* Switch off Sercos III telegrams */
    if (prHW_StrGlob->boTelOffLastSlave == TRUE)
    {
      /* Switch of Sercos telegrams on inactive port of last slave in line */
      prHW_StrPriv->boTelOffLastSlave = TRUE;
    }
    else
    {
      /* Send Sercos telegrams on inactive port of last slave in line */
      prHW_StrPriv->boTelOffLastSlave = FALSE;
    }
#endif
  }

  return (CSMD_NO_ERROR);

} /* end: CSMD_GetHW_Settings */



/**************************************************************************/ /**
\brief Detects the slave addresses in CP0.

\ingroup module_phase
\b Description: \n
   This function reads the sequence counter and slave addresses out of the local
   copy of received ATs. This process is finished if the collected information is stable for
   100 consecutive cycles.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       TRUE  - any topology address has changed, reset stable counter \n
              FALSE - no topology address has changed

\author       AlM
\date         18.02.2014

***************************************************************************** */
CSMD_BOOL CSMD_GetSlaveAddresses( CSMD_INSTANCE  *prCSMD_Instance )
{
  CSMD_BOOL    boReset  = FALSE;     /* return value */
  CSMD_BOOL    boReset2 = FALSE;
  CSMD_USHORT  ausSeqCntRx[CSMD_NBR_PORTS];  /* received sequence counters on both master ports */
  CSMD_USHORT  ausSeqCntTx[CSMD_NBR_PORTS];  /* sent sequence counters on both master ports */
  CSMD_USHORT *pusTopoAddr;

  /* read received sequence counters from both master ports */
  ausSeqCntRx[CSMD_PORT_1] = (CSMD_USHORT)(CSMD_HAL_ReadLong( prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[CSMD_TELEGRAM_NBR_0]) & 0x7FFF);
  ausSeqCntRx[CSMD_PORT_2] = (CSMD_USHORT)(CSMD_HAL_ReadLong( prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[CSMD_TELEGRAM_NBR_0]) & 0x7FFF);

  ausSeqCntTx[CSMD_PORT_1] = (CSMD_USHORT)(CSMD_HAL_ReadShort( (prCSMD_Instance->rPriv.pausSERC3_Tx_SeqCnt[CSMD_PORT_1]) ) & 0x7FFF);
  ausSeqCntTx[CSMD_PORT_2] = (CSMD_USHORT)(CSMD_HAL_ReadShort( (prCSMD_Instance->rPriv.pausSERC3_Tx_SeqCnt[CSMD_PORT_2]) ) & 0x7FFF);

  /* check if any received sequence counter has changed in comparison to previous cycle */
  if (   prCSMD_Instance->rPriv.ausSeqCnt[CSMD_PORT_1] != ausSeqCntRx[CSMD_PORT_1]
      || prCSMD_Instance->rPriv.ausSeqCnt[CSMD_PORT_2] != ausSeqCntRx[CSMD_PORT_2] )
  {
    /* if any sequence counter has changed, store received value and reset stable counter */
    prCSMD_Instance->rPriv.ausSeqCnt[CSMD_PORT_1] = ausSeqCntRx[CSMD_PORT_1];
    prCSMD_Instance->rPriv.ausSeqCnt[CSMD_PORT_2] = ausSeqCntRx[CSMD_PORT_2];
    boReset = TRUE;
  }

  if (FALSE == boReset) /* Any slaves have been scanned and received sequence counter has not changed */
  {
    /* Generate slave lists every Sercos cycle */
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
    {
      /* get number of slaves connected to port 1 from received sequence counter */
      prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb = (CSMD_USHORT) (((ausSeqCntRx[CSMD_PORT_1] - ausSeqCntTx[CSMD_PORT_1]) + 1) / 2);

      /* set topology counter to address entry of first slave scanned on port 1 */
      pusTopoAddr = (((CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[CSMD_TELEGRAM_NBR_0]) + 1);

      /* check if any scanned address has changed in comparison to previous cycle */
      boReset = CSMD_BuildAvailableSlaveList( &prCSMD_Instance->rPriv.rSlaveAvailable, pusTopoAddr );
    }

    else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
    {
      /* get number of slaves connected to port 2 from received sequence counter */
      prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb = (CSMD_USHORT) (((ausSeqCntRx[CSMD_PORT_2] - ausSeqCntTx[CSMD_PORT_2]) + 1) / 2);

      /* set topology counter to address entry of first slave scanned on port 2 */
      pusTopoAddr = ((CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[CSMD_TELEGRAM_NBR_0]) + 1;

      /* check if any scanned address has changed in comparison to previous cycle */
      boReset = CSMD_BuildAvailableSlaveList( &prCSMD_Instance->rPriv.rSlaveAvailable2, pusTopoAddr );
    }

    else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
    {
      /* get number of slaves connected to port 1 from received sequence counter */
      prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb  = (CSMD_USHORT) (((ausSeqCntRx[CSMD_PORT_1] - ausSeqCntTx[CSMD_PORT_1]) + 1) / 2);

      /* set topology counter to address entry of first slave scanned on port 1 */
      pusTopoAddr = ((CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[CSMD_TELEGRAM_NBR_0]) + 1;

      /* check if any scanned address has changed in comparison to previous cycle */
      boReset = CSMD_BuildAvailableSlaveList( &prCSMD_Instance->rPriv.rSlaveAvailable, pusTopoAddr );

      /* get number of slaves connected to port 2 from received sequence counter */
      prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb = (CSMD_USHORT) (((ausSeqCntRx[CSMD_PORT_2] - ausSeqCntTx[CSMD_PORT_2]) + 1) / 2);

      /* set topology counter to address entry of first slave scanned on port 2 */
      pusTopoAddr = ((CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[CSMD_TELEGRAM_NBR_0])
                     + prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb + 1;

      /* check if any scanned address has changed in comparison to previous cycle */
      boReset2 = CSMD_BuildAvailableSlaveList( &prCSMD_Instance->rPriv.rSlaveAvailable2, pusTopoAddr );

      if (boReset2 == TRUE) boReset = TRUE;
    }

    else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
    {
      /* total number of slaves found */
      CSMD_USHORT  usNbrSlavesFound = (CSMD_USHORT) (ausSeqCntRx[CSMD_PORT_1] - ausSeqCntTx[CSMD_PORT_2]);

      /* in ring topology, received sequence counters must be equal */
      if (usNbrSlavesFound == (ausSeqCntRx[CSMD_PORT_2] - ausSeqCntTx[CSMD_PORT_1]) )
      {
        prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb  = usNbrSlavesFound;
        prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb = usNbrSlavesFound;

        /* set topology counter to address entry of first slave scanned on port 2 */
        pusTopoAddr = ((CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[CSMD_TELEGRAM_NBR_0]) + 1;

        /* check if any scanned address has changed in comparison to previous cycle */
        boReset2 = CSMD_BuildAvailableSlaveList( &prCSMD_Instance->rPriv.rSlaveAvailable, pusTopoAddr );

        /* set topology counter to address entry of last slave scanned on port 1 */
        pusTopoAddr = ((CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[CSMD_TELEGRAM_NBR_0]) + 1;

        /* check if any scanned address has changed in comparison to previous cycle */
        boReset = CSMD_BuildAvailableSlaveList( &prCSMD_Instance->rPriv.rSlaveAvailable2, pusTopoAddr );

        if (boReset2 == TRUE) boReset = TRUE;
      }
      else
      {
        boReset = TRUE;
      }
    }

    else /* if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING) */
    {
      if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_PRIMARY)
      {
        /* get number of slaves connected to port 1 from received sequence counter */
        prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb = (CSMD_USHORT) (ausSeqCntRx[CSMD_PORT_1] - ausSeqCntTx[CSMD_PORT_2]);

        /* set topology counter to address entry of first slave scanned on port 1 */
        pusTopoAddr = ((CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[CSMD_TELEGRAM_NBR_0]) + 1;

        /* check if any scanned address has changed in comparison to previous cycle */
        boReset = CSMD_BuildAvailableSlaveList( &prCSMD_Instance->rPriv.rSlaveAvailable2, pusTopoAddr );
      }
      else /* if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_SECONDARY) */
      {
        /* get number of slaves connected to port 1 from received sequence counter */
        prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb = (CSMD_USHORT) (ausSeqCntRx[CSMD_PORT_2] - ausSeqCntTx[CSMD_PORT_1]);

        /* set topology counter to address entry of first slave scanned on port 2 */
        pusTopoAddr = ((CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[CSMD_TELEGRAM_NBR_0]) + 1;

        /* check if any scanned address has changed in comparison to previous cycle */
        boReset = CSMD_BuildAvailableSlaveList( &prCSMD_Instance->rPriv.rSlaveAvailable, pusTopoAddr );
      }
    }  /* if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING) */
  } /* if (FALSE == boReset) */

  return (boReset);
  
} /* end: CSMD_GetSlaveAddresses() */



/**************************************************************************/ /**
\brief Read the Sercos addresses from selected port.

\ingroup module_phase
\b Description: \n
   This function reads the Sercos address from the selected port.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in,out] prPrevious
                Pointer to previous stored list with available Sercos addresses
                of the selected port. Updated, if not equal to the current
                received Sercos addresses.
\param [in]     pusCurrent
                Pointer to first topology index field of the selected port
                in the received telegram.

\return         TRUE  - Current Sercos addresses have changed. Store as new
                        previous Sercos addresses.\n
                FALSE - Sercos addresses have not changed.

\author         WK
\date           02.12.2014

***************************************************************************** */
CSMD_BOOL CSMD_BuildAvailableSlaveList( CSMD_ADDR_SCAN_INFO *prPrevious,
                                        const CSMD_USHORT   *pusCurrent )
{
  CSMD_INT    nCnt;
  CSMD_BOOL   boChange = FALSE;
  CSMD_USHORT usCurrentIdx;

  /* check if any scanned address has changed in comparison to previous cycle */
  for (nCnt = 0; nCnt < prPrevious->usAddressNmb; nCnt++)
  {
    usCurrentIdx = CSMD_HAL_ReadShort( &pusCurrent[nCnt] );
    if (prPrevious->ausAddresses[nCnt] != usCurrentIdx)
    {
      /* Topology index has changed */
      prPrevious->ausAddresses[nCnt] = usCurrentIdx;
      boChange = TRUE;
    }
  }
  return (boChange);

} /* end: CSMD_BuildAvailableSlaveList() */



/**************************************************************************/ /**
\brief Deletes all slave lists before scan.

\ingroup module_phase
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
 
\return       none

\author       wk
\date         09.02.2005
    
***************************************************************************** */
CSMD_VOID CSMD_DeleteSlaveList( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT usSlaveIdx;
  CSMD_USHORT usSercAddr;
  
  /* a. Clear number of devices */
  prCSMD_Instance->rSlaveList.usNumRecogSlaves = 0;
  prCSMD_Instance->rSlaveList.usNumProjSlaves  = 0;
  
  /* list initialization */
  for (usSlaveIdx = 0; usSlaveIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usSlaveIdx++)
  {
    prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx]            = CSMD_SLAVE_INACTIVE;
    prCSMD_Instance->rPriv.ausTopologyAddresses[usSlaveIdx]          = 0;
    prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[usSlaveIdx + 2] = 0;
    prCSMD_Instance->rSlaveList.ausProjSlaveAddList[usSlaveIdx + 2]  = 0;
  }

  for (usSercAddr = 0; usSercAddr < CSMD_SLAVE_NBR_LIMIT; usSercAddr++)
  {
    prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSercAddr] = 0U;
  }
  
  /* Set current and maximum length of the Sercos lists */
  prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[0] = 0;
  prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[1] = (CSMD_USHORT) (prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves * 2);
  
  prCSMD_Instance->rSlaveList.ausProjSlaveAddList[0] = 0;
  prCSMD_Instance->rSlaveList.ausProjSlaveAddList[1] = (CSMD_USHORT) (prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves * 2);
  
} /* end: CSMD_DeleteSlaveList() */



/**************************************************************************/ /**
\brief Scans all available slaves and builds the list of recognized slaves.

\ingroup module_phase
\b Description: \n
   After checking if a stable state of physical topology and received AT telegrams
   has been reached, this function determines the available slaves and validates
   the received slave list. Finally it initiates the building process of the
   list with Sercos addresses of all recognized slaves in topological order.
   In case of broken ring topology the topological order of the slaves at port 2
   is inverted.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_LOOP_NOT_CLOSED \n
              \ref CSMD_ERROR_TIMEOUT_P0 \n
              \ref CSMD_NO_COMMUNICATION_P0 \n
              \ref CSMD_ILLEGAL_CASE \n
              \ref CSMD_FUNCTION_IN_PROCESS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         17.02.2005

***************************************************************** */
CSMD_FUNC_RET CSMD_Detect_Available_Slaves( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_INT         nLoop;
  CSMD_BOOL        boChange;
  CSMD_FUNC_STATE *prFuncState = &prCSMD_Instance->rPriv.rInternalFuncState;
  CSMD_RING_DELAY *prRingDelay = &prCSMD_Instance->rPriv.rRingDelay;
  CSMD_USHORT      usTempTopology;
  
  
  switch (prFuncState->usActState)
  {
  
  case CSMD_FUNCTION_1ST_ENTRY:
    /* ------------------------------------------------------------ */
    /*   Step 1 ::  Init counters                                   */
    /* ------------------------------------------------------------ */
    prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb  = 0;
    prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb = 0;
    
    for (nLoop = 0; nLoop < CSMD_NBR_SCAN_SLAVES; nLoop++)   /* Clear list of available drives */
    {
      prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[nLoop]  = 0;
      prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[nLoop] = 0;
    }
    
    for (nLoop = CSMD_PORT_1; nLoop < CSMD_NBR_PORTS; nLoop++)
    {
      prCSMD_Instance->rPriv.ausSeqCnt[nLoop] = 0;
    }
    
    prCSMD_Instance->rPriv.lOutTimer    = CSMD_TIMEOUT_STABLE_TOPOLOGY; /* Timeout for detection of stable topology = 1000 * tScyc */
    prCSMD_Instance->rPriv.lStableTimer = CSMD_NBR_STABLE_TOPOLOGY;     /* The topology shall be stable for 100 Sercos cycles */
    
    prRingDelay->usCount1 = 0U;
    prRingDelay->usCount2 = 0U;
    prRingDelay->ulSumRD1 = 0U;
    prRingDelay->ulSumRD2 = 0U;
#ifdef CSMD_DEBUG
    for (nLoop = 0; nLoop < CSMD_NBR_OF_RD_MEASUREMENTS; nLoop++)
    {
      prRingDelay->ulBuff1[nLoop] = 0;
      prRingDelay->ulBuff2[nLoop] = 0;
    }
#endif    
    prRingDelay->ulMinDelay1 = 0xFFFFFFFF;
    prRingDelay->ulMaxDelay1 = 0;
    prRingDelay->ulMinDelay2 = 0xFFFFFFFF;
    prRingDelay->ulMaxDelay2 = 0;
    
    prCSMD_Instance->usCSMD_Topology = CSMD_NO_LINK;
    prFuncState->ulSleepTime = 0;
    prFuncState->usActState  = CSMD_FUNCTION_STEP_1;
    break;
    
    
  case CSMD_FUNCTION_STEP_1:
    /* ------------------------------------------------------------ */
    /*   Step 2:  Check for stable physical topology                */
    /* ------------------------------------------------------------ */
    usTempTopology = CSMD_NO_LINK;
    
    if (prCSMD_Instance->rPriv.lOutTimer-- < 1 )
    {
      prCSMD_Instance->rPriv.boP1_active = FALSE;
      prCSMD_Instance->rPriv.boP2_active = FALSE;
      return (CSMD_LOOP_NOT_CLOSED);
    }
    else
    {
      /* Wait once tScyc */
      prFuncState->ulSleepTime = 
        (prCSMD_Instance->rPriv.ulActiveCycTime + 1000000 - 1) / 1000000;
    }
    
    if (prCSMD_Instance->rPriv.rRedundancy.boNewDataP1 && prCSMD_Instance->rPriv.rRedundancy.boNewDataP2)
    {   
      if (   (TRUE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP1)
          && (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP2))
      {
        /* secondary telegram on port 1*/
        /* primary  telegram on port 2*/
        usTempTopology = CSMD_TOPOLOGY_RING;
      }
      else if (   (TRUE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP1)
               && (TRUE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP2))
      {
        /* secondary telegram on port 1*/
        /* secondary telegram on port 2*/
        usTempTopology = CSMD_TOPOLOGY_DEFECT_RING;   /* ring with defect on primary line */
        prCSMD_Instance->rPriv.rRedundancy.usRingDefect = CSMD_RING_DEF_PRIMARY;
      }
      else if (   (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP1)
               && (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP2))
      {
        /* primary telegram on port 1*/
        /* primary telegram on port 2*/
        usTempTopology = CSMD_TOPOLOGY_DEFECT_RING;   /* ring with defect on secondary line */
        prCSMD_Instance->rPriv.rRedundancy.usRingDefect = CSMD_RING_DEF_SECONDARY;
      }
      else      
      {
        usTempTopology = CSMD_TOPOLOGY_BROKEN_RING;    /* interrupted ring */
      }
    }   
    else    
    {
      if (   (TRUE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP1)
          && (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP1))
      {
        usTempTopology = CSMD_TOPOLOGY_LINE_P1;                    /* flag for topology */ 
      }
      else if (   (TRUE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP2)
               && (TRUE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP2))
      {
        usTempTopology = CSMD_TOPOLOGY_LINE_P2;                    /* flag for topology */ 
      }
      else
      {
        if (!(CSMD_HAL_GetLinkStatus(&prCSMD_Instance->rCSMD_HAL)))
        {
          usTempTopology = CSMD_NO_LINK;     /* no link active */
        }
      }
    }
    
    if (prCSMD_Instance->usCSMD_Topology == CSMD_NO_LINK)
    {
      prCSMD_Instance->usCSMD_Topology = usTempTopology;
    }
    else
    {
      /* Topology must be stable 100 times */
      if (usTempTopology == prCSMD_Instance->usCSMD_Topology )
      {
        prCSMD_Instance->rPriv.lStableTimer--;
      }
      else
      {
        prCSMD_Instance->rPriv.lStableTimer = CSMD_NBR_STABLE_TOPOLOGY;   /* The topology shall be stable for 100 Sercos cycles */
        prCSMD_Instance->usCSMD_Topology = usTempTopology;
      }
    }
    
    if (prCSMD_Instance->rPriv.lStableTimer < 0)
    {
      if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
      {
        /* init SeqCnt */
        if (prCSMD_Instance->usRequired_Topology == CSMD_TOPOLOGY_RING)
        {
          prCSMD_Instance->rPriv.rSlaveAvailable2.usSeqCntInit = 0x8001U;
        }
        else
        {
          prCSMD_Instance->rPriv.rSlaveAvailable2.usSeqCntInit = 0x0001U;
        }
        CSMD_HAL_WriteShort( prCSMD_Instance->rPriv.pausSERC3_Tx_SeqCnt[CSMD_PORT_2],
                             prCSMD_Instance->rPriv.rSlaveAvailable2.usSeqCntInit );
      }
      
      prCSMD_Instance->rPriv.lOutTimer    = CSMD_TIMEOUT_STABLE_SLAVE_LIST; /* Timeout for stable slave list = 1000 * tScyc */
      prCSMD_Instance->rPriv.lStableTimer = CSMD_NBR_STABLE_SLAVE_LIST;     /* The slave list shall be stable for 100 Sercos cycles */
      
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_2;
    }
    
    break;
    
    
  case CSMD_FUNCTION_STEP_2:
    /* ---------------------------------------------------------------- */
    /* Step 3:  Check for stable received AT telegrams                  */
    /*          and determine available slaves                          */
    /* ---------------------------------------------------------------- */
    prCSMD_Instance->rPriv.lOutTimer--;               /* Timeout counter */
    prCSMD_Instance->rPriv.lStableTimer--;            /* Stable address counter */

    if (TRUE == CSMD_Telegram_Fail_CP0(prCSMD_Instance) )
    {
      /* any expected telegram missed in current cycle */
      boChange = TRUE;                                /* Drive list change flag */
    }
    else
    {
      boChange = CSMD_GetSlaveAddresses( prCSMD_Instance );
      CSMD_MeasureRingdelay( prCSMD_Instance );
    }

    if (boChange)                                     /* One or more drive counter(s) changed */
    {
      prCSMD_Instance->rPriv.lStableTimer = CSMD_NBR_STABLE_SLAVE_LIST;   /* The slave address list shall be stable for 100 Sercos cycles */

      /* Initialize the ring delay measuring structure */
      (CSMD_VOID)CSMD_HAL_memset( prRingDelay, 0, sizeof (CSMD_RING_DELAY) );

      prRingDelay->ulMinDelay1 = 0xFFFFFFFF;
      prRingDelay->ulMinDelay2 = 0xFFFFFFFF;
    }
    
    if (prCSMD_Instance->rPriv.lStableTimer < 0)      /* Stable address list? */
    {
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
      if (prRingDelay->usCount1)
      {
        prRingDelay->ulAverageRD_P1 = prRingDelay->ulSumRD1 / prRingDelay->usCount1;  /* Average value of tNetwork port 1 */
      }
      if (prRingDelay->usCount2)
      {
        prRingDelay->ulAverageRD_P2 = prRingDelay->ulSumRD2 / prRingDelay->usCount2;  /* Average value of tNetwork port 2 */
      }
      /* Save values of valid measuring of tNetwork */
      prRingDelay->rValid.ulTNetwork_P1 = prRingDelay->ulAverageRD_P1;       /* Average value of tNetwork port 1   */
      prRingDelay->rValid.ulTNetwork_P2 = prRingDelay->ulAverageRD_P2;       /* Average value of tNetwork port 2   */
      prRingDelay->rValid.usTopology    = prCSMD_Instance->usCSMD_Topology;  /* Topology during tNetwork measuring */
      prRingDelay->rValid.ulMaxTNetwork = prRingDelay->ulMaxDelay1 + prRingDelay->ulMaxDelay2;  /* Sum of tNetwork_max form both ports*/
#endif
      prFuncState->ulSleepTime = 0;
      prFuncState->usActState  = CSMD_FUNCTION_STEP_3; /* list stable, go to next state */
    }
    else if (prCSMD_Instance->rPriv.lOutTimer < 0)    /* Timeout? */
    {
      prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb  = 0;
      prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb = 0;
      prCSMD_Instance->rPriv.boP1_active = FALSE;
      prCSMD_Instance->rPriv.boP2_active = FALSE;
      return (CSMD_ERROR_TIMEOUT_P0);
    }
    else
    {
      if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)  /* interrupted ring / two lines */
      {
        /* Reconfigure SequenceCount Field of transmitted AT0 on Port 2: */
        /* SeqCntP2 = SeqCntP1 / 2 + 1                                   */
        prCSMD_Instance->rPriv.rSlaveAvailable2.usSeqCntInit = (CSMD_USHORT)
          (((prCSMD_Instance->rPriv.ausSeqCnt[CSMD_PORT_1] / 2) + 1) | 0x8000);
        
        CSMD_HAL_WriteShort( prCSMD_Instance->rPriv.pausSERC3_Tx_SeqCnt[CSMD_PORT_2],
                             prCSMD_Instance->rPriv.rSlaveAvailable2.usSeqCntInit );
      }
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING)
      {
        if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_PRIMARY)
        {
          prCSMD_Instance->rPriv.rRedundancy.usDefectRingCP0 = CSMD_RING_DEF_PRIMARY;

          /* Reconfigure SequenceCount Field of transmitted AT0 on both ports: */
          prCSMD_Instance->rPriv.rSlaveAvailable.usSeqCntInit  = 0x8001U;
          prCSMD_Instance->rPriv.rSlaveAvailable2.usSeqCntInit = 0x0001U;
          
          CSMD_HAL_WriteShort( prCSMD_Instance->rPriv.pausSERC3_Tx_SeqCnt[CSMD_PORT_1],
                               prCSMD_Instance->rPriv.rSlaveAvailable.usSeqCntInit );
          CSMD_HAL_WriteShort( prCSMD_Instance->rPriv.pausSERC3_Tx_SeqCnt[CSMD_PORT_2],
                               prCSMD_Instance->rPriv.rSlaveAvailable2.usSeqCntInit );
        }
        else /* defect ring with defect on secondary line */
        {
          prCSMD_Instance->rPriv.rRedundancy.usDefectRingCP0 = CSMD_RING_DEF_SECONDARY;
        }
      }

      /* continue check list, wait once tScyc */
      prFuncState->ulSleepTime = 
        (prCSMD_Instance->rPriv.ulActiveCycTime + 1000000 - 1) / 1000000;
    }
    break;
    
    
  case CSMD_FUNCTION_STEP_3:
    /* ---------------------------------------------------------------- */
    /* Step 3: Check received slave list                                */
    /* ---------------------------------------------------------------- */
    if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
    {
      /* Invert the list of port 2 for topology check purposes in CP4 */
      for (nLoop=0; nLoop < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; nLoop++)
      {
        prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[nLoop] =
          prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[(prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb - 1) - nLoop];
      }
    }

    if (!(prCSMD_Instance->rPriv.rRedundancy.boNewDataP1 == TRUE))
    {
      prCSMD_Instance->rPriv.boP1_active = FALSE;
    }
    
    if (!(prCSMD_Instance->rPriv.rRedundancy.boNewDataP2 == TRUE))
    {
      prCSMD_Instance->rPriv.boP2_active = FALSE;
    }
    
    if (   !prCSMD_Instance->rPriv.boP1_active 
        && !prCSMD_Instance->rPriv.boP2_active)       /* no communication */
    {
      /* #### todo: correct error message */
      return (CSMD_NO_COMMUNICATION_P0);
    }
    
    prFuncState->ulSleepTime = 0;
    prFuncState->usActState  = CSMD_FUNCTION_FINISHED;
    return (CSMD_NO_ERROR);
    
    
  default:
    return (CSMD_ILLEGAL_CASE);
    
  }   /* End: switch (prFuncState->usActState) */
  
  return (CSMD_FUNCTION_IN_PROCESS);
  
} /* end: CSMD_Detect_Available_Slaves() */



/**************************************************************************/ /**
\brief Builds a Sercos list with Sercos addresses of all recognized slaves
       in topological order.
                          
\ingroup module_phase
\b Description: \n
   This function clears and (re-)builds the list of Sercos addresses of all
   recognized slaves in topological order and, after that, writes the length
   of the list.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         18.02.2005
                          
***************************************************************************** */
CSMD_FUNC_RET CSMD_Build_Recog_Slave_AddList( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT usSlaveIdx;
  CSMD_USHORT usSlave_Add;
  CSMD_USHORT usI;
  
  /* --------------------------------------------------------------------- */
  /* Clear Sercos list of recognized slaves                                */
  /* --------------------------------------------------------------------- */
  
  /* Set current and maximum length of the Sercos list */
  prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[eCSMD_IdnListActLength] = (CSMD_USHORT) 0;
  prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[eCSMD_IdnListMaxLength] = (CSMD_USHORT) (prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves * 2);
  
  /* Clear list         */
  for (usSlaveIdx = (CSMD_USHORT)eCSMD_IdnListDataStart; usSlaveIdx < (prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves + 2); usSlaveIdx++)
    prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[usSlaveIdx] = 0U;
  
  /* Clear number of recognized slaves         */
  prCSMD_Instance->rSlaveList.usNumRecogSlaves = 0U;
  
  /* --------------------------------------------------------------------- */
  /* Build Sercos list of all recognized slaves (sercos addresses)         */
  /* in topological order.                                                 */
  /* --------------------------------------------------------------------- */
  switch (prCSMD_Instance->usCSMD_Topology)
  {
  case CSMD_TOPOLOGY_RING:
    for (usSlave_Add = 0; usSlave_Add < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usSlave_Add++ )
    {
      prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[(CSMD_USHORT)eCSMD_IdnListDataStart+usSlave_Add] =
        prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usSlave_Add];
    }
    prCSMD_Instance->rSlaveList.usNumRecogSlaves = prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb;
    prCSMD_Instance->usSercAddrLastSlaveP1 = 0U;
    prCSMD_Instance->usSercAddrLastSlaveP2 = 0U;
    break;
    
  case CSMD_TOPOLOGY_LINE_P1:
    for (usSlave_Add = 0; usSlave_Add < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usSlave_Add++ )
    {
      prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[(CSMD_USHORT)eCSMD_IdnListDataStart+usSlave_Add] =
        prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usSlave_Add];
    }
    prCSMD_Instance->rSlaveList.usNumRecogSlaves = prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb;
    prCSMD_Instance->usSercAddrLastSlaveP1 = prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb-1];
    prCSMD_Instance->usSercAddrLastSlaveP2 = 0U;
    break;
    
  case CSMD_TOPOLOGY_LINE_P2:
    if (prCSMD_Instance->usRequired_Topology == CSMD_TOPOLOGY_RING)   /* expected topology is ring */
    {
      for (usI = prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb, usSlave_Add = 0;
           usI > 0U;
           usI--, usSlave_Add++ )
      {
        prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[(CSMD_USHORT)eCSMD_IdnListDataStart+usSlave_Add] =
          prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI - 1];
      }
      prCSMD_Instance->rSlaveList.usNumRecogSlaves = prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb;
    }
    else
    {
      for (usSlave_Add = 0; usSlave_Add < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usSlave_Add++ )
      {
        prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[(CSMD_USHORT)eCSMD_IdnListDataStart+usSlave_Add] =
          prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usSlave_Add];
      }
      prCSMD_Instance->rSlaveList.usNumRecogSlaves = prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb;
    }
    
    prCSMD_Instance->usSercAddrLastSlaveP1 = 0U;
    prCSMD_Instance->usSercAddrLastSlaveP2 = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb-1];
    break;
    
  case CSMD_TOPOLOGY_BROKEN_RING:   /* interrupted ring / two lines */
    /* Get Slaves of port 1 */
    for (usSlave_Add = 0; usSlave_Add < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usSlave_Add++ )
    {
      prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[(CSMD_USHORT)eCSMD_IdnListDataStart+usSlave_Add] =
        prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usSlave_Add];
    }
    /* Merge slaves of port 2 */
    for (usI = prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI > (CSMD_USHORT)0; usI-- )
    {
      prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[(CSMD_USHORT)eCSMD_IdnListDataStart+usSlave_Add] =
        prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI - 1];
      usSlave_Add++;
    }
    prCSMD_Instance->rSlaveList.usNumRecogSlaves = usSlave_Add;
    
    /* Slave Serc. address of the last Slave at the end of line on Port 1 and Port 2 */
    prCSMD_Instance->usSercAddrLastSlaveP1 = prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb-1];
    prCSMD_Instance->usSercAddrLastSlaveP2 = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb-1];
    break;
    
  case CSMD_TOPOLOGY_DEFECT_RING: 
    
    if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_PRIMARY)
    {
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
      {
        /* move slave addresses from port list 1 to port list 2 in case of defect ring,
           because CSMD_GetSlaveAddresses() did not respect defect ring */
        prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI] = prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI];
      }
      prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb = prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb;
      
      /* invert the slave port list for port 1 (needed for later call of CSMD_SearchLineBreakPoint() ) */
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
      {
        prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI] =
          prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb - (1 + usI)];
      }
      prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb = 0; /* handling different from CP>0 ! */
      
      prCSMD_Instance->usSercAddrLastSlaveP1 = 0xFFFF;
      prCSMD_Instance->usSercAddrLastSlaveP2 = 
        *((CSMD_USHORT *)(CSMD_VOID *)prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[0] +
          ((prCSMD_Instance->rPriv.ausSeqCnt[CSMD_PORT_2] & 0x7FFF) / 2));
    }
    
    else /* ring defect on secondary line */
    {
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
      {
        /* move slave addresses from port list 1 to port list 2 in case of defect ring,
           because CSMD_GetSlaveAddresses() did not respect defect ring */              
        prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI] = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI];
      }
      prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb = prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb;

      /* invert the slave port list for port 2 (needed for later call of CSMD_SearchLineBreakPoint() ) */
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
      {
        prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI] =
          prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb - (1 + usI)];
      }
      prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb = 0; /* handling different from CP>0 ! */
      
      prCSMD_Instance->usSercAddrLastSlaveP1 = 
        *((CSMD_USHORT *)(CSMD_VOID *)prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[0] + 
          (prCSMD_Instance->rPriv.ausSeqCnt[CSMD_PORT_1] / 2));
      prCSMD_Instance->usSercAddrLastSlaveP2 = 0xFFFF;
    }
    
    
    if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_PRIMARY) /* same behaviour as for line P2 */
    {
      if (prCSMD_Instance->usRequired_Topology == CSMD_TOPOLOGY_RING)   /* expected topology is ring */
      {
        for (usI = prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb, usSlave_Add = 0;
             usI > 0U;
             usI--, usSlave_Add++ )
        {
            prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[(CSMD_USHORT)eCSMD_IdnListDataStart+usSlave_Add] =
                prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI - 1];
        }
        prCSMD_Instance->rSlaveList.usNumRecogSlaves = prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb;
      }
      else
      {
        for (usSlave_Add = 0; usSlave_Add < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usSlave_Add++ )
        {
            prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[(CSMD_USHORT)eCSMD_IdnListDataStart+usSlave_Add] =
                prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usSlave_Add];
        }
        prCSMD_Instance->rSlaveList.usNumRecogSlaves = prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb;
      }
    }
    else if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_SECONDARY) /* same behaviour as for line P1 */
    {
      for (usSlave_Add = 0; usSlave_Add < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usSlave_Add++ )
      {
        prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[(CSMD_USHORT)eCSMD_IdnListDataStart+usSlave_Add] =
            prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usSlave_Add];
      }
      prCSMD_Instance->rSlaveList.usNumRecogSlaves = prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb;
    }
    break;
    
  default:
    break;
    
  }
  
  /* --------------------------------------------------------------------- */
  /* Write current length of Sercos list                                   */
  /* --------------------------------------------------------------------- */
  prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[eCSMD_IdnListActLength] =
    (CSMD_USHORT) (prCSMD_Instance->rSlaveList.usNumRecogSlaves * 2);
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Build_Recog_Slave_AddList() */



/**************************************************************************/ /**
\brief Checks the current topology and number of recognized slaves
       at the end of setting CP0.

\b Description: \n
              - Check, if the current Topology conform with the recognized
                topology in usCSMD_Topology.
              - Check, if the current sequence counters conform with
                the number of recognized slaves.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       \ref CSMD_INCONSISTENT_RING_ADDRESSES \n
              \ref CSMD_NO_STABLE_TOPOLOGY_IN_CP0 \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         21.10.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_Check_Recognized_Topology_CP0( const CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  CSMD_ULONG     ulLinkStatus;      /* Link status (IP-Core DFCSR bit 19/18)      */
  CSMD_USHORT    usNumRecogSlaves;  /* Number of recognized slaves                */
  CSMD_USHORT    usSeqCntTxP1;      /* AT0 sequence counter transmitted on port 1 */
  CSMD_USHORT    usSeqCntTxP2;      /* AT0 sequence counter transmitted on port 2 */
  CSMD_USHORT    usSeqCntRxP1;      /* AT0 sequence counter received on port 1    */
  CSMD_USHORT    usSeqCntRxP2;      /* AT0 sequence counter received on port 2    */
  CSMD_USHORT    usSeqCntDiffP1;    /* Sequence counter difference port 1         */
  CSMD_USHORT    usSeqCntDiffP2;    /* Sequence counter difference port 2         */


  usNumRecogSlaves = prCSMD_Instance->rSlaveList.usNumRecogSlaves;
  usSeqCntRxP1     = (CSMD_USHORT)(CSMD_HAL_ReadLong( prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[0]) & 0x7FFF);
  usSeqCntRxP2     = (CSMD_USHORT)(CSMD_HAL_ReadLong( prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[0]) & 0x7FFF);
  usSeqCntTxP1     = (CSMD_USHORT)(CSMD_HAL_ReadShort( (prCSMD_Instance->rPriv.pausSERC3_Tx_SeqCnt[CSMD_PORT_1]) ) & 0x7FFF);
  usSeqCntTxP2     = (CSMD_USHORT)(CSMD_HAL_ReadShort( (prCSMD_Instance->rPriv.pausSERC3_Tx_SeqCnt[CSMD_PORT_2]) ) & 0x7FFF);

  ulLinkStatus     = CSMD_HAL_GetLinkStatus( &prCSMD_Instance->rCSMD_HAL );

  switch (ulLinkStatus & 3UL)
  {
    /* --------------------- */
    /* No link on both ports */
    /* --------------------- */
    case 0:
      if (prCSMD_Instance->usCSMD_Topology == CSMD_NO_LINK)
      {
        if (usNumRecogSlaves != 0)
        {
          eFuncRet = CSMD_INCONSISTENT_RING_ADDRESSES;
        }
      }
      else
      {
        eFuncRet = CSMD_NO_STABLE_TOPOLOGY_IN_CP0;
      }
      break;

    /* ---------------------------------- */
    /* Link on port 1 / No link on port 2 */
    /* ---------------------------------- */
    case 1:
      if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
      {
        usSeqCntDiffP1 = (CSMD_USHORT)(usSeqCntRxP1 - usSeqCntTxP1);

        if ((usSeqCntDiffP1 + 1) != 2 * usNumRecogSlaves)
        {
          eFuncRet = CSMD_INCONSISTENT_RING_ADDRESSES;
        }
      }
      else
      {
        eFuncRet = CSMD_NO_STABLE_TOPOLOGY_IN_CP0;
      }
      break;

    /* ---------------------------------- */
    /* Link on port 2 / No link on port 1 */
    /* ---------------------------------- */
    case 2:
      if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
      {
        usSeqCntDiffP2 = (CSMD_USHORT)(usSeqCntRxP2 - usSeqCntTxP2);

        if ((usSeqCntDiffP2 + 1) != 2 * usNumRecogSlaves)
        {
          eFuncRet = CSMD_INCONSISTENT_RING_ADDRESSES;
        }
      }
      else
      {
        eFuncRet = CSMD_NO_STABLE_TOPOLOGY_IN_CP0;
      }
      break;

    /* ------------------ */
    /* Link on both ports */
    /* ------------------ */
    default:
      if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
      {
        usSeqCntDiffP1 = (CSMD_USHORT)(usSeqCntRxP1 - usSeqCntTxP1);

        if ((usSeqCntDiffP1 + 1) != 2 * usNumRecogSlaves)
        {
          eFuncRet = CSMD_INCONSISTENT_RING_ADDRESSES;
        }
        /* Sercos telegram on port 2? */
        if (prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[CSMD_PORT_2] & (  CSMD_HAL_TGSR_ALL_MDT
                                                                           | CSMD_HAL_TGSR_ALL_AT
                                                                           | CSMD_HAL_TGSR_MST_VALID
                                                                           | CSMD_HAL_TGSR_MST_WIN_ERR))
        {
          eFuncRet = CSMD_NO_STABLE_TOPOLOGY_IN_CP0;
        }
      }
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
      {
        usSeqCntDiffP2 = (CSMD_USHORT)(usSeqCntRxP2 - usSeqCntTxP2);

        if ((usSeqCntDiffP2 + 1) != 2 * usNumRecogSlaves)
        {
          eFuncRet = CSMD_INCONSISTENT_RING_ADDRESSES;
        }
        /* Sercos telegram on port 1? */
        if (prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[CSMD_PORT_1] & (  CSMD_HAL_TGSR_ALL_MDT
                                                                           | CSMD_HAL_TGSR_ALL_AT
                                                                           | CSMD_HAL_TGSR_MST_VALID
                                                                           | CSMD_HAL_TGSR_MST_WIN_ERR))
        {
          eFuncRet = CSMD_NO_STABLE_TOPOLOGY_IN_CP0;
        }
      }
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
      {
        usSeqCntDiffP1 = (CSMD_USHORT)(usSeqCntRxP1 - usSeqCntTxP2); /* On P1 received telegrams are sent from Port 2 */
        usSeqCntDiffP2 = (CSMD_USHORT)(usSeqCntRxP2 - usSeqCntTxP1);
        if (   (usSeqCntDiffP1 != usNumRecogSlaves)
            || (usSeqCntDiffP2 != usNumRecogSlaves))
        {
          eFuncRet = CSMD_INCONSISTENT_RING_ADDRESSES;
        }
      }
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
      {
        usSeqCntDiffP1 = (CSMD_USHORT)(usSeqCntRxP1 - usSeqCntTxP1);
        usSeqCntDiffP2 = (CSMD_USHORT)(usSeqCntRxP2 - usSeqCntTxP2);
        if (   ((usSeqCntDiffP1 + 1) + (usSeqCntDiffP2 + 1)
            != 2 * usNumRecogSlaves))
        {
          eFuncRet = CSMD_INCONSISTENT_RING_ADDRESSES;
        }
      }
      else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING)
      {
        if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_PRIMARY)
        {
          usSeqCntDiffP1 = (CSMD_USHORT)(usSeqCntRxP1 - usSeqCntTxP2); /* On P1 received telegrams are sent from Port 2 */
          if (usSeqCntDiffP1 != usNumRecogSlaves)
          {
            eFuncRet = CSMD_INCONSISTENT_RING_ADDRESSES;
          }
        }
        else  /* Ring defect on secondary port */
        {
          usSeqCntDiffP2 = (CSMD_USHORT)(usSeqCntRxP2 - usSeqCntTxP1);
          if (usSeqCntDiffP2 != usNumRecogSlaves)
          {
            eFuncRet = CSMD_INCONSISTENT_RING_ADDRESSES;
          }
        }
      }
      else
      {
        eFuncRet = CSMD_NO_STABLE_TOPOLOGY_IN_CP0;
      }
      break;
  }

  return (eFuncRet);

} /* end: CSMD_Check_Recognized_Topology_CP0() */



#ifdef CSMD_SWC_EXT
/**************************************************************************/ /**

\brief Checks and clears the slave acknowledgment in the AT address field.

\ingroup module_phase
\todo
\b Description: \n
   This function 

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_CP0_COM_VER_CHECK \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         26.06.2012

***************************************************************************** */
CSMD_FUNC_RET CSMD_Check_Slave_Acknowledgment( CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_BOOL   boCheckAck = FALSE;
  CSMD_USHORT usI;
  
  prCSMD_Instance->rExtendedDiag.usNbrSlaves = 0;
  prCSMD_Instance->rExtendedDiag.ulIDN       = 0;
  
  /* Switch of Sercos telegrams on inactive port of last slave in line */
  if (prCSMD_Instance->rPriv.rHW_Init_Struct.boTelOffLastSlave== TRUE)
  {
    boCheckAck = TRUE;
  }
  
  /* Fast communication phase switch */
  if (prCSMD_Instance->rPriv.rHW_Init_Struct.boFastCPSwitch == TRUE)
  {
    boCheckAck = TRUE;
  }
  
  /* Transmission CP0 parameters */
  if (   (prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_1)
      || (prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_2)
      || (prCSMD_Instance->rPriv.rHW_Init_Struct.eUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_1_VAR))
  {
    boCheckAck = TRUE;
  }

  if (boCheckAck == TRUE)
  {
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
    {
      if (prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI] & CSMD_AT0_CP0_SLAVE_ACK)
      {
        prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI] &= (CSMD_USHORT)(~CSMD_AT0_CP0_SLAVE_ACK);
      }
      else
      {
        if (prCSMD_Instance->rExtendedDiag.usNbrSlaves < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
        {
          prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = 
            prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI];
          prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
        }
      }
    }
    
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
    {
      if (prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI] & CSMD_AT0_CP0_SLAVE_ACK)
      {
        prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI] &= (CSMD_USHORT)(~CSMD_AT0_CP0_SLAVE_ACK);
      }
      else
      {
        if (prCSMD_Instance->rExtendedDiag.usNbrSlaves < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
        {
          prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = 
            prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI];
          prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
        }
      }
    }
  }
  
  if (prCSMD_Instance->rExtendedDiag.usNbrSlaves)
  {
    /* Any slave reports a problem */
    return (CSMD_CP0_COM_VER_CHECK);
  }
  else
  {
    return (CSMD_NO_ERROR);
  }
  
} /* end: CSMD_Check_Slave_Acknowledgment() */
#endif /* #ifdef CSMD_SWC_EXT */



/**************************************************************************/ /**
\brief Checks if multiple Sercos addresses have been found in the
       recognized slave address list.
                          
\ingroup module_phase
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_ILLEGAL_SLAVE_ADDRESS \n
              \ref CSMD_ERROR_DOUBLE_RECOGNIZED_ADDRESS \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         25.02.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_Check_Slave_Addresses( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT  usSlaveIdx;
  CSMD_USHORT  usSAdd;
  CSMD_USHORT  usI;
  CSMD_USHORT  usNbrRecogSlaves;
  
  usNbrRecogSlaves = prCSMD_Instance->rSlaveList.usNumRecogSlaves;
  prCSMD_Instance->rPriv.boMultipleSAddress = FALSE;
  
  for (usSlaveIdx = 2; usSlaveIdx < (usNbrRecogSlaves + 2); usSlaveIdx++)
  {
    usSAdd = prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[usSlaveIdx];
    
    if (usSAdd > CSMD_MAX_SLAVE_ADD)
    {
      CSMD_RuntimeError( prCSMD_Instance, usSAdd, 
                         "CSMD_Check_Slave_Addresses: Sercos slave address > CSMD_MAX_SLAVE_ADD." );
      return (CSMD_ILLEGAL_SLAVE_ADDRESS);
    }
    
    for (usI = (CSMD_USHORT)(usSlaveIdx + 1); usI < (usNbrRecogSlaves + 2); usI++)
    {
      if (usSAdd == prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[usI])
      {
        CSMD_RuntimeError( prCSMD_Instance, usSAdd, 
                           "CSMD_Check_Slave_Addresses: Multiple Sercos slave address." );
        
        prCSMD_Instance->rPriv.boMultipleSAddress = TRUE;
        return (CSMD_ERROR_DOUBLE_RECOGNIZED_ADDRESS);
      }
    }
  }
  
  return (CSMD_NO_ERROR);

} /* end: CSMD_Check_Slave_Addresses() */



/**************************************************************************/ /**
\brief Checks the lists of recognized slaves and projected slaves.
        
\ingroup module_phase
\b Description: \n
   This function builds the following lists:
   - ausProjSlaveAddList[] (sercos-addresses in projected order)
   - ausProjSlaveIdxList[] (sercos-address -> slave index)

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   pusProjectedSlaveList
              Pointer to a Sercos list with Sercos addresses of the projected slaves.\n
              The Recognized slave list must be a subset of the projected slave list.

\return       \ref CSMD_WRONG_PROJECTED_SLAVE_LIST \n
              \ref CSMD_ILLEGAL_SLAVE_ADDRESS \n
              \ref CSMD_ERROR_DOUBLE_ADDRESS \n
              \ref CSMD_PROJ_SLAVES_NOT_ONE_TO_ONE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         18.02.2005
    
***************************************************************************** */
CSMD_FUNC_RET CSMD_ScanOperDrives( CSMD_INSTANCE     *prCSMD_Instance,
                                   const CSMD_USHORT *pusProjectedSlaveList )
{
  
  CSMD_FUNC_RET  eFuncRet;
  CSMD_USHORT    usSlaveIdx;
  CSMD_USHORT    usSAdd;
  CSMD_USHORT    usI;
  CSMD_USHORT    usNbrProjSlaves;
  CSMD_USHORT    usNbrRecogSlaves;
  CSMD_USHORT    usAddList[CSMD_MAX_SLAVE_ADD + 1];   /* Index is S-Add */
  
  
  eFuncRet = CSMD_NO_ERROR;
  
  /* --------------------------------------------------------- */
  /* Step 1 :: Initialize variables                            */
  /* --------------------------------------------------------- */
  
  /* Check list lengths */
  if (   (pusProjectedSlaveList[0]  > pusProjectedSlaveList[1])
      || ((pusProjectedSlaveList[0] | pusProjectedSlaveList[1]) & 0x0001)
      || (pusProjectedSlaveList[0]  > (CSMD_USHORT)(prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves * 2))
    )
  {
    /* Passed projected slave Sercos list
       a) Current length > Maximum length,
       b) Odd number of bytes for current or maximum length,
       c) higher number of slaves than supported
    */
    CSMD_RuntimeError( prCSMD_Instance, 0, 
                       "CSMD_ScanOperDrives: Pass invalid projected slave list" );
    return (CSMD_WRONG_PROJECTED_SLAVE_LIST);
  }
  
  /* --------------------------------------------------------- */
  /* Step 2 :: Get current list length                         */
  /* --------------------------------------------------------- */
  usNbrProjSlaves  = (CSMD_USHORT) (pusProjectedSlaveList[0] / 2);
  usNbrRecogSlaves = prCSMD_Instance->rSlaveList.usNumRecogSlaves;
  
  prCSMD_Instance->rSlaveList.ausProjSlaveAddList[0] = pusProjectedSlaveList[0];
  
  
  /* At least one Sercos address was detected (ausRecogSlaveAddList) more than once. */
  if (TRUE == prCSMD_Instance->rPriv.boMultipleSAddress)
  {
    /* Check for invalid Sercos addresses in the projected slave address list. */
    for (usSlaveIdx = 2; usSlaveIdx < (usNbrProjSlaves + 2); usSlaveIdx++)
    {
      usSAdd = pusProjectedSlaveList[usSlaveIdx];
      
      if (usSAdd > CSMD_MAX_SLAVE_ADD)
      {
        CSMD_RuntimeError( prCSMD_Instance, usSAdd, 
                           "CSMD_Check_Slave_Addresses: Projected Sercos slave address <min or >max address." );
        return (CSMD_ILLEGAL_SLAVE_ADDRESS);
      }
    }
    usNbrProjSlaves = usNbrRecogSlaves;

    /* multiple Sercos addresses in CP0 detected:   */
    /* work with order of the recognized slave list */
    /* Copy the recognized slave list into the projected slave list. */
    for (usSlaveIdx = 0; usSlaveIdx < (usNbrRecogSlaves + 2); usSlaveIdx++)
    {
      prCSMD_Instance->rSlaveList.ausProjSlaveAddList[usSlaveIdx] =
        prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[usSlaveIdx];
    }
    prCSMD_Instance->rSlaveList.usNumProjSlaves = usNbrRecogSlaves;

    /* --------------------------------------------------------- */
    /* : Build ausProjSlaveIdxList[]                             */
    /* --------------------------------------------------------- */
    for (usSlaveIdx = 0; usSlaveIdx < usNbrProjSlaves; usSlaveIdx++)
    {
      usSAdd = prCSMD_Instance->rSlaveList.ausProjSlaveAddList[(CSMD_USHORT)eCSMD_IdnListDataStart + usSlaveIdx];

      prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSAdd] = usSlaveIdx;
    }

    /* --------------------------------------------------------- */
    /* : Build list aeSlaveActive[]                              */
    /* --------------------------------------------------------- */
    for (usI = 0; usI < usNbrRecogSlaves; usI++)
    {
      prCSMD_Instance->rSlaveList.aeSlaveActive[usI] = CSMD_SLAVE_ACTIVE;
    }

  }
  else    /* if (TRUE == prCSMD_Instance->rPriv.bMultipleSAddress) */
  {
    /* Check for multiple Sercos addresses in the projected slaves address list. */
    for (usSlaveIdx = 2; usSlaveIdx < (usNbrProjSlaves + 2); usSlaveIdx++)
    {
      usSAdd = pusProjectedSlaveList[usSlaveIdx];
      
      if (usSAdd > CSMD_MAX_SLAVE_ADD)
      {
        CSMD_RuntimeError( prCSMD_Instance, usSAdd, 
                           "CSMD_Check_Slave_Addresses: Projected Sercos slave address <min or >max address." );
        return (CSMD_ILLEGAL_SLAVE_ADDRESS);
      }
      
      for (usI = (CSMD_USHORT)(usSlaveIdx + 1); usI < (usNbrProjSlaves + 2); usI++)
      {
        if (usSAdd == pusProjectedSlaveList[usI])
        {
          CSMD_RuntimeError( prCSMD_Instance, usSAdd, 
                             "CSMD_Check_Slave_Addresses: Multiple projected Sercos slave address." );
          return (CSMD_ERROR_DOUBLE_ADDRESS);
        }
      }
    }
    
    /* Take the projected slaves list incl. current & maximum length */
    for (usSlaveIdx = 0; usSlaveIdx < (usNbrProjSlaves + 2); usSlaveIdx++)
    {
      prCSMD_Instance->rSlaveList.ausProjSlaveAddList[usSlaveIdx] =
        pusProjectedSlaveList[usSlaveIdx];
      
    }
    
    /* --------------------------------------------------------- */
    /* Step 3 ::                                                 */
    /* --------------------------------------------------------- */
    /* Clear temporary list */
    for (usSAdd = 0; usSAdd <= CSMD_MAX_SLAVE_ADD; usSAdd++)
    {
      usAddList[usSAdd] = 0;
    }
    
    /* Get recognized slaves */
    for (usSlaveIdx = 2; usSlaveIdx < (usNbrRecogSlaves + 2); usSlaveIdx++)
    {
      usSAdd = prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[usSlaveIdx];
      usAddList[usSAdd] += 1;
    }
    
    /* Get projected slaves */
    for (usSlaveIdx = 2; usSlaveIdx < (usNbrProjSlaves + 2); usSlaveIdx++)
    {
      usSAdd = pusProjectedSlaveList[usSlaveIdx];
      usAddList[usSAdd] += 1;
    }
    
    
    for (usSAdd = 0, usSlaveIdx = 0; usSAdd <= CSMD_MAX_SLAVE_ADD; usSAdd++)
    {
      /* Slave recognized and projected? */
      if (usAddList[usSAdd] == 2)
      {
        usSlaveIdx++;
      }
    }
    
    if (usSlaveIdx != usNbrRecogSlaves) 
    {
      /* recognized and projected lists not one to one */
      CSMD_RuntimeError( prCSMD_Instance, usSAdd, 
                         "CSMD_ScanOperDrives: Recognized and projected lists not one to one." );
      return (CSMD_PROJ_SLAVES_NOT_ONE_TO_ONE);
    }  
    
    prCSMD_Instance->rSlaveList.usNumProjSlaves =
      (CSMD_USHORT) (prCSMD_Instance->rSlaveList.ausProjSlaveAddList[0] / 2); /* define for list element length? */
    
    /* --------------------------------------------------------- */
    /* : Build ausProjSlaveIdxList[]                             */
    /* --------------------------------------------------------- */
    for (usSlaveIdx = 0; usSlaveIdx < usNbrProjSlaves; usSlaveIdx++)
    {
      usSAdd = prCSMD_Instance->rSlaveList.ausProjSlaveAddList[(CSMD_USHORT)eCSMD_IdnListDataStart + usSlaveIdx];
      
      prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSAdd] = usSlaveIdx;
    }

    /* --------------------------------------------------------- */
    /* : Build list aeSlaveActive[]                              */
    /* --------------------------------------------------------- */
    for (usI = 0; usI < usNbrRecogSlaves; usI++)
    {
      usSAdd = prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[usI + 2];
      usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSAdd];

      prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] = CSMD_SLAVE_ACTIVE;
    }

  }   /* End: if (TRUE == prCSMD_Instance->rPriv.bMultipleSAddress) */
  
  return (eFuncRet);
  
} /* end: CSMD_ScanOperDrives() */



/**************************************************************************/ /**
\brief Reports AT misses during address scan.

\ingroup module_phase
\todo description
\b Description: This function...

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       TRUE  - telegram fail \n
              FALSE - no telegram fail

\author       AlM
\date         17.02.2014

***************************************************************************** */
CSMD_BOOL CSMD_Telegram_Fail_CP0( const CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_BOOL  boFail = FALSE;

  if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
  {
    if (   (FALSE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP1)
        || (FALSE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP2)
        || (FALSE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP1)
        || (FALSE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP2))
    {
      boFail = TRUE;
    }
  }
  else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
  {
    if (   (FALSE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP1)
        || (FALSE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP1) )
    {
      boFail = TRUE;
    }
  }
  else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
  {
    if (   (FALSE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP2)
        || (FALSE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP2) )
    {
      boFail = TRUE;
    }
  }
  else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)  /* interrupted ring / two lines */
  {
    if (   (FALSE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP1)
        || (FALSE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP2)
        || (FALSE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP1)
        || (FALSE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP2))
    {
      boFail = TRUE;
    }
  }
  else if ( prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING )
  {
    if (   (!(prCSMD_Instance->rPriv.rRedundancy.boNewDataP1 == TRUE))
        || (!(prCSMD_Instance->rPriv.rRedundancy.boNewDataP2 == TRUE)) )
    {
      boFail = TRUE;
    }
  }

  return (boFail);

} /* End: CSMD_Telegram_Fail_CP0() */

/*! \endcond */ /* PRIVATE */


/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
04 Nov 2013 WK
  - Defdb00163766
    Fixed big endian access in function CSMD_Check_Recognized_Topology_CP0().
05 Nov 2013 WK
  - Defdb00163766
    CSMD_Check_Recognized_Topology_CP0():
    Fixed check for link on both ports with line topology.
17 Feb 2014 WK
  - Defdb00163766
    CSMD_Check_Recognized_Topology_CP0():
    Fixed check for defect ring topology.
  - Defdb00166642
    CSMD_Detect_Available_Slaves()
    Fixed default of SeqCnt if required topology ring is preselected and 
    topology line port 2 is detected. This is essential for the remote 
    addressing of multi-slave-devices.
24 Nov 2014 WK
  - Enhanced function CSMD_GetHW_Settings() with mode for setting of
    default values.
02 Dec 2014 WK
  - CSMD_GetSlaveAddresses()
    - Fixed duration of address scanning.
    - Build address list in either Sercos cycle to avoid problems with
      changes in the topology index fields.
    - Consideration of endianness reading the topology index fields.
      (Get Sercos addresses with new function CSMD_BuildAvailableSlaveList().)
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
28 May 2015 WK
  - CSMD_ScanOperDrives()
    In case of multiple recognized addresses the lists ausProjSlaveIdxList[]
    and aeSlaveActive[] were build incorrect.
22 Jul 2015 WK
  - CSMD_GetSlaveAddresses()
    Allow ring topology with no slaves.
27 Jul 2015 WK
  - CSMD_GetSlaveAddresses()
    - Fixed building of rSlaveAvailable2.ausAddresses[] causing
      a redundancy problem with ring topology.
    - Invert the available slave list for port 2 in case of
      ring topology.
05 Nov 2015 WK
  - Defdb00182857
    CSMD_GetHW_Settings()
    Configuration of boHP_Field_All_Tel added.
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
28 Jan 2016 WK
  - Defdb00182067
    CSMD_BuildAvailableSlaveList(): Static definition removed.
23 Feb 2016 WK
  - Defdb00185143
    CSMD_Detect_Available_Slaves()
    Fixed faulty ring delay measurement for the case of a reset stable timer.
31 Mar 2016 WK
  - Defdb00186013
    CSMD_Detect_Available_Slaves()
    Delete the available slave number in case of a address scan timeout.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
 11 Jul 2016 WK
 - Defdb00188354
   CSMD_Detect_Available_Slaves()
   Fixed division by zero during tNetwork calculation.
  
------------------------------------------------------------------------------
*/
