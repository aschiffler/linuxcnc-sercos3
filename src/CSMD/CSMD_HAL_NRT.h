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
\file   CSMD_HAL_NRT.h
\author WK
\date   30.06.2006
\brief  This File contains the function prototypes
        and macro definitions for the file CSMD_HAL_NRT.c
*/


#ifndef _CSMD_HAL_NRT
#define _CSMD_HAL_NRT

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif


/*! \cond HAL_NRT */


/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*---------------------------------------------------- */
/* Port definitions                                    */
/*---------------------------------------------------- */
#define CSMD_HAL_IP_NO_PORT         (0)   /* no affect to any PHY interface */
#define CSMD_HAL_IP_PORT1           (1)   /* PHY interface port 1 related */
#define CSMD_HAL_IP_PORT2           (2)   /* PHY interface port 2 related */
#define CSMD_HAL_IP_BOTH_PORTS      (3)   /* PHY interface port 1 and 2 related */
/*---------------------------------------------------- */
/* FPGA IP definitions                                 */
/*---------------------------------------------------- */
#define CSMD_HAL_IP_RAM_SEG_SIZE    (256)   /* bytes */      


/* Definitions for request of IP communication status bits */
#define CSMD_HAL_IP_STATUS_INFO         0x0000000FU
#define CSMD_HAL_IP_RX_BUFFER_READY     0x00000001U
#define CSMD_HAL_IP_RX_BUFFER_FULL      0x00000002U
#define CSMD_HAL_IP_TX_BUFFER_READY     0x00000004U
#define CSMD_HAL_IP_TX_BUFFER_EMPTY     0x00000008U
#define CSMD_HAL_IP_LINK_INFO           0x00008000U

/* Definitions for IP communication control bits */
#define CSMD_HAL_IP_TX_ENABLE           0x00000001U
#define CSMD_HAL_IP_RX_ENABLE           0x00000002U
#define CSMD_HAL_BROADCAST_DISABLE      0x00000008U
#define CSMD_HAL_MULTICAST_DISABLE      0x00000010U
#define CSMD_HAL_PROMISCUOUS_MODE       0x00000040U


#define BYTE_SIZE       sizeof(CSMD_UCHAR)       /* (1) */
#define SHORT_SIZE      sizeof(CSMD_SHORT)       /* (2) */
#define LONG_SIZE       sizeof(CSMD_LONG)        /* (4) */

/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/




/*----------------------------------------------------------------------------*/
/*---- Definition public Functions: ------------------------------------------*/
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

SOURCE CSMD_VOID CSMD_HAL_SetMacAddress
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_UCHAR                 ucMacAddr[] );

SOURCE CSMD_VOID CSMD_HAL_GetMacAddress
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_UCHAR                 ucMacAddr[] );

SOURCE CSMD_ULONG CSMD_HAL_PutIPControlRegister
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                 ulIPCntrl,
                                  CSMD_USHORT                usPort );

SOURCE CSMD_ULONG CSMD_HAL_GetIPControlRegister 
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                *pulIPCntrl,
                                  CSMD_USHORT                usPort );

SOURCE CSMD_ULONG CSMD_HAL_MaskIPControlRegister
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                 ulIPCntrlMask,
                                  CSMD_BOOL                  boMode,
                                  CSMD_USHORT                usPort );

SOURCE CSMD_ULONG CSMD_HAL_GetIPStatusRegister
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                *pulIPStatus,
                                  CSMD_USHORT                usPort );

SOURCE CSMD_ULONG CSMD_HAL_ConfigIPTxDescriptor
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_USHORT                usFirstRamSegment,
                                  CSMD_USHORT                usLastRamSegment,
                                  CSMD_USHORT                usPort );


SOURCE CSMD_ULONG CSMD_HAL_WriteIPTxStack
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_USHORT                usSegAddr,
                                  CSMD_USHORT                usLength,
                                  CSMD_USHORT                usPort );

SOURCE CSMD_ULONG CSMD_HAL_ConfigIPRxDescriptor
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_USHORT                usFirstRamSegment,
                                  CSMD_USHORT                usLastRamSegment,
                                  CSMD_USHORT                usPort );

SOURCE CSMD_ULONG CSMD_HAL_ReadIPRxStack
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                *pulRegister,
                                  CSMD_USHORT                usPort );

SOURCE CSMD_ULONG CSMD_HAL_ClearIPRxStack
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_USHORT                usPort );

SOURCE CSMD_ULONG CSMD_HAL_IPGetPacket
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                 ulRxFrameOffset,
                                  CSMD_UCHAR                *pucDest,
                                  CSMD_ULONG                 ulLen );

SOURCE CSMD_ULONG CSMD_HAL_IPPutPacket
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                 ulTxFrameOffset,
                                  CSMD_UCHAR                *pucSource,
                                  CSMD_ULONG                 ulLen );

SOURCE CSMD_VOID CSMD_HAL_GetCounter_FramesRxOK
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                *pulRegister );

SOURCE CSMD_VOID CSMD_HAL_GetCounter_FramesTxOK
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                *pulRegister );

SOURCE CSMD_VOID CSMD_HAL_GetCounter_FCSErrors
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                *pulRegister );

SOURCE CSMD_VOID CSMD_HAL_GetCounter_AlignmentErrors
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                *pulRegister );

SOURCE CSMD_VOID CSMD_HAL_GetCounter_DiscardResRxBuf
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                *pulRegister );

SOURCE CSMD_VOID CSMD_HAL_GetCounter_DiscardColRxBuf
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                *pulRegister );

SOURCE CSMD_VOID CSMD_HAL_GetCounter_IPChViolation
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                *pulRegister );

SOURCE CSMD_VOID CSMD_HAL_GetCounter_SercosError
                                ( CSMD_HAL                  *prCSMD_HAL,
                                  CSMD_ULONG                *pulRegister );

SOURCE CSMD_ULONG CSMD_HAL_InitDriverCB
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_VOID                 *pvCB_Info,
                                  CSMD_CB_FUNCTIONS         *prCB_Functions );

SOURCE CSMD_ULONG CSMD_HAL_GetRxTxRamAlloc
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_VOID                 *pvCB_Info,
                                  CSMD_SIII_INFO            *prSIII_Info );

SOURCE CSMD_ULONG CSMD_HAL_GetConStatus
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_STATUS               *prStatus );

SOURCE CSMD_ULONG CSMD_HAL_GetMTU
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT               *pusMTU );

/* Function should not be called
SOURCE CSMD_VOID CSMD_HAL_CheckTypes( CSMD_VOID );
*/

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* HAL_NRT */

#endif /* _CSMD_HAL_NRT */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

02 Sep 2010
  - File created.
21 Nov 2013 WK
  - Defdb00000000
    Removed function prototype CSMD_HAL_ResetCounter_SercosError().
06 Feb 2014 WK
  - Defdb00166929
    Added function prototype for CSMD_HAL_GetIPControlRegister().
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
  
------------------------------------------------------------------------------
*/
