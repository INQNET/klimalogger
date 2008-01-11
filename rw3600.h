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

#define METERS_PER_SECOND   1.0
#define KILOMETERS_PER_HOUR 3.6
#define MILES_PER_HOUR      2.23693629
#define CELCIUS             0
#define FAHRENHEIT          1
#define MILLIMETERS         1
#define INCHES              25.4
#define HECTOPASCAL         1.0
#define MILLIBARS           1.0
#define INCHES_HG           33.8638864

#define HISTORY_BUFFER_ADR  0x1A0
#define HISTORY_REC_NO  1796

/* ONLY EDIT THESE IF WEATHER UNDERGROUND CHANGES URL */
#define WEATHER_UNDERGROUND_BASEURL "weatherstation.wunderground.com"
#define WEATHER_UNDERGROUND_PATH "/weatherstation/updateweatherstation.php"

#define WEATHER_UNDERGROUND_SOFTWARETYPE   "open2300%20v"

#define MAX_APRS_HOSTS	6

typedef struct {
	char name[50];
	int port;
} hostdata;

struct config_type
{
	char   serial_device_name[50];
	char   citizen_weather_id[30];
	char   citizen_weather_latitude[20];
	char   citizen_weather_longitude[20];
	hostdata aprs_host[MAX_APRS_HOSTS]; // max 6 possible aprs hosts 1 primary and 5 alternate
	int    num_hosts;					// total defined hosts
	char   weather_underground_id[30];
	char   weather_underground_password[50];
	char   timezone[6];                //not integer because of half hour time zones
	double wind_speed_conv_factor;     //from m/s to km/h or miles/hour
	int    temperature_conv;           //0=Celcius, 1=Fahrenheit
	double rain_conv_factor;           //from mm to inch
	double pressure_conv_factor;       //from hPa (=millibar) to mmHg
	char   mysql_host[50];             //Either localhost, IP address or hostname
	char   mysql_user[25];
	char   mysql_passwd[25];
	char   mysql_database[30];
	int    mysql_port;                 //0 works for local connection
	char   pgsql_connect[128];
	char   pgsql_table[25];
	char   pgsql_station[25];
	int    log_level;
};

struct timestamp
{
	int minute;
	int hour;
	int day;
	int month;
	int year;
};
//calibration value for nanodelay function
extern float spins_per_ns;

//configuration data, global variable for easy availability in many function
extern struct config_type config;

/* Weather data functions */

double temperature_conv(double temp_celcius);
double pressure_conv(double pressure_hpa);
double windspeed_conv(double windspeed_ms);
double rain_conv(double rain_mm);

double temperature_indoor(unsigned char *data);

void temperature_indoor_minmax(unsigned char *data,
                               double *temp_min,
                               double *temp_max,
                               struct timestamp *time_min,
                               struct timestamp *time_max);

int temperature_indoor_reset(WEATHERSTATION ws2300, char minmax);

double temperature_outdoor(unsigned char *data);

void temperature_outdoor_minmax(unsigned char *data,
                                double *temp_min,
                                double *temp_max,
                                struct timestamp *time_min,
                                struct timestamp *time_max);

int temperature_outdoor_reset(WEATHERSTATION ws2300, char minmax);

double dewpoint(unsigned char *data);

void dewpoint_minmax(unsigned char *data,
                     double *dp_min,
                     double *dp_max,
                     struct timestamp *time_min,
                     struct timestamp *time_max);
					
int dewpoint_reset(WEATHERSTATION ws2300, char minmax);

int humidity_indoor(unsigned char *data);

void humidity_indoor_minmax(unsigned char *data,
                        int *hum_min,
                        int *hum_max,
                        struct timestamp *time_min,
                        struct timestamp *time_max);

int humidity_indoor_reset(WEATHERSTATION ws2300, char minmax);

int humidity_outdoor(unsigned char *data);

void humidity_outdoor_minmax(unsigned char *data,
                        int *hum_min,
                        int *hum_max,
                        struct timestamp *time_min,
                        struct timestamp *time_max);

int humidity_outdoor_reset(WEATHERSTATION ws2300, char minmax);

double wind_current(unsigned char *data,
                    double *winddir);

double wind_minmax(unsigned char *data,
                   double *wind_min,
                   double *wind_max,
                   struct timestamp *time_min,
                   struct timestamp *time_max);

int wind_reset(WEATHERSTATION ws2300, char minmax);

double windchill(unsigned char *data);

void windchill_minmax(unsigned char *data,
                      double *wc_min,
                      double *wc_max,
                      struct timestamp *time_min,
                      struct timestamp *time_max);
                      
int windchill_reset(WEATHERSTATION ws2300, char minmax);

double rain_1h(unsigned char *data);

void rain_1h_max(unsigned char *data,
                   double *rain_max,
                   struct timestamp *time_max);

double rain_24h(unsigned char *date);

int rain_1h_max_reset(WEATHERSTATION ws2300);

int rain_1h_reset(WEATHERSTATION ws2300);

void rain_24h_max(unsigned char *data,
                   double *rain_max,
                   struct timestamp *time_max);

int rain_24h_max_reset(WEATHERSTATION ws2300);

int rain_24h_reset(WEATHERSTATION ws2300);

double rain_1w(unsigned char *data);

void rain_1w_max(unsigned char *data,
                   double *rain_max,
                   struct timestamp *time_max);
                   
double rain_1m(unsigned char *data);

void rain_1m_max(unsigned char *data,
                   double *rain_max,
                   struct timestamp *time_max);

double rain_total(unsigned char *data);

void rain_total_time(unsigned char *data,
                   struct timestamp *time_since);
                   
int rain_total_reset(WEATHERSTATION ws2300);

double rel_pressure(unsigned char *data);

void rel_pressure_minmax(unsigned char *data,
                         double *pres_min,
                         double *pres_max,
                         struct timestamp *time_min,
                         struct timestamp *time_max);
                         
double abs_pressure(unsigned char *data);

void abs_pressure_minmax(WEATHERSTATION ws2300,
                         double pressure_conv_factor,
                         double *pres_min,
                         double *pres_max,
                         struct timestamp *time_min,
                         struct timestamp *time_max);

int pressure_reset(WEATHERSTATION ws2300, char minmax);

double pressure_correction(WEATHERSTATION ws2300, double pressure_conv_factor);

double calculate_dewpoint(double temperature, double humidity);
double calculate_windchill(double temperature, double windspeed);

void tendency_forecast(unsigned char *data, char *tendency, char *forecast);

int read_history_info(WEATHERSTATION ws2300, int *interval, int *countdown,
                      struct timestamp *time_last, int *no_records);

int read_history_record(unsigned char *data,
                        int record,
                        struct config_type *config,
                        double *temperature_indoor,
                        double *temperature_outdoor,
                        double *pressure,
                        int *humidity_indoor,
                        int *humidity_outdoor,
                        double *raincount,
                        double *windspeed,
                        double *windgust,
                        double *winddir_degrees,
                        double *dewpoint,
                        double *windchill,
                        struct tm *time);
                        
void light(WEATHERSTATION ws2300, int control);


/* Generic functions */

void read_error_exit(void);

void write_error_exit(void);

void print_usage(void);

int get_configuration(struct config_type *, char *path);

WEATHERSTATION open_weatherstation(char *device);

void close_weatherstation(WEATHERSTATION ws);

void address_encoder(int address_in, unsigned char *address_out);

void data_encoder(int number, unsigned char encode_constant,
				  unsigned char *data_in, unsigned char *data_out);

unsigned char numberof_encoder(int number);

unsigned char command_check0123(unsigned char *command, int sequence);

unsigned char command_check4(int number);

unsigned char data_checksum(unsigned char *data, int number);

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
int citizen_weather_send(struct config_type *config, char *datastring);
void set_DTR(WEATHERSTATION ws, int val);
void set_RTS(WEATHERSTATION ws, int val);
int get_DSR(WEATHERSTATION ws);
int get_CTS(WEATHERSTATION ws);
long calibrate();
void nanodelay(long ns);
#endif /* _INCLUDE_RW3600_H_ */ 
