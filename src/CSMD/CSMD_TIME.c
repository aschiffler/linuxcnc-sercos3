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
\file   CSMD_TIME.c
\author WK
\date   01.09.2010
\brief  This File contains the public API functions for
        the Sercos time feature.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"


/*---- Definition public Functions: ------------------------------------------*/

/*! \cond PUBLIC */

/**************************************************************************/ /**
\brief Calculates and transmits the new Sercos time.

\ingroup func_sercostime
\b Description: \n
   This function gets the new Sercos time in seconds and calculates Sercos time
   to be inserted into Extended Function Field in nanoseconds.

<B>Call Environment:</B> \n
   This function should be called either from an interrupt or a task with
   high priority.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   rSercosTime
              New Sercos time chosen by master

\return       \ref CSMD_NO_ERROR \n
  
\author       AM
\date         22.06.2010

***************************************************************************** */
CSMD_FUNC_RET CSMD_New_Sercos_Time( CSMD_INSTANCE   *prCSMD_Instance,
                                    CSMD_SERCOSTIME  rSercosTime )
{
  
  CSMD_ULONG   ulT_forecast;
  CSMD_ULONG   ulNanos_bus;
  CSMD_ULONG   ulSecs_bus;
  
  ulT_forecast =   8 * prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002 
                 + prCSMD_Instance->rPriv.ulOffsetTNCT_SERCCycle
                 + prCSMD_Instance->rPriv.rRingDelay.ulTSref;
  
  ulNanos_bus = rSercosTime.ulNanos + ulT_forecast;
  ulSecs_bus  = rSercosTime.ulSeconds;
  
  if (rSercosTime.ulNanos + ulT_forecast >= 1000000000) /* 10^9 */
  {
    ulNanos_bus -= 1000000000; /* 10^9 */
    ulSecs_bus  += 1;
  }
  
  CSMD_HAL_SetSercosTime( &prCSMD_Instance->rCSMD_HAL,
                          rSercosTime.ulNanos,
                          rSercosTime.ulSeconds,
                          ulNanos_bus,
                          ulSecs_bus );
  
  return (CSMD_NO_ERROR);

} /* end: CSMD_New_Sercos_Time() */


/**************************************************************************/ /**
\brief Calculates and set a new demanded Sercos time.

\ingroup module_sercostime
\b Description: \n
   This function ...
   This is a modified method to set the Sercos time.
   The set Sercos time is tracked by the IP-Core and assumed by an
   external sync signal.
   Sercos IP-Code > 4.9 is required!


<B>Call Environment:</B> \n
   This function should be called either from an interrupt or a task with
   high priority.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the
              CoSeMa instance
\param [in]   rSercosTime
              New Sercos time chosen by master

\return       \ref CSMD_NO_ERROR \n

\author       WK
\date         22.10.2014

***************************************************************************** */
/*lint -save -e818 const! */
CSMD_FUNC_RET CSMD_New_Sercos_Time_ExtSync( CSMD_INSTANCE   *prCSMD_Instance,
                                            CSMD_SERCOSTIME  rSercosTime )
{
  CSMD_ULONG   ulT_forecast;

  ulT_forecast =   8 * prCSMD_Instance->rConfiguration.rComTiming.ulCommCycleTime_S01002
                 + prCSMD_Instance->rPriv.ulOffsetTNCT_SERCCycle
                 + prCSMD_Instance->rPriv.rRingDelay.ulTSref;

  CSMD_HAL_SetSercosTimeExtSync( &prCSMD_Instance->rCSMD_HAL,
                                 rSercosTime.ulNanos,
                                 rSercosTime.ulSeconds,
                                 ulT_forecast );
  return (CSMD_NO_ERROR);

} /* end: CSMD_New_Sercos_Time_ExtSync() */
/*lint -restore const! */


/**************************************************************************/ /**
\brief Writes the current Sercos time to a defined address.

\ingroup func_sercostime
\b Description: \n
   This function returns the current Sercos time in seconds as well as its
   post decimal positions in nanoseconds.
   
<B>Call Environment:</B> \n
   This function can be called either from an interrupt or from a task.
               
\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [out]  prSercosTime
              Output pointer to Sercos time structure

\return       \ref CSMD_NO_ERROR \n

\author       AM
\date         28.06.2010

***************************************************************************** */
CSMD_FUNC_RET CSMD_Get_Sercos_Time( CSMD_INSTANCE   *prCSMD_Instance,
                                    CSMD_SERCOSTIME *prSercosTime )
{
  
  CSMD_HAL_ReadSercosTime( &prCSMD_Instance->rCSMD_HAL,
                           &prSercosTime->ulNanos,
                           &prSercosTime->ulSeconds );
  
  return (CSMD_NO_ERROR);
  
} /* end: CSMD_Get_Sercos_Time() */

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
23 Oct 2014 WK
  - Defdb00174315
    Added new function CSMD_New_Sercos_Time_ExtSync()
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
