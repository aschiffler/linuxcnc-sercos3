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
\file   CSMD_CYCLIC.c
\author AlM
\date   15.11.2013
\brief  This File contains the public API functions and private functions
        for the cyclic interoperation.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"
#include "CSMD_HAL_DMA.h"

#include "CSMD_HOTPLUG.h"
#include "CSMD_TOPOLOGY.h"


#define SOURCE_CSMD
#include "CSMD_CYCLIC.h"


/*lint -save -e818 Pointer parameter '' could be declared as pointing to const */

/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/**************************************************************************/ /**
\brief This function is the framework for necessary cyclic actions.

\ingroup func_cyclic
\b Description: \n
   This function contains all necessary CoSeMa-related cyclic actions, including
   - multi buffer handling
   - telegram validation
   - topology handling
   - telegram copy mechanisms for CP0 and CP1 (topology and CP1 handshake process)
   - copy mechanisms for device control and device status
   - read and write mechanisms of hot plug fields
   - connection handling including the connection state machine

<B>Call Environment:</B> \n
   This function shall be called as of CP0 once per Sercos cycle. Due to its
   real-time critical features, it should be called from an interrupt cyclically. 
   The time for triggering the interrupt should be determined in the timing 
   calculation.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_TEL_ERROR_OVERRUN \n
              \ref CSMD_TOPOLOGY_CHANGE \n
              \ref CSMD_NO_ERROR \n
              \n
              \ref CSMD_NO_LINK_ATTACHED \n
              \ref CSMD_NO_TELEGRAMS_RECEIVED \n
              \ref CSMD_MST_MISS \n
              \ref CSMD_MST_WINDOW_ERROR \n

\author       AlM
\date         15.11.2013

*************************************************************************** */
CSMD_FUNC_RET CSMD_CyclicHandling( CSMD_INSTANCE  *prCSMD_Instance )
{
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT   usI;
  CSMD_USHORT   usBufP1;    /* active read buffer for master port 1 */
  CSMD_USHORT   usBufP2;    /* active read buffer for master port 2 */
  CSMD_ULONG    ulRXBUFTV;  /* RXBUFTV register */

  /* This function is not executed during communication phase switching process */
  if (prCSMD_Instance->rPriv.eMonitoringMode != CSMD_MONITORING_OFF)
  {
    /*  [TODO MULTIBUFFER-HANDLING IN EIGENE FUNKTION AUSLAGERN?] */

    /* Get active write buffer */
    prCSMD_Instance->rPriv.usTxBuffer =
      (CSMD_USHORT) CSMD_HAL_Usable_Tx_BufferSysA( &prCSMD_Instance->rCSMD_HAL );

#ifdef CSMD_PCI_MASTER
    if (prCSMD_Instance->rPriv.boDMA_IsActive != TRUE)
#endif
    {
      /* Request new buffer for next cycle */
      CSMD_HAL_Request_New_Rx_BufferSysA( &prCSMD_Instance->rCSMD_HAL );

      /* Get active read buffer for both ports */
      CSMD_HAL_Usable_Rx_BufferSysA( &prCSMD_Instance->rCSMD_HAL,
                                     &prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1],
                                     &prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2] );
    }

    /* ------------------------------------------------------------- */
    /* Telegram validation                                           */
    /* ------------------------------------------------------------- */
    CSMD_HAL_Get_IsValid_Rx_BufferSysA( &prCSMD_Instance->rCSMD_HAL,
                                        &prCSMD_Instance->rPriv.rRedundancy.boNewDataP1,
                                        &prCSMD_Instance->rPriv.rRedundancy.boNewDataP2 );

    /* copy RXBUFTV register (used for redundancy purposes in terms of cyclic data) */
    ulRXBUFTV = CSMD_HAL_GetValidTelegramsSysA (&prCSMD_Instance->rCSMD_HAL);

    prCSMD_Instance->rPriv.rRedundancy.aulATBufValid[CSMD_PORT_1] =
        (ulRXBUFTV & CSMD_HAL_RXBUFTV_P1_MASK) >> CSMD_HAL_RXBUFTV_AT_SHIFT;
    prCSMD_Instance->rPriv.rRedundancy.aulATBufValid[CSMD_PORT_2] =
        (ulRXBUFTV & CSMD_HAL_RXBUFTV_P2_MASK) >> (CSMD_HAL_RXBUFTV_AT_SHIFT + CSMD_HAL_RXBUFTV_PORT_SHIFT);

    /* copy TGSR registers (used for redundancy purposes in terms of topology) */
    CSMD_ReadTelegramStatus (prCSMD_Instance);

    /* In case of activated DMA, the usable buffer was determined outside this function! */
    usBufP1 = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1];
    usBufP2 = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2];

    if (prCSMD_Instance->sCSMD_Phase >= CSMD_SERC_PHASE_0)
    {
#ifdef CSMD_PCI_MASTER
      /* ------------------------------------------------------------- */
      /* Wait for completed DMA transfer from RxRam                    */
      /* ------------------------------------------------------------- */
      if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
      {
        if (   prCSMD_Instance->rPriv.boP1_active
            || prCSMD_Instance->rPriv.boP2_active)
        {
          /* Wait until Tx-DMA transfer complete */
          while ( CSMD_IsBusy_Tx_DMA( prCSMD_Instance,
                                      (CSMD_USHORT) (  prCSMD_Instance->rPriv.ausTxDMA_Start_P1[usBufP1]
                                                     | prCSMD_Instance->rPriv.ausTxDMA_Start_P2[usBufP2]) )
                ) {};
        }
      }
      else
#endif
      {
        if (prCSMD_Instance->sCSMD_Phase <  CSMD_SERC_PHASE_2)
        {
          /* ------------------------------------------------------------------------- */
          /* Copy telegrams for CP0 and CP1 (for topology and CP1 handshake mechanism) */
          /* ------------------------------------------------------------------------- */
          for (usI = 0; usI < CSMD_MAX_TEL_P0_2; usI++)    /* read AT telegrams from FPGA */
          {
            if (prCSMD_Instance->rPriv.aboAT_used[usI])
            {
              if (prCSMD_Instance->rPriv.boP1_active)
              {
                CSMD_Read_Rx_Ram(
                  (CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_NcP1_AT_Copy[usI],      /* Dst: Pointer to local RAM */
                  (CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBufP1][usI],  /* Src: Pointer to FPGA */
                  (CSMD_USHORT)(prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usI] / 2) );              /* Nbr of Words to copied */
              }

              if (prCSMD_Instance->rPriv.boP2_active)
              {
                CSMD_Read_Rx_Ram(
                  (CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_NcP2_AT_Copy[usI],
                  (CSMD_USHORT *)(CSMD_VOID *) prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBufP2][usI],
                  (CSMD_USHORT)(prCSMD_Instance->rPriv.usSERC3_Length_AT_Copy[usI] / 2) );
              }
            }
          }
        }
      }
    }

    /* ------------------------------------------------------------- */
    /* Topology handling / Redundancy                                */
    /* ------------------------------------------------------------- */
    if (prCSMD_Instance->sCSMD_Phase >= CSMD_SERC_PHASE_1)
    {
      /* [TODO AUFLÖSUNG EINES IN CP0 DETEKTIERTEN DEFECT RING VOR TOPOLOGY CHECK] */

      eFuncRet = CSMD_CheckTopology (prCSMD_Instance);
#ifdef CSMD_DEBUG_CONN_STM  /* only for debugging !!! */
      /**********************************/
      /* NUR TEMPORAER FÜR DEBUG-ZWECKE */
      if (usZaehler >= CSMD_DBG_NBR_CYCLES)
      {
        usZaehler = 0;
      }
      aeReturn[usZaehler] = eFuncRet;
      usZaehler++;
      /**********************************/
#endif
      /* ------------------------------------------------------------- */
      /* Handling of C-DEV                                             */
      /* ------------------------------------------------------------- */
      CSMD_CyclicDeviceControl (prCSMD_Instance);

      /* ------------------------------------------------------------- */
      /* Handling of S-DEV                                             */
      /* ------------------------------------------------------------- */
      CSMD_CyclicDeviceStatus (prCSMD_Instance);
    }

    if (prCSMD_Instance->sCSMD_Phase >= CSMD_SERC_PHASE_3)
    {
      /* ------------------------------------------------------------- */
      /* Write MDT0 Hot Plug Field                                     */
      /* ------------------------------------------------------------- */
      CSMD_CyclicHotPlug_MDT (prCSMD_Instance);

      /* ------------------------------------------------------------- */
      /* Read AT0 Hot Plug Fields of both ports                        */
      /* ------------------------------------------------------------- */
      CSMD_CyclicHotPlug_AT (prCSMD_Instance);
    }

    if (prCSMD_Instance->sCSMD_Phase == CSMD_SERC_PHASE_4)
    {
      /* ------------------------------------------------------------- */
      /* Determination of connections to be produced                   */
      /* ------------------------------------------------------------- */
      CSMD_CyclicConnection (prCSMD_Instance);

      /* ------------------------------------------------------------- */
      /* Update Connection state machine                               */
      /* ------------------------------------------------------------- */
      CSMD_EvaluateConnections (prCSMD_Instance);
    }

    if (prCSMD_Instance->sCSMD_Phase >= CSMD_SERC_PHASE_3)
    {
#ifdef CSMD_PCI_MASTER
      /* ------------------------------------------------------------- */
      /* Start DMA transfer to TxRam                                   */
      /* ------------------------------------------------------------- */
      if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
      {
        /* Request new buffer for next cycle */
        CSMD_HAL_Request_New_Tx_BufferSysA( &prCSMD_Instance->rCSMD_HAL );
      }
      else
#endif
      {
        /* Request new buffer for next cycle */
        CSMD_HAL_Request_New_Tx_BufferSysA( &prCSMD_Instance->rCSMD_HAL );
      }
    }
#if (defined CSMD_PCI_MASTER) && (CSMD_MAX_HW_CONTAINER == 0)
    else if (prCSMD_Instance->sCSMD_Phase == CSMD_SERC_PHASE_2)
    {
      /* Note: Combined operation with IP-Core- and emulated-SVC causes problems with double line topology! */
      if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
      {
        /* Clear the DMA Rx Ready Flags. */
        for (usI = 0; usI < (CSMD_USHORT) CSMD_DMA_NBR_USED_CHANNELS; usI++)
        {
          if (prCSMD_Instance->rPriv.ausRxDMA_Start[prCSMD_Instance->rPriv.usTxBuffer] & (1UL<<usI))
          {
            prCSMD_Instance->prTelBuffer->rLocal.rDMA_RxRdy.aulFlag[usI] = 0;
          }
        }
        CSMD_HAL_StartRxDMA( &prCSMD_Instance->rCSMD_HAL,
                             prCSMD_Instance->rPriv.ausRxDMA_Start[prCSMD_Instance->rPriv.usTxBuffer] );
      }
    }
#endif  /* #if (defined CSMD_PCI_MASTER) && (CSMD_MAX_HW_CONTAINER == 0) */

  } /*   if (prCSMD_Instance->rPriv.eMonitoringMode != CSMD_MONITORING_OFF) */

  return (eFuncRet);

}  /* end: CSMD_CyclicHandling() */


/**************************************************************************/ /**
\brief Sets the producer state of a master-produced connection.

\ingroup func_cyclic
\b Description: \n
   This function can be called by the master in order to stop a connection or
   to restart a connection which has been stopped. If the consigned connection
   state is valid, the commanded state is set. Otherwise an error message is
   generated.

<B>Call Environment:</B> \n
   This function can be called from an interrupt in CP4.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usConnIdx
              Connection index selection

\param [in]   eComState
              Valid commanded connection states:
              - CSMD_PROD_STATE_READY
              - CSMD_PROD_STATE_STOPPING
              - CSMD_PROD_STATE_PREPARE

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_CONNECTION_NOT_CONFIGURED \n
              \ref CSMD_CONNECTION_NOT_MASTERPRODUCED \n
              \ref CSMD_ILLEGAL_CONNECTION_STATE \n
              \ref CSMD_NO_ERROR \n

\author       AlM
\date         02.12.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_SetConnectionState( CSMD_INSTANCE   *prCSMD_Instance,
                                       CSMD_USHORT      usConnIdx,
                                       CSMD_PROD_STATE  eComState )
{
  CSMD_FUNC_RET          eFuncRet = CSMD_NO_ERROR;
  CSMD_CONN_MASTERPROD  *prMasterProd = &prCSMD_Instance->rPriv.parConnMasterProd[usConnIdx];
  CSMD_CONN_SLAVEPROD   *prSlaveProd  = &prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx];

  if (prCSMD_Instance->sCSMD_Phase != CSMD_SERC_PHASE_4)
  {
    eFuncRet = CSMD_WRONG_PHASE;
  }
  else
  {
    /* connection is produced by a slave => state can not be set by master */
    if (prSlaveProd->usProduced)
    {
      eFuncRet = CSMD_CONNECTION_NOT_MASTERPRODUCED;
    }
    /* connection is not configured */
    else if (prMasterProd->usProduced == 0)
    {
      eFuncRet = CSMD_CONNECTION_NOT_CONFIGURED;
    }
    else /* connection is produced by master */
    {
      if (eComState == CSMD_PROD_STATE_READY)
      {
        if (   prMasterProd->eState == CSMD_PROD_STATE_PREPARE
            || prMasterProd->eState == CSMD_PROD_STATE_STOPPING )
        {
          /* transition STOPPED -> PREPARE is redundant
          prMasterProd->eState = CSMD_PROD_STATE_PREPARE;
          prMasterProd->usC_Con = 0;
          */
          prMasterProd->eState = CSMD_PROD_STATE_READY;
          prMasterProd->usC_Con = CSMD_C_CON_PRODUCER_READY;
        }
      }
      else if (eComState == CSMD_PROD_STATE_STOPPING)
      {
        prMasterProd->eState = CSMD_PROD_STATE_STOPPING;
        prMasterProd->usC_Con |= CSMD_C_CON_FLOW_CONTROL;
      }
      else if (eComState == CSMD_PROD_STATE_PREPARE)
      {
        prMasterProd->eState = CSMD_PROD_STATE_PREPARE;
        prMasterProd->usC_Con = 0;
      }
      else /* given state is not 'ready', 'stopping' or 'prepare' */
      {
        eFuncRet = CSMD_ILLEGAL_CONNECTION_STATE;
      }
    }
  }

  return (eFuncRet);

}  /* end: CSMD_SetConnectionState() */


/**************************************************************************/ /**
\brief Gets the connection state of any configured connection.

\ingroup func_cyclic
\b Description: \n
   This function returns the connection state of the connection with the consigned
   index.

<B>Call Environment:</B> \n
   This function can be called from an interrupt in CP4 and must be called for
   any connection index before CSMD_GetConnectionData() is called with the respective
   index within the same Sercos cycle.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usConnIdx
              Connection index selection

\param [out]  pusState
              Output pointer to the connection state

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_CONNECTION_NOT_CONFIGURED \n
              \ref CSMD_NO_ERROR \n

\author       AlM
\date         04.12.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_GetConnectionState( CSMD_INSTANCE *prCSMD_Instance,
                                       CSMD_USHORT    usConnIdx,
                                       CSMD_USHORT   *pusState )
{
  CSMD_FUNC_RET          eFuncRet = CSMD_NO_ERROR;
  CSMD_CONN_MASTERPROD  *prMasterProd = &prCSMD_Instance->rPriv.parConnMasterProd[usConnIdx];
  CSMD_CONN_SLAVEPROD   *prSlaveProd =  &prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx];

  if (prCSMD_Instance->sCSMD_Phase == CSMD_SERC_PHASE_4)
  {
    /* check if connection is configured and if it is produced by master or by slave */
    if (prSlaveProd->usProduced)
    {
      *pusState = (CSMD_USHORT)prSlaveProd->eState;
    }
    else if (prMasterProd->usProduced)
    {
      *pusState = (CSMD_USHORT)prMasterProd->eState;
    }
    else
    {
      eFuncRet = CSMD_CONNECTION_NOT_CONFIGURED;
    }
  }
  else
  {
    eFuncRet = CSMD_WRONG_PHASE;
  }

  return (eFuncRet);

}  /* end: CSMD_GetConnectionState() */


/**************************************************************************/ /**
\brief Copies cyclic application data of master-produced connections
       from a given source.

\ingroup func_cyclic
\b Description: \n
   This function copies the connection data of the connection with the selected
   index to the respective offset in Tx ram. Furthermore, it copies the consigned
   values of configured real-time bits.
   The connection control word is excluded, it is copied from the CoSeMa internal
   connection structure.

<B>Call Environment:</B> \n
   This function can be called from an interrupt in CP4 after
   CSMD_CyclicHandling() has been processed.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usConnIdx
              connection index selection

\param [in]   pusConnData
              pointer to connection data to be copied (after C_CON)

\param [in]   usRTBits
              values of configured real-time-bits for the selected connection
              - Bit 6: Real-time bit 1
              - Bit 7: Real-time bit 2
              - All other bits are ignored

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_CONNECTION_NOT_CONFIGURED \n
              \ref CSMD_CONNECTION_NOT_MASTERPRODUCED \n
              \ref CSMD_NO_ERROR \n

\author       AlM
\date         03.12.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_SetConnectionData( CSMD_INSTANCE *prCSMD_Instance,
                                      CSMD_USHORT    usConnIdx,
                                      CSMD_USHORT   *pusConnData,
                                      CSMD_USHORT    usRTBits )
{
  CSMD_FUNC_RET          eFuncRet;
  CSMD_CONN_MASTERPROD  *prMasterProd = &prCSMD_Instance->rPriv.parConnMasterProd[usConnIdx];
  CSMD_CONN_SLAVEPROD   *prSlaveProd  = &prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx];


  if (prCSMD_Instance->sCSMD_Phase != CSMD_SERC_PHASE_4)
  {
    eFuncRet = CSMD_WRONG_PHASE;
  }
  else
  {
    /* connection is produced by a slave: master can not set connection data */
    if (prSlaveProd->usProduced)
    {
      eFuncRet = CSMD_CONNECTION_NOT_MASTERPRODUCED;
    }
    /* connection is not configured */
    else if (prMasterProd->usProduced == 0)
    {
      eFuncRet = CSMD_CONNECTION_NOT_CONFIGURED;
    }
    else
    {
      CSMD_USHORT *pusDest = prMasterProd->apusConnTxRam[prCSMD_Instance->rPriv.usTxBuffer];
      /* usLength: connection data length (without C-CON) in words */
      CSMD_USHORT  usLength = (CSMD_USHORT)(prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5 / 2 - 1);

      if (   (prMasterProd->eState == CSMD_PROD_STATE_READY)
          || (prMasterProd->eState == CSMD_PROD_STATE_WAITING) )
      {
        prMasterProd->eState = CSMD_PROD_STATE_PRODUCING;
      }

      /* merge C-CON with connection data and include real-time bits */
      *pusDest++ = (CSMD_USHORT)(prMasterProd->usC_Con | (usRTBits & CSMD_C_CON_RTB_MASK));

      /* copy connection data (after C-CON) to TxRam */
      CSMD_Write_Tx_Ram ( pusDest,      /* destination pointer */
                          pusConnData,  /* source pointer */
                          usLength );   /* length in words */

      eFuncRet = CSMD_NO_ERROR;
    }
  }

  return (eFuncRet);

}  /* end: CSMD_SetConnectionData() */


/**************************************************************************/ /**
\brief Copies cyclic data of master-consumed connections
       to a given destination.

\ingroup func_cyclic
\b Description: \n
   This function copies the connection data of the connection with the selected
   index from RxRam to the consigned address. Depending on the current topology,
   it will try to read connection data from the alternative port if it is not
   available on the connection's preferred port.

<B>Call Environment:</B> \n
   This function can be called from an interrupt in CP4 after
   CSMD_CyclicHandling() has been processed and the connection state of the
   respective connection has been read, e.g. CSMD_GetConnectionState() has been
   called using the same connection index.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usConnIdx
              connection index selection

\param [in]   pusDestination
              pointer to destination address of connection data

\return       \ref CSMD_CONNECTION_NOT_CONFIGURED \n
              \ref CSMD_CONNECTION_NOT_SLAVEPRODUCED \n
              \ref CSMD_CONNECTION_DATA_INVALID \n
              \ref CSMD_WRONG_PHASE \n
              \ref CSMD_NO_ERROR \n

\author       AlM
\date         03.12.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_GetConnectionData( CSMD_INSTANCE *prCSMD_Instance,
                                      CSMD_USHORT    usConnIdx,
                                      CSMD_USHORT   *pusDestination )
{
  CSMD_FUNC_RET          eFuncRet = CSMD_NO_ERROR;
  CSMD_CONN_MASTERPROD  *prMasterProd = &prCSMD_Instance->rPriv.parConnMasterProd[usConnIdx];
  CSMD_CONN_SLAVEPROD   *prSlaveProd  = &prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx];

  if (prCSMD_Instance->sCSMD_Phase == CSMD_SERC_PHASE_4)
  {
    /* connection is produced by the master: get connection data does not make sense */
    if (prMasterProd->usProduced)
    {
      eFuncRet = CSMD_CONNECTION_NOT_SLAVEPRODUCED;
    }
    /* connection is not configured */
    else if (prSlaveProd->usProduced == 0)
    {
      eFuncRet = CSMD_CONNECTION_NOT_CONFIGURED;
    }
    else
    {
      /* check connection state (not necessary?)
      if (prSlaveProd->eState == CSMD_CONS_STATE_CONSUMING)
      { */
        /* check for slave valid of the connection's producer in current cycle */
        if (prCSMD_Instance->arDevStatus[prSlaveProd->usProdIdx].usMiss == 0)
        {
          /* check preferred master port for connection (may change in CSMD_EvaluateConnections() ) */
          if (prCSMD_Instance->rPriv.ausPrefPortBySlave[prSlaveProd->usProdIdx] == CSMD_PORT_1)
          {
            /* copy connection data from RxRam port 1 */
            CSMD_Read_Rx_Ram
               ( pusDestination,                                                                /* destination pointer */
                 prSlaveProd->apusConnRxRam[CSMD_PORT_1][prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1]], /* source pointer */
                 (CSMD_USHORT)(prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5 / 2) ); /* length in words */
          }
          else
          {
            /* copy connection data from RxRam port 2 */
            CSMD_Read_Rx_Ram
               ( pusDestination,                                                                /* destination pointer */
                 prSlaveProd->apusConnRxRam[CSMD_PORT_2][prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2]], /* source pointer */
                 (CSMD_USHORT)(prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE5 / 2) ); /* length in words */
          }
        }
        else
        {
          eFuncRet = CSMD_CONNECTION_DATA_INVALID;
        }
    /*}
      else
      {
        eFuncRet = CSMD_INVALID_CONNECTION_STATE;
      }*/
    }
  }
  else
  {
    eFuncRet = CSMD_WRONG_PHASE;
  }

  return (eFuncRet);

}  /* end: CSMD_GetConnectionData() */


/**************************************************************************/ /**
\brief Returns the age of the consumed connection data in producer cycles.

\ingroup func_cyclic
\b Description: \n
   This function writes the number of consecutive misses of connection data of
   the selected connection to the pointer given by the calling instance. This
   counter contains the age of the consumed connection data in producer cycles.

<B>Call Environment:</B> \n
   This function can be called from an interrupt or from a task in CP4.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usConnIdx
              connection index selection

\param [out]  pusCycles
              pointer to data delay in producer cycles

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_CONNECTION_NOT_SLAVEPRODUCED \n
              \ref CSMD_NO_ERROR \n

\author       AlM
\date         05.12.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_GetConnectionDataDelay( CSMD_INSTANCE *prCSMD_Instance,
                                           CSMD_USHORT    usConnIdx,
                                           CSMD_USHORT   *pusCycles )
{
  CSMD_FUNC_RET         eFuncRet = CSMD_NO_ERROR;
  CSMD_CONN_SLAVEPROD  *prSlaveProd = &prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx];

  if (prCSMD_Instance->sCSMD_Phase == CSMD_SERC_PHASE_4)
  {
    /* check if connection is produced by a slave */
    if (prSlaveProd->usProduced)
    {
      *pusCycles = prSlaveProd->usConsecErr;
    }
    /* return error message if connection is not configured or produced by the master */
    else
    {
      eFuncRet = CSMD_CONNECTION_NOT_SLAVEPRODUCED;
    }
  }

  return (eFuncRet);

}  /* end: CSMD_GetConnectionDataDelay() */


/**************************************************************************/ /**
\brief Resets the consumer state of the selected connection.

\ingroup func_cyclic
\b Description: \n
   This function performs the transition from consumer state 'error' to
   'prepare' for the selected connection if it is produced by a slave.
   The clearance is performed only when the connection is in state error.

<B>Call Environment:</B> \n
   This function can be called from an interrupt or from a task in CP4.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usConnIdx
              connection index selection

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_CONNECTION_NOT_SLAVEPRODUCED \n
              \ref CSMD_NO_ERROR \n

\author       AlM
\date         05.12.2013

***************************************************************************** */
CSMD_FUNC_RET CSMD_ClearConnectionError( CSMD_INSTANCE *prCSMD_Instance,
                                         CSMD_USHORT    usConnIdx )
{
  CSMD_FUNC_RET        eFuncRet;
  CSMD_CONN_SLAVEPROD *prSlaveProd = &prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx];

  if (prCSMD_Instance->sCSMD_Phase == CSMD_SERC_PHASE_4)
  {
    /* check if connection is produced by a slave */
    if (prSlaveProd->usProduced)
    {
      if (prSlaveProd->eState == CSMD_CONS_STATE_ERROR)
      {
        prSlaveProd->eState = CSMD_CONS_STATE_PREPARE;  /* reset consumer state */
        prSlaveProd->usAbsoluteErr = 0;
        prSlaveProd->usConsecErr = 0;
      }
      eFuncRet = CSMD_NO_ERROR;
    } /* if (prSlaveProd->usProduced) */

    /* return error message if connection is not configured or produced by the master */
    else
    {
      eFuncRet = CSMD_CONNECTION_NOT_SLAVEPRODUCED;
    }
  } /* if (prCSMD_Instance->sCSMD_Phase == CSMD_SERC_PHASE_4) */
  else
  {
    eFuncRet = CSMD_WRONG_PHASE;
  }

  return (eFuncRet);

}  /* end: CSMD_ClearConnectionError() */



/**************************************************************************/ /**
\brief Gets the active Tx buffer number of buffer system A.

\ingroup module_buffer
\b Description: \n
   This function returns the active Tx buffer number of Tx Buffer system A
   that is usable by the system (processor).

<B>Call Environment:</B> \n
   This function can be called from either an interrupt or a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in,out] pusBuffer
                Active Tx buffer number for system use:
                - 0 = buffer 0
                - 1 = buffer 1
                - 2 = buffer 2
                - 3 = buffer 3

\return       none

\author       WK
\date         31.01.2011

***************************************************************************** */
CSMD_VOID CSMD_Usable_WriteBufferSysA( CSMD_INSTANCE *prCSMD_Instance,
                                       CSMD_USHORT   *pusBuffer )
{

  *pusBuffer = (CSMD_USHORT) CSMD_HAL_Usable_Tx_BufferSysA( &prCSMD_Instance->rCSMD_HAL );

} /* end: CSMD_Usable_WriteBufferSysA() */



/**************************************************************************/ /**
\brief Gets active Rx buffer number for both ports of buffer system A.

\ingroup module_buffer
\b Description: \n
   This function returns the active Rx buffer number of buffer system A that
   is usable by the system (processor).

<B>Call Environment:</B> \n
   This function can be called from either an interrupt or a task.

\param [in]     prCSMD_Instance
                Pointer to memory range allocated for the variables of the
                CoSeMa instance

\param [in,out] pusBufferP1   Active Rx buffer number port 1 for system use:
                              - 0 = buffer 0
                              - 1 = buffer 1
                              - 2 = buffer 2

\param [in,out] pusBufferP2   Active Rx buffer number port 2 for system use:
                              - 0 = buffer 0
                              - 1 = buffer 1
                              - 2 = buffer 2

\return         none

\author         WK
\date           31.01.2011

***************************************************************************** */
CSMD_VOID CSMD_Usable_ReadBufferSysA( CSMD_INSTANCE *prCSMD_Instance,
                                      CSMD_USHORT   *pusBufferP1,
                                      CSMD_USHORT   *pusBufferP2 )
{

  CSMD_HAL_Usable_Rx_BufferSysA( &prCSMD_Instance->rCSMD_HAL,
                                 pusBufferP1,
                                 pusBufferP2 );

} /* end: CSMD_Usable_ReadBufferSysA() */



/**************************************************************************/ /**
\brief Gets valid status of active Rx buffer for both ports.of buffer system A.

\ingroup module_buffer
\b Description: \n
   This function returns the status of the active Rx buffer of buffer system A.

<B>Call Environment:</B> \n
   This function can be called from either an interrupt or a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [out]  pboNewValidBufP1
              Status of the active Rx buffer system A port 1:
              - TRUE:  Data are new and valid for all configured ATs.
              - FALSE: Data are not valid.

\param [out]  pboNewValidBufP2
              Status of the active Rx buffer system A port 2:
              - TRUE:  Data are new and valid for all configured ATs.
              - FALSE: Data are not valid.

\return       none

\author       WK
\date         18.03.2011

***************************************************************************** */
CSMD_VOID CSMD_Get_IsValid_Rx_BufferSysA( CSMD_INSTANCE *prCSMD_Instance,
                                          CSMD_BOOL     *pboNewValidBufP1,
                                          CSMD_BOOL     *pboNewValidBufP2 )
{

  *pboNewValidBufP1 = prCSMD_Instance->rPriv.rRedundancy.boNewDataP1;
  *pboNewValidBufP2 = prCSMD_Instance->rPriv.rRedundancy.boNewDataP2;

} /* end: CSMD_Get_IsValid_Rx_BufferSysA() */



/**************************************************************************/ /**
\brief Retrieves the active interrupts from the FPGA and returns
       them in a bit list.

\ingroup sync_internal
\b Description: \n
   No further description.

  | Interrupt\n Number | Interrupt\n Reset\n Register 0 | Name | Event              |
  | :-- | :----- | :---------------- | :------------------------------------------- |
  |  0  | Bit  0 | Int_TINT0         | Event TINT[0], created by timer/counter TCNT |
  |  1  | Bit  1 | Int_TINT1         | Event TINT[1], created by timer/counter TCNT |
  |  2  | Bit  2 | Int_TINT2         | Event TINT[2], created by timer/counter TCNT |
  |  3  | Bit  3 | Int_TINT3         | Event TINT[3], created by timer/counter TCNT |
  |  4  | Bit  4 | Int_TINTMAX       | Event TMAX, created by timer/counter TCNT    |
  | ... |        |                   |                                              |
  |  7  | Bit  7 | Int_DivClk        | Interrupt from DivClk unit                   |
  |  8  | Bit  8 | Int_IPIntPort1    | Event IP port 1                              |
  |  9  | Bit  9 | Int_IPIntPort2    | Event IP port 2                              |
  | ... |        |                   |                                              |
  | 12  | Bit 12 | Int_RxBufReqPort1 | Internal receive buffer change for port 1    |
  | 13  | Bit 13 | Int_RxBufReqPort2 | Internal receive buffer change for port 2    |

<B>Call Environment:</B> \n
   Advantageously, this function is called within an interrupt service routine
   if the FPGA is the cycle master. Thus, the reason causing the interrupt can be
   determined. If the FPGA is not the cycle master, the function may also be called
   from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [out]  pulInt
              Output pointer to interrupt status

\return       \ref CSMD_NO_ERROR \n

\author       MS
\date         08.04.2005

*************************************************************************** */
CSMD_FUNC_RET CSMD_CheckInt( CSMD_INSTANCE *prCSMD_Instance,
                             CSMD_ULONG    *pulInt )
{

  /* read interrupt status IR0 */
  *pulInt = CSMD_HAL_GetInterrupt(&prCSMD_Instance->rCSMD_HAL, TRUE, FALSE);
  return (CSMD_NO_ERROR);

} /* end: CSMD_CheckInt() */



/**************************************************************************/ /**
\brief Clears FPGA interrupts (no SVC interrupts) from the interrupt status
       register where the corresponding bit in the bit list is set.

\ingroup sync_internal
\b Description: \n
   No further description is required.

  | Interrupt\n Number | Interrupt\n Reset\n Register 0 | Name | Event              |
  | :-- | :----- | :---------------- | :------------------------------------------- |
  |  0  | Bit  0 | Int_TINT0         | Event TINT[0], created by timer/counter TCNT |
  |  1  | Bit  1 | Int_TINT1         | Event TINT[1], created by timer/counter TCNT |
  |  2  | Bit  2 | Int_TINT2         | Event TINT[2], created by timer/counter TCNT |
  |  3  | Bit  3 | Int_TINT3         | Event TINT[3], created by timer/counter TCNT |
  |  4  | Bit  4 | Int_TINTMAX       | Event TMAX, created by timer/counter TCNT    |
  | ... |        |                   |                                              |
  |  7  | Bit  7 | Int_DivClk        | Interrupt from DivClk unit                   |
  |  8  | Bit  8 | Int_IPIntPort1    | Event IP port 1                              |
  |  9  | Bit  9 | Int_IPIntPort2    | Event IP port 2                              |
  | ... |        |                   |                                              |
  | 12  | Bit 12 | Int_RxBufReqPort1 | Internal receive buffer change for port 1    |
  | 13  | Bit 13 | Int_RxBufReqPort2 | Internal receive buffer change for port 2    |

<B>Call Environment:</B> \n
   Advantageously, this function is called within an interrupt service routine
   if the FPGA is the cycle master. If the FPGA is not the cycle master, the
   function may also be called from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [out]  ulIntClearMask
              Bitlist. Interrupts whose bits are set in this list will be cleared.

\return       \ref CSMD_NO_ERROR \n

\author       MS
\date         08.04.2005
*************************************************************************** */
CSMD_FUNC_RET CSMD_ClearInt( CSMD_INSTANCE *prCSMD_Instance,
                             CSMD_ULONG     ulIntClearMask )
{

  /* reset interrupt status IR0 */
  CSMD_HAL_ClearInterrupt(&prCSMD_Instance->rCSMD_HAL, TRUE, FALSE, ulIntClearMask);

  return (CSMD_NO_ERROR);

} /* end: CSMD_ClearInt() */



#ifdef CSMD_PCI_MASTER
/**************************************************************************/ /**
\brief Initiates a DMA transmission from local Ram to FPGA TxRam.
       
\ingroup func_dma
\b Description: \n
   This function starts the DMA transfer for the selected Rx-DMA channels.
   Polling of the DMA Ready flags can be done with the CSMD_IsBusy_DMA_Transfer()
   function (usSelDirection = 1).

\todo
   Request the current transmit buffer after completion of DMA transfers

<B>Call Environment:</B> \n
   If the PCI bus master feature is activated, this function has to be called after
   setting the data of the produced connections with CSMD_SetConnection() from CP3 on.
   Due to its real-time-critical attributes, this CSMD_DMA_Write_Tx_Ram()
   should be called cyclically from an interrupt. The call-up time for the
   interrupt trigger should be calculated in the timing calculation process.
   
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       none

\author       WK
\date         01.03.2011

***************************************************************************** */
CSMD_VOID CSMD_DMA_Write_Tx_Ram( CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_USHORT usI;

  if (   (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
      && (prCSMD_Instance->sCSMD_Phase >= CSMD_SERC_PHASE_2))
  {
    /* Clear the DMA Rx Ready Flags. */
    for (usI = 0; usI < (CSMD_USHORT)CSMD_DMA_NBR_USED_CHANNELS; usI++)
    {
      if (prCSMD_Instance->rPriv.ausRxDMA_Start[prCSMD_Instance->rPriv.usTxBuffer] & (1UL<<usI))
      {
        prCSMD_Instance->prTelBuffer->rLocal.rDMA_RxRdy.aulFlag[usI] = 0;
      }
    }
    CSMD_HAL_StartRxDMA( &prCSMD_Instance->rCSMD_HAL,
                         prCSMD_Instance->rPriv.ausRxDMA_Start[prCSMD_Instance->rPriv.usTxBuffer] );
  }

} /* end: CSMD_DMA_Write_Tx_Ram() */



/**************************************************************************/ /**
\brief Initiates a DMA transmission from RxRam (ATs) to local ram.

\ingroup func_dma
\b Description: \n
   This function requests the current RxRam system buffers, determines the
   RxRam system buffers to be used for port 1 and 2 and starts the DMA
   transfer for the selected Tx-DMA channels. Polling of the DMA Ready
   flags takes place in CSMD_CyclicHandling().

<B>Call Environment:</B> \n
   If the PCI bus master feature is activated, this function has to be called
   before reading the consumed connections with CSMD_GetConnection() from CP0 on.
   Also before reading the consumed connections (in CP4) the completion of the
   DMA transfer has to be checked with CSMD_IsBusy_DMA_Transfer(#CSMD_TX_DMA) \n
   Due to its real-time-critical attributes, CSMD_DAM_Read_Rx_Ram() should be
   called cyclically from an interrupt. The call-up time for the interrupt
   trigger should be calculated in the timing calculation process.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       none

\author       CK
\date         10.05.2007

***************************************************************************** */
CSMD_VOID CSMD_DMA_Read_Rx_Ram( CSMD_INSTANCE *prCSMD_Instance )
{

  CSMD_USHORT usI;
  CSMD_USHORT usTxChannels;


  if (prCSMD_Instance->rPriv.boDMA_IsActive == TRUE)
  {
    /* Request new buffer for this cycle */
    CSMD_HAL_Request_New_Rx_BufferSysA( &prCSMD_Instance->rCSMD_HAL );

    /* Get active read buffer for both ports */
    CSMD_HAL_Usable_Rx_BufferSysA( &prCSMD_Instance->rCSMD_HAL,
                                   &prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1],
                                   &prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2] );

    usTxChannels = (CSMD_USHORT) (  prCSMD_Instance->rPriv.ausTxDMA_Start_P1[prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1]]
                                  | prCSMD_Instance->rPriv.ausTxDMA_Start_P2[prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2]]);

    /* Clear the DMA Tx Ready Flags. */
    for (usI = 0; usI < (CSMD_USHORT)CSMD_DMA_NBR_USED_CHANNELS; usI++)
    {
      if (usTxChannels & (1UL<<usI))
      {
        prCSMD_Instance->prTelBuffer->rLocal.rDMA_TxRdy.aulFlag[usI] = 0;
      }
    }
    CSMD_HAL_StartTxDMA( &prCSMD_Instance->rCSMD_HAL,
                         usTxChannels );
  }
} /* end: CSMD_DMA_Read_Rx_Ram() */



/**************************************************************************/ /**
\brief Retrieves the active DMA interrupts from the FPGA and returns them.

\ingroup func_dma
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   Advantageously, this function is called within an interrupt service routine
   if the FPGA is the cycle master. Thus, the reasons for the interrupt can be
   determined. If the FPGA is not the cycle master, the function can also be
   called from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usSelDirection
              defines data transfer direction:\n
              0 -> TX DMA\n
                   transfer from FPGA TxRam / RxRam buffer to host ram\n
              1 -> RX DMA\n
                   transfer from host ram to FPGA TxRam / RxRam buffer

\return       TRUE  -> DMA transfer in progress \n
              FALSE -> DMA transfer complete

\author       WK
\date         19.05.2009
*************************************************************************** */
CSMD_BOOL CSMD_IsBusy_DMA_Transfer( CSMD_INSTANCE *prCSMD_Instance,
                                    CSMD_USHORT    usSelDirection )
{
  if (CSMD_TX_DMA == usSelDirection)
  {
    return ( CSMD_IsBusy_Tx_DMA(
                 prCSMD_Instance,
                 (CSMD_USHORT) (  prCSMD_Instance->rPriv.ausTxDMA_Start_P1[prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1]]
                                | prCSMD_Instance->rPriv.ausTxDMA_Start_P2[prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2]]) )
           );
  }
  else
  {
    return ( CSMD_IsBusy_Rx_DMA(
                 prCSMD_Instance,
                 prCSMD_Instance->rPriv.ausRxDMA_Start[prCSMD_Instance->rPriv.usTxBuffer] )
           );
  }

} /* end: CSMD_IsBusy_DMA_Transfer() */

#endif  /* #ifdef CSMD_PCI_MASTER */

/*! \endcond */ /* PUBLIC */

/*lint -restore */


/*---- Definition private Functions: -----------------------------------------*/

/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
02 Dec 2013 WK
  - Defdb00165150
    CSMD_ReadAT()
    Clear all CC connection data in the TxRam once during transition to CP4.
09 Dec 2013 WK
  - Defdb00165521
    CSMD_Write_HP_Field()
    Fixed compiler problem for not defined CSMD_HOTLPUG.
12 Jan 2015 WK
  - CSMD_SetConnectionData()
    Fixed merging of RT bits.
22 Jan 2015 WK
  - Convert simple HTML tables into "markdown" extra syntax to make it
    more readable in the source code.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
01 Jul 2015 WK
  - CSMD_SetConnectionData()
    Fixed merging of RT bits.
19 Oct 2015 WK
  - Defdb00182406
    CSMD_CyclicHandling()
    Enable TxRam DMA operation for SVC in CP2, if no IP-Core SVC used.
02 Nov 2015 WK
  - Defdb00182801
    CSMD_CyclicHandling()
      Removed clear of RxDMA-Ready flags / start of RxDMA
    CSMD_DMA_Write_Tx_Ram()
      Added clear of RxDMA-Ready flags / start of RxDMA
      Removed query for RxDMA-Ready flags
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
04 Apr 2016 AlM
  - Defdb00184988
    CSMD_CyclicHandling()
    Enhanced AT validation.
27 Apr 2016 WK
  - Defdb00186271
    CSMD_SetConnectionState()
    Target state PREPARE implemented.
19 May 2016 AlM
  - Defdb00187136
    CSMD_ClearConnectionError()
      Allow the fault reset of a connection in each Sercos cycle.
    CSMD_SetConnectionData()
      Adjust setting of state CSMD_PROD_STATE_PRODUCING.
  
------------------------------------------------------------------------------
*/
