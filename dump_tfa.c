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
	WEATHERSTATION ws;
	FILE *fileptr;
	unsigned char data[32768];
	unsigned char sensors;

	int i;
	int start_adr, len;
	struct config_type config;

	// Get serial port from connfig file.
	// Note: There is no command line config file path feature!
	// history3600 will only search the default locations for the config file

	get_configuration(&config, "");


	fileptr = fopen("tfa.dump", "w");
	if (fileptr == NULL) {
		printf("Cannot open file %s\n","tfa.dump");
		abort();
	}

	// Setup serial port
	ws = open_weatherstation(config.serial_device_name);

	if (read_safe(ws, 0x0c, 1, &sensors, NULL) == -1) {
		printf("error while communicating with tfastation\n");
		abort();
	}
	printf("Sensors: %d\n", sensors);

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
	// Goodbye and Goodnight
	close_weatherstation(ws);
	return(0);
}

