/* simple driver test: control on-board LEDs with ioctl()
*
* The firat board supporting this is CC_PCICAN (Contemporary Controls)
*
*/

#include <stdio.h>
#include <stdlib.h>		/*  atol() */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>		/* system() */
#include <string.h>
#include <can4linux.h>

#define STDDEV "can0"


void usage(char *pname)
{
    printf("Activates/deactivates the on board CAN termination resistor\n");
    printf("usage: %s dev on|off\n", pname);
    printf("   e.g.:\n");
    printf("   %s /dev/can0 on\n", pname);
}


/*
*
*
*/
int main(int argc,char **argv)
{
int can_fd;
char device[40];
enum can_termination_state state;
Command_par_t cmd;
int can_device;
char command[50];	/* holding the shell command line for awk */
int fd;

    if(argc < 3) {
	usage(argv[0]);
	exit(1);
    }

    /* Now checking the state for on/off */
    if (0 == strncmp("on", argv[2], 2)) {
	state = on;
    }
    else
    if (0 == strncmp("off", argv[2], 3)) {
	state = off;
    }
    else {
	usage(argv[0]);
	exit(1);
    }

    /* all parameters are checked */
    /* open the driver */
    sprintf(device, "%s", argv[1]);
    printf("using CAN device %s\n", device);
    if(( can_fd = open(device, O_WRONLY /*O_RDWR*/ )) < 0 ) {
	fprintf(stderr,"Error opening CAN device %s\n", device);
        exit(1);
    }

    /* get the board id, only on Janz CAN-PCIL */
    /* check if /proc entry is available */
    fd = open("/proc/sys/dev/Can/BoardId", O_RDONLY);
    if (fd > 0) {
	close(fd);
	/* get the CAN device number out of /dev/can* */
	can_device = atoi(device + 8);
	can_device++;
	printf("CAN Board-Id is: "); fflush(stdout);
	sprintf(command, " awk '{ print $%d }' '/proc/sys/dev/Can/BoardId'", can_device);
	system(command);
    }

    cmd.cmd  = CMD_CTRL_TERM;
    cmd.val1 = state;
    ioctl(can_fd, CAN_IOCTL_COMMAND, &cmd);

    close(can_fd);
    return 0;
}

