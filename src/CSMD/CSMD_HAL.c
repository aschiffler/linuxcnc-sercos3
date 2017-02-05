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
\file   CSMD_HAL.c
\author AM
\date   03.09.2010
\brief  This File contains the private HAL functions.
\details \b Introduction: \n
   In the past CoSeMa has depended from Hardware, especially FPGA. 
   With HAL implementation, this dependency was disposed. Therefore all 
   CoSeMa hardware accesses were packaged and replaced by functions or macros.
   \image html CoSeMa-HAL.svg
<B>HAL functions:</B>\n
   The HAL function-header is defined in this file. If needed, the developers 
   can implement their own functionality. For faultless working with CoSeMa,
   all functions must be implemented, either as function or as define.

<B>Target system:</B>
  - SH4A / SH2A / MicroBlaze
*/

#include <stdio.h>
#include <string.h>

#include "CSMD_USER.h"
#include  CSMD_TYPE_HEADER

#include "CSMD_HAL_GLOB.h"
#include "CSMD_HAL_PRIV.h"


/*lint -save -e818 Pointer parameter '' could be declared as pointing to const */

/*! \cond HAL */

/*---- Definition public Functions: ------------------------------------------*/


/*---- Definition private Functions: -----------------------------------------*/


/**************************************************************************/ /**
\brief Checks the version of the currently implemented Sercos IP-Core.

\ingroup func_timing
\b Description: \n
   All version relevant information should be checked here. If the call
   is not used, the return value must be TRUE.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure

\return       TRUE  -> version check successful\n
              FALSE -> version check failed

\author       KP
\date         13.08.2007
\version      01.03.2013 (GMy): Defdb00153553 Soft master support for CoSeMa

***************************************************************************** */
CSMD_BOOL CSMD_HAL_CheckVersion( CSMD_HAL *prCSMD_HAL )
{
  
  CSMD_USHORT usI;
  CSMD_ULONG  ulVersion;
  CSMD_ULONG  ulDeviceType;
  CSMD_ULONG  ulTstVersion;           /* Testversion */
  CSMD_BOOL   boVersionCheck  = TRUE; /* FPGA Version is supported */
  CSMD_ULONG  ulaCmpVersion[] = CSMD_HAL_SERCON100M_VERSION_LIST; /*lint !e845 The left argument to operator '6' is certain to be 0  */
  
  
  ulVersion = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulIDR) );
  
  ulDeviceType = (ulVersion & CSMD_HAL_FPGA_IDR_TYPE_MASK) >> CSMD_HAL_FPGA_IDR_TYPE_SHIFT;

  if (   (((ulVersion & CSMD_HAL_FPGA_IDR_IDENT_MASK) >> CSMD_HAL_FPGA_IDR_IDENT_SHIFT) 
      != CSMD_HAL_FPGA_IDR_SIII_IDENT))
  {
    /* Ethernet type not Sercos III */
    boVersionCheck = FALSE;
  }
  else if (   (CSMD_HAL_IsSoftMaster( prCSMD_HAL ) == TRUE)
           && !(CSMD_HAL_FPGA_IDR_SOFT_MASTER == ulDeviceType))
  {
    /* No Sercos soft-master device */
    boVersionCheck = FALSE;
  }
  else if (   (CSMD_HAL_IsSoftMaster( prCSMD_HAL ) != TRUE)
           && !(   (CSMD_HAL_FPGA_IDR_MASTER      == ulDeviceType)
                || (CSMD_HAL_FPGA_IDR_MASTER_CONF == ulDeviceType)))
  {
    /* No Sercos master IP-Core device (SERCON100M) */
    boVersionCheck = FALSE;
  }
  else
  {
    /* IP-Core release information */
    prCSMD_HAL->usFPGA_Release     = (CSMD_USHORT)((ulVersion & CSMD_HAL_FPGA_IDR_RELEASE_MASK) >> CSMD_HAL_FPGA_IDR_RELEASE_SHIFT);
    /* IP-Core testversion information */
    prCSMD_HAL->usFPGA_TestVersion = (CSMD_USHORT)((ulVersion & CSMD_HAL_FPGA_IDR_TEST_MASK) >> CSMD_HAL_FPGA_IDR_TEST_SHIFT);
    
    /* Check for blacklisted FPGA version */
    /* Extract FPGA testversion number */
    ulTstVersion = (ulVersion & CSMD_HAL_FPGA_IDR_TEST_MASK) >> CSMD_HAL_FPGA_IDR_TEST_SHIFT;
    /* extract FPGA version & release */
    ulVersion &= (CSMD_HAL_FPGA_IDR_VERSION_MASK + CSMD_HAL_FPGA_IDR_RELEASE_MASK);
    
    if (((ulVersion & CSMD_HAL_FPGA_IDR_VERSION_MASK) >> CSMD_HAL_FPGA_IDR_VERSION_SHIFT) == CSMD_HAL_SERCON100M_V4)
    {
      /* FPGA Version is V4 */
      for (usI = 0; usI < (sizeof (ulaCmpVersion)/sizeof (ulaCmpVersion[0])); usI++)
      {
        if (ulVersion == (ulaCmpVersion[usI] & ~CSMD_HAL_FPGA_TEST_MASK))
        {
          /* Found Version & Release in the blacklist */
          if (ulTstVersion)
          {
            /* Testversion */
            if (((ulaCmpVersion[usI] & CSMD_HAL_FPGA_TEST_MASK) >> CSMD_HAL_FPGA_TEST_SHIFT) & (1UL << ulTstVersion))
            {
              /* Blacklisted testversion */
#ifdef CSMD_SOFT_MASTER
              if (CSMD_HAL_IsSoftMaster( prCSMD_HAL ) == TRUE)
              {
                boVersionCheck = TRUE;  /* IP-Core test versions expressly permitted for soft-master */
              }
              else
#endif
              {
                boVersionCheck = FALSE;
              }
              break;
            }
          }
          else
          {
            /* No Testversion */
            if (((ulaCmpVersion[usI] & CSMD_HAL_FPGA_TEST_MASK) >> CSMD_HAL_FPGA_TEST_SHIFT) & 1UL)
            {
              /* Blacklisted official version */
              boVersionCheck = FALSE;
              break;
            }
          }
        }
      }
    }
  }
  return (boVersionCheck);
  
} /* end: CSMD_HAL_CheckVersion () */



/**************************************************************************/ /**
\brief Performs a FPGA software reset and/or PHY reset.

\ingroup module_init
\b Description: \n
   This function performs a software and/or a PHY reset by writing the
   corresponding bits in the GCSFR register.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   boSoftwareReset  
              TRUE -> performs a FPGA software reset\n
              FALSE -> no effect 
\param [in]   boPHY_Reset  
              TRUE -> perform a PHY reset\n
              FALSE -> no effect

\return       none

\author       WK
\date         21.03.2011

***************************************************************************** */
CSMD_VOID CSMD_HAL_SoftReset( CSMD_HAL  *prCSMD_HAL,
                              CSMD_BOOL  boSoftwareReset,
                              CSMD_BOOL  boPHY_Reset )
{
  CSMD_HAL_SERCFPGA_DATTYP ulGCSFR;
  
  ulGCSFR = prCSMD_HAL->prSERC_Reg->ulGCSFR &= 
    ~(  (CSMD_ULONG)(1 << CSMD_HAL_GCSFR_SOFT_RESET)
      | (CSMD_ULONG)(1 << CSMD_HAL_GCSFR_PHY_RESET));
  
  if (boSoftwareReset)
    ulGCSFR |= (CSMD_ULONG)(1 << CSMD_HAL_GCSFR_SOFT_RESET);
  if (boPHY_Reset)
    ulGCSFR |= (CSMD_ULONG)(1 << CSMD_HAL_GCSFR_PHY_RESET);
  
  prCSMD_HAL->prSERC_Reg->ulGCSFR = ulGCSFR;
  
} /* end: CSMD_HAL_SoftReset() */



/**************************************************************************/ /**
\brief Returns the status of FPGA software reset and PHY reset.

\ingroup module_init
\b Description: \n
   This function reads the current software and/or PHY reset states from
   the corresponding bits of the GCSFR register.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pboSoftwareReset
              Current status of FPGA software reset\n
              TRUE -> FPGA software reset is active\n
              FALSE -> FPGA software reset is not active
\param [out]  pboPHY_Reset  Current status of PHY reset\n
              TRUE -> PHY reset is active\n
              FALSE -> PHY reset is not active

\return       none

\author       WK
\date         21.03.2011

***************************************************************************** */
CSMD_VOID CSMD_HAL_StatusSoftReset( CSMD_HAL  *prCSMD_HAL,
                                    CSMD_BOOL *pboSoftwareReset,
                                    CSMD_BOOL *pboPHY_Reset )
{
  CSMD_HAL_SERCFPGA_DATTYP ulGCSFR;
  
  ulGCSFR = prCSMD_HAL->prSERC_Reg->ulGCSFR;
  
  *pboSoftwareReset = (ulGCSFR & (CSMD_ULONG)(1 << CSMD_HAL_GCSFR_SOFT_RESET)) 
                      == (CSMD_ULONG)(1 << CSMD_HAL_GCSFR_SOFT_RESET);
  
  *pboPHY_Reset     = (ulGCSFR & (CSMD_ULONG)(1 << CSMD_HAL_GCSFR_PHY_RESET)) 
                      == (CSMD_ULONG)(1 << CSMD_HAL_GCSFR_PHY_RESET);
  
} /* end: CSMD_HAL_StatusSoftReset() */



/**************************************************************************/ /**
\brief Reads identification data for output via CSMD_Version().

\ingroup func_diag
\b Description: \n
   Hardware identification data should be transferred as follows:
   -   device version 
   -   device release
   -   device identification (0x88CD for Sercos III FPGA)
   -   device type  (0=Master, 1=Slave)

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   pusRelease
              Pointer to for release info
\param [in]   pusVersion
              Pointer to version info
\param [in]   pusTestversion
              Pointer to test version info
\param [in]   pusDeviceIdent
              Pointer to device identification
\param [in]   pusSERCDeviceTyp
              Pointer to device type

\return       none

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_VOID CSMD_HAL_GetIdentification( CSMD_HAL    *prCSMD_HAL,
                                      CSMD_USHORT *pusRelease,
                                      CSMD_USHORT *pusVersion,
                                      CSMD_USHORT *pusTestversion,
                                      CSMD_USHORT *pusDeviceIdent,
                                      CSMD_USHORT *pusSERCDeviceTyp )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  CSMD_ULONG ulIDR = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulIDR) );
  
  *pusRelease       = (CSMD_USHORT) (   (ulIDR & CSMD_HAL_FPGA_IDR_RELEASE_MASK) 
                                >> CSMD_HAL_FPGA_IDR_RELEASE_SHIFT);
  *pusVersion       = (CSMD_USHORT) (   (ulIDR & CSMD_HAL_FPGA_IDR_VERSION_MASK) 
                                >> CSMD_HAL_FPGA_IDR_VERSION_SHIFT);
  *pusTestversion   = (CSMD_USHORT) (   (ulIDR & CSMD_HAL_FPGA_IDR_TEST_MASK) 
                                >> CSMD_HAL_FPGA_IDR_TEST_SHIFT);
  *pusSERCDeviceTyp = (CSMD_USHORT) (   (ulIDR & CSMD_HAL_FPGA_IDR_TYPE_MASK) 
                                >> CSMD_HAL_FPGA_IDR_TYPE_SHIFT);
  *pusDeviceIdent   = (CSMD_USHORT) (   (ulIDR & CSMD_HAL_FPGA_IDR_IDENT_MASK) 
                                >> CSMD_HAL_FPGA_IDR_IDENT_SHIFT); 
  
} /* end: CSMD_HAL_GetIdentification () */



/**************************************************************************/ /**
\brief Sets the Inter Frame Gap.

\ingroup func_timing
\b Description: \n
   This function sets the Inter Frame Gap in consideration of the minimum
   configurable value 

<B>Call environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulNbrBytes
              Inter Frame Gap [bytes]\n
              Values lower than 12 will be padded to the base IFG value of 12 Bytes.

\return       none

\author       WK
\date         28.07.2010

***************************************************************************** */
CSMD_VOID CSMD_HAL_SetInterFrameGap( CSMD_HAL   *prCSMD_HAL,
                                     CSMD_ULONG  ulNbrBytes )
{
  if (ulNbrBytes > CSMD_HAL_TXIFG_BASE)
  {
    prCSMD_HAL->prSERC_Reg->ulIFG = CSMD_END_CONV_L( ulNbrBytes );
  }
  else
  {
    prCSMD_HAL->prSERC_Reg->ulIFG = CSMD_END_CONV_L( CSMD_HAL_TXIFG_BASE );
  }

} /* end: CSMD_HAL_SetInterFrameGap() */



/**************************************************************************/ /**
\brief Writes a timer event into FPGA indirectly via an index select register.

\ingroup module_phase
\b Description: \n
   This function writes a timer event selected via the index select register
   TDSR into the registers TDRU (upper long of selected timing descriptor) and
   TDRL (lower long of selected timing descriptor). In case the soft master is
   used (CSMD_INSTANCE.boSoftMaster = TRUE), the timer event is written flat
   into the register memory area after the 'normal' IP-Core registers.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usIdx
              Timing descriptor select (Index)
\param [in]   ulTime
              Value for timer event [ns]
\param [in]   usType
              Type of event
\param [in]   usSubCycCnt
              Subcycle count for event enable
\param [in]   usSubCycCntSel
              Selection of subcycle counter
              - 0:Sercos III cycle
              - 1:subcycle counter 1
              - 2:subcycle counter 2
              - 3:reserved

\return       none

\author       WK
\date         08.02.2010

\version      25.02.2013 (GMy): Defdb00153553 Soft master support for CoSeMa

***************************************************************************** */
CSMD_VOID CSMD_HAL_SetTimerEvent( CSMD_HAL    *prCSMD_HAL,
                                  CSMD_USHORT  usIdx,
                                  CSMD_ULONG   ulTime,
                                  CSMD_USHORT  usType,
                                  CSMD_USHORT  usSubCycCnt,
                                  CSMD_USHORT  usSubCycCntSel )
{
  
#ifdef CSMD_SOFT_MASTER
  if (CSMD_HAL_IsSoftMaster( prCSMD_HAL ) == TRUE)
  {
    /* In soft master mode, store data flat in register memory at offset
    CSMD_HAL_SOFT_MASTER_REG_EVENT_OFFSET */

    CSMD_HAL_EVENT *prEvents;

    prEvents = (CSMD_HAL_EVENT*)
                 (CSMD_VOID *)((CSMD_UCHAR *)prCSMD_HAL->prSERC_Reg + CSMD_HAL_SOFT_MASTER_REG_EVENT_OFFSET);

    prEvents[usIdx].ulTime         = CSMD_END_CONV_L( ulTime );
    prEvents[usIdx].usSubCycCnt    = CSMD_END_CONV_S( usSubCycCnt );
    prEvents[usIdx].usType         = CSMD_END_CONV_S( usType );
    prEvents[usIdx].usSubCycCntSel = CSMD_END_CONV_S( usSubCycCntSel );
  }
  else
#endif
  {
    /* Select timing descriptor */
    prCSMD_HAL->prSERC_Reg->ulMTDSR =
      CSMD_END_CONV_L( (CSMD_ULONG)usIdx );

    /* Program selected timing descriptor upper long */
    prCSMD_HAL->prSERC_Reg->ulMTDRU =
      CSMD_END_CONV_L( (  ((CSMD_ULONG)usSubCycCnt & CSMD_HAL_EVENT_SUBCYCCNT_MASK)
                        | (((CSMD_ULONG)usType << CSMD_HAL_EVENT_TYPE_SHIFT) & CSMD_HAL_EVENT_TYPE_MASK)
                        | (((CSMD_ULONG)usSubCycCntSel << CSMD_HAL_EVENT_SUBCYCSEL_SHIFT) & CSMD_HAL_EVENT_SUBCYCSEL_MASK))
                      );

    /* Program selected timing descriptor lower long */
    prCSMD_HAL->prSERC_Reg->ulMTDRL =
      CSMD_END_CONV_L( ulTime & CSMD_HAL_EVENT_TIME_MASK );
  }
  
} /* end: CSMD_HAL_SetTimerEvent() */



/**************************************************************************/ /**
\brief Writes port related timer event indirectly into
       FPGA over an index select register.

\ingroup module_phase
\b Description: \n
   This function writes a port related timer event selected via the index select
   register PTDSR into the registers PTDRU (upper long of selected timing descriptor)
   and PTDRL (lower long of selected timing descriptor).

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]    prCSMD_HAL
               Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]    usIdx
               timing descriptor select (Index)
\param [in]    ulTime
               value for port related timer event [ns]
\param [in]    usType
               type of port related event
\param [in]    usSubCycCnt
               subcycle count for event enable
\param [in]    usSubCycCntSel
               selection of subcycle counter
               - 0:Sercos III cycle
               - 1:subcycle counter 1
               - 2:subcycle counter 2
               - 3:reserved

\return        none

\author       WK
\date         08.02.2010

***************************************************************************** */
CSMD_VOID CSMD_HAL_SetPortEvent( CSMD_HAL    *prCSMD_HAL,
                                 CSMD_USHORT  usIdx,
                                 CSMD_ULONG   ulTime,
                                 CSMD_USHORT  usType,
                                 CSMD_USHORT  usSubCycCnt,
                                 CSMD_USHORT  usSubCycCntSel )
{
  
  /* Select port timing descriptor */
  prCSMD_HAL->prSERC_Reg->ulPTDSR = 
    CSMD_END_CONV_L( (CSMD_ULONG)usIdx );
  
  /* Program selected port timing descriptor upper long */
  prCSMD_HAL->prSERC_Reg->ulPTDRU = 
    CSMD_END_CONV_L( (  ((CSMD_ULONG)usSubCycCnt & CSMD_HAL_EVENT_SUBCYCCNT_MASK)
                      | (((CSMD_ULONG)usType << CSMD_HAL_EVENT_TYPE_SHIFT) & CSMD_HAL_EVENT_TYPE_MASK)
                      | (((CSMD_ULONG)usSubCycCntSel << CSMD_HAL_EVENT_SUBCYCSEL_SHIFT) & CSMD_HAL_EVENT_SUBCYCSEL_MASK))
    );
  
  /* Program selected port timing descriptor lower long */
  prCSMD_HAL->prSERC_Reg->ulPTDRL = 
    CSMD_END_CONV_L( ulTime & CSMD_HAL_EVENT_TIME_MASK );
  
} /* end: CSMD_HAL_SetPortEvent() */



/**************************************************************************/ /**
\brief Activates / deactivates event or SVC interrupt sources.
  
\ingroup sync_internal
\b Description: \n
   In CoSeMa there are service channel and event interrupts.\n
   32 interrupt sources have to be provided, which can be enabled or
   disabled using the parameter ulIntEnable. The event interrupt order must
   be equal to the event order.\n
   Example: Event[0] => event interrupt[Quelle 0]\n
   Optionally, an output can be directed to the interrupt source using ulIntOutputMask.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulIntEnable
              Bit[x] = 0 disables corresponding interrupt source\n
              Bit[x] = 1 enables corresponding interrupt source
\param [in]   boSelEventIntTyp
              TRUE -> event interrupts enabled\n
              FALSE -> event interrupts disabled
\param [in]   boSelSVCIntTyp
              TRUE -> service channel interrupts enabled\n
              FALSE -> service channel interrupts disabled
\param [in]   ulIntOutputMask
              Output directed to the interrupt source

\return       none

\author       WK
\date         15.09.2005 

***************************************************************************** */
CSMD_VOID CSMD_HAL_IntControl( CSMD_HAL   *prCSMD_HAL,
                               CSMD_ULONG  ulIntEnable,
                               CSMD_BOOL   boSelEventIntTyp,
                               CSMD_BOOL   boSelSVCIntTyp,
                               CSMD_ULONG  ulIntOutputMask )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/    
  
  ulIntEnable     = CSMD_END_CONV_L( ulIntEnable );
  ulIntOutputMask = CSMD_END_CONV_L( ulIntOutputMask );
  
  if (boSelEventIntTyp)
    /* Select normal interrupts */
  {
    /* Assign to hardware output INT0/INT1 */
    prCSMD_HAL->prSERC_Reg->ulIMR0 = ulIntOutputMask;
    
    /* Enable/Disable interrupt source */
    prCSMD_HAL->prSERC_Reg->ulIER0 = ulIntEnable;
  }
  
  if (boSelSVCIntTyp) 
    /* Select SVC Interrupts */
  {
    /* Assign to hardware output INT0/INT1 */
    prCSMD_HAL->prSERC_Reg->ulIMR1 = ulIntOutputMask;
    
    /* Enable/Disable interrupt source */
    prCSMD_HAL->prSERC_Reg->ulIER1 = ulIntEnable;
  }
  
} /* end: CSMD_HAL_IntControl() */



/**************************************************************************/ /**
\brief Reads the activation status of event or SVC interrupt sources.

\ingroup sync_internal
\b Description: \n
   This function reads the activation status of event or SVC interrupt sources
   as well as the output assignment of interrupt sources.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   pulIntEnable
              Pointer to copy of interrupt sources
\param [in]   boSelEventIntTyp
              TRUE  -> for event interrupts\n
              FALSE -> not for event interrupts
\param [in]   boSelSVCIntTyp
              TRUE  -> for service channel interrupts\n
              FALSE -> not for service channel interrupts
\param [in]   pulIntOutputMask   pointer to image of output assignments

\return       none

\author       WK
\date         23.01.2008 
  
***************************************************************************** */
CSMD_VOID CSMD_HAL_GetIntControl( CSMD_HAL   *prCSMD_HAL,
                                  CSMD_ULONG *pulIntEnable,
                                  CSMD_BOOL   boSelEventIntTyp,
                                  CSMD_BOOL   boSelSVCIntTyp,
                                  CSMD_ULONG *pulIntOutputMask )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/    
  
  if (boSelEventIntTyp)
    /* Select normal interrupts */
  {
    /* Assignment to hardware output INT0/INT1 */
    *pulIntOutputMask = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulIMR0) );
    
    /* Activation state of interrupt sources */
    *pulIntEnable     = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulIER0) );
  }
  
  else if (boSelSVCIntTyp) 
    /* Select SVC interrupts */
  {
    /* Assignment to hardware output INT0/INT1 */
    *pulIntOutputMask = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulIMR1) );
    
    /* Activation state of interrupt sources */
    *pulIntEnable     = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulIER1) );
  }

} /* end: CSMD_HAL_GetIntControl() */



/**************************************************************************/ /**
\brief Reads the status of interrupt sources.

\ingroup sync_internal
\b Description: \n
   This function reads the status of interrupt sources as well as the
   current event or service channel interrupts.
          
<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   boSelEventIntTyp
              TRUE -> event interrupts enabled\n
              FALSE -> event interrupts disabled
\param [in]   boSelSVCIntTyp
              TRUE -> service channel interrupts enabled\n
              FALSE -> sercive channel interrupts disabled 
              
\return       Status of 32 interrupt sources of the selected interrupt type.

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_ULONG CSMD_HAL_GetInterrupt( CSMD_HAL  *prCSMD_HAL,
                                  CSMD_BOOL  boSelEventIntTyp,
                                  CSMD_BOOL  boSelSVCIntTyp )
{    
/*<HAL>--------------"YOUR IMPLEMENTATION OF HAL-FUNCTION"----------------- <HAL>*/
  
  if (boSelEventIntTyp)
    /* Select Normal Interrupts */
  {
    return ((CSMD_ULONG) CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulIRR0) ));
  }
  else if (boSelSVCIntTyp) 
    /* Select SVC Interrupts */
  {
    return ((CSMD_ULONG) CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulIRR1) )); 
  }
  else
  {
    return ((CSMD_ULONG) 0); 
  }
  
}/* end: CSMD_HAL_GetInterrupt() */



/**************************************************************************/ /**
\brief Clears the currently active interrupt.

\ingroup sync_internal
\b Description: \n
   This function clears the interrupt sources chosen in the mask of the
   currently active interrupt.\n
   Example:\n
   ulIntClearMask = 0x00000012\n
   boSelectIntTyp = TRUE\n
   -> 2nd and 5th interrupt source of event interrupts are cleared.
           
\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   boSelEventIntTyp
              TRUE -> event interrupts enabled\n
              FALSE -> event interrupts disabled
\param [in]   boSelSVCIntTyp
              TRUE -> service channel interrupts enabled\n
              FALSE -> service channel interrupts disabled 
\param [in]   ulIntClearMask
              The interrupt sources corresponding to the bits set in the transmitted
              mask are cleared.

\return       none

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_VOID CSMD_HAL_ClearInterrupt( CSMD_HAL   *prCSMD_HAL,
                                   CSMD_BOOL   boSelEventIntTyp,
                                   CSMD_BOOL   boSelSVCIntTyp,
                                   CSMD_ULONG  ulIntClearMask )
  
{    
/*<HAL>--------------"YOUR IMPLEMENTATION OF HAL-FUNCTION"----------------- <HAL>*/
  
  if (boSelEventIntTyp)
    /* Select normal interrupts */
  {
    prCSMD_HAL->prSERC_Reg->ulIRR0 = CSMD_END_CONV_L( ulIntClearMask );
  }
  
  if (boSelSVCIntTyp) 
    /* Select SVC interrupts */
  {
    prCSMD_HAL->prSERC_Reg->ulIRR1 = CSMD_END_CONV_L( ulIntClearMask ); 
  }
  
} /* end: CSMD_HAL_ClearInterrupt() */



/**************************************************************************/ /**
\brief Reads the current communication phase and phase switch status.

\ingroup module_phase
\b Description: \n
   This function reads the current communication phase and phase switch status from
   register PHASECR.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
 
\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pusPhase
              Output pointer for current communication phase
\param [out]  pusPhaseSwitch
              Output pointer for communication phase switch status\n
              0 = phase switch inactive\n
              1 = phase switch active

\return       none

\author       KP
\date         13.08.2007
 
***************************************************************************** */
CSMD_VOID CSMD_HAL_GetPhase( CSMD_HAL    *prCSMD_HAL,
                             CSMD_USHORT *pusPhase,
                             CSMD_USHORT *pusPhaseSwitch )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  *pusPhase = (CSMD_USHORT) 
    ((prCSMD_HAL->prSERC_Reg->ulPHASECR & CSMD_HAL_PHASECR_PHASE_MASK) >> CSMD_HAL_PHASECR_PHASE_SHIFT);
  
  *pusPhaseSwitch = (CSMD_USHORT) 
    ((prCSMD_HAL->prSERC_Reg->ulPHASECR & (1UL << CSMD_HAL_PHASECR_PS)) == (1UL << CSMD_HAL_PHASECR_PS));
  
} /* end: CSMD_HAL_GetPhase() */



/**************************************************************************/ /**
\brief Enables/disables sending of Sercos telegrams.

\ingroup module_phase
\b Description: \n
   This function either enables or disables the sending of Sercos telegrams
   (MDTs and/or ATs) writing to the corresponding bits of the register SFCR.

<B>Call environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usMDT_Enable
              Enable sending of MDTs, whose bit number is set:
              - MDT0 - Bit 0
              - MDT1 - Bit 1
              - MDT2 - Bit 2
              - MDT3 - Bit 3
\param [in]   usAT_Enable
              Enable sending of ATs, whose bit number is set:
              - AT0 - Bit 0
              - AT1 - Bit 1
              - AT2 - Bit 2
              - AT2 - Bit 3

\return       none

\author       WK
\date         11.02.2010

***************************************************************************** */
CSMD_VOID CSMD_HAL_EnableTelegrams( CSMD_HAL   *prCSMD_HAL,
                                    CSMD_ULONG  ulMDT_Enable,
                                    CSMD_ULONG  ulAT_Enable )
{
  prCSMD_HAL->rRegCopy.ulSFCR =
      (prCSMD_HAL->rRegCopy.ulSFCR & ~CSMD_HAL_SFCR_ENABLE_TEL_MASK)
    | ((ulMDT_Enable & CSMD_HAL_ENABLE_TEL_MASK) << CSMD_HAL_SFCR_SHIFT_MDT)
    | ((ulAT_Enable  & CSMD_HAL_ENABLE_TEL_MASK) << CSMD_HAL_SFCR_SHIFT_AT);
  
  prCSMD_HAL->prSERC_Reg->ulSFCR = CSMD_END_CONV_L( prCSMD_HAL->rRegCopy.ulSFCR );

} /* end: CSMD_HAL_EnableTelegrams () */



/**************************************************************************/ /**
\brief Enables/disables the counters TCNT, TCNT[1] and TCNT[2].

\ingroup module_phase
\b Description: \n
   This function either enables or disables the main timer TCNT and the port-specific
   timers TCNT[1] and TCNT[2] writing to the corresponding bits of the register TCSR.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   boEnableTimer0
              Enable main timer 0 (TCNT)
\param [in]   boEnableTimer1
              Enable timer for port 1 (TCNT[1])
\param [in]   boEnableTimer2
              Enable timer for port 2 (TCNT[2])
              
\return       none

\author       WK
\date         11.02.2010

***************************************************************************** */
CSMD_VOID CSMD_HAL_CtrlSystemCounter( CSMD_HAL  *prCSMD_HAL,
                                      CSMD_BOOL  boEnableTimer0,
                                      CSMD_BOOL  boEnableTimer1,
                                      CSMD_BOOL  boEnableTimer2 )
{
  
  CSMD_ULONG ulTCSR =
    prCSMD_HAL->prSERC_Reg->ulTCSR & (CSMD_ULONG) ~(  (1 << CSMD_HAL_TCSR_ET0)
                                                    | (1 << CSMD_HAL_TCSR_ET1)
                                                    | (1 << CSMD_HAL_TCSR_ET2));
  
  if (boEnableTimer0)  ulTCSR |= (CSMD_ULONG)(1 << CSMD_HAL_TCSR_ET0);
  if (boEnableTimer1)  ulTCSR |= (CSMD_ULONG)(1 << CSMD_HAL_TCSR_ET1);
  if (boEnableTimer2)  ulTCSR |= (CSMD_ULONG)(1 << CSMD_HAL_TCSR_ET2);
  
  prCSMD_HAL->prSERC_Reg->ulTCSR = ulTCSR;

} /* end: CSMD_HAL_CtrlSystemCounter () */



/**************************************************************************/ /**
\brief Enables or disables CYC_CLK.

\ingroup sync_ext_slave
\b Description: \n
   This function either enables or disables the CYCCLK input by writing to the
   corresponding bits of the TCSR register.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
 
\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   boEnable
              TRUE -> enables CYC_CLK input\n
              FALSE -> disables CYC_CLK input

\return       none

\author       WK
\date         05.09.2005 

***************************************************************************** */
CSMD_VOID CSMD_HAL_CtrlCYCCLKInput( CSMD_HAL  *prCSMD_HAL,
                                    CSMD_BOOL  boEnable )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  if (boEnable == TRUE)
  {
    prCSMD_HAL->prSERC_Reg->ulTCSR |= (1 << CSMD_HAL_TCSR_CYCEN);
  }
  else
  {
    prCSMD_HAL->prSERC_Reg->ulTCSR &= ~(1 << CSMD_HAL_TCSR_CYCEN);
  }
  
} /* end: CSMD_HAL_CtrlCYCCLKInput() */



/**************************************************************************/ /**
\brief Configures the CON_CLK output.

\ingroup sync_ext_master
\b Description: \n
   This function configures activation, output driver enabling and polarity
   of the CON_CLK output writing to the corresponding bits of the TCSR register.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
 
\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   boActivate
              TRUE -> enables function CON_CLK\n
              FALSE -> disables function CON_CLK
\param [in]   boEnableDriver
              TRUE -> enables output driver\n
              FALSE -> disables output driver
\param [in]   boPolarity
              TRUE -> CON_CLK is active with falling edge\n
              FALSE -> CON_CLK is active with rising edge

\return       none

\author       WK
\date         05.09.2005 

***************************************************************************** */
CSMD_VOID CSMD_HAL_ConfigCONCLK( CSMD_HAL  *prCSMD_HAL,
                                 CSMD_BOOL  boActivate,
                                 CSMD_BOOL  boEnableDriver,
                                 CSMD_BOOL  boPolarity )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  CSMD_HAL_SERCFPGA_DATTYP     ulTemp;
  
  ulTemp = prCSMD_HAL->prSERC_Reg->ulTCSR 
    & ~(  (1 << CSMD_HAL_TCSR_CONPOL)
        | (1 << CSMD_HAL_TCSR_CONEN)
        | (1 << CSMD_HAL_TCSR_CONOE)
        | (1 << CSMD_HAL_TCSR_DIVCLK_OD));
  
  if (boActivate == TRUE)
  {
    ulTemp |= (  (1 << CSMD_HAL_TCSR_CONEN)       /* Activation of CON_CLK */
               | (1 << CSMD_HAL_TCSR_DIVCLK_OD)); /* DivClk pin is in a high impedance state */
  }
  
  if (boEnableDriver == TRUE)  
  {
    ulTemp |= (1 << CSMD_HAL_TCSR_CONOE);         /* Enable output driver of CON_CLK */
  }
  
  if (boPolarity == TRUE)  
  {
    ulTemp |= (1 << CSMD_HAL_TCSR_CONPOL);        /* Polarity of output CON_CLK */
  }
  
  prCSMD_HAL->prSERC_Reg->ulTCSR = ulTemp;
  
} /* end: CSMD_HAL_ConfigCONCLK () */



/**************************************************************************/ /**
\brief Activates and configures DIVCLK functionality.

\ingroup module_sync
  \b Description: \n
   This function configures the DIV_CLK output, including:
   - activation or deactivation of the functionality
   - cycle ratio
   - polarity
   - pulse distance
   - start delay

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   boActivate
              TRUE -> enables DIV_CLK output\n
              FALSE -> disables DIV_CLK output
\param [in]   boMode
              TRUE  -> cycle division: one pulse after n Sercos cycles\n
              FALSE -> cycle multiplication: n pulses per Sercos cycle
\param [in]   boPolarity
              TRUE -> DIV_CLK is active with falling edge\n
              FALSE -> DIV_CLK is active with rising edge
\param [in]   boOutpDisable
              TRUE  -> DIV_CLK output is disabled (tristated)\n
              FALSE -> DIV_CLK output is enabled. 
\param [in]   usNbrPulses
              Number of pulses
\param [in]   ulPulseDistance
              Distance DTDIVCLK [ns] of DIV_CLK pulses in cycle multiplication mode
\param [in]   ulStartDelay
              Delay time TDIVCLK [ns] for first DIV_CLK pulse after CYC_Start trigger

\return       \ref CSMD_HAL_DIVCLK_TIMES \n
              \ref CSMD_HAL_DIVCLK_PULSES \n
              \ref CSMD_HAL_NO_ERROR \n

\author       WK
\date         16.10.2006 

***************************************************************************** */
CSMD_ULONG CSMD_HAL_ConfigDIVCLK( CSMD_HAL    *prCSMD_HAL,
                                  CSMD_BOOL    boActivate,
                                  CSMD_BOOL    boMode,
                                  CSMD_BOOL    boPolarity,
                                  CSMD_BOOL    boOutpDisable,
                                  CSMD_USHORT  usNbrPulses,
                                  CSMD_ULONG   ulPulseDistance,
                                  CSMD_ULONG   ulStartDelay )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  CSMD_HAL_SERCFPGA_DATTYP     ulTCSR;
  
  
  if (boActivate == TRUE)
  {
    if (   (ulPulseDistance > CSMD_HAL_TSCYC_MAX)
        || (ulStartDelay > CSMD_HAL_TSCYC_MAX))
    {
      return ( (CSMD_ULONG)CSMD_HAL_DIVCLK_TIMES );
    }
    if (usNbrPulses > CSMD_HAL_DIVCLK_MAX_PULSES)
    {
      return ( (CSMD_ULONG)CSMD_HAL_DIVCLK_PULSES );
    }
    
    ulTCSR = prCSMD_HAL->prSERC_Reg->ulTCSR 
      & ~(  (1 << CSMD_HAL_TCSR_DIVCLK_POL) 
          | (1 << CSMD_HAL_TCSR_DIVCLK_MODE)
          | (1 << CSMD_HAL_TCSR_DIVCLK_OD));
    
    /* Mode of DIV_CLK output */
    if (boMode == TRUE)  
    {
      ulTCSR |= (1 << CSMD_HAL_TCSR_DIVCLK_MODE);
    }
    /* Polarity of output DIV_CLK */
    if (boPolarity == TRUE)  
    {
      ulTCSR |= (1 << CSMD_HAL_TCSR_DIVCLK_POL);   
    }
    
    /* distance between two pulses */
    prCSMD_HAL->prSERC_Reg->ulDTDIVCLK = 
      CSMD_END_CONV_L( ulPulseDistance );
    
    /* number of pulses & delay time  */
    prCSMD_HAL->prSERC_Reg->ulTDIVCLK_NDIVCLK = 
      CSMD_END_CONV_L( (((CSMD_ULONG)usNbrPulses << 24) | (ulStartDelay & 0x00FFFFFF)) );
    
    /* DIV_CLK output disable */
    if (boOutpDisable == TRUE)
    {
      ulTCSR |= (1 << CSMD_HAL_TCSR_DIVCLK_OD); /* DivClk pin is in a high impedance state */
    }
    prCSMD_HAL->prSERC_Reg->ulTCSR = ulTCSR;
  }
  else
  {
    /* DIV_CLK output disable */
    ulTCSR = prCSMD_HAL->prSERC_Reg->ulTCSR & ~(1 << CSMD_HAL_TCSR_DIVCLK_OD);
    if (boOutpDisable == TRUE)
    {
      ulTCSR |= (1 << CSMD_HAL_TCSR_DIVCLK_OD);
    }
    prCSMD_HAL->prSERC_Reg->ulTCSR = ulTCSR;
    
    /* deactivate DIV_CLK function by setting nbr of pulses to 0 */
    prCSMD_HAL->prSERC_Reg->ulTDIVCLK_NDIVCLK = 0;
  }
  
  return ( (CSMD_ULONG)CSMD_HAL_NO_ERROR );
  
} /* end: CSMD_HAL_ConfigDIVCLK () */



/**************************************************************************/ /**
\brief Activates and configures CYCCLK functionality.

\ingroup sync_ext_slave
\b Description: \n
   This function configures the CYC_CLK input, including:
   - activation or deactivation of the functionality
   - polarity
   - start delay

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   boActivate
              TRUE  -> enables function CYC_CLK\n
              FALSE -> disables function CYC_CLK
\param [in]   boEnableInput
              TRUE  -> enables CYC_CLK input\n
              FALSE -> disables CYC_CLK input
\param [in]   boPolarity
              TRUE  -> CYC_CLK is active with falling edge.\n
              FALSE -> CYC_CLK is active with rising edge.
\param [in]   ulStartDelay
              Delay time TCYCSTART [ns] for timer start, triggered by CYC_Start.

\return       none

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_VOID CSMD_HAL_ConfigCYCCLK( CSMD_HAL   *prCSMD_HAL,
                                 CSMD_BOOL   boActivate,
                                 CSMD_BOOL   boEnableInput,
                                 CSMD_BOOL   boPolarity,
                                 CSMD_ULONG  ulStartDelay )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  CSMD_HAL_SERCFPGA_DATTYP     ulTCSR;
  
  prCSMD_HAL->prSERC_Reg->ulTCYCSTART = CSMD_END_CONV_L( ulStartDelay );
  
  ulTCSR = prCSMD_HAL->prSERC_Reg->ulTCSR 
    & ~((1 << CSMD_HAL_TCSR_M_S)   | 
        (1 << CSMD_HAL_TCSR_CYCEN) | 
        (1 << CSMD_HAL_TCSR_CYCPOL));
  
  if (boActivate == TRUE)
  {
    ulTCSR |= (1 << CSMD_HAL_TCSR_M_S);       /* enable CYC_CLK (timing slave mode) */
  }
  
  if (boEnableInput == TRUE)
  {
    ulTCSR |= (1 << CSMD_HAL_TCSR_CYCEN);     /* Activation of input CYC_CLK */
  }
  
  if (boPolarity == TRUE)
  {
    ulTCSR |= (1 << CSMD_HAL_TCSR_CYCPOL);    /* Polarity of input CYC_CLK */
  }
  prCSMD_HAL->prSERC_Reg->ulTCSR = ulTCSR;
  
} /* end: CSMD_HAL_ConfigCYCCLK() */



/**************************************************************************/ /**
\brief Enables or disables the descriptor functionality.

\ingroup module_phase
\b Description: \n
   Handling the descriptor functionality, this function can also enable / disable
   transmission and reception of telegrams

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   boTxEnable
              TRUE  -> Tx descriptor functionality enabled\n
              FALSE -> Tx descriptor functionality disabled
\param [in]   boRxEnable
              TRUE  -> Rx descriptor functionality enabled\n
              FALSE -> Rx descriptor functionality disabled
 
\return       none

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_VOID CSMD_HAL_CtrlDescriptorUnit( CSMD_HAL  *prCSMD_HAL,
                                       CSMD_BOOL  boTxEnable,
                                       CSMD_BOOL  boRxEnable )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  volatile CSMD_ULONG   ulDFCSR;    /* change register consistent! */
  /*Enable/Disable Tx/Rx descriptor unit*/
  
  ulDFCSR = prCSMD_HAL->prSERC_Reg->rDFCSR.ulLong;
  if (boTxEnable)
  {
    ulDFCSR |= (CSMD_ULONG) (1 << CSMD_HAL_DFCSR_TX_ENABLE);
  }
  else
  {
    ulDFCSR &= (CSMD_ULONG) ~(1 << CSMD_HAL_DFCSR_TX_ENABLE);
  }        
  
  if (boRxEnable)
  {   
    ulDFCSR |= (CSMD_ULONG) (1 << CSMD_HAL_DFCSR_RX_ENABLE);
  }
  else
  {
    ulDFCSR &= (CSMD_ULONG) ~(1 << CSMD_HAL_DFCSR_RX_ENABLE);
  }
  prCSMD_HAL->prSERC_Reg->rDFCSR.ulLong = ulDFCSR;
  
} /* end: CSMD_HAL_CtrlDescriptorUnit () */



/**************************************************************************/ /**
\brief Sets the Sercos time.

\ingroup func_sercostime
\b Description: \n
   Sercos time demanded by the master is written to the Sercos time registers
   STNS, STSEC, STNS_BUS and STSEC_BUS

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulNanos
              Post decimal positions of newly set Sercos time in nanoseconds,
              written to register STNS
\param [in]   ulSeconds 
              Newly set Sercos time in seconds, written to register STSEC
\param [in]   ulNanos_plus     
              Post decimal positions of the precalculated new Sercos time,
              written to register STNS_BUS
\param [in]   ulSeconds_plus
              Precalculated new Sercos time in seconds, written to register STSEC_BUS

\return       none

\author       AM
\date         22.06.2010

***************************************************************************** */
CSMD_VOID CSMD_HAL_SetSercosTime( CSMD_HAL   *prCSMD_HAL,
                                  CSMD_ULONG  ulNanos,
                                  CSMD_ULONG  ulSeconds,
                                  CSMD_ULONG  ulNanos_plus,
                                  CSMD_ULONG  ulSeconds_plus )
{

  /* disable Sercos time by setting TCSR Bit 3 to zero in order to allow
     changes in Sercos time registers */
  prCSMD_HAL->prSERC_Reg->ulTCSR &= ~(1UL << CSMD_HAL_TCSR_EST);
  
  /* apply Sercos time registers */
  prCSMD_HAL->prSERC_Reg->ulSTNS   = CSMD_END_CONV_L( ulNanos );
  prCSMD_HAL->prSERC_Reg->ulSTSEC  = CSMD_END_CONV_L( ulSeconds );
  prCSMD_HAL->prSERC_Reg->ulSTNSP  = CSMD_END_CONV_L( ulNanos_plus );
  prCSMD_HAL->prSERC_Reg->ulSTSECP = CSMD_END_CONV_L( ulSeconds_plus );
  
  /* (Re-)enable Sercos time by setting TCSR bit 3 */
  prCSMD_HAL->prSERC_Reg->ulTCSR |= (1UL << CSMD_HAL_TCSR_EST);
  
  /* declare new Sercos time by toggling TCSR bit 15 */
  prCSMD_HAL->prSERC_Reg->ulTCSR ^= (1UL << CSMD_HAL_TCSR_SYS_TIME_TOGGLE);

} /* end: CSMD_HAL_SetSercosTime () */


/**************************************************************************/ /**
\brief This HAL function sets the Sercos time.

\ingroup module_sercostime
\b Description: \n
        Sercos time demanded by the master is written to the Sercos time
        registers STNSOffset, STNSNew, STSECNew.
        Disabling of system time (TCSR bit 3) is not required!

\note   Sercos IP-Code > 4.9 is required!

<B>Call Environment:</B> \n
        This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulNanos
              Post decimal positions of newly set Sercos time in nanoseconds,
              written to register STNSNew.
\param [in]   ulSeconds
              Newly set Sercos time in seconds, written to register STSECNew.
\param [in]   ulNanos_Offset
              Pecalculated offset compensating the transmission time 
              relating to TSref.

\return       none

\author       WK
\date         21.10.2014

***************************************************************************** */
CSMD_VOID CSMD_HAL_SetSercosTimeExtSync( const CSMD_HAL *prCSMD_HAL,
                                         CSMD_ULONG      ulNanos,
                                         CSMD_ULONG      ulSeconds,
                                         CSMD_ULONG      ulNanos_Offset )
{
  /* Enable Sercos time by setting TCSR bit 3 */
  prCSMD_HAL->prSERC_Reg->ulTCSR |= (1UL << CSMD_HAL_TCSR_EST);

  /* apply Sercos time registers */
  prCSMD_HAL->prSERC_Reg->ulSTNSNew    = CSMD_END_CONV_L( ulNanos );
  prCSMD_HAL->prSERC_Reg->ulSTSECNew   = CSMD_END_CONV_L( ulSeconds );
  prCSMD_HAL->prSERC_Reg->ulSTNSOffset = CSMD_END_CONV_L( ulNanos_Offset );

  /* declare new Sercos time by toggling TCSR bit 15 */
  prCSMD_HAL->prSERC_Reg->ulTCSR ^= (1UL << CSMD_HAL_TCSR_SYS_TIME_TOGGLE);

} /* end: CSMD_HAL_SetSercosTimeExtSync () */


/**************************************************************************/ /**
\brief Reads the current Sercos time.

\ingroup func_sercostime
\b Description: \n
   This function provides two pointers returning the current Sercos time in
   seconds and nanoseconds to the application

   *** aus CSMD_Get_Sercos_Time() ***
   This function performs a write access to register STNS and thus causes
   a readback of the current Sercos time stored in the registers
   STNS_BUS and STSEC_BUS to itself (ns) and the register STSEC (seconds).
   The result can be read from two unsigned long variables.


<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   pulNsAdd
              Address where post decimal positions of read Sercos time
              in nanoseconds are written to
\param [in]   pulSecsAdd
              Address where read Sercos time in seconds is written to

\return       none

\author       AM
\date         28.06.2010 

***************************************************************************** */
CSMD_VOID CSMD_HAL_ReadSercosTime( CSMD_HAL   *prCSMD_HAL,
                                   CSMD_ULONG *pulNsAdd,
                                   CSMD_ULONG *pulSecsAdd )
{

#ifdef CSMD_SOFT_MASTER
  if (CSMD_HAL_IsSoftMaster( prCSMD_HAL ) == TRUE)
  {
    /* No action needed! */
  }
  else
#endif
  {
    /* fetch current sercos time values to registers ulSTNS and ulSTSEC
       by performing a write access to ulSTNS */
    prCSMD_HAL->prSERC_Reg->ulSTNS = 0;
  }
  *pulNsAdd   = CSMD_END_CONV_L( prCSMD_HAL->prSERC_Reg->ulSTNS );
  *pulSecsAdd = CSMD_END_CONV_L( prCSMD_HAL->prSERC_Reg->ulSTSEC );
  
} /* end: CSMD_HAL_ReadSercosTime () */



#ifdef CSMD_HW_WATCHDOG
/**************************************************************************/ /**
\brief Reads the current status of the hardware watchdog.

\ingroup func_watchdog
\b Description: \n
   This function returns the current value of WDCNT counter (bits 16-31) as well as
   active- and alarm bit (16 und 17) of the WDCSR-Register.\n
   The result is provided in a structure containing an unsigned short variable
   for the counter and two boolean variables for the status bits.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   prWdStatus
              Pointer to a structure containing the current value of the watchdog
              counter and the two status bits of register WDCSR

\return       none

\author       AM
\date         09.07.2010

***************************************************************************** */
CSMD_VOID CSMD_HAL_WatchdogStatus( CSMD_HAL          *prCSMD_HAL,
                                   CSMD_HAL_WDSTATUS *prWdStatus )
{
  
  CSMD_USHORT usStatus = prCSMD_HAL->prSERC_Reg->rWDCSR.rShort.usControlStatus;
  
  prWdStatus->boActive = (CSMD_HAL_WD_ACTIVE == (usStatus & CSMD_HAL_WD_ACTIVE));
  
  prWdStatus->boAlarm  = (CSMD_HAL_WD_ALARM == (usStatus & CSMD_HAL_WD_ALARM));
  
  prWdStatus->usActCount = 
    CSMD_END_CONV_S( prCSMD_HAL->prSERC_Reg->rWDCNT.rCounter.usActual );

} /* end: CSMD_HAL_WatchdogStatus () */
#endif



/**************************************************************************/ /**
\brief Gets the status of active Rx buffer for both ports of buffer system A.

\ingroup module_buffer
\b Description: \n
   This function reads the status of the active Rx buffer for both ports of buffer
   system A from the corresponding bits of the RXBUFCSR_A register.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pboNewValidBufP1
              Status of the active Rx buffer system A port 1:\n
              TRUE:  Data are valid for all configured ATs. n
              FALSE: Data are not valid.

\param [out]  pboNewValidBufP2
              Status of the active Rx buffer system A port 2:\n
              TRUE:  Data are valid for all configured ATs.\n
              FALSE: Data are not valid.

\return       none
  
\author       WK
\date         25.01.2011

***************************************************************************** */
CSMD_VOID CSMD_HAL_Get_IsValid_Rx_BufferSysA( CSMD_HAL  *prCSMD_HAL,
                                              CSMD_BOOL *pboNewValidBufP1,
                                              CSMD_BOOL *pboNewValidBufP2 )
{
  
  CSMD_HAL_SERCFPGA_DATTYP ulRXBUFCSR_A = prCSMD_HAL->prSERC_Reg->ulRXBUFCSR_A;

  *pboNewValidBufP1 = (   (ulRXBUFCSR_A & CSMD_HAL_RXBUFCSR_NEW_DATA_P1) 
                       == CSMD_HAL_RXBUFCSR_NEW_DATA_P1);
  
  *pboNewValidBufP2 = (   (ulRXBUFCSR_A & CSMD_HAL_RXBUFCSR_NEW_DATA_P2) 
                       == CSMD_HAL_RXBUFCSR_NEW_DATA_P2);
  
} /* end: CSMD_HAL_Get_IsValid_Rx_BufferSysA() */



/**************************************************************************/ /**
\brief Reads one of the telegram status registers TGSR1 or TGSR2.

\ingroup cyclic
\b Description: \n
   In this function telegram information according to the selected port is
   read and returned to CoSeMa. The information order has to be regarded for the
   return parameter.

   Telegram Status Register TGSR1 / TGSR2:

   Name                | Bits  | Size | Operation  | Description
   ------------------- | ----- | ---- | ---------- | -----------
   MDT n               | 0-3   | 1    | Read/Write | MDT n received, FCS valid
   AT n                | 4-7   | 1    | R/W        | AT n received, FCS valid
   MST Valid           | 8     | 1    | R/W        | MST received, CRC valid
   primary / secondary | 9     | 1    | R/W        | Primary / Secondary telegram
   MST Window Error    | 10    | 1    | R/W        | MST received, CRC valid, out of MST receive window
   MST Miss            | 11    | 1    | R/W        | MST missed or CRC invalid
   MST double miss     | 12    | 1    | R/W        | MST was missed for 2 times consecutively
   AT0 miss            | 13    | 1    | R/W        | AT0 header missed inside AT0 window or CRC invalid
   Cycle count         | 24-26 | 3    | Read       | Cycle count from the MST header of MDT0
   Cycle count valid   | 27    | 1    | R          | Cycle count valid bit from the MST header of MDT0
   First MST           | 28    | 1    | R          | Indicates that the first MST arrives at this port

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usPort
              1 -> telegram status port 1\n
              2 -> telegram status port 2

\return       Telegram status (see table)

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_ULONG CSMD_HAL_GetTelegramStatus( CSMD_HAL    *prCSMD_HAL,
                                       CSMD_USHORT  usPort )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  if (usPort == 1)
  {
    return ((CSMD_ULONG) CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulTGSR1) ));
  }
  else if (usPort == 2)
  {
    return ((CSMD_ULONG) CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulTGSR2) ));
  }
  else
  {
    return (0);
  }
  
} /* end: CSMD_HAL_GetTelegramStatus() */



/**************************************************************************/ /**
\brief Clears the telegram status information.

\ingroup module_cyclic
\b Description: \n
   The information corresponding to the bits set in the mask usClrStatusMask
   is cleared by this function. Only the status bits (0-15) of the TGSR
   are erasable.

   Telegram Status Register TGSR1 / TGSR2:

   Name                | Bits  | Size | Operation  | Description
   ------------------- | ----- | ---- | ---------- | -----------
   MDT n               | 0-3   | 1    | Read/Write | MDT n received, FCS valid
   AT n                | 4-7   | 1    | R/W        | AT n received, FCS valid
   MST Valid           | 8     | 1    | R/W        | MST received, CRC valid
   primary / secondary | 9     | 1    | R/W        | Primary / Secondary telegram
   MST Window Error    | 10    | 1    | R/W        | MST received, CRC valid, out of MST receive window
   MST Miss            | 11    | 1    | R/W        | MST missed or CRC invalid
   MST double miss     | 12    | 1    | R/W        | MST was missed for 2 times consecutively
   AT0 miss            | 13    | 1    | R/W        | AT0 header missed inside AT0 window or CRC invalid
   Cycle count         | 24-26 | 3    | Read       | Cycle count from the MST header of MDT0
   Cycle count valid   | 27    | 1    | R          | Cycle count valid bit from the MST header of MDT0
   First MST           | 28    | 1    | R          | Indicates that the first MST arrives at this port

   Notes:
   - All "MDT n" bits are cleared with writing 1 to bit "MDT 0".
   - All "AT n" bits are cleared with writing 1 to bit "AT 0".

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usClrStatusMask
              Mask with status information to be cleared
\param [in]   usPort
              1 -> telegramstatus for port 1\n
              2 -> telegramstatus for port 2

\return       none

\author       KP
\date         13.08.2007 
 
***************************************************************************** */
CSMD_VOID CSMD_HAL_ClearTelegramStatus ( CSMD_HAL    *prCSMD_HAL,
                                         CSMD_USHORT  usClrStatusMask,
                                         CSMD_USHORT  usPort )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  if (usPort == 1)
  {
    prCSMD_HAL->prSERC_Reg->ulTGSR1 = CSMD_END_CONV_L( ((CSMD_ULONG)usClrStatusMask) );
  }
  else if (usPort == 2)
  {
    prCSMD_HAL->prSERC_Reg->ulTGSR2 = CSMD_END_CONV_L( ((CSMD_ULONG)usClrStatusMask) );
  }
  else
  {}
  
} /* end: CSMD_HAL_ClearTelegramStatus() */



/**************************************************************************/ /**
\brief Reads the ring run time.

\ingroup func_timing
\b Description: \n
   This function reads the ring run time from the counter in FPGA register
   TMR1 or TMR2 which is triggered by a valid MST on the corresponding port.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usPort
              1 -> ring run time for port 1\n
              2 -> ring run time for port 2
 
\return       ring run time in nanoseconds

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_ULONG CSMD_HAL_GetRingRuntimeMeasure( CSMD_HAL    *prCSMD_HAL,
                                           CSMD_USHORT  usPort )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  if (usPort == 1)
  {
    return ((CSMD_ULONG) (CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulRDLY1) )));
  }
  else if (usPort == 2)
  {
    return ((CSMD_ULONG) (CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulRDLY2) )));
  }
  else
  {
    return (0U);
  }
  
} /* end: CSMD_HAL_GetRingRuntimeMeasure() */



/**************************************************************************/ /**
\brief Sets the port of SVC machine trigger.

\ingroup module_phase
\b Description: \n
   This function sets the port of the SVC machine trigger writing to the
   corresponding bit of the SVCCSR register.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usPort
              1 -> SVC is triggered via port 1\n
              2 -> SVC is triggered via port 2
 
\return       none

\author       KP
\date         13.08.2007 
 
***************************************************************************** */
CSMD_VOID CSMD_HAL_SetSVCPort( CSMD_HAL    *prCSMD_HAL,
                               CSMD_USHORT  usPort )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  /* trigger SVC machine from port 2 */
  if (usPort == 2)
  {   
    prCSMD_HAL->prSERC_Reg->ulSVCCSR |= (CSMD_ULONG)(1 << CSMD_HAL_SHIFT_SVCCSR_TRIG_PORT);
  }
  /* trigger SVC machine from port 1 */
  if (usPort == 1)
  {        
    prCSMD_HAL->prSERC_Reg->ulSVCCSR &= (CSMD_ULONG)~(1 << CSMD_HAL_SHIFT_SVCCSR_TRIG_PORT); 
  }

}/* end: CSMD_HAL_SetSVCPort() */



/**************************************************************************/ /**
\brief Configures the Tx buffer system.

\ingroup module_buffer
\b Description: \n
   With this function the Tx buffer system (A/B) and the number of buffers
   to be used can be selected.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulSystem
              System selection:\n
              0 = system A\n
              1 = system B
\param [in]   ulNumber
              Buffer selection:\n
              0 = single buffer system\n
              1 = double buffer system\n
              2 = triple buffer system\n
              3 = quad buffer sytsem

\return       none

\author       WK
\date         19.05.2010

***************************************************************************** */
CSMD_VOID CSMD_HAL_Configure_Tx_Buffer( CSMD_HAL   *prCSMD_HAL,
                                        CSMD_ULONG  ulSystem,
                                        CSMD_ULONG  ulNumber )
{
  
  if (ulSystem == CSMD_HAL_TX_BUFFER_SYSTEM_A)
  {
    prCSMD_HAL->prSERC_Reg->ulTXBUFCSR_A = 
      CSMD_END_CONV_L( ulNumber & CSMD_HAL_TXBUFCSR_COUNT_MASK);
  }
  else
  {
    prCSMD_HAL->prSERC_Reg->ulTXBUFCSR_B = 
      CSMD_END_CONV_L( ulNumber & CSMD_HAL_TXBUFCSR_COUNT_MASK);
  }
  
} /* end: CSMD_HAL_Configure_Tx_Buffer() */



/**************************************************************************/ /**
\brief Configures the Rx buffer system.

\ingroup module_buffer
\b Description: \n
   This function selects an Rx buffer system (A or B) and the number of
   buffers to be used writing to the corresponding bits of the registers
   RXBUFCSR_A or RXBUFCSR_B.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in] prCSMD_HAL
            Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in] ulSystem
            System selection:\n
            0 = system A\n
            1 = system B
\param [in] ulNumber
            Buffer selection:\n
            0 = single buffer system\n
            1 = double buffer system\n
            2 = triple buffer system

\return     none

\author     WK
\date       25.05.2010

***************************************************************************** */
CSMD_VOID CSMD_HAL_Configure_Rx_Buffer( CSMD_HAL   *prCSMD_HAL,
                                        CSMD_ULONG  ulSystem,
                                        CSMD_ULONG  ulNumber )
{
  
  if (ulSystem == CSMD_HAL_RX_BUFFER_SYSTEM_A)
  {
    prCSMD_HAL->prSERC_Reg->ulRXBUFCSR_A = 
      CSMD_END_CONV_L( ulNumber & CSMD_HAL_RXBUFCSR_COUNT_MASK);
  }
  else
  {
    prCSMD_HAL->prSERC_Reg->ulRXBUFCSR_B = 
      CSMD_END_CONV_L( ulNumber & CSMD_HAL_RXBUFCSR_COUNT_MASK);
  }
  
} /* end: CSMD_HAL_Configure_Rx_Buffer() */



/**************************************************************************/ /**
\brief Configures the required telegrams for Rx Buffer Telegram valid register.

\ingroup module_buffer
\b Description: \n
   This function configures the required telegrams for Rx Buffer Telegram
   valid register writing to the corresponding bits of either the RXBUFCSR_A
   or RXBUFCSR_B register.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulSystem
              System selection:\n
              0 = system A\n
              1 = system B
\param [in]   ulReqMDTs
              Required MDTs for Telegram valid regsiter:\n
              Bit0 = MDT0\n
              Bit2 = MDT1\n
              Bit3 = MDT2\n
              Bit4 = MDT3
\param [in]   ulReqATs
              Required ATs for Telegram valid regsiter:\n
              Bit0 = AT0\n
              Bit2 = AT1\n
              Bit3 = AT2\n
              Bit4 = AT3

\return       none

\author       WK
\date         15.02.2011

***************************************************************************** */
CSMD_VOID CSMD_HAL_Config_Rx_Buffer_Valid( CSMD_HAL   *prCSMD_HAL,
                                           CSMD_ULONG  ulSystem,
                                           CSMD_ULONG  ulReqMDTs,
                                           CSMD_ULONG  ulReqATs )
{
  
  if (ulSystem == CSMD_HAL_RX_BUFFER_SYSTEM_A)
  {
    prCSMD_HAL->prSERC_Reg->ulRXBUFTR_A = 
      CSMD_END_CONV_L( (  ((ulReqMDTs << CSMD_HAL_RXBUFTR_MDT_SHIFT) & CSMD_HAL_RXBUFTR_MDT_MASK)
                        | ((ulReqATs  << CSMD_HAL_RXBUFTR_AT_SHIFT)  & CSMD_HAL_RXBUFTR_AT_MASK) )
                       );
  }
  else
  {
    prCSMD_HAL->prSERC_Reg->ulRXBUFTR_B = 
      CSMD_END_CONV_L( (  ((ulReqMDTs << CSMD_HAL_RXBUFTR_MDT_SHIFT) & CSMD_HAL_RXBUFTR_MDT_MASK)
                        | ((ulReqATs  << CSMD_HAL_RXBUFTR_AT_SHIFT)  & CSMD_HAL_RXBUFTR_AT_MASK) )
                       );
  }
  
} /* end: CSMD_HAL_Config_Rx_Buffer_Valid() */



/**************************************************************************/ /**
\brief Gets the active Rx buffer number for both ports of buffer system A.

\ingroup module_buffer
\b Description: \n
   This function returns the active Rx buffer number of buffer system A
   that is usable by the system (processor).

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pusBufP1
              Active Rx buffer number port 1 for system use:\n
              0 = buffer 0\n
              1 = buffer 1\n
              2 = buffer 2
\param [out]  pusBufP2
              Active Rx buffer number port 2 for system use:\n
              0 = buffer 0\n 
              1 = buffer 1\n
              2 = buffer 2

\return       none

\author       WK
\date         25.01.2011

***************************************************************************** */
CSMD_VOID CSMD_HAL_Usable_Rx_BufferSysA( CSMD_HAL    *prCSMD_HAL,
                                         CSMD_USHORT *pusBufP1,
                                         CSMD_USHORT *pusBufP2 )
{
  
  CSMD_HAL_SERCFPGA_DATTYP ulRXBUFCSR;
  
  ulRXBUFCSR = CSMD_END_CONV_L( prCSMD_HAL->prSERC_Reg->ulRXBUFCSR_A );

  *pusBufP1 = (CSMD_USHORT) (   (ulRXBUFCSR & CSMD_HAL_RXBUFCSR_ACT_BUF_P1_MASK) 
                        >> CSMD_HAL_RXBUFCSR_ACT_BUF_P1_SHIFT);
  
  *pusBufP2 = (CSMD_USHORT) (   (ulRXBUFCSR & CSMD_HAL_RXBUFCSR_ACT_BUF_P2_MASK) 
                        >> CSMD_HAL_RXBUFCSR_ACT_BUF_P2_SHIFT);
  
} /* end: CSMD_HAL_Usable_Rx_Buffer() */



/**************************************************************************/ /**
\brief Reads the telegram and error counters from FPGA.

\ingroup func_diag
\b Description: \n
   This function reads the following counters from the corresponding FPGA registers:
   - frames received without an error
   - frames transmitted
   - frames received with FCS error
   - frames received with alignment error
   - received frames discarded due to insufficient Rx buffer resources
   - received frames discarded due to insufficient Rx buffer resources
   - frames violating the UC channel window
   - frames received with erroneous FCS or misaligned frames

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usPortNbr
              1 = Port 1\n
              2 = Port 2
\param [in]   prCommCounter
              Pointer to Sercos list with communication counter

\return       None

\author       WK
\date         08.12.2006

***************************************************************************** */
CSMD_VOID  CSMD_HAL_GetCommCounter( CSMD_HAL              *prCSMD_HAL,
                                    CSMD_USHORT            usPortNbr,
                                    CSMD_HAL_COMM_COUNTER *prCommCounter )
{
  
  switch (usPortNbr)
  {
  case 1:
    prCommCounter->usRealLen   = sizeof (CSMD_HAL_COMM_COUNTER) - 4;
    prCommCounter->usMaxLen    = sizeof (CSMD_HAL_COMM_COUNTER) - 4;
    
    /* Counter for error-free received frames */
    prCommCounter->usIPFRXOK   = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPFRXOK.rErrCnt.usPort1) );
    /* Counter for transmitted frames */
    prCommCounter->usIPFTXOK   = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPFTXOK.rErrCnt.usPort1) );
    
    /* Counter for received Ethernet frames with FCS error */
    prCommCounter->usIPFCSERR  = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPFCSERR.rErrCnt.usPort1) );
    /* Counter for received Ethernet frames with an alignment error */
    prCommCounter->usIPALGNERR = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPALGNERR.rErrCnt.usPort1) );
    /* Counter for discarded receive Ethernet frames based on missing rx buffer resources */
    prCommCounter->usIPDISRXB  = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPDISRXB.rErrCnt.usPort1) );
    /* Counter for discarded forwarding Ethernet frames based on missing collision buffer resources */
    prCommCounter->usIPDISCLB  = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPDISCOLB.rErrCnt.usPort1) );
    /* Counter for Ethernet frames which violate the UC channel window */
    prCommCounter->usIPCHVIOL  = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPCHVIOL.rErrCnt.usPort1) );
    /* Counter for Ethernet frames with a wrong FCS or are misaligned (resettable) */
    prCommCounter->usIPSERCERR = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPSERCERR.rErrCnt.usPort1) );
    break;
    
  case 2:
    prCommCounter->usRealLen   = sizeof (CSMD_HAL_COMM_COUNTER) - 4;
    prCommCounter->usMaxLen    = sizeof (CSMD_HAL_COMM_COUNTER) - 4;
    
    /* Counter for error-free received frames */
    prCommCounter->usIPFRXOK   = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPFRXOK.rErrCnt.usPort2) );
    /* Counter for transmitted frames */
    prCommCounter->usIPFTXOK   = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPFTXOK.rErrCnt.usPort2) );
    
    /* Counter for received Ethernet frames with FCS error */
    prCommCounter->usIPFCSERR  = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPFCSERR.rErrCnt.usPort2) );
    /* Counter for received Ethernet frames with an alignment error */
    prCommCounter->usIPALGNERR = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPALGNERR.rErrCnt.usPort2) );
    /* Counter for discarded receive Ethernet frames based on missing rx buffer resources */
    prCommCounter->usIPDISRXB  = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPDISRXB.rErrCnt.usPort2) );
    /* Counter for discarded forwarding Ethernet frames based on missing collision buffer resources */
    prCommCounter->usIPDISCLB  = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPDISCOLB.rErrCnt.usPort2) );
    /* Counter for Ethernet frames which violate the UC channel window */
    prCommCounter->usIPCHVIOL  = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPCHVIOL.rErrCnt.usPort2) );
    /* Counter for Ethernet frames with a wrong FCS or are misaligned (resettable) */
    prCommCounter->usIPSERCERR = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPSERCERR.rErrCnt.usPort2) );
    break;
    
  default:
    prCommCounter->usRealLen   = 0;
    prCommCounter->usMaxLen    = 0;
    break;
  } /* End: switch (usPortNbr) */

} /* end: CSMD_HAL_GetCommCounter() */



#ifdef CSMD_ACTIVATE_AUTONEGOTIATION
/**************************************************************************/ /**
\brief Reads the specified register and phy unit via the
       media independent interface (MII).

\ingroup module_init
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usPhyAddr
              address of selected PHY.
\param [in]   usPhyReg
              Phy register address.

\return       Content of the phy register. If no Phy is connected at the given
              Phy address, 0xFFFF should be read (returned??)

\author       WK
\date         13.08.2007 

***************************************************************************** */
CSMD_ULONG CSMD_HAL_ReadMiiPhy( CSMD_HAL    *prCSMD_HAL,
                                CSMD_USHORT  usPhyAddr,
                                CSMD_USHORT  usPhyReg )
{
  
  CSMD_USHORT  mask;
  CSMD_USHORT  data;
  int     i;
  
  /* Base address of MII Register */
  volatile CSMD_USHORT  * pusPhyAccReg = &prCSMD_HAL->prSERC_Reg->rMIICSR.rShort.LowWord;
  
  
  CSMD_USHORT  mdc;      /* select clock pin for port1 or port 2 */
  CSMD_USHORT  mdio;     /* select data IO pin for port1 or port 2 */
  CSMD_USHORT  mdio_en;  /* select direction of io pin for port1 or port 2 */
  
  /*******************************************
  * ident selected port via phy_addr
  ********************************************/
  if (usPhyAddr == PHY_PORT1) /* test exchange */
  {
    /* all pin set to high level */
    mdc     = CSMD_END_CONV_S( CSMD_PHY_PORT1_MDC );        /* clock pin flag for port 1      */
    mdio    = CSMD_END_CONV_S( CSMD_PHY_PORT1_MDIO );       /* data IO pin flag for port 1    */
    mdio_en = CSMD_END_CONV_S( CSMD_PHY_PORT1_MDIO_EN );    /* Enable access flag for port 1  */
  }
  else /* usPhyAddr == PHY_PORT2 */
  {
    mdc     = CSMD_END_CONV_S( CSMD_PHY_PORT2_MDC );        /* clock pin flag for port 2      */
    mdio    = CSMD_END_CONV_S( CSMD_PHY_PORT2_MDIO );       /* data IO pin flag for port 2    */
    mdio_en = CSMD_END_CONV_S( CSMD_PHY_PORT2_MDIO_EN );    /* Enable access flag for port 2  */
  }
  
  /*******************************************
  * Switch the Data IO pin to output for us,
  * by setting the access bit to high level
  * we can write to the internal registers
  *********************************************/
  *pusPhyAccReg |= mdio_en; /* set bit */
  
  /***********************************************
  * Clear clock pin flag  , set data io pin flag
  ***********************************************/
  *pusPhyAccReg &= (CSMD_USHORT)~mdc;
  *pusPhyAccReg |= mdio;
  
  /**************************************
  * clock input pin 32 times in order
  * to write mdio=1 into the device
  ***************************************/
  for ( i = 0; i < 64; i++ )  
    *pusPhyAccReg ^= mdc;
  
  /***************************************************
  *                                 (I)   (II)
  *                                  _____
  * generate operation start puls __/     \____
  *
  ***************************************************/
  *pusPhyAccReg &= (CSMD_USHORT)~mdio;   /* high to low                */
  *pusPhyAccReg ^= mdc;     /* high to low                */
  *pusPhyAccReg ^= mdc;     /* low to high  triggers (I)  */
  *pusPhyAccReg |= mdio;    /* low to high                */
  *pusPhyAccReg ^= mdc;     /* high to low                */
  *pusPhyAccReg ^= mdc;     /* low to high  triggers (II) */
  
  /***********************************************************
  *                                      (I) (II) 
  *                                     ________   read
  * generate operation code = READ  XXX __/     \_________
  *
  ***********************************************************/
  *pusPhyAccReg |= mdio;   /* XXXX to high                */
  *pusPhyAccReg ^= mdc;     /* high to low                */
  *pusPhyAccReg ^= mdc;     /* low to high  triggers (I)  */
  *pusPhyAccReg &= (CSMD_USHORT)~mdio;   /* high to low                */
  *pusPhyAccReg ^= mdc;     /* high to low                */
  *pusPhyAccReg ^= mdc;     /* low to high  triggers (II) (read-cycle) */
  
  /*****************************************
  * phy unit address programming 
  * by scanning all possible 16 addresses
  * via five address bits
  *******************************************/
  mask = 0x0010;
  for (i = 0; i < 5; i++)
  {
    if ( (usPhyAddr & mask) != 0)
      *pusPhyAccReg |= mdio;    /* set pin level to high */
    else 
      *pusPhyAccReg &= (CSMD_USHORT)~mdio;   /* set level to low */
    /* clock it into */
    *pusPhyAccReg ^= mdc;
    *pusPhyAccReg ^= mdc;
    /* scan next unit address bit into */
    mask = (CSMD_USHORT)(mask >> 1);
  }
  
  /**********************************************
  * scan internal phy-register address bits into
  ***********************************************/
  mask = 0x0010;
  for ( i = 0; i < 5; i++ )
  {
    if ((usPhyReg & mask) != 0)
      *pusPhyAccReg |= mdio;
    else 
      *pusPhyAccReg &= (CSMD_USHORT)~mdio;
    *pusPhyAccReg ^= mdc;
    *pusPhyAccReg ^= mdc;
    mask = (CSMD_USHORT)(mask >> 1);
  }
  
  /***************************************************
  * Turn around sequence by setting to read access
  * from phy to us
  ****************************************************/
  *pusPhyAccReg &= (CSMD_USHORT)~mdio_en;    /* set bit */
  /*taskDelay(1); */      /* wait for what time ? , stabilibilty of level*/
  *pusPhyAccReg ^= mdc;
  *pusPhyAccReg ^= mdc; /* clock it into mii */
  
  /**********************************
  * Read the data from phyReg
  * all 16 bits
  ***********************************/
  mask = 0x8000;
  data = 0;
  for (i = 0; i < 16; i++ )
  {
    /* scan data io line */
    *pusPhyAccReg ^= mdc;
    *pusPhyAccReg ^= mdc;
    
    if ((*pusPhyAccReg & mdio) != 0)
      data |= mask;   /* read flag */
    /* scan next flag */
    mask = (CSMD_USHORT)((mask & 0xFFFF) >> 1);
  }
  /***************************************    
  * set mii phy device into idle state
  *****************************************/
  *pusPhyAccReg ^= mdc;
  *pusPhyAccReg ^= mdc;
  
  return (CSMD_ULONG)data;
  
}   /* End: CSMD_HAL_ReadMiiPhy */



/**************************************************************************/ /**
\brief Writes the specified register and phy unit via the
       media independent interface (MII).

\ingroup module_init
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usPhyAddr
              Address of selected PHY
\param [in]   usPhyReg
              Phy register address.
\param [in]   inData
              Value 

\return       none

\author       WK
\date         13.08.2007 

***************************************************************************** */
CSMD_VOID CSMD_HAL_WriteMiiPhy( CSMD_HAL    *prCSMD_HAL,
                                CSMD_USHORT  usPhyAddr,
                                CSMD_USHORT  usPhyReg,
                                CSMD_USHORT  inData )
{
  
  CSMD_USHORT  mask;
  CSMD_USHORT  data;
  int     i;
  
  /*******************************************
  * Assign access pointer 
  *******************************************/
  volatile CSMD_USHORT  * pusPhyAccReg = &prCSMD_HAL->prSERC_Reg->rMIICSR.rShort.LowWord;
  
  CSMD_USHORT  mdc;      /* select clock pin for port1 or port 2 */
  CSMD_USHORT  mdio;     /* select data IO pin for port1 or port 2 */
  CSMD_USHORT  mdio_en;  /* select direction of io pin for port1 or port 2 */
  
  
  /*******************************************
  * ident selected port via phy_addr
  ********************************************/
  if (usPhyAddr == PHY_PORT1) /* test exchange */
  {
    /* all pin set to high level */
    mdc     = CSMD_END_CONV_S( CSMD_PHY_PORT1_MDC );        /* clock pin flag for port 1      */
    mdio    = CSMD_END_CONV_S( CSMD_PHY_PORT1_MDIO );       /* data IO pin flag for port 1    */
    mdio_en = CSMD_END_CONV_S( CSMD_PHY_PORT1_MDIO_EN );    /* Enable access flag for port 1 */
  }
  else /* usPhyAddr == PHY_PORT2 */
  {
    mdc     = CSMD_END_CONV_S( CSMD_PHY_PORT2_MDC );        /* clock pin flag for port 2      */
    mdio    = CSMD_END_CONV_S( CSMD_PHY_PORT2_MDIO );       /* data IO pin flag for port 2    */
    mdio_en = CSMD_END_CONV_S( CSMD_PHY_PORT2_MDIO_EN );    /* Enable access flag for port 2 */
  }
  
  /*******************************************
  * Switch the data IO pin to output for us,
  * by setting the access bit to high level
  * we can write to the internal registers
  *********************************************/
  *pusPhyAccReg |= mdio_en; /* set bit */
  
  /***********************************************
  * Clear clock pin flag  , set data io pin flag
  ***********************************************/
  *pusPhyAccReg &= (CSMD_USHORT)~mdc;
  *pusPhyAccReg |= mdio;
  
  /**************************************
  * clock input pin 32 times in order
  * to write mdio=1 into the device
  ***************************************/
  for ( i = 0; i < 64; i++ )  
    *pusPhyAccReg ^= mdc;
  
  /***************************************************
  *                                 (I)   (II)
  *                                  _____
  * generate operation start puls __/     \____
  *
  ***************************************************/
  *pusPhyAccReg &= (CSMD_USHORT)~mdio;   /* high to low                */
  *pusPhyAccReg ^= mdc;     /* high to low                */
  *pusPhyAccReg ^= mdc;     /* low to high  triggers (I)  */
  *pusPhyAccReg |= mdio;    /* low to high                */
  *pusPhyAccReg ^= mdc;     /* high to low                */
  *pusPhyAccReg ^= mdc;     /* low to high  triggers (II) */
  
  /***********************************************************
  *                                        (I) (II) 
  *                                      __        _________
  * generate operation code = WRITE  XXX __/\_____/ write
  *
  ***********************************************************/
  *pusPhyAccReg &= (CSMD_USHORT)~mdio;   /* XXXX to low                */
  *pusPhyAccReg ^= mdc;     /* high to low                */
  *pusPhyAccReg ^= mdc;     /* low to high  triggers (I)  */
  *pusPhyAccReg |= mdio;    /* low to high                */
  *pusPhyAccReg ^= mdc;     /* high to low                */
  *pusPhyAccReg ^= mdc;     /* low to high  triggers (II) (write-cycle) */
  
  /*****************************************
  * phy unit address programming 
  * by scanning all possible 16 addresses
  * via five address bits
  *******************************************/
  mask = 0x0010;
  for (i = 0; i < 5; i++)
  {
    if ( (usPhyAddr & mask) != 0)
      *pusPhyAccReg |= mdio;    /* set pin level to high */
    else 
      *pusPhyAccReg &= (CSMD_USHORT)~mdio;   /* set level to low */
    /* clock it into */
    *pusPhyAccReg ^= mdc;
    *pusPhyAccReg ^= mdc;
    /* scan next unit address bit into */
    mask = (CSMD_USHORT)(mask >> 1);
  }
  
  /**********************************************
  * scan internal phy-register address bits into
  ***********************************************/
  mask = 0x0010;
  for ( i = 0; i < 5; i++ )
  {
    if ((usPhyReg & mask) != 0)
      *pusPhyAccReg |= mdio;
    else 
      *pusPhyAccReg &= (CSMD_USHORT)~mdio;
    *pusPhyAccReg ^= mdc;
    *pusPhyAccReg ^= mdc;
    mask = (CSMD_USHORT)(mask >> 1);
  }
  
  /***************************************************
  * prog for writescan flags into internal register
  ***************************************************/
  *pusPhyAccReg |= mdio;      /* 1 */
  *pusPhyAccReg ^= mdc;
  *pusPhyAccReg ^= mdc;       /* clock it into mii */
  *pusPhyAccReg &= (CSMD_USHORT)~mdio;  /* 0 */
  *pusPhyAccReg ^= mdc;
  *pusPhyAccReg ^= mdc; /* clock it into mii */
  
  /**********************************
  * write data into
  * scan all 16 bits
  ***********************************/
  mask = 0x8000;
  for (i = 0; i < 16; i++ )
  {
    data = (CSMD_USHORT)(inData & mask);
    if (data != 0) 
      *pusPhyAccReg |= mdio;
    else 
      *pusPhyAccReg &= (CSMD_USHORT)~mdio;
    *pusPhyAccReg ^= mdc;
    *pusPhyAccReg ^= mdc;
    mask = (CSMD_USHORT)((mask & 0xFFFF) >> 1);
  }
  
  /***************************************    
  * set mii phy device into idle state
  *****************************************/
  *pusPhyAccReg ^= mdc;
  *pusPhyAccReg ^= mdc;
  
  
}   /* End: CSMD_HAL_WriteMiiPhy */
#endif  /* #ifdef CSMD_ACTIVATE_AUTONEGOTIATION */



/*###########################################################################*/
/*-----------------------{ MEMORY ACCESS FUNCTIONS }-------------------------*/
/*###########################################################################*/

/**************************************************************************/ /**
\brief Reads a memory section and copies it to another location.

\ingroup module_cyclic
\b Description: \n
   This function copies a given amount of data from a source to a destination address.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   pvDataDes
              Destination address to write data to
\param [in]   pvReadSource
              Source address to read data from
\param [in]   usLength
              Data length [words]

\return       none

\author       KP
\date         21.08.2007 
 
***************************************************************************** */
CSMD_VOID CSMD_HAL_ReadBlock( CSMD_VOID   *pvDataDes,
                              CSMD_VOID   *pvReadSource,
                              CSMD_USHORT  usLength )
{

  (CSMD_VOID) CSMD_HAL_memcpy( pvDataDes, pvReadSource, (CSMD_ULONG)(usLength*2) );

} /* end: CSMD_HAL_ReadBlock() */



/**************************************************************************/ /**
\brief Reads a memory section and writes it to another location.
    
\ingroup module_cyclic
\b Description: \n
   This function copies a given amount of data from a source to a destination address.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   pvWriteDes
              Destination address to write data to
\param [in]   pvDataSource 
              Source address to read data from
\param [in]   usLength
              Data length [words]
 
\return       none

\author       KP
\date         21.08.2007 
 
***************************************************************************** */
CSMD_VOID CSMD_HAL_WriteBlock( CSMD_VOID   *pvWriteDes,
                               CSMD_VOID   *pvDataSource,
                               CSMD_USHORT  usLength )
{

  (CSMD_VOID) CSMD_HAL_memcpy( pvWriteDes, pvDataSource, (CSMD_ULONG)(usLength*2) );

} /* end: CSMD_HAL_WriteBlock() */



/*###########################################################################*/
/*-----------------------{ DESCRIPTOR FUNCTIONS }----------------------------*/
/*###########################################################################*/

/**************************************************************************/ /**
\brief Writes the selected Tx descriptor into FPGA.

\ingroup module_phase
\b Description: \n
   This function writes a Tx descriptor specified by buffer offset, buffer system,
   telegram offset and descriptor type to the specified Tx Ram address in the FPGA.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulTxRamOffset
              Descriptor offset in TxRam
\param [in]   usBufferOffset
              Buffer offset [byte]
\param [in]   usBufSysSel
              Buffer System Select
\param [in]   usTelegramOffset 
              Offset [byte] after CRC in telegram
\param [in]   usDesType
              Tx Descriptor type

\return       none

\author       WK
\date         10.02.2010

***************************************************************************** */
CSMD_VOID CSMD_HAL_SetTxDescriptor( CSMD_HAL    *prCSMD_HAL,
                                    CSMD_ULONG   ulTxRamOffset,
                                    CSMD_USHORT  usBufferOffset,
                                    CSMD_USHORT  usBufSysSel,
                                    CSMD_USHORT  usTelegramOffset,
                                    CSMD_USHORT  usDesType )
{
  
  /* Write current Tx descriptor into TxRam */
  CSMD_HAL_WriteLong( 
    (prCSMD_HAL->prSERC_TX_Ram->aulTx_Ram + (ulTxRamOffset>>2)),
    (   ((CSMD_ULONG)usBufferOffset                                       & CSMD_HAL_DES_MASK_BUFF_OFFS)
     | (((CSMD_ULONG)usBufSysSel      << CSMD_HAL_DES_SHIFT_BUFF_SYS_SEL) & CSMD_HAL_DES_MASK_BUFF_SYS_SEL)
     | (((CSMD_ULONG)usTelegramOffset << CSMD_HAL_DES_SHIFT_TEL_OFFS)     & CSMD_HAL_DES_MASK_TEL_OFFS)
     | (((CSMD_ULONG)usDesType        << CSMD_HAL_DES_SHIFT_TYPE)         & CSMD_HAL_DES_MASK_TYPE)
    ) );
  
} /* end: CSMD_HAL_SetTxDescriptor() */



/**************************************************************************/ /**
\brief Writes the selected Rx descriptor into FPGA.

\ingroup module_phase
\b Description: \n
   This function writes a Rx descriptor specified by buffer offset, buffer system,
   telegram offset and descriptor type to the specified Rx Ram address in the FPGA.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulRxRamOffset
              descriptor offset in RxRam
\param [in]   usBufferOffset
              buffer offset [byte]
\param [in]   usBufSysSel
              buffer System Select
\param [in]   usTelegramOffset
              offset [byte] after CRC in telegram
\param [in]   usDesType
              Rx Descriptor type

\return       none

\author       WK
\date         10.02.2010

***************************************************************************** */
CSMD_VOID CSMD_HAL_SetRxDescriptor( CSMD_HAL    *prCSMD_HAL,
                                    CSMD_ULONG   ulRxRamOffset,
                                    CSMD_USHORT  usBufferOffset,
                                    CSMD_USHORT  usBufSysSel,
                                    CSMD_USHORT  usTelegramOffset,
                                    CSMD_USHORT  usDesType )
{
  
  /* Write current Rx descriptor into TxRam */
  CSMD_HAL_WriteLong( 
    (prCSMD_HAL->prSERC_RX_Ram->aulRx_Ram + (ulRxRamOffset>>2)),
    (   ((CSMD_ULONG)usBufferOffset                                       & CSMD_HAL_DES_MASK_BUFF_OFFS)
     | (((CSMD_ULONG)usBufSysSel      << CSMD_HAL_DES_SHIFT_BUFF_SYS_SEL) & CSMD_HAL_DES_MASK_BUFF_SYS_SEL)
     | (((CSMD_ULONG)usTelegramOffset << CSMD_HAL_DES_SHIFT_TEL_OFFS)     & CSMD_HAL_DES_MASK_TEL_OFFS)
     | (((CSMD_ULONG)usDesType        << CSMD_HAL_DES_SHIFT_TYPE)         & CSMD_HAL_DES_MASK_TYPE)
    ) );
  
} /* end: CSMD_HAL_SetRxDescriptor() */



/* --------------------------------------------- */
/* Replacements for functions from string.h lib  */
/* --------------------------------------------- */

/**************************************************************************/ /**
\brief  Fill block of memory.

\ingroup module_abstract
\b Description: \n
  Sets the first ulSize bytes of the block of memory pointed by pvDest
  to the specified lValue.

<B>Call Environment:</B> \n
  This is a CoSeMa-private function.

\param [in]   pvDest
              Pointer to the block of memory to fill.
\param [in]   lValue
              Value to be set.
\param [in]   ulSize
              Number of bytes to be set to the value.

\return       pvDest

\author       WK
\date         27.03.2014

***************************************************************************** */
CSMD_VOID * CSMD_HAL_memset( CSMD_VOID *pvDest, CSMD_LONG lValue, CSMD_ULONG ulSize )
{
  return memset( pvDest, (CSMD_INT)lValue, (size_t)ulSize );

} /* end: CSMD_HAL_memset() */


/**************************************************************************/ /**
\brief  Copy block of memory.

\ingroup module_abstract
\b Description: \n
  Copies the values of ulSize bytes from the location pointed by pvSource
  directly to the memory block pointed by pvDest.

<B>Call Environment:</B> \n
  This is a CoSeMa-private function.

\param [in]   pvDest
              Pointer to the destination where the content is to be copied.
\param [in]   pvSource
              Pointer to the source of data to be copied.
\param [in]   ulSize
              Number of bytes to copy.

\return       pvDest

\author       WK
\date         27.03.2014

***************************************************************************** */
CSMD_VOID * CSMD_HAL_memcpy( CSMD_VOID *pvDest, const CSMD_VOID *pvSource, CSMD_ULONG ulSize )
{

  return memcpy( pvDest, pvSource, (size_t)ulSize );

} /* end: CSMD_HAL_memcpy() */


/**************************************************************************/ /**
\brief  Copy string.

\ingroup module_abstract
\b Description: \n
  Copies the C string pointed by pccSource into the array pointed by pcDest,
  including the terminating null character.

<B>Call Environment:</B> \n
  This is a CoSeMa-private function.

\param [in]   pcDest
              Pointer to the destination array where the content is to be copied.
\param [in]   pccSource
              C string to be copied.

\return       pcDest

\author       WK
\date         27.03.2014

***************************************************************************** */
CSMD_CHAR * CSMD_HAL_strcpy( CSMD_CHAR *pcDest, const CSMD_CHAR *pccSource )
{
  strcpy( pcDest, pccSource );
  return pcDest;

} /* end: CSMD_HAL_strcpy() */


/**************************************************************************/ /**
\brief  Copy characters from string.

\ingroup module_abstract
\b Description: \n
  Copies the first ulSize characters of pccSource to pcDest.

<B>Call Environment:</B> \n
  This is a CoSeMa-private function.

\param [in]   pcDest
              Pointer to the destination array where the content is to be copied.
\param [in]   pccSource
              C string to be copied.
\param [in]   ulSize
              Maximum number of characters to be copied from pvSource.

\return       pcDest

\author       WK
\date         27.03.2014

***************************************************************************** */
CSMD_CHAR * CSMD_HAL_strncpy( CSMD_CHAR *pcDest, const CSMD_CHAR *pccSource, CSMD_ULONG ulSize )
{

  return strncpy( pcDest, pccSource, (size_t)ulSize );

} /* end: CSMD_HAL_strncpy() */


/**************************************************************************/ /**
\brief  Compare two strings.

\ingroup module_abstract
\b Description: \n
  Compares the C string pccStr1 to the C string pccStr2.

<B>Call Environment:</B> \n
  This is a CoSeMa-private function.

\param [in]   pccStr1
              C string to be compared.
\param [in]   pccStr2
              C string to be compared.
\param [in]   ulSize
              Maximum number of characters to compare.

\return       0   -> both strings are equal.
              <>0 -> error

\author       WK
\date         27.03.2014

***************************************************************************** */
CSMD_LONG CSMD_HAL_strncmp( const CSMD_CHAR * pccStr1, const CSMD_CHAR * pccStr2, CSMD_ULONG ulSize )
{

  return (CSMD_LONG) strncmp( pccStr1, pccStr2, (size_t)ulSize );

} /* end: CSMD_HAL_strcmp() */


/* --------------------------------------------- */
/* Replacements for functions from stdio.h lib   */
/* --------------------------------------------- */

/**************************************************************************/ /**
\brief  Write formatted data to string.

\ingroup module_abstract
\b Description: \n
  Composes a string with the content is stored as a C string in the buffer
  pointed by pcStr. Only one unsigned short argument is being processed.

<B>Call Environment:</B> \n
  This is a CoSeMa-private function.

\param [in]   pcStr
              Pointer to a buffer where the resulting C-string is stored.
\param [in]   pccFormat
              Format string for one unsigned short argument.
\param [in]   usArg1
              unsigned short argument.

\return       The total number of characters written.

\author       WK
\date         27.03.2014

***************************************************************************** */
CSMD_LONG CSMD_HAL_sprintf_ushort( CSMD_CHAR * pcStr, const CSMD_CHAR *pccFormat, CSMD_USHORT usArg1 )
{
  CSMD_LONG lRet;

  lRet = sprintf( pcStr, pccFormat, usArg1 );

  return lRet;

} /* end: CSMD_HAL_sprintf_ushort() */


/*! \endcond */ /* HAL */

/*lint -restore */


/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - File created.
27 Mar 2014 WK
  - Added replacements for used functions of string.h and stdio.h
      CSMD_HAL_memset()
      CSMD_HAL_memcpy()
      CSMD_HAL_strcpy()
      CSMD_HAL_strncpy()
      CSMD_HAL_strcmp()
      CSMD_HAL_sprintf_ushort()
23 Oct 2014 WK
  Defdb00174315
    Added new function CSMD_HAL_SetSercosTimeExtSync().
05 Feb 2015 WK
  - CSMD_HAL_GetTelegramStatus(), CSMD_HAL_ClearTelegramStatus():
    Replaced picture "telegram_status.png" with with table in "markdown"
    extra syntax to make it more readable in the source code.
11 Feb 2015 WK
  - Defdb00176768
    CSMD_HAL_CheckVersion(), CSMD_HAL_SetTimerEvent(),
    CSMD_HAL_ReadSercosTime():   Adjustments regarding soft master.
23 Feb 2015 WK
  - Defdb00177060
    CSMD_HAL_ReadBlock(), CSMD_HAL_WriteBlock():
    Copy mechanism replaced with CSMD_HAL_memcpy()
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
18 Mar 2015 WK
  - Replaced CSMD_HAL_strcmp() with CSMD_HAL_strncmp().
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
02 Jul 2015 WK
  - Defdb00159611
    CSMD_HAL_CheckVersion()
    IP-Core test versions expressly permitted for soft-master.
16 Nov 2015 WK
  - Defdb00000000
    Test-Code for sprintf has been removed.
08 Mar 2016 WK
  - Defdb00185518
    Functions CSMD_HAL_CheckVersion(), CSMD_HAL_SetTimerEvent() and
    CSMD_HAL_ReadSercosTime():
    Encapsulation with CSMD_SOFT_MASTER and case distinction IP-Core/Soft-Master.
27 Oct 2016
  - Defdb00182067
    Adjust include of CSMD_HAL_GLOB.h and CSMD_HAL_PRIV.h for SoftMaster.
  
------------------------------------------------------------------------------
*/
