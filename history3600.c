/*  open3600 - history3600.c
 *  
 *  Version 0.01
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
	printf("history3600 - Dump all history data from WS-3600 to file.\n");
	printf("(C)2003 Kenneth Lavrsen, Grzegorz Wisniewski, Sander Eerkes.\n");
	printf("This program is released under the GNU General Public License (GPL)\n\n");
	printf("Usage:\n");
	printf("history3600 filename start_record end_record\n");
	printf("Record number in dec, range 0 - 1796\n");
	exit(0);
}

 
/********** MAIN PROGRAM ************************************************
 *
 * This program reads the history records from a WS3600
 * weather station at a given record range
 * and prints the data to stdout and to a file.
 * Just run the program without parameters for usage.
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
	
  int i, j, rec_count;
	int start_rec, end_rec, start_adr, end_adr;
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
	
	start_rec = strtol(argv[2],NULL,10);
	end_rec = strtol(argv[3],NULL,10);
	
	if (start_rec < 0 || start_rec > 1796 || end_rec < 0 ||
	                  end_rec > 1796 || start_rec>=end_rec)
	{
		printf("Record range invalid\n");
		exit(0);
	}
	
	start_adr = start_rec * 18 + 0x1a0;
	end_adr = end_rec * 18 + 0x1a0 + 17;
		
	
	if (read_safe(ws, start_adr, end_adr-start_adr + 1, data, NULL) == -1) {
    printf("\nError reading data\n");
    close_weatherstation(ws);
	  fclose(fileptr);
	  exit(0);
  }

	// Write out the data
	rec_count = start_rec;
	for (i=0; i<=end_adr-start_adr; i+=18)
	{
	  printf("Record %04i ",rec_count);
	  fprintf(fileptr,"Record %04i ",rec_count);
	  for (j = 0; j < 18; j++)
	  {
		  printf("%02X ",data[i + j]);
		  fprintf(fileptr,"%02X ",data[i + j]);
		}
		rec_count++;
		printf("\n");
		fprintf(fileptr,"\n");
	}

 
	// Goodbye and Goodnight
	close_weatherstation(ws);
	fclose(fileptr);

	return(0);
}

