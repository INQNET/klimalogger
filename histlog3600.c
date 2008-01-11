/*  open3600 - histlog3600.c
 *  
 *  Version 0.01
 *  
 *  Control WS3600 weather station
 *  
 *  Copyright 2003-2005, Kenneth Lavrsen, Grzegorz Wisniewski, Sander Eerkes
 *  This program is published under the GNU General Public license
 */

#include "rw3600.h"

/********************************************************************
 * print_usage prints a short user guide
 *
 * Input:   none
 * 
 * Output:  prints to stdout
 * 
 * Returns: exits program
 *
 ********************************************************************/
void print_usage(void)
{
	printf("\n");
	printf("histlog3600 - Log history data from WS-3600 to file.\n");
	printf("(C)2003 Kenneth Lavrsen.\n");
	printf("(C)2005 Grzegorz Wisniewski,Sander Eerkes. (Version alfa)\n");
	printf("This program is released under the GNU General Public License (GPL)\n\n");
	printf("Usage:\n");
	printf("histlog3600 log_filename\n");
	exit(0);
}

 
/********** MAIN PROGRAM ************************************************
 *
 * This program reads the history records from a WS3600
 * weather station at a given record range
 * and prints the data to stdout and to a file.
 * Just run the program without parameters for usage.
 *
 * It uses the config file for device name.
 * Config file locations - see open3600.conf
 *
 ***********************************************************************/
int main(int argc, char *argv[])
{
	WEATHERSTATION ws;
	FILE *fileptr;
	unsigned char logline[3000] = "";
	unsigned char data[32768];
	long counter;
	char ch;
	char datestring[50];        //used to hold the date stamp for the log file
	time_t time_lastlog, time_tmp = 0, time_now;
	struct tm time_lastlog_tm, time_tm;
	int current_record, next_record, record_count;
	double temperature_in;
	double temperature_out;
	double dewpoint;
	double windchill;
	double pressure;
	int humidity_in;
	int humidity_out;
	double rain, current_rain;
	double prev_rain = 0;
	
	double windspeed;
	double windgust;
	double winddir_degrees;
	const char *directions[]= {"N","NNE","NE","ENE","E","ESE","SE","SSE",
	                           "S","SSW","SW","WSW","W","WNW","NW","NNW"};
  int minute;

	int temp_int1, temp_int2;
	int read_result;
	int i,j;

	if (argc < 2 || argc > 2)
	{
		print_usage();
	}	
	
	// Get serial port from config file

	get_configuration(&config, "");

    // Setup serial port

    // Get in-data and select mode.
	fileptr = fopen(argv[1], "ab+");
	if (fileptr == NULL)
	{
		printf("Cannot open file %s\n",argv[1]);
		exit(0);
	}
	
	fseek(fileptr, 1L, SEEK_END);
	counter = 60;
	do
	{
		counter++;
		if (fseek(fileptr, -counter, SEEK_END) < 0 )
			break;
		ch = getc(fileptr);
	} while (ch != '\n' && ch != '\r');
	if (fscanf(fileptr,"%4d%2d%2d%2d%2d", &temp_int1, &temp_int2,
	           &time_lastlog_tm.tm_mday, &time_lastlog_tm.tm_hour,
	           &time_lastlog_tm.tm_min) == 5)
	{
		time_lastlog_tm.tm_year = temp_int1 - 1900;
		time_lastlog_tm.tm_mon = temp_int2 - 1;	
		time_lastlog_tm.tm_sec = 0;
		time_lastlog_tm.tm_isdst = -1;
	}
	else
	{	//if no valid log we set the date to 1 Jan 1990 0:00
		time_lastlog_tm.tm_year = 1990; 
		time_lastlog_tm.tm_mon = 0;	
		time_lastlog_tm.tm_mday = 1;
		time_lastlog_tm.tm_hour = 0;
	  time_lastlog_tm.tm_min = 0;
		time_lastlog_tm.tm_sec = 0;
		time_lastlog_tm.tm_isdst = -1;
	}
	//printf("%4d,%2d,%2d,%2d,%2d\n", temp_int1, temp_int2,
	//           time_lastlog_tm.tm_mday, time_lastlog_tm.tm_hour,
	//           time_lastlog_tm.tm_min);
	
	time_lastlog = mktime(&time_lastlog_tm);
	i = 0;
	//time_now varialbe will be used to check correctness of time in history records
  time(&time_now);
  //add 10h margin for lower sensitivity of history time (history time may be max 10h higher than local computer time)
  time_now += 36000;
  do {
    ws = open_weatherstation(config.serial_device_name);
    read_result = read_safe(ws, HISTORY_BUFFER_ADR,0x7fff - HISTORY_BUFFER_ADR + 1,data, NULL);
    close_weatherstation(ws);
    i++;
    
    if (read_result != -1)
    {
      //check dates in history
      for (j = 0; j < HISTORY_REC_NO; j++)
      {
        read_history_record(data, j, &config,
  	                        &temperature_in,
  	                        &temperature_out,
  	                        &pressure,
  	                        &humidity_in,
  	                        &humidity_out,
  	                        &rain,
  	                        &windspeed,
  	                        &windgust,
  	                        &winddir_degrees,
  	                        &dewpoint,
  	                        &windchill,
                            &time_tm);
  
        if (time_tm.tm_min != 0xff)
        {
          time_tmp = mktime(&time_tm);
          if (time_tmp > time_now)
          {
            print_log(2,"wrong date in history");
            strftime(datestring,sizeof(datestring),"%Y%m%d%H%M%S %Y-%b-%d %H:%M:%S",
    		         &time_tm);
    		    print_log(2,datestring);
            
            break;
          }
        }
      }
    }
  } while (i < 4 && read_result == -1 && time_tmp <= time_now);		
  
  if (time_lastlog_tm.tm_year == 1990)
  {
    //find oldest record
    
    //go to the end of buffer (FF in minutes)
    next_record = 0;
    do
    {
      current_record = next_record;
      //printf("record1=%i\n",current_record);
      next_record = read_history_record(data, current_record, &config,
	                        &temperature_in,
	                        &temperature_out,
	                        &pressure,
	                        &humidity_in,
	                        &humidity_out,
	                        &rain,
	                        &windspeed,
	                        &windgust,
	                        &winddir_degrees,
	                        &dewpoint,
	                        &windchill,
                          &time_tm);
    } while (time_tm.tm_min < 60);
    /*if (current_record == 0)
    {
      read_history_record(data, 1796, &config,
	                        &temperature_in,
	                        &temperature_out,
	                        &pressure,
	                        &humidity_in,
	                        &humidity_out,
	                        &rain,
	                        &windspeed,
	                        &windgust,
	                        &winddir_degrees,
	                        &dewpoint,
	                        &windchill,
                          &time_tm);
    }*/
    
    //ommit FF's between end and begin of buffer (is only one FF byte for longer used station - with completely filled buffer)
    record_count = 0;
    do
    {
      //printf("record2=%i\n",current_record);
      current_record = next_record;
      next_record = read_history_record(data, current_record, &config,
	                        &temperature_in,
	                        &temperature_out,
	                        &pressure,
	                        &humidity_in,
	                        &humidity_out,
	                        &rain,
	                        &windspeed,
	                        &windgust,
	                        &winddir_degrees,
	                        &dewpoint,
	                        &windchill,
                          &time_tm);
      record_count++;  
    } while (time_tm.tm_min >= 60 && record_count < HISTORY_REC_NO + 1);
      //first log record will have rain = 0 (we don't know previous state of rain counter)
      if (time_tm.tm_min < 60)
          prev_rain = rain;
    //we are at the beginning of buffer now!
  } else
  {
    //find oldest new record
    //go to the end of buffer (FF in minutes)
    next_record = 0;
    do
    {
      //printf("record2=%i\n",current_record);
      current_record = next_record;
      next_record = read_history_record(data, current_record, &config,
	                        &temperature_in,
	                        &temperature_out,
	                        &pressure,
	                        &humidity_in,
	                        &humidity_out,
	                        &rain,
	                        &windspeed,
	                        &windgust,
	                        &winddir_degrees,
	                        &dewpoint,
	                        &windchill,
                          &time_tm);
    } while (time_tm.tm_min < 60 && next_record != 0);
    
    //ommit FF's between end and begin of buffer (is only one FF byte for longer used station - with completely filled buffer)
    do
    {
      //printf("record2=%i\n",current_record);
      current_record = next_record;
      next_record = read_history_record(data, current_record, &config,
	                        &temperature_in,
	                        &temperature_out,
	                        &pressure,
	                        &humidity_in,
	                        &humidity_out,
	                        &rain,
	                        &windspeed,
	                        &windgust,
	                        &winddir_degrees,
	                        &dewpoint,
	                        &windchill,
                          &time_tm);
    } while (time_tm.tm_min >= 60 && next_record != 0);
    
    //find first new record
    record_count = 0;  
    do
    {
      current_record = next_record;
      //printf("record3=%i\n",current_record);
      next_record = read_history_record(data, next_record, &config,
	                        &temperature_in,
	                        &temperature_out,
	                        &pressure,
	                        &humidity_in,
	                        &humidity_out,
	                        &rain,
	                        &windspeed,
	                        &windgust,
	                        &winddir_degrees,
	                        &dewpoint,
	                        &windchill,
                          &time_tm);

      if (time_tm.tm_min < 60)
      {
        time_tmp = mktime(&time_tm);
      }
      
      if (time_tm.tm_min < 60 && time_tmp <= time_lastlog)
      {
        prev_rain = rain;
      }
      
      if (record_count == 0)
        prev_rain = rain;
        
      record_count++; 
      
    } while (time_tmp <= time_lastlog && record_count < HISTORY_REC_NO + 1);
    
    if (time_tmp <= time_lastlog && record_count == HISTORY_REC_NO + 1)
    {
      printf("\nNew records not found\n");
      close_weatherstation(ws);
      fclose(fileptr);
	    exit(0);
    }
  }
	
	//next_record = current_record;
	next_record = current_record;
	if (record_count < HISTORY_REC_NO + 1)
	{
  	do
  	{ 
  	  //printf("record4=%i\n",next_record);
  		next_record = read_history_record(data, next_record, &config,
  	                        &temperature_in,
  	                        &temperature_out,
  	                        &pressure,
  	                        &humidity_in,
  	                        &humidity_out,
  	                        &rain,
  	                        &windspeed,
  	                        &windgust,
  	                        &winddir_degrees,
  	                        &dewpoint,
  	                        &windchill,
                            &time_tm);
  		
  		strcpy(logline,"");
  		
  		// READ TEMPERATURE INDOOR
  		
  		sprintf(logline,"%s%.1f ", logline, temperature_in);
  		
  	
  		// READ TEMPERATURE OUTDOOR
  	
  		sprintf(logline,"%s%.1f ", logline, temperature_out);
  		
  		
  		// READ DEWPOINT
  	
  		sprintf(logline,"%s%.1f ", logline, dewpoint);
  		
  		
  		// READ RELATIVE HUMIDITY INDOOR
  	
  		sprintf(logline,"%s%d ", logline, humidity_in);	
  		
  		
  		// READ RELATIVE HUMIDITY OUTDOOR
  	
  		sprintf(logline,"%s%d ", logline, humidity_out);	 
  	
  	
  		// READ WIND SPEED AND DIRECTION aND WINDCHILL
  	
  		sprintf(logline,"%s%.1f ", logline, windspeed);
  		sprintf(logline,"%s%.1f %s ", logline, winddir_degrees, directions[(int)(winddir_degrees/22.5)]);
  		
  		// READ GUST
  		sprintf(logline,"%s%.2f ", logline, windgust);
  	
  	
  		// READ WINDCHILL
  		
  		sprintf(logline,"%s%.1f ", logline, windchill);
  	
  		
  		// READ RAIN 1H
  	
  		//	sprintf(logline,"%s%.2f ", logline,
  		//        rain_1h(ws, config.rain_conv_factor));
  		//	
  		
  		// READ RAIN 24H
  	
  		//
  		//sprintf(logline,"%s%.2f ", logline,
  		//        rain_24h(ws, config.rain_conv_factor));
  		//
  		
  		// READ RAIN IN INTERVAL
  		if (rain >= prev_rain)
        current_rain = rain - prev_rain;
      else
        current_rain = rain + 4096 - prev_rain;
  		sprintf(logline,"%s%.2f ", logline, current_rain * 0.518);
  		//sprintf(logline,"%s%.2f ", logline, rain);
  		prev_rain = rain;
  		
  		// READ ABSOLUTE PRESSURE
  	
  		sprintf(logline,"%s%.3f ", logline, pressure);
  		
  		
  	
  	
  		// GET DATE AND TIME FOR LOG FILE, PLACE BEFORE ALL DATA IN LOG LINE
  		
  		//	printf("time now: %d\n",time(&basictime));
  		//	time_lastrecord_tm.tm_hour=time_last.hour;
  
      minute = time_tm.tm_min;
  		if (minute < 60)
  		{
        mktime(&time_tm);                 //normalize time_lastlog_tm
  		
  		  strftime(datestring,sizeof(datestring),"%Y%m%d%H%M%S %Y-%b-%d %H:%M:%S",
  		         &time_tm);
  		
  		
  		  // Print out
  		  fseek(fileptr, 0L, SEEK_END);
  		  fprintf(fileptr,"%s %s\n",datestring, logline);
  		  fflush(NULL);
  		}
  	
  	} while (minute < 60);
  }

	// Goodbye and Goodnight
	//close_weatherstation(ws);
	fclose(fileptr);

	return(0);
}

