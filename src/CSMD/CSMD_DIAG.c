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
 \file   CSMD_DIAG.c
 \author WK
 \date   01.09.2010
 \brief  This File contains the public functions and private functions 
         for diagnostic purposes
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#define SOURCE_CSMD
#include "CSMD_DIAG.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/**************************************************************************/ /**
\brief Returns the CoSeMa driver and Sercos controller
       version identifiers to a specific structure.

\ingroup func_diag
\b Description: \n
   No further description. See 'Structures for Diagnosis' for an example.

<B>Call Environment:</B> \n
   This function may be called at any time after CSMD_Initialize().
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [out]  prCSMD_Version
              Pointer to version info structure

\return       none

\author       WK
\date         03.05.2006

*******************************************************************************/
CSMD_VOID CSMD_Version( CSMD_INSTANCE  *prCSMD_Instance,
                        CSMD_VERSION   *prCSMD_Version )
{
  
  CSMD_ULONG   ulCnt;
  CSMD_CHAR    acBuffer[6];
  CSMD_USHORT  usRelease;
  CSMD_USHORT  usVersion;
  CSMD_USHORT  usTestversion;
  CSMD_USHORT  usDeviceIdent;
  CSMD_USHORT  usSERCDeviceTyp;
  
  
  CSMD_HAL_GetIdentification( &prCSMD_Instance->rCSMD_HAL,
                              &usRelease,
                              &usVersion,
                              &usTestversion,
                              &usDeviceIdent,
                              &usSERCDeviceTyp );
  
  /* Sercos Device Ident: (88CD for Sercos III FPGA) */
  prCSMD_Version->usDeviceIdent = usDeviceIdent;
  
  /* Sercos Device Type: 0=Master 1=Slave */
  prCSMD_Version->usDeviceType = usSERCDeviceTyp;
  
  /* Sercos master IP-Core identification */
  prCSMD_Version->usDeviceVersion[0] = usVersion;
  prCSMD_Version->usDeviceVersion[1] = usRelease;
  prCSMD_Version->usDeviceVersion[2] = usTestversion;
  
  /* CoSeMa Driver identification */
  prCSMD_Version->usDriverVersion[0] = CSMD_DRV_VERSION;
  prCSMD_Version->usDriverVersion[1] = CSMD_DRV_MINOR_VERSION;
  prCSMD_Version->usDriverVersion[2] = CSMD_DRV_RELEASE;

  /* ----------------------------------- */
  /* Build Sercos IP-Core Version String */
  /* ----------------------------------- */
  if ( CSMD_HAL_FPGA_IDR_SIII_IDENT == prCSMD_Version->usDeviceIdent)
  {
    ulCnt = 8;
    (CSMD_VOID) CSMD_HAL_strncpy( prCSMD_Version->caSIII_Device, CSMD_DEVICE_NAME, ulCnt );
    
    prCSMD_Version->caSIII_Device[ulCnt++] = 'V';
    /* Convert unsigned integer to string (add 10000 to get leading zeros) */
    (CSMD_VOID) CSMD_HAL_sprintf_ushort( acBuffer,"%5hu", (CSMD_USHORT)(10000 + usVersion) );
    if (acBuffer[3] != '0')
    {
      prCSMD_Version->caSIII_Device[ulCnt++] = acBuffer[3];
    }
    prCSMD_Version->caSIII_Device[ulCnt++] = acBuffer[4];

    
    prCSMD_Version->caSIII_Device[ulCnt++] = '.';
    (CSMD_VOID) CSMD_HAL_sprintf_ushort( acBuffer,"%5hu", (CSMD_USHORT)(10000 + usRelease) );
    if (acBuffer[3] != '0')
    {
      prCSMD_Version->caSIII_Device[ulCnt++] = acBuffer[3];
    }
    prCSMD_Version->caSIII_Device[ulCnt++] = acBuffer[4];
    
    if (usTestversion)
    {
      prCSMD_Version->caSIII_Device[ulCnt++] = ' ';
      prCSMD_Version->caSIII_Device[ulCnt++] = 'T';
      
      (CSMD_VOID) CSMD_HAL_sprintf_ushort( acBuffer,"%5hu", usTestversion );
      prCSMD_Version->caSIII_Device[ulCnt++] = acBuffer[4];
    }
  }
  else
  {
    /* No supported Sercos controller */
    ulCnt = 14;
    (CSMD_VOID) CSMD_HAL_strncpy( prCSMD_Version->caSIII_Device, CSMD_DEVICE_NONAME, ulCnt );
  }
  for ( ; ulCnt < CSMD_SIII_DEV_VER_LENGTH; ulCnt++) prCSMD_Version->caSIII_Device[ulCnt] = '\0';
  
  
  /* ----------------------------------- */
  /* Build CoSeMa Version String         */
  /* ----------------------------------- */
  ulCnt = 7;
  (CSMD_VOID) CSMD_HAL_strncpy( prCSMD_Version->caDriverVersion, CSMD_DRIVER_NAME, ulCnt );
  
  /* Version */
  /* Convert unsigned integer to string (add 10000 to get leading zeros) */
  (CSMD_VOID) CSMD_HAL_sprintf_ushort( acBuffer,"%5hu", (CSMD_USHORT)(10000 + CSMD_DRV_VERSION) );
  if (acBuffer[3] != '0')
  {
    prCSMD_Version->caDriverVersion[ulCnt++] = acBuffer[3];
  }
  prCSMD_Version->caDriverVersion[ulCnt++] = acBuffer[4];
  prCSMD_Version->caDriverVersion[ulCnt++] = '.';
  
  /* Sub-Version*/
  (CSMD_VOID) CSMD_HAL_sprintf_ushort( acBuffer,"%5hu", (CSMD_USHORT)(10000 + CSMD_DRV_MINOR_VERSION) );
  if (acBuffer[3] == '0')
  {
    prCSMD_Version->caDriverVersion[ulCnt++] = acBuffer[4];
  }
  else
  {
    prCSMD_Version->caDriverVersion[ulCnt++] = acBuffer[3];
    prCSMD_Version->caDriverVersion[ulCnt++] = '.';
    prCSMD_Version->caDriverVersion[ulCnt++] = acBuffer[4];
  }
  
  /* (V)ersion / (T)estversion */
  prCSMD_Version->caDriverVersion[ulCnt++] = ' ';
  prCSMD_Version->caDriverVersion[ulCnt++] = CSMD_DRVSTR_TYPE;
  prCSMD_Version->caDriverVersion[ulCnt++] = ' ';
  
  /* Release */
  (CSMD_VOID) CSMD_HAL_sprintf_ushort( acBuffer,"%5hu", (CSMD_USHORT)(10000 + CSMD_DRV_RELEASE) );
  prCSMD_Version->caDriverVersion[ulCnt++] = acBuffer[2];
  prCSMD_Version->caDriverVersion[ulCnt++] = acBuffer[3];
  prCSMD_Version->caDriverVersion[ulCnt++] = acBuffer[4];
  
  for ( ; ulCnt < CSMD_DRV_VER_LENGTH; ulCnt++) prCSMD_Version->caDriverVersion[ulCnt] = '\0';
  
  
  /* CoSeMa Driver Release Date String (like __DATE__) */
  (CSMD_VOID) CSMD_HAL_strncpy( prCSMD_Version->caDriverDate, CSMD_DRIVER_DATE, CSMD_DRV_DAT_LENGTH );
  
} /* end: CSMD_Version */



/**************************************************************************/ /**
\brief Returns the Communication phase currently entered in MST.

\ingroup func_diag
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   Before the call-up of this function, the basic initialization of the Sercos
   controller must have been completed.\n
   The call-up can be performed from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\param [out]  pusPhase
              Current communication phase (Sercos phase).
              Bit 4 indicates active communication phase switching.

\return       \ref CSMD_INVALID_SERCOS_PHASE \n
              \ref CSMD_NO_ERROR \n
              

\author       WK
\date         08.07.2005

*******************************************************************************/
CSMD_FUNC_RET CSMD_GetPhase( CSMD_INSTANCE *prCSMD_Instance,
                             CSMD_USHORT   *pusPhase )
{
  
  CSMD_USHORT usGetPhase;
  CSMD_USHORT usPhaseSwitch;
  
  CSMD_HAL_GetPhase( &prCSMD_Instance->rCSMD_HAL,
                     &usGetPhase,
                     &usPhaseSwitch );
  
  if (usPhaseSwitch)      /* communication phase switching active? */
  {
    *pusPhase = (CSMD_USHORT)(usGetPhase | 0x0010U);
  }
  else
  {
    *pusPhase = usGetPhase;
  }
  
  if (usGetPhase > CSMD_SERC_PHASE_4)
  {
    return (CSMD_INVALID_SERCOS_PHASE);
  }
  else
  {
    return (CSMD_NO_ERROR);
  }
  
} /* end: CSMD_GetPhase() */



/**************************************************************************/ /**
\brief Returns the physical topology of all slaves connected to the master.

\ingroup func_diag
\b Description: \n
   This function returns a Sercos list for each port containing the Sercos
   addresses of the existing slaves in physical order.

<B>Call Environment:</B> \n
   This function can be called as of CP0.\n
   The call-up should be performed from a task.\n
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [out]  pusDev_P1 
              Output pointer for a Sercos list of existing slaves in topological
              order on Port 1.
\param [out]  pusDev_P2
              Output pointer for a Sercos list of existing slaves in topological
              order on Port 2.

\return       \ref CSMD_WRONG_PHASE \n
              \ref CSMD_NO_ERROR \n
              
\author       MS
\date         05.08.2005

*******************************************************************************/
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_GetTopology( CSMD_INSTANCE *prCSMD_Instance,
                                CSMD_USHORT   *pusDev_P1,
                                CSMD_USHORT   *pusDev_P2 )
{
  
  CSMD_USHORT  usI;              /* Index */
  CSMD_USHORT *pusSlaveList;
  
  if (prCSMD_Instance->sCSMD_Phase < CSMD_SERC_PHASE_0)
  {
    return (CSMD_WRONG_PHASE);
  }
  else if (0 == prCSMD_Instance->rSlaveList.ausRecogSlaveAddList[0])
  {
    /* Returns an empty list if CP0 switch is active */
    pusDev_P1[0] = 0;   /* Current data length of the Sercos list */
    pusDev_P1[1] = 0;   /* Maximum data length of the Sercos list */

    pusDev_P2[0] = 0;   /* Current data length of the Sercos list */
    pusDev_P2[1] = 0;   /* Maximum data length of the Sercos list */
  }
  else
  {
    pusSlaveList = pusDev_P1 + 2;   /* Pointer to first entry in list */

    for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb; usI++)
    {
      *pusSlaveList++ = prCSMD_Instance->rPriv.rSlaveAvailable.ausAddresses[usI];
    }
    pusDev_P1[0] = (CSMD_USHORT)(prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb * 2);
    pusDev_P1[1] = pusDev_P1[0];


    pusSlaveList = pusDev_P2 + 2;   /* Pointer to first entry in list */

    for (usI = 0; usI < prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb; usI++)
    {
      *pusSlaveList++ = prCSMD_Instance->rPriv.rSlaveAvailable2.ausAddresses[usI];
    }
    pusDev_P2[0] = (CSMD_USHORT)(prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb * 2);
    pusDev_P2[1] = pusDev_P2[0];
  }
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_GetTopology() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Handles the identification bit in the local C-DEV.

\ingroup func_diag
\b Description: \n
   This function activates the Sercos status LED (if available) via the
   identification bit in the device control (C-DEV). Thus, a visual
   identification of slaves in the ring is possible even if, for example,
   a slave doesn't have an address display.\n
   The current status of the identification bit is maintained in CP2
   to CP4 until it is changed by this function.\n
   This function sets respectively clears the identification bit in the
   Device Control. The C-DEV will be effected in CP2, CP3 and CP4.

<B>Call Environment:</B> \n
   This function may be called in any communication phase.\n
   The call-up may be performed from either an interrupt or a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usSlaveIdx
              Device Index of the Sercos III slave inside the list of
              projected slave addresses.
\param [in]   boActivate 
              TRUE = set the identification bit
              FALSE = clear the identification bit

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         27.06.2007

*******************************************************************************/
CSMD_FUNC_RET CSMD_IdentifySlave( CSMD_INSTANCE *prCSMD_Instance,
                                  CSMD_USHORT    usSlaveIdx,
                                  CSMD_BOOL      boActivate )
{
  
  if (usSlaveIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
  {
    if (boActivate == TRUE)
      prCSMD_Instance->rPriv.ausDevControl[usSlaveIdx] |= CSMD_C_DEV_IDENTIFICATION;
    else
      prCSMD_Instance->rPriv.ausDevControl[usSlaveIdx] &= (CSMD_USHORT)~CSMD_C_DEV_IDENTIFICATION;
  }
  else
  {
    CSMD_RuntimeWarning( prCSMD_Instance, usSlaveIdx,
                         "CSMD_IdentifySlave: invalid slave index or no SCP V1.1 device!" );
  }
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_IdentifySlave() */



/**************************************************************************/ /**
\brief Returns the current C-DEV of the selected slave.

\ingroup func_diag
\b Description: \n
   This function reads the current value of the Device Control word C-DEV
   from the selected slave. The return value is valid in CP3 and later.

  \n Device Control structure (C-DEV): \n

  |Bit No.|        Description        |                   Comments                                                   |
  | ----- | ------------------------- | ---------------------------------------------------------------------------- |
  | 15    | Identification            | Slave shows the condition at Sercos III LED or at the display                |
  | 14    | Topology HS               | Master toggles every time it requires a topology change                      |
  | 13-12 | Topology control          | Master selects the new topology                                              |
  | 11    | Control physical topology | If the slave detects a toggle,\n then it shall drop the source address table |
  | 10-9  | reserved                  |                                                                              |
  | 8     | Master valid              | indicates if master is processing real-time data.                            |
  | 7-0   | reserved                  |                                                                              |
  <BR>

<B>Call Environment:</B> \n
   This function can be called either from an interrupt or from a task.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usSlaveIdx
              Slave index of the Sercos III slave inside the list of
              projected slave addresses

\return       Device Control C-DEV
              0 -> Runtime Warning

\author       WK
\date         10.09.2008
  
*******************************************************************************/
CSMD_USHORT CSMD_Read_C_Dev( CSMD_INSTANCE *prCSMD_Instance,
                        CSMD_USHORT         usSlaveIdx )
{
  if (usSlaveIdx < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
  {
    return (prCSMD_Instance->rPriv.ausDevControl[usSlaveIdx]);
  }
  else
  {
    CSMD_RuntimeWarning( prCSMD_Instance, usSlaveIdx,
                         "CSMD_Read_C_Dev: invalid slave index!" );
    return (0);
  }
} /* end: CSMD_Read_C_Dev() */



/**************************************************************************/ /**
\brief Reads the Ethernet frame error counters from IP-Core for port 1 and 2.

\ingroup func_diag
\b Description: \n
   Nt further description.

<B>Call Environment:</B> \n
   This function can be called either from an interrupt or from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\param [in]   usPortNbr
              - 1 for Port 1
              - 2 for Port 2

\param [out]  prCommCounter
              Output pointer to communication counter Sercos list

\return       none

\author       WK
\date         08.12.2006
    
*******************************************************************************/
CSMD_VOID  CSMD_GetCommCounter( CSMD_INSTANCE     *prCSMD_Instance,
                                CSMD_USHORT        usPortNbr,
                                CSMD_COMM_COUNTER *prCommCounter )
{

  CSMD_HAL_GetCommCounter( &prCSMD_Instance->rCSMD_HAL,
                           usPortNbr,
                           (CSMD_HAL_COMM_COUNTER *) (CSMD_VOID *)prCommCounter );

} /* end: CSMD_GetCommCounter() */



/**************************************************************************/ /**
\brief Resets the Ethernet frame error counter in the IP-Core.

\ingroup func_diag
\b Description: \n
   This function resets all Ethernet frame error counter of the IP-Core.

<B>Call Environment:</B> \n
   This function can be called either from an interrupt or from a task.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       none  

\author  WK
\date    26.11.2008

***************************************************************************** */
 /*lint -save -e818 const! */
CSMD_VOID  CSMD_ResetSercosErrorCounter( CSMD_INSTANCE *prCSMD_Instance )
{
  CSMD_HAL_ResetCounter_SercosError( &prCSMD_Instance->rCSMD_HAL );

} /* end: CSMD_ResetSercosErrorCounter() */
/*lint -restore const! */

/*! \endcond */ /* PUBLIC */



/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Records error and cancels SVC transmission of a slave.

\ingroup func_diag
\b Description: \n
    This function records errors in structure CSMD_EXTENDED_DIAG rExtendedDiag
    and cancels the pending service channel transmission.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvcMacro      
              Pointer to slave svc macro structure
\param [in]   usSlaveIdx
              Slave index
\param [out]  eError
              Error code to be recorded

\return       none

\author       WK
\date         28.04.2010

***************************************************************************** */
CSMD_VOID CSMD_Record_SVC_Error( CSMD_INSTANCE *prCSMD_Instance,
                                 CSMD_USHORT    usSlaveIdx,
                                 CSMD_FUNC_RET  eError )
{
  
  if (prCSMD_Instance->rExtendedDiag.usNbrSlaves < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
  {
    prCSMD_Instance->rExtendedDiag.ausSlaveIdx[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = usSlaveIdx;
    prCSMD_Instance->rExtendedDiag.aeSlaveError[prCSMD_Instance->rExtendedDiag.usNbrSlaves] = eError;
    prCSMD_Instance->rExtendedDiag.usNbrSlaves++;
  }
  
} /* end: CSMD_Record_SVC_Error() */



/**************************************************************************/ /**
\brief  In case of a runtime warning, this function adds additional
        information to a debug string variable. 

\ingroup func_diag
\b Description: \n
   No further description.

<B>Call Environment:</B>\n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usSlaveIndex
              Index of slave
\param [in]   pchMessage
              Warning text

\return       none
  
\author  bk
\date    17.10.2006
    
***************************************************************************** */
CSMD_VOID CSMD_RuntimeWarning ( CSMD_INSTANCE *prCSMD_Instance,
                                CSMD_USHORT    usSlaveIndex,
                                const char    *pchMessage )
{

  prCSMD_Instance->rPriv.rRuntimeWarning.usSlaveIndex = usSlaveIndex;
  (CSMD_VOID) CSMD_HAL_strcpy( prCSMD_Instance->rPriv.rRuntimeWarning.acInfo, pchMessage );

} /* of CSMD_RuntimeWarning */



/**************************************************************************/ /**
\brief In case of a runtime error, this function adds additional
       information to a debug string variable. 

\ingroup func_diag
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   usSlaveIndex
              Index of slave
\param [in]   pchMessage
              Additional error text

\return       none
  
\author  bk
\date    17.10.2006
    
***************************************************************************** */
CSMD_VOID CSMD_RuntimeError ( CSMD_INSTANCE *prCSMD_Instance,
                              CSMD_USHORT    usSlaveIndex,
                              const char    *pchMessage )
{
  
  prCSMD_Instance->rPriv.rRuntimeError.usSlaveIndex = usSlaveIndex;
  (CSMD_VOID) CSMD_HAL_strcpy( prCSMD_Instance->rPriv.rRuntimeError.acInfo, pchMessage );
  
} /* of CSMD_RuntimeError */

/*! \endcond */ /* PRIVATE */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
21 Nov 2013 WK
  Defdb00000000
  Fixed compiler warnings in CSMD_ResetSercosErrorCounter()
  in case of not defined CSMD_NRT_CHANNEL.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
31 Mar 2016 WK
  - Defdb00186013
    CSMD_GetTopology()
    The function can provide illegal addresses in CP0 in case of
    unstable topology.
  
------------------------------------------------------------------------------
*/
