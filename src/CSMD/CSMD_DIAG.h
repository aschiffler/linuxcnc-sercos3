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
\file   CSMD_DIAG.h
\author WK
\date   03.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_DIAG.c
*/


#ifndef _CSMD_DIAG
#define _CSMD_DIAG

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/* ========================================================================= */
/* CoSeMa Driver and Sercos Controller identifier                            */
/* ========================================================================= */

#define CSMD_DEVICE_NAME        "IP-Core "
#define CSMD_DEVICE_NONAME      "No controller!"

#define CSMD_DRIVER_NAME        "CoSeMa "
#define CSMD_DRV_VERSION                 6
#define CSMD_DRV_MINOR_VERSION             1
#define CSMD_DRVSTR_TYPE                    'V'
#define CSMD_DRV_RELEASE                       12

#define CSMD_DRIVER_DATE        "Jul 21 2016"


/*! \cond PRIVATE */

/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

/* Record SVC error for extended diagnostics */
SOURCE CSMD_VOID CSMD_Record_SVC_Error
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSlaveIdx,
                                  CSMD_FUNC_RET              eError );

/* In case of an error additional information is added to variable */
SOURCE CSMD_VOID CSMD_RuntimeError
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSlaveIndex,
                                  const char                *pchMessage );

/* In case of a Warning additional information is added to variable */
SOURCE CSMD_VOID CSMD_RuntimeWarning
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSlaveIndex,
                                  const char                *pchMessage );

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_DIAG */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
11 Nov 2013 WK
  - Start development CoSeMa Version 6.1.
20 May 2014 AlM
  - First Test-Version CoSeMa 6.1 T 001.
01 Sep 2014 WK
  - Test-Version CoSeMa 6.1 T 002.
16 Apr 2015 WK
  - Test-Version CoSeMa 6.1 T 003.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
24 Jul 2015 WK
  - Version CoSeMa 6.1 V 004.
30 Sep 2015 WK
  - Test-Version CoSeMa 6.1 T 005.
06 Nov 2015 WK
  - Test-Version CoSeMa 6.1 T 006.
19 Nov 2015 WK
  - Version CoSeMa 6.1 V 007.
03 Dec 2015 WK
  - Test-Version CoSeMa 6.1 T 008.
04 Feb 2016 WK
  - Version CoSeMa 6.1 V 009.
08 Jun 2016 WK
  - Version CoSeMa 6.1 V 010.
30 Jun 2016 WK
  - FEAT-00051878 - Support for Fast Startup
    Test-Version CoSeMa 6.1 T 011.
21 Jul 2016 WK
  - Version CoSeMa 6.1 V 012.
  
------------------------------------------------------------------------------
*/
