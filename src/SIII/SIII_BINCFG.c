/**
 * \file      SIII_BINCFG.c
 *
 * \brief     Sercos III soft master stack - Functions for binary connection
 *            configuration
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
 * \copyright Copyright Bosch Rexroth AG, 2013-2016
 *
 * \author    GMy
 *
 * \date      2013-08-05
 */

//---- includes ---------------------------------------------------------------

#include "../CSMD/CSMD_USER.h"
#include "../SIII/SIII_GLOB.h"
#include "../SIII/SIII_PRIV.h"

#ifdef __qnx__
#include "../RTQX/RTQX_GLOB.h"
#elif defined __unix__
#include "../RTLX/RTLX_GLOB.h"
#elif defined WINCE7
#include "../RTC7/RTC7_GLOB.h"
#elif defined WINCE
#include "../RTC6/RTC6_GLOB.h"
#elif defined __INTIME__
#include "../RTIT/RTIT_GLOB.h"
#elif defined __RTX__
#include "../RTRX/RTRX_GLOB.h"
#elif defined __VXWORKS__
#include "../RTVW/RTVW_GLOB.h"
#elif defined __KITHARA__
#include "../RTKT/RTKT_GLOB.h"
#elif defined WIN32
#include "../RTWI/RTWI_GLOB.h"
#elif defined WIN64
#include "../RTWI/RTWI_GLOB.h"
#else
#error Operating system not supported by SIII!
#endif

//---- defines ----------------------------------------------------------------

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SIII_FUNC_RET SIII_WriteBinConnCfg(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              UCHAR* pucBuffer,
 *              ULONG ulBufSize,
 *              BOOL boConnNumGen,
 *              BOOL boSlaveInstGen
 *          )
 *
 * \public
 *
 * \brief   This function writes a CoSeMa binary connection configuration
 *          (CSMD_Cfg_Bin) from a data buffer to CoSeMa.
 *
 * \details Existing data with the same application ID are overwritten. This
 *          feature allows to realize an incremental configuration, e.g. if
 *          several configuration tools are used in parallel.
 *
 * \note    The function shall not be called after phase CP2. It shall not be
 *          combined with parameter-based connection configuration.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       pucBuffer       Buffer with binary configuration data in
 *                                  CSMCfg_bin format
 * \param[in]       ulBufSize       Buffer size in Bytes
 * \param[in]       boConnNumGen    If TRUE, automatic numbering of connections
 *                                  (recommended)
 * \param[in]       boSlaveInstGen  If TRUE, automatic numbering of instances
 *                                  (recommended)
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:        For success
 *          - SIII_PARAMETER_ERROR: For function parameter error
 *
 * \ingroup SIII
 *
 * \author GMy
 *
 * \date 2013-08-05
 *
 */
SIII_FUNC_RET SIII_WriteBinConnCfg
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      UCHAR*  pucBuffer,
      ULONG ulBufSize,
      BOOL boConnNumGen,
      BOOL boSlaveInstGen
    )
{
  CSMD_FUNC_RET eCosemaFuncRet = CSMD_NO_ERROR;
  USHORT        ausCfgList[ SIII_MAX_SIZE_SERCOS_LIST +
                SIII_SIZE_SERCOS_LIST_HEADER];

  SIII_VERBOSE(3, "SIII_WriteBinConnCfg()\n");

  if (
      (prS3Instance   == NULL)  ||
      (pucBuffer    == NULL)    ||
      (ulBufSize    == 0)
    )
  {
    return(SIII_PARAMETER_ERROR);
  }

  // Change format of buffer to Sercos list
  // Not ideal, but effective

  ausCfgList[0] = (USHORT) ulBufSize;                   // Actual length
  ausCfgList[1] = (USHORT) SIII_MAX_SIZE_SERCOS_LIST;   // Max length

  (VOID)memcpy
      (
        &(ausCfgList[2]),
        pucBuffer,
        ulBufSize
      );

    eCosemaFuncRet = CSMD_ProcessBinConfig
      (
        &prS3Instance->rCosemaInstance,  // CoSeMa instance
        (VOID*) ausCfgList,              // Sercos list with binary configuration data
        boConnNumGen,                    // Automatic numbering of connections
        boSlaveInstGen                   // Automatic numbering of instances
      );

  return((SIII_FUNC_RET)eCosemaFuncRet);
}

/**
 * \fn SIII_FUNC_RET SIII_ReadBinConnCfg(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              UCHAR* pucBuffer,
 *              ULONG ulBufSize,
 *              USHORT usCFGbinVersion,
 *              USHORT usAppID,
 *              BOOL boAppID_Pos,
 *              ULONG* pulReadBytes
 *          )
 *
 * \public
 *
 * \brief   This function reads the current CoSeMa connection configuration and
 *          writes it into a buffer using the CSMDCfg_bin format.
 *
 * \details It is also possible to filter the connection data to that of
 *          certain application IDs only by applying positive or negative
 *          filtering to an application ID.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[out]      pucBuffer       Buffer for binary connection configuration
 *                                  in CSMCfg_bin format
 * \param[in]       ulBufSize       Buffer size in Bytes
 * \param[in]       usCFGbinVersion Version of CSMCfg_bin format
 * \param[in]       usAppID         Application ID
 * \param[in]       boAppID_Pos     Positive or negative filtering for
 *                                  application ID?
 * \param[out]      pulReadBytes    Number of bytes read
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:        For success
 *          - SIII_PARAMETER_ERROR: For function parameter error
 *          - SIII_BUFFER_ERROR:    Buffer too small to hold binary
 *                                  configuration
 *
 * \ingroup SIII
 *
 * \author GMy
 *
 * \date 2013-08-05
 *
 * \version 2016-09-06 (AlM): use correct index for initialization of maximum
 *                            list length for binary configuration data
  */
SIII_FUNC_RET SIII_ReadBinConnCfg
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      UCHAR*  pucBuffer,
      ULONG ulBufSize,
      USHORT usCFGbinVersion,
      USHORT usAppID,
      BOOL boAppID_Pos,
      ULONG* pulReadBytes
    )
{
  CSMD_FUNC_RET eCosemaFuncRet = CSMD_NO_ERROR;
  USHORT        ausCfgList[ SIII_MAX_SIZE_SERCOS_LIST +
                SIII_SIZE_SERCOS_LIST_HEADER] = {0};

  SIII_VERBOSE(3, "SIII_ReadBinConnCfg()\n");

  if (
      (prS3Instance == NULL)  ||
      (pucBuffer    == NULL)  ||
      (ulBufSize    == 0)
    )
  {
    return(SIII_PARAMETER_ERROR);
  }

  ausCfgList[1] = (USHORT)SIII_MAX_SIZE_SERCOS_LIST;  // Maximum length

  eCosemaFuncRet = CSMD_GenerateBinConfig
      (
        &prS3Instance->rCosemaInstance,     // CoSeMa instance
        usCFGbinVersion,                    // Scheme version of binary configuration
        usAppID,                            // Application ID
        boAppID_Pos,                        // Positive or negative filtering
        ausCfgList                          // Sercos list for binary configuration data
      );

  // Read more bytes than buffer can hold?
  // Should never be true
  if (ausCfgList[0] > ulBufSize)
  {
    return(SIII_BUFFER_ERROR);
  }

  // Change format of Sercos list to raw buffer
  // Not ideal, as inefficient, but effective
  (VOID)memcpy
      (
        pucBuffer,
        &(ausCfgList[2]),
        ausCfgList[0]
      );

  *pulReadBytes = (ULONG)ausCfgList[0];

  return((SIII_FUNC_RET)eCosemaFuncRet);
}



