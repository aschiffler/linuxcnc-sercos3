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
\file   CSMD_PHASEDEV.h
\author WK
\date   03.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_PHASEDEV.c
*/


#ifndef _CSMD_PHASEDEV
#define _CSMD_PHASEDEV

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif


/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! \cond PRIVATE */

/* -------------------------------------------------------------------------- */
/* Enum type used for accessing Sercos parameter lists                        */
/* -------------------------------------------------------------------------- */
typedef enum
{
  eCSMD_IdnListActLength        = 0,      /*!< - current list length (bytes)  */
  eCSMD_IdnListMaxLength        = 1,      /*!< - max. list length (bytes)     */
  eCSMD_IdnListDataStart        = 2       /*!< - start of data                */
  
} CSMD_IDNLIST_WORD;


/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

SOURCE CSMD_FUNC_RET CSMD_GetHW_Settings
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boSetDefault );

/* Address detection at CP0 */
SOURCE CSMD_BOOL CSMD_GetSlaveAddresses
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Read the Sercos addresses from selected port */
SOURCE CSMD_BOOL CSMD_BuildAvailableSlaveList
                                ( CSMD_ADDR_SCAN_INFO       *prPrevious,
                                  const CSMD_USHORT         *pusCurrent );

/* Delete all slave lists before scan */
SOURCE CSMD_VOID CSMD_DeleteSlaveList
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Scan for all available slaves and build recognized slaves list */
SOURCE CSMD_FUNC_RET CSMD_Detect_Available_Slaves
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Build a Sercos list with Sercos addresses of all recognized slaves in topological order. */
SOURCE CSMD_FUNC_RET CSMD_Build_Recog_Slave_AddList
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Checks current topology and number of recognized slaves at the end of setting CP0. */
SOURCE CSMD_FUNC_RET CSMD_Check_Recognized_Topology_CP0
                                ( const CSMD_INSTANCE       *prCSMD_Instance );

#ifdef CSMD_SWC_EXT
SOURCE CSMD_FUNC_RET CSMD_Check_Slave_Acknowledgment
                                ( CSMD_INSTANCE             *prCSMD_Instance );
#endif

/* Check for multiple Sercos addresses in the recognized slaves address list */
SOURCE CSMD_FUNC_RET CSMD_Check_Slave_Addresses
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* scans the existing drives */
SOURCE CSMD_FUNC_RET CSMD_ScanOperDrives
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  const CSMD_USHORT         *pusProjectedSlaveList );

SOURCE CSMD_BOOL CSMD_Telegram_Fail_CP0
                                ( const CSMD_INSTANCE       *prCSMD_Instance );

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_PHASEDEV */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

03 Sep 2010
  - File created.
27 Nov 2013 WK
  - Devdb00000000
    Added prototype CSMD_GetHW_Settings().
12 Sep 2014 WK
  - Defdb00150926
    Fixed lint error 18 caused by missing "SOURCE".
24 Nov 2014 WK
  - Changed prototype of function CSMD_GetHW_Settings().
02 Dec 2014 WK
  - Added prototype of function CSMD_BuildAvailableSlaveList().
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
28 Jan 2016 WK
  - Defdb00182067
    CSMD_BuildAvailableSlaveList(): Static declaration removed.

------------------------------------------------------------------------------
*/
