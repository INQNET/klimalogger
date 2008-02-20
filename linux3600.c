/*  open3600  - linux3600 library functions
 *  This file contains the common functions that are unique to
 *  Linux. The entire file is ignored in case of Windows
 *  
 *  Version 0.01
 *  
 *  Control WS3600 weather station
 *  
 *  Copyright 2003-2005, Kenneth Lavrsen, Grzegorz Wisniewski, Sander Eerkes
 *  This program is published under the GNU General Public license
 */

#define DEBUG 0

#include "eeprom.h"
#include "mcdelay.h"
#include "mcdelay.c"

/********************************************************************
 * open_weatherstation, Windows version
 *
 * Input:   devicename (COM1, COM2 etc)
 * 
 * Returns: Handle to the weatherstation (type WEATHERSTATION)
 *
 ********************************************************************/
WEATHERSTATION open_weatherstation (char *device) {
  WEATHERSTATION ws;
  struct termios adtio;
  unsigned char buffer[BUFFER_SIZE];
  long i;
  print_log(1,"open_weatherstation");

  //calibrate nanodelay function
  microdelay_init(1);

  //Setup serial port
  if ((ws = open(device, O_RDWR | O_NOCTTY)) < 0)
  {
    printf("\nUnable to open serial device %s\n", device);
    exit(EXIT_FAILURE);
  }

  if ( flock(ws, LOCK_EX) < 0 ) {
    perror("\nSerial device is locked by other program\n");
    exit(EXIT_FAILURE);
  }
  //We want full control of what is set and simply reset the entire adtio struct
  memset(&adtio, 0, sizeof(adtio));
  // Serial control options
  adtio.c_cflag &= ~PARENB;      // No parity
  adtio.c_cflag &= ~CSTOPB;      // One stop bit
  adtio.c_cflag &= ~CSIZE;       // Character size mask
  adtio.c_cflag |= CS8;          // Character size 8 bits
  adtio.c_cflag |= CREAD;        // Enable Receiver
  //adtio.c_cflag &= ~CREAD;        // Disable Receiver
  adtio.c_cflag &= ~HUPCL;       // No "hangup"
  adtio.c_cflag &= ~CRTSCTS;     // No flowcontrol
  adtio.c_cflag |= CLOCAL;       // Ignore modem control lines

  // Baudrate, for newer systems
  cfsetispeed(&adtio, BAUDRATE);
  cfsetospeed(&adtio, BAUDRATE);	

  // Serial local options: adtio.c_lflag
  // Raw input = clear ICANON, ECHO, ECHOE, and ISIG
  // Disable misc other local features = clear FLUSHO, NOFLSH, TOSTOP, PENDIN, and IEXTEN
  // So we actually clear all flags in adtio.c_lflag
  adtio.c_lflag = 0;

  // Serial input options: adtio.c_iflag
  // Disable parity check = clear INPCK, PARMRK, and ISTRIP 
  // Disable software flow control = clear IXON, IXOFF, and IXANY
  // Disable any translation of CR and LF = clear INLCR, IGNCR, and ICRNL	
  // Ignore break condition on input = set IGNBRK
  // Ignore parity errors just in case = set IGNPAR;
  // So we can clear all flags except IGNBRK and IGNPAR
  adtio.c_iflag = IGNBRK|IGNPAR;

  // Serial output options
  // Raw output should disable all other output options
  adtio.c_oflag &= ~OPOST;

  adtio.c_cc[VTIME] = 10;		// timer 1s
  adtio.c_cc[VMIN] = 0;		// blocking read until 1 char

  if (tcsetattr(ws, TCSANOW, &adtio) < 0)
  {
	  printf("Unable to initialize serial device");
	  exit(0);
  }
  tcflush(ws, TCIOFLUSH);
  
  for (i = 0; i < 448; i++) {
    buffer[i] = 'U';
  }
  write(ws, buffer, 448);

  set_DTR(ws,0);
  set_RTS(ws,0);
  i = 0;
  do {
    sleep_short(10);
    i++;
  } while (i < INIT_WAIT && !get_DSR(ws));

  if (i == INIT_WAIT)
  {
    print_log(2,"Connection timeout 1");
    printf ("Connection timeout\n");
    close_weatherstation(ws);
    exit(0);
  }
  i = 0;
  do {
    sleep_short(10);
    i++;
  } while (i < INIT_WAIT && get_DSR(ws));

  if (i != INIT_WAIT) {
    set_RTS(ws,1);
    set_DTR(ws,1);
  } else {
    print_log(2,"Connection timeout 2");
    printf ("Connection timeout\n");
    close_weatherstation(ws);
    exit(0);
  }
  write(ws, buffer, 448);
  return ws;
}


/********************************************************************
 * close_weatherstation, Linux version
 *
 * Input: Handle to the weatherstation (type WEATHERSTATION)
 *
 * Returns nothing
 *
 ********************************************************************/
void close_weatherstation(WEATHERSTATION ws)
{
  tcflush(ws,TCIOFLUSH);
  close(ws);
  return;
}

/********************************************************************
 * set_DTR  
 * Sets or resets DTR signal
 *
 * Inputs:  serdevice - opened file handle
 *          val - value to set 
 * 
 * Returns nothing
 *
 ********************************************************************/

void set_DTR(WEATHERSTATION ws, int val)
{
  //TODO: use TIOCMBIC and TIOCMBIS instead of TIOCMGET and TIOCMSET
  int portstatus;
  ioctl(ws, TIOCMGET, &portstatus);	// get current port status
  if (val)
  {
    print_log(5,"Set DTR");
    portstatus |= TIOCM_DTR;
  }
  else
  {
    print_log(5,"Clear DTR");
    portstatus &= ~TIOCM_DTR;
  }
  ioctl(ws, TIOCMSET, &portstatus);	// set current port status
  
  /*if (val)
    ioctl(ws, TIOCMBIS, TIOCM_DTR);
  else
    ioctl(ws, TIOCMBIC, TIOCM_DTR);*/
}

/********************************************************************
 * set_RTS  
 * Sets or resets RTS signal
 *
 * Inputs:  serdevice - opened file handle,
 *          val - value to set 
 * 
 * Returns nothing
 *
 ********************************************************************/

void set_RTS(WEATHERSTATION ws, int val)
{
  //TODO: use TIOCMBIC and TIOCMBIS instead of TIOCMGET and TIOCMSET
  int portstatus;
  ioctl(ws, TIOCMGET, &portstatus);	// get current port status
  if (val)
  {
    print_log(5,"Set RTS");
    portstatus |= TIOCM_RTS;
  }
  else
  {
    print_log(5,"Clear RTS");
    portstatus &= ~TIOCM_RTS;
  }
  ioctl(ws, TIOCMSET, &portstatus);	// set current port status
  
  /*if (val)
    ioctl(ws, TIOCMBIS, TIOCM_RTS);
  else
    ioctl(ws, TIOCMBIC, TIOCM_RTS);
  */
  
}

/********************************************************************
 * get_DSR  
 * Checks status of DSR signal
 *
 * Inputs:  ws - opened file handle
 *          
 * 
 * Returns: status of DSR signal
 *
 ********************************************************************/

int get_DSR(WEATHERSTATION ws)
{
  int portstatus;
  ioctl(ws, TIOCMGET, &portstatus);	// get current port status

  if (portstatus & TIOCM_DSR)
  {
    print_log(5,"Got DSR = 1");
    return 1;
  }
  else
  {
    print_log(5,"Got DSR = 0");
    return 0;
  }
}

/********************************************************************
 * get_CTS
 * Checks status of CTS signal
 *
 * Inputs:  ws - opened file handle
 *          
 * 
 * Returns: status of CTS signal
 *
 ********************************************************************/

int get_CTS(WEATHERSTATION ws)
{
  int portstatus;
  ioctl(ws, TIOCMGET, &portstatus);	// get current port status

  if (portstatus & TIOCM_CTS)
  {
    print_log(5,"Got CTS = 1");
    return 1;
  }
  else
  {
    print_log(5,"Got CTS = 0");
    return 0;
  }
}

/********************************************************************
 * sleep_short - Linux version
 * 
 * Inputs: Time in milliseconds (integer)
 *
 * Returns: nothing
 *
 ********************************************************************/
void sleep_short(int milliseconds)
{
	usleep(milliseconds * 1000);
}

/* Note: if you see timing issues, maybe you need to adjust this ... */
void nanodelay() {
	microdelay(10);
}

