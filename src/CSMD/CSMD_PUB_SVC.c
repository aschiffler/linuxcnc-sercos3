/*
CoSeMa V6.1 - Common Sercos Master function library
Copyright (c) 2004 - 2015  Bosch Rexroth AG

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
\file   CSMD_PUB_SVC.c
\author WK
\date   01.09.2010
\brief  This File contains the public primitive API functions for the 
        asynchronous communication (service channel) to a Sercos slave.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#include "CSMD_PRIV_SVC.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/**************************************************************************/ /**
\brief Reads the specified element of an IDN.
  
\ingroup func_svc_macro
<B>Description of the process of the macro state machine:</B>
  
- #CSMD_START_REQUEST:\n
  At the beginning it is checked whether the
  service channel is reserved or whether the ongoing transmission is to be 
  canceled. If yes, the prioritization information is analyzed and a process
  with lower priority is canceled by enabling the service channel. In a new
  call-up, the canceled request will in this case notify the CoSeMa error
  CSMD_REQUEST_CANCELED. The calling instance must compensate the cancellation
  of a transmission. If an ongoing transmission cannot be canceled, the current
  request will be discarded and CSMD_ReadSVCH() will return the CoSeMa error
  CSMD_SVCH_INUSE. If the service channel is free, it will be reserved and
  the information from CSMD_SVCH_MACRO_STRUCT will be transmitted into the
  internal CSMD_SVCH_MNGMT_STRUCT. Afterwards, there is an automatic change
  into the next state.
  
- #CSMD_INIT_SVCH:\n
  The service channel is opened by means of the CSMD_OpenIDN
  function. CSMD_ReadSVCH remains in this state. As soon as CMSD_OpenIDN returns
  CSMD_FINISHED_REQUEST, it will be checked whether the IDN exists. If so, 
  there is an automatic change into the subsequent state. Otherwise, the macro
  function will be terminated and a corresponding CoSeMa error is returned. The
  SVCH error message is passed on by the CSMD_OpenIDN function and also
  returned by the macro function. In case of an error, the state of the service
  channel is moreover copied into the usSvchError variable of the macro struct.

- #CSMD_CHANNEL_OPEN:\n
  It is checked at the beginning whether the attribute is
  to be obtained. The attribute is obtained if it is not made available in the
  structure and if no length information is transmitted. For accesses to element
  2 and 4, the attribute will not be obtained even if it has not been
  transmitted and length information is missing. In this case, the necessary
  length information will be read from the list. If the attribute is required,
  the CSMD_GetAttribute function will be called and this state will be repeated
  until CSMD_GetAttribute returns CSMD_FINISHED_REQUEST. If necessary, the
  information whether the parameter is a list parameter or a parameter with
  fixed length is generated from the attribute and transmitted into the usIsList
  variable of the atomic structure. Similarly, the length information in a
  parameter with fixed length is, if necessary, calculated from the attribute
  and transmitted into the atomic structure. If the requested element is a list
  value, the current list length will be obtained by means of the
  CSMD_GetListLength function and transmitted into the atomic structure, if no
  length information has been transmitted. Afterwards, the service channel will
  be prepared for the read access to the requested element. If all information
  is supplied via the macro structure, the reading preparation is completed
  immediately. Change into the subsequent state is completed automatically
  after completion of the preparation for the read access. Possible error
  messages in the transmission and/or processing are returned via the function
  return value. If SVCH errors occur, the SVCH state is additionally stored in
  the usSvchError variable of the macro structure. 

- #CSMD_GET_ATTRIBUTE:\n
  The attribute is required indispensibly
  (CSMD_BIG_ENDIAN is defined). The attribute is read and later written in the
  state CSMD_CHANNEL_OPEN. This step is followed by CSMD_SET_CMD.

- #CSMD_ATTRIBUTE_VALID:\n
  The atomic function for the requested element is
  called in order to read the element. The macro function remains in this
  state until CSMD_FINISHED_REQUEST is returned. If the transmission is
  completed, there is an automatic change into the next state. Possible 
  error messages in the transmission and/or processing are returned via the
  function return value. If SVCH errors occur, the SVCH state is additionally
  stored in the usSvchError variable of the macro structure.
  
- #CSMD_DATA_VALID:\n
  The reading process has been completed successfully. The
  requested data is stored in the transmitted buffer. The macro function
  releases the service channel and is completed.                

- #CSMD_REQUEST_ERROR:\n
  This step is called if an error has occurred. The process
  could not be finished successfully. The macro function unblocks the service channel and
  is terminated.
  <BR>

  State                | End condition
  -------------------- | -------------
  CSMD_START_REQUEST   | Management structure initialized for read access.\n SVC checked and reserved for requests, else return value is CoSeMa error code
  CSMD_INIT_SVCH       | SVCH opened and busy
  CSMD_CHANNEL_OPEN    | Attribute read, if necessary
  CSMD_GET_ATTRIBUTE   | Attribute read
  CSMD_ATTRIBUTE_VALID | All data read completely
  CSMD_DATA_VALID      | All data transmitted successfully. SVC unblocked, end of macro function
  CSMD_REQUEST_ERROR   | Error occurred during transmission, end of macro function


<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however,
   not imperatively necessary. In the first call-up of the function,
   usState must once be initialized to the value CSMD_START_REQUEST.
 
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to array with SVC macro structures
\param [out]  CallBackFunc
              Output Pointer to Callback function
 
\return       \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_INTERNAL_REQUEST_PENDING \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         13.01.2005
  
*******************************************************************************/
CSMD_FUNC_RET CSMD_ReadSVCH( CSMD_INSTANCE          *prCSMD_Instance,
                             CSMD_SVCH_MACRO_STRUCT *prSvchData, 
                             CSMD_VOID               (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_FUNC_RET  eRet = CSMD_NO_ERROR;
  CSMD_BOOL      boBreak;
  
  
  usSlaveIdx = prSvchData->usSlaveIdx;
  
  switch (prSvchData->usState)
  {
    
  case CSMD_START_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      eRet = CSMD_WRONG_SLAVE_INDEX;
      break;
    }
    eRet = CSMD_CheckSVCHInUse( prCSMD_Instance, prSvchData );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    CSMD_InitSVCHRequest( prCSMD_Instance, prSvchData, FALSE );
    
    prSvchData->usState = CSMD_INIT_SVCH;
    /*lint -fallthrough */
    
  case CSMD_INIT_SVCH:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    /*Open SVCH if not open already*/
    eRet = CSMD_OpenSVCH( prCSMD_Instance, prSvchData, usSlaveIdx, &boBreak );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    if (boBreak)
      break;
    /*lint -fallthrough */
    
  case CSMD_CHANNEL_OPEN:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
    
#if defined CSMD_BIG_ENDIAN || defined CSMD_TEST_BE
    /* Read attribute if necessary */
    eRet = CSMD_ReadAttribute( prCSMD_Instance, prSvchData, usSlaveIdx, &boBreak );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    if (boBreak)
      break;
    /*lint -fallthrough */
    
  case CSMD_GET_ATTRIBUTE:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
#endif
    eRet = CSMD_PrepFinalStepRead( prCSMD_Instance, prSvchData, usSlaveIdx, &boBreak );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    if (boBreak)
      break;
    /*lint -fallthrough */
    
  case CSMD_ATTRIBUTE_VALID:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
    
    eRet = CSMD_FinalStepRead( prCSMD_Instance, prSvchData, usSlaveIdx, &boBreak );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    if (boBreak)
      break;
    
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }
    /*lint -fallthrough */
    
  case CSMD_DATA_VALID:
    break;
    
  case CSMD_REQUEST_ERROR:
    break;
    
  default:
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
    return (CSMD_SYSTEM_ERROR);
  
  }   /* end: switch (prSvchData->usState) */
  
  if (   (eRet != CSMD_NO_ERROR)
      && (eRet != CSMD_SVCH_INUSE)
      && (eRet != CSMD_INTERNAL_REQUEST_PENDING))
  {
    prSvchData->usState = CSMD_REQUEST_ERROR;
  }
  return (eRet);
  
}   /* End: CSMD_ReadSVCH()*/



   /**************************************************************************/ /**
\brief Writes the specified element of an IDN. Permitted are
       element 1 'Data status' and element 7 'Operating data'.

\ingroup func_svc_macro
<B>Description of the process of the macro state machine:</B>
  
- #CSMD_START_REQUEST:\n
  At the beginning it is checked whether the service
  channel is reserved or whether the ongoing transmission is to be canceled.
  If yes, the prioritization information is analyzed and a process with lower
  priority is canceled by enabling the service channel. In a new call-up, the
  canceled request will in this case notify the CoSeMa error
  CSMD_REQUEST_CANCELED. The calling instance must compensate the cancellation
  of a transmission. If an ongoing transmission cannot be canceled, the current
  request will be discarded and CSMD_ReadSVCH() will return the CoSeMa error
  CSMD_SVCH_INUSE. If the service channel is free, it will be reserved and the
  information from CSMD_SVCH_MACRO_STRUCT will be transmitted into the internal
  CSMD_SVCH_MNGMT_STRUCT. Afterwards, there is an automatic change into the 
  next state.

- #CSMD_INIT_SVCH:\n
  If the service channel is not open due to a previous
  transmission, it is opened by the CSMD_OpenIDN function. CSMD_WriteSVCH
  remains in this state. As soon as CMSD_OpenIDN returns CSMD_FINISHED_REQUEST,
  it will be checked whether the IDN exists. If yes, there is an automatic
  change into the subsequent state. Otherwise, the macro function will be
  terminated and return the corresponding CoSeMa error. The SVCH error message
  is passed on by the CSMD_OpenIDN function and also returned by the macro
  function. In case of an error, the state of the service channel is moreover
  copied into the usSvchError variable of the macro structure.

- #CSMD_CHANNEL_OPEN:\n
  It is checked at the beginning whether the attribute
  is to be obtained. The attribute is obtained if it is not made available in
  the structure and if no length information is transmitted. For accesses to 
  element 2 and 4, the attribute will not be obtained even if it has not been
  transmitted and length information is missing. In this case, the necessary
  length information will be read from the list. If the attribute is required,
  the CSMD_GetAttribute function will be called and this state will be repeated
  until CSMD_GetAttribute returns CSMD_FINISHED_REQUEST. If necessary, the
  information whether the parameter is a list parameter or a parameter with 
  fixed length is generated from the attribute and transmitted into the 
  usIsList variable of the atomic structure. Similarly, the length information
  in a parameter with fixed length is, if necessary, calculated from the
  attribute and transmitted into the atomic structure. If the requested element
  is a list value, the current list length will be obtained by means of the 
  CSMD_GetListLength function and transmitted into the atomic structure, if no
  length information has been transmitted. Afterwards, the service channel will
  be prepared for the read access to the requested element. If all information
  is supplied via the macro structure, the reading preparation is completed
  immediately. Change into the subsequent state is completed automatically
  after completion of the preparation for the read access. Possible error
  messages in the transmission and/or processing are returned via the function
  return value. If SVCH errors occur, the SVCH state is additionally stored in
  the usSvchError variable of the macro structure.
  
- #CSMD_ATTRIBUTE_VALID:\n
  The CSMD_GetData function is called in order to
  write the element. The macro function remains in this state until
  CSMD_GetData returns CSMD_FINISHED_REQUEST. If the transmission is completed,
  there is an automatic change into the next state.
  
- #CSMD_DATA_VALID:\n
  The writing process has been completed successfully.
  The macro function releases the service channel and is completed.

- #CSMD_REQUEST_ERROR:\n
  This step is called if an error has occurred. The process
  could not be finished successfully. The macro function unblocks the service channel and
  is terminated.
  <BR>

  State                | End condition
  -------------------- | -------------
  CSMD_START_REQUEST   | Management structure initialized for write access.\n SVC checked and reserved for requests, else return value is CoSeMa error code
  CSMD_INIT_SVCH       | SVCH opened and busy
  CSMD_CHANNEL_OPEN    | Attribute read, if necessary
  CSMD_ATTRIBUTE_VALID | All data written completely
  CSMD_DATA_VALID      | All data transmitted successfully. SVC unblocked, end of macro function
  CSMD_REQUEST_ERROR   | Error occurred during transmission, end of macro function


<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however,
   not imperatively necessary.\n
   In the first call-up of the function, usState must once be initialized to
   the value CSMD_START_REQUEST.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to array with SVC macro structures
\param [out]  CallBackFunc
              Output Pointer to Callback function
                
\return       \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_INTERNAL_REQUEST_PENDING \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_NO_ERROR \n

\author       WK 
\date         13.01.2005

*******************************************************************************/
CSMD_FUNC_RET CSMD_WriteSVCH( CSMD_INSTANCE          *prCSMD_Instance,
                              CSMD_SVCH_MACRO_STRUCT *prSvchData, 
                              CSMD_VOID               (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_FUNC_RET  eRet = CSMD_NO_ERROR;
  CSMD_BOOL      boBreak;
  
  
  usSlaveIdx = prSvchData->usSlaveIdx;
  
  switch (prSvchData->usState)
  {
    
  case CSMD_START_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      eRet = CSMD_WRONG_SLAVE_INDEX;
      break;
    }
    eRet = CSMD_CheckSVCHInUse( prCSMD_Instance, prSvchData );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    CSMD_InitSVCHRequest( prCSMD_Instance, prSvchData, FALSE );
    
    prSvchData->usState = CSMD_INIT_SVCH;
    /*lint -fallthrough */
    
  case CSMD_INIT_SVCH:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    /*Open SVCH if not open already*/
    eRet = CSMD_OpenSVCH( prCSMD_Instance, prSvchData, usSlaveIdx, &boBreak );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    if (boBreak)
      break;
#if defined CSMD_BIG_ENDIAN || defined CSMD_TEST_BE
    else
    {
      if (!prSvchData->ulAttribute)
      {
        /* Force to read attribute at first in the next step */
        prSvchData->usLength = 0U;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute = 0U;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
      }
    }
#endif
    /*lint -fallthrough */
    
  case CSMD_CHANNEL_OPEN:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    eRet = CSMD_PrepFinalStepWrite( prCSMD_Instance, prSvchData, usSlaveIdx, &boBreak );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    if (boBreak)
      break;
    /*lint -fallthrough */
    
  case CSMD_ATTRIBUTE_VALID:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    eRet = CSMD_FinalStepWrite( prCSMD_Instance, prSvchData, usSlaveIdx, &boBreak );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    if (boBreak)
      break;
    
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }
    /*lint -fallthrough */
    
  case CSMD_DATA_VALID:
    break;
    
  case CSMD_REQUEST_ERROR:
    break;
    
  default:
    return (CSMD_SYSTEM_ERROR);
    
  }   /* end: switch (prSvchData->usState) */
  
  if (   (eRet != CSMD_NO_ERROR)
      && (eRet != CSMD_SVCH_INUSE)
      && (eRet != CSMD_INTERNAL_REQUEST_PENDING))
  {
    prSvchData->usState = CSMD_REQUEST_ERROR;
  }
  return (eRet);
    
} /* End: CSMD_WriteSVCH() */



   /**************************************************************************/ /**
\brief Sets the date of a command parameter to the value
       '3' and reads the command acknowledgment.

\ingroup func_svc_macro
CSMD_CMD_ACTIVE is returned as soon as the lowest bit of the 
command acknowledgment have adapted the value "1".\n

<B>Description of the process of the macro state machine:</B>

- #CSMD_START_REQUEST:\n
  At the beginning it is checked whether the service channel is reserved or
  whether the ongoing transmission is to be canceled. If yes, the prioritization
  information is analyzed and a process with lower priority is canceled by
  enabling the service channel. In a new call-up, the canceled request will in this
  case notify a CoSeMa error. The calling instance must compensate the
  cancellation of a transmission. If an ongoing transmission cannot be canceled,
  the current request will be discarded and CSMD_SetCommand() will return a 
  CoSeMa error.  
  If the service channel is free, it will be reserved and the information from
  CSMD_SVCH_MACRO_STRUCT will be transmitted into the internal
  CSMD_SVCH_MNGMT_STRUCT and all necessary containers will be filled.
  Afterwards, there is an automatic change into the next state. 
  
- #CSMD_INIT_SVCH:\n
  If the service channel is not open due to a previous 
  transmission, it is opened by the CSMD_OpenIDN function. CSMD_SetCommand()
  remains in this state. As soon as CMSD_OpenIDN returns CSMD_FINISHED_REQUEST,
  it will be checked whether the IDN exists. If yes, there is an automatic
  change into the subsequent state. Otherwise, the macro function will be 
  terminated and return a corresponding CoSeMa error.
  
- #CSMD_GET_ATTRIBUTE:\n
  The attribute is required indispensibly (CSMD_BIG_ENDIAN is defined).
  The attribute is read and later written in the state CSMD_CHANNEL_OPEN. This
  step is followed by CSMD_SET_CMD.

- #CSMD_SET_CMD:\n
  By means of the atomic function CSMD_PutData, the value "3" will be
  transmitted to the date of the IDN. The macro function remains in this state until
  CSMD_PutData returns CSMD_FINISHED_REQUEST.
  
- #CSMD_CHECK_CMD:\n
  By means of the atomic function CSMD_GetDataStatus(), the
  command status is queried. This is repeated until bit 0 are set in the
  command status. As soon as this status is recognized, the command status will
  be copied into the transmitted buffer. No timeout is realized in the macro
  function. This must be done in the calling instance.
  
- #CSMD_CMD_ACTIVE:\n
  Command is set. The SVCH is enabled; end of the function.

- #CSMD_REQUEST_ERROR:\n
  This step is called if an error has occurred. The process
  could not be finished successfully. The macro function unblocks the service channel and
  is terminated.
  <BR>

  State              | End condition
  ------------------ | -------------
  CSMD_START_REQUEST | Management structure initialized for write access.\n SVC checked and reserved for requests, else return value is CoSeMa error code
  CSMD_INIT_SVCH     | SVCH opened and busy
  CSMD_GET_ATTRIBUTE | Attribute read
  CSMD_SET_CMD       | '3' was written
  CSMD_CHECK_CMD     | Command status: Bit 0  = '1'
  CSMD_CMD_ACTIVE    | Command is set; SVC unblocked, end of macro function
  CSMD_REQUEST_ERROR | Error occurred during transmission, end of macro function

  <BR>

  Element of rSvchData                | Content
  ----------------------------------- | -------
  CSMD_USHORT  usSlaveIdx             | Slave Index
  CSMD_USHORT  usElem                 | Don't care
  CSMD_ULONG   ulIdent_Nbr            | Parameter IDN of the command
  CSMD_USHORT *pusAct_Data            | Don't care
  CSMD_USHORT  usPriority             | Don't care
  CSMD_USHORT  usLength               | Don't care
  CSMD_USHORT  usIsList               | Don't care
  CSMD_ULONG   ulAttribute            | Is read, if not set
  CSMD_USHORT  usSvchError            | Filled by the function in case of error: 0
  CSMD_USHORT  usState                | Managed by the function, initialized with CSMD_START_REQUEST
  CSMD_USHORT  usCancelActTrans       | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usOtherRequestCanceled | Internal variable
  CSMD_USHORT  usInternalReq          | Set to 1 for reservation of SVC, else 0


<BR><B>Call Environment:</B> \n
   The CSMD_SetCommand function can be called from an interrupt; this is, however,
   not imperatively necessary.\n  
   In the first call-up of the function, usState must once be initialized to the value
   CSMD_START_REQUEST.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to array with SVC macro structures
\param [out]  CallBackFunc
              Output Pointer to Callback function

\return       \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_INTERNAL_REQUEST_PENDING \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_NO_PROCEDURE_CMD \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_NO_ERROR \n

\author       WK 
\date         13.01.2005
  
*******************************************************************************/
CSMD_FUNC_RET CSMD_SetCommand( CSMD_INSTANCE          *prCSMD_Instance,
                               CSMD_SVCH_MACRO_STRUCT *prSvchData,
                               CSMD_VOID               (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_FUNC_RET eRet;
  CSMD_USHORT   usSlaveIdx;
  CSMD_SVCH_MNGMT_STRUCT *prSvcMngmt;

  eRet     = CSMD_NO_ERROR;
  usSlaveIdx = prSvchData->usSlaveIdx;
  prSvcMngmt = &prCSMD_Instance->parSvchMngmtData[usSlaveIdx];
  
  switch (prSvchData->usState)
  {
  case CSMD_START_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      eRet = CSMD_WRONG_SLAVE_INDEX;
      break;
    }
    eRet = CSMD_CheckSVCHInUse( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
    
    CSMD_InitSVCHRequest( prCSMD_Instance, prSvchData, TRUE );
    
    prSvchData->usState = CSMD_INIT_SVCH;
    /*lint -fallthrough */
    
    
  case CSMD_INIT_SVCH:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
    
    /* Open SVCH if not open already */
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen == 0U)
    {
      eRet = CSMD_OpenIDN( prCSMD_Instance, 
                           prSvcMngmt,
                           (CSMD_VOID(*)(CSMD_VOID))NULL );
      
      if (eRet != CSMD_NO_ERROR) 
      {
        if (eRet == CSMD_SVC_ERROR_MESSAGE) 
          prSvchData->usSvchError = prSvcMngmt->SVCH_Status.usStatus;
        prSvcMngmt->usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
        break;
      }
    }
#ifdef CSMD_HOTPLUG
    else if (prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] == CSMD_SLAVE_INACTIVE)
    {
      /* Service channel of a hot-plug slave is not active */
      prSvcMngmt->usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      eRet = CSMD_REQUEST_TO_HP_SLAVE;
      break;
    }
#endif
    
    if (   (prSvcMngmt->usActStateAtomic == CSMD_FINISHED_REQUEST)
        || (prSvcMngmt->usChannelOpen == 1U))
    {
      prSvcMngmt->pusAct_Data      = prCSMD_Instance->rPriv.parRdWrBuffer[usSlaveIdx].ausData;
      prSvcMngmt->usChannelOpen    = 1U;
      prSvcMngmt->usSetEnd         = 1U;
      prSvcMngmt->usActStateAtomic = CSMD_INIT_REQUEST;

      if (prSvcMngmt->ulAttribute)
      {
        if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute & CSMD_SERC_PROC_CMD)
        {
          /*continue with next step, initialization*/
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength       = 2U;
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem         = CSMD_SVC_DBE7_OPERATION_DATA;
          prSvchData->usState = CSMD_SET_CMD;

          eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
          if (eRet != CSMD_NO_ERROR)
            break;

          /* Start procedure command */
          prCSMD_Instance->rPriv.parRdWrBuffer[usSlaveIdx].ausData[0] = CSMD_CMD_START;

          eRet = CSMD_PutData( prCSMD_Instance,
                               &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                               (CSMD_VOID(*)(CSMD_VOID))NULL);

          if (eRet != CSMD_NO_ERROR)
          {
            if (eRet == CSMD_SVC_ERROR_MESSAGE)
              prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
            prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
            break;
          }
        }
        else
        {
          eRet = CSMD_NO_PROCEDURE_CMD;
        }
        break;
      }
      else
      {
        /*continue with next step, initialization*/
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength      = 4U;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem        = CSMD_SVC_DBE3_ATTRIBUTE;
        
        prSvchData->usState = CSMD_GET_ATTRIBUTE;
      } 
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_GET_ATTRIBUTE:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;

    eRet = CSMD_GetAttribute( prCSMD_Instance,
                              &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                              (CSMD_VOID(*)(CSMD_VOID))NULL);
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) 
        prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      break;
    }
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic != CSMD_FINISHED_REQUEST)
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
    }
    
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
    {
      if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute & CSMD_SERC_PROC_CMD)
      {
        /*continue with next step, initialization*/
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength         = 2U;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data =
          prCSMD_Instance->rPriv.parRdWrBuffer[usSlaveIdx].ausData;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd         = 1U;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen    = 1U;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem           = CSMD_SVC_DBE7_OPERATION_DATA;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;

        prSvchData->usState = CSMD_SET_CMD;
      }
      else
      {
        eRet = CSMD_NO_PROCEDURE_CMD;
        break;
      }
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_SET_CMD:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
    
    /* Start procedure command */
    prCSMD_Instance->rPriv.parRdWrBuffer[usSlaveIdx].ausData[0] = CSMD_CMD_START;
    
    eRet = CSMD_PutData( prCSMD_Instance,
                         &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                         (CSMD_VOID(*)(CSMD_VOID))NULL);
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) 
        prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      break;
    }
    
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
    {
      /* continue with next step, initialization */
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem           = CSMD_SVC_DBE1_IDN;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength         = 4U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data      =
        (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulIdent_Nbr;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Position   = 0U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd         = 1U;
      
      prSvchData->usState = CSMD_CHECK_CMD;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_CHECK_CMD:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
    
    /* check command status */
    eRet = CSMD_GetDataStatus( prCSMD_Instance, 
                               &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                               (CSMD_VOID(*)(CSMD_VOID))NULL );
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) 
        prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      break;
    }
    
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
    {
      /* Procedure command is set */
      if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus)
      {
        /* continue with next step, initialization */
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
        prSvchData->pusAct_Data[0] = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
        
        if (prSvchData->usInternalReq)
        {
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIntReqPend = 0U;
        }
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
        
        if (CallBackFunc != NULL)
        {
          CallBackFunc();
        }
        prSvchData->usState = CSMD_CMD_ACTIVE;
      }
      else
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
        break;
      }
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Position = 0U;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_CMD_ACTIVE:
    break;
    
    
  case CSMD_REQUEST_ERROR:
    break;
    
    
  default:
    return (CSMD_SYSTEM_ERROR);
  
  }   /* end: switch (prSvchData->usState) */
  
  if (   (eRet != CSMD_NO_ERROR)
      && (eRet != CSMD_SVCH_INUSE)
      && (eRet != CSMD_INTERNAL_REQUEST_PENDING))
  {
    prSvchData->usState = CSMD_REQUEST_ERROR;
  }
  return (eRet);
  
}   /* End: CSMD_SetCommand(() */



   /**************************************************************************/ /**
\brief Sets the date of a command parameter to the value '0'.
       Reads the command acknowledgment.

\ingroup func_svc_macro
CSMD_CMD_CLEARED is returned as soon as the command acknowledgment
has adapted the value "0".\n

<B>Description of the process of the macro state machine:</B>
  
- #CSMD_START_REQUEST:\n
  At the beginning it is checked whether the service
  channel is reserved or whether the ongoing transmission is to be canceled.
  If yes, the prioritization information is analyzed and a process with lower
  priority is canceled by enabling the service channel. In a new call-up, 
  the canceled request will in this case notify a CoSeMa error. The calling
  instance must compensate the cancellation of a transmission. If an ongoing
  transmission cannot be canceled, the current request will be discarded and
  CSMD_ClearCommand() will return a CoSeMa error.
  If the service channel is free, it will be reserved and the information from
  CSMD_SVCH_MACRO_STRUCT will be transmitted into the internal
  CSMD_SVCH_MNGMT_STRUCT and all necessary containers will be filled.
  Afterwards, there is an automatic change into the next state.
  
- #CSMD_INIT_SVCH:\n
  If the service channel is not still open from a previous
  transmission, it is opened by the CSMD_OpenIDN function. CSMD_ClearCommand()
  remains in this state. As soon as CMSD_OpenIDN returns CSMD_FINISHED_REQUEST,
  it will be checked whether the IDN exists. If yes, there is an automatic
  change into the subsequent state. Otherwise, the macro function will be 
  terminated and return a corresponding CoSeMa error. 
  
- #CSMD_GET_ATTRIBUTE:\n
  The attribute is required indispensibly
  (CSMD_BIG_ENDIAN is defined). The attribute is read and later written in the
  state CSMD_CHANNEL_OPEN. This step is followed by CSMD_CLEAR_CMD.

- #CSMD_CLEAR_CMD:\n
  By means of the atomic function CSMD_PutData, the value
  \"0\" will be transmitted to the date of the IDN. The macro function
  remains in this state until CSMD_PutData returns CSMD_FINISHED_REQUEST.
  
- #CSMD_CHECK_CMD:\n
  By means of the atomic function CSMD_GetDataStatus, the
  command status is queried. This is repeated until the command status is 
  \"0\". No timeout is realized in the macro function. This must be
  done in the calling instance.
  
- #CSMD_CMD_CLEARED:\n
  Clearance of the command has been acknowledged. SVCH 
  enabled and end of the function

- #CSMD_REQUEST_ERROR:\n
  This step is called if an error has occurred.
  The process could not be finished successfully. The macro function 
  unblocks the service channel and is terminated.
  <BR>

  State              | End condition
  ------------------ | -------------
  CSMD_START_REQUEST | Management structure initialized for write access.\n SVC checked and reserved for requests, else return value is CoSeMa error code
  CSMD_INIT_SVCH     | SVCH opened and busy
  CSMD_GET_ATTRIBUTE | Attribute read
  CSMD_CLEAR_CMD     | '0' was written
  CSMD_CHECK_CMD     | command status = '0' ?
  CSMD_CMD_CLEARED   | CMD clearance validated; SVC unblocked, end of macro function
  CSMD_REQUEST_ERROR | Error occurred during transmission, end of macro function

  <BR>

  Element of rSvchData                | Content
  ----------------------------------- | -------
  CSMD_USHORT  usSlaveIdx             | Slave Index
  CSMD_USHORT  usElem                 | Don't care
  CSMD_ULONG   ulIdent_Nbr            | Parameter IDN of the command
  CSMD_USHORT *pusAct_Data            | Don't care
  CSMD_USHORT  usPriority             | Don't care
  CSMD_USHORT  usLength               | Don't care
  CSMD_USHORT  usIsList               | Don't care
  CSMD_ULONG   ulAttribute            | Is read, if not set
  CSMD_USHORT  usSvchError            | Filled by the function in case of error: 0
  CSMD_USHORT  usState                | Managed by the function, initialized with CSMD_START_REQUEST
  CSMD_USHORT  usCancelActTrans       | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usOtherRequestCanceled | Internal variable
  CSMD_USHORT  usInternalReq          | Set to 1 for reservation of SVC, else 0


<BR><B>Call Environment:</B>  \n
   The CSMD_ClearCommand function can be called from an interrupt; this is, however,
   not imperatively necessary. \n
   In the first call-up of the function, usState must once be initialized to
   the value CSMD_START_REQUEST.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to array with SVC macro structures
\param [out]  CallBackFunc
              Output Pointer to Callback function

\return       \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_INTERNAL_REQUEST_PENDING \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_NO_PROCEDURE_CMD \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         13.01.2005

*******************************************************************************/
CSMD_FUNC_RET CSMD_ClearCommand( CSMD_INSTANCE          *prCSMD_Instance,
                                 CSMD_SVCH_MACRO_STRUCT *prSvchData,
                                 CSMD_VOID               (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_FUNC_RET eRet;
  CSMD_USHORT   usSlaveIdx;
  
  
  eRet     = CSMD_NO_ERROR;
  usSlaveIdx = prSvchData->usSlaveIdx;
  
  switch (prSvchData->usState)
  {
  case CSMD_START_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      eRet = CSMD_WRONG_SLAVE_INDEX;
      break;
    }
    eRet = CSMD_CheckSVCHInUse( prCSMD_Instance, prSvchData );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    CSMD_InitSVCHRequest( prCSMD_Instance, prSvchData, TRUE );
    
    prSvchData->usState = CSMD_INIT_SVCH;
    /*lint -fallthrough */
    
    
  case CSMD_INIT_SVCH:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
    
    /* Open SVCH if not open already */
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen == 0U)
    {
      eRet = CSMD_OpenIDN( prCSMD_Instance, 
                           &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                           (CSMD_VOID(*)(CSMD_VOID))NULL );
      
      if (eRet != CSMD_NO_ERROR) 
      {
        if (eRet == CSMD_SVC_ERROR_MESSAGE) 
          prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
        break;
      }
    }
#ifdef CSMD_HOTPLUG
    else if (prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] == CSMD_SLAVE_INACTIVE)
    {
      /* Service channel of a hot-plug slave is not active */
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      eRet = CSMD_REQUEST_TO_HP_SLAVE;
      break;
    }
#endif
    
    if (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
        || (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen == 1U))
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data =
        prCSMD_Instance->rPriv.parRdWrBuffer[usSlaveIdx].ausData;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen    = 1U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd         = 1U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
      
      if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
      {
        if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute & CSMD_SERC_PROC_CMD)
        {
          /*continue with next step, initialization*/
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength       = 2U;
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem         = CSMD_SVC_DBE7_OPERATION_DATA;
          prSvchData->usState = CSMD_CLEAR_CMD;

          eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
          if (eRet != CSMD_NO_ERROR)
            break;

          /* Clear procedure command */
          prCSMD_Instance->rPriv.parRdWrBuffer[usSlaveIdx].ausData[0] = CSMD_CMD_CLEAR;

          eRet = CSMD_PutData( prCSMD_Instance,
                               &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                               (CSMD_VOID(*)(CSMD_VOID))NULL );

          if (eRet != CSMD_NO_ERROR)
          {
            if (eRet == CSMD_SVC_ERROR_MESSAGE)
              prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
            prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
            break;
          }
        }
        else
        {
          eRet = CSMD_NO_PROCEDURE_CMD;
        }
        break;
      }
      else
      {
        /* continue with next step, initialization */
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength       = 4U;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem         = CSMD_SVC_DBE3_ATTRIBUTE;
        
        prSvchData->usState = CSMD_GET_ATTRIBUTE;
      }
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_GET_ATTRIBUTE:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;

    eRet = CSMD_GetAttribute( prCSMD_Instance,
                              &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                              (CSMD_VOID(*)(CSMD_VOID))NULL);
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) 
        prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      break;
    }
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic != CSMD_FINISHED_REQUEST)
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
    }
    
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
    {
      if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute & CSMD_SERC_PROC_CMD)
      {
        /* continue with next step, initialization */
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data =
          prCSMD_Instance->rPriv.parRdWrBuffer[usSlaveIdx].ausData;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength         = 2U;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd         = 1U;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen    = 1U;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem           = CSMD_SVC_DBE7_OPERATION_DATA;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;

        prSvchData->usState = CSMD_CLEAR_CMD;
      }
      else
      {
        eRet = CSMD_NO_PROCEDURE_CMD;
        break;
      }
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_CLEAR_CMD:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
    
    /* Clear procedure command */
    prCSMD_Instance->rPriv.parRdWrBuffer[usSlaveIdx].ausData[0] = CSMD_CMD_CLEAR;
    
    eRet = CSMD_PutData( prCSMD_Instance,
                         &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                         (CSMD_VOID(*)(CSMD_VOID))NULL );
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) 
        prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      break;
    }
    
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
    {
      /* continue with next step, initialization */
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem           = CSMD_SVC_DBE1_IDN;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength         = 4U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd         = 1U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data =
        (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulIdent_Nbr;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Position   = 0U;
      
      prSvchData->usState = CSMD_CHECK_CMD;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_CHECK_CMD:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
    
    eRet = CSMD_GetDataStatus( prCSMD_Instance, 
                               &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                               (CSMD_VOID(*)(CSMD_VOID))NULL );
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) 
        prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      break;
    }
    
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
    {
      /* Command cleared? */
      if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus == CSMD_CMD_NOT_SET)
      {
      /* continue with next step, initialization */
        prSvchData->pusAct_Data[0] = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
        if (prSvchData->usInternalReq)
        {
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIntReqPend = 0U;
        }
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
        
        if (CallBackFunc != NULL)
        {
          CallBackFunc();
        }
        prSvchData->usState = CSMD_CMD_CLEARED;
      }
      else
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
        break;
      }
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Position = 0U;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_CMD_CLEARED:
    break;
    
    
  case CSMD_REQUEST_ERROR:
    break;
    
    
  default:
    return (CSMD_SYSTEM_ERROR);
  
  }   /* end: switch (prSvchData->usState) */
  
  if (   (eRet != CSMD_NO_ERROR)
      && (eRet != CSMD_SVCH_INUSE)
      && (eRet != CSMD_INTERNAL_REQUEST_PENDING))
  {
    prSvchData->usState = CSMD_REQUEST_ERROR;
  }
  return (eRet);
  
}   /* End: CSMD_ClearCommand() */



/***************************************************************************/ /**
\brief Reads the data status of an IDN.

\ingroup func_svc_macro
<B>Description of the process of the macro state machine:</B>
  
- #CSMD_START_REQUEST:\n
  At the beginning it is checked whether the service
  channel is reserved or whether the ongoing transmission is to be canceled.
  If yes, the prioritization information is analyzed and a process with lower
  priority is canceled by enabling the service channel. In a new call-up, the
  canceled request will in this case notify a CoSeMa error. The calling
  instance must compensate the cancellation of a transmission. If an ongoing
  transmission cannot be canceled, the current request will be discarded and
  CSMD_ReadCmdStatus() will return a CoSeMa error. If the service channel is
  free, it will be reserved and the information from CSMD_SVCH_MACRO_STRUCT
  will be transmitted into the internal CSMD_SVCH_MNGMT_STRUCT and all the 
  necessary containers will be filled. Afterwards, there is an automatic
  change into the next state.

- #CSMD_INIT_SVCH:\n
  If the service channel is not still open from a previous
  transmission, it is opened by the CSMD_OpenIDN() function. CSMD_ReadCmdStatus()
  remains in this state. As soon as CMSD_OpenIDN returns CSMD_FINISHED_REQUEST,
  it will be checked whether the IDN exists. If yes, there is an automatic
  change into the subsequent state. Otherwise, the macro function will be 
  terminated and return a corresponding CoSeMa error.

- #CSMD_GET_CMD_STATUS:\n
  By means of the atomic function CSMD_GetDataStatus(),
  the data status is queried. The data status is stored in the pusAct_Data
  data buffer.
  
  - <B>Data status</B>
   - Procedure command acknowledgment:
     - <B>0x0003  procedure command has been executed correctly</B>
     - <B>0x000F  error: procedure command execution impossible</B>

     - 0x0000  procedure command has been canceled
     - 0x0001  procedure command is set
     - 0x0005  procedure command execution is interrupted
     - 0x0007  procedure command is processing

   - Data status for non procedure command parameters:
     - 0x0000  Operation data is valid
     - 0x0100  Operation data is invalid

- #CSMD_CMD_STATUS_VALID:\n
  The data status has been read successfully.

- #CSMD_REQUEST_ERROR:\n
  This step is called if an error has occurred.
  The process could not be finished successfully. The macro function 
  unblocks the service channel and is terminated.
  <BR>

  State                 | End condition
  --------------------- | -------------
  CSMD_START_REQUEST    | Management structure initialized for read access.\n SVC checked and reserved for requests, else return value is CoSeMa error code
  CSMD_INIT_SVCH        | SVCH opened and busy
  CSMD_GET_CMD_STATUS   | Data status read
  CSMD_CMD_STATUS_VALID | SVC unblocked, end of macro function
  CSMD_REQUEST_ERROR    | Error occurred during transmission, end of macro function

  <BR>

  Element of rSvchData                | Content
  ----------------------------------- | -------
  CSMD_USHORT  usSlaveIdx             | Slave Index
  CSMD_USHORT  usElem                 | Don't care
  CSMD_ULONG   ulIdent_Nbr            | Parameter IDN
  CSMD_USHORT *pusAct_Data            | Pointer to buffer where the data status should be stored in
  CSMD_USHORT  usPriority             | Don't care
  CSMD_USHORT  usLength               | Don't care
  CSMD_USHORT  usIsList               | Don't care
  CSMD_ULONG   ulAttribute            | Don't care
  CSMD_USHORT  usSvchError            | Filled by the function in case of error: 0
  CSMD_USHORT  usState                | Managed by the function, initialized with CSMD_START_REQUEST
  CSMD_USHORT  usCancelActTrans       | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usOtherRequestCanceled | Internal variable
  CSMD_USHORT  usInternalReq          | Set to 1 for reservation of SVC, else 0


<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however,
   not imperatively necessary.\n
   In the first call-up of the function, usState must once be initialized
   to the value CSMD_START_REQUEST.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to array with SVC macro structures
\param [out]  CallBackFunc
              Output Pointer to Callback function

\return       \ref CSMD_SVCH_INUSE \n
              \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_INTERNAL_REQUEST_PENDING \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         13.01.2005
  
*******************************************************************************/
CSMD_FUNC_RET CSMD_ReadCmdStatus( CSMD_INSTANCE          *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT *prSvchData, 
                                  CSMD_VOID               (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_FUNC_RET eRet;
  CSMD_USHORT   usSlaveIdx;
  
  
  eRet     = CSMD_NO_ERROR;
  usSlaveIdx = prSvchData->usSlaveIdx;
  
  switch (prSvchData->usState)
  {
  case CSMD_START_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      eRet = CSMD_WRONG_SLAVE_INDEX;
      break;
    }
    eRet = CSMD_CheckSVCHInUse( prCSMD_Instance, prSvchData );
    
    if (eRet != CSMD_NO_ERROR)
      break;
    
    CSMD_InitSVCHRequest( prCSMD_Instance, prSvchData, FALSE );
    
    prSvchData->usState = CSMD_INIT_SVCH;
    /*lint -fallthrough */
    
    
  case CSMD_INIT_SVCH:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
    
    /* Open SVCH if not open already */
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen == 0U)
    {
      eRet = CSMD_OpenIDN( prCSMD_Instance,
                           &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                           (CSMD_VOID(*)(CSMD_VOID))NULL );
      
      if (eRet != CSMD_NO_ERROR) 
      {
        if (eRet == CSMD_SVC_ERROR_MESSAGE) 
          prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
        break;
      }
    }
#ifdef CSMD_HOTPLUG
    else if (prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] == CSMD_SLAVE_INACTIVE)
    {
      /* Service channel of a hot-plug slave is not active */
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      eRet = CSMD_REQUEST_TO_HP_SLAVE;
      break;
    }
#endif
    
    if (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
        || (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen == 1U))
    {
      /* continue with next step, initialization */
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen    = 1U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength         = 4U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd         = 1U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem           = CSMD_SVC_DBE1_IDN;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data =
        (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulIdent_Nbr;
      
      prSvchData->usState = CSMD_GET_CMD_STATUS;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_GET_CMD_STATUS:
    
    eRet = CSMD_CheckRequestCancel( prCSMD_Instance, prSvchData );
    if (eRet != CSMD_NO_ERROR)
      break;
    
    eRet = CSMD_GetDataStatus( prCSMD_Instance, 
                               &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                               (CSMD_VOID(*)(CSMD_VOID))NULL );
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) 
        prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      break;
    }
    
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
    {
      prSvchData->pusAct_Data[0] = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      if (prSvchData->usInternalReq)
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIntReqPend = 0U;
      }
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      
      if (CallBackFunc != NULL)
      {
        CallBackFunc();
      }
      prSvchData->usState = CSMD_CMD_STATUS_VALID;
    }
    else
    {
      break;
    }
    /*lint -fallthrough */
    
    
  case CSMD_CMD_STATUS_VALID:
    break;
    
    
  case CSMD_REQUEST_ERROR:
    break;
    
    
  default:
    return (CSMD_SYSTEM_ERROR);
  
  }   /* end: switch (prSvchData->usState) */
  
  if (   (eRet != CSMD_NO_ERROR)
      && (eRet != CSMD_SVCH_INUSE)
      && (eRet != CSMD_INTERNAL_REQUEST_PENDING))
  {
    prSvchData->usState = CSMD_REQUEST_ERROR;
  }
  return (eRet);
    
}   /* End: CSMD_ReadCmdStatus() */

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
10 Apr 2014
  - CSMD_SetCommand(), CSMD_ClearCommand(), CSMD_ReadCmdStatus()
    Added check for procedure command.
21 Jan 2015 WK
  - Convert simple HTML tables into "markdown" extra syntax to make it
    more readable in the source code.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
24 Jun 2015 WK
  - Defdb00000000
    CSMD_ReadCmdStatus()
      Removed procedure command check of IDN, since the function reads
      the data status of any IDN!
      Removed reading of attribute.
    CSMD_SetCommand(), CSMD_ClearCommand()
      Added check of request cancellation before reading the attribute.
      Fixed problem, if the attribute must be read and the function is
      called depending on service channel interrupt.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
  
------------------------------------------------------------------------------
*/
