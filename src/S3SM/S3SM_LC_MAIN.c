#include "rtapi.h"
#include "rtapi_app.h"
#include "hal.h"
#include "hal_priv.h"

#include "../SIII/SIII_GLOB.h"
#include "../SIII/SIII_PRIV.h"
#include "../S3SM/S3SM_PRIV.h"
#include "../CSMD/CSMD_GLOB.h"
#include "../CSMD/CSMD_HAL_PRIV.h"
#include "../RTLX/RTLX_GLOB.h"

#define SOURCE_S3SM
#define S3SM_S_1302_0_1_FSP_IO      (1)
#define S3SM_S_1302_0_1_FSP_DRIVE   (2)
#define S3SM_S_1302_0_1_FSP_ENC     (3)

/* module information */
MODULE_AUTHOR("Andreas Schiffler");
MODULE_DESCRIPTION("HAL Driver for Sercos 3 Softmaster S3SM");
MODULE_LICENSE("GPL");

/* globals  */
typedef struct {
	hal_float_t			*pos;
	hal_float_t			*commanded_pos;
	hal_float_t			*vel;
	hal_float_t			*commanded_vel;
	hal_float_t			*torque;
	hal_bit_t			*slave_ready;
	hal_bit_t 			*slave_error;
	hal_bit_t 			*slave_power_on;
	hal_bit_t 			*op_mode_vel;
}s3sm_slave_hal_data_t;

typedef struct {
	hal_bit_t 				*power_on;
	hal_u32_t 				*act_phase;
	hal_u32_t				*active_slaves;
	hal_float_t				*jitter_us;
	s3sm_slave_hal_data_t	slave[3];
	SIII_INSTANCE_STRUCT    rS3Instance;
} s3sm_hal_data_t;

long last_start_time = 0;

static s3sm_hal_data_t *s3sm_hal_data;

static int comp_id;		/* component ID */



/* local functions */
VOID S3SM_Connection_Conf_Drive
(
		SIII_INSTANCE_STRUCT *prS3Instance,
		USHORT usDevIdx,
		USHORT *pusMasterConnOffset,
		USHORT usRTDataConnIdx
)
{
	CSMD_CONNECTION          *prConnection       = NULL;
	CSMD_CONFIGURATION       *prConfig           = NULL;
	CSMD_CONN_IDX_STRUCT     *prMasterConnIdx    = NULL;
	CSMD_SLAVE_CONFIGURATION *prSlaveConfig      = NULL;
	USHORT                   usConIdx            = 0;
	ULONG                    ulSVCWriteData      = 0;
	SIII_FUNC_RET            S3FuncRet           = SIII_NO_ERROR;

	S3SM_VERBOSE(0, "- MDT: S134, S36, S47\n");
	S3SM_VERBOSE(0, "- AT : S135, S40, S51, S84\n");

	// Set S-0-0032 of drive to selected drive operation mode
	ulSVCWriteData      = S3SM_DRIVE_OP_MODE; // Position Control
	S3SM_VERBOSE(0, "Setting 1st operation mode (S-0-0032) of device #%hu to %d\n",usDevIdx,S3SM_DRIVE_OP_MODE);
	S3FuncRet = SIII_SVCWrite(prS3Instance,usDevIdx,TRUE,32,0,0,7,(USHORT*)&ulSVCWriteData,4);
	if (S3FuncRet != SIII_NO_ERROR)
	{
		rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
				"Error %d when trying to set OP Mode "
				"for device #%d\n",
				(INT) S3FuncRet,usDevIdx);
	}
	ulSVCWriteData      = 2;// Velocity Control
	S3SM_VERBOSE(0, "Setting 2nd operation mode (S-0-0033) of device #%hu to %d\n",usDevIdx,2);
	S3FuncRet = SIII_SVCWrite(prS3Instance,usDevIdx,TRUE,33,0,0,7,(USHORT*)&ulSVCWriteData,4);
	if (S3FuncRet != SIII_NO_ERROR)
	{
		rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
				"Error %d when trying to set OP Mode "
				"for device #%d\n",
				(INT) S3FuncRet,usDevIdx);
	}

	// -----------------------------------------------------
	// Connections
	// -----------------------------------------------------

	// MDT Drive Controller
	prConnection = &prS3Instance->rCosemaInstance.rConfiguration.parConnection[ 2 * usDevIdx ];
	prConnection->usTelegramType    = (USHORT)  CSMD_TELEGRAM_TYPE_MDT;
	prConnection->usS_0_1050_SE2    = (USHORT) (2 * usDevIdx);

	// Pre-define connection length for generic device
	prConnection->usS_0_1050_SE5    = (USHORT) S3SM_CONN_LEN_MDT_DEFAULT;
	prConnection->ulS_0_1050_SE10     = SIII_GetSercosCycleTime(prS3Instance, SIII_PHASE_CP4);
	prConnection->usS_0_1050_SE11     = (USHORT)  S3SM_ACCEPTED_TEL_LOSSES;
	prConnection->usApplicationID     = (USHORT)  42;
	prConnection->ucConnectionName[0] = (UCHAR)   0;
	prConnection->pvConnInfPtr        = NULL;

	// AT Drive Controller
	prConnection = &prS3Instance->rCosemaInstance.rConfiguration.parConnection[ 2 * usDevIdx + 1 ];
	prConnection->usTelegramType    = (USHORT)  CSMD_TELEGRAM_TYPE_AT;
	prConnection->usS_0_1050_SE2    = ((USHORT) 1) + (USHORT) (2 * usDevIdx);
	// Pre-define connection length for generic device
	prConnection->usS_0_1050_SE5    = (USHORT) S3SM_CONN_LEN_AT_DEFAULT;
	prConnection->ulS_0_1050_SE10     = SIII_GetSercosCycleTime(prS3Instance, SIII_PHASE_CP4);
	prConnection->usS_0_1050_SE11     = (USHORT)  S3SM_ACCEPTED_TEL_LOSSES;
	prConnection->usApplicationID     = (USHORT)  42;
	prConnection->ucConnectionName[0] = (UCHAR)   0;
	prConnection->pvConnInfPtr      = NULL;

	// -----------------------------------------------------
	// Configurations
	// -----------------------------------------------------

	// Master produces the connection to the drive controller
	prConfig = &prS3Instance->rCosemaInstance.rConfiguration.parConfiguration[ 4 * usDevIdx ];
	prConfig->usS_0_1050_SE7 = (USHORT) 0xFFFF;
	prConfig->usS_0_1050_SE1 =
			(
					((USHORT) CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)  |   // Active producer connection
					((USHORT) CSMD_S_0_1050_SE1_IDN_LIST)         |   // IDN configuration list
					((USHORT) CSMD_S_0_1050_SE1_SYNC)                 // Cycle synchronous
			);
	prConfig->ulS_0_1050_SE6[0] = S3SM_LIST_LENGTHS(3);
	prConfig->ulS_0_1050_SE6[1] = CSMD_EIDN(134, 0, 0);   // 2 S-0-0134 (MDT) Master control word
	prConfig->ulS_0_1050_SE6[2] = CSMD_EIDN( 36, 0, 0);   // 4 S-0-0036 (MDT) Velocity command value
	prConfig->ulS_0_1050_SE6[3] = CSMD_EIDN( 47, 0, 0);   // 4 S-0-0047 (MDT & AT) Position command value
	prConfig->usTelgramTypeS00015 = (USHORT) 0;

	// Master consumes the connection from the drive controller
	prConfig = &prS3Instance->rCosemaInstance.rConfiguration.parConfiguration[ 4 * usDevIdx + 1 ];
	prConfig->usS_0_1050_SE7 = (USHORT) 0xFFFF;
	prConfig->usS_0_1050_SE1 =
			(
					((USHORT) CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)  |   // Active consumer connection
					((USHORT) CSMD_S_0_1050_SE1_IDN_LIST)         |   // IDN configuration list
					((USHORT) CSMD_S_0_1050_SE1_SYNC)                 // Cycle synchronous
			);
	prConfig->ulS_0_1050_SE6[0] = S3SM_LIST_LENGTHS(4);
	prConfig->ulS_0_1050_SE6[1] = CSMD_EIDN(135, 0, 0);   // 2 S-0-0135 (AT) Drive status word
	prConfig->ulS_0_1050_SE6[2] = CSMD_EIDN( 40, 0, 0);   // 4 S-0-0040 (AT) Velocity feedback value
	prConfig->ulS_0_1050_SE6[3] = CSMD_EIDN( 51, 0, 0);   // 4 S-0-0051 (AT) Position feedback value 1
	prConfig->ulS_0_1050_SE6[4] = CSMD_EIDN( 84, 0, 0);   // 2 S-0-0084 (AT) Torque/Force feedback value
	prConfig->usTelgramTypeS00015 = (USHORT) 0;

	// Drive controller consumes the connection from the master
	prConfig = &prS3Instance->rCosemaInstance.rConfiguration.parConfiguration[ 4 * usDevIdx + 2 ];
	prConfig->usS_0_1050_SE7 = (USHORT) 0xFFFF;
	prConfig->usS_0_1050_SE1 =
			(
					((USHORT) CSMD_S_0_1050_SE1_ACTIVE_CONSUMER)  |   // Active consumer connection
					((USHORT) CSMD_S_0_1050_SE1_IDN_LIST)         |   // IDN configuration list
					((USHORT) CSMD_S_0_1050_SE1_SYNC)                 // Cycle synchronous
			);
	prConfig->ulS_0_1050_SE6[0] = S3SM_LIST_LENGTHS(3);
	prConfig->ulS_0_1050_SE6[1] = CSMD_EIDN(134, 0, 0);   // 2 S-0-0134 (MDT) Master control word
	prConfig->ulS_0_1050_SE6[2] = CSMD_EIDN( 36, 0, 0);   // 4 S-0-0036 (MDT) Velocity command value
	prConfig->ulS_0_1050_SE6[3] = CSMD_EIDN( 47, 0, 0);   // 4 S-0-0047 (MDT & AT) Position command value
	prConfig->usTelgramTypeS00015 = (USHORT)  0;

	// Drive controller produces the connection to the master
	prConfig = &prS3Instance->rCosemaInstance.rConfiguration.parConfiguration[ 4 * usDevIdx + 3 ];
	prConfig->usS_0_1050_SE7 = (USHORT) 0xFFFF;
	prConfig->usS_0_1050_SE1 =
			(
					((USHORT) CSMD_S_0_1050_SE1_ACTIVE_PRODUCER)  |   // Active producer connection
					((USHORT) CSMD_S_0_1050_SE1_IDN_LIST)         |   // IDN configuration list
					((USHORT) CSMD_S_0_1050_SE1_SYNC)                 // Cycle synchronous
			);
	prConfig->ulS_0_1050_SE6[0]   = S3SM_LIST_LENGTHS(4);
	prConfig->ulS_0_1050_SE6[1]   = CSMD_EIDN(135, 0, 0);   // 2 S-0-0135 (AT) Drive status word
	prConfig->ulS_0_1050_SE6[2]   = CSMD_EIDN( 40, 0, 0);   // 4 S-0-0040 (AT) Velocity feedback value
	prConfig->ulS_0_1050_SE6[3]   = CSMD_EIDN( 51, 0, 0);   // 4 S-0-0051 (AT) Position feedback value 1
	prConfig->ulS_0_1050_SE6[4]  =  CSMD_EIDN( 84, 0, 0);   // 2 S-0-0084 (AT) Torque/Force feedback value
	prConfig->usTelgramTypeS00015 = (USHORT)  0;

	// -----------------------------------------------------
	// Master: Configuration
	// -----------------------------------------------------

	// Take over current master connection index, not to overwrite existing connections
	usConIdx = *pusMasterConnOffset;

	// MDT
	prMasterConnIdx = &prS3Instance->rCosemaInstance.rConfiguration.rMasterCfg.parConnIdxList[usConIdx];
	prMasterConnIdx->usConnIdx   = (USHORT) (2 * usDevIdx);
	prMasterConnIdx->usConfigIdx = (USHORT) (4 * usDevIdx);
	usConIdx++;

	// AT
	prMasterConnIdx = &prS3Instance->rCosemaInstance.rConfiguration.rMasterCfg.parConnIdxList[usConIdx];
	prMasterConnIdx->usConnIdx   = (USHORT) (2 * usDevIdx + 1);
	prMasterConnIdx->usConfigIdx = (USHORT) (4 * usDevIdx + 1);
	usConIdx++;

	prS3Instance->rCosemaInstance.rConfiguration.rMasterCfg.usNbrOfConnections = usConIdx;

	// Signal back connection index to calling entity
	*pusMasterConnOffset = usConIdx;

	// -----------------------------------------------------
	// Slave: Configuration
	// -----------------------------------------------------
	prSlaveConfig = &prS3Instance->rCosemaInstance.rConfiguration.parSlaveConfig[usDevIdx];

	usConIdx = 0;
	// Connection 0: MDT
	prSlaveConfig->arConnIdxList[usConIdx].usConnIdx   = (USHORT) (2 * usDevIdx);
	prSlaveConfig->arConnIdxList[usConIdx].usConfigIdx = (USHORT) (4 * usDevIdx + 2);
	usConIdx++;

	// Connection 1: AT
	prSlaveConfig->arConnIdxList[usConIdx].usConnIdx   = (USHORT) (2 * usDevIdx + 1);
	prSlaveConfig->arConnIdxList[usConIdx].usConfigIdx = (USHORT) (4 * usDevIdx + 3);

	usConIdx++;
	prSlaveConfig->usNbrOfConnections = usConIdx;
}

VOID S3SM_CyclicDrive_RW(SIII_INSTANCE_STRUCT *prS3Instance,USHORT usDevIdx)
{
	SIII_FUNC_RET  eS3Ret;
	USHORT			usStatus;
	USHORT*        pusMDTConn;
	USHORT*        pusATConn;
	USHORT         usMDTConnLength;  /* employ arrays for multiple connections in one telegram */
	USHORT         usATConnLength;   /* employ arrays for multiple connections in one telegram */

	S3SM_VERBOSE(2, "S3SM_CyclicDrive()\n");

	if (prS3Instance->rCosemaInstance.rSlaveList.aeSlaveActive[usDevIdx] == CSMD_SLAVE_ACTIVE)
	{
		eS3Ret = SIII_GetDeviceStatus
				(
						prS3Instance,
						usDevIdx,
						&usStatus
				);

		if (eS3Ret != SIII_SERCOS_SLAVE_VALID_NOT_SET)
		{
			eS3Ret = SIII_GetDeviceCyclicDataPtr
					(
							prS3Instance,
							usDevIdx,
							SIII_DEV_AT_DATA,
							0,
							&pusATConn,
							&usATConnLength
					);


			if (eS3Ret != SIII_NO_ERROR)
			{
				S3SM_VERBOSE
				(
						0,
						"Error %d when trying to retrieve cyclic "
						"AT data pointer for device #%d\n",
						(INT) eS3Ret,
						usDevIdx
				);
			}
		}
		else /* print error message depending on whether allowed slave valid miss limit has been exceeded */
		{
			/* special construction necessary, '<=' comparison not sufficient in this case */
			if (prS3Instance->rCosemaInstance.arDevStatus[usDevIdx].usMiss < prS3Instance->rCosemaInstance.rConfiguration.rComTiming.usAllowed_Slave_Valid_Miss)
			{
				S3SM_VERBOSE
				(
						0,
						"WARNING: Slave #%d has not set slave valid in current cycle!\n", usDevIdx
				);
			}
			/* this case implies the condition usMiss == allowed misses because of 'slave active' condition above */
			else if ( !(prS3Instance->rCosemaInstance.arDevStatus[usDevIdx].usS_Dev & CSMD_S_DEV_SLAVE_VALID))
			{
				S3SM_VERBOSE
				(
						0,
						"WARNING: Slave #%d has exceeded the limit for consecutive slave valid misses.\n"
						"It hat been set inactive by CoSeMa.\n", usDevIdx
				);
			}
		}

		eS3Ret = SIII_GetDeviceCyclicDataPtr
				(
						prS3Instance,
						usDevIdx,
						SIII_DEV_MDT_DATA,
						0,
						&pusMDTConn,
						&usMDTConnLength
				);

		if (eS3Ret != SIII_NO_ERROR)
		{
			S3SM_VERBOSE
			(
					0,
					"Error %d when trying to retrieve cyclic "
					"MDT data pointer for device #%d\n",
					(INT) eS3Ret,
					usDevIdx
			);
		}

		//----------- MachineKit / LinuxCNC Specific----START

		// read hal pins and copy to MDT
		*(LONG*)(pusMDTConn + 4) = (LONG)(*(s3sm_hal_data->slave[usDevIdx].commanded_pos) * 10000);
		*(LONG*) (pusMDTConn + 2) = (LONG)(*(s3sm_hal_data->slave[usDevIdx].commanded_vel) * 1000);

		if ( *(s3sm_hal_data->power_on) )
		{
			S3SM_VERBOSE(2, "enabled dev idx %d\n",usDevIdx);
			if (*(s3sm_hal_data->slave[usDevIdx].op_mode_vel))
			{
				*(pusMDTConn + 1) = (USHORT) SIII_SLAVE_ENABLE + 0x100; // Set Bits 15,14,13 + 8
			}
			else
			{
				*(pusMDTConn + 1) = (USHORT) SIII_SLAVE_ENABLE; // Set Bits 15,14,13
			}
		}
		else
		{
			*(pusMDTConn + 1) = (USHORT) SIII_SLAVE_DISABLE;
		}

		// read from AT and copy to hal pins
		*(s3sm_hal_data->slave[usDevIdx].pos) = (hal_float_t) ( *(LONG*)(pusATConn + 4) ) / 10000;
		*(s3sm_hal_data->slave[usDevIdx].vel) = (hal_float_t) ( *(LONG*)(pusATConn + 2) ) / 1000;
		*(s3sm_hal_data->slave[usDevIdx].torque) = (hal_float_t) ( *(SHORT*)(pusATConn + 5) ) / 100;
		*(s3sm_hal_data->slave[usDevIdx].slave_ready) = (hal_bit_t) ( ( *(pusATConn + 1) & 0x8000) == 0x8000 );
		*(s3sm_hal_data->slave[usDevIdx].slave_error) = (hal_bit_t) ( ( *(pusATConn + 1) & 0x2000) == 0x2000 );
		*(s3sm_hal_data->slave[usDevIdx].slave_power_on) = (hal_bit_t) ( ( *(pusATConn + 1) & 0xc000) == 0xc000 );

		//----------- MachineKit / LinuxCNC Specific----END

		(VOID)SIII_SetCyclicDataValid
				(
						prS3Instance,
						usDevIdx
				);

		S3SM_VERBOSE
		(
				1,
				"Drive control word for device %d: %4x, "
				"Command pos: %5d, Current pos: %5d\n",
				usDevIdx,
				*(pusMDTConn + 1),
				*(INT*)(pusMDTConn + 4),
				*(INT*)(pusATConn + 4)
		);
	} /* if (prS3Instance->rCosemaInstance.rSlaveList.aeSlaveActive[usDevIdx] == CSMD_SLAVE_ACTIVE) */
}

static int sercos_cycle_worker_func(void *arg, const hal_funct_args_t *fa)
{
	SIII_FUNC_RET   		eS3FuncRet      = SIII_NO_ERROR;
	unsigned long time_temp = fa_start_time(fa);

	*(s3sm_hal_data->jitter_us) =  ((hal_float_t)(LONG)(S3SM_CYCLE_TIME - (time_temp - last_start_time)))/1000;
	last_start_time = time_temp;

	// SIII Prepare
	eS3FuncRet = SIII_Cycle_Prepare(&(s3sm_hal_data->rS3Instance));

	// SIII Start
	eS3FuncRet = SIII_Cycle_Start(&(s3sm_hal_data->rS3Instance),(ULONG*)&time_temp);

	return(eS3FuncRet);
}


S3SM_FUNC_RET   sercos_handle_conf(SIII_INSTANCE_STRUCT *prS3Instance, const char **argv, const int argc)
{
	CHAR          cBuffer         = ' ';      // Buffer for stdin operations
	SIII_FUNC_RET eS3Ret          = SIII_NO_ERROR;
	BOOL          boIsStdPar      = TRUE;     // Selected IDN category for SVC
	USHORT        usEidnId        = 0;        // Selected IDN for SVC
	USHORT        usEidnSe        = 0;        // Selected IDN SE for SVC
	USHORT        usEidnSi        = 0;        // Selected IDN SI for SVC
	USHORT        usDevIdx        = 0;        // Sercos device index
	ULONG         ulSVCWriteData  = 0;        // Data to write via SVC
	INT iCnt  = 0;		// Counter for number of found slave during auto config

	cBuffer = *(argv[0]);

	switch (cBuffer){

	// Read slave parameter via SVC
	case 'r':
		if (SIII_GetSercosPhase(prS3Instance) >= SIII_PHASE_CP2)
		{
			if (argc == 6){ // example: r 0 S 32 0 0
				usDevIdx = atoi(argv[1]);
				boIsStdPar = (*(argv[2]) == 'S');
				usEidnId = atoi(argv[3]);
				usEidnSi = atoi(argv[4]);
				usEidnSe = atoi(argv[5]);
			}
			eS3Ret = SIII_SVCRead
					(
							prS3Instance,     // SIII instance
							usDevIdx,         // Device index
							boIsStdPar,       // Standard or specific parameter?
							usEidnId,         // IDN
							usEidnSi,         // Structural instance
							usEidnSe,         // Structural element
							(USHORT) 7,       // Element 7: Operational data
							prS3Instance->rMySVCResult.ausSVCData,// Data pointer
							(USHORT)SIII_SVC_BUF_SIZE// Buffer size
					);

			if (eS3Ret != SIII_NO_ERROR)
			{
				rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
						"Error #%X during SVC access.\n",
						(INT)eS3Ret
				);
			}
		}
		else
		{
			S3SM_VERBOSE(0, "Only possible in CP2 or higher\n");
		}
		return(S3SM_NO_ERROR);
		/*lint -save -e527 */
		break;
		/*lint -restore */

		// Write slave parameter via SVC
	case 'w':
		if (SIII_GetSercosPhase(prS3Instance) >= SIII_PHASE_CP2)
		{
			if (argc == 8){ // example: w 0 S 32 0 0 267 4
				usDevIdx = atoi(argv[1]);
				boIsStdPar = (*(argv[2]) == 'S');
				usEidnId = atoi(argv[3]);
				usEidnSi = atoi(argv[4]);
				usEidnSe = atoi(argv[5]);
				ulSVCWriteData = atoi(argv[6]);
			}
			eS3Ret = SIII_SVCWrite
					(
							prS3Instance,     // SIII instance
							usDevIdx,         // Device index
							boIsStdPar,       // Standard or specific parameter?
							usEidnId,         // IDN
							usEidnSi,         // Structural instance
							usEidnSe,         // Structural element
							(USHORT) 7,       // Element 7: Operational data
							(USHORT*)&ulSVCWriteData,	// Data buffer
							atoi(argv[7])                 // x bytes to write
					);

			if (eS3Ret != SIII_NO_ERROR)
			{
				rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
						"Error #%X during SVC access.\n",
						(INT)eS3Ret);
			}
		}
		else
		{
			S3SM_VERBOSE(0, "Only possible in CP2 or higher\n");
		}
		return(S3SM_NO_ERROR);
		/*lint -save -e527 */
		break;
		/*lint -restore */

		// Execute slave command via SVC
	case 'c':
		if (SIII_GetSercosPhase(prS3Instance) >= SIII_PHASE_CP2)
		{
			if (argc == 6){ // example: r 0 S 32 0 0
				usDevIdx = atoi(argv[1]);
				boIsStdPar = (*(argv[2]) == 'S');
				usEidnId = atoi(argv[3]);
				usEidnSi = atoi(argv[4]);
				usEidnSe = atoi(argv[5]);
			}

			eS3Ret = SIII_SVCCmd
					(
							prS3Instance,     // SIII instance
							usDevIdx,         // Device index
							boIsStdPar,       // Standard or specific parameter?
							usEidnId,         // IDN
							usEidnSi,         // Structural instance
							usEidnSe          // Structural element
					);

			if (eS3Ret != SIII_NO_ERROR)
			{
				rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
						"Error #%X during SVC access.\n",
						(INT)eS3Ret
				);
			}
		}
		else
		{
			S3SM_VERBOSE(0, "Only possible in CP2 or higher\n");
		}
		return(S3SM_NO_ERROR);
		/*lint -save -e527 */
		break;
		/*lint -restore */



		// Go directly to phase 2 without connection configuration
	case '2':
		if (SIII_GetSercosPhase(prS3Instance) < SIII_PHASE_CP2)
		{
			// Assign application-specific functions to pointers
			(VOID)SIII_ClearDeviceCallbacks(prS3Instance);

			eS3Ret = SIII_PhaseSwitch
					(
							prS3Instance,   // SIII instance
							SIII_PHASE_CP2, // CP2
							0,              // no retries
							30              // timeout: 30s
					);

			if (eS3Ret != SIII_NO_ERROR)
			{
				rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
						"Error #%X during Sercos phase switch.\n",
						(INT)eS3Ret
				);
			}
		}
		else
		{
			rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX "Already in phase 2 or higher\n");
		}
		return(S3SM_NO_ERROR);
		/*lint -save -e527 */
		break;
		/*lint -restore */

		// Go directly to phase 4 without connection configuration
	case '4':
		if (SIII_GetSercosPhase(prS3Instance) < SIII_PHASE_CP4)
		{
			// Assign application-specific functions to pointers
			(VOID)SIII_ClearDeviceCallbacks(prS3Instance);

			eS3Ret = SIII_PhaseSwitch
					(
							prS3Instance,   // SIII instance
							SIII_PHASE_CP4, // CP4
							0,              // no retries
							60              // timeout: 60s
					);

			if (eS3Ret != SIII_NO_ERROR)
			{
				rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
						"Error #%X during Sercos phase switch.\n",
						(INT)eS3Ret
				);
			}
		}
		else
		{
			rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX "Already in phase 4\n");
		}
		return(S3SM_NO_ERROR);
		/*lint -save -e527 */
		break;
		/*lint -restore */

		// Go To Phase 2 | Search for Drives | Configure all Drives | Set cyclic callback
	case 'f':
		if (SIII_GetSercosPhase(prS3Instance) < SIII_PHASE_CP4)
		{
			// Assign application-specific functions to pointers
			(VOID)SIII_ClearDeviceCallbacks(prS3Instance);

			eS3Ret = SIII_PhaseSwitch
					(
							prS3Instance,   // SIII instance
							SIII_PHASE_CP2, // CP2
							0,              // no retries
							30              // timeout: 60s
					);

			if (eS3Ret != SIII_NO_ERROR)
			{
				rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
						"Error #%X during phase switch.\n",
						(INT)eS3Ret
				);
			}
			// For each slave ...
			for (
					iCnt = 0;
					iCnt < SIII_GetNoOfSlaves(prS3Instance);
					iCnt++
			)
			{
				// Read S-0-1302.0.1
				eS3Ret = SIII_SVCRead
						(
								prS3Instance,     // SIII instance
								iCnt,             // Device index
								TRUE,             // Standard or specific parameter?
								1302,             // IDN
								0,                // Structural instance
								1,                // Structural element
								(USHORT) 7,       // Element 7: Operational data
								prS3Instance->rMySVCResult.ausSVCData,
								// Data pointer
								(USHORT)SIII_SVC_BUF_SIZE
								// Buffer size
						);

				if (eS3Ret != SIII_NO_ERROR)
				{
					rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
							"Error #%X during SVC access.\n",
							(INT)eS3Ret
					);
				}

				// Set callbacks accordingly
				switch (prS3Instance->rMySVCResult.ausSVCData[1])
				{
				case S3SM_S_1302_0_1_FSP_DRIVE:
					S3SM_VERBOSE
					(
							0,
							"Slave #%d is configured as drive.\n",
							iCnt
					);

					eS3Ret = SIII_SetDeviceCallback
							(
									prS3Instance,       // SIII instance
									iCnt,               // Slave no
									(FP_APP_CONN_CONFIG) S3SM_Connection_Conf_Drive,
									(FP_APP_CYCLIC) S3SM_CyclicDrive_RW
							);
					if (eS3Ret != SIII_NO_ERROR)
					{
						S3SM_VERBOSE(0,"Callbacks configured for dev %d\n",iCnt);
					}
					break;
				default:
					S3SM_VERBOSE
					(
							0,
							"Error: Slave #%d is an unknown device!\n",
							iCnt
					);
					break;
				}
			}
			eS3Ret = SIII_PhaseSwitch
					(
							prS3Instance,   // SIII instance
							SIII_PHASE_CP4, // CP4
							0,              // no retries
							60              // timeout: 60s
					);
			if (eS3Ret != SIII_NO_ERROR)
			{
				rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
						"Error #%X during phase switch.\n",
						(INT)eS3Ret
				);
			}
		}
		else
		{
			S3SM_VERBOSE(0, "Already in phase 4\n");
		}
		return(S3SM_NO_ERROR);
		/*lint -save -e527 */
		break;
		/*lint -restore */

		// Go back to CP0
	case '0':
		if (SIII_GetSercosPhase(prS3Instance) > SIII_PHASE_CP0)
		{

			eS3Ret = SIII_PhaseSwitch
					(
							prS3Instance,   // SIII instance
							SIII_PHASE_CP0, // CP0
							0,              // no retries
							30              // timeout: 30s
					);

			if (eS3Ret != SIII_NO_ERROR)
			{
				rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
						"Error #%X during phase switch.\n",
						(INT)eS3Ret
				);
			}
		}
		else
		{
			rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
					"Does not make sense, not yet in a higher phase "
					"than CP0.\n"
			);
		}
		break;

		// Go back to NRT from CP0
	case 'n':
		// According to Sercos spec not allowed from other phases than CP0.
		// Also blocked by CoSeMa
		if (SIII_GetSercosPhase(prS3Instance) == SIII_PHASE_CP0)
		{
			eS3Ret = SIII_PhaseSwitch
					(
							prS3Instance,   // SIII instance
							SIII_PHASE_NRT, // NRT
							0,              // no retries
							30              // timeout: 30s
					);

			if (eS3Ret != SIII_NO_ERROR)
			{
				rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
						"Error #%X during phase switch.\n",
						(INT)eS3Ret
				);
			}
		}
		else
		{
			rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX "Only allowed when in CP0\n");
		}
		break;

		// Clear errors on slave
	case 'e':

		if (SIII_GetSercosPhase(prS3Instance) >= SIII_PHASE_CP2)
		{
			eS3Ret = SIII_SVCClearErrors
					(
							prS3Instance,
							999			// ALL Devices
					);

			if (eS3Ret != SIII_NO_ERROR)
			{
				rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
						"Error #%X during SVC access.\n",
						(INT)eS3Ret
				);
			}
		}
		else
		{
			rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX "Only possible in CP2 or higher\n");
		}
		return(S3SM_NO_ERROR);
		/*lint -save -e527 */
		break;
		/*lint -restore */


	}
	return(S3SM_NO_ERROR);
}

static int sercos_cycle_conf_func(const hal_funct_args_t *fa)
{
	const int argc = fa_argc(fa);
	const char **argv = fa_argv(fa);
	int i;
	int iRet = 0;

	for (i = 0; i < argc; i++)
		rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX "sercos-conf argv[%d] = \"%s\"\n",
				i,argv[i]);

	iRet = sercos_handle_conf(&(s3sm_hal_data->rS3Instance), argv, argc);
	if (iRet > S3SM_END_ERR_CLASS_00000)
	{
		rtapi_print_msg(RTAPI_MSG_INFO,S3SM_MSG_PFX
				"Error in sercos-conf, code: 0x%x\n",
				(INT) iRet
		);
	}
	*(s3sm_hal_data->active_slaves) = SIII_GetNoOfSlaves(&(s3sm_hal_data->rS3Instance));
	*(s3sm_hal_data->act_phase) = SIII_GetSercosPhase(&(s3sm_hal_data->rS3Instance));
	return argc;
}

/* main */
int rtapi_app_main(void)
{
	rtapi_set_msg_level(5);
	SIII_COMM_PARS_STRUCT rS3Pars;
	int iRet = 0;
	int	i=0;

	// connect to the HAL
	if ((comp_id = hal_init (S3SM_MODULE_NAME)) < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,S3SM_MSG_PFX "hal_init() failed\n");
		return -1;
	}

	// allocate shared memory for hal data
	s3sm_hal_data = hal_malloc(sizeof(s3sm_hal_data_t));
	if (s3sm_hal_data == 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
				S3SM_MSG_PFX "ERROR: hal_malloc() failed\n");
		hal_exit(comp_id);
		return -1;
	}

	// export the pin(s)
	iRet = hal_pin_bit_newf(HAL_IN, &(s3sm_hal_data->power_on),
			comp_id, "s3sm.power_on");
	if (iRet < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
				S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
		hal_exit(comp_id);
		return -1;
	}
	iRet = hal_pin_float_newf(HAL_OUT, &(s3sm_hal_data->jitter_us),
			comp_id, "s3sm.jitter_us");
	if (iRet < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
				S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
		hal_exit(comp_id);
		return -1;
	}
	iRet = hal_pin_u32_newf(HAL_OUT, &(s3sm_hal_data->act_phase),
			comp_id, "s3sm.act_phase");
	if (iRet < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
				S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
		hal_exit(comp_id);
		return -1;
	}
	iRet = hal_pin_u32_newf(HAL_OUT, &(s3sm_hal_data->active_slaves),
			comp_id, "s3sm.active_slaves");
	if (iRet < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
				S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
		hal_exit(comp_id);
		return -1;
	}
	// for each slave
	for (i=0;i<=2;i++){
		iRet = hal_pin_float_newf(HAL_OUT, &(s3sm_hal_data->slave[i].torque),
				comp_id, "s3sm.%i.torque",i);
		if (iRet < 0) {
			rtapi_print_msg(RTAPI_MSG_ERR,
					S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
			hal_exit(comp_id);
			return -1;
		}
		iRet = hal_pin_float_newf(HAL_OUT, &(s3sm_hal_data->slave[i].pos),
				comp_id, "s3sm.%i.pos",i);
		if (iRet < 0) {
			rtapi_print_msg(RTAPI_MSG_ERR,
					S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
			hal_exit(comp_id);
			return -1;
		}
		iRet = hal_pin_float_newf(HAL_IN, &(s3sm_hal_data->slave[i].commanded_pos),
				comp_id, "s3sm.%i.commanded_pos",i);
		if (iRet < 0) {
			rtapi_print_msg(RTAPI_MSG_ERR,
					S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
			hal_exit(comp_id);
			return -1;
		}
		iRet = hal_pin_float_newf(HAL_IN, &(s3sm_hal_data->slave[i].commanded_vel),
				comp_id, "s3sm.%i.commanded_vel",i);
		if (iRet < 0) {
			rtapi_print_msg(RTAPI_MSG_ERR,
					S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
			hal_exit(comp_id);
			return -1;
		}iRet = hal_pin_float_newf(HAL_OUT, &(s3sm_hal_data->slave[i].vel),
				comp_id, "s3sm.%i.vel",i);
		if (iRet < 0) {
			rtapi_print_msg(RTAPI_MSG_ERR,
					S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
			hal_exit(comp_id);
			return -1;
		}
		iRet = hal_pin_bit_newf(HAL_OUT, &(s3sm_hal_data->slave[i].slave_ready),
				comp_id, "s3sm.%i.slave_ready",i);
		if (iRet < 0) {
			rtapi_print_msg(RTAPI_MSG_ERR,
					S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
			hal_exit(comp_id);
			return -1;
		}
		iRet = hal_pin_bit_newf(HAL_IN, &(s3sm_hal_data->slave[i].op_mode_vel),
				comp_id, "s3sm.%i.op_mode_vel",i);
		if (iRet < 0) {
			rtapi_print_msg(RTAPI_MSG_ERR,
					S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
			hal_exit(comp_id);
			return -1;
		}
		iRet = hal_pin_bit_newf(HAL_OUT, &(s3sm_hal_data->slave[i].slave_power_on),
				comp_id, "s3sm.%i.slave_power_on",i);
		if (iRet < 0) {
			rtapi_print_msg(RTAPI_MSG_ERR,
					S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
			hal_exit(comp_id);
			return -1;
		}
		iRet = hal_pin_bit_newf(HAL_OUT, &(s3sm_hal_data->slave[i].slave_error),
				comp_id, "s3sm.%i.slave_error",i);
		if (iRet < 0) {
			rtapi_print_msg(RTAPI_MSG_ERR,
					S3SM_MSG_PFX "Export pin failed with err=%i\n",iRet);
			hal_exit(comp_id);
			return -1;
		}

	}

	// Initalize Instance for Sercos 3 softmaster
	rS3Pars.ulCycleTime          = S3SM_CYCLE_TIME;
	rS3Pars.ulCycleTimeCP0CP2    = S3SM_CYCLE_TIME;
	rS3Pars.ulCycleTimeCP0       = S3SM_CYCLE_TIME;
	rS3Pars.ulUCCBandwidth       = S3SM_UC_BANDWIDTH;
	rS3Pars.usAccTelLosses       = S3SM_ACCEPTED_TEL_LOSSES;
	rS3Pars.usMTU                = S3SM_MTU;
	rS3Pars.usS3TimingMethod     = S3SM_SIII_TIMING_METHOD;
	rS3Pars.usSVCBusyTimeout     = S3SM_SVC_BUSY_TIMEOUT;
	rS3Pars.eComVersion          = S3SM_COM_VERSION;
	rS3Pars.boDetectSlaveConfig  = S3SM_DETECT_SLAVE_CONFIG;
	rS3Pars.boClrErrOnStartup    = S3SM_CLEAR_ERR_ON_STARTUP;
	rS3Pars.ulSwitchBackDelay    = S3SM_SWITCH_BACK_DELAY;
	rS3Pars.ulSoftMasterJitterNs = S3SM_SOFT_MASTER_JITTER_NS;

	iRet = SIII_Init(&(s3sm_hal_data->rS3Instance),0,&rS3Pars);
	if (iRet != SIII_NO_ERROR)
	{
		rtapi_print_msg(RTAPI_MSG_ERR,
				S3SM_MSG_PFX "could not initialize S3Instance err=%i\n",iRet);
		hal_exit(comp_id);
		return -1;
	}

	// init thread for sercos cycle
	iRet = hal_create_thread("sercos-cycle", S3SM_CYCLE_TIME, 0, 1);
	if (iRet < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
				S3SM_MSG_PFX "failed to create %d nsec sercos  cycle thread\n",
				S3SM_CYCLE_TIME);
		return -1;
	}
	// export realtime function that do the sercos cycle
	hal_export_xfunct_args_t sercos_cycle_thread_args = {
			.type = FS_XTHREADFUNC,
			.funct.x = sercos_cycle_worker_func,
			.arg = NULL,
			.uses_fp = 0,
			.reentrant = 0,
			.owner_id = comp_id
	};
	iRet = hal_export_xfunctf(&sercos_cycle_thread_args, "sercos-worker");
	if (iRet < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
				S3SM_MSG_PFX "failed to export sercos cycle function\n");
		hal_exit(comp_id);
		return -1;
	}

	//export configuration / phase handler function that does the config in user space/NRT
	hal_export_xfunct_args_t sercos_conf_func_args = {
			.type = FS_USERLAND,
			.funct.u = sercos_cycle_conf_func,
			.arg = NULL,
			.owner_id = comp_id
	};
	iRet = hal_export_xfunctf(&sercos_conf_func_args, "sercos-conf");
	if (iRet < 0) {
		rtapi_print_msg(RTAPI_MSG_ERR,
				S3SM_MSG_PFX "failed to export sercos conf function\n");
		hal_exit(comp_id);
		return -1;
	}

	// finished comp init
	rtapi_print_msg(RTAPI_MSG_INFO, S3SM_MSG_PFX "Installed Sercos 3 Softmaster\n");
	hal_ready (comp_id);
	return 0;
}

void rtapi_app_exit(void)
{
	//SIII_Close(&(s3sm_hal_data->rS3Instance));
	hal_exit(comp_id);
}
