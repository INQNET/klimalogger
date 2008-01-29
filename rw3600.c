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
 * read_error_exit
 * exit location for all calls to read_safe for error exit.
 * includes error reporting.
 *
 ********************************************************************/
void read_error_exit(void)
{
	perror("read_safe() error");
	exit(0);
}

/********************************************************************
 * write_error_exit
 * exit location for all calls to write_safe for error exit.
 * includes error reporting.
 *
 ********************************************************************/
void write_error_exit(void)
{
	perror("write_safe() error");
	exit(0);
}


/********************************************************************
 * command_check0123 calculates the checksum for the first 4
 * commands sent to WS2300.
 *
 * Input:   pointer to char to check
 *          sequence of command - i.e. 0, 1, 2 or 3.
 * 
 * Returns: calculated checksum as unsigned char
 *
 ********************************************************************/
unsigned char command_check0123(unsigned char *command, int sequence)
{
	int response;

	response = sequence * 16 + ((*command) - 0x82) / 4;

	return (unsigned char) response;
}


/********************************************************************
 * command_check4 calculates the checksum for the last command
 * which is sent just before data is received from WS2300
 *
 * Input: number of bytes requested
 * 
 * Returns: expected response from requesting number of bytes
 *
 ********************************************************************/
unsigned char command_check4(int number)
{
	int response;

	response = 0x30 + number;

	return response;
}


/********************************************************************
 * data_checksum calculates the checksum for the data bytes received
 * from the WS2300
 *
 * Input:   pointer to array of data to check
 *          number of bytes in array
 * 
 * Returns: calculated checksum as unsigned char
 *
 ********************************************************************/
unsigned char data_checksum(unsigned char *data, int number)
{
	int checksum = 0;
	int i;

	for (i = 0; i < number; i++)
	{
		checksum += data[i];
	}

	checksum &= 0xFF;

	return (unsigned char) checksum;
}


/********************************************************************
 * initialize resets WS2300 to cold start (rewind and start over)
 *
 * Input:   device number of the already open serial port
 *
 * Returns: 0 if fail, 1 if success
 *
 ********************************************************************/
int initialize(WEATHERSTATION ws2300)
{
	unsigned char command = 0x06;
	unsigned char answer;

	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	write_device(ws2300, &command, 1);
	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	if (answer != 2)
		return 0;

	return 1;
}


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
  nanodelay(DELAY_CONST);
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
  set_RTS(ws,1);
  nanodelay(DELAY_CONST);
  set_DTR(ws,1);
  nanodelay(DELAY_CONST);
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
  
//return -1 for errors
	return i;
}


/********************************************************************
 * read_safe Read data, retry until success or maxretries
 * Reads data from the WS2300 based on a given address,
 * number of data read, and a an already open serial port
 * Uses the read_data function and has same interface
 *
 * Inputs:  ws2300 - device number of the already open serial port
 *          address (interger - 16 bit)
 *          number - number of bytes to read, max value 15
 *
 * Output:  readdata - pointer to an array of chars containing
 *                     the just read data, not zero terminated
 *          commanddata - pointer to an array of chars containing
 *                     the commands that were sent to the station
 * 
 * Returns: number of bytes read, -1 if failed
 *
 ********************************************************************/
int read_safe(WEATHERSTATION ws, int address, int number,
			  unsigned char *readdata, unsigned char *commanddata)
{
	int i,j;
	unsigned char readdata2[32768];
	
	print_log(1,"read_safe");

	for (j = 0; j < MAXRETRIES; j++)
	{	
		write_data(ws, address, 0, NULL);
    read_data(ws, number, readdata);
    
    write_data(ws, address, 0, NULL);
    read_data(ws, number, readdata2);
    
    if (memcmp(readdata,readdata2,number) == 0)
    {
      //check if only 0's for reading memory range greater then 10 bytes
      print_log(2,"read_safe - two readings identical");
      i = 0;
      if (number > 10)
      {
        for (; readdata[i] == 0 && i < number; i++);
      }
      
      if (i != number)
        break;
      else
        print_log(2,"read_safe - only zeros");
    } else
      print_log(2,"read_safe - two readings not identical");
	}

	// If we have tried MAXRETRIES times to read we expect not to
	// have valid data
	if (j == MAXRETRIES)
	{
		return -1;
	}

	return number;
}


/********************************************************************
 * write_safe Write data, retry until success or maxretries
 * Writes data to the WS2300 based on a given address,
 * number of data to write, and a an already open serial port
 * Uses the write_data function and has same interface
 *
 * Inputs:      serdevice - device number of the already open serial port
 *              address (interger - 16 bit)
 *              number - number of nibbles to be written/changed
 *                       must 1 for bit modes (SETBIT and UNSETBIT)
 *                       unlimited for nibble mode (WRITENIB)
 *              encode_constant - unsigned char
 *                               (SETBIT, UNSETBIT or WRITENIB)
 *              writedata - pointer to an array of chars containing
 *                          data to write, not zero terminated
 *                          data must be in hex - one digit per byte
 *                          If bit mode value must be 0-3 and only
 *                          the first byte can be used.
 * 
 * Output:      commanddata - pointer to an array of chars containing
 *                            the commands that were sent to the station
 * 
 * Returns: number of bytes written, -1 if failed
 *
 ********************************************************************/
int write_safe(WEATHERSTATION ws2300, int address, int number,
               unsigned char encode_constant, unsigned char *writedata,
               unsigned char *commanddata)
{
	int j;

  print_log(2,"write_safe");
	for (j = 0; j < MAXRETRIES; j++)
	{
		// printf("Iteration = %d\n",j); // debug
		//reset_06(ws2300);

		// Read the data. If expected number of bytes read break out of loop.
		/*if (write_data(ws2300, address, number, encode_constant, writedata,
		    commanddata)==number)
		{
			break;
		}*/
	}

	// If we have tried MAXRETRIES times to read we expect not to
	// have valid data
	if (j == MAXRETRIES)
	{
		return -1;
	}

	return number;
}

void read_next_byte_seq(WEATHERSTATION ws)
{
  print_log(3,"read_next_byte_seq");
  write_bit(ws,0);
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
}

void read_last_byte_seq(WEATHERSTATION ws)
{
  print_log(3,"read_last_byte_seq");
  set_RTS(ws,1);
  nanodelay(DELAY_CONST);
  set_DTR(ws,0);
  nanodelay(DELAY_CONST);
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
  set_RTS(ws,1);
  nanodelay(DELAY_CONST);
  set_DTR(ws,1);
  nanodelay(DELAY_CONST);
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
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
  nanodelay(DELAY_CONST);
  status = get_CTS(ws);
  nanodelay(DELAY_CONST);
  set_DTR(ws,1);
  nanodelay(DELAY_CONST);
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
	nanodelay(DELAY_CONST);
	set_DTR(ws,0);
	nanodelay(DELAY_CONST);
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
  
  sprintf(str,"Read byte %i",byte);
  print_log(3,str);
  
  for (i = 0; i < 8; i++)
  {
    write_bit(ws, byte & 0x80);
    byte <<= 1;
    byte &= 0xff;
  }
  
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
  status = get_CTS(ws);
  //TODO: checking value of status, error routine
  nanodelay(DELAY_CONST);
  set_DTR(ws,0);
  nanodelay(DELAY_CONST);
  set_DTR(ws,1);
  nanodelay(DELAY_CONST);
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
