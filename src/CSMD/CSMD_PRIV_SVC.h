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
\file   CSMD_PRIV_SVC.h
\author WK
\date   01.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_PRIV_SVC.c
*/


#ifndef _CSMD_PRIV_SVC
#define _CSMD_PRIV_SVC

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! \cond PRIVATE */

#define  CSMD_SC_LENGTH_SHIFT         0x08U
#define  CSMD_SC_CLEAR_REGISTER     0x0000U
#define  CSMD_SC_SVCC0_INIT_MASK    0x0081U
#define  CSMD_SC_SVCC0_ELEM_SHIFT   0x0003U


/*-------------------------------------------------------- */
/* Definitions for Service Channel Control Words/Register  */
/*-------------------------------------------------------- */

      /* --- 1st.Word ( Index 0 ) ----------------- */
#define  CSMD_SVC_CTRL_INT_END_RDBUF      0x4000U   /* Bit 14:  End of read buffer reached  */
#define  CSMD_SVC_CTRL_INT_END_WRBUF      0x2000U   /* Bit 13:  End of write buffer reached */
#define  CSMD_SVC_CTRL_INT_ERR            0x1000U   /* Bit 12:  Slave reports error         */
#define  CSMD_SVC_CTRL_M_BUSY             0x0080U   /* Bit 7:   SVC waits for interaction   */
#define  CSMD_SVC_CTRL_SETEND             0x0040U   /* Bit 6:   END_MDT is to be set        */
/*---------------------------------------------------- */
/* Service channel control bits                        */
/*---------------------------------------------------- */
#define  CSMD_SVC_CTRL_ELEMENT_MASK       0x0038U   /* Bit 5-3: Data block element number   */
#define  CSMD_SVC_CTRL_ELEMENT_IDN        0x0008U   /*          001: Data block element 1 "IDN"  */
#define  CSMD_SVC_CTRL_LASTTRANS          0x0004U   /* Bit 2:   End of element transmission */
                                                    /*   '0' = Transmission in progress     */
                                                    /*   '1' = Last transmission            */
#define  CSMD_SVC_CTRL_WRITE              0x0002U   /* Bit 1:   Write(1)/Read(0) SVC INFO   */
#define  CSMD_SVC_CTRL_HANDSHAKE          0x0001U   /* Bit 0:   MHS  (toggle bit)           */

      /* --- 2nd.Word ( Index 1 ) ----------------- */
/*---------------------------------------------------- */
/* Service channel status bits                         */
/*---------------------------------------------------- */
#define  CSMD_SVC_STAT_VALID              0x0008U   /* Bit 3:   SVC valid                   */
                                                    /*   '0' = not valid (SVC is not processed by the slave) */
                                                    /*   '1' = valid (SVC is processed by the slave)    */
#define  CSMD_SVC_STAT_ERROR              0x0004U   /* Bit 2:   SVC Error                   */
                                                    /*   '0' = No error                     */
                                                    /*   '1' = Error in SVC, error message in SVC INFO   */
#define  CSMD_SVC_STAT_BUSY               0x0002U   /* Bit 1:   Busy                        */
                                                    /*   '0' = Step finished, slave ready for new step   */
                                                    /*   '1' = Step in process, new step not allowed.    */
#define  CSMD_SVC_STAT_HANDSHAKE          0x0001U   /* Bit 0:   AHS  (toggle bit)           */

      /* --- 5th.Word ( Index 4 ) ----------------- */
#define  CSMD_SVC_CTRL_INT_BUSY_TIMEOUT   0x0800U   /* Bit 11:  Busy timeout                */
#define  CSMD_SVC_CTRL_INT_HS_TIMEOUT     0x0400U   /* Bit 10:  Handshake timeout           */
#define  CSMD_SVC_CTRL_INT_SC_ERR         0x0200U   /* Bit 9:   protocol error              */



/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

SOURCE CSMD_VOID CSMD_EvaluateAttribute
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  const CSMD_SVCH_MACRO_STRUCT  *prSvchData );

SOURCE CSMD_BOOL CSMD_NeedListLength
                                ( const CSMD_INSTANCE           *prCSMD_Instance,
                                  const CSMD_SVCH_MACRO_STRUCT  *prSvchData );

   
SOURCE CSMD_BOOL CSMD_NeedAttribut
                                ( const CSMD_INSTANCE           *prCSMD_Instance,
                                  const CSMD_SVCH_MACRO_STRUCT  *prSvchData );

   
SOURCE CSMD_FUNC_RET CSMD_CheckSVCHInUse
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT        *prSvchData );

SOURCE CSMD_FUNC_RET CSMD_CheckRequestCancel
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT        *prSvchData );
   
SOURCE CSMD_VOID CSMD_InitSVCHRequest
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT        *prSvchData,
                                  CSMD_BOOL                      boCommandRequest );

SOURCE CSMD_FUNC_RET CSMD_CheckSVCError
                                ( const CSMD_SERC3SVC           *prSVC,
                                  CSMD_SVCH_MNGMT_STRUCT        *prSvchMngmtData,
                                  CSMD_BOOL                      boReadAccess );

SOURCE CSMD_FUNC_RET CSMD_OpenSVCH
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT        *prSvchData,
                                  CSMD_USHORT                    usSlaveIdx,
                                  CSMD_BOOL                     *pboBreak );

SOURCE CSMD_FUNC_RET CSMD_PrepFinalStepWrite
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT        *prSvchData,
                                  CSMD_USHORT                    usSlaveIdx,
                                  CSMD_BOOL                     *pboBreak );

SOURCE CSMD_FUNC_RET CSMD_PrepFinalStepRead
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT        *prSvchData,
                                  CSMD_USHORT                    usSlaveIdx,
                                  CSMD_BOOL                     *pboBreak );

#if defined CSMD_BIG_ENDIAN || defined CSMD_TEST_BE
SOURCE CSMD_FUNC_RET CSMD_ReadAttribute
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT        *prSvchData,
                                  CSMD_USHORT                    usSlaveIdx,
                                  CSMD_BOOL                     *pboBreak );
#endif

SOURCE CSMD_FUNC_RET CSMD_FinalStepRead
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT        *prSvchData,
                                  CSMD_USHORT                    usSlaveIdx,
                                  CSMD_BOOL                     *pboBreak );

SOURCE CSMD_FUNC_RET CSMD_FinalStepWrite
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT        *prSvchData,
                                  CSMD_USHORT                    usSlaveIdx,
                                  CSMD_BOOL                     *pboBreak );

SOURCE CSMD_FUNC_RET CSMD_SVCHWrite
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT        *prSvchMngmtData );

SOURCE CSMD_VOID CSMD_SVCHRead  ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  const CSMD_SVCH_MNGMT_STRUCT  *prSvchMngmtData );

SOURCE CSMD_VOID CSMD_ClearInternalSVCInterrupts
                                ( CSMD_INSTANCE                 *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_ClearInternal_SVCInterrupt
                                ( CSMD_INSTANCE                 *prCSMD_Instance,
                                  CSMD_USHORT                    usSlaveIdx );

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_PRIV_SVC */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
  
------------------------------------------------------------------------------
*/
