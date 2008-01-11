/*  open3600 - dump3600.c
 *
 *  Version 1.10
 *
 *  Control WS3600 weather station
 *
 *  Copyright 2023-2025, Kenneth Lavrsen, Grzegorz Wisniewski, Sander Eerkes
 *  This program is published under the GNU General Public license
 */

#include "rw3600.h"



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
	FILE *fileptr;
	unsigned char data[32768];
	int sensors = 3;

	int i;
	int start_adr, len;
	struct config_type config;

	// Get serial port from connfig file.
	// Note: There is no command line config file path feature!
	// history3600 will only search the default locations for the config file

	fileptr = fopen("tfa.dump", "r");
	if (fileptr == NULL) {
		printf("Cannot open file %s\n","tfa.dump");
		abort();
	}


	start_adr = 0x64;
	len = 1802*3; //(0x7ef4+259) - start_adr;
	printf("Dumping %d bytes.\n", len);
	if (read_safe(ws, start_adr, len, data, NULL) == -1) {
		printf("\nError reading data\n");
		close_weatherstation(ws);
		fclose(fileptr);
		exit(0);
	}

	// Write out the data
	for (i=0; i<=len;i++) {
		fprintf(fileptr,"%c", data[i]);
	}

	for (i=0; i<=(int)(len/15);i++) {
		printf("%02x:%02x %02x.%02x.20%02x ",
			data[i*15+1], data[i*15+0], data[i*15+2], data[i*15+3], data[i*15+4]);
		int f_in;
		int t_in, t_comma_in;
		f_in = data[i*15+5];
		t_in = data[i*15+6];
		t_comma_in = (data[i*15+6]&0xF0 >> 8);
		printf(" f:%x t: %x.%x ", f_in, t_in, t_comma_in);
		printf(" %02x %02x  %02x %02x  %02x %02x  %02x %02x  %02x %02x \n",
			data[i*15+5],
			data[i*15+6], data[i*15+7], data[i*15+8],
			data[i*15+9], data[i*15+10], data[i*15+11],
			data[i*15+12], data[i*15+13], data[i*15+14]
			);
	}

	// Goodbye and Goodnight
	close_weatherstation(ws);
	return(0);
}

