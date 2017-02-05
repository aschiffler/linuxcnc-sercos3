/**
 * \file      RTLX_USER.h
 *
 * \brief     User settings for real-time operating system abstraction layer for
 *            Linux RT-Preempt
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
 * \ingroup   RTLX
 *
 */

// avoid multiple inclusions - open

#ifndef _RTLX_USER
#define _RTLX_USER

#undef SOURCE
#ifdef SOURCE_RTLX
    #define SOURCE
#else
    #define SOURCE extern
#endif

//---- includes ---------------------------------------------------------------

//---- defines ----------------------------------------------------------------

/**
 * \def     RTLX_VERBOSE_LEVEL
 *
 * \brief   Defines the debug level that results in the number of debug outputs
 *          in the module RTLX. A value of -1 means no output at all, 0
 *          'normal' outputs and higher values are intended for debugging.
 */
#define RTLX_VERBOSE_LEVEL                  (0)

/**
 * \def     RTOS_MEM_PAGE_SIZE
 *
 * \brief   Defines memory page size. If no page alignment is required, set to
 *          1.
 */
#define RTOS_MEM_PAGE_SIZE                  (4096)

//---- type definitions -------------------------------------------------------

//---- variable declarations --------------------------------------------------

//---- function declarations --------------------------------------------------

// avoid multiple inclusions - close

#endif
