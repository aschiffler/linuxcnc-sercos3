/**
 * \file      RTLX_S3SM_SOCK.c
 *
 * \brief     Real-time operating system abstraction layer for Linux RT-Preempt:
 *            Socket functions for Sercos soft master
 *
 * \attention Prototype status! Only for demo purposes! Not to be used in
 *            machines, only in controlled safe environments! Risk of unwanted
 *            machine movement!
 *
 * THIS SOFTWARE IS PROVIDED "AS IS"; WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY;
 * FITNESS FOR A PERTICULAR PURPOSE AND NONINFRINGEMENT. THE AUTHORS OR COPYRIGHT
 * HOLDERS SHALL NOT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE;
 * UNLESS STIPULATED BY MANDATORY LAW.
 *
 *
 */

//---- includes ---------------------------------------------------------------

#define SOURCE_RTLX

/*lint -save -w0 */
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <asm-generic/errno-base.h>
#include <errno.h>
/*lint -restore */

#include "../RTLX/RTLX_GLOB.h"
#include "../RTLX/RTLX_PRIV.h"
#include "../RTLX/RTLX_S3SM_GLOB.h"
#include "../RTLX/RTLX_S3SM_USER.h"
#include "../GLOB/GLOB_DEFS.h"
#include "../GLOB/GLOB_TYPE.h"
#include "../SICE/SICE_GLOB.h"

//---- defines ----------------------------------------------------------------

#define RTLX_MAX_SOCKET_INSTANCES  (1)

//---- type definitions -------------------------------------------------------

typedef struct
{
	struct sockaddr_ll rTxSocketAddress;    /**< TX socket configuration */
	INT                iRxSocketId;         /**< TX socket handle */
	INT                iTxSocketId;         /**< RX socket handle */
	CHAR               acName[IFNAMSIZ];    /**< Interface name */
} RTLX_SOCKET_INSTANCE;

//---- variable declarations --------------------------------------------------

// Structure with socket instances. Initialization of the socket names.
static RTLX_SOCKET_INSTANCE RTLX_SocketInstances[RTLX_MAX_SOCKET_INSTANCES] =
{{.acName = {"eth0"}}
};

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------
INT RTLX_Init
(
		INT iInstance
)
{
	RTLX_VERBOSE(2, "RTLX_Init()\n");
	return(RTOS_RET_OK);
}


INT RTLX_Close
(
		INT iInstance
)
{
	RTLX_VERBOSE(2, "RTLX_Close()\n");
	return(RTOS_RET_OK);
}

/**
 * \fn INT RTLX_OpenTxSocket(
 *              INT iInstanceNo,
 *              BOOL boRedundancy,
 *              UCHAR* pucMAC
 *          )
 *
 * \param[in]   iInstanceNo     Sercos IP core emulation instance number
 * \param[in]   boRedundancy    Defines whether Sercos port redundancy is used
 * \param[out]  pucMAC          MAC address
 *
 * \brief   Opens raw Ethernet transmit socket and retrieves MAC address.
 *
 * \todo    Take care of inter-frame gap. Possible outside of driver?
 *
 * \return
 * - 0: OK
 * - -1: Error
 *
 * \ingroup RTLX
 */
INT RTLX_OpenTxSocket
(
		INT iInstanceNo,
		BOOL boRedundancy,
		UCHAR* pucMAC
)
{


	struct  ifreq rIfReq;
	UCHAR   *pucTempMAC;

	if (iInstanceNo > RTLX_MAX_SOCKET_INSTANCES)
	{
		RTLX_VERBOSE
		(
				0,
				"RTLX_OpenTxSocket() instance %d too large, only %d available\n",
				iInstanceNo,
				RTLX_MAX_SOCKET_INSTANCES
		);
		return(RTOS_RET_ERROR);
	}

	// Socket descriptor
	RTLX_SocketInstances[iInstanceNo].iTxSocketId = socket
			(
					AF_PACKET,                  // Packet mode
					SOCK_RAW,                   // Raw socket
#ifdef RTOS_FILTER_SERCOS_ETHERTYPE
					htons(SICE_SIII_ETHER_TYPE) // Sercos ether type
#else
					htons(ETH_P_ALL)
#endif
			);

	if (RTLX_SocketInstances[iInstanceNo].iTxSocketId == -1)
	{
		return(RTOS_RET_ERROR);
	}
	else
	{
		// Communication family, always AF_PACKET
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_family = AF_PACKET;

		// Physical protocol, set to Sercos ether type
#ifdef RTOS_FILTER_SERCOS_ETHERTYPE
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_protocol = htons(SICE_SIII_ETHER_TYPE);
#endif

#ifdef RTOS_BIND_NIC
		// Index of network device, hard-coded to first hardware device
		//RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_ifindex = 2;
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_ifindex =
				if_nametoindex(RTLX_SocketInstances[iInstanceNo].acName);
#endif

		// Header type
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_hatype = ARPHRD_ETHER;

		// Packet type
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_pkttype = PACKET_OTHERHOST;

		// Length of address
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_halen = ETH_ALEN;

		// Get hardware MAC address (\todo: correct?)
		ioctl
		(
				RTLX_SocketInstances[iInstanceNo].iTxSocketId,
				SIOCGIFHWADDR,
				&rIfReq
		);

		// Set hardware MAC address to Ethernet source address
		pucTempMAC = (UCHAR *)rIfReq.ifr_hwaddr.sa_data;
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_addr[0] = pucTempMAC[0];
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_addr[1] = pucTempMAC[1];
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_addr[2] = pucTempMAC[2];
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_addr[3] = pucTempMAC[3];
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_addr[4] = pucTempMAC[4];
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_addr[5] = pucTempMAC[5];
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_addr[6] = 0x00; // not used
		RTLX_SocketInstances[iInstanceNo].rTxSocketAddress.sll_addr[7] = 0x00; // not used

		// Return obtained MAC address of adapter
		(VOID)memcpy
				(
						pucMAC,
						pucTempMAC,
						6
				);
	}
	return(RTOS_RET_OK);


}

/**
 * \fn VOID RTLX_CloseTxSocket(
 *              INT iInstanceNo,
 *              BOOL boRedundancy
 *          )
 *
 * \param[in]   iInstanceNo     Sercos IP core emulation instance number
 * \param[in]   boRedundancy    Defines whether Sercos port redundancy is used
 *
 * \brief   Closes transmit socket
 *
 */
VOID RTLX_CloseTxSocket
(
		INT iInstanceNo,
		BOOL boRedundancy
)
{

	if (iInstanceNo >= RTLX_MAX_SOCKET_INSTANCES)
	{
		RTLX_VERBOSE
		(
				0,
				"RTLX_CloseTxSocket() instance %d too large, only %d available\n",
				iInstanceNo,
				RTLX_MAX_SOCKET_INSTANCES
		);
	}

	close(RTLX_SocketInstances[iInstanceNo].iTxSocketId);


}

/**
 * \fn INT RTLX_OpenRxSocket(
 *              INT iInstanceNo,
 *              BOOL boRedundancy
 *          )
 *
 * \brief   Opens raw Ethernet receive socket
 *
 * \param[in]   iInstanceNo     Sercos IP core emulation instance number
 * \param[in]   boRedundancy    Defines whether Sercos port redundancy is used
 *
 * \return
 * - 0: OK
 * - -1: Error
 *
 * \ingroup RTLX
 *
 */
INT RTLX_OpenRxSocket
(
		INT iInstanceNo,
		BOOL boRedundancy
)
{
	INT iFlags   = 0;
	INT iRet     = 0;
	INT iPortCnt = 1;
	INT iPort    = 0;
	struct sockaddr_ll rSockAddr;

	if (boRedundancy)
	{
		iPortCnt = 2;
	}

	if (iPortCnt * iInstanceNo >= RTLX_MAX_SOCKET_INSTANCES)
	{
		RTLX_VERBOSE
		(
				0,
				"RTLX_OpenRxSocket() instance %d too large\n",
				iInstanceNo
		);
		return(RTOS_RET_ERROR);
	}

	for (
			iPort = 0;
			iPort < iPortCnt;
			iPort ++
	)
	{

		// Socket descriptor
		RTLX_SocketInstances[2*iInstanceNo + iPort].iRxSocketId = socket
				(
						AF_PACKET,                    // Packet-based socket
						SOCK_RAW,                     // Raw Ethernet socket
#ifdef RTOS_FILTER_SERCOS_ETHERTYPE
						htons(SICE_SIII_ETHER_TYPE)   // Sercos ether type
#else
						htons(ETH_P_ALL)
#endif
				);

		if (RTLX_SocketInstances[2*iInstanceNo + iPort].iRxSocketId < 0)
		{
			RTLX_VERBOSE
			(
					0,
					"Error %d (%s) creating receive socket for port %d\n",
					errno,
					strerror(errno),
					iPort
			);

			return(RTOS_RET_ERROR);
		}
		else
		{
			iFlags = fcntl
					(
							RTLX_SocketInstances[2*iInstanceNo + iPort].iRxSocketId,
							// Socket
							F_GETFL                                         // Get flags command
					);

			// Activate non-blocking mode
			iFlags |= O_NONBLOCK;

			(VOID)fcntl
					(
							RTLX_SocketInstances[2*iInstanceNo + iPort].iRxSocketId,
							// Socket
							F_SETFL,                                        // Set flags command
							iFlags                                          // Flags
					);
		}
	}  // for all ports

	return(RTOS_RET_OK);
}

/**
 * \fn VOID RTLX_CloseRxSocket(
 *              INT iInstanceNo,
 *              BOOL boRedundancy
 *          )
 *
 * \param[in]   iInstanceNo     Sercos IP core emulation instance number
 * \param[in]   boRedundancy    Defines whether Sercos port redundancy is used
 *
 * \brief   Closes receive socket
 *
 */
VOID RTLX_CloseRxSocket
(
		INT iInstanceNo,
		BOOL boRedundancy
)
{
	INT iPortCnt = 1;
	INT iPort    = 0;


	if (boRedundancy)
	{
		iPortCnt = 2;
	}

	if (iPortCnt * iInstanceNo >= RTLX_MAX_SOCKET_INSTANCES)
	{
		RTLX_VERBOSE
		(
				0,
				"RTLX_CloseRxSocket() instance %d too large, only %d available\n",
				iInstanceNo,
				RTLX_MAX_SOCKET_INSTANCES
		);
	}
	for (
			iPort = 0;
			iPort < iPortCnt;
			iPort ++
	)
	{
		(VOID)close(RTLX_SocketInstances[2*iInstanceNo + iPort].iRxSocketId);
	}
}

/**
 * \fn INT RTLX_TxPacket(
 *              INT iInstanceNo,
 *              INT iPort,
 *              UCHAR* pucFrame,
 *              USHORT usLen,
 *              USHORT usIFG
 *          )
 *
 * \brief   Transmits raw Ethernet packet.
 *
 * \param[in]   iInstanceNo Sercos IP core emulation instance number
 * \param[in]   iPort       Port number
 * \param[in]   pucFrame    Pointer to packet buffer
 * \param[in]   usLen       Length of packet
 * \param[in]   usIFG       Required inter frame gap
 *
 * \note    Tx socket needs to be opened using RTLX_OpenRxSocket() before using
 *          this function.
 *
 * \note    Inter frame gap not yet taken into account
 *
 * \return
 * - >0: Number of bytes transmitted
 * - <0: Error
 *
 * \ingroup RTLX
 *
 */
INT RTLX_TxPacket
(
		INT iInstanceNo,
		INT iPort,
		UCHAR* pucFrame,
		USHORT usLen,
		USHORT usIFG
)
{
	INT iTxFlags  = 0;
	INT iRet      = 0;

	if (iInstanceNo >= RTLX_MAX_SOCKET_INSTANCES)
	{
		RTLX_VERBOSE
		(
				0,
				"RTLX_TxPacket() instance %d too large, only %d available\n",
				iInstanceNo,
				RTLX_MAX_SOCKET_INSTANCES
		);
		return(RTOS_RET_ERROR);
	}

	iRet = sendto
			(
					RTLX_SocketInstances[iInstanceNo].iTxSocketId, // Socket
					pucFrame,                                      // Pointer to data
					usLen,                                         // Length in bytes
					iTxFlags,                                      // Flags
					(struct sockaddr*) &(RTLX_SocketInstances[iInstanceNo].rTxSocketAddress),
					// Address descriptor
					sizeof(RTLX_SocketInstances[iInstanceNo].rTxSocketAddress)
					// Size of address descriptor
			);

	if (iRet >= 0)
	{
		return(iRet);
	}
	else
	{
		return(RTOS_RET_ERROR);
	}
}

/**
 * \fn INT RTLX_RxPacket(
 *              INT iInstanceNo,
 *              INT iPort,
 *              UCHAR* pucFrame,
 *              UCHAR** ppucFrame
 *          )
 *
 * \brief   Receives raw Ethernet packet. Function is non-blocking.
 *
 * \note    This function may be implemented in two ways on a certain operating
 *          system in order to optimize buffer handling. It may either (1)
 *          return a pointer to an existing buffer in ppucFrame containing the
 *          received packet, or (2) use a buffer allocated by the calling
 *          entity. If the function returns a 'NULL' in ppucFrame, (2) is used,
 *          otherwise (1).
 *
 * \note    Tx socket needs to be opened using RTLX_OpenRxSocket() before using
 *          this function. Buffer pucFrame needs to be large enough to hold an
 *          entire Ethernet frame of maximum size (ETHERNET_MAX_FRAMEBUF_LEN).
 *
 * \param[in]   iInstanceNo Sercos IP core emulation instance number
 * \param[in]   iPort       Port number
 * \param[out]  pucFrame    Pointer to packet buffer
 * \param[out]  ppucFrame   Pointer to pointer to packet buffer
 *
 * \return
 * - >0: Number of received bytes
 * - -1: Error
 *
 * \ingroup RTLX
 *
 */
INT RTLX_RxPacket
(
		INT iInstanceNo,
		INT iPort,
		UCHAR* pucFrame,
		UCHAR** ppucFrame
)
{
	INT       iRxFlags              = 0;
	INT       iRet                  = 0;
	struct    sockaddr rRXSrcAddr;
	socklen_t iRxSrcAddrLen         = sizeof(struct sockaddr);

	if (2* iInstanceNo >= RTLX_MAX_SOCKET_INSTANCES)
	{
		RTLX_VERBOSE
		(
				0,
				"RTLX_RxPacket() instance %d too large, only %d available\n",
				iInstanceNo,
				RTLX_MAX_SOCKET_INSTANCES
		);
		return(RTOS_RET_ERROR);
	}

	rRXSrcAddr.sa_family = AF_PACKET;

	iRet = recvfrom
			(
					RTLX_SocketInstances[2* iInstanceNo + iPort].iRxSocketId,  // Socket
					pucFrame,                                       // Pointer to buffer
					SICE_ETH_FRAMEBUF_LEN,                          // Buffer size
					iRxFlags,                                       // Flags
					&rRXSrcAddr,                                    // RX source address to filter for
					&iRxSrcAddrLen                                  // Length of RX source address
			);

	// Signal that provided buffer was used, not own one
	ppucFrame = NULL;

	if (iRet >= 0)
	{
		RTLX_VERBOSE
		(
				2,
				"Received packet on port %d\n",
				iPort
		);

		return(iRet);
	}
	// 'Error' is only 'try again'?
	else if (errno == EAGAIN)
	{
		// Then ignore it.
		return(RTOS_RET_OK);
	}
	// Otherwise return error
	else
	{
		return(RTOS_RET_ERROR);
	}
}

