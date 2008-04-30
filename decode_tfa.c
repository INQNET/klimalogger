/* vim:set expandtab! ts=4: */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "record.h"

int main(int argc, char *argv[]) {
	FILE *fileptr;
	unsigned char data[32768];
	int sensors;
	int block_size = 0;

	int i;
	int len;

	int data_offset = 0x64;

	char* filename;

	if (argc != 2) {
	  fprintf(stderr, "Usage: decode_tfa tfa.dump.filename\n");
	  exit(EXIT_FAILURE);
	}
	filename = argv[1];

	fileptr = fopen(filename, "r");
	if (fileptr == NULL) {
		fprintf(stderr, "Cannot open file %s\n", filename);
		return 1;
	}

	len = fread(data, 1, 32768, fileptr);

	sensors = data[0x0C];
	fprintf(stderr, "Found %d external sensors.\n", sensors);
	sensors++;
	fprintf(stderr, " ==== %d total sensors.\n", sensors);

	if (sensors != 6) {
		fprintf(stderr, "Sorry, I don't understand the data, if there are other than \n");
		fprintf(stderr, " 5 external sensors configured!\n");
		return 2;
	}

	if (sensors >= 4) block_size = 15;
	if (sensors == 6) block_size = 20;

	if (block_size == 0) {
		fprintf(stderr, "Blocksize = 0, probably did not recognize Sensor count.\n");
		return 2;
	}

	for (i=0; i<=(int)(len/block_size);i++) {

		Record r;
		unsigned char* ptr = data + (i*block_size) + data_offset;
		int rc;

		if (i >= 999 && ptr[0] == 0xff) {
			// end of ring buffer:
			// klimalogger only stores 999 entries, and bytes are marked with 0xff(?)
			break;
		}

		rc = record_parse(ptr, &r, sensors-1);
		if (rc == -1) {
			fprintf(stderr, "I: WRAPAROUND\n");
			continue;
		}

		printf("%04d %02d.%02d.20%02d %02d:%02d ",
			i, r.date_d, r.date_m, r.date_y, r.time_h, r.time_m);

		printTemp("in", r.t_in);
		printHumidity("in", r.h_in);
		printTemp("1", r.t_1);
		printHumidity("1", r.h_1);
		printTemp("2", r.t_2);
		printHumidity("2", r.h_2);
		printTemp("3", r.t_3);
		printHumidity("3", r.h_3);
		printTemp("4", r.t_4);
		printHumidity("4", r.h_4);
		printTemp("5", r.t_5);
		printHumidity("5", r.h_5);

//		printf(" Tin: %02.1f T1: %02.1f Fin: %d F1: %d T2: %02.1f F2: %d", t_in, t_1, h_in, h_1, t_2, h_2);
//		printf(" T3: %02.1f F3: %d ", t_3, h_3);
		printf("\n");
/*		printf("  %02x  %02x %02x  %02x %02x \n",
			ptr[10], ptr[11],
			ptr[12], ptr[13], ptr[14]
			); */
	}

	return(0);
}

