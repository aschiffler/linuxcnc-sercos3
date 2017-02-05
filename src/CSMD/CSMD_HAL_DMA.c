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
\file   CSMD_HAL_DMA.c
\author AM
\date   01.09.2007
\brief  This File contains the private HAL functions for PCI bus master DMA.
*/

#include "CSMD_USER.h"
#include  CSMD_TYPE_HEADER

#include "CSMD_HAL_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#define SOURCE_CSMD
#include "CSMD_HAL_DMA.h"


/*! \cond HAL_DMA */

/*---- Definition public Functions: ------------------------------------------*/

/*---- Definition private Functions: -----------------------------------------*/


#ifdef CSMD_PCI_MASTER

/*###########################################################################*/
/*-----------------------{ Sercos DMA FUNCTIONS }----------------------------*/
/*###########################################################################*/
/*
 * 
 * The Sercos FPGA received an extension with a DMA functionality in order
 * to improve data transfer between FPGA via PCI bus into the controller's DRAM
 * and vice versa. For this reason two communication register sets were included,
 * providing the bidirectional use of an independent register set for data
 * transfer between external PCI device and Sercos FPGA, which could be
 * predefined in an initialization phase. Selection of the active register set,
 * data transfer kickoff and error / process status are handled over a global
 * control and status register.
 */
/*###########################################################################*/


/**************************************************************************/ /**
\brief Checks the current PCI version.

\ingroup module_dma
\b Description: \n
   This function always returns true because PCI is no longer checked.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
 
\return       - TRUE -> version check successful
              - FALSE -> version check failed

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_BOOL CSMD_HAL_CheckDMA_PCIVersion( const CSMD_HAL *prCSMD_HAL )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  /* warning no PCI DMA version check!! */
  
  /* use prCSMD_HAL to return TRUE and prevent compiler warning */
  return (prCSMD_HAL == prCSMD_HAL);
  /* return (TRUE);*/
  
} /* end: CSMD_HAL_CheckDMA_PCIVersion() */



/**************************************************************************/ /**
\brief Starts the Tx DMA for all selected DMA channels.

\ingroup module_dma
\b Description: \n
   This function starts the Tx DMA for all selected DMA channels writing to
   the DMA register DMA_START.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usChannels
              Tx DMA channel selection. The bit number represents the corresponding
              DMA channel number 00 to 15.\n
              Up to 16 channels can be selected.

\return       none

\author       WK
\date         10.02.2011

***************************************************************************** */
CSMD_VOID CSMD_HAL_StartTxDMA( const CSMD_HAL *prCSMD_HAL,
                               CSMD_USHORT     usChannels )
{
  
  prCSMD_HAL->prSERC_DMA_Reg->ulDMA_START = CSMD_END_CONV_L( ((CSMD_ULONG)usChannels) );
  
} /* end: CSMD_HAL_StartTxDMA() */



/**************************************************************************/ /**
\brief Enables the selected Rx DMA channels.

\ingroup module_dma
\b Description: \n
   This function enables the selected Rx DMA channels and the corresponding
   Rx ready_DMA flags by writing to the DMA register EN_S3_RX.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usChannels
              Enables the Rx DMA and Rx RDY DMA for the selected channels.
              The bit number represents the corresponding DMA channel number 00 to 15.\n
              Up to 16 channels can be selected.

\return       none

\author       WK
\date         09.03.2011

***************************************************************************** */
CSMD_VOID CSMD_HAL_Enable_RXDMA( const CSMD_HAL *prCSMD_HAL,
                                 CSMD_USHORT     usChannels )
{
  prCSMD_HAL->prSERC_DMA_Reg->ulEN_S3_RX = usChannels | ((CSMD_ULONG)usChannels << 16);
  
} /* end: CSMD_HAL_Enable_RXDMA() */



/**************************************************************************/ /**
\brief Starts the Rx DMA for all selected DMA channels.

\ingroup module_dma
\b Description: \n
   This function starts the Rx DMA for all selected DMA channels writing to
   the DMA register DMA_START.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.
  
\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usChannels
              Selects the Rx DMA channels. The bit number represents the 
              corresponding DMA channel number 00 to 15.\n
              Up to 16 channels can be selected.

\return       none

\author       WK
\date         10.02.2011

***************************************************************************** */
CSMD_VOID CSMD_HAL_StartRxDMA( const CSMD_HAL *prCSMD_HAL,
                               CSMD_USHORT     usChannels )
{
  
  prCSMD_HAL->prSERC_DMA_Reg->ulDMA_START = 
    CSMD_END_CONV_L( (((CSMD_ULONG)usChannels) << CSMD_HAL_DMA_SHIFT_START_RX) );
  
} /* end: CSMD_HAL_StartRxDMA() */



/**************************************************************************/ /**
\brief Sets counter of bytes to be transferred for the selected register set.

\ingroup module_dma
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure 
\param [in]   usSelDirection 
              Defines data transfer direction\n
              0 -> transfer from dual port RAM of Sercos III FPGA
              into desired location\n
              1 -> transfer into dual port RAM of Sercos III FPGA
                    
\param [in]   usSelChannelNbr
              Selects the channel set the command bits refer to 0..15

\param [in]   ulCount
              Number of bytes to be transferred

\return       none

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_VOID CSMD_HAL_SetDMACounter( const CSMD_HAL *prCSMD_HAL,
                                  CSMD_USHORT     usSelDirection,
                                  CSMD_USHORT     usSelChannelNbr,
                                  CSMD_ULONG      ulCount )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  if (usSelDirection == 0)
  {
    prCSMD_HAL->prSERC_DMA_Reg->arTX_Control_Block[usSelChannelNbr].ulLength = 
      CSMD_END_CONV_L( ulCount );
  }
  else if (usSelDirection == 1)
  {
    prCSMD_HAL->prSERC_DMA_Reg->arRX_Control_Block[usSelChannelNbr].ulLength = 
      CSMD_END_CONV_L( ulCount );
  }
  
} /* end: CSMD_HAL_SetDMACounter() */



/**************************************************************************/ /**
\brief Defines the current address location (Sercos) of the DMA unit.

\ingroup module_dma
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure 
\param [in]   usSelDirection
              Defines data transfer direction\n
              0 -> transfer from dual port RAM of Sercos FPGA into desired location\n
              1 -> transfer into dual port RAM of Sercos FPGA
\param [in]   usSelChannelNbr
              Selects the channel set the command bits refer to (0..15)
\param [in]   ulAddr
              PCI address of DMA unit (Sercos)

\return       none

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_VOID CSMD_HAL_SetDMALocalAddr( const CSMD_HAL *prCSMD_HAL,
                                    CSMD_USHORT     usSelDirection,
                                    CSMD_USHORT     usSelChannelNbr,
                                    CSMD_ULONG      ulAddr )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  if (usSelDirection == 0)
  {
    prCSMD_HAL->prSERC_DMA_Reg->arTX_Control_Block[usSelChannelNbr].ulSourceAdd = 
      CSMD_END_CONV_L( ulAddr );
  }
  else if (usSelDirection == 1)
  {
    prCSMD_HAL->prSERC_DMA_Reg->arRX_Control_Block[usSelChannelNbr].ulDestinationAdd = 
      CSMD_END_CONV_L( ulAddr );
  }
  
} /* end: CSMD_HAL_SetDMALocalAddr() */



/**************************************************************************/ /**
\brief Defines the current PCI address of the DMA unit.

\ingroup module_dma
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure 
\param [in]   usSelDirection
              Defines data transfer direction\n
              0 -> transfer from dual port RAM of Sercos FPGA into desired location\n
              1 -> transfer into dual port RAM of Sercos FPGA
\param [in]   usSelChannelNbr
              Selects the channel set the command bits refer to (0..15)
\param [in]   ulAddr
              PCI Address of DMA unit

\return       none

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_VOID CSMD_HAL_SetDMAPCIAddr( const CSMD_HAL *prCSMD_HAL,
                                  CSMD_USHORT     usSelDirection,
                                  CSMD_USHORT     usSelChannelNbr,
                                  CSMD_ULONG      ulAddr )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  if (usSelDirection == 0)
  {
    prCSMD_HAL->prSERC_DMA_Reg->arTX_Control_Block[usSelChannelNbr].ulDestinationAdd = 
      CSMD_END_CONV_L( ulAddr );
  }
  else if (usSelDirection == 1)
  {
    prCSMD_HAL->prSERC_DMA_Reg->arRX_Control_Block[usSelChannelNbr].ulSourceAdd = 
      CSMD_END_CONV_L( ulAddr );
  }
  
} /* end: CSMD_HAL_SetDMAPCIAddr() */


/**************************************************************************/ /**
\brief Defines the current PCI address of the Ready DMAs.

\ingroup module_dma
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure 
\param [in]   usSelDirection
              Defines data transfer direction\n
              0 -> transfer from dual port RAM of Sercos FPGA into desired location\n
              1 -> transfer into dual port RAM of Sercos FPGA
\param [in]   usSelChannelNbr
              Selects the channel set the command bits refer to (0..15)
\param [in]   ulAddr
              PCI Address of DMA unit

\return       none

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_VOID CSMD_HAL_SetDMARDYAddr( const CSMD_HAL *prCSMD_HAL,
                                  CSMD_USHORT     usSelDirection,
                                  CSMD_USHORT     usSelChannelNbr,
                                  CSMD_ULONG      ulAddr )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  
  if (usSelDirection == 0)
  {
    prCSMD_HAL->prSERC_DMA_Reg->arTX_Control_Block[usSelChannelNbr].ulRdyFlagAdd = 
      CSMD_END_CONV_L( ulAddr );
  }
  else if (usSelDirection == 1)
  {
    prCSMD_HAL->prSERC_DMA_Reg->arRX_Control_Block[usSelChannelNbr].ulRdyFlagAdd = 
      CSMD_END_CONV_L( ulAddr );
  }
  
} /* end: CSMD_HAL_SetDMARDYAddr() */


/**************************************************************************/ /**
\brief Resets all Tx DMA registers.

\ingroup module_dma
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure 

\return       none

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_VOID CSMD_HAL_ResetTXDMA( const CSMD_HAL *prCSMD_HAL )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  CSMD_USHORT usI;

  for (usI = 0; usI < CSMD_DMA_MAX_TX_CHANNEL; usI++)
  {
    prCSMD_HAL->prSERC_DMA_Reg->arTX_Control_Block[usI].ulDestinationAdd = 0;
    prCSMD_HAL->prSERC_DMA_Reg->arTX_Control_Block[usI].ulSourceAdd      = 0;
    prCSMD_HAL->prSERC_DMA_Reg->arTX_Control_Block[usI].ulLength         = 0;
    prCSMD_HAL->prSERC_DMA_Reg->arTX_Control_Block[usI].ulRdyFlagAdd     = 0;
  }
  
  prCSMD_HAL->prSERC_DMA_Reg->ulEN_S3_TX = 0;
  
} /* end: CSMD_HAL_ResetDMA() */


/**************************************************************************/ /**
\brief Resets all Rx DMA registers.

\ingroup module_dma
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure

\return       none

\author       KP
\date         13.08.2007 

***************************************************************************** */
CSMD_VOID CSMD_HAL_ResetRXDMA( const CSMD_HAL *prCSMD_HAL )
{
/*HAL------------<YOUR IMPLEMENTATION OF THE HAL-FUNCTION>--------------- HAL*/
  CSMD_USHORT usI;

  for (usI = 0; usI < CSMD_DMA_MAX_RX_CHANNEL; usI++)
  {
    prCSMD_HAL->prSERC_DMA_Reg->arRX_Control_Block[usI].ulDestinationAdd = 0;
    prCSMD_HAL->prSERC_DMA_Reg->arRX_Control_Block[usI].ulSourceAdd      = 0;
    prCSMD_HAL->prSERC_DMA_Reg->arRX_Control_Block[usI].ulLength         = 0;
    prCSMD_HAL->prSERC_DMA_Reg->arRX_Control_Block[usI].ulRdyFlagAdd     = 0;
  }
  
  prCSMD_HAL->prSERC_DMA_Reg->ulEN_S3_RX = 0;
  
  
} /* end: CSMD_HAL_ResetDMA() */
#endif  /* #ifdef CSMD_PCI_MASTER */

/*! \endcond */ /* HAL_DMA */




/*
--------------------------------------------------------------------------------
  Modification history
--------------------------------------------------------------------------------

02 Sep 2010
  - File created.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
  
--------------------------------------------------------------------------------
*/
