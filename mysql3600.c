/*       mysql3600.c
 *
 *       Copyright 2003,2004, Kenneth Lavrsen/Thomas Grieder
 *       Copyright 2005, Grzegorz Wisniewski, Sander Eerkes
 *
 */

#include <mysql.h>
#include "rw3600.h"

 
/********** MAIN PROGRAM ************************************************
 *
 * This program reads current weather data from a WS3600
 * and writes the data to a MySQL database.
 * The open3600.conf config file must contain the following parameters
 *
 * MYSQL_HOST              localhost         # Localhost or IP address/hostname
 * MYSQL_USERNAME          db_user           # Name of the MySQL user that has access to the database
 * MYSQL_PASSWORD          db_pass           # Password for the MySQL user
 * MYSQL_DATABASE          db_name           # Name of your database
 * MYSQL_PORT              0                 # TCP/IP Port number. Zero means default
 *
 * It takes one parameters. The config file name with path
 * If this parameter is omitted the program will look at the default paths
 * See the open3600.conf file for info
 *
 ***********************************************************************/

int main(int argc, char *argv[])
{
        WEATHERSTATION ws;
        MYSQL mysql;
        unsigned char logline[3000] = "";
        char datestring[50];     //used to hold the date stamp for the log file
        const char *directions[]= {"N","NNE","NE","ENE","E","ESE","SE","SSE","S","SSW","SW","WSW","W","WNW","NW","NNW"};
        double winddir[6];
        char tendency[15];
        char forecast[15];
//        double tempfloat_min, tempfloat_max;
//        int tempint_min, tempint_max;
        struct timestamp;
        time_t basictime;
        unsigned char data[32768];
        char query[4096];
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

	// READ TEMPERATURE INDOOR
	
        sprintf(logline,"%s\'%.1f\',", logline,
	        temperature_indoor(data));

	// READ TEMPERATURE OUTDOOR

        sprintf(logline,"%s\'%.1f\',", logline,
	        temperature_outdoor(data));

	// READ DEWPOINT

        sprintf(logline,"%s\'%.1f\',", logline,
	        dewpoint(data) );

	// READ RELATIVE HUMIDITY INDOOR

	sprintf(logline,"%s\'%d\',", logline,
		humidity_indoor(data));

	// READ RELATIVE HUMIDITY OUTDOOR
	sprintf(logline,"%s\'%d\',", logline,
		humidity_outdoor(data));

	// READ WIND SPEED
	
        sprintf(logline,"%s\'%.1f\',", logline,
	       wind_current(data, winddir));

	// READ ANGLE AND DIRECTION

        sprintf(logline,"%s\'%s\','%.1f\',", logline,
               directions[(int)(winddir[0]/22.5)], winddir[0]);
			
	// READ WINDCHILL

        sprintf(logline,"%s\'%.1f\',", logline,
	        windchill(data));
	
	// READ RAIN 1H

        sprintf(logline,"%s\'%.2f\',", logline,
		rain_1h(data));
		
	// READ RAIN 24H

        sprintf(logline,"%s\'%.2f\',", logline,
		rain_24h(data));
	        
	// READ RAIN 1W

        sprintf(logline,"%s\'%.2f\',", logline,
		rain_1w(data));
	        
	// READ RAIN 1M

        sprintf(logline,"%s\'%.2f\',", logline,
		rain_1m(data));
	
	// READ RAIN TOTAL
	
        sprintf(logline,"%s\'%.2f\',", logline,
	        rain_total(data));

	// READ RELATIVE PRESSURE
	
        sprintf(logline,"%s\'%.2f\',", logline,
	        rel_pressure(data));
	        
	// READ ABSOLUTE PRESSURE

        sprintf(logline,"%s\'%.2f\',", logline,
	        abs_pressure(data));
	
	// READ TENDENCY AND FORECAST
	
        tendency_forecast(data, tendency, forecast);
        sprintf(logline,"%s\'%s\',\'%s\'", logline, tendency, forecast);

	// GET DATE AND TIME FOR LOG FILE, PLACE BEFORE ALL DATA IN LOG LINE
	
	time(&basictime);
        strftime(datestring,sizeof(datestring),"\'%Y%m%d%H%M%S\',\'%Y-%m-%d\',\'%H:%M:%S\'",
	         localtime(&basictime));

	// Print out and leave

	// printf("%s%s",datestring, logline);

        // printf("%s %s\n",datestring, logline);  //disabled to be used in cron job
        /* INIT MYSQL AND CONNECT */
        if(!mysql_init(&mysql))
        {
        fprintf(stderr, "Cannot initialize MySQL");
        exit(0);
        }

        if(!mysql_real_connect(&mysql, config.mysql_host, config.mysql_user,
                               config.mysql_passwd, config.mysql_database,
                               config.mysql_port, NULL, 0))
        {
                fprintf(stderr, "%d: %s \n",
                mysql_errno(&mysql), mysql_error(&mysql));
                exit(0);
        }

        sprintf(query, "INSERT INTO weatherinfo VALUES (%s,%s)", datestring, logline);

        if(mysql_query(&mysql, query))
        {
                fprintf(stderr, "Could not insert row. %s %d: \%s \n", query, mysql_errno(&mysql), mysql_error(&mysql));
                mysql_close(&mysql);
                exit(0);
        }

        mysql_close(&mysql);


	//close_weatherstation(ws);

	return(0);
}
