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
\file   CSMD_PRIV_SVC.c
\author WK
\date   01.09.2010
\brief  This File contains the private functions for the 
        asynchronous communication (service channel) to a Sercos slave.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"


#define SOURCE_CSMD
#include "CSMD_PRIV_SVC.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/*! \endcond */ /* PUBLIC */


/*---- Definition private Functions: -----------------------------------------*/

/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Evaluates the attribute of a parameter.

\ingroup func_svcc
\b Description: \n
   This function extracts the parameter type (list/fixed length) and
       length information out of the attribute.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure

\return       none

\author       RA
\date         02.05.2007

***************************************************************************** */
/*lint -efunc( 818, CSMD_EvaluateAttribute )
  Pointer parameter 'prCSMD_Instance' could be declared as pointing to const */
CSMD_VOID CSMD_EvaluateAttribute( CSMD_INSTANCE                *prCSMD_Instance,
                                  const CSMD_SVCH_MACRO_STRUCT *prSvchData )
{
  CSMD_USHORT   usSlaveIdx;
  
  
  usSlaveIdx = prSvchData->usSlaveIdx;
  
  /* Evalutate Bit 18: variable or fixed length  */
  if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute & CSMD_SERC_VAR_LEN)
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList = CSMD_ELEMENT_IS_LIST;

    /* Check if the request can be replaced */
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usPriority == 0U)
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usReplaceable = 1U;
    }
  }
  else
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList = CSMD_ELEMENT_NO_LIST;
  }
  
  /* set atomic structure back to given values */
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = prSvchData->usElem;
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 0U;
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = prSvchData->usLength;
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
  
  /* Evaluate attribute to get parameter length */
  /* Parameter has fixed length */
  if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_NO_LIST)
  {
    switch ((prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
      & CSMD_SERC_LEN)
    {
    case CSMD_SERC_WORD_LEN:
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 2U;
      break;
    case CSMD_SERC_LONG_LEN:
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
      break;
    case CSMD_SERC_DOUBLE_LEN:
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 8U;
      break;
    default:
      /*should not be here....*/
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
      break;
    }
  }
  /* Parameter has variable length but min/max values want to be accessed */
  else if ((prSvchData->usElem == CSMD_SVC_DBE5_MIN_VALUE) || (prSvchData->usElem == CSMD_SVC_DBE6_MAX_VALUE))
  {
    switch ((prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
      & CSMD_SERC_LEN)
    {
    case CSMD_SERC_VAR_BYTE_LEN:
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 1U;
      break;
    case CSMD_SERC_VAR_WORD_LEN:
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 2U;
      break;
    case CSMD_SERC_VAR_LONG_LEN:
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
      break;
    case CSMD_SERC_VAR_DOUBLE_LEN:
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 8U;
      break;
    default:
      /*should not be here....*/
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
      break;
    }
  }
  
  /* Attribute is accessed, length is always fix */
  if (prSvchData->usElem == CSMD_SVC_DBE3_ATTRIBUTE)
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
  }
  
}/* end of CSMD_EvaluateAttribute() */



/**************************************************************************/ /**
\brief Decides if the current list length must be read depending on
       accessed element.

\ingroup func_svcc
\b Description: \n

\verbatim
                +--------------+-----------------------------------------+
                | Element      |             usIsList                    |
                |              |     |           |     |     |     |     |
                |              |  0  |     1     |  2  |  3  |  4  |other|
                |              |     | usLength  |     |     |     |     |
                |              |     | =0  | >0  |     |     |  (2)|     |
                +--------------+-----+-----+-----+-----+-----+-----+-----+
                | 1: IDN       |     |     |     |     |     |     |     |
                | 2: Name      |  x  |  x  |  x  |  x  |  x  |  x  |  x  |
                | 3: Attribute |     |     |     |     |     |     |     |
                | 4: Unit      |  x  |  x  |  x  |  x  |  x  |  x  |  x  |
                | 5: Min       |     |     |     |     |     |     |     |
                | 6: Max       |     |     |     |     |     |     |     |
                | 7: Data      |     |  x  |     |  x  |  (1)|  x  |     |
                +--------------+--|--+--|--+--|--+--|--+--|--+--|--+-----+
                                  |     |     |     |     |     |
                        No list --+     |     |     |     |     |
        Is list, length unknown --------+     |     |     |     |
          Is list, length known --------------+     |     |     |
          Read only list length --------------------+     |     |
   Unknown whether it is a list --------------------------+     |
     Is list: read list segment --------------------------------+

        Note: (1)  List length for element 7 and usIsList = 3 needed ?
              (2)  usIsList = 4 available with defined CSMD_SVC_LIST_SEGMENT.
                   (Element 2 and 4 is read always complete)
\endverbatim

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure

\return       TRUE  - List length of element has to be read \n
              FALSE - List length of element doesn't have to be read

\author       RA
\date         02.05.2007

***************************************************************************** */
CSMD_BOOL CSMD_NeedListLength( const CSMD_INSTANCE          *prCSMD_Instance,
                               const CSMD_SVCH_MACRO_STRUCT *prSvchData )
{
  CSMD_USHORT   usSlaveIdx;
  
  usSlaveIdx = prSvchData->usSlaveIdx;
  
  /* we need to read the list length (current/max length) if:                    */
  /* - Operation data (Element 7) is a list parameter and the length is unknown  */
  /* - Name / Unit (Element 2 / 4) is accessed (always var. length)              */
  /* - Query for list length of operation data (Element 7)                       */
  /* - not Min/Max/Attr/IDN or invalid Element is accessed (always fixed length) */
  /* - read a list segnment                                                      */
#ifdef CSMD_SVC_LIST_SEGMENT
if (    (   (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_IS_LIST)
             && (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength == 0U))
         || (prSvchData->usElem == CSMD_SVC_DBE2_NAME)
         || (prSvchData->usElem == CSMD_SVC_DBE4_UNIT)
         || (prSvchData->usIsList == CSMD_ELEMENT_RD_LIST_LENGTH)
         || (prSvchData->usIsList == CSMD_ELEMENT_RD_LIST_SEGMENT)
        )
    && !(   (prSvchData->usElem == CSMD_SVC_DBE5_MIN_VALUE)
         || (prSvchData->usElem == CSMD_SVC_DBE6_MAX_VALUE)
         || (prSvchData->usElem == CSMD_SVC_DBE3_ATTRIBUTE)
         || (prSvchData->usElem == CSMD_SVC_DBE1_IDN)
         || (prSvchData->usElem  > CSMD_SVC_DBE7_OPERATION_DATA)
        )
   )

#else

if (    (   (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_IS_LIST)
             && (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength == 0U))
         || (prSvchData->usElem == CSMD_SVC_DBE2_NAME)
         || (prSvchData->usElem == CSMD_SVC_DBE4_UNIT)
         || (prSvchData->usIsList == CSMD_ELEMENT_RD_LIST_LENGTH)
        )
    && !(   (prSvchData->usElem == CSMD_SVC_DBE5_MIN_VALUE)
         || (prSvchData->usElem == CSMD_SVC_DBE6_MAX_VALUE)
         || (prSvchData->usElem == CSMD_SVC_DBE3_ATTRIBUTE)
         || (prSvchData->usElem == CSMD_SVC_DBE1_IDN)
         || (prSvchData->usElem  > CSMD_SVC_DBE7_OPERATION_DATA)
        )
   )
#endif
  {
    return (TRUE);
  }
  else
    return (FALSE);
  
}/* end of CSMD_NeedListLength() */



/**************************************************************************/ /**
\brief Decides if attribute must be read depending on accessed element.

\ingroup func_svcc
\b Description: \n

\verbatim
                +--------------+-----------------------------------+
                | Element      |             usIsList              |
                |              |           |     |     |     |     |
                |              |     0     |  1  |  2  |  3  |other|
                |              | usLength  |     |     |     |     |
                |              | =0  | >0  |     |     |     |     |
                +--------------+-----+-----+-----+-----+-----+-----+
                | 1: IDN       |  x  |     |     |     |  x  |     |
                | 2: Name      |     |     |     |     |     |     |
                | 3: Attribute |  x  |  x  |  x  |  x  |  x  |  x  |
                | 4: Unit      |     |     |     |     |     |     |
                | 5: Min       |  x  |     |  x  |     |  x  |     |
                | 6: Max       |  x  |     |  x  |     |  x  |     |
                | 7: Data      | (x) |     |     |     |  x  |     |
                +--------------+--|--+--|--+--|--+--|--+--|--+-----+
                                  |     |     |     |     |
        No list, length unknown --+     |     |     |     |
          Ns list, length known --------+     |     |     |
                        Is list --------------+     |     |
          Read only list length --------------------+     |
   Unknown whether it is a list --------------------------+

\endverbatim

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure

\return       TRUE  - Attribute has to be read \n
              FALSE - Attribute doesn't have to be read

\author       RA
\date         02.05.2007
    
***************************************************************************** */
CSMD_BOOL CSMD_NeedAttribut( const CSMD_INSTANCE          *prCSMD_Instance,
                             const CSMD_SVCH_MACRO_STRUCT *prSvchData )
{
#if defined CSMD_BIG_ENDIAN || defined CSMD_TEST_BE
  
  CSMD_USHORT   usSlaveIdx;
  
  usSlaveIdx = prSvchData->usSlaveIdx;
  
  if (   /* List / Element = Maximum value or minimum value or operation data */
         (    (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_IS_LIST)
          && (   (prSvchData->usElem == CSMD_SVC_DBE5_MIN_VALUE)
              || (prSvchData->usElem == CSMD_SVC_DBE6_MAX_VALUE)
              || (prSvchData->usElem == CSMD_SVC_DBE7_OPERATION_DATA) ))
      ||
         /* Unknown if list / Element: not name or not unit */
         (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_UNKNOWN_LENGTH)
          && !(   (prSvchData->usElem == CSMD_SVC_DBE2_NAME)
               || (prSvchData->usElem == CSMD_SVC_DBE4_UNIT)))
      ||
         /* No list / zero length / Element: not name or not unit */ 
         (   (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_NO_LIST)
              && (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength == 0U))
          && !(   (prSvchData->usElem == CSMD_SVC_DBE2_NAME)
               || (prSvchData->usElem == CSMD_SVC_DBE4_UNIT)))
      || 
         /* Element: Attribute */
         (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem == CSMD_SVC_DBE3_ATTRIBUTE)
     )
  {
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
  
#else
  
  CSMD_USHORT   usSlaveIdx;
  
  usSlaveIdx = prSvchData->usSlaveIdx;
  
  if (   /* List / Element = Maximum value or minimum value */
         (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_IS_LIST)
          && (   (prSvchData->usElem == CSMD_SVC_DBE5_MIN_VALUE)
              || (prSvchData->usElem == CSMD_SVC_DBE6_MAX_VALUE)))
      ||
         /* Unknown if list / Element: not name or not unit */
         (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_UNKNOWN_LENGTH)
          && !(   (prSvchData->usElem == CSMD_SVC_DBE2_NAME)
               || (prSvchData->usElem == CSMD_SVC_DBE4_UNIT)))
      ||
         /* No list / zero length / Element: not name or not unit */ 
         (   (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_NO_LIST)
              && (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength == 0U))
          && !(   (prSvchData->usElem == CSMD_SVC_DBE2_NAME)
               || (prSvchData->usElem == CSMD_SVC_DBE4_UNIT)))
      || 
         /* Element: Attribute */
         (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem == CSMD_SVC_DBE3_ATTRIBUTE)
     )
  {
    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
  
#endif
}/* end of CSMD_NeedAttribut() */



/**************************************************************************/ /**
\brief Checks if a request is canceled.

\ingroup func_svcc
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure

\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_NO_ERROR \n

\author       RA
\date         02.05.2007

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_CheckRequestCancel( CSMD_INSTANCE          *prCSMD_Instance,
                                       CSMD_SVCH_MACRO_STRUCT *prSvchData )
{
  CSMD_USHORT   usSlaveIdx;
  
  usSlaveIdx = prSvchData->usSlaveIdx;
  
  /*check if request has been cancelled by high priority request*/
  if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usRequestCanceled)
  {
    if ( !(prSvchData->usOtherRequestCanceled))
    {
      /*this request has been cancelled by a high priority request*/
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usRequestCanceled = 0U;
      return (CSMD_REQUEST_CANCELED);
    }
  }
  else
  {
    prSvchData->usOtherRequestCanceled = 0U;
  }
  
  if (prSvchData->usCancelActTrans)
  {
    if ((CSMD_SHORT)usSlaveIdx >= CSMD_MAX_HW_CONTAINER)
    {
      /* Prevents a new request before termination of the current request. */
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = 2U;
    }
    else
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
    }
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen = 0U;
    /* Adjust SVC macro structure for next request */
    prSvchData->usCancelActTrans = 0L;
    prSvchData->usState = CSMD_START_REQUEST;

    return (CSMD_REQUEST_CANCELED);
  }
  
  return (CSMD_NO_ERROR);
  
}/* end of CSMD_CheckRequestCancel() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Checks if the service channel to the given slave is free to use.

\ingroup func_svcc
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure

\return       \ref CSMD_INTERNAL_REQUEST_PENDING \n
              \ref CSMD_SVCH_INUSE \n
              \ref CSMD_NO_ERROR \n

\author       RA
\date         02.05.2007
    
***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_CheckSVCHInUse( CSMD_INSTANCE          *prCSMD_Instance,
                                   CSMD_SVCH_MACRO_STRUCT *prSvchData )
{
  CSMD_USHORT   usSlaveIdx;
  
  
  usSlaveIdx = prSvchData->usSlaveIdx;
  
  if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse)
  {
    if (   ((prSvchData->usPriority == 1U)
        && (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usPriority == 0U))
        && (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usReplaceable == 1U))
    {
      /*new request is a high priority request -> cancel current request*/
      prSvchData->usOtherRequestCanceled = 1U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usRequestCanceled = 1U;
    }
    else
    {
      if (prSvchData->usInternalReq)
      {
        /* internal request already pending */
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIntReqPend = 1U;
        return (CSMD_INTERNAL_REQUEST_PENDING);
      }
      else
      {
        /*SVCH in use, current request not replaceable */
        return (CSMD_SVCH_INUSE);
      }
    }
  }
  
  if ((prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIntReqPend) && !(prSvchData->usInternalReq))
  {
    /* this SVCH transmission is reserved for an internal request */
    return (CSMD_INTERNAL_REQUEST_PENDING);
  }
  
  return (CSMD_NO_ERROR);
  
}/* end of CSMD_CheckSVCHInUse() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Initializes all the macro-, atomic- and internal structures for
       SVC access.

\ingroup func_svcc
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure
\param [in]   boCommandRequest
              Flag signaling if the requested IDN is a command

\return       none

\author       RA
\date         02.05.2007

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_VOID CSMD_InitSVCHRequest( CSMD_INSTANCE          *prCSMD_Instance,
                                CSMD_SVCH_MACRO_STRUCT *prSvchData,
                                CSMD_BOOL               boCommandRequest )
{
  CSMD_USHORT   usSlaveIdx;
  
  
  usSlaveIdx = prSvchData->usSlaveIdx;
  
  /*initialize CSMD_SVCH_MNGMT_STRUCT */
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_IN_USE;
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSlaveIdx = prSvchData->usSlaveIdx;
  
  /*if new request has the same Ident-Nbr, SVCH may be already open. 
  Open_IDN will be skipped if SVCH is already open*/
  if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulIdent_Nbr != prSvchData->ulIdent_Nbr)
  {   
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulIdent_Nbr = prSvchData->ulIdent_Nbr;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen = 0;
  }
  
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usPriority = prSvchData->usPriority;
  if (boCommandRequest)
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 2U;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList = CSMD_ELEMENT_NO_LIST;
  }
  else
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = prSvchData->usLength;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList = prSvchData->usIsList;
  }
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usCancelActTrans = prSvchData->usCancelActTrans;
  
  if (prSvchData->ulAttribute) 
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute = prSvchData->ulAttribute;
  }
  else
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute = 0;
  }
  
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Position = 0;
  
  if (   (prSvchData->usIsList == CSMD_ELEMENT_IS_LIST)
      && prSvchData->usLength)
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Len = (CSMD_USHORT)prSvchData->usLength;
  }
  else
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Len = 0;
  }
  
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usMax_Len   = 0;
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usEmptyList = 0;
  
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
  
  
  /*if this request is a low priority request and a list 
  it may be replaced by a high priority request*/
  if (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usPriority == 0U)
      && (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList   == CSMD_ELEMENT_IS_LIST) )
    
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usReplaceable = 1U;
  }
  else
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usReplaceable = 0U;
  }
  
  if ((CSMD_SHORT)usSlaveIdx < CSMD_MAX_HW_CONTAINER)
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSrv_Cont = CSMD_HW_SRV_CONTAINER;
  }
  else
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSrv_Cont = CSMD_SW_SRV_CONTAINER;
  }
  
  /* continue with next step immediately */
  /* initialisation for opening the SVCH */
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data =
    (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulIdent_Nbr;
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 1U;
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
  prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = CSMD_SVC_DBE1_IDN;
  
}/* end of CSMD_InitSVCHRequest() */
/*lint -restore const! */



/**************************************************************************/ /**
\brief Checks for SVC errors.

\ingroup func_svcc
\b Description: \n
   This function checks if a service channel error occurred. If so, it returns
   the corresponding error code.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prSVC
              Pointer to SVC container of the selected slave
\param [in]   prSvchMngmtData
              Pointer to macro management structure
\param [in]   boReadAccess
              Marker for read access
>todo description of parameter shall be added

\return       \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_NO_ERROR \n

\author       RA
\date         02.05.2007
    
***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckSVCError( const CSMD_SERC3SVC    *prSVC,
                                  CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData,
                                  CSMD_BOOL               boReadAccess )
{
  CSMD_USHORT usTempRam;
  CSMD_USHORT usWord0;
  CSMD_USHORT usWord4;
  
  
  usWord0 = CSMD_HAL_ReadShort(&prSVC->rCONTROL.usWord[0]);
  
  if (!(usWord0 & CSMD_SVC_CTRL_M_BUSY))
  {
    return (CSMD_MBUSY_NOT_SET);
  }
  
  if (usWord0 & CSMD_SVC_CTRL_INT_ERR)
  {
    if (boReadAccess)
    {
      /* Get svc error message from last updated position in the      */
      /* svc read buffer.                                             */
      /* The pointer RDDATPT in svc control word 3 defines the offset */
      /* to the RD/WR Buffer basepointer sWR_BUFF!                    */
      /* Subtract 2 words from actual position (RDDATPT) to get the   */
      /* position of error message.                                   */
      usTempRam = CSMD_HAL_ReadShort(&prSVC->rCONTROL.usWord[3]);
      
      prSvchMngmtData->SVCH_Status.usStatus = 
        CSMD_HAL_ReadShort(&prSVC->sWR_BUFF[(usTempRam & 0xFF) - 2]);
    }
    else
    {
      prSvchMngmtData->SVCH_Status.usStatus = 
        CSMD_HAL_ReadShort(&prSVC->sRD_BUFF[0]);
    }
    
    prSvchMngmtData->usSvchError = prSvchMngmtData->SVCH_Status.usStatus;
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_SVC_ERROR_MESSAGE);
  }
  
  
  usWord4 = CSMD_HAL_ReadShort(&prSVC->rCONTROL.usWord[4]);
  
  if (usWord4 & CSMD_SVC_CTRL_INT_SC_ERR)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_PROTOCOL_ERROR);
  }
  
  if (usWord4 & CSMD_SVC_CTRL_INT_HS_TIMEOUT)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_HANDSHAKE_TIMEOUT);
  }
  
  if (usWord4 & CSMD_SVC_CTRL_INT_BUSY_TIMEOUT)
  {
    prSvchMngmtData->usActStateAtomic = CSMD_FINISHED_REQUEST;
    return (CSMD_BUSY_TIMEOUT);
  }
  
  prSvchMngmtData->usSvchError = (CSMD_USHORT) CSMD_SVC_NO_ERR;
  return (CSMD_NO_ERROR);
  
} /* end of CSMD_CheckSVCError() */


/**************************************************************************/ /**
\brief Opens the SVC channel for macro function read/write operation.

\ingroup func_svcc
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure
\param [in]   usSlaveIdx
              Slave Index
\param [out]  pboBreak
              Flag signaling if calling function shall stay in the current step 
              of the state machine or continue with processing the next step

\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_REQUEST_TO_HP_SLAVE \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n
              
\author       RA
\date         02.05.2007
    
***************************************************************************** */
CSMD_FUNC_RET CSMD_OpenSVCH( CSMD_INSTANCE          *prCSMD_Instance,
                             CSMD_SVCH_MACRO_STRUCT *prSvchData,
                             CSMD_USHORT             usSlaveIdx,
                             CSMD_BOOL              *pboBreak )

{
  CSMD_FUNC_RET  eRet;
  
  
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
      prSvchData->usState = CSMD_DATA_NOT_VALID;
      return (eRet);
    }
  }
  else
  {
#ifdef CSMD_HOTPLUG
    if (prCSMD_Instance->rSlaveList.aeSlaveActive[usSlaveIdx] == CSMD_SLAVE_INACTIVE)
    {
      /* Service channel of a Hot-Plug slave is not active */
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      prSvchData->usState = CSMD_DATA_NOT_VALID;
      return (CSMD_REQUEST_TO_HP_SLAVE);
    }
#endif
    prSvchData->usState = CSMD_CHANNEL_OPEN;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
  }
  
  if (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
      || (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen == 1U))
    
  {
    /*continue with next step, initialization*/
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data = prSvchData->pusAct_Data;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = prSvchData->usLength;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usChannelOpen = 1U;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = prSvchData->usElem;
    
    prSvchData->usState = CSMD_CHANNEL_OPEN;
    *pboBreak = FALSE;
  }
  else
  {
    *pboBreak = TRUE;
  }    
  
  return (CSMD_NO_ERROR);
  
}/* end of CSMD_OpenSVCH() */



/**************************************************************************/ /**
\brief Prepares the final step for macro function read access.

\ingroup func_svcc
\b Description: \n
   This function prepares the final step for a macro function read access.
   It reads the attribute, if needed, and checks if the parameter is a list.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure
\param [in]   usSlaveIdx
              Slave Index
\param [out]  pboBreak
              Flag signaling if calling function shall stay in the current step 
              of the state machine or continue with processing the next step

\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       RA
\date         02.05.2007

***************************************************************************** */
CSMD_FUNC_RET CSMD_PrepFinalStepRead( CSMD_INSTANCE          *prCSMD_Instance,
                                      CSMD_SVCH_MACRO_STRUCT *prSvchData,
                                      CSMD_USHORT             usSlaveIdx,
                                      CSMD_BOOL              *pboBreak )
{
  CSMD_FUNC_RET eRet;
  
  
#if defined CSMD_BIG_ENDIAN || defined CSMD_TEST_BE
  /* Code shift as separate step into CSMD_ReadSVCH() ! */
#else
  /* get attribut */
  if ( CSMD_NeedAttribut(prCSMD_Instance, prSvchData) )
  {
    
    if (!prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = CSMD_SVC_DBE3_ATTRIBUTE;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 1U;
      
      eRet = CSMD_GetAttribute( prCSMD_Instance,
                                &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                                (CSMD_VOID(*)(CSMD_VOID))NULL );
      
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 0U;
      if (eRet != CSMD_NO_ERROR) 
      {
        if (eRet == CSMD_SVC_ERROR_MESSAGE) 
          prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
        prSvchData->usState = CSMD_DATA_NOT_VALID;
        return (eRet);
      }
      if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic != CSMD_FINISHED_REQUEST)
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
      }
    }
    
    
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
    {
      CSMD_EvaluateAttribute( prCSMD_Instance,
                              prSvchData );
    }
  }
#endif
  
  /* get list length */
  if (CSMD_NeedListLength( prCSMD_Instance, prSvchData ))
    
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = prSvchData->usElem;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_INIT_REQUEST)
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 0U;
    }
    
    
    eRet = CSMD_GetListLength( prCSMD_Instance, 
                               &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                               (CSMD_VOID(*)(CSMD_VOID))NULL );
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE ) 
      {
        /* if the SVCH answers with error code 7003 (transmission to long)
           retry read access on list with SetEnd. Maybe the list is empty */
        if (   (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus == (CSMD_USHORT) CSMD_SVC_DATA_TOO_LONG)
                || (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus == (CSMD_USHORT) CSMD_SVC_NAME_TOO_LONG)
                || (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus == (CSMD_USHORT) CSMD_SVC_UNIT_TOO_LONG) )
            && !(prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd))
        {
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 1U;
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus = 0U;
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSvchError = 0U;
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
          /* eRet = CSMD_NO_ERROR; */
          
          eRet = CSMD_GetListLength( prCSMD_Instance, 
                                     &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                                     (CSMD_VOID(*)(CSMD_VOID))NULL );
          
          if (eRet != CSMD_NO_ERROR) 
          {
            if (eRet == CSMD_SVC_ERROR_MESSAGE) 
              prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
            prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
            prSvchData->usState = CSMD_DATA_NOT_VALID;
            return (eRet);
          }
        }
        else 
        {
          /*if SetEnd is set to 1 and there is still an Error, return error code*/
          prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
          prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
          prSvchData->usState = CSMD_DATA_NOT_VALID;
          return (eRet);
        }
      }
    }
    
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic != CSMD_FINISHED_REQUEST)
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
    }
    else
    {   
      if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Len == 0U)
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usEmptyList = 1U;
      }
      else
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usEmptyList = 0U;
      }
      
    }
  }
  
  
  /*if length info is known, continue with the next step*/
  if (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength)
      || (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usEmptyList))
  {
    /*set SetEnd according to the parameter length and the buffsize of the SVContainer*/
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength > CSMD_SERC_SC_WRBUF_LENGTH)
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 0U;
    }
    else
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 1U;
    }
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data = prSvchData->pusAct_Data;
    
    
    if (!prSvchData->ulAttribute)
    {
      prSvchData->ulAttribute = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute;
    }
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = prSvchData->usElem;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data = prSvchData->pusAct_Data;
    
    prSvchData->usState = CSMD_ATTRIBUTE_VALID;
    if (!prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList = prSvchData->usIsList;
    }
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
    
    if (prSvchData->usIsList == CSMD_ELEMENT_RD_LIST_LENGTH)
    {
      prSvchData->pusAct_Data[0] = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Len;
      prSvchData->pusAct_Data[1] = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usMax_Len;
      
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = CSMD_SVC_DBE0_CLOSE;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data =
        (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulIdent_Nbr;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
    }
    
#ifdef CSMD_SVC_LIST_SEGMENT
    if (prSvchData->usIsList == CSMD_ELEMENT_RD_LIST_SEGMENT)
    {
      /* Take the length of the list segment length into the atomic structure */
      if (prSvchData->usLength > 4U)
      {
        /* Length info was already taken, adjust length */
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usRequestedLength =
          (CSMD_USHORT)(prSvchData->usLength - 4U);
      }
      else
      {
        /* If length lower than 4, get only length information of the list. */
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usRequestedLength = 0U;
      }
    }
#endif
    
    *pboBreak = FALSE; 
  }
  else
  {
    *pboBreak = TRUE;
  }
  
  return (CSMD_NO_ERROR);
  
}/* end of CSMD_PrepFinalStepRead() */



#if defined CSMD_BIG_ENDIAN || defined CSMD_TEST_BE
/**************************************************************************/ /**
\brief Reads the Attribute of a parameter, if needed.

\ingroup func_svcc
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure
\param [in]   usSlaveIdx
              Slave Index
\param [out]  pboBreak
              Flag signaling if calling function shall stay in the current step 
              of the state machine or continue with processing the next step

\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       WK
\date         22.07.2009

***************************************************************************** */
CSMD_FUNC_RET CSMD_ReadAttribute( CSMD_INSTANCE          *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT *prSvchData,
                                  CSMD_USHORT             usSlaveIdx,
                                  CSMD_BOOL              *pboBreak )
{
  
  CSMD_FUNC_RET eRet;
  
  
  /* get attribute */
  if ( CSMD_NeedAttribut(prCSMD_Instance, prSvchData) )
  {
    
    if (!prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = CSMD_SVC_DBE3_ATTRIBUTE;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 1U;
      
      eRet = CSMD_GetAttribute( prCSMD_Instance,
                                &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                                (CSMD_VOID(*)(CSMD_VOID))NULL );
      
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 0U;
      if (eRet != CSMD_NO_ERROR) 
      {
        if (eRet == CSMD_SVC_ERROR_MESSAGE) 
          prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
        prSvchData->usState = CSMD_DATA_NOT_VALID;
        return (eRet);
      }
      if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic != CSMD_FINISHED_REQUEST)
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
      }
    }
    
    
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
    {
      CSMD_EvaluateAttribute( prCSMD_Instance, prSvchData );
      prSvchData->usState = CSMD_GET_ATTRIBUTE;
      *pboBreak = FALSE;
    }
    else
    {
      *pboBreak = TRUE;
    }
  }
  else
  {
    prSvchData->usState = CSMD_GET_ATTRIBUTE;
    *pboBreak = FALSE;
  }
  
  return (CSMD_NO_ERROR);
  
}/* end of CSMD_ReadAttribute() */
#endif



/**************************************************************************/ /**
\brief Prepares the final step for the macro function write access.

\ingroup func_svcc
\b Description: \n
   This function prepares the final step for a macro function write access.
   It reads the attribute, if needed, and checks if the parameter is a list.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure
\param [in]   usSlaveIdx
              Slave Index
\param [out]  pboBreak
              Flag signaling if calling function shall stay in the current step 
              of the state machine or continue with processing the next step

\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       RA
\date         02.05.2007

***************************************************************************** */
CSMD_FUNC_RET CSMD_PrepFinalStepWrite( CSMD_INSTANCE          *prCSMD_Instance,
                                       CSMD_SVCH_MACRO_STRUCT *prSvchData,
                                       CSMD_USHORT             usSlaveIdx,
                                       CSMD_BOOL              *pboBreak )
{
  CSMD_FUNC_RET eRet;
  
  
  if (   (  !(prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
          && (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength == 0U))
      || (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_UNKNOWN_LENGTH) )
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = CSMD_SVC_DBE3_ATTRIBUTE;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 1U;
    
    eRet = CSMD_GetAttribute( prCSMD_Instance, 
                              &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                              (CSMD_VOID(*)(CSMD_VOID))NULL );
    
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = prSvchData->usElem;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = prSvchData->usLength;
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) 
        prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      prSvchData->usState = CSMD_DATA_NOT_VALID;
      return (eRet);
    }
  }
  else if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength == 0U)
  {
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
    {
      switch ((prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute) & CSMD_SERC_LEN)
      {
      case CSMD_SERC_WORD_LEN:
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 2U;
        break;
      case CSMD_SERC_LONG_LEN:
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
        break;
      case CSMD_SERC_DOUBLE_LEN:
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 8U;
        break;
      default:
        /*should not be here....*/
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
        break;
      }
      
      /* check if Parameter is a list or fixed length and set isList to a proper value */
      if ((prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute) & CSMD_SERC_VAR_LEN)
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList  = CSMD_ELEMENT_IS_LIST;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength  = (CSMD_USHORT)((CSMD_USHORT)*prSvchData->pusAct_Data + 4U);
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Len = (CSMD_USHORT)((CSMD_USHORT)*prSvchData->pusAct_Data + 4U);
      }
      else
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList = CSMD_ELEMENT_NO_LIST;
      }
    }
    else
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      prSvchData->usState = CSMD_DATA_NOT_VALID;
      return (CSMD_SYSTEM_ERROR);
    }
    
    prSvchData->usState = CSMD_ATTRIBUTE_VALID;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
    
  }
  else if (    (prSvchData->usLength)
           && !(prSvchData->usIsList == CSMD_ELEMENT_UNKNOWN_LENGTH) )
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = prSvchData->usElem;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = prSvchData->usLength;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Len = prSvchData->usLength;
    
    prSvchData->usState = CSMD_ATTRIBUTE_VALID;
  }
  else
  {
    *pboBreak = TRUE;
  }
  
  
  /* all done, continue with next step */
  if (   (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
      || (    (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength > 0U)
          && !(prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_UNKNOWN_LENGTH)))
  {
    /* if no length is provided, get length from attribute */
    if (   (    (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
            && !(prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength))
        || (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList == CSMD_ELEMENT_UNKNOWN_LENGTH))
    {
      switch ((prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute) & CSMD_SERC_LEN)
      {
      case CSMD_SERC_WORD_LEN:
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 2U;
        break;
      case CSMD_SERC_LONG_LEN:
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
        break;
      case CSMD_SERC_DOUBLE_LEN:
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 8U;
        break;
      default:
        /* should not be here... */
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 0U;
        break;
      }
      
      /* check if Parameter is a list or fixed length and set isList to a proper value */
      if ((prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute) & CSMD_SERC_VAR_LEN)
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList  = CSMD_ELEMENT_IS_LIST;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength  = (CSMD_USHORT)(((CSMD_USHORT)*prSvchData->pusAct_Data) + 4U);
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Len = (CSMD_USHORT)(((CSMD_USHORT)*prSvchData->pusAct_Data) + 4U);
      }
      else
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList = CSMD_ELEMENT_NO_LIST;
      }
    }
    
    prSvchData->usState = CSMD_ATTRIBUTE_VALID;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
    *pboBreak = FALSE;
  }
  else if (    (prSvchData->usLength)
           && !(prSvchData->usIsList == CSMD_ELEMENT_UNKNOWN_LENGTH) )
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem = prSvchData->usElem;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = prSvchData->usLength;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Len = prSvchData->usLength;
    
    prSvchData->usState = CSMD_ATTRIBUTE_VALID;
    *pboBreak = FALSE;
  }
  else
  {
    *pboBreak = TRUE;
  }
  
  return (CSMD_NO_ERROR);
  
}/* end of CSMD_PrepFinalStepWrite() */



/**************************************************************************/ /**
\brief The final step for the macro function read access.

\ingroup func_svcc
\b Description: \n
   This function performs the final step for reading an IDN via the service channel
   calling all the atomic SVC functions.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure
\param [in]   usSlaveIdx
              Slave Index
\param [out]  pboBreak
              Flag signaling if calling function shall stay in the current step 
              of the state machine or continue with processing the next step

\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       RA
\date         02.05.2007

***************************************************************************** */
CSMD_FUNC_RET CSMD_FinalStepRead( CSMD_INSTANCE          *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT *prSvchData,
                                  CSMD_USHORT             usSlaveIdx,
                                  CSMD_BOOL              *pboBreak )
{
  CSMD_FUNC_RET eRet;
  
  
  switch (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem)
  {
  case 0:
    /* Close the service channel */
    eRet = CSMD_PutData( prCSMD_Instance,
                         &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                         (CSMD_VOID(*)(CSMD_VOID))NULL );
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) 
        prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      return (eRet);
    }
    break;
    
    
  case 2:
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usEmptyList == 0U)
    {
#ifdef CSMD_SVC_LIST_SEGMENT
      if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList != CSMD_ELEMENT_RD_LIST_SEGMENT)
#endif
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList = CSMD_ELEMENT_IS_LIST;
      }
      eRet = 
        CSMD_GetName( prCSMD_Instance, 
                      &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                      (CSMD_VOID(*)(CSMD_VOID))NULL );
      
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList = prSvchData->usIsList;
      if (eRet != CSMD_NO_ERROR) 
      {
        if (eRet == CSMD_SVC_ERROR_MESSAGE) prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
        return (eRet);
      }   
    }
    else
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data[0] = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Len;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data[1] = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usMax_Len;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_FINISHED_REQUEST;
    }
    break;
    
    
  case 3:
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute)
    {
      /* attribut fetched in a previous step, just copy it in the buffer */
#ifdef CSMD_BIG_ENDIAN
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data[0] = (CSMD_USHORT)(prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute >> 16);
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data[1] = (CSMD_USHORT)prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute;
#else
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data[0] = (CSMD_USHORT)prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data[1] = (CSMD_USHORT)(prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulAttribute >> 16);
#endif
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_FINISHED_REQUEST;
    }
    else
    {
      eRet = CSMD_GetAttribute( prCSMD_Instance,
                                &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                                (CSMD_VOID(*)(CSMD_VOID))NULL );
      
      if (eRet != CSMD_NO_ERROR) 
      {
        if (eRet == CSMD_SVC_ERROR_MESSAGE) prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
        return (eRet);
      }
    }
    break;
    
    
  case 4:
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usEmptyList == 0U)
    {
#ifdef CSMD_SVC_LIST_SEGMENT
      if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList != CSMD_ELEMENT_RD_LIST_SEGMENT)
#endif
      {
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList = CSMD_ELEMENT_IS_LIST;
      }
      eRet = CSMD_GetUnit( prCSMD_Instance,
                           &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                           (CSMD_VOID(*)(CSMD_VOID))NULL );
      
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIsList = prSvchData->usIsList;
      if (eRet != CSMD_NO_ERROR) 
      {
        if (eRet == CSMD_SVC_ERROR_MESSAGE) prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
        return (eRet);
      }
    }
    else
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data[0] = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Len;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data[1] = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usMax_Len;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_FINISHED_REQUEST;
    }
    break;
    
    
  case 5:
    eRet = CSMD_GetMin( prCSMD_Instance,
                        &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                        (CSMD_VOID(*)(CSMD_VOID))NULL );
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      return (eRet);
    }
    break;
    
    
  case 6:
    eRet = CSMD_GetMax( prCSMD_Instance,
                        &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                        (CSMD_VOID(*)(CSMD_VOID))NULL );
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      return (eRet);
    }
    break;
    
    
  case 7:
    
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usEmptyList == 0U)
    {
      eRet = CSMD_GetData( prCSMD_Instance, 
                           &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                           (CSMD_VOID(*)(CSMD_VOID))NULL );
      
      if (eRet != CSMD_NO_ERROR) 
      {
        if (eRet == CSMD_SVC_ERROR_MESSAGE) prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
        prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
        return (eRet);
      }
    }
    else
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data[0] = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usAct_Len;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data[1] = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usMax_Len;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_FINISHED_REQUEST;
    }
    break;
    
    
  default:
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
    return (CSMD_SYSTEM_ERROR);
  
  }    /* End: switch (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem) */
  
  
  if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
  {
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usReplaceable = 0U;
    
    prSvchData->usState = CSMD_DATA_VALID;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
    if (prSvchData->usInternalReq)
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIntReqPend = 0U;
    }
    
    /* reset identnbr in atomic struct to ensure initialization in CSMD_START_REQUEST */
    /* svc is closed by macro-function if list length is read by setting usIsList = 2 */
#ifdef CSMD_SVC_LIST_SEGMENT
    if (   (prSvchData->usIsList == CSMD_ELEMENT_RD_LIST_LENGTH)
        || (prSvchData->usIsList == CSMD_ELEMENT_RD_LIST_SEGMENT) )
#else
    if (prSvchData->usIsList == CSMD_ELEMENT_RD_LIST_LENGTH)
#endif
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulIdent_Nbr = 0U;
    }
    
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
    
    *pboBreak = FALSE;
  }
  else
  {
    *pboBreak = TRUE;
  }
  
  return (CSMD_NO_ERROR);
  
}/* end of CSMD_FinalStepRead() */



/**************************************************************************/ /**
\brief The final step for the macro function write access.

\ingroup func_svcc
\b Description: \n
   This function performs the final step for writing an IDN reading the data status
   of the IDN before calling the adequate atomic SVC function. 

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchData
              Pointer to macro management structure
\param [in]   usSlaveIdx
              Slave Index
\param [out]  pboBreak
              Flag signaling if calling function shall stay in the current step 
              of the state machine or continue with processing the next step

\return       \ref CSMD_REQUEST_CANCELED \n
              \ref CSMD_SVC_ERROR_MESSAGE \n
              \ref CSMD_MBUSY_NOT_SET \n
              \ref CSMD_PROTOCOL_ERROR \n
              \ref CSMD_HANDSHAKE_TIMEOUT \n
              \ref CSMD_BUSY_TIMEOUT \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       RA
\date         02.05.2007

***************************************************************************** */
CSMD_FUNC_RET CSMD_FinalStepWrite( CSMD_INSTANCE          *prCSMD_Instance,
                                   CSMD_SVCH_MACRO_STRUCT *prSvchData,
                                   CSMD_USHORT             usSlaveIdx,
                                   CSMD_BOOL              *pboBreak )
{
  CSMD_FUNC_RET eRet;
  
  
  switch (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem)
  {
  case 1:
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usLength = 4U;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usSetEnd = 1U;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].pusAct_Data =
      (CSMD_USHORT *)(CSMD_VOID *)&prCSMD_Instance->parSvchMngmtData[usSlaveIdx].ulIdent_Nbr;
    
    eRet = CSMD_GetDataStatus( prCSMD_Instance, 
                               &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                               (CSMD_VOID(*)(CSMD_VOID))NULL );
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) 
        prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      return (eRet);
    }
    if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
    {
      *(prSvchData->pusAct_Data) = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
    }
    break;
    
    
  case 7:
    eRet = CSMD_PutData( prCSMD_Instance,
                         &prCSMD_Instance->parSvchMngmtData[usSlaveIdx],
                         (CSMD_VOID(*)(CSMD_VOID))NULL );
    
    if (eRet != CSMD_NO_ERROR) 
    {
      if (eRet == CSMD_SVC_ERROR_MESSAGE) prSvchData->usSvchError = prCSMD_Instance->parSvchMngmtData[usSlaveIdx].SVCH_Status.usStatus;
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
      return (eRet);
    }
    break;
    
    
  default:
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
    return (CSMD_WRONG_ELEMENT_NBR);
  
  } /* end: switch (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usElem) */
  
  
  if (prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic == CSMD_FINISHED_REQUEST)
  {
    /* continue with next step, preparation etc. */
    /* request nearly finished, can not be cancelled any more */
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usReplaceable = 0U;
    
    prSvchData->usState = CSMD_DATA_VALID;
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usActStateAtomic = CSMD_INIT_REQUEST;
    if (prSvchData->usInternalReq)
    {
      prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usIntReqPend = 0U;
    }
    
    prCSMD_Instance->parSvchMngmtData[usSlaveIdx].usInUse = CSMD_SVC_CONTAINER_NOT_IN_USE;
    *pboBreak = FALSE;
  }
  else
  {
    *pboBreak=TRUE;
  }
  
  return (CSMD_NO_ERROR);
  
}/* end of CSMD_FinalStepWrite() */



/**************************************************************************/ /**
\brief Writes the appropriate data for the transmission to the service container.

\ingroup func_svcc
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchMngmtData
              Pointer to the Sercos delivery structure

\return       \ref CSMD_SVCWRITE_LENGTH_ERROR \n
              \ref CSMD_SYSTEM_ERROR \n
              \ref CSMD_NO_ERROR \n

\author       RA
\date         03.2005

***************************************************************************** */
CSMD_FUNC_RET CSMD_SVCHWrite( CSMD_INSTANCE          *prCSMD_Instance,
                              CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData )
{
  
  CSMD_USHORT    usCtrlWrd[5];    /* Service container control words */
  CSMD_USHORT    usNum_Words;
  CSMD_USHORT    usSlaveIdx;
  CSMD_SERC3SVC *prSVC;
  
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  
  usCtrlWrd[1] = CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[1] );
  usCtrlWrd[0] = CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] );
  
  /* Reset WRDATPT & WRDATLAST */
  usCtrlWrd[2] = CSMD_SC_CLEAR_REGISTER;
  
  /* Set RDDATPT & RDDATLAST */
  usCtrlWrd[3] =   CSMD_SERC_SC_WRBUF_LENGTH
                 | ((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH << CSMD_SC_LENGTH_SHIFT);
  
  /* ------------------------------------------------------------------------
    Telegram-Header Control Word 4: reset the Error-Flags
    bfERR_CNT           = 0xFF            FF
    btBUSY_CNT          = 0x00 - 0 -\_  
    btINT_SC_ERR        = 0x00 - 0 -/ \_  0
    btINT_HS_TIMEOUT    = 0x00 - 0 -\_/
    btINT_BUSY_TIMEOUT  = 0x00 - 0 -/ 
    btINT_CMD           = 0x00 - 0 ------ 0
  ------------------------------------------------------------------------*/
  usCtrlWrd[4] = CSMD_SC_CLEAR_REGISTER;
  
  /* ------------------------------------------------------------------------
    Telegram-Header Control Word 0
    init SVCC0 by setting the following bits to 0
    btLS_MDT        = 0x00
    btEND_MDT       = 0x00
    bfELEM_MDT      = 0x00
    btSETEND        = 0x00
    btINT_ERR       = 0x00
    btINT_END_WRBUF = 0x00
    btINT_END_RDBUF = 0x00
  ------------------------------------------------------------------------*/
  usCtrlWrd[0] &= CSMD_SC_SVCC0_INIT_MASK;
  
  /* Set data element type in MDT */ 
  usCtrlWrd[0] |= (CSMD_USHORT) (prSvchMngmtData->usElem << CSMD_SC_SVCC0_ELEM_SHIFT);
  
  /*Set Read/Write in MDT */
  usCtrlWrd[0] |= CSMD_SVC_CTRL_WRITE;
  
  /* Is it the last transmission? */
  if (prSvchMngmtData->usSetEnd)
  {
    /* END_MDT is to be set */
    usCtrlWrd[0] |= CSMD_SVC_CTRL_SETEND;
    
    if (prSvchMngmtData->usLength <= 4U)
    {    
      /* End of element transmission */
      usCtrlWrd[0] |= CSMD_SVC_CTRL_LASTTRANS;
    }
  }
  
  if (prSvchMngmtData->usLength)
  {
    /* Calculate and set WRDATLAST */
    usCtrlWrd[2] |= (CSMD_USHORT) (   (CSMD_USHORT) ((((prSvchMngmtData->usLength + 3)/4)*2) - 2) 
                                   << CSMD_SC_LENGTH_SHIFT);
    
#ifdef CSMD_BIG_ENDIAN
    if (   (prSvchMngmtData->usElem == CSMD_SVC_DBE1_IDN)
        || (prSvchMngmtData->usElem == CSMD_SVC_DBE0_CLOSE))
    {
      for (usNum_Words = 0;
           usNum_Words < (CSMD_USHORT)((prSvchMngmtData->usLength + (CSMD_USHORT)1)/(CSMD_USHORT)2); 
           usNum_Words += 2)
      {
        CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words+1], 
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words], 
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
      }
    }
    else
    {
      usNum_Words = 0;
      /* At the beginning of a list? */
      if (   (prSvchMngmtData->ulAttribute & CSMD_SERC_VAR_LEN)
          && (prSvchMngmtData->usAct_Position == 0))
      {
        /* Write current length of list */
        CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words++], 
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        /* Write maximum length of list */
        CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words++], 
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
      }
      
      switch (prSvchMngmtData->ulAttribute & CSMD_SERC_LEN_WO_LISTINFO)
      {
      case 0:     /* Variable length byte */
        for ( ; usNum_Words < (prSvchMngmtData->usLength + 1) / 2; usNum_Words++)
        {
          CSMD_HAL_WriteShortNoConv( &prSVC->sWR_BUFF[usNum_Words], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        }
        break;
      case CSMD_SERC_WORD_LEN:
        for ( ; usNum_Words < (prSvchMngmtData->usLength + 1) / 2; usNum_Words++)
        {
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        }
        break;
      case CSMD_SERC_LONG_LEN:
        for ( ; usNum_Words < (prSvchMngmtData->usLength + 1) / 2; usNum_Words += 2)
        {
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words+1], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        }
        break;
      case CSMD_SERC_DOUBLE_LEN:
        for ( ; usNum_Words < (prSvchMngmtData->usLength + 1) / 2; usNum_Words += 4)
        {
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words+3], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words+2], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words+1], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        }
        break;
      default:
        /* Can not be here.! */
        return (CSMD_SYSTEM_ERROR);
      }
    }
#else
#ifdef CSMD_TEST_BE
    if (   (prSvchMngmtData->usElem == CSMD_SVC_DBE1_IDN)
        || (prSvchMngmtData->usElem == CSMD_SVC_DBE0_CLOSE))
    {
      for (usNum_Words = 0;
           usNum_Words < (CSMD_USHORT)((prSvchMngmtData->usLength + 1U) / 2U);
           usNum_Words += 2)
      {
        CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words], 
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words+1], 
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
      }
    }
    else
    {
      if (!prSvchMngmtData->ulAttribute)
        return (CSMD_SYSTEM_ERROR);
      
      usNum_Words = 0;
      /* At the beginning of a list? */
      if (   (prSvchMngmtData->ulAttribute & CSMD_SERC_VAR_LEN)
          && (prSvchMngmtData->usAct_Position == 0))
      {
        /* Write current length of list */
        CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words++], 
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        /* Write maximum length of list */
        CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words++], 
          prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
      }
      
      switch (prSvchMngmtData->ulAttribute & CSMD_SERC_LEN_WO_LISTINFO)
      {
      case 0:     /* Variable length byte */
        for ( ; usNum_Words < (prSvchMngmtData->usLength + 1) / 2; usNum_Words++)
        {
          CSMD_HAL_WriteShortNoConv( &prSVC->sWR_BUFF[usNum_Words], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        }
        break;
      case CSMD_SERC_WORD_LEN:
        for ( ; usNum_Words < (prSvchMngmtData->usLength + 1) / 2; usNum_Words++)
        {
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        }
        break;
      case CSMD_SERC_LONG_LEN:
        for ( ; usNum_Words < (prSvchMngmtData->usLength + 1) / 2; usNum_Words += 2)
        {
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words+1], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        }
        break;
      case CSMD_SERC_DOUBLE_LEN:
        for ( ; usNum_Words < (prSvchMngmtData->usLength + 1) / 2; usNum_Words += 4)
        {
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words+1], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words+2], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
          CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words+3], 
            prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position++] );
        }
        break;
      default:
        /* Can not be here.! */
        return (CSMD_SYSTEM_ERROR);
      }
    }
#else
    /* Copying data into the write buffer of the service container */
    for (usNum_Words = 0;
         usNum_Words < (prSvchMngmtData->usLength + 1) / 2;
         usNum_Words++)
    {
      CSMD_HAL_WriteShort( &prSVC->sWR_BUFF[usNum_Words], 
                           prSvchMngmtData->pusAct_Data[prSvchMngmtData->usAct_Position] );
      
      prSvchMngmtData->usAct_Position++;
    }
#endif
#endif
  }
  else
  {
    return (CSMD_SVCWRITE_LENGTH_ERROR);
  }
  
  /* Toggle Handshake-bit in MDT depending on Handshake-bit in AT */
  if (usCtrlWrd[1] & CSMD_SVC_STAT_HANDSHAKE)
  {
    usCtrlWrd[0] &= (CSMD_USHORT)~CSMD_SVC_CTRL_HANDSHAKE;
  }
  else
  {
    usCtrlWrd[0] |= CSMD_SVC_CTRL_HANDSHAKE;
  }
  
  /* Starting service channel transmission */
  usCtrlWrd[0] &= (CSMD_USHORT)~CSMD_SVC_CTRL_M_BUSY;
  
  
  CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[4], usCtrlWrd[4] );
  CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[3], usCtrlWrd[3] );
  CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[2], usCtrlWrd[2] );
  
  CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[0], usCtrlWrd[0] );
  
#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
  if (prSvchMngmtData->usSrv_Cont == CSMD_SW_SRV_CONTAINER)
  {
    prCSMD_Instance->rPriv.parSoftSvc[usSlaveIdx - CSMD_MAX_HW_CONTAINER].usInUse = CSMD_SVC_CONTAINER_IN_USE;
    prCSMD_Instance->usSoftSrvcCnt++;
  }
#endif

  return (CSMD_NO_ERROR);
  
} /* end: CSMD_SVCHWrite */



/**************************************************************************/ /**
\brief Reads the appropriate data from the service container.

\ingroup func_svcc
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prSvchMngmtData
              Pointer to the Sercos delivery structure

\return       none

\author       yag
\date         30.11.2003

***************************************************************************** */
CSMD_VOID CSMD_SVCHRead( CSMD_INSTANCE                *prCSMD_Instance,
                         const CSMD_SVCH_MNGMT_STRUCT *prSvchMngmtData )
{
  
  CSMD_USHORT    usCtrlWrd[5];    /* Service container control words */
  CSMD_USHORT    usSlaveIdx;
  CSMD_SERC3SVC *prSVC;
  
  
  usSlaveIdx = prSvchMngmtData->usSlaveIdx;
  
  prSVC = prCSMD_Instance->rPriv.prSVContainer[usSlaveIdx];
  
  usCtrlWrd[1] = CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[1] );
  usCtrlWrd[0] = CSMD_HAL_ReadShort( &prSVC->rCONTROL.usWord[0] );
  
  /* Reset WRDATPT & WRDATLAST */
  usCtrlWrd[2] = CSMD_SC_CLEAR_REGISTER;
  
  /* Set RDDATPT */
  usCtrlWrd[3] = CSMD_SERC_SC_WRBUF_LENGTH;
  
  /* set RDDATLAST */
  if (prSvchMngmtData->usLength <= 4U)
  {
    usCtrlWrd[3] |= ((CSMD_USHORT)CSMD_SERC_SC_WRBUF_LENGTH << CSMD_SC_LENGTH_SHIFT);
  }
  else
  {
    usCtrlWrd[3] |= (CSMD_USHORT)(   (CSMD_USHORT)(CSMD_SERC_SC_WRBUF_LENGTH + (((prSvchMngmtData->usLength + 3)/4)*2) - 2)
                                  << CSMD_SC_LENGTH_SHIFT);
  }
  
  /* ------------------------------------------------------------------------
    Telegram-Header Control Word 4: reset the Error-Flags
    btINT_HS_TIMEOUT    = 0x00
    btINT_BUSY_TIMEOUT  = 0x00
    btBUSY_CNT          = 0x00
    btINT_SC_ERR        = 0x00
  ------------------------------------------------------------------------*/
  usCtrlWrd[4] = CSMD_SC_CLEAR_REGISTER;
  
  /* ------------------------------------------------------------------------
    Telegram-Header Control Word 0
    init SVCC0 by setting the following bits to 
    btLS_MDT        = 0x00
    btEND_MDT       = 0x00
    bfELEM_MDT      = 0x00
    btSETEND        = 0x00
    btINT_ERR       = 0x00
    btINT_END_WRBUF = 0x00
    btINT_END_RDBUF = 0x00
  ------------------------------------------------------------------------*/
  usCtrlWrd[0] &= CSMD_SC_SVCC0_INIT_MASK;
  
  /* Set data element type in MDT */ 
  usCtrlWrd[0] |= (CSMD_USHORT) (prSvchMngmtData->usElem << CSMD_SC_SVCC0_ELEM_SHIFT);
  
  /* Set Read/Write in MDT */
  /* usCtrlWrd[0] &= CSMD_SVC_CTRL_WRITE; */
  
  /* Is it the last transmission? */
  if (prSvchMngmtData->usSetEnd)
  { 
  /* Last transmission:  Either it concerns a data more firmly 
    data length or it is the last one of a ' current ' transmission */
    /* END_MDT is to be set */
    usCtrlWrd[0] |= CSMD_SVC_CTRL_SETEND;
    
    if (prSvchMngmtData->usLength <= 4U)      /*Datum 4-Byte length (2 word)*/
    {
      /* End of element transmission */
      usCtrlWrd[0] |= CSMD_SVC_CTRL_LASTTRANS;
    }
  }
  
  /* Toggle Handshake-bit in MDT depending on Handshake-bit in AT */
  if (usCtrlWrd[1] & CSMD_SVC_STAT_HANDSHAKE)
  {
    usCtrlWrd[0] &= (CSMD_USHORT)~CSMD_SVC_CTRL_HANDSHAKE;
  }
  else
  {
    usCtrlWrd[0] |= CSMD_SVC_CTRL_HANDSHAKE;
  }
  
  /* Starting service channel transmission */
  usCtrlWrd[0] &= (CSMD_USHORT)~CSMD_SVC_CTRL_M_BUSY;
  
  
  CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[4], usCtrlWrd[4] );
  CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[3], usCtrlWrd[3] );
  CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[2], usCtrlWrd[2] );
  
  CSMD_HAL_WriteShort( &prSVC->rCONTROL.usWord[0], usCtrlWrd[0] );
  
#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
  if (prSvchMngmtData->usSrv_Cont == CSMD_SW_SRV_CONTAINER)
  {
    prCSMD_Instance->rPriv.parSoftSvc[usSlaveIdx - CSMD_MAX_HW_CONTAINER].usInUse = CSMD_SVC_CONTAINER_IN_USE;
    prCSMD_Instance->usSoftSrvcCnt++;
  }
#endif
} /* end: CSMD_SVCHRead */



/**************************************************************************/ /**
\brief Clears all the SVC interrupts due to an internal call.

\ingroup func_svcc
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance

\return       none

\author       WK
\date         08.04.2008

***************************************************************************** */
CSMD_VOID CSMD_ClearInternalSVCInterrupts( CSMD_INSTANCE *prCSMD_Instance )
{
  if (prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves > CSMD_MAX_HW_CONTAINER)
  {
    CSMD_USHORT usI;

    /* clear interrupt status of software svc */
    for (usI = 0; usI < (((prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves-1) / 32)+1); usI++)
    {
      prCSMD_Instance->rPriv.aulSVC_Int_Flags[usI] = 0UL;
    }
  }
  
#if (CSMD_MAX_HW_CONTAINER > 0)
  /* clear interrupt status of hardware svc */
  CSMD_HAL_ClearInterrupt( &prCSMD_Instance->rCSMD_HAL, 
                           FALSE, 
                           TRUE,
                           0xFFFFFFFFU );
#endif
}   /* End: CSMD_ClearInternalSVCInterrupts() */



/**************************************************************************/ /**
\brief Clears the svc interrupt due to internal call.for the specified slave.

\b Description: \n
              Clears the svc interrupt due to internal call for the specified slave.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance 
              pointer to the CoSeMa instance structure
\param [in]   usSlaveIdx
              Slave index

\return       none

\author       WK
\date         27.10.2011

***************************************************************************** */
CSMD_VOID CSMD_ClearInternal_SVCInterrupt( CSMD_INSTANCE *prCSMD_Instance,
                                           CSMD_USHORT    usSlaveIdx )
{
  
  if ((CSMD_SHORT)usSlaveIdx >= CSMD_MAX_HW_CONTAINER)
  {
    /* emulated SVC */
    CSMD_ULONG  idx = usSlaveIdx / 32;
    CSMD_ULONG  bit = usSlaveIdx % 32;
    prCSMD_Instance->rPriv.aulSVC_Int_Flags[idx] &= ~(1UL << bit);
  }
  else
  {
    /* hardware SVC */
    CSMD_HAL_ClearInterrupt( &prCSMD_Instance->rCSMD_HAL, 
                             FALSE, 
                             TRUE,
                             (CSMD_ULONG) (1UL << usSlaveIdx) );
  }
}   /* End: CSMD_ClearInternal_SVCInterrupt() */

/*! \endcond */ /* PRIVATE */






/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
19 Mar 2014 WK
  - CSMD_SVCHWrite()
    Adjust setting of the "Last transmission" bit.
  - Defdb00168400
    CSMD_CheckRequestCancel()
    If the request is canceled, adjust SVC macro structure for next request.
02 Apr 2014 WK
  - Defdb00168915
    CSMD_ClearInternalSVCInterrupts()
    Fixed number of loop passes for CSMD_MAX_DEVICE is divisible by 64
    without remainder.
16 Jan 2015 WK
  - Defdb00175145
    CSMD_CheckRequestCancel()
    Set usInUse = 2 (prepare cancel) for emulated SVC to prevent start of
    a new request, if the function CSMD_TxRxSoftCont() is not yet performed.
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
