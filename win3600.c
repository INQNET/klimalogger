/*  open3600  - win3600 library functions
 *  This file contains the common functions that are unique to
 *  windows. The entire file is ignored in case of Linux
 *  
 *  Version 0.01
 *  
 *  Control WS3600 weather station
 *  
 *  Copyright 2003-2005, Kenneth Lavrsen, Grzegorz Wisniewski, Sander Eerkes
 *  This program is published under the GNU General Public license
 */

#ifdef WIN32
#define DEBUG 0

#include "rw3600.h"

/********************************************************************
 * open_weatherstation, Windows version
 *
 * Input:   devicename (COM1, COM2 etc)
 * 
 * Returns: Handle to the weatherstation (type WEATHERSTATION)
 *
 ********************************************************************/
WEATHERSTATION open_weatherstation (char *device)
{
	WEATHERSTATION ws;
	DCB dcb;
	COMMTIMEOUTS commtimeouts;
	SERIAL_QUEUE_SIZE queueSize;
	unsigned char buffer[BUFFER_SIZE];
	long i;
	LPDWORD lpBytesReturned;
  ULONG status;
  DWORD EventMask;
  DWORD Events;

  print_log(1,"open_weatherstation");
  //calibrate nanodelay function
  spins_per_ns = (float) calibrate() * (float) CLOCKS_PER_SEC * 1.0e-9f;  
  //printf("spins_per_ns=%.2f\n",spins_per_ns);
  //printf("CLOCKS_PER_SEC=%.2f\n",((float) CLOCKS_PER_SEC));
	//Setup serial port

	ws = CreateFile( device,
	               GENERIC_READ | GENERIC_WRITE,
	               0,    // must be opened with exclusive-access
	               NULL, // no security attributes
	               OPEN_EXISTING, // must use OPEN_EXISTING
	               FILE_FLAG_OVERLAPPED,    // not overlapped I/O
	               NULL  // hTemplate must be NULL for comm devices
	               );
	                     
	if (ws == INVALID_HANDLE_VALUE)
	{
		printf ("\nUnable to open serial device");
		exit (0);
	}

  queueSize.InSize = BUFFER_SIZE;
  queueSize.OutSize = BUFFER_SIZE;
  DeviceIoControl(ws,IOCTL_SERIAL_SET_QUEUE_SIZE,&queueSize,sizeof(queueSize),NULL,0,&lpBytesReturned,NULL);
  
	if (!GetCommState (ws, &dcb))
	{
		printf ("\nUnable to GetCommState");
		exit (0);
	}

	dcb.DCBlength = sizeof (DCB);
	dcb.BaudRate = BAUDRATE;
	dcb.fBinary = FALSE;
	dcb.fParity = FALSE;
	dcb.fOutxCtsFlow = FALSE;
	dcb.fOutxDsrFlow = FALSE;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	dcb.fDsrSensitivity = FALSE;
	dcb.fTXContinueOnXoff = TRUE;
	dcb.fOutX = 0;
	dcb.fInX = 0;
	dcb.fErrorChar = FALSE;
	dcb.fNull = FALSE;
	dcb.fRtsControl = RTS_CONTROL_DISABLE;
	dcb.fAbortOnError = FALSE;
	dcb.XonLim = 0;
  dcb.XoffLim = 0;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;
	dcb.fNull = false;
	dcb.XonChar = NULL;
  dcb.XoffChar = NULL;
  dcb.ErrorChar = NULL;
  dcb.EofChar = NULL;
  dcb.EvtChar = NULL;

	if (!SetCommState (ws, &dcb))
	{
		printf ("\nUnable to SetCommState");
		exit (0);
	}

	commtimeouts.ReadIntervalTimeout = 0;
	commtimeouts.ReadTotalTimeoutMultiplier = 0;
	commtimeouts.ReadTotalTimeoutConstant = 0;
	commtimeouts.WriteTotalTimeoutConstant = 0;
	commtimeouts.WriteTotalTimeoutMultiplier = 0;

	if (!SetCommTimeouts (ws, &commtimeouts))
	{
		printf ("\nUnable to SetCommTimeouts");
		exit (0);
	}
  
  for (i = 0; i < 2000; i++) {
    buffer[i] = 'U';
  }
  
  write_device(ws, buffer, 2000);
  
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
  } else
  {
    print_log(2,"Connection timeout 2");
    printf ("Connection timeout\n");
    close_weatherstation(ws);
    exit(0);
  }
  write_device(ws, buffer, 2000);
  
	return ws;
}


/********************************************************************
 * close_weatherstation, windows version
 *
 * Input: Handle to the weatherstation (type WEATHERSTATION)
 *
 * Returns nothing
 *
 ********************************************************************/
void close_weatherstation (WEATHERSTATION ws)
{
	CloseHandle (ws);
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

void set_DTR(WEATHERSTATION serdevice, int val)
{
  LPDWORD lpBytesReturned;
  
  if (val)
  {
    print_log(5,"Set DTR");
    DeviceIoControl(serdevice,IOCTL_SERIAL_SET_DTR,NULL,0,NULL,0,&lpBytesReturned,NULL);
  }
  else
  {
    DeviceIoControl(serdevice,IOCTL_SERIAL_CLR_DTR,NULL,0,NULL,0,&lpBytesReturned,NULL);
    print_log(5,"Clear DTR");
  }
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

void set_RTS(WEATHERSTATION serdevice, int val)
{
  LPDWORD lpBytesReturned;
  
  
  if (val)
  {
    print_log(5,"Set RTS");
    DeviceIoControl(serdevice,IOCTL_SERIAL_SET_RTS,NULL,0,NULL,0,&lpBytesReturned,NULL);
  }
  else
  {
    print_log(5,"Clear RTS");
    DeviceIoControl(serdevice,IOCTL_SERIAL_CLR_RTS,NULL,0,NULL,0,&lpBytesReturned,NULL);
  }
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
  ULONG status;
  LPDWORD lpBytesReturned;
  
  DeviceIoControl(ws,IOCTL_SERIAL_GET_MODEMSTATUS,NULL,0,&status,sizeof(status),&lpBytesReturned,NULL);
  if (status & SERIAL_DSR_STATE)
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
  ULONG status;
  LPDWORD lpBytesReturned;
  
  DeviceIoControl(ws,IOCTL_SERIAL_GET_MODEMSTATUS,NULL,0,&status,sizeof(status),&lpBytesReturned,NULL);
  if (status & SERIAL_CTS_STATE)
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
 * read_device WIN32 emulation of Linux read() 
 * Reads data from the handle
 *
 * Inputs:  serdevice - opened file handle
 *          buffer - pointer to the buffer to read into
 *          size - number of bytes to read
 *
 * Output:  *buffer - modified on success
 * 
 * Returns: number of bytes read
 *
 ********************************************************************/
int read_device(WEATHERSTATION serdevice, unsigned char *buffer, int size)
{
	DWORD dwRead = 0;

	if (!ReadFile(serdevice, buffer, size, &dwRead, NULL))
	{
		return -1;
	}

	return (int) dwRead;
}

/********************************************************************
 * write_device WIN32 emulation of Linux write() 
 * Writes data to the handle
 *
 * Inputs:  serdevice - opened file handle
 *          buffer - pointer to the buffer to write from
 *          size - number of bytes to write
 *
 * Returns: number of bytes written
 *
 ********************************************************************/
int write_device(WEATHERSTATION serdevice, unsigned char *buffer, int size)
{
	DWORD dwWritten;
	OVERLAPPED oOverlap;

	if (!WriteFile(serdevice, buffer, size, &dwWritten, &oOverlap))
	{
		return -1;
	}

	return (int) dwWritten;
}

/********************************************************************
 * sleep_short - Windows version
 * 
 * Inputs: Time in milliseconds (integer)
 *
 * Returns: nothing
 *
 ********************************************************************/
void sleep_short(int milliseconds)
{
	Sleep(milliseconds);
}

/********************************************************************
 * sleep_long - Windows version
 * 
 * Inputs: Time in seconds (integer)
 *
 * Returns: nothing
 *
 ********************************************************************/
void sleep_long(int seconds)
{
	Sleep(seconds*1000);
}

/********************************************************************
 * http_request_url - Windows version
 * 
 * Inputs: urlline - URL to Weather Underground with path and data
 *                   as a pointer to char array (string)
 *
 * Returns: 0 on success and -1 if fail.
 *
 * Action: Send a http request to Weather Underground
 *
 ********************************************************************/
int http_request_url(char *urlline)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	SOCKET sockfd;
	struct hostent *hostinfo;
	struct sockaddr_in urladdress;
	char buffer[1024];
	int bytes_read;
	
	wVersionRequested = MAKEWORD( 1, 1 );
	
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 )
	{
		perror("Couldn't find a useable winsock.dll");
	    return(-1);
	}
	
	/* Confirm that the Windows Sockets DLL supports 1.1.*/
	
	if ( LOBYTE( wsaData.wVersion ) != 1 ||
	         HIBYTE( wsaData.wVersion ) != 1 )
	{
	    WSACleanup();
		perror("Couldn't find a useable winsock.dll");
	    return(-1);   
	}

	if ( (hostinfo = gethostbyname(WEATHER_UNDERGROUND_BASEURL)) == NULL )
	{
		perror("Host not known by DNS server or DNS server not working");
		return(-1);
	}
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd == INVALID_SOCKET)
	{
		perror("Cannot open socket");
		return(-1);
	}

	memset(&urladdress, 0, sizeof(urladdress));
	urladdress.sin_family = AF_INET;
	urladdress.sin_port = htons(80); /*default HTTP Server port */

	urladdress.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;

	if (connect(sockfd,(struct sockaddr*)&urladdress,sizeof(urladdress)) != 0)
	{
		perror("Cannot connect to host");
		return(-1);
	}
	sprintf(buffer, "GET %s\nHTTP/1.0\n\n", urlline);
	send(sockfd, buffer, strlen(buffer), 0);

	/* While there's data, read and print it */
	do
	{
		memset(buffer, 0, sizeof(buffer));
		bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
		if ( bytes_read > 0 )
			if (DEBUG) printf("%s", buffer);
	}
	while ( bytes_read > 0 );

	/* Close socket and clean up winsock */
	closesocket(sockfd);
	
	WSACleanup();
	
	return(0);
}

/********************************************************************
 * citizen_weather_send - windows version
 * 
 * Inputs: config structure (pointer to) - containing CW ID
 *         datastring (pointer to) - containing all the data
 *
 * Returns: 0 on success and -1 if fail.
 *
 * Action: Send a http request to Weather Underground
 *
 ********************************************************************/
int citizen_weather_send(struct config_type *config, char *aprsline)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	SOCKET sockfd = INVALID_SOCKET; // Prevent warning
	struct hostent *hostinfo;
	struct sockaddr_in urladdress;
	char buffer[1024];
	int bytes_read;
	int hostnum;
	
	wVersionRequested = MAKEWORD( 1, 1 );
	
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 )
	{
		perror("Couldn't find a useable winsock.dll");
	    return(-1);
	}
	
	/* Confirm that the Windows Sockets DLL supports 1.1.*/
	
	if ( LOBYTE( wsaData.wVersion ) != 1 ||
	         HIBYTE( wsaData.wVersion ) != 1 )
	{
	    WSACleanup();
		perror("Couldn't find a useable winsock.dll");
	    return(-1);   
	}


	for (hostnum = 0; hostnum <= config->num_hosts; hostnum++)
	{
		if ( hostnum == config->num_hosts )
			return(-1);          // tried 'em all, fail exit		

		if ( (hostinfo = gethostbyname(config->aprs_host[hostnum].name) ) == NULL )		
		{
			sprintf(buffer,"Host, %s, not known ", config->aprs_host[hostnum].name);
			perror(buffer);
			continue;
		}
				
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
		if (sockfd == INVALID_SOCKET)
		{
			sprintf(buffer,"Cannot open socket on %s ", config->aprs_host[hostnum].name);
			perror(buffer);
			continue;
		}
		
		memset(&urladdress, 0, sizeof(urladdress));// clear the structure
		urladdress.sin_family = AF_INET;
		urladdress.sin_port = htons(config->aprs_host[hostnum].port);
		urladdress.sin_addr = *(struct in_addr *)*hostinfo->h_addr_list;

		if ( connect(sockfd, (struct sockaddr*)&urladdress, sizeof(urladdress)) != 0 )
		{
			sprintf(buffer,"Cannot connect to host: %s ", config->aprs_host[hostnum].name);
			perror(buffer);
			continue;
		}
		else
		{
			break;   // success
		}
	}

	if (DEBUG) printf("%d: %s: ",hostnum, config->aprs_host[hostnum].name);

	memset(buffer, 0, sizeof(buffer));

	// read login prompt
	if ( (recv(sockfd, buffer, sizeof(buffer), 0) > 0) && (DEBUG != 0) )
	{
		printf("%s", buffer);	// display prompt - if debug
	}

	// The login/header line
	sprintf(buffer,"user %s pass -1 vers open3600 %s\n",
	        config->citizen_weather_id, VERSION);
	send(sockfd, buffer, strlen(buffer), 0);
	if (DEBUG)
		printf("%s\n", buffer);

	// now the data
	sprintf(buffer,"%s\n", aprsline);
	send(sockfd, buffer, strlen(buffer), 0);
	if (DEBUG)
		printf("%s\n", buffer);

	/* While there's data, read and print it - Not sure it is needed */
	do
	{
		memset(buffer, 0, sizeof(buffer));
		bytes_read = recv(sockfd, buffer, sizeof(buffer), 0);
		if ( bytes_read > 0 )
		{
			if (DEBUG)
				printf("Data returned from server\n%s\n", buffer);
			break;
		}
	}
	while ( bytes_read > 0 );

	/* Close socket*/
	close(sockfd);
	
	WSACleanup();

	return(0);
}

/********************************************************************
 * delay_loop  
 * delay function used in nanodelay
 *
 * Inputs:  count - number of loop iterations
 * 
 * 
 * Returns: nothing
 *
 ********************************************************************/
 
void delay_loop(unsigned long count)
{
  do {} while (--count);
}

/********************************************************************
 * wait_for_tick
 * waits for clock tick
 *
 * Inputs:  
 * 
 * 
 * Returns: clock_t structure with current time
 *
 ********************************************************************/
//TODO: tick unit is probably not good enough (it can be different on variuos machines
//it should be replaced with other unit - 1s , 100ms...
clock_t wait_for_tick()
{
     clock_t last, current;

     last = clock();
     do {
          current = clock();
     } while (current == last);
     return current;
}

/********************************************************************
 * calibrate
 * calibrate() tries to figure out how many times to spin in the delay loop to delay
 * for one clock tick. So you'll have to scale that down using CLK_TCK
 * and the delay you actually want. 
 * 
 * Inputs:  
 * 
 * 
 * Returns: number of loop iteration factor
 *
 ********************************************************************/

long calibrate()
{
     clock_t current;
     unsigned long spins, adjust;

     spins = 1;
     do {
          current = wait_for_tick();
          spins += spins;
          delay_loop(spins);
     } while (current == clock());

     adjust = spins >> 1;
     spins -= adjust;
     do {
          current = wait_for_tick();
          delay_loop(spins);
          if (current == clock()) {
               spins += adjust;
               adjust >>= 1;
          } else {
               spins -= (adjust) - 1;
          }
     } while (adjust > 0);
     return spins;
}

/********************************************************************
 * nanodelay
 * delays given time in ns
 * 
 * Inputs:  ns - time to delay in ns
 * 
 * 
 * Returns: nothing
 *
 ********************************************************************/

void nanodelay(long ns)
{
     delay_loop((unsigned long) ((float) ns * spins_per_ns));
}

#endif
