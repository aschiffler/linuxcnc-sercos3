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
 \file   CSMD_PARSER.c
 \author MS
 \date   19.11.2010
 \brief  This File contains the public functions for connection configuration
         purposes through a binary format
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#ifdef CSMD_CONFIG_PARSER

#define SOURCE_CSMD
#include "CSMD_PARSER.h"

/*---- Definition public Functions: ------------------------------------------*/


/*! \cond PUBLIC */

/**************************************************************************/ /**
\brief Builds a binary representation of the current configuration.

\ingroup func_binconfig
\b Description: \n
   This function builds a Sercos list which holds a binary representation of
   the connections determined by the given parameters. The target format of
   the output can be selected. The length of the given buffer can be passed by
   setting the maximum list length in the buffer of the Sercos list. The
   application ID is taken as either a positive or a negative marker for the 
   connection selection within the list; the input parameter boAppID_Pos 
   determines the interpretation of the application ID.

<B>Call Environment:</B> \n
   The function can be called at any time.\n
   The call-up should be performed from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usCFGbinVersion
              Target format of the binary configuration list
\param [in]   usAppID
              Application ID, for which the configuration should be built
\param [in]   boAppID_Pos
              Flag for positive or negative handling of given application ID
\param [in,out] pvTargetBinConfig
                Pointer to the binary target configuration (Sercos list)

\return       \ref CSMD_DOUBLE_PRODUCER \n
              \ref CSMD_NO_PRODUCER \n
              \ref CSMD_NO_CONSUMER \n
              \ref CSMD_BUFFER_TOO_SMALL \n
              \ref CSMD_BIN_CONFIG_VERSION_UNAVAILABLE \n
              \ref CSMD_APPID_UNAVAILABLE \n
              \ref CSMD_NO_ERROR \n

\author       MSt
\date         18.12.2010

***************************************************************************** */
CSMD_FUNC_RET CSMD_GenerateBinConfig ( CSMD_INSTANCE     *prCSMD_Instance,
                                       const CSMD_USHORT  usCFGbinVersion,
                                       const CSMD_USHORT  usAppID,
                                       const CSMD_BOOL    boAppID_Pos,
                                       CSMD_VOID         *pvTargetBinConfig )
{
  CSMD_FUNC_RET       eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT        *pusActBinListPosition = (CSMD_USHORT *)pvTargetBinConfig;
  CSMD_UCHAR         *pucCharHelpPointer;
  CSMD_USHORT        *pusCnncStartPtr;
  CSMD_USHORT        *pusConsLStartPtr;
  CSMD_USHORT        *pusCnsmStartPtr;
  CSMD_USHORT        *pusConsumerListAuxPtr;
  CSMD_USHORT         usNumberOfConnections = 0;
  CSMD_USHORT         usTotalConsumersNbr = 0;
  CSMD_CONFIG_STRUCT *prConfig = &prCSMD_Instance->rConfiguration;
  CSMD_USHORT         usI, usK, usL;    /* counter variables */
  CSMD_UCHAR          aucNeededConn[CSMD_MAX_GLOB_CONN]        = {FALSE}; /* boolean marker for connection as output */
  CSMD_UCHAR          aucUsedConfigIdx[CSMD_MAX_GLOB_CONFIG]   = {FALSE}; /* boolean marker for used configuration */
  CSMD_UCHAR          aucUsedRT_BitIdx[CSMD_MAX_RT_BIT_CONFIG] = {FALSE}; /* boolean marker for used realtime bit */
  CSMD_BOOL           boUseProjAddresses = TRUE;
  CSMD_BOOL           boFound = FALSE;
  CSMD_BOOL           boFoundSlaveSetup = FALSE;
  CSMD_CONN_PROD_CONS arTempProdConsIdx[CSMD_MAX_GLOB_CONN] = {{0,0}};
  CSMD_USHORT         usActBinConfigLength = CSMD_LIST_HEADER_LEN; /* Initialize with Length of header */
  CSMD_USHORT         usActConsListKey = 1;
  CSMD_USHORT         usTempSlaveIdx = 0;
  CSMD_USHORT         usGivenBufferLength;
  CSMD_USHORT         usNbrSlaves;
  CSMD_ULONG          ulAllowedLength;

  /* Get the allowed length information from the given buffer */
  usGivenBufferLength = CSMD_LENGTH_CONV(*(pusActBinListPosition + 1));

  if (usGivenBufferLength == 0)
  {
    /* No length given? -> set length to max */
    ulAllowedLength = 0xFFFFFFFF;
  }
  else
  {
    /* Take given length */
    ulAllowedLength = (CSMD_ULONG)usGivenBufferLength;
  }

  /* Check whether the original CoSeMa structure is used or not */
  /* Check only one pointer should be sufficient */
  if (prConfig->prMaster_Config != &prConfig->rMasterCfg)
  {
    /* No CoSeMa structure -> switch off the address check */
    boUseProjAddresses = FALSE;
  }
  
  /* Get the number of slaves to check; depends on the used structure */
  if (boUseProjAddresses)
  {
    usNbrSlaves = prCSMD_Instance->rSlaveList.usNumProjSlaves;
  }
  else
  {
    usNbrSlaves = (CSMD_USHORT) (prCSMD_Instance->rSlaveList.ausParserTempAddList[0] / 2);
  }
  
  /* Search all connections for the requested application ID */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn; usI++)
  {
          /* given application ID is requested or any application */
    if (   (   (boAppID_Pos)
            && (   (usAppID == CSMD_UNIVERSAL_APP_ID)
                || (prConfig->parConnection[usI].usApplicationID == usAppID)
               )
           )
           /* given application ID is NOT requested */
        || (   (!boAppID_Pos)
            &&  (prConfig->parConnection[usI].usApplicationID != usAppID)
           )
       )
    {
      /* Check all master connections */
      for (usK = 0; usK < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usK++)
      {
        /* Is this connection used by the master? */
        if (prConfig->prMaster_Config->parConnIdxList[usK].usConnIdx == usI)
        {
          /* just to make the term shorter */
          CSMD_USHORT usTemp_CfgIdx = prConfig->prMaster_Config->parConnIdxList[usK].usConfigIdx;
          
          /* Is the connection configured as active? */
          if (prConfig->parConfiguration[usTemp_CfgIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE)
          {
            /* set binary marker */
            aucNeededConn[usI] = TRUE;
            
            /* Set marker for further processing */
            boFound = TRUE;
            
            /* Stop further processing */
            break; /* for (usK ...) */
          }
        }
      }

      /* if successful, go on with next connection */
      if (aucNeededConn[usI])
      {
        continue; /* for (usI ...) */
      }
      
      /* Check all slaves */
      for (usK = 0; usK < usNbrSlaves; usK++)
      {
        /* Check all slave connections */
        for (usL = 0; usL < CSMD_MAX_CONNECTIONS; usL++)
        {
          /* Is this connection used by the slave? */
          if (prConfig->parSlave_Config[usK].arConnIdxList[usL].usConnIdx == usI)
          {
            /* just to make the term shorter */
            CSMD_USHORT usTemp_CfgIdx = prConfig->parSlave_Config[usK].arConnIdxList[usL].usConfigIdx;
            
            /* Is the connection configured as active? */
            if (prConfig->parConfiguration[usTemp_CfgIdx].usS_0_1050_SE1 & CSMD_S_0_1050_SE1_ACTIVE)
            {
              /* set binary marker */
              aucNeededConn[usI] = TRUE;
              
              /* Set marker for further processing */
              boFound = TRUE;
              
              /* Stop further processing */
              break; /* for (usL ...) */
            }
          }
        }
        
        /* if successful, stop further processing */
        if (aucNeededConn[usI])
        {
          break; /* for (usK ...) */
        }
      }
    }/*  if (   ((boAppID_Pos) ... */
  } /* for (usI ... ) */

  /* Now the slave setup parameter */

  /* Just search for one matching AppID; further processing is done later */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList; usI++)
  {
    /* given application ID is requested or any application */
    if (   (   (boAppID_Pos)
            && (   (usAppID == CSMD_UNIVERSAL_APP_ID)
                || (prConfig->parConfigParamsList[usI].usApplicationID == usAppID))
           )
        /* given application ID is NOT requested */
        || (   (!boAppID_Pos)
            &&  (prConfig->parConfigParamsList[usI].usApplicationID != usAppID))
       )
    {
      /* Check if this parameter list is used by any slave */
      for (usK = 0; usK < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams; usK++)
      {
        if (prConfig->parSlaveParamConfig[usK].usConfigParamsList_Index == usI)
        {
          /* Set marker for further processing */
          boFoundSlaveSetup = TRUE;
          
          /* Stop further processing */
          break; 
        }
        
        if (boFoundSlaveSetup)
        {
          break;
        }
      }
    }
  }

  /* More to do */
  if (boFound || boFoundSlaveSetup)
  {
    /* Put the consumer and producer of the connection into a temporarily array */
    /* Stop if an error occurs */
    for (usI = 0;
        (usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn) && (eFuncRet == CSMD_NO_ERROR); usI++)
    {
      /* Is this connection needed? */
      if (aucNeededConn[usI])
      {
        /* search for the participants, projected slaves, all connections */
        /* Stop if an error occurs */
        for (usK = 0;
            (usK < usNbrSlaves) && (eFuncRet == CSMD_NO_ERROR); usK++)
        {
          for (usL = 0; usL < (CSMD_USHORT) CSMD_MAX_CONNECTIONS; usL++)
          {
            /* just to make the term shorter */
            CSMD_CONN_IDX_STRUCT *prConnIdx = &prConfig->parSlave_Config[usK].arConnIdxList[usL];

            /* Is this slave a participant of the connection ...? */
            if (prConnIdx->usConnIdx == usI)
            {
              /* ... as producer? */
              if ((  prConfig->parConfiguration[prConnIdx->usConfigIdx].usS_0_1050_SE1
                   & CSMD_S_0_1050_SE1_ACTIVE_PRODUCER) == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
              {
                /*security check for two producers; should not be */
                if (arTempProdConsIdx[usI].usConnection_Producer_Index)
                {
                  /* severe error !!! */
                  eFuncRet = CSMD_DOUBLE_PRODUCER;

                  break;
                }

                /* set the information for the producer */
                arTempProdConsIdx[usI].usConnection_Producer_Index = 
                  (CSMD_USHORT)((usK * CSMD_MAX_CONNECTIONS) + usL + CSMD_PREVENT_NULL);
              }
              /* ... as consumer ?? */
              else if ((  prConfig->parConfiguration[prConnIdx->usConfigIdx].usS_0_1050_SE1
                        & CSMD_S_0_1050_SE1_ACTIVE_CONSUMER) == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
              {
                /* Check for already existing consumer */
                if (arTempProdConsIdx[usI].usConnection_Consumer_Index)
                {
                  /* set the information for more than one consumer -> 0xFFFF */
                  arTempProdConsIdx[usI].usConnection_Consumer_Index = 0xFFFF;
                }
                /*first consumer */
                else
                {
                  /* set the information for the consumer */
                  arTempProdConsIdx[usI].usConnection_Consumer_Index = 
                    (CSMD_USHORT)((usK * CSMD_MAX_CONNECTIONS) + usL + CSMD_PREVENT_NULL);
                }
              }
              
              /* found, so finish */
              break;
            }
          }
        }

        /* no error occurred so far? -> go on */

        /*check the master */
        for (usL = 0;
            (usL < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster) && (eFuncRet == CSMD_NO_ERROR); usL++)
        {
          /* Is the master a participant of the connection ...? */
          if (prConfig->prMaster_Config->parConnIdxList[usL].usConnIdx == usI)
          {
            /* ... as producer? */
            if ((  prConfig->parConfiguration[prConfig->prMaster_Config->parConnIdxList[usL].usConfigIdx].usS_0_1050_SE1
                 & CSMD_S_0_1050_SE1_ACTIVE_PRODUCER) == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
            {
              /*security check for two producers; should not be */
              if (arTempProdConsIdx[usI].usConnection_Producer_Index)
              {
                /* severe error !!! */
                eFuncRet = CSMD_DOUBLE_PRODUCER;
                
                break;
              }
              
              /* set the information for the producer */
              /* take a number behind the slave configuration for the master */
              arTempProdConsIdx[usI].usConnection_Producer_Index = 
                (CSMD_USHORT)((CSMD_MAX_CONNECTIONS * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves) + usL + CSMD_PREVENT_NULL);
            }
            /* ... as consumer ?? */
            else if ((  prConfig->parConfiguration[prConfig->prMaster_Config->parConnIdxList[usL].usConfigIdx].usS_0_1050_SE1
                      & CSMD_S_0_1050_SE1_ACTIVE_CONSUMER) == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
            {
              /* Check for already existing consumer */
              if (arTempProdConsIdx[usI].usConnection_Consumer_Index)
              {
                /* set the information for more than one consumer -> 0xFFFF */
                arTempProdConsIdx[usI].usConnection_Consumer_Index = 0xFFFF;
              }
              /*first consumer */
              else
              {
                /* set the information for the consumer */
                /* take a number behind the slave configuration for the master */
                arTempProdConsIdx[usI].usConnection_Consumer_Index = 
                  (CSMD_USHORT)((CSMD_MAX_CONNECTIONS * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves) + usL + CSMD_PREVENT_NULL);
              }
            }
            
            /* found, so finish */
            break;
          }
        }
        
        /* check for existing producer */
        if ((arTempProdConsIdx[usI].usConnection_Producer_Index == 0)
          && (eFuncRet == CSMD_NO_ERROR))
        {
          /* severe error !!! */
          eFuncRet = CSMD_NO_PRODUCER;
          
          break;
        }
        
        /* check for existing consumer */
        if (   (arTempProdConsIdx[usI].usConnection_Consumer_Index == 0)
            && (eFuncRet == CSMD_NO_ERROR))
        {
          /* severe error !!! */
          eFuncRet = CSMD_NO_CONSUMER;
          
          break;
        }
        
      }
    }

    /* no error occurred so far? -> go on */
    if (eFuncRet == CSMD_NO_ERROR)
    {
      switch (usCFGbinVersion)
      {
      case BIN_CONFIG_VERSION_01_01:
        
        /* Check if the given length is exceeded */
        if (ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_TABLE_HEADER_LEN))
        {
          eFuncRet = CSMD_BUFFER_TOO_SMALL;
          break;
        }

        /* set actual list position behind length information */
        pusActBinListPosition += 2;

        /* set header to "CSMCFG_bin" + version information */
        pucCharHelpPointer = (CSMD_UCHAR *)pusActBinListPosition;
        
        *pucCharHelpPointer++ = 0x43; /* "C" */
        *pucCharHelpPointer++ = 0x53; /* "S" */
        *pucCharHelpPointer++ = 0x4D; /* "M" */
        *pucCharHelpPointer++ = 0x43; /* "C" */
        *pucCharHelpPointer++ = 0x66; /* "f" */
        *pucCharHelpPointer++ = 0x67; /* "g" */
        *pucCharHelpPointer++ = 0x5F; /* "_" */
        *pucCharHelpPointer++ = 0x62; /* "b" */
        *pucCharHelpPointer++ = 0x69; /* "i" */
        *pucCharHelpPointer++ = 0x6E; /* "n" */

        pusActBinListPosition = (CSMD_USHORT *)(CSMD_VOID *)pucCharHelpPointer;

        *pusActBinListPosition++ = CSMD_END_CONV_S(usCFGbinVersion);
        
        /***********************************
        **  copy connections              **
        ************************************
        ************************************
        **  Structure:                    **
        **                                **
        **  SHORT     connection key      **
        **  SHORT     connection number   **
        **  SHORT     telegram assignment **
        **  SHORT     length              **
        **  SHORT     application ID      **
        **  CHAR[30]  connection name     **
        **  SHORT     producer key        **
        **  SHORT     consumer list key   **
        ***********************************/
        
        /* set header of connection table to "Cnnc" */
        pucCharHelpPointer = (CSMD_UCHAR *)pusActBinListPosition;

        *pucCharHelpPointer++ = 0x43; /* "C" */
        *pucCharHelpPointer++ = 0x6E; /* "n" */
        *pucCharHelpPointer++ = 0x6E; /* "n" */
        *pucCharHelpPointer++ = 0x63; /* "c" */

        pusActBinListPosition = (CSMD_USHORT *)(CSMD_VOID *)pucCharHelpPointer;
        
        /* Adjust list length */
        usActBinConfigLength += CSMD_TABLE_HEADER_LEN;
        
        /* Store the start pointer of the connection table */
        pusCnncStartPtr = pusActBinListPosition;
        
        /* check all connections and copy if needed */
        for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn; usI++)
        {
          /* Is this connection needed? */
          if (aucNeededConn[usI])
          {
            /* Check if the given length is exceeded */
            if (ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_CONNECTION_TABLE_LEN))
            {
              eFuncRet = CSMD_BUFFER_TOO_SMALL;
              break;
            }

            /* connection key is the index */
            *pusActBinListPosition++ = CSMD_END_CONV_S(usI);
            
            /* connection number (might be empty) */
            *pusActBinListPosition++ = CSMD_END_CONV_S(prConfig->parConnection[usI].usS_0_1050_SE2);
            
            /* telegram assignment (might be empty) */
            *pusActBinListPosition++ = CSMD_END_CONV_S(prConfig->parConnection[usI].usS_0_1050_SE3);
            
            /* connection length (might be empty) */
            *pusActBinListPosition++ = CSMD_END_CONV_S(prConfig->parConnection[usI].usS_0_1050_SE5);
            
            /* application ID */
            *pusActBinListPosition++ = CSMD_END_CONV_S(prConfig->parConnection[usI].usApplicationID);
            
            /* connection name (30 byte) */
            pucCharHelpPointer = (CSMD_UCHAR *)pusActBinListPosition;

            for (usK = 0; usK < (CSMD_USHORT) CSMD_CONN_NAME_LENGTH; usK++)
            {
              *pucCharHelpPointer++ = prConfig->parConnection[usI].ucConnectionName[usK];
            }

            pusActBinListPosition = (CSMD_USHORT *)(CSMD_VOID *)pucCharHelpPointer;
            
            /* producer key */
            *pusActBinListPosition++ = CSMD_END_CONV_S(arTempProdConsIdx[usI].usConnection_Producer_Index);
            
            /* consumer list key */
            *pusActBinListPosition++ = CSMD_END_CONV_S(usActConsListKey++);
            
            /* Adjust list length */
            usActBinConfigLength += CSMD_CONNECTION_TABLE_LEN;
            
            /* Increment the number of used connections */
            usNumberOfConnections++;
          }
        }

        /* Check if the given length is exceeded */
        if (   (ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_END_SIGN_LENGTH + CSMD_TABLE_HEADER_LEN))
            || (eFuncRet != CSMD_NO_ERROR))
        {
          eFuncRet = CSMD_BUFFER_TOO_SMALL;
          break;
        }

        CSMD_Write_Header( &pusActBinListPosition,
                           &usActBinConfigLength,
                           CSMD_PROD_HEADER );

        /*************************************
        **  copy Producer                   **
        **************************************
        **************************************
        **  Structure:                      **
        **                                  **
        **  SHORT     producer key          **
        **  SHORT     Sercos address        **
        **  LONG      connection cycle time **
        **  SHORT     connection instance   **
        **  SHORT     dummy                 **
        **  SHORT     configuration key     **
        **  SHORT     RT Bits key           **
        **************************************/
        
        /* check all connections and copy the data */
        for (usI = 0; usI < usNumberOfConnections; usI++)
        {
          CSMD_USHORT    usTempConnInst = 0;
          
          /* get the actual connection */
          CSMD_CONNECTION_TABLE rConnTable 
            = *(CSMD_CONNECTION_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)pusCnncStartPtr) + (usI * CSMD_CONNECTION_TABLE_LEN));
          
          /* Check if the given length is exceeded */
          if (ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_PRODUCER_TABLE_LEN))
          {
            eFuncRet = CSMD_BUFFER_TOO_SMALL;
            break;
          }

          /* Copy the producer key */
          /* No swapping necessary, value already in correct endianness */
          *pusActBinListPosition++ = rConnTable.usProducerKey;
          
          /* Calculate the Sercos address */
          if (CSMD_END_CONV_S(rConnTable.usProducerKey) > (CSMD_MAX_CONNECTIONS * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves))
          {
            /* it`s the master */
            *pusActBinListPosition++ = CSMD_END_CONV_S(0);
          }
          else
          {
            /* a slave */
            usTempSlaveIdx = (CSMD_USHORT)((CSMD_END_CONV_S(rConnTable.usProducerKey)) / CSMD_MAX_CONNECTIONS);
            
            /* Is the CoSeMa address list used or the temporary address list? */
            if (boUseProjAddresses)
            {
              *pusActBinListPosition++ = 
                CSMD_END_CONV_S(prCSMD_Instance->rSlaveList.ausProjSlaveAddList[usTempSlaveIdx + 2]);
            }
            else
            {
              CSMD_USHORT  usTempAddress = prCSMD_Instance->rSlaveList.ausParserTempAddList[usTempSlaveIdx + 2];
              
              /* If a slave was deactivated, calculate the correct address */
              if (usTempAddress > CSMD_MAX_SLAVE_ADD)
              {
                usTempAddress -= CSMD_MAX_SLAVE_ADD;
              }

              *pusActBinListPosition++ = CSMD_END_CONV_S(usTempAddress);
            }
          }
          
          /* Cycle time */
          *pusActBinListPosition++ = CSMD_END_CONV_S((CSMD_USHORT)
            (prConfig->parConnection[CSMD_END_CONV_S(rConnTable.usConnectionKey)].ulS_0_1050_SE10 & 0xFFFF));
          *pusActBinListPosition++ = CSMD_END_CONV_S((CSMD_USHORT)
            ((prConfig->parConnection[CSMD_END_CONV_S(rConnTable.usConnectionKey)].ulS_0_1050_SE10 & 0xFFFF0000) >> 16));
          
          /* connection instance */
          if (CSMD_END_CONV_S(rConnTable.usProducerKey) > (CSMD_MAX_CONNECTIONS * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves))
          {
            /* it`s the master */
            usTempConnInst = (CSMD_USHORT)(CSMD_END_CONV_S(rConnTable.usProducerKey) 
              - (CSMD_USHORT)((CSMD_MAX_CONNECTIONS * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves) + CSMD_PREVENT_NULL));
            
            *pusActBinListPosition++ = CSMD_END_CONV_S(usTempConnInst);
          }
          else
          {
            /* a slave */
            usTempConnInst = (CSMD_USHORT)(CSMD_END_CONV_S(rConnTable.usProducerKey) - 
              (CSMD_USHORT)((((CSMD_END_CONV_S(rConnTable.usProducerKey)) / CSMD_MAX_CONNECTIONS) * CSMD_MAX_CONNECTIONS) + CSMD_PREVENT_NULL));
            
            *pusActBinListPosition++ = CSMD_END_CONV_S(usTempConnInst);
          }
          
          /* a dummy */
          *pusActBinListPosition++ = CSMD_END_CONV_S(CSMD_TABLE_DUMMY);
          
          /* the key to the config table ... */
          /* ... the key to the realtime-bits table */
          if (CSMD_END_CONV_S(rConnTable.usProducerKey) > (CSMD_MAX_CONNECTIONS * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves))
          {
            /* it`s the master */
            CSMD_USHORT  usConfIdx = (CSMD_USHORT)(prConfig->prMaster_Config->parConnIdxList[usTempConnInst].usConfigIdx
              + CSMD_PREVENT_NULL);
            CSMD_USHORT  usRTIdx;
            
            /* Keep the data if no RT bit configuration is available */
            if (prConfig->prMaster_Config->parConnIdxList[usTempConnInst].usRTBitsIdx == 0xFFFF)
            {
              usRTIdx = 0xFFFF;
            }
            else
            {
              usRTIdx = (CSMD_USHORT)(prConfig->prMaster_Config->parConnIdxList[usTempConnInst].usRTBitsIdx
                + CSMD_PREVENT_NULL);
            }
            
            *pusActBinListPosition++ = CSMD_END_CONV_S(usConfIdx);
            
            /* mark the used configuration */
            aucUsedConfigIdx[usConfIdx - CSMD_PREVENT_NULL] = TRUE;
            
            *pusActBinListPosition++ = CSMD_END_CONV_S(usRTIdx);
            
          }
          else
          {
            /* a slave */
            CSMD_USHORT  usConfIdx = (CSMD_USHORT) 
              (prConfig->parSlave_Config[usTempSlaveIdx].arConnIdxList[usTempConnInst].usConfigIdx
              + CSMD_PREVENT_NULL);
            CSMD_USHORT  usRTIdx;
            
            /* Keep the data if no RT bit configuration is available */
            if (prConfig->parSlave_Config[usTempSlaveIdx].arConnIdxList[usTempConnInst].usRTBitsIdx == 0xFFFF)
            {
              usRTIdx = 0xFFFF;
            }
            else
            {
              usRTIdx = (CSMD_USHORT)
                (prConfig->parSlave_Config[usTempSlaveIdx].arConnIdxList[usTempConnInst].usRTBitsIdx
                + CSMD_PREVENT_NULL);
            }
            
            *pusActBinListPosition++ = CSMD_END_CONV_S(usConfIdx);
            
            /* mark the used configuration */
            aucUsedConfigIdx[usConfIdx - CSMD_PREVENT_NULL] = TRUE;
            
            *pusActBinListPosition++ = CSMD_END_CONV_S(usRTIdx);
            
            /* Check for used RT bits */
            if (usRTIdx != 0xFFFF)
            {
              /* mark the used RT bits */
              aucUsedRT_BitIdx[usRTIdx - CSMD_PREVENT_NULL] = TRUE;
            }
          }
          
          /* Adjust list length */
          usActBinConfigLength += CSMD_PRODUCER_TABLE_LEN;
        }
        
        /* Check if the given length is exceeded */
        if (   (ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_END_SIGN_LENGTH + CSMD_TABLE_HEADER_LEN))
            || (eFuncRet != CSMD_NO_ERROR))
        {
          eFuncRet = CSMD_BUFFER_TOO_SMALL;
          break;
        }

        CSMD_Write_Header( &pusActBinListPosition,
                           &usActBinConfigLength,
                           CSMD_CONSLIST_HEADER );
        
        /*****************************************
        **  build Consumer list table           **
        ******************************************
        ******************************************
        **  Structure:                          **
        **                                      **
        **  SHORT   consumer list key           **
        **  SHORT   number of consumers         **
        **  SHORT   consumer key #1             **
        **  SHORT   consumer key #2 (if needed) **
        **  SHORT   consumer key #n (if needed) **
        **  SHORT   dummy (if needed)           **
        *****************************************/
        
        /* Store the start pointer of the consumer list table */
        pusConsLStartPtr = pusActBinListPosition;
        
        /* check all connections and copy the data; */
        for (usI = 0; usI < usNumberOfConnections; usI++)
        {
          /* get the actual connection */
          CSMD_CONNECTION_TABLE rConnTable 
            = *(CSMD_CONNECTION_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)pusCnncStartPtr) + (usI * CSMD_CONNECTION_TABLE_LEN));
          
          /* Check if the given length is exceeded */
          if (  ulAllowedLength
              < (CSMD_ULONG)(usActBinConfigLength + CSMD_ONE_CONSUMER_LEN + usNbrSlaves * sizeof(CSMD_USHORT)))
          {
            eFuncRet = CSMD_BUFFER_TOO_SMALL;
            break;
          }
          
          /* copy consumer list key */
          /* No swapping necessary, value already in correct endianness */
          *pusActBinListPosition++ = rConnTable.usConsumerListKey;
          
          /* is it single consumer? */
          if (arTempProdConsIdx[CSMD_END_CONV_S(rConnTable.usConnectionKey)].usConnection_Consumer_Index != 0xFFFF)
          {
            /*set number of consumer to 1 */
            *pusActBinListPosition++ = CSMD_END_CONV_S(1);
            
            /* set consumer key */
            *pusActBinListPosition++ = CSMD_END_CONV_S(
              arTempProdConsIdx[CSMD_END_CONV_S(rConnTable.usConnectionKey)].usConnection_Consumer_Index);
            
            /* a dummy */
            *pusActBinListPosition++ = CSMD_END_CONV_S(CSMD_TABLE_DUMMY);
            
            /* Adjust list length */
            usActBinConfigLength += CSMD_ONE_CONSUMER_LEN;
          }
          /* more than one consumer */
          else
          {
            /* set temporarily pointer to number of consumers and  */
            /* set actual position to the first consumer */
            CSMD_USHORT *pusNumberConsTemp = pusActBinListPosition++;
            
            /* Initialize number of consumers */
            *pusNumberConsTemp = 0;
            
            /* search for the consumers */
            for (usK = 0; usK < usNbrSlaves; usK++)
            {
              for (usL = 0; usL < CSMD_MAX_CONNECTIONS; usL++)
              {
                /* just to make it shorter */
                CSMD_CONN_IDX_STRUCT *prConnIdx = &prConfig->parSlave_Config[usK].arConnIdxList[usL];
                
                /* Is this slave a participant of the connection ...? */
                if (prConnIdx->usConnIdx == CSMD_END_CONV_S(rConnTable.usConnectionKey))
                {
                  /* ... as producer? */
                  if (  prConfig->parConfiguration[prConnIdx->usConfigIdx].usS_0_1050_SE1
                      & CSMD_S_0_1050_SE1_PRODUCER)
                  {
                    /* producer, so finish */
                    break;
                  }
                  /* ... as consumer !! */
                  else
                  {
                    /* set the consumer information */
                    *pusActBinListPosition++ = CSMD_END_CONV_S(
                      (CSMD_USHORT)((usK * CSMD_MAX_CONNECTIONS) + usL + CSMD_PREVENT_NULL));
                    
                    /*increment the number of consumers */
                    *pusNumberConsTemp += 1;
                    
                    /* found, so finish */
                    break;
                  }
                }
              }
            }

            /* Adjust list length; for each consumer 2 byte + 4 byte for key and consumer number */
            usActBinConfigLength += (CSMD_USHORT)((*pusNumberConsTemp * 2) + 4);
            
            /*all slaves are checked; is a dummy necessary (odd number of consumers)? */
            if (*pusNumberConsTemp & 0x1)
            {
              /* a dummy */
              *pusActBinListPosition++ = CSMD_END_CONV_S(CSMD_TABLE_DUMMY);
              
              /* Adjust list length; 2 byte for dummy */
              usActBinConfigLength += 2;
            }
          }
        }
        
        /* Check if the given length is exceeded */
        if ((ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_END_SIGN_LENGTH + CSMD_TABLE_HEADER_LEN))
          || (eFuncRet != CSMD_NO_ERROR))
        {
          eFuncRet = CSMD_BUFFER_TOO_SMALL;
          break;
        }

        CSMD_Write_Header( &pusActBinListPosition,
                           &usActBinConfigLength,
                           CSMD_CONSUMER_HEADER );
        
        /*************************************
        **  copy Consumer table             **
        **************************************
        **************************************
        **  Structure:                      **
        **                                  **
        **  SHORT     consumer key          **
        **  SHORT     Sercos address        **
        **  LONG      connection cycle time **
        **  SHORT     connection instance   **
        **  SHORT     allowed data losses   **
        **  SHORT     configuration key     **
        **  SHORT     RT Bits key           **
        **************************************/

        /* Store the pointer of the consumer table beginning */
        pusCnsmStartPtr = pusActBinListPosition;
        
        /* set aux pointer for the consumer list */
        pusConsumerListAuxPtr = pusConsLStartPtr;

        /* examine all data up to the beginning of the consumer table */
        while(   (pusConsumerListAuxPtr < (pusCnsmStartPtr - (CSMD_TABLE_OVERHEAD / 2)))
              && (eFuncRet == CSMD_NO_ERROR) )
        {
          CSMD_USHORT  usConsNbr;
          CSMD_USHORT  usTempConsKey;
          CSMD_USHORT  usTempConnIdx;
          CSMD_USHORT  usTempSlaveConnInst;
          
          /* key to consumer list is not important -> skip */
          pusConsumerListAuxPtr++;
          
          /* get the number of consumers for the table entry */
          usConsNbr = CSMD_END_CONV_S(*pusConsumerListAuxPtr++);
          
          /* increment total number of consumers */
          usTotalConsumersNbr += usConsNbr;
          
          /* check all consumers in the consumer list and copy the data */
          for (usI = 0; usI < usConsNbr; usI++)
          {
            /* Check if the given length is exceeded */
            if (ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength +
                (usTotalConsumersNbr * CSMD_CONSUMER_TABLE_LEN)))
            {
              eFuncRet = CSMD_BUFFER_TOO_SMALL;
              break;
            }
            
            /* Store the consumer key temporarily */
            usTempConsKey = CSMD_END_CONV_S(*pusConsumerListAuxPtr++);
            
            /* Copy the consumer key */
            *pusActBinListPosition++ = CSMD_END_CONV_S(usTempConsKey);
            
            /* Calculate the Sercos address */
            if (usTempConsKey > (CSMD_MAX_CONNECTIONS * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves))
            {
              /* it`s the master */
              *pusActBinListPosition++ = CSMD_END_CONV_S(0);
              
              /* calculate connection instance */
              usTempSlaveConnInst = (CSMD_USHORT)(usTempConsKey - 
                (CSMD_USHORT)((CSMD_MAX_CONNECTIONS * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves) + CSMD_PREVENT_NULL));
              
              /* calculate connection number */
              usTempConnIdx = 
                prConfig->prMaster_Config->parConnIdxList[usTempSlaveConnInst].usConnIdx;
            }
            else
            {
              /* a slave */
              usTempSlaveIdx = (CSMD_USHORT)((usTempConsKey - CSMD_PREVENT_NULL) / CSMD_MAX_CONNECTIONS);
              
              /* Is the CoSeMa address list used or the temporary address list? */
              if (boUseProjAddresses)
              {
                *pusActBinListPosition++ = 
                  CSMD_END_CONV_S(prCSMD_Instance->rSlaveList.ausProjSlaveAddList[usTempSlaveIdx + 2]);
              }
              else
              {
                CSMD_USHORT  usTempAddress = prCSMD_Instance->rSlaveList.ausParserTempAddList[usTempSlaveIdx + 2];
                
                /* If a slave was deactivated, calculate the correct address */
                if (usTempAddress > CSMD_MAX_SLAVE_ADD)
                {
                  usTempAddress -= CSMD_MAX_SLAVE_ADD;
                }
                
                *pusActBinListPosition++ = CSMD_END_CONV_S(usTempAddress);
              }
            
              /* calculate connection instance */
              usTempSlaveConnInst = (CSMD_USHORT)(usTempConsKey - 
                (CSMD_USHORT)((((usTempConsKey - CSMD_PREVENT_NULL) / CSMD_MAX_CONNECTIONS) * CSMD_MAX_CONNECTIONS) 
                + CSMD_PREVENT_NULL));
              
              /* calculate connection number */
              usTempConnIdx = 
                prConfig->parSlave_Config[usTempSlaveIdx].arConnIdxList[usTempSlaveConnInst].usConnIdx;
            }
            
            /* Cycle time */
            *pusActBinListPosition++ = CSMD_END_CONV_S((CSMD_USHORT)
              (prConfig->parConnection[usTempConnIdx].ulS_0_1050_SE10 & 0xFFFF));
            *pusActBinListPosition++ = CSMD_END_CONV_S((CSMD_USHORT)
              ((prConfig->parConnection[usTempConnIdx].ulS_0_1050_SE10 & 0xFFFF0000) >> 16));
            
            /* connection instance */
            *pusActBinListPosition++ = CSMD_END_CONV_S(usTempSlaveConnInst);
            
            /* allowed data losses */
            *pusActBinListPosition++ = CSMD_END_CONV_S(prConfig->parConnection[usTempConnIdx].usS_0_1050_SE11);
            
            /* the key to the config table ... */
            /* ... the key to the realtime-bits table */
            if (usTempConsKey > (CSMD_MAX_CONNECTIONS * prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves))
            {
              /* it`s the master; key "zero" not allowed */
              CSMD_USHORT  usConfIdx = (CSMD_USHORT)
                (prConfig->prMaster_Config->parConnIdxList[usTempSlaveConnInst].usConfigIdx + CSMD_PREVENT_NULL);
              CSMD_USHORT  usRTIdx;
              
              /* Keep the data if no RT bit configuration is available */
              if (prConfig->prMaster_Config->parConnIdxList[usTempSlaveConnInst].usRTBitsIdx == 0xFFFF)
              {
                usRTIdx = 0xFFFF;
              }
              else
              {
                usRTIdx = (CSMD_USHORT)(prConfig->prMaster_Config->parConnIdxList[usTempSlaveConnInst].usRTBitsIdx
                  + CSMD_PREVENT_NULL);
              }
              
              *pusActBinListPosition++ = CSMD_END_CONV_S(usConfIdx);
              
              /* mark the used configuration */
              aucUsedConfigIdx[usConfIdx - CSMD_PREVENT_NULL] = TRUE;
              
              *pusActBinListPosition++ = CSMD_END_CONV_S(usRTIdx);
              
            }
            else
            {
              /* a slave; key "zero" not allowed */
              CSMD_USHORT  usConfIdx = (CSMD_USHORT)
                (prConfig->parSlave_Config[usTempSlaveIdx].arConnIdxList[usTempSlaveConnInst].usConfigIdx
                + CSMD_PREVENT_NULL);
              CSMD_USHORT  usRTIdx;

              /* Keep the data if no RT bit configuration is available */
              if (prConfig->parSlave_Config[usTempSlaveIdx].arConnIdxList[usTempSlaveConnInst].usRTBitsIdx == 0xFFFF)
              {
                usRTIdx = 0xFFFF;
              }
              else
              {
                usRTIdx = (CSMD_USHORT)
                  (prConfig->parSlave_Config[usTempSlaveIdx].arConnIdxList[usTempSlaveConnInst].usRTBitsIdx
                  + CSMD_PREVENT_NULL);
              }
              
              *pusActBinListPosition++ = CSMD_END_CONV_S(usConfIdx);
              
              /* mark the used configuration */
              aucUsedConfigIdx[usConfIdx - CSMD_PREVENT_NULL] = TRUE;
              
              *pusActBinListPosition++ = CSMD_END_CONV_S(usRTIdx);
              
              /* Check for used RT bits */
              if (usRTIdx != 0xFFFF)
              {
                /* mark the used RT bits */
                aucUsedRT_BitIdx[usRTIdx - CSMD_PREVENT_NULL] = TRUE;
              }
            }
          }
          
          /*all consumers are checked; does a dummy exist (odd number of consumers)? */
          if (usConsNbr & 0x1)
          {
            pusConsumerListAuxPtr++;
          }
        }
        
        /* Adjust list length */
        usActBinConfigLength += (CSMD_USHORT)(CSMD_CONSUMER_TABLE_LEN * usTotalConsumersNbr);
        
        /* Check if the given length is exceeded */
        if ((ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_END_SIGN_LENGTH + CSMD_TABLE_HEADER_LEN))
            || (eFuncRet != CSMD_NO_ERROR))
        {
          eFuncRet = CSMD_BUFFER_TOO_SMALL;
          break;
        }

        CSMD_Write_Header( &pusActBinListPosition,
                           &usActBinConfigLength,
                           CSMD_CONFIG_HEADER );
        
        /*************************************
        **  copy Configuration table        **
        **************************************
        **************************************
        **  Structure:                      **
        **                                  **
        **  SHORT     configuration key     **
        **  SHORT     connection setup      **
        **  SHORT     connection capability **
        **  SHORT     number of IDNs        **
        **  LONG      IDN#1                 **
        **  LONG      IDN#2(if needed)      **
        **  LONG      IDN#n(if needed)      **
        *************************************/
        
        /* Check all configurations for being needed */
        for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig; usI++)
        {
          if (aucUsedConfigIdx[usI])
          {
            CSMD_USHORT  usTempNbrConfigured_IDNs;
            
            /* Check if the given length is exceeded */
            if (ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_CONFIG_TABLE_LEN_WO_IDN))
            {
              eFuncRet = CSMD_BUFFER_TOO_SMALL;
              break;
            }
            
            /* configuration key; don't let the key be 0 */
            *pusActBinListPosition++ = CSMD_END_CONV_S((CSMD_USHORT)(usI + CSMD_PREVENT_NULL));
            
            /* connection setup */
            *pusActBinListPosition++ = CSMD_END_CONV_S(prConfig->parConfiguration[usI].usS_0_1050_SE1);
            
            /* connection capability */
            *pusActBinListPosition++ = CSMD_END_CONV_S(prConfig->parConfiguration[usI].usS_0_1050_SE7);
            
            /* IDN list */
            usTempNbrConfigured_IDNs = 
              (CSMD_USHORT)(*((CSMD_USHORT *)(CSMD_VOID *)&prConfig->parConfiguration[usI].ulS_0_1050_SE6[0]) / 4);
            
            /* write number of IDNs */
            *pusActBinListPosition++ = CSMD_END_CONV_S(usTempNbrConfigured_IDNs);
            
            /* Check if the given length is exceeded */
            if (ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_CONFIG_TABLE_LEN_WO_IDN
                + (usTempNbrConfigured_IDNs * 4)))
            {
              eFuncRet = CSMD_BUFFER_TOO_SMALL;
              break;
            }
            
            /* write IDNs */
            for (usK = 1; usK <= (CSMD_USHORT) usTempNbrConfigured_IDNs; usK++)
            {
              /* write low word */
              *pusActBinListPosition++ = CSMD_END_CONV_S(
                (CSMD_USHORT)(prConfig->parConfiguration[usI].ulS_0_1050_SE6[usK] & 0xFFFF));
              /* write high word */
              *pusActBinListPosition++ = CSMD_END_CONV_S(
                (CSMD_USHORT)((prConfig->parConfiguration[usI].ulS_0_1050_SE6[usK] & 0xFFFF0000)>>16));
            }
            
            /* Adjust list length; first the fix beginning of the table */
            usActBinConfigLength += CSMD_CONFIG_TABLE_LEN_WO_IDN;
            
            /* ... and the dynamic length */
            usActBinConfigLength += (CSMD_USHORT)(usTempNbrConfigured_IDNs * 4);
          }
        }
        
        /* Check if the given length is exceeded */
        if ((ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_END_SIGN_LENGTH + CSMD_TABLE_HEADER_LEN))
            || (eFuncRet != CSMD_NO_ERROR))
        {
          eFuncRet = CSMD_BUFFER_TOO_SMALL;
          break;
        }

        CSMD_Write_Header( &pusActBinListPosition,
                           &usActBinConfigLength,
                           CSMD_RTBITS_HEADER );
        
        /*************************************
        **  copy RT bits table             **
        **************************************
        **************************************
        **  Structure:                      **
        **                                  **
        **  SHORT     RT bits key           **
        **  SHORT     Dummy                 **
        **  LONG      IDN for RT bit 1      **
        **  LONG      IDN for RT bit 2      **
        **  LONG      IDN for RT bit 3      **
        **  LONG      IDN for RT bit 4      **
        **  SHORT     Bit inside IDN 1      **
        **  SHORT     Bit inside IDN 2      **
        **  SHORT     Bit inside IDN 3      **
        **  SHORT     Bit inside IDN 4      **
        *************************************/
        
        /* Check all configurations for being needed */
        for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig; usI++)
        {
          if (aucUsedRT_BitIdx[usI])
          {
            /* Check if the given length is exceeded */
            if (ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_RTBITS_TABLE_LEN))
            {
              eFuncRet = CSMD_BUFFER_TOO_SMALL;
              break;
            }
            
            /* RT Bits key */
            *pusActBinListPosition++ = CSMD_END_CONV_S((CSMD_USHORT)(usI + CSMD_PREVENT_NULL));
            
            /* dummy */
            *pusActBinListPosition++ = CSMD_END_CONV_S(CSMD_TABLE_DUMMY);
            
            /* IDNs for RT bits */
            for (usK = 1; usK <= (CSMD_USHORT) CSMD_MAX_RT_BITS_PER_CONN; usK++)
            {
              /* write low word */
              *pusActBinListPosition++ = CSMD_END_CONV_S(
                (CSMD_USHORT)(prConfig->parRealTimeBit[usI].ulS_0_1050_SE20[usK] & 0xFFFF));
              /* write high word */
              *pusActBinListPosition++ = CSMD_END_CONV_S(
                (CSMD_USHORT)((prConfig->parRealTimeBit[usI].ulS_0_1050_SE20[usK] & 0xFFFF0000)>>16));
            }
            
            /* Bit inside IDN */
            for (usK = 2; usK <= (CSMD_USHORT) CSMD_MAX_RT_BITS_PER_CONN + 1; usK++)
            {
              /* write bit position */
              *pusActBinListPosition++ = CSMD_END_CONV_S(prConfig->parRealTimeBit[usI].usS_0_1050_SE21[usK]);
            }
            
            /* Adjust list length */
            usActBinConfigLength += CSMD_RTBITS_TABLE_LEN;
          }
        }
        
        /* Check if the given length is exceeded */
        if ((ulAllowedLength < (CSMD_ULONG)(usActBinConfigLength + CSMD_END_SIGN_LENGTH + CSMD_FILE_END_LENGTH))
            || (eFuncRet != CSMD_NO_ERROR))
        {
          eFuncRet = CSMD_BUFFER_TOO_SMALL;
          break;
        }

        /* There is a slave parameter setup to put into the configuration */
        if (boFoundSlaveSetup)
        {
          CSMD_UCHAR aucParamListUsed[CSMD_MAX_CONFIGPARAMS_LIST] = {FALSE};
          CSMD_UCHAR aucParamUsed[CSMD_MAX_CONFIG_PARAMETER]      = {FALSE};

          CSMD_Write_Header( &pusActBinListPosition,
                             &usActBinConfigLength,
                             CSMD_SLAVESETUP_HEADER );

          /*************************************
          **  copy Slave Setup table          **
          **************************************
          **************************************
          **  Structure:                      **
          **                                  **
          **  SHORT     Slave address         **
          **  SHORT     Parameter list key    **
          *************************************/

          /* Check all slave setup parameter lists */
          for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams; usI++)
          {
            
            CSMD_USHORT  usTempConfIdx = prConfig->parSlaveParamConfig[usI].usConfigParamsList_Index;
            
            usTempSlaveIdx = prConfig->parSlaveParamConfig[usI].usSlaveIndex;
            
            /* Parameter configuration used? -> Check application ID */
            if ( usTempConfIdx != 0xFFFF)
            {
              if (   (   (boAppID_Pos)
                      && (   (usAppID == CSMD_UNIVERSAL_APP_ID)
                          || (prConfig->parConfigParamsList[usTempConfIdx].usApplicationID == usAppID)))
                  || (   (!boAppID_Pos)
                      && (prConfig->parConfigParamsList[usTempConfIdx].usApplicationID != usAppID)) )
              {
                CSMD_USHORT  usSlaveAddresse;
                
                /* Is the CoSeMa address list used or the temporary address list? */
                if (boUseProjAddresses)
                {
                  usSlaveAddresse = prCSMD_Instance->rSlaveList.ausProjSlaveAddList[usTempSlaveIdx + 2];
                }
                else
                {
                  usSlaveAddresse = prCSMD_Instance->rSlaveList.ausParserTempAddList[usTempSlaveIdx + 2];
                  
                  /* If a slave was deactivated, calculate the correct address */
                  if (usSlaveAddresse > CSMD_MAX_SLAVE_ADD)
                  {
                    usSlaveAddresse -= CSMD_MAX_SLAVE_ADD;
                  }
                }
                
                /* Mark the setup parameter list */
                aucParamListUsed[usTempConfIdx] = TRUE;
                
                /* Write slave address and parameter list key */
                *pusActBinListPosition++ = CSMD_END_CONV_S(usSlaveAddresse);
                *pusActBinListPosition++ = CSMD_END_CONV_S((CSMD_USHORT)(usTempConfIdx + CSMD_PREVENT_NULL));
                
                /* Adjust list length */
                usActBinConfigLength += 4;
              }
            }
          }

          CSMD_Write_Header( &pusActBinListPosition,
                             &usActBinConfigLength,
                             CSMD_PARALIST_HEADER );

          /***********************************************
          **  copy Parameter list table                 **
          ************************************************
          ************************************************
          **  Structure:                                **
          **                                            **
          **  SHORT     Parameter list key              **
          **  SHORT     Number of Config Parameters     **
          **  SHORT     Parameter key #1                **
          **  SHORT     Parameter key #2 (if needed)    **
          **  SHORT     Parameter key #n (if needed)    **
          **  SHORT     Dummy (if needed)               **
          ***********************************************/

          /* Check all parameter lists */
          for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList; usI++)
          {
            if (aucParamListUsed[usI])
            {
              /* Write the parameter list key */
              *pusActBinListPosition++ = CSMD_END_CONV_S((CSMD_USHORT)(usI + CSMD_PREVENT_NULL));

              /* Write the application ID */
              *pusActBinListPosition++ = CSMD_END_CONV_S(prConfig->parConfigParamsList[usI].usApplicationID);

              /* check the number of configuration parameters */
              for (usK = 0; usK < CSMD_MAX_PARAMS_IN_CONFIG_LIST; usK++)
              {
                if (prConfig->parConfigParamsList[usI].ausParamTableIndex[usK] == 0xFFFF)
                {
                  /* This index is not used -> stop; usK holds the number of configuration parameters */
                  break;
                }
              }

              /* Write the number of config parameters */
              *pusActBinListPosition++ = CSMD_END_CONV_S(usK);

              /* Write the parameter keys */
              for (usL = 0; usL < usK; usL++)
              {
                CSMD_USHORT  usTableIndex = prConfig->parConfigParamsList[usI].ausParamTableIndex[usL];

                /* Write the parameter key */
                *pusActBinListPosition++ = CSMD_END_CONV_S((CSMD_USHORT)(usTableIndex + CSMD_PREVENT_NULL));

                /* Mark the parameter as used */
                aucParamUsed[usTableIndex] = TRUE;
              }

              /* Adjust list length */
              usActBinConfigLength += (CSMD_USHORT)(2 * (usK + 3));

              /* Write dummy, if needed */
              if (!(usK & 0x1))
              {
                *pusActBinListPosition++ = CSMD_END_CONV_S(CSMD_TABLE_DUMMY);

                /* Adjust list length */
                usActBinConfigLength += 2;
              }

            }
          }

          CSMD_Write_Header( &pusActBinListPosition,
                             &usActBinConfigLength,
                             CSMD_PARAMS_HEADER );

          /***************************************************
          **  copy Parameter table                          **
          ****************************************************
          ****************************************************
          **  Structure:                                    **
          **                                                **
          **  SHORT     Parameter key                       **
          **  SHORT     Data length                         **
          **  LONG      Parameter IDN                       **
          **  CHAR      Parameter data[CSMD_NBR_PARAM_DATA] **
          **  SHORT     Dummy (if needed)                   **
          ****************************************************/

          /* Check all parameters */
          for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter; usI++)
          {
            if (aucParamUsed[usI])
            {
              CSMD_USHORT  usDataLength = prConfig->parConfigParam[usI].usDataLength;

              /* Write the key */
              *pusActBinListPosition++ = CSMD_END_CONV_S((CSMD_USHORT)(usI + CSMD_PREVENT_NULL));

              /* Write the length */
              *pusActBinListPosition++ = CSMD_END_CONV_S(usDataLength);

              /* Write the IDN */
              /* write low word */
              *pusActBinListPosition++ = CSMD_END_CONV_S(
                (CSMD_USHORT)(prConfig->parConfigParam[usI].ulIDN & 0xFFFF));
              /* write high word */
              *pusActBinListPosition++ = CSMD_END_CONV_S(
                (CSMD_USHORT)((prConfig->parConfigParam[usI].ulIDN & 0xFFFF0000)>>16));

              /* Write the data in WORD pieces */
              for (usK = 0; usK < usDataLength; usK+=2)
              {
                /* Endianness conversion not necessary because data is only transported, not used */
                *pusActBinListPosition++ = *(CSMD_USHORT *)(CSMD_VOID *)(&prConfig->parConfigParam[usI].aucParamData[usK]);
              }

              /* Adjust list length */
              usActBinConfigLength += (CSMD_USHORT)(usDataLength + 8);

              if (usDataLength & 0x1)
              {
                usActBinConfigLength++;
              }

              /* Write dummy, if needed */
              if (   ((usDataLength & 0x1) || (usDataLength & 0x2))
                  && (!((usDataLength & 0x3) == 0x3)))
              {
                *pusActBinListPosition++ = CSMD_END_CONV_S(CSMD_TABLE_DUMMY);

                /* Adjust list length */
                usActBinConfigLength += 2;
              }

            }
          }
        }
        
        CSMD_Write_Header( &pusActBinListPosition,
                           &usActBinConfigLength,
                           CSMD_FILE_END );

        break;
        
      default:
        eFuncRet = CSMD_BIN_CONFIG_VERSION_UNAVAILABLE;
        break;
      }
    }
  }
  /* Nothing found -> finish */
  else
  {
    eFuncRet = CSMD_APPID_UNAVAILABLE;
  }

  /* Error occurred -> set the length to 0 to give no data */
  if (eFuncRet)
  {
    usActBinConfigLength = 0;
  }

  /* Write list length */
  *(CSMD_USHORT *)pvTargetBinConfig = CSMD_LENGTH_CONV(usActBinConfigLength);

  if (usGivenBufferLength == 0)
  {
    /* No length given -> write the actual length as maximum length */
    *((CSMD_USHORT *)pvTargetBinConfig + 1) = CSMD_LENGTH_CONV(usActBinConfigLength);
  }
  else
  {
    /* Don't write the max length, it is already correct given */
  }

  
  return(eFuncRet);

} /* end: CSMD_GenerateBinConfig() */

/**************************************************************************/ /**
\brief Processes a binary representation of a connection configuration
       and inserts its content into the configuration structure.

\ingroup func_binconfig
\b Description: \n
   This function processes a binary representation of a connection configuration,
   including:
   - deletion of remaining connections and corresponding configurations of the
     transmitted application IDs
   - insertion of the transmitted connections and corresponding configurations,
     either with or without automatically assignment of connection numbers and
     instances (depending on transfer parameter)
   - assignment of connections to master and slaves
   - assignment of parameter configurations for the master and the slaves; the 
     parameter configurations of the slaves are transmitted to the slaves during
     progression from CP2 to CP3
   - avoiding an address error if connection data for a non projected slave is 
     a part of the whole configuration data if this slave is marked as deactivated 
   - Cancellation of the whole connection configuration done in the current
     function call in case of error
     
   This function accepts connections between the master and slaves, either
   being in the list ausProjSlaveAddList[] or, if this list is not yet filled,
   puts the configuration data into an auxiliary structure. In this case the slave 
   addresses are taken as is without the check for being correct; this check is 
   done later when transferring the data from the auxiliary structure into the CoSeMa
   instance. Connection data for a slave with an address not existing in the list
   ausProjSlaveAddList[] will not lead to an error if this address is marked as
   deactivated in the list ausDeactSlaveAddList[]. It is not checked whether a slave
   can handle all connections configured for it, so it is possible that e.g.
   6 connections are configured for a slave that can only handle up to 4 connections.
   This error is generated by CSMD_GetTimingData().
   In case of automatically assignment of the structure instances of S-0-1050 for the
   connection configuration in the slave (boSlaveInstGen == TRUE), a new connection
   is inserted into the first empty structure instance. This might result in a
   disallowed connection configuration for a slave (command S-0-0127 fails), although
   the maximum number of connections is not exceeded. This effect results from a slave
   insisting on certain parameters configured or a limited length of a connection for
   certain structure instances of S-0-1050 (see slave documentation).\n
   The behavior may also occur if an image of a complete connection configuration
   produced by CSMD_GenerateBinConfig() is read.\n
   In contrast, using the connection capability in S-0-1050.x.7 will not result in an
   error, because in this case every connection is assigned a certain attribute.
   As a precondition for employing this feature, the affected slave has to
   support it.

<B>Call Environment:</B> \n
   Before the first call of this function, CSMD_Init_Config_Struct() should have been
   called in order to delete the existent connection configuration.\n
   The function be called at any time during communication phases CP0 to CP2.\n
   The call-up should be performed from a task.
     
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [out]  pvSourceBinConfig
              Output pointer to the source binary configuration (Sercos list)
\param [in]   boConnNumGen
              determines whether the connection number should be generated or
              taken from the source
\param [in]   boSlaveInstGen
              Determines whether the connection number inside the slave should
              be generated or taken from the source

\return       \ref CSMD_NO_BIN_CONFIG \n
              \ref CSMD_WRONG_BIN_CONFIG_VERSION \n
              \ref CSMD_WRONG_BIN_CONFIG_FORMAT \n
              \ref CSMD_WRONG_SLAVE_ADDRESS \n
              \ref CSMD_NO_PRODUCER_KEY \n
              \ref CSMD_NO_CONSUMER_LIST_KEY \n
              \ref CSMD_CONN_NBR_ALREADY_USED \n
              \ref CSMD_CONN_INST_ALREADY_USED \n
              \ref CSMD_CONN_INST_TOO_HIGH \n
              \ref CSMD_TOO_MANY_CONN_FOR_MASTER \n
              \ref CSMD_TOO_MANY_CONN_FOR_SLAVE \n
              \ref CSMD_TOO_MANY_CONNECTIONS \n
              \ref CSMD_TOO_MANY_CONFIGURATIONS \n
              \ref CSMD_TOO_MANY_RTB_CONFIG \n
              \ref CSMD_NO_CONSUMER_KEY \n
              \ref CSMD_NO_CONFIGURATION_KEY \n
              \ref CSMD_NO_RTB_CONFIG_KEY \n
              \ref CSMD_TOO_MANY_IDN_FOR_CONN \n
              \ref CSMD_CYCLE_TIME_UNEQUAL \n
              \ref CSMD_TOO_MANY_SLAVE_SETUP \n
              \ref CSMD_NO_SETUP_PARAMETER_KEY \n
              \ref CSMD_NO_SETUP_LIST_KEY \n
              \ref CSMD_TOO_MANY_PARAMETER_IN_LIST \n
              \ref CSMD_TOO_MANY_PARAMETER_DATA \n
              \ref CSMD_TOO_MANY_SETUP_PARAMETER \n
              \ref CSMD_TOO_MANY_SETUP_LISTS \n
              \ref CSMD_NO_ERROR \n

\author       MSt
\date         11.01.2011

***************************************************************************** */
CSMD_FUNC_RET CSMD_ProcessBinConfig ( CSMD_INSTANCE   *prCSMD_Instance,
                                      CSMD_VOID       *pvSourceBinConfig,
                                      const CSMD_BOOL  boConnNumGen,
                                      const CSMD_BOOL  boSlaveInstGen )
{
  CSMD_FUNC_RET       eFuncRet = CSMD_NO_ERROR;
  CSMD_CONFIG_STRUCT *prConfig = &prCSMD_Instance->rConfiguration;
  CSMD_USHORT         usI, usK, usL;    /* counter variables */
  CSMD_USHORT         usListLength;
  CSMD_USHORT        *pusStartDataPtr = ((CSMD_USHORT *)pvSourceBinConfig + 2);
  CSMD_USHORT        *pusActDataPtr = pusStartDataPtr;
  CSMD_TABLE_POINTER  rTableHeaderPtr;
  CSMD_USHORT        *pusAuxPtr = NULL;
  CSMD_UCHAR          aucMasterInstManipulated[CSMD_MAX_CONNECTIONS_MASTER] = {FALSE};
  CSMD_UCHAR          aucConnManipulated[CSMD_MAX_GLOB_CONN]                = {FALSE};
  CSMD_BOOL           boCheckAddresses = TRUE;
  CSMD_USHORT         ausConfManipulated[CSMD_MAX_GLOB_CONFIG] = {0};
  CSMD_USHORT         ausRTBtManipulated[CSMD_MAX_RT_BIT_CONFIG] = {0};
  CSMD_USHORT         ausSetupParamsListManipulated[CSMD_MAX_CONFIGPARAMS_LIST] = {0};
  CSMD_USHORT         ausSetupParameterManipulated[CSMD_MAX_CONFIG_PARAMETER] = {0};
  CSMD_USHORT         usNumberOfElements = 0;
  CSMD_USHORT         usNumberOfConnections = 0;
  CSMD_USHORT         usNumberOfConsumers = 0;
  CSMD_USHORT         usNumberOfProducers = 0;

  /* Init table header pointer */
  rTableHeaderPtr.pusCnncStartPtr = NULL;
  rTableHeaderPtr.pusPrdcStartPtr = NULL;
  rTableHeaderPtr.pusConsLStartPtr = NULL;
  rTableHeaderPtr.pusConsTableStartPtr = NULL;
  rTableHeaderPtr.pusConfigTableStartPtr = NULL;
  rTableHeaderPtr.pusRTBitsTableStartPtr = NULL;
  rTableHeaderPtr.pusConnConfigEndPtr = NULL;
  rTableHeaderPtr.pusSlaveSetupStartPtr = NULL;
  rTableHeaderPtr.pusSetupParametersListStartPtr = NULL;
  rTableHeaderPtr.pusSetupParameterStartPtr = NULL;

  /* Init used configs */
#ifndef CSMD_STATIC_MEM_ALLOC
  (CSMD_VOID) CSMD_HAL_memset( prCSMD_Instance->rPriv.rUsedCfgs.paucConnNbrUsed,
                               0,
                               sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn );
  (CSMD_VOID) CSMD_HAL_memset( prCSMD_Instance->rPriv.rUsedCfgs.paucConnUsed,
                               0,
                               sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn );
  (CSMD_VOID) CSMD_HAL_memset( prCSMD_Instance->rPriv.rUsedCfgs.paucConfUsed,
                               0,
                               sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig );
  (CSMD_VOID) CSMD_HAL_memset( prCSMD_Instance->rPriv.rUsedCfgs.paucRTBtUsed,
                               0,
                               sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig );
  (CSMD_VOID) CSMD_HAL_memset( prCSMD_Instance->rPriv.rUsedCfgs.paucSetupParamsListUsed,
                               0,
                               sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList );
  (CSMD_VOID) CSMD_HAL_memset( prCSMD_Instance->rPriv.rUsedCfgs.paucSetupParamsUsed,
                               0,
                               sizeof(CSMD_UCHAR) * prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter );
#else
  (CSMD_VOID) CSMD_HAL_memset( &prCSMD_Instance->rPriv.rUsedCfgs,
                               0,
                               sizeof (CSMD_USED_MARKER) );
#endif
  
  /* Init manipulation markers */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
  {
    for (usK = 0; usK < CSMD_MAX_CONNECTIONS; usK++)
    {
      prCSMD_Instance->rPriv.parSlaveInst[usI].aucManipulated[usK] = FALSE;
    }
  }

  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams; usI++)
  {
    prCSMD_Instance->rPriv.paucSlaveSetupManipulated[usI] = FALSE;
  }

  /* Check whether the original CoSeMa structure is used or not */
  /* Check only one pointer should be sufficient */
  if (prConfig->prMaster_Config != &prConfig->rMasterCfg)
  {
    /* No CoSeMa structure -> switch off the address check */
    boCheckAddresses = FALSE;
  }

  
  /* Check the length */
  usListLength = CSMD_LENGTH_CONV(*((CSMD_USHORT *)pvSourceBinConfig));

  if (usListLength < CSMD_LIST_HEADER_LEN)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }
    /* Check the length; has to be LONG aligned */
  else if (usListLength & 0x3)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }
  else
  {
    usListLength -= CSMD_LIST_HEADER_LEN;
  }

  /* no error occurred so far? -> go on */
  if (eFuncRet == CSMD_NO_ERROR)
  {
    /* Check the header */
    eFuncRet = CSMD_CheckCbinC_Header(pusActDataPtr);
    
    /* Adjust the pointer */
    pusActDataPtr += ((CSMD_LIST_HEADER_LEN - CSMD_CBINC_VERSION_LEN) / 2);
  }

  /* no error occurred so far? -> go on */
  if (eFuncRet == CSMD_NO_ERROR)
  {
    /* Process the data format version specific */
    switch (CSMD_END_CONV_S(*pusActDataPtr++))
    {
    case BIN_CONFIG_VERSION_01_01:

      /* Search for the headers */
      eFuncRet = CSMD_SearchTableHeaders(pusActDataPtr, usListLength, &rTableHeaderPtr);
      
      if (eFuncRet == CSMD_NO_ERROR)
      {
        /* Number of producers is determined by the difference in the pointers */
        usNumberOfProducers = (CSMD_USHORT)
          (  ((CSMD_ULONG)(  (CSMD_UCHAR *)rTableHeaderPtr.pusConsLStartPtr
                           - (CSMD_UCHAR *)rTableHeaderPtr.pusPrdcStartPtr) - CSMD_TABLE_OVERHEAD)
           / CSMD_PRODUCER_TABLE_LEN);
        
        /* Number of consumers is determined by the difference in the pointers */
        usNumberOfConsumers = (CSMD_USHORT)
          (  ((CSMD_ULONG)(  (CSMD_UCHAR *)rTableHeaderPtr.pusConfigTableStartPtr
                           - (CSMD_UCHAR *)rTableHeaderPtr.pusConsTableStartPtr) - CSMD_TABLE_OVERHEAD)
           / CSMD_CONSUMER_TABLE_LEN);
        
        /* Number of connections is determined by the difference in the pointers */
        usNumberOfConnections = (CSMD_USHORT)
          (  ((CSMD_ULONG)(  (CSMD_UCHAR *)rTableHeaderPtr.pusPrdcStartPtr
                           - (CSMD_UCHAR *)rTableHeaderPtr.pusCnncStartPtr) - CSMD_TABLE_OVERHEAD)
           / sizeof(CSMD_CONNECTION_TABLE));

        /* Check all producer keys & consumer list keys of connection table */
        eFuncRet = CSMD_CheckConnTableKeys(&rTableHeaderPtr, usNumberOfConnections, usNumberOfProducers);
      }
      
      if ((eFuncRet == CSMD_NO_ERROR) && (boCheckAddresses))
      {
        /* Check all slave addresses in producer & consumer table */
        eFuncRet = CSMD_CheckTableSlaveAddr(&prCSMD_Instance->rSlaveList, &rTableHeaderPtr,
          usNumberOfProducers, usNumberOfConsumers);
      }

      /* Clear connections with given application IDs if no error occurred so far */
      /* or if a wrong format was detected clear the connections only if connections */
      /* table exists and no producer table is there */
      if (   (eFuncRet == CSMD_NO_ERROR)
          || (   (eFuncRet == CSMD_WRONG_BIN_CONFIG_FORMAT)
              && (rTableHeaderPtr.pusCnncStartPtr != NULL)
              && (rTableHeaderPtr.pusPrdcStartPtr == NULL)))
      {
        /* Clear all connections which have got the given application IDs */
        CSMD_FUNC_RET eFuncRetTmp;
        eFuncRetTmp = CSMD_ClearAppID_Connections( prCSMD_Instance,
                                                   (CSMD_CONNECTION_TABLE *)(CSMD_VOID *)(rTableHeaderPtr.pusCnncStartPtr + 2),
                                                   usNumberOfConnections,
                                                   boCheckAddresses);
        if (eFuncRet == CSMD_NO_ERROR)
        {
          eFuncRet = eFuncRetTmp;
        }
      }
      
      if ((eFuncRet == CSMD_NO_ERROR) && (rTableHeaderPtr.pusSlaveSetupStartPtr != NULL))
      {
        /* Clear all slave parameter configurations which have got the given application IDs */
        eFuncRet = CSMD_ClearAppID_SetupParams(prCSMD_Instance, (CSMD_PARAMSLIST_TABLE_HEADER *)
          (CSMD_VOID *)(rTableHeaderPtr.pusSetupParametersListStartPtr + 2),
          rTableHeaderPtr.pusSetupParameterStartPtr);
      }
      
      if ((eFuncRet == CSMD_NO_ERROR) && (rTableHeaderPtr.pusSlaveSetupStartPtr != NULL))
      {
        /* Check all configuration parameter table keys*/
        eFuncRet = CSMD_CheckParamConfTableKeys(&rTableHeaderPtr);
      }

      if (eFuncRet == CSMD_NO_ERROR)
      {
        /* Get the markers for the actual used configuration structures */
        eFuncRet = CSMD_GetUsedCfgStructs(prCSMD_Instance, &prCSMD_Instance->rPriv.rUsedCfgs, boCheckAddresses);
      }
      else
      {
        /* There was an error somewhere above -> leave the switch */
        break;
      }
      
      /***************************/
      /* copy all configurations */
      /***************************/

      /* set the help pointer to the beginning of the configurations data,
            behind the table header */
      pusAuxPtr = rTableHeaderPtr.pusConfigTableStartPtr + 2;

      /* Check all configurations; it is finished if the actual pointer is
      higher or equal the pointer for the RT bit configuration */

      while (   (pusAuxPtr < rTableHeaderPtr.pusRTBitsTableStartPtr - 2)
             && (eFuncRet == CSMD_NO_ERROR))
      {
        CSMD_CONFIG_TABLE_HEADER *prConfigHeader = (CSMD_CONFIG_TABLE_HEADER *)(CSMD_VOID *)pusAuxPtr;
        CSMD_USHORT               usConfIdx;
        CSMD_BOOL                 boFound = FALSE;

        /* Check if the number of IDNs exceeds the maximum number */
        if (CSMD_END_CONV_S(prConfigHeader->usNumberOfIDNS) > CSMD_MAX_IDN_PER_CONNECTION)
        {
          eFuncRet = CSMD_TOO_MANY_IDN_FOR_CONN;

          break;
        }

        /* Check if this configuration is needed, i.e used by a consumer or producer */
        if (   (CSMD_END_CONV_S(prConfigHeader->usConnectionSetup) & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK)
            == CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)
        {
          /* Search the key in the producer table */
          CSMD_PRODUCER_TABLE   *prProdTable = (CSMD_PRODUCER_TABLE *)(CSMD_VOID *)
                                    (((CSMD_CHAR *)rTableHeaderPtr.pusPrdcStartPtr) + CSMD_TABLE_HEADER_LEN);
          CSMD_CONNECTION_TABLE *prConnTable = (CSMD_CONNECTION_TABLE *)(CSMD_VOID *)
                                    (((CSMD_CHAR *)rTableHeaderPtr.pusCnncStartPtr) + CSMD_TABLE_HEADER_LEN);;

          for (usI = 0; usI < usNumberOfProducers; usI++)
          {
            /* get the actual producer table element */
            prProdTable = (CSMD_PRODUCER_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)rTableHeaderPtr.pusPrdcStartPtr)
                          + CSMD_TABLE_HEADER_LEN + (usI * CSMD_PRODUCER_TABLE_LEN));

            if (prProdTable->usConfigurationKey == prConfigHeader->usConfigKey)
            {
              boFound = TRUE;

              break;
            }
          }

          /* Found? */
          if (boFound)
          {
            boFound = FALSE;
            
            /* Is the producer key referenced by a connection? */
            for (usI = 0; usI < usNumberOfConnections; usI++)
            {
              /* get the actual connection */
              prConnTable = (CSMD_CONNECTION_TABLE *)(CSMD_VOID *)
                (((CSMD_CHAR *)rTableHeaderPtr.pusCnncStartPtr) + CSMD_TABLE_HEADER_LEN + (usI * CSMD_CONNECTION_TABLE_LEN));
              
              if (prProdTable->usProducerKey == prConnTable->usProducerKey)
              {
                boFound = TRUE;
                
                break;
              }
            }
          }
        }
        else if (   (CSMD_END_CONV_S(prConfigHeader->usConnectionSetup) & CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK)
                 == CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)
        {
          /* Search the key in the consumer table */
          CSMD_CONSUMER_TABLE        *prConsTable = (CSMD_CONSUMER_TABLE *)(CSMD_VOID *)
                                          (((CSMD_CHAR *)rTableHeaderPtr.pusConsTableStartPtr) + CSMD_TABLE_HEADER_LEN);
          CSMD_CONNECTION_TABLE      *prConnTable;
          CSMD_CONSLIST_TABLE_HEADER *prConsListTable = (CSMD_CONSLIST_TABLE_HEADER *)(CSMD_VOID *)(rTableHeaderPtr.pusConsLStartPtr + 2);
          CSMD_USHORT                *pusAuxPtrTemp = NULL;

          for (usI = 0; usI < usNumberOfConsumers; usI++)
          {
            /* get the actual consumer table element */
            prConsTable = (CSMD_CONSUMER_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)rTableHeaderPtr.pusConsTableStartPtr)
                          + CSMD_TABLE_HEADER_LEN + (usI * CSMD_CONSUMER_TABLE_LEN));
            
            if (prConsTable->usConfigurationKey == prConfigHeader->usConfigKey)
            {
              boFound = TRUE;

              break;
            }
          }

          /* Found? */
          if (boFound)
          {
            boFound = FALSE;
            
            /* Is the consumer key referenced by a consumer list? */
            pusAuxPtrTemp = rTableHeaderPtr.pusConsLStartPtr + 2;
            
            while((!boFound) && (pusAuxPtrTemp < rTableHeaderPtr.pusConsTableStartPtr))
            {
              /* get the actual consumer list table element */
              prConsListTable = (CSMD_CONSLIST_TABLE_HEADER *)(CSMD_VOID *)pusAuxPtrTemp;
              
              /* Is the first consumer the right one? */
              if (prConsListTable->usFirstConsumerKey == prConsTable->usConsumerKey)
              {
                boFound = TRUE;
                
                break;
              }
              else
              {
                CSMD_USHORT  usConsListConsumerNumber;
                CSMD_USHORT  usConsKeyFromList;
                
                /* get all consumers */
                for (usConsListConsumerNumber = 0;
                        (usConsListConsumerNumber < CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers))
                     && (eFuncRet == CSMD_NO_ERROR);
                     usConsListConsumerNumber++)
                {
                  /* Get the consumer key from he consumer list table */
                  usConsKeyFromList = *((CSMD_USHORT *)(&prConsListTable->usFirstConsumerKey) + usConsListConsumerNumber);
                  
                  if (usConsKeyFromList == prConsTable->usConsumerKey)
                  {
                    boFound = TRUE;
                    
                    break;
                  }
                }
              }
              
              /* Adjust the help pointer */
              if (CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers) & 0x1)
              {
                /* odd number of customers -> dummy */
                pusAuxPtrTemp += (CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers) + 3);
              }
              else
              {
                /* even number of customers -> no dummy */
                pusAuxPtrTemp += (CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers) + 2);
              }
            }
          }


          if (boFound)
          {
            boFound = FALSE;
            
            /* Is the consumer list key referenced by a connection? */
            for (usI = 0; usI < usNumberOfConnections; usI++)
            {
              /* get the actual connection */
              prConnTable = (CSMD_CONNECTION_TABLE *)(CSMD_VOID *)
                (((CSMD_CHAR *)rTableHeaderPtr.pusCnncStartPtr) + CSMD_TABLE_HEADER_LEN + (usI * CSMD_CONNECTION_TABLE_LEN));
              
              if (prConsListTable->usConsumerListKey == prConnTable->usConsumerListKey)
              {
                boFound = TRUE;
                
                break;
              }
            }
          }
        }

        /* set the aux pointer to the beginning of the IDNs */
        pusAuxPtr += 4;

        if (!boFound)
        {
          /* Adjust the pointer */
          pusAuxPtr += prConfigHeader->usNumberOfIDNS * 2;

          continue;
        }

        /* Search free configuration */
        for (usConfIdx = 0; usConfIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig; usConfIdx++)
        {
          if (prCSMD_Instance->rPriv.rUsedCfgs.paucConfUsed[usConfIdx] == FALSE)
          {
            /* Free configuration found */
            CSMD_ULONG *pulIDN_Ptr = (CSMD_ULONG *)(CSMD_VOID *)pusAuxPtr;

            /* copy the configuration data */
            prConfig->parConfiguration[usConfIdx].usS_0_1050_SE1 =
              CSMD_END_CONV_S(prConfigHeader->usConnectionSetup);
            prConfig->parConfiguration[usConfIdx].usS_0_1050_SE7 =
              CSMD_END_CONV_S(prConfigHeader->usConnectionCapability);

            /* copy the configured IDNs */
            for (usI = 0; usI < CSMD_END_CONV_S(prConfigHeader->usNumberOfIDNS); usI++)
            {
              prConfig->parConfiguration[usConfIdx].ulS_0_1050_SE6[usI + 1]
                = CSMD_END_CONV_L(pulIDN_Ptr[usI]);

              /* adjust the aux pointer */
              pusAuxPtr += 2;
            }

            /* write the length information; S1050SE6 is a Sercos list */
            *((CSMD_USHORT *)(CSMD_VOID *)(&prConfig->parConfiguration[usConfIdx].ulS_0_1050_SE6[0])) =
              (CSMD_USHORT)(CSMD_END_CONV_S(prConfigHeader->usNumberOfIDNS) * 4);
            *(((CSMD_USHORT *)(CSMD_VOID *)(&prConfig->parConfiguration[usConfIdx].ulS_0_1050_SE6[0])) + 1) =
              (CSMD_USHORT)((CSMD_MAX_IDN_PER_CONNECTION << 16) * 4);
            
 
            /* mark the configuration as used */
            prCSMD_Instance->rPriv.rUsedCfgs.paucConfUsed[usConfIdx] = TRUE;

            /* mark the configuration as manipulated with the configuration */
            ausConfManipulated[usConfIdx] = CSMD_END_CONV_S(prConfigHeader->usConfigKey);
            
            /* Check for key being 0 */
            if (prConfigHeader->usConfigKey == (CSMD_USHORT)0)
            {
              eFuncRet = CSMD_NO_CONFIGURATION_KEY;
            }
            
            break;
          }
        }

        /* Check for no free configuration found */
        if (usConfIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig)
        {
          /* Set error */
          eFuncRet = CSMD_TOO_MANY_CONFIGURATIONS;
        }
      }

      /*****************************************/
      /* copy all Real time Bit configurations */
      /*****************************************/

      /* Number of elements is determined by the difference in the pointers */
      if (rTableHeaderPtr.pusSlaveSetupStartPtr != NULL)
      {
        /* if a slave setup exists, take the pointer for that table */
        usNumberOfElements = (CSMD_USHORT)
          (  ((CSMD_ULONG)(  (CSMD_UCHAR *)rTableHeaderPtr.pusSlaveSetupStartPtr
                           - (CSMD_UCHAR *)rTableHeaderPtr.pusRTBitsTableStartPtr) - CSMD_TABLE_OVERHEAD)
           / sizeof(CSMD_RTBITS_TABLE));
      }
      else
      {
        /* No slave setup, the end pointer is the right one */
        usNumberOfElements = (CSMD_USHORT)
          (  ((CSMD_ULONG)(  (CSMD_UCHAR *)rTableHeaderPtr.pusConnConfigEndPtr
                           - (CSMD_UCHAR *)rTableHeaderPtr.pusRTBitsTableStartPtr) - CSMD_TABLE_OVERHEAD)
           / sizeof(CSMD_RTBITS_TABLE));
      }

      for (usI = 0; (usI < usNumberOfElements) && (eFuncRet == CSMD_NO_ERROR); usI++)
      {
        CSMD_RTBITS_TABLE *prRTBt_Table;
        CSMD_USHORT        usFreeRTBits;

          /* get the actual RT bit configuration */
        prRTBt_Table = (CSMD_RTBITS_TABLE *)(CSMD_VOID *)
          (((CSMD_CHAR *)rTableHeaderPtr.pusRTBitsTableStartPtr) + (usI * CSMD_RTBITS_TABLE_LEN) + CSMD_TABLE_HEADER_LEN);

        /* search free RT bit configuration */
        for (usFreeRTBits = 0; usFreeRTBits < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig; usFreeRTBits++)
        {
          if (prCSMD_Instance->rPriv.rUsedCfgs.paucRTBtUsed[usFreeRTBits] == FALSE)
          {
            /* free RT bit configuration found */

            /* copy */
            for (usK = 0; usK < CSMD_MAX_RT_BITS_PER_CONN;usK++)
            {
              /* IDN */
              prConfig->parRealTimeBit[usFreeRTBits].ulS_0_1050_SE20[usK + 1]
                = CSMD_END_CONV_L(prRTBt_Table->ulRTBit_IDN[usK]);

              /* Bit within IDN */
              prConfig->parRealTimeBit[usFreeRTBits].usS_0_1050_SE21[usK + 2]
                = CSMD_END_CONV_S(prRTBt_Table->usBit_in_IDN[usK]);
            }

            /* List length */
            prConfig->parRealTimeBit[usFreeRTBits].ulS_0_1050_SE20[0]
              = (CSMD_MAX_RT_BITS_PER_CONN * 4) + ((CSMD_MAX_RT_BITS_PER_CONN * 4) << 16);
            prConfig->parRealTimeBit[usFreeRTBits].usS_0_1050_SE21[0]
              = CSMD_MAX_RT_BITS_PER_CONN * 2;
            prConfig->parRealTimeBit[usFreeRTBits].usS_0_1050_SE21[1]
              = CSMD_MAX_RT_BITS_PER_CONN * 2;

            /* mark the RT bit configuration as used  */
            prCSMD_Instance->rPriv.rUsedCfgs.paucRTBtUsed[usFreeRTBits] = TRUE;

            /* mark the RT bit configuration as manipulated with the key */
            ausRTBtManipulated[usFreeRTBits] = CSMD_END_CONV_S(prRTBt_Table->usRTBitsKey);

            /* Check for key being 0 */
            if (prRTBt_Table->usRTBitsKey == (CSMD_USHORT)0)
            {
              eFuncRet = CSMD_NO_RTB_CONFIG_KEY;
            }

            break;
          }
        }

        /* Check for no free RT bit configuration found */
        if (usFreeRTBits == prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig)
        {
          /* Set error */
          eFuncRet = CSMD_TOO_MANY_RTB_CONFIG;
        }
      }

      /************************/
      /* Write connections    */
      /************************/

      /* process each connection */ 
      for (usI = (CSMD_USHORT) 0; (usI < usNumberOfConnections) && (eFuncRet == CSMD_NO_ERROR); usI++)
      {
        CSMD_PRODUCER_TABLE   *prProdTable;
        CSMD_CONSUMER_TABLE   *prConsTable;
        CSMD_CONNECTION_TABLE *prConnTable;
        CSMD_USHORT            usFreeConn;

        /* get the actual connection */
        prConnTable = (CSMD_CONNECTION_TABLE *)(CSMD_VOID *)
          (((CSMD_CHAR *)rTableHeaderPtr.pusCnncStartPtr) + CSMD_TABLE_HEADER_LEN + (usI * CSMD_CONNECTION_TABLE_LEN));

        /* check if the connection is really needed, that means if the       */
        /* producer and at least one consumer do exist and are not           */
        /* a member of the list of deactivated slaves; this check is only    */
        /* done if the address check is activated                            */

        if (boCheckAddresses)
        {
          CSMD_BOOL    boProdFound = FALSE;
          CSMD_BOOL    boConsFound = FALSE;
          CSMD_USHORT  usProdAddr;
          CSMD_USHORT  usConsAddr;
          CSMD_USHORT  usConsListConsumerNumber;
          CSMD_USHORT  usActConsumer;
          CSMD_USHORT  usConsKeyFromList;
          CSMD_CONSLIST_TABLE_HEADER *prConsListTable;

          pusAuxPtr = rTableHeaderPtr.pusConsLStartPtr + 2;
          
          /* Get the producer data */
          for (usK = 0; (usK < usNumberOfProducers); usK++)
          {
            /* get the actual producer table element */
            prProdTable = (CSMD_PRODUCER_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)rTableHeaderPtr.pusPrdcStartPtr)
              + CSMD_TABLE_HEADER_LEN + (usK * CSMD_PRODUCER_TABLE_LEN));
            
            /* Check if the producer key is the right one */
            /* No swapping necessary since both values are in the same endian format */
            if (prProdTable->usProducerKey == prConnTable->usProducerKey)
            {
              usProdAddr = CSMD_END_CONV_S(prProdTable->usSERCOS_Add);

              /* Check the producer */
              if (usProdAddr == 0)
              {
                /* it's the master */
                boProdFound = TRUE;

                break;
              }
              else
              {
                CSMD_USHORT  usProdIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usProdAddr];
                
                if ((usProdIdx != 0) ||
                    (prCSMD_Instance->rSlaveList.ausProjSlaveAddList[usProdIdx + 2] == usProdAddr))
                {
                  /* The slave exists */
                  boProdFound = TRUE;
                  
                  break;
                }
              }
              
            }
          }

          /* Not found? -> this connection is not needed, process next connection */
          if (!boProdFound)
          {
            continue;
          }

          /* Check the consumer */
          
          /* For loop for all consumer list elements */
          while((eFuncRet == CSMD_NO_ERROR) && (pusAuxPtr < rTableHeaderPtr.pusConsTableStartPtr))
          {
            /* get the actual consumer list table element */
            prConsListTable = (CSMD_CONSLIST_TABLE_HEADER *)(CSMD_VOID *)pusAuxPtr;
            
            /* Check if the consumer list key is the right one */
            /* No swapping necessary since both values are in the same endian format */
            if (prConsListTable->usConsumerListKey == prConnTable->usConsumerListKey)
            {
              /* get all consumers */
              for (usConsListConsumerNumber = 0;
                      (usConsListConsumerNumber < CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers))
                   && (eFuncRet == CSMD_NO_ERROR);
                   usConsListConsumerNumber++)
              {
                /* Get the consumer key from he consumer list table */
                usConsKeyFromList = 
                  CSMD_END_CONV_S(*((CSMD_USHORT *)(&prConsListTable->usFirstConsumerKey) + usConsListConsumerNumber));
                
                /* Check the key for 0 */
                if (usConsKeyFromList == 0)
                {
                  eFuncRet = CSMD_NO_CONSUMER_KEY;
                }
                
                for (usActConsumer = 0; (usActConsumer < usNumberOfConsumers) && (eFuncRet == CSMD_NO_ERROR); usActConsumer++)
                {
                  /* get the actual consumer table element */
                  prConsTable = (CSMD_CONSUMER_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)rTableHeaderPtr.pusConsTableStartPtr)
                    + CSMD_TABLE_HEADER_LEN + (usActConsumer * CSMD_CONSUMER_TABLE_LEN));
                  
                  /* Is this consumer in the consumer list? */
                  if (CSMD_END_CONV_S(prConsTable->usConsumerKey) == usConsKeyFromList)
                  {
                    /* Is the address in the ? */
                    usConsAddr = CSMD_END_CONV_S(prConsTable->usSERCOS_Add);
                    
                    /* Check the producer */
                    if (usConsAddr == 0)
                    {
                      /* it's the master */
                      boConsFound = TRUE;
                      
                      break;
                    }
                    else
                    {
                      CSMD_USHORT  usConsIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usConsAddr];
                      
                      if ((usConsIdx != 0) ||
                        (prCSMD_Instance->rSlaveList.ausProjSlaveAddList[usConsIdx + 2] == usConsAddr))
                      {
                        /* The slave exists */
                        boConsFound = TRUE;
                        
                        break;
                      }
                    }
                  }
                }
                
                if (boConsFound)
                {
                  break;
                }
              }

              /* Consumer list key was found -> no further search needed */
              break; /* while((eFuncRet = ... */
            }
            else
            {
              /* Adjust the help pointer */
              if (CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers) & 0x1)
              {
                /* odd number of customers -> dummy */
                pusAuxPtr += (CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers) + 3);
              }
              else
              {
                /* even number of customers -> no dummy */
                pusAuxPtr += (CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers) + 2);
              }
            }
          }
          
          /* Not found? -> this connection is not needed, process next connection */
          if (!boConsFound)
          {
            continue;
          }
        }

        /* search free connection */
        for (usFreeConn = 0; usFreeConn < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn; usFreeConn++)
        {
          if (prCSMD_Instance->rPriv.rUsedCfgs.paucConnUsed[usFreeConn] == FALSE)
          {
            /* free connection found */

            /* fill application ID ... */
            prConfig->parConnection[usFreeConn].usApplicationID 
              = CSMD_END_CONV_S(prConnTable->usApplicationID);

            /* and connection name */
            for (usK = 0; usK < CSMD_CONN_NAME_LENGTH;usK++)
            {
              prConfig->parConnection[usFreeConn].ucConnectionName[usK]
                = prConnTable->ucConnectionName[usK];
            }

            /* mark the connection as globally used */
            prCSMD_Instance->rPriv.rUsedCfgs.paucConnUsed[usFreeConn] = TRUE;
            
            /* ... and temporarily as manipulated */
            aucConnManipulated[usFreeConn] = TRUE;
            
            break;
          }
        }

        /* No free connection found? */
        if (usFreeConn == prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
        {
          /* no free connection available */
          eFuncRet = CSMD_TOO_MANY_CONNECTIONS;
          
          break;
        }

        /* write connection information */

        /* automatic connection number? */
        if (boConnNumGen)
        {
          /* search first free connection number */
          for (usK = 0; usK < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn; usK++)
          {
            if (prCSMD_Instance->rPriv.rUsedCfgs.paucConnNbrUsed[usK] == FALSE)
            {
              /* free connection number found */
              prConfig->parConnection[usFreeConn].usS_0_1050_SE2 = (CSMD_USHORT)(usK + 1);
              
              /* mark the number as used */
              prCSMD_Instance->rPriv.rUsedCfgs.paucConnNbrUsed[usK] = TRUE;

              break;
            }
          }
        }
        else
        {
          /* check if connection number is already in use */
          for (usK = 0; usK < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn; usK++)
          {
            if (prConfig->parConnection[usK].usS_0_1050_SE2 == CSMD_END_CONV_S(prConnTable->usConnectionNbr))
            {
              /* slave not available */
              eFuncRet = CSMD_CONN_NBR_ALREADY_USED;

              break;
            }
          }

          /* no error -> take the connection number */
          if (eFuncRet == CSMD_NO_ERROR)
          {
            prConfig->parConnection[usFreeConn].usS_0_1050_SE2 = 
              CSMD_END_CONV_S(prConnTable->usConnectionNbr);

          }
        }

        /* Get the producer data */
        for (usK = 0; (usK < usNumberOfProducers) && (eFuncRet == CSMD_NO_ERROR); usK++)
        {
          CSMD_USHORT    usConnInstIdx;
          CSMD_USHORT    usConfIdx;
          CSMD_USHORT    usRTBtIdx;

          /* get the actual producer table element */
          prProdTable = (CSMD_PRODUCER_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)rTableHeaderPtr.pusPrdcStartPtr)
            + CSMD_TABLE_HEADER_LEN + (usK * CSMD_PRODUCER_TABLE_LEN));

          /* Check if the producer key is the right one */
          /* No swapping necessary since both values are in the same endian format */
          if (prProdTable->usProducerKey == prConnTable->usProducerKey)
          {
            /* write cycle time */
            prConfig->parConnection[usFreeConn].ulS_0_1050_SE10 = 
              CSMD_END_CONV_L(prProdTable->ulConnectionCycleTime);

            /* write the connection index to the producer data */
            if (prProdTable->usSERCOS_Add == 0)
            {
              /* it's the master */
#ifdef MPC    /* Temporary workaround for CCD */
              /* write telegram type AT */
              prConfig->parConnection[usFreeConn].usTelegramType = CSMD_TELEGRAM_TYPE_AT;
#else
  #ifdef CSMD_MASTER_PRODUCE_IN_AT
              if (CSMD_END_CONV_S(prProdTable->usDummy) == CSMD_MASTER_PROD_AT)
              {
                /* write telegram type AT */
                prConfig->parConnection[usFreeConn].usTelegramType = CSMD_TELEGRAM_TYPE_AT;
              }
              else
  #endif
              {
                /* write telegram type MDT */
                prConfig->parConnection[usFreeConn].usTelegramType = CSMD_TELEGRAM_TYPE_MDT;
              }
#endif        /* OPT_CCD */
              /* automatic connection instance ?*/
              if (boSlaveInstGen)
              {
                /* search for a free instance */
                for (usConnInstIdx = 0; usConnInstIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usConnInstIdx++)
                {
                  if (prConfig->prMaster_Config->parConnIdxList[usConnInstIdx].usConnIdx == 0xFFFF)
                  {
                    /* free master instance found */
                    
                    /* mark the master instance temporarily as manipulated */
                    aucMasterInstManipulated[usConnInstIdx] = TRUE;

                    break;
                  }
                }
                  
                /* No free master instance found? */
                if (usConnInstIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster)
                {
                  /* no free connection available */
                  eFuncRet = CSMD_TOO_MANY_CONN_FOR_MASTER;
                  
                  break; /* for (usK = 0; (usK < usNumberOfConnections)... */
                }
              }
              else /* if (boSlaveInstGen) */
              {
                CSMD_USHORT  usTempConnInst = CSMD_END_CONV_S(prProdTable->usConnectionInstance);

                /* check if the connection instance fits into the allowed range */
                if ( usTempConnInst >= prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster)
                {
                  /* given instance too high */
                  eFuncRet = CSMD_CONN_INST_TOO_HIGH;
                  
                  break; /* for (usK = 0; (usK < usNumberOfConnections)... */
                }

                /* check if connection instance is already in use */
                if (prConfig->prMaster_Config->parConnIdxList[usTempConnInst].usConnIdx != 0xFFFF)
                {
                  /* instance not available */
                  eFuncRet = CSMD_CONN_INST_ALREADY_USED;
                  
                  break; /* for (usK = 0; (usK < usNumberOfConnections)... */
                }
                
                /* no error -> take the instance number */
                usConnInstIdx = usTempConnInst;
              }
              
              /* No error? -> go on */
              
              /* write the connection index */
              prConfig->prMaster_Config->parConnIdxList[usConnInstIdx].usConnIdx = usFreeConn;
              
              /* Increment number of connections */
              prConfig->prMaster_Config->usNbrOfConnections++;
              
              /*****************
              * Configurations *
              *****************/
              
              /* search for the configuration key */
              for (usConfIdx = 0; usConfIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig; usConfIdx++)
              {
                if (ausConfManipulated[usConfIdx] == CSMD_END_CONV_S(prProdTable->usConfigurationKey))
                {
                  /* put the index to the connection instance */
                  prConfig->prMaster_Config->parConnIdxList[usConnInstIdx].usConfigIdx = usConfIdx;
                  
                  break;
                }
              }
              
              /* configuration key not found or 0? */
              if ((usConfIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig) || (prProdTable->usConfigurationKey == 0))
              {
                /* configuration key not available */
                eFuncRet = CSMD_NO_CONFIGURATION_KEY;
                
                break;
              }
              
              /****************
              * RT bits       *
              ****************/
              
              /* Is a real time bit configuration specified? */
              if (CSMD_END_CONV_S(prProdTable->usRTBitsKey) != 0xFFFF)
              {
                /* search for the RT bits configuration key */
                for (usRTBtIdx = 0; usRTBtIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig; usRTBtIdx++)
                {
                  if (ausRTBtManipulated[usRTBtIdx] == CSMD_END_CONV_S(prProdTable->usRTBitsKey))
                  {
                    /* put the index to the connection instance */
                    prConfig->prMaster_Config->parConnIdxList[usConnInstIdx].usRTBitsIdx = usRTBtIdx;
                    
                    break;
                  }
                }
                
                /* RT bits configuration key not found or 0? */
                if ((usRTBtIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig) || (prProdTable->usRTBitsKey == 0))
                {
                  /* configuration key not available */
                  eFuncRet = CSMD_NO_RTB_CONFIG_KEY;
                  
                  break;
                }
              }
            }
            else
            {
              CSMD_USHORT  usSlaveIndex;
              CSMD_USHORT  usSercAddress;
              /* it's a slave */

              usSercAddress = CSMD_END_CONV_S(prProdTable->usSERCOS_Add);

              /* write telegram type */
              prConfig->parConnection[usFreeConn].usTelegramType = CSMD_TELEGRAM_TYPE_AT;

              /* Get the slave index */
              if (boCheckAddresses)
              {
                /* The CoSeMa slave list can be used */
                usSlaveIndex = 
                  prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSercAddress];
              }
              else
              {
                /* The CoSeMa slave list is not yet filled and thus can't be used */

                /* Was the address used before? */
                if (prCSMD_Instance->rSlaveList.ausParserTempIdxList[usSercAddress] == 0)
                {
                  /* Not used before, maybe index is 0? */
                  if (prCSMD_Instance->rSlaveList.ausParserTempAddList[2] != usSercAddress)
                  {
                    /* Set the index */
                    usSlaveIndex = (CSMD_USHORT)(prCSMD_Instance->rSlaveList.ausParserTempAddList[0] / 2);
                    prCSMD_Instance->rSlaveList.ausParserTempIdxList[usSercAddress] = usSlaveIndex;                    
                    
                    /* Increment the number of configured slaves */
                    prCSMD_Instance->rSlaveList.ausParserTempAddList[0] +=2;
                    prCSMD_Instance->rSlaveList.ausParserTempAddList[1] +=2;
                    
                    /* Set the address */
                    prCSMD_Instance->rSlaveList.ausParserTempAddList[usSlaveIndex + 2] = usSercAddress;
                  }
                  else
                  {
                    usSlaveIndex = 0;
                  }
                }
                else
                {
                  usSlaveIndex =
                    prCSMD_Instance->rSlaveList.ausParserTempIdxList[usSercAddress];
                }
              }

              /* automatic connection instance ?*/
              if (boSlaveInstGen)
              {
                /* search for a free instance */
                for (usConnInstIdx = 0; usConnInstIdx < CSMD_MAX_CONNECTIONS; usConnInstIdx++)
                {
                  if (prConfig->parSlave_Config[usSlaveIndex].arConnIdxList[usConnInstIdx].usConnIdx == 0xFFFF)
                  {
                    /* free slave instance found */
                    
                    /* mark the slave instance temporarily as manipulated */
                    prCSMD_Instance->rPriv.parSlaveInst[usSlaveIndex].aucManipulated[usConnInstIdx] = TRUE;
                    
                    break;
                  }
                }
                  
                /* No free slave instance found? */
                if (usConnInstIdx == CSMD_MAX_CONNECTIONS)
                {
                  /* no free connection available */
                  eFuncRet = CSMD_TOO_MANY_CONN_FOR_SLAVE;
                  
                  break; /* for (usK = 0; (usK < usNumberOfConnections)... */
                }
              }                    
              else /* if (boSlaveInstGen) */
              {
                CSMD_USHORT  usTempConnInst = CSMD_END_CONV_S(prProdTable->usConnectionInstance);

                /* check if the connection instance fits into the allowed range */
                if (usTempConnInst >= CSMD_MAX_CONNECTIONS)
                {
                  /* given instance too high */
                  eFuncRet = CSMD_CONN_INST_TOO_HIGH;
                  
                  break; /* for (usK = 0; (usK < usNumberOfConnections)... */
                }

                /* check if connection instance is already in use */
                if (prConfig->parSlave_Config[usSlaveIndex].arConnIdxList[usTempConnInst].usConnIdx != 0xFFFF)
                {
                  /* instance not available */
                  eFuncRet = CSMD_CONN_INST_ALREADY_USED;
                  
                  break; /* for (usK = 0; (usK < usNumberOfConnections)... */
                }
                
                /* no error -> take the instance number */
                usConnInstIdx = usTempConnInst;
              }
              
              /* No error? -> go on */
              
              /* write the connection index */
              prConfig->parSlave_Config[usSlaveIndex].arConnIdxList[usConnInstIdx].usConnIdx = usFreeConn;
              
              /* Increment number of connections */
              prConfig->parSlave_Config[usSlaveIndex].usNbrOfConnections++;
              
              
              /*****************
              * Configurations *
              *****************/
              
              /* search for the configuration key */
              for (usConfIdx = 0; usConfIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig; usConfIdx++)
              {
                if (ausConfManipulated[usConfIdx] == CSMD_END_CONV_S(prProdTable->usConfigurationKey))
                {
                  /* put the index to the connection instance */
                  prConfig->parSlave_Config[usSlaveIndex].arConnIdxList[usConnInstIdx].usConfigIdx = usConfIdx;
                  
                  break;
                }
              }
              
              /* configuration key not found or 0? */
              if ((usConfIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig) || (prProdTable->usConfigurationKey == 0))
              {
                /* configuration key not available */
                eFuncRet = CSMD_NO_CONFIGURATION_KEY;
                
                break;
              }
              
              /****************
              * RT bits       *
              ****************/
              
              /* Is a real time bit configuration specified? */
              if (CSMD_END_CONV_S(prProdTable->usRTBitsKey) != 0xFFFF)
              {
                /* search for the RT bits configuration key */
                for (usRTBtIdx = 0; usRTBtIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig; usRTBtIdx++)
                {
                  if (ausRTBtManipulated[usRTBtIdx] == CSMD_END_CONV_S(prProdTable->usRTBitsKey))
                  {
                    /* put the index to the connection instance */
                    prConfig->parSlave_Config[usSlaveIndex].arConnIdxList[usConnInstIdx].usRTBitsIdx = usRTBtIdx;
                    
                    break;
                  }
                }
                
                /* RT bits configuration key not found or 0? */
                if ((usRTBtIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig) || (prProdTable->usRTBitsKey == 0))
                {
                  /* configuration key not available */
                  eFuncRet = CSMD_NO_RTB_CONFIG_KEY;
                  
                  break;
                }
              }
            } /* else "if (prProdTable->usSERCOS_Add == 0)"*/
          }
        } /* end for "Get the producer data" */
        
        /* Get the consumer data */
        
        /* set the help pointer to the beginning of the consumer list data,
        behind the table header; needed because structure of consumer list 
        table is dynamic */
        pusAuxPtr = rTableHeaderPtr.pusConsLStartPtr + 2;
        
        /* Loop for all consumer list elements */
        while((eFuncRet == CSMD_NO_ERROR) && (pusAuxPtr < rTableHeaderPtr.pusConsTableStartPtr))
        {
          CSMD_USHORT    usConsListConsumerNumber;
          CSMD_USHORT    usActConsumer;
          CSMD_USHORT    usConsKeyFromList;
          CSMD_USHORT    usConnInstIdx;
          CSMD_USHORT    usConfIdx;
          CSMD_USHORT    usRTBtIdx;
          CSMD_CONSLIST_TABLE_HEADER *prConsListTable;
          
          /* get the actual consumer table element */
          prConsListTable = (CSMD_CONSLIST_TABLE_HEADER *)(CSMD_VOID *)pusAuxPtr;
          
          /* Check if the consumer list key is the right one */
          /* No swapping necessary since both values are in the same endian format */
          if (prConsListTable->usConsumerListKey == prConnTable->usConsumerListKey)
          {
            /* get all consumers */
            for (usConsListConsumerNumber = 0;
                    (usConsListConsumerNumber < CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers))
                 && (eFuncRet == CSMD_NO_ERROR);
                 usConsListConsumerNumber++)
            {
              /* Get the consumer key from he consumer list table */
              usConsKeyFromList = 
                CSMD_END_CONV_S(*((CSMD_USHORT *)(&prConsListTable->usFirstConsumerKey) + usConsListConsumerNumber));

              /* Check the key for 0 */
              if (usConsKeyFromList == 0)
              {
                eFuncRet = CSMD_NO_CONSUMER_KEY;
              }
              
              for (usActConsumer = 0; (usActConsumer < usNumberOfConsumers) && (eFuncRet == CSMD_NO_ERROR); usActConsumer++)
              {
                /* get the actual consumer table element */
                prConsTable = (CSMD_CONSUMER_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)rTableHeaderPtr.pusConsTableStartPtr)
                  + CSMD_TABLE_HEADER_LEN + (usActConsumer * CSMD_CONSUMER_TABLE_LEN));
                
                /* Is this consumer in the consumer list? */
                if (CSMD_END_CONV_S(prConsTable->usConsumerKey) == usConsKeyFromList)
                {
                  /* check cycle time */
                  if (   prConfig->parConnection[usFreeConn].ulS_0_1050_SE10
                      != CSMD_END_CONV_L(prConsTable->ulConnectionCycleTime))
                  {
                    /* Cycle time not the same as the producer has got -> error */
                    eFuncRet = CSMD_CYCLE_TIME_UNEQUAL;
                    
                    break;
                  }

                  /* Write the allowed data losses to the connection data */
                  prConfig->parConnection[usFreeConn].usS_0_1050_SE11 = CSMD_END_CONV_S(prConsTable->usAllowedDataLosses);
                  
                  /* write the connection index to the consumer data */
                  if (prConsTable->usSERCOS_Add == 0)
                  {
                    /* it's the master */
                    
                    /* automatic connection instance ?*/
                    if (boSlaveInstGen)
                    {
                      /* search for a free instance */
                      for (usConnInstIdx = 0; usConnInstIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usConnInstIdx++)
                      {
                        if (prConfig->prMaster_Config->parConnIdxList[usConnInstIdx].usConnIdx == 0xFFFF)
                        {
                          /* free master instance found */
                          
                          /* mark the master instance temporarily as manipulated */
                          aucMasterInstManipulated[usConnInstIdx] = TRUE;
                          
                          break;
                        }
                      }
                      
                      /* No free master instance found? */
                      if (usConnInstIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster)
                      {
                        /* no free connection available */
                        eFuncRet = CSMD_TOO_MANY_CONN_FOR_MASTER;
                        
                        break; /* for (usActConsumer = 0; (usActConsumer < usNumberOfConsumers)... */
                      }
                    }
                    else /* if (boSlaveInstGen) */
                    {
                      CSMD_USHORT  usTempConnInst = CSMD_END_CONV_S(prConsTable->usConnectionInstance);

                      /* check if the connection instance fits into the allowed range */
                      if (usTempConnInst >= prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster)
                      {
                        /* given instance too high */
                        eFuncRet = CSMD_CONN_INST_TOO_HIGH;
                        
                        break; /* for (usK = 0; (usK < usNumberOfConnections)... */
                      }
                      
                      /* check if connection instance is already in use */
                      if (prConfig->prMaster_Config->parConnIdxList[usTempConnInst].usConnIdx != 0xFFFF)
                      {
                        /* instance not available */
                        eFuncRet = CSMD_CONN_INST_ALREADY_USED;
                        
                        break; /* for (usActConsumer = 0; (usActConsumer < usNumberOfConsumers)... */
                      }
                      
                      /* no error -> take the instance number */
                      usConnInstIdx = usTempConnInst;
                    }
                    
                    /* No error? -> go on */
                    
                    /* write the connection index */
                    prConfig->prMaster_Config->parConnIdxList[usConnInstIdx].usConnIdx = usFreeConn;
                    
                    /* Increment number of connections */
                    prConfig->prMaster_Config->usNbrOfConnections++;
                    
                    /*****************
                    * Configurations *
                    *****************/
                    
                    /* search for the configuration key */
                    for (usConfIdx = 0; usConfIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig; usConfIdx++)
                    {
                      if (ausConfManipulated[usConfIdx] == CSMD_END_CONV_S(prConsTable->usConfigurationKey))
                      {
                        /* put the index to the connection instance */
                        prConfig->prMaster_Config->parConnIdxList[usConnInstIdx].usConfigIdx = usConfIdx;
                        
                        break;
                      }
                    }
                    
                    /* configuration key not found or 0? */
                    if ((usConfIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig) || (prConsTable->usConfigurationKey == 0))
                    {
                      /* configuration key not available */
                      eFuncRet = CSMD_NO_CONFIGURATION_KEY;
                      
                      break;
                    }
                    
                    /****************
                    * RT bits       *
                    ****************/
                    
                    /* Is a real time bit configuration specified? */
                    if (CSMD_END_CONV_S(prConsTable->usRTBitsKey) != 0xFFFF)
                    {
                      /* search for the RT bits configuration key */
                      for (usRTBtIdx = 0; usRTBtIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig; usRTBtIdx++)
                      {
                        if (ausRTBtManipulated[usRTBtIdx] == CSMD_END_CONV_S(prConsTable->usRTBitsKey))
                        {
                          /* put the index to the connection instance */
                          prConfig->prMaster_Config->parConnIdxList[usConnInstIdx].usRTBitsIdx = usRTBtIdx;
                          
                          break;
                        }
                      }
                      
                      /* RT bits configuration key not found or 0? */
                      if ((usRTBtIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig) || (prConsTable->usRTBitsKey == 0))
                      {
                        /* configuration key not available */
                        eFuncRet = CSMD_NO_RTB_CONFIG_KEY;
                        
                        break;
                      }
                    }
                  }
                  else
                  {
                    CSMD_USHORT  usSlaveIndex;
                    CSMD_USHORT  usSercAddress;
                    /* it's a slave */

                    usSercAddress = CSMD_END_CONV_S(prConsTable->usSERCOS_Add);
                    
                    /* Get the slave index */
                    if (boCheckAddresses)
                    {
                      /* The CoSeMa slave list can be used */
                      usSlaveIndex = 
                        prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSercAddress];
                    }
                    else
                    {
                      /* The CoSeMa slave list is not yet filled and thus can't be used */
                      
                      /* Was the address used before? */
                      if (prCSMD_Instance->rSlaveList.ausParserTempIdxList[usSercAddress] == 0)
                      {
                        /* Not used before, maybe index is 0? */
                        if (prCSMD_Instance->rSlaveList.ausParserTempAddList[2] != usSercAddress)
                        {
                          /* Set the index */
                          usSlaveIndex = (CSMD_USHORT)(prCSMD_Instance->rSlaveList.ausParserTempAddList[0] / 2);
                          prCSMD_Instance->rSlaveList.ausParserTempIdxList[usSercAddress] = usSlaveIndex;                    
                          
                          /* Increment the number of configured slaves */
                          prCSMD_Instance->rSlaveList.ausParserTempAddList[0] +=2;
                          prCSMD_Instance->rSlaveList.ausParserTempAddList[1] +=2;
                          
                          /* Set the address */
                          prCSMD_Instance->rSlaveList.ausParserTempAddList[usSlaveIndex + 2] = usSercAddress;
                        }
                        else
                        {
                          usSlaveIndex = 0;
                        }
                      }
                      else
                      {
                        usSlaveIndex =
                          prCSMD_Instance->rSlaveList.ausParserTempIdxList[usSercAddress];
                      }
                    }


                    /* automatic connection instance ?*/
                    if (boSlaveInstGen)
                    {
                      /* search for a free instance */
                      for (usConnInstIdx = 0; usConnInstIdx < CSMD_MAX_CONNECTIONS; usConnInstIdx++)
                      {
                        if (prConfig->parSlave_Config[usSlaveIndex].arConnIdxList[usConnInstIdx].usConnIdx == 0xFFFF)
                        {
                          /* free slave instance found */
                          
                          /* mark the slave instance temporarily as manipulated */
                          prCSMD_Instance->rPriv.parSlaveInst[usSlaveIndex].aucManipulated[usConnInstIdx] = TRUE;
                          
                          break;
                        }
                      }
                      
                      /* No free slave instance found? */
                      if (usConnInstIdx == CSMD_MAX_CONNECTIONS)
                      {
                        /* no free connection available */
                        eFuncRet = CSMD_TOO_MANY_CONN_FOR_SLAVE;
                        
                        break; /* for (usActConsumer = 0; (usActConsumer < usNumberOfConsumers)... */
                      }
                    }                    
                    else /* if (boSlaveInstGen) */
                    {
                      CSMD_USHORT  usTempConnInst = CSMD_END_CONV_S(prConsTable->usConnectionInstance);

                      /* check if the connection instance fits into the allowed range */
                      if (usTempConnInst >= CSMD_MAX_CONNECTIONS)
                      {
                        /* given instance too high */
                        eFuncRet = CSMD_CONN_INST_TOO_HIGH;
                        
                        break; /* for (usK = 0; (usK < usNumberOfConnections)... */
                      }
                      
                      /* check if connection instance is already in use */
                      if (prConfig->parSlave_Config[usSlaveIndex].arConnIdxList[usTempConnInst].usConnIdx != 0xFFFF)
                      {
                        /* instance not available */
                        eFuncRet = CSMD_CONN_INST_ALREADY_USED;
                        
                        break; /* for (usActConsumer = 0; (usActConsumer < usNumberOfConsumers)... */
                      }
                      
                      /* no error -> take the instance number */
                      usConnInstIdx = usTempConnInst;
                    }

                    /* No error? -> go on */
                    
                    /* write the connection index */
                    prConfig->parSlave_Config[usSlaveIndex].arConnIdxList[usConnInstIdx].usConnIdx = usFreeConn;
                    
                    /* Increment number of connections */
                    prConfig->parSlave_Config[usSlaveIndex].usNbrOfConnections++;
                    
                    /*****************
                    * Configurations *
                    *****************/
                    
                    /* search for the configuration key */
                    for (usConfIdx = 0; usConfIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig; usConfIdx++)
                    {
                      if (ausConfManipulated[usConfIdx] == CSMD_END_CONV_S(prConsTable->usConfigurationKey))
                      {
                        /* put the index to the connection instance */
                        prConfig->parSlave_Config[usSlaveIndex].arConnIdxList[usConnInstIdx].usConfigIdx = usConfIdx;
                        
                        break;
                      }
                    }
                    
                    /* configuration key not found or 0? */
                    if ((usConfIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig) || (prConsTable->usConfigurationKey == 0))
                    {
                      /* configuration key not available */
                      eFuncRet = CSMD_NO_CONFIGURATION_KEY;
                      
                      break;
                    }
                    
                    /****************
                    * RT bits       *
                    ****************/
                    
                    /* Is a real time bit configuration specified? */
                    if (CSMD_END_CONV_S(prConsTable->usRTBitsKey) != 0xFFFF)
                    {
                      /* search for the RT bits configuration key */
                      for (usRTBtIdx = 0; usRTBtIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig; usRTBtIdx++)
                      {
                        if (ausRTBtManipulated[usRTBtIdx] == CSMD_END_CONV_S(prConsTable->usRTBitsKey))
                        {
                          /* put the index to the connection instance */
                          prConfig->parSlave_Config[usSlaveIndex].arConnIdxList[usConnInstIdx].usRTBitsIdx = usRTBtIdx;
                          
                          break;
                        }
                      }
                      
                      /* RT bits configuration key not found or 0? */
                      if ((usRTBtIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig) || (prConsTable->usRTBitsKey == 0))
                      {
                        /* configuration key not available */
                        eFuncRet = CSMD_NO_RTB_CONFIG_KEY;
                        
                        break;
                      }
                    }
                  } /* else "if (prConsTable->usSERCOS_Add == 0)" */
                  
                  break;
                }/* if (prConsTable->usConsumerKey == usConsKeyFromList) */
              } /* for (usActConsumer ... */

              /* No consumer key found? */
              if (usActConsumer == usNumberOfConsumers)
              {
                /* consumer key not available */
                eFuncRet = CSMD_NO_CONSUMER_KEY;
                
                break;
              }
            } /* for (usConsListConsumerNumber ... */
            break;
          } /* if (rConsListTable.usConsumerListKey == prConnTable->usConsumerListKey) */
          else
          {
            /* Adjust the help pointer */
            if (CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers) & 0x1)
            {
              /* odd number of consumers -> dummy */
              pusAuxPtr += (CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers) + 3);
            }
            else
            {
              /* even number of consumers -> no dummy */
              pusAuxPtr += (CSMD_END_CONV_S(prConsListTable->usNumberOfConsumers) + 2);
            }
          }

          /* No consumer key found? */
          if (pusAuxPtr >= rTableHeaderPtr.pusConsTableStartPtr)
          {
            /* consumer key not available */
            eFuncRet = CSMD_NO_CONSUMER_LIST_KEY;
            
            break;
          }

        } /* end for "Get the consumer data" */
      } /* end for "process each connection" */

      /* process the slave setup, if existing */
      if (rTableHeaderPtr.pusSlaveSetupStartPtr != NULL)
      {
        CSMD_USHORT    usNumberOfSlaveSetupTables;
        CSMD_USHORT    usFreeSetupList;
        CSMD_USHORT    ausSPL_Key[CSMD_MAX_CONFIGPARAMS_LIST] = {0};
        
        usNumberOfSlaveSetupTables = (CSMD_USHORT)
          (  ((CSMD_ULONG)(  (CSMD_UCHAR *)rTableHeaderPtr.pusSetupParametersListStartPtr
                           - (CSMD_UCHAR *)rTableHeaderPtr.pusSlaveSetupStartPtr) - CSMD_TABLE_OVERHEAD)
           / CSMD_SLAVE_SETUP_TABLE_LEN);
        
        for (usI = 0; (usI < usNumberOfSlaveSetupTables) && (eFuncRet == CSMD_NO_ERROR); usI++)
        {
          /* get the actual slave */
          CSMD_SLAVE_SETUP_TABLE *prSlaveSetupTable;
          CSMD_USHORT             usSercAddress;
          CSMD_USHORT             usSlaveIndex;
          CSMD_USHORT             usSlaveSetupIdx;
          CSMD_USHORT             usSlaveSetupKey;
          CSMD_BOOL               boFound = FALSE;
          CSMD_USHORT             usKeyMarkedNbr = prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList;
          
          /* get the actual slave setup table element */
          prSlaveSetupTable = (CSMD_SLAVE_SETUP_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)rTableHeaderPtr.pusSlaveSetupStartPtr)
            + CSMD_TABLE_HEADER_LEN + (usI * CSMD_SLAVE_SETUP_TABLE_LEN));
          
          /* Get the setup list key */
          usSlaveSetupKey = CSMD_END_CONV_S(prSlaveSetupTable->usSetupParamsListKey);

          /* Check all former processed keys */
          for (usK = 0; usK < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList; usK++)
          {
            /* Was the key 0 -> not processed, mark as processed, stop searching */
            if (ausSPL_Key[usK] == 0)
            {
              ausSPL_Key[usK] = usSlaveSetupKey;

              /* Keep the number to make a later remove of the marker possible */
              usKeyMarkedNbr = usK;

              break;
            }

            /* Was this the former processed key? */
            if (ausSPL_Key[usK] == usSlaveSetupKey)
            {
              boFound = TRUE;

              break;
            }
          }

          /* Was the setup list already processed? */
          if (boFound)
          {
            /* Go on with the next slave */
            continue;
          }

          /* Get Sercos address */
          usSercAddress = CSMD_END_CONV_S(prSlaveSetupTable->usSlaveAddress);

          /* Check for the master */
          if (usSercAddress == 0)
          {
            usSlaveIndex = prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves;
          }
          else
          {
            /* Get the slave index */
            if (boCheckAddresses)
            {
              /* The CoSeMa slave list can be used */
              usSlaveIndex = 
                prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSercAddress];
              
              /* Check the address*/
              if (usSlaveIndex == 0)
              {
                /* Index is 0 -> slave not available or it has got index 0 */
                if (prCSMD_Instance->rSlaveList.ausProjSlaveAddList[2] != usSercAddress)
                {
                  /* slave not available */
                  CSMD_BOOL boNext = FALSE;
                  
                  /* Slave deactivated? */
                  for (usK = 0; usK < prCSMD_Instance->rSlaveList.ausDeactSlaveAddList[0] / 2; usK++)
                  {
                    if (prCSMD_Instance->rSlaveList.ausDeactSlaveAddList[usK + 2] == usSercAddress)
                    {
                      /* Slave is deactivated -> stop further search */
                      boNext = TRUE;
                      
                      break;
                    }
                  }
                  
                  if (usK == (CSMD_USHORT)(prCSMD_Instance->rSlaveList.ausDeactSlaveAddList[0] / 2))
                  {
                    /* Slave not available */
                    eFuncRet = CSMD_WRONG_SLAVE_ADDRESS;
                    
                    boNext = TRUE;
                    
                    break;
                  }
                  
                  /* Process the next table entry, and remove marker if necessary */
                  if (boNext)
                  {
                    if (usKeyMarkedNbr != prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList)
                    {
                      ausSPL_Key[usKeyMarkedNbr] = 0;
                    }
                    
                    continue;
                  }
                }
              }
            }
            else
            {
              /* The CoSeMa slave list is not yet filled and thus can't be used */
              
              /* Was the address used before? */
              if (prCSMD_Instance->rSlaveList.ausParserTempIdxList[usSercAddress] == 0)
              {
                /* Not used before, maybe index is 0? */
                if (prCSMD_Instance->rSlaveList.ausParserTempAddList[2] != usSercAddress)
                {
                  /* Set the index */
                  usSlaveIndex = (CSMD_USHORT)(prCSMD_Instance->rSlaveList.ausParserTempAddList[0] / 2);
                  prCSMD_Instance->rSlaveList.ausParserTempIdxList[usSercAddress] = usSlaveIndex;                    
                  
                  /* Increment the number of configured slaves */
                  prCSMD_Instance->rSlaveList.ausParserTempAddList[0] +=2;
                  prCSMD_Instance->rSlaveList.ausParserTempAddList[1] +=2;
                  
                  /* Set the address */
                  prCSMD_Instance->rSlaveList.ausParserTempAddList[usSlaveIndex + 2] = usSercAddress;
                }
                else
                {
                  usSlaveIndex = 0;
                }
              }
              else
              {
                usSlaveIndex =
                  prCSMD_Instance->rSlaveList.ausParserTempIdxList[usSercAddress];
              }
            }
          }

          /* Search a free setup index */
          for (usSlaveSetupIdx = 0; usSlaveSetupIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams; usSlaveSetupIdx++)
          {
            if (prConfig->parSlaveParamConfig[usSlaveSetupIdx].usConfigParamsList_Index == 0xFFFF)
            {
              /* free slave setup found */
              
              /* mark the slave setup temporarily as manipulated */
              prCSMD_Instance->rPriv.paucSlaveSetupManipulated[usSlaveSetupIdx] = TRUE;
              
              break;
            }
          }

          if (usSlaveSetupIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams)
          {
            /* no free slave setup available */
            eFuncRet = CSMD_TOO_MANY_SLAVE_SETUP;
            
            break; /* for (usI = 0; (usI < usNumberOfSlaveSetupTables)... */
          }

          /* Search for a free setup parameters list */
          for (usFreeSetupList = 0; usFreeSetupList < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList; usFreeSetupList++)
          {
            if (prConfig->parConfigParamsList[usFreeSetupList].usApplicationID == 0)
            {
              /* free setup list found */
              CSMD_PARAMSLIST_TABLE_HEADER *prAuxParamsListPtr =
                (CSMD_PARAMSLIST_TABLE_HEADER *)(CSMD_VOID *)(rTableHeaderPtr.pusSetupParametersListStartPtr + 2);
              
              /* mark the setup list temporarily as manipulated */
              ausSetupParamsListManipulated[usFreeSetupList] = TRUE;

              /* Search the setup list */
              while(  ((CSMD_UCHAR *)prAuxParamsListPtr)
                    < ((CSMD_UCHAR *)rTableHeaderPtr.pusSetupParameterStartPtr - CSMD_END_SIGN_LENGTH))
              {
                if (CSMD_END_CONV_S(prAuxParamsListPtr->usSetupParamsListKey) == usSlaveSetupKey)
                {
                  /* Setup parameter list key was found */
                  CSMD_USHORT  usNumberOfConfParams = CSMD_END_CONV_S(prAuxParamsListPtr->usNumberOfConfigParams);

                  /* Check the number of configuration parameters */
                  if (usNumberOfConfParams > CSMD_MAX_PARAMS_IN_CONFIG_LIST)
                  {
                    /* too many parameters configured in the list */
                    eFuncRet = CSMD_TOO_MANY_PARAMETER_IN_LIST;
                    
                    break; /* while(... */
                  }

                  /* write the application ID */
                  prConfig->parConfigParamsList[usFreeSetupList].usApplicationID =
                    CSMD_END_CONV_S(prAuxParamsListPtr->usParamsApplicationID);
                  
                  /* Check all parameters in the list */
                  for (usK = 0; (usK < usNumberOfConfParams) && (eFuncRet == CSMD_NO_ERROR); usK++)
                  {
                    CSMD_USHORT *pusActParamKey = (CSMD_USHORT *)((&prAuxParamsListPtr->usFirstParameterKey) + usK);

                    /* Process the parameter */

                    /* Search the parameter key */
                    for (usL = 0; usL < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter; usL++)
                    {
                      /* Compare the actual key with the already written keys */
                      if (ausSetupParameterManipulated[usL] == CSMD_END_CONV_S(*pusActParamKey))
                      {
                        break;
                      }
                    }

                    /* Parameter key already processed? */
                    if (usL != prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter)
                    {
                      /* parameter key was used before */

                      /* write the parameter reference */
                      prConfig->parConfigParamsList[usFreeSetupList].ausParamTableIndex[usK] = usL;
                    }
                    else
                    {
                      CSMD_USHORT  usFreeSetupParam;

                      /* Search free setup parameter */
                      for (usFreeSetupParam = (CSMD_USHORT) 0;
                           (usFreeSetupParam < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter) && (eFuncRet == CSMD_NO_ERROR);
                           usFreeSetupParam++)
                      {
                        if (prConfig->parConfigParam[usFreeSetupParam].usDataLength == 0)
                        {
                          /* Free parameter found */

                          CSMD_PARAMETER_TABLE_HEADER *prAuxParamPtr =
                            (CSMD_PARAMETER_TABLE_HEADER *)(CSMD_VOID *)(rTableHeaderPtr.pusSetupParameterStartPtr + 2);
                          
                          /* Write the original key into the used array to find the key later */
                          ausSetupParameterManipulated[usFreeSetupParam]= CSMD_END_CONV_S(*pusActParamKey);

                          /* write the parameter reference */
                          prConfig->parConfigParamsList[usFreeSetupList].ausParamTableIndex[usK] = usFreeSetupParam;

                          /* Search the parameter inside the table */
                          while(  ((CSMD_UCHAR *)prAuxParamPtr)
                                < ((CSMD_UCHAR *)rTableHeaderPtr.pusConnConfigEndPtr - CSMD_END_SIGN_LENGTH))
                          {
                            /* Compare the keys */
                            /* Both values in same endianness, so no endian swapping necessary */
                            if (prAuxParamPtr->usParameterKey == *pusActParamKey)
                            {
                              CSMD_USHORT  usNumberOfData = CSMD_END_CONV_S(prAuxParamPtr->usDataLength);
                              CSMD_USHORT  usDataCounter;
                              CSMD_UCHAR  *pucDataPtr = ((CSMD_UCHAR *)(CSMD_VOID *)prAuxParamPtr) +
                                sizeof(CSMD_PARAMETER_TABLE_HEADER);
                              

                              /* check the length */
                              if (usNumberOfData > CSMD_NBR_PARAM_DATA)
                              {
                                /* Too many data */
                                eFuncRet = CSMD_TOO_MANY_PARAMETER_DATA;

                                break;
                              }

                              /* Fill the setup parameter array elements */
                              prConfig->parConfigParam[usFreeSetupParam].ulIDN =
                                CSMD_END_CONV_L(prAuxParamPtr->ulParameterIDN);
                              prConfig->parConfigParam[usFreeSetupParam].usDataLength =
                                CSMD_END_CONV_S(prAuxParamPtr->usDataLength);

                              /* parameter date (data block element 7) */
                              for (usDataCounter = 0; usDataCounter < usNumberOfData; usDataCounter++)
                              {
                                prConfig->parConfigParam[usFreeSetupParam].aucParamData[usDataCounter] =
                                  *(pucDataPtr + usDataCounter);
                              }


                              break;
                            }
                            else
                            {
                              /* Setup parameter key was not found -> go on with the search */
                              CSMD_USHORT  usLength = (CSMD_USHORT)(8 + CSMD_END_CONV_S(prAuxParamPtr->usDataLength));
                              CSMD_USHORT *pusTempPtr = (CSMD_USHORT *)(CSMD_VOID *)prAuxParamPtr;

                              /* Check fo SHORT alignment */
                              if (usLength & 0x1)
                              {
                                usLength++;
                              }
                              
                              /* Check for LONG alignment */
                              if (usLength & 0x2)
                              {
                                usLength += 2;
                              }

                              /* Convert length to number of SHORT */
                              usLength /= 2;
                              
                              prAuxParamPtr = (CSMD_PARAMETER_TABLE_HEADER *)(CSMD_VOID *)(pusTempPtr + usLength);
                            }
                          }

                          /* Free setup parameter was found -> stop the for loop */
                          break;
                          
                        }
                      }
                      
                      if (usFreeSetupParam == prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter)
                      {
                        /* no free setup list available */
                        eFuncRet = CSMD_TOO_MANY_SETUP_PARAMETER;
                        
                        break; /* for ((usFreeSetupParam = 0; usFreeSetupParam < ... */
                      }
                    }
                  } /* for (usK = 0; usK < usNumberOfConfParams;usK++) */
                  
                  break;
                }
                else
                {
                  /* Setup parameter list key was not found -> go on with the search */
                  CSMD_USHORT  usLength = (CSMD_USHORT)(3 + CSMD_END_CONV_S(prAuxParamsListPtr->usNumberOfConfigParams));
                  CSMD_USHORT *pusTempPtr = (CSMD_USHORT *)(CSMD_VOID *)prAuxParamsListPtr;
                  
                  if (usLength & 0x1)
                  {
                    usLength++;
                  }
                  
                  prAuxParamsListPtr = (CSMD_PARAMSLIST_TABLE_HEADER *)(CSMD_VOID *)(pusTempPtr + usLength);
                }
              }

              /* Stop further processing, because free array element was found, or error */
              break; /* for (usFreeSetupList = 0; usFreeSetupList... */
            }
          }

          /* Was there an error somewhere? -> stop! */
          if (eFuncRet != CSMD_NO_ERROR)
          {
            break; /* for (usI = 0; (usI < usNumberOfSlaveSetupTables)... */
          }

          if (usFreeSetupList == prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList)
          {
            /* no free setup list available */
            eFuncRet = CSMD_TOO_MANY_SETUP_LISTS;
            
            break; /* for (usI = 0; (usI < usNumberOfSlaveSetupTables)... */
          }

          /* Write the setup index */
          prConfig->parSlaveParamConfig[usSlaveSetupIdx].usConfigParamsList_Index = usFreeSetupList;
          prConfig->parSlaveParamConfig[usSlaveSetupIdx].usSlaveIndex = usSlaveIndex;
          /* Reset the next index, written later */
          prConfig->parSlaveParamConfig[usSlaveSetupIdx].usNextIndex = 0xFFFF;

          /* Is the key used somewhere else? */
          /* Start with the next slave setup table */
          for (usK = (CSMD_USHORT)(usI + 1); (usK < usNumberOfSlaveSetupTables) && (eFuncRet == CSMD_NO_ERROR); usK++)
          {
            CSMD_SLAVE_SETUP_TABLE *prTempSlaveSetupTable;
            CSMD_USHORT             usTempSlaveSetupKey;
            CSMD_USHORT             usTempSercAddress;
            CSMD_USHORT             usTempSlaveIndex;
            
            /* get the temporarily slave setup table element */
            prTempSlaveSetupTable = (CSMD_SLAVE_SETUP_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)rTableHeaderPtr.pusSlaveSetupStartPtr)
              + CSMD_TABLE_HEADER_LEN + (usK * CSMD_SLAVE_SETUP_TABLE_LEN));
            
            /* Get the setup list key */
            usTempSlaveSetupKey = CSMD_END_CONV_S(prTempSlaveSetupTable->usSetupParamsListKey);

            if (usTempSlaveSetupKey == usSlaveSetupKey)
            {
              /* Get Sercos address */
              usTempSercAddress = CSMD_END_CONV_S(prTempSlaveSetupTable->usSlaveAddress);

              /* Check for the master */
              if (usTempSercAddress == 0)
              {
                usTempSlaveIndex = prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves;
              }
              else
              {
                /* Get the slave index */
                if (boCheckAddresses)
                {
                  /* The CoSeMa slave list can be used */
                  
                  /* Get the index */
                  usTempSlaveIndex = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usTempSercAddress];
                  
                  /* Check the address*/
                  if (usTempSlaveIndex == 0)
                  {
                    /* Index is 0 -> slave not available or it has got index 0 */
                    if (prCSMD_Instance->rSlaveList.ausProjSlaveAddList[2] != usTempSercAddress)
                    {
                      /* Slave not available */
                      CSMD_BOOL boNext = FALSE;
                      
                      /* Slave deactivated? */
                      for (usL = 0; usL < prCSMD_Instance->rSlaveList.ausDeactSlaveAddList[0] / 2; usL++)
                      {
                        if (prCSMD_Instance->rSlaveList.ausDeactSlaveAddList[usL + 2] == usTempSercAddress)
                        {
                          /* Slave is deactivated -> stop further search */
                          boNext = TRUE;
                          
                          break;
                        }
                      }
                      
                      if (usL == (CSMD_USHORT)(prCSMD_Instance->rSlaveList.ausDeactSlaveAddList[0] / 2))
                      {
                        /* Slave not available */
                        eFuncRet = CSMD_WRONG_SLAVE_ADDRESS;
                        
                        boNext = TRUE;
                        
                        break;
                      }
                      
                      /* Process the next table entry, and remove marker if necessary */
                      if (boNext)
                      {
                        continue;
                      }
                    }
                  }
                }
                else
                {
                  /* The CoSeMa slave list is not yet filled and thus can't be used */
                  
                  /* Was the address used before? */
                  if (prCSMD_Instance->rSlaveList.ausParserTempIdxList[usTempSercAddress] == 0)
                  {
                    /* Not used before, maybe index is 0? */
                    if (prCSMD_Instance->rSlaveList.ausParserTempAddList[2] != usTempSercAddress)
                    {
                      /* Set the index */
                      usTempSlaveIndex = (CSMD_USHORT)(prCSMD_Instance->rSlaveList.ausParserTempAddList[0] / 2);
                      prCSMD_Instance->rSlaveList.ausParserTempIdxList[usTempSercAddress] = usTempSlaveIndex;                    
                      
                      /* Increment the number of configured slaves */
                      prCSMD_Instance->rSlaveList.ausParserTempAddList[0] +=2;
                      prCSMD_Instance->rSlaveList.ausParserTempAddList[1] +=2;
                      
                      /* Set the address */
                      prCSMD_Instance->rSlaveList.ausParserTempAddList[usTempSlaveIndex + 2] = usTempSercAddress;
                    }
                    else
                    {
                      usTempSlaveIndex = 0;
                    }
                  }
                  else
                  {
                    usTempSlaveIndex =
                      prCSMD_Instance->rSlaveList.ausParserTempIdxList[usTempSercAddress];
                  }
                }
              }

              /* Search a free setup index */
              for (usSlaveSetupIdx = 0; usSlaveSetupIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams; usSlaveSetupIdx++)
              {
                if (prConfig->parSlaveParamConfig[usSlaveSetupIdx].usConfigParamsList_Index == 0xFFFF)
                {
                  /* free slave setup found */
                  
                  /* mark the slave setup temporarily as manipulated */
                  prCSMD_Instance->rPriv.paucSlaveSetupManipulated[usSlaveSetupIdx] = TRUE;
                  
                  break;
                }
              }
              
              if (usSlaveSetupIdx == prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams)
              {
                /* no free slave setup available */
                eFuncRet = CSMD_TOO_MANY_SLAVE_SETUP;
                
                break; /* for (usK =  usI; (usK < usNumberOfSlaveSetupTables)... */
              }
              
              /* Write the setup index */
              prConfig->parSlaveParamConfig[usSlaveSetupIdx].usConfigParamsList_Index = usFreeSetupList;
              prConfig->parSlaveParamConfig[usSlaveSetupIdx].usSlaveIndex = usTempSlaveIndex;
              /* Reset the next index, written later */
              prConfig->parSlaveParamConfig[usSlaveSetupIdx].usNextIndex = 0xFFFF;
              
            }
          } /* for (usK = (CSMD_USHORT)(usI + 1); (usK < usNumberOfSlaveSetupTables) && (eFuncRet == CSMD_NO_ERROR); usK++) */
          
        } /* for (usI = 0; (usI < usNumberOfSlaveSetupTables) && (eFuncRet == CSMD_NO_ERROR); usI++) */

        /* Set the correct indices for slave parameter configurations */
        for (usI = 0; (usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams) && (eFuncRet == CSMD_NO_ERROR); usI++)
        {
          CSMD_USHORT  usSlaveIndex;

          /* Configuration parameter list used? */
          if (prConfig->parSlaveParamConfig[usI].usConfigParamsList_Index == 0xFFFF)
          {
            /* Not used */
            continue;
          }

          usSlaveIndex = prConfig->parSlaveParamConfig[usI].usSlaveIndex;

          if (usSlaveIndex == prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
          {
            /* The master */
            
            /* Is there already an entry for the master? */
            if (prConfig->prMaster_Config->usFirstConfigParamIndex == 0xFFFF)
            {
              /* No entry existing -> set entry and go on with next config */
              prConfig->prMaster_Config->usFirstConfigParamIndex = usI;
              
              continue;
            }
            else if (prConfig->prMaster_Config->usFirstConfigParamIndex == usI)
            {
              /* Don't take the same index twice! */
              continue;
            }
            else
            {
              CSMD_USHORT  usCounter = 0;
              CSMD_USHORT  usActIndex = prConfig->prMaster_Config->usFirstConfigParamIndex;
              
              /* Search the next free pointer */
              while(   (prConfig->parSlaveParamConfig[usActIndex].usNextIndex != 0xFFFF)
                    && (prConfig->parSlaveParamConfig[usActIndex].usNextIndex != usI)
                    && (usCounter++ < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams) )
              {
                usActIndex = prConfig->parSlaveParamConfig[usActIndex].usNextIndex;
              }

              /* Is there already a reference to this index? */
              if (prConfig->parSlaveParamConfig[usActIndex].usNextIndex == usI)
              {
                /* Do nothing and go on with the next one */
                continue;
              }
              
              /* Is the index not used? */
              if (prConfig->parSlaveParamConfig[usActIndex].usNextIndex == 0xFFFF)
              {
                /* Set entry and go on with next config */
                prConfig->parSlaveParamConfig[usActIndex].usNextIndex = usI;
                
                continue;
              }
              
              /* Is the counter bigger than allowed? -> error */
              if (usCounter >= prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams)
              {
                eFuncRet = CSMD_TOO_MANY_SLAVE_SETUP;
              }
            }
          }
          else
          {
            /* a slave */

            /* Is there already an entry for this slave? */
            if (prConfig->parSlave_Config[usSlaveIndex].usFirstConfigParamIndex == 0xFFFF)
            {
              /* No entry existing -> set entry and go on with next config */
              prConfig->parSlave_Config[usSlaveIndex].usFirstConfigParamIndex = usI;
              
              continue;
            }
            else if (prConfig->parSlave_Config[usSlaveIndex].usFirstConfigParamIndex == usI)
            {
              /* Don't take the same index twice! */
              continue;
            }
            else
            {
              CSMD_USHORT  usCounter = 0;
              CSMD_USHORT  usActIndex = prConfig->parSlave_Config[usSlaveIndex].usFirstConfigParamIndex;
              
              /* Search the next free pointer */
              while(   (prConfig->parSlaveParamConfig[usActIndex].usNextIndex != 0xFFFF)
                    && (prConfig->parSlaveParamConfig[usActIndex].usNextIndex != usI)
                    && (usCounter++ < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams) )
              {
                usActIndex = prConfig->parSlaveParamConfig[usActIndex].usNextIndex;
              }

              /* Is there already a reference to this index? */
              if (prConfig->parSlaveParamConfig[usActIndex].usNextIndex == usI)
              {
                /* Do nothing and go on with the next one */
                continue;
              }
              
              /* Is the index not used? */
              if (prConfig->parSlaveParamConfig[usActIndex].usNextIndex == 0xFFFF)
              {
                /* Set entry and go on with next config */
                prConfig->parSlaveParamConfig[usActIndex].usNextIndex = usI;
                
                continue;
              }
              
              /* Is the counter bigger than allowed? -> error */
              if (usCounter >= prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams)
              {
                eFuncRet = CSMD_TOO_MANY_SLAVE_SETUP;
              }
            }
          }
        } /* for (usI = 0; (usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams) && (eFuncRet == CSMD_NO_ERROR); usI++) */


      } /* if (rTableHeaderPtr.pusSlaveSetupStartPtr != NULL) */

      break; /* from case */

    default:
      eFuncRet = CSMD_WRONG_BIN_CONFIG_VERSION;
      break;
    }
  }

  /* in case of error, eliminate all changes made during the run */
  if (   (eFuncRet != CSMD_NO_ERROR)
      && !(   (eFuncRet == CSMD_WRONG_BIN_CONFIG_FORMAT)
           && (rTableHeaderPtr.pusCnncStartPtr != NULL)
           && (rTableHeaderPtr.pusCnncStartPtr == NULL)) )
  {
    /* Clear all manipulated connections */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn; usI++)
    {
      /* is this connection manipulated? -> clear */
      if (aucConnManipulated[usI] == TRUE)
      {
        /* Free the used connection */
        prCSMD_Instance->rPriv.rUsedCfgs.paucConnUsed[usI] = FALSE;

        /* Free the used connection number, if needed */
        if (prConfig->parConnection[usI].usS_0_1050_SE2 < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn)
        {
          prCSMD_Instance->rPriv.rUsedCfgs.paucConnNbrUsed[prConfig->parConnection[usI].usS_0_1050_SE2] = FALSE;
        }
      }
    }

    /* Clear all manipulated configurations */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig; usI++)
    {
      /* is this connection manipulated? -> clear */
      if (ausConfManipulated[usI] != 0)
      {
        /* Free the used connection */
        prCSMD_Instance->rPriv.rUsedCfgs.paucConfUsed[usI] = FALSE;

        /* Clear the data */
        (CSMD_VOID) CSMD_HAL_memset( &prConfig->parConfiguration[usI],
                                     0,
                                     sizeof (CSMD_CONFIGURATION) );
      }
    }

    /* Clear all manipulated RT bit configurations */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig; usI++)
    {
      /* is this connection manipulated? -> clear */
      if (ausRTBtManipulated[usI] != 0)
      {
        /* Free the used connection */
        prCSMD_Instance->rPriv.rUsedCfgs.paucRTBtUsed[usI] = FALSE;

        /* Clear the data */
        (CSMD_VOID) CSMD_HAL_memset( &prConfig->parRealTimeBit[usI],
                                     0,
                                     sizeof (CSMD_REALTIME_BIT) );
      }
    }

    /* Reset all manipulated master configurations */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usI++)
    {
      /* is this connection manipulated? -> clear */
      if (aucMasterInstManipulated[usI])
      {
        /* Free the used master configuration */
        prConfig->prMaster_Config->parConnIdxList[usI].usConfigIdx = 0xFFFF;
        prConfig->prMaster_Config->parConnIdxList[usI].usConnIdx   = 0xFFFF;
        prConfig->prMaster_Config->parConnIdxList[usI].usRTBitsIdx = 0xFFFF;

        /* Decrement the number of master connections */
        prConfig->prMaster_Config->usNbrOfConnections--;

      }
    }

    /* Reset all manipulated slave configurations */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
    {
      for (usK = 0; usK < CSMD_MAX_CONNECTIONS; usK++)
      {
        /* is this connection manipulated? -> clear */
        if (prCSMD_Instance->rPriv.parSlaveInst[usI].aucManipulated[usK])
        {
          /* Free the used slave configuration */
          prConfig->parSlave_Config[usI].arConnIdxList[usK].usConfigIdx = 0xFFFF;
          prConfig->parSlave_Config[usI].arConnIdxList[usK].usConnIdx   = 0xFFFF;
          prConfig->parSlave_Config[usI].arConnIdxList[usK].usRTBitsIdx = 0xFFFF;
          
          /* Decrement the number of master connections */
          prConfig->parSlave_Config[usI].usNbrOfConnections--;
          
        }
      }

      /* Clear the index of first configuration parameter for all slaves, reconfigured later */
      prConfig->parSlave_Config[usI].usFirstConfigParamIndex = 0xFFFF;
    }

    /* Clear the index of first configuration parameter for the master, reconfigured later */
    prConfig->prMaster_Config->usFirstConfigParamIndex = 0xFFFF;

    /* Free the used slave configuration parameters */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams; usI++)
    {
      if (prCSMD_Instance->rPriv.paucSlaveSetupManipulated[usI])
      {
        prConfig->parSlaveParamConfig[usI].usConfigParamsList_Index = 0xFFFF;
        prConfig->parSlaveParamConfig[usI].usSlaveIndex = 0xFFFF;
        prConfig->parSlaveParamConfig[usI].usNextIndex = 0xFFFF;
      }
    }
    
    /* Rebuild the correct indices for slave parameter configurations */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams; usI++)
    {
      CSMD_USHORT  usSlaveIndex;
      
      /* Configuration parameter list used? */
      if (prConfig->parSlaveParamConfig[usI].usConfigParamsList_Index == 0xFFFF)
      {
        /* Not used */
        continue;
      }
      
      usSlaveIndex = prConfig->parSlaveParamConfig[usI].usSlaveIndex;
      
      if (usSlaveIndex == prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
      {
        /* The master */
        
        /* Is there already an entry for the master? */
        if (prConfig->prMaster_Config->usFirstConfigParamIndex == 0xFFFF)
        {
          /* No entry existing -> set entry and go on with next config */
          prConfig->prMaster_Config->usFirstConfigParamIndex = usI;
          
          continue;
        }
        else if (prConfig->prMaster_Config->usFirstConfigParamIndex == usI)
        {
          /* Don't take the same index twice! */
          continue;
        }
        else
        {
          CSMD_USHORT  usCounter = 0;
          CSMD_USHORT  usActIndex = prConfig->prMaster_Config->usFirstConfigParamIndex;
          
          /* Search the next free pointer */
          while(   (prConfig->parSlaveParamConfig[usActIndex].usNextIndex != 0xFFFF)
                && (prConfig->parSlaveParamConfig[usActIndex].usNextIndex != usI)
                && (usCounter++ < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams) )
          {
            usActIndex = prConfig->parSlaveParamConfig[usActIndex].usNextIndex;
          }
          
          /* Is the index not used? */
          if (prConfig->parSlaveParamConfig[usActIndex].usNextIndex == 0xFFFF)
          {
            /* Set entry and go on with next config */
            prConfig->parSlaveParamConfig[usActIndex].usNextIndex = usI;
            
            continue;
          }
        }
      }
      else
      {
        /* a slave */
        
        /* Is there already an entry for this slave? */
        if (prConfig->parSlave_Config[usSlaveIndex].usFirstConfigParamIndex == 0xFFFF)
        {
          /* No entry existing -> set entry and go on with next config */
          prConfig->parSlave_Config[usSlaveIndex].usFirstConfigParamIndex = usI;
          
          continue;
        }
        else if (prConfig->parSlave_Config[usSlaveIndex].usFirstConfigParamIndex == usI)
        {
          /* Don't take the same index twice! */
          continue;
        }
        else
        {
          CSMD_USHORT  usCounter = 0;
          CSMD_USHORT  usActIndex = prConfig->parSlave_Config[usSlaveIndex].usFirstConfigParamIndex;
          
          /* Search the next free pointer */
          while(   (prConfig->parSlaveParamConfig[usActIndex].usNextIndex != 0xFFFF)
                && (prConfig->parSlaveParamConfig[usActIndex].usNextIndex != usI)
                && (usCounter++ < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams) )
          {
            usActIndex = prConfig->parSlaveParamConfig[usActIndex].usNextIndex;
          }
          
          /* Is the index not used? */
          if (prConfig->parSlaveParamConfig[usActIndex].usNextIndex == 0xFFFF)
          {
            /* Set entry and go on with next config */
            prConfig->parSlaveParamConfig[usActIndex].usNextIndex = usI;
            
            continue;
          }
        }
      }
    } /* for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams); usI++) */

    /* Reset all manipulated configuration parameter lists */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList; usI++)
    {
      if (ausSetupParamsListManipulated[usI])
      {
        /* Set the application ID to 0 */
        prConfig->parConfigParamsList[usI].usApplicationID = 0;
    
        /* Free the used references to the parameter table */
        for (usK = 0; usK < CSMD_MAX_PARAMS_IN_CONFIG_LIST; usK++)
        {
          prConfig->parConfigParamsList[usI].ausParamTableIndex[usK] = 0xFFFF;
        }
      }
    }

    /* Reset all manipulated configuration parameters */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter; usI++)
    {
      if (ausSetupParameterManipulated[usI])
      {
        /* Set all data to 0 */
        (CSMD_VOID) CSMD_HAL_memset( &prConfig->parConfigParam[usI],
                                     0,
                                     sizeof(CSMD_CONFIGURATION_PARAMETER) );
      }
    }

  }
  else
  {
    /* Reset the possible error state in case of only clear connections */
    eFuncRet = CSMD_NO_ERROR;
  }

  return (eFuncRet);
} /* end: CSMD_ProcessBinConfig() */



/**************************************************************************/ /**
\brief Checks the keys inside the connection table.

\ingroup func_binconfig
\b Description: \n
   This function takes the connection configurations, which are configured
   before the list of projected slaves was filled, and moves them to the
   CoSeMa structure. The data is appended to the existing connection
   configuration.

<B>Call Environment:</B> \n
   The function can only be called after the list of projected slaves
   was filled and before CSMD_GetTimingData() is called.

   \note This function shall not be called twice without deleting the temporary 
   master and slave configuration data in-between.

   The call-up should be performed from a task.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prTempMasterCfg
              Pointer to a structure holding the temporary master configuration
\param [in]   prTempSlaveCfg
              Pointer to a structure holding the temporary slave configuration

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_WRONG_SLAVE_ADDRESS \n
              \ref CSMD_TOO_MANY_CONN_FOR_MASTER \n
              \ref CSMD_TOO_MANY_CONN_FOR_SLAVE \n
              \ref CSMD_NO_ERROR \n

\author       MSt
\date         20.02.2012

***************************************************************************** */
CSMD_FUNC_RET CSMD_TransferConnConfigs( CSMD_INSTANCE                  *prCSMD_Instance,
                                       const CSMD_MASTER_CONFIGURATION *prTempMasterCfg,
                                       const CSMD_SLAVE_CONFIGURATION  *prTempSlaveCfg )
{
  CSMD_FUNC_RET       eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT         usI, usK;
  CSMD_CONFIG_STRUCT *prCfg = &prCSMD_Instance->rConfiguration;
  CSMD_UCHAR          aucMasterInstManipulated[CSMD_MAX_CONNECTIONS_MASTER] = {FALSE};
  CSMD_UCHAR          aucUsedConnections[CSMD_MAX_GLOB_CONN]                = {FALSE};
  CSMD_USHORT         usConnInstIdx;

  /* Check if the transfer is already possible */
  if (prCSMD_Instance->rSlaveList.usNumProjSlaves == 0)
  {
    /* No slaves projected so far -> error */
    eFuncRet = CSMD_WRONG_PHASE;
  }

    /* Init manipulation markers */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
  {
    for (usK = 0; usK < CSMD_MAX_CONNECTIONS; usK++)
    {
      prCSMD_Instance->rPriv.parSlaveInst[usI].aucManipulated[usK] = FALSE;
    }
  }

  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams; usI++)
  {
    prCSMD_Instance->rPriv.paucSlaveSetupManipulated[usI] = FALSE;
  }

  if (eFuncRet == CSMD_NO_ERROR)
  {
    /* Check if the former used addresses match the projected slaves */
    for (usI = 0;
         usI < (CSMD_USHORT)(prCSMD_Instance->rSlaveList.ausParserTempAddList[0] / 2);
         usI++)
    {
      CSMD_USHORT  usTempAdd = prCSMD_Instance->rSlaveList.ausParserTempAddList[2 + usI];
      
      /* Is the address projected? */
      if (prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usTempAdd] != 0)
      {
        /* The address is there -> OK */
        continue;
      }
      else
      {
        /* Is the index 0 of this address? */
        if (prCSMD_Instance->rSlaveList.ausProjSlaveAddList[2] == usTempAdd)
        {
          /* The address is there -> OK */
          continue;
        }
        else
        {
          CSMD_BOOL boFound =  FALSE;
          
          /* The address does not exist; maybe inside the list of deactivated slaves? */
          for (usK = 0;
               usK < prCSMD_Instance->rSlaveList.ausDeactSlaveAddList[0] / 2;
               usK++)
          {
            if (prCSMD_Instance->rSlaveList.ausDeactSlaveAddList[2 + usK] == usTempAdd)
            {
              /* The slave is deactivated -> OK */
              boFound = TRUE;
              
              /* Mark the slave so that no further action is performed */
              prCSMD_Instance->rSlaveList.ausParserTempAddList[2 + usI] += CSMD_MAX_SLAVE_ADD;
              
              break;
            }
          }
          
          if (boFound)
          {
            /* Check the next address */
            continue;
          }
          
          /* It was not found -> Error! */
          eFuncRet = CSMD_WRONG_SLAVE_ADDRESS;
          
          break;
        }
      }
    }
  }

  /* Copy the data, if the addresses are OK */

  /* Slave data */
  for (usI = 0;
       ((eFuncRet == CSMD_NO_ERROR) && (usI < (CSMD_USHORT)(prCSMD_Instance->rSlaveList.ausParserTempAddList[0] / 2)));
       usI++)
  {
    CSMD_USHORT  usTempAdd = prCSMD_Instance->rSlaveList.ausParserTempAddList[2 + usI];
    CSMD_USHORT  usIdx;
    
    /* Deactivated? */
    if (usTempAdd > CSMD_MAX_SLAVE_ADD)
    {
      continue;
    }
    
    /* Get the index of this slave */
    usIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usTempAdd];

    /* Check for index 0 or slave not projected */
    if (usIdx == 0)
    {
      /* Index is really 0? */
      if (prCSMD_Instance->rSlaveList.ausProjSlaveAddList[0 + 2] == usTempAdd)
      {
        /* Nothing to do */
      }
      else
      {
        /* Slave not projected -> next slave */
        continue;
      }
    }

    /* Does the number of data fit into the existing array? */
    if ((prTempSlaveCfg[usI].usNbrOfConnections + prCfg->parSlaveConfig[usIdx].usNbrOfConnections) >
         CSMD_MAX_CONNECTIONS)
    {
      /* Too many connections */
      eFuncRet = CSMD_TOO_MANY_CONN_FOR_SLAVE;
      
      break;
    }
    else
    {
      CSMD_USHORT  usNbrTransferredConns = 0;

      /* Copy the slave data */
      for (usK = 0; usK < CSMD_MAX_CONNECTIONS; usK++)
      {
        /* Is the temporary connection used? */
        if (prTempSlaveCfg[usI].arConnIdxList[usK].usConnIdx == 0xFFFF)
        {
          /* Not used -> check next connection */
          continue;
        }

        /* Copy only the number of connections of the temporary structure */
        if (usNbrTransferredConns++ < prTempSlaveCfg[usI].usNbrOfConnections)
        {
          /* search for a free instance in the real data */
          for (usConnInstIdx = (CSMD_USHORT) 0; usConnInstIdx < CSMD_MAX_CONNECTIONS; usConnInstIdx++)
          {
            if (prCfg->parSlaveConfig[usIdx].arConnIdxList[usConnInstIdx].usConnIdx == 0xFFFF)
            {
              /* free slave instance found */

              /* mark the slave instance temporarily as manipulated */
              prCSMD_Instance->rPriv.parSlaveInst[usIdx].aucManipulated[usConnInstIdx] = TRUE;

              /* Copy */
              prCfg->parSlaveConfig[usIdx].arConnIdxList[usConnInstIdx].usConnIdx
                  = prTempSlaveCfg[usI].arConnIdxList[usK].usConnIdx;
              prCfg->parSlaveConfig[usIdx].arConnIdxList[usConnInstIdx].usConfigIdx
                  = prTempSlaveCfg[usI].arConnIdxList[usK].usConfigIdx;
              prCfg->parSlaveConfig[usIdx].arConnIdxList[usConnInstIdx].usRTBitsIdx
                  = prTempSlaveCfg[usI].arConnIdxList[usK].usRTBitsIdx;

              /* Increment the number of connections */
              prCfg->parSlaveConfig[usIdx].usNbrOfConnections++;

              /* Mark the connection as used; the master copy routine needs it */
              aucUsedConnections[prTempSlaveCfg[usI].arConnIdxList[usK].usConnIdx] = TRUE;

              break;
            }
          }
        }
      }
    }
    
    /* Check for slave setup parameters */
    for (usK=0; usK < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams;usK++)
    {
      /* Is something configured? */
      if (   (prCfg->parSlaveParamConfig[usK].usSlaveIndex == usI)
          && (!prCSMD_Instance->rPriv.paucSlaveSetupManipulated[usK]))
      {
        /* There is a configuration; change the slave index */
        prCfg->parSlaveParamConfig[usK].usSlaveIndex = usIdx;

        /* Mark the configuration as changed to prevent further operations */
        prCSMD_Instance->rPriv.paucSlaveSetupManipulated[usK] = TRUE;
      }
    }
  }

  /* Master data */
  if (eFuncRet == CSMD_NO_ERROR)
  {
    /* Does the number of data fit into the existing array? */
    if ( (prTempMasterCfg->usNbrOfConnections + prCfg->rMasterCfg.usNbrOfConnections)
        > prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster)
    {
      /* Too many connections */
      eFuncRet = CSMD_TOO_MANY_CONN_FOR_MASTER;
    }
    else
    {
      /* Copy the master data */
      for (usI = 0; usI < prTempMasterCfg->usNbrOfConnections;usI++)
      {
        /* Is this connection used by an existing slave? */
        if (!(aucUsedConnections[prTempMasterCfg->parConnIdxList[usI].usConnIdx]))
        {
          /* Not used -> check the next connection */
          continue;
        }

        /* search for a free instance */
        for (usConnInstIdx = 0; usConnInstIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usConnInstIdx++)
        {
          if (prCfg->rMasterCfg.parConnIdxList[usConnInstIdx].usConnIdx == 0xFFFF)
          {
            /* free master instance found */
            
            /* mark the master instance temporarily as manipulated */
            aucMasterInstManipulated[usConnInstIdx] = TRUE;
            
            /* Copy */
            prCfg->rMasterCfg.parConnIdxList[usConnInstIdx].usConnIdx =
              prTempMasterCfg->parConnIdxList[usI].usConnIdx;
            prCfg->rMasterCfg.parConnIdxList[usConnInstIdx].usConfigIdx =
              prTempMasterCfg->parConnIdxList[usI].usConfigIdx;
            prCfg->rMasterCfg.parConnIdxList[usConnInstIdx].usRTBitsIdx =
              prTempMasterCfg->parConnIdxList[usI].usRTBitsIdx;
            
            /* Increment the number of connections */
            prCfg->rMasterCfg.usNbrOfConnections++;
            
            break;
          }
        }
      }
    }

    /* Copy the setup parameters for the master */
/*    for (usK = 0; usK < CSMD_MAX_SLAVEPARAMS_LIST;usK++)
    {
      prCfg->rMasterCfg.ausMasterConfigParams_Index[usK] =
        prTempMasterCfg->ausMasterConfigParams_Index[usK];
    }*/
  }

  if (eFuncRet != CSMD_NO_ERROR)
  {
    /* Reset all manipulated master configurations */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usI++)
    {
      /* is this connection manipulated? -> clear */
      if (aucMasterInstManipulated[usI])
      {
        /* Free the used master configuration */
        prCfg->prMaster_Config->parConnIdxList[usI].usConfigIdx = 0xFFFF;
        prCfg->prMaster_Config->parConnIdxList[usI].usConnIdx   = 0xFFFF;
        prCfg->prMaster_Config->parConnIdxList[usI].usRTBitsIdx = 0xFFFF;

        /* Decrement the number of master connections */
        prCfg->prMaster_Config->usNbrOfConnections--;

      }
    }

    /* Reset all manipulated slave configurations */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
    {
      for (usK = 0; usK < CSMD_MAX_CONNECTIONS; usK++)
      {
        /* is this connection manipulated? -> clear */
        if (prCSMD_Instance->rPriv.parSlaveInst[usI].aucManipulated[usK])
        {
          /* Free the used slave configuration */
          prCfg->parSlave_Config[usI].arConnIdxList[usK].usConfigIdx = 0xFFFF;
          prCfg->parSlave_Config[usI].arConnIdxList[usK].usConnIdx   = 0xFFFF;
          prCfg->parSlave_Config[usI].arConnIdxList[usK].usRTBitsIdx = 0xFFFF;
          
          /* Decrement the number of slave connections */
          prCfg->parSlave_Config[usI].usNbrOfConnections--;
          
        }
      }
    }
  }
  
  return(eFuncRet);
} /* end CSMD_TransferConnConfigs */



/**************************************************************************/ /**
\brief Initializes connection configs if projected slaves are not configured.

\ingroup func_binconfig
\b Description: \n
   This function initializes the structures for the connection configuration
   of the master and the slaves which are maintained by the application. This
   is only done if the destination of the pointer for the master configuration
   is not the master configuration of CoSeMa.

<B>Call Environment:</B> \n
   The function should be called before the first call of CSMD_ProcessBinConfig()
   in case of empty projected slaves list .

   The call-up should be performed from a task.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       MSt
\date         20.02.2012

***************************************************************************** */
CSMD_FUNC_RET CSMD_InitTempConnConfigs( CSMD_INSTANCE *prCSMD_Instance)
{
  CSMD_FUNC_RET       eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT         usI;
  CSMD_ULONG          ulSize;
  CSMD_CONFIG_STRUCT *prCfg;
  
  prCfg = &prCSMD_Instance->rConfiguration;
  
  /* Check whether the original CoSeMa structure is used or not */
  /* Check only one pointer should be sufficient */
  if (prCfg->prMaster_Config != &prCfg->rMasterCfg)
  {
    /* Init the structs */
    
    /* Set default value 0xFFFF for all indices in master connection index list. */
    ulSize =   sizeof(CSMD_CONN_IDX_STRUCT)
             * prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster;

    (CSMD_VOID) CSMD_HAL_memset( prCfg->prMaster_Config->parConnIdxList,
                                 0xFF,
                                 ulSize );

    prCfg->rMasterCfg.usNbrOfConnections = 0;
    
    /* Set default value 0xFFFF for all indexes in connection index list for all possible slaves. */
    ulSize = sizeof( prCfg->parSlaveConfig[0].arConnIdxList );

    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
    {
      (CSMD_VOID) CSMD_HAL_memset( prCfg->parSlave_Config[usI].arConnIdxList,
                                   0xFF,
                                   ulSize );
      
      prCfg->parSlaveConfig[usI].usNbrOfConnections = 0;
    }
  }

  return(eFuncRet);
} /* end CSMD_InitTempConnConfigs */

/*! \endcond */ /* PUBLIC */

#endif  /* #ifdef CSMD_CONFIG_PARSER */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

19 Nov 2010 MSt
  - File created.
09 Dec 2013 MSt
  - Defdb00163778
    Temporary workaround for CCD (Master is not able to produce into the AT).
04 Jan 2014 MSt
  - Defdb00165956
    Transfer of temporary connection configuration fails in some cases
23 Jan 2014 WK
    Renamed elements in structure CSMD_CONFIG_STRUCT:
      prMasterConfig --> prMaster_Config
      parSlaveConfig --> parSlave_Config
26 Feb 2014 WK
      BOOL aboSlaveInstManipulated[CSMD_MAX_SLAVES][CSMD_MAX_CONNECTIONS];
  --> CSMD_SLAVE_INST_MANIPULATED arSlaveInst[CSMD_MAX_SLAVES];
24 Mar 2014 MSt
  - Defdb00168583
    Too many local variables cause stack overflow 
24 Mar 2014 MSt
  - Defdb00168609
    Endianness conversion missing
14 Nov 2014 WK
  - Removed private functions.
16 Jan 2015 WK
  - Defdb00163778
    Removed temporary workaround for CCD. Fixed short access.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 Feb 2015 WK
  - CSMD_ProcessBinConfig()
    Fixed clearing the CSMD_USED_MARKER array.
31 Mar 2015 WK
  - Defdb00163778
    Reinsert temporary workaround for CCD.
28 Apr 2015 WK
  - Defdb00178597
    Fixed type castings for 64-bit systems.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
21 Jun 2016 WK
  - Defdb00187787
    CSMD_ProcessBinConfig()
    Deleting the last connection in the binary configuration fails with
    a segmentation fault.
  
------------------------------------------------------------------------------
*/
