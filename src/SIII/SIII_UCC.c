/**
 * \file      SIII_UCC.c
 *
 * \brief     Sercos III soft master stack - UCC handling
 *
 * \note      Currently only test functionality
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
 * \copyright Copyright Bosch Rexroth AG, 2015-2016
 *
 * \date      2015-06-11
 */

//---- includes ---------------------------------------------------------------

#include "../SIII/SIII_GLOB.h"
#include "../SIII/SIII_PRIV.h"

//---- defines ----------------------------------------------------------------

//---- type definitions -------------------------------------------------------

typedef struct
{
  UCHAR   aucDestMAC[6];    /**< Destination MAC address */
  UCHAR   aucSrcMAC[6];     /**< Source MAC address */
  USHORT  usEtherType;      /**< EtherType */
} SIII_ETH_HDR;


//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SIII_FUNC_RET SIII_UccHandling(
 *              SIII_INSTANCE_STRUCT    *prS3Instance
 *          )
 *
 * \public
 *
 * \brief   This function provides a UCC test functionality by just sending
 *          back received UCC frames using the SICE UCC interface
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:        No error
 *          - SIII_PARAMETER_ERROR: Parameter error
 *          - SIII_UCC_ERROR:       UCC handling problem
 *          - SIII_CONFIG_ERROR:    UCC not enabled, bit function called
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2015-06-11
 */
SIII_FUNC_RET SIII_UccHandling
    (
      SIII_INSTANCE_STRUCT    *prS3Instance
    )
{

#ifdef SICE_UC_CHANNEL

  SICE_FUNC_RET   rSiceRet  = SICE_NO_ERROR;
  SICE_FUNC_RET   rSiceRet2 = SICE_NO_ERROR;
  UCHAR           ucPort    = 0;
  USHORT          usLen     = 0;
  UCHAR           aucBuf[SICE_ETH_FRAMEBUF_LEN];
  UCHAR           aucDestMAC[6];
  SIII_ETH_HDR    *pEthHdr;

  SIII_VERBOSE(3, "SIII_UccHandling()\n");

  if (prS3Instance == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  // Do while packets left in queue
  do
  {
    rSiceRet = SICE_UCC_GetPacket
        (
          &prS3Instance->rSiceInstance,
          &ucPort,
          aucBuf,
          &usLen
        );

    if (rSiceRet == SICE_NO_ERROR)
    {
      pEthHdr = (SIII_ETH_HDR*)aucBuf;

      SIII_VERBOSE
          (
            0,
            "UCC packet: %luBytes, DEST=0x%.2X%.2X%.2X%.2X%.2X%.2X, "
            "SRC=0x%.2X%.2X%.2X%.2X%.2X%.2X, EtherType=0x%.4X\n",
            usLen,
            pEthHdr->aucDestMAC[0],
            pEthHdr->aucDestMAC[1],
            pEthHdr->aucDestMAC[2],
            pEthHdr->aucDestMAC[3],
            pEthHdr->aucDestMAC[4],
            pEthHdr->aucDestMAC[5],
            pEthHdr->aucSrcMAC[0],
            pEthHdr->aucSrcMAC[1],
            pEthHdr->aucSrcMAC[2],
            pEthHdr->aucSrcMAC[3],
            pEthHdr->aucSrcMAC[4],
            pEthHdr->aucSrcMAC[5],
            ntohs(pEthHdr->usEtherType)
          );

      // If IP packet, sent it back
      if (ntohs(pEthHdr->usEtherType) == 0x0800)
      {
        // Exchange destination and source address
        memcpy(aucDestMAC, pEthHdr->aucDestMAC, 6);
        memcpy(pEthHdr->aucDestMAC, pEthHdr->aucSrcMAC, 6);
        memcpy(pEthHdr->aucSrcMAC, aucDestMAC, 6);
        // pEthHdr->usEtherType = htons(0x08CE);

        // Send IP packet back

        rSiceRet2 = SICE_UCC_PutPacket
            (
              &prS3Instance->rSiceInstance,
              ucPort,
              aucBuf,
              usLen
            );

        if (rSiceRet2 != SICE_NO_ERROR)
        {
          SIII_VERBOSE(0, "Error sending UC packet\n");
        }
        else
        {
          SIII_VERBOSE
              (
                1,
                "Successfully sent back IP packet (EtherType=0x%X) with %lu Bytes\n",
                ntohs(pEthHdr->usEtherType),
                usLen
              );
        }
      }
    }
    else if (rSiceRet != SICE_NO_PACKET)
    {
      SIII_VERBOSE(0, "Error receiving UC packets");
      return(SIII_UCC_ERROR);
    }
  } while (rSiceRet != SICE_NO_PACKET);

  return(SIII_NO_ERROR);

#else
  SIII_VERBOSE(3, "Warning: SIII_UccHandling() called, but UCC disabled\n");
  return (SIII_CONFIG_ERROR);
#endif
}


