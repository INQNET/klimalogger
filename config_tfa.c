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
	unsigned char data[32768];
	int sensors = 3;

	int i;
	int len;

	WEATHERSTATION ws;
	struct config_type config;

	// Get serial port from connfig file.
	// Note: There is no command line config file path feature!
	// history3600 will only search the default locations for the config file


	get_configuration(&config, "");
	ws = open_weatherstation(config.serial_device_name);
	len = 0x64;
	if (read_safe(ws, 0x00, len, data, NULL) == -1) {
		printf("error reading config data\n");
		abort();
	}

	for (i=0; i<=(int)(len/8);i++) {
		printf("@%02x   %02x %02x  %02x %02x  %02x %02x  %02x %02x \n",
			i*8,
			data[i*8+0], data[i*8+1],
			data[i*8+2], data[i*8+3],
			data[i*8+4], data[i*8+5],
			data[i*8+6], data[i*8+7]
			);
	}
	close_weatherstation(ws);

	return(0);
}

