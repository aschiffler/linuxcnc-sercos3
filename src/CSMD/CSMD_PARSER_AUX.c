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
 \file   CSMD_PARSER_AUX.c
 \author WK
 \date   14.11.2014
 \brief  This File contains the private functions for connection configuration
         purposes through a binary format
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"

#ifdef CSMD_CONFIG_PARSER
#include "CSMD_HAL_PRIV.h"
#include "CSMD_PARSER.h"

/*---- Definition private Functions: -----------------------------------------*/


/*! \cond PRIVATE */

/**************************************************************************/ /**
\brief Checks the header of the configuration.

\ingroup func_binconfig
\b Description: \n
   This function checks whether the header of the given configuration
   corresponds to the requested "CbinC_". If the first 6 bytes don't match,
   it is a wrong format.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
   
\param [in]   pusHeaderPtr
              Pointer to the header of the configuration

\return       \ref CSMD_NO_BIN_CONFIG \n
              \ref CSMD_NO_ERROR \n

\author       MSt
\date         11.01.2011

***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckCbinC_Header( const CSMD_USHORT *pusHeaderPtr )
{
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  CSMD_CHAR     *pcCheckPointer = (CSMD_CHAR *)pusHeaderPtr;

  if (*pcCheckPointer++ != 0x43 /* "C" */)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }
  else if (*pcCheckPointer++ != 0x53 /* "S" */)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }
  else if (*pcCheckPointer++ != 0x4D /* "M" */)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }
  else if (*pcCheckPointer++ != 0x43 /* "C" */)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }
  else if (*pcCheckPointer++ != 0x66 /* "f" */)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }
  else if (*pcCheckPointer++ != 0x67 /* "g" */)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }
  else if (*pcCheckPointer++ != 0x5F /* "_" */)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }
  else if (*pcCheckPointer++ != 0x62 /* "b" */)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }
  else if (*pcCheckPointer++ != 0x69 /* "i" */)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }
  else if (*pcCheckPointer != 0x6E /* "n" */)
  {
    eFuncRet = CSMD_NO_BIN_CONFIG;
  }

  return(eFuncRet);
} /* end CSMD_CheckCbinC_Header */

/**************************************************************************/ /**
\brief Searches the table headers.

\ingroup func_binconfig
\b Description: \n
   This function checks whether the header of the given configuration
   corresponds to the requested "CbinC_". If the first 6 bytes don't match,
   it is a wrong format.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
   
\param [in]   pusConfigDataPtr
              Pointer to the first data behind configuration header
\param [in]   usListLength
              Length of the configuration data
\param [out]  prTableHeaderPtr
              Outpu pointer to a list of table headers

\return       \ref CSMD_WRONG_BIN_CONFIG_FORMAT \n
              \ref CSMD_NO_ERROR \n

\author       MSt
\date         11.01.2011

***************************************************************************** */
CSMD_FUNC_RET CSMD_SearchTableHeaders( CSMD_USHORT        *pusConfigDataPtr,
                                       CSMD_USHORT         usListLength,
                                       CSMD_TABLE_POINTER *prTableHeaderPtr )
{
  CSMD_FUNC_RET  eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT    usLongDataOffset     = 1U;
  CSMD_USHORT    usTempLongDataOffset = 0U;
  CSMD_CHAR     *pcCheckPointer = (CSMD_CHAR *)pusConfigDataPtr;
  CSMD_BOOL      boSlaveSetup = FALSE;
  
  /* Search for connections table at the beginning of the data */
  
  if (*pcCheckPointer++ != 0x43 /* "C" */)
  {
    eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
  }
  else if (*pcCheckPointer++ != 0x6E /* "n" */)
  {
    eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
  }
  else if (*pcCheckPointer++ != 0x6E /* "n" */)
  {
    eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
  }
  else if (*pcCheckPointer++ != 0x63 /* "c" */)
  {
    eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
  }
  else
  {
    /* no error */
    prTableHeaderPtr->pusCnncStartPtr = pusConfigDataPtr;
  }
  

  /* no error occured so far? -> go on */
  if (eFuncRet == CSMD_NO_ERROR)
  {
    /* Search for producer table */
    do
    {
      CSMD_ULONG *pulHeaderPtr = (CSMD_ULONG *)(CSMD_VOID *)pusConfigDataPtr;

      /* Check only long aligned data */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset));
      
      if (*pcCheckPointer++ != 0x50 /* "P" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x72 /* "r" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x64 /* "d" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x63 /* "c" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }

      /* Correct header was found */

      /* Check the position before the header; has to be end sign */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset - 1));
      
      if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else
      {
        /* no error */
        prTableHeaderPtr->pusPrdcStartPtr = (CSMD_USHORT *)(CSMD_VOID *)(pulHeaderPtr + usLongDataOffset);
        
        /* Plausibility check for the pointer; 
        does the distance match the struct size? */
        if ((  ((CSMD_ULONG)(  (CSMD_UCHAR *)prTableHeaderPtr->pusPrdcStartPtr
                             - (CSMD_UCHAR *)prTableHeaderPtr->pusCnncStartPtr) - CSMD_TABLE_OVERHEAD)
             % CSMD_CONNECTION_TABLE_LEN) == 0)
        {
          /* size OK -> search next header pointer */
          break;
        }
      }

    }while(usLongDataOffset++ < usListLength / 4);
  }

  /* Nothing found? -> error! */
  if (!(usLongDataOffset++ < usListLength / 4))
  {
    eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
  }
  
  /* no error occured so far? -> go on */
  if (eFuncRet == CSMD_NO_ERROR)
  {  /* Search for consumer list table */
    do
    {
      CSMD_ULONG *pulHeaderPtr = (CSMD_ULONG *)(CSMD_VOID *)pusConfigDataPtr;

      /* Check only long aligned data */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset));
      
      if (*pcCheckPointer++ != 0x43 /* "C" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x6E /* "n" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x4C /* "L" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x73 /* "s" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }

      /* Correct header was found */

      /* Check the position before the header; has to be end sign */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset - 1));
      
      if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else
      {
        /* no error */
        prTableHeaderPtr->pusConsLStartPtr = (CSMD_USHORT *)(CSMD_VOID *)(pulHeaderPtr + usLongDataOffset);
        
        /* Plausibility check for the pointer; 
        does the distance match the struct size? */
        if ((  ((CSMD_ULONG)(  (CSMD_UCHAR *)prTableHeaderPtr->pusConsLStartPtr
                             - (CSMD_UCHAR *)prTableHeaderPtr->pusPrdcStartPtr) - CSMD_TABLE_OVERHEAD)
             % CSMD_PRODUCER_TABLE_LEN) == 0)
        {
          /* size OK -> search next header pointer */
          break;
        }
      }
      
    }while(usLongDataOffset++ < usListLength / 4);
  }

  /* Nothing found? -> error! */
  if (!(usLongDataOffset++ < usListLength / 4))
  {
    eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
  }
  
  /* no error occured so far? -> go on */
  if (eFuncRet == CSMD_NO_ERROR)
  {
    /* Search for consumer table */
    do
    {
      CSMD_ULONG *pulHeaderPtr = (CSMD_ULONG *)(CSMD_VOID *)pusConfigDataPtr;

      /* Check only long aligned data */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset));
      
      if (*pcCheckPointer++ != 0x43 /* "C" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x6E /* "n" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x73 /* "s" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x6D /* "m" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }

      /* Correct header was found */

      /* Check the position before the header; has to be end sign */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset - 1));
      
      if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else
      {
        /* no error */
        prTableHeaderPtr->pusConsTableStartPtr = (CSMD_USHORT *)(CSMD_VOID *)(pulHeaderPtr + usLongDataOffset);

        /* Plausibility check for the pointer not possible because 
          consumer list table has dynamic size */

        break;
      }
      
    }while(usLongDataOffset++ < usListLength / 4);
  }
  
  /* Nothing found? -> error! */
  if (!(usLongDataOffset++ < usListLength / 4))
  {
    eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
  }
  
  /* no error occured so far? -> go on */
  if (eFuncRet == CSMD_NO_ERROR)
  {
    /* Search for configurations table */
    do
    {
      CSMD_ULONG *pulHeaderPtr = (CSMD_ULONG *)(CSMD_VOID *)pusConfigDataPtr;

      /* Check only long aligned data */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset));
      
      if (*pcCheckPointer++ != 0x43 /* "C" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x6E /* "n" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x66 /* "f" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x67 /* "g" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }

      /* Correct header was found */

      /* Check the position before the header; has to be end sign */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset - 1));
      
      if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else
      {
        /* no error */
        prTableHeaderPtr->pusConfigTableStartPtr = (CSMD_USHORT *)(CSMD_VOID *)(pulHeaderPtr + usLongDataOffset);
        
        /* Plausibility check for the pointer; 
        does the distance match the struct size? */
        if ((  ((CSMD_ULONG)(  (CSMD_UCHAR *)prTableHeaderPtr->pusConfigTableStartPtr
                             - (CSMD_UCHAR *)prTableHeaderPtr->pusConsTableStartPtr) - CSMD_TABLE_OVERHEAD)
             % CSMD_CONSUMER_TABLE_LEN) == 0)
        {
          /* size OK -> search next header pointer */
          break;
        }
      }
      
    }while(usLongDataOffset++ < usListLength / 4);
  }
  
  /* Nothing found? -> error! */
  if (!(usLongDataOffset++ < usListLength / 4))
  {
    eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
  }
  
  /* no error occured so far? -> go on */
  if (eFuncRet == CSMD_NO_ERROR)
  {
    /* Search for RT bits configuration table */
    do
    {
      CSMD_ULONG *pulHeaderPtr = (CSMD_ULONG *)(CSMD_VOID *)pusConfigDataPtr;

      /* Check only long aligned data */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset));
      
      if (*pcCheckPointer++ != 0x52 /* "R" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x54 /* "T" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x42 /* "B" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x74 /* "t" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }

      /* Correct header was found */

      /* Check the position before the header; has to be end sign */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset - 1));
      
      if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else
      {
        /* no error */
        prTableHeaderPtr->pusRTBitsTableStartPtr = (CSMD_USHORT *)(CSMD_VOID *)(pulHeaderPtr + usLongDataOffset);
        
        /* Plausibility check not possible because configuration table
        has dynamic size */
        
        break;
      }
      
    }while(usLongDataOffset++ < usListLength / 4);
  }

  /* Nothing found? -> error! */
  if (!(usLongDataOffset++ < usListLength / 4))
  {
    eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
  }
  
  /* no error occured so far? -> go on */
  if (eFuncRet == CSMD_NO_ERROR)
  {
    /* Store the search pointer temporarily; needed in case no slave setup exists */
    usTempLongDataOffset = usLongDataOffset;

    /* Search for slave setup table; this table is optional */
    do
    {
      CSMD_ULONG *pulHeaderPtr = (CSMD_ULONG *)(CSMD_VOID *)pusConfigDataPtr;

      /* Check only long aligned data */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset));

      if (*pcCheckPointer++ != 0x53 /* "S" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x6C /* "l" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x53 /* "S" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x74 /* "t" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }

      /* Correct header was found, slave setup exists */
      boSlaveSetup = TRUE;

      /* Check the position before the header; has to be end sign */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset - 1));
      
      if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else
      {
        /* no error */
        prTableHeaderPtr->pusSlaveSetupStartPtr = (CSMD_USHORT *)(CSMD_VOID *)(pulHeaderPtr + usLongDataOffset);
        
        /* Plausibility check for the pointer; 
        does the distance match the struct size? */
        if ((  ((CSMD_ULONG)(  (CSMD_UCHAR *)prTableHeaderPtr->pusSlaveSetupStartPtr
                             - (CSMD_UCHAR *)prTableHeaderPtr->pusRTBitsTableStartPtr) - CSMD_TABLE_OVERHEAD)
             % CSMD_RTBITS_TABLE_LEN) != 0)
        {
          eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
        }
        
        break;
      }
      
    }while(usLongDataOffset++ < usListLength / 4);
  }
      
  /* Nothing found? -> probably no slave configuration */
  if (!(usLongDataOffset++ < usListLength / 4))
  {
    /* restore the search pointer */
    usLongDataOffset = usTempLongDataOffset;
  }

  if (boSlaveSetup)
  {
    
    /* no error occured so far and slave setup exists? -> go on */
    if ((eFuncRet == CSMD_NO_ERROR))
    {
      /* Search for setup parameters list table; only exisiting if slave setup exists */
      do
      {
        CSMD_ULONG *pulHeaderPtr = (CSMD_ULONG *)(CSMD_VOID *)pusConfigDataPtr;
        
        /* Check only long aligned data */
        pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset));
        
        if (*pcCheckPointer++ != 0x53 /* "S" */)
        {
          /* Wrong value -> check the next long data element */
          continue;
        }
        else if (*pcCheckPointer++ != 0x74 /* "t" */)
        {
          /* Wrong value -> check the next long data element */
          continue;
        }
        else if (*pcCheckPointer++ != 0x50 /* "P" */)
        {
          /* Wrong value -> check the next long data element */
          continue;
        }
        else if (*pcCheckPointer++ != 0x4C /* "L" */)
        {
          /* Wrong value -> check the next long data element */
          continue;
        }

        /* Correct header was found */
        
        /* Check the position before the header; has to be end sign */
        pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset - 1));
        
        if (*pcCheckPointer++ != 0x7E /* "~" */)
        {
          /* Wrong value -> set error and stop */
          eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
          
          break;
        }
        else if (*pcCheckPointer++ != 0x5E /* "^" */)
        {
          /* Wrong value -> set error and stop */
          eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
          
          break;
        }
        else if (*pcCheckPointer++ != 0x7E /* "~" */)
        {
          /* Wrong value -> set error and stop */
          eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
          
          break;
        }
        else if (*pcCheckPointer++ != 0x5E /* "^" */)
        {
          /* Wrong value -> set error and stop */
          eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
          
          break;
        }
        else
        {
          /* no error */
          prTableHeaderPtr->pusSetupParametersListStartPtr = (CSMD_USHORT *)(CSMD_VOID *)(pulHeaderPtr + usLongDataOffset);
          
          /* Plausibility check for the pointer; 
          does the distance match the struct size? */
          if ((  ((CSMD_ULONG)(  (CSMD_UCHAR *)prTableHeaderPtr->pusSetupParametersListStartPtr
                               - (CSMD_UCHAR *)prTableHeaderPtr->pusSlaveSetupStartPtr) - CSMD_TABLE_OVERHEAD)
               % CSMD_SLAVE_SETUP_TABLE_LEN) != 0)
          {
            eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
          }
          
          break;
        }
        
      }while(usLongDataOffset++ < usListLength / 4);
    }
    
    /* Nothing found? -> error! */
    if (!(usLongDataOffset++ < usListLength / 4))
    {
      eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
    }
    
    /* no error occured so far and slave setup exists? -> go on */
    if ((eFuncRet == CSMD_NO_ERROR))
    {
      /* Search for setup parameter table; only exisiting if slave setup exists */
      do
      {
        CSMD_ULONG *pulHeaderPtr = (CSMD_ULONG *)(CSMD_VOID *)pusConfigDataPtr;
        
        /* Check only long aligned data */
        pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset));
        
        if (*pcCheckPointer++ != 0x50 /* "P" */)
        {
          /* Wrong value -> check the next long data element */
          continue;
        }
        else if (*pcCheckPointer++ != 0x72 /* "r" */)
        {
          /* Wrong value -> check the next long data element */
          continue;
        }
        else if (*pcCheckPointer++ != 0x6D /* "m" */)
        {
          /* Wrong value -> check the next long data element */
          continue;
        }
        else if (*pcCheckPointer++ != 0x74 /* "t" */)
        {
          /* Wrong value -> check the next long data element */
          continue;
        }
        
        /* Correct header was found */
        
        /* Check the position before the header; has to be end sign */
        pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset - 1));
        
        if (*pcCheckPointer++ != 0x7E /* "~" */)
        {
          /* Wrong value -> set error and stop */
          eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
          
          break;
        }
        else if (*pcCheckPointer++ != 0x5E /* "^" */)
        {
          /* Wrong value -> set error and stop */
          eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
          
          break;
        }
        else if (*pcCheckPointer++ != 0x7E /* "~" */)
        {
          /* Wrong value -> set error and stop */
          eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
          
          break;
        }
        else if (*pcCheckPointer++ != 0x5E /* "^" */)
        {
          /* Wrong value -> set error and stop */
          eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
          
          break;
        }
        else
        {
          /* no error */
          prTableHeaderPtr->pusSetupParameterStartPtr = (CSMD_USHORT *)(CSMD_VOID *)(pulHeaderPtr + usLongDataOffset);
          
          break;
        }
        
      }while(usLongDataOffset++ < usListLength / 4);
    }
  }
      
  /* no error occured so far? -> go on */
  if (eFuncRet == CSMD_NO_ERROR)
  {
    /* Search for end of table */
    do
    {
      CSMD_ULONG *pulHeaderPtr = (CSMD_ULONG *)(CSMD_VOID *)pusConfigDataPtr;

      /* Check only long aligned data */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset));
      
      if (*pcCheckPointer++ != 0x65 /* "e" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x4E /* "N" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x44 /* "D" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }
      else if (*pcCheckPointer++ != 0x45 /* "E" */)
      {
        /* Wrong value -> check the next long data element */
        continue;
      }

      /* Correct header was found */

      /* Check the position before the header; has to be end sign */
      pcCheckPointer = ((CSMD_CHAR *)(pulHeaderPtr + usLongDataOffset - 1));
      
      if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x7E /* "~" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else if (*pcCheckPointer++ != 0x5E /* "^" */)
      {
        /* Wrong value -> set error and stop */
        eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;

        break;
      }
      else
      {
        /* no error */
        prTableHeaderPtr->pusConnConfigEndPtr = (CSMD_USHORT *)(CSMD_VOID *)(pulHeaderPtr + usLongDataOffset);
        
        break;
      }
      
    }while(usLongDataOffset++ < usListLength / 4);
  }
  
  /* Check for error; if the last pointer has no value, something was wrong */
  if (prTableHeaderPtr->pusConnConfigEndPtr == NULL)
  {
    eFuncRet = CSMD_WRONG_BIN_CONFIG_FORMAT;
  }

  return(eFuncRet);
} /* end CSMD_SearchTableHeaders */

/**************************************************************************/ /**
\brief Checks Sercos addresses in the tables.

\ingroup func_binconfig
\b Description: \n
   This function checks the Sercos addresses which are given in the producer
   table, the consumer table, and in the slave setup table, if it exists. If 
   an address is not projected, an error will occur.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prSlaveList
              Pointer to the CoSeMa slave list
\param [in]   prTableHeaderPtr
              Pointer to a list of table headers
\param [in]   usNumberOfProducers
              Number of table entries in producer table
\param [in]   usNumberOfConsumers
              Number of table entries in consumer table

\return       \ref CSMD_WRONG_SLAVE_ADDRESS \n
              \ref CSMD_NO_ERROR \n

\author       MSt
\date         11.01.2011

***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckTableSlaveAddr( const CSMD_SLAVE_LIST    *prSlaveList,
                                        const CSMD_TABLE_POINTER *prTableHeaderPtr,
                                        CSMD_USHORT               usNumberOfProducers,
                                        CSMD_USHORT               usNumberOfConsumers )
{
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT   usI;
  
  /* Check producer table */
  for (usI = 0; usI < usNumberOfProducers; usI++)
  {
    /* get the actual producer */
    CSMD_PRODUCER_TABLE *prProdTable;
    CSMD_USHORT          usSercAddress;
    
    /* get the actual producer table element */
    prProdTable = (CSMD_PRODUCER_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)prTableHeaderPtr->pusPrdcStartPtr)
      + CSMD_TABLE_HEADER_LEN + (usI * CSMD_PRODUCER_TABLE_LEN));
    
    /* Get Sercos address */
    usSercAddress = CSMD_END_CONV_S(prProdTable->usSERCOS_Add);

    /* Check the address for exceeding maximum address */
    if (usSercAddress > CSMD_MAX_SLAVE_ADD)
    {
      /* Slave not available */
      eFuncRet = CSMD_WRONG_SLAVE_ADDRESS;
      
      break;
    }
    
    /* Address 0 is the master should be OK */
    if (usSercAddress != 0)
    {
      /* Check the address*/
      if (prSlaveList->ausProjSlaveIdxList[usSercAddress] == 0)
      {
        /* Index is 0 -> slave not available or it has got index 0 */
        if (prSlaveList->ausProjSlaveAddList[2] != usSercAddress)
        {
          CSMD_USHORT  usK;

          /* Slave deactivated? */
          for (usK = 0; usK < prSlaveList->ausDeactSlaveAddList[0] / 2; usK++)
          {
            if (prSlaveList->ausDeactSlaveAddList[usK + 2] == usSercAddress)
            {
              /* Slave is deactivated -> stop further search */
              break;
            }
          }
          
          if (usK == (CSMD_USHORT)(prSlaveList->ausDeactSlaveAddList[0] / 2))
          {
            /* Slave not available */
            eFuncRet = CSMD_WRONG_SLAVE_ADDRESS;
            
            break;
          }
        }
      }
    }
  }

  /* Check consumer table */
  if (eFuncRet == CSMD_NO_ERROR)
  {
    for (usI = 0; usI < usNumberOfConsumers; usI++)
    {
      /* get the actual consumer */
      CSMD_CONSUMER_TABLE *prConsTable;
      CSMD_USHORT          usSercAddress;
      
      /* get the actual consumer table element */
      prConsTable = (CSMD_CONSUMER_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)prTableHeaderPtr->pusConsTableStartPtr)
        + CSMD_TABLE_HEADER_LEN + (usI * CSMD_CONSUMER_TABLE_LEN));
      
      /* Get Sercos address */
      usSercAddress = CSMD_END_CONV_S(prConsTable->usSERCOS_Add);
      
      /* Check the address for exceeding maximum address */
      if (usSercAddress > CSMD_MAX_SLAVE_ADD)
      {
        /* Slave not available */
        eFuncRet = CSMD_WRONG_SLAVE_ADDRESS;
        
        break;
      }
      
      /* Address 0 is the master should be OK */
      if (usSercAddress != 0)
      {
        /* Check the address*/
        if (prSlaveList->ausProjSlaveIdxList[usSercAddress] == 0)
        {
          /* Index is 0 -> slave not available or it has got index 0 */
          if (prSlaveList->ausProjSlaveAddList[2] != usSercAddress)
          {
            CSMD_USHORT  usK;
            
            /* Slave deactivated? */
            for (usK = 0; usK < prSlaveList->ausDeactSlaveAddList[0] / 2; usK++)
            {
              if (prSlaveList->ausDeactSlaveAddList[usK + 2] == usSercAddress)
              {
                /* Slave is deactivated -> stop further search */
                break;
              }
            }
            
            if (usK == (CSMD_USHORT)(prSlaveList->ausDeactSlaveAddList[0] / 2))
            {
              /* Slave not available */
              eFuncRet = CSMD_WRONG_SLAVE_ADDRESS;
              
              break;
            }
          }
        }
      }
    }
  }

  /* Check slave setup table, if exists */
  if ((eFuncRet == CSMD_NO_ERROR) &&(prTableHeaderPtr->pusSlaveSetupStartPtr != NULL))
  {
    CSMD_USHORT    usNumberOfSlaveSetupTables;

    usNumberOfSlaveSetupTables = (CSMD_USHORT)
      (  ((CSMD_ULONG)(  (CSMD_UCHAR *)prTableHeaderPtr->pusSetupParametersListStartPtr
                       - (CSMD_UCHAR *)prTableHeaderPtr->pusSlaveSetupStartPtr) - CSMD_TABLE_OVERHEAD)
       / CSMD_SLAVE_SETUP_TABLE_LEN);

    for (usI = 0; usI < usNumberOfSlaveSetupTables; usI++)
    {
      /* get the actual slave */
      CSMD_SLAVE_SETUP_TABLE *prSlaveSetupTable;
      CSMD_USHORT             usSercAddress;
      
      /* get the actual slave setup table element */
      prSlaveSetupTable = (CSMD_SLAVE_SETUP_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)prTableHeaderPtr->pusSlaveSetupStartPtr)
        + CSMD_TABLE_HEADER_LEN + (usI * CSMD_SLAVE_SETUP_TABLE_LEN));
      
      /* Get Sercos address */
      usSercAddress = CSMD_END_CONV_S(prSlaveSetupTable->usSlaveAddress);
      
      /* Check the address for exceeding maximum address */
      if (usSercAddress > CSMD_MAX_SLAVE_ADD)
      {
        /* Slave not available */
        eFuncRet = CSMD_WRONG_SLAVE_ADDRESS;
        
        break;
      }
      
      /* Check the address*/
      if (usSercAddress != 0)
      {
        /* Only check if it is not the master */
        if (prSlaveList->ausProjSlaveIdxList[usSercAddress] == 0)
        {
          /* Index is 0 -> slave not available or it has got index 0 */
          if (prSlaveList->ausProjSlaveAddList[2] != usSercAddress)
          {
            CSMD_USHORT  usK;

            /* Slave deactivated? */
            for (usK = 0; usK < prSlaveList->ausDeactSlaveAddList[0] / 2; usK++)
            {
              if (prSlaveList->ausDeactSlaveAddList[usK + 2] == usSercAddress)
              {
                /* Slave is deactivated -> stop further search */
                break;
              }
            }

            if (usK == (CSMD_USHORT)(prSlaveList->ausDeactSlaveAddList[0] / 2))
            {
              /* Slave not available */
              eFuncRet = CSMD_WRONG_SLAVE_ADDRESS;

              break;
            }
          }
        }
      }
    }
  }

  return(eFuncRet);
} /* end CSMD_CheckTableSlaveAddr */

/**************************************************************************/ /**
\brief Clears connections with given Application ID.

\ingroup func_binconfig
\b Description: \n
   This function checks the connection table and clears all connections which
   contain one of the application IDs given in the new connection data.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prConnections
              Pointer to the connection table within the configuration data
\param [in]   usNumberOfConnections
              Number of table entries in connections table
\param [in]   boAddCheck
              Flag which determines whether the slave addresses from the
              slave list or the temporary addressews have to be used

\return       \ref CSMD_NO_ERROR \n

\author       MSt
\date         11.01.2011

***************************************************************************** */
CSMD_FUNC_RET CSMD_ClearAppID_Connections( CSMD_INSTANCE               *prCSMD_Instance,
                                           const CSMD_CONNECTION_TABLE *prConnections,
                                           CSMD_USHORT                  usNumberOfConnections,
                                           CSMD_BOOL                    boAddCheck )
{
  CSMD_CONFIG_STRUCT *prConfiguration = &prCSMD_Instance->rConfiguration;
  CSMD_FUNC_RET       eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT         usAppIdx;
  CSMD_USHORT         usNumSlaves;
  CSMD_USHORT         usI, usK, usL;
  CSMD_USHORT         ausApplicationIDs[CSMD_NUMBER_OF_APPLICATIONS];
  CSMD_UCHAR          aucNeededConn[CSMD_MAX_GLOB_CONN]       = {FALSE};
  CSMD_UCHAR          aucConfig2Check[CSMD_MAX_GLOB_CONFIG]   = {FALSE};
  CSMD_UCHAR          aucRTBits2Check[CSMD_MAX_RT_BIT_CONFIG] = {FALSE};

  /* Counter for appplication ID */
  usAppIdx = 0;

  /* Connection counter */
  usI = 0;

  /* Get the number of slaves */
  if (boAddCheck)
  {
    /* Projected list already filled */
    usNumSlaves = prCSMD_Instance->rSlaveList.usNumProjSlaves;
  }
  else
  {
    /* No information about pojected slaves */
    usNumSlaves = (CSMD_USHORT)(prCSMD_Instance->rSlaveList.ausParserTempAddList[0] / 2);
  }

  /***************************************************/
  /* Search for connections which have to be cleared */
  /***************************************************/

  while(usI < usNumberOfConnections)
  {
    /* Init application ID array */
    for (usK = 0; usK < CSMD_NUMBER_OF_APPLICATIONS; usK++)
    {
      ausApplicationIDs[usK] = 0;
    }
    
    /* Check all connections from configuration table */
    /* No initialization, it was done some lines above */
    for (; usI < usNumberOfConnections; usI++)
    {
      /* Check all connection configurations */
      for (usK = 0; usK < CSMD_NUMBER_OF_APPLICATIONS; usK++)
      {
        if (CSMD_END_CONV_S(prConnections[usI].usApplicationID) == ausApplicationIDs[usK])
        {
          /* if found -> don't search further, break */
          break;
        }
      }
      
      /* Was AppID found? */
      if (usK == CSMD_NUMBER_OF_APPLICATIONS)
      {
        /* Not found? -> write to array with application IDs */
        ausApplicationIDs[usAppIdx++] = CSMD_END_CONV_S(prConnections[usI].usApplicationID);
      }
      
      /* Array full? */
      if (usAppIdx == CSMD_NUMBER_OF_APPLICATIONS)
      {
        /* Stop for processsing the found application IDs */
        break;
      }
    }
      
    /* Check all application IDs found so far */
    for (usK = 0; usK < usAppIdx; usK++)
    {
      for (usL = 0; usL < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn; usL++)
      {
        /* Is application ID inside configured configuration? */
        if ( ausApplicationIDs[usK] == prConfiguration->parConnection[usL].usApplicationID)
        { 
          /* Mark the connection for further processing */
          aucNeededConn[usL] = TRUE;
        }
      }
    }

    /* Reset Counter */
    usAppIdx = 0;

  } /* while(usI < usNumberOfConnections) */

  /***************************************************/
  /* Clear found connections                         */
  /***************************************************/
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn; usI++)
  {
    /* Treat this connection? */
    if (aucNeededConn[usI] == TRUE)
    {
      /* Clear */
      (CSMD_VOID) CSMD_HAL_memset( &prConfiguration->parConnection[usI],
                                   0,
                                   sizeof (CSMD_CONNECTION) );

      /* Check for master configurations which use this connection */
      for (usK = 0; usK < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usK++)
      {
        if (prConfiguration->prMaster_Config->parConnIdxList[usK].usConnIdx == usI)
        {
          /* Copy data to check configurations and RT bits later */
          aucConfig2Check[prConfiguration->prMaster_Config->parConnIdxList[usK].usConfigIdx] = TRUE;

          /* Rt bit configuration used? */
          if (prConfiguration->prMaster_Config->parConnIdxList[usK].usRTBitsIdx != 0xFFFF)
          {
            aucRTBits2Check[prConfiguration->prMaster_Config->parConnIdxList[usK].usRTBitsIdx] = TRUE;
          }

          /* reset the configuration information */
          prConfiguration->prMaster_Config->parConnIdxList[usK].usConnIdx = 0xFFFF;
          prConfiguration->prMaster_Config->parConnIdxList[usK].usConfigIdx = 0xFFFF;
          prConfiguration->prMaster_Config->parConnIdxList[usK].usRTBitsIdx = 0xFFFF;

          /* decrement number of connections */
          prConfiguration->prMaster_Config->usNbrOfConnections--;
        }
      }

      /* Check the slave connections which use this connection */
      for (usK = 0; usK < usNumSlaves; usK++)
      {
        for (usL = 0; usL < CSMD_MAX_CONNECTIONS; usL++)
        {
          if (prConfiguration->parSlave_Config[usK].arConnIdxList[usL].usConnIdx == usI)
          {
            /* Copy data to check configurations and RT bits later */
            aucConfig2Check[prConfiguration->parSlave_Config[usK].arConnIdxList[usL].usConfigIdx] = TRUE;
            
            /* Rt bit configuration used? */
            if (prConfiguration->parSlave_Config[usK].arConnIdxList[usL].usRTBitsIdx != 0xFFFF)
            {
              aucRTBits2Check[prConfiguration->parSlave_Config[usK].arConnIdxList[usL].usRTBitsIdx] = TRUE;
            }
            
            /* reset the configuration information */
            prConfiguration->parSlave_Config[usK].arConnIdxList[usL].usConnIdx = 0xFFFF;
            prConfiguration->parSlave_Config[usK].arConnIdxList[usL].usConfigIdx = 0xFFFF;
            prConfiguration->parSlave_Config[usK].arConnIdxList[usL].usRTBitsIdx = 0xFFFF;
            
            /* decrement number of connections */
            prConfiguration->parSlave_Config[usK].usNbrOfConnections--;
            
          }
        }
      }
    }
  }

  /***************************************************/
  /* Check and clear found configurations            */
  /***************************************************/
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig; usI++)
  {
    /* Configuration marked ? */
    if (aucConfig2Check[usI] == TRUE)
    {
      /* Is this connection still in use? */
      for (usK = 0; usK < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usK++)
      {
        /* Check master configurations */
        if (prConfiguration->prMaster_Config->parConnIdxList[usK].usConfigIdx == usI)
        {
          /* It is used */
          aucConfig2Check[usI] = FALSE;

          break;
        }
      }

      /* Not in use by the master? */
      if (aucConfig2Check[usI] == TRUE)
      {
        
        for (usK = 0; usK < usNumSlaves; usK++)
        {
          for (usL = 0; usL < CSMD_MAX_CONNECTIONS; usL++)
          {
            /* Check slave configurations */
            if (prConfiguration->parSlave_Config[usK].arConnIdxList[usL].usConfigIdx == usI)
            {
              /* It is used */
              aucConfig2Check[usI] = FALSE;
              
              break;
            }
          }
        }
      }

      /* Not in use anymore? */
      if (aucConfig2Check[usI] == TRUE)
      {
        /* Clear */
        (CSMD_VOID) CSMD_HAL_memset( &prConfiguration->parConfiguration[usI],
                                     0,
                                     sizeof(CSMD_CONFIGURATION) );
      }
    }
  }

  /***************************************************/
  /* Check and clear found RT Bit configurations     */
  /***************************************************/
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig; usI++)
  {
    /* Configuration marked ? */
    if (aucRTBits2Check[usI] == TRUE)
    {
      /* Is this connection still in use? */
      for (usK = 0; usK < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usK++)
      {
        /* Check master configurations */
        if (prConfiguration->prMaster_Config->parConnIdxList[usK].usRTBitsIdx == usI)
        {
          /* It is used */
          aucRTBits2Check[usI] = FALSE;

          break;
        }
      }

      /* Still in use by the master? */
      if (aucRTBits2Check[usI] == TRUE)
      {
        
        for (usK = 0; usK < usNumSlaves; usK++)
        {
          for (usL = 0; usL < CSMD_MAX_CONNECTIONS; usL++)
          {
            /* Check slave configurations */
            if (prConfiguration->parSlave_Config[usK].arConnIdxList[usL].usRTBitsIdx == usI)
            {
              /* It is used */
              aucRTBits2Check[usI] = FALSE;
              
              break;
            }
          }
        }
      }

      /* Not in use anymore? */
      if (aucRTBits2Check[usI] == TRUE)
      {
        /* Clear */
        (CSMD_VOID) CSMD_HAL_memset( &prConfiguration->parRealTimeBit[usI],
                                     0,
                                     sizeof (CSMD_REALTIME_BIT) );
      }
    }
  }

  return(eFuncRet);
} /* end CSMD_ClearAppID_Connections */

/**************************************************************************/ /**
\brief Clears connections with given Application ID.

\ingroup func_binconfig
\b Description: \n
   This function checks the parameter configuration table and clears all 
   parameter configurations which contain one of the application IDs given in 
   the new parameter configuration data.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prParamsList
              Pointer to the parameter list table within the configuration data
\param [in]   pusParamTablePtr
              Pointer to the parameter table

\return       \ref CSMD_NO_ERROR \n

\author       MSt
\date         13.12.2011

***************************************************************************** */
CSMD_FUNC_RET CSMD_ClearAppID_SetupParams( CSMD_INSTANCE                      *prCSMD_Instance,
                                           const CSMD_PARAMSLIST_TABLE_HEADER *prParamsList,
                                           const CSMD_USHORT                  *pusParamTablePtr )
{
  CSMD_CONFIG_STRUCT *prConfiguration = &prCSMD_Instance->rConfiguration;
  const CSMD_PARAMSLIST_TABLE_HEADER *prAuxParameterListPtr = prParamsList;
  CSMD_FUNC_RET       eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT         usAppIdx;
  CSMD_USHORT         usI, usK, usL;
  CSMD_USHORT         ausApplicationIDs[CSMD_NUMBER_OF_APPLICATIONS];
  CSMD_UCHAR          aucNeededParamsList[CSMD_MAX_CONFIGPARAMS_LIST] = {FALSE};
  CSMD_UCHAR          aucParam2Check[CSMD_MAX_CONFIG_PARAMETER]       = {FALSE};

  /* Counter for appplication ID */
  usAppIdx = 0;

  /*******************************************************/
  /* Search for parameter lists which have to be cleared */
  /*******************************************************/

  while (  ((CSMD_UCHAR *)prAuxParameterListPtr)
         < ((CSMD_UCHAR *)pusParamTablePtr - CSMD_END_SIGN_LENGTH))
  {
    /* Init application ID array */
    for (usK = 0; usK < CSMD_NUMBER_OF_APPLICATIONS; usK++)
    {
      ausApplicationIDs[usK] = 0;
    }
    
    /* Check all setup parameter lists from configuration table */
    /* This while() seems to be duplicate, but is intentional */
    while (  ((CSMD_UCHAR *)prAuxParameterListPtr)
           < ((CSMD_UCHAR *)pusParamTablePtr - CSMD_END_SIGN_LENGTH))
    {
      
      /* Check all setup parameter lists */
      for (usK = 0; usK < CSMD_NUMBER_OF_APPLICATIONS; usK++)
      {
        if (CSMD_END_CONV_S(prAuxParameterListPtr->usParamsApplicationID) == ausApplicationIDs[usK])
        {
          /* if found -> don't search further, break */
          break;
        }
      }
      
      /* Was AppID found? */
      if (usK == CSMD_NUMBER_OF_APPLICATIONS)
      {
        /* Not found? -> write to array with application IDs */
        ausApplicationIDs[usAppIdx++] = CSMD_END_CONV_S(prAuxParameterListPtr->usParamsApplicationID);
      }
      
      /* Array full? */
      if (usAppIdx == CSMD_NUMBER_OF_APPLICATIONS)
      {
        /* Stop for processsing the found application IDs */
        break;
      }
      else
      {
        CSMD_USHORT  usLength = (CSMD_USHORT)(3 + prAuxParameterListPtr->usNumberOfConfigParams);
        CSMD_USHORT *pusTempPtr = (CSMD_USHORT *)(CSMD_VOID *)prAuxParameterListPtr;
        
        if (usLength & 1U)
        {
          usLength++;
        }
        
        prAuxParameterListPtr = (CSMD_PARAMSLIST_TABLE_HEADER *)(CSMD_VOID *)(CSMD_USHORT *)(pusTempPtr + usLength); 
      }
    } /* while(((CSMD_USHORT)prAuxParameterListPtr) < ((CSMD_USHORT)pusParamTablePtr)) */
      
    /* Check all application IDs found so far */
    for (usK = 0; usK < usAppIdx; usK++)
    {
      for (usL = 0; usL < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList; usL++)
      {
        /* Is application ID inside configured configuration? */
        if ( ausApplicationIDs[usK] == prConfiguration->parConfigParamsList[usL].usApplicationID)
        { 
          /* Mark the connection for further processing */
          aucNeededParamsList[usL] = TRUE;
        }
      }
    }

    /* Reset Counter */
    usAppIdx = 0;

  } /* while(((CSMD_USHORT)prAuxParameterListPtr) < ((CSMD_USHORT)pusParamTablePtr)) */

  /***************************************************/
  /* Clear found parameter lists                     */
  /***************************************************/
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList; usI++)
  {
    /* Treat this parameter list */
    if (aucNeededParamsList[usI] == TRUE)
    {
      /* Mark used parameters */
      for (usK = 0; usK < CSMD_MAX_PARAMS_IN_CONFIG_LIST; usK++)
      {
        CSMD_USHORT *pusConfigParamIdx = &prConfiguration->parConfigParamsList[usI].ausParamTableIndex[usK];

        /* Is the parameter index used? */
        if (*pusConfigParamIdx != 0xFFFF)
        {
          /* Mark the parameter */
          aucParam2Check[*pusConfigParamIdx] = TRUE;

          /* Clear the referenced index */
          *pusConfigParamIdx = 0xFFFF;
        }
      }

      /* Clear the application ID */
      prConfiguration->parConfigParamsList[usI].usApplicationID = 0;

      /* Clear the list reference in the slave configuration */
      for (usK = 0; usK < prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaveConfigParams; usK++)
      {
        /* Is this parameter list used? */
        if (prConfiguration->parSlaveParamConfig[usK].usConfigParamsList_Index == usI)
        {
          /* Is this parameter list the first one of the slave? */
          if (prConfiguration->parSlaveParamConfig[usK].usSlaveIndex == prCSMD_Instance->rPriv.rSystemLimits.usMaxSlaves)
          {
            /* The master */
            if (prConfiguration->prMaster_Config->usFirstConfigParamIndex == usK)
            {
              /* Clear the index of first configuration parameter */
              prConfiguration->prMaster_Config->usFirstConfigParamIndex = 0xFFFF;
            }
          }
          else
          {
            /* A slave */
            if (prConfiguration->parSlave_Config[ prConfiguration->parSlaveParamConfig[usK].usSlaveIndex ].usFirstConfigParamIndex == usK)
            {
              /* Clear the index of first configuration parameter */
              prConfiguration->parSlave_Config[ prConfiguration->parSlaveParamConfig[usK].usSlaveIndex ].usFirstConfigParamIndex = 0xFFFF;
            }
          }

          /* Clear */
          prConfiguration->parSlaveParamConfig[usK].usConfigParamsList_Index = 0xFFFF;
          prConfiguration->parSlaveParamConfig[usK].usSlaveIndex = 0xFFFF;
        }
      }
    }
  }

  /* Check used parameters to be used somewhere else */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter; usI++)
  {
    if (aucParam2Check[usI] == TRUE)
    {
      for (usK = 0; usK < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList; usK++)
      {
        if (aucNeededParamsList[usK] != TRUE)
        {
          /* is the parameter list active? */
          if (prConfiguration->parConfigParamsList[usK].usApplicationID != 0)
          {
            /* check all used parameters */
            for (usL = 0; usL < CSMD_MAX_PARAMS_IN_CONFIG_LIST; usL++)
            {
              CSMD_USHORT usIndex = prConfiguration->parConfigParamsList[usK].ausParamTableIndex[usL];

              /* Is the parameter marked? */
              if (usIndex == aucParam2Check[usI])
              {
                /* Clear the mark */
                aucParam2Check[usI] = FALSE;

                /* Leave loop (for (usL ...)*/
                break;
              }
            }
          }
        }

        /* Leave the loop if parameter is further needed */
        if (aucParam2Check[usI] == FALSE)
        {
          break;
        }
      }
    }

    /* Is the parameter still marked? -> clear! */
    if (aucParam2Check[usI] == TRUE)
    {
      /* Set all data to 0 */
      (CSMD_VOID) CSMD_HAL_memset( &prConfiguration->parConfigParam[usI],
                                   0,
                                   sizeof(CSMD_CONFIGURATION_PARAMETER) );
    }
  }

  return(eFuncRet);
} /* end CSMD_ClearAppID_SetupParams */

/**************************************************************************/ /**
\brief Checks the keys inside the connection table.

\ingroup func_binconfig
\b Description: \n
   This function checks the producer and consumer keys inside the connection
   table. If a key is not present, an error will occur.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prTableHeaderPtr
              Pointer to a list of table headers
\param [in]   usNumberOfConnections
              Number of table entries in connections table
\param [in]   usNumberOfProducers
              Number of table entries in producer table

\return       \ref CSMD_NO_PRODUCER_KEY \n
              \ref CSMD_NO_CONSUMER_LIST_KEY \n
              \ref CSMD_NO_ERROR \n

\author       MSt
\date         11.01.2011

***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckConnTableKeys( const CSMD_TABLE_POINTER *prTableHeaderPtr,
                                       CSMD_USHORT               usNumberOfConnections,
                                       CSMD_USHORT               usNumberOfProducers )
{
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT   usI, usK;
  
  /* Check connection table */
  for (usI = 0; ((usI < usNumberOfConnections) && (eFuncRet == CSMD_NO_ERROR)); usI++)
  {
    /* get the actual producer */
    CSMD_CONNECTION_TABLE *prConnTable;
    CSMD_USHORT            usProdKey;
    CSMD_USHORT            usConsListKey;
    CSMD_USHORT           *pusAuxPtr;
    
    /* get the actual connection table element */
    prConnTable = (CSMD_CONNECTION_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)prTableHeaderPtr->pusCnncStartPtr)
      + CSMD_TABLE_HEADER_LEN + (usI * CSMD_CONNECTION_TABLE_LEN));
    
    /* Get keys */
    usProdKey = prConnTable->usProducerKey;
    usConsListKey = prConnTable->usConsumerListKey;

    /* Check for producer key 0 */
    if (usProdKey == 0U)
    {
      /* producer key not valid */
      eFuncRet = CSMD_NO_PRODUCER_KEY;
    }

    /* Check for consumer list key 0 */
    if (usConsListKey == 0U)
    {
      /* consumer list key not valid */
      eFuncRet = CSMD_NO_CONSUMER_LIST_KEY;
    }

    /* Check producer table */
    for (usK = 0; usK < usNumberOfProducers; usK++)
    {
      /* get the actual producer */
      CSMD_PRODUCER_TABLE *prProdTable;
      
      /* get the actual producer table element */
      prProdTable = (CSMD_PRODUCER_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)prTableHeaderPtr->pusPrdcStartPtr)
        + CSMD_TABLE_HEADER_LEN + (usK * CSMD_PRODUCER_TABLE_LEN));
      
      /* Address 0 is the master should be OK */
      if (prProdTable->usProducerKey == usProdKey)
      {
        /* producer key exists -> check the consumer list as next step */
        break;
      }
    }

    /* producer key not found? */
    if (usK == usNumberOfProducers)
    {
      /* producer key not available */
      eFuncRet = CSMD_NO_PRODUCER_KEY;
    }
    
    /* set the help pointer to the beginning of the consumer list data,
    behind the table header; needed because structure of consumer list 
    table is dynamic */
    pusAuxPtr = prTableHeaderPtr->pusConsLStartPtr + 2;
    
    /* Check consumer list table */
    /* For loop for all consumer list elements */
    for (usK = 0; (usK < usNumberOfConnections) && (eFuncRet == CSMD_NO_ERROR); usK++)
    {
      CSMD_CONSLIST_TABLE_HEADER *prConsListTable;
      
      /* get the actual consumer table element */
      prConsListTable = (CSMD_CONSLIST_TABLE_HEADER *)(CSMD_VOID *)pusAuxPtr;
      
      /* Check if the consumer list key is the right one */
      if (prConsListTable->usConsumerListKey == usConsListKey)
      {
        /* consumer list key exists -> check the consumer list as next step */
        break;
      }
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
    }

    /* consumer list key not found? */
    if (usK == usNumberOfConnections)
    {
      /* consumer list key not available */
      eFuncRet = CSMD_NO_CONSUMER_LIST_KEY;
    }
  }

  return(eFuncRet);
} /* end CSMD_CheckConnTableKeys */


/**************************************************************************/ /**
\brief Checks the keys inside the connection table.

\ingroup func_binconfig
\b Description: \n
   This function checks the producer and consumer keys inside the connection
   table. If a key is not present, an error will occur.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prTableHeaderPtr
              Pointer to a list of table headers

\return       \ref CSMD_NO_SETUP_PARAMETER_KEY \n
              \ref CSMD_NO_SETUP_LIST_KEY \n
              \ref CSMD_NO_ERROR \n

\author       MSt
\date         12.02.2011

***************************************************************************** */
CSMD_FUNC_RET CSMD_CheckParamConfTableKeys( const CSMD_TABLE_POINTER *prTableHeaderPtr )
{
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT   usNumberOfSlaveSetupTables;
  CSMD_USHORT   usI;
  
  usNumberOfSlaveSetupTables = (CSMD_USHORT)
    (  ((CSMD_ULONG)(  (CSMD_UCHAR *)prTableHeaderPtr->pusSetupParametersListStartPtr
                     - (CSMD_UCHAR *)prTableHeaderPtr->pusSlaveSetupStartPtr) - CSMD_TABLE_OVERHEAD)
     / CSMD_SLAVE_SETUP_TABLE_LEN);
  
  for (usI = 0; (usI < usNumberOfSlaveSetupTables) && (eFuncRet == CSMD_NO_ERROR); usI++)
  {
    CSMD_SLAVE_SETUP_TABLE       *prSlaveSetupTable;
    CSMD_USHORT                   usSlaveSetupKey;
    CSMD_BOOL                     boSetupListFound = FALSE;
    CSMD_PARAMSLIST_TABLE_HEADER *prAuxParamsListPtr;
    
    /* get the actual slave setup table element */
    prSlaveSetupTable = (CSMD_SLAVE_SETUP_TABLE *)(CSMD_VOID *)(((CSMD_CHAR *)prTableHeaderPtr->pusSlaveSetupStartPtr)
      + CSMD_TABLE_HEADER_LEN + (usI * CSMD_SLAVE_SETUP_TABLE_LEN));
    
    /* Get the setup list key */
    usSlaveSetupKey = CSMD_END_CONV_S(prSlaveSetupTable->usSetupParamsListKey);

    /* Get the pointer to the setup lists table */
    prAuxParamsListPtr = (CSMD_PARAMSLIST_TABLE_HEADER *)
                (CSMD_VOID *)(prTableHeaderPtr->pusSetupParametersListStartPtr + 2);

    /* Search the setup list */
    while (  (boSetupListFound == FALSE)
           && (   (CSMD_UCHAR *)prAuxParamsListPtr
               < ((CSMD_UCHAR *)prTableHeaderPtr->pusSetupParameterStartPtr - CSMD_END_SIGN_LENGTH)))
    {
      CSMD_USHORT *pusActParamListKey = &prAuxParamsListPtr->usSetupParamsListKey;

      if (CSMD_END_CONV_S(*pusActParamListKey) == usSlaveSetupKey)
      {
        /* Setup parameter list key was found */
        CSMD_USHORT  usNumberOfConfParams = CSMD_END_CONV_S(prAuxParamsListPtr->usNumberOfConfigParams);
        CSMD_USHORT  usK;
        
        boSetupListFound = TRUE;

        /* Check all parameters in the list */
        for (usK = 0; (usK < usNumberOfConfParams) && (eFuncRet == CSMD_NO_ERROR); usK++)
        {
          CSMD_BOOL    boSetupParameterFound = FALSE;
          CSMD_USHORT *pusActParamKey =
            (CSMD_USHORT *)((&prAuxParamsListPtr->usFirstParameterKey) + usK);
          CSMD_PARAMETER_TABLE_HEADER *prAuxParamPtr =
            (CSMD_PARAMETER_TABLE_HEADER *)(CSMD_VOID *)(prTableHeaderPtr->pusSetupParameterStartPtr + 2);
          
          /* Search the parameter key inside the table */
          while (   (boSetupParameterFound == FALSE)
                 &&  (  (CSMD_UCHAR *)prAuxParamPtr
                     < ((CSMD_UCHAR *)prTableHeaderPtr->pusConnConfigEndPtr - CSMD_END_SIGN_LENGTH)))
          {
            /* Compare the keys */
            /* Both values in same endianness, so no endian swapping necessary */
            if (prAuxParamPtr->usParameterKey == *pusActParamKey)
            {
              /* Setup parameter key was found */
              boSetupParameterFound = TRUE;
            }
            else
            {
              /* Setup parameter key was not found -> go on with the search */
              CSMD_USHORT  usLength = (CSMD_USHORT)(8 + CSMD_END_CONV_S(prAuxParamPtr->usDataLength));
              CSMD_USHORT *pusTempPtr = (CSMD_USHORT *)(CSMD_VOID *)prAuxParamPtr;
              
              /* Check for SHORT alignment */
              if (usLength & 0x1U)
              {
                usLength++;
              }
              
              /* Check for LONG alignment */
              if (usLength & 0x2U)
              {
                usLength += 2;
              }
              
              /* Convert length to number of SHORT */
              usLength /= 2;
              
              prAuxParamPtr = (CSMD_PARAMETER_TABLE_HEADER *)(CSMD_VOID *)(pusTempPtr + usLength);
            }
          }

          /* Setup parameter key not found? */
          if (!(   (CSMD_UCHAR *)prAuxParamPtr
                < ((CSMD_UCHAR *)prTableHeaderPtr->pusConnConfigEndPtr - CSMD_END_SIGN_LENGTH)))
          {
            /* setup parameter key not available */
            eFuncRet = CSMD_NO_SETUP_PARAMETER_KEY;
          }
        }
        
      }
      else
      {
        /* Setup parameter list key was not found -> go on with the search */
        CSMD_USHORT  usLength = (CSMD_USHORT)(3 + CSMD_END_CONV_S(prAuxParamsListPtr->usNumberOfConfigParams));
        CSMD_USHORT *pusTempPtr = (CSMD_USHORT *)(CSMD_VOID *)prAuxParamsListPtr;
        
        if (usLength & 0x1U)
        {
          usLength++;
        }
        
        prAuxParamsListPtr = (CSMD_PARAMSLIST_TABLE_HEADER *)(CSMD_VOID *)(pusTempPtr + usLength);
      }
    }

    /* Setup parameter list key not found? */
    if (!(   (CSMD_UCHAR *)prAuxParamsListPtr
          < ((CSMD_UCHAR *)prTableHeaderPtr->pusSetupParameterStartPtr - CSMD_END_SIGN_LENGTH)))
    {
      /* setup list key not available */
      eFuncRet = CSMD_NO_SETUP_LIST_KEY;
    }
  }

  return(eFuncRet);
} /* end CSMD_CheckParamConfTableKeys */


/**************************************************************************/ /**
\brief Checks the keys inside the connection table.

\ingroup func_binconfig
\b Description: \n
   This function checks which connections, configurations and real-time bit 
   configurations are currently in use in order to determine which elements
   may still be written in the configuration structure.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   prUsedMarker
              Pointer to a structure indicating which configuration
              elements are already in use
\param [in]   boAddCheck
              Flag which determines whether the slave addresses from the
              slave list or the temporary addressews have to be used

\return       \ref CSMD_NO_ERROR \n

\author       MSt
\date         21.04.2011

***************************************************************************** */
/*lint -save -e818 Pointer parameter could be declared ptr to const */
CSMD_FUNC_RET CSMD_GetUsedCfgStructs( const CSMD_INSTANCE *prCSMD_Instance,
                                      CSMD_USED_MARKER    *prUsedMarker,
                                      CSMD_BOOL            boAddCheck )
{
  CSMD_FUNC_RET eFuncRet = CSMD_NO_ERROR;
  CSMD_USHORT   usNumSlaves;
  CSMD_USHORT   usI, usK;
  const CSMD_CONFIG_STRUCT *prConfig = &prCSMD_Instance->rConfiguration;

  /* Get the number of slaves */
  if (boAddCheck)
  {
    /* Projected list already filled */
    usNumSlaves = prCSMD_Instance->rSlaveList.usNumProjSlaves;
  }
  else
  {
    /* No information about projected slaves */
    usNumSlaves = (CSMD_USHORT)(prCSMD_Instance->rSlaveList.ausParserTempAddList[0] / 2);
  }

  /* Initialize the structures */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usI++)
  {
    prUsedMarker->paucConnUsed[usI] = FALSE;
    prUsedMarker->paucConnNbrUsed[usI] = FALSE;
  }

  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConfig; usI++)
  {
    prUsedMarker->paucConfUsed[usI] = FALSE;
  }

  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxRtBitConfig; usI++)
  {
    prUsedMarker->paucRTBtUsed[usI] = FALSE;
  }

  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList; usI++)
  {
    prUsedMarker->paucSetupParamsListUsed[usI] = FALSE;
  }

  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter; usI++)
  {
    prUsedMarker->paucSetupParamsUsed[usI] = FALSE;
  }

  /* Check all master connections */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConnMaster; usI++)
  {
    CSMD_USHORT  usTempConnIndex = 
      prConfig->prMaster_Config->parConnIdxList[usI].usConnIdx;
    CSMD_USHORT  usTempConfIndex = 
      prConfig->prMaster_Config->parConnIdxList[usI].usConfigIdx;
    CSMD_USHORT  usTempRTBConfIndex = 
      prConfig->prMaster_Config->parConnIdxList[usI].usRTBitsIdx;

    /* Is the instance in use? */
    if (usTempConnIndex != 0xFFFF)
    {
      CSMD_USHORT  usTempConnNbr = 
        prConfig->parConnection[usTempConnIndex].usS_0_1050_SE2;

      /* Mark the connection as used */
      prUsedMarker->paucConnUsed[usTempConnIndex] = TRUE;

      /* Is the connection number in the range for automatic numbering */
      if (   (usTempConnNbr >= 1)
          && (usTempConnNbr <= prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn))
      {
        /* Mark the connection number as used */
        prUsedMarker->paucConnNbrUsed[usTempConnNbr - 1] = TRUE;
      }

      /* Mark the configuration index as well as used, because if the 
         connection is used a configuration is also used */
      prUsedMarker->paucConfUsed[usTempConfIndex] = TRUE;

      /* Check if a RT bits configuration is used */
      if (usTempRTBConfIndex != 0xFFFF)
      {
        /* Mark the RT bits configuration as used */
        prUsedMarker->paucRTBtUsed[usTempRTBConfIndex] = TRUE;
      }

    }
  }

  /* Check all slave connections */
  for (usI = 0; usI < usNumSlaves; usI++)
  {
    for (usK = 0; usK < CSMD_MAX_CONNECTIONS; usK++)
    {
      CSMD_USHORT  usTempConnIndex = 
        prConfig->parSlave_Config[usI].arConnIdxList[usK].usConnIdx;
      CSMD_USHORT  usTempConfIndex = 
        prConfig->parSlave_Config[usI].arConnIdxList[usK].usConfigIdx;
      CSMD_USHORT  usTempRTBConfIndex = 
        prConfig->parSlave_Config[usI].arConnIdxList[usK].usRTBitsIdx;
      
      /* Is the instance in use? */
      if (usTempConnIndex != 0xFFFF)
      {
        CSMD_USHORT  usTempConnNbr = 
          prConfig->parConnection[usTempConnIndex].usS_0_1050_SE2;
        
        /* Mark the connection as used */
        prUsedMarker->paucConnUsed[usTempConnIndex] = TRUE;
        
        /* Is the connection number in the range for automatic numbering */
        if (   (usTempConnNbr >= 1)
            && (usTempConnNbr <= prCSMD_Instance->rPriv.rSystemLimits.usMaxGlobConn))
        {
          /* Mark the connection number as used */
          prUsedMarker->paucConnNbrUsed[usTempConnNbr - 1] = TRUE;
        }
        
        /* Mark the configuration index as well as used, because if the 
        connection is used a configuration is also used */
        prUsedMarker->paucConfUsed[usTempConfIndex] = TRUE;
        
        /* Check if a RT bits configuration is used */
        if (usTempRTBConfIndex != 0xFFFF)
        {
          /* Mark the RT bits configuration as used */
          prUsedMarker->paucRTBtUsed[usTempRTBConfIndex] = TRUE;
        }
      }
    }
  }

  /* Check the setup parameters list */
  for (usI = 0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParamsList; usI++)
  {
    if (prConfig->parConfigParamsList[usI].usApplicationID != 0)
    {
      prUsedMarker->paucSetupParamsListUsed[usI] = TRUE;
    }
  }
  
  /* Check the setup parameters */
  for (usI =  0; usI < prCSMD_Instance->rPriv.rSystemLimits.usMaxConfigParameter; usI++)
  {
    if (prConfig->parConfigParam[usI].usDataLength != 0)
    {
      prUsedMarker->paucSetupParamsUsed[usI] = TRUE;
    }
  }

  return(eFuncRet);

} /* end CSMD_GetUsedCfgStructs */
/*lint -restore const! */


/**************************************************************************/ /**
\brief Writes the headers of the tables.

\ingroup func_binconfig
\b Description: \n
   This function writes the header belonging to the given data to the 
   generated binary configuration.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   ppusActListPos
              Pointer to a pointer with the present position in the list
              for the binary configuration
\param [in]   pusActLength
              Pointer to present length of the binary configuration
\param [in]   eHeaderInfo
              Gives information for the string which is written into the 
              binary configuration

\return       none

\author       MSt
\date         06.12.2012

***************************************************************************** */
CSMD_VOID CSMD_Write_Header( CSMD_USHORT      **ppusActListPos,
                             CSMD_USHORT       *pusActLength,
                             CSMD_HEADER_ENUM   eHeaderInfo )
{
  CSMD_UCHAR *pucCharHelpPointer = (CSMD_UCHAR *)(*ppusActListPos);

  /* At first write the table end sign  */
  *pucCharHelpPointer++ = 0x7E; /* "~" */
  *pucCharHelpPointer++ = 0x5E; /* "^" */
  *pucCharHelpPointer++ = 0x7E; /* "~" */
  *pucCharHelpPointer++ = 0x5E; /* "^" */
  
  switch(eHeaderInfo)
  {
  case CSMD_PROD_HEADER:
    /* set header of producer table to "Prdc" */
    *pucCharHelpPointer++ = 0x50; /* "P" */
    *pucCharHelpPointer++ = 0x72; /* "r" */
    *pucCharHelpPointer++ = 0x64; /* "d" */
    *pucCharHelpPointer++ = 0x63; /* "c" */
    
    break;

  case CSMD_CONSLIST_HEADER:
    /* set header of consumer list table to "CnLs" */
    *pucCharHelpPointer++ = 0x43; /* "C" */
    *pucCharHelpPointer++ = 0x6E; /* "n" */
    *pucCharHelpPointer++ = 0x4C; /* "L" */
    *pucCharHelpPointer++ = 0x73; /* "s" */

    break;
    
  case CSMD_CONSUMER_HEADER:
    /* set header of consumer table to "Cnsm" */
    *pucCharHelpPointer++ = 0x43; /* "C" */
    *pucCharHelpPointer++ = 0x6E; /* "n" */
    *pucCharHelpPointer++ = 0x73; /* "s" */
    *pucCharHelpPointer++ = 0x6D; /* "m" */

    break;
    
  case CSMD_CONFIG_HEADER:
    /* set header of configuration table to "Cnfg" */
    *pucCharHelpPointer++ = 0x43; /* "C" */
    *pucCharHelpPointer++ = 0x6E; /* "n" */
    *pucCharHelpPointer++ = 0x66; /* "f" */
    *pucCharHelpPointer++ = 0x67; /* "g" */

    break;
    
  case CSMD_RTBITS_HEADER:
    /* set header of real time bit table to "RTBt" */
    *pucCharHelpPointer++ = 0x52; /* "R" */
    *pucCharHelpPointer++ = 0x54; /* "T" */
    *pucCharHelpPointer++ = 0x42; /* "B" */
    *pucCharHelpPointer++ = 0x74; /* "t" */

    break;
    
  case CSMD_SLAVESETUP_HEADER:
    /* set header of slave setup table to "SlSt" */
    *pucCharHelpPointer++ = 0x53; /* "S" */
    *pucCharHelpPointer++ = 0x6C; /* "l" */
    *pucCharHelpPointer++ = 0x53; /* "S" */
    *pucCharHelpPointer++ = 0x74; /* "t" */

    break;
    
  case CSMD_PARALIST_HEADER:
    /* set header of parameter list table to "StPL" */
    *pucCharHelpPointer++ = 0x53; /* "S" */
    *pucCharHelpPointer++ = 0x74; /* "t" */
    *pucCharHelpPointer++ = 0x50; /* "P" */
    *pucCharHelpPointer++ = 0x4C; /* "L" */

    break;
    
  case CSMD_PARAMS_HEADER:
    /* set header of parameters table to "Prmt" */
    *pucCharHelpPointer++ = 0x50; /* "P" */
    *pucCharHelpPointer++ = 0x72; /* "r" */
    *pucCharHelpPointer++ = 0x6D; /* "m" */
    *pucCharHelpPointer++ = 0x74; /* "t" */

    break;

  case CSMD_FILE_END:
    /* Write file end sign */
    *pucCharHelpPointer++ = 0x65; /* "e" */
    *pucCharHelpPointer++ = 0x4E; /* "N" */
    *pucCharHelpPointer++ = 0x44; /* "D" */
    *pucCharHelpPointer++ = 0x45; /* "E" */
    
    break;

  case CSMD_CONN_HEADER:
  default:
    break;
  }

  /* set the list pointer to the next position */
  *ppusActListPos = (CSMD_USHORT *)(CSMD_VOID *)pucCharHelpPointer;
  
  /* Adjust list length */
  *pusActLength += (CSMD_END_SIGN_LENGTH + CSMD_TABLE_HEADER_LEN);
  
  return;
} /* end CSMD_Write_Header */

/*! \endcond */ /* PRIVATE */

#endif  /* #ifdef CSMD_CONFIG_PARSER */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

14 Nov 2014 WK
  - File created. Shifted private function to this file.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
28 Apr 2015 WK
  - Defdb00178597
    Fixed type castings for 64-bit systems.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
  
------------------------------------------------------------------------------
*/
