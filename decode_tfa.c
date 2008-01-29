/* vim:set expandtab! ts=4: */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void printTemp(const char* name, float value) {
	if (value == 0xFF) return;
	printf("T%s: %02.1f ", name, value);
}
void printHumidity(const char* name, float value) {
	if (value == 0xFF) return;
	printf("H%s: %02.1f ", name, value);
}

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
	  printf("Usage: decode_tfa tfa.dump.filename\n");
	  exit(EXIT_FAILURE);
	}
	filename = argv[1];

	fileptr = fopen(filename, "r");
	if (fileptr == NULL) {
		printf("Cannot open file %s\n", filename);
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
		int date_d, date_m, date_y;
		int time_h, time_m;
		int h_in, h_1, h_2, h_3;
		float t_in, t_1, t_2, t_3;

		unsigned char* ptr = data + (i*block_size) + data_offset;

		if (i >= 999 && ptr[0] == 0xff) {
			// end of ring buffer:
			// klimalogger only stores 999 entries, and bytes are marked with 0xff(?)
			break;
		}

		if ((ptr[0] & 0xF0) >> 4 == 0xF) {
			printf(">>> Wraparound.\n");
			continue;
		}

		// hh:mm positions are reversed in eeprom
		time_m = (((ptr[0] & 0xF0) >> 4) * 10) + (ptr[0] & 0x0F);
		time_h = (((ptr[1] & 0xF0) >> 4) * 10) + (ptr[1] & 0x0F);

		date_d = (((ptr[2] & 0xF0) >> 4) * 10) + (ptr[2] & 0x0F);
		date_m = (((ptr[3] & 0xF0) >> 4) * 10) + (ptr[3] & 0x0F);
		date_y = (((ptr[4] & 0xF0) >> 4) * 10) + (ptr[4] & 0x0F);

		printf("%04d %02d.%02d.20%02d %02d:%02d ",
			i, date_d, date_m, date_y, time_h, time_m);

		t_in = (ptr[5] >> 4)*10 + (ptr[5]&0x0F) + ((ptr[6] & 0x0F))*100;
		t_in -= 300; t_in /= 10;
		if (ptr[5] == 0xaa) { t_in = 0xFF; }
		t_1 = (ptr[7] & 0x0F)*10 + (ptr[7] >> 4)*100 + (ptr[6] >> 4);
		t_1 -= 300; t_1 /= 10;
		if (ptr[7] == 0xaa) { t_1 = 0xFF; }
		h_in = (ptr[8] & 0x0F) + ((ptr[8] & 0xF0) >> 4) *10;
		if (ptr[8] == 0xaa) { h_in = 0xFF; }
		h_1 = (ptr[9] & 0x0F) + ((ptr[9] & 0xF0) >> 4) *10;
		if (ptr[9] == 0xaa) { h_1 = 0xFF; }

		t_2 = (ptr[10] & 0x0F) + ((ptr[10] >> 4) * 10) + (ptr[11] & 0x0F)*100;
		t_2 -= 300; t_2 /= 10;
		h_2 = (ptr[11] >> 4) + (ptr[12] & 0x0F)*10;
		if (ptr[11] == 0xaa) { t_2 = h_2 = 0xFF; }

		t_3 = (ptr[12] >> 4) + (ptr[13] >> 4)*100 + (ptr[13] & 0x0F)*10;
		t_3 -= 300; t_3 /= 10;
		if (ptr[13] == 0xaa) t_3 = 0xFF;
		h_3 = (ptr[14] & 0x0F) + ((ptr[14] & 0xF0) >> 4) *10;
		if (ptr[13] == 0xaa) h_3 = 0xFF;

		printTemp("in", t_in);
		printHumidity("in", h_in);
		printTemp("1", t_1);
		printHumidity("1", h_1);
		printTemp("2", t_2);
		printHumidity("2", h_2);
		printTemp("3", t_3);
		printHumidity("3", h_3);

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

