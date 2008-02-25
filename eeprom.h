 
#ifndef _INCLUDE_RW3600_H_
#define _INCLUDE_RW3600_H_ 

#include "linux3600.h"

#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAXRETRIES          20


/* Generic functions */

int eeprom_read(WEATHERSTATION ws, unsigned char *buf, size_t count);
int eeprom_seek(WEATHERSTATION ws, off_t pos);



WEATHERSTATION open_weatherstation(char *device);

void close_weatherstation(WEATHERSTATION ws);

int read_data(WEATHERSTATION ws, int number,
			  unsigned char *readdata);
			  
int write_data(WEATHERSTATION ws, int address, int number,
			   unsigned char *writedata);


void read_next_byte_seq(WEATHERSTATION ws);
void read_last_byte_seq(WEATHERSTATION ws);

int read_bit(WEATHERSTATION ws);
void write_bit(WEATHERSTATION ws,int bit);
int read_byte(WEATHERSTATION ws);
int write_byte(WEATHERSTATION ws,int byte);
void print_log(int log_level, char* str);

void sleep_short(int milliseconds);
void set_DTR(WEATHERSTATION ws, int val);
void set_RTS(WEATHERSTATION ws, int val);
int get_DSR(WEATHERSTATION ws);
int get_CTS(WEATHERSTATION ws);
long calibrate();
void nanodelay();
#endif /* _INCLUDE_RW3600_H_ */

