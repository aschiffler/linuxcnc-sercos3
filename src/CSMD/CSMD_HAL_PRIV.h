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
\file   CSMD_HAL_PRIV.h
\author KP
\date   03.09.2007
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_HAL.c
\details \b Description: \n
        This header file is included in CSMD_GLOB.h and therefore valid in whole 
        CoSeMa files. 
        The developer should note followings:
        - Supports Sercos master IP-Core SERCON100M Version 4.
        - RAM-addresses for SVC, TX and RX must defined 
        - SVC-, TX- and RX-RAM structures must be retained
        - pointer to SVC-, TX- and RX-RAM in CSMD_HAL structure must be retained
        - reserve sufficient memory for SVC-, TX- and RX- data \n
          minimum:                \n
                SVC -> 4K   \n
                TX  -> 8K   \n
                RX  -> 16K  
 
<B>Target system:</B>
      - SH4A / SH2A / MicroBlaze

\note   IP-Functions are not in use. IP-functionality is disabled because some
        functions access to CSMD_Instance variables.\n That variables are
        defined in CSMD_GLOB.h but in CSMD_HAL1.c is only  CSMD_HAL1.h included.
*/

/*---- Includes: -------------------------------------------------------------*/

#ifndef _CSMD_HAL_PRIV
#define _CSMD_HAL_PRIV

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*lint -save --e{717} do ... while(0)  specifically authorized for each function macro */

/*! \cond HAL */

#ifdef CSMD_BIG_ENDIAN
#include <machine.h>
#endif

/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/* ------------------------------------------------------------------------- */
/* Bit assignment                                                            */
/*                                                                           */
/* Big Endian:     31     24 23     16 15      8 7       0                   */
/*                 +-------+ +-------+ +-------+ +-------+                   */
/*                 |       | |       | |       | |       |                   */
/*                 +-------+ +-------+ +-------+ +-------+                   */
/* Little Endian:  7       0 15      8 23     16 31     24                   */
/*                                                                           */
/* ------------------------------------------------------------------------- */

#ifdef CSMD_BIG_ENDIAN
#define  CSMD_REVERSE_CSMD_USHORT(_x)   ((CSMD_USHORT)swapb(_x))    /* Reverses the byte order inside 2-byte data.  */
#define  CSMD_SWAP_WORD(_x)             ((CSMD_ULONG) swapw(_x))    /* Upper/lower unsigned word swap.              */
#define  CSMD_REVERSE_CSMD_ULONG(_x)    ((CSMD_ULONG) end_cnvl(_x)) /* Reverses the byte order inside 4-byte data.  */
#endif

#ifdef CSMD_BIG_ENDIAN

#define  CSMD_END_CONV_S(_x)      CSMD_REVERSE_CSMD_USHORT(_x)  /* Reverses the byte order inside 2-byte data.              */
#define  CSMD_END_CONV_W(_x)      CSMD_SWAP_WORD(_x)            /* Upper/lower unsigned word swap.                          */
#define  CSMD_END_CONV_L(_x)      CSMD_REVERSE_CSMD_ULONG(_x)   /* Reverses the byte order inside 4-byte data.              */

/* #define  CSMD_LE16_CONV(_x)    CSMD_END_CONV_S(_x) */        /* Reverses the byte order inside 2-byte, if BE is set.     */
/* #define  CSMD_BE16_CONV(_x)    (_x)                */        /* Do nothing, if BE is set.                                */
/* #define  CSMD_LE32_CONV(_x)    CSMD_END_CONV_L(_x) */        /* Reverses the byte order inside 4-byte, if BE is set.     */
/* #define  CSMD_BE32_CONV(_x)    (_x)                */        /* Do nothing, if BE is set.                                */

#else

#define  CSMD_END_CONV_S(_x)      (_x)                          /* Reverses the byte order inside 2-byte data.              */
#define  CSMD_END_CONV_W(_x)      (_x)                          /* Upper/lower unsigned word swap.                          */
#define  CSMD_END_CONV_L(_x)      (_x)                          /* Reverses the byte order inside 4-byte data.              */

/* #define  CSMD_LE16_CONV(_x)    (_x)                */        /* Do nothing, if BE is not set.                            */
/* #define  CSMD_BE16_CONV(_x)    CSMD_END_CONV_S(_x) */        /* Reverses the byte order inside 2-byte, if BE is not set. */
/* #define  CSMD_LE32_CONV(_x)    (_x)                */        /* Do nothing, if BE is not set.                            */
/* #define  CSMD_BE32_CONV(_x)    CSMD_END_CONV_L(_x) */        /* Reverses the byte order inside 4-byte, if BE is not set. */

#endif /* BIG_ENDIAN */


/* --------------------------------------------- */
/*  This CoSeMa supports only FPGA Version 4.x ! */
/* --------------------------------------------- */
#define CSMD_HAL_SERCON100M_V4          (4)

#define CSMD_HAL_FPGA_TEST_MASK         ((CSMD_ULONG) 0x00FF0000)
#define CSMD_HAL_FPGA_TEST_SHIFT        (16)

/* Macro CSMD_SERCON100M_( )
   __ver - FPGA version
   __rel - FPGA release
   __tst - FPGA testversion: Bit-mask for not supported test versions.
                             The bit number corresponds to the test version number
                             Bit 7...1 - test version 7...1
                             Bit 0     - officially version (no test version)
                             Blacklisted test versions are indicated by a set bit
                             for example 0x7E --> from T1 to T6 are not allowed
*/
#define CSMD_HAL_SERCON100M_(__ver,__rel,__tst)   (  (((__ver) << CSMD_HAL_FPGA_IDR_VERSION_SHIFT) & CSMD_HAL_FPGA_IDR_VERSION_MASK) \
                                                   + (((__rel) << CSMD_HAL_FPGA_IDR_RELEASE_SHIFT) & CSMD_HAL_FPGA_IDR_RELEASE_MASK) \
                                                   + (((__tst) << CSMD_HAL_FPGA_TEST_SHIFT)        & CSMD_HAL_FPGA_TEST_MASK) )

#define CSMD_HAL_SERCON100M_VERSION_LIST \
{                                        \
  /* Blacklist                                                                */  \
  CSMD_HAL_SERCON100M_(4, 0,0xFF),  /* official release and all test-versions */  \
  CSMD_HAL_SERCON100M_(4, 1,0xFF),  /*   "                                    */  \
  CSMD_HAL_SERCON100M_(4, 2,0xFF),  /*   "                                    */  \
  CSMD_HAL_SERCON100M_(4, 3,0xFF),  /*   "                                    */  \
  CSMD_HAL_SERCON100M_(4, 4,0xFF),  /*   "                                    */  \
  CSMD_HAL_SERCON100M_(4, 5,0xFF),  /*   "                                    */  \
  CSMD_HAL_SERCON100M_(4, 6,0xFF),  /*   "                                    */  \
  CSMD_HAL_SERCON100M_(4, 7,0xFF),  /*   "                                    */  \
  CSMD_HAL_SERCON100M_(4, 8,0xFF),  /*   "                                    */  \
  CSMD_HAL_SERCON100M_(4, 9,0xFE),  /* V4.9  all test versions                */  \
  CSMD_HAL_SERCON100M_(4,10,0xFE),  /* V4.10 all test versions                */  \
  CSMD_HAL_SERCON100M_(4,11,0xFE),  /* V4.11 all test versions                */  \
  CSMD_HAL_SERCON100M_(4,12,0xFF)   /* official release and all test-versions */  \
}



/*--------------------------------------------------------- */
/* FPGA identification register                       (00h) */
/*--------------------------------------------------------- */
#define  CSMD_HAL_FPGA_IDR_IDENT_MASK       0xFFFF0000U
#define  CSMD_HAL_FPGA_IDR_IDENT_SHIFT      (16)    
#define  CSMD_HAL_FPGA_IDR_SIII_IDENT       (0x88CD)            /* Sercos Ethernet frame type */

#define  CSMD_HAL_FPGA_IDR_TYPE_MASK        0x0000E000U
#define  CSMD_HAL_FPGA_IDR_TYPE_SHIFT       (13)
#define  CSMD_HAL_FPGA_IDR_MASTER           ( 0)                /* Sercos master device */
#define  CSMD_HAL_FPGA_IDR_SLAVE            ( 1)                /* Sercos slave device */
#define  CSMD_HAL_FPGA_IDR_MASTER_CONF      ( 2)                /* Sercos master device with conformizer features */
#define  CSMD_HAL_FPGA_IDR_SOFT_MASTER      ( 6)                /* Sercos soft master device */

#define  CSMD_HAL_FPGA_IDR_TEST_MASK        0x00001C00U
#define  CSMD_HAL_FPGA_IDR_TEST_SHIFT       (10)
#define  CSMD_HAL_FPGA_IDR_VERSION_MASK     0x000003E0U
#define  CSMD_HAL_FPGA_IDR_VERSION_SHIFT    ( 5)
#define  CSMD_HAL_FPGA_IDR_RELEASE_MASK     0x0000001FU
#define  CSMD_HAL_FPGA_IDR_RELEASE_SHIFT    ( 0)


/*--------------------------------------------------------- */
/* Global control / status / feature register         (04h) */
/*--------------------------------------------------------- */
#ifdef CSMD_BIG_ENDIAN

#define  CSMD_HAL_GCSFR_HW_BUSINTERFACE     (0x000000FF)    /* Hardware version of bus interface */
#define  CSMD_HAL_GCSFR_SHIFT_LINE_BR_SENS  ( 8+ 8)         /* Shift define for line break sensivity in GCSFR */
#define  CSMD_HAL_GCSFR_LINEBREAK_SENS      (0x000F0000)    /* Mask for line break sensitivity in GCSFR */
#define  CSMD_HAL_GCSFR_PHY_RESET           ( 1+24)         /* PHY reset */
#define  CSMD_HAL_GCSFR_SOFT_RESET          ( 0+24)         /* Software reset */

#else

#define  CSMD_HAL_GCSFR_HW_BUSINTERFACE     (0xFF000000)    /* Hardware version of bus interface */
#define  CSMD_HAL_GCSFR_LINEBREAK_SENS      (0x00000F00)    /* Mask for line break sensitivity in GCSFR */
#define  CSMD_HAL_GCSFR_SHIFT_LINE_BR_SENS  (8)             /* Shift define for line break sensivity in GCSFR */
#define  CSMD_HAL_GCSFR_PHY_RESET           (1)             /* PHY reset */
#define  CSMD_HAL_GCSFR_SOFT_RESET          (0)             /* Software reset */

#endif


/*--------------------------------------------------------- */
/* Data flow control/status register                  (20h) */
/*--------------------------------------------------------- */
#ifdef CSMD_BIG_ENDIAN

/* Status: */
#define  CSMD_HAL_DFCSR_LINE_STS_DELAY_P2       (13)
#define  CSMD_HAL_DFCSR_LINE_STS_DELAY_P1       (12)
#define  CSMD_HAL_DFCSR_LINE_STS_DELAY_SHIFT    (12)
#define  CSMD_HAL_DFCSR_LINK_P2                 (11)
#define  CSMD_HAL_DFCSR_LINK_P1                 (10)
#define  CSMD_HAL_DFCSR_LINK_SHIFT              (10)
#define  CSMD_HAL_DFCSR_LINE_STS_P2             ( 9)
#define  CSMD_HAL_DFCSR_LINE_STS_P1             ( 8)
#define  CSMD_HAL_DFCSR_LINE_STS_SHIFT          ( 8)

/* Control: */
#define  CSMD_HAL_DFCSR_ENABLE_TX_SERCOSTIME    (11+8)
#define  CSMD_HAL_DFCSR_RX_ENABLE               ( 9+8)
#define  CSMD_HAL_DFCSR_TX_ENABLE               ( 8+8)

#define  CSMD_HAL_DMA_IP_DISABLE                ( 5+24)
#define  CSMD_HAL_DFCSR_DBUG_PORT_ENABLE        ( 4+24)

#define  CSMD_HAL_DFCSR_TOPOLOGY                (0x03000000 + CSMD_HAL_DFCSR_TOPOLOGY_DISABLE_COLBUF)
#define  CSMD_HAL_DFCSR_TOPOLOGY_RT_P1_MODE     (0x02000000)
#define  CSMD_HAL_DFCSR_TOPOLOGY_RT_P2_MODE     (0x01000000)
#define  CSMD_HAL_DFCSR_TOPOLOGY_RT_P12_MODE    (0x00000000)
#define  CSMD_HAL_DFCSR_TOPOLOGY_RT_RING_MODE   (0x00000000 + CSMD_HAL_DFCSR_TOPOLOGY_DISABLE_COLBUF)
#define  CSMD_HAL_DFCSR_TOPOLOGY_UC_RING_MODE   (0x03000000 + CSMD_HAL_DFCSR_TOPOLOGY_DISABLE_COLBUF)
#define  CSMD_HAL_DFCSR_TOPOLOGY_UC_LINE_MODE   (0x03000000)
#define  CSMD_HAL_DFCSR_TOPOLOGY_DISABLE_COLBUF (0x04000000)

#else

/* Status: */
/*#define  CSMD_HAL_DFCSR_RX_FIFO_ERR_P2        (31)*/
/*#define  CSMD_HAL_DFCSR_RX_FIFO_ERR_P1        (30)*/

/*#define  CSMD_HAL_DFCSR_RX_ALL_DONE_P2        (27)*/
/*#define  CSMD_HAL_DFCSR_TX_ALL_DONE_P2        (26)*/
/*#define  CSMD_HAL_DFCSR_RX_ALL_DONE_P1        (25)*/
/*#define  CSMD_HAL_DFCSR_TX_ALL_DONE_P1        (24)*/

#define  CSMD_HAL_DFCSR_LINE_STS_DELAY_P2       (21)
#define  CSMD_HAL_DFCSR_LINE_STS_DELAY_P1       (20)
#define  CSMD_HAL_DFCSR_LINE_STS_DELAY_SHIFT    (20)
#define  CSMD_HAL_DFCSR_LINK_P2                 (19)
#define  CSMD_HAL_DFCSR_LINK_P1                 (18)
#define  CSMD_HAL_DFCSR_LINK_SHIFT              (18)
#define  CSMD_HAL_DFCSR_LINE_STS_P2             (17)
#define  CSMD_HAL_DFCSR_LINE_STS_P1             (16)
#define  CSMD_HAL_DFCSR_LINE_STS_SHIFT          (16)

/* Control: */
#define  CSMD_HAL_DFCSR_ENABLE_TX_SERCOSTIME    (11)
/*#define  CSMD_HAL_DFCSR_LEN_CTRL_DISABLE        (10)*/
#define  CSMD_HAL_DFCSR_RX_ENABLE               ( 9)
#define  CSMD_HAL_DFCSR_TX_ENABLE               ( 8)

#define  CSMD_HAL_DMA_IP_DISABLE                ( 5)
/*#define  CSMD_HAL_DFCSR_DBUG_PORT_ENABLE        ( 4)*/

#define  CSMD_HAL_DFCSR_TOPOLOGY                (0x00000003 + CSMD_HAL_DFCSR_TOPOLOGY_DISABLE_COLBUF)
#define  CSMD_HAL_DFCSR_TOPOLOGY_RT_P1_MODE     (0x00000002)
#define  CSMD_HAL_DFCSR_TOPOLOGY_RT_P2_MODE     (0x00000001)
#define  CSMD_HAL_DFCSR_TOPOLOGY_RT_P12_MODE    (0x00000000)
#define  CSMD_HAL_DFCSR_TOPOLOGY_RT_RING_MODE   (0x00000000 + CSMD_HAL_DFCSR_TOPOLOGY_DISABLE_COLBUF)
#define  CSMD_HAL_DFCSR_TOPOLOGY_UC_RING_MODE   (0x00000003 + CSMD_HAL_DFCSR_TOPOLOGY_DISABLE_COLBUF)
#define  CSMD_HAL_DFCSR_TOPOLOGY_UC_LINE_MODE   (0x00000003)
#define  CSMD_HAL_DFCSR_TOPOLOGY_DISABLE_COLBUF (0x00000004)

#endif


/*--------------------------------------------------------- */
/* Phase control register                             (24h) */
/*--------------------------------------------------------- */
#ifdef CSMD_BIG_ENDIAN

#define  CSMD_HAL_PHASECR_PHASE_MASK        (0x0F000000L)   /* Communication phase information to be sent in MST of packets */
#define  CSMD_HAL_PHASECR_PHASE_SHIFT       (24)            /* Communication phase information Nbr of bit shift  */
#define  CSMD_HAL_PHASECR_PS                (31)            /* Communication Phase switch bit to be sent in MST of packets */
#define  CSMD_HAL_PHASECR_CYCLE_COUNT       (0x00007000)    /* Cycle count from the internal cycle counter */

#else

#define  CSMD_HAL_PHASECR_PHASE_MASK        (0x0000000FL)   /* Communication phase information to be sent in MST of packets */
#define  CSMD_HAL_PHASECR_PHASE_SHIFT       (0)             /* Communication phase information Nbr of bit shift  */
#define  CSMD_HAL_PHASECR_PS                (7)             /* Communication Phase switch bit to be sent in MST of packets */
#define  CSMD_HAL_PHASECR_CYCLE_COUNT       (0x00000070)    /* Cycle count from the internal cycle counter */

#endif


/*--------------------------------------------------------- */
/* Telegram status register port 1 or port 2      (28h/2Ch) */
/*--------------------------------------------------------- */
/*#define  CSMD_HAL_TGSR_FIRST_MST                (0x10000000UL)*/  /* Bit 28     Indicates that the first MST arrives at this port */
/*#define  CSMD_HAL_TGSR_CYC_CNT_VALID            (0x08000000UL)*/  /* Bit 27     Cycle count valid bit from the MST header of MDT0 */
/*#define  CSMD_HAL_TGSR_CYC_CNT                  (0x07000000UL)*/  /* Bit 26-24  Cycle count from the MST header of MDT0 */

#define  CSMD_HAL_TGSR_AT0_MISS                 (0x00002000UL)      /* Bit 13 */
#define  CSMD_HAL_TGSR_MST_DMISS                (0x00001000UL)      /* Bit 12 */
#define  CSMD_HAL_TGSR_MST_MISS                 (0x00000800UL)      /* Bit 11 */
#define  CSMD_HAL_TGSR_MST_WIN_ERR              (0x00000400UL)      /* Bit 10 */
#define  CSMD_HAL_TGSR_SEC_TEL                  (0x00000200UL)      /* Bit  9 */
#define  CSMD_HAL_TGSR_MST_VALID                (0x00000100UL)      /* Bit  8 */
#define  CSMD_HAL_TGSR_AT3                      (0x00000080UL)      /* Bit  7 */
#define  CSMD_HAL_TGSR_AT2                      (0x00000040UL)      /* Bit  6 */
#define  CSMD_HAL_TGSR_AT1                      (0x00000020UL)      /* Bit  5 */
#define  CSMD_HAL_TGSR_AT0                      (0x00000010UL)      /* Bit  4 */
#define  CSMD_HAL_TGSR_MDT3                     (0x00000008UL)      /* Bit  3 */
#define  CSMD_HAL_TGSR_MDT2                     (0x00000004UL)      /* Bit  2 */
#define  CSMD_HAL_TGSR_MDT1                     (0x00000002UL)      /* Bit  1 */
#define  CSMD_HAL_TGSR_MDT0                     (0x00000001UL)      /* Bit  0 */

#define  CSMD_HAL_TGSR_ALL_AT                   (0x000000F0UL)      /* Bit  7...4 */
#define  CSMD_HAL_TGSR_ALL_MDT                  (0x0000000FUL)      /* Bit  3...0 */
#define  CSMD_HAL_TGSR_ALL_TEL                  (  CSMD_HAL_TGSR_ALL_AT \
                                                 | CSMD_HAL_TGSR_ALL_MDT )  /* Bit  7...0 */
/* 
#define  CSMD_HAL_TGSR_MST_AT0_VALID_MASK       (  CSMD_HAL_TGSR_MST_VALID\
                                                 | CSMD_HAL_TGSR_AT0       )
#define  CSMD_HAL_TGSR_MST_VALID_PRISEC_MASK    (  CSMD_HAL_TGSR_MST_VALID\
                                                 | CSMD_HAL_TGSR_SEC_TEL   )
#define  CSMD_HAL_TGSR_VALID_SEC_TEL            CSMD_HAL_TGSR_MST_VALID_PRISEC_MASK
#define  CSMD_HAL_TGSR_VALID_PRI_TEL            CSMD_HAL_TGSR_MST_VALID
*/
#define  CSMD_HAL_TGSR_VALID_MST_MASK           (  CSMD_HAL_TGSR_MST_VALID\
                                                 | CSMD_HAL_TGSR_MST_WIN_ERR )

/*--------------------------------------------------------- */
/* Timing Control / Status register                   (38h) */
/*--------------------------------------------------------- */
#ifdef CSMD_BIG_ENDIAN

#define  CSMD_HAL_TCSR_TCNT_RUNNING     (20- 8)        /* Timer TCNT is running */
#define  CSMD_HAL_TCSR_SYS_TIME_TOGGLE  (15 +8)        /* Toggle bit in time field (Extended function field bit 15 - active time toggle) */
#define  CSMD_HAL_TCSR_DIVCLK_OD        (14+ 8)        /* DivClk output disable */
#define  CSMD_HAL_TCSR_DIVCLK_POL       (13+ 8)        /* DivClk polarity */
#define  CSMD_HAL_TCSR_DIVCLK_MODE      (12+ 8)        /* DivClk modes */
#define  CSMD_HAL_TCSR_CON_PLL_EN       (11+ 8)        /* CON_CLK PLL enable */
#define  CSMD_HAL_TCSR_CONOE            (10+ 8)        /* Enable of output driver of CON_CLK */
#define  CSMD_HAL_TCSR_CONPOL           ( 9+ 8)        /* Polarity of output CON_CLK */
#define  CSMD_HAL_TCSR_CONEN            ( 8+ 8)        /* Activation of output CON_CLK */
#define  CSMD_HAL_TCSR_CYCST            ( 7+24)        /* CYCSTART activation by processor */
#define  CSMD_HAL_TCSR_CYCPOL           ( 6+24)        /* Polarity of input CYC_CLK */
#define  CSMD_HAL_TCSR_CYCEN            ( 5+24)        /* Enable of input CYC_CLK */
#define  CSMD_HAL_TCSR_M_S              ( 4+24)        /* Timing Master/Slave Mode */
#define  CSMD_HAL_TCSR_EST              ( 3+24)        /* Enable Sercos Time (C-Time Bit 14 - Time Valid) */
#define  CSMD_HAL_TCSR_ET2              ( 2+24)        /* Enable timer for port 2 (TCNT[2]) */
#define  CSMD_HAL_TCSR_ET1              ( 1+24)        /* Enable timer for port 1 (TCNT[1]) */
#define  CSMD_HAL_TCSR_ET0              ( 0+24)        /* Enable main timer 0 (TCNT) */

#else

#define  CSMD_HAL_TCSR_TCNT_RUNNING     (20)           /* Timer TCNT is running */
#define  CSMD_HAL_TCSR_SYS_TIME_TOGGLE  (15)           /* Toggle bit in time field (Extended function field bit 15 - active time toggle) */
#define  CSMD_HAL_TCSR_DIVCLK_OD        (14)           /* DivClk output disable */
#define  CSMD_HAL_TCSR_DIVCLK_POL       (13)           /* DivClk polarity */
#define  CSMD_HAL_TCSR_DIVCLK_MODE      (12)           /* DivClk modes */
#define  CSMD_HAL_TCSR_CON_PLL_EN       (11)           /* CON_CLK PLL enable */
#define  CSMD_HAL_TCSR_CONOE            (10)           /* Enable of output driver of CON_CLK */
#define  CSMD_HAL_TCSR_CONPOL           ( 9)           /* Polarity of output CON_CLK */
#define  CSMD_HAL_TCSR_CONEN            ( 8)           /* Activation of output CON_CLK */
#define  CSMD_HAL_TCSR_CYCST            ( 7)           /* CYCSTART activation by processor */
#define  CSMD_HAL_TCSR_CYCPOL           ( 6)           /* Polarity of input CYC_CLK */
#define  CSMD_HAL_TCSR_CYCEN            ( 5)           /* Enable of input CYC_CLK */
#define  CSMD_HAL_TCSR_M_S              ( 4)           /* Timing Master/Slave Mode */
#define  CSMD_HAL_TCSR_EST              ( 3)           /* Enable Sercos Time (C-Time Bit 14 - Time Valid) */
#define  CSMD_HAL_TCSR_ET2              ( 2)           /* Enable timer for port 2 (TCNT[2]) */
#define  CSMD_HAL_TCSR_ET1              ( 1)           /* Enable timer for port 1 (TCNT[1]) */
#define  CSMD_HAL_TCSR_ET0              ( 0)           /* Enable main timer 0 (TCNT) */

#endif


/*--------------------------------------------------------- */
/* SVC Control / Status Register                      (50h) */
/*--------------------------------------------------------- */
#ifdef CSMD_BIG_ENDIAN

#define  CSMD_HAL_SVCCSR_BUSY_TO_MASK        (0x000000FF)   /* Bit 31-24: Busy timeout value */
#define  CSMD_HAL_SVCCSR_BUSY_TO_SHIFT       (0)            /*   Shift for busys timeout value */
#define  CSMD_HAL_SVCCSR_BUSY_HS_MASK        (0x0000FF00)   /* Bit 23-16: Busy timeout value */
#define  CSMD_HAL_SVCCSR_BUSY_HS_SHIFT       (8)            /*   Shift for busys timeout value */
#define  CSMD_HAL_SVCCSR_BUSY_BIT_MASK       (0x00020000)   /* Bit 9:     SVC state machine busy */
/*#define  CSMD_HAL_SVCCSR_PROCESS_ERROR       (8+8)*/      /* Bit 8:     Process error occurred while AT processing (read) clear process error (write) */
/*#define  CSMD_HAL_SVCCSR_PROCESS_START       (7+24)*/     /* Bit 7:     Start SVC machine manually with positive edge of bit location (for debug use) */
#define  CSMD_HAL_SVCCSR_START_MODE_MASK     (0x20000000)   /* Bit 5:     Use port timer event to start service channel processor */
#define  CSMD_HAL_SVCCSR_REDUNDANCY_MASK     (0x10000000)   /* Bit 4:     Activate service chanel redundancy function */
#define  CSMD_HAL_SVCCSR_AT_SELECT           (0x0C000000)   /* Bit 3-2:   AT (0-3) that triggers the SVC engine */
#define  CSMD_HAL_SVCCSR_AT_SELECT_SHIFT     (26)           /*   Shift for AT select */
#define  CSMD_HAL_SHIFT_SVCCSR_TRIG_PORT     (25)           /* Bit 1:     Trigger SVC machine from port1 (0) or port 2 (1) */
#define  CSMD_HAL_SVCCSR_ENABLE              (24)           /* Bit 0:     Enable SVC operation */

#else

#define  CSMD_HAL_SVCCSR_BUSY_TO_MASK        (0xFF000000)   /* Bit 31-24: Busy timeout value */
#define  CSMD_HAL_SVCCSR_BUSY_TO_SHIFT       (24)           /*   Shift for busys timeout value */
#define  CSMD_HAL_SVCCSR_BUSY_HS_MASK        (0x00FF0000)   /* Bit 23-16: Busy timeout value */
#define  CSMD_HAL_SVCCSR_BUSY_HS_SHIFT       (16)           /*   Shift for busys timeout value */
#define  CSMD_HAL_SVCCSR_BUSY_BIT_MASK       (0x00000200)   /* Bit 9:     SVC state machine busy */
/*#define  CSMD_HAL_SVCCSR_PROCESS_ERROR       (8)*/        /* Bit 8:     Process error occurred while AT processing (read) clear process error (write) */
/*#define  CSMD_HAL_SVCCSR_PROCESS_START       (7)*/        /* Bit 7:     Start SVC machine manually with positive edge of bit location (for debug use) */
#define  CSMD_HAL_SVCCSR_START_MODE_MASK     (0x00000020)   /* Bit 5:     Use port timer event to start service channel processor */
#define  CSMD_HAL_SVCCSR_REDUNDANCY_MASK     (0x00000010)   /* Bit 4:     Activate service chanel redundancy function */
#define  CSMD_HAL_SVCCSR_AT_SELECT           (0x0000000C)   /* Bit 3-2:   AT (0-3) that triggers the SVC engine */
#define  CSMD_HAL_SVCCSR_AT_SELECT_SHIFT     (2)            /*   Shift for AT select */
#define  CSMD_HAL_SHIFT_SVCCSR_TRIG_PORT     (1)            /* Bit 1:     Trigger SVC machine from port1 (0) or port 2 (1) */
#define  CSMD_HAL_SVCCSR_ENABLE              (0)            /* Bit 0:     Enable SVC operation */

#endif

#define  CSMD_HAL_SVCCSR_TRIG_SELECT_AT0     0UL        /* Trigger SVC with AT0 */
#define  CSMD_HAL_SVCCSR_TRIG_SELECT_AT1     1UL        /* Trigger SVC with AT1 */
#define  CSMD_HAL_SVCCSR_TRIG_SELECT_AT2     2UL        /* Trigger SVC with AT2 */
#define  CSMD_HAL_SVCCSR_TRIG_SELECT_AT3     3UL        /* Trigger SVC with AT3 */

#define  CSMD_HAL_SVCCSR_TRIG_PORT1          1U         /* Trigger SVC machine from port1 */
#define  CSMD_HAL_SVCCSR_TRIG_PORT2          2U         /* Trigger SVC machine from port2 */


/*--------------------------------------------------------- */
/* Sercos Frame Control Register                      (70h) */
/*--------------------------------------------------------- */
#define  CSMD_HAL_SFCR_ENABLE_AT3       (27)                /* Bit 27:  Send AT3 after Send-AT-Event */
#define  CSMD_HAL_SFCR_ENABLE_AT2       (26)                /* Bit 26:  Send AT2 after Send-AT-Event */
#define  CSMD_HAL_SFCR_ENABLE_AT1       (25)                /* Bit 25:  Send AT1 after Send-AT-Event */
#define  CSMD_HAL_SFCR_ENABLE_AT0       (24)                /* Bit 24:  Send AT0 after Send-AT-Event */

#define  CSMD_HAL_SFCR_ENABLE_MDT3      (19)                /* Bit 19:  Send MDT3 after Send-MDT-Event */
#define  CSMD_HAL_SFCR_ENABLE_MDT2      (18)                /* Bit 18:  Send MDT2 after Send-MDT-Event */
#define  CSMD_HAL_SFCR_ENABLE_MDT1      (17)                /* Bit 17:  Send MDT1 after Send-MDT-Event */
#define  CSMD_HAL_SFCR_ENABLE_MDT0      (16)                /* Bit 16:  Send MDT0 after Send-MDT-Event */

/*#define  CSMD_HAL_SFCR_P_S_SWITCH       (15)*/            /* Bit 15:  Switch P and S port */
/*#define  CSMD_HAL_SFCR_FRAME_TYPE_INT_6 (13)*/            /* Bit 13:  Frame type (MDT,AT) of interrupt 6 */
/*#define  CSMD_HAL_SFCR_PORT_NBR_INT_6   (12)*/            /* Bit 12:  Reception port of interrupt 6 */
/*#define  CSMD_HAL_SFCR_FRAME_NBR_INT_6  (0x00000300UL)*/  /* Bit 9-8: Sercos frame number of interrupt 6 */
/*#define  CSMD_HAL_SFCR_FRAME_TYPE_INT_5 (5)*/             /* Bit 5:   Frame type (MDT,AT) of interrupt 5 */
/*#define  CSMD_HAL_SFCR_PORT_NBR_INT_5   (4)*/             /* Bit 4:   Reception port of interrupt 5 */
/*#define  CSMD_HAL_SFCR_FRAME_NBR_INT_5  (0x00000003UL)*/  /* Bit 1-0: Sercos frame number of interrupt 5 */

#define  CSMD_HAL_SFCR_ENABLE_TEL_MASK  0x0F0F0000U

#define  CSMD_HAL_ENABLE_TEL_MASK       0x0000000FUL
#define  CSMD_HAL_SFCR_SHIFT_MDT        (16)                /* Shift left for MDTs */
#define  CSMD_HAL_SFCR_SHIFT_AT         (24)                /* Shift left for ATs  */


/*--------------------------------------------------------- */
/* Debug Output Control Register                      (78h) */
/*--------------------------------------------------------- */
#define  CSMD_DBGOCR_PORT1_MST               ( 0)
#define  CSMD_DBGOCR_PORT2_MST               ( 1)
#define  CSMD_DBGOCR_SVC_INT                 ( 2)
#define  CSMD_DBGOCR_CON_CLK                 ( 3)
#define  CSMD_DBGOCR_DIV_CLK                 ( 4)
#define  CSMD_DBGOCR_TCNT_RELOAD             ( 5)
#define  CSMD_DBGOCR_START_MDT               ( 6)
#define  CSMD_DBGOCR_START_AT                ( 7)
#define  CSMD_DBGOCR_PORT1_IP_OPEN           ( 8)
#define  CSMD_DBGOCR_PORT1_IP_OPEN_WRITE     ( 9)
#define  CSMD_DBGOCR_PORT2_IP_OPEN           (10)
#define  CSMD_DBGOCR_PORT2_IP_OPEN_WRITE     (11)
#define  CSMD_DBGOCR_PORT1_MST_WIN_OPEN      (12)
#define  CSMD_DBGOCR_PORT2_MST_WIN_OPEN      (13)
#define  CSMD_DBGOCR_PORT1_RX_FRAME          (14)
#define  CSMD_DBGOCR_PORT2_RX_FRAME          (15)


/*--------------------------------------------------------- */
/* IP Status- and Control register IPCSR 1 / 2    (90h/94h) */
/* The 16 lsb [15:0] are the control register (read/write)  */
/* The 16 msb [31:16] are the status register (read)        */
/*--------------------------------------------------------- */
/* Status word: */
/* Status bits could NOT be cleared by writing ZERO into the status register! */
#define CSMD_HAL_IPSR_LINK                    0x8000U   /* Bit 15 (31): Link exists at appropriate port. */
#define CSMD_HAL_IPSR_IP_TX_BUF_EMPTY         0x0008U   /* Bit  3 (19): Set when IP Transmit buffer empty port.
                                                                        Cleared when at least one descriptor is on the Tx stack register. */
#define CSMD_HAL_IPSR_IP_TX_BUF_RDY           0x0004U   /* Bit  2 (18): Set when IP Transmit buffer able to accept a frame by the host.
                                                                        Cleared when IP Tx-buffer is full. */
#define CSMD_HAL_IPSR_IP_RX_BUF_FULL          0x0002U   /* Bit  1 (17): Set when IP Receive buffer is full
                                                                        (all descriptors on the Rx stack register are used).
                                                                        Cleared otherwise */
#define CSMD_HAL_IPSR_IP_RX_RDY               0x0001U   /* Bit  0 (16): Set when an IP Ethernet frame is received without error
                                                                        (at least one descriptor is on the Rx stack register).
                                                                        Cleared otherwise */
/* Control word: */
#define CSMD_HAL_IPCR_IP_TX_BUF_EMPTY_INT_EN  0x0800U   /* Bit 11: Set to ONE enables Interrupt Int_IPIntPort1 or Int_IPIntPort2 on event IPTxBufEmpty. */
#define CSMD_HAL_IPCR_IP_TX_BUF_RDY_INT_EN    0x0400U   /* Bit 10: Set to ONE enables Interrupt Int_IPIntPort1 or Int_IPIntPort2 on event IPTxBufRdy. */
#define CSMD_HAL_IPCR_IP_RX_BUF_FULL_INT_EN   0x0200U   /* Bit  9: Set to ONE enables Interrupt Int_IPIntPort1 or Int_IPIntPort2 on event IPRxBufFull. */
#define CSMD_HAL_IPCR_IP_RX_RDY_INT_EN        0x0100U   /* Bit  8: Set to ONE enables Interrupt Int_IPIntPort1 or Int_IPIntPort2 on event IPRxRdy. */
#define CSMD_HAL_IPCR_PROMISCUOUS             0x0040U   /* Bit  6: Receive all Frames without checking the destination address.*/
#define CSMD_HAL_IPCR_COL_BUF_DISABLE         0x0020U   /* Bit  5: Disables collision buffer. Frames are not forwarded to opposite port. */
#define CSMD_HAL_IPCR_MUL_CAST_DISABLE        0x0010U   /* Bit  4: Disables reception of Multicast frames. Forwarding is not affected. */
#define CSMD_HAL_IPCR_BROAD_DISABLE           0x0008U   /* Bit  2: Disables reception of Broadcast frames. Forwarding is not affected. */
#define CSMD_HAL_IPCR_IP_RX_ENABLE            0x0002U   /* Bit  1: Enable TX Ports */
#define CSMD_HAL_IPCR_IP_TX_ENABLE            0x0001U   /* Bit  0: Enable RX Ports */


#ifdef CSMD_SYNC_DPLL
/*--------------------------------------------------------- */
/* PLL Control- and Status Register                  (120h) */
/* -------------------------------------------------------- */
#ifdef CSMD_BIG_ENDIAN
                                                    /* Bit  7- 0: PLL command                     */
#define CSMD_HAL_PLL_STOP_SYNC    (0x01000000UL)    /*    1: PLL command "stop synchronisation"   */
#define CSMD_HAL_PLL_START_SYNC   (0x02000000UL)    /*    2: PLL command "start synchronisation"  */

#define CSMD_HAL_PLL_INIT         (0x00010000UL)    /* Bit  8: PLL is in init state               */
#define CSMD_HAL_PLL_ACTIVE       (0x00020000UL)    /* Bit  9: PLL is active (set after start
                                                                 sync command is executed)        */
#define CSMD_HAL_PLL_LOCKED       (0x00040000UL)    /* Bit 10: PLL is locked to the master        */
#define CSMD_HAL_PLL_SYNC_ERROR   (0x00080000UL)    /* Bit 11: PLL is outside of the 
                                                                 synchronisation window           */
#define CSMD_HAL_PLL_CMD_ACTIVE   (0x00800000UL)    /* Bit 15: Command for starting/stoping 
                                                                 the PLL is active                */
#else
                                                    /* Bit  7- 0: PLL command                     */
#define CSMD_HAL_PLL_STOP_SYNC    (0x00000001UL)    /*    1: PLL command "stop synchronisation"   */
#define CSMD_HAL_PLL_START_SYNC   (0x00000002UL)    /*    2: PLL command "start synchronisation"  */

#define CSMD_HAL_PLL_INIT         (0x00000100UL)    /* Bit  8: PLL is in init state               */
#define CSMD_HAL_PLL_ACTIVE       (0x00000200UL)    /* Bit  9: PLL is active (set after start
                                                                 sync command is executed)        */
#define CSMD_HAL_PLL_LOCKED       (0x00000400UL)    /* Bit 10: PLL is locked to the master        */
#define CSMD_HAL_PLL_SYNC_ERROR   (0x00000800UL)    /* Bit 11: PLL is outside of the 
                                                                 synchronisation window           */
#define CSMD_HAL_PLL_CMD_ACTIVE   (0x00008000UL)    /* Bit 15: Command for starting/stoping 
                                                                 the PLL is active                */
#endif  /* #ifdef CSMD_BIG_ENDIAN */

#endif  /* #ifdef CSMD_SYNC_DPLL */


/*--------------------------------------------------------- */
/* Tx Buffer Control & Status Register A/B      (20Ch/21Ch) */
/*--------------------------------------------------------- */
#define CSMD_HAL_TXBUFCSR_COUNT_MASK        0x00000003U     /* Bit  1- 0: Buffer count                           */

#ifdef CSMD_BIG_ENDIAN
#define CSMD_HAL_TXBUFCSR_ACT_BUF_MASK      0x00000300U     /* Bit 17-16: Active buffer nbr. (for system)        */
#define CSMD_HAL_TXBUFCSR_ACT_BUF_SHIFT     ( 8)            /* Bit shifts for active buffer nbr.                 */
#define CSMD_HAL_TXBUFCSR_REQ_BUFFER        0x00000080U     /* Bit 31:    Acknowledge current transmit buffer (write) */
#define CSMD_HAL_TXBUFCSR_BUF_USABLE        0x00000080U     /* Bit 31:    Buffer system is usable (read)         */
#else
#define CSMD_HAL_TXBUFCSR_ACT_BUF_MASK      0x00030000U     /* Bit 17-16: Active buffer nbr. (for system)        */
#define CSMD_HAL_TXBUFCSR_ACT_BUF_SHIFT     (16)            /* Bit shifts for active buffer nbr.                 */
#define CSMD_HAL_TXBUFCSR_REQ_BUFFER        0x80000000U     /* Bit 31:    Acknowledge current transmit buffer (write) */
#define CSMD_HAL_TXBUFCSR_BUF_USABLE        0x80000000U     /* Bit 31:    Buffer system is usable (read)         */
#endif

#define CSMD_HAL_TX_BUFFER_SYSTEM_A         (0)
#define CSMD_HAL_TX_BUFFER_SYSTEM_B         (1)


/*--------------------------------------------------------- */
/* Rx Buffer Control & Status Register A/B      (200h/210h) */
/*--------------------------------------------------------- */
#define CSMD_HAL_RXBUFCSR_COUNT_MASK        0x00000003U     /* Bit  1- 0: Buffer count                           */
#define CSMD_HAL_RXBUFCSR_ACT_BUF_P1_MASK   0x00030000U     /* Bit 17-16: Active buffer nbr.port 1 (for system)  */
#define CSMD_HAL_RXBUFCSR_ACT_BUF_P1_SHIFT  (16)            /* Bit shifts for active buffer nbr. port 1          */
#define CSMD_HAL_RXBUFCSR_ACT_BUF_P2_MASK   0x03000000U     /* Bit 25-24: Active buffer nbr.port 2 (for system)  */
#define CSMD_HAL_RXBUFCSR_ACT_BUF_P2_SHIFT  (24)            /* Bit shifts for active buffer nbr. port 2          */

#ifdef CSMD_BIG_ENDIAN
#define CSMD_HAL_RXBUFCSR_REQ_BUFFER        0x00000080U     /* Bit 31:    Request newest transmit buffer (write) */
#define CSMD_HAL_RXBUFCSR_BUF_USABLE        0x00000080U     /* Bit 31:    Buffer system is usable (read)         */
#define CSMD_HAL_RXBUFCSR_NEW_DATA_P2       0x00000010U     /* Bit 28:    Current receive buffer for port 2
                                                                          contains new data.                     */
#define CSMD_HAL_RXBUFCSR_NEW_DATA_P1       0x00001000U     /* Bit 20:    Current receive buffer for port 1
                                                                          contains new data.                     */
#else
#define CSMD_HAL_RXBUFCSR_REQ_BUFFER        0x80000000U     /* Bit 31:    Request newest transmit buffer (write) */
#define CSMD_HAL_RXBUFCSR_BUF_USABLE        0x80000000U     /* Bit 31:    Buffer system is usable (read)         */
#define CSMD_HAL_RXBUFCSR_NEW_DATA_P2       0x10000000U     /* Bit 28:    Current receive buffer for port 2
                                                                          contains new data.                     */
#define CSMD_HAL_RXBUFCSR_NEW_DATA_P1       0x00100000U     /* Bit 20:    Current receive buffer for port 1
                                                                          contains new data.                     */
#endif

#define CSMD_HAL_RX_BUFFER_SYSTEM_A         (0)
#define CSMD_HAL_RX_BUFFER_SYSTEM_B         (1)


/*--------------------------------------------------------- */
/* Rx Buffer Telegram Requirements Register A/B (208h/218h) */
/*--------------------------------------------------------- */
#define CSMD_HAL_RXBUFTR_MDT_SHIFT          (0)             /* Bit shifts for required MDT's                     */
#define CSMD_HAL_RXBUFTR_MDT_MASK           0x0000000FU     /* Bit  3- 0: Mask for required MDT's                */
#define CSMD_HAL_RXBUFTR_AT_SHIFT           (8)             /* Bit shifts for required AT's                      */
#define CSMD_HAL_RXBUFTR_AT_MASK            0x00000F00U     /* Bit 11- 8: Mask for required AT's                 */

/*--------------------------------------------------------- */
/* Rx Buffer Telegram Valid Register A/B (204h/214h)        */
/*--------------------------------------------------------- */
#define CSMD_HAL_RXBUFTV_MDT_SHIFT          (0)             /* Bit shifts for required MDT's         */
#define CSMD_HAL_RXBUFTV_MDT_MASK           (0x0000000FU)   /* Bit  3- 0: Mask for required MDT's    */
#define CSMD_HAL_RXBUFTV_AT_SHIFT           (8)             /* Bit shifts for required AT's          */
#define CSMD_HAL_RXBUFTV_AT_MASK            (0x00000F00U)   /* Bit 11- 8: Mask for required AT's     */
#define CSMD_HAL_RXBUFTV_P1_MASK            (0x0000FFFFU)   /* mask for port 1                       */
#define CSMD_HAL_RXBUFTV_P2_MASK            (0xFFFF0000U)   /* mask for port 2                       */
#define CSMD_HAL_RXBUFTV_PORT_SHIFT         (16)            /* Bit shift for port differentiation    */



/*--------------------------------------------------------- */
/* For Tx-/Rx-Multi-Buffering                               */
/*--------------------------------------------------------- */
#define CSMD_HAL_BUFFER_0_ACTIVE        (0)
#define CSMD_HAL_BUFFER_1_ACTIVE        (1)
#define CSMD_HAL_BUFFER_2_ACTIVE        (2)
#define CSMD_HAL_BUFFER_3_ACTIVE        (3)

#define CSMD_HAL_SINGLE_BUFFER_SYSTEM   0U
#define CSMD_HAL_DOUBLE_BUFFER_SYSTEM   1U
#define CSMD_HAL_TRIPLE_BUFFER_SYSTEM   2U




/*---------------------------------------------------- */
/* Tx/Rx descriptor definitions                        */
/*---------------------------------------------------- */
#define  CSMD_HAL_TX_DESR_INDEX_NUMBER      (8)           /* 4 MDTs / 4 ATs */
#define  CSMD_HAL_TX_MDT_DESR_IDX_OFFSET    (0)
#define  CSMD_HAL_TX_AT_DESR_IDX_OFFSET     (4)
#define  CSMD_HAL_RX_DESR_INDEX_NUMBER      (8)           /* 4 MDTs / 4 ATs */
#define  CSMD_HAL_RX_MDT_DESR_IDX_OFFSET    (0)
#define  CSMD_HAL_RX_AT_DESR_IDX_OFFSET     (4)

#define  CSMD_HAL_DES_MASK_BUFF_OFFS        0x00003FFEU   /* Mask for buffer offset        */
#define  CSMD_HAL_DES_MASK_BUFF_SYS_SEL     0x0000C000U   /* Mask for buffer system select */
#define  CSMD_HAL_DES_MASK_TEL_OFFS         0x07FE0000U   /* Mask for telegram offset      */
#define  CSMD_HAL_DES_MASK_TYPE             0xF0000000U   /* Mask for descriptor type      */

#define  CSMD_HAL_DES_SHIFT_BUFF_SYS_SEL    (14)          /* # shift left for buffer system select */
#define  CSMD_HAL_DES_SHIFT_TEL_OFFS        (16)          /* # shift left for telegram offset      */
#define  CSMD_HAL_DES_SHIFT_TYPE            (28)          /* # shift left for descriptor type      */

#define CSMD_DESCR_INDEX_ENABLE             (0x00000001)
#define CSMD_DESCR_INDEX_OFFSET_SHIFT       (2)

typedef union CSMD_DESCR_INDEX_UN
{
  CSMD_ULONG   ulIndex;
#ifdef CSMD_DEBUG
  struct
  {
    CSMD_ULONG               : 18;   /*!< Bit 31-14:   Reserved (future use, must be  zero)                 */
    CSMD_ULONG   OFFSET      : 12;   /*!< Bit 13- 2: Long offset in RAM of the first descriptor table entry */
    CSMD_ULONG               :  1;   /*!< Bit 1    :   Reserved (must be zero)                              */
    CSMD_ULONG   ENABLE      :  1;   /*!< Bit 0    : Enable of according descriptor table                   */
    
  } rIndex;
#endif
} CSMD_DESCR_INDEX;

enum
{
  CSMD_DES_IDX_MDT0 = 0,
  CSMD_DES_IDX_MDT1,
  CSMD_DES_IDX_MDT2,
  CSMD_DES_IDX_MDT3,
  CSMD_DES_IDX_AT0,
  CSMD_DES_IDX_AT1,
  CSMD_DES_IDX_AT2,
  CSMD_DES_IDX_AT3
  
};
/* } CSMD_DESCR_TABLE_IDX; */


typedef enum
{
  /* --------------------------------------------------------- */
  /* no error: states, warning codes  (error_class 0x00000nnn) */
  /* --------------------------------------------------------- */
  CSMD_HAL_NO_ERROR       = (0x00000000),      /*!< 0x00  Function successfully completed */
  /* --------------------------------------------------------- */
  /* Sercos error codes               (error_class 0x00020nnn) */
  /* --------------------------------------------------------- */
  CSMD_HAL_DIVCLK_TIMES   = (0x00020029),      /*!< 0x29  Config. DivClk: Pulse delay or Distance exceeds max. Sercos cycle time */
  CSMD_HAL_DIVCLK_PULSES  = (0x0002002a)       /*!< 0x2a  Config. DivClk: Number of pulses to high */
  
}  CSMD_HAL_FUNC_RET;



/**************************************************************************/ /**

\def CSMD_HAL_IsSoftMaster ( _pHal )

\b HAL-Function: CSMD_BOOL CSMD_HAL_IsSoftMaster ( CSMD_HAL *prCSMD_HAL )

\brief CSMD_HAL_IsSoftMaster provides information if soft-master
       or Sercos-master-IP-Core is used.

\b Description:

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _pHal  Pointer to the CoSeMa HAL structure

\return       TRUE  = Sercos soft master is used instead of hard master IP-Core.\n
              FALSE = Hard master IP-Core is used.

\author       WK
\date         11.02.2015

***************************************************************************** */
#define CSMD_HAL_IsSoftMaster( _pHal )\
            ( (_pHal)->boSoftMaster )



#ifdef CSMD_SYNC_DPLL

/**************************************************************************/ /**
\todo review

\def CSMD_HAL_Read_PLL_Status( _Inst )

\b HAL-Function CSMD_ULONG CSMD_HAL_Read_PLL_Status( CSMD_HAL *prCSMD_HAL )

\brief Gets the PLL state from the PLLCSR (PLL control and status register).

\b Description

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]  _Inst         base address of the CoSeMa HAL structure

\return       Bitlist with current state of the PLL: 
              - CSMD_HAL_PLL_INIT         Bit  8: PLL is in init state
              - CSMD_HAL_PLL_ACTIVE       Bit  9: PLL is active (set after start sync command is executed)
              - CSMD_HAL_PLL_LOCKED       Bit 10: PLL is locked to the master
              - CSMD_HAL_PLL_SYNC_ERROR   Bit 11: PLL is outside of the synchronisation window

              - CSMD_HAL_PLL_CMD_ACTIVE   Bit 15: Command for starting/stoping the PLL is active

\author       WK
\date         17.11.2011

***************************************************************************** */
#define CSMD_HAL_Read_PLL_Status( _Inst )\
             ((CSMD_ULONG) ( CSMD_END_CONV_L( *_Inst.prSERC_Reg->ulPLLCSR )\
                            & (CSMD_ULONG)(  CSMD_HAL_PLL_INIT      \
                                           | CSMD_HAL_PLL_ACTIVE    \
                                           | CSMD_HAL_PLL_LOCKED    \
                                           | CSMD_HAL_PLL_SYNC_ERROR\
                                           | CSMD_HAL_PLL_CMD_ACTIVE) ))



/**************************************************************************/ /**
\todo review
\def CSMD_HAL_Enable_PLL( _Inst, _Enable )

\b HAL-Function CSMD_VOID CSMD_HAL_Enable_PLL( CSMD_HAL  *prCSMD_HAL,
                                               CSMD_BOOL  boEnable )

\brief Enables respectively disables the synchronization with PLL.

\b Description  Enable / Disable the PLL by controlling the CYC_CLK_PLL\n
                    enable bit (11) in the TCSR 
                    (Timing control and status register).

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst     base address of the CoSeMa HAL structure
\param [in]   _Enable   PLL activation:
                        - TRUE  Enable the CYC_CLK PLL
                        - FALSE Disable the CYC_CLK PLL

\return       none

\author       WK
\date         17.11.2011

***************************************************************************** */
/*lint -emacro( {774}, CSMD_HAL_Enable_PLL ) Boolean always evaluates to [True/False] */
#define CSMD_HAL_Enable_PLL( _Inst, _Enable ) \
  do { CSMD_BOOL boEnable = (_Enable); \
       if (boEnable) \
       { \
         *_Inst.prSERC_Reg->ulTCSR |= (CSMD_ULONG)(1 << CSMD_HAL_TCSR_CON_PLL_EN); \
       } \
       else \
       { \
         *_Inst.prSERC_Reg->ulTCSR &= ~(CSMD_ULONG)(1 << CSMD_HAL_TCSR_CON_PLL_EN); \
       } \
  } while (0)



/**************************************************************************/ /**
\todo review
\def CSMD_HAL_Write_PLL_Control( _Inst, _Control )

\b HAL-Function CSMD_VOID CSMD_HAL_Write_PLL_Control( CSMD_HAL   *prCSMD_HAL,
                                                      CSMD_ULONG  ulPLL_Control )

\brief Set a command to control the PLL functionality.

\b Description  Write a command into the PLLCSR to control the PLL..

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst     base address of the CoSeMa HAL structure
\param [in]   _Control  PLL command
                        - CSMD_HAL_PLL_START_SYNC   stops the synchronization
                        - CSMD_HAL_PLL_STOP_SYNC    starts the synchronization

\return       none

\author       WK
\date         17.11.2011

***************************************************************************** */
/*lint -emacro( {774}, CSMD_HAL_Write_PLL_Control ) Boolean always evaluates to [True/False] */
#define CSMD_HAL_Write_PLL_Control( _Inst, _Control ) \
  do { CSMD_ULONG ulControl = (_Control); \
       if (ulControl == CSMD_HAL_PLL_START_SYNC) \
       { \
         *_Inst.prSERC_Reg->ulPLLCSR = CSMD_HAL_PLL_START_SYNC; \
       } \
       else \
       { \
         *_Inst.prSERC_Reg->ulPLLCSR = CSMD_HAL_PLL_STOP_SYNC; \
       } \
  } while (0)
#endif  /* #ifdef CSMD_SYNC_DPLL */



/**************************************************************************/ /**

\def CSMD_HAL_SetPLLCycleTime( _Inst, _CycleTime )

\b HAL-Function CSMD_VOID CSMD_HAL_SetPLLCycleTime( CSMD_HAL   *prCSMD_HAL,
                                                    CSMD_ULONG  ulCycleTime )

\brief Writes Sercos Cycle Time for PLL into FPGA DECR register.

       Also necessary for 
       - FPGA activity LED control
       - Line break sensitivity
       - Reload condition for main timer in master mode.
       The register must be set before first sending Sercos telegrams.

\b Description

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst        base address of the CoSeMa HAL structure
\param [in]   _CycleTime   TCNT cycle time [ns]

\return       none

\author       WK
\date         11.06.2010

***************************************************************************** */
#define CSMD_HAL_SetPLLCycleTime( _Inst, _CycleTime )\
            do { *_Inst.prSERC_Reg->ulTCNTCYCR =\
                    CSMD_END_CONV_L( (_CycleTime) ); } while (0)


/**************************************************************************/ /**

\def CSMD_HAL_SoftwareReset( _Inst )

\b HAL-Function: CSMD_VOID CSMD_HAL_SoftwareReset( CSMD_HAL *prCSMD_HAL )

\b Description:  resets hardware by performing a software reset.
                 Hardware is re-initialized, reset is cleared automatically.
 
<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\return       NONE

\author       KP
\date         13.08.2007 

***************************************************************************** */
#define CSMD_HAL_SoftwareReset( _Inst )\
            do { *_Inst.prSERC_Reg->ulGCSFR |=\
                    (CSMD_ULONG)(1 << CSMD_HAL_GCSFR_SOFT_RESET); } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_Initialize(_Inst)

\b HAL-Function: CSMD_VOID CSMD_HAL_Initialize( CSMD_HAL *prCSMD_HAL )

\b Description: Initialization of HAL-specific variables.
                All variables, registers etc. defined for the HAL can be
                initialized by this macro.
 
<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure
 
\return       NONE

\author       KP
\date         13.09.2007 

***************************************************************************** */
#define CSMD_HAL_Initialize(_Inst)\
            do { *_Inst.prSERC_Reg->ulTCSR     &= (1U << CSMD_HAL_TCSR_EST);\
                 *_Inst.prSERC_Reg->ulTCYCSTART = 0UL; } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_SetPhase( _Inst, _PhaseSwitch, _Phase )

\b HAL-Function: CSMD_VOID CSMD_HAL_SetPhase( CSMD_HAL    *prCSMD_HAL,
                                              CSMD_USHORT  usPhaseSwitch,
                                              CSMD_USHORT  usPhase )

\b Description: The communication hardware gets the communication phase switch
                status and the new communication phase. This information is
                required for construction of MST.
                 
<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure

\param [in]   _PhaseSwitch  0 = communication phase switch inactive
                            1 = communication phase switch active

\param [in]   _Phase        new communication phase (CP0 to CP4)
 
\return       NONE

\author       KP
\date         13.08.2007 

***************************************************************************** */
/*lint -emacro( {845}, CSMD_HAL_SetPhase ) The right argument to operator '|' is certain to be 0 */
#define CSMD_HAL_SetPhase(_Inst, _PhaseSwitch, _Phase)\
            do { *_Inst.prSERC_Reg->ulPHASECR = \
                 (  *_Inst.prSERC_Reg->ulPHASECR\
                  & ((CSMD_ULONG) ~(  (1UL << CSMD_HAL_PHASECR_PS)\
                                    | CSMD_HAL_PHASECR_PHASE_MASK)))\
                  | (CSMD_ULONG)(  (((CSMD_ULONG)(_PhaseSwitch)) << CSMD_HAL_PHASECR_PS)\
                                 | (((CSMD_ULONG)(_Phase))<< CSMD_HAL_PHASECR_PHASE_SHIFT) ); } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_IsRunning_TCNT (_Inst )

\b HAL-Function: CSMD_BOOL CSMD_HAL_IsRunning_TCNT ( CSMD_HAL *prCSMD_HAL )

\brief Provides information if timer TCNT is running or not.

\b Description:

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\return       TRUE  = Timer TCNT is halted or not triggered yet.\n
              FALSE = Timer TCNT was triggered by CYCCLK event.

\author       WK
\date         24.03.2010

***************************************************************************** */
#define CSMD_HAL_IsRunning_TCNT( _Inst ) \
            ((*_Inst.prSERC_Reg->ulTCSR & (1 << CSMD_HAL_TCSR_TCNT_RUNNING)) != 0)



/**************************************************************************/ /**

\def CSMD_HAL_Retrigger_TCNT(_Inst)
 
\b HAL-Function CSMD_VOID CSMD_HAL_Retrigger_TCNT( CSMD_HAL *prCSMD_HAL );

\brief Re-trigger the main TCNT timer in FPGA.

\b Description:
                - Triggers the TCNT cycle counter once (software trigger).

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\return       None

\author       WK
\date         12.05.2010

***************************************************************************** */
#define CSMD_HAL_Retrigger_TCNT(_Inst)\
            do { *_Inst.prSERC_Reg->ulTCSR |=\
                    (1UL << CSMD_HAL_TCSR_CYCST); } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_SetLineBreakSensitivity(_Inst, _Value) 

\b HAL-Function: CSMD_VOID CSMD_HAL_SetLineBreakSensitivity( CSMD_HAL    *prCSMD_HAL,
                                                             CSMD_USHORT  usValue )

\b Description:    

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure

\param [in]   _Value   
 
\return       NONE

\author       KP
\date         13.09.2007 

***************************************************************************** */
#define CSMD_HAL_SetLineBreakSensitivity(_Inst, _Value)\
            do { *_Inst.prSERC_Reg->ulGCSFR =\
                 (  (*_Inst.prSERC_Reg->ulGCSFR & ~(CSMD_ULONG) CSMD_HAL_GCSFR_LINEBREAK_SENS)\
                  | ((CSMD_ULONG) (_Value) << CSMD_HAL_GCSFR_SHIFT_LINE_BR_SENS )); } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_Enable_SERCOS_Time(_Inst, _Enable )
 
\b HAL-Function CSMD_VOID CSMD_HAL_Enable_SERCOS_Time( CSMD_HAL  *prCSMD_HAL,
                                                       CSMD_BOOL  boST_Enable )

\brief Enables insertion of Sercos time into the telegram EF field.

\b Description:
                - 

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst        base address of the CoSeMa HAL structure
\param [in]   _Enable
                           - TRUE  : Enable insertion of Sercos time.
                           - FALSE : Disable insertion of Sercos time.
\return       None

\author       WK
\date         14.07.2010

***************************************************************************** */
/*lint -emacro( {774}, CSMD_HAL_Enable_SERCOS_Time ) Boolean always evaluates to [True/False] */
#define CSMD_HAL_Enable_SERCOS_Time( _Inst, _Enable ) \
  do { CSMD_BOOL boEnable = (_Enable); \
       if (boEnable) \
       { /* enable insertion of Sercos time */ \
         *_Inst.prSERC_Reg->rDFCSR.ulLong |= (CSMD_ULONG)(1 << CSMD_HAL_DFCSR_ENABLE_TX_SERCOSTIME); \
       } \
       else \
       { /* disable insertion of Sercos time */ \
         *_Inst.prSERC_Reg->rDFCSR.ulLong &= ~(CSMD_ULONG)(1 << CSMD_HAL_DFCSR_ENABLE_TX_SERCOSTIME); \
       } \
  } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_GetTelegramStatusP1( _Inst )

\b HAL-Function: CSMD_ULONG CSMD_HAL_GetTelegramStatusP1( CSMD_HAL *prCSMD_HAL )

\b Description: Reads the telegram status register port 1 \n

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure

\return       CSMD_ULONG  Content of the telegram status register port 1 (TGSR1)\n
                     of the IP-Core.

\author       WK
\date         12.09.2012

***************************************************************************** */
#define CSMD_HAL_GetTelegramStatusP1( _Inst )\
            CSMD_END_CONV_L( (*_Inst.prSERC_Reg->ulTGSR1) )



/**************************************************************************/ /**

\def CSMD_HAL_GetTelegramStatusP1( _Inst )

\b HAL-Function: CSMD_ULONG CSMD_HAL_GetTelegramStatusP1( CSMD_HAL *prCSMD_HAL )

\b Description: Reads the telegram status register port 1 \n

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure

\return       CSMD_ULONG  Content of the telegram status register port 1 (TGSR1)\n
                     of the IP-Core.

\author       WK
\date         12.09.2012

***************************************************************************** */
#define CSMD_HAL_GetTelegramStatusP2( _Inst )\
            CSMD_END_CONV_L( (*_Inst.prSERC_Reg->ulTGSR2) )



/**************************************************************************/ /**

\def CSMD_HAL_ClearTelegramStatusP1( _Inst, _ClrStatusMask )

\b HAL-Function: CSMD_VOID CSMD_HAL_ClearTelegramStatusP1( CSMD_HAL   *prCSMD_HAL,
                                                           CSMD_ULONG  ulClrStatusMask )

\b Description: Clears the bits of the telegram status register port 1 \n
                given by the bits set in the ClrStatusMask.

    Set bit | Clears
   -------: | :--------------------
          0 | all MDT Bits
          4 | all AT Bits
          8 | MST Valid Bit
         10 | MST Window Error Bit
         11 | MST Miss Bit
         12 | MST Double Miss Bit
         13 | AT0 miss Bit

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure

\param [in]   _ClrStatusMask
              Mask with status information to be cleared

\return       none

\author       WK
\date         12.09.2012
 
***************************************************************************** */
#ifdef CSMD_SOFT_MASTER
#define CSMD_HAL_ClearTelegramStatusP1( _Inst, _ClrStatusMask )\
          do{ if (CSMD_HAL_IsSoftMaster( _Inst ) == TRUE)\
              {\
                *_Inst.prSERC_Reg->ulTGSR1 &=\
                  CSMD_END_CONV_L( ~((_ClrStatusMask) & CSMD_HAL_TGSR_ALL_TEL) );\
              }\
              else\
              {\
                *_Inst.prSERC_Reg->ulTGSR1 =\
                  CSMD_END_CONV_L( (_ClrStatusMask) );\
              }\
          } while (0)
#else
#define CSMD_HAL_ClearTelegramStatusP1( _Inst, _ClrStatusMask )\
            do { *_Inst.prSERC_Reg->ulTGSR1 =\
                   CSMD_END_CONV_L( (_ClrStatusMask) );} while (0)
#endif



/**************************************************************************/ /**

\def CSMD_HAL_ClearTelegramStatusP2( _Inst, _ClrStatusMask )

\b HAL-Function: CSMD_VOID CSMD_HAL_ClearTelegramStatusP2( CSMD_HAL   *prCSMD_HAL,
                                                           CSMD_ULONG  ulClrStatusMask )

\b Description: Clears the bits of the telegram status register port 2 \n
                given by the bits set in the ClrStatusMask.

    Set bit | Clears
   -------: | :--------------------
          0 | all MDT Bits
          4 | all AT Bits
          8 | MST Valid Bit
         10 | MST Window Error Bit
         11 | MST Miss Bit
         12 | MST Double Miss Bit
         13 | AT0 miss Bit

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure

\param [in]   _ClrStatusMask
              Mask with status information to be cleared

\return       none

\author       WK
\date         12.09.2012
 
***************************************************************************** */
#ifdef CSMD_SOFT_MASTER
#define CSMD_HAL_ClearTelegramStatusP2( _Inst, _ClrStatusMask )\
          do{ if (CSMD_HAL_IsSoftMaster( _Inst ) == TRUE)\
              {\
                *_Inst.prSERC_Reg->ulTGSR2 &=\
                  CSMD_END_CONV_L( ~((_ClrStatusMask) & CSMD_HAL_TGSR_ALL_TEL) );\
              }\
              else\
              {\
                *_Inst.prSERC_Reg->ulTGSR2 =\
                  CSMD_END_CONV_L( (_ClrStatusMask) );\
              }\
          } while (0)
#else
#define CSMD_HAL_ClearTelegramStatusP2( _Inst, _ClrStatusMask )\
            do { *_Inst.prSERC_Reg->ulTGSR2 =\
                   CSMD_END_CONV_L( (_ClrStatusMask) );} while (0)
#endif



/**************************************************************************/ /**

\def CSMD_HAL_GetValidTelegramsSysA( _Inst )

\b HAL-Function: CSMD_ULONG CSMD_HAL_GetValidTelegramsSysA( CSMD_HAL *prCSMD_HAL )

\b Description: Gets the valid telegrams of both ports for currently active Rx buffer.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\return       CSMD_ULONG

\author       AlM
\date         15.05.2014

***************************************************************************** */
#define CSMD_HAL_GetValidTelegramsSysA( _Inst )\
            CSMD_END_CONV_L(( *_Inst.prSERC_Reg->ulRXBUFTV_A))



/**************************************************************************/ /**

\def CSMD_HAL_GetLineStatus ( _Inst ) 

\b HAL-Function: CSMD_ULONG CSMD_HAL_GetLineStatus( CSMD_HAL *prCSMD_HAL )

\b Description: Reads line status information from both ports.
                (hold until firmware acknowledgment)

\param [in]   _Inst   base address of the CoSeMa HAL structure
 
\return       CSMD_ULONG  Bit[0] ='1' -> Port1 error detected, '0'-> Port1 no error on line\n
                     Bit[1] ='1' -> Port2 error detected, '0'-> Port2 no error on line\n

\author       AlM
\date         31.05.2012

**************************************************************************** */
#define CSMD_HAL_GetLineStatus(_Inst)\
            (((CSMD_ULONG) (  *_Inst.prSERC_Reg->rDFCSR.ulLong\
                            & (CSMD_ULONG)(  (1 << CSMD_HAL_DFCSR_LINE_STS_DELAY_P1)\
                                           | (1 << CSMD_HAL_DFCSR_LINE_STS_DELAY_P2)))) >> CSMD_HAL_DFCSR_LINE_STS_DELAY_SHIFT)



/**************************************************************************/ /*
\def CSMD_HAL_GetLinkStatus( _Inst ) 

\b HAL-Function: CSMD_ULONG CSMD_HAL_GetLinkStatus( CSMD_HAL *prCSMD_HAL )

\b Description: Reads link status information from both ports.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure
 
\return       CSMD_ULONG   Bit[0] ='1' -> Port1 LINK, '0'-> Port1 NO LINK
                      Bit[1] ='1' -> Port2 LINK, '0'-> Port2 NO LINK

\author       KP
\date         13.08.2007 

***************************************************************************** */
#define CSMD_HAL_GetLinkStatus(_Inst)\
            (((CSMD_ULONG) (  *_Inst.prSERC_Reg->rDFCSR.ulLong\
                            & (CSMD_ULONG)(  (1 << CSMD_HAL_DFCSR_LINK_P1)\
                                           | (1 << CSMD_HAL_DFCSR_LINK_P2)))) >> CSMD_HAL_DFCSR_LINK_SHIFT)



/**************************************************************************/ /**

\def CSMD_HAL_ResetLinestatus ( _Inst )

\b HAL-Function: CSMD_VOID CSMD_HAL_ResetLineStatus( CSMD
,+

 _HAL prCSMD_HAL )

\b Description: Reset all bits of the DFCSR status.

\param [in]   _Inst   base address of the CoSeMa HAL structure
 
\return       NONE

\author       AlM
\date         25.05.2012 

***************************************************************************** */
#ifdef CSMD_SOFT_MASTER
#define CSMD_HAL_ResetLineStatus(_Inst)\
          do{ if (CSMD_HAL_IsSoftMaster( _Inst ) == TRUE)\
              {\
                *_Inst.prSERC_Reg->rDFCSR.ulLong &=\
                  ~(CSMD_ULONG)(  (1 << CSMD_HAL_DFCSR_LINE_STS_DELAY_P1)\
                                | (1 << CSMD_HAL_DFCSR_LINE_STS_DELAY_P2));\
              }\
              else\
              {\
                *_Inst.prSERC_Reg->rDFCSR.ulLong = *_Inst.prSERC_Reg->rDFCSR.ulLong;\
              }\
          } while (0)
#else
#define CSMD_HAL_ResetLineStatus(_Inst)\
            do { *_Inst.prSERC_Reg->rDFCSR.ulLong = *_Inst.prSERC_Reg->rDFCSR.ulLong; } while(0)
#endif



/**************************************************************************/ /**

\def CSMD_HAL_GetSystemTimer( _Inst )

\b HAL-Function: CSMD_ULONG CSMD_HAL_GetSystemTimer( CSMD_HAL *prCSMD_HAL ) 

\b Description: Gets current system timer. 

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure
 
\return       CSMD_ULONG

\author       KP
\date         13.08.2007 
 
***************************************************************************** */
#define CSMD_HAL_GetSystemTimer( _Inst )\
            CSMD_END_CONV_L(( *_Inst.prSERC_Reg->ulSTRBR))



#ifdef CSMD_HW_WATCHDOG
/**************************************************************************/ /**

\def CSMD_HAL_WatchdogTrigger( _Inst )

\b HAL-Function: CSMD_VOID CSMD_HAL_WatchdogTrigger( CSMD_HAL *prCSMD_HAL ) 

\b Description: Triggers hardware watchdog by writing 0x88CD to low word
                of register WDCSR.
                 
<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure
 
\return       None

\author       AM
\date         09.07.2010

***************************************************************************** */
#define CSMD_HAL_WatchdogTrigger( _Inst ) \
            do { if (TRUE == *_Inst.boWD_Active) \
                 { \
                   *_Inst.prSERC_Reg->rWDCSR.rShort.usMagicPattern = CSMD_HAL_WD_MAGIC_PATTERN; \
                 } \
            } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_WatchdogSet( _Inst, _Activate )

\b HAL-Function: CSMD_VOID CSMD_HAL_WatchdogSet( CSMD_HAL  *prCSMD_HAL,
                                                 CSMD_BOOL  boActivate )

\b Description: Activates or deactivates hardware watchdog according to 
                transferred value.
                 FALSE = deactivation
                 TRUE = activation

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\param [in]   _Activate

\return       None

\author       AM
\date         09.07.2010

***************************************************************************** */
/*lint -emacro( {774}, CSMD_HAL_WatchdogSet ) Boolean always evaluates to [True/False] */
#define CSMD_HAL_WatchdogSet( _Inst, _Activate ) \
  do { CSMD_BOOL boActivate = (_Activate); \
       if (boActivate) \
       { \
         *_Inst.prSERC_Reg->rWDCSR.rShort.usMagicPattern = (CSMD_USHORT)CSMD_HAL_WD_MAGIC_PATTERN; \
         *_Inst.boWD_Active = TRUE; \
       } \
       else \
       { \
         *_Inst.prSERC_Reg->rWDCSR.rShort.usMagicPattern = (CSMD_USHORT)~CSMD_HAL_WD_MAGIC_PATTERN; \
         *_Inst.boWD_Active = FALSE; \
       } \
  } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_WatchdogMode( _Inst, _Mode )

\b HAL-Function: CSMD_VOID CSMD_HAL_WatchdogMode( CSMD_HAL    *prCSMD_HAL,
                                                  CSMD_USHORT  usWD_Mode )

\b Description: Selects the reaction to an watchdog timeout.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\param [in]   _Mode
               CSMD_HAL_WD_TO_DISABLE_TX_TEL
               CSMD_HAL_WD_TO_SEND_EMPTY_TEL

\return       none

\author       WK
\date         03.01.2013

***************************************************************************** */
#define CSMD_HAL_WatchdogMode( _Inst, _Mode ) \
  do { CSMD_USHORT usWD_Mode = (_Mode); \
       if (usWD_Mode == CSMD_HAL_WD_TO_DISABLE_TX_TEL) \
       { \
         *_Inst.prSERC_Reg->rWDCSR.rShort.usControlStatus = CSMD_HAL_WDCSR_MODE_DIS_TEL; \
       } \
       else \
       { \
         *_Inst.prSERC_Reg->rWDCSR.rShort.usControlStatus = 0U; \
       } \
  } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_WatchdogConfigure( _Inst, _Cycles )

\b HAL-Function: CSMD_VOID CSMD_HAL_WatchdogConfigure( CSMD_HAL    *prCSMD_HAL,
                                                       CSMD_USHORT  usCycles )

\b Description: Writes number of cycles required for watchdog action
                (initial count of watchdog) to low word of register WDCNT.
                 
<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\param [in]   _Cycles

\return       None

\author       AM
\date         09.07.2010

***************************************************************************** */
#define CSMD_HAL_WatchdogConfigure( _Inst, _Cycles )\
            do { *_Inst.prSERC_Reg->rWDCNT.ulCounter =\
                    CSMD_END_CONV_L( (CSMD_ULONG)(_Cycles) ); } while (0)

#endif  /* #ifdef CSMD_HW_WATCHDOG */



/**************************************************************************/ /**

\def CSMD_HAL_Set_MTU( _Inst, _MTU )

\b HAL-Function: CSMD_VOID CSMD_HAL_Set_MTU( CSMD_HAL    *prCSMD_HAL,
                                             CSMD_USHORT  usMTU )

\b Description: Writes the Maximum Transmission Unit (MTU) into the
                IPLASTFL register.
                 
<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\param [in]   _MTU   Maximum Transmission Unit

\return       None

\author       WK
\date         16.11.2011

***************************************************************************** */
#define CSMD_HAL_Set_MTU( _Inst, _MTU )\
            do { *_Inst.prSERC_Reg->ulIPLASTFL =\
                    CSMD_END_CONV_L( (_MTU) ); } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_SetComMode( _Inst, _Mode )

\b HAL-Function: CSMD_VOID CSMD_HAL_SetComMode( CSMD_HAL    *prCSMD_HAL,
                                                CSMD_USHORT  usMode )

\b Description: This function defines the communication mode for either 
                 realtime (cyclic) or non-realtime (IP) data transfer.
                 
<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\param [in]   _Mode
                   - 0 = Real time mode       line P1 + P2
                   - 1 = Real time mode       line P2
                   - 2 = Real time mode       line P1
                   - 3 = Non real time mode   line
                   - 4 = Real time mode       ring  (collision buffer disabled)
                   - 7 = Non real time mode   ring  (collision buffer disabled)

\return       NONE

\author       KP
\date         13.08.2007 

***************************************************************************** */
/*lint -emacro( {845}, CSMD_HAL_SetComMode ) The right argument to operator '|' is certain to be 0 */
#ifdef CSMD_SOFT_MASTER
#define CSMD_HAL_SetComMode( _Inst, _Mode )\
    do{ if (CSMD_HAL_IsSoftMaster( _Inst ) == TRUE)\
        {\
          *_Inst.prSERC_Reg->rDFCSR.ulLong =\
            (  (  *_Inst.prSERC_Reg->rDFCSR.ulLong & ~(  (CSMD_ULONG)CSMD_HAL_DFCSR_TOPOLOGY\
                                                       | ((CSMD_ULONG)(1 << CSMD_HAL_DFCSR_LINE_STS_DELAY_P1))\
                                                       | ((CSMD_ULONG)(1 << CSMD_HAL_DFCSR_LINE_STS_DELAY_P2))) )\
             | ((CSMD_ULONG)(_Mode) & ((CSMD_ULONG)CSMD_HAL_DFCSR_TOPOLOGY)) );\
        }\
        else\
        {\
          *_Inst.prSERC_Reg->rDFCSR.ulLong =\
            (  (*_Inst.prSERC_Reg->rDFCSR.ulLong & ~((CSMD_ULONG)CSMD_HAL_DFCSR_TOPOLOGY))\
             | (  ((CSMD_ULONG)(1 << CSMD_HAL_DFCSR_LINE_STS_DELAY_P1))\
                | ((CSMD_ULONG)(1 << CSMD_HAL_DFCSR_LINE_STS_DELAY_P2))\
                | ((CSMD_ULONG)(_Mode) & ((CSMD_ULONG)CSMD_HAL_DFCSR_TOPOLOGY)) ));\
        }\
    } while (0)
#else
#define CSMD_HAL_SetComMode( _Inst, _Mode )\
            do { *_Inst.prSERC_Reg->rDFCSR.ulLong =\
                (  (*_Inst.prSERC_Reg->rDFCSR.ulLong & ~((CSMD_ULONG)CSMD_HAL_DFCSR_TOPOLOGY))\
                 | (  ((CSMD_ULONG)(1 << CSMD_HAL_DFCSR_LINE_STS_DELAY_P1))\
                    | ((CSMD_ULONG)(1 << CSMD_HAL_DFCSR_LINE_STS_DELAY_P2))\
                    | ((CSMD_ULONG)(_Mode) & ((CSMD_ULONG)CSMD_HAL_DFCSR_TOPOLOGY)) )) ; } while(0)
#endif



/**************************************************************************/ /**

\def CSMD_HAL_GetComMode( _Inst )

\b HAL-Function: CSMD_ULONG CSMD_HAL_GetComMode( CSMD_HAL *prCSMD_HAL )

\b Description: Reads communication mode.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure
 
\return       CSMD_ULONG Communication mode

\author       WK
\date         14.01.2008

***************************************************************************** */
#define CSMD_HAL_GetComMode(_Inst)\
            ( *_Inst.prSERC_Reg->rDFCSR.ulLong &\
                 (CSMD_ULONG)CSMD_HAL_DFCSR_TOPOLOGY )



/**************************************************************************/ /**

\def CSMD_HAL_SetSVCLastAT( _Inst, _TrigSlctAT )

\b HAL-Function: CSMD_VOID CSMD_HAL_SetSVCLastAT( CSMD_HAL    *prCSMD_HAL,
                                                  CSMD_USHORT  usTrigSelectAT )

\b Description: AT selection for SVC machine trigger.

                 AT0/AT1 is the last SVC telegram, trigger svc state machine 
                 after reception of AT0/AT1.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\param [in]   _TrigSlctAT    0 = AT0, 1 = AT1, 2 = AT2, 3 = AT3
 
\return       NONE

\author       KP
\date         13.08.2007 

***************************************************************************** */
#define CSMD_HAL_SetSVCLastAT(_Inst, _TrigSlctAT)\
            do { *_Inst.prSERC_Reg->ulSVCCSR = \
                 (*_Inst.prSERC_Reg->ulSVCCSR  & ~(CSMD_ULONG) CSMD_HAL_SVCCSR_AT_SELECT) \
                  | (CSMD_ULONG) ((_TrigSlctAT) << CSMD_HAL_SVCCSR_AT_SELECT_SHIFT); } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_CtrlSVCMachine( _Inst, _SVCEnable ) 

\b HAL-Function: CSMD_VOID CSMD_HAL_CtrlSVCMachine( CSMD_HAL  *prCSMD_HAL,
                                                    CSMD_BOOL  boSVCEnable )

\b Description: Activates or deactivates SVC machine operations.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\param [in]   _SVCEnable  TRUE  -> activates SVC machine
                          FALSE -> deactivates SVC machine

\return       NONE

\author       KP
\date         13.08.2007 

***************************************************************************** */
/*lint -emacro( {774}, CSMD_HAL_CtrlSVCMachine ) Boolean always evaluates to [True/False] */
#if (CSMD_MAX_HW_CONTAINER > 0)

#define CSMD_HAL_CtrlSVCMachine(_Inst, _SVCEnable) \
  do { CSMD_BOOL boSVCEnable = (_SVCEnable); \
       if (boSVCEnable) \
       { /* Enable SVC operation */ \
         *_Inst.prSERC_Reg->ulSVCCSR |= (CSMD_ULONG)(1 << CSMD_HAL_SVCCSR_ENABLE); \
       } \
       else \
       { /* Disable SVC operation */ \
         *_Inst.prSERC_Reg->ulSVCCSR &= ~(CSMD_ULONG)(1 << CSMD_HAL_SVCCSR_ENABLE); \
       } \
  } while (0)

#else

#define CSMD_HAL_CtrlSVCMachine(_Inst, _SVCEnable)

#endif



/**************************************************************************/ /**

\def CSMD_HAL_DisableSVCMachine ( _Inst ) 

\b HAL-Function: CSMD_VOID CSMD_HAL_DisableSVCMachine( CSMD_HAL *prCSMD_HAL )

\b Beschreibung: Disable the service channel state machine operation.

\param [in]   _Inst   base address of the CoSeMa HAL structure
 
\return       none

\author       WK
\date         11.07.2012

**************************************************************************** */
#define CSMD_HAL_DisableSVCMachine( _Inst  ) \
            do { *_Inst.prSERC_Reg->ulSVCCSR &= \
                 ~(CSMD_ULONG)(1 << CSMD_HAL_SVCCSR_ENABLE); } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_CtrlSVC_Redundancy( _Inst, _Enable ) 

\b HAL-Function: CSMD_VOID CSMD_HAL_CtrlSVC_Redundancy( CSMD_HAL  *prCSMD_HAL,
                                                        CSMD_BOOL  boEnable )

\b Description: Activates or deactivates SVC redundancy in the IP-Core.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst  base address of the CoSeMa HAL structure

\param [in]   _Enable  TRUE  -> activates SVC redundancy
                       FALSE -> deactivates SVC redundancy

\return       NONE

\author       WK
\date         05.09.2012

***************************************************************************** */
/*lint -emacro( {774}, CSMD_HAL_CtrlSVC_Redundancy ) Boolean always evaluates to [True/False] */
#define CSMD_HAL_CtrlSVC_Redundancy(_Inst, _Enable) \
  do { CSMD_BOOL boEnable = (_Enable); \
       if (boEnable) \
       { /* Enable SVC redundancy */ \
         *_Inst.prSERC_Reg->ulSVCCSR |= (  CSMD_HAL_SVCCSR_REDUNDANCY_MASK \
                                         | CSMD_HAL_SVCCSR_START_MODE_MASK); \
       } \
       else \
       { /* Disable SVC redundancy */ \
         *_Inst.prSERC_Reg->ulSVCCSR &= ~(  CSMD_HAL_SVCCSR_REDUNDANCY_MASK \
                                          | CSMD_HAL_SVCCSR_START_MODE_MASK); \
       } \
  } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_GetSVCBusyStatus( _Inst ) 

\b HAL-Function: CSMD_BOOL CSMD_HAL_CtrlSVCMachine( CSMD_HAL *prCSMD_HAL )

\b Description: Busy status of SVC status machine. 

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure

\return       TRUE  -> SVC-Machine is busy
              FALSE -> SVC-Machine is inactive

\author       KP
\date         13.08.2007 

***************************************************************************** */
#define CSMD_HAL_GetSVCBusyStatus(_Inst) \
            ((*_Inst.prSERC_Reg->ulSVCCSR & CSMD_HAL_SVCCSR_BUSY_BIT_MASK) != 0)



/**************************************************************************/ /**

\def CSMD_HAL_SetSVCTimeouts( _Inst, _BusyTMOut, _HSTMOut )

\b HAL-Function: CSMD_VOID CSMD_HAL_SetSVCTimeouts( CSMD_HAL    *prCSMD_HAL,
                                                    CSMD_USHORT  usBusyTimeout,
                                                    CSMD_USHORT  usHSTimeout )

\b Description: Sets SVC timeout values (handshake and busy timeout).  

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst       base address of the CoSeMa HAL structure

\param [in]   _BusyTMOut  busy timeout value.

\param [in]   _HSTMOut    handshake timeout value.
 
\return       NONE

\author       KP
\date         13.08.2007 

***************************************************************************** */
#define CSMD_HAL_SetSVCTimeouts( _Inst, _BusyTMOut, _HSTMOut )\
            do { *_Inst.prSERC_Reg->ulSVCCSR =\
                 (  (CSMD_ULONG) ( ((CSMD_ULONG)(_BusyTMOut) << CSMD_HAL_SVCCSR_BUSY_TO_SHIFT) & CSMD_HAL_SVCCSR_BUSY_TO_MASK )\
                  | (CSMD_ULONG) ( ((CSMD_ULONG)(_HSTMOut)   << CSMD_HAL_SVCCSR_BUSY_HS_SHIFT) & CSMD_HAL_SVCCSR_BUSY_HS_MASK )); } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_Usable_Tx_BufferSysA( _prCSMD_HAL ) gets active Tx buffer number
        of Tx buffer system A.

\b HAL-Function: CSMD_ULONG CSMD_HAL_Usable_Tx_BufferSysA( CSMD_HAL *prCSMD_HAL )

\b Description:
                - Returns the active Tx buffer number of system A that is
                  usable by the system (processor).

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _prCSMD_HAL   pointer to the CoSeMa HAL structure

\return       CSMD_ULONG     Active Tx buffer nbr. of system A to use:
                        - 0 = buffer 0
                        - 1 = buffer 1
                        - 2 = buffer 2
                        - 3 = buffer 3

\author       WK
\date         28.01.2011

***************************************************************************** */
#define CSMD_HAL_Usable_Tx_BufferSysA( _prCSMD_HAL )\
            (   (*_prCSMD_HAL.prSERC_Reg->ulTXBUFCSR_A & CSMD_HAL_TXBUFCSR_ACT_BUF_MASK)\
             >> CSMD_HAL_TXBUFCSR_ACT_BUF_SHIFT )



/**************************************************************************/ /**

\def CSMD_HAL_Request_New_Tx_BufferSysA( _prCSMD_HAL ) request new Tx buffer
        of Tx buffer system A.

\b HAL-Function: CSMD_VOID CSMD_HAL_Request_New_Tx_BufferSysA( CSMD_HAL *prCSMD_HAL )

\b Description:
                - Requests newest transmit buffer of Tx buffer system A.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _prCSMD_HAL   pointer to the CoSeMa HAL structure

\author       WK
\date         28.01.2011

***************************************************************************** */
#define CSMD_HAL_Request_New_Tx_BufferSysA( _prCSMD_HAL )\
            ( *_prCSMD_HAL.prSERC_Reg->ulTXBUFCSR_A\
              |= CSMD_HAL_TXBUFCSR_REQ_BUFFER )



/**************************************************************************/ /**

\def CSMD_HAL_Request_New_Rx_BufferSysA( _prCSMD_HAL ) request new Rx buffer
        of Rx buffer system A.

\b HAL-Function: CSMD_VOID CSMD_HAL_Request_New_Rx_BufferSysA( CSMD_HAL *prCSMD_HAL )

\b Description:
                - Requests newest receive buffer of Rx buffer system A.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _prCSMD_HAL   pointer to the CoSeMa HAL structure

\author       WK
\date         31.01.2011

***************************************************************************** */
#define CSMD_HAL_Request_New_Rx_BufferSysA( _prCSMD_HAL )\
            ( *_prCSMD_HAL.prSERC_Reg->ulRXBUFCSR_A\
              |= CSMD_HAL_RXBUFCSR_REQ_BUFFER )



/**************************************************************************/ /**
\def CSMD_HAL_ResetCounter_SercosError() resets all frame error counters.

\b HAL-Function: CSMD_VOID CSMD_HAL_ResetCounter_SercosError( CSMD_HAL *prCSMD_HAL )
\b Description: \n
   This function clear all IP-Core Ethernet frame error counters of both ports.

\note Any write access to these register clears always all error counters \n
      of both ports!

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]  _prCSMD_HAL   pointer to the CoSeMa HAL structure

\return       none

\author       wk
\date         21.11.2013

***************************************************************************** */
#define CSMD_HAL_ResetCounter_SercosError( _Inst  ) \
            do { *_Inst.prSERC_Reg->rIPSERCERR.ulErrCnt = 0UL; } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_ReadLong( _pVoid ) 

\b HAL-Function: CSMD_ULONG CSMD_HAL_ReadLong( CSMD_VOID *pvRdAddress )

\b Description: Reads an CSMD_ULONG data segment from memory.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _pVoid   read address
 
\return       CSMD_ULONG    Read data

\author       KP
\date         17.08.2007 

***************************************************************************** */
#define CSMD_HAL_ReadLong( _pVoid )\
            CSMD_END_CONV_L( (*((CSMD_HAL_SERCFPGA_DATTYP *)(_pVoid))) )



/**************************************************************************/ /**

\def CSMD_HAL_WriteLong( _pVoid, _val )

\b HAL-Function: CSMD_VOID CSMD_HAL_WriteLong( CSMD_VOID  *pvWrAddress,
                                               CSMD_ULONG  ulWriteData )

\b Description: Writes an CSMD_ULONG data segment to memory.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _pVoid    write address

\param [in]   _val      Data to be written

\return       NONE

\author       KP
\date         19.08.2007 

***************************************************************************** */
#define CSMD_HAL_WriteLong(_pVoid, _val)\
            do { *((CSMD_HAL_SERCFPGA_DATTYP *)(_pVoid)) = CSMD_END_CONV_L( ((CSMD_ULONG)(_val)) ); } while (0)
 


/**************************************************************************/ /**

\def CSMD_HAL_ReadShort( _pVoid ) 

\b HAL-Function: CSMD_USHORT CSMD_HAL_ReadShort( CSMD_VOID *pvRdAddress )

\b Description: Reads an CSMD_USHORT data segment from memory.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _pVoid   read address
 
\return       CSMD_USHORT   Read data

\author       KP
\date         17.08.2007 

***************************************************************************** */
#define CSMD_HAL_ReadShort(_pVoid)\
            CSMD_END_CONV_S( (*((CSMD_HAL_SERCFPGA_WDATTYP *)(_pVoid))) )



/**************************************************************************/ /**

\def CSMD_HAL_WriteShort(_pVoid, _val) 

\b HAL-Function: CSMD_HAL_WriteShort( CSMD_VOID   *pvWrAddress,
                                      CSMD_USHORT  usWriteData )

\b Description: Writes an CSMD_USHORT data segment to memory.

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _pVoid   write address

\param [in]   _val     Data to be written

\return       NONE

\author       KP
\date         19.08.2007 

***************************************************************************** */
#define CSMD_HAL_WriteShort(_pVoid, _val)\
            do { *((CSMD_HAL_SERCFPGA_WDATTYP *)(_pVoid)) = CSMD_END_CONV_S( ((CSMD_USHORT)(_val)) ); } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_ReadShortNoConv( _pVoid ) 

\b HAL-Function: CSMD_USHORT CSMD_HAL_ReadShortNoConv( CSMD_VOID *psRdAddress )

\b Description: Reads an CSMD_USHORT data segment from memory ignoring byte order.
                 
<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _pVoid   read address
 
\return       CSMD_USHORT   Read data

\author       WK
\date         22.07.2009

***************************************************************************** */
#define CSMD_HAL_ReadShortNoConv(_pVoid)\
             (*((CSMD_HAL_SERCFPGA_WDATTYP *)(_pVoid))) 



#if defined CSMD_BIG_ENDIAN || defined CSMD_TEST_BE
/**************************************************************************/ /**

\def CSMD_HAL_WriteShortNoConv( _pVoid, _val ) 

\b HAL-Function: CSMD_HAL_WriteShortNoConv( CSMD_VOID   *pvWrAddress,
                                            CSMD_USHORT  usWriteData )

\b Description: Writes an CSMD_USHORT data segment to memory ignoring byte order.
                 
<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _pVoid   write address

\param [in]   _val     Data to be written

\return       NONE

\author       WK
\date         22.07.2009 

***************************************************************************** */
#define CSMD_HAL_WriteShortNoConv(_pVoid, _val)\
            do { *((CSMD_HAL_SERCFPGA_WDATTYP *)(_pVoid)) = ((CSMD_USHORT)(_val)); } while (0)
#endif



/**************************************************************************/ /**

\def CSMD_HAL_SetDesIdxTableOffsTxRam( _Inst, _Offset ) 

\b HAL-Function: CSMD_VOID CSMD_HAL_SetDesIdxTableOffsTxRam( CSMD_HAL    *prCSMD_HAL,
                                                             CSMD_USHORT  usOffset )

\brief Writes Tx-Descriptor index table offset in TxRam into FPGA DECR register.

\b Description:

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure
\param [in]   _Offset

\return       none

\author       WK
\date         09.02.2010

***************************************************************************** */
#define CSMD_HAL_SetDesIdxTableOffsTxRam( _Inst, _Offset )\
            do { *_Inst.prSERC_Reg->rDECR.rDesIdx.usOffsetTxRam =\
                    CSMD_END_CONV_S( (_Offset) ); } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_SetDesIdxTableOffsRxRam( _Inst, _Offset )

\b HAL-Function: CSMD_VOID CSMD_HAL_SetDesIdxTableOffsRxRam( CSMD_HAL    *prCSMD_HAL,
                                                             CSMD_USHORT  usOffset )

\brief Writes Rx-Descriptor index table offset in RxRam into FPGA DECR register.

\b Description:

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure
\param [in]   _Offset

\return       none

\author       WK
\date         10.02.2010

***************************************************************************** */
#define CSMD_HAL_SetDesIdxTableOffsRxRam( _Inst, _Offset )\
            do { *_Inst.prSERC_Reg->rDECR.rDesIdx.usOffsetRxRam =\
                    CSMD_END_CONV_S( (_Offset) ); } while (0)



/**************************************************************************/ /**

\def CSMD_HAL_GetTSrefCounter( _Inst)

\b HAL-Function: CSMD_VOID CSMD_HAL_GetTSrefCounter  ( CSMD_HAL *prCSMD_HAL )

\brief This macro equals the current value of TSref counter.

\b Description:

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure

\return       none

\author       AlM
\date         09.01.2014

***************************************************************************** */
#define CSMD_HAL_GetTSrefCounter( _Inst )\
            CSMD_END_CONV_S( (*_Inst.prSERC_Reg->rSCCMDT.rSCCNT.usSccReadBack) )


/**************************************************************************/ /**

\def CSMD_HAL_SetTSrefMax( _Inst)

\b HAL-Function: CSMD_VOID CSMD_HAL_SetTSrefMax  ( CSMD_HAL    *prCSMD_HAL,
                                                   CSMD_USHORT  usTSrefMax )

\brief This macro writes the maximum value of TSref to SCCMDT register.

\b Description:

<B>Call Environment:</B> \n
   This is a CoSeMa-private function macro.

\param [in]   _Inst   base address of the CoSeMa HAL structure

\param [in]   _Max    maximum value of TSref counter

\return       none

\author       AlM
\date         21.03.2014

***************************************************************************** */
#define CSMD_HAL_SetTSrefMax( _Inst, _Max )\
          do { *_Inst.prSERC_Reg->rSCCMDT.rSCCNT.usScCount =\
                CSMD_END_CONV_S( (_Max) ); } while (0)



/*---- Declaration private Types: --------------------------------------------*/

/* -------------------------------------------------------------------------- */
/*! \brief Timer Event data structure                                         */
/* -------------------------------------------------------------------------- */
typedef struct CSMD_HAL_EVENT_STR
{
  CSMD_HAL_SERCFPGA_DATTYP    ulTime;         /*!< Event TCNT Value [ns]    */
  CSMD_HAL_SERCFPGA_WDATTYP   usSubCycCnt;    /*!< Sub Cycle Count Value    */
  CSMD_HAL_SERCFPGA_WDATTYP   usType;         /*!< Event Type               */
  CSMD_HAL_SERCFPGA_WDATTYP   usSubCycCntSel; /*!< Sub Cycle Counter Select */
  
} CSMD_HAL_EVENT;


/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

SOURCE CSMD_VOID CSMD_HAL_GetIdentification
                                ( CSMD_HAL              *prCSMD_HAL, 
                                  CSMD_USHORT           *pusRelease,
                                  CSMD_USHORT           *pusVersion,
                                  CSMD_USHORT           *pusTestversion,
                                  CSMD_USHORT           *pusDeviceIdent,
                                  CSMD_USHORT           *pusSERCDeviceTyp );

SOURCE CSMD_VOID CSMD_HAL_SetInterFrameGap
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_ULONG             ulNbrBytes );

SOURCE CSMD_VOID CSMD_HAL_SetTimerEvent
                                ( CSMD_HAL              *prCSMD_HAL, 
                                  CSMD_USHORT            usIdx,
                                  CSMD_ULONG             ulTime,
                                  CSMD_USHORT            usSubCycCnt,
                                  CSMD_USHORT            usType,
                                  CSMD_USHORT            usSubCycCntSel );

SOURCE CSMD_VOID CSMD_HAL_SetPortEvent
                                ( CSMD_HAL              *prCSMD_HAL, 
                                  CSMD_USHORT            usIdx,
                                  CSMD_ULONG             ulTime,
                                  CSMD_USHORT            usSubCycCnt,
                                  CSMD_USHORT            usType,
                                  CSMD_USHORT            usSubCycCntSel );

SOURCE CSMD_BOOL CSMD_HAL_CheckVersion
                                ( CSMD_HAL              *prCSMD_HAL );
 
SOURCE CSMD_VOID CSMD_HAL_SoftReset
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_BOOL              boSoftwareReset,
                                  CSMD_BOOL              boPHY_Reset );

SOURCE CSMD_VOID CSMD_HAL_StatusSoftReset
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_BOOL             *pboSoftwareReset,
                                  CSMD_BOOL             *pboPHY_Reset );

SOURCE CSMD_VOID CSMD_HAL_IntControl
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_ULONG             ulIntEnable,
                                  CSMD_BOOL              boSelEventIntTyp,
                                  CSMD_BOOL              boSelSVCIntTyp,
                                  CSMD_ULONG             ulIntOutputMask );

SOURCE CSMD_VOID CSMD_HAL_GetIntControl
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_ULONG            *pulIntEnable,
                                  CSMD_BOOL              boSelEventIntTyp,
                                  CSMD_BOOL              boSelSVCIntTyp,
                                  CSMD_ULONG            *pulIntOutputMask );

SOURCE CSMD_ULONG CSMD_HAL_GetInterrupt
                                ( CSMD_HAL              *prCSMD_HAL, 
                                  CSMD_BOOL              boSelEventIntTyp,
                                  CSMD_BOOL              boSelSVCIntTyp );

SOURCE CSMD_VOID CSMD_HAL_ClearInterrupt
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_BOOL              boSelEventIntTyp,
                                  CSMD_BOOL              boSelSVCIntTyp,
                                  CSMD_ULONG             ulIntClearMask );

SOURCE CSMD_VOID CSMD_HAL_CtrlCYCCLKInput
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_BOOL              boEnable );

SOURCE CSMD_VOID CSMD_HAL_ConfigCONCLK
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_BOOL              boActivate,
                                  CSMD_BOOL              boEnableDriver,
                                  CSMD_BOOL              boPolarity );

SOURCE CSMD_ULONG CSMD_HAL_ConfigDIVCLK
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_BOOL              boActivate,
                                  CSMD_BOOL              boMode,
                                  CSMD_BOOL              boPolarity,
                                  CSMD_BOOL              boOutpDisable,
                                  CSMD_USHORT            usNbrPulses,
                                  CSMD_ULONG             ulPulseDistance,
                                  CSMD_ULONG             ulStartDelay );

SOURCE CSMD_VOID CSMD_HAL_ConfigCYCCLK
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_BOOL              boActivate,
                                  CSMD_BOOL              boEnableInput,
                                  CSMD_BOOL              boPolarity,
                                  CSMD_ULONG             ulStartDelay );

SOURCE CSMD_VOID CSMD_HAL_EnableTelegrams
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_ULONG             ulMDT_Enable,
                                  CSMD_ULONG             ulAT_Enable );

SOURCE CSMD_VOID CSMD_HAL_CtrlSystemCounter
                                ( CSMD_HAL              *prCSMD_HAL, 
                                  CSMD_BOOL              boEnableTimer0,
                                  CSMD_BOOL              boEnableTimer1,
                                  CSMD_BOOL              boEnableTimer2 );

SOURCE CSMD_VOID CSMD_HAL_CtrlDescriptorUnit
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_BOOL              boTxEnable,
                                  CSMD_BOOL              boRxEnable );

SOURCE CSMD_VOID CSMD_HAL_SetSercosTime
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_ULONG             ulNanos,
                                  CSMD_ULONG             ulSeconds,
                                  CSMD_ULONG             ulNanos_plus,
                                  CSMD_ULONG             ulSeconds_plus );

SOURCE CSMD_VOID CSMD_HAL_SetSercosTimeExtSync
                                ( const CSMD_HAL        *prCSMD_HAL,
                                  CSMD_ULONG             ulNanos,
                                  CSMD_ULONG             ulSeconds,
                                  CSMD_ULONG             ulNanos_Offset );

SOURCE CSMD_VOID CSMD_HAL_ReadSercosTime
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_ULONG            *pulNsAdd,
                                  CSMD_ULONG            *pulSecsAdd );

#ifdef CSMD_HW_WATCHDOG
SOURCE CSMD_VOID CSMD_HAL_WatchdogStatus
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_HAL_WDSTATUS     *prWdStatus );
#endif

SOURCE CSMD_VOID CSMD_HAL_GetPhase
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_USHORT           *pusPhase,
                                  CSMD_USHORT           *pusPhaseSwitch );

SOURCE CSMD_VOID CSMD_HAL_ClearTelegramStatus
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_USHORT            usClrStatusMask,
                                  CSMD_USHORT            usPort );

SOURCE CSMD_VOID CSMD_HAL_Get_IsValid_Rx_BufferSysA
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_BOOL             *pboNewValidBufP1,
                                  CSMD_BOOL             *pboNewValidBufP2 );

SOURCE CSMD_ULONG CSMD_HAL_GetTelegramStatus
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_USHORT            usPort );

SOURCE CSMD_ULONG CSMD_HAL_GetRingRuntimeMeasure
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_USHORT            usPort );

SOURCE CSMD_VOID CSMD_HAL_SetSVCPort
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_USHORT            usPort );

SOURCE CSMD_VOID CSMD_HAL_Configure_Tx_Buffer
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_ULONG             ulSystem,
                                  CSMD_ULONG             ulNumber );
SOURCE CSMD_VOID CSMD_HAL_Configure_Rx_Buffer
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_ULONG             ulSystem,
                                  CSMD_ULONG             ulNumber );

SOURCE CSMD_VOID CSMD_HAL_Config_Rx_Buffer_Valid
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_ULONG             ulSystem,
                                  CSMD_ULONG             ulReqMDTs,
                                  CSMD_ULONG             ulReqATs );

SOURCE CSMD_VOID CSMD_HAL_Usable_Rx_BufferSysA
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_USHORT           *pusBufP1,
                                  CSMD_USHORT           *pusBufP2 );

SOURCE CSMD_VOID CSMD_HAL_GetCommCounter
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_USHORT            usPortNbr,
                                  CSMD_HAL_COMM_COUNTER *prCommCounter );


SOURCE CSMD_VOID CSMD_HAL_ReadBlock
                                ( CSMD_VOID             *pvDataDes,
                                  CSMD_VOID             *pvReadSource,
                                  CSMD_USHORT            usLength );

SOURCE CSMD_VOID CSMD_HAL_WriteBlock
                                ( CSMD_VOID             *pvWriteDes,
                                  CSMD_VOID             *pvDataSource,
                                  CSMD_USHORT            usLength );

SOURCE CSMD_VOID CSMD_HAL_SetTxDescriptor
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_ULONG             ulTxRamOffset,
                                  CSMD_USHORT            usBufferOffset,
                                  CSMD_USHORT            usBufSysSel,
                                  CSMD_USHORT            usTelegramOffset,
                                  CSMD_USHORT            usDesType );

SOURCE CSMD_VOID CSMD_HAL_SetRxDescriptor
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_ULONG             ulRxRamOffset,
                                  CSMD_USHORT            usBufferOffset,
                                  CSMD_USHORT            usBufSysSel,
                                  CSMD_USHORT            usTelegramOffset,
                                  CSMD_USHORT            usDesType );

#ifdef CSMD_ACTIVATE_AUTONEGOTIATION

SOURCE CSMD_ULONG CSMD_HAL_ReadMiiPhy
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_USHORT            usPhyAddr,
                                  CSMD_USHORT            usPhyReg );

SOURCE CSMD_VOID CSMD_HAL_WriteMiiPhy
                                ( CSMD_HAL              *prCSMD_HAL,
                                  CSMD_USHORT            usPhyAddr,
                                  CSMD_USHORT            usPhyReg,
                                  CSMD_USHORT            inData );

#endif  /* #ifdef CSMD_ACTIVATE_AUTONEGOTIATION */


SOURCE CSMD_VOID * CSMD_HAL_memset
                                ( CSMD_VOID             *pvDest,
                                  CSMD_LONG              lValue,
                                  CSMD_ULONG             ulSize );

SOURCE CSMD_VOID * CSMD_HAL_memcpy
                                ( CSMD_VOID             *pvDest,
                                  const CSMD_VOID       *pvSource,
                                  CSMD_ULONG             ulSize );

SOURCE CSMD_CHAR * CSMD_HAL_strcpy
                                ( CSMD_CHAR             *pcDest,
                                  const CSMD_CHAR       *pcSource );

SOURCE CSMD_CHAR * CSMD_HAL_strncpy
                                ( CSMD_CHAR             *pcDest,
                                  const CSMD_CHAR       *pccSource,
                                  CSMD_ULONG             ulSize );

SOURCE CSMD_LONG CSMD_HAL_strncmp
                                ( const CSMD_CHAR       *pccStr1,
                                  const CSMD_CHAR       *pccStr2,
                                  CSMD_ULONG             ulSize );


SOURCE CSMD_LONG CSMD_HAL_sprintf_ushort
                                ( CSMD_CHAR             *pcStr,
                                  const CSMD_CHAR       *pccFormat,
                                  CSMD_USHORT            usArg1 );

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* HAL */

/*lint -restore */

#endif /* _CSMD_HAL_PRIV */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - Refactoring of CoSeMa files.
11 Nov 2013 WK
  - Added FPGA Releases 4.7.x to the blacklist.
    Added FPGA Releases 4.8.x to the blacklist.
    Added FPGA Releases 4.9.T1-T2 to the blacklist.
21 Nov 2013 WK
  - Defdb00000000
    Added function macro CSMD_HAL_ResetCounter_SercosError().
    This function is required for CSMD_ResetSercosErrorCounter() in case
    of not defined CSMD_NRT_CHANNEL.
29 Nov 2013 WK
  - Defdb00150926 
    CSMD_HAL_CtrlSVC_Redundancy
    Fixed lint warning 774 "Boolean always evaluates to [True/False]".
15 Apr 2014 WK
    Added FPGA Releases  4.9.T3-T7 to the blacklist.
    Added FPGA Releases 4.10.T1-T7 to the blacklist.
15 May 2014 AlM
  - Added function macro CSMD_HAL_GetValidTelegramsSysA().
19 Aug 2014 WK
  - Added define CSMD_HAL_DMA_IP_DISABLE for IP-Core 4.10
23 Oct 2014 WK
  Defdb00174315
    Added prototype for new function CSMD_HAL_SetSercosTimeExtSync().
11 Feb 2015 WK
  - Defdb00176768
    Added function macro CSMD_HAL_IsSoftMaster.
17 Feb 2015 WK
    Added FPGA Releases 4.11.T1-T7 to the blacklist.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
18 Mar 2015 WK
  - Replaced CSMD_HAL_strcmp() with CSMD_HAL_strncmp().
02 Apr 2015 WK
  - Defdb00177706
    Added define CSMD_HAL_IPCR_COL_BUF_DISABLE.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
14 Oct 2015 WK
  - Defdb00182283
    CSMD_HAL_Initialize()
    The Sercos-time no longer becomes disabled.
16 Nov 2015 WK
  - Defdb00000000
    Test-Code for sprintf has been removed.
01 Dec 2015 WK
  - Defdb00000000
    Added defines for Control- and Status-Word of IPCSR.
12 Jan 2016 WK
  - Defdb00184128
    Function macros CSMD_HAL_SetComMode, CSMD_HAL_ResetLineStatus,
    CSMD_HAL_ClearTelegramStatusP1 and CSMD_HAL_ClearTelegramStatusP2:
    Adjustment for defined CSMD_SOFT_MASTER.
11 Jan 2016 WK
  - Defdb00182067
    Inhibits "Lint Info 717 do ... while(0)  specifically authorized"
    for macros within this file.
28 Jan 2016 WK
  - Defdb00182067
    Function macro CSMD_HAL_WatchdogSet:
    Fixed lint warning 569 respectively compiler warning caused
    by CSMD_HAL_WD_MAGIC_PATTERN.
08 Mar 2016 WK
  Defdb00185518
  - Function macros CSMD_HAL_SetComMode, CSMD_HAL_ResetLineStatus,
    CSMD_HAL_ClearTelegramStatusP1 and CSMD_HAL_ClearTelegramStatusP2:
    Encapsulation with CSMD_SOFT_MASTER and case distinction IP-Core/Soft-Master.
  - Malfunction of CSMD_HAL_ClearTelegramStatusP1 and
    CSMD_HAL_ClearTelegramStatusP2 with defined CSMD_SOFT_MASTER fixed.
21 Mar 2016 WK
  - Defdb00000000
    Added the FPGA 4.12 release and all test-versions to the blacklist.
04 Apr 2016 AlM
  -	Defdb00184988
	Added defines for telegram validation
------------------------------------------------------------------------------
*/
