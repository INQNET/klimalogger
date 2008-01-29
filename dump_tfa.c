/* vim:set expandtab! ts=4: */

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

	int start_adr, len;
	struct config_type config;
	int block_len = 1800;

	// Get serial port from config file.
	// Note: There is no command line config file path feature!
	// history3600 will only search the default locations for the config file

	get_configuration(&config, "");

	memset(data, 0xAA, 32768);

	fileptr = fopen("tfa.dump", "w");
	if (fileptr == NULL) {
		printf("Cannot open file %s\n","tfa.dump");
		abort();
	}

	// Setup serial port
	ws = open_weatherstation(config.serial_device_name);

	start_adr = 0x00;
	len = 0x7FFF; // 1802*3; //(0x7ef4+259) - start_adr;
	printf("Dumping %d bytes.\n", len);
	//if (read_safe(ws, start_adr, len, data, NULL) == -1) {
	//	printf("\nError reading data\n");
	//}

	while (start_adr < len) {
		int this_len = block_len;
		if (start_adr + block_len > len)
			this_len = len-start_adr;

		printf("   ... reading %d bytes beginning from %d\n", this_len, start_adr);

		write_data(ws, start_adr, 0, NULL);
		read_data(ws, this_len, data+start_adr);

		start_adr += block_len;
	}
	fwrite(data, len, 1, fileptr);

	close_weatherstation(ws);
	fclose(fileptr);
	return(0);
}

