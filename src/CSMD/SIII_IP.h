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
\file   SIII_IP.h
\author GB
\date   29.07.2009
\brief  This File contains all the CoSeMa related
        definitions for a UC channel driver.
*/


#ifndef _SIII_IP
#define _SIII_IP


/*---- Definition resp. Declaration public Constants and Macros: -------------*/


/* -------------------------------------------------------------------------- */
/*! \brief Enumeration of callback events to ip driver                        */
/* -------------------------------------------------------------------------- */
typedef enum SIII_IP_EVENT_NBR_EN
{
  SIII_IP_NO_EVENT = 0,       /*!< unused */
  
  /*! Link Events */
  SIII_IP_LINK_DOWN_PORT1,    /*!< unused */
  SIII_IP_LINK_DOWN_PORT2,    /*!< unused */
  
  /*! Topology Events */
  SIII_IP_RING_BREAK,         /*!< Ring break detected   */
  SIII_IP_RING_CLOSED,        /*!< Ring closure detected */
  
  /*! Sercos Events */
  SIII_STOP_COMMUNICATION,    /*!< Shall stop communication in the UC channel  */
  SIII_START_COMMUNICATION    /*!< Shall start communication in the UC channel */
  
  /* Timer Events  */
  
} SIII_IP_EVENT_NBR;

/* -------------------------------------------------------------------------- */
/*! \brief callback function: information parameter                           */
/* -------------------------------------------------------------------------- */
typedef struct SIII_IP_EVENT_STR
{
  SIII_IP_EVENT_NBR     eEventId;
    /* other data required? */
} SIII_IP_EVENT;

/* -------------------------------------------------------------------------- */
/*! \brief Table of callback functions to ip driver                           */
/* -------------------------------------------------------------------------- */
typedef struct SIII_IP_CB_FUNCTIONS_STR
{
  CSMD_ULONG (*RxTxRamAlloc)   ( CSMD_VOID     *pvCB_Info,      /*!< distinguish the instance       */
                                 /* The following values are segment (256 bytes) length aligned     */
                                 CSMD_ULONG     ulTxRamS3Used,  /*!< Tx-Ram: used for Sercos frames */
                                 CSMD_ULONG     ulTxRamSize,    /*!< Tx-Ram: total available size   */
                                 CSMD_ULONG     ulRxS3Used,     /*!< Rx-Ram: used for Sercos frames */
                                 CSMD_ULONG     ulRxRamSize );  /*!< Rx-Ram: total available size   */
  
  CSMD_ULONG (*S3Event)        ( CSMD_VOID     *pvCB_Info,      /*!< distinguish the instance */
                                 SIII_IP_EVENT *prEvent );      /*!< Sercos event information */
  
  CSMD_ULONG (*S3EventFromISR) ( CSMD_VOID     *pvCB_Info,      /*!< distinguish the instance */
                                 SIII_IP_EVENT *prEvent );      /*!< Sercos event information */
  
} SIII_IP_CB_FUNCTIONS;

/* -------------------------------------------------------------------------- */
/*! \brief Sercos communication FPGA Ram layout information structure         */
/* -------------------------------------------------------------------------- */
typedef struct SIII_IP_INFO_STR
{
  /*! Allocated FPGA Tx-Ram for Sercos telegrams (IP segment length aligned) */
  CSMD_ULONG   ulTxRamS3Used;
  
  /*! Size of available FPGA Tx-Ram for Sercos and IP telegrams */
  CSMD_ULONG   ulTxRamSize;
  
  /*! Allocated FPGA Rx-Ram for Sercos telegrams (IP segment length aligned) */
  CSMD_ULONG   ulRxRamS3Used;
  
  /*! Size of available FPGA Rx-Ram for Sercos and IP telegrams */
  CSMD_ULONG   ulRxRamSize;
  
} SIII_IP_INFO;

/*---------------------------------------------------- */
/* Sercos topology                                     */
/*---------------------------------------------------- */
#define SIII_IP_NO_LINK                 (0x0000)
#define SIII_IP_TOPOLOGY_LINE_P1        (0x0001)
#define SIII_IP_TOPOLOGY_LINE_P2        (0x0002)
#define SIII_IP_TOPOLOGY_BROKEN_RING    (SIII_IP_TOPOLOGY_LINE_P1 | SIII_IP_TOPOLOGY_LINE_P2)
#define SIII_IP_TOPOLOGY_RING           (0x0004)
#define SIII_IP_TOPOLOGY_MASK           (0x0007)
#define SIII_IP_TOPOLOGY_DEFECT_RING    (0x0008)

/*---------------------------------------------------- */
/* Port definitions */
/*---------------------------------------------------- */
#define SIII_IP_HAL_IP_NO_PORT         (0)    /* no affect to any PHY interface */
#define SIII_IP_HAL_IP_PORT1           (1)    /* PHY interface port 1 related */
#define SIII_IP_HAL_IP_PORT2           (2)    /* PHY interface port 2 related */
#define SIII_IP_HAL_IP_BOTH_PORTS      (3)    /* PHY interface port 1 and 2 related */


/*--------------------------------------------------------------------------- */
/*! \brief CoSeMa communication status structure                              */
/*--------------------------------------------------------------------------- */
typedef struct SIII_IP_STATUS_STR
{
  CSMD_USHORT  usNumOfSlavesP1;               /*!< Number of operable slaves connected to port 1 */
  CSMD_USHORT  usNumOfSlavesP2;               /*!< Number of operable slaves connected to port 2 */
  CSMD_USHORT  usRingStatus;                  /*!< Ring, single line, broken ring (topology) */
  CSMD_USHORT  usLinkStatusP1;                /*!< Link status port 1 (1 = Link is active) */
  CSMD_USHORT  usLinkStatusP2;                /*!< Link status port 2 (1 = Link is active) */
  CSMD_SHORT   sSERCOS_Phase;                 /*!< Communication phase (Sercos phase) */
  
} SIII_IP_STATUS;


#endif /* _SIII_IP */



/*
--------------------------------------------------------------------------------
  Modification history
--------------------------------------------------------------------------------

29.Jul 2009
  - File created.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
  
--------------------------------------------------------------------------------
*/
