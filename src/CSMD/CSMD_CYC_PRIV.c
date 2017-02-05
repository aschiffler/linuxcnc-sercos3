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
\file   CSMD_CYC_PRIV.c
\author AlM
\date   11.12.2013
\brief  This file contains the  private functions for the cyclic interoperation.
*/

/*---- Includes: -------------------------------------------------------------*/


#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"
#include "CSMD_HAL_DMA.h"

#include "CSMD_HOTPLUG.h"
#include "CSMD_TOPOLOGY.h"
#include "CSMD_CP_AUX.h"
#include "CSMD_CYCLIC.h"


/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief CSMD_CyclicDeviceControl() copies the device control of all active slaves.

\ingroup module_cyclic
\b Description: \n
   This function copies the device control of all slaves marked as active
   from local RAM into Tx RAM.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function which is called in every Sercos cycle.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       none

\author       AlM
\date         19.11.2013

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_VOID CSMD_CyclicDeviceControl( CSMD_INSTANCE  *prCSMD_Instance )
{
  CSMD_INT      nLoop;
  CSMD_USHORT **pusTxRam  = &prCSMD_Instance->rPriv.pausC_Dev[prCSMD_Instance->rPriv.usTxBuffer][0];
  CSMD_USHORT  *pusLocRam = &prCSMD_Instance->rPriv.ausDevControl[0];

  /* Copy C-DEV of all slaves into TxRam */
  for (nLoop = 0; nLoop < prCSMD_Instance->rSlaveList.usNumProjSlaves; nLoop++)
  {
    /* copy only if slave is currently active */
    if (prCSMD_Instance->rSlaveList.aeSlaveActive[nLoop] != CSMD_SLAVE_INACTIVE)
    {
      CSMD_HAL_WriteShort( pusTxRam[nLoop], pusLocRam[nLoop] );
    }
  }
}  /* end: CSMD_CyclicDeviceControl() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief CSMD_CyclicDeviceStatus() handles the device status of all slaves.

\ingroup module_cyclic
\b Description: \n
   This function checks if the device status of all slaves marked as active
   has been received with slave valid bit set. If slave valid is not set on
   the slave's preferred master port, the device status is read from the other
   master port, as long as the respective AT has been received there. In this case,
   if slave valid is set on the alternative port, the preferred master port for
   this slave is adjusted.
   Depending on whether slave valid bit is set on either master port, the
   slave specific counter for consecutive misses of slave valid is either
   incremented or set to zero.
   If any AT has not been received on either master port, this function
   uses the telegram assignment list of device status (ausLast_S_Dev_Idx)
   to increment all miss counters of slaves assigned to the respective telegram.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function which is called in every Sercos cycle.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       none

\author       AlM
\date         19.11.2013

***************************************************************************** */
CSMD_VOID CSMD_CyclicDeviceStatus( CSMD_INSTANCE  *prCSMD_Instance )
{
  CSMD_INT     nSlaveIdx = 0;
  CSMD_INT     nTelNbr;
  CSMD_ULONG   ulTelMask;                      /* Bit mask for current AT */
  CSMD_USHORT  usDeviceStatus;
  CSMD_USHORT  usPrefPort;                     /* Preferred master port by slave */
  CSMD_USHORT  ausRxBuffer[CSMD_NBR_PORTS];    /* Current Rx buffer Port 1/2 */
  CSMD_ULONG   aulATBufValid[CSMD_NBR_PORTS];  /* ATx valid bits from Port 1/2 */

  ausRxBuffer[CSMD_PORT_1]   = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1];
  ausRxBuffer[CSMD_PORT_2]   = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2];
  aulATBufValid[CSMD_PORT_1] = prCSMD_Instance->rPriv.rRedundancy.aulATBufValid[CSMD_PORT_1];
  aulATBufValid[CSMD_PORT_2] = prCSMD_Instance->rPriv.rRedundancy.aulATBufValid[CSMD_PORT_2];

  for (nTelNbr = 0; nTelNbr < (CSMD_SHORT)CSMD_MAX_TEL; nTelNbr++)
  {
    /* check if telegram is configured */
    if (prCSMD_Instance->rPriv.aboAT_used[nTelNbr])
    {
      ulTelMask = 1UL << nTelNbr;
      /* check if configured AT has been received on either port */
      if ( !((aulATBufValid[CSMD_PORT_1] | aulATBufValid[CSMD_PORT_2]) & ulTelMask) )
      {
        for (; nSlaveIdx <= prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[nTelNbr]; nSlaveIdx++)
        {
          if (prCSMD_Instance->rSlaveList.aeSlaveActive[nSlaveIdx] == CSMD_SLAVE_ACTIVE)
          {
            CSMD_SlaveValidError (prCSMD_Instance, (CSMD_USHORT)nSlaveIdx);
          }
        }
      }
      else /* AT has been received on either master port */
      {
        for (; nSlaveIdx <= prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[nTelNbr]; nSlaveIdx++)
        {
          if (prCSMD_Instance->rSlaveList.aeSlaveActive[nSlaveIdx] == CSMD_SLAVE_ACTIVE)
          {
            usPrefPort = prCSMD_Instance->rPriv.ausPrefPortBySlave[nSlaveIdx];

            /* check if AT has been received on preferred port */
            if (aulATBufValid[usPrefPort] & ulTelMask)
            {
              /* read S-DEV from preferred port */
              usDeviceStatus = CSMD_END_CONV_S(
                  *prCSMD_Instance->rPriv.apusS_DEV[nSlaveIdx][usPrefPort][ausRxBuffer[usPrefPort]]);

              /* check if slave valid is set on preferred port */
              if (usDeviceStatus & CSMD_S_DEV_SLAVE_VALID)
              {
                prCSMD_Instance->arDevStatus[nSlaveIdx].usS_Dev = usDeviceStatus;
                prCSMD_Instance->arDevStatus[nSlaveIdx].usMiss = 0;
                continue; /* continue with next slave if slave valid is set on preferred port */
              }
            } /* if (aulATBufValid[usPrefPort] & ulTelMask) */

            /* If AT has not been received or slave valid is not set on preferred port,
               read S-DEV from other port if AT has been received there */
            usPrefPort = (CSMD_USHORT)(CSMD_PORT_2 - usPrefPort);
            if (aulATBufValid[usPrefPort] & ulTelMask)
            {
              /* read S-DEV from alternative port */
              usDeviceStatus = CSMD_END_CONV_S(
                  *prCSMD_Instance->rPriv.apusS_DEV[nSlaveIdx][usPrefPort][ausRxBuffer[usPrefPort]]);

              /* check if slave valid is set on alternative port */
              if (usDeviceStatus & CSMD_S_DEV_SLAVE_VALID)
              {
                /* set alternative port as preferred port */
                prCSMD_Instance->rPriv.ausPrefPortBySlave[nSlaveIdx] = usPrefPort;

                prCSMD_Instance->arDevStatus[nSlaveIdx].usS_Dev = usDeviceStatus;
                prCSMD_Instance->arDevStatus[nSlaveIdx].usMiss = 0;
                continue; /* continue with next slave if slave valid is set on alternative port */
              }
            } /* if (aulATBufValid[usPrefPort] & ulTelMask) */

            /* slave valid is not set on either port, perform error handling */
            CSMD_SlaveValidError (prCSMD_Instance, (CSMD_USHORT)nSlaveIdx);

          } /* if (prCSMD_Instance->rSlaveList.aeSlaveActive[nSlaveIdx] == CSMD_SLAVE_ACTIVE) */
        } /* for (nSlaveIdx; nSlaveIdx <= prCSMD_Instance->rPriv.ausLast_S_Dev_Idx[nTelNbr]; nSlaveIdx++) */
      } /* telegram has been received on either master port */
    } /* if (prCSMD_Instance->rPriv.aboAT_used[nTelNbr]) */
  } /* for (nTelNbr = 0; nTelNbr < CSMD_MAX_TEL; nTelNbr++) */
}  /* end: CSMD_CyclicDeviceStatus() */



/**************************************************************************/ /**
\brief CSMD_CyclicConnection() updates internal C_CON values for all connections.

\ingroup module_cyclic
\b Description: \n
   This function reads the current value of TSref counter. Comparing the value
   of the connection production list related to the current TSref counter with
   the production bit list of every connection, it is determined whether a
   connection has to be produced in the current Sercos cycle. In this case,
   the CoSeMa-internal values for connection control of connections with state
   'ready', 'producing' or 'waiting' are updated toggling the new data bit and
   incrementing the C-CON counter.
   The connection state of all master-produced connections with state 'producing'
   is set to 'waiting' for internal purposes.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function which is called in every Sercos cycle.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       none

\author       AlM
\date         20.11.2013

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_VOID CSMD_CyclicConnection( CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_USHORT  usTSref;
  CSMD_INT     nLoop;
  CSMD_USHORT  usConnIdx;
  CSMD_USHORT  usConfigIdx;

  usTSref = CSMD_HAL_GetTSrefCounter( &prCSMD_Instance->rCSMD_HAL );

  for (nLoop = 0; nLoop < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; nLoop++)
  {
    /* check if connection is produced by the master or by a slave */
    usConnIdx   = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nLoop].usConnIdx;
    usConfigIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nLoop].usConfigIdx;

    if ((  prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1
         & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
    {
      /* connection is produced by the master */
      CSMD_CONN_MASTERPROD *prMasterProd = &prCSMD_Instance->rPriv.parConnMasterProd[usConnIdx];

      /* check if connection has to be produced in current Sercos cycle */
      if (prMasterProd->usProduced & prCSMD_Instance->rPriv.ausTSrefList[usTSref])
      {
        if (   prMasterProd->eState == CSMD_PROD_STATE_PRODUCING
            || prMasterProd->eState == CSMD_PROD_STATE_WAITING
            || prMasterProd->eState == CSMD_PROD_STATE_READY )
        {
          /* toggle new data bit and increment C-CON counter */
          prMasterProd->usC_Con = (CSMD_USHORT)((prMasterProd->usC_Con + (1UL << CSMD_C_CON_COUNTER_SHIFT))
            ^ CSMD_C_CON_NEW_DATA);
        }
      }

      if (prMasterProd->eState == CSMD_PROD_STATE_PRODUCING)
      {
        prMasterProd->eState = CSMD_PROD_STATE_WAITING;
        /* transition ready/waiting -> producing is processed in CSMD_SetConnectionData() */
      }
    }
    else
    {
      /* connection is produced by a slave */
      CSMD_CONN_SLAVEPROD *prSlaveProd = &prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx];

      /* check if connection has to be produced in current Sercos cycle */
      if (prSlaveProd->usProduced & prCSMD_Instance->rPriv.ausTSrefList[usTSref])
      {
        if (   prSlaveProd->eState == CSMD_CONS_STATE_CONSUMING
            || prSlaveProd->eState == CSMD_CONS_STATE_WARNING )
        {
          /* The following distinction of cases is not necessarily needed, all expectation values
           * for C-CON could increment the counter value independent from check mode, but this might
           * be confusing considering connections with CSMD_CHECK_MODE_NEW_DATA. */
          if (prSlaveProd->eCheckMode == CSMD_CHECK_MODE_COUNTER)
          {
            /* calculate expected value for C-CON counter */
            prSlaveProd->usExpected_C_Con =
              (CSMD_USHORT)((prSlaveProd->usExpected_C_Con + (1UL << CSMD_C_CON_COUNTER_SHIFT)) ^ CSMD_C_CON_NEW_DATA);
          }
          else
          {
            /* toggle new data bit in expected C-CON */
            prSlaveProd->usExpected_C_Con ^= CSMD_C_CON_NEW_DATA;
          }
        }
      }
    }
  } /* for (nLoop = 0; nLoop < prCSMD_Instance->rConfiguration.prMasterConfig->usNbrOfConnections; nLoop++) */
}  /* end: CSMD_CyclicConnection() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief CSMD_EvaluateConnections() handles the consumer state of connections
       consumed by the master.

\ingroup module_cyclic
\b Description: \n
   This function evaluates the content of all slave-produced connections which
   have to be produced in the current Sercos cycle. If slave valid of a
   connection's producer has been received on either master port, the
   connection control of the respective connection is read from this port
   preferably and is used for processing of the consumer state machine for
   this connection.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function which is called in every Sercos cycle.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       none

\author       AlM
\date         21.11.2013

***************************************************************************** */
CSMD_VOID CSMD_EvaluateConnections( CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_USHORT  usTSref;
  CSMD_USHORT  usConnIdx;
  CSMD_USHORT  usConfigIdx;
  CSMD_USHORT  usC_Con;
  CSMD_USHORT  usPrefPort;
  CSMD_USHORT  usConnSetup;
  CSMD_INT     nLoop;
  CSMD_CONN_SLAVEPROD *prSlaveProd;
  CSMD_USHORT  usTelNbr;
  CSMD_ULONG   ulTelMask;
  CSMD_ULONG   aulATBufValid[CSMD_NBR_PORTS];  /* ATx valid bits from Port 1/2 */

  /* telegram validation */
  aulATBufValid[CSMD_PORT_1] = prCSMD_Instance->rPriv.rRedundancy.aulATBufValid[CSMD_PORT_1];
  aulATBufValid[CSMD_PORT_2] = prCSMD_Instance->rPriv.rRedundancy.aulATBufValid[CSMD_PORT_2];

  usTSref = CSMD_HAL_GetTSrefCounter( &prCSMD_Instance->rCSMD_HAL );

  for (nLoop = 0; nLoop < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; nLoop++)
  {
    usConfigIdx = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nLoop].usConfigIdx;
    usConnSetup = prCSMD_Instance->rConfiguration.parConfiguration[usConfigIdx].usS_0_1050_SE1;

    /* only evaluate connections produced by slaves */
    if ((usConnSetup & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
    {
      usConnIdx   = prCSMD_Instance->rConfiguration.rMasterCfg.parConnIdxList[nLoop].usConnIdx;
      prSlaveProd = &prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx];

      /* check if connection has to be produced in current Sercos cycle */
      if (prSlaveProd->usProduced & prCSMD_Instance->rPriv.ausTSrefList[usTSref])
      {
        /* check if connection's producer has set slave valid in current Sercos cycle */
        if (prCSMD_Instance->arDevStatus[prSlaveProd->usProdIdx].usMiss == 0)
        {
          /* get preferred master port for this producer */
          usPrefPort = prCSMD_Instance->rPriv.ausPrefPortBySlave[prSlaveProd->usProdIdx];

          /* check if AT containing the connection data has been received on the preferred port */
          usTelNbr = (CSMD_USHORT)(  (prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE3 & CSMD_S_0_1050_SE3_TEL_NBR_MASK)
                                   >> CSMD_S_0_1050_SE3_TEL_NBR_SHIFT );
          ulTelMask = 1UL << usTelNbr;

          if (aulATBufValid[usPrefPort] & ulTelMask)
          {
            /* read C-CON from master port determined for the connection's producer
             * by CSMD_CyclicDeviceStatus() */
            usC_Con = CSMD_HAL_ReadShort( prSlaveProd->apusConnRxRam[usPrefPort]
                        [prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[usPrefPort]] );
          }
          /* if not, check if that AT has been received on the other port,
             given slave valid is set on either port */
          else if (aulATBufValid[CSMD_PORT_2 - usPrefPort] & ulTelMask)
          {
            usC_Con = CSMD_HAL_ReadShort( prSlaveProd->apusConnRxRam[CSMD_PORT_2 - usPrefPort]
                        [prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2 - usPrefPort]] );

            /* switch preferred master port for any data the master consumes from this slave */
            prCSMD_Instance->rPriv.ausPrefPortBySlave[prSlaveProd->usProdIdx] = (CSMD_USHORT)(CSMD_PORT_2 - usPrefPort);
          }
          else
          {
            usC_Con = 0; /* initialize C-Con value with zero */
          }
        }
        else
        {
          usC_Con = 0; /* initialize C-Con value with zero */
        }

#ifdef CSMD_DEBUG_CONN_STM  /* only for debugging !!! */
        if (usConnIdx == usConnSelect)
          rConDebug[usZaehler].eEntry_State = prSlaveProd->eState;
#endif
        switch (prSlaveProd->eState)
        {
          /* --------------------------------------------------- */
          /* State PREPARE                                       */
          /* --------------------------------------------------- */
          case CSMD_CONS_STATE_PREPARE:
            /* --------------------------------------------------- */
            /* Transition PREPARE => WAITING                       */
            /* --------------------------------------------------- */
            /* check if producer ready is set */
            if ( !(usC_Con & CSMD_C_CON_PRODUCER_READY) )
            {
              break;
            }
            else /* producer ready is set */
            {
              prSlaveProd->eState = CSMD_CONS_STATE_WAITING;
              prSlaveProd->usExpected_C_Con = CSMD_C_CON_PRODUCER_READY;
              prSlaveProd->usAbsoluteErr = 0;
            }
            /*lint -fallthrough */

          /* --------------------------------------------------- */
          /* State WAITING                                       */
          /* --------------------------------------------------- */
          case CSMD_CONS_STATE_WAITING:
            /* --------------------------------------------------- */
            /* Transition WAITING => PREPARE                       */
            /* --------------------------------------------------- */
            /* check if producer ready is not set */
            if ( !(usC_Con & CSMD_C_CON_PRODUCER_READY) )
            {
              prSlaveProd->eState = CSMD_CONS_STATE_PREPARE;
            }
            else /* producer ready is set */
            {
              /* --------------------------------------------------- */
              /* Transition WAITING => STOPPED                       */
              /* --------------------------------------------------- */
              /* check if flow control is set */
              if (usC_Con & CSMD_C_CON_FLOW_CONTROL)
              {
                prSlaveProd->eState = CSMD_CONS_STATE_STOPPED;
              }
              /* --------------------------------------------------- */
              /* Transition WAITING => CONSUMING                     */
              /* --------------------------------------------------- */
              else
              {
                /* check if either new data is set or C_CON counter > 0 */
                if ( usC_Con & (CSMD_C_CON_NEW_DATA | CSMD_C_CON_COUNTER_MASK) )
                {
                  prSlaveProd->eState = CSMD_CONS_STATE_CONSUMING;
                  /* adapt expectation value for C-CON to produced value */
                  prSlaveProd->usExpected_C_Con = usC_Con;

                  /* initialize mode for connection monitoring */
                  CSMD_Set_C_Con_Check_Mode( usC_Con, prSlaveProd );
                }
              }
            }
            break;

          /* --------------------------------------------------- */
          /* State CONSUMING                                     */
          /* --------------------------------------------------- */
          case CSMD_CONS_STATE_CONSUMING:

            switch (usConnSetup & CSMD_S_0_1050_SE1_MONITOR)  /* type of connection */
            {
              /* --------------------------------------------------- */
              /* CONSUMING:  clock synchronous                       */
              /* --------------------------------------------------- */
              case CSMD_S_0_1050_SE1_SYNC:     /* Connection type 00 */
                /* --------------------------------------------------- */
                /* Transition CONSUMING => WARNING/ERROR               */
                /* --------------------------------------------------- */
                /* check if producer ready is not set */
                if ( !(usC_Con & CSMD_C_CON_PRODUCER_READY) )
                {
                  /* C-CON.ProducerReady == 0, perform connection handling for invalid data */
                  CSMD_InvalidConnectionData( prCSMD_Instance, usConnIdx, prSlaveProd );
                }
                else /* producer ready is set */
                {
                  /* --------------------------------------------------- */
                  /* Transition CONSUMING => STOPPED                     */
                  /* --------------------------------------------------- */
                  /* check if flow control is set */
                  if (usC_Con & CSMD_C_CON_FLOW_CONTROL)
                  {
                    prSlaveProd->eState = CSMD_CONS_STATE_STOPPED;
                  }
                  else
                  {
                    /* ----------------------------------------------------------- */
                    /* Transition CONSUMING => WARNING (start-warning)             */
                    /* ----------------------------------------------------------- */
                    if (usC_Con == prSlaveProd->usLatest_C_Con)
                    {
                      /* perform connection handling for invalid data */
                      CSMD_InvalidConnectionData( prCSMD_Instance, usConnIdx, prSlaveProd );
                    }
                    else /* current C_CON is different from latest C_CON value */
                    {
                      if (prSlaveProd->eCheckMode == CSMD_CHECK_MODE_COUNTER)
                      {
                        /* if the producer supports C_CON counter, check if the counter matches
                         * the expected value */
                        if ( (usC_Con >> CSMD_C_CON_COUNTER_SHIFT) !=
                             (prSlaveProd->usExpected_C_Con >> CSMD_C_CON_COUNTER_SHIFT) )
                        {
                          /* perform connection handling for invalid data */
                          CSMD_InvalidConnectionData( prCSMD_Instance, usConnIdx, prSlaveProd );
                        }
                        else
                        {
                          /* --------------------------------------------------- */
                          /* Transition CONSUMING => CONSUMING                   */
                          /* --------------------------------------------------- */
                          prSlaveProd->usLatest_C_Con = usC_Con;
                          prSlaveProd->usConsecErr = 0;
                          /* --------------------------------------------------- */
                        }
                      }
                      else /* producer does not support C_CON counter */
                      {
                        /* if the producer does not support C_CON counter, check if new data bit
                         * has toggled in C_CON */
                        if ( (usC_Con & CSMD_C_CON_NEW_DATA) !=
                              (prSlaveProd->usExpected_C_Con & CSMD_C_CON_NEW_DATA) )
                        {
                          /* perform connection handling for invalid data */
                          CSMD_InvalidConnectionData( prCSMD_Instance, usConnIdx, prSlaveProd );
                        }
                        else
                        {
                          /* --------------------------------------------------- */
                          /* Transition CONSUMING => CONSUMING                   */
                          /* --------------------------------------------------- */
                          prSlaveProd->usLatest_C_Con = usC_Con;
                          prSlaveProd->usConsecErr = 0;
                          /* --------------------------------------------------- */
                        }
                      }
                    }
                  } /* else: (usC_Con & CSMD_C_CON_FLOW_CONTROL) */
                } /* else: if (usC_Con & CSMD_C_CON_PRODUCER_READY) */
                break;

              /* --------------------------------------------------- */
              /* CONSUMING:  non-cyclic type 2 (without watchdog)    */
              /* --------------------------------------------------- */
              case CSMD_S_0_1050_SE1_ASYNC:    /* Connection type 10 */
                /* --------------------------------------------------- */
                /* Transition CONSUMING => WARNING                     */
                /* --------------------------------------------------- */
                /* check if producer ready is not set */
                if ( !(usC_Con & CSMD_C_CON_PRODUCER_READY) )
                {
                  /* C-CON.ProducerReady == 0, perform connection handling for invalid data */
                  prSlaveProd->eState = CSMD_CONS_STATE_WARNING;
                  prSlaveProd->usAbsoluteErr++;
                }
                else /* producer ready is set */
                {
                  /* --------------------------------------------------- */
                  /* Transition CONSUMING => STOPPED                     */
                  /* --------------------------------------------------- */
                  /* check if flow control is set */
                  if (usC_Con & CSMD_C_CON_FLOW_CONTROL)
                  {
                    prSlaveProd->eState = CSMD_CONS_STATE_STOPPED;
                  }
                  else
                  {
                    /* ----------------------------------------------------------- */
                    /* Transition CONSUMING => WARNING (start-warning)             */
                    /* ----------------------------------------------------------- */
                    if (usC_Con == prSlaveProd->usLatest_C_Con)
                    {
                      /* perform connection handling for invalid data */
                      prSlaveProd->eState = CSMD_CONS_STATE_WARNING;
                      prSlaveProd->usAbsoluteErr++;
                    }
                    else
                    {
                      /* --------------------------------------------------- */
                      /* Transition CONSUMING => CONSUMING                   */
                      /* --------------------------------------------------- */
                      prSlaveProd->usLatest_C_Con = usC_Con;
                      /* --------------------------------------------------- */
                    }
                  } /* else: (usC_Con & CSMD_C_CON_FLOW_CONTROL) */
                } /* else: if (usC_Con & CSMD_C_CON_PRODUCER_READY) */
                break;

              /* --------------------------------------------------- */
              /* CONSUMING:  non-cyclic type 1 (with watchdog)       */
              /* --------------------------------------------------- */
              case CSMD_S_0_1050_SE1_ASYNC_WD: /* Connection type 01 */
                prSlaveProd->eState = CSMD_CONS_STATE_ERROR;
                break;

              /* --------------------------------------------------- */
              /* CONSUMING:  cyclic                                  */
              /* --------------------------------------------------- */
              default: /* CSMD_S_0_1050_SE1_CYCLIC  Connection type 03 */
                prSlaveProd->eState = CSMD_CONS_STATE_ERROR;
                break;

            } /* CSMD_CONS_STATE_CONSUMING: switch (usConnSetup & CSMD_S_0_1050_SE1_MONITOR) */
            break;

          /* --------------------------------------------------- */
          /* State WARNING                                       */
          /* --------------------------------------------------- */
          case CSMD_CONS_STATE_WARNING:

            switch (usConnSetup & CSMD_S_0_1050_SE1_MONITOR)  /* type of connection */
            {
              /* --------------------------------------------------- */
              /* WARNING:  clock synchronous                         */
              /* --------------------------------------------------- */
              case CSMD_S_0_1050_SE1_SYNC:     /* Connection type 00 */
                /* --------------------------------------------------- */
                /* Transition WARNING => WARNING/ERROR                 */
                /* --------------------------------------------------- */
                /* check if producer ready is not set */
                if ( !(usC_Con & CSMD_C_CON_PRODUCER_READY) )
                {
                  /* C-CON.ProducerReady == 0, perform connection handling for invalid data */
                  CSMD_InvalidConnectionData( prCSMD_Instance, usConnIdx, prSlaveProd );
                }
                else /* producer ready is set */
                {
                  /* --------------------------------------------------- */
                  /* Transition WARNING => STOPPED                       */
                  /* --------------------------------------------------- */
                  /* check if flow control is set */
                  if (usC_Con & CSMD_C_CON_FLOW_CONTROL)
                  {
                    prSlaveProd->eState = CSMD_CONS_STATE_STOPPED;
                  }
                  else
                  {
                    /* ----------------------------------------------------------- */
                    /* Transition WARNING => WARNING (start-warning)               */
                    /* ----------------------------------------------------------- */
                    /* check if current C_CON is different from latest C_CON value (i.e. Counter/New Data) */
                    if (usC_Con == prSlaveProd->usLatest_C_Con)
                    {
                      /* perform connection handling for invalid data */
                      CSMD_InvalidConnectionData( prCSMD_Instance, usConnIdx, prSlaveProd );
                    }
                    else /* current C_CON is different from latest C_CON value */
                    {
                      if (prSlaveProd->eCheckMode == CSMD_CHECK_MODE_COUNTER)
                      {
                        /* if the producer supports C_CON counter, check if the counter matches
                         * the expected value */
                        if ( (usC_Con >> CSMD_C_CON_COUNTER_SHIFT) !=
                             (prSlaveProd->usExpected_C_Con >> CSMD_C_CON_COUNTER_SHIFT) )
                        {
                          /* perform connection handling for invalid data */
                          CSMD_InvalidConnectionData( prCSMD_Instance, usConnIdx, prSlaveProd );
                        }
                        else
                        {
                          /* --------------------------------------------------- */
                          /* Transition WARNING => CONSUMING                     */
                          /* --------------------------------------------------- */
                          prSlaveProd->eState = CSMD_CONS_STATE_CONSUMING;
                          prSlaveProd->usLatest_C_Con = usC_Con;
                          prSlaveProd->usConsecErr = 0;
                          /* --------------------------------------------------- */
                        }
                      }
                      else /* producer does not support C_CON counter */
                      {
                        /* if the producer does not support C_CON counter, check if new data bit
                         * has toggled in C_CON */
                        if ( (usC_Con & CSMD_C_CON_NEW_DATA) !=
                              (prSlaveProd->usExpected_C_Con & CSMD_C_CON_NEW_DATA) )
                        {
                          /* perform connection handling for invalid data */
                          CSMD_InvalidConnectionData( prCSMD_Instance, usConnIdx, prSlaveProd );
                        }
                        else
                        {
                          /* --------------------------------------------------- */
                          /* Transition WARNING => CONSUMING                     */
                          /* --------------------------------------------------- */
                          prSlaveProd->eState = CSMD_CONS_STATE_CONSUMING;
                          prSlaveProd->usLatest_C_Con = usC_Con;
                          prSlaveProd->usConsecErr = 0;
                          /* --------------------------------------------------- */
                        }
                      } /* else: (prSlaveProd->eCheckMode == CSMD_CHECK_MODE_COUNTER) */
                    } /* else: (usC_Con == prSlaveProd->usLatest_C_Con) */
                  } /* else: (usC_Con & CSMD_C_CON_FLOW_CONTROL) */
                } /* if (usC_Con & CSMD_C_CON_PRODUCER_READY) */
                break;

              /* --------------------------------------------------- */
              /* WARNING:  non-cyclic type 2 (without watchdog)      */
              /* --------------------------------------------------- */
              case CSMD_S_0_1050_SE1_ASYNC:    /* Connection type 10 */
                /* --------------------------------------------------- */
                /* Transition WARNING => WARNING                       */
                /* --------------------------------------------------- */
                /* check if producer ready is not set */
                if ( !(usC_Con & CSMD_C_CON_PRODUCER_READY) )
                {
                  /* C-CON.ProducerReady == 0 */
                  prSlaveProd->usAbsoluteErr++;
                }
                else /* producer ready is set */
                {
                  /* --------------------------------------------------- */
                  /* Transition WARNING => STOPPED                       */
                  /* --------------------------------------------------- */
                  /* check if flow control is set */
                  if (usC_Con & CSMD_C_CON_FLOW_CONTROL)
                  {
                    prSlaveProd->eState = CSMD_CONS_STATE_STOPPED;
                  }
                  else
                  {
                    /* The consumer has no activities in this state. */

                    /* check if current C_CON is different from latest C_CON value (i.e. Counter/New Data) */
                    if (usC_Con == prSlaveProd->usLatest_C_Con)
                    {
                      /* perform connection handling for invalid data */
                      prSlaveProd->usAbsoluteErr++;
                    }
                    else
                    {
                      /* --------------------------------------------------- */
                      /* Transition WARNING => CONSUMING                     */
                      /* --------------------------------------------------- */
                      prSlaveProd->eState = CSMD_CONS_STATE_CONSUMING;
                      prSlaveProd->usLatest_C_Con = usC_Con;
                      /* --------------------------------------------------- */
                    }
                  } /* else: (usC_Con & CSMD_C_CON_FLOW_CONTROL) */
                } /* if (usC_Con & CSMD_C_CON_PRODUCER_READY) */
                break;

              /* --------------------------------------------------- */
              /* WARNING:  non-cyclic type 1 (with watchdog)         */
              /* --------------------------------------------------- */
              case CSMD_S_0_1050_SE1_ASYNC_WD: /* Connection type 01 */
                prSlaveProd->eState = CSMD_CONS_STATE_ERROR;
                break;

              /* --------------------------------------------------- */
              /* warning:  cyclic                                    */
              /* --------------------------------------------------- */
              default: /* CSMD_S_0_1050_SE1_CYCLIC  Connection type 03 */
                prSlaveProd->eState = CSMD_CONS_STATE_ERROR;
                break;

            } /* CSMD_CONS_STATE_WARNING: switch (usConnSetup & CSMD_S_0_1050_SE1_MONITOR) */
            break;

          /* --------------------------------------------------- */
          /* State STOPPED                                       */
          /* --------------------------------------------------- */
          case CSMD_CONS_STATE_STOPPED:
            /* ---------------------------------------------------------- */
            /* Transition STOPPED => PREPARE                              */
            /* ---------------------------------------------------------- */
            /* check if producer ready is set */
            if (usC_Con & CSMD_C_CON_PRODUCER_READY)
            {
              /* check if flow control is not set */
              if ( !(usC_Con & CSMD_C_CON_FLOW_CONTROL) )
              {
                prSlaveProd->eState = CSMD_CONS_STATE_PREPARE;
              }
            }
            break;

          /* --------------------------------------------------- */
          /* State INIT                                          */
          /* --------------------------------------------------- */
          case CSMD_CONS_STATE_INIT:
            break;

          /* --------------------------------------------------- */
          /* State ERROR                                         */
          /* --------------------------------------------------- */
          case CSMD_CONS_STATE_ERROR:
            break;

        } /* end: switch (prSlaveProd->eState) */
      } /* if (prSlaveProd->usProduced & prCSMD_Instance->rPriv.ausTSrefList[usTSref]) */

#ifdef CSMD_DEBUG_CONN_STM  /* only for debugging !!! */
      if (usConnIdx == usConnSelect)
      {
        if (usZaehler >= CSMD_DBG_NBR_CYCLES)
        {
          //usZaehler = 0;
        }
        else
        {
          rConDebug[usZaehler].eState = prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].eState;
          rConDebug[usZaehler].usSoll = prCSMD_Instance->rPriv.parConnSlaveProd[usConnIdx].usExpected_C_Con;
          rConDebug[usZaehler].usIst  = usC_Con;
          usZaehler++;
        }
      }
#endif

    } /* if (   (usConnSetup & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK) == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER) */
  } /* for (nLoop = 0; nLoop < prCSMD_Instance->rConfiguration.rMasterCfg.usNbrOfConnections; nLoop++) */
}  /* end: CSMD_EvaluateConnections() */



/**************************************************************************/ /**
\brief CSMD_Set_C_Con_Check_Mode() sets the checking mode for connection synchronization.

\ingroup module_cyclic
\b Description: \n
   This function evaluates the connection control of the selected connection
   and thus determines if this connection supports the C-CON counter.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]       usC_Con CSMD_USHORT
                  C_Con which has been read from RxRam in current Sercos cycle

\param [in, out]  prSlaveProd
                  Pointer to CoSeMa internal structure of slaveproduced connections

\return       none

\author       AlM
\date         22.11.2013

***************************************************************************** */
CSMD_VOID CSMD_Set_C_Con_Check_Mode( CSMD_USHORT          usC_Con,
                                     CSMD_CONN_SLAVEPROD *prSlaveProd )
{
  /* producer supports C_CON counter */
  if (usC_Con & CSMD_C_CON_COUNTER_MASK)
  {
    prSlaveProd->eCheckMode = CSMD_CHECK_MODE_COUNTER;
  }
  else /* producer does not support C_CON counter */
  {
    prSlaveProd->eCheckMode = CSMD_CHECK_MODE_NEW_DATA;
  }

  prSlaveProd->usLatest_C_Con = usC_Con;
  prSlaveProd->usConsecErr = 0;

}  /* end: CSMD_Set_C_Con_Check_Mode() */



/**************************************************************************/ /**
\brief CSMD_InvalidConnectionData() performs the consumer state machine process for connection data miss.

\ingroup module_cyclic
\b Description: \n
   This function increments the internal counters for consecutive and absolute errors of a
   connection consumed by the master. After that, it is checked whether the tolerated
   consecutive connection errors have been exceeded. In this case the consumer state of
   this connection is set to 'error', else it is set to 'warning'.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]       usConnIdx
                  index of connection with invalid data

\param [in, out]  prSlaveProd
                  Pointer to CoSeMa internal structure of slaveproduced connections

\return       none

\author       AlM
\date         02.12.2013

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_VOID CSMD_InvalidConnectionData( CSMD_INSTANCE       *prCSMD_Instance,
                                      CSMD_USHORT          usConnIdx,
                                      CSMD_CONN_SLAVEPROD *prSlaveProd )
{
  prSlaveProd->usConsecErr++;
  prSlaveProd->usAbsoluteErr++;

  /* The following distinction of cases is not necessarily needed, all expectation values
   * for C-CON could increment the counter value independent from check mode, but this might
   * be confusing considering connections with CSMD_CHECK_MODE_NEW_DATA. */

  if (prSlaveProd->usConsecErr <= prCSMD_Instance->rConfiguration.parConnection[usConnIdx].usS_0_1050_SE11)
  {
    prSlaveProd->eState = CSMD_CONS_STATE_WARNING;
  }
  else
  {
    prSlaveProd->eState = CSMD_CONS_STATE_ERROR;
  }

}  /* end: CSMD_InvalidConnectionData() */
/*lint -restore const! */



/**************************************************************************/ /**

\brief This function read and clear the telegram status register of both.ports.

\ingroup module_cyclic
\b Description: \n
   - The telegram status register (TGSR) of both ports is read and stored into \n
     the private CoSeMa structure.

   - The deletable bits of the telegram status register (except MDTx/ATx valid) \n
     of both ports are deleted. These bits are
     - AT0 miss
     - MST double miss
     - MST miss
     - MST window error
     - MST valid
   - Build flags for valid MST (inside or outside of MST monitoring window) for
     topology check purposes
     - Valid primary telegram at port 1
     - Valid secondary telegram at port 1
     - Valid primary telegram at port 2
     - Valid secondary telegram at port 2

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       none

\author       WK
\date         12.09.2012

***************************************************************************** */
CSMD_VOID CSMD_ReadTelegramStatus( CSMD_INSTANCE *prCSMD_Instance )
{
  /* Read telegram status register port 1 */
  prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[CSMD_PORT_1] =
    CSMD_HAL_GetTelegramStatusP1( &prCSMD_Instance->rCSMD_HAL );

  /* Read telegram status register port 2 */
  prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[CSMD_PORT_2] =
    CSMD_HAL_GetTelegramStatusP2( &prCSMD_Instance->rCSMD_HAL );

  /* clear MDT relevant bits for port 1 */
  CSMD_HAL_ClearTelegramStatusP1( &prCSMD_Instance->rCSMD_HAL,
                                  (CSMD_USHORT)(  CSMD_HAL_TGSR_AT0_MISS
                                                | CSMD_HAL_TGSR_MST_DMISS
                                                | CSMD_HAL_TGSR_MST_MISS
                                                | CSMD_HAL_TGSR_MST_WIN_ERR
                                                | CSMD_HAL_TGSR_MST_VALID) );

  /* clear MDT relevant bits for port 2 */
  CSMD_HAL_ClearTelegramStatusP2( &prCSMD_Instance->rCSMD_HAL,
                                  (CSMD_USHORT)(  CSMD_HAL_TGSR_AT0_MISS
                                                | CSMD_HAL_TGSR_MST_DMISS
                                                | CSMD_HAL_TGSR_MST_MISS
                                                | CSMD_HAL_TGSR_MST_WIN_ERR
                                                | CSMD_HAL_TGSR_MST_VALID) );

  {
    CSMD_ULONG ulMST_OK;

    ulMST_OK =
      prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[CSMD_PORT_1] & CSMD_HAL_TGSR_VALID_MST_MASK;
    prCSMD_Instance->rPriv.rRedundancy.boSecTelP1 =
      ulMST_OK && (prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[CSMD_PORT_1] & CSMD_HAL_TGSR_SEC_TEL);
    prCSMD_Instance->rPriv.rRedundancy.boPriTelP1 =
      ulMST_OK && !(prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[CSMD_PORT_1] & CSMD_HAL_TGSR_SEC_TEL);

    ulMST_OK =
      prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[CSMD_PORT_2] & CSMD_HAL_TGSR_VALID_MST_MASK;
    prCSMD_Instance->rPriv.rRedundancy.boSecTelP2 =
      ulMST_OK &&  (prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[CSMD_PORT_2] & CSMD_HAL_TGSR_SEC_TEL);
    prCSMD_Instance->rPriv.rRedundancy.boPriTelP2 =
      ulMST_OK && !(prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[CSMD_PORT_2] & CSMD_HAL_TGSR_SEC_TEL);
  }

} /* end: CSMD_ReadTelegramStatus() */



/**************************************************************************/ /**
\brief Copies the Hot Plug field into Tx Ram.

\ingroup module_cyclic
\b Description: \n
   This function copies the hot plug field into Tx Ram depending on the current
   communication phase:
   - In CP3 it sends the UC transmission bandwidth (t6/ t7).
     Both values are transmitted 16 times each at start of CP3.
   - In CP4 it sends the control data for the Hot Plug mechanism.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       none

\author       WK
\date         24.01.2011

***************************************************************************** */
CSMD_VOID CSMD_CyclicHotPlug_MDT( CSMD_INSTANCE *prCSMD_Instance )
{
  if (prCSMD_Instance->rPriv.usTimerHP_CP3 <= (2 * CSMD_HP_CP3_REPEATRATE_T6_T7))
  {
#ifdef CSMD_HOTPLUG
    if (prCSMD_Instance->rPriv.rHotPlug.boHotPlugActive != TRUE)
#endif
    {
      if (prCSMD_Instance->rPriv.usTimerHP_CP3 == 0)
      {
        /* Send t6 in MDT0 HP field */
        prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rSelection.usWord = CSMD_HP_ADD_BRDCST_ADD;

        prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord =
          (CSMD_USHORT) ((prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord
                     & ~CSMD_HP_CNTRL_PAR_CODING_MASK) | CSMD_HP_CODE_T6);

        prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rUC_Channel.ulBegin_T6_S01017;

        prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0 =
          prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0;
      }
      else if (prCSMD_Instance->rPriv.usTimerHP_CP3 == CSMD_HP_CP3_REPEATRATE_T6_T7)
      {
        /* Send t7 in MDT0 HP field */
        prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord =
          (CSMD_USHORT) ((prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord
                     & ~CSMD_HP_CNTRL_PAR_CODING_MASK) | CSMD_HP_CODE_T7);

        prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rInfo.ulLong =
          prCSMD_Instance->rConfiguration.rUC_Channel.ulEnd_T7_S01017;

        prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0 =
          prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0;
      }
      else if (prCSMD_Instance->rPriv.usTimerHP_CP3 == (2 * CSMD_HP_CP3_REPEATRATE_T6_T7))
      {
        /* Reset MDT0 HP field */
        prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rSelection.usWord = CSMD_HP_ADD_DEFAULT_SADD;

        prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord =
          (CSMD_USHORT) (prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord
                    & ~CSMD_HP_CNTRL_PAR_CODING_MASK);

        prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rInfo.ulLong = 0;

        prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0 =
          prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0;
      }
    }
#ifdef CSMD_HOTPLUG
    else
    {
      /* Hot-plug activated. Terminate sending t6/t7 */
      prCSMD_Instance->rPriv.usTimerHP_CP3 = (2 * CSMD_HP_CP3_REPEATRATE_T6_T7);
    }
#endif
    prCSMD_Instance->rPriv.usTimerHP_CP3++;

    /* Copy MDT0 HP field port 1 from local Ram into TxRam */
    CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P1_Struct.pulTxRam,
                        *((CSMD_ULONG *)(CSMD_VOID *)&prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0) );

    CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P1_Struct.pulTxRam + 1,
                        prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rInfo.ulLong );

    /* Copy MDT0 HP field port 2 from local Ram into TxRam */
    CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P2_Struct.pulTxRam,
                        *((CSMD_ULONG *)(CSMD_VOID *)&prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0) );

    CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P2_Struct.pulTxRam + 1,
                        prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0.rInfo.ulLong );

  } /* End: if (prCSMD_Instance->rPriv.usTimerHP_CP3 > (2 * CSMD_HP_CP3_REPEATRATE_T6_T7)) */
#ifdef CSMD_HOTPLUG
  else
  {
    /* ------------------------------------------------------------- */
    /* MDT0 Hot-Plug field                                           */
    /* ------------------------------------------------------------- */
    if (CSMD_IsHotplugSupported (prCSMD_Instance))
    {
      /* Copy MDT0 HP field port 1 from local Ram into TxRam */
      CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P1_Struct.pulTxRam,
                          *((CSMD_ULONG *)(CSMD_VOID *)&prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0) );

      CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P1_Struct.pulTxRam + 1,
                          prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rInfo.ulLong );

      /* Copy MDT0 HP field port 2 from local Ram into TxRam */
      CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P2_Struct.pulTxRam,
                          *((CSMD_ULONG *)(CSMD_VOID *)&prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0) );

      CSMD_HAL_WriteLong( prCSMD_Instance->rPriv.rHP_P2_Struct.pulTxRam + 1,
                          prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0.rInfo.ulLong );
    }
  }
#endif

} /* end: CSMD_CyclicHotPlug_MDT() */



/**************************************************************************/ /**
\brief Read the Hot Plug fields from Rx Ram.

\ingroup module_cyclic
\b Description: \n
   This function reads the hot plug fields of both ports into Rx Ram.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       none

\author       WK
\date         03.04.2014

***************************************************************************** */
CSMD_VOID CSMD_CyclicHotPlug_AT( CSMD_INSTANCE *prCSMD_Instance )
{
  /* Read AT0 Hot-Plug field */
  if (CSMD_IsHotplugSupported (prCSMD_Instance))
  {
    /*! >todo Is this query correct? */
    if (prCSMD_Instance->rPriv.rHotPlug.usHP_Phase != CSMD_NON_HP_MODE)
    {
      CSMD_USHORT usBufP1 = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1];
      CSMD_USHORT usBufP2 = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2];

      /* Copy AT0 HP field port 1 into local Ram */
      *((CSMD_ULONG *)(CSMD_VOID *)&prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_AT0) =
        CSMD_HAL_ReadLong( (prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBufP1][0]) );

      prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_AT0.rInfo.ulLong =
        CSMD_HAL_ReadLong( (prCSMD_Instance->rPriv.apulSERC3_RxP1_AT[usBufP1][0] + 1) );

      /* Copy AT0 HP field port 2 into local Ram */
      *((CSMD_ULONG *)(CSMD_VOID *)&prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_AT0) =
        CSMD_HAL_ReadLong( (prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBufP2][0]) );

      prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_AT0.rInfo.ulLong =
        CSMD_HAL_ReadLong( (prCSMD_Instance->rPriv.apulSERC3_RxP2_AT[usBufP2][0] + 1) );
    }
  }

} /* end: CSMD_CyclicHotPlug_AT() */



/**************************************************************************/ /**
\brief Copies telegram data from FPGA RxRam to local Ram.

\ingroup module_cyclic
\b Description: \n
   This function copies a certain number of words from a given source address
   in FPGA RxRam to a given destination address in local Ram.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   pusTelDest
              Pointer to linear data Ram destination
\param [in]   pusTelSourceRxRam
              Pointer to data source Rx Ram
\param [in]   usNbrWords
              Number of Words to be copied

\return       none

\author       WK
\date         14.03.2005

***************************************************************************** */
CSMD_VOID CSMD_Read_Rx_Ram( CSMD_USHORT *pusTelDest,
                            CSMD_USHORT *pusTelSourceRxRam,
                            CSMD_USHORT  usNbrWords )
{

  CSMD_HAL_ReadBlock( pusTelDest, pusTelSourceRxRam, usNbrWords );

} /* end: CSMD_Read_Rx_Ram() */



/**************************************************************************/ /**
\brief Copies telegram data from local Ram to FPGA TxRam.

\ingroup module_cyclic
\b Description: \n
   This function copies a certain number of words from a given source address
   in local Ram to a given destination address in FPGA TxRam.


<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   pusTelDestTxRam
              Pointer to destination in linear Tx Ram
\param [in]   pusTelSource
              Pointer to data source
\param [in]   usNbrWords
              Number of words to be copied

\return       none

\author       WK
\date         14.03.2005

***************************************************************************** */
CSMD_VOID CSMD_Write_Tx_Ram( CSMD_USHORT *pusTelDestTxRam,
                             CSMD_USHORT *pusTelSource,
                             CSMD_USHORT  usNbrWords )
{

  CSMD_HAL_WriteBlock( pusTelDestTxRam, pusTelSource, usNbrWords );

} /* end: CSMD_Write_Tx_Ram() */



#ifdef CSMD_PCI_MASTER
/**************************************************************************/ /**
\brief Checks if all selected DMA channels returns a Tx DMA ready message.

\ingroup module_dma
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usChannels
              Mask with all selected Tx DMA channels

\return       TRUE  -> Tx DMA in progress \n
              FALSE -> Tx DMA finished

\author       WK
\date         21.06.2011

***************************************************************************** */
CSMD_BOOL CSMD_IsBusy_Tx_DMA( const CSMD_INSTANCE *prCSMD_Instance,
                              CSMD_USHORT          usChannels )
{
  CSMD_USHORT usI;

  for (usI = 0; usI < (CSMD_USHORT)CSMD_DMA_NBR_USED_CHANNELS; usI++)
  {
    if (usChannels & (1UL<<usI))
    {
      if (!prCSMD_Instance->prTelBuffer->rLocal.rDMA_TxRdy.aulFlag[usI] )
        return (TRUE);
    }
  }
  return (FALSE);

} /* end: CSMD_IsBusy_Tx_DMA() */



/**************************************************************************/ /**
\brief Checks if all selected DMA channels return a Rx DMA ready message.

\ingroup module_dma
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usChannels
              Mask with all selected Rx DMA channels

\return       TRUE  -> Rx DMA in progress \n
              FALSE -> Rx DMA finished

\author       WK
\date         21.06.2011

***************************************************************************** */
CSMD_BOOL CSMD_IsBusy_Rx_DMA( const CSMD_INSTANCE *prCSMD_Instance,
                              CSMD_USHORT          usChannels )
{
  CSMD_USHORT usI;

  for (usI = 0; usI < (CSMD_USHORT)CSMD_DMA_NBR_USED_CHANNELS; usI++)
  {
    if (usChannels & (1UL<<usI))
    {
      if (!prCSMD_Instance->prTelBuffer->rLocal.rDMA_RxRdy.aulFlag[usI] )
        return (TRUE);
    }
  }
  return (FALSE);

} /* end: CSMD_IsBusy_Rx_DMA() */
#endif  /* #ifdef CSMD_PCI_MASTER */


/**************************************************************************/ /**
\brief Initialize CC data in TxRam while switching to CP4.

\ingroup module_cyclic
\b Description: \n
   This function clears the CC connection data in the TxRam once,
   if CP = CP4 and CPS = 1.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       none

\author       WK
\date         28.11.2013

***************************************************************************** */
CSMD_VOID CSMD_Clear_Tx_CC_Data( CSMD_INSTANCE *prCSMD_Instance )
{

  CSMD_UCHAR *pucTxRam;

  /* Clear all CC connection data in the port relative buffer port 1 in TxRam */
  pucTxRam =   (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aucTx_Ram
             + prCSMD_Instance->rPriv.ulTxRamOffsetPRel_P1;

  CSMD_Telegram_Clear( prCSMD_Instance,
                       pucTxRam,
                       prCSMD_Instance->rPriv.usTxRamPRelBuffLen,
                       0
                     );

  /* Clear all CC connection data in the port relative buffer port 2 in TxRam */
  pucTxRam =   (CSMD_UCHAR *)prCSMD_Instance->rCSMD_HAL.prSERC_TX_Ram->aucTx_Ram
             + prCSMD_Instance->rPriv.ulTxRamOffsetPRel_P2;

  CSMD_Telegram_Clear( prCSMD_Instance,
                       pucTxRam,
                       prCSMD_Instance->rPriv.usTxRamPRelBuffLen,
                       0
                     );

} /* end: CSMD_Clear_Tx_CC_Data() */


/**************************************************************************/ /**
\brief CSMD_DeleteSlave() removes a slave from CoSeMa-internal lists.

\ingroup module_cyclic
\b Description: \n
   This function removes a slave from the CoSeMa-internal lists for
   topology information and service channel access. If the selected slave
   represented the only physical connection of any other slaves to a master port,
   all disconnected slaves are removed as well.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function. It is called if a slave has exceeded
   the tolerated value for consecutive misses of slave valid bit in its device status.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usSlaveIdx
              Slave index of the slave which is deleted from lists

\return       none

\author       AlM
\date         21.01.2014

***************************************************************************** */
CSMD_VOID CSMD_DeleteSlave( CSMD_INSTANCE *prCSMD_Instance,
                            CSMD_USHORT    usSlaveIdx )
{
  /* Sercos address of slave to be deleted */
  CSMD_USHORT  usSAdd = prCSMD_Instance->rSlaveList.ausProjSlaveAddList[usSlaveIdx + 2];
  CSMD_USHORT  usI;
  CSMD_USHORT  usK;
  CSMD_BOOL    boFoundOnPort1 = FALSE;   /* flag if slave address has been found in topology list for master port 1 */

  CSMD_ADDR_SCAN_INFO*  prListPort1 = &prCSMD_Instance->rPriv.rSlaveAvailable;
  CSMD_ADDR_SCAN_INFO*  prListPort2 = &prCSMD_Instance->rPriv.rSlaveAvailable2;

  prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] = CSMD_SLAVE_INACTIVE;

  /* scan topology list for master port 1; delete slave address and reduce
   * number of slaves on this master port if slave address is found there.
   * This process includes removal of any slaves behind the slave to be deleted. */
  for (usI = 1; usI < prListPort1->usAddressNmb; usI++)
  {
    if (usSAdd == prListPort1->ausAddresses[usI])
    {
      for (usK = (CSMD_USHORT)(prListPort1->usAddressNmb - 1U); usK >= usI; usK--)
      {
        prListPort1->ausAddresses[usK] = 0;
      }

      prListPort1->usAddressNmb = usI;
      prCSMD_Instance->usSercAddrLastSlaveP1 = prListPort1->ausAddresses[usI - 1U];
      boFoundOnPort1 = TRUE;
    }
  }

  /* if slave address has been found in topology list for master port 1,
   * scanning of topology list for master port 2 is skipped */
  if (!boFoundOnPort1)
  {
    /* scan topology list for master port 2; delete slave address and reduce
     * number of slaves on this master port if slave address is found there.
     * This process includes removal of any slaves behind the slave to be deleted. */
    for (usI = 1; usI < prListPort2->usAddressNmb; usI++)
    {
      if (usSAdd == prListPort2->ausAddresses[usI])
      {
        for (usK = (CSMD_USHORT)(prListPort2->usAddressNmb - 1U); usK >= usI; usK--)
        {
          prListPort2->ausAddresses[usK] = 0;
        }

        prListPort2->usAddressNmb = usI;
        prCSMD_Instance->usSercAddrLastSlaveP2 = prListPort2->ausAddresses[usI - 1U];
      }
    }
  }

} /* end: CSMD_DeleteSlave() */


/**************************************************************************/ /**
\brief CSMD_SlaveValidError() performs the error handling for slave valid bit in device status.

\ingroup module_cyclic
\b Description: \n
   This function is called if the device status word with slave valid bit set
   has not been received on either master port. It sets the public S-DEV value to
   zero, increments the counter for consecutive slave valid misses and checks if
   this counter has exceeded the selected maximum value. In this case, CSMD_DeleteSlave()
   is called.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usSlaveIdx
              Slave index of the slave which is deleted from lists

\return       none

\author       AlM
\date         06.05.2014

***************************************************************************** */
CSMD_VOID CSMD_SlaveValidError( CSMD_INSTANCE *prCSMD_Instance,
                                CSMD_USHORT    usSlaveIdx )
{
  prCSMD_Instance->arDevStatus[usSlaveIdx].usS_Dev = 0;

  /* if slave valid is not set, check if slave valid monitoring is enabled */
  if (prCSMD_Instance->rPriv.eMonitoringMode == CSMD_MONITORING_FULL)
  {
    /* copy zero and increment slave specific counter for consecutively missed slave valid bits */
    prCSMD_Instance->arDevStatus[usSlaveIdx].usMiss++;
  }

  /* if number of allowed consecutive slave valid misses is exceeded, the slave activity status
   * is set to inactive and its address is deleted from port topology list(s) */
  if (prCSMD_Instance->arDevStatus[usSlaveIdx].usMiss >
      prCSMD_Instance->rConfiguration.rComTiming.usAllowed_Slave_Valid_Miss)
  {
    CSMD_DeleteSlave (prCSMD_Instance, usSlaveIdx);
  }

} /* end: CSMD_SlaveValidError() */

/*! \endcond */ /* PRIVATE */


/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

31 Jan 2014 AlM
- File created.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
01 Sep 2015 WK
  - Defdb00181092
    CSMD_EvaluateConnections()
    Implemented support of "non cyclic type 2" connections.
    - Split common state into separate state CONSUMING and WARNING for
      connection type specific handling.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
07 Dec 2015 WK
  - Defdb00183676
    CSMD_EvaluateConnections()
    Consumed connections of the type "non cyclic type 2" were only
    about every 5 to 10 telegram considered valid.
04 Apr 2016 AlM
  - Defdb00184988
    CSMD_EvaluateConnections()
    Fixed problem with device status and connection data produced by one slave
    configured in different ATs
19 May 2016 AlM
  - Defdb00187136
    CSMD_EvaluateConnections() Transition WAITING to CONSUMING:
      Adapt expectation value for C-CON to produced value.
    CSMD_Set_C_Con_Check_Mode()
      Removed calculation of the expectation value for C-CON.
  
------------------------------------------------------------------------------
*/
