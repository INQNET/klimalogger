/*  open3600 - dump3600.c
 *  
 *  Version 1.10
 *  
 *  Control WS3600 weather station
 *  
 *  Copyright 2003-2005, Kenneth Lavrsen, Grzegorz Wisniewski, Sander Eerkes
 *  This program is published under the GNU General Public license
 */

#include "rw3600.h"


/********************************************************************
 * print_usage prints a short user guide
 *
 * Input:   none
 * 
 * Output:  prints to stdout
 * 
 * Returns: exits program
 *
 ********************************************************************/
void print_usage(void)
{
	printf("\n");
	printf("dump3600 - Dump all data from WS-3600 to file.\n");
	printf("Data is stored with address in human readable format\n");
	printf("(C)2003 Kenneth Lavrsen.\n");
	printf("(C)2005 Grzegorz Wisniewski,Sander Eerkes. (Version alfa)\n");
	printf("This program is released under the GNU General Public License (GPL)\n\n");
	printf("Usage:\n");
	printf("dump3600 filename start_address end_address\n");
	printf("Addresses in hex, range 0-7FFF\n");
	exit(0);
}

 
/********** MAIN PROGRAM ************************************************
 *
 * This program reads from a WS3600 weather station at a given address
 * range and write the data in a text file in human readable format.
 *
 * Just run the program without parameters
 * for usage.
 *
 * It uses the config file for device name.
 * Config file locations - see open3600.conf
 *
 ***********************************************************************/
int main(int argc, char *argv[])
{
		WEATHERSTATION ws;
	FILE *fileptr;
	unsigned char data[32768];
	
  int i;
	int start_adr, end_adr;
	struct config_type config;

  // Get in-data and select mode.
  
 	// Get serial port from connfig file.
	// Note: There is no command line config file path feature!
	// history3600 will only search the default locations for the config file
	
	get_configuration(&config, "");

	if (argc!=4)
	{
		print_usage();
		exit(0);
	}

  // Setup serial port
	ws = open_weatherstation(config.serial_device_name);

	
	fileptr = fopen(argv[1], "w");
	if (fileptr == NULL)
	{
		printf("Cannot open file %s\n",argv[1]);
		exit(0);
	}
	
	start_adr = strtol(argv[2],NULL,16);
	end_adr = strtol(argv[3],NULL,16);
	
	if (start_adr < 0 || start_adr > 0x7FFF || end_adr < 0 ||
	                  end_adr > 0x7FFF || start_adr>=end_adr)
	{
		printf("Address range invalid\n");
		exit(0);
	}
	
		if (read_safe(ws, start_adr, end_adr-start_adr + 1, data, NULL) == -1) {
      printf("\nError reading data\n");
      close_weatherstation(ws);
		  fclose(fileptr);
		  exit(0);
    }

	// Write out the data
	for (i=0; i<=end_adr-start_adr;i++)
	{		
		printf("Address: %04X - Data: %02X\n",start_adr+i,data[i]);
		fprintf(fileptr,"Address: %04X - Data: %02X\n",start_adr+i,data[i]);
	}


	// Goodbye and Goodnight
	close_weatherstation(ws);

	return(0);
}
