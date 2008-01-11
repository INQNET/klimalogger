/*  open3600  - rw3600.c library functions
 *  This is a library of functions common to Linux and Windows
 *  
 *  Version 0.10
 *  
 *  Control WS3600 weather station
 *  
 *  Copyright 2003-2005, Kenneth Lavrsen, Grzegorz Wisniewski, Sander Eerkes
 *  This program is published under the GNU General Public license
 */

#include "rw3600.h"

//calibration value for nanodelay function
float spins_per_ns;

//configuration data, global variable for easy availability in many function
struct config_type config;

/********************************************************************/
/* temperature_conv
 * Converts temperature according to configuration settings
 * 
 * Input: temp_celcius - temperature in Celcius
 *
 * Returns: Temperature (deg C or F)
 *
 ********************************************************************/
double temperature_conv(double temp_celcius)
{
  if (config.temperature_conv)
		return temp_celcius * 9 / 5 + 32;
	else
		return temp_celcius;
}

/********************************************************************/
/* pressure_conv
 * Converts temperature according to configuration settings
 * 
 * Input: pressure_hpa - pressure in hPa
 *
 * Returns: pressure (hPa or inHg)
 *
 ********************************************************************/
double pressure_conv(double pressure_hpa)
{
  return pressure_hpa / config.pressure_conv_factor;
}

/********************************************************************/
/* windspeed_conv
 * Converts wind speed according to configuration settings
 * 
 * Input: windspeed_ms - wind speed in m/s
 *
 * Returns: wind speed (m/s, km/h or b)
 *
 ********************************************************************/
double windspeed_conv(double windspeed_ms)
{
  return windspeed_ms * config.wind_speed_conv_factor;
}

/********************************************************************/
/* rain_conv
 * Converts rain according to configuration settings
 * 
 * Input: rain_mm - rain in mm
 *
 * Returns: rain in appropriate units
 *
 ********************************************************************/
double rain_conv(double rain_mm)
{
  return rain_mm / config.rain_conv_factor;
}


/********************************************************************/
/* temperature_indoor
 * Read indoor temperature, current temperature only
 * 
 * Input: data - pointer to data buffer
 *
 * Returns: Temperature (deg C if temperature_conv is 0)
 *                      (deg F if temperature_conv is 1)
 *
 ********************************************************************/
double temperature_indoor(unsigned char *data)
{
	int address=0x26;
  unsigned char *tempdata = data + address;

  return temperature_conv(((tempdata[1] & 0xF) * 10 +
		          (tempdata[0] >> 4) + (tempdata[0] & 0xF) / 10.0) - 40.0);
}


/********************************************************************/
/* temperature_indoor_minmax
 * Read indoor min/max temperatures with timestamps
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Output: Temperatures temp_min and temp_max
 *                (deg C if temperature_conv is 0)
 *                (deg F if temperature_conv is 1)
 *         Timestamps for temp_min and temp_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 ********************************************************************/
void temperature_indoor_minmax(unsigned char *data,
                               double *temp_min,
                               double *temp_max,
                               struct timestamp *time_min,
                               struct timestamp *time_max)
{
	int address_min=0x28;
	int address_max=0x2B;
	int address_mintime=0x2C;
	int address_maxtime=0x31;
	
  unsigned char *tempdata = data + address_min;
	
	*temp_min = temperature_conv(((tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) +
		          (tempdata[0] >> 4) / 10.0 ) - 40.0);

  tempdata = data + address_max;
	*temp_max = temperature_conv(((tempdata[1] & 0xF) * 10 +
		          (tempdata[0] >> 4) + (tempdata[0] & 0xF) / 10.0) - 40.0);

	tempdata = data + address_mintime;
	if (time_min != NULL)
	{	
		time_min->minute = (tempdata[0] >> 4) + (tempdata[1] & 0xF) * 10;
		time_min->hour = (tempdata[1] >> 4) + (tempdata[2] & 0xF) * 10;
		time_min->day = (tempdata[2] >> 4) + (tempdata[3] & 0xF) * 10;
		time_min->month = (tempdata[3] >> 4) + (tempdata[4] & 0xF) * 10;
    time_min->year = 2000 + (tempdata[4] >> 4) + (tempdata[5] & 0xF) * 10;
	}
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = (tempdata[0] >> 4) + (tempdata[1] & 0xF) * 10;
		time_max->hour = (tempdata[1] >> 4) + (tempdata[2] & 0xF) * 10;
		time_max->day = (tempdata[2] >> 4) + (tempdata[3] & 0xF) * 10;
		time_max->month = (tempdata[3] >> 4) + (tempdata[4] & 0xF) * 10;
    time_max->year = 2000 + (tempdata[4] >> 4) + (tempdata[5] & 0xF) * 10;
	}
	
	return;
}

/********************************************************************/
/* temperature_indoor_reset
 * Reset indoor min/max temperatures with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int temperature_indoor_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current temperature into data_value
	address=0x346;
	number=2;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x34B;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x354;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x350;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x35E;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}

	return 1;
}


/********************************************************************/
/* temperature_outdoor
 * Read indoor temperature, current temperature only
 * 
 * Input: data - pointer to data buffer
 *
 * Returns: Temperature (deg C if temperature_conv is 0)
 *                      (deg F if temperature_conv is 1)
 *
 ********************************************************************/
double temperature_outdoor(unsigned char *data)
{
	int address=0x3D;
  unsigned char *tempdata = data + address;

  return temperature_conv(((tempdata[1] & 0xF) * 10 +
		          (tempdata[0] >> 4) + (tempdata[0] & 0xF) / 10.0) - 40.0);
}


/********************************************************************
 * temperature_outdoor_minmax
 * Read outdoor min/max temperatures with timestamps
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Output: Temperatures temp_min and temp_max
 *                (deg C if temperature_conv is 0)
 *                (deg F if temperature_conv is 1)
 *         Timestamps for temp_min and temp_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 ********************************************************************/
void temperature_outdoor_minmax(unsigned char *data,
                                double *temp_min,
                                double *temp_max,
                                struct timestamp *time_min,
                                struct timestamp *time_max)
{
	int address_min=0x3F;
	int address_max=0x42;
	int address_mintime=0x43;
	int address_maxtime=0x48;
	
  unsigned char *tempdata = data + address_min;
	
	*temp_min = temperature_conv(((tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) +
		          (tempdata[0] >> 4) / 10.0 ) - 40.0);

  tempdata = data + address_max;
	*temp_max = temperature_conv(((tempdata[1] & 0xF) * 10 +
		          (tempdata[0] >> 4) + (tempdata[0] & 0xF) / 10.0) - 40.0);

	tempdata = data + address_mintime;
	if (time_min != NULL)
	{	
		time_min->minute = (tempdata[0] >> 4) + (tempdata[1] & 0xF) * 10;
		time_min->hour = (tempdata[1] >> 4) + (tempdata[2] & 0xF) * 10;
		time_min->day = (tempdata[2] >> 4) + (tempdata[3] & 0xF) * 10;
		time_min->month = (tempdata[3] >> 4) + (tempdata[4] & 0xF) * 10;
    time_min->year = 2000 + (tempdata[4] >> 4) + (tempdata[5] & 0xF) * 10;
	}
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = (tempdata[0] >> 4) + (tempdata[1] & 0xF) * 10;
		time_max->hour = (tempdata[1] >> 4) + (tempdata[2] & 0xF) * 10;
		time_max->day = (tempdata[2] >> 4) + (tempdata[3] & 0xF) * 10;
		time_max->month = (tempdata[3] >> 4) + (tempdata[4] & 0xF) * 10;
    time_max->year = 2000 + (tempdata[4] >> 4) + (tempdata[5] & 0xF) * 10;
	}
	
	return;
}


/********************************************************************/
/* temperature_outdoor_reset
 * Reset outdoor min/max temperatures with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int temperature_outdoor_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current temperature into data_value
	address=0x373;
	number=2;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x378;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x381;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x37D;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x38B;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}

	return 1;
}


/********************************************************************
 * dewpoint
 * Read dewpoint, current value only
 * 
 * Input: data - pointer to data buffer
 *
 * Returns: Dewpoint
 *              
 *
 ********************************************************************/
double dewpoint(unsigned char *data)
{
	int address=0x6B;
  unsigned char *tempdata = data + address;

  return temperature_conv(((tempdata[1] & 0xF) * 10 +
		          (tempdata[0] >> 4) + (tempdata[0] & 0xF) / 10.0) - 40.0);
}


/********************************************************************
 * dewpoint_minmax
 * Read outdoor min/max dewpoint with timestamps
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Output: Dewpoints dp_min and dp_max
 *                (deg C if temperature_conv is 0),
 *                (deg F if temperature_conv is 1)
 *         Timestamps for dp_min and dp_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 ********************************************************************/
void dewpoint_minmax(unsigned char *data,
                     double *dp_min,
                     double *dp_max,
                     struct timestamp *time_min,
                     struct timestamp *time_max)
{
	int address_min=0x6D;
	int address_max=0x70;
	int address_mintime=0x71;
	int address_maxtime=0x76;
	
  unsigned char *tempdata = data + address_min;
	
	*dp_min = temperature_conv(((tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) +
		          (tempdata[0] >> 4) / 10.0 ) - 40.0);

  tempdata = data + address_max;
	*dp_max = temperature_conv(((tempdata[1] & 0xF) * 10 +
		          (tempdata[0] >> 4) + (tempdata[0] & 0xF) / 10.0) - 40.0);

	tempdata = data + address_mintime;
	if (time_min != NULL)
	{	
		time_min->minute = (tempdata[0] >> 4) + (tempdata[1] & 0xF) * 10;
		time_min->hour = (tempdata[1] >> 4) + (tempdata[2] & 0xF) * 10;
		time_min->day = (tempdata[2] >> 4) + (tempdata[3] & 0xF) * 10;
		time_min->month = (tempdata[3] >> 4) + (tempdata[4] & 0xF) * 10;
    time_min->year = 2000 + (tempdata[4] >> 4) + (tempdata[5] & 0xF) * 10;
	}
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = (tempdata[0] >> 4) + (tempdata[1] & 0xF) * 10;
		time_max->hour = (tempdata[1] >> 4) + (tempdata[2] & 0xF) * 10;
		time_max->day = (tempdata[2] >> 4) + (tempdata[3] & 0xF) * 10;
		time_max->month = (tempdata[3] >> 4) + (tempdata[4] & 0xF) * 10;
    time_max->year = 2000 + (tempdata[4] >> 4) + (tempdata[5] & 0xF) * 10;
	}
	
	return;
}


/********************************************************************/
/* dewpoint_reset
 * Reset min/max dewpoint with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int dewpoint_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current dewpoint into data_value
	address=0x3CE;
	number=2;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x3D3;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x3DC;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x3D8;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x3E6;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}

	return 1;
}


/********************************************************************
 * humidity_indoor
 * Read indoor relative humidity, current value only
 * 
 * Input: Handle to weatherstation
 * Returns: relative humidity in percent (integer)
 * 
 ********************************************************************/
int humidity_indoor(unsigned char *data)
{
	int address=0x81;
  unsigned char *tempdata = data + address;

	return ((tempdata[0] >> 4) * 10 + (tempdata[0] & 0xF));
}


/********************************************************************
 * humidity_indoor_minmax
 * Read min/max values with timestamps
 * 
 * Input: Handle to weatherstation
 * Output: Relative humidity in % hum_min and hum_max (integers)
 *         Timestamps for hum_min and hum_max in pointers to
 *                timestamp structures for time_min and time_max
 * Returns: releative humidity current value in % (integer)
 *
 ********************************************************************/
void humidity_indoor_minmax(unsigned char *data,
                        int *hum_min,
                        int *hum_max,
                        struct timestamp *time_min,
                        struct timestamp *time_max)
{
	int address_min = 0x82;
  int address_max = 0x83;
	int address_mintime = 0x84;
  int address_maxtime = 0x89;
  unsigned char *tempdata = data + address_min;
  
	if (hum_min != NULL)
		*hum_min = (tempdata[0] >> 4) * 10  + (tempdata[0] & 0xF);
	
	tempdata = data + address_max;
	if (hum_max != NULL)
		*hum_max = (tempdata[0] >> 4) * 10  + (tempdata[0] & 0xF);

  tempdata = data + address_mintime;
	if (time_min != NULL)
	{	
		time_min->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_min->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_min->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_min->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_min->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_max->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_max->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_max->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_max->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	return;
}


/********************************************************************/
/* humidity_indoor_reset
 * Reset min/max indoor humidity with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int humidity_indoor_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current humidity into data_value
	address=0x3FB;
	number=1;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x3FD;
		number=2;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x401;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x3FF;
		number=2;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x40B;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();		
	}

	return 1;
}


/********************************************************************
 * humidity_outdoor
 * Read relative humidity, current value only
 * 
 * Input: data - pointer to data buffer
 * Returns: relative humidity in percent (integer)
 *
 ********************************************************************/
int humidity_outdoor(unsigned char *data)
{
	int address=0x90;
  unsigned char *tempdata = data + address;

	return ((tempdata[0] >> 4) * 10 + (tempdata[0] & 0xF));
}


/********************************************************************
 * humidity_outdoor_minmax
 * Read min/max values with timestamps
 * 
 * Input: Handle to weatherstation
 * Output: Relative humidity in % hum_min and hum_max (integers)
 *         Timestamps for hum_min and hum_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: releative humidity current value in % (integer)
 *
 ********************************************************************/
void humidity_outdoor_minmax(unsigned char *data,
                        int *hum_min,
                        int *hum_max,
                        struct timestamp *time_min,
                        struct timestamp *time_max)
{
	int address_min = 0x91;
  int address_max = 0x92;
	int address_mintime = 0x93;
  int address_maxtime = 0x98;
  unsigned char *tempdata = data + address_min;
  
	if (hum_min != NULL)
		*hum_min = (tempdata[0] >> 4) * 10  + (tempdata[0] & 0xF);
	
	tempdata = data + address_max;
	if (hum_max != NULL)
		*hum_max = (tempdata[0] >> 4) * 10  + (tempdata[0] & 0xF);

  tempdata = data + address_mintime;
	if (time_min != NULL)
	{	
		time_min->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_min->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_min->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_min->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_min->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_max->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_max->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_max->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_max->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	return;
}


/********************************************************************/
/* humidity_outdoor_reset
 * Reset min/max outdoor humidity with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int humidity_outdoor_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current humidity into data_value
	address=0x419;
	number=1;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x41B;
		number=2;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x41F;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x41D;
		number=2;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x429;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}

	return 1;
}


/********************************************************************
 * wind_current
 * Read wind speed, wind direction and last 5 wind directions
 *
 * Input: data - pointer to data buffer
 *
 * Output: winddir - pointer to array of doubles in degrees
 *
 * Returns: Wind speed (double) in the unit given in the loaded config
 *
 ********************************************************************/
double wind_current(unsigned char *data,
                    double *winddir)
{
	int address_windspeed = 0xfd;
	int address_winddirections = 0x106;
  unsigned char *tempdata = data + address_winddirections;
	
	//Calculate wind directions

	winddir[0] = (tempdata[0] & 0xf)*22.5;
	winddir[1] = (tempdata[0] >> 4)*22.5;
	winddir[2] = (tempdata[1] & 0xf)*22.5;
	winddir[3] = (tempdata[1] >> 4)*22.5;
	winddir[4] = (tempdata[2] & 0xf)*22.5;
	winddir[5] = (tempdata[2] >> 4)*22.5;

  tempdata = data + address_windspeed;
	//Calculate raw wind speed 	- convert from m/s to whatever
	return windspeed_conv( (tempdata[1] >> 4) * 10 + (tempdata[1]&0xF) +
                         (tempdata[0] >> 4) * 0.1 + (tempdata[1]&0xF) * 0.01 );
}

/********************************************************************
 * wind_minmax
 * Read min/max wind speeds with timestamps
 * 
 * Input: data - pointer to data buffer
 *
 * Output: Wind wind_min and wind_max (double)
 *                unit defined by config conversion factor
 *         Timestamps for wind_min and wind_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: wind max (double)
 *
 * Note: The function is made so that if a pointer to
 *       wind_min/max and time_min/max is a NULL pointer the function
 *       ignores this parameter. Example: if you only need wind_max
 *       use the function like this..
 *       windmax = wind_minmax(data,NULL,NULL,NULL,NULL);
 *
 ********************************************************************/
double wind_minmax(unsigned char *data,
                   double *wind_min,
                   double *wind_max,
                   struct timestamp *time_min,
                   struct timestamp *time_max)
{
	int address_max = 0x102;
	int address_min = 0x100;
	int address_maxtime = 0xf2;
	int address_mintime = 0xed;
  unsigned char *tempdata = data + address_min;
  
	if (wind_min != NULL)
		*wind_min = windspeed_conv((tempdata[1] & 0xF) * 10 +
                         (tempdata[0] >> 4)  + (tempdata[0] & 0xF) * 0.1 );
	
	tempdata = data + address_max;
	if (wind_max != NULL)
		*wind_max = windspeed_conv( (tempdata[1] >> 4) * 10 + (tempdata[1]&0xF) +
                         (tempdata[0] >> 4) * 0.1);

  tempdata = data + address_mintime;
	if (time_min != NULL)
	{	
		time_min->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_min->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_min->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_min->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_min->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_max->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_max->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_max->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_max->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	return *wind_max;
}


/********************************************************************/
/* wind_reset
 * Reset min/max wind with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int wind_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;
	int i;
	int current_wind;
	
	address=0x527; //Windspeed
	number=3;
	
	for (i=0; i<MAXWINDRETRIES; i++)
	{
		if (read_safe(ws2300, address, number, data_read, command)!=number)
			read_error_exit();
		     
		if ((data_read[0]!=0x00) ||                            //Invalid wind data
		    ((data_read[1]==0xFF)&&(((data_read[2]&0xF)==0)||((data_read[2]&0xF)==1))))
		{
			sleep_long(10); //wait 10 seconds for new wind measurement
			continue;
		}
		else
		{
			break;
		}
	}
	
	current_wind = ( ((data_read[2]&0xF)<<8) + (data_read[1]) ) * 36;

	data_value[0] = current_wind&0xF;
	data_value[1] = (current_wind>>4)&0xF;
	data_value[2] = (current_wind>>8)&0xF;
	data_value[3] = (current_wind>>12)&0xF;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x4EE;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x4F8;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x4F4;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x502;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();		
	}

	return 1;
}


/********************************************************************
 * windchill
 * Read wind chill, current value only
 * 
 * Input: data - pointer to data buffer
 *
 * Returns: wind chill  (deg C if config.temperature_conv is not set)
 *                      (deg F if config.temperature_conv is set)
 *
 * It is recommended to run this right after a wind speed reading
 * to enhance the likelyhood that the wind speed is valid
 *
 ********************************************************************/
double windchill(unsigned char *data)
{
	int address=0x54;
  unsigned char *tempdata = data + address;

  return temperature_conv(((tempdata[1] & 0xF) * 10 +
		          (tempdata[0] >> 4) + (tempdata[0] & 0xF) / 10.0) - 40.0);
}


/********************************************************************
 * windchill_minmax
 * Read wind chill min/max with timestamps
 * 
 * Input: Handle to weatherstation
 *        temperature_conv flag (integer) controlling
 *            convertion to deg F
 *
 * Output: Windchill wc_min and wc_max
 *                (deg C if config.temperature_conv is not set)
 *                (deg F if config.temperature_conv is set)
 *         Timestamps for wc_min and wc_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: Nothing
 *
 ********************************************************************/
void windchill_minmax(unsigned char *data,
                      double *wc_min,
                      double *wc_max,
                      struct timestamp *time_min,
                      struct timestamp *time_max)
{
	int address_min=0x56;
	int address_max=0x59;
	int address_mintime=0x5A;
	int address_maxtime=0x5F;
	
  unsigned char *tempdata = data + address_min;
	
	*wc_min = temperature_conv(((tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) +
		          (tempdata[0] >> 4) / 10.0 ) - 40.0);

  tempdata = data + address_max;
	*wc_max= temperature_conv(((tempdata[1] & 0xF) * 10 +
		          (tempdata[0] >> 4) + (tempdata[0] & 0xF) / 10.0) - 40.0);

	tempdata = data + address_mintime;
	if (time_min != NULL)
	{	
		time_min->minute = (tempdata[0] >> 4) + (tempdata[1] & 0xF) * 10;
		time_min->hour = (tempdata[1] >> 4) + (tempdata[2] & 0xF) * 10;
		time_min->day = (tempdata[2] >> 4) + (tempdata[3] & 0xF) * 10;
		time_min->month = (tempdata[3] >> 4) + (tempdata[4] & 0xF) * 10;
    time_min->year = 2000 + (tempdata[4] >> 4) + (tempdata[5] & 0xF) * 10;
	}
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = (tempdata[0] >> 4) + (tempdata[1] & 0xF) * 10;
		time_max->hour = (tempdata[1] >> 4) + (tempdata[2] & 0xF) * 10;
		time_max->day = (tempdata[2] >> 4) + (tempdata[3] & 0xF) * 10;
		time_max->month = (tempdata[3] >> 4) + (tempdata[4] & 0xF) * 10;
    time_max->year = 2000 + (tempdata[4] >> 4) + (tempdata[5] & 0xF) * 10;
	}
	
	return;
}


/********************************************************************/
/* windchill_reset
 * Reset min/max windchill with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int windchill_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current windchill into data_value
	address=0x3A0;
	number=2;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min value to current value
		address=0x3A5;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x3AE;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max value to current value
		address=0x3AA;
		number=4;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x3B8;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}

	return 1;
}


/********************************************************************
 * rain_1h
 * Read rain last 1 hour, current value only
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double rain_1h(unsigned char *data)
{
	int address=0xCC;
  unsigned char *tempdata = data + address;

  return rain_conv((double)((tempdata[0] >> 4) * 0.1 + (tempdata[0] & 0xF) * 0.01 +
         (tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) + 
         (tempdata[2] >> 4) * 1000 + (tempdata[2] & 0xF) * 100));
}

/********************************************************************
 * rain_1h_max
 * Read rain last 1 hourand maximum with timestamp
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Output: Rain maximum in rain_max (double)
 *                unit defined by config conversion factor
 *         Timestamps for rain_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
void rain_1h_max(unsigned char *data,
                   double *rain_max,
                   struct timestamp *time_max)
{
	int address_max = 0xcf;
	int address_maxtime = 0xd2;
  unsigned char *tempdata = data + address_max;
  
	if (rain_max != NULL)
		*rain_max = rain_conv((tempdata[0] >> 4) * 0.1 + (tempdata[0] & 0xF) * 0.01 +
                          (tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) + 
                          (tempdata[2] >> 4) * 1000 + (tempdata[2] & 0xF) * 100);
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_max->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_max->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_max->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_max->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	return;
}


/********************************************************************/
/* rain_1h_max_reset
 * Reset max rain 1h with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int rain_1h_max_reset(WEATHERSTATION ws2300)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current rain 1h into data_value
	address=0x4B4;
	number=3;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	data_value[4] = data_read[2]&0xF;
	data_value[5] = data_read[2]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	// Set max value to current value
	address=0x4BA;
	number=6;
	
	if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
		write_error_exit();

	// Set max value timestamp to current time
	address=0x4C0;
	number=10;

	if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
		write_error_exit();

	return 1;
}

/********************************************************************/
/* rain_1h_reset
 * Reset current rain 1h
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int rain_1h_reset(WEATHERSTATION ws2300)
{
	unsigned char data[50];
	unsigned char command[60];	//room for write data also
	int address;
	int number;

	// First overwrite the 1h rain history with zeros
	address=0x479;
	number=30;
	memset(&data, 0, sizeof(data));
	
	if (write_safe(ws2300, address, number, WRITENIB, data, command) != number)
		write_error_exit();
	
	// Set value to zero
	address=0x4B4;
	number=6;
	
	if (write_safe(ws2300, address, number, WRITENIB, data, command) != number)
		write_error_exit();

	return 1;
}


/********************************************************************
 * rain_24h
 * Read rain last 24 hours, current value only
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double rain_24h(unsigned char *data)
{
	int address=0xBD;
  unsigned char *tempdata = data + address;

	return rain_conv((tempdata[0] >> 4) * 0.1 + (tempdata[0] & 0xF) * 0.01 +
         (tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) + 
         (tempdata[2] >> 4) * 1000 + (tempdata[2] & 0xF) * 100);
}


/********************************************************************
 * rain_24h_max
 * Read 24 hours maximum rain with timestamp
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Output: Rain maximum in rain_max (double)
 *                unit defined by config conversion factor
 *         Timestamp for rain_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
void rain_24h_max(unsigned char *data,
                   double *rain_max,
                   struct timestamp *time_max)
{
	int address_max = 0xc0;
	int address_maxtime = 0xc3;
  unsigned char *tempdata = data + address_max;
  
	if (rain_max != NULL)
		*rain_max = rain_conv((tempdata[0] >> 4) * 0.1 + (tempdata[0] & 0xF) * 0.01 +
                          (tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) + 
                          (tempdata[2] >> 4) * 1000 + (tempdata[2] & 0xF) * 100);
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_max->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_max->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_max->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_max->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	return;
}


/********************************************************************/
/* rain_24h_max_reset
 * Reset max rain 24h with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int rain_24h_max_reset(WEATHERSTATION ws2300)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current rain 24h into data_value
	address=0x497;
	number=3;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_value[0] = data_read[0]&0xF;
	data_value[1] = data_read[0]>>4;
	data_value[2] = data_read[1]&0xF;
	data_value[3] = data_read[1]>>4;
	data_value[4] = data_read[2]&0xF;
	data_value[5] = data_read[2]>>4;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
	
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	// Set max value to current value
	address=0x49D;
	number=6;
	
	if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
		write_error_exit();

	// Set max value timestamp to current time
	address=0x4A3;
	number=10;

	if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
		write_error_exit();

	return 1;
}


/********************************************************************/
/* rain_24h_reset
 * Reset current rain 24h
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int rain_24h_reset(WEATHERSTATION ws2300)
{
	unsigned char data[50];
	unsigned char command[60];	//room for write data also
	int address;
	int number;

	// First overwrite the 24h rain history with zeros
	address=0x446;
	number=48;
	memset(&data, 0, sizeof(data));
	
	if (write_safe(ws2300, address, number, WRITENIB, data, command) != number)
		write_error_exit();
	
	// Set value to zero
	address=0x497;
	number=6;
	
	if (write_safe(ws2300, address, number, WRITENIB, data, command) != number)
		write_error_exit();

	return 1;
}

/********************************************************************
 * rain_1w
 * Read rain last week, current value only
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double rain_1w(unsigned char *data)
{
	int address=0xae;
  unsigned char *tempdata = data + address;

	return rain_conv((tempdata[0] >> 4) * 0.1 + (tempdata[0] & 0xF) * 0.01 +
         (tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) + 
         (tempdata[2] >> 4) * 1000 + (tempdata[2] & 0xF) * 100);
}


/********************************************************************
 * rain_1w_max
 * Read 1 week maximum rain with timestamp
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Output: Rain maximum in rain_max (double)
 *                unit defined by config conversion factor
 *         Timestamp for rain_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
void rain_1w_max(unsigned char *data,
                   double *rain_max,
                   struct timestamp *time_max)
{
	int address_max = 0xb1;
	int address_maxtime = 0xb4;
  unsigned char *tempdata = data + address_max;
  
	if (rain_max != NULL)
		*rain_max = rain_conv((tempdata[0] >> 4) * 0.1 + (tempdata[0] & 0xF) * 0.01 +
                          (tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) + 
                          (tempdata[2] >> 4) * 1000 + (tempdata[2] & 0xF) * 100);
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_max->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_max->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_max->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_max->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	return;
}

/********************************************************************
 * rain_1m
 * Read rain last month, current value only
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double rain_1m(unsigned char *data)
{
	int address=0x9f;
  unsigned char *tempdata = data + address;

	return rain_conv((tempdata[0] >> 4) * 0.1 + (tempdata[0] & 0xF) * 0.01 +
         (tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) + 
         (tempdata[2] >> 4) * 1000 + (tempdata[2] & 0xF) * 100);
}


/********************************************************************
 * rain_1m_max
 * Read 1 month maximum rain with timestamp
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Output: Rain maximum in rain_max (double)
 *                unit defined by config conversion factor
 *         Timestamp for rain_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
void rain_1m_max(unsigned char *data,
                   double *rain_max,
                   struct timestamp *time_max)
{
	int address_max = 0xa2;
	int address_maxtime = 0xa5;
  unsigned char *tempdata = data + address_max;
  
	if (rain_max != NULL)
		*rain_max = rain_conv((tempdata[0] >> 4) * 0.1 + (tempdata[0] & 0xF) * 0.01 +
                          (tempdata[1] >> 4) * 10 + (tempdata[1] & 0xF) + 
                          (tempdata[2] >> 4) * 1000 + (tempdata[2] & 0xF) * 100);
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_max->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_max->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_max->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_max->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	return;
}

/********************************************************************
 * rain_total
 * Read rain accumulated total, current value only
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
double rain_total(unsigned char *data)
{
	int address=0xdc;
  unsigned char *tempdata = data + address;

	return rain_conv((tempdata[0] >> 4) + (tempdata[0] & 0xF) * 0.1 +
         (tempdata[1] >> 4) * 100 + (tempdata[1] & 0xF) * 10 + 
         (tempdata[2] & 0xF) * 1000);
}


/********************************************************************
 * rain_total_time
 * Read rain total accumulated with timestamp
 * 
 * Input: Handle to weatherstation
 *        rain_conv_factor controlling convertion to other
 *             units than mm
 *
 * Output: Timestamp for rain total in pointers to
 *                timestamp structures for time_since
 *
 * Returns: rain (double) converted to unit given in config
 *
 ********************************************************************/
void rain_total_time(unsigned char *data,
                   struct timestamp *time_since)
{
	int address_time = 0xde;
  unsigned char *tempdata = data + address_time;
  
	if (time_since != NULL)
	{
		time_since->minute = (tempdata[0] >> 4) + (tempdata[1] & 0xF) * 10;
		time_since->hour = (tempdata[1] >> 4) + (tempdata[2] & 0xF) * 10;
		time_since->day = (tempdata[2] >> 4) + (tempdata[3] & 0xF) * 10;
		time_since->month = (tempdata[3] >> 4) + (tempdata[4] & 0xF) * 10;
    time_since->year = 2000 + (tempdata[4] >> 4) + (tempdata[5] & 0xF) * 10;
	}
	
	return;
}


/********************************************************************/
/* rain_total_reset
 * Reset current total rain
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int rain_total_reset(WEATHERSTATION ws2300)
{
	unsigned char data_read[20];
	unsigned char data_value[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	// Set value to zero
	address=0x4D1;
	number=7;
	memset(&data_value, 0, sizeof(data_value));
	
	if (write_safe(ws2300, address, number, WRITENIB, data_value, command) != number)
		write_error_exit();

	// Set max value timestamp to current time
	address=0x4D8;
	number=10;

	if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
		write_error_exit();

	return 1;
}


/********************************************************************
 * rel_pressure
 * Read relaive air pressure, current value only
 * 
 * Input: data - pointer to data buffer
 *
 * 
 * Returns: pressure (double) converted to unit given in config
 *
 ********************************************************************/
double rel_pressure(unsigned char *data)
{
	int address=0x13D;
  unsigned char *tempdata = data + address;

  return pressure_conv((tempdata[2] & 0xF) * 1000 + (tempdata[1] >> 4) * 100 +
	         (tempdata[1] & 0xF) * 10 + (tempdata[0] >> 4) +
	         (tempdata[0] & 0xF) / 10.0);
}


/********************************************************************
 * rel_pressure_minmax
 * Read relative pressure min/max with timestamps
 * 
 * Input: Handle to weatherstation
 *        pressure_conv_factor controlling convertion to other
 *             units than hPa
 *
 * Output: Pressure pres_min and pres_max (double)
 *                unit defined by config conversion factor
 *         Timestamps for pres_min and pres_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: nothing
 *
 ********************************************************************/
void rel_pressure_minmax(unsigned char *data,
                         double *pres_min,
                         double *pres_max,
                         struct timestamp *time_min,
                         struct timestamp *time_max)
{
	int address_max = 0x156;
	int address_min = 0x14C;
	int address_maxtime = 0x160;
	int address_mintime = 0x15b;
  unsigned char *tempdata = data + address_min;
  
	if (pres_min != NULL)
		*pres_min = pressure_conv((tempdata[2] & 0xF) * 1000 +
                         (tempdata[1] >> 4) * 100  + (tempdata[1] & 0xF) * 10 +
                         (tempdata[0] >> 4)  + (tempdata[0] & 0xF) * 0.1);
	
	tempdata = data + address_max;
	if (pres_max != NULL)
		*pres_max = pressure_conv((tempdata[2] & 0xF) * 1000 +
                         (tempdata[1] >> 4) * 100  + (tempdata[1] & 0xF) * 10 +
                         (tempdata[0] >> 4)  + (tempdata[0] & 0xF) * 0.1);
	
  tempdata = data + address_mintime;
	if (time_min != NULL)
	{	
		time_min->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_min->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_min->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_min->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_min->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	tempdata = data + address_maxtime;
	if (time_max != NULL)
	{
		time_max->minute = ((tempdata[0] >> 4) * 10) + (tempdata[0] & 0xF);
		time_max->hour = ((tempdata[1] >> 4) * 10) + (tempdata[1] & 0xF);
		time_max->day = ((tempdata[2] >> 4) * 10) + (tempdata[2] & 0xF);
		time_max->month = ((tempdata[3] >> 4) * 10) + (tempdata[3] & 0xF);
		time_max->year = 2000 + ((tempdata[4] >> 4) * 10) + (tempdata[4] & 0xF);
	}
	
	return;
}


/********************************************************************
 * abs_pressure
 * Read absolute air pressure, current value only
 * 
 * Input: data - pointer to data buffer
 *
 * Returns: pressure (double) converted to unit given in config
 *
 ********************************************************************/
double abs_pressure(unsigned char *data)
{
	int address=0x138;
  unsigned char *tempdata = data + address;

  return pressure_conv((tempdata[2] & 0xF) * 1000 + (tempdata[1] >> 4) * 100 +
	         (tempdata[1] & 0xF) * 10 + (tempdata[0] >> 4) +
	         (tempdata[0] & 0xF) / 10.0);
}


/********************************************************************
 * abs_pressure_minmax
 * Read absolute pressure min/max with timestamps
 * 
 * Input: Handle to weatherstation
 *        pressure_conv_factor controlling convertion to other
 *             units than hPa
 *
 * Output: Pressure pres_min and pres_max (double)
 *                unit defined by config conversion factor
 *         Timestamps for pres_min and pres_max in pointers to
 *                timestamp structures for time_min and time_max
 *
 * Returns: nothing
 *
 ********************************************************************/
void abs_pressure_minmax(WEATHERSTATION ws2300,
                         double pressure_conv_factor,
                         double *pres_min,
                         double *pres_max,
                         struct timestamp *time_min,
                         struct timestamp *time_max)
{
	unsigned char data[20];
	unsigned char command[25];
	int address=0x5F6;
	int bytes=13;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();
	
	*pres_min = ((data[2]&0xF)*1000 + (data[1]>>4)*100 +
	            (data[1]&0xF)*10 + (data[0]>>4) +
	            (data[0]&0xF)/10.0) / pressure_conv_factor;
	
	*pres_max = ((data[12]&0xF)*1000 + (data[11]>>4)*100 +
	            (data[11]&0xF)*10 + (data[10]>>4) +
	            (data[10]&0xF)/10.0) / pressure_conv_factor;
		
	address=0x61E; //Relative pressure time and date for min/max
	bytes=10;
	
	if (read_safe(ws2300, address, bytes, data, command)!=bytes)	
		read_error_exit();

	time_min->minute = ((data[0] >> 4) * 10) + (data[0] & 0xF);
	time_min->hour = ((data[1] >> 4) * 10) + (data[1] & 0xF);
	time_min->day = ((data[2] >> 4) * 10) + (data[2] & 0xF);
	time_min->month = ((data[3] >> 4) * 10) + (data[3] & 0xF);
	time_min->year = 2000 + ((data[4] >> 4) * 10) + (data[4] & 0xF);
	
	time_max->minute = ((data[5] >> 4) * 10) + (data[5] & 0xF);
	time_max->hour = ((data[6] >> 4) * 10) + (data[6] & 0xF);
	time_max->day = ((data[7] >> 4) * 10) + (data[7] & 0xF);
	time_max->month = ((data[8] >> 4) * 10) + (data[8] & 0xF);
	time_max->year = 2000 + ((data[9] >> 4) * 10) + (data[9] & 0xF);
	
	return;
}



/********************************************************************/
/* pressure_reset
 * Reset min/max pressure (relative and absolute) with timestamps
 * 
 * Input: Handle to weatherstation
 *        minmax - char (8 bit integer) that controls if minimum,
 *                 maximum or both are reset
 * Output: None
 *
 * Returns: 1 if success
 *
 ********************************************************************/
int pressure_reset(WEATHERSTATION ws2300, char minmax)
{
	unsigned char data_read[20];
	unsigned char data_value_abs[20];
	unsigned char data_value_rel[20];
	unsigned char data_time[20];
	unsigned char command[25];	//room for write data also
	int address;
	int number;

	// First read current abs/rel pressure into data_value_abs/rel
	address=0x5D8;
	number=8;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_value_abs[0] = data_read[0]&0xF;
	data_value_abs[1] = data_read[0]>>4;
	data_value_abs[2] = data_read[1]&0xF;
	data_value_abs[3] = data_read[1]>>4;
	data_value_abs[4] = data_read[2]&0xF;
	
	data_value_rel[0] = data_read[5]&0xF;
	data_value_rel[1] = data_read[5]>>4;
	data_value_rel[2] = data_read[6]&0xF;
	data_value_rel[3] = data_read[6]>>4;
	data_value_rel[4] = data_read[7]&0xF;
	
	// Get current time from station
	address=0x23B;
	number=6;
	
	if (read_safe(ws2300, address, number, data_read, command) != number)
		read_error_exit();
		
	data_time[0] = data_read[0]&0xF;
	data_time[1] = data_read[0]>>4;
	data_time[2] = data_read[1]&0xF;
	data_time[3] = data_read[1]>>4;
	data_time[4] = data_read[2]>>4;
	data_time[5] = data_read[3]&0xF;
	data_time[6] = data_read[3]>>4;
	data_time[7] = data_read[4]&0xF;
	data_time[8] = data_read[4]>>4;
	data_time[9] = data_read[5]&0xF;

	if (minmax & RESET_MIN) // minimum
	{
		// Set min abs value to current abs value
		address=0x5F6;
		number=5;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value_abs, command) != number)
			write_error_exit();
			
		// Set min rel value to current rel value
		address=0x600;
		number=5;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value_rel, command) != number)
			write_error_exit();

		// Set min value timestamp to current time
		address=0x61E;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();
	}
	
	if (minmax & RESET_MAX) // maximum
	{
		// Set max abs value to current abs value
		address=0x60A;
		number=5;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value_abs, command) != number)
			write_error_exit();
			
		// Set max rel value to current rel value
		address=0x614;
		number=5;
		
		if (write_safe(ws2300, address, number, WRITENIB, data_value_rel, command) != number)
			write_error_exit();

		// Set max value timestamp to current time
		address=0x628;
		number=10;

		if (write_safe(ws2300, address, number, WRITENIB, data_time, command) != number)
			write_error_exit();		
	}

	return 1;
}


/********************************************************************
 * pressure_correction
 * Read the correction from absolute to relaive air pressure
 * 
 * Input: Handle to weatherstation
 *        pressure_conv_factor controlling convertion to other
 *             units than hPa
 *
 * Returns: pressure (double) converted to unit given in conv factor
 *
 ********************************************************************/
double pressure_correction(WEATHERSTATION ws2300, double pressure_conv_factor)
{
	unsigned char data[20];
	unsigned char command[25];
	int address=0x5EC;
	int bytes=3;
	
	if (read_safe(ws2300, address, bytes, data, command) != bytes)
		read_error_exit();


	return ((data[2] & 0xF) * 1000 +
	        (data[1] >> 4) * 100 +
	        (data[1] & 0xF) * 10 +
	        (data[0] >> 4) +
	        (data[0] & 0xF) / 10.0 - 
	        1000
	       ) / pressure_conv_factor;
}


/********************************************************************
 * tendency_forecast
 * Read Tendency and Forecast
 * 
 * Input: Handle to weatherstation
 *
 * Output: tendency - string Steady, Rising or Falling
 *         forecast - string Rainy, Cloudy or Sunny
 *
 * Returns: nothing
 *
 ********************************************************************/
void tendency_forecast(unsigned char *data, char *tendency, char *forecast)
{
	int address=0x24;
  unsigned char *tempdata = data + address;

	const char *tendency_values[] = { "Steady", "Rising", "Falling" };
	const char *forecast_values[] = { "Rainy", "Cloudy", "Sunny" };

	strcpy(tendency, tendency_values[tempdata[0] >> 4]);
	strcpy(forecast, forecast_values[tempdata[0] & 0xF]);

	return;
}


/********************************************************************
 * read_history_info
 * Read the history information like interval, countdown, time
 * of last record, pointer to last record.
 * 
 * Input:  Handle to weatherstation
 *        
 * Output: interval - Current interval in minutes (integer)
 *         countdown - Countdown to next measurement (integer)
 *         timelast - Time/Date for last measurement (timestamp struct)
 *         no_records - number of valid records (integer)
 *
 * Returns: interger pointing to last written record. [0x00-0xAE]
 *
 ********************************************************************/
int read_history_info(WEATHERSTATION ws2300, int *interval, int *countdown,
                 struct timestamp *time_last, int *no_records)
{
	unsigned char data[20];
	unsigned char command[25];
	int address=0x6B2;
	int bytes=10;

	if (read_safe(ws2300, address, bytes, data, command) != bytes)
	    read_error_exit();
	
	*interval = (data[1] & 0xF)*256 + data[0] + 1;
	*countdown = data[2]*16 + (data[1] >> 4) + 1;
	time_last->minute = ((data[3] >> 4) * 10) + (data[3] & 0xF);
	time_last->hour = ((data[4] >> 4) * 10) + (data[4] & 0xF);
	time_last->day = ((data[5] >> 4) * 10) + (data[5] & 0xF);
	time_last->month = ((data[6] >> 4) * 10) + (data[6] & 0xF);
	time_last->year = 2000 + ((data[7] >> 4) * 10) + (data[7] & 0xF);
	*no_records = data[9];

	return data[8];

}


/********************************************************************
 * read_history_record
 * Read the history information like interval, countdown, time
 * of last record, pointer to last record.
 * 
 * Input:  Handle to weatherstation
 *         config structure with conversion factors
 *         record - record index number to be read [0x00-0xAE]
 *        
 * Output: temperature_indoor (double)
 *         temperature_indoor (double)
 *         pressure (double)
 *         humidity_indoor (integer)
 *         humidity_outdoor (integer)
 *         raincount (double)
 *         windspeed (double)
 *         windir_degrees (double)
 *         dewpoint (double) - calculated
 *         windchill (double) - calculated, new post 2001 formula
 *
 * Returns: interger index number pointing to next record 
 *
 ********************************************************************/
int read_history_record(unsigned char *data,
                        int record_no,
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
                        struct tm *time)
{
	unsigned char *record;
	char str[30];
	
	sprintf(str,"read_history_record, rec=%i",record_no);
  print_log(1,str);
	record = data + record_no * 18;

	//if (read_safe(ws, address, bytes, data, NULL) != bytes)
	//    read_error_exit();
	
	//for (i = 0; i < 18; i++)
	 //printf("%02X ",record[i]);
	//printf("\n");
	
	time->tm_sec = 0;
	time->tm_min = ((record[0] & 0xf0) >> 4)* 10 + (record[0] & 0xf);
	time->tm_hour = ((record[1] & 0xf0) >> 4) * 10 + (record[1] & 0xf);
	time->tm_mday = ((record[2] & 0xf0) >> 4) * 10 + (record[2] & 0xf);
	time->tm_mon = ((record[3] & 0xf0) >> 4) * 10 + (record[3] & 0xf) - 1;
	time->tm_year = ((record[4] & 0xf0) >> 4) * 10 + (record[4] & 0xf) + 100;
	time->tm_isdst = -1;
	*temperature_indoor = ((record[5] & 0xf0) >> 4) * 10 + (record[5] & 0xf);
	*temperature_indoor += (record[6] & 0xf) * 100 - 400;
	*temperature_indoor /= 10;
		
  *temperature_outdoor = (record[6] & 0xf0) >> 4;
  *temperature_outdoor += ((record[7] & 0xf0) >> 4) * 100 + (record[7] & 0xf) * 10 - 400;
  *temperature_outdoor /= 10;
  *pressure = ((record[8] & 0xf0) >> 4) * 10 + (record[8] & 0xf);
  *pressure += ((record[9] & 0xf0) >> 4) * 1000 + (record[9] & 0xf) * 100;
  *pressure += (record[10] & 0xf) * 10000;
  *pressure /= 10;
  *humidity_indoor = (record[10] & 0xf0) >> 4;
  *humidity_indoor += (record[11] & 0xf) * 10;
  *humidity_outdoor = (record[11] & 0xf0) >> 4;;
  *humidity_outdoor += (record[12] & 0xf) * 10;
  *raincount = record[12] >> 4;
  *raincount += record[13] * 16;
  *windspeed = (record[15] & 0xf) * 256 + record[14]; 
  *windspeed /= 10;
  if ((record[17] & 0xf) != 1 || record[16] != 0xFE)
  {
    *windgust = (record[17] & 0xf) * 256 + record[16]; 
    *windgust /= 10;  
  } else
    *windgust = -1;  
  *winddir_degrees = ((record[15] & 0xF0)>> 4) * 22.5;
  *dewpoint = calculate_dewpoint(*temperature_outdoor,*humidity_outdoor);
  *windchill = calculate_windchill(*temperature_outdoor,*windspeed);
	
	return (++record_no)%1797;
}


/********************************************************************
 * light
 * Turns display light on and off
 *
 * Input: control - integer -   0 = off, Anything else = on
 *
 * Returns: Nothing
 *
 ********************************************************************/
void light(WEATHERSTATION ws2300, int control)
{
	unsigned char data;
	unsigned char command[25];  //Data returned is just ignored
	int address=0x016;
	int number=1;
	unsigned char encode_constant;
	
	data = 0;
	encode_constant = UNSETBIT;
	if (control != 0)
		encode_constant = SETBIT;
		
	if (write_safe(ws2300, address, number, encode_constant, &data, command)!=number)
		write_error_exit();
	
	return;	
}


/********************************************************************
 * read_error_exit
 * exit location for all calls to read_safe for error exit.
 * includes error reporting.
 *
 ********************************************************************/
void read_error_exit(void)
{
	perror("read_safe() error");
	exit(0);
}

/********************************************************************
 * write_error_exit
 * exit location for all calls to write_safe for error exit.
 * includes error reporting.
 *
 ********************************************************************/
void write_error_exit(void)
{
	perror("write_safe() error");
	exit(0);
}


/********************************************************************
 * get_configuration()
 *
 * read setup parameters from ws3600.conf
 * It searches in this sequence:
 * 1. Path to config file including filename given as parameter
 * 2. ./open3600.conf
 * 3. /usr/local/etc/open3600.conf
 * 4. /etc/open3600.conf
 *
 * See file open3600.conf-dist for the format and option names/values
 *
 * input:    config file name with full path - pointer to string
 *
 * output:   struct config populated with valid settings either
 *           from config file or defaults
 *
 * returns:  0 = OK
 *          -1 = no config file or file open error
 *
 ********************************************************************/
int get_configuration(struct config_type *config, char *path)
{
	FILE *fptr;
	char inputline[1000] = "";
	char token[100] = "";
	char val[100] = "";
	char val2[100] = "";
	
	// First we set everything to defaults - faster than many if statements
	strcpy(config->serial_device_name, DEFAULT_SERIAL_DEVICE);  // Name of serial device
	strcpy(config->citizen_weather_id, "CW0000");               // Citizen Weather ID
	strcpy(config->citizen_weather_latitude, "5540.12N");       // latitude default Glostrup, DK
	strcpy(config->citizen_weather_longitude, "01224.60E");     // longitude default, Glostrup, DK
	strcpy(config->aprs_host[0].name, "aprswest.net");         // host1 name
	config->aprs_host[0].port = 23;                            // host1 port
	strcpy(config->aprs_host[1].name, "indiana.aprs2.net");    // host2 name
	config->aprs_host[1].port = 23;                            // host2 port
	config->num_hosts = 2;                                     // default number of defined hosts
	strcpy(config->weather_underground_id, "WUID");             // Weather Underground ID 
	strcpy(config->weather_underground_password, "WUPassword"); // Weather Underground Password
	strcpy(config->timezone, "1");                              // Timezone, default CET
	config->wind_speed_conv_factor = 1.0;                   // Speed dimention, m/s is default
	config->temperature_conv = 0;                           // Temperature in Celcius
	config->rain_conv_factor = 1.0;                         // Rain in mm
	config->pressure_conv_factor = 1.0;                     // Pressure in hPa (same as millibar)
	strcpy(config->mysql_host, "localhost");            // localhost, IP or domainname of server
	strcpy(config->mysql_user, "open3600");             // MySQL database user name
	strcpy(config->mysql_passwd, "mysql3600");          // Password for MySQL database user
	strcpy(config->mysql_database, "open3600");         // Name of MySQL database
	config->mysql_port = 0;                             // MySQL port. 0 means default port/socket
	strcpy(config->pgsql_connect, "hostaddr='127.0.0.1'dbname='open3600'user='postgres'"); // connection string
	strcpy(config->pgsql_table, "weather");             // PgSQL table name
	strcpy(config->pgsql_station, "open3600");          // Unique station id
	config->log_level = 0;

	// open the config file

	fptr = NULL;
	if (path != NULL)
		fptr = fopen(path, "r");       //first try the parameter given
	if (fptr == NULL)                  //then try default search
	{
		if ((fptr = fopen("open3600.conf", "r")) == NULL)
		{
			if ((fptr = fopen("/usr/local/etc/open3600.conf", "r")) == NULL)
			{
				if ((fptr = fopen("/etc/open3600.conf", "r")) == NULL)
				{
					//Give up and use defaults
					return(-1);
				}
			}
		}
	}

	while (fscanf(fptr, "%[^\n]\n", inputline) != EOF)
	{
		sscanf(inputline, "%[^= \t]%*[ \t=]%s%*[, \t]%s%*[^\n]", token, val, val2);

		if (token[0] == '#')	// comment
			continue;

		if ((strcmp(token,"SERIAL_DEVICE")==0) && (strlen(val) != 0))
		{
			strcpy(config->serial_device_name,val);
			continue;
		}

		if ((strcmp(token,"CITIZEN_WEATHER_ID")==0) && (strlen(val) != 0))
		{
			strcpy(config->citizen_weather_id, val);
			continue;
		}
		
		if ((strcmp(token,"CITIZEN_WEATHER_LATITUDE")==0) && (strlen(val)!=0))
		{
			strcpy(config->citizen_weather_latitude, val);
			continue;
		}

		if ((strcmp(token,"CITIZEN_WEATHER_LONGITUDE")==0) && (strlen(val)!=0))
		{
			strcpy(config->citizen_weather_longitude, val);
			continue;
		}
		
		if ((strcmp(token,"APRS_SERVER")==0) && (strlen(val)!=0) && (strlen(val2)!=0))
		{
			if ( config->num_hosts >= MAX_APRS_HOSTS)
				continue;           // ignore host definitions over the defined max
			strcpy(config->aprs_host[config->num_hosts].name, val);
			config->aprs_host[config->num_hosts].port = atoi(val2);
			config->num_hosts++;    // increment for next
			continue;
		}

		if ((strcmp(token,"WEATHER_UNDERGROUND_ID")==0) && (strlen(val)!=0))
		{
			strcpy(config->weather_underground_id, val);
			continue;
		}

		if ((strcmp(token,"WEATHER_UNDERGROUND_PASSWORD")==0)&&(strlen(val)!=0))
		{
			strcpy(config->weather_underground_password, val);
			continue;
		}

		if ((strcmp(token,"TIMEZONE")==0) && (strlen(val) != 0))
		{
			strcpy(config->timezone, val);
			continue;
		}

		if ((strcmp(token,"WIND_SPEED") == 0) && (strlen(val) != 0))
		{
			if (strcmp(val, "m/s") == 0)
				config->wind_speed_conv_factor = METERS_PER_SECOND;
			else if (strcmp(val, "km/h") == 0)
				config->wind_speed_conv_factor = KILOMETERS_PER_HOUR;
			else if (strcmp(val, "MPH") == 0)
				config->wind_speed_conv_factor = MILES_PER_HOUR;
			continue; //else default remains
		}

		if ((strcmp(token,"TEMPERATURE") == 0) && (strlen(val) != 0))
		{
			if (strcmp(val, "C") == 0)
				config->temperature_conv = CELCIUS;
			else if (strcmp(val, "F") == 0)
				config->temperature_conv = FAHRENHEIT;
			continue; //else default remains
		}

		if ((strcmp(token,"RAIN") == 0) && (strlen(val) != 0))
		{
			if (strcmp(val, "mm") == 0)
				config->rain_conv_factor = MILLIMETERS;
			else if (strcmp(val, "IN") == 0)
				config->rain_conv_factor = INCHES;
			continue; //else default remains
		}

		if ((strcmp(token,"PRESSURE") == 0) && (strlen(val) != 0))
		{
			if ( (strcmp(val, "hPa") == 0) || (strcmp(val, "mb") == 0))
				config->pressure_conv_factor = HECTOPASCAL;
			else if (strcmp(val, "INHG") == 0)
				config->pressure_conv_factor = INCHES_HG;
			continue; //else default remains
		}

		if ((strcmp(token,"MYSQL_HOST") == 0) && (strlen(val) != 0))
		{
			strcpy(config->mysql_host, val);
			continue;
		}

		if ( (strcmp(token,"MYSQL_USERNAME") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->mysql_user, val);
			continue;
		}

		if ( (strcmp(token,"MYSQL_PASSWORD") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->mysql_passwd, val);
			continue;
		}

		if ( (strcmp(token,"MYSQL_DATABASE") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->mysql_database, val);
			continue;
		}
		
		if ( (strcmp(token,"MYSQL_PORT") == 0) && (strlen(val) != 0) )
		{
			config->mysql_port = atoi(val);
			continue;
		}

		if ( (strcmp(token,"PGSQL_CONNECT") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->pgsql_connect, val);
			continue;
		}
		
		if ( (strcmp(token,"PGSQL_TABLE") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->pgsql_table, val);
			continue;
		}
		
		if ( (strcmp(token,"PGSQL_STATION") == 0) && (strlen(val) != 0) )
		{
			strcpy(config->pgsql_station, val);
			continue;
		}
		
		if ((strcmp(token,"LOG_LEVEL")==0) && (strlen(val)!=0))
		{
			config->log_level = atoi(val);
			continue;
		}
	}

	return (0);
}


 /********************************************************************
 * address_encoder converts an 16 bit address to the form needed
 * by the WS-2300 when sending commands.
 *
 * Input:   address_in (interger - 16 bit)
 * 
 * Output:  address_out - Pointer to an unsigned character array.
 *          3 bytes, not zero terminated.
 * 
 * Returns: Nothing.
 *
 ********************************************************************/
void address_encoder(int address_in, unsigned char *address_out)
{
	int i = 0;
	int adrbytes = 4;
	unsigned char nibble;

	for (i = 0; i < adrbytes; i++)
	{
		nibble = (address_in >> (4 * (3 - i))) & 0x0F;
		address_out[i] = (unsigned char) (0x82 + (nibble * 4));
	}

	return;
}


/********************************************************************
 * data_encoder converts up to 15 data bytes to the form needed
 * by the WS-2300 when sending write commands.
 *
 * Input:   number - number of databytes (integer)
 *          encode_constant - unsigned char
 *                            0x12=set bit, 0x32=unset bit, 0x42=write nibble
 *          data_in - char array with up to 15 hex values
 * 
 * Output:  address_out - Pointer to an unsigned character array.
 * 
 * Returns: Nothing.
 *
 ********************************************************************/
void data_encoder(int number, unsigned char encode_constant,
                  unsigned char *data_in, unsigned char *data_out)
{
	int i = 0;

	for (i = 0; i < number; i++)
	{
		data_out[i] = (unsigned char) (encode_constant + (data_in[i] * 4));
	}

	return;
}


/********************************************************************
 * numberof_encoder converts the number of bytes we want to read
 * to the form needed by the WS-2300 when sending commands.
 *
 * Input:   number interger, max value 15
 * 
 * Returns: unsigned char which is the coded number of bytes
 *
 ********************************************************************/
unsigned char numberof_encoder(int number)
{
	int coded_number;

	coded_number = (unsigned char) (0xC2 + number * 4);
	if (coded_number > 0xfe)
		coded_number = 0xfe;

	return coded_number;
}


/********************************************************************
 * command_check0123 calculates the checksum for the first 4
 * commands sent to WS2300.
 *
 * Input:   pointer to char to check
 *          sequence of command - i.e. 0, 1, 2 or 3.
 * 
 * Returns: calculated checksum as unsigned char
 *
 ********************************************************************/
unsigned char command_check0123(unsigned char *command, int sequence)
{
	int response;

	response = sequence * 16 + ((*command) - 0x82) / 4;

	return (unsigned char) response;
}


/********************************************************************
 * command_check4 calculates the checksum for the last command
 * which is sent just before data is received from WS2300
 *
 * Input: number of bytes requested
 * 
 * Returns: expected response from requesting number of bytes
 *
 ********************************************************************/
unsigned char command_check4(int number)
{
	int response;

	response = 0x30 + number;

	return response;
}


/********************************************************************
 * data_checksum calculates the checksum for the data bytes received
 * from the WS2300
 *
 * Input:   pointer to array of data to check
 *          number of bytes in array
 * 
 * Returns: calculated checksum as unsigned char
 *
 ********************************************************************/
unsigned char data_checksum(unsigned char *data, int number)
{
	int checksum = 0;
	int i;

	for (i = 0; i < number; i++)
	{
		checksum += data[i];
	}

	checksum &= 0xFF;

	return (unsigned char) checksum;
}


/********************************************************************
 * initialize resets WS2300 to cold start (rewind and start over)
 * 
 * Input:   device number of the already open serial port
 *           
 * Returns: 0 if fail, 1 if success
 *
 ********************************************************************/
int initialize(WEATHERSTATION ws2300)
{
	unsigned char command = 0x06;
	unsigned char answer;

	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	write_device(ws2300, &command, 1);
	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	write_device(ws2300, &command, 1);

	if (read_device(ws2300, &answer, 1) != 1)
		return 0;

	if (answer != 2)
		return 0;

	return 1;
}


/********************************************************************
 * read_data reads data from the WS2300 based on a given address,
 * number of data read, and a an already open serial port
 *
 * Inputs:  serdevice - device number of the already open serial port
 *          number - number of bytes to read, max value 15
 *
 * Output:  readdata - pointer to an array of chars containing
 *                     the just read data, not zero terminated
 * 
 * Returns: number of bytes read, -1 if failed
 *
 ********************************************************************/
int read_data(WEATHERSTATION ws, int number,
			  unsigned char *readdata)
{
  unsigned char command = 0xa1;
  int i;
  
  if (write_byte(ws,command))
  {
    for (i = 0; i < number; i++)
    {
      readdata[i] = read_byte(ws);
      if (i + 1 < number)
        read_next_byte_seq(ws);
      //printf("%i\n",readdata[i]);
    }
    
    read_last_byte_seq(ws);
    
  	return i;
  } else
    return -1;
}


/********************************************************************
 * write_data writes data to the WS2300.
 * It can both write nibbles and set/unset bits
 *
 * Inputs:      ws2300 - device number of the already open serial port
 *              address (interger - 16 bit)
 *              number - number of nibbles to be written/changed
 *                       must 1 for bit modes (SETBIT and UNSETBIT)
 *                       max 80 for nibble mode (WRITENIB)
 *              writedata - pointer to an array of chars containing
 *                          data to write, not zero terminated
 *                          data must be in hex - one digit per byte
 *                          If bit mode value must be 0-3 and only
 *                          the first byte can be used.
 * 
 * Output:      commanddata - pointer to an array of chars containing
 *                            the commands that were sent to the station
 *
 * Returns:     number of bytes written, -1 if failed
 *
 ********************************************************************/
int write_data(WEATHERSTATION ws, int address, int number,
			   unsigned char *writedata)
{
  unsigned char command = 0xa0;
  int i = -1;
  
  write_byte(ws,command);
  write_byte(ws,address/256);
  write_byte(ws,address%256);
  
  if (writedata!=NULL) {
    for (i = 0; i < number; i++)
    {
      write_byte(ws,writedata[i]);
    }
  }
  
  set_DTR(ws,0);
  nanodelay(DELAY_CONST);
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
  set_RTS(ws,1);
  nanodelay(DELAY_CONST);
  set_DTR(ws,1);
  nanodelay(DELAY_CONST);
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
  
//return -1 for errors
	return i;
}


/********************************************************************
 * read_safe Read data, retry until success or maxretries
 * Reads data from the WS2300 based on a given address,
 * number of data read, and a an already open serial port
 * Uses the read_data function and has same interface
 *
 * Inputs:  ws2300 - device number of the already open serial port
 *          address (interger - 16 bit)
 *          number - number of bytes to read, max value 15
 *
 * Output:  readdata - pointer to an array of chars containing
 *                     the just read data, not zero terminated
 *          commanddata - pointer to an array of chars containing
 *                     the commands that were sent to the station
 * 
 * Returns: number of bytes read, -1 if failed
 *
 ********************************************************************/
int read_safe(WEATHERSTATION ws, int address, int number,
			  unsigned char *readdata, unsigned char *commanddata)
{
	int i,j;
	unsigned char readdata2[32768];
	
	print_log(1,"read_safe");

	for (j = 0; j < MAXRETRIES; j++)
	{	
		write_data(ws, address, 0, NULL);
    read_data(ws, number, readdata);
    
    write_data(ws, address, 0, NULL);
    read_data(ws, number, readdata2);
    
    if (memcmp(readdata,readdata2,number) == 0)
    {
      //check if only 0's for reading memory range greater then 10 bytes
      print_log(2,"read_safe - two readings identical");
      i = 0;
      if (number > 10)
      {
        for (; readdata[i] == 0 && i < number; i++);
      }
      
      if (i != number)
        break;
      else
        print_log(2,"read_safe - only zeros");
    } else
      print_log(2,"read_safe - two readings not identical");
	}

	// If we have tried MAXRETRIES times to read we expect not to
	// have valid data
	if (j == MAXRETRIES)
	{
		return -1;
	}

	return number;
}


/********************************************************************
 * write_safe Write data, retry until success or maxretries
 * Writes data to the WS2300 based on a given address,
 * number of data to write, and a an already open serial port
 * Uses the write_data function and has same interface
 *
 * Inputs:      serdevice - device number of the already open serial port
 *              address (interger - 16 bit)
 *              number - number of nibbles to be written/changed
 *                       must 1 for bit modes (SETBIT and UNSETBIT)
 *                       unlimited for nibble mode (WRITENIB)
 *              encode_constant - unsigned char
 *                               (SETBIT, UNSETBIT or WRITENIB)
 *              writedata - pointer to an array of chars containing
 *                          data to write, not zero terminated
 *                          data must be in hex - one digit per byte
 *                          If bit mode value must be 0-3 and only
 *                          the first byte can be used.
 * 
 * Output:      commanddata - pointer to an array of chars containing
 *                            the commands that were sent to the station
 * 
 * Returns: number of bytes written, -1 if failed
 *
 ********************************************************************/
int write_safe(WEATHERSTATION ws2300, int address, int number,
               unsigned char encode_constant, unsigned char *writedata,
               unsigned char *commanddata)
{
	int j;

  print_log(2,"write_safe");
	for (j = 0; j < MAXRETRIES; j++)
	{
		// printf("Iteration = %d\n",j); // debug
		//reset_06(ws2300);

		// Read the data. If expected number of bytes read break out of loop.
		/*if (write_data(ws2300, address, number, encode_constant, writedata,
		    commanddata)==number)
		{
			break;
		}*/
	}

	// If we have tried MAXRETRIES times to read we expect not to
	// have valid data
	if (j == MAXRETRIES)
	{
		return -1;
	}

	return number;
}

void read_next_byte_seq(WEATHERSTATION ws)
{
  print_log(3,"read_next_byte_seq");
  write_bit(ws,0);
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
}

void read_last_byte_seq(WEATHERSTATION ws)
{
  print_log(3,"read_last_byte_seq");
  set_RTS(ws,1);
  nanodelay(DELAY_CONST);
  set_DTR(ws,0);
  nanodelay(DELAY_CONST);
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
  set_RTS(ws,1);
  nanodelay(DELAY_CONST);
  set_DTR(ws,1);
  nanodelay(DELAY_CONST);
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
}

/********************************************************************
 * read_bit  
 * Reads one bit from the COM
 *
 * Inputs:  serdevice - opened file handle
 * 
 * Returns: bit read from the COM
 *
 ********************************************************************/

int read_bit(WEATHERSTATION ws)
{
  int status;
  char str[20];
  
  set_DTR(ws,0);
  nanodelay(DELAY_CONST);
  status = get_CTS(ws);
  nanodelay(DELAY_CONST);
  set_DTR(ws,1);
  nanodelay(DELAY_CONST);
  sprintf(str,"Read bit %i",!status);
  print_log(4,str);
  
  return !status;
}

/********************************************************************
 * write_bit  
 * Writes one bit to the COM
 *
 * Inputs:  serdevice - opened file handle
 *          bit - bit to write 
 * 
 * Returns: nothing
 *
 ********************************************************************/
void write_bit(WEATHERSTATION ws,int bit)
{
  char str[20];
  
  set_RTS(ws,!bit);
	nanodelay(DELAY_CONST);
	set_DTR(ws,0);
	nanodelay(DELAY_CONST);
	set_DTR(ws,1);
	
  sprintf(str,"Write bit %i",bit);
  print_log(4,str);
}



/********************************************************************
 * read_byte  
 * Reads one byte from the COM
 *
 * Inputs:  serdevice - opened file handle
 * 
 * Returns: byte read from the COM
 *
 ********************************************************************/
int read_byte(WEATHERSTATION serdevice)
{
  int byte = 0;
  int i;
  char str[20];
  
  for (i = 0; i < 8; i++)
  {
    byte *= 2;
    byte += read_bit(serdevice);
  }
  sprintf(str,"Read byte %i",byte);
  print_log(3,str);
  
  return byte;
}

/********************************************************************
 * write_byte  
 * Writes one byte to the COM
 *
 * Inputs:  serdevice - opened file handle
 *          byte - byte to write 
 * 
 * Returns: nothing
 *
 ********************************************************************/
int write_byte(WEATHERSTATION ws,int byte)
{ 
  int status;
  int i;
  char str[20];
  
  sprintf(str,"Read byte %i",byte);
  print_log(3,str);
  
  for (i = 0; i < 8; i++)
  {
    write_bit(ws, byte & 0x80);
    byte <<= 1;
    byte &= 0xff;
  }
  
  set_RTS(ws,0);
  nanodelay(DELAY_CONST);
  status = get_CTS(ws);
  //TODO: checking value of status, error routine
  nanodelay(DELAY_CONST);
  set_DTR(ws,0);
  nanodelay(DELAY_CONST);
  set_DTR(ws,1);
  nanodelay(DELAY_CONST);
  if (status)
    return 1;
  else
    return 0;
}

/********************************************************************
 * calculate_dewpoint 
 * Calculates dewpoint value
 * REF http://www.faqs.org/faqs/meteorology/temp-dewpoint/
 *  
 * Inputs:  temperature  in Celcius
 *          humidity
 * 
 * Returns: dewpoint
 *
 ********************************************************************/
double calculate_dewpoint(double temperature, double humidity)
{
  double A, B, C;
  double dewpoint;
	
	A = 17.2694;
	B = (temperature > 0) ? 237.3 : 265.5;
	C = (A * temperature)/(B + temperature) + log((double)humidity/100);
	dewpoint = B * C / (A - C);
	
	return dewpoint;
}

/********************************************************************
 * calculate_windchill
 * Calculates windchill value
 * Calculate windchill using new post 2001 USA/Canadian formula
 * Twc = 13.112 + 0.6215*Ta -11.37*V^0.16 + 0.3965*Ta*V^0.16 [Celcius and km/h] 
 *  
 * Inputs:  temperature in Celcius
 *          windspeed in m/s
 * 
 * Returns: windchill
 *
 ********************************************************************/
double calculate_windchill(double temperature, double windspeed)
{
  double windchill;
  double wind_kmph; 
	
	wind_kmph = 3.6 * windspeed;
	if (wind_kmph > 4.8)
	{
		windchill = 13.12 + 0.6215 * temperature -
		             11.37 * pow(wind_kmph, 0.16) +
		             0.3965 * temperature * pow(wind_kmph, 0.16);
	}
	else
	{
		windchill = temperature;
	}
	
	return windchill;
}

/********************************************************************
 * log
 * Calculates windchill value
 * Calculate windchill using new post 2001 USA/Canadian formula
 * Twc = 13.112 + 0.6215*Ta -11.37*V^0.16 + 0.3965*Ta*V^0.16 [Celcius and km/h] 
 *  
 * Inputs:  temperature in Celcius
 *          windspeed in m/s
 * 
 * Returns: windchill
 *
 ********************************************************************/
void print_log(int log_level, char* str)
{
  if (log_level <= config.log_level)
    fprintf(stderr,"%s\n",str);
}
