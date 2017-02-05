/**
 * \file      SIII_SVC.c
 *
 * \brief     Sercos III soft master stack - Service channel handling
 *
 * THIS SOFTWARE IS PROVIDED "AS IS"; WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY;
 * FITNESS FOR A PERTICULAR PURPOSE AND NONINFRINGEMENT. THE AUTHORS OR COPYRIGHT
 * HOLDERS SHALL NOT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE;
 * UNLESS STIPULATED BY MANDATORY LAW.
 *
 * \ingroup   SIII
 *
 * \author    GMy
 *
 * \copyright Copyright Bosch Rexroth AG, 2013-2016
 *
 * \date      2013-01-18
 *
 * \version 2013-02-19 (GMy): Performance optimization: SVC thread now
 *                            continuously runs and does not need to be started
 *                            again for every SVC transfer. Support of list
 *                            data types.
 * \version 2013-02-28 (GMy): Lint code optimization
 * \version 2013-03-05 (GMy): New high-level functions for SVC access
 * \version 2013-04-11 (GMy): Module re-arrangement
 * \version 2013-04-22 (GMy): New function SIII_SVCClearErrors()
 * \version 2013-05-07 (GMy): Added support for INtime
 * \version 2013-05-17 (GMy): Code optimization
 * \version 2013-06-06 (GMy): Added support for RTX and Kithara
 * \version 2013-06-20 (GMy): Optimization of error handling
 * \version 2013-06-21 (GMy): Added support for Windows desktop versions and
 *                            QNX
 * \version 2016-08-22 WK:    Added support for parameter set of an IDN.
 * \version 2016-10-27 (AlM): Support for CoSeMa V5 removed.
 */

//---- includes ---------------------------------------------------------------

#include "../SIII/SIII_GLOB.h"
#include "../SIII/SIII_PRIV.h"

#include "../CSMD/CSMD_GLOB.h"
#include "../CSMD/CSMD_PRIV_SVC.h"

#ifdef __qnx__
#include "../RTQX/RTQX_GLOB.h"
#include "../RTQX/RTQX_S3SM_GLOB.h"
#elif defined __unix__
#include "../RTLX/RTLX_GLOB.h"
#include "../RTLX/RTLX_S3SM_GLOB.h"
#elif defined WINCE7
#include "../RTC7/RTC7_GLOB.h"
#include "../RTC7/RTC7_S3SM_GLOB.h"
#elif defined WINCE
#include "../RTC6/RTC6_GLOB.h"
#include "../RTC6/RTC6_S3SM_GLOB.h"
#elif defined __INTIME__
#include "../RTIT/RTIT_GLOB.h"
#include "../RTIT/RTIT_S3SM_GLOB.h"
#elif defined __RTX__
#include "../RTRX/RTRX_GLOB.h"
#include "../RTRX/RTRX_S3SM_GLOB.h"
#elif defined __VXWORKS__
#include "../RTVW/RTVW_GLOB.h"
#include "../RTVW/RTVW_S3SM_GLOB.h"
#elif defined __KITHARA__
#include "../RTKT/RTKT_GLOB.h"
#include "../RTKT/RTKT_S3SM_GLOB.h"
#elif defined WIN32
#include "../RTWI/RTWI_GLOB.h"
#include "../RTWI/RTWI_S3SM_GLOB.h"
#elif defined WIN64
#include "../RTWI/RTWI_GLOB.h"
#include "../RTWI/RTWI_S3SM_GLOB.h"
#else
#error Operating system not supported by SIII!
#endif

//---- defines ----------------------------------------------------------------

// Macros for decoding parameters from CoSeMa's internal idn representation

/**
 * \def SIII_GET_S_P(_idn)
 *
 * \brief   This macro returns either 'S' in case it is a standard parameter or
 *          'P' in case of a device-specific parameter.
 */
#define SIII_GET_S_P(_idn)              ((((_idn) & ((ULONG) 1) << ((ULONG) 15))   \
                                            != ((ULONG) 1) << ((ULONG) 15)) ?   \
                                                (UCHAR)'S' : (UCHAR)'P')

/**
 * \def SIII_GET_IDNSET(_idn)
 *
 * \brief   This macro returns the set of the parameter.
 */
#define SIII_GET_IDNSET(_idn)          ((ULONG)((_idn & 0x00007000) >> 12))

 /**
 * \def SIII_GET_IDN(_idn)
 *
 * \brief   This macro returns the IDN of the parameter.
 */
#define SIII_GET_IDN(_idn)              ((ULONG)(_idn & 0x00000FFF))

/**
 * \def SIII_GET_SE(_idn)
 *
 * \brief   This macro returns the structural element (SE) of the parameter.
 */
#define SIII_GET_SE(_idn)               ((ULONG)((_idn & 0x00FF0000) >> 16))

/**
 * \def SIII_GET_SI(_idn)
 *
 * \brief   This macro returns the structural instance (SI) of the parameter.
 */
#define SIII_GET_SI(_idn)               ((ULONG)((_idn & 0xFF000000) >> 24))

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn VOID SIII_SVCThread(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \private
 *
 * \brief   Service channel thread.
 *
 * \param[in,out] prS3Instance Pointer to SIII instance structure
 *
 * \details This function is activated when the semaphore
 *          prS3Instance->semSVCStart is posted. The thread handles the calling
 *          of the CoSeMa soft service channel interface. The thread must not
 *          be activated before Sercos phase CP2.
 *
 *          Input to the function are the data structures
 *          prS3Instance->rMySvcMacro and prS3Instance->rMySVCAccess that have
 *          to be initialized accordingly before, as well as the CoSeMa instance
 *          prS3Instance->rCosemaInstance.
 *
 *          The output data is returned in the data structure
 *          prS3Instance->rMySVCResult.
 *          Possible values of the return value
 *          prS3Instance->rMySVCResult.eFuncRet are as follows:
 *          - SIII_NO_ERROR             for success
 *          - SIII_SVC_ERROR            for a generic service channel error
 *          - SIII_SVC_NO_CMD_PAR_ERROR when it is tried to perform a command
 *                                      on a non-command parameter
 *
 * \note    This function is private. It shall not be called directly by an
 *          application.
 *
 * \ingroup SIII
 *
 * \author GMy
 *
 * \date 18.01.2013
 *
 * \version 2013-02-19 (GMy): Performance optimization: SVC thread now
 *                            continuously runs and does not need to be started
 *                            again for every SVC transfer. Support for list data
 *                            types.
 * \version 2013-06-12 (GMy): Bugfix / optimization: Corrected error reaction
 *                            when command error occurs.
 * \version 2016-08-22 WK:    Support parameter set of the IDN.
 * \version 2016-09-20 WK:    Check MBusy as a substitute for a sleeptime for the 
 +                            execution of the next SVC step.
 */
VOID SIII_SVCThread
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
#undef SIII_SVC_RUNTIME_MEASUREMENT  // For debug purposes only!

  CSMD_FUNC_RET eMyFuncRet      = CSMD_NO_ERROR;
  ULONG         ulSvcDataFormat = 0;
  INT           iCnt            = 0;
#ifdef SIII_SVC_RUNTIME_MEASUREMENT
  RTOS_TIMESPEC   timeStart;
  RTOS_TIMESPEC   timeEnd;
  RTOS_TIMESPEC   timeDiff;
  LONG            lSIII_SVC_WorkingTimeS;
  LONG            lSIII_SVC_WorkingTimeNs;
  DOUBLE          dSIII_SVC_WorkingTime;
#endif

  SIII_VERBOSE(3, "SIII_SVCThread()\n");

  // Set own thread priority
  if (RTOS_SetThreadPriority(RTOS_THREAD_PRIORITY_SVC) != RTOS_RET_OK)
  {
    SIII_VERBOSE(0, "Error: Failed to set priority of SVC thread.\n");
    exit(-1);
  }

  RTOS_AllowRemoteClose();

  /*lint -save -e716 */
  while (TRUE)
  /*lint -restore */
  {
    // Wait for started SVC request
    /*lint -save -e722 */
    while(RTOS_WaitForSemaphore(&prS3Instance->semSVCStart) != RTOS_RET_OK);
    /*lint -restore */

    prS3Instance->rMySvcMacro.usState = (USHORT) CSMD_START_REQUEST;

    switch(prS3Instance->rMySVCAccessMode) {

      case SIII_SVC_READ:

        SIII_VERBOSE
            (
              0,
              "- Starting SVC read (%c-%1u-%04lu.%lu.%lu) on device #%d...\n",
              SIII_GET_S_P(prS3Instance->rMySvcMacro.ulIdent_Nbr),
              SIII_GET_IDNSET(prS3Instance->rMySvcMacro.ulIdent_Nbr),
              SIII_GET_IDN(prS3Instance->rMySvcMacro.ulIdent_Nbr),
              SIII_GET_SI(prS3Instance->rMySvcMacro.ulIdent_Nbr),
              SIII_GET_SE(prS3Instance->rMySvcMacro.ulIdent_Nbr),
              prS3Instance->rMySvcMacro.usSlaveIdx
            );

#ifdef SIII_SVC_RUNTIME_MEASUREMENT
        // Get start time for calculation of runtime of cyclic functions
        RTOS_GetSystemTime( &timeStart );
#endif
        do {
          // Check if MBUSY is set, if it is not set do nothing
          if ( prS3Instance->rCosemaInstance.rPriv.prSVContainer[prS3Instance->rMySvcMacro.usSlaveIdx]->rCONTROL.usWord[0] & CSMD_SVC_CTRL_M_BUSY )
          {
            eMyFuncRet = CSMD_ReadSVCH
                (
                  &prS3Instance->rCosemaInstance, // CoSeMa instance
                  &prS3Instance->rMySvcMacro,     // SVC macro structure
                  NULL                            // No callback
                );

            SIII_VERBOSE
                (
                  1,
                  "- CSMD_ReadSVCH - state = %i, return value = 0x%X\n",
                  prS3Instance->rMySvcMacro.usState,
                  eMyFuncRet
                );
          }

        } while (
              (prS3Instance->rMySvcMacro.usState != (USHORT) CSMD_DATA_VALID)     &&
              (prS3Instance->rMySvcMacro.usState != (USHORT) CSMD_REQUEST_ERROR)
            );
#ifdef SIII_SVC_RUNTIME_MEASUREMENT
        // Get start time for calculation of runtime of cyclic functions
        RTOS_GetSystemTime( &timeEnd );

        RTOS_GetTimeDifference
        (
          &timeEnd,       // End time of measurement interval
          &timeStart,     // Start time of measurement interval
          &timeDiff       // Time difference (= working time)
        );

        lSIII_SVC_WorkingTimeNs = RTOS_GetTimeNs(&timeDiff);
        lSIII_SVC_WorkingTimeS  = RTOS_GetTimeS(&timeDiff);
        dSIII_SVC_WorkingTime =  (DOUBLE)(lSIII_SVC_WorkingTimeS * 1000) + (DOUBLE)(lSIII_SVC_WorkingTimeNs) / (DOUBLE)(1000*1000);

        SIII_VERBOSE
        (
          0,
          "- Time required for the SVC transmission: %.2f ms\n",
          dSIII_SVC_WorkingTime
        );
#endif

        if (prS3Instance->rMySvcMacro.usState == (USHORT) CSMD_DATA_VALID)
        {
          SIII_VERBOSE(0, "- SVC read successful.\n");

          ulSvcDataFormat = prS3Instance->rMySvcMacro.ulAttribute &
                    (ULONG) CSMD_SERC_LEN;

          // Type cast of retrieved data, depending on data type of parameter
          // \todo: OK for big endian?
          switch(ulSvcDataFormat)
          {
            case CSMD_SERC_WORD_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 16 bit value\n"
                  );
              /*lint -save -e740 cast! */
              SIII_VERBOSE
                  (
                    0,
                    "- Retrieved data: %hu, 0x%04hX\n",
                    *((USHORT*)prS3Instance->rMySVCResult.ausSVCData),
                    *((USHORT*)prS3Instance->rMySVCResult.ausSVCData)
                  );
              /*lint -restore cast! */
              break;
            case CSMD_SERC_LONG_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 32 bit value\n"
                  );
              /*lint -save -e740 cast! */
              SIII_VERBOSE
                  (
                    0,
                    "- Retrieved data: %lu, 0x%08lX\n",
                    *((ULONG*)prS3Instance->rMySVCResult.ausSVCData),
                    *((ULONG*)prS3Instance->rMySVCResult.ausSVCData)
                  );
              /*lint -restore cast! */
              break;
            case CSMD_SERC_DOUBLE_LEN:
              SIII_VERBOSE(   0,
                      "- Data type: 64 bit value\n");
              /*lint -save -e740 cast! */
              SIII_VERBOSE
                  (
                    0,
                    "- Retrieved data: %llu, 0x%016llX\n",
                    *((ULONGLONG*)prS3Instance->rMySVCResult.ausSVCData),
                    *((ULONGLONG*)prS3Instance->rMySVCResult.ausSVCData)
                  );
              /*lint -restore cast! */
              break;
            case CSMD_SERC_VAR_BYTE_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 8 bit value list / string with %hu elements\n",
                    prS3Instance->rMySVCResult.ausSVCData[0]
                  );
              SIII_VERBOSE(0, "- Retrieved data:\n");
              /*lint -save -e740 cast! */
              for (   
                  iCnt = 0;
                  iCnt < prS3Instance->rMySVCResult.ausSVCData[0];
                  iCnt++
                )
              {
                SIII_VERBOSE
                    (
                      0,
                      "%c",
                      (((CHAR*)(((UCHAR*)prS3Instance->rMySVCResult.ausSVCData)+4)))[iCnt]
                    );
              }
              SIII_VERBOSE(0, "\n");
              for (   
                  iCnt = 0;
                  iCnt < prS3Instance->rMySVCResult.ausSVCData[0];
                  iCnt++
                )
              {
                SIII_VERBOSE
                    (
                      0,
                      "0x%02hhX ",
                      (((CHAR*)(((UCHAR*)prS3Instance->rMySVCResult.ausSVCData)+4)))[iCnt]
                    );
              }
              /*lint -restore cast! */
              SIII_VERBOSE(0, "\n");
              break;
            case CSMD_SERC_VAR_WORD_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 16 bit value list with %hu elements\n",
                    prS3Instance->rMySVCResult.ausSVCData[0] / 2
                  );
              SIII_VERBOSE(0, "- Retrieved data:\n");
              for (   
                  iCnt = 0;
                  iCnt < (prS3Instance->rMySVCResult.ausSVCData[0] / 2);
                  iCnt++
                )
              {
                /*lint -save -e740 cast! */
                SIII_VERBOSE
                  (
                    0,
                    "0x%04hX ",
                    (((USHORT*)(((UCHAR*)prS3Instance->rMySVCResult.ausSVCData)+4)))[iCnt]
                  );
                /*lint -restore cast! */
              }
              SIII_VERBOSE(0, "\n");
              break;
            case CSMD_SERC_VAR_LONG_LEN:
              SIII_VERBOSE
                  (   0,
                    "- Data type: 32 bit value list with %hu elements\n",
                    prS3Instance->rMySVCResult.ausSVCData[0] / 4
                  );
              SIII_VERBOSE(0, "- Retrieved data:\n");
              for (   iCnt = 0;
                  iCnt < (prS3Instance->rMySVCResult.ausSVCData[0] / 4);
                  iCnt++)
              {
                /*lint -save -e740 cast! */
                SIII_VERBOSE
                    (
                      0,
                      "0x%08lX ",
                      (((ULONG*)(((UCHAR*)prS3Instance->rMySVCResult.ausSVCData)+4)))[iCnt]
                    );
                /*lint -restore cast! */
                if (((iCnt+1) % 4)  == 0)
                {
                  SIII_VERBOSE( 0, "\n");
                }
              }
              SIII_VERBOSE(0, "\n");
              break;
            case CSMD_SERC_VAR_DOUBLE_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 64 bit value list with %hu elements\n",
                    prS3Instance->rMySVCResult.ausSVCData[0] / 8
                  );
              SIII_VERBOSE(0, "- Retrieved data:\n");
              for (   iCnt = 0;
                  iCnt < (prS3Instance->rMySVCResult.ausSVCData[0] / 8);
                  iCnt++)
              {
                /*lint -save -e740 cast! */
                SIII_VERBOSE
                    (
                      0,
                      "0x%16llx ",
                      (((ULONGLONG*)(((UCHAR*)prS3Instance->rMySVCResult.ausSVCData)+4)))[iCnt]
                    );
                /*lint -restore cast! */
              }
              SIII_VERBOSE(0, "\n");
              break;
            default:
              SIII_VERBOSE(0, "- Unknown data format, unable to display data.\n");
              break;
          }
          prS3Instance->rMySVCResult.eFuncRet = SIII_NO_ERROR;
        }
        else
        {
          SIII_VERBOSE
              (
                0,
                "- Error: SVC read not successful. Error: 0x%hX\n",
                prS3Instance->rMySvcMacro.usSvchError
              );
          prS3Instance->rMySVCResult.eFuncRet = SIII_SVC_ERROR;
        }

        break;

      case SIII_SVC_WRITE:
        SIII_VERBOSE
            (
              0,
              "- Starting SVC write (%c-%1u-%04lu.%lu.%lu) on device #%d...\n",
              SIII_GET_S_P(prS3Instance->rMySvcMacro.ulIdent_Nbr),
              SIII_GET_IDNSET(prS3Instance->rMySvcMacro.ulIdent_Nbr),
              SIII_GET_IDN(prS3Instance->rMySvcMacro.ulIdent_Nbr),
              SIII_GET_SI(prS3Instance->rMySvcMacro.ulIdent_Nbr),
              SIII_GET_SE(prS3Instance->rMySvcMacro.ulIdent_Nbr),
              prS3Instance->rMySvcMacro.usSlaveIdx
            );

        do {
          // Check if MBUSY is set, if it is not set do nothing
          if ( prS3Instance->rCosemaInstance.rPriv.prSVContainer[prS3Instance->rMySvcMacro.usSlaveIdx]->rCONTROL.usWord[0] & CSMD_SVC_CTRL_M_BUSY )
          {
            eMyFuncRet = CSMD_WriteSVCH
                (
                  &prS3Instance->rCosemaInstance, // CoSeMa instance
                  &prS3Instance->rMySvcMacro,     // SVC macro structure
                  NULL                            // No callback
                );

            SIII_VERBOSE
                (
                  1,
                  "- CSMD_WriteSVCH - state = %i, return value = 0x%X\n",
                  prS3Instance->rMySvcMacro.usState,
                  eMyFuncRet
                );
          }

        } while (
              (prS3Instance->rMySvcMacro.usState != CSMD_DATA_VALID)    &&
              (prS3Instance->rMySvcMacro.usState != CSMD_REQUEST_ERROR)
            );

        if (prS3Instance->rMySvcMacro.usState == CSMD_DATA_VALID)
        {
          SIII_VERBOSE(0, "- SVC write successful.\n");

/*          // Display written data
          ulSvcDataFormat = prS3Instance->rMySvcMacro.ulAttribute &
                              (ULONG) CSMD_SERC_LEN;

          // Type cast of retrieved data, depending on data type of parameter
          // \todo: OK for big endian?
          switch(ulSvcDataFormat)
          {
            case CSMD_SERC_WORD_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 16 bit value\n"
                  );
              //lint -save -e740 cast!
              SIII_VERBOSE
                  (
                    0,
                    "- Wrote data: %hu, 0x%04hX\n",
                    *((USHORT*)prS3Instance->rMySVCResult.ausSVCData),
                    *((USHORT*)prS3Instance->rMySVCResult.ausSVCData)
                  );
              //lint -restore cast!
              break;
            case CSMD_SERC_LONG_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 32 bit value\n"
                  );
              //lint -save -e740 cast!
              SIII_VERBOSE
                  (
                    0,
                    "- Wrote data: %lu, 0x%08lX\n",
                    *((ULONG*)prS3Instance->rMySVCResult.ausSVCData),
                    *((ULONG*)prS3Instance->rMySVCResult.ausSVCData)
                  );
              //lint -restore cast!
              break;
            case CSMD_SERC_DOUBLE_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 64 bit value\n"
                  );
              //lint -save -e740 cast!
              SIII_VERBOSE
                  (
                    0,
                    "- Wrote data: %llu, 0x%016llX\n",
                    *((ULONGLONG*)prS3Instance->rMySVCResult.ausSVCData),
                    *((ULONGLONG*)prS3Instance->rMySVCResult.ausSVCData)
                  );
              //lint -restore cast!
              break;
            case CSMD_SERC_VAR_BYTE_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 8 bit value list / string with %hu elements\n",
                    prS3Instance->rMySVCResult.ausSVCData[0]
                  );
              SIII_VERBOSE(0, "- Retrieved data:\n");
              //lint -save -e740 cast!
              for ( 
                  iCnt = 0;
                  iCnt < prS3Instance->rMySVCResult.ausSVCData[0];
                  iCnt++
                )
              {
                SIII_VERBOSE
                    (
                      0,
                      "%c",
                      (((CHAR*)(((UCHAR*)prS3Instance->rMySVCResult.ausSVCData)+4)))[iCnt]
                    );
              }
              SIII_VERBOSE(0, "\n");
              for (   
                  iCnt = 0;
                  iCnt < prS3Instance->rMySVCResult.ausSVCData[0];
                  iCnt++
                )
              {
                SIII_VERBOSE
                    (
                      0,
                      "0x%02hhX ",
                      (((CHAR*)(((UCHAR*)prS3Instance->rMySVCResult.ausSVCData)+4)))[iCnt]
                    );
              }
              //lint -restore cast!
              SIII_VERBOSE(0, "\n");
              break;
            case CSMD_SERC_VAR_WORD_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 16 bit value list with %hu elements\n",
                    prS3Instance->rMySVCResult.ausSVCData[0] / 2
                  );
              SIII_VERBOSE(0, "- Wrote data:\n");
              for (   
                  iCnt = 0;
                  iCnt < (prS3Instance->rMySVCResult.ausSVCData[0] / 2);
                  iCnt++
                )
              {
                //lint -save -e740 cast!
                SIII_VERBOSE
                    (
                      0,
                      "0x%04hX ",
                      (((USHORT*)(((UCHAR*)prS3Instance->rMySVCResult.ausSVCData)+4)))[iCnt]
                    );
                //lint -restore cast!
              }
              SIII_VERBOSE(0, "\n");
              break;
            case CSMD_SERC_VAR_LONG_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 32 bit value list with %hu elements\n",
                    prS3Instance->rMySVCResult.ausSVCData[0] / 4
                  );
              SIII_VERBOSE(0, "- Wrote data:\n");
              for (   
                  iCnt = 0;
                  iCnt < (prS3Instance->rMySVCResult.ausSVCData[0] / 4);
                  iCnt++
                )
              {
                //lint -save -e740 cast!
                SIII_VERBOSE
                    (
                      0,
                      "0x%08lX ",
                      (((ULONG*)(((UCHAR*)prS3Instance->rMySVCResult.ausSVCData)+4)))[iCnt]
                    );
                //lint -restore cast!
                if (((iCnt+1) % 4)  == 0)
                {
                  SIII_VERBOSE( 0, "\n");
                }
              }
              SIII_VERBOSE(0, "\n");
              break;
            case CSMD_SERC_VAR_DOUBLE_LEN:
              SIII_VERBOSE
                  (
                    0,
                    "- Data type: 64 bit value list with %hu elements\n",
                    prS3Instance->rMySVCResult.ausSVCData[0] / 8
                  );
              SIII_VERBOSE(0, "- Wrote data:\n");
              for (   
                  iCnt = 0;
                  iCnt < (prS3Instance->rMySVCResult.ausSVCData[0] / 8);
                  iCnt++
                )
              {
                //lint -save -e740 cast!
                SIII_VERBOSE
                    (
                      0,
                      "0x%16llx ",
                      (((ULONGLONG*)(((UCHAR*)prS3Instance->rMySVCResult.ausSVCData)+4)))[iCnt]
                    );
                //lint -restore cast!
              }
              SIII_VERBOSE(0, "\n");
              break;
            default:
              SIII_VERBOSE(0, "- Unknown data format, unable to display data.\n");
              break;
          }*/
          prS3Instance->rMySVCResult.eFuncRet = SIII_NO_ERROR;
        }
        else
        {
          SIII_VERBOSE
              (
                0,
                "- Error: SVC write not successful. Error: 0x%hX\n",
                prS3Instance->rMySvcMacro.usSvchError
              );
          prS3Instance->rMySVCResult.eFuncRet = SIII_SVC_ERROR;
        }

        break;

      case SIII_SVC_CMD:

        SIII_VERBOSE
            (
              1,
              "- Read parameter attribute to check whether it"
              "really is a command.\n"
            );

        prS3Instance->rMySvcMacro.usState   = (USHORT) CSMD_START_REQUEST;
        prS3Instance->rMySvcMacro.usElem  = (USHORT) 3;
                                  // Element 3: Attribute
        do {
          // Check if MBUSY is set, if it is not set do nothing
          if ( prS3Instance->rCosemaInstance.rPriv.prSVContainer[prS3Instance->rMySvcMacro.usSlaveIdx]->rCONTROL.usWord[0] & CSMD_SVC_CTRL_M_BUSY )
          {
            eMyFuncRet = CSMD_ReadSVCH
              (
                &prS3Instance->rCosemaInstance,   // CoSeMa instance
                &prS3Instance->rMySvcMacro,       // SVC macro structure
                NULL                              // No callback
              );

            SIII_VERBOSE
              (
                1,
                "- CSMD_ReadSVCH - state = %i, return value = 0x%X\n",
                prS3Instance->rMySvcMacro.usState,
                eMyFuncRet
              );
          }
        } while (
              (prS3Instance->rMySvcMacro.usState != (USHORT) CSMD_DATA_VALID)     &&
              (prS3Instance->rMySvcMacro.usState != (USHORT) CSMD_REQUEST_ERROR)
            );

        if (prS3Instance->rMySvcMacro.usState != (USHORT) CSMD_DATA_VALID)
        {
          SIII_VERBOSE(0, "- Error reading parameter attribute.\n");
          prS3Instance->rMySVCResult.eFuncRet = SIII_SVC_ERROR;
        }
        else if (
              (
                (*((ULONG*)prS3Instance->rMySvcMacro.pusAct_Data)) &
                    CSMD_SERC_PROC_CMD
              ) == 0
            )
        {
          SIII_VERBOSE(0, "- Error: Parameter is not a command.\n");
          prS3Instance->rMySVCResult.eFuncRet = SIII_SVC_NO_CMD_PAR_ERROR;
        }
        else
        {
          SIII_VERBOSE
              (
                0,
                "- Starting SVC command (%c-%1u-%04lu.%lu.%lu) on device #%d...\n",
                SIII_GET_S_P(prS3Instance->rMySvcMacro.ulIdent_Nbr),
                SIII_GET_IDNSET(prS3Instance->rMySvcMacro.ulIdent_Nbr),
                SIII_GET_IDN(prS3Instance->rMySvcMacro.ulIdent_Nbr),
                SIII_GET_SI(prS3Instance->rMySvcMacro.ulIdent_Nbr),
                SIII_GET_SE(prS3Instance->rMySvcMacro.ulIdent_Nbr),
                prS3Instance->rMySvcMacro.usSlaveIdx
              );

          prS3Instance->rMySvcMacro.usState = (USHORT) CSMD_START_REQUEST;
          prS3Instance->rMySvcMacro.usElem  = (USHORT) 7;
                                    // Element 7: Operation data

          // Set command
          do {
            // Check if MBUSY is set, if it is not set do nothing
            if ( prS3Instance->rCosemaInstance.rPriv.prSVContainer[prS3Instance->rMySvcMacro.usSlaveIdx]->rCONTROL.usWord[0] & CSMD_SVC_CTRL_M_BUSY )
            {
              eMyFuncRet = CSMD_SetCommand
                (
                  &prS3Instance->rCosemaInstance, // CoSeMa instance
                  &prS3Instance->rMySvcMacro,     // SVC macro structure
                  NULL                            // No callback
                );

              SIII_VERBOSE
                (
                  1,
                  "- CSMD_SetCommand - state = %i, return value = 0x%X\n",
                  prS3Instance->rMySvcMacro.usState,
                  eMyFuncRet
                );
            }
          } while (
                (prS3Instance->rMySvcMacro.usState  !=  CSMD_CMD_ACTIVE)  &&
                (prS3Instance->rMySvcMacro.usState  !=  CSMD_REQUEST_ERROR)
              );

          if (prS3Instance->rMySvcMacro.usState == CSMD_REQUEST_ERROR)
          {
            SIII_VERBOSE(0, "- Error: SVC set command not successful.\n");
            prS3Instance->rMySVCResult.eFuncRet = SIII_SVC_ERROR;
          }
          else
          {
            SIII_VERBOSE(0, "- SVC command active...\n");

            // Wait for command completion, quick-and-dirty
            //RTOS_SimpleMicroWait(SIII_SVC_CMD_WAIT_TIME);

            // Wait for command completion, 'clean' version

            do {
              prS3Instance->rMySvcMacro.usState   = (USHORT) CSMD_START_REQUEST;
              prS3Instance->rMySvcMacro.usLength  = (USHORT) 4;     // 4 Byte value
              prS3Instance->rMySvcMacro.usElem    = (USHORT) 1;
                                          // Element 1: IDN
              do {
                // Check if MBUSY is set, if it is not set do nothing
                if ( prS3Instance->rCosemaInstance.rPriv.prSVContainer[prS3Instance->rMySvcMacro.usSlaveIdx]->rCONTROL.usWord[0] & CSMD_SVC_CTRL_M_BUSY )
                {
                  eMyFuncRet = CSMD_ReadCmdStatus
                    (
                      &prS3Instance->rCosemaInstance, // CoSeMa instance
                      &prS3Instance->rMySvcMacro,     // SVC macro structure
                      NULL                            // No callback
                    );

                  SIII_VERBOSE
                    (
                      1,
                      "- CSMD_ReadCmdStatus - state = %i, return value = 0x%X\n",
                      prS3Instance->rMySvcMacro.usState,
                      eMyFuncRet
                    );
                }
              } while (
                    (prS3Instance->rMySvcMacro.usState != CSMD_CMD_STATUS_VALID) &&
                    (prS3Instance->rMySvcMacro.usState != CSMD_REQUEST_ERROR)
                  );              // while CSMD_ReadCmdStatus() is in process

            } while (
                  (prS3Instance->rMySVCResult.ausSVCData[0] != CSMD_CMD_FINISHED)   &&
                  (prS3Instance->rMySVCResult.ausSVCData[0] != CSMD_CMD_STOPPED)    &&
                  (prS3Instance->rMySVCResult.ausSVCData[0] != CSMD_CMD_ERROR)      &&
                  (prS3Instance->rMySvcMacro.usState        != CSMD_REQUEST_ERROR)
                );                // while CSMD_ReadCmdStatus() is not done or error

            if (prS3Instance->rMySvcMacro.usState   != CSMD_REQUEST_ERROR)
            {
              SIII_VERBOSE(0, "- SVC command done.\n");
            }
            else
            {
              SIII_VERBOSE(0, "- Error: SVC command not successfully completed.\n");
            }

            // Clear command
            prS3Instance->rMySvcMacro.usState  = (USHORT) CSMD_START_REQUEST;
            prS3Instance->rMySvcMacro.usLength = (USHORT) 0;
            prS3Instance->rMySvcMacro.usElem   = (USHORT) 7;
                                // Element 7: Operation data

            do {
              // Check if MBUSY is set, if it is not set do nothing
              if ( prS3Instance->rCosemaInstance.rPriv.prSVContainer[prS3Instance->rMySvcMacro.usSlaveIdx]->rCONTROL.usWord[0] & CSMD_SVC_CTRL_M_BUSY )
              {
                eMyFuncRet = CSMD_ClearCommand
                  (
                    &prS3Instance->rCosemaInstance,
                    &prS3Instance->rMySvcMacro,
                    NULL
                  );

                SIII_VERBOSE
                  (
                    1,
                    "- CSMD_ClearCommand - state = %i, return value = 0x%X\n",
                    prS3Instance->rMySvcMacro.usState,
                    eMyFuncRet
                  );
              }
            } while (
                  (prS3Instance->rMySvcMacro.usState != CSMD_CMD_CLEARED) &&
                  (prS3Instance->rMySvcMacro.usState != CSMD_REQUEST_ERROR)
                );

            if (prS3Instance->rMySvcMacro.usState == CSMD_CMD_CLEARED)
            {
              SIII_VERBOSE(0, "- SVC command cleared.\n");
              prS3Instance->rMySVCResult.eFuncRet = SIII_NO_ERROR;
            }
            else
            {
              SIII_VERBOSE(0, "- Error: SVC clear command not successful.\n");
              prS3Instance->rMySVCResult.eFuncRet = SIII_SVC_ERROR;
            }
          }   // if command activation successful
        }   // if parameter is a command
        break;

      default:
        SIII_VERBOSE(0, "- Error: Illegal SVC transaction\n");
        prS3Instance->rMySVCResult.eFuncRet = SIII_SVC_ERROR;
        break;

    }   // switch(rMySVCAccessMode)

    // Remove blocking of SVC
    (VOID) RTOS_PostSemaphore(&prS3Instance->semSVCBlock);

  }   // while(TRUE)
}

/**
 * \fn SIII_FUNC_RET SIII_SVCWrite(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT  usDevIdx,
 *              BOOL    boIsStdPar,
 *              USHORT  usIdn,
 *              USHORT  usSI,
 *              USHORT  usSE,
 *              USHORT  usElem,
 *              USHORT* pusValue,
 *              USHORT  usLen
 *          )
 *
 * \public
 *
 * \brief   This function writes an IDN to a Sercos slave via the Sercos
 *          service channel. Also supports lists if pusValue contains a Sercos
 *          list.
 *
 * \note    The Sercos phase needs to be CP2 or higher. The function is
 *          blocking.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       usDevIdx        Slave device index
 * \param[in]       boIsStdPar      Indicates whether a standard (S) parameter
 *                                  - TRUE for standard (S) parameter
 *                                  - FALSE for device-specific (P) parameter
 * \param[in]       usIdn           IDN of parameter
 * \param[in]       usSI            Structural instance of parameter
 * \param[in]       usSE            Structural element of parameter
 * \param[in]       usElem          Element of parameter
 * \param[in]       pusValue        Pointer to data buffer
 * \param[in]       usLen           Length of buffer pusValue in bytes
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR         for success
 *          - SIII_SVC_PHASE_ERROR  when phase is not CP2 or higher
 *          - SIII_DEVICE_IDX_ERROR for illegal device index
 *          - Inherited error from SVC access otherwise
 *
 * \ingroup SIII
 *
 * \author GMy
 *
 * \date 2013-03-05
 *
 * \version 2013-04-16 (GMy): Added support for elements different from
 *                            operation data.
 * \version 2016-09-22 (WK):  Added check for invalid slave index.
 */
SIII_FUNC_RET SIII_SVCWrite
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT usDevIdx,
      BOOL boIsStdPar,
      USHORT usIdn,
      USHORT usSI,
      USHORT usSE,
      USHORT usElem,
      USHORT* pusValue,
      USHORT  usLen
    )
{
  SIII_VERBOSE(3, "SIII_SVCWrite()\n");

  if (SIII_GetSercosPhase(prS3Instance) < SIII_PHASE_CP2)
  {
    return(SIII_SVC_PHASE_ERROR);
  }
  else if (
        (prS3Instance == NULL)            ||
         usDevIdx >= prS3Instance->rCosemaInstance.rSlaveList.usNumProjSlaves
      )
  {
    return(SIII_DEVICE_IDX_ERROR);
  }
  else
  {
    // Wait for SVC semaphore
    /*lint -save -e722 */
    while(RTOS_WaitForSemaphore(&prS3Instance->semSVCBlock) != RTOS_RET_OK);
    /*lint -restore */

    prS3Instance->rMySVCResult.eFuncRet   = SIII_FUNCTION_IN_PROCESS;

    // Set SVC access data structures
    prS3Instance->rMySVCAccessMode        = SIII_SVC_WRITE;
    prS3Instance->rMySvcMacro.pusAct_Data = pusValue;
    prS3Instance->rMySvcMacro.usSlaveIdx  = usDevIdx;
    prS3Instance->rMySvcMacro.usElem      = usElem;
    prS3Instance->rMySvcMacro.ulIdent_Nbr = 
        CSMD_EIDN
            (
              usIdn,
              usSI,
              usSE
            );

    if (!boIsStdPar)
    {
      // Device-specific IDN (P parameter)
      prS3Instance->rMySvcMacro.ulIdent_Nbr |= ((ULONG) 1) << ((ULONG) 15);
    }

    prS3Instance->rMySvcMacro.usIsList               = (USHORT)  0;  // auto-detected
    prS3Instance->rMySvcMacro.usLength               = (USHORT)  0;  // auto-detected
    prS3Instance->rMySvcMacro.ulAttribute            = (ULONG)   0;
    prS3Instance->rMySvcMacro.usCancelActTrans       = (USHORT)  0;
    prS3Instance->rMySvcMacro.usPriority             = (USHORT)  0;
    prS3Instance->rMySvcMacro.usOtherRequestCanceled = (USHORT)  0;
    prS3Instance->rMySvcMacro.usSvchError            = (USHORT)  0;
    prS3Instance->rMySvcMacro.usInternalReq          = (USHORT)  0;
    prS3Instance->rMySvcMacro.usState                = (USHORT)  CSMD_START_REQUEST;

    // Start SVC transfer
    (VOID)RTOS_PostSemaphore(&prS3Instance->semSVCStart);

    // Wait until transfer done.
    do
    {
      // TODO: Check whether the value can be reduced or replaced by another mechanism
      RTOS_SimpleMicroWait(SIII_SVC_CMD_WAIT_TIME);
    }
    while (prS3Instance->rMySVCResult.eFuncRet == SIII_FUNCTION_IN_PROCESS);

    return(prS3Instance->rMySVCResult.eFuncRet);
  }
}

/**
 * \fn SIII_FUNC_RET SIII_SVCRead(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT usDevIdx,
 *              BOOL boIsStdPar,
 *              USHORT usIdn,
 *              USHORT usSI,
 *              USHORT usSE,
 *              USHORT  usElem,
 *              USHORT* pusValue,
 *              USHORT usLen
 *          )
 *
 * \public
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       usDevIdx        Slave device index
 * \param[in]       boIsStdPar      Indicates whether a standard (S) parameter
 *                                  - TRUE for standard (S) parameter
 *                                  - FALSE for device-specific (P) parameter
 * \param[in]       usIdn           IDN of parameter
 * \param[in]       usSI            Structural instance of parameter
 * \param[in]       usSE            Structural element of parameter
 * \param[in]       usElem          Element of parameter
 * \param[out]      pusValue        Pointer to data buffer
 * \param[in]       usLen           Length of buffer (pusValue) in bytes
 *
 * \brief   This function reads an IDN from a Sercos slave via the Sercos
 *          service channel.
 *
 * \note    The Sercos phase needs to be CP2 or higher. The function is
 *          blocking.
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR         for success
 *          - SIII_SVC_PHASE_ERROR, when phase is not CP2 or higher
 *          - SIII_DEVICE_IDX_ERROR for illegal device index
 *          - Inherited error from SVC access otherwise
 *
 * \ingroup SIII
 *
 * \author GMy
 *
 * \date 2013-03-05
 *
 * \version 2013-04-16 (GMy): Added support for elements different from
 *                            operation data.
 * \version 2016-09-22 (WK):  Added check for invalid slave index.
 */
SIII_FUNC_RET SIII_SVCRead
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT  usDevIdx,
      BOOL  boIsStdPar,
      USHORT  usIdn,
      USHORT  usSI,
      USHORT  usSE,
      USHORT  usElem,
      USHORT* pusValue,
      USHORT  usLen
    )
{
  SIII_VERBOSE(3, "SIII_SVCRead()\n");

  if (SIII_GetSercosPhase(prS3Instance) < SIII_PHASE_CP2)
  {
    return(SIII_SVC_PHASE_ERROR);
  }
  else if (
        (prS3Instance == NULL)             ||
         usDevIdx >= prS3Instance->rCosemaInstance.rSlaveList.usNumProjSlaves
      )
  {
    return(SIII_DEVICE_IDX_ERROR);
  }
  else
  {
    // Wait for SVC semaphore
    /*lint -save -e722 */
    while(RTOS_WaitForSemaphore(&prS3Instance->semSVCBlock) != RTOS_RET_OK);
    /*lint -restore */

    prS3Instance->rMySVCResult.eFuncRet   = SIII_FUNCTION_IN_PROCESS;

    // Set SVC access data structures
    prS3Instance->rMySVCAccessMode        = SIII_SVC_READ;
    prS3Instance->rMySvcMacro.pusAct_Data = pusValue;
    prS3Instance->rMySvcMacro.usSlaveIdx  = usDevIdx;
    prS3Instance->rMySvcMacro.usElem      = usElem;
    prS3Instance->rMySvcMacro.ulIdent_Nbr = 
        CSMD_EIDN
            (
              usIdn,
              usSI,
              usSE
            );

    if (!boIsStdPar)
    {
      // Device-specific IDN (P parameter)
      prS3Instance->rMySvcMacro.ulIdent_Nbr |= ((ULONG) 1) << ((ULONG) 15);
    }

    prS3Instance->rMySvcMacro.usIsList               = (USHORT) 3;
                                            // Automatic detection if list or single parameter
    prS3Instance->rMySvcMacro.usLength               = (USHORT)  0;
    prS3Instance->rMySvcMacro.ulAttribute            = (ULONG)   0;
    prS3Instance->rMySvcMacro.usCancelActTrans       = (USHORT)  0;
    prS3Instance->rMySvcMacro.usPriority             = (USHORT)  0;
    prS3Instance->rMySvcMacro.usOtherRequestCanceled = (USHORT)  0;
    prS3Instance->rMySvcMacro.usSvchError            = (USHORT)  0;
    prS3Instance->rMySvcMacro.usInternalReq          = (USHORT)  0;
    prS3Instance->rMySvcMacro.usState                = (USHORT)  CSMD_START_REQUEST;

    // Start SVC transfer
    (VOID)RTOS_PostSemaphore(&prS3Instance->semSVCStart);

    // Wait until transfer done.
    do
    {
      // TODO: Check whether the value can be reduced or replaced by another mechanism
      RTOS_SimpleMicroWait(SIII_SVC_CMD_WAIT_TIME);
    }
    while (prS3Instance->rMySVCResult.eFuncRet == SIII_FUNCTION_IN_PROCESS);

    return(prS3Instance->rMySVCResult.eFuncRet);
  }
}

/**
 * \fn SIII_FUNC_RET SIII_SVCCmd(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT usDevIdx,
 *              BOOL boIsStdPar,
 *              USHORT usIdn,
 *              USHORT usSE,
 *              USHORT usSI
 *          )
 *
 * \public
 *
 * \brief This function performs a command on Sercos slave via service channel.
 *
 * \note The Sercos phase needs to be CP2 or higher. The function is blocking.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       usDevIdx        Slave device index
 * \param[in]       boIsStdPar      Indicates whether a standard (S) parameter
 *                                  - TRUE for standard (S) parameter
 *                                  - FALSE for device-specific (P) parameter
 * \param[in]       usIdn           IDN of parameter
 * \param[in]       usSI            Structural instance of parameter
 * \param[in]       usSE            Structural element of parameter
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR         for success
 *          - SIII_SVC_PHASE_ERROR, when phase is not CP2 or higher
 *          - SIII_DEVICE_IDX_ERROR for illegal device index
 *          - Inherited error from SVC access otherwise
 *
 * \ingroup SIII
 *
 * \author GMy
 *
 * \date 2013-03-05
 *
 * \version 2016-09-22 (WK):  Added check for invalid slave index.
 *
 */
SIII_FUNC_RET SIII_SVCCmd
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT  usDevIdx,
      BOOL  boIsStdPar,
      USHORT  usIdn,
      USHORT  usSI,
      USHORT  usSE
    )
{
  SIII_VERBOSE(3, "SIII_SVCCmd()\n");

  if (SIII_GetSercosPhase(prS3Instance) < SIII_PHASE_CP2)
  {
    return(SIII_SVC_PHASE_ERROR);
  }
  else if (
        (prS3Instance == NULL)             ||
         usDevIdx >= prS3Instance->rCosemaInstance.rSlaveList.usNumProjSlaves
      )
  {
    return(SIII_DEVICE_IDX_ERROR);
  }
  else
  {
    // Wait for SVC semaphore
    /*lint -save -e722 */
    while(RTOS_WaitForSemaphore(&prS3Instance->semSVCBlock)!= RTOS_RET_OK);
    /*lint -restore */

    prS3Instance->rMySVCResult.eFuncRet   = SIII_FUNCTION_IN_PROCESS;

    // Set SVC access data structures
    prS3Instance->rMySVCAccessMode        = SIII_SVC_CMD;
    prS3Instance->rMySvcMacro.pusAct_Data = prS3Instance->rMySVCResult.ausSVCData;
    prS3Instance->rMySvcMacro.usSlaveIdx  = usDevIdx;
    prS3Instance->rMySvcMacro.usElem      = (USHORT) 7;
                              // Element 7: Operation data
    prS3Instance->rMySvcMacro.ulIdent_Nbr =
        CSMD_EIDN
          (
            usIdn,
            usSI,
            usSE
          );

    if (!boIsStdPar)
    {
      // Device-specific IDN (P parameter)
      prS3Instance->rMySvcMacro.ulIdent_Nbr |= ((ULONG) 1) << ((ULONG) 15);
    }

    prS3Instance->rMySvcMacro.usIsList               = (USHORT)  0;
    prS3Instance->rMySvcMacro.usLength               = (USHORT)  0;
    prS3Instance->rMySvcMacro.ulAttribute            = (ULONG)   0;
    prS3Instance->rMySvcMacro.usCancelActTrans       = (USHORT)  0;
    prS3Instance->rMySvcMacro.usPriority             = (USHORT)  0;
    prS3Instance->rMySvcMacro.usOtherRequestCanceled = (USHORT)  0;
    prS3Instance->rMySvcMacro.usSvchError            = (USHORT)  0;
    prS3Instance->rMySvcMacro.usInternalReq          = (USHORT)  0;
    prS3Instance->rMySvcMacro.usState                = (USHORT)  CSMD_START_REQUEST;

    // Start SVC transfer
    (VOID)RTOS_PostSemaphore(&prS3Instance->semSVCStart);

    // Wait until transfer done.
    do
    {
      // TODO: Check whether the value can be reduced or replaced by another mechanism
      RTOS_SimpleMicroWait(SIII_SVC_CMD_WAIT_TIME);
    }
    while (prS3Instance->rMySVCResult.eFuncRet == SIII_FUNCTION_IN_PROCESS);

    return(prS3Instance->rMySVCResult.eFuncRet);
  }
}

/**
 * \fn SIII_FUNC_RET SIII_SVCClearErrors(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT usDevIdx
 *          )
 *
 * \public
 *
 * \brief   This function clears present errors on Sercos slave devices. The
 *          function is blocking.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       usDevIdx        Slave device index or 'SIII_ALL_DEVICES'.
 *
 * \note    The 'SIII_ALL_DEVICES' option does not work properly when hotplug
 *          slaves are configured but not (yet) present.
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR         for success
 *          - SIII_SVC_PHASE_ERROR  when phase is not CP2 or higher
 *          - SIII_DEVICE_IDX_ERROR for illegal device index
 *          - Inherited error from SVC access function SIII_SVCCmd() otherwise
 *
 * \ingroup SIII
 *
 * \author GMy
 *
 * \date 2013-04-11
 *
 */
SIII_FUNC_RET SIII_SVCClearErrors
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT  usDevIdx
    )
{
  INT           iCnt       = 0;
  SIII_FUNC_RET eS3FuncRet = SIII_NO_ERROR;

  SIII_VERBOSE(3, "SIII_SVCClearErrors()\n");

  if (
      (prS3Instance   ==  NULL)         ||
      (
        (usDevIdx >= SIII_MAX_SLAVES)   &&
        (usDevIdx != SIII_ALL_DEVICES)
      )
    )
  {
    return(SIII_DEVICE_IDX_ERROR);
  }


  if (
      (usDevIdx == (USHORT) SIII_ALL_DEVICES)       ||
        (usDevIdx < (USHORT) SIII_GetNoOfSlaves(prS3Instance))
     )
  {
    if (SIII_GetSercosPhase(prS3Instance) >= SIII_PHASE_CP2)
    {
      if (usDevIdx == (USHORT) SIII_ALL_DEVICES)
      {
        for (
            iCnt = 0;
            iCnt < SIII_GetNoOfSlaves(prS3Instance);
            iCnt++
          )
        {
          eS3FuncRet = SIII_SVCCmd
              (
                prS3Instance, // SIII instance
                iCnt,         // Device number
                TRUE,         // S-Parameter
                99,           // S-0-99: Clear errors command
                0,            // SI = 0
                0             // SE = 0
              );

          if (eS3FuncRet != SIII_NO_ERROR)
          {
            return(eS3FuncRet);
          }
        }
      }
      else
      {
        eS3FuncRet = SIII_SVCCmd
            (
              prS3Instance,
              usDevIdx, // Device number
              TRUE,     // S-Parameter
              99,       // S-0-99: Clear errors command
              0,        // SI = 0
              0         // SE = 0
            );

        if (eS3FuncRet != SIII_NO_ERROR)
        {
          return(eS3FuncRet);
        }
      }
    }
    else
    {
      return(SIII_SVC_PHASE_ERROR);
    }
  }
  else
  {
    return(SIII_PARAMETER_ERROR);
  }
  return(SIII_NO_ERROR);
}
