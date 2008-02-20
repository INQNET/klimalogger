/* vim:set expandtab! ts=4: */

typedef struct _Record {
	int date_d, date_m, date_y;
	int time_h, time_m;
	int h_in, h_1, h_2, h_3, h_4, h_5;
	float t_in, t_1, t_2, t_3, t_4, t_5;
} Record;

/* parse a record, pointed to by *data, into Record *r. Decode a maximum of
 * sensors sensors. */
extern int record_parse(const void* data, Record* r, int sensors);


/* helper functions for users */
extern void printTemp(const char* name, float value);
extern void printHumidity(const char* name, float value);

