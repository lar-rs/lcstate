/* pyCan.i
 *
 * interface file for simple layer 2 message handling using can4linux
 */

%module pyCan

%{
/* Put header files here (optional) */
#define SWIG_FILE_WITH_INIT
extern int can_open(int port, char block);
extern int can_close(int fd);
extern int can_send(int fd, int len,const char *message);
extern char *can_read(int fd);
extern char *can_read1(int fd, int timeout);
extern char *can_read2(int fd, int timeout);
%}

int can_open(int port, char block = 'n');
int can_close(int fd);
int can_send(int fd, int len,const char *message);
char *can_read(int fd);
char *can_read1(int fd, int timeout = 5000000);
char *can_read2(int fd, int timeout = 5000000);

