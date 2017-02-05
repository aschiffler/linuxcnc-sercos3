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
\file   CSMD_SERC_MON_CFG.h
\author ROL
\date   01.09.2013
\brief  This File contains the private function prototypes
        and macro definitions for the file CSMD_SERC_MON_CFG.c
*/


#ifndef _CSMD_SERC_MON
#define _CSMD_SERC_MON

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*! \cond PRIVATE */

#ifdef CSMD_SERCOS_MON_CONFIG


/*---- Definition resp. Declaration private Constants and Macros: ------------*/


/* Note:  Set INDENT_WIDTH to 0L if no indentation necessary. */
#if(1)
/* Use spaces for indentation */
#define INDENT_CHAR             0x20    /* ASCII Space */
#define INDENT_WIDTH              2L    /* 2 spaces */
#define MAX_INDENTATION_MASK  0x3FUL    /* Max. indentation = 63 spaces */
#else
/* Use tabs for indentation */
#define INDENT_CHAR             0x09    /* ASCII horizontal tab */
#define INDENT_WIDTH              1L    /* 1 tab */
#define MAX_INDENTATION_MASK  0x1FUL    /* Max. indentation = 31 Tabs */
#endif

const CSMD_CHAR XML_Header[] = {"<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\"?>\n"};

/* Sercos Monitor Version */
#define CSMD_SERCMON_MAJOR_VER          2
#define CSMD_SERCMON_MINOR_VER          5

/* Sercos Monitor Network Configuration Version */
#define CSMD_SERCMON_CFG_MAJOR_VER      1
#define CSMD_SERCMON_CFG_MINOR_VER      1

#define CSMD_MIN_BUFFER_LENTGH          0x4000UL    /* 16 kByte */
#define CSMD_MAX_EXPECTED_LINE_LENGTH   120UL

#define PrimaryPort   "PrimaryPort"
#define SecondaryPort "SecondaryPort"
#define BothPorts     "BothPorts"

#define CSMD_RESOURCE_UNKNOWN           0UL
#define CSMD_RESOURCE_FSP_DRIVE         1UL
#define CSMD_RESOURCE_FSP_IO            2UL
#define CSMD_RESOURCE_FSP_ENCODER       5UL
#define CSMD_RESOURCE_FSP_POWER_SUPPLY  7UL

#define CSMD_IDN_SI_MASK            0xFF000000UL                  /* Mask for IDN structure instance */
/* Defines for FSP IO */
#define CSMD_FSP_TYPE_IO            1
#define CSMD_FSP_VERS_IO            1
#define CSMD_IDN_S_1500_0_1         CSMD_IDN_S_0_( 1000, 0, 1 )   /* S-0-1500.x.01 IO Control */
#define CSMD_IDN_S_1500_0_2         CSMD_IDN_S_0_( 1000, 0, 2 )   /* S-0-1500.x.02 IO Status */
/* Defines for FSP Drive */
#define CSMD_FSP_TYPE_DRIVE         2
#define CSMD_FSP_VERS_DRIVE         1
#define CSMD_IDN_S_0_0134           134U                          /* Drive control */
#define CSMD_IDN_S_0_0135           135U                          /* Drive status */
/* Defines for FSP Encoder */
#define CSMD_FSP_TYPE_ENCODER       5
#define CSMD_FSP_VERS_ENCODER       1
#define CSMD_IDN_S_0605_0_1         CSMD_IDN_S_0_( 605, 0, 1 )    /* S-0-0605.x.01 Encoder control */
#define CSMD_IDN_S_0600_0_1         CSMD_IDN_S_0_( 600, 0, 1 )    /* S-0-0600.x.01 Encoder status */
/* Defines for FSP PowerSupply */
#define CSMD_FSP_TYPE_POWERSUPPLY   7
#define CSMD_FSP_VERS_POWERSUPPLY   1
#define CSMD_IDN_S_1720_0_1         CSMD_IDN_S_0_( 1720, 0, 1 )   /* S-0-1720.0.01 Power supply control */
#define CSMD_IDN_S_1720_0_2         CSMD_IDN_S_0_( 1720, 0, 2 )   /* S-0-1720.0.02 Power supply status */


/* ---------------------------------------------- */
/* Definitions for S-0-1050.x.01 Connection setup */
/* ---------------------------------------------- */
#define CONN_SETUP_USAGE                      0x8000    /* 15:    usage of configuration                     */
#define CONN_SETUP_FUNCTION                   0x4000    /* 14:    function within connection                 */
#define CONN_SETUP_SOURCE                     0x3000    /* 13-12: source of connection configuration         */
#define CONN_SETUP_SOURCE_MASTER              0x0000    /*          00:  source is master                    */
#define CONN_SETUP_SOURCE_EXTERNAL            0x2000    /*          10:  source external                     */
#define CONN_SETUP_SOURCE_EXPECTATION         0x0040    /* 6:     Expectation of application data processing */
#define CONN_SETUP_SOURCE_TYPE                0x0030    /* 5-4:   type of configuration                      */
#define CONN_SETUP_SOURCE_TYPE_LIST           0x0000    /*          00:  IDN list (SE6 relevant)             */
#define CONN_SETUP_SOURCE_TYPE_CONTAIN        0x0010    /*          01:  Container (SE5 relevant)            */
#define CONN_SETUP_SOURCE_TYPE_FSP_DRV        0x0020    /*          10:  Tel.type par. (S-0-0015 relevant)   */
#define CONN_SETUP_SOURCE_TYPE_CONN           0x0003    /* 1-0:   type of connection                         */
#define CONN_SETUP_SOURCE_TYPE_CONN_SYNC      0x0000    /*          00:  clock synchronous                   */
#define CONN_SETUP_SOURCE_TYPE_CONN_ASYNC_WD  0x0001    /*          01:  non-cyclic type 1 (with watch dog)  */
#define CONN_SETUP_SOURCE_TYPE_CONN_ASYNC     0x0002    /*          10:  non-cyclic type 2 (no watch dog     */
/*#define CONN_SETUP_SOURCE_TYPE_CONN_CYCLIC  0x0003*/  /*          11:  cyclic connection                   */

/*---- Declaration private Types: --------------------------------------------*/

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/

#ifdef __cplusplus
extern "C"
{
#endif

/* Public function moved to CSMD_GLOB.h */
/* 
SOURCE CSMD_FUNC_RET CSMD_Serc_Mon_Config
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  const CSMD_ULONG           ulMaxBufferLength,
                                  CSMD_CHAR                 *pcText,
                                  CSMD_ULONG                *pulLength );
*/

CSMD_FUNC_RET CSMD_Serc_Mon_PluginConfig
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_CHAR           *pcName,
                                  const CSMD_LONG            lMajorRevision,
                                  const CSMD_LONG            lMinorRevision,
                                  const CSMD_CHAR           *pcName1 );

CSMD_FUNC_RET CSMD_Serc_Mon_Plugin
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_CHAR           *pcName,
                                  const CSMD_LONG            lMajorRevision,
                                  const CSMD_LONG            lMinorRevision );

CSMD_FUNC_RET CSMD_Serc_Mon_Setting_Header
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_LONG            lMajorRevision,
                                  const CSMD_LONG            lMinorRevision );

CSMD_FUNC_RET CSMD_Serc_Mon_FramePlugin
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_CHAR           *pcName1 );

CSMD_FUNC_RET CSMD_Serc_Mon_GenericNodes
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_CHAR           *pcName1 );

CSMD_FUNC_RET CSMD_Serc_Mon_SercosPlugin
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation );

CSMD_FUNC_RET CSMD_Serc_Mon_SmpPlugin
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_CHAR           *pcName1 );

CSMD_FUNC_RET CSMD_Serc_Mon_MasterConfig
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation );

CSMD_FUNC_RET CSMD_Serc_Mon_NetworkTopology
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  const CSMD_ULONG          *pulIndentation,
                                  CSMD_CHAR                 *pcElementName,
                                  CSMD_USHORT                usTopology );

CSMD_FUNC_RET CSMD_Serc_Mon_NetworkComponents
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation );

CSMD_FUNC_RET CSMD_Serc_Mon_All_Slave_Configs
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation );

CSMD_FUNC_RET CSMD_Serc_Mon_All_ConfigurationSlave
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation );

CSMD_VOID CSMD_Serc_Mon_Topology_Port
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_BOOL                 *pboPort1,
                                  CSMD_BOOL                 *pboPort2,
                                  CSMD_CHAR                **pacPort1,
                                  CSMD_CHAR                **pacPort2 );

CSMD_FUNC_RET CSMD_Serc_Mon_SlaveConfig
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_CHAR           *pcMasterP,
                                  const CSMD_ULONG           ulTopologyI,
                                  const CSMD_ULONG           ulSercosA,
                                  CSMD_ULONG                 ulSlaveIdx,
                                  CSMD_BOOL                  boSlaveForTimeEval );
       
CSMD_FUNC_RET CSMD_Serc_Mon_DeviceSvc
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  const CSMD_ULONG          *pulIndentation,
                                  const CSMD_CHAR           *pcMdtOrAt,
                                  const CSMD_ULONG           ulTopologyI );

CSMD_FUNC_RET CSMD_Serc_Mon_Resources
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  CSMD_ULONG                 ulSlaveIdx,
                                  const CSMD_CONFIG_STRUCT  *prConfiguration );

CSMD_FUNC_RET CSMD_Serc_Mon_Timing
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  CSMD_ULONG                 ulSlaveIdx,
                                  CSMD_BOOL                  boEvalTiming );

CSMD_FUNC_RET CSMD_Serc_Mon_ConfigurationSlave
                                ( const CSMD_INSTANCE       *prCSMD_Instance,
                                  CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_ULONG           ulConnectionInstance,
                                  const CSMD_CONN_IDX_STRUCT arConnIdxList,
                                  const CSMD_ULONG           ulTopologyI,
                                  const CSMD_ULONG           ulSercosA );

CSMD_FUNC_RET CSMD_Serc_Mon_ConfigSlave_Unused
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_ULONG           ulConnectionInstance,
                                  const CSMD_ULONG           ulTopologyI,
                                  const CSMD_ULONG           ulSercosA );

CSMD_FUNC_RET CSMD_Serc_Mon_ConnectionSetup
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_USHORT          usConnectionSetup );

CSMD_FUNC_RET CSMD_Serc_Mon_TelegramOffset
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_USHORT          usTelegramOffset );

CSMD_FUNC_RET CSMD_Serc_Mon_ConnectionElements
                                ( CSMD_CHAR                **pcText,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  const CSMD_ULONG          *pulIDN_Name );

CSMD_FUNC_RET CSMD_Serc_ConvIdnToString
                                ( const CSMD_ULONG           ulIDN,
                                  CSMD_CHAR                 *pcIdnString,
                                  const CSMD_ULONG           ulStringLen );

CSMD_CHAR * CSMD_Serc_XML_Node_Begin
                                ( CSMD_CHAR                 *pcString,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  CSMD_CHAR                  acStartTAG[] );

CSMD_CHAR * CSMD_Serc_XML_Node_End
                                ( CSMD_CHAR                 *pcString,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_ULONG                *pulIndentation,
                                  CSMD_CHAR                  acEndTAG[] );

CSMD_CHAR * CSMD_Serc_XML_Element_Begin
                                ( CSMD_CHAR                 *pcString,
                                  const CSMD_CHAR           *pcEnd,
                                  const CSMD_ULONG          *pulIndentation,
                                  CSMD_CHAR                  acElement[] );

CSMD_CHAR * CSMD_Serc_XML_Element_End
                                ( CSMD_CHAR                 *pcString,
                                  const CSMD_CHAR           *pcEnd,
                                  CSMD_CHAR                  acElement[] );

CSMD_CHAR * CSMD_Serc_XML_Element_Dezimal
                                ( CSMD_CHAR                 *pcString,
                                  const CSMD_CHAR           *pcEnd,
                                  const CSMD_ULONG          *pulIndentation,
                                  CSMD_LONG                  lValue,
                                  CSMD_CHAR                  acElement[] );

CSMD_CHAR * CSMD_Serc_XML_Element_String
                                ( CSMD_CHAR                 *pcString,
                                  const CSMD_CHAR           *pcEnd,
                                  const CSMD_ULONG          *pulIndentation,
                                  const CSMD_CHAR            acValue[],
                                  CSMD_CHAR                  acElement[] );

CSMD_CHAR * CSMD_Serc_XML_Empty_Element
                                ( CSMD_CHAR                 *pcString,
                                  const CSMD_CHAR           *pcEnd,
                                  const CSMD_ULONG          *pulIndentation,
                                  CSMD_CHAR                  acElement[] );

#ifdef __cplusplus
} // extern "C"
#endif

#endif  /* End: #ifdef CSMD_SERCOS_MON_CONFIG */

/*! \endcond */ /* PRIVATE */

#endif /* _CSMD_SERC_MON */




/*
--------------------------------------------------------------------------------
  Modification history
--------------------------------------------------------------------------------

07-Jan-2014 RL: CSMD Sercos Monitor Configuration
    -first Version 
16 Apr 2014 WK
  FEAT-00051252 - Generation of configuration files for the Sercos Monitor
  - Added function prototypes:
    CSMD_Serc_XML_Node_Begin(), CSMD_Serc_XML_Node_End()
    CSMD_Serc_XML_Element_Begin(), CSMD_Serc_XML_Element_End()
    CSMD_Serc_XML_Empty_Element()
23 Apr 2014 WK
  - Added function prototypes:
    CSMD_Serc_Mon_NetworkTopology(), CSMD_Serc_Mon_NetworkComponents()
24 Apr 2014 WK
  - Added function prototypes:
    CSMD_Serc_XML_Element_Dezimal(), CSMD_Serc_XML_Element_String()
  - Removed string defines.
  - Indentation with spaces or tabs. No indentation also possible.
25 Apr 2014 WK
  - Added function prototypes:
    CSMD_Serc_Mon_All_ConfigurationSlave(), CSMD_Serc_Mon_Topology_Port()
29 Apr 2014 Wk
  - Deleted defines for xml element lengths.
  - Added function prototypes:
    CSMD_Serc_Mon_Resources(), CSMD_Serc_Mon_ConfigSlave_Unused()
20 Aug 2014 WK
  - Replace SOURCE with static in all prototypes.
02 Feb 2015 WK
  - Added function prototype:
    CSMD_Serc_Mon_MasterConfig()
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
18 Jul 2016 WK
  - Added Major/Minor version for SercosMonitor.
  - Added function prototype: CSMD_Serc_Mon_Setting_Header()
  - Defdb00182067
    Static function declaration removed.
  
--------------------------------------------------------------------------------
*/
