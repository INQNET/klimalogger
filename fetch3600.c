/*  open3600 - fetch3600.c
 *
 *  Version 0.01
 *
 *  Control WS3600 weather station
 *
 *  Copyright 2003, Kenneth Lavrsen
 *  Copyright 2005, Grzegorz Wisniewski, Sander Eerkes
 *  This program is published under the GNU General Public license
 */

#include "rw3600.h"

 
/********** MAIN PROGRAM ************************************************
 *
 * This program reads all current and min/max data from a WS3600
 * weather station and write it to standard out. This is the program that
 * the program weatherstation.php uses to display a nice webpage with
 * current weather data.
 *
 * It takes one parameter which is the config file name with path
 * If this parameter is omitted the program will look at the default paths
 * See the open3600.conf file for info
 *
 ***********************************************************************/
int main(int argc, char *argv[])
{
	WEATHERSTATION ws;
	unsigned char logline[3000] = "";
	char datestring[50];     //used to hold the date stamp for the log file
	const char *directions[]= {"N","NNE","NE","ENE","E","ESE","SE","SSE",
	                           "S","SSW","SW","WSW","W","WNW","NW","NNW"};
	double winddir[6];
	char tendency[15];
	char forecast[15];
	double tempfloat_min, tempfloat_max;
	int tempint_min, tempint_max;
	struct timestamp time_min, time_max;
	time_t basictime;
	unsigned char data[32768];
	int read_result;
	int i;

	get_configuration(&config, argv[1]);



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

	sprintf(logline, "%sTi %.1f\n", logline,
	        temperature_indoor(data));

	temperature_indoor_minmax(data,&tempfloat_min,
	                          &tempfloat_max, &time_min, &time_max);
	
	sprintf(logline, "%sTimin %.1f\n", logline, tempfloat_min);
	sprintf(logline, "%sTimax %.1f\n", logline, tempfloat_max);
	sprintf(logline,"%sTTimin %02d:%02d\nDTimin %02d-%02d-%04d\n", logline,
	        time_min.hour, time_min.minute, time_min.day,
	        time_min.month, time_min.year);
	sprintf(logline,"%sTTimax %02d:%02d\nDTimax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);
	
	
	// READ TEMPERATURE OUTDOOR

	sprintf(logline, "%sTo %.1f\n", logline,
	        temperature_outdoor(data));

	temperature_outdoor_minmax(data,&tempfloat_min,
	                           &tempfloat_max, &time_min, &time_max);
	
	sprintf(logline, "%sTomin %.1f\n", logline, tempfloat_min);
	sprintf(logline, "%sTomax %.1f\n", logline, tempfloat_max);
	sprintf(logline,"%sTTomin %02d:%02d\nDTomin %02d-%02d-%04d\n", logline,
	        time_min.hour, time_min.minute, time_min.day,
	        time_min.month, time_min.year);
	sprintf(logline,"%sTTomax %02d:%02d\nDTomax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);

	// READ DEWPOINT

	sprintf(logline, "%sDP %.1f\n", logline,
	        dewpoint(data) );

	dewpoint_minmax(data,&tempfloat_min,
	                &tempfloat_max, &time_min, &time_max);
	
	sprintf(logline, "%sDPmin %.1f\n", logline, tempfloat_min);
	sprintf(logline, "%sDPmax %.1f\n", logline, tempfloat_max);
	sprintf(logline,"%sTDPmin %02d:%02d\nDDPmin %02d-%02d-%04d\n", logline,
	        time_min.hour, time_min.minute, time_min.day,
	        time_min.month, time_min.year);
	sprintf(logline,"%sTDPmax %02d:%02d\nDDPmax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);
	
	
	// READ RELATIVE HUMIDITY INDOOR

	sprintf(logline, "%sRHi %d\n", logline,
		humidity_indoor(data));

  humidity_indoor_minmax(data, &tempint_min, &tempint_max,
		                    &time_min, &time_max);
	
	sprintf(logline, "%sRHimin %d\n", logline, tempint_min);
	sprintf(logline, "%sRHimax %d\n", logline, tempint_max);
	sprintf(logline,"%sTRHimin %02d:%02d\nDRHimin %02d-%02d-%04d\n", logline,
	        time_min.hour, time_min.minute, time_min.day,
	        time_min.month, time_min.year);
	sprintf(logline,"%sTRHimax %02d:%02d\nDRHimax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);

	
	// READ RELATIVE HUMIDITY OUTDOOR
	sprintf(logline, "%sRHo %d\n", logline,
		humidity_outdoor(data));


	humidity_outdoor_minmax(data, &tempint_min, &tempint_max,
		                         &time_min, &time_max);
	
	sprintf(logline, "%sRHomin %d\n", logline, tempint_min);
	sprintf(logline, "%sRHomax %d\n", logline, tempint_max);
	sprintf(logline,"%sTRHomin %02d:%02d\nDRHomin %02d-%02d-%04d\n", logline,
	        time_min.hour, time_min.minute, time_min.day,
	        time_min.month, time_min.year);
	sprintf(logline,"%sTRHomax %02d:%02d\nDRHomax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);


	// READ WIND SPEED AND DIRECTION
	
	sprintf(logline,"%sWS %.1f\n", logline,
	       wind_current(data, winddir));
	sprintf(logline,"%sDIRtext %s\nDIR0 %.1f\nDIR1 %0.1f\n"
	        "DIR2 %0.1f\nDIR3 %0.1f\nDIR4 %0.1f\nDIR5 %0.1f\n",
			logline, directions[(int)(winddir[0]/22.5)], winddir[0], winddir[1],
			winddir[2], winddir[3], winddir[4], winddir[5]);
			
	// WINDCHILL
	sprintf(logline, "%sWC %.1f\n", logline,
	        windchill(data));
	
	windchill_minmax(data,&tempfloat_min,
	                 &tempfloat_max, &time_min, &time_max); 

	sprintf(logline, "%sWCmin %.1f\n", logline, tempfloat_min);
	sprintf(logline, "%sWCmax %.1f\n", logline, tempfloat_max);

	sprintf(logline,"%sTWCmin %02d:%02d\nDWCmin %02d-%02d-%04d\n", logline,
	        time_min.hour, time_min.minute, time_min.day,
	        time_min.month, time_min.year);
	sprintf(logline,"%sTWCmax %02d:%02d\nDWCmax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);
	        

	// READ WINDSPEED MIN/MAX

	wind_minmax(data, &tempfloat_min,
	            &tempfloat_max, &time_min, &time_max);

	sprintf(logline, "%sWSmin %.1f\n", logline, tempfloat_min);
	sprintf(logline, "%sWSmax %.1f\n", logline, tempfloat_max);

	sprintf(logline,"%sTWSmin %02d:%02d\nDWSmin %02d-%02d-%04d\n", logline,
	        time_min.hour, time_min.minute, time_min.day,
	        time_min.month, time_min.year);
	sprintf(logline,"%sTWSmax %02d:%02d\nDWSmax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);

	
	// READ RAIN 1H

  sprintf(logline, "%sR1h %.1f\n", logline,
		rain_1h(data));
		
	rain_1h_max(data,&tempfloat_max, &time_max);
	sprintf(logline,"%sR1hmax %.1f\n", logline, tempfloat_max);
	sprintf(logline,"%sTR1hmax %02d:%02d\nDR1hmax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);


	// READ RAIN 24H

  sprintf(logline, "%sR24h %.1f\n", logline,
		rain_24h(data));
  rain_24h_max(data,&tempfloat_max, &time_max);
	sprintf(logline,"%sR24hmax %.1f\n", logline, tempfloat_max);
	sprintf(logline,"%sTR24hmax %02d:%02d\nDR24hmax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);
	        
	// READ RAIN 1W

  sprintf(logline, "%sR1w %.1f\n", logline,
		rain_1w(data));
  rain_1w_max(data,&tempfloat_max, &time_max);
	sprintf(logline,"%sR1wmax %.1f\n", logline, tempfloat_max);
	sprintf(logline,"%sTR1wmax %02d:%02d\nDR1wmax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);
	        
	// READ RAIN 1M

  sprintf(logline, "%sR1m %.1f\n", logline,
		rain_1m(data));
  rain_1m_max(data,&tempfloat_max, &time_max);
	sprintf(logline,"%sR1mmax %.1f\n", logline, tempfloat_max);
	sprintf(logline,"%sTR1mmax %02d:%02d\nDR1mmax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);

	
	// READ RAIN TOTAL
	
	sprintf(logline,"%sRtot %.1f\n", logline,
	        rain_total(data));

  rain_total_time(data, &time_max);

	sprintf(logline,"%sTRtot %02d:%02d\nDRtot %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);


	// READ RELATIVE PRESSURE
	
	sprintf(logline,"%sRP %.1f\n", logline,
	        rel_pressure(data));
	        
	// READ ABSOLUTE PRESSURE

	sprintf(logline,"%sAP %.1f\n", logline,
	        abs_pressure(data));

	
	// RELATIVE PRESSURE MIN/MAX

	rel_pressure_minmax(data, &tempfloat_min,
	                    &tempfloat_max, &time_min, &time_max);

	sprintf(logline, "%sRPmin %.1f\n", logline, tempfloat_min);
	sprintf(logline, "%sRPmax %.1f\n", logline, tempfloat_max);

	sprintf(logline,"%sTRPmin %02d:%02d\nDRPmin %02d-%02d-%04d\n", logline,
	        time_min.hour, time_min.minute, time_min.day,
	        time_min.month, time_min.year);
	sprintf(logline,"%sTRPmax %02d:%02d\nDRPmax %02d-%02d-%04d\n", logline,
	        time_max.hour, time_max.minute, time_max.day,
	        time_max.month, time_max.year);


	// READ TENDENCY AND FORECAST
	
        tendency_forecast(data, tendency, forecast);
        sprintf(logline, "%sTendency %s\nForecast %s\n", logline, tendency, forecast);

	// GET DATE AND TIME FOR LOG FILE, PLACE BEFORE ALL DATA IN LOG LINE
	
	time(&basictime);
	strftime(datestring,sizeof(datestring),"Date %d-%b-%Y\nTime %H:%M:%S\n",
	         localtime(&basictime));

	// Print out and leave

	printf("%s%s",datestring, logline);

	//close_weatherstation(ws);

	return(0);
}

