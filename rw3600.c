/*  open3600  - rw3600.c library functions
 *  This is a library of functions common to Linux and Windows
 *  
 *  Version 0.10
 *  
 *  Control WS3600 weather station
 *  
 *  Copyright 2003-2005, Kenneth Lavrsen, Grzegorz Wisniewski, Sander Eerkes
 *  This program is published under the GNU General Public license
 */

#include "rw3600.h"


/********************************************************************
 * read_data reads data from the WS2300 based on a given address,
 * number of data read, and a an already open serial port
 *
 * Inputs:  serdevice - device number of the already open serial port
 *          number - number of bytes to read, max value 15
 *
 * Output:  readdata - pointer to an array of chars containing
 *                     the just read data, not zero terminated
 * 
 * Returns: number of bytes read, -1 if failed
 *
 ********************************************************************/
int read_data(WEATHERSTATION ws, int number,
			  unsigned char *readdata)
{
  unsigned char command = 0xa1;
  int i;

  if (write_byte(ws,command))
  {
    for (i = 0; i < number; i++)
    {
      readdata[i] = read_byte(ws);
      if (i + 1 < number)
        read_next_byte_seq(ws);
      //printf("%i\n",readdata[i]);
    }

    read_last_byte_seq(ws);

	  return i;
  } else
    return -1;
}


/********************************************************************
 * write_data writes data to the WS2300.
 * It can both write nibbles and set/unset bits
 *
 * Inputs:      ws2300 - device number of the already open serial port
 *              address (interger - 16 bit)
 *              number - number of nibbles to be written/changed
 *                       must 1 for bit modes (SETBIT and UNSETBIT)
 *                       max 80 for nibble mode (WRITENIB)
 *              writedata - pointer to an array of chars containing
 *                          data to write, not zero terminated
 *                          data must be in hex - one digit per byte
 *                          If bit mode value must be 0-3 and only
 *                          the first byte can be used.
 * 
 * Output:      commanddata - pointer to an array of chars containing
 *                            the commands that were sent to the station
 *
 * Returns:     number of bytes written, -1 if failed
 *
 ********************************************************************/
int write_data(WEATHERSTATION ws, int address, int number,
			   unsigned char *writedata)
{
  unsigned char command = 0xa0;
  int i = -1;
  
  write_byte(ws,command);
  write_byte(ws,address/256);
  write_byte(ws,address%256);
  
  if (writedata!=NULL) {
    for (i = 0; i < number; i++)
    {
      write_byte(ws,writedata[i]);
    }
  }
  
  set_DTR(ws,0);
  nanodelay();
  set_RTS(ws,0);
  nanodelay();
  set_RTS(ws,1);
  nanodelay();
  set_DTR(ws,1);
  nanodelay();
  set_RTS(ws,0);
  nanodelay();
  
//return -1 for errors
	return i;
}


void read_next_byte_seq(WEATHERSTATION ws)
{
  print_log(3,"read_next_byte_seq");
  write_bit(ws,0);
  set_RTS(ws,0);
  nanodelay();
}

void read_last_byte_seq(WEATHERSTATION ws)
{
  print_log(3,"read_last_byte_seq");
  set_RTS(ws,1);
  nanodelay();
  set_DTR(ws,0);
  nanodelay();
  set_RTS(ws,0);
  nanodelay();
  set_RTS(ws,1);
  nanodelay();
  set_DTR(ws,1);
  nanodelay();
  set_RTS(ws,0);
  nanodelay();
}

/********************************************************************
 * read_bit  
 * Reads one bit from the COM
 *
 * Inputs:  serdevice - opened file handle
 * 
 * Returns: bit read from the COM
 *
 ********************************************************************/

int read_bit(WEATHERSTATION ws)
{
  int status;
  char str[20];
  
  set_DTR(ws,0);
  nanodelay();
  status = get_CTS(ws);
  nanodelay();
  set_DTR(ws,1);
  nanodelay();
  sprintf(str,"Read bit %i",!status);
  print_log(4,str);
  
  return !status;
}

/********************************************************************
 * write_bit  
 * Writes one bit to the COM
 *
 * Inputs:  serdevice - opened file handle
 *          bit - bit to write 
 * 
 * Returns: nothing
 *
 ********************************************************************/
void write_bit(WEATHERSTATION ws,int bit)
{
  char str[20];
  
  set_RTS(ws,!bit);
	nanodelay();
	set_DTR(ws,0);
	nanodelay();
	set_DTR(ws,1);
	
  sprintf(str,"Write bit %i",bit);
  print_log(4,str);
}



/********************************************************************
 * read_byte  
 * Reads one byte from the COM
 *
 * Inputs:  serdevice - opened file handle
 * 
 * Returns: byte read from the COM
 *
 ********************************************************************/
int read_byte(WEATHERSTATION serdevice)
{
  int byte = 0;
  int i;
  char str[20];
  
  for (i = 0; i < 8; i++)
  {
    byte *= 2;
    byte += read_bit(serdevice);
  }
  sprintf(str,"Read byte %i",byte);
  print_log(3,str);
  
  return byte;
}

/********************************************************************
 * write_byte  
 * Writes one byte to the COM
 *
 * Inputs:  serdevice - opened file handle
 *          byte - byte to write 
 * 
 * Returns: nothing
 *
 ********************************************************************/
int write_byte(WEATHERSTATION ws,int byte)
{
  int status;
  int i;
  char str[20];

  sprintf(str,"Writing byte %i",byte);
  print_log(3,str);

  for (i = 0; i < 8; i++)
  {
    write_bit(ws, byte & 0x80);
    byte <<= 1;
    byte &= 0xff;
  }

  set_RTS(ws,0);
  nanodelay();
  status = get_CTS(ws);
  //TODO: checking value of status, error routine
  nanodelay();
  set_DTR(ws,0);
  nanodelay();
  set_DTR(ws,1);
  nanodelay();
  if (status)
    return 1;
  else
    return 0;
}

/********************************************************************
 * log
 ********************************************************************/
void print_log(int log_level, char* str) {
  if (log_level < 0)
    fprintf(stderr,"%s\n",str);
}
