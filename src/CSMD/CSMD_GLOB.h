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
\file   CSMD_GLOB.h
\author MSt
\date   11.01.2005
\brief  This File contains the public API function prototypes
        and macro definitions.
*/

/*---- Includes: -------------------------------------------------------------*/

#ifndef _CSMD_GLOB
#define _CSMD_GLOB

#include "CSMD_USER.h"
#include  CSMD_TYPE_HEADER

#include "CSMD_ERR_CODES.h"

#include "CSMD_HAL_GLOB.h"

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif



#ifndef NULL
  #define NULL  (0)
#endif


#if ((CSMD_MAX_TX_BUFFER == 0) || (CSMD_MAX_TX_BUFFER > 3))
#error csmd_user.h  Tx Single-, double- or triple-buffer system must be selected !
#endif

#if ((CSMD_MAX_RX_BUFFER == 0) || (CSMD_MAX_RX_BUFFER > 3))
#error csmd_user.h  Rx Single-, double- or triple-buffer system must be selected !
#endif

#if defined CSMD_PCI_MASTER && (CSMD_MAX_HW_CONTAINER & 1)
#error csmd_user.h: With PCI Busmaster DMA, the number of HW SVC container shall be divisible by 2 !
#endif

#if ((CSMD_CONN_NAME_LENGTH % 4) != 2)
#error csmd_user.h: The length shall be divisible by 2 and not by 4 to insure proper function of the parser!
#endif

#if defined CSMD_SYNC_DPLL && (CSMD_MAX_HW_CONTAINER > 0)
#error csmd_user.h: With DPLL, the hardware service channel processor is not available !
#endif

#if (CSMD_MAX_RT_BITS_PER_CONN > 2)
#error csmd_user.h: Take care with the size of the CSMD_RD_WR_BUFFER (4 longs) !
#endif

#ifdef CSMD_DOXYGEN
#error csmd_user.h: This macro is defined only in the doxygen configuration file for generating the documentation!.
#endif

#if (defined CSMD_FAST_STARTUP) && (!defined CSMD_STABLE_TSREF)
#error csmd_user.h: The Fast Start feature also requires the definition of CSMD_STABLE_TSREF !
#endif
  
/*---- Definition resp. Declaration public Constants and Macros: -------------*/

/*! \cond PUBLIC */

/*---------------------------------------------------- */
/* master ports                                        */
/*---------------------------------------------------- */
#define CSMD_PORT_1                 0
#define CSMD_PORT_2                 1
#define CSMD_NBR_PORTS              2       /* number of Sercos ports */

/*---------------------------------------------------- */
/* Sercos cycle time constants [nano seconds]          */
/*---------------------------------------------------- */
#define CSMD_TSCYC_MIN                         31250
#define CSMD_TSCYC_62_5_US                     62500
#define CSMD_TSCYC_125_US                     (  125 * 1000)
#define CSMD_TSCYC_250_US                     (  250 * 1000)
#define CSMD_TSCYC_500_US                     (  500 * 1000)
#define CSMD_TSCYC_1_MS                       ( 1000 * 1000)
#define CSMD_TSCYC_2_MS                       ( 2000 * 1000)
#define CSMD_TSCYC_MAX                        (65000 * 1000)

/*---------------------------------------------------- */
/* Sercos timing constants [nano seconds]              */
/*---------------------------------------------------- */

#define CSMD_FPGA_MDT_START_TIME                100           /* Time of IP-Core Event Start MDT [ns] */
#define CSMD_MAX_TIME_ETHERNET_FRAME          ( 125 * 1000)   /* Maximum time for Ethernet frames rounded up to 125 microsecond */

/*---------------------------------------------------- */
/* Sercos timing constants for CP0 [nano seconds]      */
/*---------------------------------------------------- */

#define CSMD_FPGA_MST_WINDOW_OPEN_P0_2            0           /* MST window open time */
#define CSMD_FPGA_MST_WINDOW_CLOSE_CP0        ( 990 * 1000)   /* MST window close time */
#define CSMD_FPGA_AT_START_TIME_CP0           ( 300 * 1000)   /* AT  start time */

#define CSMD_FPGA_IP_WINDOW_OPEN_CP0          ( 650 * 1000)   /* IP window open time */
#define CSMD_FPGA_IP_LAST_TRASMISS_CP0        ( 825 * 1000)   /* IP last transmission start time */
#define CSMD_FPGA_IP_WINDOW_CLOSE_CP0         ( 950 * 1000)   /* IP window close time */

#define CSMD_FPGA_TX_BUF_CHNG_REQ_CP0         ( 900 * 1000)   /* default time for Tx buffer change request event in CP0. */
/*      CSMD_FPGA_RX_BUF_CHNG_REQ_CP0         ( 700 * 1000)*/ /* default time for Rx buffer change request event in CP0. */

/*----------------------------------------------------------------------------------- */
/* Sercos timing constants for CP1 / CP2 (Fixed mode with 2 MDT / 2 AT [nano seconds] */
/*----------------------------------------------------------------------------------- */

#define CSMD_FPGA_AT_START_TIME_CP12          ( 300 * 1000)   /* AT  start time */
#define CSMD_FPGA_IP_WINDOW_OPEN_CP12         ( 650 * 1000)   /* IP window open time */
#define CSMD_FPGA_IP_LAST_TRANSMISS_CP12      ( 825 * 1000)   /* IP last transmission start time */
#define CSMD_FPGA_IP_WINDOW_CLOSE_CP12        ( 950 * 1000)   /* IP window close time */
#define CSMD_FPGA_SERCOS_CYCLE_TIME_P0_2      (1000 * 1000)   /* Default value Sercos cycle time */

#define CSMD_FPGA_TX_BUF_CHNG_REQ_CP12        ( 900 * 1000)   /* default time for Tx buffer change request event in CP1/CP2. */
/*      CSMD_FPGA_RX_BUF_CHNG_REQ_CP12        ( 700 * 1000)*/ /* default time for Rx buffer change request event in CP1/CP2. */

/* Examples for timer interrups */
#define CSMD_FPGA_INT0_P0_2                   ( 600 * 1000)   /* Data processing (CSMD_CyclicHandling) */

/*----------------------------------------------------------------------------------- */
/* Sercos timing constants for CP1 / CP2 (Fixed mode with 4 MDT / 4 AT [nano seconds] */
/*----------------------------------------------------------------------------------- */

#define CSMD_FPGA_AT_START_TIME_CP12_4TEL     ( 520 * 1000)   /* AT  start time */
#define CSMD_FPGA_IP_WIN_OPEN_CP12_4TEL       (1050 * 1000)   /* IP window open time */
#define CSMD_FPGA_IP_LAST_TRANS_CP12_4TEL     (1825 * 1000)   /* IP last transmission start time */
#define CSMD_FPGA_IP_WIN_CLOSE_CP12_4TEL      (1950 * 1000)   /* IP window close time */

#define CSMD_FPGA_TX_BUF_CHNG_REQ_CP12_4TEL   (1800 * 1000)   /* default time for Tx buffer change request event in CP1/CP2.and 4MDT/4AT */
/*      CSMD_FPGA_RX_BUF_CHNG_REQ_CP12_4TEL   (1100 * 1000)*/ /* default time for Rx buffer change request event in CP1/CP2.and 4MDT/4AT */

/* Examples for timer interrups */
#define CSMD_FPGA_INT0_P0_2_4TEL              (1200 * 1000)   /* Data processing (CSMD_CyclicHandling) */



/* -------------------------------------------------------------------------- */
/*! \brief Preset for timing mode of the UC channel in CP1 and CP2.           */
/* -------------------------------------------------------------------------- */
typedef enum CSMD_UCC_MODE_EN
{
  CSMD_UCC_MODE_CP12_FIX = 0,   /*!< fixed UC channel according to previous specification   */
#ifdef CSMD_SWC_EXT
  CSMD_UCC_MODE_CP12_1,         /*!< UC channel with maximum width (MDT / AT / UCC)         */
  CSMD_UCC_MODE_CP12_2,         /*!< UC channel with maximum width (MDT / UCC / AT)         */
  CSMD_UCC_MODE_CP12_1_VAR,     /*!< Centered UC channel with given width (MDT / AT / UCC)  */
#endif
  CSMD_UCC_MODE_VARIANTS        /* number of timings variants                               */
  
} CSMD_UCC_MODE_ENUM;


/* -------------------------------------------------------------------------- */
/*! \brief Structure that contains all timing constants for the possible
           timing variants in CP1 and CP2.                                    */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_TIMING_CP12_STR
{
  
  CSMD_ULONG   ulCycleTime_CP0;    /* Sercos cycle time for CP0 [ns]           */
  CSMD_ULONG   ulCycleTime_CP1_2;  /* Sercos cycle time for CP1/CP2 [ns]       */
  CSMD_ULONG   ulStartAT_t1;       /* AT0 transmission starting time (t1) [ns] */
  CSMD_ULONG   ulOpenUCC_t6;       /* Begin of the UC channel (t6) [ns]        */
  CSMD_ULONG   ulCloseUCC_t7;      /* End of the UC channel (t7) [ns]          */
  
  CSMD_ULONG   ulUCC_Latest;       /* Latest transmission point in the UC channel (t7) [ns] */
  CSMD_ULONG   ulTxBufReqA;        /* Tx buffer change request buffer system A event [ns]  */
  CSMD_ULONG   ulRxBufReqA;        /* Rx buffer change request buffer system A event [ns]  */
  
} CSMD_TIMING_CP12_STRUCT;


#ifdef CSMD_SWC_EXT
#define CSMD_AT0_CP0_SLAVE_ACK  0x8000U   /* Bit 15: AT0-CP0 slave acknowledgment for the requested add-ons
                                                     of Communication version field in MDT0-CP0               */
#endif


#define CSMD_SVC_BYTESIZE       (6)     /* byte size of service channel */
#define CSMD_SVC_WORDSIZE       (3)     /* word size of service channel */

/* Number of Sercos MDT- or AT-telegrams */
#define CSMD_TELEGRAM_NBR_0               0U
#define CSMD_TELEGRAM_NBR_1               1U
#define CSMD_TELEGRAM_NBR_2               2U
#define CSMD_TELEGRAM_NBR_3               3U

#define CSMD_MAX_TEL                      4U    /* maximum number of Sercos MDT- or AT-telegrams */


#define CSMD_MIN_SLAVE_ADD                1U    /* Minimum Sercos address for operating in CP3 to CP4 (according to Sercos specification) */
#define CSMD_MAX_SLAVE_ADD              511U    /* Maximum Sercos address (according to Sercos specification) */
#define CSMD_MAX_SLAVES_PER_TEL_CP1_2   128U    /* maximum number of slaves in one telegram in CP1 and CP2 */

#ifdef CSMD_4MDT_4AT_IN_CP1_2
#define CSMD_MAX_TEL_P0_2                 4U    /* max. nbr. of MDT- or AT-telegrams in CP0 to CP2 */
#define CSMD_NBR_SCAN_SLAVES             512    /* Topology addresses 1 to 511 */
#else
#define CSMD_MAX_TEL_P0_2                 2U    /* max. nbr. of MDT- or AT-telegrams in CP0 to CP2 */
#define CSMD_NBR_SCAN_SLAVES             256    /* Topology addresses 1 to 255 */
#endif


#define CSMD_MAX_SLAVE_IDX        (CSMD_MAX_SLAVES-1)       /* upper bound of slave index */

#define CSMD_SLAVE_NBR_LIMIT      (CSMD_MAX_SLAVE_ADD + 1) /* upper bound for loops of kind "slave_add" */



/*---------------------------------------------------- */
/* Sercos Topology                                     */
/*---------------------------------------------------- */
#define CSMD_NO_LINK                  0x0000U
#define CSMD_TOPOLOGY_LINE_P1         0x0001U
#define CSMD_TOPOLOGY_LINE_P2         0x0002U
#define CSMD_TOPOLOGY_BROKEN_RING     (CSMD_TOPOLOGY_LINE_P1 | CSMD_TOPOLOGY_LINE_P2)  /* Replaces CSMD_TOPOLOGY_DOUBLE_LINE */
#define CSMD_TOPOLOGY_RING            0x0004U
#define CSMD_TOPOLOGY_MASK            0x0007U
#define CSMD_TOPOLOGY_DEFECT_RING     0x0008U


/* Timing Method */
#define CSMD_METHOD_MDT_AT_IPC        1U      /* timing method 1 */
#define CSMD_METHOD_MDT_IPC_AT        2U      /* timing method 2 */
#define CSMD_METHOD_AT_CYC_END        3U      /* timing method 3 */

/*-------------------------------------------------- */
/* Communication phase (Sercos Phase)                */
/*-------------------------------------------------- */
#define CSMD_SERC_PHASE_NRT_STATE    -1
#define CSMD_SERC_PHASE_0             0
#define CSMD_SERC_PHASE_1             1
#define CSMD_SERC_PHASE_2             2
#define CSMD_SERC_PHASE_3             3
#define CSMD_SERC_PHASE_4             4

#define CSMD_CPS_CURRENT_CP           0U
#define CSMD_CPS_NEW_CP               1U

#ifdef CSMD_HOTPLUG
/*-------------------------------------------------------- */
/* Definitions for Hot-Plug phase (usHP_Phase)             */
/*-------------------------------------------------------- */
#define CSMD_NON_HP_MODE              0xFFFFU
#define CSMD_HP_PHASE_0               0U
#define CSMD_HP_PHASE_1               1U
#define CSMD_HP_PHASE_2               2U
#endif


/*---------------------------------------------------- */
/* Wait time constants [Milliseconds]                  */
/*---------------------------------------------------- */
#define CSMD_PHASE_WAIT             2000U     /* not used by CoSeMa */
#define CSMD_WAIT_1MS                  1U
#define CSMD_WAIT_2MS                  2U
#define CSMD_WAIT_4MS                  4U
#define CSMD_WAIT_10MS                10U
#define CSMD_WAIT_100MS              100U
/* During Communication phase change to CP0/CP1/CP2/CP3 stop transmission of MDTs/ATs at least 120 ms */
#define CSMD_WAIT_120MS              120U

#define CSMD_WAIT_200MS              200U
#define CSMD_WAIT_500MS              500U     /* not used by CoSeMa */

/* Maximum sleeptime [ms] at maximum cycle time for functions state machine */
#define CSMD_WAIT_MAX               ((CSMD_ULONG)((CSMD_TSCYC_MAX * 12) / 1000))

#define CSMD_TIMEOUT_FPGA_RESET     4000U
#define CSMD_TIMEOUT_PHY_RESET      4000U

#define CSMD_SVC_BUSY_TIEMOUT_DEFAULT   1000UL   /* SVC busy timeout default value [ms] */


/* -------------------------------------------------------------------------- */
/* Round to second decimal place                                              */
/* -------------------------------------------------------------------------- */
#define CSMD_RND_UP_10( _uint )     ((((_uint) +  9) / 10) * 10)
#define CSMD_RND_DOWN_10( _uint )   (((_uint) / 10) * 10)

/* Round up to the next divisible by 2 without a remainder */
#define CSMD_RND_UP_2(__uint)   ((CSMD_ULONG)(((__uint) + 1U) & ~1U))


/* -------------------------------------------------------------------------- */
/* States of the SVC macro function state machine                             */
/* -------------------------------------------------------------------------- */
#define CSMD_START_REQUEST          0x00U
#define CSMD_INIT_SVCH              0x01U
#define CSMD_CHANNEL_OPEN           0x02U
#define CSMD_ATTRIBUTE_VALID        0x03U
#define CSMD_DATA_VALID             0x04U
#define CSMD_SET_CMD                0x05U
#define CSMD_CMD_ACTIVE             0x06U
#define CSMD_CLEAR_CMD              0x07U
#define CSMD_CMD_CLEARED            0x08U
#define CSMD_GET_CMD_STATUS         0x0AU
#define CSMD_CHECK_CMD              0x0BU
#define CSMD_CMD_STATUS_VALID       0x0CU
#define CSMD_REQUEST_ERROR          0x0DU

#define CSMD_GET_ATTRIBUTE          0x13U   /* is essential for big endian */
#define CSMD_DATA_NOT_VALID         0x14U


/* -------------------------------------------------------------------------- */
/* States of the SVC atomic function state machine                            */
/* -------------------------------------------------------------------------- */
#define CSMD_INIT_REQUEST           0x20U
#define CSMD_REQUEST_IN_PROGRESS    0x21U
#define CSMD_LAST_STEP              0x22U
#define CSMD_FINISHED_REQUEST       0x23U


/* -------------------------------------------------------------------------- */
/* States of the function state machine                                       */
/* -------------------------------------------------------------------------- */
/*! Entry first execution step */
#define CSMD_FUNCTION_1ST_ENTRY             0U
#define CSMD_FUNCTION_STEP_1                1U
#define CSMD_FUNCTION_STEP_2                2U
#define CSMD_FUNCTION_STEP_3                3U
#define CSMD_FUNCTION_STEP_4                4U
#define CSMD_FUNCTION_STEP_5                5U
#define CSMD_FUNCTION_STEP_6                6U
#define CSMD_FUNCTION_STEP_7                7U
#define CSMD_FUNCTION_STEP_8                8U
#define CSMD_FUNCTION_STEP_9                9U
#define CSMD_FUNCTION_STEP_10             0xAU
#define CSMD_FUNCTION_STEP_11             0xBU
#define CSMD_FUNCTION_STEP_12             0xCU
#define CSMD_FUNCTION_STEP_13             0xDU
#define CSMD_FUNCTION_STEP_14             0xEU
#define CSMD_FUNCTION_STEP_15             0xFU
#define CSMD_FUNCTION_STEP_16            0x10U
#define CSMD_FUNCTION_STEP_17            0x11U

/*! Entry communication phase switch without execution of command S-0-1024 */
#define CSMD_FUNCTION_CP3_TRANS_CHECK    0x30U
/*! Clear possible active procedure command */
#define CSMD_FUNCTION_CLR_CMD            0x31U
/*! Set procedure command */
#define CSMD_FUNCTION_SET_CMD            0x32U
/*! Check if setting of procedure command is properly executed */
#define CSMD_FUNCTION_SET_CHECK          0x33U
/*! Clear procedure command again */
#define CSMD_FUNCTION_CLR_CMD_AGAIN      0x34U

/*! Entry communication phase switch without execution of CP3/CP4 transition check procedure command */
#define CSMD_FUNCTION_CHANGE_PHASE       0x35U
/*! Communication phase switch to CP0 or CP3: Change of clock mode possible (CYC_CLK) */
#define CSMD_FUNCTION_CHG_CLK_POSSIBLE   0x36U
/*! Execute CP4 transition check procedure command (CSMD_TransHP2Para()) */
#define CSMD_FUNCTION_CP4_TRANS_CHECK    CSMD_FUNCTION_STEP_14

/* Ring delay has changed, transmit new value to all slaves */
#define CSMD_FUNCTION_NEW_RINGDELAY      0x40U
/* Function state machine successful finished */
#define CSMD_FUNCTION_FINISHED          0x100U


/*---------------------------------------------------- */
/* Sercos SVC constants                                */
/*---------------------------------------------------- */
/* #define CSMD_SERC_WDATTYP      volatile unsigned short */   /* word data type */
#define CSMD_SVC_CONTAINER_NOT_IN_USE   0U
#define CSMD_SVC_CONTAINER_IN_USE       1U        /* use emulated service container */

/*--------------------------------------------------------------------------------------------- */
/* Type of Service Container (usSrv_Cont)                                                       */
/*--------------------------------------------------------------------------------------------- */
#define CSMD_HW_SRV_CONTAINER           0U        /* Flag for ASIC service container indication */
#define CSMD_SW_SRV_CONTAINER           1U        /* Flag for Software container indication */


/*-------------------------------------------------------------------------------------------- */
/* Data Block Elements (service channel)                                                       */
/*-------------------------------------------------------------------------------------------- */
#define CSMD_SVC_DBE0_CLOSE             0U        /* Closing the service channel  */
#define CSMD_SVC_DBE1_IDN               1U        /* IDN                          */
#define CSMD_SVC_DBE2_NAME              2U        /* Name                         */
#define CSMD_SVC_DBE3_ATTRIBUTE         3U        /* Attribute                    */
#define CSMD_SVC_DBE4_UNIT              4U        /* Unit                         */
#define CSMD_SVC_DBE5_MIN_VALUE         5U        /* Minimum value                */
#define CSMD_SVC_DBE6_MAX_VALUE         6U        /* Maximum Value                */
#define CSMD_SVC_DBE7_OPERATION_DATA    7U        /* Operation data               */


/*-------------------------------------------------------------------------- */
/* Attribute    Bits 22-20:     Data type and display format                 */
/*-------------------------------------------------------------------------- */
#define CSMD_ATTRIBUTE_FORMAT           0x00700000U   /* And-Mask: Get data format     */
#define CSMD_ATTRIBUTE_FORMAT_IDN       0x00500000U   /* IDN (unsigned integer)        */

/*-------------------------------------------------------------------------- */
/* Attribute    Bits 18-16:     Data length                                  */
/*-------------------------------------------------------------------------- */
#define CSMD_SERC_LEN                   0x00070000U   /* And-Mask: Get data length     */
#define CSMD_SERC_WORD_LEN              0x00010000U   /* Bit-Mask length word          */
#define CSMD_SERC_LONG_LEN              0x00020000U   /* Bit-Mask length long          */
#define CSMD_SERC_DOUBLE_LEN            0x00030000U   /* Bit-Mask length Eight Byte    */
#define CSMD_SERC_VAR_LEN               0x00040000U   /* Bit-Mask length variable      */
#define CSMD_SERC_VAR_BYTE_LEN          0x00040000U   /* Bit-Mask String (v.len.byte)  */
#define CSMD_SERC_VAR_WORD_LEN          0x00050000U   /* Bit-Mask variable length word  */
#define CSMD_SERC_VAR_LONG_LEN          0x00060000U   /* Bit-Mask variable length long  */
#define CSMD_SERC_VAR_DOUBLE_LEN        0x00070000U   /* Bit-Mask variable length Eight Byte */
#define CSMD_SERC_LEN_WO_LISTINFO       0x00030000U   /* Bit-Mask length without variable length info */

/*-------------------------------------------------------------------------------- */
/* Attribute    Bit 19:         Function of operation data                         */
/*-------------------------------------------------------------------------------- */
#define CSMD_SERC_OP_DATA_MASK          0x00080000U   /* AND-Mask: Get op. data function   */
#define CSMD_SERC_DATA_PAR              0x00000000U   /* Operation data or parameter       */
#define CSMD_SERC_PROC_CMD              0x00080000U   /* Procedure command      */

/*-------------------------------------------------------------------------------- */
/* Attribute    Bit 15-0:       Conversion factor                                  */
/*-------------------------------------------------------------------------------- */
#define CSMD_ATTR_CONV_FACTOR_1         0x00000001U   /* Conversion factor = 1     */

/* Attribute for a generic procedure command IDN */
#define CSMD_ATTR_GENERIC_PROC_CMD      (CSMD_SERC_PROC_CMD | CSMD_SERC_WORD_LEN | CSMD_ATTR_CONV_FACTOR_1)


/*-------------------------------------------------------------------------- */
/* Definitions to the command default over the service channel               */
/*  (Element 7)                                                              */
/*-------------------------------------------------------------------------- */
#define CSMD_CMD_CLEAR        0U
#define CSMD_CMD_SET          1U
#define CSMD_CMD_ENABLE       2U
#define CSMD_CMD_START        (CSMD_CMD_SET | CSMD_CMD_ENABLE)
/* #define CSMD_CMD_BREAK       (CSMD_CMD_SET) */

/*--------------------------------------------------------------------------- */
/* Definitions for procedure command acknowledgment over the SVC              */
/* (data status)                                                              */
/*--------------------------------------------------------------------------- */
#define CSMD_CMD_NOT_SET      0U
#define CSMD_CMD_IS_SET       1U
#define CSMD_CMD_RUNNING      7U
#define CSMD_CMD_STOPPED      6U
#define CSMD_CMD_FINISHED     3U
#define CSMD_CMD_ERROR      0xFU


/*-------------------------------------------------------------------------------------------- */
/* Sources for Interrupts (IRR0)                                                               */
/*-------------------------------------------------------------------------------------------- */

#define CSMD_INT_TINT0              0x00000001U
#define CSMD_INT_TINT1              0x00000002U
#define CSMD_INT_TINT2              0x00000004U
#define CSMD_INT_TINT3              0x00000008U
#define CSMD_INT_TINTMAX            0x00000010U
#define CSMD_INT_DIVCLK             0x00000080U
#define CSMD_INT_IPINT_P1           0x00000100U
#define CSMD_INT_IPINT_P2           0x00000200U

#define CSMD_INT_IPINT              (CSMD_INT_IPINT_P1 | CSMD_INT_IPINT_P2)     /* IP Port 1 and 2 */

/*-------------------------------------------------------------------------------------------- */
/* Event Identifier for CSMD_EventControl() (max nbr = CSMD_NBR_EVENT_ID)                      */
/*-------------------------------------------------------------------------------------------- */
#define CSMD_EVENT_ID_TIMER_0               (0)   /* Timing Event TINT0 (can operate as an interrupt source)  */
#define CSMD_EVENT_ID_TIMER_1               (1)   /* Timing Event TINT1 (can operate as an interrupt source)  */
#define CSMD_EVENT_ID_TIMER_2               (2)   /* Timing Event TINT2 (can operate as an interrupt source)  */
#define CSMD_EVENT_ID_TIMER_3               (3)   /* Timing Event TINT3 (can operate as an interrupt source)  */
#define CSMD_EVENT_ID_CONCLK_SET            (4)   /* Set   Con clock event                                    */
#define CSMD_EVENT_ID_CONCLK_RESET          (5)   /* Reset Con clock event                                    */
#define CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A    (6)   /* Transmit buffer change request buffer system A event     */
#define CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A    (7)   /* Receive  buffer change request buffer system A event     */


#if(0)
/* Definitions for request of IP communication status bits */
#define CSMD_IP_STATUS_INFO         0x0000000FU
#define CSMD_IP_RX_BUFFER_READY     0x00000001U
#define CSMD_IP_RX_BUFFER_FULL      0x00000002U
#define CSMD_IP_TX_BUFFER_READY     0x00000004U
#define CSMD_IP_TX_BUFFER_EMPTY     0x00000008U
#define CSMD_IP_LINK_INFO           0x00008000U

/* Definitions for IP communication control bits */
#define CSMD_IP_TX_ENABLE           0x00000001U
#define CSMD_IP_RX_ENABLE           0x00000002U
#define CSMD_BROADCAST_DISABLE      0x00000008U
#define CSMD_MULTICAST_DISABLE      0x00000010U
#define CSMD_PROMISCUOUS_MODE       0x00000040U
#endif


/*-------------------------------------------------------------- */
/* definitions for multiple buffering system for                 */
/* Tx and Rx realtime data                                       */
/*-------------------------------------------------------------- */

/* Definitions for rHW_Init_Struct.usTxBufferMode                */
/*! Activate TxMAC single buffer system */
#define CSMD_TX_SINGLE_BUFFER       0U
/*! Activate TxMAC double buffer system */
#define CSMD_TX_DOUBLE_BUFFER       1U
/*! Activate TxMAC triple buffer system */
#define CSMD_TX_TRIPLE_BUFFER       2U
/* Activate TxMAC quad   buffer system */
#define CSMD_TX_QUAD_BUFFER         3U    /* currently not realized */

/* Definitions for active Tx buffer                              */
#define CSMD_TX_BUFFER_0            0U
#define CSMD_TX_BUFFER_1            1U
#define CSMD_TX_BUFFER_2            2U

/* Definitions for rHW_Init_Struct.usRxBufferMode                */
/*! Activate RxMAC single buffer system */
#define CSMD_RX_SINGLE_BUFFER       0U
/*! Activate RxMAC double buffer system */
#define CSMD_RX_DOUBLE_BUFFER       1U
/*! Activate RxMAC triple buffer system */
#define CSMD_RX_TRIPLE_BUFFER       2U

/* Definitions for active Rx buffer                              */
#define CSMD_RX_BUFFER_0            0U
#define CSMD_RX_BUFFER_1            1U
#define CSMD_RX_BUFFER_2            2U



#ifdef CSMD_PCI_MASTER
/*-------------------------------------------------------------- */
/* definitions for PCI Master support */
/*-------------------------------------------------------------- */
#define CSMD_NO_DMA             (FALSE)
#define CSMD_DMA                (TRUE)

#define CSMD_TX_DMA               0U      /* Read  RxRam  */
#define CSMD_RX_DMA               1U      /* Write TxRam  */

#endif



/*-------------------------------------------------------------------------- */
/* CSMD_CONNECTION.usTelegramType                                            */
/*-------------------------------------------------------------------------- */
#define CSMD_TELEGRAM_TYPE_MDT              1U
#define CSMD_TELEGRAM_TYPE_AT               2U

/*-------------------------------------------------------------------------- */
/* S-0-1050, SE1 - Configuration of connection                               */
/*-------------------------------------------------------------------------- */
#define CSMD_S_0_1050_SE1_ACTIVE            0x8000U     /* Bit 15: Connection: active respectively existing */
#define CSMD_S_0_1050_SE1_PRODUCER          0x4000U     /* Bit 14: Connection type: producer */
#define CSMD_S_0_1050_SE1_ACTIVE_TYPE_MASK  0xC000U     /* Bit 15-14: Mask for active connection and connection type */
#define CSMD_S_0_1050_SE1_ACTIVE_CONSUMER   0x8000U     /* Bit 15-14: 1 0 - Active consumer connection */
#define CSMD_S_0_1050_SE1_ACTIVE_PRODUCER   0xC000U     /* Bit 15-14: 1 1 - Active producer connection */


#define CSMD_S_0_1050_SE1_TYPE              0x0030U     /* Bit 5-4: Configuration type mask */
#define CSMD_S_0_1050_SE1_IDN_LIST          0x0000U     /*   Configuration type: IDN list (SE6 relevant) */
#define CSMD_S_0_1050_SE1_CONTAINER         0x0010U     /*   Configuration type: Container (SE5 relevant) */
#define CSMD_S_0_1050_SE1_FSP_DRIVE         0x0020U     /*   Configuration type: Tel.type par. (S-0-0015 relevant) */
#define CSMD_S_0_1050_SE1_RESERVED_TYPE     0x0030U     /*   Configuration type: Reserved */

#define CSMD_S_0_1050_SE1_PROD_SYNC         0x0008U     /* Bit 3:   Reserved */

#define CSMD_S_0_1050_SE1_MONITOR           0x0003U     /* Bit 1-0: Type of connection                  */
#define CSMD_S_0_1050_SE1_SYNC              0x0000U     /*   clock synchronous                          */
#define CSMD_S_0_1050_SE1_ASYNC_WD          0x0001U     /*   non-cyclic type 1 (with watchdog)          */
#define CSMD_S_0_1050_SE1_ASYNC             0x0002U     /*   non-cyclic type 2 (watchdog not possible)  */
#define CSMD_S_0_1050_SE1_CYCLIC            0x0003U     /*   cyclic connection                          */

/*-------------------------------------------------------------------------- */
/* S-0-1050, SE3 - Telegram Assignment                                       */
/*-------------------------------------------------------------------------- */
#define CSMD_S_0_1050_SE3_TEL_NBR_MASK      0x3000U     /* (Bit 13...12 Telegram number */
#define CSMD_S_0_1050_SE3_TEL_NBR0          0x0000U     /* MDT0 / AT0 */
#define CSMD_S_0_1050_SE3_TEL_NBR1          0x1000U     /* MDT1 / AT1 */
#define CSMD_S_0_1050_SE3_TEL_NBR2          0x2000U     /* MDT2 / AT2 */
#define CSMD_S_0_1050_SE3_TEL_NBR3          0x3000U     /* MDT3 / AT3 */

#define CSMD_S_0_1050_SE3_TELTYPE_MDT       0x0800U     /* (Bit 11: 1=MDT / 0=AT) */
#define CSMD_S_0_1050_SE3_TEL_OFFSET        0x07FFU     /* Telegram offset mask   */

#define CSMD_S_0_1050_SE3_TEL_NBR_SHIFT     12U         /* Bits to be shifted for telegram number in S-0-1050.x.3 */
#define CSMD_S_0_1050_SE3_TEL_NBR(__telnbr) ( (CSMD_USHORT)((__telnbr) << CSMD_S_0_1050_SE3_TEL_NBR_SHIFT) )


/*---------------------------------------------------- */
/* Master: Number of allowed telegram errors          */
/*---------------------------------------------------- */
#define CSMD_NBR_ALLOWED_TEL_ERROR          1U




/*---- Declaration public Types: ---------------------------------------------*/

/* -------------------------------------------------------------------------- */
/*! \brief Pointer list to FPGA resources                                     */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_INIT_POINTER_STR
{
  /*! Pointer to Master-Control/Status-Register area           */
  CSMD_VOID    *pvSERCOS_Register;
  
  /*! Pointer to Service Channel Ram                           */
  CSMD_VOID    *pvSERCOS_SVC_Ram;
  
  /*! Pointer to Tx Ram for Sercos and IP transmit data        */
  CSMD_VOID    *pvSERCOS_TX_Ram;
  
  /*! Pointer to Rx Ram for Sercos and IP receive data         */
  CSMD_VOID    *pvSERCOS_RX_Ram;
#ifdef CSMD_PCI_MASTER
  /*! Pointer to PCI Bus Master DMA Control/Status Register    */
  CSMD_VOID    *pvSERCOS_DMA_Register;
#endif
  
} CSMD_INIT_POINTER;


/* -------------------------------------------------------------------------- */
/*! \brief Function state machine data                                        */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_FUNC_STATE_STR
{
  CSMD_USHORT      usActState;     /*!< Actual state in state machine */
  CSMD_ULONG       ulSleepTime;    /*!< [Milliseconds] Zero, if no Sleep needed. */
  
} CSMD_FUNC_STATE;


/* -------------------------------------------------------------------------- */
/*! \brief Service channel status word                                        */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_S3_SVCH_SW_STR
{
  CSMD_USHORT  usStatus;              /*!< SVCH status Word (2 Byte) */
  
} CSMD_S3_SVCH_SW;

/* -------------------------------------------------------------------------- */
/*! \brief enumeration slave activity status                                  */
/* -------------------------------------------------------------------------- */
typedef enum  CSMD_SLAVE_ACTIVITY_STATUS_EN
{
    CSMD_SLAVE_INACTIVE,          /*!< slave is currently not taking part in the Sercos communication       */
    CSMD_SLAVE_ACTIVE,            /*!< slave is currently taking part in the Sercos communication           */
    CSMD_SLAVE_HP_IN_PROCESS      /*!< hot plug procedure is in process for this slave                      */

} CSMD_SLAVE_ACTIVITY_STATUS;

/* -------------------------------------------------------------------------- */
/*! \brief Service channel management structure for atomic function call      */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_SVCH_MNGMT_STR
{
  CSMD_USHORT         usSlaveIdx;             /*!< Slave index of request   */
  CSMD_USHORT         usElem;                 /*!< Requested element   */
  CSMD_ULONG          ulIdent_Nbr;            /*!< Parameter IDN */
  CSMD_USHORT        *pusAct_Data;            /*!< Actual data pointer */
  CSMD_USHORT         usPriority;             /*!< '0' = Low priority call */
                                              /*!< '1' = High priority call */
  CSMD_USHORT         usLength;               /*!< Data length in bytes (if list, incl. Current/Max length) */
  CSMD_USHORT         usIsList;               /*!< Flag for list handling: */
                                              /*!< '0' = No list */
                                              /*!< '1' = Is list */
                                              /*!< '2' = Is list, read only length */
                                              /*!< '4' = Is list, read list-segment (only available for defined CSMD_SVC_LIST_SEGMENT) */
  CSMD_ULONG          ulAttribute;            /*!< Attribute   */
  CSMD_USHORT         usSvchError;            /*!< Service Channel error message */
  CSMD_USHORT         usCancelActTrans;       /*!< Break current transmission   */
  CSMD_USHORT         usActStateAtomic;       /*!< State of the atomic State Machine */
  CSMD_USHORT         usAct_Position;         /*!< Actual position of the data in the list  */
  CSMD_USHORT         usReplaceable;          /*!< '0' = not replaceable */
                                              /*!< '1' = Replaceable     */
  CSMD_S3_SVCH_SW     SVCH_Status;            /*!< SVCH status Word (2 Byte) */
  CSMD_USHORT         usAct_Len;              /*!< Current length of the list */
  CSMD_USHORT         usMax_Len;              /*!< Maximum length of the list */
#ifdef CSMD_SVC_LIST_SEGMENT
  CSMD_USHORT         usRequestedLength;      /*!< Length (incl. current/max length) of the requested list segment if usIsList = 4 */
#endif
  CSMD_USHORT         usSrv_Cont;             /*!< '0' = Hardware Service container */
  CSMD_USHORT         usSetEnd;               /*!< Set end Bit  */
  CSMD_USHORT         usNumWords;             /*!< number of remaining data (words) to be read or write */
  CSMD_USHORT         usInUse;                /*!< Service channel is in use for this slave   */
  CSMD_USHORT         usChannelOpen;          /*!< The service channel is already open for this IDN   */
  CSMD_USHORT         usRequestCanceled;      /*!< The SVCH request is canceled    */
  CSMD_USHORT         usLLR;                  /*!< Internal flag: list length has been read (1) */
  CSMD_USHORT         usIntReqPend;           /*!< next svc-transmission is reserved for an internal request */
  CSMD_USHORT         usEmptyList;            /*!< empty list -> list length == 0 */
  
} CSMD_SVCH_MNGMT_STRUCT;


/* -------------------------------------------------------------------------- */
/*! \brief Service channel Management Structure for Macro function call       */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_SVCH_MACRO_STR
{
  CSMD_USHORT         usSlaveIdx;             /*!< Slave index of request   */
  CSMD_USHORT         usElem;                 /*!< Requested element   */
  CSMD_ULONG          ulIdent_Nbr;            /*!< Parameter IDN */
  CSMD_USHORT        *pusAct_Data;            /*!< Actual data pointer */
  CSMD_USHORT         usPriority;             /*!< '0' = Low Priority Call */
                                              /*!< '1' = High Priority Call */
  CSMD_USHORT         usLength;               /*!< Data length in bytes (if list, incl. Current/Max length) */
  CSMD_USHORT         usIsList;               /*!< Flag for List handling; */
                                              /*!< '0' = No List */
                                              /*!< '1' = Is List */
                                              /*!< '2' = Is List, read only length */
                                              /*!< '3' = No list information, read attribute and extract information */
                                              /*!< '4' = Is list, read list-segment (only available for defined CSMD_SVC_LIST_SEGMENT) */
  CSMD_ULONG          ulAttribute;            /*!< Attribute   */
  CSMD_USHORT         usSvchError;            /*!< Service Channel Error Message */
  CSMD_USHORT         usState;                /*!< State of macro state machine for macro request function   */
  CSMD_USHORT         usCancelActTrans;       /*!< Break current transmission   */
  CSMD_USHORT         usOtherRequestCanceled; /*!< Request of lower priority call is canceled */
  CSMD_USHORT         usInternalReq;          /*!< reserve SVC for an internal request */
  
} CSMD_SVCH_MACRO_STRUCT;


/* -------------------------------------------------------------------------- */
/*! \brief Defines for the service channel management structures variables    */
/* -------------------------------------------------------------------------- */

/* usIsList */
#define CSMD_ELEMENT_NO_LIST            0U      /* no list */
#define CSMD_ELEMENT_IS_LIST            1U      /* is list */
#define CSMD_ELEMENT_RD_LIST_LENGTH     2U      /* Is list, read only length */
#define CSMD_ELEMENT_UNKNOWN_LENGTH     3U      /* No list information, read attribute and extract information
                                                   (only applicable for macro function calls)  */
#define CSMD_ELEMENT_RD_LIST_SEGMENT    4U      /* Is list, read list-segment (only available for defined CSMD_SVC_LIST_SEGMENT) */



/* -------------------------------------------------------------------------- */
/*! \brief Structure for Hardware initialization                              */
/* -------------------------------------------------------------------------- */
typedef struct  CSMD_HW_INIT_STR
{
  CSMD_ULONG   ulCycleTime_CP0;       /*!< Sercos communication cycle time in CP0 [ns]             */
  CSMD_ULONG   ulCycleTime_CP12;      /*!< Sercos communication cycle time in CP1 and CP2 [ns]     */
  CSMD_ULONG   ulUCC_Width;           /*!< UC channel time width [ns] for CP3/CP4  (for CP1/CP2,
                                           if eUCC_Mode_CP12 == CSMD_UCC_MODE_CP12_1_VAR)          */
  CSMD_USHORT  usIP_MTU_P34;          /*!< IP Maximum Transmission Unit in CP3 to CP4 [Byte]       */
#ifdef CSMD_HOTPLUG
  CSMD_BOOL    boHotPlug;             /*!< Hot-Plug support active by master                       */
#endif
  CSMD_USHORT  usTxBufferMode;        /*!< Mode of multiple Tx buffering system for real-time data */
  CSMD_USHORT  usRxBufferMode;        /*!< Mode of multiple Rx buffering system for real-time data */
#ifdef CSMD_PCI_MASTER
  CSMD_BOOL    boPciMode;             /*!< PCI bus master DMA enabled                              */
#endif
#ifdef CSMD_4MDT_4AT_IN_CP1_2
  CSMD_BOOL    boFourMDT_AT_CP12;     /*!< Number of Telegrams: 4 MDT and 4 AT in CP1 and CP2      */
#endif
  CSMD_BOOL    boHP_Field_All_Tel;    /*!< HotPlug field in all telegrams                          */
  CSMD_USHORT  usSVC_BusyTimeout;     /*!< SVC Busy Timeout [ms]                                   */
  CSMD_USHORT  usSVC_Valid_TOut_CP1;  /*!< Timeout for S-SVC.Valid at switch to CP1 [ms].          */
#ifdef CSMD_SWC_EXT
  CSMD_UCC_MODE_ENUM 
               eUCC_Mode_CP12;        /*!< Mode for UC channel in CP1/CP2                          */
  CSMD_BOOL    boFastCPSwitch;        /*!< Fast communication phase switch                         */
  CSMD_BOOL    boTelOffLastSlave;     /*!< Switch off Sercos III telegrams                         */
#endif
  
} CSMD_HW_INIT_STRUCT;


/* *************************************************************************/
/*! \struct CSMD_SLAVE_LIST

\brief Lists of recognized, projected and operable slaves / \n
       Slave lists for the binary configuration.

- ausRecogSlaveAddList[]
  Index | Element description | Source
  ----- | ------------------- | --------------------
  0     | current list length | determined by master
  1     | maximum list length | determined by master
  2...n | Sercos addresses of the recognized slaves \n detected in CP0 [topology address - 1 + 2] | determined by master
  \n

- ausProjSlaveAddList[]
  Index | Element description | Source
  ----- | ------------------- | ------------------------
  0     | current list length | specified by application
  1     | maximum list length | specified by application
  2...n | Sercos addresses of the projected slaves [slave index + 2] | specified by application
  \n

- ausProjSlaveIdxList[]
  Index | Element description | Source
  ----- | ------------------- | --------------------
  0     | don't care          | determined by master
  1...n | list containing slave indices of all projected slaves [Sercos address] | determined by master
  \n

- aeSlaveActive[]
  Index | Element description | Source
  ----- | ------------------- | --------------------
  0...n | activity status of slave (0=active; 1=inactive; 2=HP in process) [slave index] | determined by master
  \n

- ausDeactSlaveAddList[] (if defined CSMD_CONFIG_PARSER)
  Index | Element description | Source
  ----- | ------------------- | ------------------------
  0     | current list length | specified by application
  1     | maximum list length | specified by application
  2...n | Sercos addresses of slaves which do not take part in communication [slave index + 2] | specified by application
  \n

- ausParserTempAddList[] (if defined CSMD_CONFIG_PARSER)
  Index | Element description | Source
  ----- | ------------------- | --------------------
  0     | current list length | determined by master
  1     | maximum list length | determined by master
  2...n | Sercos addresses of configured slaves by the parser [slave index + 2] | determined by master
  \n

- ausParserTempIdxList[] (if defined CSMD_CONFIG_PARSER)
  Index | Element description | Source
  ----- | ------------------- | --------------------
  0     | don't care          | determined by master
  1...n | Slave indices of configured slaves by the parser [Sercos address] | determined by master
  \n
 ******************************************************************** */
typedef struct CSMD_SLAVE_LIST_STR
{
  CSMD_USHORT  usNumRecogSlaves;                                /*!< Number of recognized slaves */
  CSMD_USHORT  usNumProjSlaves;                                 /*!< Number of projected slaves */

  CSMD_USHORT  ausRecogSlaveAddList[CSMD_NBR_SCAN_SLAVES + 2];  /*!< Sercos list with Sercos addresses of recognized slaves [topology address - 1 + 2]
                                                                     this list is built in CP0 and is never updated! */
  CSMD_USHORT  ausProjSlaveAddList[CSMD_MAX_SLAVES + 2];        /*!< Sercos list with Sercos addresses of projected slaves [slave index + 2] */
  CSMD_USHORT  ausProjSlaveIdxList[CSMD_SLAVE_NBR_LIMIT];       /*!< list containing slave indices of all projected slaves [Sercos address] */
  
  CSMD_SLAVE_ACTIVITY_STATUS  aeSlaveActive[CSMD_MAX_SLAVES];   /*!< activity status of slave (0=active; 1=inactive; 2=HP in process) [slave index] */
  
#ifdef CSMD_CONFIG_PARSER
  CSMD_USHORT  ausDeactSlaveAddList[CSMD_MAX_SLAVES+2];         /*!< Sercos list with Sercos addresses of slaves which do not take part in communication [slave index + 2] */
  
  CSMD_USHORT  ausParserTempAddList[CSMD_MAX_SLAVES+2];         /*!< Sercos list with Sercos addresses of configured slaves by the parser [slave index + 2] */
  CSMD_USHORT  ausParserTempIdxList[CSMD_SLAVE_NBR_LIMIT];      /*!< list with slave indices of configured slaves by the parser [Sercos address] */
#endif
} CSMD_SLAVE_LIST;


/* ******************************************************************** */
/* Sercos IDN defines                                                   */
/* ******************************************************************** */

/* Sercos ident number: IDN.SI.SE
     IDN = parameter S/P - parameter set - data block number
     SI  = structure instance
     SE  = structure element)
   IDN coding:
     Bit 31-24: structure instance (__si)
     Bit 23-16: structure element  (__se)
     Bit 15:    parameter "S/P"    (0/1)
     Bit 14-12: parameter set      (__set)
     Bit 11-0:  data block number  (__id)
*/
#define CSMD_IDN_S_(__set,__id,__si,__se)   ((CSMD_ULONG)(((CSMD_ULONG)(__si) << 24) + ((CSMD_ULONG)(__se) << 16) ((CSMD_ULONG)(__set) << 12) + (__id)))
#define CSMD_IDN_P_(__set,__id,__si,__se)   ((CSMD_ULONG)(((CSMD_ULONG)(__si) << 24) + ((CSMD_ULONG)(__se) << 16) + (1UL << 15) + ((CSMD_ULONG)(__set) << 12) + (__id)))

#define CSMD_IDN_S_0_(__id,__si,__se)       ((CSMD_ULONG)(((CSMD_ULONG)(__si) << 24) + ((CSMD_ULONG)(__se) << 16) + (__id)))

#define CSMD_IDN_S_0_0000          0U       /* Dummy parameter */
#define CSMD_IDN_S_0_0015         15U       /* Telegram type */
#define CSMD_IDN_S_0_0127        127U       /* CP3 transition check procedure command */
#define CSMD_IDN_S_0_0128        128U       /* CP4 transition check procedure command */
#define CSMD_IDN_S_0_1000       1000U       /* List of SCP classes & version */
#define CSMD_IDN_S_0_1000_0_1   CSMD_IDN_S_0_( 1000, 0, 1 ) /* List of Active SCP Classes & Version */
#define CSMD_IDN_S_0_1002       1002U       /* Communication cycle time (tScyc) */
#define CSMD_IDN_S_0_1003       1003U       /* Allowed MST losses in CP3/CP4 */
#define CSMD_IDN_S_0_1005       1005U       /* Maximum Producer processing Time (t5) */
#define CSMD_IDN_S_0_1006       1006U       /* AT0 transmission starting time (t1) */
#define CSMD_IDN_S_0_1007       1007U       /* Synchronization time (tSync) */
#define CSMD_IDN_S_0_1008       1008U       /* Command value valid time (t3) */
#define CSMD_IDN_S_0_1009       1009U       /* Device Control (C-DEV) offset in MDT */
#define CSMD_IDN_S_0_1010       1010U       /* Lengths of MDTs */
#define CSMD_IDN_S_0_1011       1011U       /* Device Status (S-DEV) offset in AT */
#define CSMD_IDN_S_0_1012       1012U       /* Lengths of ATs */
#define CSMD_IDN_S_0_1013       1013U       /* SVC offset in MDT */
#define CSMD_IDN_S_0_1014       1014U       /* SVC offset in AT */
#define CSMD_IDN_S_0_1015       1015U       /* Ring delay */
#define CSMD_IDN_S_0_1017       1017U       /* UC transmission time */
#define CSMD_IDN_S_0_1023       1023U       /* SYNC jitter */
#define CSMD_IDN_S_0_1024       1024U       /* SYNC delay measuring procedure command */
#define CSMD_IDN_S_0_1032       1032U       /* Communication control */
#define CSMD_IDN_S_0_1036       1036U       /* Inter Frame Gap */
#define CSMD_IDN_S_0_1037       1037U       /* Slave Jitter */
#define CSMD_IDN_S_0_1047       1047U       /* Maximum Consumer Processing Time (t11) */
#define CSMD_IDN_S_0_1048       1048U       /* Activate network settings procedure command */
#define CSMD_IDN_S_0_1051       1051U       /* Image of connection setups */
#define CSMD_IDN_S_0_1061       1061U       /* Maximum TSref-Counter */

#define CSMD_IDN_MAX_NBR_SI     ((CSMD_USHORT)( 256 ))       /* Max. nbr of structure instances of an IDN (defined in Sercos Spec. */


/*-------------------------------------------------------------------------- */
/* S-0-1032 - Communication control (bit definitions)                        */
/*-------------------------------------------------------------------------- */
#define CSMD_S_0_1032_MQUAL_GRADE     0x00000010UL    /*!< Bit 4: Master quality grade
                                                           - 0  Quality grade 0 (highest quality) 
                                                                  MST jitter = +/- 1 microsecond maximum (default setting)
                                                           - 1  Quality grade 1 
                                                                  MST jitter     > 1 microsecond */
#define CSMD_S_0_1032_LB_NO_FORWARD   0x00000008UL    /*!< Bit 3: Loopback 
                                                           - 0  Loopback with forward 
                                                           - 1  Loopback without forward, if the slave is last in line
                                                                  Sercos telegrams switched off
                                                                  (support opperation with Industrial Ethernet devices) */
#define CSMD_S_0_1032_EF_MDT0         0x00000004UL    /*!< Bit 2: Extended field of MDT0 
                                                           - 0  Extended field is not configured
                                                           - 1  Extended field is configured */


/* ========================================================================== */

/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Sercos V1.3                                                               */
/*  Configuration Structures for Telegram and Timing                          */
/*                                                                            */
/* -------------------------------------------------------------------------- */

#define  CSMD_NBR_CONNECTIONS_FIX_CFG     2U

#define  CSMD_DEFAULT_SLAVE_VALID_MISS    2U    /* default value for allowed consecutive S-DEV.SlaveValid misses */

/* -------------------------------------------------------------------------- */
/*! \brief Common timing configuration for all slaves                         */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_COMMON_TIMING_STR
{
  CSMD_ULONG   ulCommCycleTime_S01002;                            /*!< SCP Basic:   S-0-1002  Communication Cycle time (tScyc) [ns] */
  CSMD_ULONG   ulATTxStartTimeT1_S01006;                          /*!< SCP Sync:    S-0-1006  AT0 transmission starting time (t1) [ns] */
  CSMD_ULONG   ulSynchronizationTime_S01007;                      /*!< SCP Sync:    S-0-1007  Synchronization time (tSync) [ns] */
  CSMD_ULONG   ulCmdValidTimeT3_S01008;                           /*!< SCP Sync:    S-0-1008  Command value valid time (t3) [ns] */
  CSMD_USHORT  usMDT_Length_S01010[CSMD_MAX_TEL];                 /*!< SCP Basic:   S-0-1010  Lengths of MDTs [bytes] */
  CSMD_USHORT  usAT_Length_S01012[CSMD_MAX_TEL];                  /*!< SCP Basic:   S-0-1012  Length of ATs [bytes] */
  CSMD_ULONG   ulRingDelay_S01015;                                /*!< SCP Sync:    S-0-1015  Ring delay [ns] */
  CSMD_ULONG   ulRingDelay_S01015_P2;                             /*!< SCP Sync:    S-0-1015  Ring delay [ns] (for slaves on port 2 in case of broken ring topology) */
  CSMD_ULONG   ulSyncJitter_S01023;                               /*!< SCP Sync:    S-0-1023  SYNC jitter [ns] (is calculated) */
  CSMD_USHORT  usInterFrameGap_S1036;                             /*!< SCP Sync V2: S-0-1036  Inter Frame Gap [bytes] */
  CSMD_ULONG   ulMinTimeStartAT;                                  /*!< Minimum Value for S-0-1006 [ns] */
  CSMD_ULONG   ulEndofAT;                /* Ersetzen durch ??? */ /*!< End of last received AT telegram [ns] */
  CSMD_USHORT  usMaxTSRefCount_S1061;                             /*!< SCP Sync V3: S-0-1061  Maximum TSref-Counter  */
  CSMD_USHORT  usAllowed_Slave_Valid_Miss;                        /*!< Number of allowed consecutively missed S-DEV.SlaveValid bits.
                                                                       If this number is exceeded, the activity status of the
                                                                       respective slave is set to inactive. This number is relevant for all slaves */
  
  CSMD_ULONG   ulCalc_RingDelay;                                  /*!< calculated value of ring delay [ns] */
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
  CSMD_ULONG   ulCalc_DelayMaster;                                /*!< Is no longer used:\n master port delay [ns] */
  CSMD_ULONG   ulCalc_DelaySlave;                                 /*!< slave port delay [ns] */
  CSMD_LONG    lCompDelay;                                        /*!< components delay [ns] */
#else
  CSMD_ULONG   ulCalc_DelayMaster;                                /*!< master port delay [ns] */
  CSMD_ULONG   ulCalc_DelaySlave;                                 /*!< slave port delay [ns] */
  CSMD_ULONG   ulCalc_DelayComp;                                  /*!< components delay [ns] */
#endif
} CSMD_COMMON_TIMING;


/* -------------------------------------------------------------------------- */
/*! \brief Configuration of the UC channel                                    */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_UC_CHANNEL_STR
{
  CSMD_ULONG   ulBegin_T6_S01017;                                 /*!< SCP Basic:   S-0-1017[0]  Begin of the UC channel (t6) [ns] */
  CSMD_ULONG   ulEnd_T7_S01017;                                   /*!< SCP Basic:   S-0-1017[1]  End of the UC channel (t7) [ns] */
  CSMD_ULONG   ulLatestTransmissionUC;                            /*!< Latest Transmission start time in UC channel [ns] */
  
} CSMD_UC_CHANNEL_STRUCT;


/* -------------------------------------------------------------------------- */
/*! \brief Configuration indexes of a connection                              */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_CONN_IDX_STR
{
  CSMD_USHORT  usConnIdx;                                         /*!< Connection index for structure CSMD_CONNECTION */
  CSMD_USHORT  usConfigIdx;                                       /*!< Configuration index for structure CSMD_CONFIGURATION */
  CSMD_USHORT  usRTBitsIdx;                                       /*!< Real-time Bit Configuration index for structure CSMD_REALTIME_BIT */
  
} CSMD_CONN_IDX_STRUCT;


/* -------------------------------------------------------------------------- */
/*! \brief Master configuration structure                                     */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_MASTER_CONFIGURATION_STR
{
  CSMD_ULONG            ulJitter_Master;                          /*!< MDT send jitter (Sync_jitter) [ns] */
  CSMD_USHORT           usMaxNbrTelErr;                           /*!< From master allowed telegram losses */
  
  CSMD_USHORT           usNbrOfConnections;                       /*!< Number of programmed connections (only used by configuration parser) */
#ifdef CSMD_STATIC_MEM_ALLOC
  CSMD_CONN_IDX_STRUCT  parConnIdxList
                          [CSMD_MAX_CONNECTIONS_MASTER];          /*!< List of Master connections */
#else
  /* Pointer to array with dynamically allocated number of elements */
  CSMD_CONN_IDX_STRUCT *parConnIdxList;                           /*!< List of Master connections */
#endif
#ifdef CSMD_CONFIG_PARSER
  CSMD_USHORT           usFirstConfigParamIndex;                  /*!< First configuration parameter index of the master */
#endif
} CSMD_MASTER_CONFIGURATION;


/* -------------------------------------------------------------------------- */
/*! \brief Slave specific telegram offsets                                    */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_TELEGRAM_CONFIGURATION_STR
{
  CSMD_USHORT  usC_DEV_OffsetMDT_S01009;                          /*!< SCP Basic:   S-0-1009  Device Control (C-DEV) offset in MDT */
  CSMD_USHORT  usS_DEV_OffsetAT_S01011;                           /*!< SCP Basic:   S-0-1011  Device Status (S-DEV) offset in AT */
  CSMD_USHORT  usSvcOffsetMDT_S01013;                             /*!< SCP Basic:   S-0-1013  SVC offset in MDT */
  CSMD_USHORT  usSvcOffsetAT_S01014;                              /*!< SCP Basic:   S-0-1014  SVC offset in AT  */
  
} CSMD_TELEGRAM_CONFIGURATION;


/* -------------------------------------------------------------------------- */
/*! \brief Slave specific telegram timing                                     */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_TIMING_STR
{
  CSMD_USHORT  usMaxNbrTelErr_S1003;                              /*!< SCP Basic:   S-0-1003  Allowed MST losses in CP3/CP4 */
  CSMD_USHORT  usSlaveJitter_S1037;                               /*!< SCP Basic02: S-0-1037: Slave jitter [ns] */
  CSMD_ULONG   ulMinFdbkProcTime_S01005;                          /*!< SCP Sync:    S-0-1005  Maximum Producer processing Time (t5)  */
  CSMD_ULONG   ulMaxConsActTimeT11_S01047;                        /*!< SCP Sync_02: S-0-1047  Maximum Consumer Processing Time (t11) */
} CSMD_TIMING;


/* -------------------------------------------------------------------------- */
/*! \brief Slave specific configuration structure                             */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_SLAVE_CONFIGURATION_STR
{
#ifdef CSMD_FAST_STARTUP
  CSMD_USHORT                 usSettings;                         /*!< Settings */
#endif
  CSMD_USHORT                 usNbrOfConnections;                 /*!< Number of programmed connections (only used by configuration parser) */
  CSMD_USHORT                 usMaxNbrOfConnections;              /*!< Number of max. possible connections of the slave */
  CSMD_USHORT                 ausSCPClasses
                                [CSMD_MAX_ENTRIES_S_0_1000 + 2];  /*!< SCP_VarCFG/SCP_FixCFG:  S-0-1000  List of SCP classes & version (Sercos IDN list) */
  CSMD_USHORT                 ausActiveSCPClasses
                                [CSMD_MAX_ENTRIES_S_0_1000 + 2];  /*!< S-0-1000.0.1  List of active SCP classes & version (Sercos IDN list) */
  CSMD_CONN_IDX_STRUCT        arConnIdxList
                                [CSMD_MAX_CONNECTIONS];           /*!< List of Slave connections */
#ifdef CSMD_CONFIG_PARSER
  CSMD_USHORT                 usFirstConfigParamIndex;            /*!< First configuration parameter index of this slave */
#endif
  CSMD_TELEGRAM_CONFIGURATION rTelegramConfig;                    /*!< Telegram configuration of the slave */
  CSMD_TIMING                 rTiming;                            /*!< Slave specific timing configuration */
  
} CSMD_SLAVE_CONFIGURATION;

/* -------------------------------------------------------------------------- */
/*! \brief Defines for Slave usSettings in CSMD_SLAVE_CONFIGURATION           */
/* -------------------------------------------------------------------------- */
#define CSMD_SLAVECONFIG_UNCHANGED    0x0001    /* Bit  0 „Configuration-Data Unchanged“:
                                                   1 = Slave-Configuration-Data has not been changed. Data will not be transferred */
#define CSMD_SLAVE_SETUP_UNCHANGED    0x0002    /* Bit  1 „Binary-Config-Setup-Data Unchanged“:
                                                   1 = Slave-Setup-Data from binary configuration has not been changed.
                                                       Data will not be transferred */

/* -------------------------------------------------------------------------- */
/*! \brief Connection configuration of a connection                           */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_CONNECTION_STR
{
  CSMD_USHORT  usTelegramType;                                     /*!< Telegram type: CSMD_TELEGRAM_TYPE_MDT / CSMD_TELEGRAM_TYPE_AT */
  CSMD_USHORT  usS_0_1050_SE2;                                     /*!< SCP VarCFG:  S-0-1050.x.02: Connection Number */
  CSMD_USHORT  usS_0_1050_SE3;                                     /*!< SCP Basic:   S-0-1050.x.03: Telegram Assignment */
  CSMD_USHORT  usS_0_1050_SE5;                                     /*!< SCP Basic:   S-0-1050.x.05: Current length of connection */
  CSMD_ULONG   ulS_0_1050_SE10;                                    /*!< SCP Sync:    S-0-1050.x.10: Producer Cycle Time */
  CSMD_USHORT  usS_0_1050_SE11;                                    /*!< SCP Sync:    S-0-1050.x.11: Allowed Data Losses */
  CSMD_USHORT  usApplicationID;                                    /*!< Application identifier */
  CSMD_UCHAR   ucConnectionName[CSMD_CONN_NAME_LENGTH];            /*!< Name of the connection */
  CSMD_VOID *  pvConnInfPtr;                                       /*!< Connection information pointer */
  
} CSMD_CONNECTION;


/* -------------------------------------------------------------------------- */
/*! \brief Configuration of a connection                                      */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_CONFIGURATION_STR
{
  CSMD_USHORT  usS_0_1050_SE7;                                     /*!< SCP Cap   :  S-0-1050.x.07: Assigned connection capability */
  CSMD_USHORT  usS_0_1050_SE1;                                     /*!< SCP VarCFG:  S-0-1050.x.01: Connection setup */
  CSMD_ULONG   ulS_0_1050_SE6[CSMD_MAX_IDN_PER_CONNECTION + 1];    /*!< SCP VarCFG:  S-0-1050.x.06: Configuration list (Sercos IDN list) */
  CSMD_USHORT  usTelgramTypeS00015;                     /* ??? */  /*!<        :     S-0-0015  Telegram type */
  
} CSMD_CONFIGURATION;


/* -------------------------------------------------------------------------- */
/*! \brief Real-time-bit configuration of a connection                        */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_REALTIME_BIT_STR
{
  CSMD_ULONG   ulS_0_1050_SE20[CSMD_MAX_RT_BITS_PER_CONN + 1];     /*!< SCP RTB   :  S-0-1050.x.20: IDN Allocation of real-time bit (Sercos IDN list) */
  CSMD_USHORT  usS_0_1050_SE21[CSMD_MAX_RT_BITS_PER_CONN + 2];     /*!< SCP RTB   :  S-0-1050.x.21 :Bit allocation of real-time bit (Sercos IDN list) */
  
} CSMD_REALTIME_BIT;


#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
/* -------------------------------------------------------------------------- */
/*! \brief Reference to parameter lists,
           Written into a slave during switch to CP3.                         */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_SLAVE_PARAMCONFIG_STR
{
  CSMD_USHORT  usConfigParamsList_Index;  /*!< Reference to configuration parameter list */
  CSMD_USHORT  usSlaveIndex;              /*!< Index of the target slave */
  CSMD_USHORT  usNextIndex;               /*!< Index of the next configuration for this slave */ 
  
} CSMD_SLAVE_PARAMCONFIG;


/* -------------------------------------------------------------------------- */
/*! \brief List of references to parameters,
           written into a slave during switch to CP3.                         */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_CONFIGPARAMS_LIST_STR
{
  CSMD_USHORT  usApplicationID;                                     /*!< ApplicationID indicating the source of the configuration */ 
  CSMD_USHORT  ausParamTableIndex[CSMD_MAX_PARAMS_IN_CONFIG_LIST];  /*!< Reference to configuration parameters */ 
  
} CSMD_CONFIGPARAMS_LIST;


/* --------------------------------------------------------------------------- */
/*! \brief Parameter data, which is written into a slave during switch to CP3. */
/* --------------------------------------------------------------------------- */
typedef struct CSMD_CONFIGURATION_PARAMETER_STR
{
  CSMD_ULONG   ulIDN;                              /*!< Parameter IDN */
  CSMD_USHORT  usDataLength;                       /*!< ApplicationID indicating the source of the configuration */ 
  CSMD_USHORT  usDummy;                            /*!< Make the data LONG aligned */ 
  CSMD_UCHAR   aucParamData[CSMD_NBR_PARAM_DATA];  /*!< Byte array containing the parameter data */
  
} CSMD_CONFIGURATION_PARAMETER;
#endif /* #if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS */

#ifdef CSMD_CONFIG_PARSER
/* --------------------------------------------------------------------------- */
/*! \brief    TRUE/FALSE todo description                                                 */
/* --------------------------------------------------------------------------- */
typedef struct CSMD_SLAVE_INST_MANIPULATED_STR
{
  CSMD_UCHAR     aucManipulated[CSMD_MAX_CONNECTIONS];

} CSMD_SLAVE_INST_MANIPULATED;


/* --------------------------------------------------------------------------- */
/*! \brief    todo description                                                 */
/* --------------------------------------------------------------------------- */
typedef struct CSMD_USED_MARKER_STR
{
#ifdef CSMD_STATIC_MEM_ALLOC
  CSMD_UCHAR   paucConnNbrUsed[CSMD_MAX_GLOB_CONN];
  CSMD_UCHAR   paucConnUsed[CSMD_MAX_GLOB_CONN];
  CSMD_UCHAR   paucConfUsed[CSMD_MAX_GLOB_CONFIG];
  CSMD_UCHAR   paucRTBtUsed[CSMD_MAX_RT_BIT_CONFIG];
  CSMD_UCHAR   paucSetupParamsListUsed[CSMD_MAX_CONFIGPARAMS_LIST];
  CSMD_UCHAR   paucSetupParamsUsed[CSMD_MAX_CONFIG_PARAMETER];
#else
  /* Pointer to arrays with dynamically allocated number of elements */
  CSMD_UCHAR  *paucConnNbrUsed;
  CSMD_UCHAR  *paucConnUsed;
  CSMD_UCHAR  *paucConfUsed;
  CSMD_UCHAR  *paucRTBtUsed;
  CSMD_UCHAR  *paucSetupParamsListUsed;
  CSMD_UCHAR  *paucSetupParamsUsed;
#endif
} CSMD_USED_MARKER;
#endif  /* #ifdef CSMD_CONFIG_PARSER */


/* -------------------------------------------------------------------------- */
/*! \brief Common configuration structure (master and all slaves)             */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_CONFIG_STR
{
  CSMD_COMMON_TIMING            rComTiming;                             /*!< Common timing parameters */
  CSMD_UC_CHANNEL_STRUCT        rUC_Channel;                            /*!< UC channel parameters */
  CSMD_MASTER_CONFIGURATION     rMasterCfg;                             /*!< Master configuration */
#ifdef CSMD_STATIC_MEM_ALLOC
  CSMD_SLAVE_CONFIGURATION      parSlaveConfig[CSMD_MAX_SLAVES];        /*!< Slave configurations (first slave = index 0) */
  CSMD_CONNECTION               parConnection[CSMD_MAX_GLOB_CONN];      /*!< Connections list                             */
  CSMD_CONFIGURATION            parConfiguration[CSMD_MAX_GLOB_CONFIG]; /*!< Configurations list                          */
  CSMD_REALTIME_BIT             parRealTimeBit[CSMD_MAX_RT_BIT_CONFIG]; /*!< Real-time Bit configurations list            */
#else
  /* Pointer to arrays with dynamically allocated number of elements */
  CSMD_SLAVE_CONFIGURATION     *parSlaveConfig;                         /*!< Slave configurations (first slave = index 0) */
  CSMD_CONNECTION              *parConnection;                          /*!< Connections list                             */
  CSMD_CONFIGURATION           *parConfiguration;                       /*!< Configurations list                          */
  CSMD_REALTIME_BIT            *parRealTimeBit;                         /*!< Real-time Bit configurations list            */
#endif
#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
  #ifdef CSMD_STATIC_MEM_ALLOC
  CSMD_SLAVE_PARAMCONFIG        parSlaveParamConfig[CSMD_MAX_SLAVE_CONFIGPARAMS]; /*!< References to parameters list      */
  CSMD_CONFIGPARAMS_LIST        parConfigParamsList[CSMD_MAX_CONFIGPARAMS_LIST];  /*!< List with references to parameters */
  CSMD_CONFIGURATION_PARAMETER  parConfigParam[CSMD_MAX_CONFIG_PARAMETER];        /*!< Parameter IDN + data */
  #else
  /* Pointer to arrays with dynamically allocated number of elements */
  CSMD_SLAVE_PARAMCONFIG       *parSlaveParamConfig;                    /*!< References to parameters list      */
  CSMD_CONFIGPARAMS_LIST       *parConfigParamsList;                    /*!< List with references to parameters */
  CSMD_CONFIGURATION_PARAMETER *parConfigParam;                         /*!< Parameter IDN + data */
  #endif
#endif
#ifdef CSMD_CONFIG_PARSER
  CSMD_MASTER_CONFIGURATION    *prMaster_Config;    /*!< Pointer to master config for configuration without projected slaves */
  CSMD_SLAVE_CONFIGURATION     *parSlave_Config;    /*!< Pointer to slave config for configuration without projected slaves  */
#endif
#ifdef CSMD_DEBUG
  CSMD_ULONG                    ulSizeOfStructure;
#endif
  
} CSMD_CONFIG_STRUCT;

/* ========================================================================== */



/* -------------------------------------------------------------------------- */
/*! \brief Information about a fault in the connection configuration          */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_CONFIG_ERROR_STR
{
  /*! 1.participant: slave index / CSMD_MAX_SLAVES, if fault in master configuration */
  CSMD_USHORT  usSlaveIndex;
  
  /*! 1.participant: index in CSMD_CONN_IDX_STRUCT list */
  CSMD_USHORT  usConnectionIdx;
  
  /*! 2.participant: slave index / CSMD_MAX_SLAVES, if fault in master configuration */
  CSMD_USHORT  usSlaveIndex2;
  
  /*! 2.participant: index in CSMD_CONN_IDX_STRUCT list */
  CSMD_USHORT  usConnectionIdx2;
  
} CSMD_CONFIG_ERROR;


/* -------------------------------------------------------------------------- */
/*! \brief Call-back functions for memory allocation                          */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_MEM_ALLOC_CB_STR
{
  CSMD_VOID  *(*fpCSMD_set_mem_ptr) ( CSMD_INT nByteSize );
/*CSMD_VOID  *(*fpCSMD_malloc)      ( CSMD_INT nByteSize );*/
/*CSMD_ULONG  (*fpCSMD_free)        ( CSMD_VOID * pvMemAddress );*/

} CSMD_MEM_ALLOC_CB_STRUCT;

/* -------------------------------------------------------------------------- */
/*! \brief By the system set limits for maximum  number of slaves,
           connections, configurations etc.\n  These are limited by the
           maximum values specified by define in CSMD_USER.h.                 */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_SYSTEM_LIMITS_STR
{
  CSMD_USHORT  usMaxSlaves;            /*!< Maximum number of operable slaves
                                            (Limited by the define \ref CSMD_MAX_SLAVES) */
  CSMD_USHORT  usMaxConnMaster;        /*!< Maximum number of master produced and consumed connections
                                            (Limited by the define \ref CSMD_MAX_CONNECTIONS_MASTER) */
  CSMD_USHORT  usMaxGlobConn;          /*!< Maximum number of connections for master and slaves
                                            (Limited by the define \ref CSMD_MAX_GLOB_CONN) */
  CSMD_USHORT  usMaxGlobConfig;        /*!< Maximum number of  configurations for master and slaves
                                            (Limited by the define \ref CSMD_MAX_GLOB_CONFIG) */
  CSMD_USHORT  usMaxRtBitConfig;       /*!< Maximum number of configurable real-time-bit configurations
                                            (Limited by the define \ref CSMD_MAX_RT_BIT_CONFIG) */
#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
  CSMD_USHORT  usMaxSlaveConfigParams; /*!< Maximum number of references to parameter lists
                                            (Limited by the define \ref CSMD_MAX_SLAVE_CONFIGPARAMS) */
  CSMD_USHORT  usMaxConfigParamsList;  /*!< Maximum number of configuration parameter lists
                                            (Limited by the define \ref CSMD_MAX_CONFIGPARAMS_LIST) */
  CSMD_USHORT  usMaxConfigParameter;   /*!< Maximum number of configuration parameters CP2 --&gt; CP3
                                            (Limited by the define \ref CSMD_MAX_CONFIG_PARAMETER) */
#endif
  CSMD_USHORT  usMaxProdConnCC;        /*!< Maximum number of configurable cross-communication (CC) connections per slave
                                            (Limited by the define \ref CSMD_MAX_PROD_CONN_CC) */
} CSMD_SYSTEM_LIMITS_STRUCT;

/*! \endcond */ /* PUBLIC */


/*! \cond PRIVATE */

typedef struct CSMD_MEM_ALLOCATION_STR
{
  CSMD_ULONG  *pulBase;                /*|< By the system provided base pointer to allocated memory for CoSeMNa arrays/structures */
  CSMD_ULONG   ulAllocatedMemory;      /*!< By the system allocated memory for CoSeMNa arrays/structures */
  CSMD_ULONG   ulCSMD_Instance_Size;   /*!< Size of static allocated structure CSMD_INSTANCE */
  CSMD_MEM_ALLOC_CB_STRUCT
               rCB_FuncTable;          /*!< Table with function pointers of the memory allocation functions */

} CSMD_MEM_ALLOCATION_STRUCT;

/*! \endcond */ /* PRIVATE */



/*! \cond PUBLIC */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Sercos V1.3                                                               */
/*  Control and Status Words of communication and connection layer            */
/*                                                                            */
/*                                  |---> Application layer                   */
/*                                  |                                         */
/*              16Bit       16Bit       16Bit                                 */
/*          --------------------------------------------------------- - - -   */
/*          |           |           |           |                             */
/*  MS-MDT  |   C-DEV   |   C-CON   |  C-RES    |                             */
/*          |           |           |           |                             */
/*          --------------------------------------------------------- - - -   */
/*                                                                            */
/*          ----------------------------------------------------- - - - - -   */
/*          |           |           |           |                             */
/*  MS-AT   |   S-DEV   |   C-CON   |  C-RES    |                             */
/*          |           |           |           |                             */
/*          ----------------------------------------------------- - - - - -   */
/*                                                                            */
/*                      ------------------------------------------------- -   */
/*                      |           |           |                             */
/*  CC                  |   C-CON   | C/S-RES   |                             */
/*                      |           |           |                             */
/*                      ------------------------------------------------- -   */
/*                                                                            */
/*          |           |                                                     */
/*          |___________|                                                     */
/*           constant part of communication layer                             */
/*                                                                            */
/*                      |           |                                         */
/*                      |___________|                                         */
/*                       constant part of connection layer                    */
/*                                                                            */
/*                                  |                                       | */
/*                                  |_______________________________________| */
/*                                   configurable part of connection layer    */
/*                                                                            */
/* -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------- */
/* Device Control (C-DEV):   definitions                                      */
/* -------------------------------------------------------------------------- */
#define CSMD_C_DEV_LENGTH               2U        /* Length C-DEV [byte] */
#define CSMD_C_DEV_LENGTH_CP1_2         4U        /* Length C-DEV [byte] in CP1/CP2 (2 Byte + 2 Byte reserved) */
/* -------------------------------------------------------------------------- */
/* Device Control:   Bit definitions                                          */
/* -------------------------------------------------------------------------- */
#define CSMD_C_DEV_IDENTIFICATION       0x8000U   /* Bit 15   : Identification LED              */
#define CSMD_C_DEV_TOPOLOGY_HS          0x4000U   /* Bit 14   : Topology change command         */
#define CSMD_C_DEV_CMD_TOPOLOGY_MASK    0x3000U   /* Bit 13-12: Commanded Topology              */
#define CSMD_C_DEV_CMD_TOPOLOGY_FF_BOTH 0x0000U   /*  00 - fast forward on both ports           */
#define CSMD_C_DEV_CMD_LOOPB_FW_P_TEL   0x1000U   /*  01 - loopback with forward of P-Telegrams */
#define CSMD_C_DEV_CMD_LOOPB_FW_S_TEL   0x2000U   /*  10 - loopback with forward of S-Telegrams */
#define CSMD_C_DEV_PHYSICAL_STS         0x0800U   /* Bit 11   : Physical ring is closed         */
#define CSMD_C_DEV_MASTER_VALID         0x0100U   /* Bit  8   : Master valid                    */


/* -------------------------------------------------------------------------- */
/* Device Status (S-DEV):   definitions                                       */
/* -------------------------------------------------------------------------- */
#define CSMD_S_DEV_LENGTH               2U        /* Length of Device Status (S-DEV) in CP1/CP2 [byte] */
#define CSMD_S_DEV_LENGTH_CP1_2         4U        /* Length of Device Status in CP1/CP2 [byte] (2 Bytes + 2 Bytes reserved) */
/* -------------------------------------------------------------------------- */
/* Device Status:   Bit definitions                                           */
/* -------------------------------------------------------------------------- */
#define CSMD_S_DEV_TOPOLOGY_HS          0x4000U   /* Bit 14   : Topology change command acknowledge  */

#define CSMD_S_DEV_TOPOLOGY_STS_MASK    0x3000U   /* Bit 13-12: Current topology                */
#define CSMD_S_DEV_CURRENT_FF_BOTH      0x0000U   /*  00 - fast forward on both ports           */
#define CSMD_S_DEV_CURRENT_LB_FW_P_TEL  0x1000U   /*  01 - loopback with forward of P-Telegrams */
#define CSMD_S_DEV_CURRENT_LB_FW_S_TEL  0x2000U   /*  10 - loopback with forward of S-Telegrams */
                                                  /*  11 - NRT mode (store and forward)         */

#define CSMD_S_DEV_STAT_INACT_MASK      0x0C00U   /* Bit 11-10: Status on inactive port      */
#define CSMD_S_DEV_STAT_INACT_NO_LINK   0x0000U   /*  00 - No link on inactive.port          */
#define CSMD_S_DEV_STAT_INACT_LINK      0x0400U   /*  01 - Link on inactive.port             */
#define CSMD_S_DEV_STAT_INACT_P_TEL     0x0800U   /*  10 - P-telegram on inactive.port       */
#define CSMD_S_DEV_STAT_INACT_S_TEL     0x0C00U   /*  11 - S-telegram on inactive.port       */
#define CSMD_S_DEV_SLAVE_VALID          0x0100U   /* Bit 8: Bus slave valid                  */


/* Definitions for C-DEV and S-DEV */
#define CSMD_MASK_TOPO_TOGGLE           0x7000U /* Mask for toggle bit and topology bits 14-12  */


/* -------------------------------------------------------------------------- */
/*! \brief public structure for device status (S-DEV) information             */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_S_DEV_STR
{
  CSMD_USHORT    usS_Dev;    /*!< device status */
  CSMD_USHORT    usMiss;     /*!< consecutive slave valid misses (S-DEV is valid only if usMiss == 0) */

} CSMD_S_DEV_STRUCT;


/* -------------------------------------------------------------------------- */
/*! \brief Connection Control  C-CON   (16 Bit)                               */
/* -------------------------------------------------------------------------- */

#define CSMD_C_CON_LENGTH               2U                  /* Length of Connection Control (C-CON) [byte] */
/* -------------------------------------------------------------------------- */
/* Connection Control (C-CON):   Bit definitions                              */
/* -------------------------------------------------------------------------- */
#define CSMD_C_CON_PRODUCER_READY       0x0001U   /* Bit 0: Producer Ready          */
#define CSMD_C_CON_NEW_DATA             0x0002U   /* Bit 1: New Data (toggle bit)   */
#define CSMD_C_CON_DATA_FIELD_DELAY     0x0004U   /* Bit 2: Data Field Delay        */
#define CSMD_C_CON_FLOW_CONTROL         0x0010U   /* Bit 4: Flow Control            */
#define CSMD_C_CON_RTB_1                0x0040U   /* Bit 6: Real Time Bit 1         */
#define CSMD_C_CON_RTB_2                0x0080U   /* Bit 7: Real Time Bit 2         */
#define CSMD_C_CON_RTB_MASK \
            (CSMD_C_CON_RTB_1 | CSMD_C_CON_RTB_2) /* Mask for Real-time bits        */
#define CSMD_C_CON_RTB_SHIFT            6U                  /* Bits to be shifted for Real-time bits */

#define CSMD_C_CON_COUNTER_MASK         0xF000U   /* Bit 15-12: C-CON- Counter      */

#define CSMD_C_CON_COUNTER_SHIFT        12U

/* -------------------------------------------------------------------------- */
/*! \brief Service channel container control word structure                   */
/* -------------------------------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_SC_CONTROL_UN  CSMD_SERC_SC_CONTROL;

#define CSMD_SVC_FIELDWIDTH             6U        /* fixed SVC field: 2 byte control/status, 4 byte Info */
#define CSMD_SVC_INFO_WIDTH             4U        /* SVC Info field with 4 Byte length */
#define CSMD_HOTPLUG_FIELDWIDTH         8U        /* Size of the Hot-Plug field in MDT0 and AT0 [Byte] */
#define CSMD_EXT_FUNCT_FIELDWIDTH       4U        /* Length of extended function field in bytes
                                                     Extented functions field in MDT0 contains:
                                                     - C-Time: Least Common Multiple (LCM) of all
                                                       cycle times which shall be synchronized.
                                                    - Time: Sercos Time.                          */


/*---- Definition resp. Declaration private Constants and Macros: ------------*/


/*-------------------------------------------------------------------------- */
/* Sercos Definitions for length of                                          */
/*            telegrams, data and header                                     */
/*-------------------------------------------------------------------------- */
/* Sercos min. data length */
#define  CSMD_SERC3_MIN_DATA_LENGTH         40U

/* Sercos max. data length */
#define  CSMD_SERC3_MAX_DATA_LENGTH         1494U     /* 1500 - 6 */

/* MDT Sercos data length in P0 */
#define  CSMD_SERC3_MDT_DATA_LENGTH_P0      40U

/* AT  Sercos data length in P0 */
#define  CSMD_SERC3_AT_DATA_LENGTH_P0_V10   1024U     /* 512 * 2 used for ComVers 1.0 */

/* Sercos Length of SVC in P12 */
#define  CSMD_SERC3_SVC_DATA_LENGTH_P12     768U      /* 128 * (2+4) */

/* Sercos Length of RTD in P12 */
#define  CSMD_SERC3_RTD_DATA_LENGTH_P12     512U      /* 128 * 4 */

/* Sercos telegram length in P12 */
#define  CSMD_SERC3_DATA_LENGTH_P12         1280U     /* 128 * (2+4 + 4) */



/* With or without Sercos time (4 Byte): 247 SVC per telegram are possible */
/* Future in Spec 1.1.3: If no HP must be inserted in telegram 1 and higher, 249 SVC will be possible */
#define CSMD_MAX_NBR_SVC_PER_TEL            ((CSMD_SERC3_MAX_DATA_LENGTH - CSMD_HOTPLUG_FIELDWIDTH) / CSMD_SVC_FIELDWIDTH)


/* Sercos Tx Telegram InterFrameGap default [Bytes] */
#define  CSMD_IFG_DEFAULT                   125U      /* InterFrameGap in CP0 to CP2 */
#define  CSMD_IFG_DEFAULT_CP3_CP4           37U

/* Ethernet II MTU (maximum transmission unit) */
#define CSMD_ETHERNET_MTU_MIN               46U

/* Ethernet II MTU (maximum transmission unit) */
#define CSMD_ETHERNET_MTU_MAX               1500U

/*---------------------------------------------------- */
/*                                                     */
/*---------------------------------------------------- */
#define CSMD_IP_RAM_SEG_SIZE                256U      /* bytes */
#ifdef CSMD_SWC_EXT
#define CSMD_IP_TX_RAM_SIZE_CP0_CP2         ((((    CSMD_ETHERNET_MTU_MAX - 1) / CSMD_IP_RAM_SEG_SIZE) + 1) * CSMD_IP_RAM_SEG_SIZE)
#define CSMD_IP_RX_RAM_SIZE_CP0_CP2         ((((2 * CSMD_ETHERNET_MTU_MAX - 1) / CSMD_IP_RAM_SEG_SIZE) + 1) * CSMD_IP_RAM_SEG_SIZE)
#endif


/*! Maximum number of TSref counter (needed for length of ausTSrefList in private CoSeMa Structure */
#define CSMD_MAX_TSREF                      (50 * CSMD_MAX_CYC_TIMES - 49)

/*! Max. number of configurable events for CSMD_EventControl() */
#define CSMD_NBR_EVENT_ID                   8U




/*---- Declaration private Types: --------------------------------------------*/

/* -------------------------------------------------------------------------- */
/*! \brief Structure of external telegram buffers                             */
/* -------------------------------------------------------------------------- */
typedef union CSMD_TEL_BUFFER_UN
{
  struct
  {
    CSMD_HAL_TX_RAM             rBuffTxRam;   /*!< Local buffer for TxRam data */
    CSMD_HAL_RX_RAM             rBuffRxRam;   /*!< Local buffer for RxRam data */
#ifdef CSMD_PCI_MASTER
    CSMD_HAL_DMA_TX_RDY_FLAGS   rDMA_TxRdy;   /*!< DMA Tx Ready Flags */
    CSMD_HAL_DMA_RX_RDY_FLAGS   rDMA_RxRdy;   /*!< DMA Rx Ready Flags */
#endif
    
  } rLocal;
  
#ifdef CSMD_DEBUG
  struct
  {
    CSMD_HAL_SERCFPGA_WDATTYP usTx_Ram[ CSMD_HAL_TX_RAM_SIZE / sizeof (CSMD_HAL_SERCFPGA_WDATTYP) ];
    CSMD_HAL_SERCFPGA_WDATTYP usRx_Ram[ CSMD_HAL_RX_RAM_SIZE / sizeof (CSMD_HAL_SERCFPGA_WDATTYP) ];
    
  } rShort;
  
#endif
  
} CSMD_TEL_BUFFER;



/* -------------------------------------------------------------------------- */
/*! \brief SVC management structure (SOURCE)                                  */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_SVC_INTERNAL_STR
{
  CSMD_USHORT  usSVCRxRamPntrP1;           /*!< Pointer to SVC in Rx Ram Port 1              */
  CSMD_USHORT  usSVCRxRamPntrP2;           /*!< Pointer to SVC in Rx Ram Port 2              */
  
} CSMD_SVC_INTERNAL_STRUCT;


/* -------------------------------------------------------------------------- */
/*! \brief Communication Version Field   (for MDT0 in CP0)                    */
/* -------------------------------------------------------------------------- */
#ifdef CSMD_PACK_PRAGMA
#pragma pack(push,1)
#endif

union CSMD_COM_VER_FIELD_UN
{
  CSMD_ULONG    ulLong;                 /*!< Communication version field                                  */
  
#ifdef CSMD_DEBUG
  /* Note: maybe this structure must be declared via define in                                            */
  /*       reverse order for other compilers respectively machines !!!                                    */
  struct
  {
    CSMD_ULONG  bfReserved_9   : 9;     /* Bit 31-23: Currently without function                          */
    CSMD_ULONG  SWITCH_OFF_TEL : 1;     /* Bit    22: Switch off Sercos III telegrams                     */
                                        /*              0 = Last slave in line with forwarding Sercos tel */
                                        /*              1 = Last in line without forwarding Sercos tel.   */
    CSMD_ULONG  FAST_CP_SWITCH : 1;     /* Bit    21: Fast Communication Phase switch                     */
                                        /*              0 = transmission of MST interrupted during switch */
                                        /*              1 = Reduced interruption time                     */
    CSMD_ULONG  COMM_PAR_CP0   : 1;     /* Bit    20: Transmission of comm. parameters in MDT0 0f CP0     */
                                        /*              0 = no transmission of parameters                 */
                                        /*              1 = transmission of t1, t6 and t7                 */
    CSMD_ULONG  bfReserved_2   : 2;     /* Bit 19-18: Currently without function                          */
    CSMD_ULONG  NBR_TEL_CP12   : 2;     /* Bit 17-16: Structure and Number of MDTs and ATs in CP1 and CP2 */
                                        /*              00 = 2 MDTs and 2 ATs  (up to 255 slaves)         */
                                        /*              01 = 4 MDTs and 4 ATs  (up to 511 slaves)         */
                                        /*              10 = undefined combination                        */
                                        /*              11 = undefined combination                        */
    CSMD_ULONG  bfReserved_8   : 8;     /* Bit 15- 8: Currently without function                          */
    CSMD_ULONG  bfReserved_7   : 7;     /* Bit  7- 1: Currently without function                          */
    CSMD_ULONG  ADD_ALLOC      : 1;     /* Bit     0: Address allocation                                  */
                                        /*              0 = used for Sercos version 1.0 only              */
                                        /*              1 = address allocation (Sercos version >= 1.1.1   */
  
  } rBit;
#endif  /* End: #ifdef CSMD_DEBUG */
  
} /*lint !e659  (Nothing follows '}' on line terminating struct/union/enum definition) */
#ifdef CSMD_PACK_ATTRIBUTE
__attribute__((packed))
#endif
;
#ifdef CSMD_PACK_PRAGMA
#pragma pack(pop)
#endif
typedef union CSMD_COM_VER_FIELD_UN  CSMD_COM_VER_FIELD;


/* -------------------------------------------------------------------------- */
/*! \brief Address allocation mode for CSMD_SetPhase0()                       */
/* -------------------------------------------------------------------------- */
typedef enum CSMD_COM_VERSION_EN
{
/*CSMD_COMVERSION_V0_0,= 0*/        /*   without address allocation )used for Sercos version 1.0 only) */
  CSMD_COMVERSION_V1_0 = 1          /*!< address allocation (Sercos 1.1.1 and greater) */
  
} CSMD_COM_VERSION;


#define CSMD_NBR_OF_RD_MEASUREMENTS       64

#define CSMD_NBR_STABLE_TOPOLOGY         100                          /* The topology shall be stable for 100 Sercos cycles */
#define CSMD_TIMEOUT_STABLE_TOPOLOGY    1000                          /* Timeout for detection of stable topology = 1000 * tScyc */

#define CSMD_NBR_STABLE_SLAVE_LIST      CSMD_NBR_STABLE_TOPOLOGY      /* The slave address list shall be stable for 100 Sercos cycles */
#define CSMD_TIMEOUT_STABLE_SLAVE_LIST  CSMD_TIMEOUT_STABLE_TOPOLOGY  /* Timeout for stable slave address list = 1000 * tScyc */

#if (CSMD_NBR_OF_RD_MEASUREMENTS > CSMD_NBR_STABLE_TOPOLOGY)
#error csmd_glob.h: Number of measurements shall be lower than stable topology counter !
#endif
#if (CSMD_NBR_STABLE_SLAVE_LIST > CSMD_TIMEOUT_STABLE_SLAVE_LIST) || (CSMD_NBR_STABLE_TOPOLOGY > CSMD_TIMEOUT_STABLE_TOPOLOGY)
#error csmd_glob.h: Number of stable topology cycles shall be lower than stable topology counter timeout !
#endif

/* -------------------------------------------------------------------------- */
/*! \brief Structure for Ring Delay measurement                               */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_RING_DELAY_STR
{
  CSMD_ULONG       ulAverageRD_P1;     /*!< Average value of tNetwork port 1   */
  CSMD_ULONG       ulAverageRD_P2;     /*!< Average value of tNetwork port 2   */
  CSMD_USHORT      usCount1;           /*!< TMR measurement counter for port 1 */
  CSMD_USHORT      usCount2;           /*!< TMR measurement counter for port 2 */
  CSMD_ULONG       ulSumRD1;           /*!< Sum of tNetwork port 1 over all nbr of measurements */
  CSMD_ULONG       ulSumRD2;           /*!< Sum of tNetwork port 2 over all nbr of measurements */
  CSMD_ULONG       ulMinDelay1;        /*!< Minimum value of tNetwork port 1   */
  CSMD_ULONG       ulMaxDelay1;        /*!< Maximum value of tNetwork port 1   */
  CSMD_ULONG       ulMinDelay2;        /*!< Minimum value of tNetwork port 2   */
  CSMD_ULONG       ulMaxDelay2;        /*!< Maximum value of tNetwork port 2   */
  
#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
  struct CSMD_VAILD_RD_MEAS_STRUCT {
    CSMD_USHORT    usTopology;         /*!< Topology during tNetwork measuring */
    CSMD_ULONG     ulMaxTNetwork;      /*!< Sum of tNetwork_max form both ports*/
    CSMD_ULONG     ulTNetwork_P1;      /*!< Average value of tNetwork port 1   */
    CSMD_ULONG     ulTNetwork_P2;      /*!< Average value of tNetwork port 2   */
  } rValid;
#else
  CSMD_ULONG       ulSlaveJitter;      /*!< Sum of slave jitter                */
  CSMD_ULONG       ulExtraDelay;       /*!< Minimum extra delay                */
#endif
  CSMD_ULONG       ulTSref;            /*!< Synchronization reference time     */
#ifdef CSMD_DEBUG
  CSMD_ULONG       ulBuff1[CSMD_NBR_OF_RD_MEASUREMENTS];   /*!< Buffer for tNetwork port 1 values */
  CSMD_ULONG       ulBuff2[CSMD_NBR_OF_RD_MEASUREMENTS];   /*!< Buffer for tNetwork port 2 values */
#endif
  
} CSMD_RING_DELAY;


/* -------------------------------------------------------------------------- */
/*! \brief Parameter for the ring delay calculation function.                 */
/* -------------------------------------------------------------------------- */
typedef enum CSMD_RD_CALC_MODE_EN
{
  CSMD_CALC_RD_MODE_NORMAL,               /*!< Ring delay calculation during a communication phase switch  */
  CSMD_CALC_RD_MODE_RECOVER_RING_ACTIVE,  /*!< Ring delay calculation during a ring recovery */
  CSMD_CALC_RD_MODE_HOTPLUG_ACTIVE        /*!< Ring delay calculation during hot-plug        */

} CSMD_RD_CALC_MODE;


#define CSMD_RD_WR_BUFFER_NBR_CSMD_ULONGS     4U
#define CSMD_RD_WR_BUFFER_NBR_CSMD_USHORTS    8U

/* -------------------------------------------------------------------------- */
/*! \brief Structure for temp. data buffer for svc macro function calls       */
/* -------------------------------------------------------------------------- */
typedef union CSMD_RD_WR_BUFFER_UN
{
  CSMD_ULONG   aulData[CSMD_RD_WR_BUFFER_NBR_CSMD_ULONGS];
  CSMD_USHORT  ausData[CSMD_RD_WR_BUFFER_NBR_CSMD_USHORTS];
  
} CSMD_RD_WR_BUFFER;


/* -------------------------------------------------------------------------- */
/*! \brief Field lengths of MDT telegram segments                             */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_MDT_LENGTH_STR
{
  CSMD_USHORT  usTel;    /*!< Byte length of whole telegram (HP+EF+SVC+RTD)               */
  CSMD_USHORT  usHP;     /*!< Byte length of Hot Plug field                               */
  CSMD_USHORT  usEF;     /*!< Byte length of Extended Function field                      */
  CSMD_USHORT  usSVC;    /*!< Byte length of Service Channel field                        */
  CSMD_USHORT  usRTD;    /*!< Byte length of Real time data field                         */
  
} CSMD_MDT_LENGTH;


/* -------------------------------------------------------------------------- */
/*! \brief Field lengths of AT telegram segments                              */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_AT_LENGTH_STR
{
  CSMD_USHORT  usTel;    /*!< Byte length of whole telegram (HP+SVC+CC+RTD)               */
  CSMD_USHORT  usHP;     /*!< Byte length of Hot Plug field                               */
  CSMD_USHORT  usSVC;    /*!< Byte length of Service Channel field                        */
  CSMD_USHORT  usCC;     /*!< Byte length of all CC connections                           */
  CSMD_USHORT  usCC_M;   /*!< Byte length of CC connections also consumed by the master (part of usCC !) */
  CSMD_USHORT  usRTD;    /*!< Byte length of Real Time Data field                         */
  CSMD_USHORT  usMProd;  /*!< Byte length of all Master Produced con. (part of the RTD !) */
  
} CSMD_AT_LENGTH;


/* -------------------------------------------------------------------------- */
/*! \brief Structure for Event control                                        */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_EVENT_CONTROL_STR
{
  CSMD_ULONG   aulTime[CSMD_NBR_EVENT_ID];         /*!< Event time [ns] related to tScyc  */
  CSMD_BOOL    aboConfigured[CSMD_NBR_EVENT_ID];   /*!< Event is configured               */
  
} CSMD_EVENT_CONTROL;


/* -------------------------------------------------------------------------- */
/*! \brief Structure for control of CYC_CLK                                   */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_CYCCLK_CONTROL_STR
{
  CSMD_BOOL   boActivate;         /*!< Activate CYC_CLK (Timing Slave mode) */
  CSMD_BOOL   boEnableInput;      /*!< Enable of Input CYC_CLK */
  CSMD_BOOL   boPolarity;         /*!< Polarity of input CYC_CLK */
  CSMD_ULONG  ulStartDelay;       /*!< Value [ns] for TCYCSTART */
  
} CSMD_CYCCLK_CONTROL;





/*---- Declaration public Types ---------------------------------------------*/


/*------------------------------------------------------------- */
/* CoSeMa Driver and Sercos controller identifier structure     */
/*------------------------------------------------------------- */

#define CSMD_SIII_DEV_VER_LENGTH    24U
#define CSMD_DRV_VER_LENGTH         24U
#define CSMD_DRV_DAT_LENGTH         16U

/* -------------------------------------------------------------------------- */
/*! \brief CoSeMa and Sercos controller version information                   */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_VERSION_STR
{
  /*! Sercos controller identification: (88CD for Sercos III IP-Core) */
  CSMD_USHORT  usDeviceIdent;
  
  /*! Sercos controller type: 0=Master 1=Slave 2=Master device with conformizer features */
  CSMD_USHORT  usDeviceType;
  
  /*! Sercos controller version: Index 0: Version, Index 1: Release, Index 2: Testversion */
  CSMD_USHORT  usDeviceVersion [3];
  
  /*! CoSeMa Driver Version: Index 0 for Version, 1 for Minor-version and 2 for Release */
  CSMD_USHORT  usDriverVersion [3];
  
  /*! Sercos Device Version */
  CSMD_CHAR    caSIII_Device [CSMD_SIII_DEV_VER_LENGTH];
  
  /*! CoSeMa Driver Version (Name/Version/Subversion/Release) */
  CSMD_CHAR    caDriverVersion [CSMD_DRV_VER_LENGTH];
  
  /*! CoSeMa Driver Release Date (like ANSI C: __DATE__) */
  CSMD_CHAR    caDriverDate [CSMD_DRV_DAT_LENGTH];
  
} CSMD_VERSION;


/* -------------------------------------------------------------------------- */
/*! \brief Extended diagnosis                                                 */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_EXTENDED_DIAG_STR
{
  CSMD_USHORT   usNbrSlaves;                      /*!< Number of slaves with error                       */
  CSMD_ULONG    ulIDN;                            /*!< Affected Sercos parameter (processing step)       */
  CSMD_USHORT   ausSlaveIdx[CSMD_MAX_SLAVES];     /*!< Slave index of all affected slaves [slave index]  */
  CSMD_FUNC_RET aeSlaveError[CSMD_MAX_SLAVES];    /*!< Error code of all affected slaves [slave index]   */
  
} CSMD_EXTENDED_DIAG;


/* -------------------------------------------------------------------------- */
/*! \brief Sercos time structure                                              */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_SERCOSTIME_STR
{
  CSMD_ULONG   ulNanos;        /*!< post decimal positions of Sercos time in nanoseconds */
  CSMD_ULONG   ulSeconds;      /*!< Sercos time in seconds */
  
} CSMD_SERCOSTIME;


#ifdef CSMD_HW_WATCHDOG

#define CSMD_WD_TO_SEND_EMPTY_TEL   CSMD_HAL_WD_TO_SEND_EMPTY_TEL
#define CSMD_WD_TO_DISABLE_TX_TEL   CSMD_HAL_WD_TO_DISABLE_TX_TEL

/* -------------------------------------------------------------------------- */
/*! \brief Watchdog Status structure                                          */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_WDSTATUS_STR
{
  CSMD_USHORT  usActCount;     /*!< actual value of watchdog counter (WDCNT bit 31 - 16) */
  CSMD_BOOL   boActive;   /*!< watchdog is active (WDCSR bit 16) */
  CSMD_BOOL   boAlarm;    /*!< watchdog alarm. Actual watchdog count has reached zero (WDCSR bit 17) */
  
} CSMD_WDSTATUS;
#endif


#ifdef CSMD_SYNC_DPLL
/* -------------------------------------------------------------------------- */
/*! \brief Definitions for PLL status return messages                         */
/* -------------------------------------------------------------------------- */
typedef enum CSMD_PLL_STATE_EN
{
  CSMD_PLL_NO_INFO        = 0x0000,   /*!< No status information from PLL available       */
  CSMD_PLL_INIT           = 0x0100,   /*!< PLL is in init state                           */
  CSMD_PLL_ACTIVE         = 0x0200,   /*!< PLL is active 
                                               (set after start sync command is executed) */
  CSMD_PLL_LOCKED         = 0x0400,   /*!< PLL is locked to the master                    */
  CSMD_PLL_SYNC_ERROR     = 0x0800,   /*!< PLL is outside of the synchronization window   */
  
  CSMD_PLL_CMD_ACTIVE     = 0x8000    /*!< Command for starting/stoping the PLL is active */
  
} CSMD_PLL_STATE;


#ifdef CSMD_DEBUG_DPLL
#define CSMD_RPLL_STAT_MAX_ENTRYS   64U

/* -------------------------------------------------------------------------- */
/*! \brief PLL debug: Full PLL status info for a change in the PLL state      */
/* -------------------------------------------------------------------------- */
typedef struct LIST_PLL_STAT_STR
{
  CSMD_ULONG   ulStatus[CSMD_RPLL_STAT_MAX_ENTRYS];
  
} LIST_PLL_STAT;

/* -------------------------------------------------------------------------- */
/*! \brief PLL debug: PLL status return for a change in the PLL state.        */
/* -------------------------------------------------------------------------- */
typedef struct LIST_PLL_RET_STR
{
  CSMD_PLL_STATE  eState[CSMD_RPLL_STAT_MAX_ENTRYS];
  
} LIST_PLL_RET;

/* -------------------------------------------------------------------------- */
/*! \brief PLL debug: cycle count for a change in the PLL state               */
/* -------------------------------------------------------------------------- */
typedef struct LIST_CYC_CNT_STR
{
  CSMD_ULONG   ulCycCnt[CSMD_RPLL_STAT_MAX_ENTRYS];
  
} LIST_CYC_CNT;

/* -------------------------------------------------------------------------- */
/*! \brief PLL debug: information structure                                   */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_PLL_DBG_INFO_STR
{
  
  CSMD_ULONG      ulCurrIdx;  /* Current list index for last stored PLL status        */
  CSMD_ULONG      ulCycCnt;   /* Current Sercos cycle cnt                             */
  
  LIST_PLL_STAT   rPLL_Stat;  /* List with PLL status                                 */
  LIST_PLL_RET    rSPLL_State;
  LIST_CYC_CNT    rCyc_Cnt;   /* List with CycCnt correspondig to stored PLL status.  */
  
} CSMD_PLL_DBG_INFO;
#endif  /* #ifdef CSMD_DEBUG_DPLL */

#endif  /* #ifdef CSMD_SYNC_DPLL */


/* -------------------------------------------------------------------------- */
/*! \brief Sercos list: FPGA Telegram and Error counter                       */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_COMM_COUNTER_STR
{
  CSMD_USHORT  usRealLen;      /*!< Current length */
  CSMD_USHORT  usMaxLen;       /*!< Max length of the Sercos list */
  CSMD_USHORT  usIPFRXOK;      /*!< Counter for error-free received frames */
  CSMD_USHORT  usIPFTXOK;      /*!< Counter for transmitted frames */
  CSMD_USHORT  usIPFCSERR;     /*!< Counter for received Ethernet frames with FCS error */
  CSMD_USHORT  usIPALGNERR;    /*!< Counter for received Ethernet frames with an alignment error */
  CSMD_USHORT  usIPDISRXB;     /*!< Counter for discarded receive Ethernet frames based on missing rx buffer resources */
  CSMD_USHORT  usIPDISCLB;     /*!< Counter for discarded forwarding Ethernet frames based on missing collision buffer resources */
  CSMD_USHORT  usIPCHVIOL;     /*!< Counter for Ethernet frames which violate the UC channel window */
  CSMD_USHORT  usIPSERCERR;    /*!< Counter for Ethernet frames with a wrong FCS or which are misaligned (resettable) */
  
} CSMD_COMM_COUNTER;




/* -------------------------------------------------------------------------- */
/*! Definitions for the UC channel driver                                     */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/*! \brief Enumeration of callback events to ip driver                        */
/* -------------------------------------------------------------------------- */
typedef enum CSMD_SIII_EVENT_NBR_EN
{
  CSMD_SIII_NO_EVENT = 0,       /*!< unused */
  
  /*! Link Events */
  CSMD_LINK_DOWN_PORT1,         /*!< unused */
  CSMD_LINK_DOWN_PORT2,         /*!< unused */
  
  /*! Topology Events */
  CSMD_SIII_RING_BREAK,         /*!< Ring break detected */
  CSMD_SIII_RING_CLOSED,        /*!< Ring closure detected */
  
  /*! Sercos Events */
  CSMD_SIII_STOP_COMMUNICATION,   /*!< Shall stop communication in the UC channel               */
  CSMD_SIII_START_COMMUNICATION   /*!< Shall start communication in the UC channel              */
  
  /* Timer Events */
  
} CSMD_SIII_EVENT_NBR;

/* -------------------------------------------------------------------------- */
/*! \brief callback function: information parameter                           */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_SIII_EVENT_STR
{
  CSMD_SIII_EVENT_NBR     eEventId;
  /*!< other data required? */
  
} CSMD_SIII_EVENT;

/* -------------------------------------------------------------------------- */
/*! \brief Table of callback functions to ip driver                           */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_CB_FUNCTIONS_STR
{
  CSMD_ULONG (*RxTxRamAlloc)   ( CSMD_VOID       *pvCB_Info,      /*!< distinguish the instance       */
                                 /* The following values are segment (256 bytes) length aligned       */
                                 CSMD_ULONG       ulTxRamS3Used,  /*!< Tx-Ram: used for Sercos frames */
                                 CSMD_ULONG       ulTxRamSize,    /*!< Tx-Ram: total available size   */
                                 CSMD_ULONG       ulRxRamS3Used,  /*!< Rx-Ram: used for Sercos frames */
                                 CSMD_ULONG       ulRxRamSize );  /*!< Rx-Ram: total available size   */
  
  CSMD_ULONG (*S3Event)        ( CSMD_VOID       *pvCB_Info,      /*!< distinguish the instance */
                                 CSMD_SIII_EVENT *prEvent );      /*!< Sercos Event information */
  
  CSMD_ULONG (*S3EventFromISR) ( CSMD_VOID       *pvCB_Info,      /*!< distinguish the instance */
                                 CSMD_SIII_EVENT *prEvent );      /*!< Sercos Event information */
  
} CSMD_CB_FUNCTIONS;


/* -------------------------------------------------------------------------- */
/*! \brief Sercos communication FPGA Ram layout information structure         */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_SIII_INFO_STR
{
  /*! Allocated FPGA Tx-Ram for Sercos telegrams (IP segment length aligned) */
  CSMD_ULONG   ulTxRamS3Used;
  
  /*! Size of available FPGA Tx-Ram for Sercos and IP telegrams */
  CSMD_ULONG   ulTxRamSize;
  
  /*! Allocated FPGA Rx-Ram for Sercos telegrams (IP segment length aligned) */
  CSMD_ULONG   ulRxRamS3Used;
  
  /*! Size of available FPGA Rx-Ram for Sercos and IP telegrams */
  CSMD_ULONG   ulRxRamSize;
  
} CSMD_SIII_INFO;


/* -------------------------------------------------------------------------- */
/*! \brief CoSeMa communication status structure                              */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_STATUS_STR
{
  CSMD_USHORT  usNumOfSlavesP1;   /*!< Number of operable slaves connected to port 1 */
  CSMD_USHORT  usNumOfSlavesP2;   /*!< Number of operable slaves connected to port 2 */
  CSMD_USHORT  usRingStatus;      /*!< Ring, single line, broken ring (topology) */
  CSMD_USHORT  usLinkStatusP1;    /*!< Link status port 1 (1 = Link is active) */
  CSMD_USHORT  usLinkStatusP2;    /*!< Link status port 2 (1 = Link is active) */
  CSMD_SHORT   sSERCOS_Phase;     /*!< Communication phase (Sercos phase) */
  
} CSMD_STATUS;






/* -------------------------------------------------------------------------- */
/*! \brief enumeration for connection producer states                         */
/* -------------------------------------------------------------------------- */
typedef enum CSMD_PROD_STATE_EN
{
    CSMD_PROD_STATE_INIT,         /*!< init       */
    CSMD_PROD_STATE_PREPARE,      /*!< prepare    */
    CSMD_PROD_STATE_READY,        /*!< ready      */
    CSMD_PROD_STATE_PRODUCING,    /*!< producing  */
    CSMD_PROD_STATE_STOPPING,     /*!< stopping   */
    CSMD_PROD_STATE_WAITING       /*!< waiting    */

} CSMD_PROD_STATE;

/* -------------------------------------------------------------------------- */
/*! \brief enumeration for connection consumer states                         */
/* -------------------------------------------------------------------------- */
typedef enum CSMD_CONS_STATE_EN
{
    CSMD_CONS_STATE_INIT,         /*!< init       */
    CSMD_CONS_STATE_PREPARE,      /*!< prepare    */
    CSMD_CONS_STATE_WAITING,      /*!< waiting    */
    CSMD_CONS_STATE_CONSUMING,    /*!< consuming  */
    CSMD_CONS_STATE_STOPPED,      /*!< stopping   */
    CSMD_CONS_STATE_WARNING,      /*!< warning    */
    CSMD_CONS_STATE_ERROR = 7     /*!< error      */

} CSMD_CONS_STATE;

/*! \endcond */ /* PUBLIC */


/*! \cond PRIVATE */

/* -------------------------------------------------------------------------- */
/*! \brief enumeration for C_CON check mode                                   */
/* -------------------------------------------------------------------------- */
typedef enum  CSMD_C_CON_CHECK_MODE_EN
{
    CSMD_CHECK_MODE_INIT,         /*!< check mode not yet initialized                                       */
    CSMD_CHECK_MODE_NEW_DATA,     /*!< only new data bit is checked in every producing cycle                */
    CSMD_CHECK_MODE_COUNTER       /*!< new data bit and C_CON counter are checked in every producing cycle  */

} CSMD_C_CON_CHECK_MODE;

/* -------------------------------------------------------------------------- */
/*! \brief private structure for master-produced connections                  */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_CONN_MASTERPROD_STR
{
  CSMD_PROD_STATE   eState;           /*!< producer state of connection (according to S-0-1050.x.09)      */
  CSMD_USHORT       usC_Con;          /*!< connection control of connection (according to S-0-1050.x.08)  */
  CSMD_USHORT       usProduced;       /*!< bit list for connection production time                        */
  CSMD_USHORT      *apusConnTxRam     /*!< array of pointers to connection data in TxRam [buffer index]   */
                      [CSMD_MAX_TX_BUFFER];

} CSMD_CONN_MASTERPROD;

/* -------------------------------------------------------------------------- */
/*! \brief private structure for slave-produced connections                   */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_CONN_SLAVEPROD_STR
{
  CSMD_CONS_STATE         eState;           /*!< producer state of connection (according to S-0-1050.x.09)         */
  CSMD_USHORT             usExpected_C_Con; /*!< expectation value of C_CON in current producer cycle              */
  CSMD_USHORT             usProdIdx;        /*!< slave index of connection's producer                              */
  CSMD_USHORT             usLatest_C_Con;   /*!< latest read value of C_CON                                        */
  CSMD_USHORT             usProduced;       /*!< bit list for connection production time                           */
  CSMD_C_CON_CHECK_MODE   eCheckMode;       /*!< mode of synchronization check (NewData / Counter)                 */
  CSMD_USHORT             usAbsoluteErr;    /*!< error counter for absolute connection data losses                 */
  CSMD_USHORT             usConsecErr;      /*!< error counter for consecutive connection data losses              */
  CSMD_USHORT            *apusConnRxRam     /*!< array of pointers to connection data by port and active Rx buffer */
                            [CSMD_NBR_PORTS][CSMD_MAX_RX_BUFFER];

} CSMD_CONN_SLAVEPROD;


typedef struct CSMD_CC_CONN_LIST_STR
{
    CSMD_USHORT  usConnIdx;          /*!< connection index in configuration structure                 */
    CSMD_USHORT  usDataOffset;       /*!< data offset, used for telegram and RAM calculation [bytes]  */
    CSMD_USHORT  usTelNbr;           /*!< Telegram number (0-3)                                       */
    CSMD_USHORT  usMasterConsumeCC;  /*!< flag if master consumes connection (TRUE = master-consumed) */

} CSMD_CC_CONN_LIST;


typedef struct CSMD_CC_CONN_STRUCT_STR
{
    CSMD_USHORT        usNbrCC_Connections;   /*!< total number of CC connections configured                */
#ifdef CSMD_STATIC_MEM_ALLOC
    CSMD_CC_CONN_LIST  parCC_ConnList         /*!< list of CC connections needed for descriptor assignment  */
                         [CSMD_MAX_GLOB_CONN];
#else
    CSMD_CC_CONN_LIST *parCC_ConnList;        /*!< list of CC connections needed for descriptor assignment  */
#endif
} CSMD_CC_CONN_STRUCT;


/* --------------------------------------------------*/
/*! \brief enumeration for ring monitoring mode      */
/* --------------------------------------------------*/
typedef enum CSMD_MONITORING_MODE_EN
{
    CSMD_MONITORING_OFF,            /*!< cyclic handling inactive (during communication phase switch)                */
    CSMD_MONITORING_COPY_ONLY,      /*!< cyclic handling active (after phase switch has been processed successfully) */
    CSMD_MONITORING_FULL            /*!< cyclic handling active including check of S-DEV of all slaves               */

} CSMD_MONITORING_MODE;


/* -------------------------------------------------------------------------- */
/*! \brief Structure used for port related scan information                   */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_ADDR_SCAN_INFO_STR
{
  CSMD_USHORT   usSeqCntInit;       /*!< initial value for sequence counter             */
  CSMD_USHORT   usAddressNmb;       /*!< Number of detected slaves                      */
  CSMD_USHORT   ausAddresses        /*!< Array with scan results [Topology address - 1] */
                  [CSMD_NBR_SCAN_SLAVES];
  
} CSMD_ADDR_SCAN_INFO;


/* -------------------------------------------------------------------------- */
/*! \brief Structure for SWC related variables                                */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_SWC_CONTROL_STR
{
  CSMD_BOOL     boNoForwardLastSlave;   /*!< Active state of "Switch off Sercos III telegrams" */
  CSMD_BOOL     boFastCPSwitchActive;   /*!< Active state of "Fast communication phase switch" */
  CSMD_UCC_MODE_ENUM
                eActiveUCC_Mode_CP12;   /*!< Active Mode for UC channel in CP1/CP2 */

} CSMD_SWC_CONTROL;


/* ------------------------------------------------------------------------------------------------ */
/*! \brief Structure containing auxiliary variables for determination of incoming telegrams by port */
/* ------------------------------------------------------------------------------------------------ */
typedef struct CSMD_REDUNDANCY_AUX_STR
{
  CSMD_ULONG  aulReg_TGSR[CSMD_NBR_PORTS];  /*!< Copy of TGSR registers of latest cycle by master port */

  CSMD_USHORT ausRxBuffer[CSMD_NBR_PORTS];  /*!< Copy of active Rx Ram buffer [master port 0/1] */
  CSMD_ULONG  aulATBufValid[CSMD_NBR_PORTS];  /*!< Copy of RXBUFTV register of currently active buffer */

  CSMD_BOOL   boNewDataP1;
  CSMD_BOOL   boSecTelP1;       /*!< Valid MST (inside or outside of monitoring window) with secondary telegram received at port 1 */
  CSMD_BOOL   boPriTelP1;       /*!< Valid MST (inside or outside of monitoring window) with primary telegram received at port 1 */

  CSMD_BOOL   boNewDataP2;
  CSMD_BOOL   boSecTelP2;       /*!< Valid MST (inside or outside of monitoring window) with secondary telegram received at port 2 */
  CSMD_BOOL   boPriTelP2;       /*!< Valid MST (inside or outside of monitoring window) with primary telegram received at port 2 */

  CSMD_USHORT usRingDefect;     /*!< 1 = defect on primary line; 2 = defect on secondary line */
  CSMD_USHORT usDefectRingCP0;  /*!< Defect Ring in CP0, 1 = primary, 2 = secondary */

} CSMD_REDUNDANCY_AUX;


/* -------------------------------------------------------------------------- */
/*! \brief Structure used for runtime warnings and errors                     */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_RUNTIME_INFO_STR
{
  CSMD_USHORT    usSlaveIndex;           /*!< slave index               */
  CSMD_CHAR      acInfo[120];            /*!< displayed message         */
  
} CSMD_RUNTIME_INFO;


/*-------------------------------------------------------------------------- */
/* Mask definitions for SCP configuration bitlist                            */
/*-------------------------------------------------------------------------- */
/* SCP Type: basic communication profile classes                                              */
#define CSMD_SCP_FIXCFG           0x01000000U   /* FIXed ConFiGuration                        */
#define CSMD_SCP_FIXCFG_V2        0x02000000U   /* FIXed ConFiGuration    V2                  */
#define CSMD_SCP_FIXCFG_V3        0x04000000U   /* FIXed ConFiGuration    V3                  */
#define CSMD_SCP_VARCFG           0x10000000U   /* VARiable ConFiGuration                     */
#define CSMD_SCP_VARCFG_V2        0x20000000U   /* VARiable ConFiGuration V2                  */
#define CSMD_SCP_VARCFG_V3        0x40000000U   /* VARiable ConFiGuration V3                  */

/* SCP Type: additinal communication profile classes                                          */
#define CSMD_SCP_SYNC             0x00000001U   /* SYNChronisation                            */
#define CSMD_SCP_SYNC_V2          0x00000002U   /* SYNChronisation (tSync > tScyc)            */
#define CSMD_SCP_SYNC_V3          0x00000004U   /* SYNChronisation (tSync > tScyc)            */
#define CSMD_SCP_WD               0x00000008U   /* WatchDog                                   */
#define CSMD_SCP_WDCON            0x00000010U   /* WatchDog (with tPcyc & data losses)        */
#define CSMD_SCP_RTB              0x00000020U   /* RealTimeBits                               */
#define CSMD_SCP_NRT              0x00000040U   /* UC channel                                 */
#define CSMD_SCP_CAP              0x00000080U   /* Connection Capabilities                    */
#define CSMD_SCP_SYSTIME          0x00000100U   /* Set Sercos Time using MDT extended field   */
#define CSMD_SCP_NRTPC            0x00000200U   /* UC channel & IP settings                   */
#define CSMD_SCP_CYC              0x00000400U   /* Cyclic communication                       */
#define CSMD_SCP_SWC              0x00000800U   /* Industrial Ethernet protocols via UCC      */


#define CSMD_SCP_FIXCFG_ALL       (  CSMD_SCP_FIXCFG       /* Mask over all FixCFG versions              */\
                                   | CSMD_SCP_FIXCFG_V2\
                                   | CSMD_SCP_FIXCFG_V3)

#define CSMD_SCP_VARCFG_ALL       (  CSMD_SCP_VARCFG       /* Mask over all FixCFG versions              */\
                                   | CSMD_SCP_VARCFG_V2\
                                   | CSMD_SCP_VARCFG_V3)

#define CSMD_SCP_BASIC            (  CSMD_SCP_FIXCFG_ALL   /* Mask over all basic Communication Profiles */\
                                   | CSMD_SCP_VARCFG_ALL)


/* -------------------------------------------------------------------------- */
/*! \brief Sercos service channel container structure                         */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_HAL_SERC3SVC_STR  CSMD_SERC3SVC;


/* -------------------------------------------------------------------------- */
/*! \brief structure for software service channel variables                   */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_SOFT_SVC_STR
{
  CSMD_USHORT         usHSTimeout;       /*!< Handshake Timeout counter */
  CSMD_USHORT         usBusyTimeout;     /*!< SVC Busy Timeout counter  */
  CSMD_USHORT         usInUse;           /*!< SVC in use marker         */

} CSMD_SOFT_SVC_STRUCT;



/* #ifdef CSMD_HOTPLUG */
/* -------------------------------------------------------------------------- */
/*                                                                            */
/*  Sercos V1.3                                                               */
/*                                                                            */
/*          |<-------------  Hot-Plug Field  ------------->|                  */
/*          |                                              |                  */
/*            2 Byte      2 Byte            4 Byte                            */
/*          +-----------+-----------+----------------------+                  */
/*  MDT0    | selection | control   |        Info          |                  */
/*          |           |  word     |                      |                  */
/*          +-----------+-----------+----------------------+                  */
/*                                                                            */
/*            2 Byte      2 Byte            4 Byte                            */
/*          +-----------+-----------+----------------------+                  */
/*  AT0     | selection | status    |        Info          |                  */
/*          |           |  word     |                      |                  */
/*          +-----------+-----------+----------------------+                  */
/*                                                                            */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*! \brief Hot-Plug: MDT-HP.Selection / AT-HP.Selection                       */
/* -------------------------------------------------------------------------- */
typedef union HP_SELECTION_UN
{
  CSMD_USHORT    usWord;
#ifdef CSMD_DEBUG
  /* Note: maybe this structure must be declared via define in              */
  /*       reverse order for other compilers respectively machines !!!      */
  struct
  {
    CSMD_USHORT  bfSLAVE_INDEX    :  4;  /*!< Bit 15-12: Slave index        */
    CSMD_USHORT  bfSERCOS_ADDRESS : 12;  /*!< Bit 11-0 : Sercos address     */
    
  } rBit;
#endif  /* End: #ifdef CSMD_DEBUG */
  
} HP_SELECTION;


/* -------------------------------------------------------------------------- */
/*! \brief Hot-Plug: MDT-HP.Control (C-HP)                                    */
/* -------------------------------------------------------------------------- */
#ifdef CSMD_PACK_PRAGMA
#pragma pack(push,2)
#endif

union HP_CONTROL_UN
{
  CSMD_USHORT    usWord;
#ifdef CSMD_DEBUG
  /* Note: maybe this structure must be declared via define in              */
  /*       reverse order for other compilers respectively machines !!!      */
  struct
  {
    CSMD_USHORT  btHP_SUPPORT   :  1;   /*!< Bit 15   : Hot-Plug Support                    */
                                        /*!<            '1' = HP supported by master        */
                                        /*!<            '0' = HP not supported by master    */
    CSMD_USHORT  bfReserved5    :  5;   /*!< Bit 14-10: reserved                            */
    CSMD_USHORT  btHP_ENABLE    :  1;   /*!< Bit 9    : Enable Hot-Plug                     */
                                        /*!<            '1' = Hot-Plug enabled              */
                                        /*!<            '0' = Hot-Plug disabled             */
    CSMD_USHORT  btHP2_Trans    :  1;   /*!< Bit 8    : Master changes to HP2               */
                                        /*!<            '1' = Use SVC to communicate        */
                                        /*!<            '0' = Use HP fields to communicate  */
    CSMD_USHORT  bfHP_Coding    :  8;   /*!< Bit 7-0  : Coding of parameter                 */
                                        /*!               1...127 = Coding of HP0 parameter */
                                        /*!             128...255 = Coding of HP1 parameter */
  } rBit;
#endif  /* End: #ifdef CSMD_DEBUG */
  
} /*lint !e659  (Nothing follows '}' on line terminating struct/union/enum definition) */
#ifdef CSMD_PACK_ATTRIBUTE
__attribute__((packed))
#endif
;
#ifdef CSMD_PACK_PRAGMA
#pragma pack(pop)
#endif
typedef union HP_CONTROL_UN  HP_CONTROL;


/* -------------------------------------------------------------------------- */
/*! \brief Hot-Plug: AT-HP.Status (S-HP)                                      */
/* -------------------------------------------------------------------------- */
#ifdef CSMD_PACK_PRAGMA
#pragma pack(push,2)
#endif

union HP_STATUS_UN
{
  CSMD_USHORT    usWord;
#ifdef CSMD_DEBUG
  /* Note: maybe this structure must be declared via define in              */
  /*       reverse order for other compilers respectively machines !!!      */
  struct
  {
    CSMD_USHORT  bfReserved7    :  7;   /*!< Bit 15-9 : reserved                                        */
    CSMD_USHORT  btHP_CONDITION :  1;   /*!< Bit 8    : HP Condition                                    */
                                        /*!<            '1' = Error in HP1 (code see bit 7-0)           */
                                        /*!<            '0' = Acknowledgment in HP1 (code see bit 7-0)  */
    CSMD_USHORT  bfHP1_ACKN_ERR :  8;   /*!< Bit 7-0  : HP1 Acknowledgment or Error code                */
    
  } rBit;
#endif  /* End: #ifdef CSMD_DEBUG */
  
} /*lint !e659  (Nothing follows '}' on line terminating struct/union/enum definition) */
#ifdef CSMD_PACK_ATTRIBUTE
__attribute__((packed))
#endif
;
#ifdef CSMD_PACK_PRAGMA
#pragma pack(pop)
#endif
typedef union HP_STATUS_UN  HP_STATUS;


/* -------------------------------------------------------------------------- */
/*! \brief Hot-Plug: MDT-HP.Info / AT-HP.Info                                 */
/* -------------------------------------------------------------------------- */
#ifdef CSMD_PACK_PRAGMA
#pragma pack(push,1)
#endif

union HP_INFO_UN
{
  CSMD_ULONG       ulLong;                 /*!< Hot-Plug info field                */
  CSMD_USHORT      usWord[2];             
  CSMD_UCHAR       ucByte[4];             
  
} /*lint !e659  (Nothing follows '}' on line terminating struct/union/enum definition) */
#ifdef CSMD_PACK_ATTRIBUTE
__attribute__((packed))
#endif
;
#ifdef CSMD_PACK_PRAGMA
#pragma pack(pop)
#endif
typedef union HP_INFO_UN  HP_INFO;


/* -------------------------------------------------------------------------- */
/*! \brief Hot-Plug Field in MDT0  (8 Byte)                                   */
/* -------------------------------------------------------------------------- */
#ifdef CSMD_PACK_PRAGMA
#pragma pack(push,2)
#endif

struct CSMD_HP_FIELD_MDT0_STR
{
#ifdef CSMD_BIG_ENDIAN
  HP_CONTROL      rControl;       /*!< Hot-Plug Control word (C-HP) */
  HP_SELECTION    rSelection;     /*!< Selection of HP slave        */
#else
  HP_SELECTION    rSelection;     /*!< Selection of HP slave        */
  HP_CONTROL      rControl;       /*!< Hot-Plug Control word (C-HP) */
#endif
  HP_INFO         rInfo;          /*!< Container to transmit data from master to HP slave */
  
} /*lint !e659  (Nothing follows '}' on line terminating struct/union/enum definition) */
#ifdef CSMD_PACK_ATTRIBUTE
__attribute__((packed))
#endif
;
#ifdef CSMD_PACK_PRAGMA
#pragma pack(pop)
#endif
typedef struct CSMD_HP_FIELD_MDT0_STR  CSMD_HP_FIELD_MDT0;


/* -------------------------------------------------------------------------- */
/*! \brief Hot-Plug Field in AT0  (8 Byte)                                    */
/* -------------------------------------------------------------------------- */
#ifdef CSMD_PACK_PRAGMA
#pragma pack(push,2)
#endif

struct CSMD_HP_FIELD_AT0_STR
{
#ifdef CSMD_BIG_ENDIAN
  HP_STATUS       rStatus;        /*!< Hot-Plug Status word (S-HP)      */
  HP_SELECTION    rSelection;     /*!< Selected acknowledge of HP slave */
#else
  HP_SELECTION    rSelection;     /*!< Selected acknowledge of HP slave */
  HP_STATUS       rStatus;        /*!< Hot-Plug Status word (S-HP)      */
#endif
  HP_INFO         rInfo;          /*!< Container to transmit data form HP slave to master */
  
} /*lint !e659  (Nothing follows '}' on line terminating struct/union/enum definition) */
#ifdef CSMD_PACK_ATTRIBUTE
__attribute__((packed))
#endif
;
#ifdef CSMD_PACK_PRAGMA
#pragma pack(pop)
#endif
typedef struct CSMD_HP_FIELD_AT0_STR  CSMD_HP_FIELD_AT0;



/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*! Repeat rate for t6 or t7 HP field transmission in CP3 [Sercos cycles] */
#define  CSMD_HP_CP3_REPEATRATE_T6_T7   16U

/*! Timeout for number of HP0 parameter send cycles */
#define  CSMD_HP0_PAR_SEND_CYCLES       10U

/*! Wait time for HP port change if case of broken ring topology and non-Sercos link on port 2 (Sercos cycles) */
#define  CSMD_PORT_CHANGE_WAIT_CYCLES   10U

/*! Timeout for slave scan in HP0 [Sercos cycles] */
#define  CSMD_HP0_SCAN_TIMEOUT          50U

/*-------------------------------------------------------- */
/* Definitions for Hot-Plug: MDT/AT-HP.Selection           */
/*-------------------------------------------------------- */
#define  CSMD_HP_ADD_SLAVE_IDX_MASK     0xF000U             /* Bit 15-12: Slave Index (0...15)               */
#define  CSMD_HP_ADD_SLAVE_IDX_OFFSET   0x1000U             /*   "Add value" for slave index                 */
#define  CSMD_HP_ADD_SLAVE_IDX_0        0x0000U             /*   Slave Index 0  (= min. slave index)         */
#define  CSMD_HP_ADD_SLAVE_IDX_SHIFT    12U
#define  CSMD_HP_ADD_MAX_SLAVES         16U                 /*   Max. nbr. of Slaves of a multi slave device */

#define  CSMD_HP_ADD_SERCOS_ADD_MASK    0x0FFFU             /* Bit 11-0 : Sercos address                     */
#define  CSMD_HP_ADD_DEFAULT_SADD       0U                  /*   Default Sercos address                      */
#define  CSMD_HP_ADD_MIN_SADD           CSMD_MIN_SLAVE_ADD  /*   Min. operation Sercos address               */
#define  CSMD_HP_ADD_MAX_SADD           CSMD_MAX_SLAVE_ADD  /*   Max. operation Sercos address               */
#define  CSMD_HP_ADD_NOT_EXIST          4093U               /*   AT-HP:  End of scanning                     */
#define  CSMD_HP_ADD_BRDCST_ADD         4095U               /*   MDT_HP: Broadcast address                   */


/*-------------------------------------------------------- */
/* Definitions for Hot-Plug: MDT-HP.Control                */
/*-------------------------------------------------------- */
#define  CSMD_HP_CNTRL_SUPPORTED            0x8000U   /* Bit 15   : Hot-Plug supported by master       */
#define  CSMD_HP_CNTRL_ENABLED              0x0200U   /* Bit 9    : Hot-Plug enabled                   */
#define  CSMD_HP_CNTRL_SVC_ACTIVE           0x0100U   /* Bit 8    : Transmission via SVC is used       */
#define  CSMD_HP_CNTRL_PAR_CODING_MASK      0x00FFU   /* Bit 7-0  : HP0/HP1 parameter coding           */


/*-------------------------------------------------------- */
/* Definitions for Hot-Plug: MDT-HP.Control HP0 coding     */
/*-------------------------------------------------------- */
#define  CSMD_HP_CODE_NO_DATA               0U        /* No HP parameter                               */

#define  CSMD_HP_CODE_TS_CYC                1U        /* S-0-1002     Communication Cycle time (tScyc) */
#define  CSMD_HP_CODE_T6                    2U        /* S-0-1017 [0] Begin of the UC channel (t6)     */
#define  CSMD_HP_CODE_T7                    3U        /* S-0-1017 [1] End of the UC channel (t7)       */
#define  CSMD_HP_CODE_REQ_MTU               4U        /* S-0-1027.0.1 Requested MTU                    */
#define  CSMD_HP_CODE_COMM_VERSION          5U        /* Communication version (MDT0 of CP0)           */

#define  CSMD_HP_CODE_MDT0_LENGTH          16U        /* S-0-1010 [0] Lengths of MDTs  (MDT0)          */
#define  CSMD_HP_CODE_MDT1_LENGTH          17U        /* S-0-1010 [1] Lengths of MDTs  (MDT1)          */
#define  CSMD_HP_CODE_MDT2_LENGTH          18U        /* S-0-1010 [2] Lengths of MDTs  (MDT2)          */
#define  CSMD_HP_CODE_MDT3_LENGTH          19U        /* S-0-1010 [3] Lengths of MDTs  (MDT3)          */

#define  CSMD_HP_CODE_AT0_LENGTH           32U        /* S-0-1012 [0] Lengths of ATs   (AT0)           */
#define  CSMD_HP_CODE_AT1_LENGTH           33U        /* S-0-1012 [1] Lengths of ATs   (AT0)           */
#define  CSMD_HP_CODE_AT2_LENGTH           34U        /* S-0-1012 [2] Lengths of ATs   (AT0)           */
#define  CSMD_HP_CODE_AT3_LENGTH           35U        /* S-0-1012 [3] Lengths of ATs   (AT0)           */

/*-------------------------------------------------------- */
/* Definitions for Hot-Plug: MDT-HP.Control HP1 coding     */
/*-------------------------------------------------------- */
#define  CSMD_HP_CODE_MDT_SVC_OFFS        128U        /* S-0-1013 SVC offset in MDT                    */
#define  CSMD_HP_CODE_AT_SVC_OFFS         129U        /* S-0-1014 SVC offset in AT                     */


/*-------------------------------------------------------- */
/* Definitions for Hot-Plug: AT-HP.Status                  */
/*-------------------------------------------------------- */
#define  CSMD_HP_STAT_ERROR               0x0100U     /* Bit 8    : Hot-Plug error condition           */
                                                      /*            '0' = Acknowledgment in HP1        */
                                                      /*            '1' = Error in HP1                 */
#define  CSMD_HP_STAT_ACKN_ERR_MASK       0x00FFU     /* Bit 7-0  : HP acknowledgment/error code       */


/*------------------------------------------------------------------------ */
/* Definitions for Hot-Plug: AT-HP.Status HP1 acknowledgment/error codes   */
/*------------------------------------------------------------------------ */
/* Error */
#define  CSMD_HP_ERR_SWITCH_TO_SVC          2U        /* Error occurs during activating of SVC         */
#define  CSMD_HP_ERR_SWITCH_TO_HP1          4U        /* Master does not transmit the Sercos address   */
#define  CSMD_HP_ERR_SLAVE_SCAN             5U        /* Slave scan error (not all slaves scanned)     */
/* Acknowledgment */
#define  CSMD_HP_ACKN_SLAVE_SCAN_OK         1U        /* The HP slave was successfully scanned         */
/* Acknowledgment or Error */
#define  CSMD_HP_STAT_CODE_MDT_SVC_OFFS   128U        /* S-0-1013 SVC offset in MDT                    */
#define  CSMD_HP_STAT_CODE_AT_SVC_OFFS    129U        /* S-0-1014 SVC offset in AT                     */


/* -------------------------------------------------------------------------- */
/*! \brief Hot Plug mechanism data                                            */
/* -------------------------------------------------------------------------- */
#ifdef CSMD_PACK_PRAGMA
#pragma pack(push,2)
#endif

struct CSMD_HP_STR
{
  CSMD_HP_FIELD_MDT0  rHpField_MDT0;
  CSMD_HP_FIELD_AT0   rHpField_AT0;
  CSMD_ULONG         *pulTxRam;

} /*lint !e659  (Nothing follows '}' on line terminating struct/union/enum definition) */
#ifdef CSMD_PACK_ATTRIBUTE
__attribute__((packed))
#endif
;
#ifdef CSMD_PACK_PRAGMA
#pragma pack(pop)
#endif
typedef struct CSMD_HP_STR  CSMD_HP_STRUCT;
/* #endif   End: #ifdef CSMD_HOTPLUG */


#ifdef CSMD_HOTPLUG
/* ------------------------------------------------------------------- */
/*! \brief structure containing private variables for hot plug feature */
/* ------------------------------------------------------------------- */
typedef struct CSMD_HOT_PLUG_AUX_STR
{
  CSMD_USHORT     usHP_Phase;                     /*!< Hot-Plug Phase */
  CSMD_BOOL       boHotPlugActive;                /*!< Flag which prevents overwriting of DFCSR by CSMD_SetCollisionBuffer() after HP */
  CSMD_HP_STRUCT *prHP_ActPort;                   /*!< HP   pointer to data structure for the port with activated hot plug */
  CSMD_FUNC_RET   eHP_FuncRet;                    /*!< HP   temporary buffer for error return */
  CSMD_USHORT     usHP0_RepeatRate;               /*!< HP0: parameter transmission repeat rate */
  CSMD_USHORT     usHP0_RepeatCnt;                /*!< HP0: parameter transmission counter [Sercos cycles] */
  CSMD_USHORT     usHP0_ParamCnt;                 /*!< HP0: parameter counter */
  CSMD_BOOL       boHP0_CheckLink;                /*!< HP0: Check for Sercos link is activated */
  CSMD_USHORT     usHP_Timeout;                   /*!< HP : parameter block transmission timeout counter */
  CSMD_USHORT     usHP_Topology;                  /*!< HP   detected topology at start of HP mechanism */
  CSMD_USHORT     usHP_IdxLastSlave;              /*!< HP   Index of last slave on inactive port */
  CSMD_USHORT     usHP_ScanIdx;                   /*!< HP0: slave index for address scan */
  CSMD_USHORT     usHP0_ScanTimeout;              /*!< HP0: Timeout for S-DEV Topology check / slave scan timeout repeat rate [Sercos cycles] */
  CSMD_USHORT     usHP_NbrSlaves;                 /*!< HP0: Number of found slaves in the scanned device */
  CSMD_USHORT     ausHP_SlaveAddList
                    [CSMD_HP_ADD_MAX_SLAVES+2];   /*!< HP   Sercos list with Sercos addresses of the HP device [slave index] */
} CSMD_HOT_PLUG_AUX;
#endif

/*! \endcond */ /* PRIVATE */




/*! \cond PUBLIC */
/*! Number of configurable TCNT Events */
#define  CSMD_TIMER_EVENT_NUMBER    16U
/*! Number of configurable TCNT[1] TCNT[2] Events (port related) */
#define  CSMD_PORTS_EVENT_NUMBER    16U

/* -------------------------------------------------------------------------- */
/*! \brief Timer Event data structure                                         */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_EVENT_STR
{
  CSMD_HAL_SERCFPGA_DATTYP    ulTime;         /*!< Event TCNT Value [ns]    */
  CSMD_HAL_SERCFPGA_WDATTYP   usSubCycCnt;    /*!< Sub Cycle Count Value    */
  CSMD_HAL_SERCFPGA_WDATTYP   usType;         /*!< Event Type               */
  CSMD_HAL_SERCFPGA_WDATTYP   usSubCycCntSel; /*!< Sub Cycle Counter Select */

} CSMD_EVENT;

#ifdef CSMD_DEBUG
/* -------------------------------------------------------------------------- */
/*! \brief Variable with debug informations                                   */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_DBG_DATA_STR
{
  CSMD_USHORT  *pusTelDbg;      /*!< CSMD_Telegram_Clear() */
  CSMD_USHORT   usLenDbg;       /*!< CSMD_Telegram_Clear() */

  /*! Rx Buffer Offsets */
  CSMD_ULONG    aulRxBufBasePtr[ CSMD_HAL_RX_BASE_PTR_NBR ];
  /*! Rx Buffer Sizes */
  CSMD_ULONG    aulRxBufSize[ CSMD_HAL_RX_BASE_PTR_NBR ];
  /*! Tx Buffer Offsets */
  CSMD_ULONG    aulTxBufBasePtr[ CSMD_HAL_TX_BASE_PTR_NBR ];
  /*! Tx Buffer Sizes */
  CSMD_ULONG    aulTxBufSize[ CSMD_HAL_TX_BASE_PTR_NBR ];

  CSMD_EVENT    arEvTimerSorted[ CSMD_TIMER_EVENT_NUMBER ];
  CSMD_ULONG    aulEvTimerFPGA[CSMD_TIMER_EVENT_NUMBER][2];
  CSMD_EVENT    arEvPortsSorted[ CSMD_TIMER_EVENT_NUMBER ];
  CSMD_ULONG    aulEvPortsFPGA[CSMD_TIMER_EVENT_NUMBER][2];

} CSMD_DBG_DATA;
#endif

/*! \endcond */ /* PUBLIC */



/*! \cond PRIVATE */
/* -------------------------------------------------------------------------- */
/*! \brief Private CoSeMa variables                                           */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_PRIV_STR
{

  CSMD_SYSTEM_LIMITS_STRUCT
                        rSystemLimits;                        /*!< System limits for maximum number of slaves, connections, configurations etc. */
  CSMD_MEM_ALLOCATION_STRUCT
                        rMemAlloc;                            /*!< Dynamic memory allocation configuration */
  CSMD_HW_INIT_STRUCT   rHW_Init_Struct;                      /*!< hardware dependent settings */

  CSMD_USHORT           ausSERC3_MDT_Control_SVC              /*!< Slave SVC control word copy from MDT for communication phase switch [slave index] */
                          [CSMD_MAX_SLAVES];
  
  CSMD_USHORT          *pusTxRam;                             /*!< Pointer to start of Tx ram for SVC emulation */
  CSMD_USHORT          *pusRxRam;                             /*!< Pointer to start of Rx ram for SVC emulation */
  
  CSMD_USHORT          *pusTxRam_MDT_RTData[CSMD_MAX_TEL];    /*!< Pointer to MDT-RTData in FPGA.TxRam */
  CSMD_USHORT          *pusTxRam_AT_RTData[CSMD_MAX_TEL];     /*!< Pointer to  AT-RTData in FPGA.TxRam */
  
  CSMD_USHORT          *pausSERC3_Tx_SeqCnt[CSMD_NBR_PORTS];  /*!< Pointer to SequenceCounter in FPGA TxRam for CP0 */
  CSMD_ULONG           *pulSERC3_Tx_MDT_SVC;                  /*!< Pointer to SVC of MDT in TxRam of FPGA in CP1/CP2 */
  CSMD_ULONG           *pulSERC3_Tx_MDT_RTD;                  /*!< Pointer to RT data of MDT in TxRam of FPGA in CP1/CP2 */
  
  /*!< Pointer to FPGA buffers with received AT RTD port 1. */
  CSMD_ULONG           *apulSERC3_RxP1_AT[ CSMD_MAX_RX_BUFFER ][ CSMD_MAX_TEL ];
  /*!< Pointer to FPGA buffers with received AT RTD port 2. */
  CSMD_ULONG           *apulSERC3_RxP2_AT[ CSMD_MAX_RX_BUFFER ][ CSMD_MAX_TEL ];
  CSMD_ULONG           *apulSERC3_NcP1_AT_Copy[CSMD_MAX_TEL_P0_2];  /*!< Pointer to copied data port 1 in NC Copy of received AT */
  CSMD_ULONG           *apulSERC3_NcP2_AT_Copy[CSMD_MAX_TEL_P0_2];  /*!< Pointer to copied data port 2 in NC Copy of received AT */
  CSMD_ULONG           *pulTxRam_CP0_MDT0;                    /*!< Pointer to CP0 MDT0 telegram data in the FPGA TxRam */
  
  CSMD_USHORT           usSERC3_Length_AT_Copy                /*!< Length of AT telegram data to be copied [Bytes] */
                          [CSMD_MAX_TEL_P0_2];
  
  CSMD_USHORT           usRequested_MTU;                      /*!< IP Maximum Transmission Unit in CP3 to CP4 [Byte] */
  CSMD_ULONG            ulUCC_Width;                          /*!< UC channel time width [ns] for CP1/CP2 and CP3/CP4 */
  CSMD_COM_VER_FIELD    rComVersion;                          /*!< Communication Version Field   (for MDT0 in CP0) */
  CSMD_USHORT           usNbrTel_CP12;                        /*!< Number of Telegrams in CP1 and CP2: 2 MDT/AT or 4 MDT/AT */
  CSMD_TIMING_CP12_STRUCT
                        rTimingCP12;                          /*!< Timing parameters for CP1/CP2 */

  CSMD_SWC_CONTROL      rSWC_Struct;                          /*!< structure for SWC related variables */
  
  CSMD_ULONG            ulTxRamTelOffset;                     /*!< Pointer to telegram Tx ram (behind Tx descr. index table) */
  CSMD_ULONG            ulRxRamTelOffset;                     /*!< Pointer to telegram Rx ram (behind Rx descr. index table) */
  
  CSMD_MDT_LENGTH       rMDT_Length[CSMD_MAX_TEL];            /*!< Lengths of MDT telegram segments (whole, HP, EF, SVC, RTD) */
  CSMD_AT_LENGTH        rAT_Length[CSMD_MAX_TEL];             /*!< Lengths of  AT telegram segments (whole, HP, SVC, RTD) */

  CSMD_ULONG            ulTxRamOffsetPRel_P1;                 /*!< TxRamOffset for Rx Buffer Basepointer: Port 1 relative buffer (write to TxRam) */
  CSMD_ULONG            ulTxRamOffsetPRel_P2;                 /*!< TxRamOffset for Rx Buffer Basepointer: Port 2 relative buffer (write to TxRam) */
  CSMD_USHORT           usTxRamPortOffset;                    /*!< Offset to first CC connection in Port relative Tx Buffer */
  CSMD_USHORT           usTxRamPRelBuffLen;                   /*!< Length of the port relative buffer
                                                                   (have to be replaced, if rCSMD_Debug.aulRxBufSize is available) */
  CSMD_BOOL             boClear_Tx_CC_Data;                   /*!< Clear CC data in TxRam. Set once when CP=CP4 and CPS=1 */
#ifdef CSMD_PCI_MASTER
  CSMD_BOOL             boDMA_IsActive;                       /*!< Copied Sercos telegrams in CP0 to CP4 with DMA access into local memory */
  
  CSMD_USHORT           ausRxDMA_Start[CSMD_MAX_TX_BUFFER];     /* Activated RX-DMA channels for transfer from Host to TxRam */
  CSMD_USHORT           ausTxDMA_Start_P1[CSMD_MAX_RX_BUFFER];  /* Activated TX-DMA channels for transfer from RxRam port 1 to Host */
  CSMD_USHORT           ausTxDMA_Start_P2[CSMD_MAX_RX_BUFFER];  /* Activated TX-DMA channels for transfer from RxRam port 2 to Host */
#endif
#ifdef CSMD_SYNC_DPLL
  CSMD_BOOL             boPLL_Mode;                           /*!< PLL for external synchronization is active */
#endif
  CSMD_EVENT_CONTROL    rCSMD_Event;                          /*!< Event control */
  
  CSMD_CYCCLK_CONTROL   rCycClk;                              /*!< CLC_CLK control */
  
  CSMD_LONG             lOutTimer;                            /*!< timeout timer for CSMD_Detect_Available_Slaves() */
  CSMD_LONG             lStableTimer;                         /*!< timer for stable slave list / stable topology */
  CSMD_USHORT           ausSeqCnt[CSMD_NBR_PORTS];            /*!< sequence counter required for remote address of each port */
  CSMD_ADDR_SCAN_INFO   rSlaveAvailable;                      /*!< for CSMD_Detect_Available_Slaves() */
  CSMD_ADDR_SCAN_INFO   rSlaveAvailable2;                     /*!< for CSMD_Detect_Available_Slaves() */

  CSMD_USHORT           ausTopologyAddresses                  /*!< topology address (use for initialization only!) [slave index] */
                          [CSMD_SLAVE_NBR_LIMIT];

  CSMD_USHORT           ausPrefPortBySlave[CSMD_MAX_SLAVES];  /*!< (preferred) port assignment of SVC and RT data by slave [slave index] */

  CSMD_FUNC_STATE       rInternalFuncState;                   /*!< state machine for internal function calls (e.g. CSMD_Detect_Available_Slaves()) */
  CSMD_BOOL             boMultipleSAddress;                   /*!< multiple equal slave addresses detected in CP0 */
  CSMD_ULONG            ulActiveCycTime;                      /*!< Active cycle time [ns] (used for communication phase switch) */
  
#ifdef CSMD_STATIC_MEM_ALLOC
  CSMD_RD_WR_BUFFER     parRdWrBuffer                         /*!< Temp. data buffer for svc macro function calls [slave index] */
                          [CSMD_MAX_SLAVES];
#else
  /* Pointer to array with dynamically allocated number of elements */
  CSMD_RD_WR_BUFFER    *parRdWrBuffer;                        /*!< Temp. data buffer for svc macro function calls [slave index] */
#endif
  CSMD_RING_DELAY       rRingDelay;                           /*!< Structure for ring delay measuring */
  
  CSMD_ULONG            ulOffsetTNCT_SERCCycle;               /*!< Offset between start sending MDT0 and start Sercos cycle [ns] */
  CSMD_USHORT           usMaxSlaveJitter;                     /*!< maximum jitter of a slave [ns] */
  CSMD_ULONG            ulInterFrameGap;                      /*!< InterFrameGap for Tx frame transmission [Bytes] */
  
  CSMD_MONITORING_MODE  eMonitoringMode;                      /*!< monitoring mode: off; copy only; full monitoring */
  CSMD_BOOL             boP1_active;                          /*!< flag port 1 active */
  CSMD_BOOL             boP2_active;                          /*!< flag port 2 active */
  
  CSMD_BOOL             aboMDT_used[CSMD_MAX_TEL];            /*!< flag MDT telegram is used */
  CSMD_BOOL             aboAT_used[CSMD_MAX_TEL];             /*!< flag AT telegram is used */
  CSMD_USHORT           usMDT_Enable;                         /*!< Enable MDT telegram number corresponding to bit number */
  CSMD_USHORT           usAT_Enable;                          /*!< Enable AT  telegram number corresponding to bit number */
#ifdef CSMD_STATIC_MEM_ALLOC
  CSMD_CONN_MASTERPROD  parConnMasterProd                     /*!< internal structure for master-produced connections */
                          [CSMD_MAX_CONNECTIONS_MASTER];
  CSMD_CONN_SLAVEPROD   parConnSlaveProd                      /*!< internal structure for slave-produced connections */
                          [CSMD_MAX_CONNECTIONS_MASTER];
#else
  /* Pointer to arrays with dynamically allocated number of elements */
  CSMD_CONN_MASTERPROD *parConnMasterProd;                    /*!< internal structure for master-produced connections */
  CSMD_CONN_SLAVEPROD  *parConnSlaveProd;                     /*!< internal structure for slave-produced connections */
#endif
  CSMD_CC_CONN_STRUCT   rCC_Connections;                      /*!< private structure for CC connection info */

  CSMD_USHORT           ausLast_S_Dev_Idx[CSMD_MAX_TEL];      /*!< last slave index in telegram [TelNbr] */
  CSMD_USHORT           usNumProdCycTimes;                    /*!< number of different producer cycle times configured */
  CSMD_ULONG            aulProdCycTimes[CSMD_MAX_CYC_TIMES];  /*!< array with different producer cycle times configured */
  CSMD_USHORT           ausTSrefList[CSMD_MAX_TSREF];         /*!< bit list for connection production time related to TSref counter [TSref] */

  CSMD_SVC_INTERNAL_STRUCT
                        arSVCInternalStruct[CSMD_MAX_SLAVES]; /*!< list for internal SVC access in Rx Ram [slave index] */
  
  /*!< Buffer for handling of Device Control word [slave index]       */
  CSMD_USHORT           ausDevControl[CSMD_MAX_SLAVES];
  /*!< Pointer to Device control [TxBuffer Nbr (0...2)] [slave index] */
  CSMD_USHORT          *pausC_Dev[ CSMD_MAX_TX_BUFFER ][ CSMD_MAX_SLAVES ];

  /*!< pointer to device status in RxRam, dependent on slave index, master port and RX buffer */
  CSMD_USHORT          *apusS_DEV[CSMD_MAX_SLAVES][CSMD_NBR_PORTS][CSMD_MAX_RX_BUFFER];

  CSMD_USHORT           usTxBuffer;                           /*!< Copy of active Tx Ram buffer number */

  CSMD_REDUNDANCY_AUX   rRedundancy;                          /*!< structure containing auxiliary variables for redundancy feature */

// is no longer used      CSMD_USHORT  usMST_ErrorCounter1;     /*!< MST error counter 1 */
// is no longer used      CSMD_USHORT  usAT_ErrorCounter1;      /*!< AT error counter_1 */

  CSMD_USHORT           usTelErrCnt;                          /*!< Counter for successive telegram errors */
  
  CSMD_SERC3SVC        *prSVContainer[CSMD_MAX_SLAVES];       /*!< Pointer to Service Container [slave index] */
  
  /* Software container Declarations Start */
#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
  #ifdef CSMD_STATIC_MEM_ALLOC
  CSMD_SERC3SVC         parSoftSvcContainer[CSMD_MAX_SLAVES - CSMD_MAX_HW_CONTAINER];   /*!< Soft Service Container [slave index - NbrHWCont]             */
  CSMD_SOFT_SVC_STRUCT  parSoftSvc[CSMD_MAX_SLAVES - CSMD_MAX_HW_CONTAINER];            /*!< Structure containing variables for soft SVC feature [slave index] */
  #else
  /* Pointer to arrays with dynamically allocated number of elements */
  CSMD_SERC3SVC        *parSoftSvcContainer;                  /*!< Soft Service Container [slave index - NbrHWCont]             */
  CSMD_SOFT_SVC_STRUCT *parSoftSvc;                           /*!< Structure containing variables for soft SVC feature [slave index] */
  #endif
#endif
  CSMD_USHORT           usSVC_HS_Timeout;                     /*!< SVC handshake timeout [Sercos cycles] */
  CSMD_USHORT           usSVC_BUSY_Timeout;                   /*!< SVC busy timeout [Sercos cycles] */
  CSMD_BOOL             boHW_SVC_Redundancy;                  /*!< Support IP-Core service channel redundancy */

  /* The lower bits in this array are not used for soft svc respectively reserved for hard svc */
  CSMD_ULONG            aulSVC_Int_Flags                      /*!< Interrupt flags for soft svc interrupts */
                          [((CSMD_MAX_SLAVES-1)/32)+1];

  CSMD_VOID            *pvCB_Info;                            /*!< Instance Info of IP Driver */
  CSMD_CB_FUNCTIONS     rCbFuncTable;                         /*!< Table with callback functions to IP driver */
  CSMD_SIII_EVENT       rSercEvent;                           /*!< Sercos Event information */

  
  CSMD_RUNTIME_INFO     rRuntimeWarning;                      /*!< Debug variable used for warnings */
  CSMD_RUNTIME_INFO     rRuntimeError;                        /*!< Debug variable used for errors   */
  
  CSMD_SLAVE_ACTIVITY_STATUS
                        eRequiredState;                       /*!< Compare value for aeSlaveActive[] used in CSMD_Proceed_Cmd_S_0_1024().
                                                                   Set to CSMD_SLAVE_ACTIVE in CP2 or for ring recovery.
                                                                   Set to CSMD_SLAVE_HP_IN_PROCESS during Hot-plug. */
  CSMD_BOOL             boSCP_Checked;                        /*!< Flag 'SCP configuration has been checked in CSMD_CheckVersion()' */
  CSMD_ULONG            aulSCP_Config[CSMD_MAX_SLAVES];       /*!< SCP configuration bitlist [slave index] */
  CSMD_USHORT           ausActConnection[CSMD_MAX_SLAVES];    /*!< Position in connection config list [slave index] */
#if defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS
  CSMD_USHORT           ausActParam[CSMD_MAX_SLAVES];         /*!< Position in config parameter list [slave index] */
#endif
  CSMD_USHORT           usTimingMethod;                       /*!< Timing method , MDT|AT|IP or MDT|IP|AT */
  CSMD_BOOL             boSoftMaster;                         /*!< Sercos soft master is used instead of hard master IP-Core. */
  
  CSMD_USHORT           usTimerHP_CP3;                        /*!< Timer for HP transmission of t6/t7 in CP3 */
  CSMD_HP_STRUCT        rHP_P1_Struct;                        /*!< HP   data structure for port 1 */
  CSMD_HP_STRUCT        rHP_P2_Struct;                        /*!< HP   data structure for port 2 */
#ifdef CSMD_HOTPLUG
  CSMD_HOT_PLUG_AUX     rHotPlug;                             /*!< Structure containing variables for the Hot-Plug feature */
#endif
#if (defined CSMD_CONFIG_PARSER || defined CSMD_CONFIGURATION_PARAMETERS) && defined CSMD_BIG_ENDIAN
  CSMD_UCHAR            aucAuxConfParam                       /* converted parameter data array */
                          [CSMD_MAX_SLAVES][CSMD_NBR_PARAM_DATA];
#endif
#ifdef CSMD_CONFIG_PARSER
  #ifdef CSMD_STATIC_MEM_ALLOC
  CSMD_SLAVE_INST_MANIPULATED
                        parSlaveInst[CSMD_MAX_SLAVES];        /* todo description */
  CSMD_UCHAR            paucSlaveSetupManipulated
                          [CSMD_MAX_SLAVE_CONFIGPARAMS];      /* todo description */
  #else
  /* Pointer to arrays with dynamically allocated number of elements */
  CSMD_SLAVE_INST_MANIPULATED
                       *parSlaveInst;                         /* todo description */
  CSMD_UCHAR           *paucSlaveSetupManipulated;            /* todo description */
  #endif
  CSMD_USED_MARKER      rUsedCfgs;                            /* todo description */
#endif
  
} CSMD_PRIV;

/*! \endcond */ /* PRIVATE */


/*! \cond PUBLIC */
/* -------------------------------------------------------------------------- */
/*! \brief CoSeMa instance data structure                                     */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_INSTANCE_STR
{
  /*-----------------------------------------------------------------------*/
  /* Public Variables                                                      */
  /*-----------------------------------------------------------------------*/
  
  /*!< hardware dependent settings */
  CSMD_HW_INIT_STRUCT     rHW_Settings;

  CSMD_SLAVE_LIST         rSlaveList;                         /*!< Slave configuration lists */

  /*! Sercos V1.3 configuration */
  CSMD_CONFIG_STRUCT      rConfiguration;
  
  /*! List with slaves which report a problem for functions with multiple slave access */
  CSMD_EXTENDED_DIAG      rExtendedDiag;
  
  /*! Connection configuration error info */
  CSMD_CONFIG_ERROR       rConfig_Error;
  
  /*! Device status S-Dev */
  CSMD_S_DEV_STRUCT       arDevStatus[CSMD_MAX_SLAVES];       /*!< Device status structure [slave index] */
  
  /* is no longer used (20.01.2015)  CSMD_USHORT              usAT_ErrorCounter2; */                /*!< AT error counter_2 (current count) */
  
  CSMD_SHORT              sCSMD_Phase;                        /*!< Global variable to reflect actual/current communication phase (Sercos phase) */
  
  CSMD_USHORT             usRequired_Topology;                /*!< Required Sercos topology (default is 0 --> no specific topology required) */
  CSMD_USHORT             usCSMD_Topology;                    /*!< Current Sercos topology */
  CSMD_USHORT             usSercAddrLastSlaveP1;              /*!< Sercos address of slave at the end of line on port 1 */
  CSMD_USHORT             usSercAddrLastSlaveP2;              /*!< Sercos address of slave at the end of line on port 2 */
  CSMD_BOOL               boRingRedundant;                    /*!< Topology is ring: FALSE --> redundancy feature is unavailable due to a single wire break */
  
  CSMD_TEL_BUFFER        *prTelBuffer;                        /*!< Pointer to local RAM for Sercos telegrams */
#ifdef CSMD_PCI_MASTER
  CSMD_TEL_BUFFER        *prTelBuffer_Phys;                   /*!< Pointer to local RAM for Sercos telegrams (physical address) */
  CSMD_USHORT             usTX_DMA_Datalength;                /*!< Required size of MDT data buffer for DMA (CP3 & CP4) */
  CSMD_USHORT             usRX_DMA_Datalength;                /*!< Required size of  AT data buffer for DMA (CP3 & CP4) */
#endif
  
  CSMD_BOOL               boIFG_V1_3;                         /*!< All slaves with SCP_Sync supports S-0-1036 */
  /*! Limit for minimum event time [ns] */
  CSMD_ULONG              ulMinTime;
  /*! Limit for maximum event time [ns] */
  CSMD_ULONG              ulMaxTime;
  
  /*! ----- Global Variable Related to SVCH ----- */
#ifdef CSMD_STATIC_MEM_ALLOC
  CSMD_SVCH_MNGMT_STRUCT  parSvchMngmtData[CSMD_MAX_SLAVES];  /*!< Service channel management structure [slave index] */
#else
  /* Pointer to array with dynamically allocated number of elements */
  CSMD_SVCH_MNGMT_STRUCT *parSvchMngmtData;                   /*!< Service channel management structure [slave index] */
#endif
  CSMD_USHORT             usSoftSrvcCnt;                      /*!< If not zero, CSMD_TxRxSoftCont() should be called for handling soft svc */
  
  CSMD_USHORT             usEnableTel;                        /*!< Ready to enable CYC_CLK in timing slave mode  */
  CSMD_USHORT             usWD_Mode;                          /*!< Mode for the behavior of the IP-Core watchdog */
  
  /*-----------------------------------------------------------------------*/
  /* Private variables                                                     */
  /*-----------------------------------------------------------------------*/
  
  CSMD_PRIV               rPriv;                              /*!< Private variables */
  
  CSMD_HAL                rCSMD_HAL;                          /*!< HAL variables */
  
#ifdef CSMD_DEBUG
  CSMD_DBG_DATA           rCSMD_Debug;                        /*!< Data only for debug purpose */
#endif
#if defined CSMD_SYNC_DPLL && defined CSMD_DEBUG_DPLL
  CSMD_PLL_DBG_INFO       rCSMD_Dbg_PLL;                      /*!< Data only for debug purposes of DPLL */
#endif
  
} CSMD_INSTANCE;


/* #define CSMD_DEBUG_CONN_STM */

#ifdef CSMD_DEBUG_CONN_STM  /* only for debugging !!! */
/* CSMD_CyclicHandling() / CSMD_EvaluateConnections() */
SOURCE  CSMD_USHORT     usZaehler
#ifdef SOURCE_CSMD
= 0
#endif
;
#define CSMD_DBG_NBR_CYCLES   10000
/* CSMD_EvaluateConnections() */
SOURCE CSMD_USHORT      usConnSelect;
typedef struct CSMD_CON_DEBUG_STR
{
  CSMD_CONS_STATE eEntry_State;
  CSMD_CONS_STATE eState;    /* Connection state             */
  CSMD_USHORT     usSoll;    /* Expected C-CON               */
  CSMD_USHORT     usIst;     /* C-CON of consumed connection */
} CSMD_CON_DEBUG;
SOURCE CSMD_CON_DEBUG rConDebug[CSMD_DBG_NBR_CYCLES];

/* CSMD_CyclicHandling() */
SOURCE CSMD_FUNC_RET  aeReturn[CSMD_DBG_NBR_CYCLES];
#endif  /* #ifdef CSMD_DEBUG_CONN_STM */








/*---- Declaration public Functions: -----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

SOURCE CSMD_FUNC_RET CSMD_Initialize
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_INIT_POINTER         *prSERCOS );

SOURCE CSMD_FUNC_RET CSMD_InitSystemLimits
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_MEM_ALLOC_CB_STRUCT  *prMemAllocCB_Table,
                                  CSMD_SYSTEM_LIMITS_STRUCT *prSysLimits,
                                  CSMD_LONG                 *plBytes );

SOURCE CSMD_FUNC_RET CSMD_InitMemory
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_LONG                 *plBytes );

SOURCE CSMD_FUNC_RET CSMD_InitHardware
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState );

SOURCE CSMD_FUNC_RET CSMD_ResetHardware
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE  CSMD_FUNC_RET CSMD_Init_Config_Struct
                                ( CSMD_INSTANCE             *prCSMD_Instance );

#ifdef CSMD_SOFT_MASTER
SOURCE CSMD_FUNC_RET CSMD_UseSoftMaster
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boSoftMaster );
#endif
SOURCE  CSMD_VOID CSMD_Version  ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_VERSION              *prCSMD_Version );

SOURCE CSMD_FUNC_RET CSMD_Set_NRT_State
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_FUNC_RET CSMD_Control_UC
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boEnablePort1,
                                  CSMD_BOOL                  boEnablePort2 );

SOURCE CSMD_FUNC_RET CSMD_SetPhase0 
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_USHORT              **pusRecognizedSlaveList,
                                  CSMD_COM_VERSION           eComVersion );

SOURCE CSMD_FUNC_RET CSMD_SetPhase1 
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_USHORT               *pusProjectedSlaveList );

SOURCE CSMD_FUNC_RET CSMD_SetPhase2 
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState );

SOURCE CSMD_FUNC_RET CSMD_SetPhase3 
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_FUNC_RET CSMD_SetPhase4 
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_FUNC_RET CSMD_GetPhase
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT               *pusPhase );

/* Check all slaves  */
SOURCE CSMD_FUNC_RET  CSMD_CheckVersion
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_FUNC_RET CSMD_CyclicHandling
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_FUNC_RET CSMD_SetConnectionState
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usConnIdx,
                                  CSMD_PROD_STATE            eComState );

SOURCE CSMD_FUNC_RET CSMD_GetConnectionState
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usConnIdx,
                                  CSMD_USHORT               *pusState );

SOURCE CSMD_FUNC_RET CSMD_SetConnectionData
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usConnIdx,
                                  CSMD_USHORT               *pusConnData,
                                  CSMD_USHORT                usRTBits );

SOURCE CSMD_FUNC_RET CSMD_GetConnectionData
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usConnIdx,
                                  CSMD_USHORT               *pusDestination );

SOURCE CSMD_FUNC_RET CSMD_GetConnectionDataDelay
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usConnIdx,
                                  CSMD_USHORT               *pusCycles );

SOURCE CSMD_FUNC_RET CSMD_ClearConnectionError
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usConnIdx );


/* --- CSMD - Timing Functions --- */
SOURCE CSMD_FUNC_RET CSMD_GetTimingData
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_FUNC_RET CSMD_CalculateTiming
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usTimingMethod );

#ifdef CSMD_STABLE_TSREF  /* Ring delay calculation corresponding to Sercos III Spec. 1.3.2 */
SOURCE CSMD_FUNC_RET CSMD_Check_TSref
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_LONG                 *plTSref,
                                  CSMD_LONG                 *plTNetworkMax );
#endif

SOURCE CSMD_FUNC_RET CSMD_TransmitTiming
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

SOURCE CSMD_VOID CSMD_GetCommCounter
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usPortNbr,
                                  CSMD_COMM_COUNTER         *prErrCounter );

SOURCE CSMD_VOID CSMD_ResetSercosErrorCounter
                                ( CSMD_INSTANCE             *prCSMD_Instance );


/* --- CSMD - Macro Functions --- */
SOURCE CSMD_FUNC_RET CSMD_ReadSVCH
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT    *prSvchData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_WriteSVCH
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT    *prSvchData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_SetCommand
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT    *prSvchData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_ClearCommand
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT    *prSvchData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_ReadCmdStatus
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MACRO_STRUCT    *prSvchData, 
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );


/* --- CSMD - Atomic Functions --- */
SOURCE CSMD_FUNC_RET CSMD_OpenIDN
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT    *prSvchMngmtData, 
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_GetData
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT    *prSvchMngmtData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_GetDataStatus
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT    *prSvchMngmtData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_GetName
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT    *prSvchMngmtData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_GetAttribute
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT    *prSvchMngmtData, 
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_GetUnit
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT    *prSvchMngmtData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_GetMin( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT    *prSvchMngmtData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_GetMax( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT    *prSvchMngmtData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_GetListLength
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT    *prSvchMngmtData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_PutData
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SVCH_MNGMT_STRUCT    *prSvchMngmtData,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );

SOURCE CSMD_FUNC_RET CSMD_ResetSVCH
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSlaveIdx,
                                  CSMD_VOID                 (*CallBackFunc)(CSMD_VOID) );



SOURCE CSMD_VOID CSMD_Usable_WriteBufferSysA
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT               *pusBuffer );

SOURCE CSMD_VOID CSMD_Usable_ReadBufferSysA
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT               *pusBufferP1,
                                  CSMD_USHORT               *pusBufferP2 );

SOURCE CSMD_VOID CSMD_Get_IsValid_Rx_BufferSysA
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                 *pboNewValidBufP1,
                                  CSMD_BOOL                 *pboNewValidBufP2 );

#if CSMD_MAX_SLAVES > CSMD_MAX_HW_CONTAINER
SOURCE CSMD_FUNC_RET CSMD_TxRxSoftCont
                                ( CSMD_INSTANCE             *prCSMD_Instance );
#endif

SOURCE CSMD_FUNC_RET CSMD_IntControl
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_ULONG                 ulIntEnable,
                                  CSMD_ULONG                 ulIntOutputMask );

SOURCE CSMD_FUNC_RET CSMD_ClearSvchInt
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_ULONG                *paulIntClearMask );

SOURCE CSMD_FUNC_RET CSMD_CheckSvchInt
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_ULONG                *paulSvcIntStatus );

SOURCE CSMD_FUNC_RET CSMD_CheckInt
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_ULONG                *pulInt );

SOURCE CSMD_FUNC_RET CSMD_ClearInt
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_ULONG                 ulIntClearMask );

#ifdef CSMD_PCI_MASTER
SOURCE CSMD_VOID CSMD_DMA_Write_Tx_Ram
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_DMA_Read_Rx_Ram
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_BOOL CSMD_IsBusy_DMA_Transfer
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSelDirection );
#endif

SOURCE CSMD_FUNC_RET CSMD_PrepareCYCCLK 
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boActivate,
                                  CSMD_BOOL                  boEnableInput,
                                  CSMD_BOOL                  boPolarity,
                                  CSMD_ULONG                 ulStartDelay );

SOURCE CSMD_FUNC_RET CSMD_PrepareCYCCLK_2
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boActivate,
                                  CSMD_BOOL                  boEnableInput,
                                  CSMD_BOOL                  boPolarity,
                                  CSMD_ULONG                 ulDelaySercosCycle );

SOURCE CSMD_VOID CSMD_Enable_CYCCLK_Input 
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boEnable );

SOURCE CSMD_VOID CSMD_Retrigger_System_Timer
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_BOOL CSMD_IsRunning_System_Timer
                                ( CSMD_INSTANCE             *prCSMD_Instance );

#ifdef CSMD_SYNC_DPLL
SOURCE CSMD_PLL_STATE CSMD_PLL_Control
                                ( CSMD_INSTANCE             *prCSMD_Instance, 
                                  CSMD_BOOL                  boRunPLL );
#endif

SOURCE CSMD_FUNC_RET CSMD_New_Sercos_Time
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SERCOSTIME            rSercosTime );

SOURCE CSMD_FUNC_RET CSMD_New_Sercos_Time_ExtSync
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_SERCOSTIME            rSercosTime );

SOURCE CSMD_FUNC_RET CSMD_Get_Sercos_Time
                                ( CSMD_INSTANCE             *prCSMD_Instance, 
                                  CSMD_SERCOSTIME           *prSercosTime );

#ifdef CSMD_HW_WATCHDOG
SOURCE CSMD_VOID CSMD_Watchdog_Trigger
                                ( CSMD_INSTANCE             *prCSMD_Instance );

SOURCE CSMD_VOID CSMD_Watchdog_Control
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boControl );

SOURCE CSMD_VOID CSMD_Watchdog_Configure
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usCycles );

SOURCE CSMD_VOID CSMD_Watchdog_Status
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_WDSTATUS             *prWdStatus );
#endif

SOURCE CSMD_VOID CSMD_SetCONCLK ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boActivate,
                                  CSMD_BOOL                  boEnableDriver,
                                  CSMD_BOOL                  boPolarity );

SOURCE CSMD_FUNC_RET CSMD_ConfigDIVCLK 
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_BOOL                  boActivate,
                                  CSMD_BOOL                  boMode,
                                  CSMD_BOOL                  boPolarity,
                                  CSMD_BOOL                  boOutpDisable,
                                  CSMD_USHORT                usNbrPulses,
                                  CSMD_ULONG                 ulPulseDistance,
                                  CSMD_ULONG                 ulStartDelay );

SOURCE CSMD_FUNC_RET CSMD_EventControl
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usEventID,
                                  CSMD_BOOL                  boActivate,
                                  CSMD_ULONG                 ulEvent_TCNT_Value );

SOURCE CSMD_FUNC_RET CSMD_GetEventTime
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_ULONG                *pulTime,
                                  CSMD_USHORT                usEventID );

SOURCE CSMD_VOID CSMD_Get_TCNT  ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_ULONG                *pulSysTimerReadbackValue );

SOURCE CSMD_VOID CSMD_Get_TCNT_Relative
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_ULONG                *pulCurrentTimeInCycle );

SOURCE CSMD_FUNC_RET CSMD_Get_TSref
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_ULONG                *pulTSref );

SOURCE CSMD_FUNC_RET CSMD_GetTopology
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT               *pusDev_P1,
                                  CSMD_USHORT               *pusDev_P2 );

SOURCE CSMD_FUNC_RET CSMD_IdentifySlave
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSlaveIdx,
                                  CSMD_BOOL                  boActivate );

SOURCE CSMD_USHORT CSMD_Read_C_Dev
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSlaveIdx );


#ifdef CSMD_HOTPLUG
/*  */
SOURCE CSMD_FUNC_RET CSMD_HotPlug 
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_USHORT               *pusHPDevAddList,
                                  CSMD_BOOL                  boCancel );

/*  */
SOURCE CSMD_FUNC_RET CSMD_TransHP2Para
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro,
                                  CSMD_BOOL                  boCancel );
#endif  /* End: #ifdef CSMD_HOTPLUG */

/*  */
SOURCE CSMD_SHORT CSMD_ActiveSlavePort
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_USHORT                usSlaveIdx );

/*  */
SOURCE CSMD_FUNC_RET CSMD_RecoverRingTopology
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_SVCH_MACRO_STRUCT    *parSvcMacro );

/*  */
SOURCE CSMD_FUNC_RET CSMD_OpenRing
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_FUNC_STATE           *prFuncState,
                                  CSMD_USHORT                usDevAddr1,
                                  CSMD_USHORT                usDevAddr2 );

#ifdef CSMD_CONFIG_PARSER
/*  */
SOURCE CSMD_FUNC_RET CSMD_GenerateBinConfig
                                ( CSMD_INSTANCE             *prCSMD_Instance, 
                                  const CSMD_USHORT          usS3binVersion,
                                  const CSMD_USHORT          usAppID,
                                  const CSMD_BOOL            boAppID_Pos,
                                  CSMD_VOID                 *pvTargetBinConfig );
/*  */
SOURCE CSMD_FUNC_RET CSMD_ProcessBinConfig 
                                ( CSMD_INSTANCE             *prCSMD_Instance,
                                  CSMD_VOID                 *pvSourceBinConfig,
                                  const CSMD_BOOL            boConnNumGen,
                                  const CSMD_BOOL            boSlaveInstGen );
/*  */
SOURCE CSMD_FUNC_RET CSMD_TransferConnConfigs
                                ( CSMD_INSTANCE                   *prCSMD_Instance,
                                  const CSMD_MASTER_CONFIGURATION *prTempMasterCfg,
                                  const CSMD_SLAVE_CONFIGURATION  *prTempSlaveCfg );
/*  */
SOURCE CSMD_FUNC_RET CSMD_InitTempConnConfigs
                                ( CSMD_INSTANCE             *prCSMD_Instance );
#endif  /* #ifdef CSMD_CONFIG_PARSER */

#ifdef CSMD_SERCOS_MON_CONFIG
SOURCE CSMD_FUNC_RET CSMD_Serc_Mon_Config
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  const CSMD_ULONG           ulMaxBufferLength,
                                  CSMD_CHAR                 *pcText,
                                  CSMD_ULONG                *pulLength );
#endif  /* #ifdef CSMD_SERCOS_MON_CONFIG */


#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PUBLIC */


#endif /* _CSMD_GLOB */




/*
--------------------------------------------------------------------------------
  Modification history
--------------------------------------------------------------------------------

01 Sep 2010
  - Refactoring of CoSeMa files.
05 Nov 2013 WK
  Defdb00165297
  Fixed alignment of elements in structure CSMD_HP_FIELD_AT0 and
  CSMD_HP_FIELD_MDT0 for big endian systems.
02 Dec 2013 WK
  - Defdb00165150
    Added elements in structure CSMD_PRIV:
      USHORT          usTxRamPRelBuffLen
      BOOL            boClear_Tx_CC_Data
  - Defdb00150926
    Fixed Lint message 740 "Unusual pointer cast (incompatible indirect types)"
    for CSMD_MAX_HW_CONTAINER > 0.
05 Dec 2013 WK
  - Remove UCC width from prototype CSMD_CalculateTiming().
09 Dec 2013 WK
  - Defdb00165521
    Fixed compiler problem for not defined CSMD_HOTLPUG:
      Include CSMD_PRIV.rHP_P2_Struct[] independent from CSMD_REDUNDANCY.
09 Jan 2014 WK
  - Defdb00165281
    Added element in structure CSMD_FUNC_RET:
      CSMD_HP_SWITCH_TO_CP4_FAILED
    Added define CSMD_WAIT_4MS.
  - Defdb00150926
    Fixed lint warning 537 Repeated include file 'CSMD_USER.h'
23 Jan 2014 WK
    Renamed elements in structure CSMD_CONFIG_STRUCT:
      prMasterConfig --> prMaster_Config
      parSlaveConfig --> parSlave_Config
    Replace CSMD_DEV_IDX_LIMIT with CSMD_MAX_SLAVES
05 Feb 2014 WK
  WP-00044851 - Miscellaneous code clean-up for CoSeMa 6.1
  - Remove svc dummy structures for CSMD_MAX_SLAVES <= CSMD_MAX_HW_CONTAINER.
  - Added elements to enumeration  CSMD_FUNC_RET_EN:
      CSMD_INVALID_SYSTEM_LIMITS
      CSMD_MEMORY_ALLOCATION_FAILED
04 Mar 2014 WK
  Defdb00155249
  Added define CSMD_S_0_1050_SE1_RESERVED_TYPE.
25 Mar 2014 WK
  - Added prototype of function CSMD_Serc_Mon_Config().
02 Apr 2014 WK
  - Defdb00168915
    rPriv.aulSVC_Int_Flags[]
    Fixed array size of aulSVC_Int_Flags[].
10 Apr 2014 WK
  Added element in structure CSMD_FUNC_RET:
    CSMD_NO_PROCEDURE_CMD
15 Apr 2014 AlM
  Replaced element in structure CSMD_FUNC_RET:
  CSMD_DOUBLE_AT_FAIL  with  CSMD_TEL_ERROR_OVERRUN.
25 Jun 2014 AlM
  - Added prototype of function CSMD_Control_UC().
05 Aug 2014 AlM
  - Removed element in structure CSMD_INSTANCE:
      CSMD_OFFSET_LIST    rOffsetList;
20 Aug 2014 WK
  - Removed element in structure CSMD_SYSTEM_LIMITS_STRUCT:
      USHORT  usMaxProdConnAtMaster;
26 Aug 2014 WK
  - FEAT-00051252 - Generation of configuration files for the Sercos Monitor
    Include shifted mask definitions for SCP configuration bitlist.
  - Revised doxygen documentation for elements in CSMD_FUNC_RET.
    Added complete error codes.
03 Sep 2014 WK
  WP-00044851 - Miscellaneous code clean-up for CoSeMa 6.1
    Shifted enumerations CSMD_FUC_RET and to CSMD_SVC_ERROR_CODES to
    the new file CSMD_ERR_CODES.h.
23 Oct 2014 WK
  - Defdb00174315
    added new function CSMD_New_Sercos_Time_ExtSync().
09 Oct 2014 WK
  Defdb00174053
  - Added define for CSMD_FUNC_STATE.usActState
      CSMD_FUNCTION_CP4_TRANS_CHECK
  - Added define CSMD_TSCYC_500_US.
  - Added define CSMD_HP0_PAR_SEND_CYCLES
  - Removed defines CSMD_HP_ERR_PAR_RECEPTION and CSMD_HP_ACKN_NEXT_SAME_SADD.
  - Renamed structure from CSMD_TINING_CP12_... to CSMD_TIMING_CP12_...
15 Oct 2014 WK
  - Defdb00174053
    Added define CSMD_HP_ADD_SLAVE_IDX_SHIFT.
23 Oct 2014 WK
  - Defdb00173986
    Added define CSMD_PORT_CHANGE_WAIT_CYCLES.
30 Oct 2014 WK
  - Renamed element in structure CSMD_INSTANCE:
      CSMD_RING_DELAY    rCSMD_RingDelay      --> rRingDelay
      CSMD_SERC3SVC      parCSMD_SC_S3        --> parSoftSvcContainer
      CSMD_RUNTIME_INFO  rCSMD_RuntimeWarning --> rRuntimeWarning
      CSMD_RUNTIME_INFO  rCSMD_RuntimeError   --> rRuntimeError
12 Nov 2014 WK
  - Reduce value of CSMD_HP0_PAR_SEND_CYCLES from 100 to 10.
13 Nov 2014 WK
  - Defdb00000000
    Removed test code for defined CSMD_DEBUG.
19 Nov 2014 WK
  - Defdb00174985
    Added prototype of the new function CSMD_PrepareCYCCLK_2().
16 Jan 2015 WK
  - Defdb00175997
    Added prototype of the new function CSMD_Get_TSref().
  - Renamed structure rCSMD_CycClk to rCycClk.
26 Jan 2015 WK
  - Convert simple HTML tables into "markdown" extra syntax to make it
    more readable in the source code.
04 Feb 2015 WK
  - Removed  CSMD_MASTER_CONFIGURATION.ulJitter_Start_AT.
11 Feb 2015 WK
  - Defdb00176768
    Added element in structure CSMD_PRIV: BOOL  boSoftMaster.
    Added prototype of the new access function CSMD_UseSoftMaster().
24 Feb 2015 WK
  - Defdb00177060
    Added define CSMD_C_CON_LENGTH.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
27 Feb 2015 WK
  - Renamed element in structure CSMD_USED_MARKER:
      CSMD_UCHAR *paucCSMD_ConnNbrUsed          --> paucConnNbrUsed
      CSMD_UCHAR *paucCSMD_ConnUsed             --> paucConnUsed
      CSMD_UCHAR *paucCSMD_ConfUsed             --> paucConfUsed
      CSMD_UCHAR *paucCSMD_RTBtUsed             --> paucRTBtUsed
      CSMD_UCHAR *paucCSMD_SetupParamsListUsed  --> paucSetupParamsListUsed
      CSMD_UCHAR *paucCSMD_SetupParamsUsed      --> paucSetupParamsUsed
03 Mar 2015 WK
  - Defdb00177704
    Change meaning of CSMD_CALC_RD_MODE_HOTPLUG_ACTIVE.
    Added element in structure CSMD_PRIV:
      CSMD_SLAVE_ACTIVITY_STATUS eRequiredState;
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
01 Sep 2015 WK
  - Defdb00181092
    Correct value of #define CSMD_S_0_1050_SE1_ASYNC_WD.
05 Nov 2015 WK
  - Defdb00182857
    Added element in structure CSMD_HW_INIT_STRUCT:
      CSMD_BOOL  boHP_Field_All_Tel;
  - Defdb00182757
    Header CSMD_TYPE_HEADER is integrated.
08 Mar 2016 WK
  - Defdb00185518
    Encapsulation of prototype CSMD_UseSoftMaster() with CSMD_SOFT_MASTER.
04 Apr 2016 AlM
  - Defdb00184988
    Added element aulATBufValid[CSMD_NBR_PORTS] to struct type CSMD_REDUNDANCY_AUX
31 May 2016 WK
  - Defdb00187475
    Added element in structure CSMD_PRIV: BOOL  boHP0_CheckLink.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
14 Jul 2016 WK
 - FEAT-00051878 - Support for Fast Startup
   Added define CSMD_SLAVE_SETUP_UNCHANGED.

--------------------------------------------------------------------------------
*/
