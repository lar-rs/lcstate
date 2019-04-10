/*
 * can4linux -- LINUX CAN device driver source
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 *
 *
 *
 * derived from the the LDDK can4linux version
 *     (c) 1996,1997 Claus Schroeter (clausi@chemie.fu-berlin.de)
 * (c) 2013-2014 Heinz-Jürgen Oertel (hj.oertel@t-online.de)
 *
 *------------------------------------------------------------------
 */

/* use it for pr_info() and consorts */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/time.h>
#include "defs.h"


/* each CAN channel has one wait_queue for read and one for write */
wait_queue_head_t can_wait[MAX_CHANNELS][CAN_MAX_OPEN];
wait_queue_head_t canout_wait[MAX_CHANNELS];

spinlock_t write_splock[MAX_CHANNELS];

/* Flag for the ISR, which read queue is in use */
int can_waitflag[MAX_CHANNELS][CAN_MAX_OPEN] = { {0} };

/* for each CAN channel allocate a TX and RX FIFO */
msg_fifo_t   tx_buf[MAX_CHANNELS] = { {0} };
msg_fifo_t   rx_buf[MAX_CHANNELS][CAN_MAX_OPEN] = { { {0} } };

#ifdef CAN_USE_FILTER
msg_filter_t rx_filter[MAX_CHANNELS] = { {0} };
#endif
/* used to store always the last frame sent on this channel */
canmsg_t     last_tx_object[MAX_CHANNELS];

#if defined(CAN_PORT_IO)
/* can_iobase holds a pointer, in order to use the access macros
   for address calculation with the given CAN register structure.
   before accessing the registers, the pointer is casted to int */
unsigned char *can_iobase[MAX_CHANNELS] = {NULL};
#else
/* Memory access to CAN */
void __iomem *can_iobase[MAX_CHANNELS] = {NULL}; /* ioremapped addresses */
#endif /* defined (CAN_PORT_IO) */

unsigned int can_range[MAX_CHANNELS] = {0};	/* width of the address range */

/* flag indicating that selfreception of frames is allowed */
int selfreception[MAX_CHANNELS][CAN_MAX_OPEN] = { {0} };
int use_timestamp[MAX_CHANNELS] = {0};	/**< flag indicating that timestamp
				       value should assigned to rx messages */
int wakeup[MAX_CHANNELS] = {0};		/* flag indicating that sleeping
				    processes are waken up in case of events */

int erroractive[MAX_CHANNELS] = {0};
/* this is a global module parameter */
int errint;

/**
*
\brief get the time stamp from the kernel and provide it in different formats

The function evaluates the global variable use_timestamp[minor]
for the format information.

\param [in] minor	minor device number to use
\param[out] timestamp Pointer to the result
*/
void get_timestamp(int minor, struct timeval *timestamp)
{
s64 nsec;
s64 diff;
struct timespec ts;
struct timeval  tv;
static s64 old_time;

	switch (use_timestamp[minor]) {
	/* zeroed out */
	case 0:
		timestamp->tv_sec  = 0;
		timestamp->tv_usec = 0;
		break;

	/* monotonic time */
	case 2:
		ktime_get_ts(&ts);
		nsec = timespec_to_ns(&ts);
		tv = ns_to_timeval(nsec);
		timestamp->tv_sec  = tv.tv_sec;
		timestamp->tv_usec = tv.tv_usec;
		break;

	/* time difference */
	case 3:
		ktime_get_ts(&ts);
		nsec = timespec_to_ns(&ts);
		diff = nsec - old_time;
		tv = ns_to_timeval(diff);
		timestamp->tv_sec  = tv.tv_sec;
		timestamp->tv_usec = tv.tv_usec;
		/* save this time stamp for next time difference in nsecs */
		old_time = nsec;
		break;

	/* gettimeofday */
	case 1:
	default:
		do_gettimeofday(timestamp);
	break;
	}
}

/*
initialize RX Fifo
*/
int can_rx_fifo_init(int minor, int fifo)
{
	DBGIN();
	/* pr_info("can_rx_fifo_init minor %d, fifo %d\n", minor, fifo); */

	rx_buf[minor][fifo].head   = 0;
	rx_buf[minor][fifo].tail   = 0;
	rx_buf[minor][fifo].status = 0;
	rx_buf[minor][fifo].active = 0;

	init_waitqueue_head(&can_wait[minor][fifo]);
	can_waitflag[minor][fifo] = 1;

	DBGOUT();
	return 0;
}
/*
initialize  TX Fifo
*/
int can_tx_fifo_init(int minor)
{
int i;

	DBGIN();

	tx_buf[minor].head   = 0;
	tx_buf[minor].tail   = 0;
	tx_buf[minor].status = 0;
	tx_buf[minor].active = 0;
	for (i = 0; i < MAX_BUFSIZE; i++)
		tx_buf[minor].free[i]  = BUF_EMPTY;
	init_waitqueue_head(&canout_wait[minor]);

	DBGOUT();
	return 0;
}


/* refresh open count in /proc/^ */
void format_proc_device_open_count(void)
{
int i;
char buf[sizeof("\tyy(xx)")];
int l = sizeof("\tyy(xx)");

	buf[0] = proc_opencount[0] = '\0';
	for(i = 0; i < MAX_CHANNELS; i++) {
	    snprintf(buf, l, "\t%d(%d)", atomic_read(&can_isopen[i]), CAN_MAX_OPEN);
	    strncat(proc_opencount,  buf, l);
	}
}

#ifdef CAN_USE_FILTER
int can_filter_init(int minor)
{
int i;

	DBGIN();
	rx_filter[minor].use      = 0;
	rx_filter[minor].signo[0] = 0;
	rx_filter[minor].signo[1] = 0;
	rx_filter[minor].signo[2] = 0;

	for (i = 0; i < MAX_ID_NUMBER; i++)
		rx_filter[minor].filter[i].rtr_response = NULL;

	DBGOUT();
	return 0;
}

int can_filter_cleanup(int minor)
{
int i;

	DBGIN();
	for (i = 0; i < MAX_ID_NUMBER; i++) {
		if (rx_filter[minor].filter[i].rtr_response != NULL)
			kfree(rx_filter[minor].filter[i].rtr_response);
		rx_filter[minor].filter[i].rtr_response = NULL;
	}
	DBGOUT();
	return 0;
}


int can_filter_onoff(int minor, unsigned on)
{
	DBGIN();
	rx_filter[minor].use = (on != 0);
	DBGOUT();
	return 0;
}

int can_filter_message(int minor, unsigned message, unsigned enable)
{
	DBGIN();
	rx_filter[minor].filter[message].enable = (enable != 0);
	DBGOUT();
	return 0;
}

int can_filter_timestamp(int minor, unsigned message, unsigned stamp)
{
	DBGIN();
	rx_filter[minor].filter[message].timestamp = (stamp != 0);
	DBGOUT();
	return 0;
}

int can_filter_signal(int minor, unsigned id, unsigned signal)
{
	DBGIN();
	if (signal <= 3)
		rx_filter[minor].filter[id].signal = signal;
	DBGOUT();
	return 0;
}

int can_filter_signo(int minor, unsigned signo, unsigned signal)
{
	DBGIN();
	if (signal < 3)
		rx_filter[minor].signo[signal] = signo;
	DBGOUT();
	return 0;
}
#endif

#ifdef CAN_RTR_CONFIG
int can_config_rtr(int minor, unsigned message, canmsg_t *tx)
{
canmsg_t *tmp;

	DBGIN();
	tmp = kmalloc(sizeof(canmsg_t), GFP_ATOMIC);
	if (tmp == NULL) {
		DBGPRINT(DBG_BRANCH, ("memory problem"));
		DBGOUT();
		return -1;
	}
	rx_filter[minor].filter[message].rtr_response = tmp;
	memcpy(rx_filter[minor].filter[message].rtr_response,
			tx, sizeof(canmsg_t));
	DBGOUT();
	return 1;
}

int can_unconfig_rtr(int minor, unsigned message)
{
canmsg_t *tmp;

	DBGIN();
	if (rx_filter[minor].filter[message].rtr_response != NULL) {
		kfree(rx_filter[minor].filter[message].rtr_response);
		rx_filter[minor].filter[message].rtr_response = NULL;
	}
	DBGOUT();
	return 1;
}
#endif


#ifdef DEBUG

/* can_dump_mem or can_dump() which is better ?? */
#if defined(CAN4LINUX_PCI)
#else
#endif
#include <linux/io.h>

#if 1
/* simply dump a memory area byte wise for n*16 addresses */
/*
 * address - start address
 * n      - number of 16 byte rows,
 * offset - print every n-th byte
 */
void can_dump_mem(unsigned long address, int n, int offset)
{
int i, j;

	pr_info("     CAN at Adress 0x%lx\n", address);
	for (i = 0; i < n; i++) {
		pr_info("     ");
		for (j = 0; j < 16; j++) {
			/* pr_info("%02x ", *ptr++); */
			pr_info("%02x ", readb((void __iomem *)address));
			address += offset;
		}
		pr_info("\n");
	}
}
#endif

#ifdef CPC_PCI
# define REG_OFFSET 4
#else
# define REG_OFFSET 1
#endif
/**
*   Dump the CAN controllers register contents,
*   identified by the device minor number to stdout
*
*   can_iobase[minor] should contain the virtual address
*   in case of PORT IO, proc_base is used directly
*/
void can_dump(int minor)
{
int i, j;
int index;

	index = 0;
	for (i = 0; i < 2; i++) {
		pr_info("0x%p: ", can_iobase[minor] + (i * 16));
		for (j = 0; j < 16; j++) {
#ifdef CAN_PORT_IO
		unsigned int address;
			address = inb((unsigned int)(proc_base[minor] + index));
			pr_info("%02x ", address);
#else
			pr_info("%02x ",
			readb((void __iomem *)(can_iobase[minor] + index)));
#endif
			index += REG_OFFSET;
		}
		pr_info("\n");
	}
}
#endif

/*----------------------------------------------------------------------------*/
