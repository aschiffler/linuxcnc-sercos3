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
\file   CSMD_HAL_GLOB.h
\author KP
\date   03.09.2007
\brief  This file contains the public HAL declarations and macro definitions.
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

#ifndef _CSMD_HAL_GLOB
#define _CSMD_HAL_GLOB

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif


#ifdef __cplusplus
extern "C"
{
#endif

/*! \cond HAL */

/* ------------------------------------------------------------------------- */
/* --> Is not supported at this time!                                        */
/* Sercos IP-Core SERCON100M endianness                                      */
/*  CSMD_BIG_ENDIAN_HW                                                       */
/*  Macro defined  :    Big endian IP-Core SERCON100M                        */
/*  Macro undefined:    Little endian IP-Core SERCON100M                     */
/* ------------------------------------------------------------------------- */
/* #undef  CSMD_BIG_ENDIAN_HW */      /* little endian IP-Core SERCON100M    */



/*---------------------------------------------------- */
/* Interrupt masks register 0 */
/*---------------------------------------------------- */
#define CSMD_HAL_MASK_TINT_0        (0x00000001)
#define CSMD_HAL_MASK_TINT_1        (0x00000002)
#define CSMD_HAL_MASK_TINT_2        (0x00000004)
#define CSMD_HAL_MASK_TINT_3        (0x00000008)
#define CSMD_HAL_MASK_TINT_MAX      (0x00000010)




/*---- Declaration public Types ---------------------------------------------*/

#define CSMD_HAL_SERCFPGA_DATTYP       volatile CSMD_ULONG    /* long word data type   */
#define CSMD_HAL_SERCFPGA_WDATTYP      volatile CSMD_USHORT   /* word data type */
#define CSMD_HAL_SERCFPGA_BDATTYP      volatile CSMD_UCHAR    /* byte data type */


/* SERCON100M address map

+----------------+----------+------------+-------------------+---------------------------+
| Name           | CPU base | Total size | FPGA address      |  Description              |
|                | address  |   [Byte]   |                   |                           |
+----------------+----------+------------+-------------------+---------------------------+
| Register       |  +0x0000 |     1K     | 0x0000 ... 0x03FF | Register Area             |
|                |          |            |                   |                           |
+----------------+----------+------------+-------------------+---------------------------+
| SVC Legacy Ram |  +0x4000 |     4K     | 0x0000 ... 0x3FFF | Service-channel Ram       |
|                |          |            |                   |                           |
+----------------+----------+------------+-------------------+---------------------------+
| TX Ram         |  +0x8000 |     8K     | 0x0000 ... 0x1FFF | Tx Ram  (shared memory)   |
|                |          |            |                   |   for Sercos III          |
|                |          |            |                   |   and IP transmit data    |
|                |          |            +-------------------+---------------------------+
|                |          |            | 0x0000 ...        | Tx Ram: Sercos III        |
|                |          |            |                   |   transmit real time data |
|                |          |            +-------------------+---------------------------+
|                |          |            |        ... 0x1FFF | Tx Ram:                   |
|                |          |            |                   |   IP transmit data        |
+----------------+----------+------------+-------------------+---------------------------+
| RX Ram         |  +0xC000 |    16K     | 0x0000 ... 0x3FFF | Rx Ram  (shared memory)   |
|                |          |            |                   |   for Sercos III          |
|                |          |            |                   |   and IP receive data     |
|                |          |            +-------------------+---------------------------+
|                |          |            | 0x0000 ...        | Rx Ram:  Sercos III       |
|                |          |            |                   |   receive real time data  |
|                |          |            |                   |   port 1 and port 2       |
|                |          |            +-------------------+---------------------------+
|                |          |            |        ... 0x3FFF | Rx Ram:                   |
|                |          |            |                   |   IP receive data         |
|                |          |            |                   |   port 1 and port 2       |
+----------------+----------+------------+-------------------+---------------------------+
*/

/*---------------------------------------------------- */
/*              RAM SIZE DEFINITION                    */
/*          (must be integer divisible by 4)           */
/*---------------------------------------------------- */
#define CSMD_HAL_SVC_RAM_SIZE            4096U  /*  4K SVC RAM */
#define CSMD_HAL_TX_RAM_SIZE             8192U  /*  8K Tx  RAM */
#define CSMD_HAL_RX_RAM_SIZE            16384U  /* 16K Rx  RAM */


/*---------------------------------------------------- */
/*                                                     */
/*---------------------------------------------------- */

/* Number of Tx buffer base pointer */
#define CSMD_HAL_TX_BASE_PTR_NBR            (16)
/* Index for Tx base buffer base pointer list */
#define CSMD_HAL_IDX_TX_BUFF_0_SYS_A        (0)
#define CSMD_HAL_IDX_TX_BUFF_1_SYS_A        (1)
#define CSMD_HAL_IDX_TX_BUFF_2_SYS_A        (2)
#define CSMD_HAL_IDX_TX_BUFF_3_SYS_A        (3)

#define CSMD_HAL_IDX_TX_BUFF_0_SYS_B        (4)
#define CSMD_HAL_IDX_TX_BUFF_1_SYS_B        (5)
#define CSMD_HAL_IDX_TX_BUFF_2_SYS_B        (6)
#define CSMD_HAL_IDX_TX_BUFF_3_SYS_B        (7)

#define CSMD_HAL_IDX_TX_BUFF_PORT_1         (12)
#define CSMD_HAL_IDX_TX_BUFF_PORT_2         (13)

#define CSMD_HAL_IDX_TX_BUFF_SVC            (15)


/* Number of Rx buffer base pointer */
#define CSMD_HAL_RX_BASE_PTR_NBR            (16)
/* Index for Rx base buffer base pointer list */
#define CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_A     (0)
#define CSMD_HAL_IDX_RX_P1_BUFF_1_SYS_A     (1)
#define CSMD_HAL_IDX_RX_P1_BUFF_2_SYS_A     (2)
#define CSMD_HAL_IDX_RX_P1_BUFF_PORT_WR_TX  (3)
#define CSMD_HAL_IDX_RX_P1_BUFF_0_SYS_B     (4)
#define CSMD_HAL_IDX_RX_P1_BUFF_1_SYS_B     (5)
#define CSMD_HAL_IDX_RX_P1_BUFF_2_SYS_B     (6)
#define CSMD_HAL_IDX_RX_P1_BUFF_SVC         (7)

#define CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_A     (8)
#define CSMD_HAL_IDX_RX_P2_BUFF_1_SYS_A     (9)
#define CSMD_HAL_IDX_RX_P2_BUFF_2_SYS_A     (10)
#define CSMD_HAL_IDX_RX_P2_BUFF_PORT_WR_TX  (11)
#define CSMD_HAL_IDX_RX_P2_BUFF_0_SYS_B     (12)
#define CSMD_HAL_IDX_RX_P2_BUFF_1_SYS_B     (13)
#define CSMD_HAL_IDX_RX_P2_BUFF_2_SYS_B     (14)
#define CSMD_HAL_IDX_RX_P2_BUFF_SVC         (15)



/*---------------------------------------------------- */
/*             FPGA Event Typ                          */
/*---------------------------------------------------- */
#define CSMD_HAL_EVENT_TIME_MASK        0x07FFFFFFU

#define CSMD_HAL_EVENT_SUBCYCCNT_MASK   0x000000FFU

#define CSMD_HAL_EVENT_TYPE_MASK        0x0F000000U
#define CSMD_HAL_EVENT_TYPE_SHIFT       (24)

#define CSMD_HAL_EVENT_SUBCYCSEL_MASK   0x00030000U
#define CSMD_HAL_EVENT_SUBCYCSEL_SHIFT  (16)


/*---------------------------------------------------- */
/* 18h - DFCSR - Data flow control & status register   */
/*---------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_DFCSR_UN
{
    CSMD_HAL_SERCFPGA_DATTYP    ulLong;

  struct
  {
#ifdef CSMD_BIG_ENDIAN_HW
    CSMD_HAL_SERCFPGA_WDATTYP   usStatus;
    CSMD_HAL_SERCFPGA_WDATTYP   usControl;
#else
    CSMD_HAL_SERCFPGA_WDATTYP   usControl;
    CSMD_HAL_SERCFPGA_WDATTYP   usStatus;
#endif
  } rShort;

} CSMD_HAL_SERCFPGA_DFCSR_UNION;


/*---------------------------------------------------- */
/* 30h - DECR - Descriptor Control Register            */
/*---------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_DECR_UN
{
  CSMD_HAL_SERCFPGA_DATTYP      ulDesIdxTableOffsets;
  
  struct
  {
#ifdef CSMD_BIG_ENDIAN_HW
    CSMD_HAL_SERCFPGA_WDATTYP   usOffsetTxRam;
    CSMD_HAL_SERCFPGA_WDATTYP   usOffsetRxRam;
#else
    CSMD_HAL_SERCFPGA_WDATTYP   usOffsetRxRam;
    CSMD_HAL_SERCFPGA_WDATTYP   usOffsetTxRam;
#endif
  } rDesIdx;
  
} CSMD_HAL_SERCFPGA_DECR_UNION;


/*---------------------------------------------------- */
/* 68h - WDCSR - Watchdog Control & Status Register    */
/*---------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_WDCSR_UN
{
  CSMD_HAL_SERCFPGA_DATTYP      ulWDCSR;
  
  struct
  {
#ifdef CSMD_BIG_ENDIAN_HW
    CSMD_HAL_SERCFPGA_WDATTYP   usControlStatus;
    CSMD_HAL_SERCFPGA_WDATTYP   usMagicPattern;
#else
    CSMD_HAL_SERCFPGA_WDATTYP   usMagicPattern;
    CSMD_HAL_SERCFPGA_WDATTYP   usControlStatus;
#endif
  } rShort;
  
} CSMD_HAL_SERCFPGA_WDCSR_UNION;


/*---------------------------------------------------- */
/* 6Ch - WDCNT - Watchdog Counter Register             */
/*---------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_WDCNT_UN
{
  CSMD_HAL_SERCFPGA_DATTYP      ulCounter;
  
  struct
  {
#ifdef CSMD_BIG_ENDIAN_HW
    CSMD_HAL_SERCFPGA_WDATTYP   usActual;
    CSMD_HAL_SERCFPGA_WDATTYP   usReset;
#else
    CSMD_HAL_SERCFPGA_WDATTYP   usReset;
    CSMD_HAL_SERCFPGA_WDATTYP   usActual;
#endif
  } rCounter;
  
} CSMD_HAL_SERCFPGA_WDCNT_UNION;


/* ---------------------------------------------------- */
/* 40h - TxIFG: Tx MAC additive Interframe gap for      */
/*              RTD frame transmission.                 */
/* ---------------------------------------------------- */

/* FPGA internal base value of Inter Framne Gap [bytes] */
#define CSMD_HAL_TXIFG_BASE             12UL


/*---------------------------------------------------- */
/* 74h - MII Control / Status Register                 */
/*---------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_MIICSR_UN
{
  CSMD_HAL_SERCFPGA_DATTYP      ulLong;     /*!< MII C/S register */
  
  struct
  {
#ifdef CSMD_BIG_ENDIAN_HW
    CSMD_HAL_SERCFPGA_WDATTYP   HighWord;   /*!< High word not used */
    CSMD_HAL_SERCFPGA_WDATTYP   LowWord;    /*!< MII C/S register   */
#else
    CSMD_HAL_SERCFPGA_WDATTYP   LowWord;    /*!< MII C/S register   */
    CSMD_HAL_SERCFPGA_WDATTYP   HighWord;   /*!< High word not used */
#endif
  } rShort;
  
} CSMD_HAL_SERCFPGA_MIICSR_UNION;


/*---------------------------------------------------- */
/* 80h - MAC address                                   */
/*---------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_MACADD_UN
{
  struct
  {
#ifdef CSMD_BIG_ENDIAN
    CSMD_HAL_SERCFPGA_DATTYP  AddressL;     /*!< MAC address byte b1, b0         */
    CSMD_HAL_SERCFPGA_DATTYP  AddressH;     /*!< MAC address byte b5, b4, b3, b2 */
#else
    CSMD_HAL_SERCFPGA_DATTYP  AddressH;     /*!< MAC address byte b5, b4, b3, b2 */
    CSMD_HAL_SERCFPGA_DATTYP  AddressL;     /*!< MAC address byte b1, b0         */
#endif
  } ul;
  
  CSMD_HAL_SERCFPGA_BDATTYP   ucAddress[8]; /*!< MAC address byte b5, b4, b3, b2, b1, b0 */
  
} CSMD_HAL_SERCFPGA_MACADD_UNION;


/*---------------------------------------------------- */
/* 90h/94h - IP Control / Status Register              */
/*---------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_IPCSR_UN
{
  CSMD_HAL_SERCFPGA_DATTYP      ulIPCSR;    /* IP Control-/Statusregister */
  
  struct
  {
#ifdef CSMD_BIG_ENDIAN_HW
    CSMD_HAL_SERCFPGA_WDATTYP   IPSR;       /*!< IP Status register  */
    CSMD_HAL_SERCFPGA_WDATTYP   IPCR;       /*!< IP Control register */
#else
    CSMD_HAL_SERCFPGA_WDATTYP   IPCR;       /*!< IP Control register */
    CSMD_HAL_SERCFPGA_WDATTYP   IPSR;       /*!< IP Status register  */
#endif
  } us;
  
} CSMD_HAL_SERCFPGA_IPCSR_UNION;


/*---------------------------------------------------- */
/* C0h-DFh - Frame and error counters                  */
/*---------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_ERRCNT_UN
{
  CSMD_HAL_SERCFPGA_DATTYP      ulErrCnt;
  
  struct
  {
#ifdef CSMD_BIG_ENDIAN_HW
    CSMD_HAL_SERCFPGA_WDATTYP   usPort2;
    CSMD_HAL_SERCFPGA_WDATTYP   usPort1;
#else
    CSMD_HAL_SERCFPGA_WDATTYP   usPort1;
    CSMD_HAL_SERCFPGA_WDATTYP   usPort2;
#endif
  } rErrCnt;
  
} CSMD_HAL_SERCFPGA_ERRCNT_UNION;


/*---------------------------------------------------- */
/* 140h - SubCycle Counter Config register             */
/*---------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_SCCAB_UN
{
  CSMD_HAL_SERCFPGA_DATTYP      ulSCCSR;
  
  struct
  {
#ifdef CSMD_BIG_ENDIAN_HW
    CSMD_HAL_SERCFPGA_BDATTYP   ucSccValueBufB;     /* Bit 31 - 24 */
    CSMD_HAL_SERCFPGA_BDATTYP   ucSccValueBufA;     /* Bit 23 - 16 */
    CSMD_HAL_SERCFPGA_BDATTYP   ucSccDifBufB;       /* Bit 15 -  8 */
    CSMD_HAL_SERCFPGA_BDATTYP   ucSccDifBufA;       /* Bit  7 -  0 */
#else
    CSMD_HAL_SERCFPGA_BDATTYP   ucSccDifBufA;       /* Bit  7 -  0 */
    CSMD_HAL_SERCFPGA_BDATTYP   ucSccDifBufB;       /* Bit 15 -  8 */
    CSMD_HAL_SERCFPGA_BDATTYP   ucSccValueBufA;     /* Bit 23 - 16 */
    CSMD_HAL_SERCFPGA_BDATTYP   ucSccValueBufB;     /* Bit 31 - 24 */
#endif
  } rSCC;
  
} CSMD_HAL_SERCFPGA_SCCAB_UNION;


/*---------------------------------------------------- */
/* 150h - SubCycleCounter Count register               */
/*---------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_SCCMDT_UN
{
  CSMD_HAL_SERCFPGA_DATTYP      ulSCCNT;
  
  struct
  {
#ifdef CSMD_BIG_ENDIAN_HW
    CSMD_HAL_SERCFPGA_WDATTYP   usSccReadBack;      /* Bit 31 - 16 */
    CSMD_HAL_SERCFPGA_WDATTYP   usScCount;          /* Bit 15 -  0 */
#else
    CSMD_HAL_SERCFPGA_WDATTYP   usScCount;          /* Bit 15 -  0 */
    CSMD_HAL_SERCFPGA_WDATTYP   usSccReadBack;      /* Bit 31 - 16 */
#endif
  } rSCCNT;
  
} CSMD_HAL_SERCFPGA_SCCMDT_UNION;



/*-------------------------------------------------- */
/*! SERCON100M Tx ram                                */
/*-------------------------------------------------- */
typedef union CSMD_HAL_TX_RAM_UN
{
  CSMD_HAL_SERCFPGA_DATTYP      aulTx_Ram[ CSMD_HAL_TX_RAM_SIZE / sizeof (CSMD_HAL_SERCFPGA_DATTYP) ];
  CSMD_HAL_SERCFPGA_WDATTYP     ausTx_Ram[ CSMD_HAL_TX_RAM_SIZE / sizeof (CSMD_HAL_SERCFPGA_WDATTYP) ];
  CSMD_HAL_SERCFPGA_BDATTYP     aucTx_Ram[ CSMD_HAL_TX_RAM_SIZE / sizeof (CSMD_HAL_SERCFPGA_BDATTYP) ];
  
} CSMD_HAL_TX_RAM;


/*-------------------------------------------------- */
/*! SERCON100M Rx ram                                */
/*-------------------------------------------------- */
typedef union CSMD_HAL_RX_RAM_UN
{
  CSMD_HAL_SERCFPGA_DATTYP      aulRx_Ram[ CSMD_HAL_RX_RAM_SIZE / sizeof (CSMD_HAL_SERCFPGA_DATTYP) ];
  CSMD_HAL_SERCFPGA_WDATTYP     ausRx_Ram[ CSMD_HAL_RX_RAM_SIZE / sizeof (CSMD_HAL_SERCFPGA_WDATTYP) ];
  CSMD_HAL_SERCFPGA_BDATTYP     aucRx_Ram[ CSMD_HAL_RX_RAM_SIZE / sizeof (CSMD_HAL_SERCFPGA_BDATTYP) ];
  
} CSMD_HAL_RX_RAM;


#ifdef CSMD_PCI_MASTER

#define CSMD_DMA_TXRAM_COPYTIME_CP34_DEF    (50U * 1000U)    /* Event controlled DMA: Copy time default value for TxRam in CP3/CP4.[ns] */

/*-------------------------------------------------- */
/*! SERCON100M DMA Ready ram                         */
/*-------------------------------------------------- */
typedef volatile struct CSMD_HAL_DMA_RDY_RAM_STR
{
  CSMD_HAL_SERCFPGA_DATTYP      aulDMA_Ready_Ram[ 16*2 ];
  
} CSMD_HAL_DMA_RDY_RAM;


#define CSMD_DMA_MAX_RX_CHANNEL     (16)
#define CSMD_DMA_MAX_TX_CHANNEL     (16)

typedef volatile struct CSMD_HAL_DMA_TX_RDY_FLAGS_STR
{
  CSMD_HAL_SERCFPGA_DATTYP      aulFlag[ CSMD_DMA_MAX_TX_CHANNEL ];
  
} CSMD_HAL_DMA_TX_RDY_FLAGS;

typedef volatile struct CSMD_HAL_DMA_RX_RDY_FLAGS_STR
{
  CSMD_HAL_SERCFPGA_DATTYP      aulFlag[ CSMD_DMA_MAX_RX_CHANNEL ];
  
} CSMD_HAL_DMA_RX_RDY_FLAGS;
#endif


/*---------------------------------------------------- */
/* Read / Write buffer lengths for Service channels    */
/*---------------------------------------------------- */
/* Buffer Length integer divisible by 8 ! */
#define CSMD_SERC_SC_WRBUF_LENGTH    (((((CSMD_SVC_CONTAINER_LENGTH / 4) * 2 - (2+5+2)) / 2) / 4) * 4) 
#define CSMD_SERC_SC_RDBUF_LENGTH    (CSMD_SERC_SC_WRBUF_LENGTH + 2)          
#define CSMD_SERC_SC_FREE_LENGTH     ((CSMD_SVC_CONTAINER_LENGTH  / 4) * 2 - ((2+5) + CSMD_SERC_SC_WRBUF_LENGTH + CSMD_SERC_SC_RDBUF_LENGTH))


/*---------------------------------------------------------- */
/*! \brief Service channel container control word structure  */
/*---------------------------------------------------------- */
typedef union CSMD_HAL_SERCFPGA_SC_CONTROL_UN
{
  CSMD_HAL_SERCFPGA_WDATTYP  usWord[5];
#ifdef CSMD_DEBUG
  /* Note: maybe that this structure must be declared via define in         */
  /*       reverse order for other compilers respectively machines !!!      */
  struct
  {
    struct
    {
      /*! --- 1st.Word ( Index 0 ) ----------------- */
      CSMD_HAL_SERCFPGA_WDATTYP   bt_15_not_used       : 1;    /*!< Bit 15   : Not used */
      CSMD_HAL_SERCFPGA_WDATTYP   btINT_END_RDBUF      : 1;    /*!< Bit 14   : End of read buffer is reached */
      CSMD_HAL_SERCFPGA_WDATTYP   btINT_END_WRBUF      : 1;    /*!< Bit 13   : End of write buffer is reached */
      CSMD_HAL_SERCFPGA_WDATTYP   btINT_ERR            : 1;    /*!< Bit 12   : Slave reports error */
      CSMD_HAL_SERCFPGA_WDATTYP   bf_11_8_not_used     : 4;    /*!< Bit 11-8 : Not used  */
      CSMD_HAL_SERCFPGA_WDATTYP   btM_BUSY             : 1;    /*!< Bit 7    : Service container waits for interaction of the host CPU (M_BUSY=1) */
      CSMD_HAL_SERCFPGA_WDATTYP   btSETEND             : 1;    /*!< Bit 6    : END_MDT is to be set */
      CSMD_HAL_SERCFPGA_WDATTYP   bfELEM_MDT           : 3;    /*!< Bit 5-3  : Data element type in MDT */
      CSMD_HAL_SERCFPGA_WDATTYP   btEND_MDT            : 1;    /*!< Bit 2    : End in MDT */
      CSMD_HAL_SERCFPGA_WDATTYP   btLS_MDT             : 1;    /*!< Bit 1    : Read / Write in MDT */
      CSMD_HAL_SERCFPGA_WDATTYP   btHS_MDT             : 1;    /*!< Bit 0    : Handshake-bit in MDT */
    } sWord_0;
    struct
    {
      /*! --- 2nd.Word ( Index 1 ) ----------------- */
      CSMD_HAL_SERCFPGA_WDATTYP   bf15_4_not_used      :12;    /*!< Bit 15-10: Not used */
      CSMD_HAL_SERCFPGA_WDATTYP   btSVC_VALID          : 1;    /*!< Bit 3    : SVC valid-bit in AT */
      CSMD_HAL_SERCFPGA_WDATTYP   btERR_AT             : 1;    /*!< Bit 2    : Error-bit in AT */
      CSMD_HAL_SERCFPGA_WDATTYP   btBUSY_AT            : 1;    /*!< Bit 1    : Busy-bit in AT */
      CSMD_HAL_SERCFPGA_WDATTYP   btHS_AT              : 1;    /*!< Bit 0    : Handshake-bit in AT */
    } sWord_1;
    struct
    {
      /*! --- 3rd.Word ( Index 2 ) ----------------- */
      CSMD_HAL_SERCFPGA_WDATTYP   bfWRDATLAST          : 8;    /*!< Bit 15-8 : Pointer to last position in write buffer */
      CSMD_HAL_SERCFPGA_WDATTYP   bfWRDATPT            : 8;    /*!< Bit 7-0  : Pointer to present position in write buffer */
    } sWord_2;
    struct
    {
      /*! --- 4th.Word ( Index 3 ) ----------------- */
      CSMD_HAL_SERCFPGA_WDATTYP   bfRDDATLAST          : 8;    /*!< Bit 15-8 : Pointer to last position in read buffer */
      CSMD_HAL_SERCFPGA_WDATTYP   bfRDDATPT            : 8;    /*!< Bit 7-0  : Pointer to present position in read buffer */
    } sWord_3;
    struct
    {
      /*! --- 5th.Word ( Index 4 ) ----------------- */
      CSMD_HAL_SERCFPGA_WDATTYP   bf_15_12_not_used    : 4;    /*!< Bit 15-13: Not used */
      CSMD_HAL_SERCFPGA_WDATTYP   btINT_BUSY_TIMEOUT   : 1;    /*!< Bit 11   : Interrupt BUSY timeout */
      CSMD_HAL_SERCFPGA_WDATTYP   btINT_HS_TIMEOUT     : 1;    /*!< Bit 10   : Interrupt due to handshake timeout */
      CSMD_HAL_SERCFPGA_WDATTYP   btINT_SC_ERR         : 1;    /*!< Bit 9    : Interrupt due to protocol error */
      CSMD_HAL_SERCFPGA_WDATTYP   btBUSY_CNT           : 1;    /*!< Bit 8    : 0: Error counts differences of handshake / 1: Error counts BUSY cycles */
      CSMD_HAL_SERCFPGA_WDATTYP   bfERR_CNT            : 8;    /*!< Bit 7-0  : Error counter */
    } sWord_4;
  } rBit;
#endif  /* End: #ifdef CSMD_DEBUG */
} CSMD_HAL_SERCFPGA_SC_CONTROL_UNION;



#ifdef CSMD_PACK_PRAGMA
#pragma pack(push,2)
#endif

/* -------------------------------------------------------------------------- */
/*! \brief Sercos service channel container structure                         */
/* -------------------------------------------------------------------------- */
struct CSMD_HAL_SERC3SVC_STR
{
  CSMD_HAL_SERCFPGA_WDATTYP      usSVCRxPointer_Status;                 /*!< Pointer to SVC status word in Rx Ram */
  CSMD_HAL_SERCFPGA_WDATTYP      usSVCTxPointer_Control;                /*!< Pointer to SVC control word in Tx Ram */
  CSMD_HAL_SERCFPGA_SC_CONTROL_UNION
                                 rCONTROL;                              /*!< control words */
  CSMD_HAL_SERCFPGA_WDATTYP      sWR_BUFF[CSMD_SERC_SC_WRBUF_LENGTH];   /*!< Write buffer (integer divisible by 8 bytes) */
  CSMD_HAL_SERCFPGA_WDATTYP      sRD_BUFF[CSMD_SERC_SC_RDBUF_LENGTH];   /*!< Read buffer  (integer divisible by 8 bytes) */
#if CSMD_SERC_SC_FREE_LENGTH != 0
  CSMD_HAL_SERCFPGA_WDATTYP      usFree[CSMD_SERC_SC_FREE_LENGTH];      /*!< Redundant memory to guarantee that buffer are interger divisible by 8 */
#endif

} /*lint !e659  (Nothing follows '}' on line terminating struct/union/enum definition) */
#ifdef CSMD_PACK_ATTRIBUTE
__attribute__((packed))
#endif
;
#ifdef CSMD_PACK_PRAGMA
#pragma pack(pop)
#endif
typedef struct CSMD_HAL_SERC3SVC_STR  CSMD_HAL_SERC3SVC;


/*-------------------------------------------------- */
/* SERCON100M service channel ram                    */
/*-------------------------------------------------- */
#define  CSMD_NUMBER_OF_SVC                      32          /* Max. Number of service container in FPGA */
typedef struct CSMD_HAL_SVC_RAM_STR
{
  CSMD_HAL_SERCFPGA_WDATTYP    uwSVC_Pointer[CSMD_NUMBER_OF_SVC];     /*!< Pointer to SVC */
#if CSMD_MAX_HW_CONTAINER > 0
  CSMD_HAL_SERC3SVC            rSC_S3[CSMD_MAX_HW_CONTAINER];         /*!< Service Containters */
  CSMD_HAL_SERCFPGA_WDATTYP    usFree[ (CSMD_HAL_SVC_RAM_SIZE - (CSMD_NUMBER_OF_SVC * 2 + sizeof (CSMD_HAL_SERC3SVC) * CSMD_MAX_HW_CONTAINER)) / 2];
#else
  CSMD_HAL_SERCFPGA_WDATTYP    usFree[ (CSMD_HAL_SVC_RAM_SIZE - (CSMD_NUMBER_OF_SVC * 2 )) / 2 ];
#endif
} CSMD_HAL_SVC_RAM;



#if ((CSMD_NUMBER_OF_SVC * 2 + CSMD_SVC_CONTAINER_LENGTH * CSMD_MAX_HW_CONTAINER) > 4096)
  #error CSMD_SVC_CONTAINER_LENGTH too great, please reduce.
#endif

#if CSMD_MAX_HW_CONTAINER > CSMD_NUMBER_OF_SVC
  #error Max. Limit of FPGA service containers exceeded 
#endif

/*-------------------------------------------------- */
/* Definitions for uwSVC_Pointer[]                   */
/*-------------------------------------------------- */
#define  CSMD_SVC_POINTER_CONT_EN         0x8000U       /* Bit 15:  Service container enable */
#define  CSMD_SVC_POINTER_AT_CHECK        0x4000U       /* Bit 14:  Use the lower two bits of the container to select the AT (Container LONG-aligned) */
#define  CSMD_SVC_POINTER_AT_SELECT_AT0   0x0000U       /* Bit 1-0: Select AT0 for the SVC container */
#define  CSMD_SVC_POINTER_AT_SELECT_AT1   0x0001U       /* Bit 1-0: Select AT1 for the SVC container */
#define  CSMD_SVC_POINTER_AT_SELECT_AT2   0x0002U       /* Bit 1-0: Select AT2 for the SVC container */
#define  CSMD_SVC_POINTER_AT_SELECT_AT3   0x0003U       /* Bit 1-0: Select AT3 for the SVC container */


/*---------------------------------------------------- */
/*!                IP-Core Register                    */
/*---------------------------------------------------- */
typedef volatile struct CSMD_HAL_SERCFPGA_REGISTER_STR
{
  /* Base Registers */
  CSMD_HAL_SERCFPGA_DATTYP          ulIDR;              /*!< 00h - IDentification Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulGCSFR;            /*!< 04h - Global Control / Status / Feature Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulIER0;             /*!< 08h - Interrupt Enable Register 0 */
  CSMD_HAL_SERCFPGA_DATTYP          ulIER1;             /*!< 0Ch - Interrupt Enable Register 1 */
  CSMD_HAL_SERCFPGA_DATTYP          ulIMR0;             /*!< 10h - Interrupt Multiplex Register 0 */
  CSMD_HAL_SERCFPGA_DATTYP          ulIMR1;             /*!< 14h - Interrupt Multiplex Register 1 */
  CSMD_HAL_SERCFPGA_DATTYP          ulIRR0;             /*!< 18h - Interrupt Reset/Status Register 0 */
  CSMD_HAL_SERCFPGA_DATTYP          ulIRR1;             /*!< 1Ch - Interrupt Reset/Status Register 1 */
  CSMD_HAL_SERCFPGA_DFCSR_UNION     rDFCSR;             /*!< 20h - Data Flow Control and Status Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulPHASECR;          /*!< 24h - Phase Control Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulTGSR1;            /*!< 28h - Telegram Status Register port 1 */
  CSMD_HAL_SERCFPGA_DATTYP          ulTGSR2;            /*!< 2Ch - Telegram Status Register port 2 */
  CSMD_HAL_SERCFPGA_DECR_UNION      rDECR;              /*!< 30h - DEscriptor Control Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulSTRBR;            /*!< 34h - Sercos Time Read Back Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulTCSR;             /*!< 38h - Timing Control and Status Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulTCYCSTART;        /*!< 3Ch - Delaying CYCSTART input by Sync Delay Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulIFG;              /*!< 40h - Inter Frame Gap (Tx MAC) for Sercos frames */
  CSMD_HAL_SERCFPGA_DATTYP          aulReserved_44[1];  /*   44h - reserved */
  CSMD_HAL_SERCFPGA_DATTYP          ulRDLY1;            /*!< 48h - Ring DeLaY Register port 1 */
  CSMD_HAL_SERCFPGA_DATTYP          ulRDLY2;            /*!< 4Ch - Ring DeLaY Register port 2 */
  CSMD_HAL_SERCFPGA_DATTYP          ulSVCCSR;           /*!< 50h - SVC Control and Status Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulDTDIVCLK;         /*!< 54h - Distance Time between two DIVCLK pulses */
  CSMD_HAL_SERCFPGA_DATTYP          ulTDIVCLK_NDIVCLK;  /*!< 58h - DIVCLK delay Time / Number of DIVCLK pulses */
  CSMD_HAL_SERCFPGA_DATTYP          aulReserved_5C[1];  /*   5Ch - reserved */
  CSMD_HAL_SERCFPGA_DATTYP          ulASCR0;            /*!< 60h - Address Segment Control Register 0 (not used) */
  CSMD_HAL_SERCFPGA_DATTYP          ulASCR1;            /*!< 64h - Address Segment Control Register 1 (not used) */
  CSMD_HAL_SERCFPGA_WDCSR_UNION     rWDCSR;             /*!< 68h - Watchdog Control and Status Register */
  CSMD_HAL_SERCFPGA_WDCNT_UNION     rWDCNT;             /*!< 6Ch - Watchdog CouNTer Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulSFCR;             /*!< 70h - Sercos Frame Control Register */
  CSMD_HAL_SERCFPGA_MIICSR_UNION    rMIICSR;            /*!< 74h - MII Control / Status Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulDBGOCR;           /*!< 78h - Debug Output Control Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulSEQCNT;           /*!< 7Ch - SEQence CouNTer Register */
  
  /* IP Registers */
  CSMD_HAL_SERCFPGA_MACADD_UNION    rMAC1;              /*!< 80h ...
                                                             84h - MAC Address for UC channel transmission */
  CSMD_HAL_SERCFPGA_DATTYP          aulReserved_88_8C[2]; /* 88h ...
                                                             8Ch - reserved */
  CSMD_HAL_SERCFPGA_IPCSR_UNION     rIPCSR1;            /*!< 90h - IP Control and Status Register port 1 */
  CSMD_HAL_SERCFPGA_IPCSR_UNION     rIPCSR2;            /*!< 94h - IP Control and Status Register port 2 */
  CSMD_HAL_SERCFPGA_DATTYP          ulIPRRS1;           /*!< 98h - IP RxRam Segment Register for port 1 */
  CSMD_HAL_SERCFPGA_DATTYP          ulIPRRS2;           /*!< 9Ch - IP RxRam Segment Register for port 2 */
  CSMD_HAL_SERCFPGA_DATTYP          ulIPRXS1;           /*!< A0h - IP Receive Stack for received Ethernet frames on port 1 */
  CSMD_HAL_SERCFPGA_DATTYP          ulIPRXS2;           /*!< A4h - IP Receive Stack for received Ethernet frames on port 2 */
  CSMD_HAL_SERCFPGA_DATTYP          ulIPTXS1;           /*!< A8h - IP Transmit Stack for Ethernet frames on port 1 */
  CSMD_HAL_SERCFPGA_DATTYP          ulIPTXS2;           /*!< ACh - IP Transmit Stack for Ethernet frames on port 2 */

  CSMD_HAL_SERCFPGA_DATTYP          ulIPLASTFL;         /*!< B0h - Remaining frame length after last transmit event */
  CSMD_HAL_SERCFPGA_DATTYP          aulReserved_B4_BC[3]; /* B4h ...
                                                             BCh - reserved */
  CSMD_HAL_SERCFPGA_ERRCNT_UNION    rIPFRXOK;           /*!< C0h - Counter for errorfree received frames on a port */
  CSMD_HAL_SERCFPGA_ERRCNT_UNION    rIPFTXOK;           /*!< C4h - Counter for transmitted frames on a port */
  CSMD_HAL_SERCFPGA_ERRCNT_UNION    rIPFCSERR;          /*!< C8h - Counter for received Ethernet frames with FCS error */
  CSMD_HAL_SERCFPGA_ERRCNT_UNION    rIPALGNERR;         /*!< CCh - Counter for received Ethernet frames with an alignment error */
  CSMD_HAL_SERCFPGA_ERRCNT_UNION    rIPDISRXB;          /*!< D0h - Counter for dicarded receive Ethernet frames based on missing rx buffer ressources */
  CSMD_HAL_SERCFPGA_ERRCNT_UNION    rIPDISCOLB;         /*!< D4h - Counter for dicarded forwarding Ethernet frames based on missing collision buffer ressources */
  CSMD_HAL_SERCFPGA_ERRCNT_UNION    rIPCHVIOL;          /*!< D8h - Counter for Ethernet frames which violate the UC channel window */
  CSMD_HAL_SERCFPGA_ERRCNT_UNION    rIPSERCERR;         /*!< DCh - Counter for Ethernet frames with a wrong FCS or are misaligned (resettable) */
  CSMD_HAL_SERCFPGA_DATTYP          aulReserved_E0_FC[8]; /* E0h ...
                                                             FCh - reserved */
  
  /* Timing Registers */
  CSMD_HAL_SERCFPGA_DATTYP          ulMTDRL;            /*!< 100h - Lower Long of selected Main Timing Descriptor */
  CSMD_HAL_SERCFPGA_DATTYP          ulMTDRU;            /*!< 104h - Upper Long of selected Main Timing Descriptor */
  CSMD_HAL_SERCFPGA_DATTYP          ulMTDSR;            /*!< 108h - Main Timing Descriptor Select Register (indirect addressing) */
  CSMD_HAL_SERCFPGA_DATTYP          aulReserved_10C[1]; /*   10Ch - reserved */
  
  CSMD_HAL_SERCFPGA_DATTYP          ulPTDRL;            /*!< 110h - Lower Long of selected Port Timing Descriptor */
  CSMD_HAL_SERCFPGA_DATTYP          ulPTDRU;            /*!< 114h - Upper Long of selected Port Timing Descriptor */
  CSMD_HAL_SERCFPGA_DATTYP          ulPTDSR;            /*!< 118h - Port Timing Descriptor Select Register (indirect addressing) */
  CSMD_HAL_SERCFPGA_DATTYP          aulReserved_11C[1]; /*   11Ch - reserved */
  
  CSMD_HAL_SERCFPGA_DATTYP          ulPLLCSR;           /*!< 120h - PLL Control and Status Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulTCNTCYCR;         /*!< 124h - TCNT Cycle time Register */
  CSMD_HAL_SERCFPGA_DATTYP          aulReserved_128_12C[2];  /*   128h ...
                                                             12Ch - reserved */
  
  CSMD_HAL_SERCFPGA_DATTYP          ulSTNS;             /*!< 130h - Latched value of Sercos time nanoseconds */
  CSMD_HAL_SERCFPGA_DATTYP          ulSTSEC;            /*!< 134h - Latched value of Sercos time seconds */
  CSMD_HAL_SERCFPGA_DATTYP          ulSTNSP;            /*!< 138h - Precalculated value of Sercos time nanoseconds */
  CSMD_HAL_SERCFPGA_DATTYP          ulSTSECP;           /*!< 13Ch - Precalculated value of Sercos time seconds */
  CSMD_HAL_SERCFPGA_SCCAB_UNION     rSCCAB;             /*!< 140h - Buffer A/B Subcycle Counter Register */
  CSMD_HAL_SERCFPGA_DATTYP          aulReserved_144_14C[3];  /*   144h ...
                                                             14Ch - reserved */
  CSMD_HAL_SERCFPGA_SCCMDT_UNION    rSCCMDT;            /*!< 150h - SubCycleCounter Register */
  CSMD_HAL_SERCFPGA_DATTYP          ulSTNSOffset;       /*   154h - Pecalculated offset compensating the transmission time relating to TSref */
  CSMD_HAL_SERCFPGA_DATTYP          ulSTNSNew;          /*   158h - New value of Sercos time nanoseconds */
  CSMD_HAL_SERCFPGA_DATTYP          ulSTSECNew;         /*   15Ch - New value of Sercos time seconds */
  CSMD_HAL_SERCFPGA_DATTYP          aulReserved_160_17C[8];  /*   160h ...
                                                             17Ch - reserved for conformizer feature */
#if(1)
  /* Rx Buffer Offsets */
  CSMD_HAL_SERCFPGA_DATTYP          aulRxBufBasePtr[
                            CSMD_HAL_RX_BASE_PTR_NBR];  /*!< 180h ...
                                                             1BCh - Rx Buffer base pointer list */
  /* Tx Buffer Offsets */
  CSMD_HAL_SERCFPGA_DATTYP          aulTxBufBasePtr[
                            CSMD_HAL_TX_BASE_PTR_NBR];  /*!< 1C0h ...
                                                             1FCh - Tx Buffer base pointer list */
#else
  /* Rx Buffer Offsets */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF0_P1A;       /*!< 180h - RX BUFfer 0 base address for RTD Port 1 and buffer system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF1_P1A;       /*!< 184h - RX BUFfer 1 base address for RTD Port 1 and buffer system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF2_P1A;       /*!< 188h - RX BUFfer 2 base address for RTD Port 1 and buffer system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF_P1REL_TX;   /*!< 18Ch - RX BUFfer base address for Port 1 relative data written to TxRam */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF0_P1B;       /*!< 190h - RX BUFfer 0 base address for RTD Port 1 and buffer system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF1_P1B;       /*!< 194h - RX BUFfer 1 base address for RTD Port 1 and buffer system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF2_P1B;       /*!< 198h - RX BUFfer 2 base address for RTD Port 1 and buffer system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF_P1SVC;      /*!< 19Ch - RX BUFfer base address for SerVice Channel data at Port 1 */
  
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF0_P2A;       /*!< 1A0h - RX BUFfer 0 base address for RTD Port 2 and buffer system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF1_P2A;       /*!< 1A4h - RX BUFfer 1 base address for RTD Port 2 and buffer system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF2_P2A;       /*!< 1A8h - RX BUFfer 2 base address for RTD Port 2 and buffer system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF_P2REL_TX;   /*!< 1ACh - RX BUFfer base address for Port 2 relative data written to TxRam */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF0_P2B;       /*!< 1B0h - RX BUFfer 0 base address for RTD Port 2 and buffer system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF1_P2B;       /*!< 1B4h - RX BUFfer 1 base address for RTD Port 2 and buffer system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF2_P2B;       /*!< 1B8h - RX BUFfer 2 base address for RTD Port 2 and buffer system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUF_P2SVC;      /*!< 1BCh - RX BUFfer base address for SerVice Channel data at Port 2 */
  
  /* Tx Buffer Offsets */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUF0_A;         /*!< 1C0h - TX BUFfer 0 base address for RTD buffer system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUF1_A;         /*!< 1C4h - TX BUFfer 1 base address for RTD buffer system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUF2_A;         /*!< 1C8h - TX BUFfer 2 base address for RTD buffer system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUF3_A;         /*!< 1CCh - TX BUFfer 3 base address for RTD buffer system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUF0_B;         /*!< 1D0h - TX BUFfer 0 base address for RTD buffer system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUF1_B;         /*!< 1D4h - TX BUFfer 1 base address for RTD buffer system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUF2_B;         /*!< 1D8h - TX BUFfer 2 base address for RTD buffer system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUF3_B;         /*!< 1DCh - TX BUFfer 3 base address for RTD buffer system B */
  CSMD_HAL_SERCFPGA_DATTYP          aulReserved_1E0_1EC[4];  /*   1E0h
                                                             1E4h
                                                             1E8h
                                                          .. 1ECh - reserved */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUF_P1;         /*!< 1F0h - TX BUFfer base address for Port 1 only */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUF_P2;         /*!< 1F4h - TX BUFfer base address for Port 2 only */
  CSMD_HAL_SERCFPGA_DATTYP          ulReserved_1F8[1];  /*   1F8h - reserved */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUF_SVC;        /*!< 1FCh - TX BUFfer base address for SerVice Channel data */
#endif
  /* Buffer Control Registers */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUFCSR_A;       /*!< 200h - RX BUFfer Control and Status Register system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUFTV_A;        /*!< 204h - RX BUFfer Telegram Valid register system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUFTR_A;        /*!< 208h - RX BUFfer Telegram Requirements register system A */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUFCSR_A;       /*!< 20Ch - TX BUFfer Control and Status Register system A */

  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUFCSR_B;       /*!< 210h - RX BUFfer Control and Status Register system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUFTV_B;        /*!< 214h - RX BUFfer Telegram Valid register system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulRXBUFTR_B;        /*!< 218h - RX BUFfer Telegram Requirements register system B */
  CSMD_HAL_SERCFPGA_DATTYP          ulTXBUFCSR_B;       /*!< 21Ch - TX BUFfer Control and Status Register system B */
  
} CSMD_HAL_SERCFPGA_REGISTER;

#define CSMD_HAL_SOFT_MASTER_REG_EVENT_OFFSET   0x400
                                                  /*!< Byte offset for event mapping to register memory for soft master*/

#ifdef CSMD_PCI_MASTER
/*-------------------------------------------------- */
/*! SERCON100M DMA Registers (local address space 1  */
/*-------------------------------------------------- */
typedef volatile struct CSMD_HAL_SERCFPGA_DMA_TX_CONTROL_BLOCK_STR
{                                                 /*   TX-DMA control block                                       */
  CSMD_HAL_SERCFPGA_DATTYP    ulLength;           /*!< 00h - Number of bytes to be transfered (divisible by 4)    */
  CSMD_HAL_SERCFPGA_DATTYP    ulDestinationAdd;   /*!< 04h - Destination address of the data in HOST memory       */
  CSMD_HAL_SERCFPGA_DATTYP    ulSourceAdd;        /*!< 08h - Source address-offset of the data Sercos-DPM         */
  CSMD_HAL_SERCFPGA_DATTYP    ulRdyFlagAdd;       /*!< 0Ch - Destination address of TX_RDY-Flag in HOST memory    */

} CSMD_HAL_SERCFPGA_DMA_TX_CONTROL_BLOCK;

typedef volatile struct CSMD_HAL_SERCFPGA_DMA_RX_CONTROL_BLOCK_STR
{                                                 /*   RX-DMA control block                                       */
  CSMD_HAL_SERCFPGA_DATTYP    ulLength;           /*!< 00h - Number of bytes to be transfered (divisible by 4)    */
  CSMD_HAL_SERCFPGA_DATTYP    ulDestinationAdd;   /*!< 04h - Destination address-offset of the data in Sercos-DPM */
  CSMD_HAL_SERCFPGA_DATTYP    ulSourceAdd;        /*!< 08h - Source address of the data in HOST memory            */
  CSMD_HAL_SERCFPGA_DATTYP    ulRdyFlagAdd;       /*!< 0Ch - Destination address of RX_RDY-Flag in HOST memory    */

} CSMD_HAL_SERCFPGA_DMA_RX_CONTROL_BLOCK;

typedef volatile struct CSMD_HAL_SERCFPGA_DMA_REGISTER_STR
{
  CSMD_HAL_SERCFPGA_DATTYP    ulDMA_START;          /*!< 000h - DMA_START Register                                */
  CSMD_HAL_SERCFPGA_DATTYP    ulEN_S3_TX;           /*!< 004h - Enable-Control for Sercos TX_DMA Requests         */
  CSMD_HAL_SERCFPGA_DATTYP    ulEN_S3_RX;           /*!< 008h - Enable-Control for Sercos RX_DMA Requests         */
  CSMD_HAL_SERCFPGA_DATTYP    ulRX_RDY_START;       /*!< 00Ch - Starts the RX_RDY DMA's (only for self test)      */
  CSMD_HAL_SERCFPGA_DATTYP    ulReserved_10[0x3C];  /*!< 010h ...
                                                         0FCh - reserved                                          */
  CSMD_HAL_SERCFPGA_DMA_TX_CONTROL_BLOCK 
    arTX_Control_Block[CSMD_DMA_MAX_TX_CHANNEL];    /*!< 100h ...
                                                         1F0h - TX DMA control blocks                             */
  CSMD_HAL_SERCFPGA_DMA_RX_CONTROL_BLOCK 
    arRX_Control_Block[CSMD_DMA_MAX_RX_CHANNEL];    /*!< 200h ...
                                                         2F0h - RX DMA control blocks                             */
} CSMD_HAL_SERCFPGA_DMA_REGISTER;
#endif


/*---------------------------------------------------- */
/*!            Image of IP-Core Register               */
/*---------------------------------------------------- */
typedef volatile struct CSMD_HAL_IP_CORE_REG_COPY_STR
{
  CSMD_ULONG  ulSFCR;    /*!< 70h - Sercos Frame Control Register */

} CSMD_HAL_IP_CORE_REG_COPY;


/*-------------------------------------------------- */
/*! HAL structure                                    */
/*-------------------------------------------------- */

typedef struct CSMD_HAL_STR
{ 
  CSMD_HAL_SERCFPGA_REGISTER      *prSERC_Reg;
  CSMD_HAL_SVC_RAM                *prSERC_SVC_Ram;
  CSMD_HAL_TX_RAM                 *prSERC_TX_Ram;
  CSMD_HAL_RX_RAM                 *prSERC_RX_Ram;
#ifdef CSMD_PCI_MASTER
  CSMD_HAL_SERCFPGA_DMA_REGISTER  *prSERC_DMA_Reg;
#endif
  CSMD_HAL_IP_CORE_REG_COPY        rRegCopy;

  CSMD_ULONG   ulSvcRamBase;        /*!< Offset of SVC Ram to the FPGA base address */
  CSMD_ULONG   ulTxRamBase;         /*!< Offset of Tx Ram to the FPGA base address */
  CSMD_ULONG   ulRxRamBase;         /*!< Offset of Rx Ram to the FPGA base address */

  CSMD_ULONG   ulSizeOfTxRam;       /*!< Size of available RAM for Sercos and IP TX data */
  CSMD_ULONG   ulSizeOfRxRam;       /*!< Size of available RAM for Sercos and IP RX data*/
  CSMD_ULONG   ulTxRamInUse;        /*!< Allocated FPGA Tx-Ram for Sercos telegrams (IP segment length aligned) */
  CSMD_ULONG   ulRxRamInUse;        /*!< Allocated FPGA Rx-Ram for Sercos telegrams (IP segment length aligned) */
  
  CSMD_USHORT  usIPTxRamP1Start;    /*!< IP transmit Buffer port 1 start address */
  CSMD_USHORT  usIPTxRamP1End;      /*!< IP transmit Buffer port 1 end address */
  CSMD_USHORT  usIPTxRamP2Start;    /*!< IP transmit Buffer port 2 start address */
  CSMD_USHORT  usIPTxRamP2End;      /*!< IP transmit Buffer port 2 end address */
  
  CSMD_USHORT  usIPRxRamP1Start;    /*!< IP receive Buffer port 1 start address */
  CSMD_USHORT  usIPRxRamP1End;      /*!< IP receive Buffer port 1 end address */
  CSMD_USHORT  usIPRxRamP2Start;    /*!< IP receive Buffer port 2 start address */
  CSMD_USHORT  usIPRxRamP2End;      /*!< IP receive Buffer port 2 end address */

#ifdef CSMD_HW_WATCHDOG
  CSMD_BOOL    boWD_Active;         /* IP-Core watchdog functionality is active */
#endif
  CSMD_USHORT  usFPGA_Release;      /* IP-Core release nbr.      */
  CSMD_USHORT  usFPGA_TestVersion;  /* IP-Core test version nbr. */
  
  CSMD_BOOL    boSoftMaster;        /* Sercos soft-master is used */

} CSMD_HAL;


/*-------------------------------------------------------------------------------------- */
/*! Sercos list: FPGA Telegram and Error counter                                         */
/*-------------------------------------------------------------------------------------- */
typedef struct CSMD_HAL_COMM_COUNTER_STR
{
  CSMD_USHORT  usRealLen;
  CSMD_USHORT  usMaxLen;
  CSMD_USHORT  usIPFRXOK;      /*!< Counter for error-free received frames */
  CSMD_USHORT  usIPFTXOK;      /*!< Counter for transmitted frames */
  CSMD_USHORT  usIPFCSERR;     /*!< Counter for received Ethernet frames with FCS error */
  CSMD_USHORT  usIPALGNERR;    /*!< Counter for received Ethernet frames with an alignment error */
  CSMD_USHORT  usIPDISRXB;     /*!< Counter for discarded receive Ethernet frames based on missing rx buffer resources */
  CSMD_USHORT  usIPDISCLB;     /*!< Counter for discarded forwarding Ethernet frames based on missing collision buffer resources */
  CSMD_USHORT  usIPCHVIOL;     /*!< Counter for Ethernet frames which violate the UC channel window */
  CSMD_USHORT  usIPSERCERR;    /*!< Counter for Ethernet frames with a wrong FCS or are misaligned (resetable) */
  
} CSMD_HAL_COMM_COUNTER;


#ifdef CSMD_HW_WATCHDOG
/*---------------------------------------------------- */
/*       definitions for watchdog functionality        */
/*---------------------------------------------------- */

#ifdef  CSMD_BIG_ENDIAN
/* Control word */
#define CSMD_HAL_WD_MAGIC_PATTERN     0xCD88        /* magic pattern to trigger watchdog (writing the magic pattern triggers the watchdog)
                                                       (writing inverse magic pattern disables watchdog) */
/* High word:  Status bits */
#define CSMD_HAL_WD_ACTIVE            0x0100U       /* Bit 0: watchdog is active! (default is inactive after reset) */
#define CSMD_HAL_WD_ALARM             0x0200U       /* Bit 1: watchdog alarm when actual count is zero */
/* High word:  Control bits */
#define CSMD_HAL_WDCSR_MODE_DIS_TEL   0x0001U       /* Bit 8: Disables main timing processor with alarm to stop sending Sercos frames. */

#else

/* Control word */
#define CSMD_HAL_WD_MAGIC_PATTERN     0x88CD        /* magic pattern to trigger watchdog (writing the magic pattern triggers the watchdog)
                                                       (writing inverse magic pattern disables watchdog) */
/* High word:  Status bits */
#define CSMD_HAL_WD_ACTIVE            0x0001U       /* Bit 0: watchdog is active! (default is inactive after reset) */
#define CSMD_HAL_WD_ALARM             0x0002U       /* Bit 1: watchdog alarm when actual count is zero */
/* High word:  Control bits */
#define CSMD_HAL_WDCSR_MODE_DIS_TEL   0x0100U       /* Bit 8: Disables main timing processor with alarm to stop sending Sercos frames. */
#endif


/* Mode for reaction to an watchdog timeout */
#define CSMD_HAL_WD_TO_SEND_EMPTY_TEL     0U
#define CSMD_HAL_WD_TO_DISABLE_TX_TEL     1U

typedef struct CSMD_HAL_WDSTATUS_STR
{
  CSMD_USHORT usActCount;   /*!< actual value of watchdog counter (WDCNT bit 31 - 16) */
  CSMD_BOOL   boActive;     /*!< watchdog is active (WDCSR bit 16) */
  CSMD_BOOL   boAlarm;      /*!< watchdog alarm. Actual watchdog count reaches zero (WDCSR bit 17) */
  
} CSMD_HAL_WDSTATUS;
#endif


/*---------------------------------------------------- */
/*          definitions for DivClk functionality       */
/*---------------------------------------------------- */
#define CSMD_HAL_DIVCLK_MAX_PULSES  (255)

/*---------------------------------------------------- */
/* Sercos cycle time constants [nano seconds]          */
/*---------------------------------------------------- */
#define CSMD_HAL_TSCYC_MIN                  (31250)
#define CSMD_HAL_TSCYC_MAX               (65000*1000)




#ifdef CSMD_ACTIVATE_AUTONEGOTIATION
/****************************************
* define PHY Internal Access Registers
* bit serial bypin programming via
* the MII Register defined above
*****************************************/
#define CSMD_PHYCR                  (0)     /* internal control register address*/
#define CSMD_PHYSR                  (1)     /* internal status  register address*/
#define CSMD_PHYID1                 (2)     /* internal PHY identification register 1 address*/
#define CSMD_PHYID2                 (3)     /* internal PHY identification register 2 address*/
#define CSMD_PHYAN                  (4)     /* internal Auto-Negotiation advertisement*/

/*******************************************************
* defines for PHY Internal Control Register 
*******************************************************/
#define CSMD_PHYCR_ANENAB           (1<<12) /* auto negotiation enable */
#define CSMD_PHYCR_ANRESTART        (1<<9)  /* auto negotiation restart */
#define CSMD_PHYCR_FD               (1<<8)  /* full duplex mode */ 
#define CSMD_PHYCR_SPMSB            (1<<6)  /* speed msbit */
#define CSMD_PHYCR_SPLSB            (1<<13) /* speed lsbit */ 
#define CSMD_PHYCR_SP100            (1<<13) /* 100 Mbps */

/*******************************************************
* defines for PHY Internal Status Register 
*******************************************************/
#define CSMD_PHYSR_100BASE_T4       (1<<15) /* phy able to perform 100BASE_T4 */
#define CSMD_PHYSR_100BASE_X_FD     (1<<14) /* phy able to perform full duplex 100BASE_X */
#define CSMD_PHYSR_100BASE_T2_FD    (1<<10) /* phy able to perform full duplex 100BASE_T2 */
#define CSMD_PHYSR_ANCOMP           (1<<5)  /* phy auto negotiation complete */
#define CSMD_PHYSR_ANABLE           (1<<3)  /* phy able to auto negotiation */
#define CSMD_PHYSR_LINK             (1<<2)  /* phy link is up */

/***********************************************************
* defines for PHY Internal Auto negotiation advertisement register
************************************************************/
#define CSMD_PHYAN_DIS_10MBIT       0x0181  /* disable 10BASE-T capability */

#endif  /* #ifdef CSMD_ACTIVATE_AUTONEGOTIATION */



/*###########################################################################*/
/*-------------------------------{ Macros }----------------------------------*/
/*###########################################################################*/

/* --> CSMD_HAL_PRIV.h */


/*! \endcond */ /* HAL */

#ifdef __cplusplus
} // extern "C"
#endif


#endif /* _CSMD_HAL_GLOB */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

01 Sep 2010
  - Refactoring of CoSeMa files.
20 Nov 2013 WK
  - Defdb00150926
    Fixed Lint message 740 "Unusual pointer cast (incompatible indirect types)"
    for CSMD_MAX_HW_CONTAINER > 0.
23 Oct 2014 WK
  Defdb00174315
  Added new registers (IP-Core > 4.9) for new Sercos time mechanism (non standard!):
    CSMD_HAL_SERCFPGA_DATTYP    ulSTNSOffset
    CSMD_HAL_SERCFPGA_DATTYP    ulSTNSNew
    CSMD_HAL_SERCFPGA_DATTYP    ulSTSECNew
11 Feb 2015 WK
  - Defdb00176768
    Added element boSoftMaster to structure type CSMD_HAL.
25 Feb 2015 WK
  - FEAT-00059338 - Conversion of redefined data types to CSMD types.
28 Apr 2015 WK
  - Defdb00178597
    Added elements in structure type CSMD_HAL:
      ULONG  ulSvcRamBase;  Offset of SVC Ram to the FPGA base address
      ULONG  ulTxRamBase;   Offset of Tx Ram to the FPGA base address
      ULONG  ulRxRamBase;   Offset of Rx Ram to the FPGA base address
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
28 Jan 2016 WK
  - Defdb00182067
    Function macro CSMD_HAL_WatchdogSet:
    Fixed lint warning 569 respectively compiler warning in macro
    CSMD_HAL_WatchdogSet caused by CSMD_HAL_WD_MAGIC_PATTERN.
  
------------------------------------------------------------------------------
*/
