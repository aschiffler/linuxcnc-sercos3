/*
 * Sercos Soft Master Core Library
 * Version: see SICE_GLOB.h
 * Copyright (C) 2012 - 2016 Bosch Rexroth AG
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS"; WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY;
 * FITNESS FOR A PERTICULAR PURPOSE AND NONINFRINGEMENT. THE AUTHORS OR COPYRIGHT
 * HOLDERS SHALL NOT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE;
 * UNLESS STIPULATED BY MANDATORY LAW.
 *
 * You may contact us at open.source@boschrexroth.de if you are interested in
 * contributing a modification to the Software.
 */

/**
 * \file      SICE_SIII.c
 *
 * \brief     Sercos SoftMaster core: Basic Sercos III functions
 *
 * \ingroup   SICE
 *
 * \author    GMy, partially based on former work by SBe
 *
 * \copyright Copyright Bosch Rexroth AG, 2012-2016
 *
 * \date      2012-10-11
 *
 * \version 2013-03-22 (GMy): Moved from module SIII to module SICE
 * \version 2013-08-02 (GMy): New function SICE_CheckCycleTime()
 */

//---- includes ---------------------------------------------------------------

#include "../SICE/SICE_GLOB.h"
#include "../SICE/SICE_PRIV.h"

//---- defines ----------------------------------------------------------------

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

/** Array for pre-calculated CRC32 table */
static ULONG    aulCRC32Table[SICE_CRC_TABLE_SIZE];

//---- function declarations --------------------------------------------------

//---- function implementations -----------------------------------------------

/**
 * \fn SICE_FUNC_RET SICE_CRC32BuildTable(
 *              VOID
 *          )
 *
 * \private
 *
 * \brief   Builds table for CRC32 checksum calculation.
 *
 * \return  Always SICE_NO_ERROR
 *
 * \ingroup SICE
 *
 * \author  SBe
 *
 * \date    2006
 *
 * \version 2012-11-14 (GMy): Changed type from int to ULONG
 * \version 2013-03-22 (GMy): Code optimization, moved to module SICE
 */
SICE_FUNC_RET SICE_CRC32BuildTable
    (
      VOID
    )
{
  INT   i,j;
  ULONG ulCRC;
  ULONG ulTemp;

  SICE_VERBOSE(3, "SICE_CRC32BuildTable()\n");

  for (
      i = 0;
      i <= SICE_CRC_TABLE_SIZE - 1;
      i++
    )
  {
    ulCRC = (ULONG) i;
    for (
        j = 8;
        j >= 1;
        j--
      )
    {
      ulTemp = ulCRC / ((ULONG) 2);

      if (ulCRC & ((ULONG) 1))
      {
        ulCRC = ulTemp ^ ((ULONG)SICE_CRC_POLY);
      }
      else
      {
        ulCRC = ulTemp;
      }
    }
    aulCRC32Table[i] = ulCRC;
  }
  return(SICE_NO_ERROR);
}

/**
 * \fn ULONG SICE_CRC32Calc(
 *              UCHAR * pucBuffer,
 *              INT iSize,
 *              ULONG ulStartCRC
 *          )
 *
 * \private
 *
 * \brief   Calculates CRC32 checksum
 *
 * \param[in]   pucBuffer   Buffer for checksum calculation
 * \param[in]   iSize       Size of buffer
 * \param[in]   ulStartCRC  CRC calculation initialization value
 *
 * \return  Calculated CRC
 *
 * \ingroup SICE
 *
 * \author SBe
 *
 * \date 2006
 *
 * \version 2012-11-14 (GMy): Changed type from int to ULONG
 * \version 2013-03-22 (GMy): Code optimization, moved to module SICE
 */
ULONG SICE_CRC32Calc
    (
      UCHAR * pucBuffer,
      INT iSize,
      ULONG ulStartCRC
    )
{
  INT   i;
  ULONG ulCRC;
  ULONG ulTemp1;
  ULONG ulTemp2;

  SICE_VERBOSE(3, "SICE_CRC32Calc()\n");

  ulCRC = ulStartCRC ^ (ULONG) 0xFFFFFFFF;
  for (
      i = 0;
      i < iSize;
      i++
    )
  {
    ulTemp1 = (ulCRC / (ULONG) 256) & (ULONG) 0xFFFFFF;
    ulTemp2 = (ULONG)pucBuffer[i];
    ulTemp2 = aulCRC32Table[(ulCRC ^ ulTemp2) & (ULONG) 0xFF];
    ulCRC   = ulTemp1 ^ ulTemp2;
  }
  ulCRC = ulCRC ^ (ULONG)0xFFFFFFFF;
  return(ulCRC);
}

/**
 * \fn SICE_FUNC_RET SICE_CheckCycleTime(
 *              ULONG ulNewCycleTime,
 *              UCHAR ucPhase
 *          )
 *
 * \private
 *
 * \brief   Checks whether the cycle time is valid according to Sercos
 *          specification.
 *
 * \param[in]   ulNewCycleTime  Cycle time
 * \param[in]   ucPhase         Sercos phase
 *
 * \return  See definition of SICE_FUNC_RET
 *          - SICE_NO_ERROR                 for valid cycle time
 *          - SICE_SERCOS_CYCLE_TIME_ERROR  for illegal cycle time
 *
 * \ingroup SICE
 *
 * \author  GMy
 *
 * \date    2013-08-02
 *
 */
SICE_FUNC_RET SICE_CheckCycleTime
    (
      ULONG ulNewCycleTime,
      UCHAR ucPhase
    )
{
  SICE_VERBOSE(3, "SICE_CheckCycleTime()\n");

  // Check for reset values
  if ((ucPhase == 0) && (ulNewCycleTime == 0))
  {
    return(SICE_NO_ERROR);
  }

/*
  // Cycle times < 1ms are only allowed in CP3 and CP4
  if (  (
        (ucPhase == 0) ||
        (ucPhase == 1) ||
        (ucPhase == 2)
      )                   &&
      (ulNewCycleTime < ((ULONG) CSMD_TSCYC_1_MS))
    )
  {
    return(SICE_SERCOS_CYCLE_TIME_INVALID);
  }
*/

  if (ulNewCycleTime >= ((ULONG) CSMD_TSCYC_250_US))
  {
    // Cycle time must be multiple of 250us and not greater than maximum
    // cycle time
    if  (
        ((ulNewCycleTime % ((ULONG) CSMD_TSCYC_250_US)) != 0)   ||
        (ulNewCycleTime > ((ULONG) CSMD_TSCYC_MAX))
      )
    {
      return(SICE_SERCOS_CYCLE_TIME_INVALID);
    }
    else
    {
      return(SICE_NO_ERROR);
    }
  }
  else
  {
    if  (
        (ulNewCycleTime != ((ULONG) CSMD_TSCYC_MIN))      &&
        (ulNewCycleTime != ((ULONG) CSMD_TSCYC_62_5_US))  &&
        (ulNewCycleTime != ((ULONG) CSMD_TSCYC_125_US))
      )
    {
      return(SICE_SERCOS_CYCLE_TIME_INVALID);
    }
    else
    {
      return(SICE_NO_ERROR);
    }
  }
}
