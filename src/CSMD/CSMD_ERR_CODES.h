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
\file   CSMD_ERR_CODES.h
\author WK
\date   03.09.2014
\brief  This File contains the public error code definitions
        for the file CSMD_GLOB.h
*/


#ifndef _CSMD_ERR_CODES
#define _CSMD_ERR_CODES

#undef SOURCE
#ifdef SOURCE_CSMD
    #define SOURCE
#else
    #define SOURCE extern
#endif

/*! \cond PUBLIC */



/*---- Definition resp. Declaration private Constants and Macros: ------------*/

/*---- Declaration private Types: --------------------------------------------*/


/* -------------------------------------------------------------------------- */
/*! \brief CSMD Function Return Values                                        */
/* -------------------------------------------------------------------------- */
/*
  +---------------------------------------------------------------------------+
  | Name of the enumerator                                                    |
  +-------+-----------+-------------------------------------------------------+
  | Error | Concerned |                                                       |
  | code  | functions | Description                                           |
  +-------+-----------+-------------------------------------------------------+
*/
typedef enum CSMD_FUNC_RET_EN
{
  /* --------------------------------------------------------- */
  /* no error: states, warning codes  (error_class 0x00 nnn)   */
  /* --------------------------------------------------------- */
                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_ERROR                               </B></TD></TR><TR><TD width=90>\b
  0x00000000                                  </TD><TD width=240>
  All functions                               </TD><TD width=800>
      The function has finished without errors.
                                              </TD></TR></TABLE> */
  CSMD_NO_ERROR          = (0x00000000),

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_FUNCTION_IN_PROCESS                    </B></TD></TR><TR><TD width=90>\b
  0x00000001                                  </TD><TD width=240>
  All functions using a state machine with
  transfer parameters of type CSMD_FUNC_STATE </TD><TD width=800>
      The function is not terminated yet and must be called once again.
                                              </TD></TR></TABLE> */
  CSMD_FUNCTION_IN_PROCESS,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_ERROR_DOUBLE_RECOGNIZED_ADDRESS        </B></TD></TR><TR><TD width=90>\b
  0x00000002                                  </TD><TD width=240>
  CSMD_SetPhase0()                            </TD><TD width=800>
      Two or more slaves with the same Sercos address have been found.
                                              </TD></TR></TABLE> */
  CSMD_ERROR_DOUBLE_RECOGNIZED_ADDRESS,

  __CSMD_S3_LINK_DETECTED_not_used,           /* 0x03 */      /* CSMD_HotPlug(): Sercos link at last slave in line detected */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WARNING_SAME_PHASE                     </B></TD></TR><TR><TD width=90>\b
  0x00000004                                  </TD><TD width=240>
  CSMD_SetPhase1()<BR>CSMD_SetPhase2()<BR>
  CSMD_SetPhase3()<BR>CSMD_SetPhase4()        </TD><TD width=800>
      Switching to the same communication phase as currently active is not allowed.
                                              </TD></TR></TABLE> */
  CSMD_WARNING_SAME_PHASE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WARNING_IFG_MISMATCH                   </B></TD></TR><TR><TD width=90>\b
  0x00000005                                  </TD><TD width=240>
  CSMD_CalculateTiming())                     </TD><TD width=800>
      Not all slaves with the class SCP_Sync supports the IDN S-0-1036 "Inter Frame Gap".
                                              </TD></TR></TABLE> */
  CSMD_WARNING_IFG_MISMATCH,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WARNING_MTU_MISMATCH                   </B></TD></TR><TR><TD width=90>\b
  0x00000006                                  </TD><TD width=240>
  CSMD_TransmitTiming()                       </TD><TD width=800>
      Not all slaves with the classes SCP_NRT or SCP_NRTPC were able to adapt the MTU.
                                              </TD></TABLE> */
  CSMD_WARNING_MTU_MISMATCH,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WARN_TOO_FEW_TX_RAM_FOR_UCC            </B></TD></TR><TR><TD width=90>\b
  0x00000007                                  </TD><TD width=240>
  CSMD_SetPhase3()                            </TD><TD width=800>
      The function was finished without fault but TxRam for Sercos configuration allows no UCC with max. MTU.
                                              </TD></TR></TABLE> */
  CSMD_WARN_TOO_FEW_TX_RAM_FOR_UCC,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WARN_TOO_FEW_RX_RAM_FOR_UCC            </B></TD></TR><TR><TD width=90>\b
  0x00000008                                  </TD><TD width=240>
  CSMD_SetPhase3()                            </TD><TD width=800>
      The function was finished without fault but RxRam for Sercos configuration allows no UCC with max. MTU (both ports).
                                              </TD></TR></TABLE> */
  CSMD_WARN_TOO_FEW_RX_RAM_FOR_UCC,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WARN_RECALCULATED_MTU                  </B></TD></TR><TR><TD width=90>\b
  0x00000009                                  </TD><TD width=240>
  CSMD_CalculateTiming())                     </TD><TD width=800>
      The given MTU is recalculated due to shortened UCC bandwidth.
                                              </TD></TR></TABLE> */
  CSMD_WARN_RECALCULATED_MTU,

  CSMD_END_ERR_CLASS_00000,                   /* End marker for error class 0x00 nnn */


  /* --------------------------------------------------------- */
  /* system error codes               (error_class 0x10 nnn)   */
  /* --------------------------------------------------------- */
                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_SYSTEM_ERROR                           </B></TD></TR><TR><TD width=90>\b
  0x00010000                                  </TD><TD width=240>
  All functions                               </TD><TD width=800>
      System error                            </TD></TR></TABLE> */
  CSMD_SYSTEM_ERROR      = (0x00010000),

  __CSMD_ERROR_FPGA_IDENT_not_Used,           /* 0x01 */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_ILLEGAL_CASE                           </B></TD></TR><TR><TD width=90>\b
  0x00010002                                  </TD><TD width=240>
  All functions                               </TD><TD width=800>
      System error <BR>(inadmissible case in switch instruction)
                                              </TD></TR></TABLE> */
  CSMD_ILLEGAL_CASE,

  __CSMD_FPGA_IDENT_NO_MASTER_not_Used,       /* 0x03 */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_FPGA_IDENT_VERSION                     </B></TD></TR><TR><TD width=90>\b
  0x00010004                                  </TD><TD width=240>
  CSMD_InitHardware()                         </TD><TD width=800>
      Found version of the SERCON100M IP-Core is not supported.
                                              </TD></TR></TABLE> */
  CSMD_FPGA_IDENT_VERSION,

  CSMD_END_ERR_CLASS_00010,                   /* End marker for error class 0x00010 nnn */


  /* --------------------------------------------------------- */
  /* Sercos error codes               (error_class 0x20 nnn)   */
  /* --------------------------------------------------------- */
                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_SVC_ERROR_MESSAGE                      </B></TD></TR><TR><TD width=90>\b
  0x00020000                                  </TD><TD width=240>
  Each macro service channel function.<BR>
  Each atomic service channel function.       </TD><TD width=800>
      Service channel:error message of the slave.<BR> The error message can be read out of the variable usSvchError
      in the structure rSvchData and/or rSvchMngmtData.<BR>(see \ref CSMD_SVC_ERROR_CODES)
                                              </TD></TR></TR></TABLE> */
  CSMD_SVC_ERROR_MESSAGE = (0x00020000),

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_SVCH_INUSE                             </B></TD></TR><TR><TD width=90>\b
  0x00020001                                  </TD><TD width=240>
  CSMD_ReadSVCH()<BR>CSMD_WriteSVCH()<BR>
  CSMD_SetCommand()<BR>CSMD_ClearCommand()<BR>
  CSMD_ReadCmdStatus()                        </TD><TD width=800>
      Service channel error:<BR>The function has been called once again with the \ref CSMD_START_REQUEST state although the service channel is still
      internally reserved. There is e.g. an internal reservation of the SVC if the S-0-0127 command is set in the CSMD_SetPhase3() function.
                                              </TD></TR></TABLE> */
  CSMD_SVCH_INUSE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_INTERNAL_REQUEST_PENDING               </B></TD></TR><TR><TD width=90>\b
  0x00020002                                  </TD><TD width=240>
  CSMD_ReadSVCH()<BR>CSMD_WriteSVCH()<BR>
  CSMD_SetCommand()<BR>CSMD_ClearCommand()<BR>
  CSMD_ReadCmdStatus()                        </TD><TD width=800>
      Service channel error:<BR>The function has been called once again as internal request with the \ref CSMD_START_REQUEST state although the service channel
      is still internally reserved. There is e.g. an internal reservation of the SVC if the S-0-0127 command is set in the CSMD_SetPhase3() function.
                                              </TD></TR></TABLE> */
  CSMD_INTERNAL_REQUEST_PENDING,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_REQUEST_CANCELED                       </B></TD></TR><TR><TD width=90>\b
  0x00020003                                  </TD><TD width=240>
  Each macro service channel function.<BR>
  Each atomic service channel function.       </TD><TD width=800>
      Service channel error:<BR>The current transmission has been canceled by the call-up of the same function with higher priority.
                                              </TD></TR></TABLE> */
  CSMD_REQUEST_CANCELED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_MBUSY_NOT_SET                          </B></TD></TR><TR><TD width=90>\b
  0x00020004                                  </TD><TD width=240>
  Each macro service channel function.<BR>
  Each atomic service channel function.       </TD><TD width=800>
      Service channel error:<BR>The function has been called once again although the Sercos controller has not yet completed the current
      transmission by setting the M_Busy bit. After the first call-up for the slave, the function may only be re-called in case of an SVC
      interrupt signal. A pending SVC interrupt has been recognized. The corresponding bit has not be reset using the CSMD_ClearSvchInt() function.
                                              </TD></TR></TABLE> */
  CSMD_MBUSY_NOT_SET,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_PROTOCOL_ERROR                         </B></TD></TR><TR><TD width=90>\b
  0x00020005                                  </TD><TD width=240>
  Each macro service channel function.<BR>
  Each atomic service channel function.       </TD><TD width=800>
      Service channel error:<BR>The AHS and MHS handshake bits are different and the BUSY_AT bit is set. This state is invalid.
                                              </TD></TR></TABLE> */
  CSMD_PROTOCOL_ERROR,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HANDSHAKE_TIMEOUT                      </B></TD></TR><TR><TD width=90>\b
  0x00020006                                  </TD><TD width=240>
  Each macro service channel function.<BR>
  Each atomic service channel function.       </TD><TD width=800>
      Service channel error:<BR>An addressed slave has not acknowledged its AHS bit in the status word after 10 communication cycles.
                                              </TD></TR></TABLE> */
  CSMD_HANDSHAKE_TIMEOUT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_BUSY_TIMEOUT                           </B></TD></TR><TR><TD width=90>\b
  0x00020007                                  </TD><TD width=240>
  Each macro service channel function.<BR>
  Each atomic service channel function.       </TD><TD width=800>
      Service channel error:<BR>The addressed slave has accepted the transmission request. The AHS and MHS handshake bits are identical and the BUSY_AT bit
      is still set after timeout.
                                              </TD></TR></TABLE> */
  CSMD_BUSY_TIMEOUT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_ELEMENT_NBR                      </B></TD></TR><TR><TD width=90>\b
  0x00020008                                  </TD><TD width=240>
  CSMD_WriteSVCH()                            </TD><TD width=800>
      Service channel error:<BR>Inadmissible element for write access. Permitted are element 1 'Data status' and element 7 'Operating data'.
                                              </TD></TR></TABLE> */
  CSMD_WRONG_ELEMENT_NBR,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_SVCWRITE_LENGTH_ERROR                  </B></TD></TR><TR><TD width=90>\b
  0x00020009                                  </TD><TD width=240>
  CSMD_ReadSVCH()<BR>CSMD_WriteSVCH()<BR>
  CSMD_OpenIDN()<BR>CSMD_GetDataStatus()<BR>
  CSMD_PutData()                              </TD><TD width=800>
      Service channel error:<BR>The data length specified in the rSvchData and/or rSvchMngmtData structure is 0.
                                              </TD></TR></TABLE> */
  CSMD_SVCWRITE_LENGTH_ERROR,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_ERROR_PHASE_CHANGE_CHECK               </B></TD></TR><TR><TD width=90>\b
  0x0002000A                                  </TD><TD width=240>
  CSMD_SetPhase0()<BR>CSMD_SetPhase1()<BR>
  CSMD_SetPhase2()<BR>CSMD_SetPhase3()<BR>
  CSMD_SetPhase4()                            </TD><TD width=800>
      A new communication phase has been specified and the new CP bit has been set. By the expiry of the timeout time, the transmission of telegrams
      has not been set in all slaves at the bus. The telegram structure cannot be reconfigured --&gt; Communication Phase change failed.
                                              </TD></TR></TABLE> */
  CSMD_ERROR_PHASE_CHANGE_CHECK,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_ERROR_PHASE_CHANGE_START               </B></TD></TR><TR><TD width=90>\b
  0x0002000B                                  </TD><TD width=240>
  CSMD_SetPhase0()<BR>CSMD_SetPhase1()<BR>
  CSMD_SetPhase2()<BR>CSMD_SetPhase3()<BR>
  CSMD_SetPhase4()                            </TD><TD width=800>
      The telegram structure has been reconfigured for the new communication phase. The new CP has been specified and the new CP bit
      has been cleared. By the expiry of the timeout time, not all slaves at the bus have started the transmission of telegrams.
      Until the timeout was over, not all slaves at the bus had responded to the new telegrams in the AT.
      The slave valid bit in the S-DEV, or with CSMD_SetPhase1() the valid it in the SVC status, has not been set --&gt; Communication Phase change failed.<BR>
      Cause:<BR>
      a) CSMD_CyclicHandling() is not called.<BR>
      b) External triggering of the Sercos cycle is missing.<BR>
      c) One or several slaves do not respond.
                                              </TD></TR></TABLE> */
  CSMD_ERROR_PHASE_CHANGE_START,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_ERROR_TIMEOUT_P0                       </B></TD></TR><TR><TD width=90>\b
  0x0002000C                                  </TD><TD width=240>
  CSMD_SetPhase0()                            </TD><TD width=800>
      Within the timeout period (2000 Sercos cycles) no stable slave configuration was recognized at the bus (100 subsequent AT0 telegrams
      with identical content) --&gt; Phase change failed.
                                              </TD></TR></TABLE> */
  CSMD_ERROR_TIMEOUT_P0,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_COMMUNICATION_P0                    </B></TD></TR><TR><TD width=90>\b
  0x0002000D                                  </TD><TD width=240>
  CSMD_SetPhase0()                            </TD><TD width=800>
      Neither at port 1 nor at port 2 valid Sercos AT0 telegrams have been received --&gt; Communication Phase change failed.
                                              </TD></TR></TABLE> */
  CSMD_NO_COMMUNICATION_P0,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_ERROR_DOUBLE_ADDRESS                   </B></TD></TR><TR><TD width=90>\b
  0x0002000E                                  </TD><TD width=240>
  CSMD_SetPhase1()                            </TD><TD width=800>
      The transferred list of the projected slaves may contain 2 or more slaves with identical slave address --&gt; Communication Phase change failed.
                                              </TD></TR></TABLE> */
  CSMD_ERROR_DOUBLE_ADDRESS,

  /*! \todo translation: CSMD_LOOP_NOT_CLOSED */
                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_LOOP_NOT_CLOSED                        </B></TD></TR><TR><TD width=90>\b
  0x0002000F                                  </TD><TD width=240>
  CSMD_SetPhase0()                            </TD><TD width=800>
      No stable Sercos topology detected --&gt; Communication Phase change failed.
      Cause:<BR>
      a) No link on both ports.<BR>
      b) CSMD_CyclicHandling() is not called.<BR>
      c) Within 1000 Sercos cycles no stable Sercos topology over 100 Sercos cycles detected.
      d) Neither at port 1 nor at port 2, valid Sercos AT0 telegrams have been received
                                              </TD></TR></TABLE> */
  CSMD_LOOP_NOT_CLOSED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_SLAVE_INDEX                      </B></TD></TR><TR><TD width=90>\b
  0x00020010                                  </TD><TD width=240>
  Each macro service channel function.<BR><BR>
  Each atomic service channel function.<BR><BR>   </TD><TD width=800>
      --&gt; The index \ref CSMD_SVCH_MACRO_STRUCT.usSlaveIdx is greater than the system limit \ref CSMD_SYSTEM_LIMITS_STRUCT.usMaxSlaves.<BR>
      --&gt; The index \ref CSMD_SVCH_MNGMT_STRUCT.usSlaveIdx is greater than the system limit \ref CSMD_SYSTEM_LIMITS_STRUCT.usMaxSlaves.
                                              </TD></TR></TABLE> */
  CSMD_WRONG_SLAVE_INDEX,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_PROJECTED_SLAVE_LIST             </B></TD></TR><TR><TD width=90>\b
  0x00020011                                  </TD><TD width=240>
  CSMD_SetPhase1()                            </TD><TD width=800>
      The transferred list of the projected slaves is not correct. --&gt; Communication Phase change failed.<BR>Cause:<BR>
      a) List length current &gt; maximum<BR>
      b) Odd list length information<BR>
      c) List contains more than \ref CSMD_MAX_SLAVES entries
                                              </TD></TR></TABLE> */
  CSMD_WRONG_PROJECTED_SLAVE_LIST,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_PROCEDURE_CMD                       </B></TD></TR><TR><TD width=90>\b
  0x00020012                                  </TD><TD width=240>
  CSMD_SetCommand()<BR>CSMD_ClearCommand()<BR>
  CSMD_ReadCmdStatus()                        </TD><TD width=800>
      Service channel error:<BR>The indicated IDN is not a procedure command.
                                              </TD></TR></TABLE> */
  CSMD_NO_PROCEDURE_CMD,


                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_PHASE                            </B></TD></TR><TR><TD width=90>\b
  0x00020013                                  </TD><TD width=240>
  <BR>
  CSMD_Init_Config_Struct() <BR>  CSMD_Set_NRT_State()       <BR>  CSMD_SetPhase1()      <BR>  CSMD_SetPhase2()    <BR>
  CSMD_SetPhase3()          <BR>  CSMD_SetPhase4()           <BR>  CSMD_CheckVersion()   <BR>
  CSMD_GetTimingData()      <BR>  CSMD_CalculateTiming()     <BR>  CSMD_TransmitTiming() <BR>  CSMD_GetTopology()  <BR>
  CSMD_OpenRing()           <BR>  CSMD_RecoverRingTopology() <BR>  CSMD_HotPlug()        <BR>  CSMD_TransHP2Para()
                                              </TD><TD width=800>
      In the current communication phase calling the function is not allowed:<BR>
      Phase &gt;     CP2 <BR>  Phase &gt;     CP0 <BR>  Phase &lt;&gt; CP0 <BR>  Phase &lt;&gt; CP1 <BR>
      Phase &lt;&gt; CP2 <BR>  Phase &lt;&gt; CP3 <BR>  Phase &lt;     CP2 <BR>
      Phase &lt;&gt; CP2 <BR>  Phase &lt;&gt; CP2 <BR>  Phase &lt;&gt; CP2 <BR>  Phase &lt;     CP0 <BR>
      Phase &lt;     CP1 <BR>  Phase &lt;     CP1 <BR>  Phase &lt;&gt; CP4 <BR>  Phase &lt;&gt; CP4
                                              </TD></TR></TABLE> */
  CSMD_WRONG_PHASE,

  __CSMD_ERROR_TIMEOUT_MST_P0_not_Used,       /* 0x14 */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_FAULTY_MDT_LENGTH                      </B></TD></TR><TR><TD width=90>\b
  0x00020015                                  </TD><TD width=240>
  CSMD_SetPhase3()                            </TD><TD width=800>
      The values for the parameter 'S-0-1010, MDT lengths' entered in the rComTiming structure are faulty: --&gt; Communication Phase change failed.<BR>
      Valid values are 40..1494. Non-existent MDT telegrams are marked with length = 0.
                                              </TD></TR></TABLE> */
  CSMD_FAULTY_MDT_LENGTH,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_FAULTY_AT_LENGTH                       </B></TD></TR><TR><TD width=90>\b
  0x00020016                                  </TD><TD width=240>
  CSMD_SetPhase3()                            </TD><TD width=800>
      The values for the parameter 'S-0-1012, AT lengths' entered in the rComTiming structure are faulty: --&gt; Communication Phase change failed.<BR>
      Valid values are 40..1494. Non-existent AT telegrams are marked with length = 0.
                                              </TD></TR></TABLE> */
  CSMD_FAULTY_AT_LENGTH,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_INCONSISTENT_RING_ADDRESSES            </B></TD></TR><TR><TD width=90>\b
  0x00020017                                  </TD><TD width=240>
  CSMD_SetPhase0()                            </TD><TD width=800>
      With recognized ring topology: The number and/or the Sercos addresses of the recognized slaves at port 1 and port 2 are not identical.<BR>
      --&gt; Communication Phase change failed.
                                              </TD></TR></TABLE> */
  CSMD_INCONSISTENT_RING_ADDRESSES,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_INVALID_SERCOS_CYCLE_TIME              </B></TD></TR><TR><TD width=90>\b
  0x00020018                                  </TD><TD width=240>
  CSMD_InitHardware()<BR>
  CSMD_SetPhase0()<BR>CSMD_SetPhase1()<BR>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      Wrong Sercos cycle time tScyc has been specified. According to the Sercos specification, the following values are permitted:<BR>
      tScyc = 31.25 microseconds, 62.5 microseconds, 125 microseconds, 250 microseconds and even multiples of 250 to 65000 microseconds.
                                              </TD></TR></TABLE> */
  CSMD_INVALID_SERCOS_CYCLE_TIME,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TEL_NBR_MDT_SVC                        </B></TD></TR><TR><TD width=90>\b
  0x00020019                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      System error: Telegram number > 3 for service channel data field in the MDT.
                                              </TD></TR></TABLE> */
  CSMD_TEL_NBR_MDT_SVC,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TEL_NBR_MDT_RTD                        </B></TD></TR><TR><TD width=90>\b
  0x0002001A                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      Telegram number > 3 for cyclic data in the MDT. The command values for all slaves to be operated, which are to be transmitted cyclically
      are not suitable for 4 MDT telegrams.
                                              </TD></TR></TABLE> */
  CSMD_TEL_NBR_MDT_RTD,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TEL_NBR_AT_SVC                         </B></TD></TR><TR><TD width=90>\b
  0x0002001B                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      System error: Telegram number > 3 for service channel data field in the AT.
                                              </TD></TR></TABLE> */
  CSMD_TEL_NBR_AT_SVC,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TEL_NBR_AT_RTD                         </B></TD></TR><TR><TD width=90>\b
  0x0002001C                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      Telegram number > 3 for cyclic data in the AT. The actual values for all slaves to be operated, which are to be transmitted cyclically
      are not suitable for 4 AT telegrams.
                                              </TD></TR></TABLE> */
  CSMD_TEL_NBR_AT_RTD,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_ILLEGAL_TIMING_METHOD                  </B></TD></TR><TR><TD width=90>\b
  0x0002001D                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      Permitted time slot calculation methods:<BR>
      - 1 = UC channel behind the last AT.
      - 2 = UC channel between the last MDT and the first AT.
      - 3 = as method 2, however AT telegram tot he end of the Sercos cycle.
                                              </TD></TR></TABLE> */
  CSMD_ILLEGAL_TIMING_METHOD,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CYCTEL_LEN_GT_TSCYC                    </B></TD></TR><TR><TD width=90>\b
  0x0002001E                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      The transmission time of all MDT and AT telegrams for the specified configuration is larger than the Sercos cycle time tScyc.
                                              </TD></TR></TABLE> */
  CSMD_CYCTEL_LEN_GT_TSCYC,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TEL_LEN_GT_TSCYC                       </B></TD></TR><TR><TD width=90>\b
  0x0002001F                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      The total of the transmission times for all MDT, AT telegrams and the UC channel width for the specified configuration is larger than the
      Sercos cycle time tScyc.
                                              </TD></TR></TABLE> */
  CSMD_TEL_LEN_GT_TSCYC,

  /*! \todo CSMD_NO_LINK_ATTACHED: (The sensitivity for this error is adjustable with the define CSMD_MAX_NBR_NO_LINK). --> das define gibt es nicht mehr! */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_LINK_ATTACHED                       </B></TD></TR><TR><TD width=90>\b
  0x00020020                                  </TD><TD width=240>
  CSMD_CyclicHandling()                       </TD><TD width=800>
      In the cyclic examination of the topology, a connection has neither been recognized at port 1 nor at port 2.<BR>
                                              </TD></TR></TABLE> */
  CSMD_NO_LINK_ATTACHED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TEL_ERROR_OVERRUN                      </B></TD></TR><TR><TD width=90>\b
  0x00020021                                  </TD><TD width=240>
  CSMD_CyclicHandling()                       </TD><TD width=800>
      The successive telegram errors (no telegrams received, MST missed or window error) have exceeded the maximum value.
                                              </TD></TR></TABLE> */
  CSMD_TEL_ERROR_OVERRUN,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_OPER_SLAVES                   </B></TD></TR><TR><TD width=90>\b
  0x00020022                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      The number of slaves to be operated in the rSlaveList structure is larger than \ref CSMD_MAX_SLAVES.
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_OPER_SLAVES,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_INVALID_SERCOS_PHASE                   </B></TD></TR><TR><TD width=90>\b
  0x00020023                                  </TD><TD width=240>
  CSMD_GetPhase()                             </TD><TD width=800>
      System error: The internal specification of the communication phase in the Sercos controller is incorrect.<BR>
      (Valid values: 0 .. 4 and set Communication phase change bit)
                                              </TD></TR></TABLE> */
  CSMD_INVALID_SERCOS_PHASE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_EVENT_ID                         </B></TD></TR><TR><TD width=90>\b
  0x00020024                                  </TD><TD width=240>
  CSMD_EventControl()                         </TD><TD width=800>
      Wrong event identifier. Supported events are:<BR>
      - Timer 0 to timer 3<BR>
        \ref CSMD_EVENT_ID_TIMER_0<BR> \ref CSMD_EVENT_ID_TIMER_1<BR> \ref CSMD_EVENT_ID_TIMER_2<BR> \ref CSMD_EVENT_ID_TIMER_3
      - Setting and clearing the ConClk output<BR>
        \ref CSMD_EVENT_ID_CONCLK_SET<BR>
        \ref CSMD_EVENT_ID_CONCLK_RESET
      - Transmit buffer change request buffer system A<BR>
        \ref CSMD_EVENT_ID_TX_BUFREQ_BUFSYS_A
      - Receivebuffer change request buffer system A<BR>
        \ref CSMD_EVENT_ID_RX_BUFREQ_BUFSYS_A
                                              </TD></TR></TABLE> */
  CSMD_WRONG_EVENT_ID,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_TOPOLOGY                         </B></TD></TR><TR><TD width=90>\b
  0x00020025                                  </TD><TD width=240>
  CSMD_OpenRing()<BR>
  CSMD_RecoverRingTopology()                  </TD><TD width=800>
      - In CSMD_OpenRing():           <BR> There is already a line topology.<BR>
      - In CSMD_RecoverRingTopology():<BR> The current topology is already a ring.
                                              </TD></TR></TABLE> */
  CSMD_WRONG_TOPOLOGY,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_INVALID_ETHERNET_MTU                   </B></TD></TR><TR><TD width=90>\b
  0x00020026                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      Inadmissible value for the MTU (maximum transmission unit) in the UC channel. (Valid values: 46 .. 1500)
                                              </TD></TR></TABLE> */
  CSMD_INVALID_ETHERNET_MTU,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CP3_TRANS_CHECK_CMD_ERROR              </B></TD></TR><TR><TD width=90>\b
  0x00020027                                  </TD><TD width=240>
  CSMD_SetPhase3()<BR>CSMD_TransHP2Para()     </TD><TD width=800>
      Execution of the 'S-0-0127, Communication Phase 3 Transition Check' command has been terminated with an error
      in at least one slave.<BR> --&gt; Communication Phase change failed.
                                              </TD></TR></TABLE> */
  CSMD_CP3_TRANS_CHECK_CMD_ERROR,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CP4_TRANS_CHECK_CMD_ERROR              </B></TD></TR><TR><TD width=90>\b
  0x00020028                                  </TD><TD width=240>
  CSMD_SetPhase4()<BR>CSMD_TransHP2Para()     </TD><TD width=800>
      Execution of the 'S-0-0128, Communication Phase 4 Transition Check' command has been terminated with an error
      in at least one slave.<BR> --&gt; Communication Phase change failed.
                                              </TD></TR></TABLE> */
  CSMD_CP4_TRANS_CHECK_CMD_ERROR,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_DIVCLK_TIMES                           </B></TD></TR><TR><TD width=90>\b
  0x00020029                                  </TD><TD width=240>
  CSMD_ConfigDIVCLK()                         </TD><TD width=800>
      The pulse distance or the delay time is larger than the maximum Sercos cycle time (65 000 000 ns).
                                              </TD></TR></TABLE> */
  CSMD_DIVCLK_TIMES,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_DIVCLK_PULSES                          </B></TD></TR><TR><TD width=90>\b
  0x0002002A                                  </TD><TD width=240>
  CSMD_ConfigDIVCLK()                         </TD><TD width=800>
      The number n (of pulses and/or Sercos cycles) is too large (> 255).
                                              </TD></TR></TABLE> */
  CSMD_DIVCLK_PULSES,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_SERCOS_VERSION_MISMATCH                </B></TD></TR><TR><TD width=90>\b
  0x0002002B                                  </TD><TD width=240>
  CSMD_CheckVersion()<BR>CSMD_GetTimingData()<BR>
  CSMD_TransHP2Para()                         </TD><TD width=800>
      One slave does not support the specification Sercos &gt;= V1.1:
      - a) The IDN S-0-1000 "List of SCP classes & version" does not exist.
      - b) The IDN S-0-1051 "Image of connection setups" does not exist.
                                              </TD></TR></TABLE> */
  CSMD_SERCOS_VERSION_MISMATCH,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_BASIC_SCP_TYPE_MISMATCH                </B></TD></TR><TR><TD width=90>\b
  0x0002002C                                  </TD><TD width=240>
  CSMD_GetTimingData()<BR>CSMD_CheckVersion()<BR>
  CSMD_TransHP2Para()                         </TD><TD width=800>
      - a) In one slave, both basic classifications (SCP_FixCFG and SCP_VarCFG) are activated in S-0-1000 "List of SCP classes & version".
      - b) In the configuration of one slave, both basic classifications (SCP_FixCFG and SCP_VarCFG) are activated in S-0-1000.0.1 "List of Active SCP Classes & Version".
                                              </TD></TR></TABLE> */
  CSMD_BASIC_SCP_TYPE_MISMATCH,

  __CSMD_UNSUPPORTED_SCP_TYPE_not_Used,       /* 0x2D */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_RAM_MIRROR_ALLOCATED                </B></TD></TR><TR><TD width=90>\b
  0x0002002E                                  </TD><TD width=240>
  CSMD_SetPhase0()                            </TD><TD width=800>
      In the CoSeMa instance, the pointers to the telegram buffers were not initialized:
      - a) rCSMD_Instance.prTelBuffer
      - b) rCSMD_Instance.prTelBuffer and/or prTelBuffer_Phys with activated PCI bus master DMA.
                                              </TD></TR></TABLE> */
  CSMD_NO_RAM_MIRROR_ALLOCATED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_ILLEGAL_SLAVE_ADDRESS                  </B></TD></TR><TR><TD width=90>\b
  0x0002002F                                  </TD><TD width=240>
  CSMD_SetPhase0()<BR>CSMD_SetPhase1()<BR>
  CSMD_SetPhase3()                            </TD><TD width=800>
      - In CSMD_SetPhase0():<BR> Slaves with Sercos addresses larger than \ref CSMD_MAX_SLAVE_ADD were found.
      - In CSMD_SetPhase1():<BR> The transferred list of the slaved to be operated contains a Sercos address larger than \ref CSMD_MAX_SLAVE_ADD.
      - In CSMD_SetPhase3():<BR> The operation in CP3 and later is only possible with Sercos addresses &gt;= \ref CSMD_MIN_SLAVE_ADD respectively &lt;= \ref CSMD_MAX_SLAVE_ADD.
                                              </TD></TR></TABLE> */
  CSMD_ILLEGAL_SLAVE_ADDRESS,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_NBR_OF_CONNECTIONS               </B></TD></TR><TR><TD width=90>\b
  0x00020030                                  </TD><TD width=240>
  CSMD_GetTimingData()<BR>CSMD_TransHP2Para() </TD><TD width=800>
      In one slave with basic classification SCP_FixCFG, the number of usNbrOfConnections connections in the configuration structure is not 2.
                                              </TD></TR></TABLE> */
  CSMD_WRONG_NBR_OF_CONNECTIONS,

  __not_used_CSMD_TOO_MANY_CONNECTIONS_FOR_SLAVE,
                                              /* 0x31 */      /* In one slave with basic classifications SCP_VarCFG, the number of usNbrOfConnections connections
                                                                 in the configuration structure is greater than \ref CSMD_MAX_CONNECTIONS. */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_INVALID_CONNECTION                     </B></TD></TR><TR><TD width=90>\b
  0x00020032                                  </TD><TD width=240>
  CSMD_GetTimingData()<BR>CSMD_TransHP2Para() </TD><TD width=800>
      In one slave with basic classifications SCP_VarCFG, one connection configuration contains an unsupported configuration type in usS_0_1050_SE1.
      The following values are permitted:
      - 0 = Configuration list with IDNs (SE6 relevant)
      - 1 = Container without content specifications (SE5 relevant)
      - 2 = Telegram type parameter FSP drive (S-0-0015 relevant)
                                              </TD></TR></TABLE> */
  CSMD_INVALID_CONNECTION,

  __CSMD_TOPOLOGY_CHANGE_not_Used,            /* 0x33 */      /* CSMD_LINE_BREAK_DETECTION */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_PROJ_SLAVES_NOT_ONE_TO_ONE             </B></TD></TR><TR><TD width=90>\b
  0x00020034                                  </TD><TD width=240>
  CSMD_SetPhase1()                            </TD><TD width=800>
      Lists of the recognized slaves and the projected slaves are not one-to-one.
                                              </TD></TR></TABLE> */
  CSMD_PROJ_SLAVES_NOT_ONE_TO_ONE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_UNIQUE_RECOGNIZED_ADDRESSES         </B></TD></TR><TR><TD width=90>\b
  0x00020035                                  </TD><TD width=240>
  CSMD_SetPhase3()                            </TD><TD width=800>
      Operation with two or several identical Sercos addresses in the list of recognized slaves is only possible in CP0 to CP2.
                                              </TD></TR></TABLE> */
  CSMD_NO_UNIQUE_RECOGNIZED_ADDRESSES,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_REQUEST_TO_HP_SLAVE                    </B></TD></TR><TR><TD width=90>\b
  0x00020036                                  </TD><TD width=240>
  CSMD_ReadSVCH()<BR>CSMD_WriteSVCH()<BR>
  CSMD_SetCommand()<BR>CSMD_ClearCommand()<BR>
  CSMD_ReadCmdStatus()<BR>CSMD_OpenIDN()      </TD><TD width=800>
      Service channel access to a Hot-Plug slave is not possible.
                                              </TD></TR></TABLE> */
  CSMD_REQUEST_TO_HP_SLAVE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_S_0_1024_CMD_ERROR                     </B></TD></TR><TR><TD width=90>\b
  0x00020037                                  </TD><TD width=240>
  CSMD_SetPhase3()<BR>CSMD_RecoverRingTopology()<BR>
  CSMD_TransHP2Para()                         </TD><TD width=800>
      The procedure command S-0-1024 "SYNC delay measuring procedure command" has failed in at least one slave.
                                              </TD></TR></TABLE> */
  CSMD_S_0_1024_CMD_ERROR,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_EVENT_TIME_MAX_LIMIT                   </B></TD></TR><TR><TD width=90>\b
  0x00020038                                  </TD><TD width=240>
  CSMD_SetPhase0()<BR>CSMD_SetPhase1()<BR>
  CSMD_SetPhase3()                            </TD><TD width=800>
      The time of an event programmed by CSMD_EventControl() exceeds the maximum time permitted for events.
                                              </TD></TR></TABLE> */
  CSMD_EVENT_TIME_MAX_LIMIT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_INVALID_MASTER_JITTER                  </B></TD></TR><TR><TD width=90>\b
  0x00020039                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      The master jitter has not been defined by the master or is greater than (tScyc / 8).
                                              </TD></TR></TABLE> */
  CSMD_INVALID_MASTER_JITTER,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_INVALID_MTU                            </B></TD></TR><TR><TD width=90>\b
  0x0002003A                                  </TD><TD width=240>
  CSMD_TransmitTiming()<BR>
  CSMD_TransHP2Para()                         </TD><TD width=800>
      The effective MTU (S-0-1027.0.2) does not equal the requested MTU (S-0-1027.0.1).
                                              </TD></TR></TABLE> */
  CSMD_INVALID_MTU,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_S_0_1048_CMD_ERROR                     </B></TD></TR><TR><TD width=90>\b
  0x0002003B                                  </TD><TD width=240>
  CSMD_TransmitTiming()<BR>
  CSMD_TransHP2Para()                         </TD><TD width=800>
      The procedure command S-0-1048 "Activate network settings" has failed in at least one slave.(If class SCP_NRTPC is activated)
                                              </TD></TR></TABLE> */
  CSMD_S_0_1048_CMD_ERROR,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CP0_COM_VER_CHECK                      </B></TD></TR><TR><TD width=90>\b
  0x0002003C                                  </TD><TD width=240>
  CSMD_SetPhase0()                            </TD><TD width=800>
      Check of requested functions (CP0-MDT0 communication version field) failled.
                                              </TD></TR></TABLE> */
  CSMD_CP0_COM_VER_CHECK,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_STABLE_TOPOLOGY_IN_CP0              </B></TD></TR><TR><TD width=90>\b
  0x0002003D                                  </TD><TD width=240>
  CSMD_SetPhase0()                            </TD><TD width=800>
      The recognized stable topology has changed during further course of the function.
                                              </TD></TR></TABLE> */
  CSMD_NO_STABLE_TOPOLOGY_IN_CP0,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_INVALID_SYSTEM_LIMITS                  </B></TD></TR><TR><TD width=90>\b
  0x0002003E                                  </TD><TD width=240>
  CSMD_InitSystemLimits()                     </TD><TD width=800>
      Die in der Struktur CSMD_SYSTEM_LIMITS_STRUCT vorgegebenen Systemgrenzen liegen ausserhalb der erlaubten Bereiche:
      - 2 &lt; usMaxConnMaster        &lt; \ref CSMD_MAX_CONNECTIONS_MASTER
      - 1 &lt; usMaxDevice            &lt; \ref CSMD_MAX_SLAVES
      - 2 &lt; usMaxGlobConn          &lt; \ref CSMD_MAX_GLOB_CONN
      - 4 &lt; usMaxGlobConfig        &lt; \ref CSMD_MAX_GLOB_CONFIG
      - 1 &lt; usMaxRtBitConfig       &lt; \ref CSMD_MAX_RT_BIT_CONFIG

      - 1 &lt; usMaxSlaveConfigparams &gt; \ref CSMD_MAX_SLAVE_CONFIGPARAMS
      - 1 &lt; usMaxConfigparamsList  &gt; \ref CSMD_MAX_CONFIGPARAMS_LIST
      - 1 &lt; usMaxConfigParameter   &gt; \ref CSMD_MAX_CONFIG_PARAMETER

      - 1 &lt; usMaxProdConnCC        &lt; \ref CSMD_MAX_PROD_CONN_CC
                                              </TD></TR></TABLE> */
  CSMD_INVALID_SYSTEM_LIMITS,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_MEMORY_ALLOCATION_FAILED               </B></TD></TR><TR><TD width=90>\b
  0x0002003F                                  </TD><TD width=240>
  CSMD_InitSystemLimits()                     </TD><TD width=800>
      The memory request necessary for the CoSeMa structure has failed
                                              </TD></TR></TABLE> */
  CSMD_MEMORY_ALLOCATION_FAILED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_TELEGRAMS_RECEIVED                  </B></TD></TR><TR><TD width=90>\b
  0x00020040                                  </TD><TD width=240>
  CSMD_CyclicHandling()                       </TD><TD width=800>
      No telegrams have been received on either master port with active Sercos communication.
                                              </TD></TR></TABLE> */
  CSMD_NO_TELEGRAMS_RECEIVED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_MST_MISS                               </B></TD></TR><TR><TD width=90>\b
  0x00020041                                  </TD><TD width=240>
  CSMD_CyclicHandling()                       </TD><TD width=800>
      At least one MST has no been received in the current cycle.
                                              </TD></TR></TABLE> */
  CSMD_MST_MISS,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_MST_WINDOW_ERROR                       </B></TD></TR><TR><TD width=90>\b
  0x00020042                                  </TD><TD width=240>
  CSMD_CyclicHandling()                       </TD><TD width=800>
      At least one MST has been received outside the appropriate window.
                                              </TD></TR></TABLE> */
  CSMD_MST_WINDOW_ERROR,

  /*! <TABLE><TR><TD colspan=3><B>
  CSMD_MAX_T_NETWORK_GT_T_SYNCDELAY           </B></TD></TR><TR><TD width=90>\b
  0x00020043                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      The sum of maximum tNetwork from both ports is greater than the calculated tSyncDelay.
      - Increasing the value of CSMD_COMMON_TIMING.lCompDelay
      - Increasing the value of CSMD_COMMON_TIMING.ulCalc_DelaySlave \n (anticipated average value)
                                              </TD></TR></TABLE> */
  CSMD_MAX_T_NETWORK_GT_T_SYNCDELAY,

  CSMD_END_ERR_CLASS_00020,                   /* End marker for error class 0x20 nnn */


  /* --------------------------------------------------------- */
  /* Configuration error codes        (error_class 0x21 nnn)   */
  /* --------------------------------------------------------- */
                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_MASTER_CONNECTIONS            </B></TD></TR><TR><TD width=90>\b
  0x00021000                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   Number of master connections > \ref CSMD_MAX_CONNECTIONS_MASTER.
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_MASTER_CONNECTIONS = (0x00021000),

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_SLAVE_CONNECTIONS             </B></TD></TR><TR><TD width=90>\b
  0x00021001                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n  Number of connections of a slave > \ref CSMD_MAX_CONNECTIONS.
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_SLAVE_CONNECTIONS,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_UNIQUE_CON_NBR                      </B></TD></TR><TR><TD width=90>\b
  0x00021002                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   Connection number is produced twice (or more).
                                              </TD></TR></TABLE> */
  CSMD_NO_UNIQUE_CON_NBR,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_MASTER_PROD_CONN_IN_AT                 </B></TD></TR><TR><TD width=90>\b
  0x00021003                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   Master produce a connection in AT.
                                              </TD></TR></TABLE> */
  CSMD_MASTER_PROD_CONN_IN_AT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_MASTER_CONSUME_IN_MDT                  </B></TD></TR><TR><TD width=90>\b
  0x00021004                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   Master consume a connection in MDT.
                                              </TD></TR></TABLE> */
  CSMD_MASTER_CONSUME_IN_MDT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_SLAVE_PRODUCE_IN_MDT                   </B></TD></TR><TR><TD width=90>\b
  0x00021005                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   Slave produce a connection in MDT.
                                              </TD></TR></TABLE> */
  CSMD_SLAVE_PRODUCE_IN_MDT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_PROD_CYC_TIME_INVALID                  </B></TD></TR><TR><TD width=90>\b
  0x00021006                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   Invalid producer cycle time in connection.
                                              </TD></TR></TABLE> */
  CSMD_PROD_CYC_TIME_INVALID,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONNECTION_WRONG_TEL_TYPE              </B></TD></TR><TR><TD width=90>\b
  0x00021007                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n
                                              </TD></TR></TABLE> */
  CSMD_CONNECTION_WRONG_TEL_TYPE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONN_MULTIPLE_PRODUCED                 </B></TD></TR><TR><TD width=90>\b
  0x00021008                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   Connection is produced twice (or more).
                                              </TD></TR></TABLE> */
  CSMD_CONN_MULTIPLE_PRODUCED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONNECTION_NOT_PRODUCED                </B></TD></TR><TR><TD width=90>\b
  0x00021009                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   Consumed connection is not produced.
                                              </TD></TR></TABLE> */
  CSMD_CONNECTION_NOT_PRODUCED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONFIGURATION_NOT_CONFIGURED           </B></TD></TR><TR><TD width=90>\b
  0x0002100A                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n
                                              </TD></TR></TABLE> */
  CSMD_CONFIGURATION_NOT_CONFIGURED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONNECTION_DATALENGTH_INCONSISTENT     </B></TD></TR><TR><TD width=90>\b
  0x0002100B                                  </TD><TD width=240>
  CSMD_GetTimingData(), CSMD_TransHP2Para()   </TD><TD width=800>
      The configured connection length is not equal to the read length.
                                              </TD></TR></TABLE> */
  CSMD_CONNECTION_DATALENGTH_INCONSISTENT,

  __CSMD_TOO_MANY_PROD_MASTER_CONN_IN_AT_not_used,       /* 0x0C */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_INSUFFICIENT_TX_RAM                    </B></TD></TR><TR><TD width=90>\b
  0x0002100D                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   Telegram configuration don't fit into TxRam of the IP-Core.
                                              </TD></TR></TABLE> */
  CSMD_INSUFFICIENT_TX_RAM,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_INSUFFICIENT_RX_RAM                    </B></TD></TR><TR><TD width=90>\b
  0x0002100E                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   Telegram configuration don't fit into RxRam of the IP-Core.
                                              </TD></TR></TABLE> */
  CSMD_INSUFFICIENT_RX_RAM,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONNECTION_NOT_CONSUMED                </B></TD></TR><TR><TD width=90>\b
  0x0002100F                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n
                                              </TD></TR></TABLE> */
  CSMD_CONNECTION_NOT_CONSUMED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_CONNECTION_INDEX                 </B></TD></TR><TR><TD width=90>\b
  0x00021010                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n
                                              </TD></TR></TABLE> */
  CSMD_WRONG_CONNECTION_INDEX,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_CONFIGURATION_INDEX              </B></TD></TR><TR><TD width=90>\b
  0x00021011                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n
                                              </TD></TR></TABLE> */
  CSMD_WRONG_CONFIGURATION_INDEX,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_RTBIT_CONFIGURATION_INDEX        </B></TD></TR><TR><TD width=90>\b
  0x00021012                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n
                                              </TD></TR></TABLE> */
  CSMD_WRONG_RTBIT_CONFIGURATION_INDEX,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_SCP_CAP_CONFIGURATION            </B></TD></TR><TR><TD width=90>\b
  0x00021013                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   A slave does not support the communication class SCP_CAP but a connection is configured with a capability.
                                              </TD></TR></TABLE> */
  CSMD_WRONG_SCP_CAP_CONFIGURATION,

  /*!\todo translation for CSMD_SERCOS_LIST_TOO_LONG */
                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_SERCOS_LIST_TOO_LONG                   </B></TD></TR><TR><TD width=90>\b
  0x00021014                                  </TD><TD width=240>
  CSMD_CheckVersion()                         </TD><TD width=800>
      - Der Parameter S-0-1000.0.0 "List of SCP classes & version" enthaelt mehr Elemente, als CoSeMa intern speichern kann.
      - The current list length in \ref CSMD_SLAVE_CONFIGURATION.ausActiveSCPClasses[ ] (S-0-1000.0.1) is too long.

      see define \ref CSMD_MAX_ENTRIES_S_0_1000
                                              </TD></TR></TABLE> */
  CSMD_SERCOS_LIST_TOO_LONG,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_S_0_1000_0_1_NOT_SUPPORTED             </B></TD></TR><TR><TD width=90>\b
  0x00021015                                  </TD><TD width=240>
  CSMD_CheckVersion()                         </TD><TD width=800>
      todo description:\n   S-0-1000.0.1 "List of Active SCP Classes & Version" not available by the slave, but application gives active classes.
                                              </TD></TR></TABLE> */
  CSMD_S_0_1000_0_1_NOT_SUPPORTED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_ACT_SCP_CLASS_NOT_SUPPORTED            </B></TD></TR><TR><TD width=90>\b
  0x00021016                                  </TD><TD width=240>
  CSMD_CheckVersion()                         </TD><TD width=800>
      todo description:\n   A class in \ref CSMD_SLAVE_CONFIGURATION.ausActiveSCPClasses[ ] is not a member of S-0-1000.0.0.
                                              </TD></TR></TABLE> */
  CSMD_ACT_SCP_CLASS_NOT_SUPPORTED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_ACT_SCP_MULTIPLE_VERSIONS              </B></TD></TR><TR><TD width=90>\b
  0x00021017                                  </TD><TD width=240>
  CSMD_CheckVersion()                         </TD><TD width=800>
      todo description:\n   Application gives a SCP class with more than one version in \ref CSMD_SLAVE_CONFIGURATION.ausActiveSCPClasses[ ].
                                              </TD></TR></TABLE> */
  CSMD_ACT_SCP_MULTIPLE_VERSIONS,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONNECTION_LENGTH_0                    </B></TD></TR><TR><TD width=90>\b
  0x00021018                                  </TD><TD width=240>
  CSMD_CalculateTiming()<BR>
  CSMD_GetTimingData()<BR>CSMD_TransHP2Para() </TD><TD width=800>
      The configured respectively read connection length is equal 0.
                                              </TD></TR></TABLE> */
  CSMD_CONNECTION_LENGTH_0,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_PRODUCER_CYCLETIMES           </B></TD></TR><TR><TD width=90>\b
  0x00021019                                  </TD><TD width=240>
  CSMD_CalculateTiming()                      </TD><TD width=800>
      todo description:\n   Too many different producer cycle times.
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_PRODUCER_CYCLETIMES,

  CSMD_END_ERR_CLASS_00021,                   /* End marker for error class 0x21 nnn */


  /* --------------------------------------------------------- */
  /* Redundancy error codes           (error_class 0x22 nnn)   */
  /* --------------------------------------------------------- */
                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOPOLOGY_CHANGE                        </B></TD></TR><TR><TD width=90>\b
  0x00022000                                  </TD><TD width=240>
  CSMD_CyclicHandling()                       </TD><TD width=800>
      The topology is checked in each cycle. The network topology has changed.
                                              </TD></TR></TABLE> */
  CSMD_TOPOLOGY_CHANGE = (0x00022000),

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_LINE_BREAK_ERROR                       </B></TD></TR><TR><TD width=90>\b
  0x00022001                                  </TD><TD width=240>
  CSMD_CyclicHandling()                       </TD><TD width=800>
      The call-up for the management of the data in case of cable break has been transferred an invalid topology.
                                              </TD></TR></TABLE> */
  CSMD_LINE_BREAK_ERROR,

  __CSMD_CC_CONFIG_ERROR_not_used,            /* 0x02 */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_RECOVER_RING_ERROR                     </B></TD></TR><TR><TD width=90>\b
  0x00022003                                  </TD><TD width=240>
  CSMD_RecoverRingTopology()                  </TD><TD width=800>
      It has not been possible to heal (restore) the ring, as<BR>
      - the topology of the last slaves of the line is wrong. There is either no or a wrong telegram at the inactive port.
      - there is no ring topology.
      - the communication phase is not equal to CP4.
      - at last one slave has not taken over the specified topology.
      - the physical connection is missing or faulty.
                                              </TD></TR></TABLE> */
  CSMD_RECOVER_RING_ERROR,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_RECOVER_RING_OK                        </B></TD></TR><TR><TD width=90>\b
  0x00022004                                  </TD><TD width=240>
  CSMD_RecoverRingTopology()                  </TD><TD width=800>
      The ring has been recovered successfully.
                                              </TD></TR></TABLE> */
  CSMD_RECOVER_RING_OK,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_OPEN_RING_ERROR                        </B></TD></TR><TR><TD width=90>\b
  0x00022005                                  </TD><TD width=240>
  CSMD_OpenRing()                             </TD><TD width=800>
      It has not been possible to open the ring, as<BR>
      - there is no ring topology.
      - the communication phase is not equal to CP4.
      - at last one slave has not taken over the specified topology.
                                              </TD></TR></TABLE> */
  CSMD_OPEN_RING_ERROR,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_OPEN_RING_INVALID_ADDR                 </B></TD></TR><TR><TD width=90>\b
  0x00022006                                  </TD><TD width=240>
  CSMD_OpenRing()                             </TD><TD width=800>
      The ring cannot be opened as the addresses for opening the ring are not valid.
                                              </TD></TR></TABLE> */
  CSMD_OPEN_RING_INVALID_ADDR,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_OPEN_RING_OK                           </B></TD></TR><TR><TD width=90>\b
  0x00022007                                  </TD><TD width=240>
  CSMD_OpenRing()                             </TD><TD width=800>
      The ring has been opened successfully.
                                              </TD></TR></TABLE> */
  CSMD_OPEN_RING_OK,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_RING_RECOVERY_ABORTED                  </B></TD></TR><TR><TD width=90>\b
  0x00022008                                  </TD><TD width=240>
  CSMD_RecoverRingTopology()                  </TD><TD width=800>
      Topology is changed during ring recovery process.
                                              </TD></TR></TABLE> */
  CSMD_RING_RECOVERY_ABORTED,

  CSMD_END_ERR_CLASS_00022,                   /* End marker for error class 0x22 nnn */


  /* --------------------------------------------------------- */
  /* Hot Plug error codes             (error_class 0x23 nnn)   */
  /* --------------------------------------------------------- */
                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_NOT_SUPPORTED                       </B></TD></TR><TR><TD width=90>\b
  0x00023000                                  </TD><TD width=240>
  CSMD_HotPlug()<BR>CSMD_TransHP2Para()       </TD><TD width=800>
      Plug functionality has not been activated by the master in this CoSeMa instance (see CSMD_InitHardware() ).
                                              </TD></TR></TABLE> */
  CSMD_HP_NOT_SUPPORTED = (0x00023000),

  /*! \todo translation: CSMD_HP_NOT_WITH_CLOSED_RING */
                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_NOT_WITH_CLOSED_RING                </B></TD></TR><TR><TD width=90>\b
  0x00023001                                  </TD><TD width=240>
  CSMD_HotPlug()<BR>CSMD_TransHP2Para()       </TD><TD width=800>
      The current topology is ring. Hot Plug can only be processed at the end of a line or at the second free master port.
                                              </TD></TR></TABLE> */
  CSMD_HP_NOT_WITH_CLOSED_RING,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_NO_HOTPLUG_SLAVE                    </B></TD></TR><TR><TD width=90>\b
  0x00023002                                  </TD><TD width=240>
  CSMD_HotPlug()<BR>CSMD_TransHP2Para()       </TD><TD width=800>
      HP0: No device supporting Hot Plug has been found at the end of either line or at a unused master port. \n
           If a device supporting Hot Plug is connected to the end of a line or unused master port, it provides a link but does not send Sercos telegrams.
                                              </TD></TR></TABLE> */
  CSMD_HP_NO_HOTPLUG_SLAVE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_WRONG_TOPOLOGY                      </B></TD></TR><TR><TD width=90>\b
  0x00023003                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      HP0: The current topology is ring or defect ring.
                                              </TD></TR></TABLE> */
  CSMD_HP_WRONG_TOPOLOGY,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_TOPOLOGY_CHANGE                     </B></TD></TR><TR><TD width=90>\b
  0x00023004                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      All HP0 parameters have been transmitted. The topology recognized in the beginning of the function has changed.
                                              </TD></TR></TABLE> */
  CSMD_HP_TOPOLOGY_CHANGE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_PHASE_0_TIMEOUT                     </B></TD></TR><TR><TD width=90>\b
  0x00023005                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      The transmission of HP0 parameters has been repeated 10 times. A change of the topology bits of the last slave in line or at the unused master port could not be recognized. \n
      Thus the newly connected device does not support Hot Plug.
                                              </TD></TR></TABLE> */
  CSMD_HP_PHASE_0_TIMEOUT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_SLAVE_SCAN_TIMEOUT                  </B></TD></TR><TR><TD width=90>\b
  0x00023006                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      - The Hot Plug device had acknowledged the transmission of HP0 parameters by activating 'Loopback + Forward'. Then the master \n
        commanded 'Fast Forward on both ports', which had not been acknowledged by the last slave in line.<BR>
      - HP0: Timeout scanning the slave addresses of the Hot Plug device. Slave index has not been acknowledged.</TD></TR>
                                              </TD></TR></TABLE> */
  CSMD_HP_SLAVE_SCAN_TIMEOUT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_NO_SLAVE_FOUND                      </B></TD></TR><TR><TD width=90>\b
  0x00023007                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      HP0: No slaves were found scanning the Hot Plug device.
                                              </TD></TR></TABLE> */
  CSMD_HP_NO_SLAVE_FOUND,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_PHASE_0_FAILED                      </B></TD></TR><TR><TD width=90>\b
  0x00023008                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      HP0: Scanning the Hot Plug device,<BR>
           - more than 16 slaves were found.
           - an invalid Sercos address (&lt;1 or &gt;511) was found.
                                              </TD></TR></TABLE> */
  CSMD_HP_PHASE_0_FAILED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_SLAVE_RECOGNIZED_IN_CP0             </B></TD></TR><TR><TD width=90>\b
  0x00023009                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      HP1: An address found scanning the Hot Plug device was already found during regular Communication phase progression in CP0 \n
           (see \ref CSMD_SLAVE_LIST.ausRecogSlaveAddList[] ).
                                              </TD></TR></TABLE> */
  CSMD_HP_SLAVE_RECOGNIZED_IN_CP0,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_SLAVE_IS_NOT_PROJECTED              </B></TD></TR><TR><TD width=90>\b
  0x0002300A                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      HP1: A slave address found scanning the Hot Plug device has not been projected.<BR>
           (see \ref CSMD_SLAVE_LIST.ausProjSlaveAddList[] ).
                                              </TD></TR></TABLE> */
  CSMD_HP_SLAVE_IS_NOT_PROJECTED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_PHASE_1_TIMEOUT                     </B></TD></TR><TR><TD width=90>\b
  0x0002300B                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      The transmission of a HP1 parameter was not acknowledged positively by a slave of the Hot Plug device.
                                              </TD></TR></TABLE> */
  CSMD_HP_PHASE_1_TIMEOUT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_SWITCH_TO_SVC_FAILED                </B></TD></TR><TR><TD width=90>\b
  0x0002300C                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      After the transmission of the HP1 parameters, the Hot Plug device is commanded to prepare itself for SVC access. \n
      This transition has been acknowledged with an error.
                                              </TD></TR></TABLE> */
  CSMD_HP_SWITCH_TO_SVC_FAILED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_SWITCH_TO_SVC_TIMEOUT               </B></TD></TR><TR><TD width=90>\b
  0x0002300D                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      HP1: The correct transition of all slave of the Hot Plug device to SVC communication is validated. At least one slave of the Hot Plug device \n
           does not generate SVC_Valid or did not acknowledge the SVC_Handshake.
                                              </TD></TR></TABLE> */
  CSMD_HP_SWITCH_TO_SVC_TIMEOUT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_WRONG_PHASE                         </B></TD></TR><TR><TD width=90>\b
  0x0002300E                                  </TD><TD width=240>
  CSMD_TransHP2Para()                         </TD><TD width=800>
      The master is not in HP2. The function CSMD_HotPlug() was not called or finished erroneously.
                                              </TD></TR></TABLE> */
  CSMD_HP_WRONG_PHASE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_OPERATION_ABORTED                   </B></TD></TR><TR><TD width=90>\b
  0x0002300F                                  </TD><TD width=240>
  CSMD_HotPlug()<BR>CSMD_TransHP2Para()       </TD><TD width=800>
      Via the boCancel flag, the function was instructed to abort the hot plug sequence. The original topology is re-established.
                                              </TD></TR></TABLE> */
  CSMD_HP_OPERATION_ABORTED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_LAST_SLAVE_LB_RESTORE_TIMEOUT       </B></TD></TR><TR><TD width=90>\b
  0x00023010                                  </TD><TD width=240>
  CSMD_HotPlug()<BR>CSMD_TransHP2Para()       </TD><TD width=800>
      In case of an Hot Plug error: Switch to loopback & forward of last slave in line failed.
                                              </TD></TR></TABLE> */
  CSMD_HP_LAST_SLAVE_LB_RESTORE_TIMEOUT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_SWITCH_TO_CP4_FAILED                </B></TD></TR><TR><TD width=90>\b
  0x00023011                                  </TD><TD width=240>
  CSMD_TransHP2Para()                         </TD><TD width=800>
      Not all slave(s) of the HP device have set the S-DEV.Valid after successful execution of the CP4 transition check command.
                                              </TD></TR></TABLE> */
  CSMD_HP_SWITCH_TO_CP4_FAILED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_DOUBLE_SLAVE_ADDRESSES              </B></TD></TR><TR><TD width=90>\b
  0x00023012                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      HP1: Two or more slaves with identical Sercos address were found during scanning the Hot Plug device.
                                              </TD></TR></TABLE> */
  CSMD_HP_DOUBLE_SLAVE_ADDRESSES,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_HP_ILLEGAL_SLAVE_ADDRESS               </B></TD></TR><TR><TD width=90>\b
  0x00023013                                  </TD><TD width=240>
  CSMD_HotPlug()                              </TD><TD width=800>
      HP1: HP slave scan found a slave with Sercos address &lt; \ref CSMD_MIN_SLAVE_ADD respectively &gt; \ref CSMD_MAX_SLAVE_ADD.
                                              </TD></TR></TABLE> */
  CSMD_HP_ILLEGAL_SLAVE_ADDRESS,

  CSMD_END_ERR_CLASS_00023,                   /* End marker for error class 0x23 nnn */


  /* --------------------------------------------------------- */
  /* Configuration Parser error codes (error_class 0x24 nnn)   */
  /* --------------------------------------------------------- */
                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_BIN_CONFIG                          </B></TD></TR><TR><TD width=90>\b
  0x00024000                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      - The length of the list in pvSourceBinConfig is shorter than the header (12 bytes)
      - The length of the list in pvSourceBinConfig is not long-aligned
      - The header does not start with 'CSMCfg_bin'
                                              </TD></TR></TABLE> */
  CSMD_NO_BIN_CONFIG = (0x00024000),

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_BIN_CONFIG_VERSION               </B></TD></TR><TR><TD width=90>\b
  0x00024001                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      The transmitted version is not supported. Currently supported versions:
      - 01V01
                                              </TD></TR></TABLE> */
  CSMD_WRONG_BIN_CONFIG_VERSION,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_BIN_CONFIG_FORMAT                </B></TD></TR><TR><TD width=90>\b
  0x00024002                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      - The Connections Table does not directly follow the header
      - The length of the RT Bits Configuration Table exceeds the list length
      - The end label is missing
                                              </TD></TR></TABLE> */
  CSMD_WRONG_BIN_CONFIG_FORMAT,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_WRONG_SLAVE_ADDRESS                    </B></TD></TR><TR><TD width=90>\b
  0x00024003                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      At least one slave address given in either the Producer or the Consumer Table does not exist in the list of projected slaves.
                                              </TD></TR></TABLE> */
  CSMD_WRONG_SLAVE_ADDRESS,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONN_NBR_ALREADY_USED                  </B></TD></TR><TR><TD width=90>\b
  0x00024004                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      The transmitted connection number is already in use (only if boConnNumGen == FALSE).
                                              </TD></TR></TABLE> */
  CSMD_CONN_NBR_ALREADY_USED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONN_INST_ALREADY_USED                 </B></TD></TR><TR><TD width=90>\b
  0x00024005                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      The transmitted connection instance is already in use (only if boSlaveInstGen == FALSE).
                                              </TD></TR></TABLE> */
  CSMD_CONN_INST_ALREADY_USED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONN_INST_TOO_HIGH                     </B></TD></TR><TR><TD width=90>\b
  0x00024006                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      The transmitted connection instance is greater than allowed, i.e. &gt; \ref CSMD_MAX_CONNECTIONS_MASTER for the master, &gt; \ref CSMD_MAX_CONNECTIONS \n
      for the slave (only if boSlaveInstGen == FALSE).
                                              </TD></TR></TABLE> */
  CSMD_CONN_INST_TOO_HIGH,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_CONN_FOR_MASTER               </B></TD></TR><TR><TD width=90>\b
  0x00024007                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      There are too many connections configured for the master (&gt; \ref CSMD_MAX_CONNECTIONS_MASTER, only for boSlaveInstGen == TRUE)
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_CONN_FOR_MASTER,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_CONN_FOR_SLAVE                </B></TD></TR><TR><TD width=90>\b
  0x00024008                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      There are too many connections configured for a slave (&gt; \ref CSMD_MAX_CONNECTIONS, only for boSlaveInstGen == TRUE)
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_CONN_FOR_SLAVE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_CONNECTIONS                   </B></TD></TR><TR><TD width=90>\b
  0x00024009                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      There are more connections to be configured than available (overall number of connections &gt; \ref CSMD_MAX_GLOB_CONN)
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_CONNECTIONS,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_CONFIGURATIONS                </B></TD></TR><TR><TD width=90>\b
  0x0002400A                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      There are more configurations to be configured than available (overall number of configurations &gt; \ref CSMD_MAX_GLOB_CONFIG)
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_CONFIGURATIONS,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_RTB_CONFIG                    </B></TD></TR><TR><TD width=90>\b
  0x0002400B                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      There are more Real-Time Bit configurations to be configured than available (overall number of Real-Time Bit configurations &gt; \ref CSMD_MAX_RT_BIT_CONFIG)
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_RTB_CONFIG,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_IDN_FOR_CONN                  </B></TD></TR><TR><TD width=90>\b
  0x0002400C                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      There are more IDN to be configured for a connection than available (overall number of IDNs &gt; \ref CSMD_MAX_IDN_PER_CONNECTION)
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_IDN_FOR_CONN,

  __CSMD_TOO_MANY_RTB_FOR_CONN_not_used,      /* 0x0D */      /* Number of real time bits exceeds \ref CSMD_MAX_RT_BITS_PER_CONN */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_BIN_CONFIG_VERSION_UNAVAILABLE         </B></TD></TR><TR><TD width=90>\b
  0x0002400E                                  </TD><TD width=240>
  CSMD_GenerateBinConfig()                    </TD><TD width=800>
      The version consigned in usS3binVersion of the binary configuration is unknown.
                                              </TD></TR></TABLE> */
  CSMD_BIN_CONFIG_VERSION_UNAVAILABLE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_APPID_UNAVAILABLE                      </B></TD></TR><TR><TD width=90>\b
  0x0002400F                                  </TD><TD width=240>
  CSMD_GenerateBinConfig()                    </TD><TD width=800>
      The version consigned in usAppID of the binary configuration is not available at the moment.
                                              </TD></TR></TABLE> */
  CSMD_APPID_UNAVAILABLE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_PRODUCER_KEY                        </B></TD></TR><TR><TD width=90>\b
  0x00024010                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      - A producer key consigned in the Connections Table equals 0.
      - A producer key consigned in the Connections Table does not exist in the Producer Table
                                              </TD></TR></TABLE> */
  CSMD_NO_PRODUCER_KEY,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_CONSUMER_LIST_KEY                   </B></TD></TR><TR><TD width=90>\b
  0x00024011                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      - A consumer list key consigned in the Connections Table equals 0.
      - A consumer list key consigned in the Connections Table does not exist in the Consumer List Table
                                              </TD></TR></TABLE> */
  CSMD_NO_CONSUMER_LIST_KEY,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_CONSUMER_KEY                        </B></TD></TR><TR><TD width=90>\b
  0x00024012                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      - A consumer key consigned in the Consumer List Table equals 0.
      - A consumer key consigned in the Consumer Table does not exist in the Consumer Table
                                              </TD></TR></TABLE> */
  CSMD_NO_CONSUMER_KEY,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_CONFIGURATION_KEY                   </B></TD></TR><TR><TD width=90>\b
  0x00024013                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      - A configuration key consigned in the Producer or Consumer Table equals 0.
      - A configuration key consigned in the Producer or Consumer Table does not exist in the Configuration Table
                                              </TD></TR></TABLE> */
  CSMD_NO_CONFIGURATION_KEY,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_RTB_CONFIG_KEY                      </B></TD></TR><TR><TD width=90>\b
  0x00024014                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      - A Real-Time Bit configuration key consigned in the Producer or Consumer Table equals 0.
      - A Real-Time Bit configuration key consigned in the Producer or Consumer Table does not exist in the Real-Time Bit Configuration Table
                                              </TD></TR></TABLE> */
  CSMD_NO_RTB_CONFIG_KEY,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_PRODUCER                            </B></TD></TR><TR><TD width=90>\b
  0x00024015                                  </TD><TD width=240>
  CSMD_GenerateBinConfig()                    </TD><TD width=800>
      A connection for which a binary connection configuration should be created does not have a producer.
                                              </TD></TR></TABLE> */
  CSMD_NO_PRODUCER,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_CONSUMER                            </B></TD></TR><TR><TD width=90>\b
  0x00024016                                  </TD><TD width=240>
  CSMD_GenerateBinConfig()                    </TD><TD width=800>
      A connection for which a binary connection configuration should be created does not have a consumer.
                                              </TD></TR></TABLE> */
  CSMD_NO_CONSUMER,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_DOUBLE_PRODUCER                        </B></TD></TR><TR><TD width=90>\b
  0x00024017                                  </TD><TD width=240>
  CSMD_GenerateBinConfig()                    </TD><TD width=800>
      A connection for which a binary connection configuration should be created has more than one producer.
                                              </TD></TR></TABLE> */
  CSMD_DOUBLE_PRODUCER,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CYCLE_TIME_UNEQUAL                     </B></TD></TR><TR><TD width=90>\b
  0x00024018                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      The connection cycle time of at least one consumer does not match the connection cycle time of the corresponding producer.
                                              </TD></TR></TABLE> */
  CSMD_CYCLE_TIME_UNEQUAL,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_BUFFER_TOO_SMALL                       </B></TD></TR><TR><TD width=90>\b
  0x00024019                                  </TD><TD width=240>
  CSMD_GenerateBinConfig()<BR>
  CSMD_Serc_Mon_Config()                      </TD><TD width=800>
      - In CSMD_GenerateBinConfig():<BR>
        todo description:  Given length of buffer is too small to hold the connection configuration.
      - In CSMD_Serc_Mon_Config():<BR>
        - Given length of buffer is smaller than define CSMD_MIN_BUFFER_LENTGH.
        - Given length of buffer is too small to hold the generated network configuration.
                                              </TD></TR></TABLE> */
  CSMD_BUFFER_TOO_SMALL,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_SLAVE_SETUP                   </B></TD></TR><TR><TD width=90>\b
  0x0002401A                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      todo description:\n   Number of slave parameter configs exceeds \ref CSMD_MAX_SLAVE_CONFIGPARAMS.
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_SLAVE_SETUP,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_PARAMETER_IN_LIST             </B></TD></TR><TR><TD width=90>\b
  0x0002401B                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      todo description:\n   Number of parameters in list exceeds \ref CSMD_MAX_PARAMS_IN_CONFIG_LIST.
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_PARAMETER_IN_LIST,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_PARAMETER_DATA                </B></TD></TR><TR><TD width=90>\b
  0x0002401C                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      todo description:\n   Amount of parameter data exceeds \ref CSMD_NBR_PARAM_DATA.
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_PARAMETER_DATA,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_SETUP_PARAMETER               </B></TD></TR><TR><TD width=90>\b
  0x0002401D                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      todo description:\n   Number of parameters exceeds \ref CSMD_MAX_CONFIG_PARAMETER.
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_SETUP_PARAMETER,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TOO_MANY_SETUP_LISTS                   </B></TD></TR><TR><TD width=90>\b
  0x0002401E                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      todo description:\n   Number of parameter setup lists exceeds \ref CSMD_MAX_CONFIGPARAMS_LIST.
                                              </TD></TR></TABLE> */
  CSMD_TOO_MANY_SETUP_LISTS,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_SETUP_PARAMETER_KEY                 </B></TD></TR><TR><TD width=90>\b
  0x0002401F                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      todo description:\n   Setup parameter key not found in table.
                                              </TD></TR></TABLE> */
  CSMD_NO_SETUP_PARAMETER_KEY,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_NO_SETUP_LIST_KEY                      </B></TD></TR><TR><TD width=90>\b
  0x00024020                                  </TD><TD width=240>
  CSMD_ProcessBinConfig()                     </TD><TD width=800>
      todo description:\n   Setup parameter list key not found in table.
                                              </TD></TR></TABLE> */
  CSMD_NO_SETUP_LIST_KEY,

  CSMD_END_ERR_CLASS_00024,                   /* End marker for error class 0x24 nnn */


  /* ----------------------------------------------------------------------------- */
  /* errors codes resulting from connection state machine (error_class 0x25 nnn)   */
  /* ----------------------------------------------------------------------------- */
                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONNECTION_NOT_CONFIGURED              </B></TD></TR><TR><TD width=90>\b
  0x00025000                                  </TD><TD width=240>
  CSMD_SetConnectionState()<BR> CSMD_GetConnectionState()<BR> CSMD_SetConnectionData()<BR>
  CSMD_GetConnectionData()                    </TD><TD width=800>
      Desired connection is not configured.
                                              </TD></TR></TABLE> */
  CSMD_CONNECTION_NOT_CONFIGURED = (0x00025000),

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONNECTION_NOT_MASTERPRODUCED          </B></TD></TR><TR><TD width=90>\b
  0x00025001                                  </TD><TD width=240>
  CSMD_SetConnectionState()<BR>
  CSMD_SetConnectionData()                    </TD><TD width=800>
      Connection desired in set function is produced by a slave.
                                              </TD></TR></TABLE> */
  CSMD_CONNECTION_NOT_MASTERPRODUCED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONNECTION_NOT_SLAVEPRODUCED           </B></TD></TR><TR><TD width=90>\b
  0x00025002                                  </TD><TD width=240>
  CSMD_GetConnectionData()<BR> CSMD_GetConnectionDataDelay()<BR>
  CSMD_ClearConnectionError()                 </TD><TD width=800>
      Connection desired in get function is produced by the master.
                                              </TD></TR></TABLE> */
  CSMD_CONNECTION_NOT_SLAVEPRODUCED,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_ILLEGAL_CONNECTION_STATE               </B></TD></TR><TR><TD width=90>\b
  0x00025003                                  </TD><TD width=240>
  CSMD_SetConnectionState()                   </TD><TD width=800>
      The given connection state is not 'ready', 'stopping' or 'prepare'.
                                              </TD></TR></TABLE> */
  CSMD_ILLEGAL_CONNECTION_STATE,

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_CONNECTION_DATA_INVALID                </B></TD></TR><TR><TD width=90>\b
  0x00025004                                  </TD><TD width=240>
  CSMD_GetConnectionData()                    </TD><TD width=800>
      Slave valid in producer's S-DEV is not set in current cycle.
                                              </TD></TR></TABLE> */
  CSMD_CONNECTION_DATA_INVALID,

                                              /*! <TABLE><TR><TD colspan=3 style="color:DarkGray"><B>
  CSMD_WAITING_FOR_COUNTER                    </B></TD></TR><TR><TD width=90 style="color:DarkGray">\b
  0x00025005                                  </TD><TD width=240>
  CSMD_ClearConnectionError()                 </TD><TD width=800>
      Either C-CON counter or New Data bit is greater than zero.\n
      <B>Note: This error message is no longer generated.</B>
                                              </TD></TR></TABLE> */
  CSMD_WAITING_FOR_COUNTER,

  CSMD_END_ERR_CLASS_00025,                   /* End marker for error class 0x25 nnn */

#if(0)  /* wk/16.07.2014 Test: Integration Dokumentation der Fehler komplett in CSMD_GLOB.h
           --> Geht nicht: CSMD_TEST_1 wird nicht in die section error_sec26 eingetragen!
               Ursache: Beginn eines neuen Kommentarblocks                                 */

  /* ----------------------------------------------------------------------------- */
  /* Test error codes             (error_class 0x26 nnn)                           */
  /* ----------------------------------------------------------------------------- */

  /*! \defgroup module_code_err_26 Test error codes (26xxx)
      \ingroup module_errors
      \section error_sec26 Test (error_class 0x00026nnn)
      <TABLE>
      <TR><TD colspan=3 align=left bgcolor="lightgrey"><B>Name of the enumerator</B></TD></TR>
      <TR><TD width=90 bgcolor="lightgrey">Error code</TD> <TD width=240 bgcolor="lightgrey">Concerned functions</TD>
          <TD width=800 bgcolor="lightgrey">Description</TD></TR></TABLE> */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TEST_1                                 </B></TD></TR><TR><TD width=90>\b
  0x00026005                                  </TD><TD width=240>
  todo:<BR> Concerned function(s)             </TD><TD width=800>
      Documentation for CSMD_TEST_1.
                                              </TD></TR></TABLE> */
  CSMD_TEST_1 = (0x00026000),                 /* 0x00 */

                                              /*! <TABLE><TR><TD colspan=3><B>
  CSMD_TEST_2                                 </B></TD></TR><TR><TD width=90>\b
  0x00026006                                  </TD><TD width=240>
  todo:<BR> Concerned function(s)             </TD><TD width=800>
      Documentation for CSMD_TEST_1.
                                              </TD></TR></TABLE> */
  CSMD_TEST_2,                                /* 0x01 */

  CSMD_END_ERR_CLASS_00026,                   /* End marker for error class 0x26 nnn */
#endif


  /* ----------------------------------------------------- */
  /* function error codes ???                              */
  /* ----------------------------------------------------- */
  /* CSMD_FUNCTION_ERROR   = (0x00030000) */  /*!< General: */

  CSMD_FUNC_RET_END_OF_LIST                   /*  */

} CSMD_FUNC_RET;




/* -------------------------------------------------------------------------- */
/* todo review translation */
/*! \brief <B> Service channel error messages </B>
      \n
      The slave announce an error code in the service INFO field of the AT.
      If a service channel function returns with the error code \n
      CSMD_SVC_ERROR_MESSAGE (CSMD_SERCOS_ERROR), the service channel error
      message is stored as follows:
      - for macro functions in....CSMD_SVCH_MACRO_STRUCT.usSvchError
      - for atomic functions in...CSMD_SVCH_MNGMT_STRUCT.usSvchError          */
/* -------------------------------------------------------------------------- */
typedef enum CSMD_SVC_ERROR_CODES_EN
{
  /*---------------------------------------------------- */
  /* Element independent              (err_class 0x0nnn) */
  /*---------------------------------------------------- */
  CSMD_SVC_NO_ERR,                            /*!<    0  No error in the service channel */
  CSMD_SVC_NOT_OPEN,                          /*!<    1  Service channel not open */
  CSMD_SVC_GRIP_ELEM0             = (0x0009), /*!<    9  Invalid access to closing the service channel */

  /*---------------------------------------------------- */
  /* Element 1: IDN                   (err_class 0x1nnn) */
  /*---------------------------------------------------- */
  CSMD_SVC_ID_NOT_THERE           = (0x1001), /*!< 1001  IDN not available */
  CSMD_SVC_GRIP_ELEM1             = (0x1009), /*!< 1009  Invalid access to element 1 */

  /*---------------------------------------------------- */
  /* Element 2: Name                  (err_class 0x2nnn) */
  /*---------------------------------------------------- */
  CSMD_SVC_NO_NAME                = (0x2001), /*!< 2001  Name not available */
  CSMD_SVC_NAME_TOO_SHORT,                    /*!< 2002  Name transmission too short */
  CSMD_SVC_NAME_TOO_LONG,                     /*!< 2003  Name transmission too long */
  CSMD_SVC_WRITE_NAME,                        /*!< 2004  Name cannot be changed (read only) */
  CSMD_SVC_JUST_NO_NAME,                      /*!< 2005  Name is write-protected at this time */
                                              /*!< 2016  Application timeout occurs in the slave */

  /*---------------------------------------------------- */
  /* Element 3: Attribute             (err_class 0x3nnn) */
  /*---------------------------------------------------- */
  CSMD_SVC_ATTR_TOO_SHORT         = (0x3002), /*!< 3002  Attribute transmission too short */
  CSMD_SVC_ATTR_TOO_LONG,                     /*!< 3003  Attribute transmission too long */
  CSMD_SVC_WRITE_ATTR,                        /*!< 3004  Attribute cannot be changed (read only) */
  CSMD_SVC_JUST_NO_ATTR,                      /*!< 3005  Attribute is write-protected at this time */
                                              /*!< 3016  Application timeout occurs in the slave */

  /*---------------------------------------------------- */
  /* Element 4: Unit                  (err_class 0x4nnn) */
  /*---------------------------------------------------- */
  CSMD_SVC_NO_UNIT                = (0x4001), /*!< 4001  Unit not available */
  CSMD_SVC_UNIT_TOO_SHORT,                    /*!< 4002  Unit transmission too short */
  CSMD_SVC_UNIT_TOO_LONG,                     /*!< 4003  Unit transmission too long */
  CSMD_SVC_WRITE_UNIT,                        /*!< 4004  Unit cannot be changed (read only) */
  CSMD_SVC_JUST_NO_UNIT,                      /*!< 4005  Unit is write-protected at this time */
                                              /*!< 4016  Application timeout occurs in the slave */

  /*---------------------------------------------------- */
  /* Element 5: Minimum input value   (err_class 0x5nnn) */
  /*---------------------------------------------------- */
  CSMD_SVC_NO_MIN                 = (0x5001), /*!< 5001  Minimum input value not available */
  CSMD_SVC_MIN_TOO_SHORT,                     /*!< 5002  Minimum input value transmission too short */
  CSMD_SVC_MIN_TOO_LONG,                      /*!< 5003  Minimum input value transmission too long */
  CSMD_SVC_WRITE_MIN,                         /*!< 5004  Minimum input value cannot be changed (read only) */
  CSMD_SVC_JUST_NO_MIN,                       /*!< 5005  Minimum input value is write-protected at this time */
                                              /*!< 5016  Application timeout occurs in the slave */

  /*---------------------------------------------------- */
  /* Element 6: Maximum input value   (err_class 0x6nnn) */
  /*---------------------------------------------------- */
  CSMD_SVC_NO_MAX                 = (0x6001), /*!< 6001  Maximum input value not available */
  CSMD_SVC_MAX_TOO_SHORT,                     /*!< 6002  Maximum input value transmission too short */
  CSMD_SVC_MAX_TOO_LONG,                      /*!< 6003  Maximum input value transmission too long */
  CSMD_SVC_WRITE_MAX,                         /*!< 6004  Maximum input value cannot be changed (read only) */
  CSMD_SVC_JUST_NO_MAX,                       /*!< 6005  Maximum input value is write-protected at this time */
                                              /*!< 6016  Application timeout occurs in the slave */

  /*---------------------------------------------------- */
  /* Element 7: Operation data        (err_class 0x7nnn) */
  /*---------------------------------------------------- */
  CSMD_SVC_DATA_TOO_SHORT         = (0x7002), /*!< 7002  Operation data transmission too short */
  CSMD_SVC_DATA_TOO_LONG,                     /*!< 7003  Operation data transmission too long */
  CSMD_SVC_WRITE_DATA,                        /*!< 7004  Operation data cannot be changed (read only) */
  CSMD_SVC_JUST_NO_DATA,                      /*!< 7005  Operation data is write-protected at this communication phase */
  CSMD_SVC_DATA_TOO_SMALL,                    /*!< 7006  Operation data is smaller than the minimum input value */
  CSMD_SVC_DATA_TOO_LARGE,                    /*!< 7007  Operation data is greater than the maximum input value */
  CSMD_SVC_DATA_NOT_CORRECT,                  /*!< 7008  Invalid operation data. (Configured IDN will not be supported. /
                                                         Invalid bit number or bit combination) */
  CSMD_SVC_WRITE_DATA_PASS,                   /*!< 7009  Operation data write protected by a password */
  CSMD_SVC_NO_WRITE_DATA_NOW,                 /*!< 700A  Operation data is write protected, it is configured cyclically.
                                                         (IDN is configured in the MDT or AT. Therefore writing via
                                                          the service channel is not allowed).*/
  CSMD_SVC_ADR_NOT_KORREKT,                   /*!< 700B  Invalid indirect addressing: (e.g., data container, list handling) */
  CSMD_SVC_PROTECTED_BY_OTHERS,               /*!< 700C  Operation data is write protected due to other settings.
                                                         (e.g. parameter, operation mode, sub-device is enabled etc.) */
  CSMD_SVC_INVALID_FLOAT,                     /*!< 700D  Invalid floating point number */
  CSMD_SVC_NO_WRITE_DATA_PL,                  /*!< 700E  Operation data is write protected at parameterization level */
  CSMD_SVC_NO_WRITE_DATA_OL,                  /*!< 700F  Operation data is write protected at operating level */
  CSMD_SVC_COM_ACTIVE_NOW,                    /*!< 7010  Procedure command already active */
  CSMD_SVC_COM_CANT_BREAK,                    /*!< 7011  Procedure command not interruptible */
  CSMD_SVC_COM_CANT_ACTIVE_NOW,               /*!< 7012  Procedure command at this time not executable
                                                         (e.g. in this communication phase the procedure command cannot be activated. */
  CSMD_SVC_COM_PARA_FALSE,                    /*!< 7013  Procedure command not executable (invalid or false parameters) */
                                              /*!  7014  The received current length of list parameter
                                                         does not match to expectation */
                                              /*!< 7015  Operation data is not yet created completely */
                                              /*!< 7016  Application timeout occurs in the slave */

  /*---------------------------------------------------- */
  /* Segmentwise SVC access for                          */
  /* parameters with variable length  (err_class 0x71nn) */
  /*---------------------------------------------------- */
  CSMD_SVC_SEG_IDN_NOT_VALID      = (0x7101), /*!< 7101  IDN in S-0-0394 not valid */
  CSMD_SVC_SEG_WRITE_EMPTY_LIST,              /*!< 7102  Empty list in S-0-0397 not allowed for write access */
  CSMD_SVC_SEG_MAX_LEN_EXCEEDED,              /*!< 7103  Maximum length of the list in S-0-0394 is exceeded
                                                         by take-over of the list segment.*/
  CSMD_SVC_SEG_READ_ONLY,                     /*!< 7104  Read access only: The length of the list segment as of the
                                                         list index exceeds the current length of the list in S-0-0394.*/
  CSMD_SVC_SEG_WRITE_PROTECTED,               /*!< 7105  IDN in S-0-0394 is write protected */
  CSMD_SVC_SEG_DATA_TOO_SMALL,                /*!< 7106  Operation data in list segment is smaller than the minimum input value */
  CSMD_SVC_SEG_DATA_TOO_LARGE,                /*!< 7107  Operation data in list segment is greater than the maximum input value */
  CSMD_SVC_SEG_INVALID_LIST_IDX,              /*!< 7108  Invalid list index in S-0-0395 */
  CSMD_SVC_SEG_IDN_NO_LIST,                   /*!< 7109  Parameter in IDN S-0-0394 does not have variable length */
  CSMD_SVC_SEG_IDN_NOT_PERMITTED              /*!< 710A  IDN S-0-0397 not permitted as data in S-0-0394 */

} CSMD_SVC_ERROR_CODES;

/*---- Definition resp. Declaration private Variables: -----------------------*/

/*---- Declaration private Functions: ----------------------------------------*/


/*! \endcond */ /* PUBLIC */

#endif /* _CSMD_ERR_CODES */




/*
--------------------------------------------------------------------------------
  Modification history
--------------------------------------------------------------------------------

03 Sep 2014 WK
  First Version
  WP-00044851 - Miscellaneous code clean-up for CoSeMa 6.1
    Insert enumerations CSMD_FUC_RET and CSMD_SVC_ERROR_CODES
    to this new file.
09 Oct 2014 WK
  Defdb00174053
  - Added elements in structure CSMD_FUNC_RET:
      CSMD_HP_DOUBLE_SLAVE_ADDRESSES
      CSMD_HP_ILLEGAL_SLAVE_ADDRESS
24 Feb 2015 WK
  - Defdb00177060
    Added element in structure CSMD_FUNC_RET:
      CSMD_CONNECTION_LENGTH_0
27 May 2015 WK
  - Defdb00179390
    Change from LGPL to a modified MIT License.
19 May 2016 WK
  - Defdb00187136
    Revised description of errors codes resulting from connection state machine.
16 Jun 2016 WK
  - FEAT-00051878 - Support for Fast Startup
    Added element in structure CSMD_FUNC_RET:
      CSMD_MAX_T_NETWORK_GT_T_SYNCDELAY
01 Jul 2016 WK
  - Adjustment for doxygen documentation.

--------------------------------------------------------------------------------
*/
