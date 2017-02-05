/**
 * \file      SIII_CYCLIC.c
 *
 * \brief     Sercos III soft master stack - Cyclic data handling
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
 * \copyright Copyright Bosch Rexroth AG, 2012-2016
 *
 * \date      2012-10-11
 *
 * \version 2012-10-11 (GMy): Baseline for CoSeMa v4
 * \version 2012-10-31 (GMy): Updated for CoSeMa v5
 * \version 2012-11-05 (GMy): Added slave application support
 * \version 2013-04-11 (GMy): Module re-arrangement
 * \version 2013-06-20 (GMy): Optimization of error handling
 * \version 2013-11-04 (GMy): Bugfix (Hot-Plug called cycle-exact)
 * \version 2014-04-01 (GMy): Added support for CoSeMa 6VRS
 * \version 2014-04-04 (GMy): Separated function SIII_CyclicHotplugFunc()
 * \version 2014-09-12 (GMy): Improved connection handling
 * \version 2015-03-20 (GMy): Moved auto-power-on to S3SM
 * \version 2015-04-09 (GMy): Separation of SIII_Cycle() into preparation and
 *                            start function
 * \version 2015-12-08 (GMy): Support for topology change in redundancy mode
 * \version 2016-07-12 (GMy): Added SIII_SetCyclicDataValid() and
 *                            SIII_ClearCyclicDataValid() to detect cyclic data
 *                            errors more safely
 * \version 2016-10-27 (AlM): Support for CoSeMa V5 removed.
 */

//---- includes ---------------------------------------------------------------

#include "../SIII/SIII_GLOB.h"
#include "../SIII/SIII_PRIV.h"

//---- defines ----------------------------------------------------------------

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SIII_FUNC_RET SIII_Cycle(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              ULONG *pulCycleTime
 *          )
 *
 * \public
 *
 * \brief   Handling of time-critical cyclic soft master, CoSeMa, and user
 *          application functions.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[out]      pulCycleTime    Current cycle time in ns. It shall be used
 *                                  by  the calling entity to update the
 *                                  calling interval of this function.
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR             For success
 *          - SIII_PARAMETER_ERROR      for illegal function parameter
 *          - Inherited CoSeMa (CSMD_FUNC_RET) error code otherwise
 *
 * \details This function calls SIII_Cycle_Prepare() and SIII_Cycle_Start().
 *
 *          Instead of this function, the sub-functions SIII_Cycle_Prepare()
 *          and SIII_Cycle_Start() may be called directly in order to optimize
 *          the timing of the soft master.
 *
 * \note    This function shall be called from a high priority thread every
 *          Sercos cycle. Exact timing is crucial that the soft master will
 *          work.
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2012-10-11
 *
 * \version 2012-10-11 (GMy): Baseline for CoSeMa v4
 * \version 2012-10-31 (GMy): Updated for CoSeMa v5
 * \version 2012-11-05 (GMy): Added slave application support
 * \version 2012-11-12 (GMy): Slight changes (error handling)
 * \version 2013-04-11 (GMy): Module re-arrangement
 * \version 2013-08-05 (GMy): Added support for device-independent (global)
 *                            callback
 * \version 2013-08-19 (GMy): Bugfix: Corrected calling conditions for global
 *                            callback
 * \version 2013-11-04 (GMy): Bugfix (Hot-Plug called cycle-exact)
 * \version 2015-04-09 (GMy): Separation into preparation and start function
 */
SIII_FUNC_RET SIII_Cycle
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      ULONG *pulCycleTime
    )
{
  SIII_FUNC_RET   eS3FuncRet;                   // Temporary error code
  SIII_FUNC_RET   eErrorCode  = SIII_NO_ERROR;  // Error code to be returned

  SIII_VERBOSE(3, "SIII_Cycle()\n");

  if  (
      (prS3Instance   == NULL)  ||
      (pulCycleTime   == NULL)
    )
  {
    return(SIII_PARAMETER_ERROR);
  }

  // Preparation of Sercos cycle
  // Introduces additional jitter if called directly after the timer event
  // and before SIII_Cycle_Start(), but is easier to handle this way.
  eS3FuncRet = SIII_Cycle_Prepare
      (
        prS3Instance
      );

  if (eS3FuncRet != SIII_NO_ERROR)
  {
    eErrorCode = eS3FuncRet;
    SIII_VERBOSE
        (
          1,
          "Error: SIII_Cycle_Prepare() returned 0x%X!\n",
          eS3FuncRet
        );
  }

  // Start of Sercos cycle
  eS3FuncRet = SIII_Cycle_Start
      (
        prS3Instance,
        pulCycleTime
      );

  if (eS3FuncRet != SIII_NO_ERROR)
  {
    eErrorCode = eS3FuncRet;
    SIII_VERBOSE
        (
          1,
          "Error: SIII_Cycle_Start() returned 0x%X!\n",
          eS3FuncRet
        );
  }

  return (eErrorCode);
}

/**
 * \fn SIII_FUNC_RET SIII_Cycle_Prepare(
 *          SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \public
 *
 * \brief   Prepares Sercos cycle. This function is called by SIII_Cycle(),
 *          but it may also called directly by the application in order to
 *          optimize Sercos cycle timing.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR             For success
 *          - SIII_PARAMETER_ERROR      For parameter error
 *          - Inherited SICE (SICE_FUNC_RET) error code otherwise
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2015-04-09
 *
 * \version 2015-04-09 (GMy): Created from SIII_Cycle()
 * \version 2016-07-12 (GMy): Added support for cyclic data validity flag
 */
SIII_FUNC_RET SIII_Cycle_Prepare
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  SIII_FUNC_RET   eS3FuncRet;    // Temporary error code
  SICE_FUNC_RET   eSiceFuncRet;  // Temporary error code

  SIII_VERBOSE(3, "SIII_Cycle_Prepare()\n");

  if  (prS3Instance   == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  // Preparation of Sercos cycle of SICE
  eSiceFuncRet = SICE_Cycle_Prepare
      (
        &prS3Instance->rSiceInstance
      );
  //Re-set cyclic data validity flags

  eS3FuncRet = SIII_ClearCyclicDataValid(prS3Instance);
  if (eS3FuncRet != SIII_NO_ERROR)
  {
    SIII_VERBOSE(0, "Error: Could not re-set cyclic data flags!");
    return(SIII_MEM_ERROR);
  }


  return((SIII_FUNC_RET)eSiceFuncRet);

}

/**
 * \fn SIII_FUNC_RET SIII_Cycle_Start(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              ULONG *pulCycleTime
 *          )
 *
 * \public
 *
 * \brief   Handling of time-critical cyclic soft master, CoSeMa, and user
 *          application functions.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[out]      pulCycleTime    Current cycle time in ns. It shall be used
 *                                  by  the calling entity to update the
 *                                  calling interval of this function.
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR             For success
 *          - SIII_PARAMETER_ERROR      for illegal function parameter
 *          - Inherited CoSeMa (CSMD_FUNC_RET) error code otherwise
 *
 * \details This function manages the calls of SICE_Cycle(), CSMD_ReadAT(),
 *          CSMD_WriteMDT(), and CSMD_TxRxSoftCont(). The more, it controls
 *          cyclic transmissions and the C-CON (C-CON toggle bit, producer
 *          ready bit). The device-specific function pointer array
 *          afpAppCyclic and the global one fpAppCyclicGlob within the SIII
 *          instance structure are used to call application-layer cyclic
 *          functions. This function is called by SIII_Cycle(), but it may also
 *          called directly by the application in order to optimize Sercos
 *          cycle timing.
 *
 * \note    This function shall be called from a high priority thread every
 *          Sercos cycle. Exact timing is crucial that the soft master will
 *          work.
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2015-04-09
 *
 * \version 2015-04-09 (GMy): Created from SIII_Cycle()
 * \version 2015-06-11 (GMy): Added preliminary UCC support
 * \version 2016-07-12 (GMy): Added support for cyclic data validity flag
 */
SIII_FUNC_RET SIII_Cycle_Start
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      ULONG *pulCycleTime
    )
{
  INT             iCnt;
  USHORT          usI;
  SIII_FUNC_RET   eErrorCode    = SIII_NO_ERROR;  // Error code to be returned
  CSMD_FUNC_RET   eCsmdFuncRet  = CSMD_NO_ERROR;  // Temporary error code
  SICE_FUNC_RET   eSiceFuncRet  = SICE_NO_ERROR;  // Temporary error code
  SIII_FUNC_RET   eS3FuncRet    = SIII_NO_ERROR;  // Temporary error code
  USHORT*         pusC_Con;

  SIII_VERBOSE(3, "SIII_Cycle_Start()\n");

  if (
      (prS3Instance   == NULL)  ||
      (pulCycleTime   == NULL)
    )
  {
    return(SIII_PARAMETER_ERROR);
  }

  // Sercos III IP core emulation
  eSiceFuncRet = SICE_Cycle_Start
      (
        &prS3Instance->rSiceInstance,   // SICE instance
        pulCycleTime                    // cycle time
      );

  if (eSiceFuncRet != SICE_NO_ERROR)
  {
    eErrorCode = (SIII_FUNC_RET)eSiceFuncRet;
  }

#if (defined SICE_UC_CHANNEL) && !(defined SICE_USE_NIC_TIMED_TX)

  // TODO wait for UC interval start

  eSiceFuncRet = SICE_UCC_Cycle
      (
        &prS3Instance->rSiceInstance,          // SICE instance
        prS3Instance->rS3Pars.ulUCCBandwidth   // TODO: take packet jitter into account
      );

  if (eSiceFuncRet != SICE_NO_ERROR)
  {
    eErrorCode = (SIII_FUNC_RET)eSiceFuncRet;
  }

#endif

  eCsmdFuncRet = CSMD_CyclicHandling
      (
        &prS3Instance->rCosemaInstance    // CoSeMa instance
      );
/*
  if (
      (eCsmdFuncRet                                    != CSMD_NO_ERROR)          &&
      (eCsmdFuncRet                                    != CSMD_TOPOLOGY_CHANGE)   &&
      (prS3Instance->rCyclicCommCtrl.boCyclicDataError == FALSE)
    ) AlM: Ignore errors from CSMD_CyclicHandling() except for CSMD_TEL_ERROR_OVERRUN */
  if (
       (eCsmdFuncRet == CSMD_TEL_ERROR_OVERRUN)                   &&
       (prS3Instance->rCyclicCommCtrl.boCyclicDataError == FALSE)
     )
  {
    prS3Instance->rCyclicCommCtrl.boCyclicDataError = TRUE;
    eErrorCode    = (SIII_FUNC_RET)eCsmdFuncRet;
    SIII_VERBOSE
        (
          0,
          "\nCSMD_CyclicHandling() Error %x\n",
          (INT)eCsmdFuncRet
        );
  }
  else if (eCsmdFuncRet == CSMD_TOPOLOGY_CHANGE)
  {
    (VOID)SIII_UpdateTopology(prS3Instance);
  }
  else if (eCsmdFuncRet == CSMD_NO_ERROR)
  {
    prS3Instance->rCyclicCommCtrl.boCyclicDataError = FALSE;
  }

  if (
       (prS3Instance->rCyclicCommCtrl.boAppDataValid)        &&
       (SIII_GetSercosPhase(prS3Instance) == SIII_PHASE_CP4)
     )
  {
    eS3FuncRet = SIII_GetConnections(prS3Instance);

    if (eS3FuncRet != SIII_NO_ERROR)
    {
      eErrorCode = eS3FuncRet;
    }
  }

  // Call functions for cyclic user data, if needed
  if (prS3Instance->rCyclicCommCtrl.boAppDataValid)
  {
    // Call callbacks only if device power on
    //if (prS3Instance->rCyclicCommCtrl.boPowerOn)
    //{

      // Call global cyclic callback, if needed
      if (prS3Instance->fpAppCyclicGlob != NULL)
      {
        prS3Instance->fpAppCyclicGlob(prS3Instance);
      }

      // Call application-specific cyclic function via pointer
      for (
          iCnt = 0;
          iCnt < SIII_MAX_SLAVES;
          iCnt++
          )
      {
        if (prS3Instance->afpAppCyclic[iCnt] != NULL)
        {
          prS3Instance->afpAppCyclic[iCnt]
              (
                prS3Instance,
                (USHORT)iCnt
              );
        }
      }

    //}
    // Activate master producer connections
    for (iCnt = 0; iCnt < SIII_MAX_SLAVES; iCnt++)
    {
      if (prS3Instance->rCosemaInstance.rSlaveList.aeSlaveActive[iCnt] ==
          CSMD_SLAVE_ACTIVE)
      {
        /* check if cyclic data for this slave is valid in current cycle */
        if (prS3Instance->rCyclicCommCtrl.aboCycDataValid[iCnt])
        {
          /* set producer ready in all C-CON words of this slave's MDT connections */
          for (usI = 0; usI < prS3Instance->rCosemaInstance.rConfiguration.parSlaveConfig[iCnt].usNbrOfConnections; usI++)
          {
            /* check if connection is configured */
            if (prS3Instance->arConnInfoMDT[iCnt][usI].usConnIdx != 0xFFFF)
            {
              pusC_Con = (USHORT*)&prS3Instance->aucCyclicMDTBuffer[prS3Instance->arConnInfoMDT[iCnt][usI].usOffset];
              *pusC_Con |= (USHORT) SIII_C_CON_PROD_RDY;

              SIII_VERBOSE(2, "Slave active: %i \n", iCnt);
            }
          }
        }
        else
        {
          for (usI = 0; usI < prS3Instance->rCosemaInstance.rConfiguration.parSlaveConfig[iCnt].usNbrOfConnections; usI++)
          {
            /* check if connection is configured */
            if (prS3Instance->arConnInfoMDT[iCnt][usI].usConnIdx != 0xFFFF)
            {
              pusC_Con = (USHORT*)&prS3Instance->aucCyclicMDTBuffer[prS3Instance->arConnInfoMDT[iCnt][usI].usOffset];
              *pusC_Con &= (USHORT) ~SIII_C_CON_PROD_RDY;

              SIII_VERBOSE(2, "Slave active: %i \n", iCnt);
            }
          }
        } /* if (prS3Instance->rCyclicCommCtrl.aboCycDataValid[iCnt]) */
      } /* if (prS3Instance->rCosemaInstance.rSlaveList.aeSlaveActive[iCnt] == CSMD_SLAVE_ACTIVE) */
    } /* for (iCnt = 0; iCnt < SIII_MAX_SLAVES; iCnt++) */

    if (SIII_GetSercosPhase(prS3Instance) == SIII_PHASE_CP4)
    {
      eS3FuncRet = SIII_SetConnections(prS3Instance);

      if (eS3FuncRet != SIII_NO_ERROR)
      {
        eErrorCode = eS3FuncRet;
        SIII_VERBOSE
            (
              0,
              "SIII_SetConnections() Error %x\n",
              (INT)eS3FuncRet
            );
      }
    }
  }

#ifdef CSMD_HOTPLUG

  eS3FuncRet = SIII_CyclicHotplugFunc(prS3Instance);
  if (eS3FuncRet != SIII_NO_ERROR)
  {
    eErrorCode = eS3FuncRet;
  }

#endif

  // Call CSMD_TxRxSoftCont, if needed
  if (prS3Instance->rCyclicCommCtrl.boCallTxRxSoftCont)
  {
    // Optimization possible: Only call in case of SVC use
    eCsmdFuncRet = CSMD_TxRxSoftCont(&prS3Instance->rCosemaInstance);
    if (eCsmdFuncRet != CSMD_NO_ERROR)
    {
      SIII_VERBOSE
          (
            0,
            "TxRxSoftCont() Error %x\n",
            (INT) eCsmdFuncRet
          );
      eErrorCode = (SIII_FUNC_RET) eCsmdFuncRet;
    }
  }

  return(eErrorCode);
 }


/**
 * \fn SIII_FUNC_RET SIII_GetConnections(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \private
 *
 * \brief   Actual connection handling using CoSeMa 6VRS, only to be called by
 *          SIII_Cyclic().
 *
 * \note    No clean error handling yet!
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR             For success
 *          - Inherited CoSeMa (CSMD_FUNC_RET) error code otherwise
 *
 * \ingroup SIII
 *
 * \author  GMy, based on code by AlM, PROBABLY!
 *
 * \date    2014-04-02
 *
 * \version 2014-09-12 (GMy):    Improved connection handling
 */
SIII_FUNC_RET SIII_GetConnections(SIII_INSTANCE_STRUCT *prS3Instance)
{
  USHORT          usI, usK;
  USHORT          usConnIdx;
  USHORT          usState             = 0;
  USHORT*         pusConnDestination  = NULL; /* destination pointer for connection data */
  CSMD_FUNC_RET   eCsmdFuncRet    = CSMD_NO_ERROR;

  /* parse through all projected slaves */
  for (usI = 0; usI < prS3Instance->rCosemaInstance.rSlaveList.usNumProjSlaves; usI++)
  {
    /* parse through number of connections configured for the slave */
    for (usK = 0; usK < prS3Instance->rCosemaInstance.rConfiguration.parSlaveConfig[usI].usNbrOfConnections; usK++)
    {
      usConnIdx = prS3Instance->arConnInfoAT[usI][usK].usConnIdx;

      if (usConnIdx <= CSMD_MAX_GLOB_CONN)
      {
        /* get producer state of connection */
        eCsmdFuncRet = CSMD_GetConnectionState
            (
              &prS3Instance->rCosemaInstance,   // CoSeMa instance structure
              usConnIdx,                        // Connection index
              &usState                          // Connection state
            );

        if (eCsmdFuncRet > CSMD_END_ERR_CLASS_00000)
        {
          return((SIII_FUNC_RET)eCsmdFuncRet);
        }

        /* copy data if connection state is 'consuming' */
        if ((CSMD_CONS_STATE)usState == CSMD_CONS_STATE_CONSUMING)
        {
          /* read data offset from SIII connection info structure for AT */
          pusConnDestination = (USHORT*)&prS3Instance->aucCyclicATBuffer[prS3Instance->arConnInfoAT[usI][usK].usOffset];

          SIII_VERBOSE
              (
                2,
                "local AT data offset: %d\n",
                prS3Instance->arConnInfoAT[usI][usK].usOffset
              );

          eCsmdFuncRet = CSMD_GetConnectionData
              (
                &prS3Instance->rCosemaInstance,   // CoSeMa instance structure
                usConnIdx,                        // Connection index
                pusConnDestination                // Data buffer
              );

          if (eCsmdFuncRet > CSMD_END_ERR_CLASS_00000)
          {
            // Forward error
            return((SIII_FUNC_RET)eCsmdFuncRet);
          }
        }
        /* always clear occurring connection errors */
        else if ((CSMD_CONS_STATE)usState == CSMD_CONS_STATE_ERROR)
        {
          eCsmdFuncRet = CSMD_ClearConnectionError
              (
                &prS3Instance->rCosemaInstance,
                usConnIdx
              );

          if (eCsmdFuncRet > CSMD_END_ERR_CLASS_00000)
          {
            // Forward error
            //return((SIII_FUNC_RET)eCsmdFuncRet);
          }
        }
      } /* if (usConnIdx <= CSMD_MAX_GLOB_CONN) */
    } /* for (usK = 0; usK < prS3Instance->rCosemaInstance.rConfiguration.parSlaveConfig[usI].usNbrOfConnections; usK++) */
  } /* for (usI = 0; usI < prS3Instance->rCosemaInstance.rSlaveList.usNumProjSlaves; usI++) */

  return(SIII_NO_ERROR);
}

/**
 * \fn SIII_FUNC_RET SIII_SetConnections(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \private
 *
 * \brief   Command connection handling using CoSeMa 6VRS, only to be called by
 *          SIII_Cyclic().
 *
 * \note    No clean error handling yet!
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR             For success
 *          - Inherited CoSeMa (CSMD_FUNC_RET) error code otherwise
 *
 * \ingroup SIII
 *
 * \author GMy, based on code by AlM
 *
 * \date 2014-04-02
 *
 * \version 2014-09-12 (GMy): Improved connection handling
 * \version 2016-07-12 (GMy): Added support for cyclic data validity flag
 */
SIII_FUNC_RET SIII_SetConnections(SIII_INSTANCE_STRUCT *prS3Instance)
{
  USHORT          usI, usK;
  USHORT          usC_Con;
  USHORT          usConnIdx;
  USHORT*         pusConnSource   = NULL; /* pointer to connection data to be copied */
  CSMD_FUNC_RET   eCsmdFuncRet    = CSMD_NO_ERROR;

  /* parse through all projected slaves */
  for (usI = 0; usI < prS3Instance->rCosemaInstance.rSlaveList.usNumProjSlaves; usI++ )
  {
    /* parse through number of connections configured for the slave */
    for (usK = 0; usK < prS3Instance->rCosemaInstance.rConfiguration.parSlave_Config[usI].usNbrOfConnections; usK++)
    {
      usConnIdx = prS3Instance->arConnInfoMDT[usI][usK].usConnIdx;

      if (usConnIdx <= CSMD_MAX_GLOB_CONN)
      {
        /* read data offset from SIII connection info structure for MDT */
        pusConnSource = (USHORT*)&prS3Instance->aucCyclicMDTBuffer[prS3Instance->arConnInfoMDT[usI][usK].usOffset];

        SIII_VERBOSE
            (
              2,
              "local MDT data offset: %d\n",
              prS3Instance->arConnInfoMDT[usI][usK].usOffset
            );

        usC_Con = *pusConnSource;

        /* check C-CON of connection (written by application) */
        if ((usC_Con & CSMD_C_CON_PRODUCER_READY) != 0)
        {
          if ((usC_Con & CSMD_C_CON_FLOW_CONTROL) != 0)
          {
            /* producer ready is set and flow control is set: state ==> 'stopping' */
            eCsmdFuncRet = CSMD_SetConnectionState
                (
                  &prS3Instance->rCosemaInstance,   // CoSeMa instance structure
                  usConnIdx,                        // Connection instance
                  CSMD_PROD_STATE_STOPPING          // New state
                );

            if (eCsmdFuncRet > CSMD_END_ERR_CLASS_00000)
            {
              // Forward error
              return((SIII_FUNC_RET)eCsmdFuncRet);
            }
          }
          else
          {
            /* producer ready is set and flow control is not set: state ==> 'ready' */
            eCsmdFuncRet = CSMD_SetConnectionState
                (
                  &prS3Instance->rCosemaInstance,   // CoSeMa instance structure
                  usConnIdx,                        // Connection instance
                  CSMD_PROD_STATE_READY             // New state
                );

            if (eCsmdFuncRet > CSMD_END_ERR_CLASS_00000)
            {
              // Forward error
              return((SIII_FUNC_RET)eCsmdFuncRet);
            }

            /* increment pointer because connection is copied without C-CON */
            pusConnSource++;

            /* check if cyclic data is valid for this slave in current cycle */
            if (prS3Instance->rCyclicCommCtrl.aboCycDataValid[usI])
            {
              eCsmdFuncRet = CSMD_SetConnectionData
                  (
                    &prS3Instance->rCosemaInstance,   // CoSeMa instance structure
                    usConnIdx,                        // Connection index
                    pusConnSource,                    // Data buffer
                    usC_Con                           // C-CON
                  ); /* Real-time bits */

              if (eCsmdFuncRet > CSMD_END_ERR_CLASS_00000)
              {
                // Forward error
                return ((SIII_FUNC_RET)eCsmdFuncRet);
              }
            }
          } /* if ((usC_Con & CSMD_C_CON_FLOW_CONTROL) != 0) */
        } /* if ((usC_Con & CSMD_C_CON_PRODUCER_READY) != 0) */
      } /* if (usConnIdx <= CSMD_MAX_GLOB_CONN) */
    } /* for (usK = 0; usK < prS3Instance->rCosemaInstance.rConfiguration.parSlave_Config[usI].usNbrOfConnections; usK++) */
  } /* for (usI = 0; usI < prS3Instance->rCosemaInstance.rSlaveList.usNumProjSlaves; usI++ ) */

  return(SIII_NO_ERROR);
}


/**
 * \fn SIII_FUNC_RET SIII_CyclicHotplugFunc(
 *              SIII_INSTANCE_STRUCT *prS3Instance
 *          )
 *
 * \private
 *
 * \brief   Performs cyclic hotplug functionality.
 *
 * \note    The function shall only be called in Sercos phase CP4.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:            Success
 *          - Inherited CoSeMa (CSMD_FUNC_RET) error code otherwise
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2014-04-04
 */
SIII_FUNC_RET SIII_CyclicHotplugFunc
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  CSMD_FUNC_RET   eCsmdFuncRet  = CSMD_NO_ERROR;

  // Call CSMD_HotPlug, if in cyclic hotplug phase
  if (prS3Instance->rCyclicCommCtrl.boHotplugCyclicPhase)
  {
    eCsmdFuncRet = CSMD_HotPlug
        (
          &prS3Instance->rCosemaInstance,       // CoSeMa state machine state
          &prS3Instance->rCosemaFuncState,      // CoSeMa state machine state
          prS3Instance->ausCosemaHPDevAddList,  // Hotplug slave list
          FALSE                                 // Start, not cancel
        );

    SIII_VERBOSE
        (
          1,
          "CSMD_Hotplug - state = %i, return value = 0x%x\n",
          prS3Instance->rCosemaFuncState.usActState,
          eCsmdFuncRet
        );

    if (
        (eCsmdFuncRet != CSMD_FUNCTION_IN_PROCESS)  &&
        (eCsmdFuncRet != CSMD_NO_ERROR)         // Hotplug error
      )
    {
      SIII_VERBOSE(1, "Hotplug error in cyclic function\n");

      prS3Instance->rCyclicCommCtrl.eCyclicCsmdError = eCsmdFuncRet;
      prS3Instance->rCyclicCommCtrl.boHotplugCyclicPhase = FALSE;
    }
    else if (eCsmdFuncRet == CSMD_NO_ERROR)         // Cyclic hotplug portion done
      // Currently, the whole state machine of CSMD_Hotplug() is called
      // in the cyclic handling function.
    {
      prS3Instance->rCyclicCommCtrl.eCyclicCsmdError = CSMD_FUNCTION_IN_PROCESS;
      prS3Instance->rCyclicCommCtrl.boHotplugCyclicPhase = FALSE;
    }
  }
  return((SIII_FUNC_RET) eCsmdFuncRet);
}

/**
 * \fn SIII_FUNC_RET SIII_DevicePower(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT usDevIdx,
 *              BOOL boPower
 *          )
 *
 * \public
 *
 * \brief   Switches Sercos slave device power on or off using the control word
 *          in the command data.
 *
 * \note    The function shall only be called in Sercos phase CP4.
 *
 * \attention   Danger! This function shall only be used if fixed connection
 *              configuration is used, or if the device control word is mapped
 *              as first parameter in cyclic command data (MDT data)! Otherwise
 *              unwanted machine movement may occur!
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       usDevIdx        Device index or 'SIII_ALL_DEVICES'
 * \param[in]       boPower         Should power be switched on or off?
 *                                  - 'TRUE' for switching power on
 *                                  - 'FALSE' for switching power off
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:            Success
 *          - SIII_SERCOS_PHASE_ERROR:  Sercos phase lower than CP4
 *          - SIII_DEVICE_IDX_ERROR:    Invalid device index
 *          - SIII_PARAMETER_ERROR:     Other function parameter error
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-03-08
 *
 * \version 2013-04-11 (GMy): Module re-arrangement
 * \version 2016-11-08 (AlM): support of more than two connections for one slave
 */
SIII_FUNC_RET SIII_DevicePower
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT usDevIdx,
      BOOL boPower
    )
{
  SIII_FUNC_RET eRet = SIII_NO_ERROR;
  INT           iCnt;
  USHORT*       pusControlWord;
  USHORT        usConnIdx;

  SIII_VERBOSE(3, "SIII_DevicePower()\n");

  if (prS3Instance  == NULL)
  {
    eRet = (SIII_PARAMETER_ERROR);
  }
  else if (SIII_GetSercosPhase(prS3Instance) != SIII_PHASE_CP4)
  {
    eRet = (SIII_SERCOS_PHASE_ERROR);
  }
  else
  {
    if (
        (usDevIdx == (USHORT) SIII_ALL_DEVICES)   ||
        (usDevIdx < (USHORT) SIII_MAX_SLAVES)
       )
    {
      if (usDevIdx == (USHORT) SIII_ALL_DEVICES)
      {
        for (iCnt = 0; iCnt < prS3Instance->rCosemaInstance.rSlaveList.usNumProjSlaves; iCnt++ )
        {
          eRet = SIII_GetControlIdx(
                     prS3Instance,
                     iCnt,
                    &usConnIdx
                  );

          if (eRet == SIII_NO_ERROR)
          {
            /* a connection containing S-0-0134 has been found */
            pusControlWord = (USHORT*)&prS3Instance->aucCyclicMDTBuffer[prS3Instance->arConnInfoMDT[iCnt][usConnIdx].usOffset + 2];

            if (boPower)
            {
              *pusControlWord = SIII_SLAVE_ENABLE;
              prS3Instance->rCyclicCommCtrl.boPowerOn = TRUE;
            }
            else
            {
              *pusControlWord = SIII_SLAVE_DISABLE;
              prS3Instance->rCyclicCommCtrl.boPowerOn = FALSE;
            }
          }
        }
      }
      else
      {
        eRet = SIII_GetControlIdx(
                   prS3Instance,
                   usDevIdx,
                  &usConnIdx
                );

        pusControlWord = (USHORT*)&prS3Instance->aucCyclicMDTBuffer[prS3Instance->arConnInfoMDT[usDevIdx][usConnIdx].usOffset + 2];

        if (boPower)
        {
          *pusControlWord = SIII_SLAVE_ENABLE;
          prS3Instance->rCyclicCommCtrl.boPowerOn = TRUE;
        }
        else
        {
          *pusControlWord = SIII_SLAVE_DISABLE;
          prS3Instance->rCyclicCommCtrl.boPowerOn = FALSE;
        }
      }
    }
    else
    {
      eRet = (SIII_DEVICE_IDX_ERROR);
    }
  }

  return (eRet);
}

/**
 * SIII_FUNC_RET SIII_GetDeviceCyclicDataPtr(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT  usDevIdx,
 *              SIII_DEV_DATA eMDTorAT,
 *              USHORT   usConnIdx,
 *              USHORT** ppusBuffer,
 *              USHORT*  pusLength
 *          )
 *
 * \brief   Returns a pointer to the cyclic data image of a Sercos slave
 *          device, either command or actual data. The returned pointer is NULL
 *          in case of an error.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       usDevIdx        Sercos slave device index
 * \param[in]       eMDTorAT
 *                                  - SIII_DEV_MDT_DATA for MDT
 *                                  - SIII_DEV_AT_DATA for AT
 * \param[in]       usConnIdx       Slave specific connection index in telegram
 *                                  selected via eMDTorAT
 * \param[out]      ppusBuffer      Pointer to pointer to cyclic data image that is returned
 * \param[out]      pusLength       Pointer to length of desired connection that is returned
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:            Success
 *          - SIII_DEVICE_IDX_ERROR:    Invalid device index
 *          - SIII_PARAMETER_ERROR:     Other function parameter error
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2013-02-05
 *
 * \version 2013-04-11 (GMy): Module re-arrangement
 * \version 2016-07-12 (GMy): Added connection index as function parameters for
 *                            future use
 * \version 2016-11-08 (AlM): Support for more than two connections for one slave
 */
SIII_FUNC_RET SIII_GetDeviceCyclicDataPtr
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT        usDevIdx,
      SIII_DEV_DATA eMDTorAT,
      USHORT        usConnIdx,
      USHORT**      ppusBuffer,
      USHORT*       pusLength
    )
{
  SIII_VERBOSE(3, "SIII_GetDeviceCyclicDataPtr()\n");

  if  (
      (prS3Instance   == NULL)  ||
      (ppusBuffer    == NULL)
    )
  {
    return(SIII_PARAMETER_ERROR);
  }

  if  (usDevIdx < (USHORT) SIII_MAX_SLAVES)
  {
    switch (eMDTorAT)
    {
      case SIII_DEV_MDT_DATA:
        *ppusBuffer = (USHORT*)&prS3Instance->aucCyclicMDTBuffer[prS3Instance->arConnInfoMDT[usDevIdx][usConnIdx].usOffset];
        *pusLength  = prS3Instance->arConnInfoMDT[usDevIdx][usConnIdx].usLength;
        break;
      case SIII_DEV_AT_DATA:
        *ppusBuffer = (USHORT*)&prS3Instance->aucCyclicATBuffer[prS3Instance->arConnInfoAT[usDevIdx][usConnIdx].usOffset];
        *pusLength  = prS3Instance->arConnInfoAT[usDevIdx][usConnIdx].usLength;
        break;
      default:
        *ppusBuffer = NULL;
        return(SIII_PARAMETER_ERROR);
        break;
    }
    return(SIII_NO_ERROR);
  }
  else
  {
    *ppusBuffer = NULL;
    return(SIII_DEVICE_IDX_ERROR);
  }
}

/**
 * SIII_FUNC_RET SIII_GetDeviceStatus(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT  usDevIdx,
 *              USHORT* pusStatus
 *          )
 *
 * \brief   Returns the device status (S-DEV) of a slave to an output pointer.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       usDevIdx        Sercos slave device index
 * \param[out]      pusStatus       Output pointer for device status of selected slave
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:                    Success
 *          - SIII_DEVICE_IDX_ERROR:            Invalid device index
 *          - SIII_PARAMETER_ERROR:             Other function parameter error
 *          - SIII_SERCOS_SLAVE_VALID_NOT_SET:  Slave valid not set by selected slave;
 *                                              pusStatus is invalid and must not be used
 *
 * \ingroup SIII
 *
 * \author  AlM
 *
 * \date    2016-09-06
 *
 */
SIII_FUNC_RET SIII_GetDeviceStatus
    (
        SIII_INSTANCE_STRUCT *prS3Instance,
        USHORT                usDevIdx,
        USHORT*               pusStatus
    )
{
  SIII_VERBOSE(3, "SIII_GetDeviceStatus()\n");

  if  (
      (prS3Instance   == NULL)
    )
  {
    return(SIII_PARAMETER_ERROR);
  }

  if  (usDevIdx < (USHORT) SIII_MAX_SLAVES)
  {
    if (prS3Instance->rCosemaInstance.arDevStatus[usDevIdx].usMiss == 0)
    {
      *pusStatus = prS3Instance->rCosemaInstance.arDevStatus[usDevIdx].usS_Dev;
      return(SIII_NO_ERROR);
    }
    else
    {
      *pusStatus = 0;
      return (SIII_SERCOS_SLAVE_VALID_NOT_SET);
    }
  }
  else
  {
    return(SIII_DEVICE_IDX_ERROR);
  }
}

/**
 * SIII_FUNC_RET SIII_SetCyclicDataValid(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *              USHORT usDevIdx
 *          )
 *
 * \brief   Signals validity information to SIII. To be used from cyclic
 *          callback functions.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 * \param[in]       usDevIdx        Sercos slave device index
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:            Success
 *          - SIII_DEVICE_IDX_ERROR:    Invalid device index
 *          - SIII_PARAMETER_ERROR:     Other function parameter error
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2016-07-12
 */
SIII_FUNC_RET SIII_SetCyclicDataValid
    (
      SIII_INSTANCE_STRUCT *prS3Instance,
      USHORT usDevIdx
    )
{
  INT iCnt;

  SIII_VERBOSE(3, "SIII_SetCyclicDataValid()\n");

  if  (prS3Instance   == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  if (usDevIdx == SIII_ALL_DEVICES)
  {
    for (iCnt = 0; iCnt < SIII_MAX_SLAVES; iCnt++)
    {
      prS3Instance->rCyclicCommCtrl.aboCycDataValid[iCnt] = TRUE;
    }
    return(SIII_NO_ERROR);
  }
  else if (usDevIdx < (USHORT) SIII_MAX_SLAVES)
  {
    prS3Instance->rCyclicCommCtrl.aboCycDataValid[usDevIdx] = TRUE;
    return(SIII_NO_ERROR);
  }
  else
  {
    return(SIII_DEVICE_IDX_ERROR);
  }
}

/**
 * SIII_FUNC_RET SIII_ClearCyclicDataValid(
 *              SIII_INSTANCE_STRUCT *prS3Instance,
 *          )
 *
 * \brief   Clears data validity flags for all slaves.
 *
 * \param[in,out]   prS3Instance    Pointer to SIII instance structure
 *
 * \return  See definition of SIII_FUNC_RET
 *          - SIII_NO_ERROR:            Success
 *          - SIII_PARAMETER_ERROR:     Other function parameter error
 *
 * \ingroup SIII
 *
 * \author  GMy
 *
 * \date    2016-07-12
 */
SIII_FUNC_RET SIII_ClearCyclicDataValid
    (
      SIII_INSTANCE_STRUCT *prS3Instance
    )
{
  INT iCnt;

  SIII_VERBOSE(3, "SIII_ClearCyclicDataValid()\n");

  if  (prS3Instance   == NULL)
  {
    return(SIII_PARAMETER_ERROR);
  }

  for (iCnt = 0; iCnt < SIII_MAX_SLAVES; iCnt++)
  {
    prS3Instance->rCyclicCommCtrl.aboCycDataValid[iCnt] = FALSE;
  }

  return(SIII_NO_ERROR);
}
