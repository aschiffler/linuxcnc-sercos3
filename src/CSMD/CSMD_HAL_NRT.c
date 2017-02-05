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
\file   CSMD_HAL_NRT.c
\author WK
\date   30.06.2006
\brief  This File contains the functions for the UC channel.
*/

/*---- Includes: -------------------------------------------------------------*/

#include "CSMD_GLOB.h"
#include "CSMD_HAL_PRIV.h"

#ifdef   CSMD_IP_HEADER
#include CSMD_IP_HEADER    /* Check values in separate header for IP driver */
#endif

#define SOURCE_CSMD
#include "CSMD_HAL_NRT.h"

/*lint -save -e818 Pointer parameter '' could be declared as pointing to const */

/*! \cond HAL_UC */

/*---- Definition public Functions: ------------------------------------------*/

/**************************************************************************/ /**
\brief Sets the MAC address in FPGA.

\ingroup func_UC
\b Description: \n
   This function sets the MAC address of the Sercos master writing to the
   register MAC1.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ucMacAddr[]
              MAC address (6 Bytes) b5-b4-b3-b2-b1-b0 

\return       none

\author       wk
\date         11.09.2006

***************************************************************************** */
CSMD_VOID  CSMD_HAL_SetMacAddress( CSMD_HAL   *prCSMD_HAL,
                                   CSMD_UCHAR  ucMacAddr[] )
{
  
  CSMD_HAL_SERCFPGA_MACADD_UNION   rTempMac;
  CSMD_USHORT   usI;
  
  
  for (usI=0; usI<6; usI++)
    rTempMac.ucAddress[usI] = ucMacAddr[usI];
  
  rTempMac.ucAddress[usI++] = 0;
  rTempMac.ucAddress[usI]   = 0;
  
  /* prCSMD_HAL->prSERC_Reg->rMAC1 = rTempMac; */
  prCSMD_HAL->prSERC_Reg->rMAC1.ul.AddressH = rTempMac.ul.AddressH;
  prCSMD_HAL->prSERC_Reg->rMAC1.ul.AddressL = rTempMac.ul.AddressL;
  
} /* end: CSMD_HAL_SetMacAddress */



/**************************************************************************/ /**
\brief Reads the MAC address from FPGA.

\ingroup func_UC
\b Description: \n
   This function reads the MAC address of the Sercos master from the
   register MAC1.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ucMacAddr[]
              MAC address (6 Bytes) b5-b4-b3-b2-b1-b0 

\return       none

\author       wk
\date         11.09.2006

***************************************************************************** */
CSMD_VOID CSMD_HAL_GetMacAddress( CSMD_HAL   *prCSMD_HAL,
                                  CSMD_UCHAR  ucMacAddr[] )
{
  
  CSMD_HAL_SERCFPGA_MACADD_UNION   rTempMac;
  CSMD_USHORT   usI;
  
  rTempMac.ul.AddressH = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->rMAC1.ul.AddressH) );
  rTempMac.ul.AddressL = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->rMAC1.ul.AddressL) );
  
  for (usI=0; usI<6; usI++)
    ucMacAddr[usI] = rTempMac.ucAddress[usI];
  
} /* end: CSMD_HAL_GetMacAddress */



/**************************************************************************/ /**
\brief Writes the IP Control Register for one or both ports.

\ingroup func_UC
\b Description: \n
   This function handles the IP channel writing to the IP Control and Status 
   Registers IPCSR1 and/or IPCSR2.
   <BR>

   Control word in the IP Control- and Status register IPCSR 1/2: \n

   | Bit No.| Control Bit      | Comments                                                             |
   | ------ | ---------------- | -------------------------------------------------------------------- |
   | 11     | PTxBufEmptyIntEn | Enables Interrupt Int_IPIntPort1/2 on event IPTxBufEmpty.            |
   | 10     | IPTxBufRdyIntEn  | Enables Interrupt Int_IPIntPort1/2 on event IPTxBufRdy.              |
   |  9     | IPRxBufFullIntEn | Enables Interrupt Int_IPIntPort1/2 on event IPRxBufFull.             |
   |  8     | IPRxRdyIntEn     | Enables Interrupt Int_IPIntPort1/2 on event IPRxRdy.                 |
   |  6     | Promiscuous      | Receive all Frames without checking the destination address.         |
   |  5     | ColBufDisable    | Disables collision buffer. Frames are not forwarded to opposite port.|
   |  4     | MulticastDisable | Disables reception of multicast frames in IP channel.                |
   |  3     | BroadcastDisable | Disables reception of broadcast frames in IP channel.                |
   |  1     | IPRxEn           | Receive enable port 1/2                                              |
   |  0     | IPTxEn           | Transmit enable port 1/2                                             |
   <BR>


<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulIPCntrl
              The lower word contains the IP Control Register\n
              If both ports are selected, low word holds port 1 and high word
              holds port 2 data.
\param [in]   usPort
              Selected Port\n
              1 = Port 1\n
              2 = Port 2\n
              3 = both ports

\return       0

\author       wk
\date         11.09.2006

***************************************************************************** */
CSMD_ULONG CSMD_HAL_PutIPControlRegister( CSMD_HAL    *prCSMD_HAL,
                                          CSMD_ULONG   ulIPCntrl,
                                          CSMD_USHORT  usPort )
{
  
  if (usPort == CSMD_HAL_IP_PORT1)
  {
    prCSMD_HAL->prSERC_Reg->rIPCSR1.us.IPCR =
      CSMD_END_CONV_S( ((CSMD_USHORT) ulIPCntrl) );
  }
  else if (usPort == CSMD_HAL_IP_PORT2)
  {
    prCSMD_HAL->prSERC_Reg->rIPCSR2.us.IPCR =
      CSMD_END_CONV_S( ((CSMD_USHORT) ulIPCntrl) );
  }
  else if (usPort == CSMD_HAL_IP_BOTH_PORTS)
  {
    prCSMD_HAL->prSERC_Reg->rIPCSR1.us.IPCR =
      CSMD_END_CONV_S( ((CSMD_USHORT) ulIPCntrl) );
    
    prCSMD_HAL->prSERC_Reg->rIPCSR2.us.IPCR =
      CSMD_END_CONV_S( ((CSMD_USHORT) (ulIPCntrl >> 16)) );
  }
  
  return (0);
  
} /* end: CSMD_HAL_PutIPControlRegister */



/**************************************************************************/ /**
\brief Sets or clears the specified bits in the IP Control Register
       for port 1 and/or 2.

\ingroup func_UC
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulIPCntrlMask
              IP Control Register\n
              Take mask from low word. If both ports are selected, low word holds
              port 1 and high word holds port 2.
\param [in]   boMode
              0 = clear
              else = set bits
\param [in]   usPort
              Selected port\n
              1 = Port 1\n
              2 = Port 2\n
              3 = both ports

\return       0

\author       wk
\date         25.09.2006

***************************************************************************** */
CSMD_ULONG  CSMD_HAL_MaskIPControlRegister( CSMD_HAL    *prCSMD_HAL,
                                            CSMD_ULONG   ulIPCntrlMask,
                                            CSMD_BOOL    boMode,
                                            CSMD_USHORT  usPort )
{
  
  if (usPort == CSMD_HAL_IP_PORT1)
  {
    if (boMode)
    {
      prCSMD_HAL->prSERC_Reg->rIPCSR1.us.IPCR 
        |= CSMD_END_CONV_S( ((CSMD_USHORT) ulIPCntrlMask) );
    }
    else
    {
      prCSMD_HAL->prSERC_Reg->rIPCSR1.us.IPCR 
        &= CSMD_END_CONV_S( ((CSMD_USHORT) ~ulIPCntrlMask) );
    }
  }
  else if (usPort == CSMD_HAL_IP_PORT2)
  {
    if (boMode)
    {
      prCSMD_HAL->prSERC_Reg->rIPCSR2.us.IPCR 
        |= CSMD_END_CONV_S( ((CSMD_USHORT) ulIPCntrlMask) );
    }
    else
    {
      prCSMD_HAL->prSERC_Reg->rIPCSR2.us.IPCR 
        &= CSMD_END_CONV_S( ((CSMD_USHORT) ~ulIPCntrlMask) );
    }
  }
  else if (usPort == CSMD_HAL_IP_BOTH_PORTS)
  {
    if (boMode)
    {
      prCSMD_HAL->prSERC_Reg->rIPCSR1.us.IPCR 
        |= CSMD_END_CONV_S( ((CSMD_USHORT) ulIPCntrlMask) );
      prCSMD_HAL->prSERC_Reg->rIPCSR2.us.IPCR 
        |= CSMD_END_CONV_S( ((CSMD_USHORT) (ulIPCntrlMask >> 16)) );
    }
    else
    {
      prCSMD_HAL->prSERC_Reg->rIPCSR1.us.IPCR 
        &= CSMD_END_CONV_S( ((CSMD_USHORT) ~ulIPCntrlMask) );
      prCSMD_HAL->prSERC_Reg->rIPCSR2.us.IPCR 
        &= CSMD_END_CONV_S( ((CSMD_USHORT) ~(ulIPCntrlMask >> 16)) );
    }
  }
  
  return (0);

} /* end: CSMD_HAL_MaskIPControlRegister */



/**************************************************************************/ /**
\brief Reads the IP Status Register for one or both ports.

\ingroup func_UC
\b Description: \n
   This function reads the status of the IP channel from the IP Control
   and Status Registers IPCSR1 and IPCSR2.
   <BR>

   Status word in the IP Control- and Status register IPCSR 1/2: \n

   | Bit No.| Status Bit      | Comments                                                         |
   | ------ | --------------- | ---------------------------------------------------------------- |
   | 15     | Link            | Link exists at appropriate port                                  |
   |  3     | IPTxBufEmpty1/2 | Set when IP Transmit buffer empty port 1/2                       |
   |  2     | IPTxBufRdy1/2   | Set when IP Transmit buffer able to accept a frame by the host   |
   |  1     | IPRxBufFull1/2  | Set when IP Rx-buffer of port 1/2 is full                        |
   |  0     | IPRxRdy1/2      | Set when an IP Ethernet frame is received port 1/2 without error |
   <BR>

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pulIPStatus
              Pointer to IP Status Register\n
              The lower word contains the IP Status Register\n
              If both ports are selected, low word holds port 1 and high word
              holds port 2 data.

\param [in]   usPort
              Selected port\n
              1 = Port 1\n
              2 = Port 2\n
              3 = both ports

\return       0

\author       wk
\date         11.09.2006

***************************************************************************** */
CSMD_ULONG  CSMD_HAL_GetIPStatusRegister( CSMD_HAL    *prCSMD_HAL,
                                          CSMD_ULONG  *pulIPStatus,
                                          CSMD_USHORT  usPort )
{
  
  if (usPort == CSMD_HAL_IP_PORT1)
  {
    *((CSMD_USHORT *)(CSMD_VOID *)pulIPStatus) = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPCSR1.us.IPSR) );
  }
  else if (usPort == CSMD_HAL_IP_PORT2)
  {
    *((CSMD_USHORT *)(CSMD_VOID *)pulIPStatus) = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPCSR2.us.IPSR) );
  }
  else if (usPort == CSMD_HAL_IP_BOTH_PORTS)
  {
    *((CSMD_USHORT *)(CSMD_VOID *)pulIPStatus)     = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPCSR1.us.IPSR) );
    
    *(((CSMD_USHORT *)(CSMD_VOID *)pulIPStatus) + 1) = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPCSR2.us.IPSR) );
  }
  
  return (0);
  
} /* end: CSMD_HAL_GetIPStatusRegister */



/**************************************************************************/ /** 
\brief Reads the IP Control Register for one or both ports.

\ingroup func_UC
\b Description: \n
   This function handles the IP channel reading to the IP Control and Status 
   Registers IPCSR1 and/or IPCSR2.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
                Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pulIPCntrl
                The lower word contains the IP Control Register\n
                If both ports are selected, low word holds port 1 and high word
                holds port 2 data.
\param [in]   usPort
                Selected Port\n
                1 = Port 1\n
                2 = Port 2\n
                3 = both ports

\return       0

\author       wk
\date         06.02.2014

***************************************************************************** */
CSMD_ULONG CSMD_HAL_GetIPControlRegister( CSMD_HAL    *prCSMD_HAL,
                                          CSMD_ULONG  *pulIPCntrl,
                                          CSMD_USHORT  usPort )
{
  
  if (usPort == CSMD_HAL_IP_PORT1)
  {
    *((CSMD_USHORT *)(CSMD_VOID *)pulIPCntrl) = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPCSR1.us.IPCR) );
  }
  else if (usPort == CSMD_HAL_IP_PORT2)
  {
    *((CSMD_USHORT *)(CSMD_VOID *)pulIPCntrl) = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPCSR2.us.IPCR) );
  }
  else if (usPort == CSMD_HAL_IP_BOTH_PORTS)
  {
    *((CSMD_USHORT *)(CSMD_VOID *)pulIPCntrl) = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPCSR1.us.IPCR) );
    
    *(((CSMD_USHORT *)(CSMD_VOID *)pulIPCntrl) + 1) = 
      CSMD_END_CONV_S( (prCSMD_HAL->prSERC_Reg->rIPCSR2.us.IPCR) );
    
  }
  
  return (0);
  
} /* end: CSMD_HAL_GetIPControlRegister */



/**************************************************************************/ /**
\brief Defines the IP transmit buffer in RxRam for port 1 and/or 2.

\ingroup func_UC
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usFirstRamSegment
              first RxRam segment (256 byte aligned) 
\param [in]   usLastRamSegment
              last RxRam segment (256 byte aligned)
\param [in]   usPort
              Selected port\n
              1 = Port 1\n
              2 = Port 2\n

\return       0 - no error\n
              1 - error

\author       wk
\date         29.09.2006

***************************************************************************** */
CSMD_ULONG  CSMD_HAL_ConfigIPTxDescriptor( CSMD_HAL    *prCSMD_HAL,
                                           CSMD_USHORT  usFirstRamSegment,
                                           CSMD_USHORT  usLastRamSegment,
                                           CSMD_USHORT  usPort )
{
  
  if (   (usFirstRamSegment == 0) && (usLastRamSegment  == 0))
  {
    /* not enough RAM available - delete informationen for both ports */
    prCSMD_HAL->usIPTxRamP1Start = 0U;
    prCSMD_HAL->usIPTxRamP1End   = 0U;
    prCSMD_HAL->usIPTxRamP2Start = 0U;
    prCSMD_HAL->usIPTxRamP2End   = 0U;
    
    return (0);
  }
  else if (   (usFirstRamSegment <= usLastRamSegment)
           && (usFirstRamSegment >= (prCSMD_HAL->ulTxRamInUse / CSMD_HAL_IP_RAM_SEG_SIZE))
           && (usLastRamSegment  <  (prCSMD_HAL->ulSizeOfTxRam / CSMD_HAL_IP_RAM_SEG_SIZE)))
  {
    /* ulReg = (CSMD_ULONG)usFirstRamSegment + ((CSMD_ULONG)usLastRamSegment) << 16; */
    
    if (usPort == CSMD_HAL_IP_PORT1)
    {
      /* set corresponding fpga register here when available */
      prCSMD_HAL->usIPTxRamP1Start = (CSMD_USHORT)(usFirstRamSegment * CSMD_HAL_IP_RAM_SEG_SIZE);
      prCSMD_HAL->usIPTxRamP1End   = (CSMD_USHORT)((usLastRamSegment + 1) * CSMD_HAL_IP_RAM_SEG_SIZE - 1);
    }
    else if (usPort == CSMD_HAL_IP_PORT2)
    {
      /* set corresponding fpga register here when available */
      prCSMD_HAL->usIPTxRamP2Start = (CSMD_USHORT)(usFirstRamSegment * CSMD_HAL_IP_RAM_SEG_SIZE);
      prCSMD_HAL->usIPTxRamP2End   = (CSMD_USHORT)((usLastRamSegment + 1) * CSMD_HAL_IP_RAM_SEG_SIZE - 1);
    }
    else if (usPort == CSMD_HAL_IP_BOTH_PORTS)
    {
      /* set corresponding fpga register here when available */
      prCSMD_HAL->usIPTxRamP1Start = (CSMD_USHORT)(usFirstRamSegment * CSMD_HAL_IP_RAM_SEG_SIZE);
      prCSMD_HAL->usIPTxRamP1End   = (CSMD_USHORT)((usLastRamSegment + 1) * CSMD_HAL_IP_RAM_SEG_SIZE - 1);
      prCSMD_HAL->usIPTxRamP2Start = (CSMD_USHORT)(usFirstRamSegment * CSMD_HAL_IP_RAM_SEG_SIZE);
      prCSMD_HAL->usIPTxRamP2End   = (CSMD_USHORT)((usLastRamSegment + 1) * CSMD_HAL_IP_RAM_SEG_SIZE - 1);
    }
    return (0);
  }
  else
  {
    return (1);
  }

} /* end: CSMD_HAL_ConfigIPTxDescriptor */



/**************************************************************************/ /**
\brief Writes the TxIP Descriptor Register for port 1 and/or 2 in order
       to initiate a transmission.

\ingroup func_UC
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usSegAddr
              TxRam segment address (256 byte aligned) 
\param [in]   usLength
              data length (max. 0x7FF = 2047 Byte)
\param [in]   usPort
              Selected port\n
              1 = Port 1\n
              2 = Port 2\n
              3 = both ports

\return       0

\author       wk
\date         29.09.2006

***************************************************************************** */
CSMD_ULONG  CSMD_HAL_WriteIPTxStack( CSMD_HAL    *prCSMD_HAL,
                                     CSMD_USHORT  usSegAddr,
                                     CSMD_USHORT  usLength,
                                     CSMD_USHORT  usPort )
{
  
  CSMD_ULONG ulReg;
  
  ulReg = CSMD_END_CONV_L( ((CSMD_ULONG)usSegAddr + (((CSMD_ULONG)usLength) << 16)) );
  
  if (usPort == CSMD_HAL_IP_PORT1)
  {
    prCSMD_HAL->prSERC_Reg->ulIPTXS1 = ulReg;
  }
  else if (usPort == CSMD_HAL_IP_PORT2)
  {
    prCSMD_HAL->prSERC_Reg->ulIPTXS2 = ulReg;
  }
  else if (usPort == CSMD_HAL_IP_BOTH_PORTS)
  {
    prCSMD_HAL->prSERC_Reg->ulIPTXS1 = ulReg;
    prCSMD_HAL->prSERC_Reg->ulIPTXS2 = ulReg;
  }
  
  return (0);

} /* end: CSMD_HAL_WriteIPTxStack */



/**************************************************************************/ /**
\brief Defines the IP receive buffer in RxRam for port 1 and/or 2

\ingroup func_UC
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usFirstRamSegment
              first RxRam segment (256 byte aligned) 
\param [in]   usLastRamSegment
              last RxRam segment (256 byte aligned)
\param [in]   usPort
              Selected port\n
              1 = Port 1\n
              2 = Port 2\n
              
\return       0 - no error
              1 - error

\author       wk
\date         12.09.2006

***************************************************************************** */
CSMD_ULONG  CSMD_HAL_ConfigIPRxDescriptor( CSMD_HAL    *prCSMD_HAL,
                                           CSMD_USHORT  usFirstRamSegment,
                                           CSMD_USHORT  usLastRamSegment,
                                           CSMD_USHORT  usPort )
{
  
  CSMD_ULONG ulReg;
  
  if (   (usFirstRamSegment == 0) && (usLastRamSegment  == 0))
  {
    /* not enough RAM available - delete informationen for both ports */
    prCSMD_HAL->prSERC_Reg->ulIPRRS1 = 0U;
    prCSMD_HAL->prSERC_Reg->ulIPRRS2 = 0U;
    
    prCSMD_HAL->usIPRxRamP1Start = 0U;
    prCSMD_HAL->usIPRxRamP1End   = 0U;
    prCSMD_HAL->usIPRxRamP2Start = 0U;
    prCSMD_HAL->usIPRxRamP2End   = 0U;
    
    return (0);
  }
  else if (   (usFirstRamSegment <= usLastRamSegment)
           && (usFirstRamSegment >= (prCSMD_HAL->ulRxRamInUse / CSMD_HAL_IP_RAM_SEG_SIZE))
           && (usLastRamSegment  <  (prCSMD_HAL->ulSizeOfRxRam / CSMD_HAL_IP_RAM_SEG_SIZE)))
  {
    ulReg = CSMD_END_CONV_L( ((CSMD_ULONG)usFirstRamSegment + (((CSMD_ULONG)usLastRamSegment) << 16)) );
    
    if (usPort == CSMD_HAL_IP_PORT1)
    {
      prCSMD_HAL->prSERC_Reg->ulIPRRS1 = ulReg;
      prCSMD_HAL->usIPRxRamP1Start = (CSMD_USHORT)(usFirstRamSegment * CSMD_HAL_IP_RAM_SEG_SIZE);
      prCSMD_HAL->usIPRxRamP1End   = (CSMD_USHORT)((usLastRamSegment + 1) * CSMD_HAL_IP_RAM_SEG_SIZE - 1);
    }
    else if (usPort == CSMD_HAL_IP_PORT2)
    {
      prCSMD_HAL->prSERC_Reg->ulIPRRS2 = ulReg;
      prCSMD_HAL->usIPRxRamP2Start = (CSMD_USHORT)(usFirstRamSegment * CSMD_HAL_IP_RAM_SEG_SIZE);
      prCSMD_HAL->usIPRxRamP2End   = (CSMD_USHORT)((usLastRamSegment + 1) * CSMD_HAL_IP_RAM_SEG_SIZE - 1);
    }
    return (0);
  }
  else
  {
    return (1);
  }

} /* end: CSMD_HAL_ConfigIPRxDescriptor */



/**************************************************************************/ /**
\brief Reads the RxIP descriptor register for port 1 and/or 2.

\ingroup func_UC
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pulRegister
              Output pointer to RxIP descriptor
\param [in]   usPort
              Selected port\n
              1 = Port 1\n
              2 = Port 2\n

\return       0

\author       wk
\date         12.09.2006

***************************************************************************** */
CSMD_ULONG  CSMD_HAL_ReadIPRxStack( CSMD_HAL    *prCSMD_HAL,
                                    CSMD_ULONG  *pulRegister,
                                    CSMD_USHORT  usPort )
{
  
  if (usPort == CSMD_HAL_IP_PORT1)
  {
    *pulRegister = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulIPRXS1) );
  }
  else if (usPort == CSMD_HAL_IP_PORT2)
  {
    *pulRegister = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->ulIPRXS2) );
  }
  
  return (0);
  
} /* end: CSMD_HAL_ReadIPRxStack */



/**************************************************************************/ /**
\brief Clears the RxIP descriptor register for port 1 and/or 2.

\ingroup func_UC
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   usPort
              Selected port\n
              1 = Port 1\n
              2 = Port 2\n

\return       0

\author       WK
\date         12.09.2006

***************************************************************************** */
CSMD_ULONG  CSMD_HAL_ClearIPRxStack( CSMD_HAL    *prCSMD_HAL,
                                     CSMD_USHORT  usPort )
{
  
  if (usPort == CSMD_HAL_IP_PORT1)
  {
    prCSMD_HAL->prSERC_Reg->ulIPRXS1 = 0;
  }
  
  else if (usPort == CSMD_HAL_IP_PORT2)
  {
    prCSMD_HAL->prSERC_Reg->ulIPRXS2 = 0;
  }
  
  return (0);
  
} /* end: CSMD_HAL_ClearIPRxStack */



/**************************************************************************/ /**
\brief Reads a received frame from FPGA RxRam.

\ingroup func_UC
\b Description: \n
   This function can be used to read a frame either completely or partially.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulRxFrameOffset
              Frame offset in RxRam of FPGA in bytes
\param [out]  pucDest
              Output pointer to driver's destination frame
\param [in]   ulLen
              Frame length incl. FCS

\return       0 - no error\n
              1 - error

\author       wk
\date         13.09.2006

***************************************************************************** */
CSMD_ULONG  CSMD_HAL_IPGetPacket( CSMD_HAL   *prCSMD_HAL,
                                  CSMD_ULONG  ulRxFrameOffset,
                                  CSMD_UCHAR *pucDest,
                                  CSMD_ULONG  ulLen )
{
  
  CSMD_UCHAR  *pucDst;
  CSMD_UCHAR  *pucSrc;
  
  CSMD_ULONG   ulStart;
  CSMD_ULONG   ulEnd;
  CSMD_ULONG   ulFlatSize;
  CSMD_ULONG   ulRest;
  
  
  /****************************************************
  * determine lower and upper limit of current 
  * frame buffer by means of current frame offset
  ****************************************************/
  if (   (ulRxFrameOffset >= prCSMD_HAL->usIPRxRamP1Start)
      && (ulRxFrameOffset <= prCSMD_HAL->usIPRxRamP1End))
  {
    ulStart = prCSMD_HAL->usIPRxRamP1Start;
    ulEnd   = prCSMD_HAL->usIPRxRamP1End;
  }
  else if (   (ulRxFrameOffset >= prCSMD_HAL->usIPRxRamP2Start)
           && (ulRxFrameOffset <= prCSMD_HAL->usIPRxRamP2End))
  {
    ulStart = prCSMD_HAL->usIPRxRamP2Start;
    ulEnd   = prCSMD_HAL->usIPRxRamP2End;
  }
  else
  {
    return (1);
  }
  
  /****************************************************
  * evaluate current flatram size up to 
  * receive buffer end.
  ****************************************************/
  ulFlatSize = ulEnd + 1 - ulRxFrameOffset;
  if (ulLen > ulFlatSize)
  {
    /* frame does not fit in flatram, copy rest starting from first segment */ 
    ulRest = ulLen - ulFlatSize;
    ulLen  = ulFlatSize;
  }
  else
  {
    /* fits all in flatram */
    ulRest = 0;
  }
  
  pucSrc = (CSMD_UCHAR *)prCSMD_HAL->prSERC_RX_Ram->aucRx_Ram  + ulRxFrameOffset;
  pucDst = pucDest;
  
  (CSMD_VOID) CSMD_HAL_memcpy( pucDst, pucSrc, ulLen );
  
  if (ulRest != 0)
  {
    /* Buffer overrun. Copy rest from the begin of frame buffer */
    pucSrc = (CSMD_UCHAR *)prCSMD_HAL->prSERC_RX_Ram->aucRx_Ram  + ulStart;
    pucDst = pucDst + ulLen;
    
    (CSMD_VOID) CSMD_HAL_memcpy( pucDst, pucSrc, ulRest );
  }
  
  return (0);

} /* end: CSMD_HAL_IPGetPacket */



/**************************************************************************/ /**
\brief Writes a transmit frame into the FPGA TxRam.

\ingroup func_UC
\b Description: \n
   Currently only one frame will be written into the FPGA. 
   ulTxFrameOffset points to the first transmitter segment. Therefore the
   length of txram is correct, so no check of ram size is needed.
   This function can be used to read a frame either completely or partially.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [in]   ulTxFrameOffset 
              Frame offset in TxRam of FPGA in bytes
\param [out]  pucSource
              Output pointer to driver's source frame
\param [in]   ulLen
              Frame length without FCS.

\return       0 - no error\n
              1 - error

\author       wk
\date         13.09.2006

***************************************************************************** */
CSMD_ULONG  CSMD_HAL_IPPutPacket( CSMD_HAL   *prCSMD_HAL,
                                  CSMD_ULONG  ulTxFrameOffset,
                                  CSMD_UCHAR *pucSource,
                                  CSMD_ULONG  ulLen )
{
  
#undef  CSMD_HAL_IP_TX_RINGBUFFER
/*
#define CSMD_IP_TX_RINGBUFFER
  */
  
  CSMD_UCHAR  *pucDst;
  CSMD_UCHAR  *pucSrc;
#ifdef CSMD_HAL_IP_TX_RINGBUFFER
  CSMD_ULONG   ulStart;
  CSMD_ULONG   ulEnd;
  CSMD_ULONG   ulFlatSize;
  CSMD_ULONG   ulRest;
  
  /****************************************************
  * determine lower and upper limit of current 
  * frame buffer by means of current frame offset
  ****************************************************/
  if (   (ulTxFrameOffset >= prCSMD_HAL->usIPTxRamP1Start)
      && (ulTxFrameOffset <= prCSMD_HAL->usIPTxRamP1End))
  {
    ulStart = prCSMD_HAL->usIPTxRamP1Start;
    ulEnd   = prCSMD_HAL->usIPTxRamP1End;
  }
  else if (   (ulTxFrameOffset >= prCSMD_HAL->usIPTxRamP2Start)
           && (ulTxFrameOffset <= prCSMD_HAL->usIPTxRamP2End))
  {
    ulStart = prCSMD_HAL->usIPTxRamP2Start;
    ulEnd   = prCSMD_HAL->usIPTxRamP2End;
  }
  else
  {
    return (1);
  }
  
  /****************************************************
  * evaluate current flatram size up to 
  * transmit buffer end
  ****************************************************/
  ulFlatSize = ulEnd + 1 - ulTxFrameOffset;
  if (ulLen > ulFlatSize)
  {
    /* frame does not fit in flatram, copy rest starting from first segment */ 
    ulRest = ulLen - ulFlatSize;
    ulLen  = ulFlatSize;
  }
  else
  {
    /* fits all in flatram */
    ulRest = 0;
  }
#else
  /****************************************************
  * determine whether current frame offset fits into 
  * lower and upper limit of current frame buffer.
  ****************************************************/
  if ( !(   (   (ulTxFrameOffset           >= prCSMD_HAL->usIPTxRamP1Start)
             && ((ulTxFrameOffset + ulLen) <= prCSMD_HAL->usIPTxRamP1End))
         ||
            (   (ulTxFrameOffset           >= prCSMD_HAL->usIPTxRamP2Start)
             && ((ulTxFrameOffset + ulLen) <= prCSMD_HAL->usIPTxRamP2End))))
  {
    return (1);
  }
#endif
  
  pucSrc = pucSource;
  pucDst = (CSMD_UCHAR *)prCSMD_HAL->prSERC_TX_Ram->aucTx_Ram + ulTxFrameOffset;
  
  (CSMD_VOID) CSMD_HAL_memcpy( pucDst, pucSrc, ulLen );
  
#ifdef CSMD_HAL_IP_TX_RINGBUFFER
  if (ulRest != 0)
  {
    pucSrc = pucSrc + ulLen;
    pucDst = prCSMD_HAL->prSERC_TX_Ram->aucTx_Ram  + ulStart;
    
    (CSMD_VOID) CSMD_HAL_memcpy( pucDst, pucSrc, ulRest );
  }
#endif
  
  return (0);
  
} /* end: CSMD_HAL_IPPutPacket */



/**************************************************************************/ /**
\brief Provides the current value of the counter register IPFRXOK.

\ingroup func_UC
\b Description: \n
   This function provides the counter of all frames received with no error on the
   port (includes forwarded and discarded frames due to low resource).

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pulRegister
              Output pointer to frame counter RxOK\n
              low word: port 1\n
              high word: port 2 

\return       none

\author       wk
\date         12.09.2006

***************************************************************************** */
CSMD_VOID  CSMD_HAL_GetCounter_FramesRxOK( CSMD_HAL   *prCSMD_HAL,
                                           CSMD_ULONG *pulRegister )
{
  
  /* Counter for errorfree received frames on a port */
  *pulRegister = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->rIPFRXOK.ulErrCnt) );
  
} /* end: CSMD_HAL_GetCounter_FramesRxOK */



/**************************************************************************/ /**
\brief Provides the current value of the counter register IPFTXOK.

\ingroup func_UC
\b Description: \n
   This function provides the counter of all frames transmitted with no error on the
   port (includes forwarded and discarded frames due to low resource).

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pulRegister
              Output pointer to frame counter TxOK\n
              low word: port 1\n
              high word: port 2 

\return       none

\author       wk
\date         12.09.2006

***************************************************************************** */
CSMD_VOID  CSMD_HAL_GetCounter_FramesTxOK( CSMD_HAL   *prCSMD_HAL,
                                           CSMD_ULONG *pulRegister )
{
  
  /* Counter for transmitted frames on a port */
  *pulRegister = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->rIPFTXOK.ulErrCnt) );
  
} /* end: CSMD_HAL_GetCounter_FramesTxOK */



/**************************************************************************/ /**
\brief Provides the current value of the counter register IPFCSERR.

\ingroup func_UC
\b Description: \n
   This function provides the current value of the counter for the received
   Ethernet frames with defective frame check sequence FCS or RxER indications.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pulRegister
              Output pointer to FCS error counter\n
              low word: port 1\n
              high word: port 2 

\return       none

\author       wk
\date         12.09.2006

***************************************************************************** */
CSMD_VOID CSMD_HAL_GetCounter_FCSErrors( CSMD_HAL   *prCSMD_HAL,
                                         CSMD_ULONG *pulRegister )
{
  
  /* Counter for received Ethernet frames with FCS error */
  *pulRegister = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->rIPFCSERR.ulErrCnt) );
  
} /* end: CSMD_HAL_GetCounter_FCSErrors */



/**************************************************************************/ /**
\brief Provides the current value of the counter register IPALGNERR.

\ingroup func_UC
\b Description: \n
   This function provides the current value of the counter for frames received
   with a wrong alignment.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pulRegister
              Output pointer to alignment error counter\n
              low word: port 1\n
              high word: port 2 

\return       none

\author       wk
\date         12.09.2006

***************************************************************************** */
CSMD_VOID  CSMD_HAL_GetCounter_AlignmentErrors( CSMD_HAL   *prCSMD_HAL,
                                                CSMD_ULONG *pulRegister )
{
  
  /* Counter for received Ethernet frames with an alignment error */
  *pulRegister = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->rIPALGNERR.ulErrCnt) );
  
} /* end: CSMD_HAL_GetCounter_AlignmentErrors */



/**************************************************************************/ /**
\brief Crovides the current value of counter register IPDISRXB.

\ingroup func_UC
\b Description: \n
   This function provides the current value of the counter for receive Ethernet
   frames discarded because of missing Rx buffer resources.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pulRegister
              Output pointer to error counter of discarded Rx frames\n
              low word: port 1\n
              high word: port 2 

\return       none

\author       wk
\date         12.09.2006

***************************************************************************** */
CSMD_VOID  CSMD_HAL_GetCounter_DiscardResRxBuf( CSMD_HAL   *prCSMD_HAL,
                                                CSMD_ULONG *pulRegister )
{
  
  /* Counter for dicarded receive Ethernet frames based on missing rx buffer ressources */
  *pulRegister = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->rIPDISRXB.ulErrCnt) );
  
} /* end: CSMD_HAL_GetCounter_DiscardResRxBuf */



/**************************************************************************/ /**
\brief Provides the current value of counter register IPDISCLB.

\ingroup func_UC
\b Description: \n
   This function provides the current value of the counter for receive Ethernet
   frames discarded because of missing collision buffer resources.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pulRegister
              Output pointer to error counter of discarded Rx frames\n
              low word: port 1\n
              high word: port 2 

\return       none

\author       wk
\date         12.09.2006

***************************************************************************** */
CSMD_VOID  CSMD_HAL_GetCounter_DiscardColRxBuf( CSMD_HAL   *prCSMD_HAL,
                                                CSMD_ULONG *pulRegister )
{
  
  /* Counter for dicarded forwarding Ethernet frames based on missing collision buffer ressources */
  *pulRegister = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->rIPDISCOLB.ulErrCnt) );
  
}   /* end: CSMD_HAL_GetCounter_DiscardResRxBuf */



/**************************************************************************/ /**
\brief Provides the current value of counter register IPCHVIOL.

\ingroup func_UC
\b Description: \n
   This function provides the current value of the counter for Ethernet frames
   violating IP channel time boundaries.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pulRegister
              Output pointer to error counter of frames violating IP channel
              time boundaries\n
              low word: port 1\n
              high word: port 2 

\return       none

\author       wk
\date         12.09.2006

***************************************************************************** */
CSMD_VOID  CSMD_HAL_GetCounter_IPChViolation( CSMD_HAL   *prCSMD_HAL,
                                              CSMD_ULONG *pulRegister )
{
  
  /* Counter for Ethernet frames which violate the IP channel window */
  *pulRegister = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->rIPCHVIOL.ulErrCnt) );
  
}  /* end: CSMD_HAL_GetCounter_IPChViolation */



/**************************************************************************/ /**
\brief Provides the current value of counter register IPSERCERR.

\ingroup func_UC
\b Description: \n
   This function provides the current value of the counter for misaligned frames 
   or Ethernet frames received with defective frame check sequence FCS.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_HAL
              Pointer to the memory range allocated for the CoSeMa HAL structure
\param [out]  pulRegister
              Output pointer to Sercos error counter
              low word: port 1\n
              high word: port 2

\return       none

\author       wk
\date         26.11.2008

***************************************************************************** */
CSMD_VOID  CSMD_HAL_GetCounter_SercosError( CSMD_HAL   *prCSMD_HAL,
                                            CSMD_ULONG *pulRegister )
{
  
  /* Read Counter for Ethernet frames with FCS errors or are misaligned */
  *pulRegister = CSMD_END_CONV_L( (prCSMD_HAL->prSERC_Reg->rIPSERCERR.ulErrCnt) );
  
}  /* end: CSMD_HAL_GetCounter_SercosError */



/**************************************************************************/ /**
\brief Initializes the call-back functions to IP driver
       and the instance information.

\ingroup func_UC
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   pvCB_Info
              Distinguish the instance
\param [in]   prCB_Functions
              Pointer to list of callback functions

\return       0

\author       wk
\date         30.06.2006

***************************************************************************** */
CSMD_ULONG  CSMD_HAL_InitDriverCB( CSMD_INSTANCE     *prCSMD_Instance,
                                   CSMD_VOID         *pvCB_Info,
                                   CSMD_CB_FUNCTIONS *prCB_Functions )
{
  
  /* Function pointer to inform about changing in Sercos
     FPGA Rx/Tx-Ram allocation.                               */
  prCSMD_Instance->rPriv.rCbFuncTable.RxTxRamAlloc = 
    prCB_Functions->RxTxRamAlloc;
  
  /* Function pointer to inform about Sercos events like
     communication phase change etc..
     This function is called in Task context only.            */
  prCSMD_Instance->rPriv.rCbFuncTable.S3Event = 
    prCB_Functions->S3Event;
  
  /* Function pointer to inform about Sercos events
     take place in interrupt context like "Topology Change"
     This function is called in interrupt context only.       */ 
  prCSMD_Instance->rPriv.rCbFuncTable.S3EventFromISR = 
    prCB_Functions->S3EventFromISR;
  
  /* Instance Information from IP driver                      */
  prCSMD_Instance->rPriv.pvCB_Info = pvCB_Info;
  
  return (0);
  
} /* end: CSMD_HAL_InitDriverCB. */



/**************************************************************************/ /**
\brief Provides the current allocation of Tx ram and Rx ram.

\ingroup func_UC
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [in]   pvCB_Info
              Distinguish the instance
\param [in]   prSIII_Info
              Pointer to Sercos ram layout structure

\return       0

\author       wk
\date         06.08.2009

***************************************************************************** */
CSMD_ULONG  CSMD_HAL_GetRxTxRamAlloc( CSMD_INSTANCE  *prCSMD_Instance,
                                      CSMD_VOID      *pvCB_Info,
                                      CSMD_SIII_INFO *prSIII_Info )
{
  
  if (pvCB_Info == prCSMD_Instance->rPriv.pvCB_Info)
  {
    /* Allocated FPGA Tx-Ram for Sercos telegrams (IP segment length aligned) */
    prSIII_Info->ulTxRamS3Used = prCSMD_Instance->rCSMD_HAL.ulTxRamInUse;
    
    /* Size of available FPGA Tx-Ram for Sercos and IP telegrams */
    prSIII_Info->ulTxRamSize   = prCSMD_Instance->rCSMD_HAL.ulSizeOfTxRam;
    
    
    /* Allocated FPGA Rx-Ram for Sercos telegrams (IP segment length aligned) */
    prSIII_Info->ulRxRamS3Used = prCSMD_Instance->rCSMD_HAL.ulRxRamInUse;
    
    /* Size of available FPGA Rx-Ram for Sercos and IP telegrams */
    prSIII_Info->ulRxRamSize   = prCSMD_Instance->rCSMD_HAL.ulSizeOfRxRam;
    
    return (0);
  }
  else
  {
    /* Wrong IP driver instance */
    return (1);
  }
  
} /* end: CSMD_HAL_GetRxTxRamAlloc. */



/**************************************************************************/ /**
\brief Provides the collected status information of the Ethernet connection.

\ingroup func_UC
\b Description: \n
   This function reads the status of the Ethernet connection from Data Flow
   Control and Status Register DFCSR.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [out]  prStatus
              Output pointer to status information relevant for IP

\return       0

\author       wk
\date         30.06.2006

***************************************************************************** */
CSMD_ULONG CSMD_HAL_GetConStatus( CSMD_INSTANCE *prCSMD_Instance,
                                  CSMD_STATUS   *prStatus )
{
  
  CSMD_ULONG ulDFCSR = prCSMD_Instance->rCSMD_HAL.prSERC_Reg->rDFCSR.ulLong;
  
  prStatus->usNumOfSlavesP1 = prCSMD_Instance->rPriv.rSlaveAvailable.usAddressNmb;
  
  prStatus->usNumOfSlavesP2 = prCSMD_Instance->rPriv.rSlaveAvailable2.usAddressNmb;
  
  prStatus->usLinkStatusP1 = 
    (CSMD_USHORT) ((ulDFCSR & (1UL << CSMD_HAL_DFCSR_LINK_P1)) == (1UL << CSMD_HAL_DFCSR_LINK_P1));
  
  prStatus->usLinkStatusP2 = 
    (CSMD_USHORT) ((ulDFCSR & (1UL << CSMD_HAL_DFCSR_LINK_P2)) == (1UL << CSMD_HAL_DFCSR_LINK_P2));
  
  prStatus->usRingStatus = 
    prCSMD_Instance->usCSMD_Topology;
  
  prStatus->sSERCOS_Phase = 
    prCSMD_Instance->sCSMD_Phase;
  
  return (0);
  
} /* end: CSMD_HAL_GetConStatus. */



/**************************************************************************/ /**
\brief Provides the MTU value requested by the master.

\ingroup func_UC
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function.

\param [in]   prCSMD_Instance
              Pointer to memory range allocated for the variables of the 
              CoSeMa instance
\param [out]  pusMTU
              Output pointer to currently requested MTU value

\return       0

\author       AlM
\date         07.12.2010

***************************************************************************** */
CSMD_ULONG CSMD_HAL_GetMTU( CSMD_INSTANCE *prCSMD_Instance,
                            CSMD_USHORT   *pusMTU )
{
  
  *pusMTU = (CSMD_USHORT) CSMD_END_CONV_L( (prCSMD_Instance->rCSMD_HAL.prSERC_Reg->ulIPLASTFL) );

  return (0);
  
} /* end: CSMD_HAL_GetMTU. */



#ifdef CSMD_IP_HEADER
/**************************************************************************/ /**
\brief Checks defines and size of structures in the header file
       SIII_IP.h at compiler time.

\ingroup func_UC
\b Description: \n
   No further description.

<B>Call Environment:</B> \n
   Function should not be called!

\return       none

\author       GB
\date         29.07.2009

***************************************************************************** */
/*lint -save -e749  (Info -- local enumeration constant 'assert_line_2 */
/*lint -save -e753  (Info -- local enum '(untagged)' */
/*lint -save -e514  (Warning -- Unusual use of a Boolean expression) */
/*lint -save -e641  (Warning -- Unusual use of a Boolean expression) */
CSMD_VOID CSMD_HAL_CheckTypes( CSMD_VOID )
{
  
  #define CSMD_ASSERT_CONCAT_(a, b) a##b
  #define CSMD_ASSERT_CONCAT(a, b) CSMD_ASSERT_CONCAT_(a, b)
  /* These can't be used after statements in c89. */
  #ifdef __COUNTER__
    /* microsoft */
    #define CSMD_STATIC_ASSERT(e) \
      enum { CSMD_ASSERT_CONCAT(static_assert_, __COUNTER__) = 1/(!!(e)) }
  #else
    /* This can't be used twice on the same line so ensure if using in headers
     * that the headers are not included twice (by wrapping in #ifndef...#endif)
     * Note it doesn't cause an issue when used on same line of separate modules
     * compiled with gcc -combine -fwhole-program.  */
    #define CSMD_STATIC_ASSERT(e) \
      enum { CSMD_ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }
  #endif

  CSMD_STATIC_ASSERT( SIII_IP_NO_EVENT                   == CSMD_SIII_NO_EVENT );
  CSMD_STATIC_ASSERT( SIII_IP_LINK_DOWN_PORT1            == CSMD_LINK_DOWN_PORT1 );
  CSMD_STATIC_ASSERT( SIII_IP_LINK_DOWN_PORT2            == CSMD_LINK_DOWN_PORT2 );
  CSMD_STATIC_ASSERT( SIII_IP_RING_BREAK                 == CSMD_SIII_RING_BREAK );
  CSMD_STATIC_ASSERT( SIII_IP_RING_CLOSED                == CSMD_SIII_RING_CLOSED );
  CSMD_STATIC_ASSERT( SIII_STOP_COMMUNICATION            == CSMD_SIII_STOP_COMMUNICATION );
  CSMD_STATIC_ASSERT( SIII_START_COMMUNICATION           == CSMD_SIII_START_COMMUNICATION );
  CSMD_STATIC_ASSERT( sizeof (SIII_IP_EVENT)             == sizeof (CSMD_SIII_EVENT) );
  CSMD_STATIC_ASSERT( sizeof (SIII_IP_CB_FUNCTIONS)      == sizeof (CSMD_CB_FUNCTIONS) );
  CSMD_STATIC_ASSERT( sizeof (SIII_IP_INFO)              == sizeof (CSMD_SIII_INFO));
  CSMD_STATIC_ASSERT( SIII_IP_NO_LINK                    == CSMD_NO_LINK );
  CSMD_STATIC_ASSERT( SIII_IP_TOPOLOGY_LINE_P1           == CSMD_TOPOLOGY_LINE_P1 );
  CSMD_STATIC_ASSERT( SIII_IP_TOPOLOGY_LINE_P2           == CSMD_TOPOLOGY_LINE_P2 );
  CSMD_STATIC_ASSERT( SIII_IP_TOPOLOGY_BROKEN_RING       == CSMD_TOPOLOGY_BROKEN_RING );
  CSMD_STATIC_ASSERT( SIII_IP_TOPOLOGY_RING              == CSMD_TOPOLOGY_RING );
  CSMD_STATIC_ASSERT( SIII_IP_TOPOLOGY_MASK              == CSMD_TOPOLOGY_MASK );
  CSMD_STATIC_ASSERT( SIII_IP_TOPOLOGY_DEFECT_RING       == CSMD_TOPOLOGY_DEFECT_RING );
  CSMD_STATIC_ASSERT( SIII_IP_HAL_IP_NO_PORT             == CSMD_HAL_IP_NO_PORT );
  CSMD_STATIC_ASSERT( SIII_IP_HAL_IP_PORT1               == CSMD_HAL_IP_PORT1 );
  CSMD_STATIC_ASSERT( SIII_IP_HAL_IP_PORT2               == CSMD_HAL_IP_PORT2 );
  CSMD_STATIC_ASSERT( SIII_IP_HAL_IP_BOTH_PORTS          == CSMD_HAL_IP_BOTH_PORTS );
  CSMD_STATIC_ASSERT( sizeof (SIII_IP_STATUS)            == sizeof (CSMD_STATUS) );

} /* end: CSMD_HAL_CheckTypes. */
/*lint -restore (641 514 753 749) */
#endif  /* #ifdef CSMD_IP_HEADER */



/*---- Definition private Functions: -----------------------------------------*/

/*! \endcond */ /* HAL_UC */

/*lint -restore */



/*
--------------------------------------------------------------------------------
  Modification history
--------------------------------------------------------------------------------

02 Sep 2010
  - File created.
21 Nov 2013 WK
  - Defdb00000000
    Removed function CSMD_HAL_ResetCounter_SercosError().
06 Feb 2014 WK
  - Defdb00166929
    Added function CSMD_HAL_GetIPControlRegister().
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
05 Nov 2015 WK
  - Defdb00182757
    Including the header CSMD_USER.h and CSMD_TYPE_HEADER removed.
  
--------------------------------------------------------------------------------
*/
