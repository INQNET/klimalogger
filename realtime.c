/* vim:set expandtab! ts=4: */

#include "eeprom.h"
#include <time.h>
#include <unistd.h>
#include "record.h"

#define BUFSIZE 32768

void print_usage() {
	fprintf(stderr, "Usage: realtime /dev/ttyS0\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	WEATHERSTATION ws;
	unsigned char data[BUFSIZE];
	char* serial_device;

	int sensors;
	int block_size = 0;
	int data_offset = 0x64;
	int start_adr;
	int record_interval;
	int skip, skip_offset;
	int sequential = 0;
	Record r;
	Record rnext;

	time_t t;
	struct tm *tm;

	memset(data, 0xAA, BUFSIZE);

	if (argc != 2) {
		fprintf(stderr, "E: no serial device specified.\n");
		print_usage();
	}

	serial_device = argv[1];
	
	// need root for (timing) portio
	if (geteuid() != 0) {
		fprintf(stderr, "E: this program needs root privileges to do direct port I/O.\n");
		exit(EXIT_FAILURE);
	}

	t = time(NULL);
	tm = localtime(&t);
	if (tm == NULL) {
		perror("localtime");
		exit(EXIT_FAILURE);
	}


	// Setup serial port
	ws = open_weatherstation(serial_device);

	// read config
	nanodelay();
	eeprom_seek(ws, 0);
	eeprom_read(ws, data, data_offset);
	
	// config check
	sensors = data[0x0C];
	sensors++;
	if (sensors == 4 || sensors == 5) block_size = 15;
	if (sensors == 6) block_size = 20;
	printf("Using sensors=%d\n", sensors);
	if (block_size == 0) {
		fprintf(stderr,"E: dont understand this format, found sensors=%d\n", sensors);
		exit(EXIT_FAILURE);
	}

	printf("Scanning for two connected records\n");
	start_adr = data_offset;
	while(start_adr < (data_offset + block_size*50)) {
		int read_len = (2*block_size);
		eeprom_seek(ws, start_adr);
		eeprom_read(ws, data + start_adr, read_len);
		record_parse(data+data_offset, &r, sensors);
		record_parse(data+data_offset+block_size, &rnext, sensors);
		record_interval = rnext.time_m - r.time_m;
		if (record_interval > 0 && record_interval < 30) {
			break;
		}
		record_interval = -1;
		start_adr += read_len;
	}
	if (record_interval == -1) {
		fprintf(stderr,"E: can't discover recording interval\n");
		exit(EXIT_FAILURE);
	}

	printf("%02d.%02d.20%02d %02d:%02d \n", r.date_d, r.date_m, r.date_y, r.time_h, r.time_m);
	printf("%02d.%02d.20%02d %02d:%02d \n", rnext.date_d, rnext.date_m, rnext.date_y, rnext.time_h, rnext.time_m);

	printf("Recording interval: %d\n", record_interval);

	skip = 0;

	if (tm->tm_year-(2000+r.date_y) > 0) {
		sequential = 1;
	}

	if (tm->tm_hour-r.time_h > 1) {
		skip += (tm->tm_hour-1-r.time_h)*60/record_interval;
		printf("hours off: %d\n", (tm->tm_hour-r.time_h));
		skip += (tm->tm_min-r.time_m)/record_interval;
	}
	if (tm->tm_hour-r.time_h == 0 && tm->tm_min-r.time_m > (2*record_interval)) {
		printf("only correcting minutes\n");
		skip += (tm->tm_min-r.time_m)/record_interval;
	}
	skip -= 5;
	printf("skip=%d, seq=%d\n", skip, sequential);
	if (skip > 1100) {
		sequential = 1;
	}

	if (sequential == 1) {
		printf("Falling back to sequential read.\n");
		printf("not implemented ;)\n");
	} else {
		printf("Jumping to record %d\n", skip);

		// now read from guessed record position
		skip_offset = data_offset + skip*block_size;
		eeprom_seek(ws, skip_offset);
		eeprom_read(ws, data+skip_offset, block_size*2);

		record_parse(data+skip_offset, &r, sensors);

		printf("%02d.%02d.20%02d %02d:%02d ",
			r.date_d, r.date_m, r.date_y, r.time_h, r.time_m);
		printf("\n");
	}

	close_weatherstation(ws);
	return(0);
}

