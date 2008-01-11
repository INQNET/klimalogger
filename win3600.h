/* Include file for the open3600 Windows specific functions
 */

#ifndef _INCLUDE_WIN3600_H_
#define _INCLUDE_WIN3600_H_ 

#include <windows.h>
#include <winsock.h>
#include "ntddser.h"

#define STRINGIZE(x) #x
#define BUFFER_SIZE 16384
#define DELAY_CONST 1000
#define INIT_WAIT 500

typedef HANDLE WEATHERSTATION;

#define BAUDRATE CBR_300
#define DEFAULT_SERIAL_DEVICE "COM1"

#endif /* _INCLUDE_WIN3600_H_ */

