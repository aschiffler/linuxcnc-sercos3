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
\file   CSMD_ATOM_SVC.c
\author WK
\date   01.09.2010
\brief  This File contains the public atomic API functions for the
        asynchronous communication (service channel) to a Sercos Slave.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#include "CSMD_PRIV_SVC.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/**************************************************************************/ /**
\brief Opens the service channel for accessing a certain IDN
  
\ingroup func_svc_atomic
<B>Description of the process of the atomic function state machine:</B>
  
- #CSMD_INIT_REQUEST:\n
  Initialization of the containers and start of the transmission

- #CSMD_REQUEST_IN_PROGRESS:\n
  Evaluation of the data status. In case of
  an error, notification of the error takes place via function return value.

- #CSMD_FINISHED_REQUEST:\n
  SVCH enabled, end of the function
  <BR>

  Element of rSvchMngmtData      | Content
  :----------------------------- | :------
  CSMD_USHORT  usSlaveIdx        | Slave index
  CSMD_ULONG   ulIdent_Nbr       | Parameter IDN
  CSMD_USHORT  usElem            | Element: 1
  CSMD_USHORT *pusAct_Data       | Pointer to data containing the IDN
  CSMD_USHORT  usPriority        | Don`t care
  CSMD_USHORT  usLength          | Data length: 4
  CSMD_USHORT  usIsList          | No list parameter: 0
  CSMD_ULONG   ulAttribute       | Don`t care: 0
  CSMD_USHORT  usSvchError       | Filled by the function in case of error: 0
  CSMD_USHORT  usCancelActTrans  | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usActStateAtomic  | Managed internally: CSMD_INIT_REQUEST
  CSMD_USHORT  usAct_Position    | Managed internally: 0
  CSMD_USHORT  usReplaceable     | Don't care: 0
  CSMD_S3_SVCH_SW  SVCH_Status   | Filled by the function: 0
  CSMD_USHORT  usAct_Len         | Current length of list parameter (filled if known): 0
  CSMD_USHORT  usMax_Len         | Maximum length of list parameter (filled if known): 0
  CSMD_USHORT  usRequestedLength | Don't care
  CSMD_USHORT  usSrv_Cont        | Software(1) or hardware(0) processing: 0
  CSMD_USHORT  usSetEnd          | For lists: managed by the function: 1
  CSMD_USHORT  usCycle           | Internal variable: 0
  CSMD_USHORT  usNumWords        | Internal variable: 0
  CSMD_USHORT  usInUse           | Internal variable: 0
  CSMD_USHORT  usChannelOpen     | Internal variable; is set to 1 after SVC has been opened
  CSMD_USHORT  usRequestCanceled | Internal variable; is set after the request has been canceled: 0
  CSMD_USHORT  usLLR             | Internal variable; length of list parameter has been read: 0
  CSMD_USHORT  usIntReqPend      | Internal variable, set by macro function
  CSMD_USHORT  usEmptyList       | Don't care

<BR><B>Call Environment:</B> \n
   The CSMD_OpenIDN function can be called from an interrupt; this is, however,
   not imperatively necessary. The CSMD_SVCH_MNGMT_STRUCT must be completed with
   all necessary data.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchMngmtData
              Pointer to macro management structure
\param [out]  CallBackFunc
              Pointer to callback function 
                
\return       \ref CSMD_REQUEST_CANCELED \n
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
CSMD_FUNC_RET CSMD_OpenIDN( CSMD_INSTANCE          *prCSMD_Instance,
                            CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData, 
                            CSMD_VOID              (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_SERC3SVC *prSVC;
  CSMD_FUNC_RET  eRet;
  
  
#ifdef CSMD_HOTPLUG
  if (!prCSMD_Instance->rSlaveList.aeSlaveActive[ prSvchMngmtData->usSlaveIdx ])
  {
    /* Service channel of a hot-plug slave is not active */
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_REQUEST_TO_HP_SLAVE);
  }
#endif
  if (prSvchMngmtData->usCancelActTrans)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_REQUEST_CANCELED);
  }
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  
  
  switch (prSvchMngmtData->usActStateAtomic)
  {
  case CSMD_INIT_REQUEST:
    
  /* --------------------------------------------------------------------
      Step : 1 Open the service channel, as the Sercos is written
      to element 1 (the ID number).
    --------------------------------------------------------------------*/
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      return (CSMD_WRONG_SLAVE_INDEX);
    }
    /*reset flag for ListLengthRead*/
    prSvchMngmtData->usLLR = 0U;
    
    prSvchMngmtData->usAct_Position = 0U;
    
    if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0]) & CSMD_SVC_CTRL_M_BUSY) )
      return (CSMD_MBUSY_NOT_SET);
    
    
    eRet = CSMD_SVCHWrite( prCSMD_Instance, prSvchMngmtData );
    
    if (eRet != CSMD_NO_ERROR)
    {
      return (eRet);
    }
    
    prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
    break;
    
    
  case CSMD_REQUEST_IN_PROGRESS:
    
    eRet = CSMD_CheckSVCError( prSVC,prSvchMngmtData, FALSE );
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    /*lint -fallthrough */
    
    
  case CSMD_FINISHED_REQUEST:
    
    /* copy data status*/
    prSvchMngmtData->SVCH_Status.usStatus = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
    
    /* Call the Callback function*/
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }
    break;
    
    
  default:
    return (CSMD_SYSTEM_ERROR);
    
  }   /* end: switch (prSvchMngmtData->usActStateAtomic) */
  
  return (CSMD_NO_ERROR);
  
} /* End: CSMD_OpenIDN()*/



/**************************************************************************/ /**
\brief Reads the attribute (data block element 3) of an IDN that
       is already open.

\ingroup func_svc_atomic
<B>Description of the process of the atomic function state machine:</B>
  
- #CSMD_INIT_REQUEST:\n
  Initialization of the containers and start of the transmission.

- #CSMD_REQUEST_IN_PROGRESS:\n
  Reading of the element.
  The function remains in this state until all data has been read.

- #CSMD_FINISHED_REQUEST:\n
  SVCH enabled, end of the function
  <BR>
  
  Element of rSvchMngmtData      | Content
  :----------------------------- | :------
  CSMD_USHORT  usSlaveIdx        | Slave index
  CSMD_ULONG   ulIdent_Nbr       | Parameter IDN
  CSMD_USHORT  usElem            | Element: 3
  CSMD_USHORT *pusAct_Data       | Pointer to buffer where the attribute should be stored in
  CSMD_USHORT  usPriority        | Don`t care
  CSMD_USHORT  usLength          | Data length: 4
  CSMD_USHORT  usIsList          | Element 3 is not a list: 0
  CSMD_ULONG   ulAttribute       | Don`t care (filled by the function after successful reading of the attribute): 0
  CSMD_USHORT  usSvchError       | Filled by the function in case of error: 0
  CSMD_USHORT  usCancelActTrans  | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usActStateAtomic  | Managed internally: CSMD_INIT_REQUEST
  CSMD_USHORT  usAct_Position    | Managed internally: 0
  CSMD_USHORT  usReplaceable     | Don't care: 0
  CSMD_S3_SVCH_SW  SVCH_Status   | Filled by the function: 0
  CSMD_USHORT  usAct_Len         | Don't care: 0
  CSMD_USHORT  usMax_Len         | Don't care: 0
  CSMD_USHORT  usRequestedLength | Don't care
  CSMD_USHORT  usSrv_Cont        | Software(1) or hardware(0) processing: 0
  CSMD_USHORT  usSetEnd          | No list: managed by the function: 1
  CSMD_USHORT  usCycle           | Internal variable: 0
  CSMD_USHORT  usNumWords        | Internal variable: 0
  CSMD_USHORT  usInUse           | Internal variable: 0
  CSMD_USHORT  usChannelOpen     | Internal variable; is set to 1 after SVC has been opened
  CSMD_USHORT  usRequestCanceled | Internal variable; is set after the request has been canceled: 0
  CSMD_USHORT  usLLR             | Internal variable; length of list parameter has been read: 0
  CSMD_USHORT  usIntReqPend      | Internal variable, set by macro function
  CSMD_USHORT  usEmptyList       | Don't care

<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however, not
   imperatively necessary.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchMngmtData
              Pointer to macro management structure
\param [out]  CallBackFunc 
              Pointer to callback function

\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n
  
\author   WK 
\date     13.01.2005

*******************************************************************************/
CSMD_FUNC_RET CSMD_GetAttribute( CSMD_INSTANCE          *prCSMD_Instance,
                                 CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData,
                                 CSMD_VOID              (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_SERC3SVC *prSVC;
  CSMD_FUNC_RET  eRet;
  
  
  if (prSvchMngmtData->usCancelActTrans)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_REQUEST_CANCELED);
  }
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  
  
  switch (prSvchMngmtData->usActStateAtomic)
  {
    
  case CSMD_INIT_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      return (CSMD_WRONG_SLAVE_INDEX);
    }
    prSvchMngmtData->usSetEnd = 1U;
    prSvchMngmtData->usLLR = 0U;
    
    if (!(CSMD_HAL_ReadShort(&prSVC->rCONTROL.usWord[0]) & CSMD_SVC_CTRL_M_BUSY))
      return (CSMD_MBUSY_NOT_SET);
    
    /* Read the  Sercos-Attributes*/
    CSMD_SVCHRead( prCSMD_Instance, prSvchMngmtData );
    
    prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
    
    break;
    
    
  case CSMD_REQUEST_IN_PROGRESS:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError( prSVC, prSvchMngmtData, TRUE );
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
#ifdef CSMD_BIG_ENDIAN
    /* Conversion of  the Two short word in Long word*/
    prSvchMngmtData->ulAttribute  = 
      (  ((CSMD_ULONG)CSMD_HAL_ReadShort(&prSVC->sRD_BUFF[1]) << 16) 
       | (CSMD_ULONG)CSMD_HAL_ReadShort(&prSVC->sRD_BUFF[0]) );
#else
    /* Conversion of  the Two short word in Long word*/
    prSvchMngmtData->ulAttribute  = 
      (  ((CSMD_ULONG)CSMD_HAL_ReadShort(&prSVC->sRD_BUFF[1]) << 16) 
       | (CSMD_ULONG)CSMD_HAL_ReadShort(&prSVC->sRD_BUFF[0]) );
#endif
    
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    /*lint -fallthrough */
    
    
  case CSMD_FINISHED_REQUEST:
    
    if (!prSvchMngmtData->usIsList)
    {
      prSvchMngmtData->usLength = 
        (CSMD_USHORT)(1U << ((prSvchMngmtData->ulAttribute & CSMD_SERC_LEN_WO_LISTINFO) >> 16));
    }
    
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }   
    break;
    
    
  default:
    return (CSMD_SYSTEM_ERROR);
    
  }   /* end: switch (prSvchMngmtData->usActStateAtomic) */
  
  return (CSMD_NO_ERROR);
  
}   /* End: CSMD_GetAttribute()*/


/**************************************************************************/ /**
\brief Reads the name (data block element 2) of an IDN that is
       already open.

\ingroup func_svc_atomic
<B>Description of the process of the atomic function state machine:</B>

- #CSMD_INIT_REQUEST:\n
  Initialization of the containers and start of the transmission

- #CSMD_REQUEST_IN_PROGRESS:\n
  Reading of the element.
  The function remains in this state until all data has been read.

- #CSMD_FINISHED_REQUEST:\n
  SVCH enabled, end of the function
  <BR>

  Element of rSvchMngmtData      | Content
  :----------------------------- | :------
  CSMD_USHORT  usSlaveIdx        | Slave index
  CSMD_ULONG   ulIdent_Nbr       | Parameter IDN
  CSMD_USHORT  usElem            | Element: 2
  CSMD_USHORT *pusAct_Data       | Pointer to buffer where the name should be stored in
  CSMD_USHORT  usPriority        | Don`t care
  CSMD_USHORT  usLength          | Length of data to be read in bytes (don't care for elements 2 and 4, data is read directly): 0
  CSMD_USHORT  usIsList          | Element 2 is a list: 1
  CSMD_ULONG   ulAttribute       | Don`t care: 0
  CSMD_USHORT  usSvchError       | Filled by the function in case of error: 0
  CSMD_USHORT  usCancelActTrans  | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usActStateAtomic  | Managed internally: CSMD_INIT_REQUEST
  CSMD_USHORT  usAct_Position    | Managed internally: 0
  CSMD_USHORT  usReplaceable     | Don't care: 0
  CSMD_S3_SVCH_SW  SVCH_Status   | Filled by the function: 0
  CSMD_USHORT  usAct_Len         | Current length of the element (possibly read via CSMD_GetListLength() ): ActLen
  CSMD_USHORT  usMax_Len         | Maximum length of element (possibly read via CSMD_GetListLength() ): 0
  CSMD_USHORT  usRequestedLength | Don't care
  CSMD_USHORT  usSrv_Cont        | Software(1) or hardware(0) processing: 0
  CSMD_USHORT  usSetEnd          | Managed internally for lists: 0
  CSMD_USHORT  usCycle           | Internal variable: 0
  CSMD_USHORT  usNumWords        | Internal variable: 0
  CSMD_USHORT  usInUse           | Internal variable: 0
  CSMD_USHORT  usChannelOpen     | Internal variable; is set to 1 after SVC has been opened
  CSMD_USHORT  usRequestCanceled | Internal variable; is set after the request has been canceled: 0
  CSMD_USHORT  usLLR             | Internal variable; indicates if length info of the list has been read yet\n length info is transmitted to the list during the following read access (0)\n or not (1): Don't change
  CSMD_USHORT  usIntReqPend      | Internal variable, set by macro function
  CSMD_USHORT  usEmptyList       | Don't care

<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however, not
   imperatively necessary.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance.
\param [in]   prSvchMngmtData
              Pointer to macro management structure
\param [out]  CallBackFunc
              Pointer to callback function

\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       WK 
\date         13.01.2005

*******************************************************************************/
CSMD_FUNC_RET CSMD_GetName( CSMD_INSTANCE          *prCSMD_Instance,
                            CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData,
                            CSMD_VOID              (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_USHORT    usLoop;
  CSMD_SERC3SVC *prSVC;
  CSMD_FUNC_RET  eRet;
  
  
  if (prSvchMngmtData->usCancelActTrans)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_REQUEST_CANCELED);
  }
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  

  switch (prSvchMngmtData->usActStateAtomic)
  {
    
  case CSMD_INIT_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      return (CSMD_WRONG_SLAVE_INDEX);
    }
    if (prSvchMngmtData->usIsList)
    {
#ifdef CSMD_SVC_LIST_SEGMENT
      if (prSvchMngmtData->usIsList == CSMD_ELEMENT_RD_LIST_SEGMENT)
      {
        /* Read list segment */
        if (((prSvchMngmtData->usRequestedLength + 1U) / 2U) > (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH)
        {
          prSvchMngmtData->usNumWords = (CSMD_USHORT)(((prSvchMngmtData->usRequestedLength + 1U) / 2U)
            - (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH);
          prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH * 2U); /* in byte */
          prSvchMngmtData->usSetEnd = 0U;
          prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
        }
        else
        {
          prSvchMngmtData->usLength = prSvchMngmtData->usRequestedLength;
          /* SetEnd flag should be set:                                         */
          /*  - for transmitting less than 4 bytes of the current list length   */
          /*  - if requested length is lower equal than the current list length */
          
          if (   ((((prSvchMngmtData->usAct_Len + 3)/4) * 4 - prSvchMngmtData->usRequestedLength) < 4)
              && (prSvchMngmtData->usAct_Len >= prSvchMngmtData->usRequestedLength))
          {
            prSvchMngmtData->usSetEnd = 1U;
          }
          else
          {
            prSvchMngmtData->usSetEnd = 0U;
          }
          
          prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
        }
        
        prSvchMngmtData->usAct_Position = 0U;
      }
      else
#endif
      {
        if (((prSvchMngmtData->usAct_Len + 1U) / 2U) > (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH)
        {
          prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH * 2U); /* in byte */
          prSvchMngmtData->usSetEnd = 0U;
          prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
          prSvchMngmtData->usNumWords = (CSMD_USHORT)(((prSvchMngmtData->usAct_Len + 1U) / 2U)
            - (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH);
        }
        else
        {
          prSvchMngmtData->usLength = prSvchMngmtData->usAct_Len;
          prSvchMngmtData->usSetEnd = 1U;
          prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
        }
      
        prSvchMngmtData->usAct_Position = 0U;
      }
    }
    else
    {
      /*should not be here*/
      return (CSMD_SYSTEM_ERROR);
    }
    
    if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0]) & CSMD_SVC_CTRL_M_BUSY) )
      return (CSMD_MBUSY_NOT_SET);
    
    CSMD_SVCHRead( prCSMD_Instance, prSvchMngmtData );
    
    break;
    
    
  case CSMD_REQUEST_IN_PROGRESS:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError( prSVC, prSvchMngmtData, TRUE );
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
    if (prSvchMngmtData->usIsList)
    {
      /*copy data from SVCH-Container to user buffer*/
      /*if ListLength already read in a previous step build list length info */
      usLoop = 0;
      if (prSvchMngmtData->usLLR)     /* List length to be read */
      {
        prSvchMngmtData->pusAct_Data[0] = prSvchMngmtData->usAct_Len;
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->pusAct_Data[1] = prSvchMngmtData->usMax_Len;
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->usLLR = 0U;
      }
      else if (prSvchMngmtData->usAct_Position == 0)
      {
        prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort(&prSVC->sRD_BUFF[usLoop++]);
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort(&prSVC->sRD_BUFF[usLoop++]);
        prSvchMngmtData->usAct_Position++;
      }
      
      for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop++)
      {
        prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position] = 
          CSMD_HAL_ReadShortNoConv( &prSVC->sRD_BUFF[usLoop] );
        /*add offset*/
        prSvchMngmtData->usAct_Position++;
      }
      
      if (prSvchMngmtData->usNumWords > (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH)
      {
        prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH * 2U); /* in byte */
        prSvchMngmtData->usSetEnd = 0U;
        prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
        prSvchMngmtData->usNumWords -= (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH;
      }
      else
      {
        prSvchMngmtData->usLength = (CSMD_USHORT)(prSvchMngmtData->usNumWords * 2U);
#ifdef CSMD_SVC_LIST_SEGMENT
        if (prSvchMngmtData->usIsList == CSMD_ELEMENT_RD_LIST_SEGMENT)
        {
          /* SetEnd flag should be set:                                         */
          /*  - for transmitting less than 4 bytes of the current list length   */
          /*  - if requested length is lower equal than the current list length */
          
          if (   ((((prSvchMngmtData->usAct_Len + 3)/4) * 4 - prSvchMngmtData->usRequestedLength) < 4)
              && (prSvchMngmtData->usAct_Len >= prSvchMngmtData->usRequestedLength))
          {
            prSvchMngmtData->usSetEnd = 1U;
          }
          else
          {
            prSvchMngmtData->usSetEnd = 0U;
          }
        }
        else
#endif
        {
          prSvchMngmtData->usSetEnd =1U;
        }
        prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
      } 
      
      if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY))
      {
        return (CSMD_MBUSY_NOT_SET);
      }
      
      CSMD_SVCHRead( prCSMD_Instance, prSvchMngmtData );
    }
    else
    {
      /*if not a list we should not be here*/
      return (CSMD_SYSTEM_ERROR);
    }
    
    break;
    
    
  case CSMD_LAST_STEP:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError( prSVC, prSvchMngmtData, TRUE );
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
    if (prSvchMngmtData->usIsList) /*Data of fixed Length*/
    {   
      usLoop = 0;
      if (prSvchMngmtData->usLLR)
      {
        prSvchMngmtData->pusAct_Data[0] = prSvchMngmtData->usAct_Len;
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->pusAct_Data[1] = prSvchMngmtData->usMax_Len;
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->usLLR = 0U;
      }
      else if (prSvchMngmtData->usAct_Position == 0)
      {
        prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop++] );
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop++] );
        prSvchMngmtData->usAct_Position++;
      }
      
      for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop++)
      {
        prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position] = 
          CSMD_HAL_ReadShortNoConv(&prSVC->sRD_BUFF[usLoop]);
        /*add offset*/
        prSvchMngmtData->usAct_Position++;
      }
      
    } /* End of if: Data of fixed length*/
    
    else
    {
      /*should not be here*/
      return (CSMD_SYSTEM_ERROR);
      
    }/* end of else of Variable  length data*/
    
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    /*lint -fallthrough */
    
    
  case CSMD_FINISHED_REQUEST:
    /* Call the Callback function*/
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }
    break;
    
    
  default:
    return (CSMD_SYSTEM_ERROR);
    
  }   /* end: switch (prSvchMngmtData->usActStateAtomic) */
  
  return (CSMD_NO_ERROR);
  
}   /* End: CSMD_GetName()*/



/**************************************************************************/ /**
\brief Reads the unit (data block element 4) of an IDN that is
       already open.

\ingroup func_svc_atomic
<B>Description of the process of the atomic function state machine:</B>

- #CSMD_INIT_REQUEST:\n
  Initialization of the containers and start of the transmission.

- #CSMD_REQUEST_IN_PROGRESS:\n
  Reading of the element.
  The function remains in this state until all data has been read.

- #CSMD_FINISHED_REQUEST:\n
  SVCH enabled, end of the function
  <BR>

  Element of rSvchMngmtData      | Content
  :----------------------------- | :------
  CSMD_USHORT  usSlaveIdx        | Slave index
  CSMD_ULONG   ulIdent_Nbr       | Parameter IDN
  CSMD_USHORT  usElem            | Element: 4
  CSMD_USHORT *pusAct_Data       | Pointer to buffer where the unit should be stored in
  CSMD_USHORT  usPriority        | Don`t care
  CSMD_USHORT  usLength          | Length of data to be read in bytes (don't care for elements 2 and 4, data is read directly): 0
  CSMD_USHORT  usIsList          | Element 4 is a list: 1
  CSMD_ULONG   ulAttribute       | Don`t care: 0
  CSMD_USHORT  usSvchError       | Filled by the function in case of error: 0
  CSMD_USHORT  usCancelActTrans  | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usActStateAtomic  | Managed internally: CSMD_INIT_REQUEST
  CSMD_USHORT  usAct_Position    | Managed internally: 0
  CSMD_USHORT  usReplaceable     | Don't care: 0
  CSMD_S3_SVCH_SW  SVCH_Status   | Filled by the function: 0
  CSMD_USHORT  usAct_Len         | Current length of the element (possibly read via CSMD_GetListLength() ): ActLen
  CSMD_USHORT  usMax_Len         | Maximum length of element (possibly read via CSMD_GetListLength() ): 0
  CSMD_USHORT  usRequestedLength | Don't care
  CSMD_USHORT  usSrv_Cont        | Software(1) or hardware(0) processing: 0
  CSMD_USHORT  usSetEnd          | Managed internally for lists: 0
  CSMD_USHORT  usCycle           | Internal variable: 0
  CSMD_USHORT  usNumWords        | Internal variable: 0
  CSMD_USHORT  usInUse           | Internal variable: 0
  CSMD_USHORT  usChannelOpen     | Internal variable; is set to 1 after SVC has been opened
  CSMD_USHORT  usRequestCanceled | Internal variable; is set after the request has been canceled: 0
  CSMD_USHORT  usLLR             | Internal variable; indicates if length info of the list has been read yet\n length info is transmitted to the list during the following read access (0)\n or not (1): Don't change
  CSMD_USHORT  usIntReqPend      | Internal variable, set by macro function
  CSMD_USHORT  usEmptyList       | Don't care

<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however, not
   imperatively necessary.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance.
\param [in]   prSvchMngmtData
              Pointer to macro management structure
\param [out]  CallBackFunc
              Pointer to callback function
              
\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       WK 
\date         13.01.2005

*******************************************************************************/
CSMD_FUNC_RET CSMD_GetUnit( CSMD_INSTANCE          *prCSMD_Instance,
                            CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData,
                            CSMD_VOID              (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_USHORT    usLoop;
  CSMD_SERC3SVC *prSVC;
  CSMD_FUNC_RET  eRet;
  
  
  if (prSvchMngmtData->usCancelActTrans)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_REQUEST_CANCELED);
  }
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  

  switch (prSvchMngmtData->usActStateAtomic)
  {
    
  case CSMD_INIT_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      return (CSMD_WRONG_SLAVE_INDEX);
    }
    if (prSvchMngmtData->usIsList)
    {
#ifdef CSMD_SVC_LIST_SEGMENT
      if (prSvchMngmtData->usIsList == CSMD_ELEMENT_RD_LIST_SEGMENT)
      {
        /* Read list segment */
        if (((prSvchMngmtData->usRequestedLength + 1U) / 2U) > (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH)
        {
          prSvchMngmtData->usNumWords = (CSMD_USHORT)(((prSvchMngmtData->usRequestedLength + 1U) / 2U)
            - (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH);
          prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH * 2U); /* in byte */
          prSvchMngmtData->usSetEnd = 0U;
          prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
        }
        else
        {
          prSvchMngmtData->usLength = prSvchMngmtData->usRequestedLength;
          /* SetEnd flag should be set:                                         */
          /*  - for transmitting less than 4 bytes of the current list length   */
          /*  - if requested length is lower equal than the current list length */
          
          if (   ((((prSvchMngmtData->usAct_Len + 3)/4) * 4 - prSvchMngmtData->usRequestedLength) < 4)
              && (prSvchMngmtData->usAct_Len >= prSvchMngmtData->usRequestedLength))
          {
            prSvchMngmtData->usSetEnd = 1U;
          }
          else
          {
            prSvchMngmtData->usSetEnd = 0U;
          }
          
          prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
        }
        
        prSvchMngmtData->usAct_Position = 0U;
      }
      else
#endif
      {
        if (((prSvchMngmtData->usAct_Len + 1U) / 2U) > (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH)
        {
          prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH * 2U); /* in byte */
          prSvchMngmtData->usSetEnd = 0U;
          prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
          prSvchMngmtData->usNumWords = (CSMD_USHORT)(((prSvchMngmtData->usAct_Len + 1U) / 2U)
            - (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH);
        }
        else
        {
          prSvchMngmtData->usLength = prSvchMngmtData->usAct_Len;
          prSvchMngmtData->usSetEnd = 1U;
          prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
        }
      
        prSvchMngmtData->usAct_Position = 0U;
      }
    }
    else
    {
      /*should not be here*/
      return (CSMD_SYSTEM_ERROR);
    }
    
    if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY))
      return (CSMD_MBUSY_NOT_SET);
    
    CSMD_SVCHRead( prCSMD_Instance, prSvchMngmtData );
    
    break;
    
    
  case CSMD_REQUEST_IN_PROGRESS:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError( prSVC, prSvchMngmtData, TRUE );
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
    if (prSvchMngmtData->usIsList)
    {
      /*copy data from SVCH-Container to user buffer*/
      usLoop = 0;
      if (prSvchMngmtData->usLLR)
      {
        prSvchMngmtData->pusAct_Data[0] = prSvchMngmtData->usAct_Len;
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->pusAct_Data[1] = prSvchMngmtData->usMax_Len;
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->usLLR = 0U;
      }
      else if (prSvchMngmtData->usAct_Position == 0)
      {
        prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop++] );
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop++] );
        prSvchMngmtData->usAct_Position++;
      }
      
      for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop++)
      {
        prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position] = 
          CSMD_HAL_ReadShortNoConv(&prSVC->sRD_BUFF[usLoop]);
        
        /*add offset*/
        prSvchMngmtData->usAct_Position++;
      }
      
      if (prSvchMngmtData->usNumWords > (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH)
      {
        prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH * 2U); /* in byte */
        prSvchMngmtData->usSetEnd = 0U;
        prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
        prSvchMngmtData->usNumWords -= (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH;
      }
      else
      {
        prSvchMngmtData->usLength = (CSMD_USHORT)(prSvchMngmtData->usNumWords * 2U);
#ifdef CSMD_SVC_LIST_SEGMENT
        if (prSvchMngmtData->usIsList == CSMD_ELEMENT_RD_LIST_SEGMENT)
        {
          /* SetEnd flag should be set:                                         */
          /*  - for transmitting less than 4 bytes of the current list length   */
          /*  - if requested length is lower equal than the current list length */
          
          if (   ((((prSvchMngmtData->usAct_Len + 3)/4) * 4 - prSvchMngmtData->usRequestedLength) < 4)
              && (prSvchMngmtData->usAct_Len >= prSvchMngmtData->usRequestedLength))
          {
            prSvchMngmtData->usSetEnd = 1U;
          }
          else
          {
            prSvchMngmtData->usSetEnd = 0U;
          }
        }
        else
#endif
        {
          prSvchMngmtData->usSetEnd = 1U;
        }
        prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
      } 
      
      if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY))
      {
        return (CSMD_MBUSY_NOT_SET);
      }
      
      CSMD_SVCHRead( prCSMD_Instance, prSvchMngmtData );
    }
    else
    {
      /*if not a list we should not be here*/
      return (CSMD_SYSTEM_ERROR);
    }
    
    break;
    
    
  case CSMD_LAST_STEP:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError( prSVC, prSvchMngmtData, TRUE );
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
    if (prSvchMngmtData->usIsList) /*Data of fixed Length*/
    {   
      usLoop = 0;
      if (prSvchMngmtData->usLLR)
      {
        prSvchMngmtData->pusAct_Data[0] = prSvchMngmtData->usAct_Len;
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->pusAct_Data[1] = prSvchMngmtData->usMax_Len;
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->usLLR = 0U;
      }
      else if (prSvchMngmtData->usAct_Position == 0)
      {
        prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop++] );
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop++] );
        prSvchMngmtData->usAct_Position++;
      }
      
      for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop++)
      {
        prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position] = 
          CSMD_HAL_ReadShortNoConv( &prSVC->sRD_BUFF[usLoop] );
        
        /*add offset*/
        prSvchMngmtData->usAct_Position++;
      }
      prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
      
    } /* End of if: Data of fixed length*/
    
    else
    {
      /*should not be here*/
      return (CSMD_SYSTEM_ERROR);
      
    }/* end of else of Variable  length data*/
    /*lint -fallthrough */
    
    
  case CSMD_FINISHED_REQUEST:
    /* Call the Callback function*/
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }
    break;
    
    
  default:
    return (CSMD_SYSTEM_ERROR);
    
  }   /* end: switch (prSvchMngmtData->usActStateAtomic) */
  
  return (CSMD_NO_ERROR);
  
}   /* End: CSMD_GetUnit()*/



/**************************************************************************/ /**
\brief Reads the minimum (data block element 5) of an IDN that is already open.

\ingroup func_svc_atomic
<B>Description of the process of the atomic function state machine:</B>

- #CSMD_INIT_REQUEST:\n
  Initialization of the containers and start of the transmission

- #CSMD_REQUEST_IN_PROGRESS:\n
  Reading of the element. The function remains
  in this state until all data has been read.

- #CSMD_FINISHED_REQUEST:\n
  SVCH enabled, end of the function
  <BR>

  Element of rSvchMngmtData      | Content
  :----------------------------- | :------
  CSMD_USHORT  usSlaveIdx        | Slave index
  CSMD_ULONG   ulIdent_Nbr       | Parameter IDN
  CSMD_USHORT  usElem            | Element: 5
  CSMD_USHORT *pusAct_Data       | Pointer to buffer where the minimum input value should be stored in
  CSMD_USHORT  usPriority        | Don`t care
  CSMD_USHORT  usLength          | Length of data to be read in bytes: Len
  CSMD_USHORT  usIsList          | Element 5 is not a list: 0
  CSMD_ULONG   ulAttribute       | Don't care: 0
  CSMD_USHORT  usSvchError       | Filled by the function in case of error: 0
  CSMD_USHORT  usCancelActTrans  | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usActStateAtomic  | Managed internally: CSMD_INIT_REQUEST
  CSMD_USHORT  usAct_Position    | Managed internally: 0
  CSMD_USHORT  usReplaceable     | Don't care: 0
  CSMD_S3_SVCH_SW  SVCH_Status   | Filled by the function: 0
  CSMD_USHORT  usAct_Len         | Don't care: 0
  CSMD_USHORT  usMax_Len         | Don't care: 0
  CSMD_USHORT  usRequestedLength | Don't care
  CSMD_USHORT  usSrv_Cont        | Software(1) or hardware(0) processing: 0
  CSMD_USHORT  usSetEnd          | No list: managed by the function: 1
  CSMD_USHORT  usCycle           | Internal variable: 0
  CSMD_USHORT  usNumWords        | Internal variable: 0
  CSMD_USHORT  usInUse           | Internal variable: 0
  CSMD_USHORT  usChannelOpen     | Internal variable; is set to 1 after SVC has been opened
  CSMD_USHORT  usRequestCanceled | Internal variable; is set after the request has been canceled: 0
  CSMD_USHORT  usLLR             | Internal variable; length of list parameter has been read: 0
  CSMD_USHORT  usIntReqPend      | Internal variable, set by macro function
  CSMD_USHORT  usEmptyList       | Don't care

<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however not
   imperatively necessary.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance.
\param [in]   prSvchMngmtData
              Pointer to macro management structure
\param [out]  CallBackFunc
              Pointer to callback function
              
\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       WK 
\date         13.01.2005
  
*******************************************************************************/
CSMD_FUNC_RET CSMD_GetMin( CSMD_INSTANCE          *prCSMD_Instance,
                           CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData,
                           CSMD_VOID              (*CallBackFunc)(CSMD_VOID))
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_SERC3SVC *prSVC;
  CSMD_FUNC_RET  eRet;
  
  
  if (prSvchMngmtData->usCancelActTrans)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_REQUEST_CANCELED);
  }
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  
  
  switch (prSvchMngmtData->usActStateAtomic)
  {
  case CSMD_INIT_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      return (CSMD_WRONG_SLAVE_INDEX);
    }
    prSvchMngmtData->usSetEnd = 1U;
    prSvchMngmtData->usLLR = 0U;
    
    if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY))
      return (CSMD_MBUSY_NOT_SET);
    
    CSMD_SVCHRead( prCSMD_Instance, prSvchMngmtData );
    
    prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
    
    break;
    
  case CSMD_REQUEST_IN_PROGRESS:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError( prSVC, prSvchMngmtData, TRUE );
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
#ifdef CSMD_BIG_ENDIAN
    if (prSvchMngmtData->usLength == 4U)  /* Data length of 2 words */
    { 
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] );
      prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
    }
    else if (prSvchMngmtData->usLength == 2U) /* Data length 1 word */
    {
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
    }
    else /*Data of 4 Words (= 8 Byte) */
    {
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[3] );
      prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[2] );
      prSvchMngmtData->pusAct_Data[2] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] );
      prSvchMngmtData->pusAct_Data[3] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
    }
#else
    if (prSvchMngmtData->usLength == 4U)  /* Data length of 2 words */
    { 
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] ); 
      prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] );
    }
    else if (prSvchMngmtData->usLength == 2U) /* Data length 1 word */
    {
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
    }
    else /*Data of 4 Words (= 8 Byte) */
    {
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
      prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] );
      prSvchMngmtData->pusAct_Data[2] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[2] );
      prSvchMngmtData->pusAct_Data[3] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[3] );
    }
#endif
    
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    /*lint -fallthrough */
    
    
  case CSMD_FINISHED_REQUEST:
    
    /* Call the Callback function*/
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }
    break;
    
    
  default :
    return (CSMD_SYSTEM_ERROR);
    
  }   /* end: switch (prSvchMngmtData->usActStateAtomic) */
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_GetMin()*/



/**************************************************************************/ /**
\brief Reads the maximum (data block element 6) of an IDN that is already open.

\ingroup func_svc_atomic
<B>Description of the process of the atomic function state machine:</B>

- #CSMD_INIT_REQUEST:\n
  Initialization of the containers and start of the transmission

- #CSMD_REQUEST_IN_PROGRESS:\n
  Reading of the element. The function remains in
  this state until all data has been read.

- #CSMD_FINISHED_REQUEST:\n
  SVCH enabled, end of the function
  <BR>

  Element of rSvchMngmtData      | Content
  :----------------------------- | :------
  CSMD_USHORT  usSlaveIdx        | Slave index
  CSMD_ULONG   ulIdent_Nbr       | Parameter IDN
  CSMD_USHORT  usElem            | Element: 6
  CSMD_USHORT *pusAct_Data       | Pointer to buffer where the maximum input value should be stored in
  CSMD_USHORT  usPriority        | Don`t care
  CSMD_USHORT  usLength          | Length of data to be read in bytes: Len
  CSMD_USHORT  usIsList          | Element 6 is not a list: 0
  CSMD_ULONG   ulAttribute       | Don't care: 0
  CSMD_USHORT  usSvchError       | Filled by the function in case of error: 0
  CSMD_USHORT  usCancelActTrans  | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usActStateAtomic  | Managed internally: CSMD_INIT_REQUEST
  CSMD_USHORT  usAct_Position    | Managed internally: 0
  CSMD_USHORT  usReplaceable     | Don't care: 0
  CSMD_S3_SVCH_SW  SVCH_Status   | Filled by the function: 0
  CSMD_USHORT  usAct_Len         | Don't care: 0
  CSMD_USHORT  usMax_Len         | Don't care: 0
  CSMD_USHORT  usRequestedLength | Don't care
  CSMD_USHORT  usSrv_Cont        | Software(1) or hardware(0) processing: 0
  CSMD_USHORT  usSetEnd          | No list: managed by the function: 1
  CSMD_USHORT  usCycle           | Internal variable: 0
  CSMD_USHORT  usNumWords        | Internal variable: 0
  CSMD_USHORT  usInUse           | Internal variable: 0
  CSMD_USHORT  usChannelOpen     | Internal variable; is set to 1 after SVC has been opened
  CSMD_USHORT  usRequestCanceled | Internal variable; is set after the request has been canceled: 0
  CSMD_USHORT  usLLR             | Internal variable; length of list parameter has been read: 0
  CSMD_USHORT  usIntReqPend      | Internal variable, set by macro function
  CSMD_USHORT  usEmptyList       | Don't care

<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however, not
   imperatively necessary.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance.
\param [in]   prSvchMngmtData
              Pointer to macro management structure
\param [out]  CallBackFunc
              Pointer to callback function
              
\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       WK 
\date         13.01.2005
  
*******************************************************************************/
CSMD_FUNC_RET CSMD_GetMax( CSMD_INSTANCE          *prCSMD_Instance,
                           CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData,
                           CSMD_VOID              (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_SERC3SVC *prSVC;
  CSMD_FUNC_RET  eRet;
  
  
  if (prSvchMngmtData->usCancelActTrans)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_REQUEST_CANCELED);
  }
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  
  
  switch (prSvchMngmtData->usActStateAtomic)
  {
  case CSMD_INIT_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      return (CSMD_WRONG_SLAVE_INDEX);
    }
    prSvchMngmtData->usSetEnd = 1U;
    prSvchMngmtData->usLLR = 0U;
    
    if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY))
      return (CSMD_MBUSY_NOT_SET);
    
    CSMD_SVCHRead( prCSMD_Instance, prSvchMngmtData );
    
    prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
    
    break;
    
    
  case CSMD_REQUEST_IN_PROGRESS:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError( prSVC, prSvchMngmtData, TRUE );
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
#ifdef CSMD_BIG_ENDIAN
    if (prSvchMngmtData->usLength == 4U)  /* Data length of 2 words */
    { 
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] ); 
      prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
    }
    else if (prSvchMngmtData->usLength == 2U) /* Data length 1 word */
    {
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
    }
    else /*Data of 4 Words (= 8 Byte) */
    {
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[3] );
      prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[2] );
      prSvchMngmtData->pusAct_Data[2] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] );
      prSvchMngmtData->pusAct_Data[3] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
    }
#else
    if (prSvchMngmtData->usLength == 4U)  /* Data length of 2 words */
    { 
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] ); 
      prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] );
    }
    else if (prSvchMngmtData->usLength == 2U) /* Data length 1 word */
    {
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
    }
    else /*Data of 4 Words (= 8 Byte) */
    {
      prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
      prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] );
      prSvchMngmtData->pusAct_Data[2] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[2] );
      prSvchMngmtData->pusAct_Data[3] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[3] );
    }
#endif
    
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    /*lint -fallthrough */
    
    
  case CSMD_FINISHED_REQUEST:
    
    /* Call the Callback function*/
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }
    break;
    
    
  default :
    return (CSMD_SYSTEM_ERROR);
    
  }   /* end: switch (prSvchMngmtData->usActStateAtomic) */
  
  return (CSMD_NO_ERROR);
  
}   /* End: CSMD_GetMax()*/



/**************************************************************************/ /**
\brief Reads the operation data (data block element 7) of an
       IDN that is already open.

\ingroup func_svc_atomic
<B>Description of the process of the atomic function state machine:</B>

- #CSMD_INIT_REQUEST:\n
  Initialization of the containers and start of the transmission

- #CSMD_REQUEST_IN_PROGRESS:\n
  Reading of the element. The function remains in
  this state until all data has been read.

- #CSMD_FINISHED_REQUEST:\n
  SVCH enabled, end of the function
  <BR>

  Element of rSvchMngmtData      | Content
  :----------------------------- | :------
  CSMD_USHORT  usSlaveIdx        | Slave index
  CSMD_ULONG   ulIdent_Nbr       | Parameter IDN
  CSMD_USHORT  usElem            | Element: 7
  CSMD_USHORT *pusAct_Data       | Pointer to buffer where the data should be stored in
  CSMD_USHORT  usPriority        | Don`t care
  CSMD_USHORT  usLength          | Length of data to be read in bytes (in case of a list including 4 bytes length info): Len, \n don't care if usIsList = 4
  CSMD_USHORT  usIsList          | For list: 1, else 0
  CSMD_ULONG   ulAttribute       | Don't care: 0
  CSMD_USHORT  usSvchError       | Filled by the function in case of error: 0
  CSMD_USHORT  usCancelActTrans  | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usActStateAtomic  | Managed internally: CSMD_INIT_REQUEST
  CSMD_USHORT  usAct_Position    | Managed internally: 0
  CSMD_USHORT  usReplaceable     | Don't care: 0
  CSMD_S3_SVCH_SW  SVCH_Status   | Filled by the function: 0
  CSMD_USHORT  usAct_Len         | For list: current length, else 0 (don't care)
  CSMD_USHORT  usMax_Len         | For list: maximum length (don't care), else 0 (don't care)
  CSMD_USHORT  usRequestedLength | Length (inclusive 4 bytes for length info) of the requested list segment \n if usIsList = 4 (else don't care)
  CSMD_USHORT  usSrv_Cont        | Software(1) or hardware(0) processing: 0
  CSMD_USHORT  usSetEnd          | Managed internally; for lists: 0
  CSMD_USHORT  usCycle           | Internal variable: 0
  CSMD_USHORT  usNumWords        | Internal variable: 0
  CSMD_USHORT  usInUse           | Internal variable: 0
  CSMD_USHORT  usChannelOpen     | Internal variable; is set to 1 after SVC has been opened
  CSMD_USHORT  usRequestCanceled | Internal variable; is set after the request has been canceled: 0
  CSMD_USHORT  usLLR             | Internal variable; indicates if length info of the list has been read yet / \n length info is transmitted to the list during the following read access (0)\n or not (1): don't change
  CSMD_USHORT  usIntReqPend      | Internal variable, set by macro function
  CSMD_USHORT  usEmptyList       | Don't care

<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however, not 
   imperatively necessary. Before the call-up, a sufficiently large memory
   must have been allocated for the list data if a list is concerned. Breakdown
   of the transmission into several list segments is not intended.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]   prSvchMngmtData
              Pointer to macro management structure
\param [out]  CallBackFunc
              Pointer to callback function
              
\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       WK 
\date         13.01.2005

*******************************************************************************/
CSMD_FUNC_RET CSMD_GetData( CSMD_INSTANCE          *prCSMD_Instance,
                            CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData,
                            CSMD_VOID              (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_USHORT    usLoop;
  CSMD_SERC3SVC *prSVC;
  CSMD_FUNC_RET  eRet;
  
  
  if (prSvchMngmtData->usCancelActTrans)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_REQUEST_CANCELED);
  }
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  
  
  switch (prSvchMngmtData->usActStateAtomic)
  {
    
  case CSMD_INIT_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      return (CSMD_WRONG_SLAVE_INDEX);
    }
    if (prSvchMngmtData->usIsList)
    {
#ifdef CSMD_SVC_LIST_SEGMENT
      if (prSvchMngmtData->usIsList == CSMD_ELEMENT_RD_LIST_SEGMENT)
      {
        /* Read list segment */
        if (((prSvchMngmtData->usRequestedLength + 1U) / 2U) > (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH)
        {
#ifdef CSMD_BIG_ENDIAN
          if ((prSvchMngmtData->ulAttribute & CSMD_SERC_LEN) == CSMD_SERC_VAR_DOUBLE_LEN)
          {
            /* First access fits with complete elements and length information into the read buffer */
            prSvchMngmtData->usNumWords = (CSMD_USHORT)(((prSvchMngmtData->usRequestedLength + 1U) / 2U)
              - (CSMD_USHORT)(CSMD_SERC_SC_WRBUF_LENGTH - 2U));
            prSvchMngmtData->usLength = (CSMD_USHORT)( (CSMD_USHORT)(CSMD_SERC_SC_WRBUF_LENGTH - 2U) * 2); /* in byte */
          }
          else
#endif
          {
            prSvchMngmtData->usNumWords = (CSMD_USHORT)(((prSvchMngmtData->usRequestedLength + 1U) / 2U)
              - (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH);
            prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH * 2U); /* in byte */
          }
          prSvchMngmtData->usSetEnd = 0U;
          prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
        }
        else
        {
          prSvchMngmtData->usLength = prSvchMngmtData->usRequestedLength;
          /* SetEnd flag should be set:                                         */
          /*  - for transmitting less than 4 bytes of the current list length   */
          /*  - if requested length is lower equal than the current list length */
          
          if (   ((((prSvchMngmtData->usAct_Len + 3)/4) * 4 - prSvchMngmtData->usRequestedLength) < 4)
              && (prSvchMngmtData->usAct_Len >= prSvchMngmtData->usRequestedLength))
          {
            prSvchMngmtData->usSetEnd = 1U;
          }
          else
          {
            prSvchMngmtData->usSetEnd = 0U;
          }
          
          prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
        }
        
        prSvchMngmtData->usAct_Position = 0U;
      }
      else
#endif
      {
        prSvchMngmtData->usAct_Len = prSvchMngmtData->usLength;
        
        if (((prSvchMngmtData->usAct_Len + 1U) / 2U) > (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH)
        {
#ifdef CSMD_BIG_ENDIAN
          if ((prSvchMngmtData->ulAttribute & CSMD_SERC_LEN) == CSMD_SERC_VAR_DOUBLE_LEN)
          {
            /* First access fits with complete elements and length information into the read buffer */
            prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)(CSMD_SERC_SC_WRBUF_LENGTH - 2U) * 2U); /* in byte */
            prSvchMngmtData->usNumWords = (CSMD_USHORT)(((prSvchMngmtData->usAct_Len + 1U) / 2U)
              - (CSMD_USHORT)(CSMD_SERC_SC_WRBUF_LENGTH - 2U));
          }
          else
#endif
          {
            prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH * 2U); /* in byte */
            prSvchMngmtData->usNumWords = (CSMD_USHORT)(((prSvchMngmtData->usAct_Len + 1U) / 2U)
              - (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH);
          }
          prSvchMngmtData->usSetEnd = 0U;
          prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
        }
        else
        {
          prSvchMngmtData->usLength = prSvchMngmtData->usAct_Len;
          prSvchMngmtData->usSetEnd = 1U;
          prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
        }
      
        prSvchMngmtData->usAct_Position = 0U;
      }
    }
    else
    {
      prSvchMngmtData->usSetEnd = 1U;
      prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
    }
    
    if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY))
      return (CSMD_MBUSY_NOT_SET);
    
    CSMD_SVCHRead( prCSMD_Instance, prSvchMngmtData );
    
    break;
    
    
  case CSMD_REQUEST_IN_PROGRESS:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError( prSVC,prSvchMngmtData, TRUE );
    
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
    if (prSvchMngmtData->usIsList)
    {
      /*copy data from SVCH-Container to user buffer*/
      usLoop = 0;
      if (prSvchMngmtData->usLLR)
      {
        prSvchMngmtData->pusAct_Data[0] = prSvchMngmtData->usAct_Len;
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->pusAct_Data[1] = prSvchMngmtData->usMax_Len;
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->usLLR = 0U;
      }
      else if (prSvchMngmtData->usAct_Position == 0)
      {
        prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop++] );
        prSvchMngmtData->usAct_Position++;
        prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop++] );
        prSvchMngmtData->usAct_Position++;
      }
      
#ifdef CSMD_BIG_ENDIAN
      switch (prSvchMngmtData->ulAttribute & CSMD_SERC_LEN)
      {
      case CSMD_SERC_VAR_BYTE_LEN:
        for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop++)
        {
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
            CSMD_HAL_ReadShortNoConv( &prSVC->sRD_BUFF[usLoop] );
        }
        break;
      case CSMD_SERC_VAR_WORD_LEN:
        for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop++)
        {
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
            CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop] );
        }
        break;
      case CSMD_SERC_VAR_LONG_LEN:
        for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop += 2)
        {
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
            CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop+1] );
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
            CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop] );
        }
        break;
      case CSMD_SERC_VAR_DOUBLE_LEN:
        for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop += 4)
        {
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
            CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop+3] );
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
            CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop+2] );
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
            CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop+1] );
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
            CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop] );
        }
        break;
      default:
        /* Should not be here.! */
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
        break;
      }   /* End: switch (prSvchMngmtData->ulAttribute & CSMD_SERC_LEN) */
#else
      for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop++)
      {
        *(CSMD_USHORT *)(prSvchMngmtData->pusAct_Data + prSvchMngmtData->usAct_Position) = 
          CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop] );
        
        /*add offset*/
        prSvchMngmtData->usAct_Position++;
      }
#endif
      
      if (prSvchMngmtData->usNumWords > (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH)
      {
        prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH * 2U); /* in byte */
        prSvchMngmtData->usSetEnd = 0U;
        prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
        prSvchMngmtData->usNumWords -= (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH;
      }
      else
      {
        prSvchMngmtData->usLength = (CSMD_USHORT)(prSvchMngmtData->usNumWords * 2U);
#ifdef CSMD_SVC_LIST_SEGMENT
        if (prSvchMngmtData->usIsList == CSMD_ELEMENT_RD_LIST_SEGMENT)
        {
          /* SetEnd flag should be set:                                         */
          /*  - for transmitting less than 4 bytes of the current list length   */
          /*  - if requested length is lower equal than the current list length */
          
          if (   ((((prSvchMngmtData->usAct_Len + 3)/4) * 4 - prSvchMngmtData->usRequestedLength) < 4)
              && (prSvchMngmtData->usAct_Len >= prSvchMngmtData->usRequestedLength))
          {
            prSvchMngmtData->usSetEnd = 1U;
          }
          else
          {
            prSvchMngmtData->usSetEnd = 0U;
          }
        }
        else
#endif
        {
          prSvchMngmtData->usSetEnd = 1U;
        }
        prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
      } 
      
      if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY))
      {
        return (CSMD_MBUSY_NOT_SET);
      }
      
      CSMD_SVCHRead( prCSMD_Instance, prSvchMngmtData );
    }
    else
    {
      /*if not a list we should not be here*/
      return (CSMD_SYSTEM_ERROR);
    }
    
    break;
    
    
  case CSMD_LAST_STEP:
      
      /* check error flags */
      eRet = CSMD_CheckSVCError( prSVC, prSvchMngmtData, TRUE );
      
      if (eRet != CSMD_NO_ERROR)
        return (eRet);
      
      if (!prSvchMngmtData->usIsList) /*Data of fixed Length*/
      {   
#ifdef CSMD_BIG_ENDIAN
        if (prSvchMngmtData->usLength == 4U)  /* Data length of 2 words */
        { 
          prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] );
          prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
        }
        else if (prSvchMngmtData->usLength == 2U) /* Data length 1 word */
        {
          prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
        }
        else /*Data of 4 Words (= 8 Byte) */
        {
          prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[3] );
          prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[2] );
          prSvchMngmtData->pusAct_Data[2] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] );
          prSvchMngmtData->pusAct_Data[3] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
        }
#else
        if (prSvchMngmtData->usLength == 4U)  /* Data length of 2 words */
        { 
          prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
          prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] );
        }
        else if (prSvchMngmtData->usLength == 2U) /* Data length 1 word */
        {
          prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );
        }
        else /*Data of 4 Words (= 8 Byte) */
        {
          for (usLoop = 0; usLoop < (prSvchMngmtData->usLength/2); usLoop++)
          {
            prSvchMngmtData->pusAct_Data[usLoop] = 
              CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop] );
          }
        }
#endif
      } /* End of if: Data of fixed length */
      
      else
      {
        /*copy data from SVCH-Container to user buffer*/
        usLoop = 0;
        if (prSvchMngmtData->usLLR)     /* List length to be read? */
        {
          prSvchMngmtData->pusAct_Data[0] = prSvchMngmtData->usAct_Len;
          prSvchMngmtData->usAct_Position++;
          prSvchMngmtData->pusAct_Data[1] = prSvchMngmtData->usMax_Len;
          prSvchMngmtData->usAct_Position++;
          prSvchMngmtData->usLLR = 0U;
        }
        else if (prSvchMngmtData->usAct_Position == 0)
        {
          prSvchMngmtData->pusAct_Data[0] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop++] );
          prSvchMngmtData->usAct_Position++;
          prSvchMngmtData->pusAct_Data[1] = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop++] );
          prSvchMngmtData->usAct_Position++;
        }
#ifdef CSMD_BIG_ENDIAN
        switch (prSvchMngmtData->ulAttribute & CSMD_SERC_LEN)
        {
        case CSMD_SERC_VAR_BYTE_LEN:
          for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop++)
          {
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
              CSMD_HAL_ReadShortNoConv( &prSVC->sRD_BUFF[usLoop] );
          }
          break;
        case CSMD_SERC_VAR_WORD_LEN:
          for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop++)
          {
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
              CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop] );
          }
          break;
        case CSMD_SERC_VAR_LONG_LEN:
          for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop += 2)
          {
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
              CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop+1] );
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
              CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop] );
          }
          break;
        case CSMD_SERC_VAR_DOUBLE_LEN:
          for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop += 4)
          {
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
              CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop+3] );
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
              CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop+2] );
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
              CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop+1] );
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] = 
              CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop] );
          }
          break;
        default:
          /*should not be here...ToDo: Error return?.*/
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
          break;
        }   /* End: switch (prSvchMngmtData->ulAttribute & CSMD_SERC_LEN) */
#else
        for ( ; usLoop < (prSvchMngmtData->usLength + 1U) / 2U; usLoop++)
        {
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position] = 
            CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[usLoop] );
          
          /*add offset*/
          prSvchMngmtData->usAct_Position++;
        }
#endif
      }/* end of else of Variable  length data*/
      
      prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
      /*lint -fallthrough */
      
      
    case CSMD_FINISHED_REQUEST:
      /* Call the Callback function */
      if (CallBackFunc != NULL)
      {
        CallBackFunc();
      }
      break;
      
      
    default:
      return (CSMD_SYSTEM_ERROR);
      
  }   /* end: switch (prSvchMngmtData->usActStateAtomic) */
  
  return (CSMD_NO_ERROR);
  
}   /* End: CSMD_GetData() */



/**************************************************************************/ /**
\brief Writes the operation data (data block element 7) of an
 IDN that is already open.

\ingroup func_svc_atomic
<B>Description of the process of the atomic function state machine:</B>

- #CSMD_INIT_REQUEST:\n
  Initialization of the containers and start of the transmission

- #CSMD_REQUEST_IN_PROGRESS:\n
  Data is being transmitted. This status is
  maintained until all data has been transmitted.

- #CSMD_FINISHED_REQUEST:\n
  SVCH enabled, end of the function
  <BR>

  Element of rSvchMngmtData      | Content
  :----------------------------- | :------
  CSMD_USHORT  usSlaveIdx        | Slave index
  CSMD_ULONG   ulIdent_Nbr       | Parameter IDN
  CSMD_USHORT  usElem            | Element: 7
  CSMD_USHORT *pusAct_Data       | Pointer to the buffer for the data to be written
  CSMD_USHORT  usPriority        | Don`t care
  CSMD_USHORT  usLength          | Length of data to be write in bytes (in case of a list including 4 bytes length info): Len, \n don't care if usIsList = 4
  CSMD_USHORT  usIsList          | For list: 1, else 0
  CSMD_ULONG   ulAttribute       | Don't care: 0
  CSMD_USHORT  usSvchError       | Filled by the function in case of error: 0
  CSMD_USHORT  usCancelActTrans  | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usActStateAtomic  | Managed internally: CSMD_INIT_REQUEST
  CSMD_USHORT  usAct_Position    | Managed internally: 0
  CSMD_USHORT  usReplaceable     | Don't care: 0
  CSMD_S3_SVCH_SW  SVCH_Status   | Filled by the function: 0
  CSMD_USHORT  usAct_Len         | For list: current length, else 0 (don't care)
  CSMD_USHORT  usMax_Len         | For list: maximum length (don't care), else 0 (don't care)
  CSMD_USHORT  usRequestedLength | Don't care
  CSMD_USHORT  usSrv_Cont        | Software(1) or hardware(0) processing: 0
  CSMD_USHORT  usSetEnd          | Managed internally; for lists: 0
  CSMD_USHORT  usCycle           | Internal variable: 0
  CSMD_USHORT  usNumWords        | Internal variable: 0
  CSMD_USHORT  usInUse           | Internal variable: 0
  CSMD_USHORT  usChannelOpen     | Internal variable; is set to 1 after SVC has been opened
  CSMD_USHORT  usRequestCanceled | Internal variable; is set after the request has been canceled: 0
  CSMD_USHORT  usLLR             | Internal variable; length of list parameter has been read: 0
  CSMD_USHORT  usIntReqPend      | Internal variable, set by macro function
  CSMD_USHORT  usEmptyList       | Don't care

<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however, not
   imperatively necessary.

\param [in]   prCSMD_Instance 
              Pointer to allocated memory range for the variables of the
              CoSeMa instance.
\param [in]   prSvchMngmtData 
              Pointer to macro management structure
\param [out]  CallBackFunc
              Pointer to callback function
              
\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       WK 
\date         13.01.2005

*******************************************************************************/
CSMD_FUNC_RET CSMD_PutData( CSMD_INSTANCE          *prCSMD_Instance,
                            CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData,
                            CSMD_VOID              (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_SERC3SVC *prSVC;
  CSMD_FUNC_RET  eRet;
  
  
  if (prSvchMngmtData->usCancelActTrans)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_REQUEST_CANCELED);
  }
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  
  
  switch (prSvchMngmtData->usActStateAtomic)
  {
    
  case CSMD_INIT_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      return (CSMD_WRONG_SLAVE_INDEX);
    }
    if (prSvchMngmtData->usIsList == CSMD_ELEMENT_IS_LIST)
    {
      if (((prSvchMngmtData->usAct_Len + 1U) / 2U) > (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH)
      {
#ifdef CSMD_BIG_ENDIAN
        if ((prSvchMngmtData->ulAttribute & CSMD_SERC_LEN) == CSMD_SERC_VAR_DOUBLE_LEN)
        {
          /* First access fits with complete elements and length information into the write buffer */
          prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)(CSMD_SERC_SC_WRBUF_LENGTH - 2U) * 2U); /* in byte */
          prSvchMngmtData->usNumWords = (CSMD_USHORT)(((prSvchMngmtData->usAct_Len + 1) / 2U)
            - (CSMD_USHORT)(CSMD_SERC_SC_WRBUF_LENGTH - 2U));
        }
        else
#endif
        {
          prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH * 2U); /* in byte */
          prSvchMngmtData->usNumWords = (CSMD_USHORT)(((prSvchMngmtData->usAct_Len + 1U) / 2U)
            - (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH);
        }
        prSvchMngmtData->usSetEnd = 0U;
        prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
      }
      else
      {
        prSvchMngmtData->usLength = prSvchMngmtData->usAct_Len;
        prSvchMngmtData->usSetEnd = 1U;
        prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
      }
    }
    else
    {
      prSvchMngmtData->usSetEnd = 1U;
      prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
    }
    
    prSvchMngmtData->usAct_Position = 0U;
    
    if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY))
      return (CSMD_MBUSY_NOT_SET);
    
    eRet = CSMD_SVCHWrite( prCSMD_Instance, prSvchMngmtData );
    
    if (eRet != CSMD_NO_ERROR )
    {
      return (eRet);
    }
    
    break;
    
    
  case CSMD_REQUEST_IN_PROGRESS:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError( prSVC, prSvchMngmtData, FALSE );
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
    if (prSvchMngmtData->usIsList == CSMD_ELEMENT_IS_LIST)
    {
      if (prSvchMngmtData->usNumWords > (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH)
      {
        prSvchMngmtData->usLength = (CSMD_USHORT)((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH * 2U); /* in byte */
        prSvchMngmtData->usSetEnd = 0U;
        prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
        prSvchMngmtData->usNumWords -= (CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH;
      }
      else
      {
        prSvchMngmtData->usLength = (CSMD_USHORT)(prSvchMngmtData->usNumWords * 2U);
        prSvchMngmtData->usSetEnd = 1U;
        prSvchMngmtData->usActStateAtomic = CSMD_LAST_STEP;
      } 
      
      if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY))
      {
        return (CSMD_MBUSY_NOT_SET);
      }
      
      eRet = CSMD_SVCHWrite( prCSMD_Instance, prSvchMngmtData );
      
      if (eRet != CSMD_NO_ERROR )
      {
        return (eRet);
      }
    }
    else
    {
      /*if not a list we should not be here*/
      return (CSMD_SYSTEM_ERROR);
    }
    
    break;
    
    
  case CSMD_LAST_STEP:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError( prSVC, prSvchMngmtData, FALSE );
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    /*lint -fallthrough */
    
    
  case CSMD_FINISHED_REQUEST:
    
    /* Call the Callback function*/
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }
    break;
    
    
  default:
    return (CSMD_SYSTEM_ERROR);
    
  }   /* end: switch (prSvchMngmtData->usActStateAtomic) */
  
  return (CSMD_NO_ERROR);
  
}   /* End: CSMD_PutData()*/



/**************************************************************************/ /**
\brief Reads the data status of a parameter.

\ingroup func_svc_atomic
<B>Description of the process of the atomic function state machine:</B>

- #CSMD_INIT_REQUEST:\n
  Initialization of the containers and start of the IDN transmission

- #CSMD_REQUEST_IN_PROGRESS:\n
  Evaluation of the data status, data status is
  transmitted in the psActData data buffer.

- #CSMD_FINISHED_REQUEST:\n
  SVCH enabled, end of the function

- Data status for command parameters:
  - 0x0 not set and not enabled
  - 0x7 in process
  - 0xF Error, command execution impossible
  - 0x5 Command execution interrupted
  - 0x3 Command correctly executed
- Data status for non-command parameters:
  - 0x0000 Valid data
  - 0x0100 Data is invalid

  <BR>

  Element of rSvchMngmtData      | Content
  :----------------------------- | :------
  CSMD_USHORT  usSlaveIdx        | Slave index
  CSMD_ULONG   ulIdent_Nbr       | Parameter IDN
  CSMD_USHORT  usElem            | Element: 1
  CSMD_USHORT *pusAct_Data       | Pointer to data containing the IDN
  CSMD_USHORT  usPriority        | Don`t care
  CSMD_USHORT  usLength          | Data length: 4
  CSMD_USHORT  usIsList          | No list parameter: 0
  CSMD_ULONG   ulAttribute       | Don`t care: 0
  CSMD_USHORT  usSvchError       | Filled by the function in case of error: 0
  CSMD_USHORT  usCancelActTrans  | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usActStateAtomic  | Managed internally: CSMD_INIT_REQUEST
  CSMD_USHORT  usAct_Position    | Managed internally: 0
  CSMD_USHORT  usReplaceable     | Don't care: 0
  CSMD_S3_SVCH_SW  SVCH_Status   | Data status
  CSMD_USHORT  usAct_Len         | Don't care: 0
  CSMD_USHORT  usMax_Len         | Don't care: 0
  CSMD_USHORT  usRequestedLength | Don't care
  CSMD_USHORT  usSrv_Cont        | Software(1) or hardware(0) processing: 0
  CSMD_USHORT  usSetEnd          | Managed by the function: 1
  CSMD_USHORT  usCycle           | Internal variable: 0
  CSMD_USHORT  usNumWords        | Internal variable: 0
  CSMD_USHORT  usInUse           | Internal variable: 0
  CSMD_USHORT  usChannelOpen     | Internal variable; is set to 1 after SVC has been opened
  CSMD_USHORT  usRequestCanceled | Internal variable; is set after the request has been canceled: 0
  CSMD_USHORT  usLLR             | Internal variable; length of list parameter has been read: 0
  CSMD_USHORT  usIntReqPend      | Internal variable, set by macro function
  CSMD_USHORT  usEmptyList       | Don't care

<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however, not
   imperatively necessary.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]   prSvchMngmtData
              Pointer to macro management structure
\param [out]  CallBackFunc
              Pointer to callback function

\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n
              
\author       WK
\date         13.01.2005
  
*******************************************************************************/
CSMD_FUNC_RET CSMD_GetDataStatus( CSMD_INSTANCE          *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData,
                                  CSMD_VOID              (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_SERC3SVC *prSVC;
  CSMD_FUNC_RET  eRet;
  
  
  if (prSvchMngmtData->usCancelActTrans)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_REQUEST_CANCELED);
  }
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  
  
  switch (prSvchMngmtData->usActStateAtomic)
  {
  case CSMD_INIT_REQUEST:
    
  /* --------------------------------------------------------------------
      Step : 1 Open the service channel, as the Sercos is written
      to element 1 (the ID number).
    --------------------------------------------------------------------*/
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      return (CSMD_WRONG_SLAVE_INDEX);
    }
    /*reset flag for ListLengthRead*/
    prSvchMngmtData->usLLR = 0U;
    
    if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY))
      return (CSMD_MBUSY_NOT_SET);
    
    prSvchMngmtData->usAct_Position = 0U;
    
    eRet = CSMD_SVCHWrite( prCSMD_Instance, prSvchMngmtData );
    
    if (eRet != CSMD_NO_ERROR)
    {
      return (eRet);
    }
    
    prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
    break;
    
    
  case CSMD_REQUEST_IN_PROGRESS:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError( prSVC, prSvchMngmtData, FALSE );
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    /*lint -fallthrough */
    
    
  case CSMD_FINISHED_REQUEST:
    
    /* copy data status*/
    prSvchMngmtData->SVCH_Status.usStatus = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] );

    /* Call the Callback function*/
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }
    break;
    
    
  default:
    return (CSMD_SYSTEM_ERROR);
    
  }   /* end: switch (prSvchMngmtData->usActStateAtomic) */
  
  return (CSMD_NO_ERROR);
  
}   /* End: CSMD_GetDataStatus()*/



/**************************************************************************/ /**
\brief Reads the current and the maximum list length
       of the operation data (element 7) of an IDN that is already open.

\ingroup func_svc_atomic
<B>Description of the process of the atomic function state machine:</B>

- #CSMD_INIT_REQUEST:\n
  Initialization of the containers and start of the transmission

- #CSMD_REQUEST_IN_PROGRESS:\n
  Reading of the first 4 bytes of element 7 and
  copying of the Current/Max length into CSMD_SVCH_MNGMT_STRUCT

- #CSMD_FINISHED_REQUEST:\n
  SVCH enabled, end of the function
  <BR>

  Element of rSvchMngmtData      | Content
  :----------------------------- | :------
  CSMD_USHORT  usSlaveIdx        | Slave index
  CSMD_ULONG   ulIdent_Nbr       | Parameter IDN
  CSMD_USHORT  usElem            | Element number of list: 2, 4 or 7
  CSMD_USHORT *pusAct_Data       | Don`t care
  CSMD_USHORT  usPriority        | Don`t care
  CSMD_USHORT  usLength          | Length info: 4
  CSMD_USHORT  usIsList          | List length only readable for lists: 1
  CSMD_ULONG   ulAttribute       | Don`t care: 0
  CSMD_USHORT  usSvchError       | Filled by the function in case of error: 0
  CSMD_USHORT  usCancelActTrans  | Set to 1 for abortion of processing, else 0
  CSMD_USHORT  usActStateAtomic  | Managed internally: CSMD_INIT_REQUEST
  CSMD_USHORT  usAct_Position    | Managed internally: 0
  CSMD_USHORT  usReplaceable     | Don't care: 0
  CSMD_S3_SVCH_SW  SVCH_Status   | Filled by the function: 0
  CSMD_USHORT  usAct_Len         | Don't care (written by the function): 0
  CSMD_USHORT  usMax_Len         | Don't care (written by the function): 0
  CSMD_USHORT  usRequestedLength | Don't care
  CSMD_USHORT  usSrv_Cont        | Software(1) or hardware(0) processing: 0
  CSMD_USHORT  usSetEnd          | Managed internally for lists: 0
  CSMD_USHORT  usCycle           | Internal variable: 0
  CSMD_USHORT  usNumWords        | Internal variable: 0
  CSMD_USHORT  usInUse           | Internal variable: 0
  CSMD_USHORT  usChannelOpen     | Internal variable; is set to 1 after SVC has been opened
  CSMD_USHORT  usRequestCanceled | Internal variable; is set after the request has been canceled: 0
  CSMD_USHORT  usLLR             | Internal variable; indicates if length info of the list has been read yet / \n length info is transmitted to the list during the following read access (0)\n or not (1): don't change
  CSMD_USHORT  usIntReqPend      | Internal variable, set by macro function
  CSMD_USHORT  usEmptyList       | Don't care

<BR><B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however, not 
   imperatively necessary.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance.
\param [in]   prSvchMngmtData
              Pointer to macro management structure
\param [out]  CallBackFunc
              Pointer to callback function
              
\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       WK 
\date         13.01.2005

*******************************************************************************/
CSMD_FUNC_RET CSMD_GetListLength( CSMD_INSTANCE          *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData, 
                                  CSMD_VOID              (*CallBackFunc)(CSMD_VOID) )
{
  
  CSMD_USHORT    usSlaveIdx;
  CSMD_SERC3SVC *prSVC;
  CSMD_FUNC_RET  eRet;
  
  
  if (prSvchMngmtData->usCancelActTrans)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_REQUEST_CANCELED);
  }
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  
  
  switch (prSvchMngmtData->usActStateAtomic)
  {
  case CSMD_INIT_REQUEST:
    
    /* Check slave index once at initialization of the function */
    if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
    {
      return (CSMD_WRONG_SLAVE_INDEX);
    }
    if (!(CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] ) & CSMD_SVC_CTRL_M_BUSY))
      return (CSMD_MBUSY_NOT_SET);
    
    CSMD_SVCHRead( prCSMD_Instance, prSvchMngmtData );
    
    prSvchMngmtData->usActStateAtomic = CSMD_REQUEST_IN_PROGRESS;
    
    break;
    
    
  case CSMD_REQUEST_IN_PROGRESS:
    
    /* check error flags */
    eRet = CSMD_CheckSVCError(prSVC, prSvchMngmtData, TRUE); 
    if (eRet != CSMD_NO_ERROR)
      return (eRet);
    
    prSvchMngmtData->usLength  = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[0] ); /* current list length */
    prSvchMngmtData->usAct_Len = prSvchMngmtData->usLength;
    prSvchMngmtData->usMax_Len = CSMD_HAL_ReadShort( &prSVC->sRD_BUFF[1] ); /* maximum list length */
    
    /*set flag for ListLengthRead */
    prSvchMngmtData->usLLR = 1U;
    
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    /*lint -fallthrough */
    
    
  case CSMD_FINISHED_REQUEST:
    
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }
    
    break;
    
    
  default:
    return (CSMD_SYSTEM_ERROR);
    
  }   /* end: switch (prSvchMngmtData->usActStateAtomic) */
  
  return (CSMD_NO_ERROR);
  
} /* End: CSMD_GetListLength()*/



/**************************************************************************/ /**
\brief Resets the CSMD_SVCH_MNGMT_STRUCT of a slave with the
       specified index. 

\ingroup func_svcg
\b Description: \n
   This function should only be used if a service channel transmission cannot be
   canceled using the CancelActTrans flag in the CSMD_SVCH_MACRO_STRUCT.

<B>Call Environment:</B> \n
   This function can be called from an interrupt; this is, however,
   not imperatively necessary.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]   usSlaveIdx
              Slave index
\param [out]  CallBackFunc
              Pointer to callback function
              
\return       \ref CSMD_WRONG_SLAVE_INDEX \n
              \ref CSMD_NO_ERROR \n

\author       WK 
\date         13.01.2005

*******************************************************************************/
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_ResetSVCH( CSMD_INSTANCE *prCSMD_Instance,
                              CSMD_USHORT    usSlaveIdx,
                              CSMD_VOID      (*CallBackFunc)(CSMD_VOID) )
{
 
  /* Check slave index */
  if (usSlaveIdx > prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
  {
    return (CSMD_WRONG_SLAVE_INDEX);
  }
  else
  {
    (CSMD_VOID) CSMD_HAL_memset( &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                                 0,
                                 sizeof(CSMD_SVCH_MNGMT_STRUCT) );

    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;

    /* Call the Callback function*/
    if (CallBackFunc != NULL)
    {
      CallBackFunc();
    }

    return (CSMD_NO_ERROR);
  }
}   /* End: CSMD_ResetSVCH() */
/*lint -restore const! */

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
22 Jan 2015 WK
  - Convert simple HTML tables into "markdown" extra syntax to make it
    more readable in the source code.
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
