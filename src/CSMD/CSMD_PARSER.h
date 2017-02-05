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
\file   CSMD_PARSER.h
\author MSt
\date   19.11.2010
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_PARSER.c
*/


#ifndef _CSMD_PARSER
#define _CSMD_PARSER

#ifdef CSMD_CONFIG_PARSER

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
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

#if (defined CSMD_BIG_ENDIAN) && (!defined CSMD_PRESERVE_END_FOR_LENGTH)

#define  CSMD_LENGTH_CONV(_x)   CSMD_REVERSE_CSMD_USHORT(_x) /* Reverses the byte order inside 2-byte data.   */

#else /* Little endian */

#define  CSMD_LENGTH_CONV(_x)   (_x)                    /* Leave the byte order, only for length info.   */

#endif



/*! \cond PUBLIC */

/*---- Definition resp. Declaration public Constants and Macros: -------------*/
#define BIN_CONFIG_VERSION_01_01      (0x0101)
#define CSMD_UNIVERSAL_APP_ID         (0x0)

/**
\def CSMD_PREVENT_NULL
Prevent consumer or producer from being 0
*/
#define CSMD_PREVENT_NULL             1U

#define CSMD_FILE_END_SIGN            0x45444E65U   /* "EDNe" */
#define CSMD_TABLE_END_SIGN           0x5E7E5E7EU   /* "^-^-" */

#define CSMD_CONN_TABLE_HEADER        0x636E6E43U   /* "cnnC" */
#define CSMD_PRDC_TABLE_HEADER        0x63647250U   /* "cdrP" */
#define CSMD_CNLS_TABLE_HEADER        0x734C6E43U   /* "sLnC" */
#define CSMD_CNSM_TABLE_HEADER        0x6D736E43U   /* "msnC" */
#define CSMD_CONF_TABLE_HEADER        0x67666E43U   /* "gfnC" */
#define CSMD_RTBT_TABLE_HEADER        0x74425452U   /* "tBTR" */

#define CSMD_TABLE_DUMMY              0x4241U       /* "BA" */
#define CSMD_MASTER_PROD_AT           0x4343U       /* "CC" */

#define CSMD_LIST_HEADER_LEN          12
#define CSMD_CBINC_VERSION_LEN         2
#define CSMD_TABLE_HEADER_LEN          4
#define CSMD_TABLE_OVERHEAD           (CSMD_USHORT)(CSMD_TABLE_HEADER_LEN + CSMD_END_SIGN_LENGTH)
#define CSMD_CONNECTION_TABLE_LEN     (CSMD_USHORT)(14 + CSMD_CONN_NAME_LENGTH)
#define CSMD_PRODUCER_TABLE_LEN       16
#define CSMD_CONSUMER_TABLE_LEN       16
#define CSMD_ONE_CONSUMER_LEN          8
#define CSMD_CONFIG_TABLE_LEN_WO_IDN   8
#define CSMD_RTBITS_TABLE_LEN         (CSMD_USHORT)(4 + CSMD_MAX_RT_BITS_PER_CONN * 6)
#define CSMD_SLAVE_SETUP_TABLE_LEN     4
#define CSMD_FILE_END_LENGTH           4
#define CSMD_END_SIGN_LENGTH           4

/*---- Declaration public Types: ---------------------------------------------*/



/*---- Declaration public Functions: -----------------------------------------*/

/*! \endcond */ /* PUBLIC */


/*! \cond PRIVATE */

/*---- Definition resp. Declaration private Constants and Macros: ------------*/
#define CSMD_NUMBER_OF_APPLICATIONS    4

/*---- Declaration private Types: --------------------------------------------*/

typedef struct CSMD_CONN_PROD_CONS_STR
{
  CSMD_USHORT  usConnection_Producer_Index;    /*!< producer-index of the connection */
  CSMD_USHORT  usConnection_Consumer_Index;    /*!< consumer-index of the connection */
  
} CSMD_CONN_PROD_CONS;

typedef struct CSMD_CONNECTION_TABLE_STR
{
  CSMD_USHORT  usConnectionKey;                          /*!< Key for the connection; at present not used */
  CSMD_USHORT  usConnectionNbr;                          /*!< Connection Number */
  CSMD_USHORT  usTelegramAssignment;                     /*!< Telegram Assignment */
  CSMD_USHORT  usConnectionLength;                       /*!< Current length of connection */
  CSMD_USHORT  usApplicationID;                          /*!< Application identifier */
  CSMD_UCHAR   ucConnectionName[CSMD_CONN_NAME_LENGTH];  /*!< Name of the connection */
  CSMD_USHORT  usProducerKey;                            /*!< Key for the Producer */
  CSMD_USHORT  usConsumerListKey;                        /*!< Key for the Consumer */

} CSMD_CONNECTION_TABLE;

typedef struct CSMD_PRODUCER_TABLE_STR
{
  CSMD_USHORT  usProducerKey;                            /*!< Key for the producer */
  CSMD_USHORT  usSERCOS_Add;                             /*!< Connection Number */
  CSMD_ULONG   ulConnectionCycleTime;                    /*!< Cycle time of connection */
  CSMD_USHORT  usConnectionInstance;                     /*!< Used connection instance */
  CSMD_USHORT  usDummy;                                  /*!< a dummy for long alignment */
  CSMD_USHORT  usConfigurationKey;                       /*!< Key for the Configuration */
  CSMD_USHORT  usRTBitsKey;                              /*!< Key for the RT bits */

} CSMD_PRODUCER_TABLE;

typedef struct CSMD_CONSLIST_TABLE_HEADER_STR
{
  CSMD_USHORT  usConsumerListKey;                        /*!< Key for the consumer list */
  CSMD_USHORT  usNumberOfConsumers;                      /*!< Number of consumers for this connection */
  CSMD_USHORT  usFirstConsumerKey;                       /*!< Key for the 1st consumer */

} CSMD_CONSLIST_TABLE_HEADER;

typedef struct CSMD_CONSUMER_TABLE_STR
{
  CSMD_USHORT  usConsumerKey;                            /*!< Key for the consumer */
  CSMD_USHORT  usSERCOS_Add;                             /*!< Connection Number */
  CSMD_ULONG   ulConnectionCycleTime;                    /*!< Cycle time of connection */
  CSMD_USHORT  usConnectionInstance;                     /*!< Used connection instance */
  CSMD_USHORT  usAllowedDataLosses;                      /*!< Allowed data losses for this slave in this connection */
  CSMD_USHORT  usConfigurationKey;                       /*!< Key for the Configuration */
  CSMD_USHORT  usRTBitsKey;                              /*!< Key for the RT bits */

} CSMD_CONSUMER_TABLE;

typedef struct CSMD_CONFIG_TABLE_HEADER_STR
{
  CSMD_USHORT  usConfigKey;                              /*!< Key for the configuration */
  CSMD_USHORT  usConnectionSetup;                        /*!< setup for S-0-1050.x.1 */
  CSMD_USHORT  usConnectionCapability;                   /*!< data for S-0-1050.x.7 */
  CSMD_USHORT  usNumberOfIDNS;                           /*!< Number of configured IDNs for this configuration */

} CSMD_CONFIG_TABLE_HEADER;

typedef struct CSMD_RTBITS_TABLE_STR
{
  CSMD_USHORT  usRTBitsKey;                              /*!< Key for the real time bits configuration */
  CSMD_USHORT  usDummy;                                  /*!< a dummy for long alignment */
  CSMD_ULONG   ulRTBit_IDN[CSMD_MAX_RT_BITS_PER_CONN];   /*!< data for S-0-1050.x.20 */
  CSMD_USHORT  usBit_in_IDN[CSMD_MAX_RT_BITS_PER_CONN];  /*!< data for S-0-1050.x.21 */

} CSMD_RTBITS_TABLE;

typedef struct CSMD_SLAVE_SETUP_TABLE_STR
{
  CSMD_USHORT  usSlaveAddress;                           /*!< Sercos address of the configured slave */
  CSMD_USHORT  usSetupParamsListKey;                     /*!< Key for the config parameters list */

} CSMD_SLAVE_SETUP_TABLE;

typedef struct CSMD_PARAMSLIST_TABLE_HEADER_STR
{
  CSMD_USHORT  usSetupParamsListKey;                     /*!< Key for the config parameters list */
  CSMD_USHORT  usParamsApplicationID;                    /*!< Application ID for this parameter list */
  CSMD_USHORT  usNumberOfConfigParams;                   /*!< Number of configuration parameters in the list */
  CSMD_USHORT  usFirstParameterKey;                      /*!< Key for the 1st parameter */

} CSMD_PARAMSLIST_TABLE_HEADER;

typedef struct CSMD_PARAMETER_TABLE_HEADER_STR
{
  CSMD_USHORT  usParameterKey;                           /*!< Key for the parameter */
  CSMD_USHORT  usDataLength;                             /*!< Length of the parameter data */
  CSMD_ULONG   ulParameterIDN;                           /*!< IDN of the parameter */

} CSMD_PARAMETER_TABLE_HEADER;

typedef struct CSMD_TABLE_POINTER_STR
{
  CSMD_USHORT *pusCnncStartPtr;
  CSMD_USHORT *pusPrdcStartPtr;
  CSMD_USHORT *pusConsLStartPtr;
  CSMD_USHORT *pusConsTableStartPtr;
  CSMD_USHORT *pusConfigTableStartPtr;
  CSMD_USHORT *pusRTBitsTableStartPtr;
  CSMD_USHORT *pusConnConfigEndPtr;
  CSMD_USHORT *pusSlaveSetupStartPtr;
  CSMD_USHORT *pusSetupParametersListStartPtr;
  CSMD_USHORT *pusSetupParameterStartPtr;

} CSMD_TABLE_POINTER;

typedef enum CSMD_HEADER_EN
{
  CSMD_CONN_HEADER = 0, /*! Connections table, not used but for completeness */
  CSMD_PROD_HEADER,
  CSMD_CONSLIST_HEADER,
  CSMD_CONSUMER_HEADER,
  CSMD_CONFIG_HEADER,
  CSMD_RTBITS_HEADER,
  CSMD_SLAVESETUP_HEADER,
  CSMD_PARALIST_HEADER,
  CSMD_PARAMS_HEADER,
  CSMD_FILE_END
  
} CSMD_HEADER_ENUM;


/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

CSMD_FUNC_RET CSMD_CheckCbinC_Header      ( const CSMD_USHORT                  *pusHeaderPtr );

CSMD_FUNC_RET CSMD_SearchTableHeaders     ( CSMD_USHORT                        *pusActDataPtr,
                                            CSMD_USHORT                         usListLength,
                                            CSMD_TABLE_POINTER                 *prTablePtrList );

CSMD_FUNC_RET CSMD_CheckTableSlaveAddr    ( const CSMD_SLAVE_LIST              *prSlaveList,
                                            const CSMD_TABLE_POINTER           *prTableHeaderPtr,
                                            CSMD_USHORT                         usNumberOfProducers,
                                            CSMD_USHORT                         usNumberOfConsumers );

CSMD_FUNC_RET CSMD_ClearAppID_Connections ( CSMD_INSTANCE                      *prCSMD_Instance,
                                            const CSMD_CONNECTION_TABLE        *prConnections,
                                            CSMD_USHORT                         usNumberOfConnections,
                                            CSMD_BOOL                           boAddCheck );

CSMD_FUNC_RET CSMD_ClearAppID_SetupParams ( CSMD_INSTANCE                      *prCSMD_Instance,
                                            const CSMD_PARAMSLIST_TABLE_HEADER *prParamsList,
                                            const CSMD_USHORT                  *pusParamTablePtr );

CSMD_FUNC_RET CSMD_CheckConnTableKeys     ( const CSMD_TABLE_POINTER           *prTableHeaderPtr,
                                            CSMD_USHORT                         usNumberOfConnections,
                                            CSMD_USHORT                         usNumberOfProducers );

CSMD_FUNC_RET CSMD_CheckParamConfTableKeys( const CSMD_TABLE_POINTER           *prTableHeaderPtr );

CSMD_FUNC_RET CSMD_GetUsedCfgStructs      ( const CSMD_INSTANCE                *prCSMD_Instance,
                                            CSMD_USED_MARKER                   *prUsedMarker,
                                            CSMD_BOOL                           boAddCheck );

CSMD_VOID CSMD_Write_Header               ( CSMD_USHORT                       **ppusActListPos,
                                            CSMD_USHORT                        *pusActLength,
                                            CSMD_HEADER_ENUM                    eHeaderInfo );

#ifdef __cplusplus
} // extern "C"
#endif

/*! \endcond */ /* PRIVATE */

#endif /* #ifdef CSMD_CONFIG_PARSER */

#endif /* _CSMD_PARSER */




/*
------------------------------------------------------------------------------
  Modification history
------------------------------------------------------------------------------

19 Nov 2010
  - File created.
28 Apr 2015 WK
  - Defdb00178597
    Fixed type castings for 64-bit systems.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
  
------------------------------------------------------------------------------
*/
