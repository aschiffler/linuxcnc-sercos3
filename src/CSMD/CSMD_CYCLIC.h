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
\file   CSMD_CYCLIC.h
\author WK
\date   03.09.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_CYCLIC.c
*/


#ifndef _CSMD_CYCLIC
#define _CSMD_CYCLIC

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! \cond PRIVATE */

/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

/* Memory copy function to copy from local Ram to FPGA TxRam. */
SOURCE CSMD_VOID CSMD_Write_Tx_Ram
                                ( CSMD_USHORT               *pusTelDestTxRam,
                                  CSMD_USHORT               *pusTelSource,
                                  CSMD_USHORT                usNbrWords );

#ifdef CSMD_PCI_MASTER
/* Check, if all selected DMA channels returns a Tx DMA ready message */
SOURCE CSMD_BOOL CSMD_IsBusy_Tx_DMA
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_USHORT                usChannels );

/* Check, if all selected DMA channels returns a Rx DMA ready message */
SOURCE CSMD_BOOL CSMD_IsBusy_Rx_DMA
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_USHORT                usChannels );
#endif

/* Clears the CC connection data in TxRam */
SOURCE CSMD_VOID CSMD_Clear_Tx_CC_Data
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_DeleteSlave
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSlaveIdx );

SOURCE CSMD_VOID CSMD_SlaveValidError
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSlaveIdx );

SOURCE CSMD_VOID CSMD_CyclicDeviceControl
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_CyclicDeviceStatus
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_CyclicConnection
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_EvaluateConnections
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_Set_C_Con_Check_Mode
                                ( CSMD_USHORT                usC_Con,
                                  CSMD_CONN_SLAVEPROD       *prSlaveProd );

SOURCE CSMD_VOID CSMD_InvalidConnectionData
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usConnIdx,
                                  CSMD_CONN_SLAVEPROD       *prSlaveProd );

/* read and clear the telegram status register of both.ports / build flags for topology checks */
SOURCE CSMD_VOID CSMD_ReadTelegramStatus
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Write hot-plug field communication phase depending into Tx Ram */
SOURCE CSMD_VOID CSMD_CyclicHotPlug_MDT
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Read the Hot-Plug fields of both ports from Rx Ram */
SOURCE CSMD_VOID CSMD_CyclicHotPlug_AT
                                ( CSMD_INSTANCE             *prCSMD_Instance );

/* Memory copy function to copy from FPGA RxRam to local Ram. */
SOURCE CSMD_VOID CSMD_Read_Rx_Ram
                                ( CSMD_USHORT               *pusTelDest,
                                  CSMD_USHORT               *pusTelSourceRxRam,
                                  CSMD_USHORT                usNbrWords );

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_CYCLIC */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

03 Sep 2010
  - File created.
02 Dec 2013 WK
  - Defdb00165150
    Added prototype for function CSMD_Clear_Tx_CC_Data().
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.

------------------------------------------------------------------------------
*/
