/* Include file for the open3600 Linux specific functions
 */
 
#ifndef _INCLUDE_LINUX3600_H_
#define _INCLUDE_LINUX3600_H_ 

#include <termios.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/file.h>

#define BUFFER_SIZE 16384
#define DELAY_CONST 1
#define INIT_WAIT 500 

#define BAUDRATE B300
#define DEFAULT_SERIAL_DEVICE "/dev/ttyS0"

typedef int WEATHERSTATION;

#endif /* _INCLUDE_LINUX3600_H_ */

