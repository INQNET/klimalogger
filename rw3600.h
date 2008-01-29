/* open3600 - rw3600.h
 * Include file for the open2300 read and write functions
 * including the data conversion functions
 * version 0.05
 */
 
#ifndef _INCLUDE_RW3600_H_
#define _INCLUDE_RW3600_H_ 

#ifdef WIN32
#include "win3600.h"
#endif

#ifndef WIN32
#include "linux3600.h"
#endif

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
#define MAXWINDRETRIES      20
#define WRITENIB            0x42
#define SETBIT              0x12
#define UNSETBIT            0x32
#define WRITEACK            0x10
#define SETACK              0x04
#define UNSETACK            0x0C
#define RESET_MIN           0x01
#define RESET_MAX           0x02

#define HISTORY_BUFFER_ADR  0x1A0
#define HISTORY_REC_NO  1796

/* Generic functions */

void read_error_exit(void);

void write_error_exit(void);

void print_usage(void);

WEATHERSTATION open_weatherstation(char *device);

void close_weatherstation(WEATHERSTATION ws);

int initialize(WEATHERSTATION ws2300);

void reset_06(WEATHERSTATION ws2300);

int read_data(WEATHERSTATION ws, int number,
			  unsigned char *readdata);
			  
int write_data(WEATHERSTATION ws, int address, int number,
			   unsigned char *writedata);

int read_safe(WEATHERSTATION ws2300, int address, int number,
			  unsigned char *readdata, unsigned char *commanddata);
			  
int write_safe(WEATHERSTATION ws2300, int address, int number,
			   unsigned char encode_constant, unsigned char *writedata,
			   unsigned char *commanddata);

void read_next_byte_seq(WEATHERSTATION ws);
void read_last_byte_seq(WEATHERSTATION ws);

int read_bit(WEATHERSTATION ws);
void write_bit(WEATHERSTATION ws,int bit);
int read_byte(WEATHERSTATION ws);
int write_byte(WEATHERSTATION ws,int byte);
void print_log(int log_level, char* str);

/* Platform dependent functions */
int read_device(WEATHERSTATION serdevice, unsigned char *buffer, int size);
int write_device(WEATHERSTATION serdevice, unsigned char *buffer, int size);
//void sleep_very_short(int n);
void sleep_short(int milliseconds);
void sleep_long(int seconds);
int http_request_url(char *urlline);
void set_DTR(WEATHERSTATION ws, int val);
void set_RTS(WEATHERSTATION ws, int val);
int get_DSR(WEATHERSTATION ws);
int get_CTS(WEATHERSTATION ws);
long calibrate();
void nanodelay();
#endif /* _INCLUDE_RW3600_H_ */

