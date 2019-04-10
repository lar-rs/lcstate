/* simple CAN application example using can4linux
 * 
 * open CAN and test the read(2) call
 * An calling option  decides if the CAN device is opened for
 * blocking or nonblocking read.
 * Copyright (c) 2014-2015 H.J.Oertel 
 *
 * Option: -H
 * Explaining the 'horch' like Format
 * 7884.098543       10/0x0000000a : bD ( 8): 55 02 03 04 05 06 07 aa 
 *   |               |      |         |   |    |
 *   |               |      |         |   |    +- data bytes, hex coded
 *   |               |      |         |   +- length information of CAN frame
 *   |               |      |         +- Frame information
 *   |               |      |         	 bD  - Base Frame Format, Data Frame 
 *   |               |      |         	 eD  - Extended Frame Format, Data Frame
 *   |               |      |         	 bR  - Base Frame Format, Remote Frame 
 *   |               |      |         	 eR  - Extended Format, Remote Frame
 *   |               |      +- CAN Frame ID in Hex
 *   |               +- CAN Frame ID in decimal
 *   +--- Time stamp: s.µs
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#ifdef USE_RT_SCHEDULING
# include <sched.h>
# include <sys/mman.h>
#endif

# define  HAVE_TERMIOS
# include <termios.h>
# include <sys/resource.h>
# include <signal.h>

#include <can4linux.h>

// #define USER_TERMIOS

#define STDDEV "/dev/can0"
#define COMMANDNAME "receive"
#define VERSION "SVN $Revision$"
#define RXBUFFERSIZE 100

#ifndef TRUE
# define TRUE  1
# define FALSE 0
#endif

int sleeptime            = 1000;	/* standard sleep time */
int debug                = FALSE;
int baud		 = -1;		/* dont change baud rate */
int blocking		 = TRUE;	/* open() mode */
int buffers		 = RXBUFFERSIZE;/* open() mode */
int listenmode		 = FALSE;	/* listen only mode */
int selfreception	 = FALSE;	/* selfreception mode */
int settsmode		 = FALSE;	/* Set Time Stamp Format */
int tsmode		 = 0;		/* Time Stamp mode value */
#ifdef USE_RT_SCHEDULING
static int priority	= -1;		/* don't change priority rate */
#endif

/* the following flags can be toggled on-line */
unsigned int horchmode	 = FALSE;	/* Horch-Format mode */
unsigned int plugfest	 = FALSE;

#ifdef HAVE_TERMIOS
static struct termios oldtty;
#endif

void display_participant(unsigned id);
void set_terminal_mode(void);
void clean_up(int sig);
void cut_mark(void);
static void clean(void);

/* ----------------------------------------------------------------------- */

void usage(char *s)
{
const  char *usage_text  = "\
 Open CAN device and display read messages\n\
 Default device is %s. \n\
Options:\n\
-d   - debug On\n\
       swich on additional debugging\n\
-b baudrate (Standard uses value of /proc/sys/Can/baud)\n\
-n   - non-blocking mode (default blocking)\n\
-B n - buffer size used in non-blocking mode (100)\n\
-s sleep sleep in ms between read() calls in non-blocking mode\n\
-H   - output displayed in \'horch \' format\n\
-l     Listen only mode\n\
-f     self-reception mode\n\
-t   - time stamp mode [0123]\n\
       0 - no time stamp (time stamp is zero)\n\
       1 - absolute time as gettimeofday()\n\
       2 - absolute rate monotonic time\n\
       3 - time difference to the last event (received message)\n\
       4 - absolute time, readable   year-month-day h:m:s.us\n\
-p p   change scheduling priority of receive\n\
-V   version\n\
-P   CiA PlugFest mode (komws manufacturer CAN Ids\n\
\n\
";

const  char *i_usage_text  = "\
Interactive usage commands:\n\
h - toggle horch display mode\n\
f - toggle self-reception on sent frames\n\
c - issue a cut-mark on stdout\n\
p - display manufacturers name for CiA PlugFest\n\
q - quit\n\
\n\
";

    fprintf(stderr, "usage: %s [options] [device]\n", s);
    fprintf(stderr, (const char *)usage_text, STDDEV);
    fprintf(stderr, (const char *)i_usage_text, STDDEV);
}


/***********************************************************************
*
* set_bitrate - sets the CAN bit rate
*
*
* Changing these registers only possible in Reset mode.
*
* RETURN:
*
*/

int	set_bitrate(
	int fd,			/* device descriptor */
	int baud		/* bit rate */
	)
{
config_par_t  cfg;
volatile command_par_t cmd;
int ret;


    cmd.cmd = CMD_STOP;
    ioctl(fd, CAN_IOCTL_COMMAND, &cmd);

    cfg.target = CONF_TIMING; 
    cfg.val1   = baud;
    ret = ioctl(fd, CAN_IOCTL_CONFIG, &cfg);

    cmd.cmd = CMD_START;
    ioctl(fd, CAN_IOCTL_COMMAND, &cmd);

    if (ret < 0) {
	perror("set_bitrate");
	exit(-1);
    } else {
	ret = 0;
    }
    return ret;
}

/***********************************************************************
*
* set_lomode - sets the CAN in Listen Only Mode
*
*
* Changing these registers only possible in Reset mode.
*
* RETURN:
*
*/
int set_lomode(int fd)
{
config_par_t  cfg;
volatile command_par_t cmd;
int ret;

    cmd.cmd = CMD_STOP;
    ioctl(fd, CAN_IOCTL_COMMAND, &cmd);

    cfg.target = CONF_LISTEN_ONLY_MODE; 
    cfg.val1   = 1;
    ret = ioctl(fd, CAN_IOCTL_CONFIG, &cfg);

    cmd.cmd = CMD_START;
    ioctl(fd, CAN_IOCTL_COMMAND, &cmd);

    if (ret < 0) {
	perror("set_lomode");
	exit(-1);
    } else {
	ret = 0;
    }
    return ret;
}


/***********************************************************************
*
*\brief  set_tsmode - sets the time stamp mode for received messages
*
*   0 - no time stamp (time stamp is zero)
*   1 - absolute time as gettimeofday()
*   2 - absolute rate monotonic time
*   3 - time difference to the last event (received message)
*
* if an ioctl() error occurs, exit() is called. 
*
* \return 0 OK
*/
int set_tsmode(int fd, int mode)
{
config_par_t  cfg;
int ret;


    cfg.target = CONF_TIMESTAMP; 
    cfg.val1   = mode;
    ret = ioctl(fd, CAN_IOCTL_CONFIG, &cfg);


    if (ret < 0) {
	perror("set_tsmode");
	exit(-1);
    } else {
	ret = 0;
    }
    return ret;
}


/***********************************************************************
* \brief set_selfreception
*
* toggle the self reception ability of the CAN driver
*
* A message frame sent out by the controller is copied into
* the receive queue after successful transmission.
*/
void set_selfreception(int fd, int v)
{
#if CAN4LINUXVERSION >= 0x0402		/* defined in can4linux.h */
config_par_t cfg;
#else
Config_par_t  cfg;
#endif

    if(debug) {
	printf(" set selfreception to %d\n", v);
    }
    cfg.cmd    = CAN_IOCTL_CONFIG;
    cfg.target = CONF_SELF_RECEPTION; 
    cfg.val1   = v;
    ioctl(fd, CAN_IOCTL_CONFIG, &cfg);

}


/***********************************************************************
*
* main - 
*
*
*/

int main(int argc, char **argv)
{
int fd;
int got;
int c;
char *pname;
extern char *optarg;
extern int optind;

canmsg_t rx[RXBUFFERSIZE];
char device[50] = "/dev/can0";
int messages_to_read = 1;

#ifdef USE_RT_SCHEDULING
    int ret;
    int max_rr_priority, min_rr_priority;
    int max_ff_priority, min_ff_priority;
#else
    int max_priority;
#endif

    pname = *argv;

#ifdef USE_RT_SCHEDULING
    max_rr_priority = sched_get_priority_max(SCHED_RR);
    min_rr_priority = sched_get_priority_min(SCHED_RR);
    max_ff_priority = sched_get_priority_max(SCHED_FIFO);
    min_ff_priority = sched_get_priority_min(SCHED_FIFO);
#else 
    max_priority = 1;
#endif

    /* parse command line */
    while ((c = getopt(argc, argv, "B:b:dD:fhHlp:s:t:nPV")) != EOF) {
	switch (c) {
	    case 'b':
		baud = atoi(optarg);
		break;
	    case 'B':
	        buffers = atoi(optarg);
	    case 's':
		sleeptime = atoi(optarg);
		break;
	    case 'd':
		debug = true;
		break;
	    case 'H':
		horchmode = TRUE;
		break;
	    case 'l':		/* set listen only mode */
		listenmode = TRUE;
		break;
	    case 'f':		/* set selfreception mode */
		selfreception = TRUE;
		break;
	    case 'n':
		blocking = FALSE;
		messages_to_read = RXBUFFERSIZE;
		break;
	    case 't':		    /* set time stamp mode */
		settsmode = TRUE;
		tsmode = atoi(optarg);
		break;
	    case 'P':
		plugfest = TRUE;
		break;
	    case 'p':
	        {
#ifdef USE_RT_SCHEDULING
	        struct sched_param mysched;
	        /* use real time round-robin or real time first-in first-out */
		    priority = atoi(optarg);
		    if (priority < min_rr_priority ) {
		      fprintf(stderr, "Priority < %d not allowed\n",
		      					min_rr_priority);
		    }
		    if (priority > max_rr_priority) {
		      fprintf(stderr, "Priority > %d not allowed\n",
		      					max_rr_priority);
		    }
		    mysched.sched_priority = priority;	
		    		/* sched_get_priority_max(SCHED_RR) - 1; */

		    ret = sched_setscheduler(0, SCHED_RR, &mysched);
		    if ( debug == true ) {
			printf("sched_setscheduler() = %d\n", ret);
		    }
		    if(ret == -1) {
			printf("No permission to change process priorities\n");
		    }
		    /* lock all currently and in future
			allocated memory blocks in physical ram */
		    ret = mlockall(MCL_CURRENT | MCL_FUTURE);
		    if ( debug == true ) {
			printf("mlockall() = %d\n", ret);
		    }

#endif
		}
		break;
	    case 'V':
		printf("%s %s %s\n", argv[0], " V " VERSION ", " __DATE__ ,
#if defined(CANFD)
			    " (CAN FD data structure used)"
#else
			    ""
#endif
			);
		printf(" can4linux.h header version %d.%d\n",
			CAN4LINUXVERSION >> 8, CAN4LINUXVERSION & 0xFF);
		exit(0);
		break;

		/* not used, device name is parameter */ 
	    case 'D':
		if (
		    /* path ist starting with '.' or '/', use it as it is */
			optarg[0] == '.'
			|| 
			optarg[0] == '/'
			) {
		    snprintf(device, 50, "%s", optarg);

	        } else {
		    snprintf(device, 50, "/dev/%s", optarg);
		}
		break;
	    case 'h':
	    default: usage(pname); exit(0);
	}
    }

    /* look for additional arguments given on the command line */
    if ( argc - optind > 0 ) {
        /* at least one additional argument, the device name is given */
        char *darg = argv[optind];

	if (
	    /* path ist starting with '.' or '/', use it as it is */
		    darg[0] == '.'
		    || 
		    darg[0] == '/'
		    ) {
		snprintf(device, 50, "%s", darg);
	} else {
		snprintf(device, 50, "/dev/%s", darg);
	}
    } else {
		snprintf(device, 50, "%s", STDDEV);
    }

    if (debug == true) {
	printf("%s %s\n", argv[0], " V " VERSION ", " __DATE__ );
	printf("(c) 2012-2014 H.J. Oertel\n");
	printf(" using canmsg_t with %d bytes\n", (int)sizeof(canmsg_t));
#if defined(CANFD)
	printf("  CAN FD data structure used\n");
#endif
	printf("  open CAN device \"%s\" in %sblocking mode\n",
		device, blocking ? "" : "non-");
#ifdef USE_RT_SCHEDULING
	printf("  possible process RR priority is \"-p %d - %d\"\n",
			min_rr_priority, max_rr_priority);
	{
	int pol;
	struct sched_param sp;

	    pol = sched_getscheduler(0);
	    sched_getparam(0, &sp);
	    printf("using real time %s policy and priority %d\n",
	    	(pol == SCHED_OTHER) ? "OTHER" :
	    	(pol == SCHED_RR)    ? "RR" :
	    	(pol == SCHED_FIFO)  ? "FIFO" :
#ifdef SCHED_BATCH
	    	(pol == SCHED_BATCH) ? "BATCH" :
#endif
#ifdef SCHED_IDLE
	    	(pol == SCHED_IDLE)  ? "IDLE" :
#endif
		"???", sp.sched_priority);
	}
#endif /* USE_RT_SCHEDULING */
    }

    sleeptime *= 1000;
    
    if(blocking == TRUE) {
	/* fd = open(device, O_RDWR); */
	fd = open(device, O_RDONLY);
    } else {
	fd = open(device, O_RDONLY | O_NONBLOCK);
    }
    if( fd < 0 ) {
	fprintf(stderr,"Error opening CAN device %s\n", device);
	perror("open");
	exit(1);
    }
    if (baud > 0) {
	if ( debug == TRUE ) {
	    printf("change Bit-Rate to %d Kbit/s\n", baud);
	}
	set_bitrate(fd, baud);
    }
    if (listenmode == TRUE) {
	if ( debug == TRUE ) {
	    printf(" Change to Listen Only Mode\n");
	}
	set_lomode(fd);
    }
    if (selfreception == TRUE) {
	if ( debug == TRUE ) {
	    printf(" Switch on selfreception of sent frames\n");
	}
	set_selfreception(fd, 1);
    }
    if (settsmode == TRUE) {
	if ( debug == TRUE ) {
	    printf(" Change to Time Stamp Mode %d\n", tsmode);
	}
	if (tsmode == 4) {
	    /* mode 4 uses absulte time stamp of the driver,
	     * but displays  a *ctime string */
	    set_tsmode(fd, 1);
	} else {
	    set_tsmode(fd, tsmode);
	}
    }

/* all parameters are valid, before going into the receive loop
 * set up the console stdin in raw mode to be able to change 
 * something at run time */
    atexit(clean);
    /* Installing Signal handler */
    if (signal (SIGINT, clean_up) == SIG_IGN)
	signal (SIGINT, SIG_IGN);

	// set_terminal_mode(); 

    /* printf("waiting for msg at %s\n", device); */

    while(1) {
#if 0
	int i;

	/* check for anything at stdin */
	ioctl(0, FIONREAD, &i);
	while (i--) { /* input available */
	    int c;
	    	/* get it */
 		c = getchar();
		switch (c) {
		    case 'c':
			cut_mark();
			break;
		    case 'p':
			plugfest = !plugfest;
			break;
		    case 'h':
			horchmode = !horchmode;
			break;
		    case 'f':
			selfreception = !selfreception;
			break;
		    case 'q':
			exit(0);
			break;
		    default:
			break;
		}
	}
#endif
      got=read(fd, &rx[0], messages_to_read);
      if( got > 0) {
        int i;
        int j;
	char *format;

        for(i = 0; i < got; i++) {
            if(horchmode) {
		if (tsmode == 4) {
		    time_t nowtime;
		    struct tm *nowtm;
		    char tmbuf[64];
		    nowtime = rx[i].timestamp.tv_sec;
		    nowtm = localtime(&nowtime);
		    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
		    printf("%s.%06lu", tmbuf, rx[i].timestamp.tv_usec);
		} else {
		printf("%12lu.%06lu",
			rx[i].timestamp.tv_sec,
			rx[i].timestamp.tv_usec);
		}
		if (plugfest) {
		    display_participant(rx[i].id);
		}
		if(rx[i].id != 0xffffffff) {
		    printf(" %9u/0x%08x",
			rx[i].id, rx[i].id);
		} else {
		    printf("%20s", "error");
		}
            } else {
		printf("Received with ret=%d: %12lu.%06lu id=%u/0x%08x\n",
			got, 
			rx[i].timestamp.tv_sec,
			rx[i].timestamp.tv_usec,
			rx[i].id, rx[i].id);

		printf("\tlen=%d", rx[i].length);
		printf(" flags=0x%02x", rx[i].flags );
            }

	    /* check if an CAN FD frame was received
	     * and change frame formatting rules for display */
	    if(rx[i].flags & MSG_CANFD ) {
		format = " : %c%c%c [%2d]:";
	    } else {
		format = " : %c%c%c (%2d):";
	    }
	    printf(format,
			/* extended/base */
	    		(rx[i].flags & MSG_EXT) ?
			    (rx[i].flags & MSG_SELF) ? 'E' : 'e'
			    : 
			    (rx[i].flags & MSG_SELF) ? 'B' : 'b',
			/* remote/data */
	    		(rx[i].flags & MSG_RTR) ? 'R' : 'D',
	    		(rx[i].flags & MSG_RBRS) ? 'F' : ' ',
	    		rx[i].length );
	    if( !(rx[i].flags & MSG_RTR) ) {
		int length;
		    length = rx[i].length;
		/* check canfd flag for length calculation */
		if( rx[i].flags & MSG_CANFD ) {
		    if(length > 64) length = 64;
		} else {
		    /* classic CAN */
		    /* restrict display to 8 byte */
		    if(length > 8) length = 8;
		}
		for(j = 0; j < length; j++) {
		    printf(" %02x", rx[i].data[j]);
		}
	    }
	    if(horchmode && (rx[i].id == -1)) {
	    unsigned char f = ' '; 
		printf(" Error flags=0x%03x\n\tError: ", rx[i].flags);
		if(rx[i].flags & MSG_OVR) {
		    printf("%c CAN controller msg overflow", f);
		    f = ',';
		}
		if(rx[i].flags & MSG_PASSIVE) {
		    printf("%c CAN Error Passive", f);
		    f = ',';
		}
		if(rx[i].flags & MSG_BUSOFF) {
		    printf("%c CAN controller Bus off", f);
		    f = ',';
		}
		if(rx[i].flags & MSG_WARNING) {
		    printf("%c CAN controller Error Warnig Level reached", f);
		    f = ',';
		}
		if(rx[i].flags & MSG_BOVR) {
		    printf("%c can4linux rx/tx buffer overflow", f);
		    f = ',';
		}
	    }
	    printf("\n"); fflush(stdout);
	}
      } else {
	printf("Received with ret=%d\n", got);
	fflush(stdout);
      }
      if(blocking == FALSE) {
	  /* wait some time before doing the next read() */
	  usleep(sleeptime);
      }
    }

    close(fd);
    return 0;
}

/**************************************************************************
*
* clean_up
*
*/
void clean_up(int sig)
{
    (void)sig;		/* not evaluated */
#ifdef USER_TERMIOS
    tcsetattr (0, TCSANOW, &oldtty);
#endif
    /* clean(); */ /* clean wird per atexit() eingebunden */
    exit(0);
}

void set_terminal_mode(void)
{
#ifdef USER_TERMIOS
	struct termios tty;

fprintf(stderr, "Setting RAW terminal mode\n");

	tcgetattr (0, &tty);
	oldtty = tty;

	tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
			      |INLCR|IGNCR|ICRNL|IXON);
	tty.c_oflag |= OPOST;
	tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
	tty.c_cflag &= ~(CSIZE|PARENB);
	tty.c_cflag |= CS8;
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 0;

	tcsetattr (0, TCSANOW, &tty);
	signal(SIGQUIT, clean_up); /* Quit (POSIX).  */
#else
//fprintf(stderr, "Setting RAW terminal mode using stty\n");
	int ret = system("stty cbreak -echo");
	if(ret != 0) {
	    fprintf(stderr, "  system(stty) returns %d\n", ret);
	    fflush(stderr);
	}
#endif
}

/***********************************************************************
* cut_mark - draw a line on the console
*
*/
void cut_mark(void)
{
static char line[70] =  "------------------------------------------------------------\r\n";
	puts(line);
}

/**************************************************************************
*
* clean
*
*/
static void clean(void)
{
    (void)system("stty sane");
}


/* at the CiA Plugfest
 * participants are coded in the last 4 bits of the CAN Id */
void display_participant(unsigned id)
{
char *participants[16] =  {
		"BOSCH",	/* 0 */
		"Vector",
		"Peak",
		"Infineon",
		"Renesas",
		"Spansion",
		"ESD",
		"STM",
		"IXXAT",
		"",
		"",		/* 0x0a -- 10 */
		"",
		"NI",
		"IFI emtas",	/* 0x0d  */
		NULL,
	};
	printf(" %11s ", participants[id & 0x0f]);
}

