/* vim:set expandtab! ts=4: */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "record.h"

void printTemp(const char* name, float value) {
	if (value == 0xFF) return;
	printf("T%s: %02.1f ", name, value);
}
void printHumidity(const char* name, float value) {
	if (value == 0xFF) return;
	printf("H%s: %02.1f ", name, value);
}

int record_parse(const void* data, Record* r, int sensors) {
	
	const unsigned char* ptr = data;

	if ((ptr[0] & 0xF0) >> 4 == 0xF) {
		return -1;
	}

	// hh:mm positions are reversed in eeprom
	r->time_m = (((ptr[0] & 0xF0) >> 4) * 10) + (ptr[0] & 0x0F);
	r->time_h = (((ptr[1] & 0xF0) >> 4) * 10) + (ptr[1] & 0x0F);

	r->date_d = (((ptr[2] & 0xF0) >> 4) * 10) + (ptr[2] & 0x0F);
	r->date_m = (((ptr[3] & 0xF0) >> 4) * 10) + (ptr[3] & 0x0F);
	r->date_y = (((ptr[4] & 0xF0) >> 4) * 10) + (ptr[4] & 0x0F);

	// begin decoding sensors
	r->t_in = (ptr[5] >> 4)*10 + (ptr[5]&0x0F) + ((ptr[6] & 0x0F))*100;
	r->t_in -= 300; r->t_in /= 10;
	if (ptr[5] == 0xaa) { r->t_in = 0xFF; }
	r->t_1 = (ptr[7] & 0x0F)*10 + (ptr[7] >> 4)*100 + (ptr[6] >> 4);
	r->t_1 -= 300; r->t_1 /= 10;
	if (ptr[7] == 0xaa) { r->t_1 = 0xFF; }
	r->h_in = (ptr[8] & 0x0F) + ((ptr[8] & 0xF0) >> 4) *10;
	if (ptr[8] == 0xaa) { r->h_in = 0xFF; }
	r->h_1 = (ptr[9] & 0x0F) + ((ptr[9] & 0xF0) >> 4) *10;
	if (ptr[9] == 0xaa) { r->h_1 = 0xFF; }

	if (sensors < 2) return 0;

	r->t_2 = (ptr[10] & 0x0F) + ((ptr[10] >> 4) * 10) + (ptr[11] & 0x0F)*100;
	r->t_2 -= 300; r->t_2 /= 10;
	r->h_2 = (ptr[11] >> 4) + (ptr[12] & 0x0F)*10;
	if (ptr[11] == 0xaa || ptr[10] == 0xaa || (ptr[12] & 0x0F) == 0x0a) { r->t_2 = r->h_2 = 0xFF; }

	r->t_3 = (ptr[12] >> 4) + (ptr[13] >> 4)*100 + (ptr[13] & 0x0F)*10;
	r->t_3 -= 300; r->t_3 /= 10;
	if (ptr[13] == 0xaa) r->t_3 = 0xFF;
	r->h_3 = (ptr[14] & 0x0F) + ((ptr[14] & 0xF0) >> 4) *10;
	if (ptr[14] == 0xaa) r->h_3 = 0xFF;

	if (sensors < 4) return 0;

	return 0;
}

