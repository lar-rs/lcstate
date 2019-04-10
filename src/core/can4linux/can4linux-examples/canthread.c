/*
 * canthread - demonstration of using can4linux with threads
 *
 * Copyright (c) 2008-2015 Heinz-Jürgen Oertel hj.oertel@t-online.de
 * Copyright (c) 2008 port GmbH Halle/Saale
 *------------------------------------------------------------------
 * $Id$
 *
 *--------------------------------------------------------------------------
 *
 *
 */


/**
* \file canthread.c
* $Author$ 
* $Revision$
* $Date$
*
*
*/


/* header of standard C - libraries */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <errno.h>
#include <inttypes.h>

/* header of common types */
#include <pthread.h>
#include <can4linux.h>

/* shared common header */

/* header of project specific types */

/* project headers */

/* local header */

/* constant definitions
---------------------------------------------------------------------------*/
#define VERSION "$Revision$"

#if defined(EMBED)
# if defined(COLDFIRE)
/* IGW900 */
#  define STDDEV "/home/bin/can0"
# else
#  define STDDEV "/dev/can0"
# endif
#else
# define STDDEV "/dev/can1"
#endif


#ifndef TRUE
# define TRUE  1
# define FALSE 0
#endif

#ifndef CO_CONST
# define CO_CONST const
#endif

#define MAX_DEVNAME 32

#define MAX_THREADS 2
#define RX_THREAD 0
#define TX_THREAD 1

/* local defined data types
---------------------------------------------------------------------------*/
typedef struct {
    int tno;

    int can_fd;
    int loop_cnt;
} thread_data_t;

/* list of external used functions, if not in headers
---------------------------------------------------------------------------*/

/* list of global defined functions
---------------------------------------------------------------------------*/

/* list of local defined functions
---------------------------------------------------------------------------*/
static void *can_rx_thread( void *ptr );
static void *can_tx_thread( void *ptr );

static int set_bitrate( int fd, int baud );
static int can_start( int fd );
static int can_stop( int fd );
static int can_reset( int fd );

static void usage (char *s);

/* external variables
---------------------------------------------------------------------------*/

/* global variables
---------------------------------------------------------------------------*/

/* local defined variables
---------------------------------------------------------------------------*/
#ifdef CONFIG_RCS_IDENT
static CO_CONST char _rcsid[] = "$Id$";
#endif

static int o_debug;
static int o_bitrate = 0;
static char o_device[MAX_DEVNAME];

static pthread_t threads[MAX_THREADS];
static thread_data_t td[MAX_THREADS];


/***************************************************************************/
/**
*
* \brief  main entry function
*
* The main function opens the CAN device given as #define STDEV or as optional
* parameter to the function using the -D device flag.
* if a bitrate is given by the -b bitrate, the bit rate is set to this value,
* otherwise the current bitrate of the CAN device is used.
*
* After these preparations, the rx-thread and the tx-thread are created and executed.
*
*
* \internal
*/
int main (
	int argc,		/**< */
	char **argv			/**< */
	)
{
int c;
int can_fd;
    snprintf(o_device, MAX_DEVNAME, "%s", STDDEV);

    while ((c = getopt(argc, argv, "b:dD:hV")) != EOF) {
	switch (c) {
	    case 'b':
		o_bitrate = atoi(optarg);
		break;
	    case 'D':
		if (
		    /* path ist starting with '.' or '/', use it as it is */
			optarg[0] == '.'
			|| 
			optarg[0] == '/'
			) {
		    snprintf(o_device, MAX_DEVNAME, "%s", optarg);

	        } else {
		    snprintf(o_device, MAX_DEVNAME, "/dev/%s", optarg);
		}
		break;
	    case 'd':
		o_debug = TRUE;
		break;

	    case 'V':
		printf("%s V " VERSION ", " __DATE__ "\n"
#if defined(CANFD)
			    " (CAN FD data structure used)"
#else
			    ""
#endif
			, *argv);
		printf(" can4linux.h header version %d.%d\n",
			CAN4LINUXVERSION >> 8, CAN4LINUXVERSION & 0xFF);
		exit(0);
		break;

	    case 'h': /* fall through */
	    default:
	    	usage(*argv);
	    	exit(0);
	}
    }

    if (o_debug == 1) {
	fprintf(stderr, "%s V " VERSION ", " __DATE__ "\n", *argv);
	fprintf(stderr, "Opening %s with %d KBit/s\n", o_device, o_bitrate);
    }

    can_fd = open(o_device, O_RDWR /* | O_NONBLOCK */ );
    if (can_fd == -1) {
	fprintf(stderr, "Open error %d;", errno);
	perror(o_device);
	exit(1);
    }

    if(o_bitrate > 0) {
	set_bitrate(can_fd, o_bitrate);
    }

    td[RX_THREAD].tno = 1;
    td[RX_THREAD].can_fd = can_fd;
    td[RX_THREAD].loop_cnt = 100;
    pthread_create(&threads[RX_THREAD], NULL,
		can_rx_thread, (void *) &td[RX_THREAD]);


    /* need to protect resource can_fd with a mutex */
    td[TX_THREAD].tno = 2;
    td[TX_THREAD].can_fd = can_fd;
    td[TX_THREAD].loop_cnt = 100;
    pthread_create(&threads[TX_THREAD], NULL,
		can_tx_thread, (void *) &td[TX_THREAD]);

    pthread_join(threads[RX_THREAD], NULL);
    pthread_join(threads[TX_THREAD], NULL);

    return 0;
}

/***************************************************************************/
/**
*
* CAN thread that reads from the CAN controller
*
*
* \retval 0
*
*/
static void *can_rx_thread (
	void *ptr
    )
{
int i;
int ret;
int tno;
int can_fd;
int loop_cnt;
int received = 0;
char type;

pthread_t tid;
canmsg_t rx;

    tid = pthread_self();
    tno = ((thread_data_t *) ptr)->tno;
    fprintf(stderr, "Thread(%d) RX started: Id = %x\n",  tno, (int)tid);

    can_fd      = ((thread_data_t *) ptr)->can_fd;
    loop_cnt    = ((thread_data_t *) ptr)->loop_cnt;

    if (o_debug) {
	fprintf(stderr, "Thread(%d) RX can_fd: %d, loop_cnt: %d\n",
				tno, can_fd, loop_cnt);
    }

    do {
	ret = read(can_fd, &rx, 1);
	if (ret <= 0 ) {
	    /* perror("read error"); */
	    usleep(200);
	} else {

	    /* if (-1ul == rx.id)  */
	    if (CANDRIVERERROR == rx.id) {
		perror("driver error");
		usleep(200);
	    } else {

		received++;

	        type = (rx.flags & MSG_EXT) ? 'x' : 's';

		printf("Rx %4d : %4u/0x%03x : ",
			received, rx.id, rx.id);
	        if ((rx.flags & MSG_RTR) != 0) {
		    printf("%cR : (length = %d)\n",
		    	type, rx.length );
		} else {
		    printf(" %cD : ", type );
		    for (i=0; i<rx.length; i++) {
			printf("%02x ", rx.data[i]);
		    }
		    printf("\n");
		}
	    }
	}
    } while (received < loop_cnt);

    fprintf(stderr, "Thread(%d) RX exit\n", tno);
    pthread_exit(NULL);
}

/***************************************************************************/
/**
*
* CAN thread that writes to the CAN controller
*
* \retval 0
*
*/
static void *can_tx_thread (
	void *ptr
    )
{
int i;
int ret;
int tno;
int can_fd;
uint32_t loop_cnt;
uint32_t *dataptr;

pthread_t tid;
canmsg_t tx;

    tid = pthread_self();
    tno = ((thread_data_t *) ptr)->tno;
    fprintf(stderr, "Thread(%d) TX startet: Id = %x\n",  tno, (int)tid);

    can_fd      = ((thread_data_t *) ptr)->can_fd;
    loop_cnt    = ((thread_data_t *) ptr)->loop_cnt;

    if (o_debug) {
	fprintf(stderr, "Thread(%d) TX can_fd: %d, loop_cnt: %d\n",
				tno, can_fd, loop_cnt);
    }

    tx.id = 0x100;
    tx.length = 4;
    dataptr = (uint32_t *)&tx.data;

    i = 0;
    do {
	*dataptr = i++;
        ret = write(can_fd, &tx, 1);
        if (ret < 0) {
	    perror("write error");
        } else {
            /* fprintf(stderr, "ret %d\n", ret); */
            /* fprintf(stderr, "loop_cnt %d\n", loop_cnt); */
        }
	usleep(20000);
    } while (i < loop_cnt);

    fprintf(stderr, "Thread(%d) TX exit\n", tno);

    pthread_exit(NULL);
}

/***************************************************************************/
/**
*
* display usage text
*
*/
static void usage (char *s) {
static char *usage_text  = "\
Options:\n\
-b baudrate (Standard uses value of /proc/sys/dev/Can/baud)\n\
-d   - debug On\n\
       schaltet zusaetzlich Debugging im Treiber an/aus\n\
-D dev use /dev/dev/{can0,can1,can2,can3} (real nodes, std: can1)\n\
-h show this help \n\
-V   print program version\n\
\n\
";

     fprintf(stderr, "usage: %s [options] [id [ byte ..]]\n", s);
     fputs(usage_text, stderr);
}

/***************************************************************************/
/**
*
* Set CAN bitrate.
*
* Stops the program when setting the bitrate fails.
*
* \retval 0
*
*/
static int set_bitrate(
	int fd,			/**< device descriptor */
	int baud		/**< bit rate */
    )
{
int ret;
volatile config_par_t  cfg;

    ret = can_stop(fd);
    
    cfg.target = CONF_TIMING; 
    cfg.val1   = baud;
    ret = ioctl(fd, CAN_IOCTL_CONFIG, &cfg);

    ret = can_start(fd);

    if (ret < 0) {
	perror("set_bitrate");
	exit(-1);
    } else {
	ret = 0;
    }
    return ret;
}

/***************************************************************************/
/**
*
* Reset the CAN-Controller
*
* \retval 0 success
* \retval !=0 failure
*
*/
static int can_reset(
	int fd			/**< device descriptor */
    )
{
int ret;
volatile command_par_t cmd;


    cmd.cmd = CMD_RESET;
    ret = ioctl(fd, CAN_IOCTL_COMMAND, &cmd);

    return ret;
}

/***************************************************************************/
/**
*
* Start the CAN-Controller
*
* \retval 0 success
* \retval !=0 failure
*
*/
static int can_start(
	int fd			/**< device descriptor */
    )
{
int ret;
volatile command_par_t cmd;


    cmd.cmd = CMD_CLEARBUFFERS;
    ret = ioctl(fd, CAN_IOCTL_COMMAND, &cmd);
    cmd.cmd = CMD_START;
    ret = ioctl(fd, CAN_IOCTL_COMMAND, &cmd);

    return ret;
}


/***************************************************************************/
/**
*
* Stop the CAN-Controller
*
* \retval 0 success
* \retval !=0 failure
*
*/
static int can_stop (
	int fd			/**< device descriptor */
    )
{
int ret;
volatile command_par_t cmd;


    cmd.cmd = CMD_STOP;
    ret = ioctl(fd, CAN_IOCTL_COMMAND, &cmd);

    return ret;
}

/*______________________________________________________________________EOF_*/
