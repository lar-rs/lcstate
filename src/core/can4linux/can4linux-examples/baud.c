/* simple driver test: change the bit rate registers with ioctl()
* 
*
* first argument can be the device name -- else it uses can0
*
* if a second arg is given, it is used as new bit rate
* 
* The example also demonstrates CAN auto bit rate detection.
* In this case /proc/sys/dev/Can/Baud entry for this channel 
* should be set to 0 in order to start without valid bit rate
* when open() the CAN channel.
*
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <can4linux.h>

#define STDDEV "can0"
#define SLEEPTIME		30 /* seconds before closing the CAN channel */

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
	int can_fd,			/* device descriptor */
	int baud		/* bit rate */
	)
{
config_par_t  cfg;
volatile command_par_t cmd;


    cmd.cmd = CMD_STOP;
    ioctl(can_fd, CAN_IOCTL_COMMAND, &cmd);

    cfg.target = CONF_TIMING; 
    cfg.val1   = baud;
    ioctl(can_fd, CAN_IOCTL_CONFIG, &cfg);

    cmd.cmd = CMD_START;
    ioctl(can_fd, CAN_IOCTL_COMMAND, &cmd);
    return 0;
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
int set_lomode(
	int can_fd,			/* device descriptor */
	int flag			/* true or false */
	)
{
config_par_t  cfg;
volatile command_par_t cmd;
int ret;

    cmd.cmd = CMD_STOP;
    ioctl(can_fd, CAN_IOCTL_COMMAND, &cmd);

    cfg.target = CONF_LISTEN_ONLY_MODE; 
    cfg.val1   = flag ? 1 : 0;
    ret = ioctl(can_fd, CAN_IOCTL_CONFIG, &cfg);

    cmd.cmd = CMD_START;
    ioctl(can_fd, CAN_IOCTL_COMMAND, &cmd);

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
* do_autobaud - do auto bit rate detection
*
* "Automatic Baudrate Detection in CANopen Networks"
* 	U. Koppe, MicroControl GmbH & Co. KG
*
* RETURN: 0 - no success
* RETURN. bit rate detected
*
*/
int do_autobaud(can_fd)
{
int bitrate[] = {10, 20,  50, 125, 250, 500, 800, 1000 , 0};
int i;
int ret;
can_statuspar_t status;

	printf("\tswitch into listen only mode\n"); 
	set_lomode(can_fd, 1);
	/* for all possible bit rates */
	for (i = 0; ; i++) {
		if (bitrate[i] == 0) {
			ret = 0;
			break;
		}
		printf("\tTry %d Kbit/s\n", bitrate[i]);
		set_bitrate(can_fd, bitrate[i]);
		/* wait one second */
		sleep(1);
		ioctl(can_fd, CAN_IOCTL_STATUS, &status);
		// printf("Rx Errors: %d\n", status.rx_errors);
		// printf("Tx Errors: %d\n", status.tx_errors);
		// printf("Rx buffer: %d\n", status.rx_buffer_used);
		if (status.rx_buffer_used > 1) {
			printf("\t\tCould receive frames. Switch back to active mode\n");
			ret = bitrate[i];
			break;
		}
	}
	if (ret > 0) {
		/* bit rate detected */
		set_lomode(can_fd, 0);
	}
	return ret;
}



/*
* main
*
*/
int main(int argc,char **argv)
{
int can_fd;
char device[40];
int autobaud = 0;
int newbaud = 250;

    printf("usage: %s [dev] [bit_rate]\n", argv[0]);
    printf("   e.g.:\n");
    printf("   ./baud /dev/can0 125\n");
    printf("   sends out a message at /dev/can0 with 125Kbit/s\n");
    printf("   which can be watched at the CAN cable using an scope\n\n");
    printf("   After sending the frame, the process sleeps for %d s\n", SLEEPTIME);
    printf("   before closing CAN.\n\n");
    printf("   If bit_rate is the letter 'a' autobaud is used to determine current bit rate.\n");
    printf("   on the bus.\n\n");


    sprintf(device, "%s", argv[1]);
    printf("using CAN device %s\n", device);
    
    if(( can_fd = open(device, O_RDWR )) < 0 ) {
	fprintf(stderr,"Error opening CAN device %s\n", device);
        exit(1);
    }
    if (argc == 3) {
		if (*argv[2] == 'a') {
			autobaud = 1;
		} else 
			newbaud = atoi(argv[2]);
    }

	if (autobaud) {
		int bitrate;
		printf("doing auto bit rate detection now\n");
		printf("take care that traffic is on the bus\n"); 
		bitrate = do_autobaud(can_fd);
		printf("Detected Bit rate is %d, using it now to send a frame\n",
			bitrate);
	} else {
		printf("set baudrate to %d Kbit/s\n", newbaud);
		set_bitrate(can_fd, newbaud);
	}

    /* Use the new CAN bit rate to send one message 
     * If no other CAN node is connected, we can see this message
     * using an oscilloscope and we can measure the bit width 
     */
    {
    canmsg_t txmsg;
    int ret;

    	txmsg.id = 0x555;
    	txmsg.flags = 0;
    	txmsg.length = 8;
    	txmsg.data[0] = 0x55;
    	txmsg.data[1] = 0x55;
    	txmsg.data[2] = 0x55;
    	txmsg.data[3] = 0x55;
    	txmsg.data[4] = 0x55;
    	txmsg.data[5] = 0x55;
    	txmsg.data[6] = 0x55;
    	txmsg.data[7] = 0x55;

	ret = write(can_fd, &txmsg, 1);
	if (ret == -1) {
	    /* int e = errno; */
	    perror("write error");
	    /* } */
	} else if (ret == 0) {
	    printf("transmit timed out\n");
	} else {
	}
    }

    sleep(SLEEPTIME);    
    close(can_fd);
    return 0;
}

