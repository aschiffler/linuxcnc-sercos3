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
\file   CSMD_USER.h
\author WK
\date   12.05.2005
\brief  This File contains the CoSeMa global user defines
        for conditional compiling and size of structures.
*/


#ifndef _CSMD_USER
#define _CSMD_USER

#if defined OFFLINE
#pragma warning( disable : 4068 )  /* Disable warning messages in Visual C++ */
#endif


/*---- Declaration public Types: ---------------------------------------------*/

/*---- Definition resp. Declaration public Constants and Macros: -------------*/

/*---- Definition resp. Declaration public Variables: ------------------------*/

/*---- Declaration public Functions: -----------------------------------------*/

/* This file contains CoSeMa global compiler macros 
   to make system-specific settings.
  
  +---------------------------------------------------+
  | Name of the define                                |
  +--------------+------------------------------------+
  | Input values |       Description                  |
  +--------------+------------------------------------+
  
 */


/*!\todo translate: Description of CSMD_STATIC_MEM_ALLOC */
#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_STATIC_MEM_ALLOC
                      </B></TD></TR><TR><TD width=120></TD><TD width=800>
    Define the memory allocation for the most arrays/structures inside the global structure CSMD_INSTANCE.\n
    - Bei dynamischer Speichervergabe erfolgt der Zugriff auf die meisten arrays/structuren ueber pointer.\n
      Zur Initialisierung werden die Funktionenn CSMD_InitSystemLimits() und CSMD_InitMemory() benoetigt.

    \#define  &nbsp;        Static memory allocation<BR>
    \#undef   &nbsp;&nbsp;  Dynamic memory allocation
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_STATIC_MEM_ALLOC
#else
#define CSMD_STATIC_MEM_ALLOC
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_BIG_ENDIAN
                   </B></TD></TR><TR><TD width=120></TD><TD width=800>
    Processor endianness<BR>
    
    \#define  &nbsp;        Big endian machine<BR>
    \#undef   &nbsp;&nbsp;  Little endian machine
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_BIG_ENDIAN
#else
#undef  CSMD_BIG_ENDIAN         /* little endian machine */
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_PRESERVE_END_FOR_LENGTH
                                </B></TD></TR><TR><TD width=120></TD><TD width=800>
    Only in case of big endianness and parser.
    The connection configuration via the binary CSMCfg_bin format is a Sercos list where the <BR>
    elements are little endian. If this list is given by a system which is big endian the <BR>
    length information might be already big endian. This would lead to an error if the <BR>
    length information is swapped like the data. To avoid this, this macro has to be defined.<BR>
    
    \#define   &nbsp;       List length is taken as is<BR>
    \#undef    &nbsp;&nbsp; List length is converted in the same way as the list data
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_PRESERVE_END_FOR_LENGTH
#else
#undef  CSMD_PRESERVE_END_FOR_LENGTH         /* little endian machine */
#endif


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_TYPE_HEADER
                    </B></TD></TR><TR><TD width=120>
    Filename        </TD><TD width=800>
    The file with this name contains the global type definitions and is included 
    in all CoSeMa files. (e.g. "CSMD_TYPE.h")
    </TD></TR></TABLE>
*/
#define CSMD_TYPE_HEADER        "../GLOB/GLOB_TYPE.h"


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_IP_HEADER
                  </B></TD></TR><TR><TD width=120>
    Filename      </TD><TD width=800>
    Name of the Include file with the CoSeMa-specific \#defines and structures required for an UC channel driver.<BR>
    If defined, this file is checked for plausibility during the compilation process. (e.g. 'SIII_IP.h')
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_IP_HEADER  "SIII_IP.h"
#else
#define CSMD_IP_HEADER        "SIII_IP.h"
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_CONFIG_PARSER
                      </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Parser for binary connection configuration available<BR>
    \#undef   &nbsp;&nbsp;  Parser for binary connection configuration not available
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_CONFIG_PARSER
#else
#define CSMD_CONFIG_PARSER
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_CONFIGURATION_PARAMETERS
                                 </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Optional slave setup parameter configuration available<BR>
    \#undef   &nbsp;&nbsp;  Optional slave setup parameter configuration not available<BR><BR>
    (Optional slave setup parameter configuration is also available with defined \ref CSMD_CONFIG_PARSER !)
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_CONFIGURATION_PARAMETERS
#else
#undef CSMD_CONFIGURATION_PARAMETERS
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_4MDT_4AT_IN_CP1_2
                          </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Configuration of 4 MDT and 4 AT in CP1 and CP2 is possible<BR>
    \#undef   &nbsp;&nbsp;  Configuration of 4 MDT and 4 AT in CP1 and CP2 is not possible
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_4MDT_4AT_IN_CP1_2
#else
#define CSMD_4MDT_4AT_IN_CP1_2
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_PCI_MASTER
                   </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define &nbsp;       For systems with PCI bus master DMA functionality<BR>
    \#undef  &nbsp;&nbsp; PCI bus master DMA functionality is not supported
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_PCI_MASTER
#else
#undef  CSMD_PCI_MASTER
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_PCI_MASTER_EVENT_DMA
                             </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        For system that supports additional event controled DMA.<BR>
    \#undef   &nbsp;&nbsp;  Event controlled PCI bus master DMA is not supported.
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_PCI_MASTER_EVENT_DMA
#else
#undef CSMD_PCI_MASTER_EVENT_DMA
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_SYNC_DPLL
                  </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        For system with DPLL synchronized CYC_CLK functionality.<BR>
    \#undef   &nbsp;&nbsp;  DPLL for CYC_CLK is not supported.
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_SYNC_DPLL
#else
#undef  CSMD_SYNC_DPLL
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_DISABLE_CMD_S_0_1048
                             </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Suppress execution of the procedure command "S-0-1048 Activate network settings"<BR>
    \#undef   &nbsp;&nbsp;  Procedure command S-0-1048 is executed for slaves with active class SCP_NRTPC
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_DISABLE_CMD_S_0_1048
#else
#undef CSMD_DISABLE_CMD_S_0_1048
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_SWC_EXT
                </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Activate extensions for SWC "single wire coexistence".<BR>
     - Fast CP switch
     - Variable UCC timing in CP1/CP2
     - Deactivate forwarding at last slave in line<BR>

    \#undef   &nbsp;&nbsp;  No support for SWC extensions
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_SWC_EXT
#else
#define CSMD_SWC_EXT
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_SOFT_MASTER
                    </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;       Sercos soft master is used as default.<BR>
    \#undef   &nbsp;&nbsp; Hard master IP-Core is used.<BR><BR>
    This presetting will be taken over in the function CSMD_Initialize().<BR>
    If defined, changes are possible by calling CSMD_UseSoftMaster() before calling the function CSMD_InitHardware().
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_SOFT_MASTER
#else
#define CSMD_SOFT_MASTER
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_HOTPLUG
                </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Hot-Plugging possible<BR>
    \#undef   &nbsp;&nbsp;  Hot-Plugging not possible
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_HOTPLUG
#else
#define CSMD_HOTPLUG
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_FAST_STARTUP
                          </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Fast StartUp applicable<BR>
    \#undef   &nbsp;&nbsp;  Fast StartUp feature not applicable
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_FAST_STARTUP
#else
#undef CSMD_FAST_STARTUP
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_STABLE_TSREF
                          </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Ring delay calculation corresponding to communication specification as of version 1.3.2<BR>
    \#undef   &nbsp;&nbsp;  Ring delay determination corresponding to communication specification up to version 1.3.1
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_STABLE_TSREF
#else
#define CSMD_STABLE_TSREF
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_HW_SVC_REDUNDANCY
                          </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Activate IP-Core service channel prozessor redundancy function.<BR>
    \#undef   &nbsp;&nbsp;  Without IP-Core service channel prozessor redundancy.
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_HW_SVC_REDUNDANCY
#else
#undef CSMD_HW_SVC_REDUNDANCY
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_SVC_LIST_SEGMENT
                         </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Reading of list segemnts (relating to start of the list) via service channel possible.<BR>
    \#undef   &nbsp;&nbsp;  No reading of list segments via service channel.
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_SVC_LIST_SEGMENT
#else
#define CSMD_SVC_LIST_SEGMENT
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MASTER_PRODUCE_IN_AT
                             </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Master may produce connection data into AT<BR>
    \#undef   &nbsp;&nbsp;  Master may not produce connection data into AT
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_MASTER_PRODUCE_IN_AT
#else
#define CSMD_MASTER_PRODUCE_IN_AT
#endif


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_TX_BUFFER
                      </B></TD></TR><TR><TD width=120>
    1 ... 3           </TD><TD width=800>
    Maximum number of possible buffers for the FPGA Tx buffering system for transmitted real time data:<BR>
    1   -&gt; Tx single buffer system<BR>
    2   -&gt; Tx single or double buffer system possible<BR>
    3   -&gt; Tx single, double or triple buffer system possible
    </TD></TR></TABLE>
*/
#define  CSMD_MAX_TX_BUFFER             (1)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_RX_BUFFER
                      </B></TD></TR><TR><TD width=120>
    1 ... 3           </TD><TD width=800>
    Maximum number of possible buffers for the FPGA Rx buffering systemfor received real time data:<BR>
    1   -&gt; Rx single buffer system<BR>
    2   -&gt; Rx single or double buffer system possible<BR>
    3   -&gt; Rx single, double or triple buffer system possible
    </TD></TR></TABLE>
*/
#define  CSMD_MAX_RX_BUFFER             (1)


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_ACTIVATE_AUTONEGOTIATION
                                 </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Activates PHY Autonegotiation by means of software<BR>
    \#undef   &nbsp;&nbsp;  Default setting of the PHY hardware
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_ACTIVATE_AUTONEGOTIATION
#else
#undef CSMD_ACTIVATE_AUTONEGOTIATION
#endif


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_HW_CONTAINER
                         </B></TD></TR><TR><TD width=120>
    0 ... 32             </TD><TD width=800>
    Maximum number of the service containers to be used realized per hardware (max. 32 with SERCON100M V4)<BR>
    If PCI bus master is activated, the number shall be divisible by two.
    </TD></TR></TABLE>
*/
#define CSMD_MAX_HW_CONTAINER           (0)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_SVC_CONTAINER_LENGTH
                             </B></TD></TR><TR><TD width=120>
    64 ... 512               </TD><TD width=800>
    Service container length in bytes for each slave (With CSMD_MAX_HW_CONTAINER = 32, a maximum of 126 byte is possible)
    </TD></TR></TABLE>
*/
#define CSMD_SVC_CONTAINER_LENGTH      (128)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_SLAVES
                   </B></TD></TR><TR><TD width=120>
    1 ... 511      </TD><TD width=800>
    Maximum number of slaves to be operated
    </TD></TR></TABLE>
*/
#define CSMD_MAX_SLAVES                (256)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_JITTER_CYCCLK
                      </B></TD></TR><TR><TD width=120>
    typical 2000      </TD><TD width=800>
    No longer used but applicable as default for SYNC Jitter of master [ns] (ulJitter_Master)
    </TD></TR></TABLE>
*/
#define CSMD_JITTER_CYCCLK          (2000)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_JITTER_SLAVE
                     </B></TD></TR><TR><TD width=120>
    20 ... 100       </TD><TD width=800>
    Default value for the jitter of the slave interface (one port) [ns]<BR>
    Typically 80ns for SERCON100S V3 / 40ns for SERCON100S V4 / 50ns for netX
    </TD></TR></TABLE>
*/
#define CSMD_JITTER_SLAVE               (80)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_DELAY_MASTER_DELAY
                           </B></TD></TR><TR><TD width=120></TD><TD width=800>
    Default value for the delay of the master interface (one port) [ns]<BR>
    Typically 500..900ns with SERCON100M.<BR><BR>
    If defined \ref CSMD_STABLE_TSREF :<BR> This parameter is no longer used!
    </TD></TR></TABLE>
*/
#define CSMD_DELAY_MASTER_DELAY         (900)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_DELAY_SLAVE_DELAY
                          </B></TD></TR><TR><TD width=120></TD><TD width=800>
    Default value for the delay of a slave interface (one port) [ns]<BR>
    Typically 500..900ns with SERCON100S.
    </TD></TR></TABLE>
*/
#define CSMD_DELAY_SLAVE_DELAY          (900)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_DELAY_COMPONENTS_DELAY
                               </B></TD></TR><TR><TD width=120></TD><TD width=800>
    Default value for the sum of all non-Sercos interfaces in the Sercos ring [ns]
    </TD></TR></TABLE>
*/
#define CSMD_DELAY_COMPONENTS_DELAY     (2 * CSMD_DELAY_SLAVE_DELAY)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MIN_TIME_START_AT
                          </B></TD></TR><TR><TD width=120>
    typical: 100000       </TD><TD width=800>
    Default for minimum value [ns] for parameter S-0-1006 'AT Transmission starting time (t1)'<BR>
    <B>Caution! This value is not taken, if CSMD_GetTimingData() is used</B>
    </TD></TR></TABLE>
*/
#define CSMD_MIN_TIME_START_AT    (100*1000)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_GLOB_CONN
                      </B></TD></TR><TR><TD width=120>
    Min. = 2          </TD><TD width=800>
    Maximum number of connections (for master and slaves) 
    </TD></TR></TABLE>
*/
#define  CSMD_MAX_GLOB_CONN            (512)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_GLOB_CONFIG
                        </B></TD></TR><TR><TD width=120>
    Min. = 4            </TD><TD width=800>
    Maximum number of configurations (for master and slaves)
    </TD></TR></TABLE>
*/
#define  CSMD_MAX_GLOB_CONFIG          (512)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_RT_BIT_CONFIG
                          </B></TD></TR><TR><TD width=120>
    Min. = 1              </TD><TD width=800>
    Maximum number of configurable Real Time Bit configurations
    </TD></TR></TABLE>
*/
#define  CSMD_MAX_RT_BIT_CONFIG         (64)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_RT_BITS_PER_CONN
                             </B></TD></TR><TR><TD width=120>
    1 .. 2(4)                </TD><TD width=800>
    Maximum number of configurable real-time bits per connection
    </TD></TR></TABLE>
*/
#define  CSMD_MAX_RT_BITS_PER_CONN       (2)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_SLAVE_CONFIGPARAMS
                               </B></TD></TR><TR><TD width=120>
    Min. = 1                   </TD><TD width=800>
    Maximum number of references to parameter lists.
    </TD></TR></TABLE>
*/
#define  CSMD_MAX_SLAVE_CONFIGPARAMS    (64)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_CONFIGPARAMS_LIST
                              </B></TD></TR><TR><TD width=120>
    Min. = 1                  </TD><TD width=800>
    Maximum number of configuration parameter lists.
    </TD></TR></TABLE>
*/
#define  CSMD_MAX_CONFIGPARAMS_LIST     (200)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_PARAMS_IN_CONFIG_LIST
                                  </B></TD></TR><TR><TD width=120>
    Min. = 1                      </TD><TD width=800>
    Maximum number of parameters per configuration list.
    </TD></TR></TABLE>
*/
#define  CSMD_MAX_PARAMS_IN_CONFIG_LIST  (20)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_CONFIG_PARAMETER
                             </B></TD></TR><TR><TD width=120>
    Min. = 1                 </TD><TD width=800>
    Maximum number of configuration parameters CP2 --&gt; CP3
    </TD></TR></TABLE>
*/
#define  CSMD_MAX_CONFIG_PARAMETER       (300)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_NBR_PARAM_DATA
                       </B></TD></TR><TR><TD width=120>
    Min. = 1           </TD><TD width=800>
    Maximum number of bytes per configuration parameter.
    </TD></TR></TABLE>
*/
#define  CSMD_NBR_PARAM_DATA             (16)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_CONNECTIONS_MASTER
                               </B></TD></TR><TR><TD width=120>
    Min. = 2                   </TD><TD width=800>
    Maximum number of configurable connections of the Sercos master (produced and consumed)
    </TD></TR></TABLE>
*/
#define CSMD_MAX_CONNECTIONS_MASTER    (512)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_CONNECTIONS
                        </B></TD></TR><TR><TD width=120>
    Min. = 2            </TD><TD width=800>
    Maximum number of configurable connections per slave (produced and consumed)
    </TD></TR></TABLE>
*/
#define CSMD_MAX_CONNECTIONS             (8)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_PROD_CONN_CC
                         </B></TD></TR><TR><TD width=120>
    Min. = 1             </TD><TD width=800>
    Maximum number of configurable cross-communication (CC) connections per slave.
    </TD></TR></TABLE>
*/
#define CSMD_MAX_PROD_CONN_CC            (2)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_IDN_PER_CONNECTION
                               </B></TD></TR><TR><TD width=120>
    Min. = 1                   </TD><TD width=800>
    A connection may contain several elements (IDNs)<BR>
    This define specifies how many elements may be contained in a connection.  (Typically = 16)
    </TD></TR></TABLE>
*/
#define CSMD_MAX_IDN_PER_CONNECTION     (16)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_CONN_NAME_LENGTH
                         </B></TD></TR><TR><TD width=120>
    Min = 2 Typical 30   </TD><TD width=800>
    Maximum length of connection name [bytes]<BR>
    Shall be divisible by 2!. Shall not be divisible by 4!
    </TD></TR></TABLE>
*/
#define  CSMD_CONN_NAME_LENGTH          (30)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_ENTRIES_S_0_1000
                             </B></TD></TR><TR><TD width=120>
    Min = 32 Typical 64      </TD><TD width=800>
    Maximum number of classification elements to be expected for the parameters:<BR>
    - S-0-1000     'List of SCP classes & version'
    - S-0-1000.0.1 'List of active SCP classes & version'<BR>
    .
    <B>Caution! This value shall be at least the maximum number of elements of all slaves.<BR> todo better description</B>
    </TD></TR></TABLE>
*/
#define CSMD_MAX_ENTRIES_S_0_1000       (64)


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_MAX_CYC_TIMES
                             </B></TD></TR><TR><TD width=120>
    Min = 1 Typical 3        </TD><TD width=800>
    Maximum number of producer cycle times that may be configured.
    </TD></TR></TABLE>
*/
#define CSMD_MAX_CYC_TIMES              (3)


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_PACK_ATTRIBUTE
                       </B></TD></TR><TR><TD width=120></TD><TD width=800>
    For some structures, a defined alignment of the structure elements is required. The settings depend on the compiler and are set<BR>
    with the defines CSMD_PACK_ATTRIBUTE and CSMD_PACK_PRAGMA. There are three different configurations possible.<BR>
    System:
    -# (e.g. Hitachi Compiler for ST40):<BR>\#undef  &nbsp; CSMD_PACK_ATTRIBUTE
    -# (e.g. GNU Compiler for ARM):     <BR>\#define        CSMD_PACK_ATTRIBUTE
    -# (e.g. MS Windows with RTX):      <BR>\#undef  &nbsp; CSMD_PACK_ATRIBUTE
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_PACK_ATTRIBUTE
#else
#undef  CSMD_PACK_ATTRIBUTE
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_PACK_PRAGMA
                    </B></TD></TR><TR><TD width=120></TD><TD width=800>
    For some structures, a defined alignment of the structure elements is required. The settings depend on the compiler and are set<BR>
    with the defines CSMD_PACK_ATTRIBUTE and CSMD_PACK_PRAGMA. There are three different configurations possible.<BR>
    System:
    -# (e.g. Hitachi Compiler for ST40) <BR>\#undef  &nbsp; CSMD_PACK_PRAGMA
    -# (e.g. GNU Compiler for ARM):     <BR>\#undef  &nbsp; CSMD_PACK_PRAGMA
    -# (e.g. MS Windows with RTX):      <BR>\#define        CSMD_PACK_PRAGMA
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_PACK_PRAGMA
#else
#undef  CSMD_PACK_PRAGMA
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_HW_WATCHDOG
                    </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Hardware watchdog feature activated<BR>
    \#undef   &nbsp;&nbsp;  Hardware watchdog feature deactivated
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_HW_WATCHDOG
#else
#define  CSMD_HW_WATCHDOG
#endif


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_SERCOS_MON_CONFIG
                          </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Configuration parser for generating of Sercos Monitor configuration XML files available<BR>
    \#undef   &nbsp;&nbsp;  Configuration parser not available
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_SERCOS_MON_CONFIG
#else
#undef  CSMD_SERCOS_MON_CONFIG
#endif


/*! <TABLE><TR><TD colspan=2><B>
    CSMD_DEF_CONS_ACT_TIME_T11
                              </B></TD></TR><TR><TD width=120>
    typical: 500000           </TD><TD width=800>
    Default value for t11 (Consumer Activation Time)<BR> \note No longer used in this CoSeMa version.
    </TD></TR></TABLE>
*/
#define CSMD_DEF_CONS_ACT_TIME_T11  (500*1000)


#ifdef CSMD_DOXYGEN
/*! <TABLE><TR><TD colspan=2><B>
    CSMD_DEBUG
              </B></TD></TR><TR><TD width=120></TD><TD width=800>
    \#define  &nbsp;        Build with debug code. <B>Caution! Only for development purposes</B><BR>
    \#undef   &nbsp;&nbsp;  Build without debug code
    </TD></TR></TABLE>
*/
/* Here, the macro is defined only for generation of doxygen documentation */ #define CSMD_DEBUG
#else
#undef  CSMD_DEBUG
#endif



#ifdef CSMD_ACTIVATE_AUTONEGOTIATION
/* define PHY UNITS */
#define PHY_PORT1                       (0)     /* PHY Port 1 */
#define PHY_PORT2                       (1)     /* PHY Port 2 */

/* define MII_PHY_ACCESS register flags */
#define CSMD_PHY_PORT1_MDC              (1<<0)  /* PYH port 1 pin clock programming bit        */
#define CSMD_PHY_PORT1_MDIO             (1<<1)  /* PHY port 1 pin data IO programming bit      */
#define CSMD_PHY_PORT1_MDIO_EN          (1<<2)  /* PHY port 1 pin data IO direction/enable bit */

#define CSMD_PHY_PORT2_MDC              (1<<0)  /* PHY port 2 pin clock programming bit        */
#define CSMD_PHY_PORT2_MDIO             (1<<1)  /* PHY port 2 pin data IO programming bit      */
#define CSMD_PHY_PORT2_MDIO_EN          (1<<2)  /* PHY port 2 pin data IO direction/enable bit */
#endif


#undef  CSMD_DEBUG_DPLL     /* Record changings in PLLCSR status with current cycle and function return */
#undef  CSMD_DEBUG_HP       /* Hot-Plug Debug print on */
#undef  CSMD_TEST_BE        /* Test some Big-Endian things under little endian machine */

#endif /* _CSMD_USER */



/*
--------------------------------------------------------------------------------
  Modification history
--------------------------------------------------------------------------------

01 Sep 2010
  - Refactoring of CoSeMa files.
27 Jan 2014 WK
  - Added define CSMD_CONFIGURATION_PARAMETERS
02 Feb 2014 WK
  - Added define CSMD_STATIC_MEM_ALLOC
dd mmm 2014 AlM
  - Added define CSMD_MAX_CYC_TIMES
  - Removed define CSMD_REDUNDANCY
25 Mar 2014 WK
  - Added define CSMD_SERCOS_MON_CONFIG
15 Apr 2014 AlM
  - Removed define CSMD_MAX_NBR_NO_LINK
29 Apr 2014 WK
  Defdb FEAT-00051252 - Generation of configuration files for the Sercos Monitor
  - Added define CSMD_SERCOS_MON_CONFIG
05 Aug 2014 AlM
  - Removed defines
    CSMD_SERCOS_TIME, CSMD_CONN_BEHIND_CDEV, CSMD_CONN_BEHIND_SDEV
20 Aug 2014 WK
  - Removed define CSMD_MAX_PROD_CONN_AT_MASTER
21 Nov 2014 WK
  - Renamed define CSMD_IP_CHANNEL --> CSMD_UC_CHANNEL.
20 Jan 2015 WK
  - Default values CSMD_DELAY_SLAVE_DELAY and CSMD_DELAY_MASTER_DELAY
    increased to 900 ns.
22 Jan 2015 WK
  - Example for CSMD_MAX_ENTRIES_S_0_1000 increased to 64.
11 Feb 2015 WK
  - Defdb00176768
    The define CSMD_SOFT_MASTER is now used as a default value
    for CSMD_PRIV.boSoftMaster.
02 Mar 2015 WK
  - Removed define CSMD_UC_CHANNEL.
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
08 Mar 2016 WK
  - Defdb00185518
    Adjust description of CSMD_SOFT_MASTER
    The CSMD_PRIV.boSoftMaster mode can be changed only with defined CSMD_SOFT MASTER.
16 Jun 2016 WK
 - FEAT-00051878 - Support for Fast Startup
   Added defines CSMD_FAST_STARTUP and CSMD_STABLE_TSREF.
20 Jul 2016 Wk
 - FEAT-00051878 - Support for Fast Startup
   Added description for the defines CSMD_FAST_STARTUP and CSMD_STABLE_TSREF.
  
--------------------------------------------------------------------------------
*/
