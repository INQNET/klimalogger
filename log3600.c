/*  open3600 - log3600.c
 *  
 *  Version 1.10
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
	printf("log3600 - Read and interpret data from WS-3600 weather station\n");
	printf("and write it to a log file. Perfect for a cron driven task.\n");
	printf("(C)2003 Kenneth Lavrsen.\n");
	printf("(C)2005 Grzegorz Wisniewski,Sander Eerkes. (Version alfa)\n");
	printf("This program is released under the GNU General Public License (GPL)\n\n");
	printf("Usage:\n");
	printf("Save current data to logfile:  log3600 filename config_filename\n");
	exit(0);
}
 
/********** MAIN PROGRAM ************************************************
 *
 * This program reads current weather data from a WS3600
 * and writes the data to a log file.
 *
 * Log file format:
 * Timestamp Date Time Ti To DP RHi RHo Wind Dir-degree Dir-text WC
 *              Rain1h Rain24h Rain-tot Rel-Press Tendency Forecast
 *
 * Just run the program without parameters for usage.
 *
 * It takes two parameters. The first is the log filename with path
 * The second is the config file name with path
 * If this parameter is omitted the program will look at the default paths
 * See the open3600.conf file for info
 *
 ***********************************************************************/
int main(int argc, char *argv[])
{
	WEATHERSTATION ws;
	FILE *fileptr;
	unsigned char logline[3000] = "";
	char datestring[50];        //used to hold the date stamp for the log file
	const char *directions[]= {"N","NNE","NE","ENE","E","ESE","SE","SSE",
	                           "S","SSW","SW","WSW","W","WNW","NW","NNW"};
	double winddir[6];
	char tendency[15];
	char forecast[15];
	time_t basictime;
	unsigned char data[32768];
	int read_result;
	int i;

	get_configuration(&config, argv[2]);

   /* Get log filename. */

	if (argc < 2 || argc > 3)
	{
		print_usage();
	}			

	fileptr = fopen(argv[1], "a+");
	if (fileptr == NULL)
	{
		printf("Cannot open file %s\n",argv[1]);
		exit(-1);
	}
  
  i = 0;
  do {
    ws = open_weatherstation(config.serial_device_name);
    read_result = read_safe(ws, 0, HISTORY_BUFFER_ADR,data, NULL);
    close_weatherstation(ws);
    i++;
  } while (i < 4 && read_result == -1);
  
  if (read_result == -1)
	{
    printf("\nError reading data\n");
	  exit(0);
  }

	/* READ TEMPERATURE INDOOR */
	
	sprintf(logline,"%s%.1f ", logline,
	        temperature_indoor(data));
	

	/* READ TEMPERATURE OUTDOOR */

	sprintf(logline,"%s%.1f ", logline,
	        temperature_outdoor(data));
	
	
	/* READ DEWPOINT */

	sprintf(logline,"%s%.1f ", logline,
	        dewpoint(data));
	
	
	/* READ RELATIVE HUMIDITY INDOOR */

	sprintf(logline,"%s%d ", logline, humidity_indoor(data));	
	
	
	/* READ RELATIVE HUMIDITY OUTDOOR */

	sprintf(logline,"%s%d ", logline, humidity_outdoor(data));	 


	/* READ WIND SPEED AND DIRECTION */

  sprintf(logline,"%s%.1f ", logline,
	       wind_current(data, winddir));
	sprintf(logline,"%s%.1f %s ", logline, winddir[0], directions[(int)(winddir[0]/22.5)]);


	/* READ WINDCHILL */
	
	sprintf(logline,"%s%.1f ", logline,
	        windchill(data));

	
	/* READ RAIN 1H */
	sprintf(logline,"%s%.2f ", logline,
	        rain_1h(data));
	
	/* READ RAIN 24H */

	sprintf(logline,"%s%.2f ", logline,
	        rain_24h(data));

  	// READ RAIN 1W

  sprintf(logline, "%s%.2f ", logline,
		rain_1w(data));
	        
	// READ RAIN 1M

  sprintf(logline, "%s%.2f ", logline,
		rain_1m(data));
	
	/* READ RAIN TOTAL */
	
	sprintf(logline,"%s%.2f ", logline,
	        rain_total(data));

	
	/* READ RELATIVE PRESSURE */

	sprintf(logline,"%s%.3f ", logline,
	        rel_pressure(data));
	

	/* READ TENDENCY AND FORECAST */
	
	tendency_forecast(data, tendency, forecast);
	sprintf(logline,"%s%s %s", logline, tendency, forecast);


	/* GET DATE AND TIME FOR LOG FILE, PLACE BEFORE ALL DATA IN LOG LINE */
	
	time(&basictime);
	strftime(datestring,sizeof(datestring),"%Y%m%d%H%M%S %Y-%b-%d %H:%M:%S",
	         localtime(&basictime));

	// Print out and leave

	// printf("%s %s\n",datestring, logline); //disabled to be used in cron job
	fprintf(fileptr,"%s %s\n",datestring, logline);

//	close_weatherstation(ws);
	
	fclose(fileptr);

	exit(0);
	
	return 0;
}

