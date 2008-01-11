# Database : `weather`
#
# open3600.conf settings to be changed:
# -----------------------------------
# MYSQL_HOST              localhost         # Localhost or IP address/hostname
# MYSQL_USERNAME          db_user           # Name of the MySQL user that has access to the database
# MYSQL_PASSWORD          db_pass           # Password for the MySQL user
# MYSQL_DATABASE          db_name           # Name of your database
# MYSQL_PORT              0                 # TCP/IP Port number. Zero means default
# -----------------------------------
#
# Table structure for table `weatherinfo`
#
# usage: mysql -udb_name -pdb_password < mysql3600.sql

DROP DATABASE IF EXISTS weather;
CREATE DATABASE weather;
use weather;

CREATE TABLE `data` (
  `timestamp` bigint(14) NOT NULL default '0',
  `rec_date` char NOT NULL default '00-00-0000',
  `rec_time` time NOT NULL default '00:00:00',
  `temp_in` decimal(3,1) NOT NULL default '0.0',
  `temp_out` decimal(3,1) NOT NULL default '0.0',
  `dewpoint` decimal(3,1) NOT NULL default '0.0',
  `rel_hum_in` tinyint(3) NOT NULL default '0',
  `rel_hum_out` tinyint(3) NOT NULL default '0',
  `windspeed` decimal(3,1) NOT NULL default '0.0',
  `wind_direction` char(3) NOT NULL default '',
  `wind_angle` decimal(3,1) NOT NULL default '0.0',
  `wind_chill` decimal(3,1) NOT NULL default '0.0',
  `rain_1h` decimal(5,1) NOT NULL default '0.0',
  `rain_24h` decimal(5,1) NOT NULL default '0.0',
  `rain_1w` decimal(6,1) NOT NULL default '0.0',
  `rain_1m` decimal(6,1) NOT NULL default '0.0',
  `rain_total` decimal(7,1) NOT NULL default '0.0',
  `rel_pressure` decimal(7,1) NOT NULL default '0.0',
  `abs_pressure` decimal(7,1) NOT NULL default '0.0',
  `tendency` varchar(10) NOT NULL default '',
  `forecast` varchar(7) NOT NULL default '',
  UNIQUE KEY `timestamp` (`timestamp`)
) TYPE=MyISAM;
