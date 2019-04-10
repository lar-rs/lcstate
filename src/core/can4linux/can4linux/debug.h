/*
 * can4linux project
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
*/
#ifndef __DEBUG_INCLUDED

#if defined(DEBUG)
#define DRIVER_NAME "Can"

#define DBG_ALL      (1 << 0)
#define DBG_ENTRY   ((1 << 1) | DBG_ALL)
#define DBG_EXIT    ((1 << 2) | DBG_ALL)
#define DBG_BRANCH  ((1 << 3) | DBG_ALL)
#define DBG_DATA    ((1 << 4) | DBG_ALL)
#define DBG_INTR    ((1 << 5) | DBG_ALL)
#define DBG_REG     ((1 << 6) | DBG_ALL)
#define DBG_SPEC    ((1 << 7) | DBG_ALL)
#define DBG_1PPL     (1 << 8)		/* one DBG print statement/line */

extern unsigned int proc_dbgmask;


/* class of debug statements allowed		*/

extern int   fidx;
extern char *fstk[];
extern char *ffmt[];

#define DBGPRINT(ms, ar)	{ if (proc_dbgmask && (proc_dbgmask & ms)) \
	{ pr_info("Can[%d]: - :", minor); printk ar; printk("\n"); } }
#define DBGIN()		{ if (proc_dbgmask && (proc_dbgmask & DBG_ENTRY)) \
	{ pr_info("Can[%d]: - : in  %s()\n", minor, __func__); } }
#define DBGOUT()	{ if (proc_dbgmask && (proc_dbgmask & DBG_EXIT)) \
	{ pr_info("Can[%d]: - : out %s()\n", minor, __func__); } }

#define DEBUG_TTY(n, args...) { if (proc_dbgmask >= (n)) print_tty(args); }

#else
#define DBGPRINT(ms, ar) { }
#define DBGIN()	{ }
#define DBGOUT()	{ }
#define DEBUG_TTY(n, args...)
extern unsigned int proc_dbgmask;

#endif

#define __DEBUG_INCLUDED
#endif



