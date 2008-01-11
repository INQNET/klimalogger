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

#ifndef WIN32
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
	struct termios adtio;
	unsigned char buffer[BUFFER_SIZE];
	long i;
	char str[100];
  print_log(1,"open_weatherstation");
  
  //calibrate nanodelay function
  spins_per_ns = (float) calibrate() * (float) CLOCKS_PER_SEC * 1.0e-9f;
  sprintf(str,"spins_per_ns=%.2f",spins_per_ns);
  print_log(2,str);
  sprintf(str,"CLOCKS_PER_SEC=%.2f",((float) CLOCKS_PER_SEC));
  print_log(2,str);
  
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
  
  write_device(ws, buffer, 448);
  
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
  } else
  {
    print_log(2,"Connection timeout 2");
    printf ("Connection timeout\n");
    close_weatherstation(ws);
    exit(0);
  }
  write_device(ws, buffer, 448);
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
 * read_device in the Linux version is identical
 * to the standard Linux read() 
 *
 * Inputs:  serdevice - opened file handle
 *          buffer - pointer to the buffer to read into (unsigned char)
 *          size - number of bytes to read
 *
 * Output:  *buffer - modified on success (pointer to unsigned char)
 * 
 * Returns: number of bytes read
 *
 ********************************************************************/
int read_device(WEATHERSTATION ws, unsigned char *buffer, int size)
{
	int ret;

	for (;;) {
		ret = read(ws, buffer, size);
		if (ret == 0 && errno == EINTR)
			continue;
		return ret;
	}
}

/********************************************************************
 * write_device in the Linux version is identical
 * to the standard Linux write()
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
	int ret = write(serdevice, buffer, size);
	return ret;
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

/********************************************************************
 * sleep_long - Linux version
 * 
 * Inputs: Time in seconds (integer)
 *
 * Returns: nothing
 *
 ********************************************************************/
void sleep_long(int seconds)
{
	sleep(seconds);
}

/********************************************************************
 * http_request_url - Linux version
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
	int sockfd;
	struct hostent *hostinfo;
	struct sockaddr_in urladdress;
	char buffer[1024];
	int bytes_read;
	
	if ( (hostinfo = gethostbyname(WEATHER_UNDERGROUND_BASEURL)) == NULL )
	{
		perror("Host not known by DNS server or DNS server not working");
		return(-1);
	}
	
	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
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
	close(sockfd);
	
	return(0);
}

/********************************************************************
 * citizen_weather_send - Linux version
 * 
 * Inputs: config structure (pointer to) - containing CW ID
 *         datastring (pointer to) - containing all the data
 *
 * Returns: 0 on success and -1 if fail.
 *
 * Action: Send data to Citizen Weather
 *
 ********************************************************************/
int citizen_weather_send(struct config_type *config, char *aprsline)
{
	int sockfd = -1; // just to eliminate a warning we'll set this
	int bytes_read;
	struct hostent *hostinfo;
	struct sockaddr_in urladdress;
	char buffer[1024];          //Enough to hold a response
	int hostnum;
	
	// Connect to server and send the record
	// loop trying all of the defined servers
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
			
		if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		{
			sprintf(buffer,"Cannot open socket on %s ", config->aprs_host[hostnum].name);
			perror(buffer);
			continue;
		}
	
		memset(&urladdress, 0, sizeof(urladdress)); // clear the structure
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
	
	if ( (recv(sockfd, buffer, sizeof(buffer), 0) > 0) && (DEBUG != 0) )                 // read login prompt
	{
		printf("%s", buffer);	// display prompt - if debug
	}

	// The login/header line
	sprintf(buffer,"user %s pass -1 vers open2300\n",
	        config->citizen_weather_id);
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
 * for one clock tick. So you'll have to scale that down using CLOCKS_PER_SEC
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
