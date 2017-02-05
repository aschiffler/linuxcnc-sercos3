/**
 * \file      RTLX_S3SM_USER.h
 *
 * \brief     User settings for real-time operating system abstraction layer for
 *            Linux RT-Preempt, specific functionality for Sercos soft master
 *            (S3SM)
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
 * \ingroup   RTLX_S3SM
 *
 */

// avoid multiple inclusions - open

#ifndef _RTLX_S3SM_USER
#define _RTLX_S3SM_USER

#undef SOURCE
#ifdef SOURCE_RTLX
    #define SOURCE
#else
    #define SOURCE extern
#endif

//---- includes ---------------------------------------------------------------

//---- defines ----------------------------------------------------------------

/**
 * \def     RTOS_THREAD_PRIORITY_MAIN_STARTUP
 *
 * \brief   Priority for main thread used in application at startup.
 */
#define RTOS_THREAD_PRIORITY_MAIN_STARTUP   (RTOS_RT_PRIO-2)

/**
 * \def     RTOS_THREAD_PRIORITY_MAIN
 *
 * \brief   Priority for main thread used in application after startup.
 */
#define RTOS_THREAD_PRIORITY_MAIN           (RTOS_THREAD_PRIORITY_STD)

/**
 * \def     RTOS_THREAD_PRIORITY_WORKER
 *
 * \brief   Priority for worker thread used in application
 */
#define RTOS_THREAD_PRIORITY_WORKER         (RTOS_RT_PRIO-1)

/**
 * \def     RTOS_THREAD_PRIORITY_USER
 *
 * \brief   Priority for user thread used in application
 */
#define RTOS_THREAD_PRIORITY_USER           (RTOS_RT_PRIO-10)

/**
 * \def     RTOS_THREAD_PRIORITY_USER_SETPHASE3
 *
 * \brief   Priority for user thread used in application when CSMD_SetPhase3 is
 *          called.
 */
#define RTOS_THREAD_PRIORITY_USER_SETPHASE3 (RTOS_RT_PRIO-5)

/**
 * \def     RTOS_THREAD_PRIORITY_TIMER
 *
 * \brief   Priority for timer thread used in application
 */
#define RTOS_THREAD_PRIORITY_TIMER          (RTOS_RT_PRIO)

/**
 * \def     RTOS_TIMING_MODE
 *
 * \brief   Selects which timing mode is used for cyclic timing.
 */
#define RTOS_TIMING_MODE                    (RTOS_TIMING_TIMER)

/**
 * \def     RTOS_USE_NIC_TIMED_TX
 *
 * \brief   If defined, NIC-timed packet transmission. This requires
 *          RTOS_TIMING_MODE to be set to RTOS_TIMING_NIC.
 */
#undef RTOS_USE_NIC_TIMED_TX

/**
 * \def     RTOS_NANOSLEEP_CORR_INTERVAL
 *
 * \brief   Number of of Sercos cycles used as timing correction interval in
 *          timing mode RTOS_TIMING_REL_NANOSLEEP_COMP. This interval must be
 *          smaller than (1s) / (Sercos cycle time).
 */
#define RTOS_NANOSLEEP_CORR_INTERVAL        (300)

/**
 * \def     RTOS_NANOSLEEP_INIT_CORR_VALUE
 *
 * \brief   Initial timing correction value (ns per Sercos cycle). In case
 *          RTOS_NANOSLEEP_DYN_CORR is undefined, this value is used all the
 *          time.
 */
#define RTOS_NANOSLEEP_INIT_CORR_VALUE      (-10000)

/**
 * \def     RTOS_NANOSLEEP_CORR_FACTOR
 *
 * \brief   Weighing factor of new differential correction interval. Higher
 *          value means faster reaction, lower smoother control
 */
#define RTOS_NANOSLEEP_CORR_FACTOR          (0.2)

/**
 * \def     RTOS_UCC_MAX_PACKETS
 *
 * \brief   Defines maximum number of packets per UCC cycle (TX and RX); Sercos
 *          master IP core v4 has a buffer of 8 packets.
 */
#define RTOS_UCC_MAX_PACKETS                (8)

/**
 * \def     RTOS_TIME_SHIFT_NIC_TIMED
 *
 * \brief   Offset of NIC-timed telegram transmission to timing interrupt in ns
 */
#define RTOS_TIME_SHIFT_NIC_TIMED           (200 * 1000)

/**
 * \def     RTOS_FILTER_SERCOS_ETHERTYPE
 *
 * \brief   If activated, it is filtered for Sercos packets only. This function
 *          should be de-activated in case the UCC channel with soft master
 *          support (SICE_UC_CHANNEL) is activated.
 */
#undef RTOS_FILTER_SERCOS_ETHERTYPE

/**
 * \def     RTOS_BIND_NIC
 *
 * \brief   If activated, the sockets are bound to the selected NIC adapters.
 */
#define RTOS_BIND_NIC

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

// avoid multiple inclusions - close

#endif
