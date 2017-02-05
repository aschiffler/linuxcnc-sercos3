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
\file   CSMD_HOTPLUG.h
\author WK
\date   01.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_HOTPLUG.c
*/


#ifndef _CSMD_HOTPLUG
#define _CSMD_HOTPLUG

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif



/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*---- Declaration private Types: --------------------------------------------*/
#if(0)
/* States for the function state machine of CSMD_HotPlug() */
enum  CSMD_HP_FUNC_STATES_EN
{
  CSMD_FUNCSTATE_HP0_ENTRY = 0,       /* Step  0 */
  CSMD_FUNCSTATE_HP0_DO_TX_PARAM,     /* Step  1 */
  CSMD_FUNCSTATE_HP0_DO_SCAN_SLAVES,  /* Step  2 */
  CSMD_FUNCSTATE_HP0_EXIT,            /* Step  3 */
  CSMD_FUNCSTATE_HP1_ENTRY,           /* Step  4 */
  CSMD_FUNCSTATE_HP1_DO_CHECK_ADDR,   /* Step  5 */
  CSMD_FUNCSTATE_HP1_DO_TX_PARAM,     /* Step  6 */
  CSMD_FUNCSTATE_HP1_DO_CHECK_SVC,    /* Step  7 */
  CSMD_FUNCSTATE_HP1_DO_ACTIVATE,     /* Step  8 */
  CSMD_FUNCSTATE_HP1_EXIT,            /* Step  9 */
  CSMD_FUNCSTATE_HP__ENTRY_ERROR,     /* Step 10 */
  CSMD_FUNCSTATE_HP__EXIT_ERROR       /* Step 11 */

};
#endif
/*! \cond PRIVATE */

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef CSMD_HOTPLUG

SOURCE CSMD_BOOL CSMD_IsHotplugSupported
                                ( const CSMD_INSTANCE       *prCSMD_Instance );

SOURCE CSMD_BOOL CSMD_HP_Is_Sercos_Link
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_Send_HP0_Parameter
                                ( CSMD_INSTANCE             *prCSMD_Instance );

#endif  /* End: #ifdef CSMD_HOTPLUG */

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_HOTPLUG */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
31 May 2016 WK
  - Defdb00187475
    Added function prototypes:
    CSMD_HP_Is_Sercos_Link(), CSMD_Send_HP0_Parameter()

------------------------------------------------------------------------------
*/
