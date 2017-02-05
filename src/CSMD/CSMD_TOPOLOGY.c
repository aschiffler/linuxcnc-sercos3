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
\file   CSMD_TOPOLOGY.c
\author WK
\date   01.09.2010
\brief  This File contains the private functions 
        which provides topology information.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#include "CSMD_CP_AUX.h"

#define SOURCE_CSMD
#include "CSMD_TOPOLOGY.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */


/**************************************************************************/ /**
\brief Returns the active port for the selected slave.

\ingroup module_redundancy
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This function can be called as of CP1 from either an interrupt or a task.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usSlaveIdx
              Slave index of the slave

\return       Active port for the selected slave \n
              0 --> Slave active on Port 1 \n
              1 --> Slave active on Port 2 \n
             -1 --> no valid data

\author       WK
\date         16.02.2011

***************************************************************************** */
/*lint -save -e818 const!*/
CSMD_SHORT CSMD_ActiveSlavePort( CSMD_INSTANCE *prCSMD_Instance,
                                 CSMD_USHORT    usSlaveIdx )
{
  if (prCSMD_Instance->rPriv.ausPrefPortBySlave[usSlaveIdx] == CSMD_PORT_1)
  {
    /* Slave is assigned to port 1 */
    if (TRUE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP1)
    {
      return (0);
    }
    else
    {
      return (-1);
    }
  }
  else
  {
    /* Slave is assigned to port 2 */
    if (TRUE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP2)
      return (1);
    else
      return (-1);
  }

}  /* end: CSMD_ActiveSlavePort() */
/*lint -restore const!*/


/*! \endcond */ /* PUBLIC */



/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Selects the requested topology in C-DEV and toggles the
       topology change command bit.
  
\ingroup module_redundancy
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usDevTopology
              Topology to be set\n
              00 --> Fast-Forward on both ports\n
              01 --> Loopback with Forward of P-Telegrams\n
              10 --> Loopback with Forward of S-Telegrams\n
              11 --> reserved
\param [in]   usSlaveIdx
              Slave index for access of correct S-DEV and C-DEV in the list

\return       none
 
\author       KP
\date         25.01.2008
 
***************************************************************************** */
CSMD_VOID CSMD_SetC_DevTopology( CSMD_INSTANCE *prCSMD_Instance,
                                 CSMD_USHORT    usDevTopology,
                                 CSMD_USHORT    usSlaveIdx )
{
  
  CSMD_USHORT usTempC_DevWord;

  /* read current C-DEV */
  usTempC_DevWord = prCSMD_Instance->rPriv.ausDevControl[usSlaveIdx];

  usTempC_DevWord &= (CSMD_USHORT)~CSMD_C_DEV_CMD_TOPOLOGY_MASK;
  usTempC_DevWord |= usDevTopology; 
  
  /* toggle the bit and write back in C-DEV */
  if (usTempC_DevWord & CSMD_C_DEV_TOPOLOGY_HS)
  {
    usTempC_DevWord &= (CSMD_USHORT)~CSMD_C_DEV_TOPOLOGY_HS;
  }
  else
  {
    usTempC_DevWord |= CSMD_C_DEV_TOPOLOGY_HS;
  }
  
  /* Change Slave topology */
  prCSMD_Instance->rPriv.ausDevControl[usSlaveIdx] = usTempC_DevWord;

}  /* end: CSMD_SetC_DevTopology */


/* ------------------------------------------------------------------

      |-----------------------------------------------------------|
      | Recognized telegram on | Recognized topology              |
      |   port 1   |  port 2   |                                  |
      |------------|-----------|----------------------------------|
      |     P      |     -     | Line on port 1                   |
      |     -      |     S     | Line in port 2                   |
      |     P      |     S     | Broken ring                      |
      |            |           |                                  |
      |     S      |     P     | Ring                             |
      |     S      |     S     | Ring  (P channel defective)      |
      |     P      |     P     | Ring  (S channel defective)      |
      |     -      |     -     | Inactive                         |
      |------------|-----------|----------------------------------|

   ------------------------------------------------------------------ */


/**************************************************************************/ /**
\brief Checks the current topology and generates an error message if the
       topology has changed.

\ingroup module_cyclic
\b Description: \n
   This function returns "CSMD_CHANGE_TOPOLOGY" if the topology has changed.
   This is validated by evaluating the registers TGSR1 and TGSR2.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       \ref CSMD_NO_LINK_ATTACHED \n
              \ref CSMD_NO_TELEGRAMS_RECEIVED \n
              \ref CSMD_MST_MISS \n
              \ref CSMD_MST_WINDOW_ERROR \n
              \ref CSMD_TEL_ERROR_OVERRUN \n
              \ref CSMD_TOPOLOGY_CHANGE \n
              \ref CSMD_NO_ERROR \n
 
\author       KP
\date         04.01.2008 

***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckTopology( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_FUNC_RET eFuncRet;
  CSMD_USHORT   usTempTopology = prCSMD_Instance->usCSMD_Topology;

#ifndef CSMD_SOFT_MASTER
  CSMD_USHORT   usLineStatus;
  CSMD_ULONG    ulDFCSR;
  CSMD_USHORT   usSlaveIdx;

  usLineStatus = (CSMD_USHORT) CSMD_HAL_GetLineStatus( &prCSMD_Instance->rCSMD_HAL );

  /* ------------------------------------- */
  /* check if telegram errors occurred     */
  /* ------------------------------------- */
  if (CSMD_NO_ERROR != (eFuncRet = CSMD_Check_Telegram_Errors ( prCSMD_Instance) ) )
  {
    /* increment error counter for consecutive telegram errors.
     * MST window error is not critical as CoSeMa considers MSTs received outside
     * the appropriate window as valid */
    if (eFuncRet != CSMD_MST_WINDOW_ERROR)
    {
      prCSMD_Instance->rPriv.usTelErrCnt++;
    }

    /* check if maximum for successive telegram errors has been exceeded */
    if (prCSMD_Instance->rPriv.usTelErrCnt >
        prCSMD_Instance->rConfiguration.rMasterCfg.usMaxNbrTelErr)
    {
      return(CSMD_TEL_ERROR_OVERRUN);
    }
  }
  else
  {
    /* reset counter for consecutive telegram errors */
    prCSMD_Instance->rPriv.usTelErrCnt = 0;
  }

  switch (usLineStatus)
  {
  case 0U:    /* Line Status: both ports no error on line */
    /* ------------------------------------------------------------------------ */
    /*  CHECK TOPOLOGY FOR RING                                                 */
    /* ------------------------------------------------------------------------ */
    /* secondary telegram on port 1 and primary telegram on port 2 */
    if (   (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boSecTelP1)
        && (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boPriTelP2))
    {
      prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_RING;
      prCSMD_Instance->rPriv.rRedundancy.usRingDefect = 0;
      prCSMD_Instance->boRingRedundant = TRUE;
    }
    /* secondary telegram on port 1 and no telegram on port 2
       or primary telegram on port 2 and no telegram on port 1 */
    else if (   (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
             && (    (    (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boSecTelP1)
                       && (FALSE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP2))
                 ||  (    (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boPriTelP2)
                       && (FALSE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP1)) ) )
    {
      /* ring with undetected single wire break between two slaves */
      prCSMD_Instance->boRingRedundant = FALSE;
    }

    /* ------------------------------------------------------------------------ */
    /*  CHECK TOPOLOGY FOR BROKEN RING                                          */
    /* ------------------------------------------------------------------------ */
    /* primary telegram on port 1 and secondary telegram on port 2 */
    else if (   (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP1)
             && (TRUE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP2))
    {
      prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_BROKEN_RING;
      prCSMD_Instance->rPriv.rRedundancy.usRingDefect = 0;
    }

    /* ------------------------------------------------------------------------ */
    /*  CHECK TOPOLOGY FOR DEFECT RING                                          */
    /* ------------------------------------------------------------------------ */
    else if (   prCSMD_Instance->rPriv.boP1_active
             && prCSMD_Instance->rPriv.boP2_active )
    {
      /* ------------------------------------------------------------------------ */
      /*  CHECK TOPOLOGY FOR DEFECT RING ON PRIMARY LINE                          */
      /* ------------------------------------------------------------------------ */
      /* secondary telegram on port 1 and secondary telegram on port 2 */
      /* Ring with defect on Primary line */
      if (   (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boSecTelP1)
          && (   (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boSecTelP2)
              || (FALSE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP2) ) )
      {
        /* if the ring defect point is detected at master port, the DFCSR topology is set to line topology */
        ulDFCSR = CSMD_HAL_GetComMode( &prCSMD_Instance->rCSMD_HAL);
        if ( ulDFCSR == CSMD_HAL_DFCSR_TOPOLOGY_RT_P2_MODE )
        {
          prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_LINE_P2;
        }
        else
        {
          prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_DEFECT_RING;
          prCSMD_Instance->rPriv.rRedundancy.usRingDefect = CSMD_RING_DEF_PRIMARY;
        }
      }

      /* ------------------------------------------------------------------------ */
      /*  CHECK TOPOLOGY FOR DEFECT RING ON SECONDARY LINE                        */
      /* ------------------------------------------------------------------------ */
      /* primary telegram on port 1 and primary telegram on port 2 */
      /* Ring with defect on Secondary line */
      if (   (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP2)
          && (   (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boPriTelP1)
              || (FALSE == prCSMD_Instance->rPriv.rRedundancy.boNewDataP1) ) )
      {
        /* if the ring defect point is detected at master port, the DFCSR topology is set to line topology */
        ulDFCSR = CSMD_HAL_GetComMode( &prCSMD_Instance->rCSMD_HAL);
        if ( ulDFCSR == CSMD_HAL_DFCSR_TOPOLOGY_RT_P1_MODE )
        {
          prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_LINE_P1;
        }
        else
        {
          prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_DEFECT_RING;
          prCSMD_Instance->rPriv.rRedundancy.usRingDefect = CSMD_RING_DEF_SECONDARY;
        }
      }
    }
    break;

  case 1U:    /* Line Status: Error on line port 1, no error on line port 2 */
    /* ------------------------------------------------------------------------ */
    /*  CHECK TOPOLOGY FOR LINE AT P2                                           */
    /* ------------------------------------------------------------------------ */
    if (prCSMD_Instance->rPriv.rRedundancy.boNewDataP2 == TRUE)
    {
      prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_LINE_P2;
      prCSMD_Instance->rPriv.rRedundancy.usRingDefect = 0;

      if (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP2)
      {
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[
                    prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[
                      prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb - 1]];

        CSMD_SetC_DevTopology( prCSMD_Instance,
                               CSMD_C_DEV_CMD_LOOPB_FW_S_TEL,
                               usSlaveIdx );
      }

      /* reset line status bits 20 and 21 in DFCSR */
      CSMD_HAL_ResetLineStatus (&prCSMD_Instance->rCSMD_HAL);
    }
    break;

  case 2U:    /* Line Status: No error on line port 1, error on line port 2 */
    /* ------------------------------------------------------------------------ */
    /*  CHECK TOPOLOGY FOR LINE AT P1                                           */
    /* ------------------------------------------------------------------------ */
    if (prCSMD_Instance->rPriv.rRedundancy.boNewDataP1 == TRUE)
    {
      prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_LINE_P1;
      prCSMD_Instance->rPriv.rRedundancy.usRingDefect = 0;

      if (TRUE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP1)
      {
        usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[
                    prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[
                      prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb - 1]];

        CSMD_SetC_DevTopology( prCSMD_Instance,
                               CSMD_C_DEV_CMD_LOOPB_FW_P_TEL,
                               usSlaveIdx );
      }

      /* reset line status bits 20 and 21 in DFCSR */
      CSMD_HAL_ResetLineStatus (&prCSMD_Instance->rCSMD_HAL);
    }
    break;

  default:            /* Line Status: Both ports with error on line */
    /* ------------------------------------------------------------------------ */
    /*  NO LINK AT PORT 1 AND PORT 2                                            */
    /* ------------------------------------------------------------------------ */
    CSMD_HAL_ResetLineStatus (&prCSMD_Instance->rCSMD_HAL);

    prCSMD_Instance->rPriv.boP1_active = FALSE;
    prCSMD_Instance->rPriv.boP2_active = FALSE;
    prCSMD_Instance->usCSMD_Topology = CSMD_NO_LINK;
    prCSMD_Instance->rPriv.rRedundancy.usRingDefect = 0;

    return (CSMD_NO_LINK_ATTACHED);     /* No link active */
  }

#else

  /* ------------------------------------- */
  /* check if telegram errors occurred     */
  /* ------------------------------------- */
  if (CSMD_NO_ERROR != (eFuncRet = CSMD_Check_Telegram_Errors ( prCSMD_Instance) ) )
  {
    /* increment error counter for consecutive telegram errors.
     * MST window error is not critical as CoSeMa considers MSTs received outside
     * the appropriate window as valid */
    if (eFuncRet != CSMD_MST_WINDOW_ERROR)
    {
      prCSMD_Instance->rPriv.usTelErrCnt++;
    }

    /* check if maximum for successive telegram errors has been exceeded */
    if (prCSMD_Instance->rPriv.usTelErrCnt >
        prCSMD_Instance->rConfiguration.rMasterCfg.usMaxNbrTelErr)
    {
      return(CSMD_TEL_ERROR_OVERRUN);
    }
  }
  else
  {
    /* reset counter for consecutive telegram errors */
    prCSMD_Instance->rPriv.usTelErrCnt = 0;
  }

  /* ------------------------------------------------------------------------ */
  /*  CHECK TOPOLOGY FOR RING                                                 */
  /* ------------------------------------------------------------------------ */
  /* secondary telegram on port 1 and primary telegram on port 2 */
  if (   (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boSecTelP1)
      && (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boPriTelP2))
  {
    prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_RING;
    prCSMD_Instance->rPriv.rRedundancy.usRingDefect = 0;
    prCSMD_Instance->boRingRedundant = TRUE;
  }

  /* ------------------------------------------------------------------------ */
  /*  CHECK TOPOLOGY FOR BROKEN RING                                          */
  /* ------------------------------------------------------------------------ */
  /* primary telegram on port 1 and secondary telegram on port 2 */
  else if (   (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP1)
           && (TRUE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP2))
  {
    prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_BROKEN_RING;
    prCSMD_Instance->rPriv.rRedundancy.usRingDefect = 0;
  }

  /* ------------------------------------------------------------------------ */
  /*  CHECK TOPOLOGY FOR LINE AT P1                                           */
  /* ------------------------------------------------------------------------ */
  else if (   (prCSMD_Instance->rPriv.rRedundancy.boNewDataP1 == TRUE)
           && (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP1) )
  {
    prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_LINE_P1;
    prCSMD_Instance->rPriv.rRedundancy.usRingDefect = 0;
  }

  /* ------------------------------------------------------------------------ */
  /*  CHECK TOPOLOGY FOR LINE AT P2                                           */
  /* ------------------------------------------------------------------------ */
  else if (   (prCSMD_Instance->rPriv.rRedundancy.boNewDataP2 == TRUE)
           && (TRUE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP2) )
  {
    prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_LINE_P2;
    prCSMD_Instance->rPriv.rRedundancy.usRingDefect = 0;
  }

  /* ------------------------------------------------------------------------ */
  /*  CHECK TOPOLOGY FOR DEFECT RING                                          */
  /* ------------------------------------------------------------------------ */
  else if (   prCSMD_Instance->rPriv.boP1_active
           && prCSMD_Instance->rPriv.boP2_active )
  {
    /* ------------------------------------------------------------------------ */
    /*  CHECK TOPOLOGY FOR DEFECT RING ON PRIMARY LINE                          */
    /* ------------------------------------------------------------------------ */
    /* secondary telegram on port 1 and secondary telegram on port 2 */
    /* Ring with defect on Primary line */
    if (   (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boSecTelP1)
        && (   (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boSecTelP2)
            ) )
    {
      prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_DEFECT_RING;
      prCSMD_Instance->rPriv.rRedundancy.usRingDefect = CSMD_RING_DEF_PRIMARY;
    }

    /* ------------------------------------------------------------------------ */
    /*  CHECK TOPOLOGY FOR DEFECT RING ON SECONDARY LINE                        */
    /* ------------------------------------------------------------------------ */
    /* primary telegram on port 1 and primary telegram on port 2 */
    /* Ring with defect on Secondary line */
    if (   (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP2)
        && (   (TRUE  == prCSMD_Instance->rPriv.rRedundancy.boPriTelP1)
            ) )
    {
      prCSMD_Instance->usCSMD_Topology = CSMD_TOPOLOGY_DEFECT_RING;
      prCSMD_Instance->rPriv.rRedundancy.usRingDefect = CSMD_RING_DEF_SECONDARY;
    }
  }

  else
  {

  /* ------------------------------------------------------------------------ */
  /*  NO LINK AT PORT 1 AND PORT 2                                            */
  /* ------------------------------------------------------------------------ */
  CSMD_HAL_ResetLineStatus (&prCSMD_Instance->rCSMD_HAL);

  prCSMD_Instance->rPriv.boP1_active = FALSE;
  prCSMD_Instance->rPriv.boP2_active = FALSE;
  prCSMD_Instance->usCSMD_Topology = CSMD_NO_LINK;
  prCSMD_Instance->rPriv.rRedundancy.usRingDefect = 0;

  return (CSMD_NO_LINK_ATTACHED);     /* No link active */
  }

#endif // #ifndef CSMD_SOFT_MASTER (#else)

  /* compare old topology with current topology, topology change detected? */
  if (usTempTopology != prCSMD_Instance->usCSMD_Topology)
  {
#ifdef CSMD_HOTPLUG
    if (usTempTopology == CSMD_TOPOLOGY_LINE_P1)
    {
      if (prCSMD_Instance->rPriv.rHP_P2_Struct.rHpField_MDT0.rControl.usWord & CSMD_HP_CNTRL_ENABLED)
      {
        prCSMD_Instance->rPriv.boP2_active = TRUE;
        return (eFuncRet);
      }
    }
    else if (usTempTopology == CSMD_TOPOLOGY_LINE_P2)
    {
      if (prCSMD_Instance->rPriv.rHP_P1_Struct.rHpField_MDT0.rControl.usWord & CSMD_HP_CNTRL_ENABLED)
      {
        prCSMD_Instance->rPriv.boP1_active = TRUE;
        return (eFuncRet);
      }
    }
#endif

#ifdef CSMD_HW_SVC_REDUNDANCY
    if (FALSE == prCSMD_Instance->rPriv.boHW_SVC_Redundancy)
#endif
    {
      CSMD_HAL_CtrlSVCMachine( &prCSMD_Instance->rCSMD_HAL, FALSE );
    }

    /* perform line break management */
    (CSMD_VOID)CSMD_LineBreakMngmnt (prCSMD_Instance);

    /* reorganize service channel port assignment */
    CSMD_SVCMngmnt4LineBreak (prCSMD_Instance);

    if (*prCSMD_Instance->rPriv.rCbFuncTable.S3EventFromISR != NULL)
    {
      if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
      {
        /* Notification to IP-Driver about topology change to physically closed ring          */
        prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_RING_CLOSED;     /* Event Id */
      }
      else
      {
        /* Notification to IP-Driver about topology change to physically opened ring          */
        prCSMD_Instance->rPriv.rSercEvent.eEventId = CSMD_SIII_RING_BREAK;      /* Event Id */
      }

      (CSMD_VOID)(*prCSMD_Instance->rPriv.rCbFuncTable.S3EventFromISR)
        ( prCSMD_Instance->rPriv.pvCB_Info,         /* distinguish the instance */
          &prCSMD_Instance->rPriv.rSercEvent );     /* Sercos Event information */
    }

    eFuncRet = CSMD_TOPOLOGY_CHANGE;
  }

#ifdef CSMD_HOTPLUG
  if (prCSMD_Instance->rPriv.rHotPlug.boHotPlugActive != TRUE)
#endif
  {
    CSMD_SetCollisionBuffer(prCSMD_Instance);
  }

  return (eFuncRet);
  
} /* end: CSMD_CheckTopology() */



/**************************************************************************/ /**
\brief Locates the position where the Sercos ring has a defect.

\ingroup module_redundancy
\b Description: \n
   This function searches for the first slave on the faultless line which has not
   entered a value into its S-DEV and thus determines the only slave which is in
   loopback and forward mode. This information is required to allow ring recovery
   with defect ring topology.
   
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       none

\author       KP
\date         09.01.2008 

***************************************************************************** */
CSMD_VOID CSMD_SearchLineDefectPoint( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT  usDevStatus;
  CSMD_USHORT  usI;
  CSMD_USHORT  usSlaveAddr;
  CSMD_USHORT  usSlaveIdx;
  CSMD_USHORT  usBuf;


  if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_PRIMARY)
  {
    usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2];

    /* Check all slaves connected to port 2 */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
    {
      /* Get Sercos address and index of the slave */
      usSlaveAddr = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI];
      usSlaveIdx  = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr];

      usDevStatus   = CSMD_HAL_ReadShort( prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx][CSMD_PORT_2][usBuf] );

      if (!(usDevStatus & CSMD_S_DEV_SLAVE_VALID))
      {
        /* slave valid not set => previous slave is considered to be in loopback and forward of S-telegrams topology */
        if (usI > 0)
        {
          prCSMD_Instance->usSercAddrLastSlaveP2 = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI - 1];
        }
      }
    }
    /* every slave returned slave valid to port 2 =>
     * slave directly connected to master port 1 is considered to be in loopback and forward of S-telegrams topology */
    if (prCSMD_Instance->usSercAddrLastSlaveP2 == 0)
    {
      prCSMD_Instance->usSercAddrLastSlaveP2 = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI - 1];
    }
  }
  else  /* CSMD_RING_DEF_SECONDARY */
  {
    usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1];

    /* Check all slaves connected to port 1 */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
    {
      /* Get Sercos address and index of the slave */
      usSlaveAddr = prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI];
      usSlaveIdx  = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[usSlaveAddr];

      usDevStatus = CSMD_HAL_ReadShort( prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx][CSMD_PORT_1][usBuf] );

      if (!(usDevStatus & CSMD_S_DEV_SLAVE_VALID))
      {
        /* First slave found without S-DEV information.
           Return Sercos address of first slave found with topology loopback and forward with P-telegrams */
        if (usI > 0)
        {
          prCSMD_Instance->usSercAddrLastSlaveP1 = prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI - 1];
        }
      }
    }
    /* every slave returned slave valid to port 1 =>
     * slave directly connected to master port 2 is considered to be in loopback and forward of P-telegrams topology */
    if (prCSMD_Instance->usSercAddrLastSlaveP1 == 0)
    {
      prCSMD_Instance->usSercAddrLastSlaveP1 = prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI - 1];
    }
  }
}/* end: CSMD_SearchLineDefectPoint() */



/**************************************************************************/ /**
\brief Detects the last slaves on both ports if the topology is changed
       from ring to broken ring.
       
\ingroup module_redundancy
\b Description: \n
   This function adjusts the lists of available slaves. The slave index and
   Sercos address of the last slave at the end of both lines are stored in
   the CoSeMa structure.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       none

\author       KP
\date         09.01.2008 

\b Modifications

  - 28.03.2011 / WK - Revised and optimized. Fixed lint error.

***************************************************************************** */
CSMD_VOID CSMD_SearchLineBreakPoint( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT  usI;             /* Count variable                   */
  CSMD_USHORT  usIdx;           /* Index into available slave lists */
  CSMD_USHORT  usSlaveIdx;      /* Slave Index                      */
  CSMD_USHORT  usDeviceStatus;
  CSMD_USHORT  usBuf;

  usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1];

  /* initial index is 1 because at least one slave is connected to master port 1 in broken ring topology */
  for (usIdx = 1; usIdx < prCSMD_Instance->rSlaveList.usNumProjSlaves; usIdx++)
  {
    usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[
                 prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usIdx]];

    usDeviceStatus = CSMD_HAL_ReadShort( prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx][CSMD_PORT_1][usBuf] );

    if (!(usDeviceStatus & CSMD_S_DEV_SLAVE_VALID))
    {
      /* usIdx = index of first slave with slave valid not set in device status at port 1 */
      break;
    }
  }
  /* Refresh the available slave list for port 1 */
  for (usI = usIdx; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
  {
    prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI] = 0U;
  }
  
  /* Adjust number of slaves at port 1 */
  prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb  = usIdx;
  /* Sercos address of last slave at port 1 */
  prCSMD_Instance->usSercAddrLastSlaveP1 = prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usIdx-1];

  usBuf = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2];

  /* initial index is 1 because at least one slave is connected to master port 2 in broken topology */
  for (usIdx = 1; usIdx < prCSMD_Instance->rSlaveList.usNumProjSlaves; usIdx++)
  {
    usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[
                 prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usIdx]];

    usDeviceStatus = CSMD_HAL_ReadShort( prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx][CSMD_PORT_2][usBuf] );

    if (!(usDeviceStatus & CSMD_S_DEV_SLAVE_VALID))
    {
      /* usIdx = index of first slave with slave valid not set in device status at port 2 */
      break;
    }
  }

  /* Refresh the available slave list for port 2 */
  for (usI = usIdx; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
  {
    prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI] = 0U;
  }
  
  /* Adjust number of slaves at port 2 */
  prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb = usIdx;
  /* Sercos address of last slave at port 2 */
  prCSMD_Instance->usSercAddrLastSlaveP2 = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usIdx-1];

}/* end: CSMD_SearchLineBreakPoint() */



/**************************************************************************/ /**
\brief Manages the active pointers and handles new topology addresses
       in case of a topology change.

\ingroup module_redundancy
\b Description: \n
   Based on the new topology, this function manages the pointers for the active
   ports and finds out the topology addresses of slaves between which the ring is broken.
     
<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
   If the topology has changed, CSMD_LineBreakMngmnt() is called by CSMD_CheckTopology().

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
 
\return       \ref CSMD_LINE_BREAK_ERROR \n
              \ref CSMD_NO_ERROR \n
 
\author       KP
\date         12.12.2007 

***************************************************************************** */
CSMD_FUNC_RET CSMD_LineBreakMngmnt( CSMD_INSTANCE *prCSMD_Instance )
{

  CSMD_USHORT  usH;
  CSMD_USHORT  usI;
  CSMD_USHORT  usSlaveIdx;
  CSMD_USHORT  usNumUsedSlaves = 0; /* Lint requires initialization */

  /* ------------------------------------------------------------------------ */
  /* Management for RING TOPOLOGY                                             */
  /* ------------------------------------------------------------------------ */
  
  /* recover Ring */
  if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
  {
    /* count number of slaves currently active and set preferred port to port 1 */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves; usI++)
    {
      if (prCSMD_Instance->rSlaveList.aeSlaveActive[usI] > CSMD_SLAVE_INACTIVE)
      {
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] = CSMD_PORT_1;
        usNumUsedSlaves++;
      }
    }
    /* update list of available slaves on port 1 */
    for (usH = 0; usH < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usH++)
    {
      prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb + usH] =
        prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[(prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb - 1U) - usH];
    }
    prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb = usNumUsedSlaves;

    /* update list of available slaves on port 2 (reverse order of port 1) */
    for (usH = 0; usH < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usH++)
    {
      prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usH] =
        prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[(prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb - 1U) - usH];
    }
    prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb = prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb;

    prCSMD_Instance->usSercAddrLastSlaveP2 = 0U;
    prCSMD_Instance->usSercAddrLastSlaveP1 = 0U;

    prCSMD_Instance->rPriv.boP1_active = TRUE;
    prCSMD_Instance->rPriv.boP2_active = TRUE;
    
  }
  
  /* ------------------------------------------------------------------------ */
  /* Management for BROKEN RING TOPOLOGY                                      */
  /* ------------------------------------------------------------------------ */
  
  /* Topology was RING before change */
  else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING)
  {
    /* search for break point, refresh rSlaveAvailable list and get last Slaves at the end of line */
    CSMD_SearchLineBreakPoint(prCSMD_Instance);

    /* refresh addresses in the redundancy-topology list for port 1 */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
    {
      usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[
                   prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI]];
      prCSMD_Instance->rPriv.ausPrefPortBySlave[usSlaveIdx] = CSMD_PORT_1;
    }

    /* refresh addresses in the redundancy-topology list for port 2 */
    for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
    {
      usSlaveIdx = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[
                   prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI]];
      prCSMD_Instance->rPriv.ausPrefPortBySlave[usSlaveIdx] = CSMD_PORT_2;
    }

    prCSMD_Instance->rPriv.boP1_active = TRUE;
    prCSMD_Instance->rPriv.boP2_active = TRUE;
    
  }   /* if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_BROKEN_RING) */
  
  /* ------------------------------------------------------------------------ */
  /* Management for LINE ON P1 TOPOLOGY                                       */
  /* ------------------------------------------------------------------------ */
  
  /* If topology is line port 1, read data from port 1 */
  else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
  {
    /* Set preferred master port to read data from to port 1 for all projected slaves.
     * This segment is needed for detection of slave valid miss of slaves if one line
     * of a broken ring topology has been disconnected entirely */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] = CSMD_PORT_1;
    }

    /* erase topology list for master port 2 */
    for (usH = 0; usH <= prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usH++)
    {
      prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usH]  = 0U;
    }
    prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb = 0U;

    prCSMD_Instance->usSercAddrLastSlaveP1 = prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb-1];
    prCSMD_Instance->usSercAddrLastSlaveP2 = 0U;

    prCSMD_Instance->rPriv.boP1_active = TRUE;
    prCSMD_Instance->rPriv.boP2_active = FALSE;
  }
  
  /* ------------------------------------------------------------------------ */
  /* Management for LINE ON P2 TOPOLOGY                                       */
  /* ------------------------------------------------------------------------ */
  
  /* If topology is line port 2, read data from port 2 */
  else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2) 
  {
    /* Set preferred master port to read data from to port 2 for all projected slaves.
     * This segment is needed for detection of slave valid miss of slaves if one line
     * of a broken ring topology has been disconnected entirely */
    for (usI = 0; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] = CSMD_PORT_2;
    }

    /* erase topology list for master port 1 */
    for (usH = 0; usH <= prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usH++)
    {
      prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usH]  = 0U;
    }
    prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb = 0U;

    prCSMD_Instance->usSercAddrLastSlaveP1 = 0U;
    prCSMD_Instance->usSercAddrLastSlaveP2 = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb-1];

    prCSMD_Instance->rPriv.boP1_active = FALSE;
    prCSMD_Instance->rPriv.boP2_active = TRUE;
  } /* if ((prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2) || (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING) */
  
  /* ------------------------------------------------------------------------ */
  /* Management for DEFECT RING TOPOLOGY                                      */
  /* ------------------------------------------------------------------------ */
  
  /* if topology is defect ring, split data assignment onto both ports (similar to broken ring handling) */
  else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING)
  {
    if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_PRIMARY)
    {
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
      {
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] = CSMD_PORT_1;
      }

      CSMD_SearchLineDefectPoint(prCSMD_Instance); /* determine prCSMD_Instance->usSercAddrLastSlaveP2 */

      prCSMD_Instance->usSercAddrLastSlaveP1 = 0xFFFF;
      prCSMD_Instance->rPriv.boP1_active = TRUE;
      prCSMD_Instance->rPriv.boP2_active = TRUE;
    }
    else if (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_SECONDARY)
    {
      for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
      {
        prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] = CSMD_PORT_2;
      }

      CSMD_SearchLineDefectPoint(prCSMD_Instance); /* determine prCSMD_Instance->usSercAddrLastSlaveP1 */

      prCSMD_Instance->usSercAddrLastSlaveP2 = 0xFFFF;
      prCSMD_Instance->rPriv.boP1_active = TRUE;
      prCSMD_Instance->rPriv.boP2_active = TRUE;
    }
  } /* if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING) */
  else
  {
    return (CSMD_LINE_BREAK_ERROR);
  }
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_LineBreakMngmnt() */



/**************************************************************************/ /**
\brief Manages the SVC pointers and SVC machine, if the topology was changed.

\ingroup module_redundancy
\b Description: \n
   If the topology has changed, this function reorganizes the port-specific
   service channel settings depending on the new topology.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
 
\return       none

\author       KP
\date         23.06.2007 
 
***************************************************************************** */
CSMD_VOID CSMD_SVCMngmnt4LineBreak( CSMD_INSTANCE *prCSMD_Instance )
{
  
  CSMD_USHORT     usI;
  CSMD_SERC3SVC  *prSVC;
  CSMD_USHORT     usStartIndex = 0;

#if CSMD_MAX_HW_CONTAINER > 0
#ifdef CSMD_HW_SVC_REDUNDANCY
  if (TRUE == prCSMD_Instance->rPriv.boHW_SVC_Redundancy)
  {
    usStartIndex = CSMD_MAX_HW_CONTAINER;
  }
  else
#endif
  {
    if (   (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
        || (   (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING)
            && (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_PRIMARY)) )
    {
      /* trigger SVC machine from port 2 */
      CSMD_HAL_SetSVCPort( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_SVCCSR_TRIG_PORT2 );
    }
    else
    {
      /* trigger SVC machine from port 1 */
      CSMD_HAL_SetSVCPort( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_SVCCSR_TRIG_PORT1 );
    }
  }
#endif /* #if CSMD_MAX_HW_CONTAINER > 0 */

  if (usStartIndex < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
  {
    /* Refresh active-pointer list and SVC-settings only for emulated SVC  */
    for (usI = usStartIndex; usI < prCSMD_Instance->rSlaveList.usNumProjSlaves; usI++)
    {
      prSVC = prCSMD_Instance->rPriv.prSVContainer[usI];
      /* add the difference only to Slaves on Port 2 */
      if (prCSMD_Instance->rPriv.ausPrefPortBySlave[usI] == CSMD_PORT_1)
      {
        /* refresh SVCRxRam Pointer for active Port */
        CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                             prCSMD_Instance->rPriv.arSVCInternalStruct[usI].usSVCRxRamPntrP1 );
      }
      else
      {
        /* refresh SVCRxRam Pointer for active Port */
        CSMD_HAL_WriteShort( &prSVC->usSVCRxPointer_Status,
                             prCSMD_Instance->rPriv.arSVCInternalStruct[usI].usSVCRxRamPntrP2 );
      }
    }
  }
  /* Enable service-channel operation */
  CSMD_HAL_CtrlSVCMachine( &prCSMD_Instance->rCSMD_HAL, TRUE ); /*lint !e774 Boolean within 'if' always evaluates to True */

} /* end: CSMD_SVCMngmnt4LineBreak() */



/**************************************************************************/ /**
\brief Sets the collision buffer state depending on the current physical topology.

\ingroup module_redundancy
\b Description: \n
   This function basically replaces CSMD_ComModeMngmnt4LineBreak(), which had
   set the collision buffer state according to the current Sercos topology.
   With this improved function, the collision buffer state is also adjusted
   if a change in physical topology does not result in a new Sercos topology.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
 
\return       none

\author       AlM
\date         25.08.2011 

***************************************************************************** */
CSMD_VOID CSMD_SetCollisionBuffer( const CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_BOOL boEnable = TRUE;
  
  if (   (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
      || (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING))
  {
    boEnable = FALSE;
  }
  else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
  {
    /* valid P-telegram on port 2? => Ring is physically closed */
    if (TRUE == prCSMD_Instance->rPriv.rRedundancy.boPriTelP2)
    {
      boEnable = FALSE;
    }
    else
    {
      CSMD_HAL_SetComMode( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P1_MODE );
    }
  }
  else if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
  {
    /* valid S-telegram on port 1? => Ring is physically closed */
    if (TRUE == prCSMD_Instance->rPriv.rRedundancy.boSecTelP1)
    {
      boEnable = FALSE;
    }
    else
    {
      CSMD_HAL_SetComMode( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P2_MODE );
    }
  }
  else /* broken ring topology */
  {
    CSMD_USHORT usBufP1 = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_1];
    CSMD_USHORT usBufP2 = prCSMD_Instance->rPriv.rRedundancy.ausRxBuffer[CSMD_PORT_2];
    CSMD_USHORT usSlaveIdx_P1 = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[prCSMD_Instance->usSercAddrLastSlaveP1];
    CSMD_USHORT usSlaveIdx_P2 = prCSMD_Instance->rSlaveList.ausProjSlaveIdxList[prCSMD_Instance->usSercAddrLastSlaveP2];

    /* Device status of last slave on port 1 */
    CSMD_USHORT usS_DevP1 = CSMD_END_CONV_S( *prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx_P1][CSMD_PORT_1][usBufP1] );
    /* Device status of last slave on port 2 */
    CSMD_USHORT usS_DevP2 = CSMD_END_CONV_S( *prCSMD_Instance->rPriv.apusS_DEV[usSlaveIdx_P2][CSMD_PORT_2][usBufP2] );

    /* P-Telegram on inactive port of last slave in line of port 2? */
    if ((usS_DevP2 & CSMD_S_DEV_STAT_INACT_MASK) == CSMD_S_DEV_STAT_INACT_P_TEL)
    {
      /* If the last slave in secondary line receives a P-Telegram on its inactive port with broken ring topology,
         the master collision buffer has to be disabled in two cases:
         - if the last slave on primary line receives an S-Telegram on its inactive port
         - if the last slave on primary line does not provide a valid S-DEV yet after Hot Plug (S-DEV == 0) */
      if ( ((usS_DevP1 & CSMD_S_DEV_STAT_INACT_MASK) == CSMD_S_DEV_STAT_INACT_S_TEL) || (usS_DevP1 == 0) )
      {
        boEnable = FALSE;
      }
    }
    else
    {
      CSMD_HAL_SetComMode( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_P12_MODE );
    }
  }
  
  if (!boEnable)
  {
    CSMD_HAL_SetComMode( &prCSMD_Instance->rCSMD_HAL, CSMD_HAL_DFCSR_TOPOLOGY_RT_RING_MODE );
  }
  
} /* end: CSMD_SetCollisionBuffer() */


/**************************************************************************/ /**
\brief This functions checks if any telegram receive errors occurred.

\ingroup module_cyclic
\b Description: \n
   This function checks if any expected telegrams have not been received or any
   MST was missed or received outside the appropriate window on either port depending
   on current topology.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\return       \ref CSMD_NO_TELEGRAMS_RECEIVED \n
              \ref CSMD_MST_MISS \n
              \ref CSMD_MST_WINDOW_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       AlM
\date         15.04.2014

***************************************************************************** */
CSMD_FUNC_RET CSMD_Check_Telegram_Errors( CSMD_INSTANCE *prCSMD_Instance )

{
  CSMD_FUNC_RET   eFuncRet = CSMD_NO_ERROR;

  /* For line topology, the loopback telegrams have to be received on the sending master port.
   * In case of defect ring topology, reception of the non-loopback telegram is evaluated. */
  if (   (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P1)
      || ( (   prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING)
            && (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_PRIMARY) ) )
  {
    eFuncRet = CSMD_Evaluate_TGSR ( prCSMD_Instance,
                                    CSMD_PORT_1,
                                    CSMD_PORT_1 );
  }
  else  if (   (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_LINE_P2)
            || ( (   prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_DEFECT_RING)
                  && (prCSMD_Instance->rPriv.rRedundancy.usRingDefect == CSMD_RING_DEF_SECONDARY) ) )
  {
    eFuncRet = CSMD_Evaluate_TGSR ( prCSMD_Instance,
                                    CSMD_PORT_2,
                                    CSMD_PORT_2 );
  }
  /* for any topology other than line or defect ring, the TGSR registers for both ports are evaluated */
  else
  {
    eFuncRet = CSMD_Evaluate_TGSR ( prCSMD_Instance,
                                    CSMD_PORT_1,
                                    CSMD_PORT_2 );
  }

  return (eFuncRet);

}  /* end: CSMD_Check_Telegram_Errors() */


/**************************************************************************/ /**
\brief This functions evaluates the TGSR register(s) given by the calling instance and returns
       any occurring errors.

\ingroup module_cyclic
\b Description: \n
   This function checks if any expected telegrams have not been received or any
   expected MST was missed or received outside the appropriate window on relevant
   master ports. The TGSR registers to evaluate are determined by the calling instance
   depending on current topology. If both input parameters are equal, the TGSR register
   related to this port is evaluated only.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance

\param [in]   usFirstPort
              First TGSR port given by calling instance

\param [in]   usSecondPort
              Second TGSR port given by calling instance (may be equal to first port)

\return       \ref CSMD_NO_TELEGRAMS_RECEIVED \n
              \ref CSMD_MST_MISS \n
              \ref CSMD_MST_WINDOW_ERROR \n
              \ref CSMD_NO_ERROR  \n

\author       AlM
\date         16.04.2014

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_Evaluate_TGSR( CSMD_INSTANCE *prCSMD_Instance,
                                  CSMD_USHORT    usFirstPort,
                                  CSMD_USHORT    usSecondPort )
{
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;

  /* handling for ring topology: error on both ports */
  if (prCSMD_Instance->usCSMD_Topology == CSMD_TOPOLOGY_RING)
  {
    /* no telegrams have been received on both ports */
    if ( !((prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usFirstPort] |
            prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usSecondPort]) &
            CSMD_HAL_TGSR_ALL_TEL) )
    {
      eFuncRet = CSMD_NO_TELEGRAMS_RECEIVED;
    }
    /* MST miss on both ports */
    else if ( !((prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usFirstPort] |
                 prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usSecondPort]) &
                 CSMD_HAL_TGSR_MST_VALID) )
    {
      eFuncRet = CSMD_MST_MISS;
    }
    /* at least one MST has been received outside the appropriate window */
    else if ( (prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usFirstPort] |
               prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usSecondPort]) &
               CSMD_HAL_TGSR_MST_WIN_ERR )
    {
      eFuncRet = CSMD_MST_WINDOW_ERROR;
    }
  }
  /* handling for any topology other than ring: error on one or both ports */
  else
  {
    /* no telegrams have been received on the expected port(s) */
    if ( !( prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usFirstPort] &
            prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usSecondPort] &
            CSMD_HAL_TGSR_ALL_TEL) )
    {
      eFuncRet = CSMD_NO_TELEGRAMS_RECEIVED;
    }
    /* the expected MST(s) was/were missed on the expected port(s) */
    else if ( !( prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usFirstPort] &
                 prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usSecondPort] &
                 CSMD_HAL_TGSR_MST_VALID) )
    {
      eFuncRet = CSMD_MST_MISS;
    }
    /* at least one MST has been received outside the appropriate window */
    else if ( (prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usFirstPort] |
               prCSMD_Instance->rPriv.rRedundancy.aulReg_TGSR[usSecondPort]) &
               CSMD_HAL_TGSR_MST_WIN_ERR )
    {
      eFuncRet = CSMD_MST_WINDOW_ERROR;
    }
  }

  return (eFuncRet);

} /* end: CSMD_Evaluate_TGSR() */
/*lint -restore const! */


/*! \endcond */ /* PRIVATE */



/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
10 Dec 2013 AlM
  Defdb00161754
  - CSMD_CheckTopology()
    Fixed automatic topology change from defect ring to ring.
22 Aug 2013 WK
  Defdbxxxxxxxx
  - CSMD_CheckTopology(), case CHECK TOPOLOGY FOR RING 
    If hot plug is active, don't check for ring topology with 
    telegram loss at one port.
06 Feb 2014 AlM
  Defdb00166917
  - CSMD_CheckTopology()
    Fixed brackets in handling for defect ring topology.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
12 Nov 2015 AlM
  - Defdb00182904
    Fixed potential array-out-of-bounds problem in CSMD_SetCollisionBuffer()
    and changed calling order inside CSMD_CheckTopology().
  - Defdb00182913
    CSMD_CheckTopology()
    Changed reaction to return values generated by CSMD_Check_Telegram_Errors.
18 Apr 2016 WK
  - Defdb00182067
    CSMD_SVCMngmnt4LineBreak()
    Fixed possible Lint warning 661.
09 Aug 2016 WK
  - Defdb00190817
    CSMD_SVCMngmnt4LineBreak()
    SVC redundancy failed in case of defined CSMD_HW_SVC_REDUNDANCY
    and CSMD_MAX_SLAVES <= CSMD_MAX_HW_CONTAINER.
27 Oct 2016 AlM
  - Defdb00184116
    CSMD_CheckTopology()
    Adjustments for SoftMaster.
  
------------------------------------------------------------------------------
*/
