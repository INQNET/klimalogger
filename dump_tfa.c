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
#include <time.h>
#include <unistd.h>
#define MAX_RETRIES 10

int main(int argc, char *argv[]) {
	WEATHERSTATION ws;
	FILE *fileptr;
	unsigned char data[32768];

	int start_adr, len;
	int block_len = 1800;
	int retries = 0;
	char filename[50];

	if (geteuid() != 0) {
		fprintf(stderr, "E: this program needs root privileges to do direct port I/O.\n");
		exit(EXIT_FAILURE);
	}

	// Get serial port from config file.
	// Note: There is no command line config file path feature!
	// history3600 will only search the default locations for the config file

	memset(data, 0xAA, 32768);

	// generate filename based on current time
	{
		time_t t;
		struct tm *tm;
		t = time(NULL);
		tm = localtime(&t);
		if (tm == NULL) {
			perror("localtime");
			exit(EXIT_FAILURE);
		}
		sprintf(filename, "tfa.dump.");
		strftime(filename+strlen(filename), sizeof(filename)-strlen(filename), "%Y%m%d.%H%M", tm);
	}


	fileptr = fopen(filename, "w");
	if (fileptr == NULL) {
		printf("Cannot open file %s\n", filename);
		abort();
	}

	// Setup serial port
	ws = open_weatherstation("/dev/ttyS0");

	start_adr = 0x00;
	len = 0x7FFF; // 1802*3; //(0x7ef4+259) - start_adr;
	printf("Dumping %d bytes.\n", len);

	while (start_adr < len) {
		int got_len;
		int this_len = block_len;
		if (start_adr + block_len > len)
			this_len = len-start_adr;

		printf("   ... reading %d bytes beginning from %d\n", this_len, start_adr);

		nanodelay();
		write_data(ws, start_adr, 0, NULL);
		got_len = read_data(ws, this_len, data+start_adr);
		printf("   >>> got     %d bytes\n", got_len);
		if (got_len == -1 && retries < MAX_RETRIES) {
			retries++;
			fprintf(stderr, "W: eeprom ack failed, retrying read (retries left: %d).\n", MAX_RETRIES-retries);
			continue;
		}
		if (got_len != this_len) {
			fprintf(stderr, "E: got less than requested bytes, dump is probably unusable.\n");
			break;
		}

		start_adr += block_len;
		retries = 0;
	}
	fwrite(data, len, 1, fileptr);

	close_weatherstation(ws);
	fclose(fileptr);
	return(0);
}

