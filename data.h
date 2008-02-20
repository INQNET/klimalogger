/* vim:set expandtab! ts=4: */

typedef struct _Record {
	int date_d, date_m, date_y;
	int time_h, time_m;
	int h_in, h_1, h_2, h_3;
	float t_in, t_1, t_2, t_3;
} Record;

extern void printTemp(const char* name, float value);
extern void printHumidity(const char* name, float value);
extern int parseRecord(const void* data, Record* r, int sensors);

