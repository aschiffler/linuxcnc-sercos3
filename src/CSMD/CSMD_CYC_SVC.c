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
 \file   CSMD_CYC_SVC.c
 \author WK
 \date   01.09.2010
 \brief  This File contains the public API functions for
         service channel which are called cyclically 
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#include "CSMD_PRIV_SVC.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */


/**************************************************************************/ /**
\brief Returns a bit list of pending SVC interrupts.

\ingroup sync_internal
\b Description: \n
   This function checks all service channels for pending interrupts. The interrupt 
   status (TRUE/FALSE) is returned in a bit list and the corresponding interrupt is 
   cleared. If for the specified slave address, a service channel is emulated by
   means of software, TRUE will always be returned as there must be processing in 
   each Sercos cycle.\n\n

   SVC_Channel_Index = Array_Index * 32 + BitNumber \n\n

   The 32 maximally possible hardware service channels are assigned the 
   SVC_Channel_Indices in ascending order, starting with 0. They are directly 
   followed by the service channels emulated by means of software. E.g. with 
   CSMD_MAX_HW_CONTAINER = 16, the first interrupt emulated by means of software is 
   located at array index 0 and bit number 16. Assignment of the slave addresses
   to the service channels is effected via the list of slaves to be operated
   transferred in CSMD_SetPhase1().<BR>

  |     7       | ... |     1       |     0       |Array_Index\n of aulSvcIntStatus[ ]|
  | :---------: | :-: | :---------: | :---------: | :-------------------------------: |
  |  31 ...   0 | ... |   31 ...  0 |    31 ... 0 |   BitNumber                       |
  | 255 ... 224 | ... |   63 ... 32 |    31 ... 0 |   SVC_Channel_Index               |

<BR>
\todo translation \n
      Zuordnung der Hardware Service Kanal Interrupts: \n

  |Interrupt\n Number|Interrupt\n Reset\n Register 1|   Name    |            Event             |
  | :--------------- | :--------------------------- | :-------- | :--------------------------- |
  |   0              | Bit  0                       | Int_SVC0  | Service Channel Interrupt  1 |
  |   1              | Bit  1                       | Int_SVC1  | Service Channel Interrupt  2 |
  | ...              | ...                          | ...       | ...                          |
  |  30              | Bit 30                       | Int_SVC30 | Service Channel Interrupt 31 |
  |  31              | Bit 31                       | Int_SVC31 | Service Channel Interrupt 32 |
<BR>

<B>Call Environment:</B> \n
   Advantageously, this function is called within an interrupt service routine. 
   The call-up can also be performed from a task if the handling of the non-cyclical 
   service channel and the Sercos cycle are to be asynchronous.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [out]  paulSvcIntStatus
              Pointer to bit list with the interrupt status of all service channels
              (Array with 8/16 elements for max 256/512 slaves)

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         16.02.2005

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_CheckSvchInt( CSMD_INSTANCE *prCSMD_Instance,
                                 CSMD_ULONG    *paulSvcIntStatus )
{
  
#if (CSMD_MAX_HW_CONTAINER == 0)
  {
    CSMD_USHORT usI;
    /* Read interrupt status of software svc */
    for (usI = 0; usI < (((prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves-1) / 32)+1); usI++)
    {
      paulSvcIntStatus[usI] = prCSMD_Instance->rPriv.aulSVC_Int_Flags[usI];
    }
  }
#elif (CSMD_MAX_HW_CONTAINER == 32)
  /* Read interrupt status of hardware svc */
  paulSvcIntStatus[0] = CSMD_HAL_GetInterrupt(&prCSMD_Instance->rCSMD_HAL, FALSE, TRUE);
  #if CSMD_MAX_SLAVES > 32
  {
    CSMD_USHORT usI;
    /* Read interrupt status of software svc */
    for (usI = 1; usI < (((prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves-1) / 32)+1); usI++)
    {
      paulSvcIntStatus[usI] = prCSMD_Instance->rPriv.aulSVC_Int_Flags[usI];
    }
  }
  #endif
#else
  /* Read interrupt status of hardware svc */
  paulSvcIntStatus[0] =
      (CSMD_HAL_GetInterrupt(&prCSMD_Instance->rCSMD_HAL, FALSE, TRUE) & ((1UL << CSMD_MAX_HW_CONTAINER)-1))
    + (prCSMD_Instance->rPriv.aulSVC_Int_Flags[0] & ~((1UL << CSMD_MAX_HW_CONTAINER)-1));
  #if CSMD_MAX_SLAVES > 32
  {
    CSMD_USHORT usI;
    /* Read interrupt status of software svc */
    for (usI = 1; usI < (((prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves-1) / 32)+1); usI++)    /*lint !e681: Loop is not entered */
    {
      paulSvcIntStatus[usI] = prCSMD_Instance->rPriv.aulSVC_Int_Flags[usI];
    }
  }
  #endif
#endif
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_CheckSvchInt() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Resets the service channel interrupts selected in the clear mask.

\ingroup sync_internal
\b Description: \n
   No further description.
  
<BR>

  |     7       | ... |     1       |     0       |Array_Index\n of aulIntClearMask[ ]|
  | :---------: | :-: | :---------: | :---------: | :-------------------------------: |
  |  31 ...   0 | ... |   31 ...  0 |    31 ... 0 |   BitNumber                       |
  | 255 ... 224 | ... |   63 ... 32 |    31 ... 0 |   SVC_Channel_Index               |
<BR>

   SVC_Channel_Index = Array_Index * 32 + BitNumber

   \todo translation
   Der FPGA stellt maximal 32 Service Container (einstellbar ueber \#define \ref CSMD_MAX_HW_CONTAINER) zur Verfuegung.


  |Interrupt\n Number|Interrupt\n Reset\n Register 1|   Name    |            Event             |
  | :--------------- | :--------------------------- | :-------- | :--------------------------- |
  |   0              | Bit  0                       | Int_SVC0  | Service Channel Interrupt  1 |
  |   1              | Bit  1                       | Int_SVC1  | Service Channel Interrupt  2 |
  | ...              | ...                          | ...       | ...                          |
  |  30              | Bit 30                       | Int_SVC30 | Service Channel Interrupt 31 |
  |  31              | Bit 31                       | Int_SVC31 | Service Channel Interrupt 32 |


<B>Call Environment:</B> \n
   Advantageously, this function is called within an interrupt service routine if 
   the FPGA is the cycle master. If the FPGA is not the cycle master, the 
   function may also be called in a task.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   paulIntClearMask
              The bits of the interrupts which are set in the transferred mask
              are cleared

\return       \ref CSMD_NO_ERROR \n

\author       WK 
\date         16.02.2005

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_ClearSvchInt( CSMD_INSTANCE *prCSMD_Instance,
                                 CSMD_ULONG    *paulIntClearMask )
{
  
#if (CSMD_MAX_HW_CONTAINER == 0)
  {
    CSMD_USHORT usI;
    /* clear interrupt status of software svc */
    for (usI = 0; usI < (((prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves-1) / 32)+1); usI++)
    {
      prCSMD_Instance->rPriv.aulSVC_Int_Flags[usI] &= ~paulIntClearMask[usI];
    }
  }
#elif (CSMD_MAX_HW_CONTAINER == 32)
  /* clear interrupt status of hardware svc */
  CSMD_HAL_ClearInterrupt( &prCSMD_Instance->rCSMD_HAL, FALSE, TRUE, (CSMD_ULONG) paulIntClearMask[0] );
  #if CSMD_MAX_SLAVES > 32
  {
    CSMD_USHORT usI;
    /* clear interrupt status of software svc */
    for (usI = 0; usI < (((prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves-1) / 32)+1); usI++)
    {
      prCSMD_Instance->rPriv.aulSVC_Int_Flags[usI] &= ~paulIntClearMask[usI];
    }
  }
  #endif
#else
  /* clear interrupt status of hardware svc */
  CSMD_HAL_ClearInterrupt( &prCSMD_Instance->rCSMD_HAL, FALSE, TRUE, 
                           (CSMD_ULONG) (paulIntClearMask[0] & ((1UL << CSMD_MAX_HW_CONTAINER)-1)) );
  #if CSMD_MAX_SLAVES > 32
  {
    CSMD_USHORT usI;
    /* clear interrupt status of software svc */
    for (usI = 0; usI < (((prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves-1) / 32)+1); usI++)    /*lint !e681: Loop is not entered */
    {
      prCSMD_Instance->rPriv.aulSVC_Int_Flags[usI] &= ~paulIntClearMask[usI];
    }
  }
  #endif
#endif
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_ClearSvchInt() */
/*lint -restore const! */



#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
/**************************************************************************/ /**
\brief Emulate the service channel state machine for slaves without IP-Core
       state machine.

\ingroup func_svcg
\b Description: \n
   This function is required, if more service channels than the number of
   available hardware service channels (CSMD_MAX_HW_CONTAINER) are to be used.
   It emulates the service channel handling if the application requests
   a service channel transmission via the atomic and/or macro functions.\n
   When the busy bit is deleted by the application requesting for software service 
   channel handling (i.e. when the slave index is more than the number of Hardware
   containers), this function needs to be called cyclically or (n * cycle time period) 
   in CP2 to CP4.
  
<B>Call Environment:</B> \n
   As of CP2, the CSMD_TxRxSoftCont function must be called cyclically
   or every (n * tScyc), as soon as a transmission is requested via at least one
   channel (usSoftSrvcCnt > 0).

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_ERROR \n

\author       ARK 
\date         07.10.2005
    
*******************************************************************************/
CSMD_FUNC_RET CSMD_TxRxSoftCont ( CSMD_INSTANCE *prCSMD_Instance )
{
  
  /* This function is not executed during communication phase switching process */
  if (prCSMD_Instance->rPriv.eMonitoringMode != CSMD_MONITORING_OFF)
  {
    CSMD_USHORT           usI;
    CSMD_USHORT           usCntrWord;     /* Local control word in service container for next step */
    CSMD_USHORT           usRdCntrWrd;    /* Copy of service channel control word from TX ram */
    CSMD_USHORT           usRdStatWrd;    /* Copy of service channel status word from RX ram  */
    CSMD_USHORT           usBusyTimeout;  /* Handshake timeout [Sercos cycles] */
    CSMD_USHORT           usHSTimeout;    /* Busy timeout [Sercos cycles] */
    CSMD_SERC3SVC        *prSvcCont;      /* Service container in local ram */
    CSMD_USHORT          *pusBuffer;      /* Write/read buffer in service container */
    CSMD_USHORT          *pusSvcTxRam;    /* Service channel in TX ram */
    CSMD_USHORT          *pusSvcRxRam;    /* Service channel in RX ram */
    CSMD_SOFT_SVC_STRUCT *prSoftSvc;      /* Software service channel variables */
    
    /* --------------------------------------------------------------------- */
    /* Prior to calling this function check prCSMD_Instance->usSoftSrvcCnt   */
    /* if the count is not zero this function should be called               */
    /* if the number of slaves is more than the Hardware containers process  */
    /* the Software container Control words                                  */
    /* --------------------------------------------------------------------- */
    for (usI = CSMD_MAX_HW_CONTAINER; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      prSoftSvc = &prCSMD_Instance->rPriv.parSoftSvc[usI-CSMD_MAX_HW_CONTAINER];

      if (prSoftSvc->usInUse == CSMD_SVC_CONTAINER_IN_USE)
      {
        CSMD_BOOL  boHandshake_OK  = FALSE;
        CSMD_BOOL  boBusy_OK       = FALSE;
        
        /* Get pointer to service container in local ram */
        prSvcCont   = prCSMD_Instance->rPriv.prSVContainer[usI];
        pusSvcTxRam = prCSMD_Instance->rPriv.pusTxRam + prSvcCont->usSVCTxPointer_Control/2;
        pusSvcRxRam = prCSMD_Instance->rPriv.pusRxRam + prSvcCont->usSVCRxPointer_Status/2;
        
        /* --------------------------------------------------------------------- */
        /* Copy the control and status word data from the TX and RX buffer slave */
        /* memory locations to local data, for checking the status of data flow  */
        /* --------------------------------------------------------------------- */
        usRdCntrWrd = CSMD_HAL_ReadShort( pusSvcTxRam );
        usRdStatWrd = CSMD_HAL_ReadShort( pusSvcRxRam );
        
        /* Increment HS time out if Control and Status word handshake bits */
        /* are not same */
        if (   ((usRdCntrWrd & CSMD_SVC_CTRL_HANDSHAKE ) != (usRdStatWrd & CSMD_SVC_STAT_HANDSHAKE))
            || !(usRdStatWrd & CSMD_SVC_STAT_VALID))
        {
          prSoftSvc->usHSTimeout++;
        }
        else
        {
          boHandshake_OK = TRUE;
        }
        
        /* Increment Busy time out if Busy bit is set in the Status word */
        if ((usRdStatWrd & CSMD_SVC_STAT_BUSY) == CSMD_SVC_STAT_BUSY)
        {
          prSoftSvc->usBusyTimeout++;
        }
        else
        {
          boBusy_OK = TRUE;
        }
        
        /* Check for a canceled request */
        /* A request is canceled if the channel is marked as not in use and not open */
        if (   (prCSMD_Instance->parSvchMngmtData[usI].usInUse != CSMD_SVC_CONTAINER_IN_USE)  /* Not in use respectively prepared for cancel request */
            && (prCSMD_Instance->parSvchMngmtData[usI].usChannelOpen == 0U))
        {
          prCSMD_Instance->parSvchMngmtData[usI].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
          /*set the M Busy bit */
          prSvcCont->rCONTROL.usWord[0] |= CSMD_SVC_CTRL_M_BUSY;
          
          /* Set the interrupt bit, mark the software service channel as not used and */
          /* decrement the software service container count*/
          prCSMD_Instance->rPriv.aulSVC_Int_Flags[usI / 32] |= (1UL << (usI - (32 * (usI /32))));
          prSoftSvc->usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
          prCSMD_Instance->usSoftSrvcCnt --;
          
          /* Copy the status word of the requested slave in RX ram to the local status word in service container */
          prSvcCont->rCONTROL.usWord[1] = usRdStatWrd;
        }
        
        /* --------------------------------------------------------------------- */
        /* Check whether the application using the Software container has set    */
        /* the Busy bit low the application either waits for interrupt or for    */
        /* high busy bit                                                         */
        /* Before copying processing the data check for status and control       */
        /* handshake bits if they are same check Busy bit in Control word        */
        /* if busy bit is low then process the control word for Service channel  */
        /* --------------------------------------------------------------------- */
        if (   !(prSvcCont->rCONTROL.usWord[0] & CSMD_SVC_CTRL_M_BUSY)
            &&  (boHandshake_OK)
            &&  (boBusy_OK)
            &&  ((usRdStatWrd & CSMD_SVC_STAT_ERROR) == (prSvcCont->rCONTROL.usWord[1] & CSMD_SVC_STAT_ERROR))
           )
        {
          /* Initialize, or next step necessary? */
          if ((prSvcCont->rCONTROL.usWord[0] & CSMD_SVC_CTRL_HANDSHAKE) != (usRdStatWrd & CSMD_SVC_STAT_HANDSHAKE))
          {
            /* Set the Busy and Handshake time out to zero */
            prSoftSvc->usHSTimeout   = 0U;
            prSoftSvc->usBusyTimeout = 0U;
            
            /* --------------------------------------------------------------------- */
            /* Copy the Data block element number to local control Word              */
            /* Copy Read / Write bit to local control word                           */
            /* Copy the handshake bit to local control word                          */
            /* --------------------------------------------------------------------- */
            usCntrWord = (CSMD_USHORT)((prSvcCont->rCONTROL.usWord[0] & (  CSMD_SVC_CTRL_ELEMENT_MASK
                                                                         | CSMD_SVC_CTRL_WRITE
                                                                         | CSMD_SVC_CTRL_HANDSHAKE) ));
            
            /* --------------------------------------------------------------------- */
            /* If the length of service channel data is less than service channel    */
            /* width + the number of cycles already used for this IDN then set the   */
            /* Set_End bit. Applicable for data lengths more than 4 bytes            */
            /* --------------------------------------------------------------------- */
                   /* Write Request */
            if (   (  (CSMD_USHORT)( (prSvcCont->rCONTROL.usWord[2] & 0x00FF) * 2 + CSMD_SVC_INFO_WIDTH )
                    >= prCSMD_Instance->parSvchMngmtData[usI].usLength)
                   /* Read Request */ 
                || (  (CSMD_USHORT)( ((prSvcCont->rCONTROL.usWord[3] & 0x00FF) - CSMD_SERC_SC_WRBUF_LENGTH) * 2 + CSMD_SVC_INFO_WIDTH )
                    >= prCSMD_Instance->parSvchMngmtData[usI].usLength)
               )
            {
              if (prSvcCont->rCONTROL.usWord[0] & CSMD_SVC_CTRL_SETEND)
              {
                usCntrWord |= CSMD_SVC_CTRL_LASTTRANS;
              }
            }
            /* ########----------------------------------------------------------------- */
            /* ######## Write Request handling ----------------------------------------- */
            /* ########----------------------------------------------------------------- */
            
            /* if Write operation is requested*/
            if (prSvcCont->rCONTROL.usWord[0] & CSMD_SVC_CTRL_WRITE)
            {
              /* Copy data from write buffer to the MDT info the size of data copied is */
              /* equal to the width of service channel */
              /* The following sequence must be changed, if CSMD_SVC_INFO_WIDTH is not equal to 4 ! */
              pusBuffer = (CSMD_USHORT *)&prSvcCont->sWR_BUFF[ prSvcCont->rCONTROL.usWord[2] & 0x00FF ];
              CSMD_HAL_WriteShort( (pusSvcTxRam+1), *pusBuffer++ );
              CSMD_HAL_WriteShort( (pusSvcTxRam+2), *pusBuffer );
              
              /* If length is more than the number of cycles + the service channel width */
              /* i.e. data length more than 4 bytes */
              if (  (CSMD_USHORT)( (prSvcCont->rCONTROL.usWord[2] & 0x00FF) * 2 + CSMD_SVC_INFO_WIDTH )
                  < prCSMD_Instance->parSvchMngmtData[usI].usLength)
              {
                /* Increment the cycle count with the width of the service channel */
                prSvcCont->rCONTROL.usWord[2] += ((CSMD_USHORT)(CSMD_SVC_INFO_WIDTH / 2));
                /* Toggle the handshake bit */
                prSvcCont->rCONTROL.usWord[0] ^= (CSMD_SHORT)CSMD_SVC_CTRL_HANDSHAKE;
              }
              /* else = End of Write operation reached for the requested IDN */
            }
            
            /* Copy the local control word to the control word of the requested slave in TX ram */
            CSMD_HAL_WriteShort( pusSvcTxRam, usCntrWord );
            
            /* Copy the status word of the requested slave in RX ram to the local status word in service container */
            prSvcCont->rCONTROL.usWord[1] = usRdStatWrd;
          }
          
          /* Error bit not set? */
          else if ((usRdStatWrd & CSMD_SVC_STAT_ERROR) != CSMD_SVC_STAT_ERROR)
          {
            /* --------------------------------------------------------------------- */
            /* Check if the busy bit is low                                          */
            /* Service container handshake bit and status word handshake bit         */
            /* should be equal and control word and status word handshake bits       */
            /* need to be equal                                                      */
            /* and busy bit in control word should be low                            */
            /* --------------------------------------------------------------------- */
            
            /* Set the Busy and Handshake time out to zero */
            prSoftSvc->usHSTimeout   = 0U;
            prSoftSvc->usBusyTimeout = 0U;
            
            /* ########----------------------------------------------------------------- */
            /* ######## Read Request handling ------------------------------------------ */
            /* ########----------------------------------------------------------------- */
            
            /* if Read operation is requested or element 1 is written */
            if (   !(prSvcCont->rCONTROL.usWord[0] & CSMD_SVC_CTRL_WRITE) 
                || ((prSvcCont->rCONTROL.usWord[0] & CSMD_SVC_CTRL_ELEMENT_MASK) == CSMD_SVC_CTRL_ELEMENT_IDN)
               )
            {
              if (  (CSMD_USHORT)( ((prSvcCont->rCONTROL.usWord[3] & 0x00FF) - CSMD_SERC_SC_WRBUF_LENGTH) * 2 + CSMD_SVC_INFO_WIDTH )
                  < prCSMD_Instance->parSvchMngmtData[usI].usLength)
              {
                pusBuffer = (CSMD_USHORT *)&prSvcCont->sRD_BUFF[ (prSvcCont->rCONTROL.usWord[3] & 0x00FF) - CSMD_SERC_SC_WRBUF_LENGTH ];
                *pusBuffer++ = CSMD_HAL_ReadShort( (pusSvcRxRam+1) );
                *pusBuffer   = CSMD_HAL_ReadShort( (pusSvcRxRam+2) );
                
                prSvcCont->rCONTROL.usWord[3] += ((CSMD_USHORT)(CSMD_SVC_INFO_WIDTH / 2));
                prSvcCont->rCONTROL.usWord[0] ^= (CSMD_SHORT)CSMD_SVC_CTRL_HANDSHAKE;
              }
              else
              {
                pusBuffer = (CSMD_USHORT *)&prSvcCont->sRD_BUFF[ (prSvcCont->rCONTROL.usWord[3] & 0x00FF) - CSMD_SERC_SC_WRBUF_LENGTH ];
                *pusBuffer++ = CSMD_HAL_ReadShort( (pusSvcRxRam+1) );
                *pusBuffer   = CSMD_HAL_ReadShort( (pusSvcRxRam+2) );
                
                /* End of data transfer for requested IDN reached */
                /* Set the Busy bit to indicate control has been transferred to application */
                prSvcCont->rCONTROL.usWord[0] |= CSMD_SVC_CTRL_M_BUSY;
                
                /* For CSMD internal functions priority is set to 1 if it is not CSMD internal */
                /* function then set the bit for the requested slave (index) decrement the     */
                /* software service container count */
                prCSMD_Instance->rPriv.aulSVC_Int_Flags[usI / 32] |= (1UL << (usI - (32 * (usI /32))));
                prSoftSvc->usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
                prCSMD_Instance->usSoftSrvcCnt --;
              }
            }
            /* Write operation completed */
            else 
            {
              /*set the M Busy bit */
              if ((prSvcCont->rCONTROL.usWord[0] & CSMD_SVC_CTRL_ELEMENT_MASK) != CSMD_SVC_CTRL_ELEMENT_IDN)
              {
                prSvcCont->rCONTROL.usWord[0] |= CSMD_SVC_CTRL_M_BUSY;
              }
              /* For CSMD internal functions priority is set to 1 if it is not CSMD internal */
              /* function then set the bit for the requested slave (index) decrement the     */
              /* software service container count */
              prCSMD_Instance->rPriv.aulSVC_Int_Flags[usI / 32] |= (1UL << (usI - (32 * (usI /32))));
              prSoftSvc->usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
              prCSMD_Instance->usSoftSrvcCnt --;
            }
            
            /* Copy the status word of the requested slave in RX ram to the local status word in service container */
            prSvcCont->rCONTROL.usWord[1] = usRdStatWrd;
          }
        }
        
        /* ########----------------------------------------------------------------- */
        /* ######## Error Condition handling --------------------------------------- */
        /* ########----------------------------------------------------------------- */
        
        /* --------------------------------------------------------------------- */
        /* Copy the control and status word data from the TX and RX buffer slave */
        /* memory locations to local data for checking the status of data flow   */
        /* --------------------------------------------------------------------- */
        usRdCntrWrd = CSMD_HAL_ReadShort( pusSvcTxRam );
        usRdStatWrd = CSMD_HAL_ReadShort( pusSvcRxRam );
        
        /* Busy time out value = SVCCSR bits 24 to 31 */
        /* HS time out value = SVCCSR bits 16 to 23 */
        usHSTimeout   = prCSMD_Instance->rPriv.usSVC_HS_Timeout;          /* SVC handshake timeout [Sercos cycles] */
        usBusyTimeout = prCSMD_Instance->rPriv.usSVC_BUSY_Timeout;        /* SVC busy timeout [Sercos cycles] */
        
        /* --------------------------------------------------------------------- */
        /* If error bit is set then set the error in service container control   */
        /* word. If busy time out count is more than the set count when Busy bit */
        /* is set or when the handshake are not same and hand shake time out     */
        /* has occured.                                                          */
        /* --------------------------------------------------------------------- */
        
        if (   (    ((usRdStatWrd & CSMD_SVC_STAT_BUSY)      != CSMD_SVC_STAT_BUSY)   /* if busy not set */
                &&  ((usRdStatWrd & CSMD_SVC_STAT_ERROR)     == CSMD_SVC_STAT_ERROR) 
                &&  ((usRdCntrWrd & CSMD_SVC_CTRL_HANDSHAKE) == (usRdStatWrd & CSMD_SVC_STAT_HANDSHAKE)) 
                && !(prSvcCont->rCONTROL.usWord[0] & CSMD_SVC_CTRL_M_BUSY)
               )
            || (   (boBusy_OK == FALSE)
                && (usBusyTimeout < prSoftSvc->usBusyTimeout)
               )
            || (   (boHandshake_OK == FALSE)
                && (usHSTimeout < prSoftSvc->usHSTimeout)
               )
           )
        {
          /* General Error when error bit is set */
          if ((usRdStatWrd & CSMD_SVC_STAT_ERROR) == CSMD_SVC_STAT_ERROR)
          {
            prSvcCont->rCONTROL.usWord[0] |= CSMD_SVC_CTRL_INT_ERR;
            /* Copy the Error code from AT info to Read buffer */
            pusBuffer = (CSMD_USHORT *)&prSvcCont->sRD_BUFF[ (prSvcCont->rCONTROL.usWord[3] & 0x00FF) - CSMD_SERC_SC_WRBUF_LENGTH ];
            *pusBuffer++ = CSMD_HAL_ReadShort( (pusSvcRxRam+1) );
            *pusBuffer   = CSMD_HAL_ReadShort( (pusSvcRxRam+2) );
            
            prSvcCont->rCONTROL.usWord[3] += ((CSMD_USHORT)(CSMD_SVC_INFO_WIDTH / 2));
          }
          /* Busy time out error */
          else if (usBusyTimeout < prSoftSvc->usBusyTimeout)
          {
            prSvcCont->rCONTROL.usWord[4] |= CSMD_SVC_CTRL_INT_BUSY_TIMEOUT;
          }
          /* Handshake time out error */
          else if (usHSTimeout < prSoftSvc->usHSTimeout)
          {
            prSvcCont->rCONTROL.usWord[4] |= CSMD_SVC_CTRL_INT_HS_TIMEOUT;
          }
          else /* when to get it*/
          {
            prSvcCont->rCONTROL.usWord[4] |= CSMD_SVC_CTRL_INT_SC_ERR;
          }
          
          prSvcCont->rCONTROL.usWord[0] |= CSMD_SVC_CTRL_M_BUSY;
          
          /* For CSMD internal functions priority is set to 1 if it is not CSMD internal */
          /* function then set the bit for the requested slave (index) decrement the     */
          /* software service container count */
          prCSMD_Instance->rPriv.aulSVC_Int_Flags[usI / 32] |= (1UL << (usI - (32 * (usI /32))));
          prSoftSvc->usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
          prCSMD_Instance->usSoftSrvcCnt --;
        }
        
        /* Copy the status word of the requested slave in RX ram to the local status word in svc container */
        prSvcCont->rCONTROL.usWord[1] = usRdStatWrd;
        
      } /* End: if (prSoftSvc->usInUse == CSMD_SVC_CONTAINER_IN_USE) */
    } /* End: for (usI = CSMD_MAX_HW_CONTAINER; ... */
  } /* if (prCSMD_Instance->rPriv.eMonitoringMode != CSMD_MONITORING_OFF) */
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_TxRxSoftCont() */
#endif /* #if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER */

/*! \endcond */ /* PUBLIC */



/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/*! \endcond */ /* PRIVATE */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
19 Mar 2014 WK
  - Defdb00168400
    CSMD_TxRxSoftCont()
    Added check for a canceled request. A request is canceled if the channel
    is marked as not in use and not open.
02 Apr 2014 WK
  - Defdb00168915
    CSMD_CheckSvchInt(), CSMD_ClearSvchInt()
    Fixed number of loop passes for CSMD_MAX_DEVICE is divisible by 64
    without remainder.
16 Jan 2015 WK
  - Defdb00175145
    CSMD_TxRxSoftCont()
    Check for a canceled request:
    Cancel request, if usInUse == 2 and clear usInUse so that the function
    CSMD_CheckSVCHInUse() is no longer blocked.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
  
------------------------------------------------------------------------------
*/
