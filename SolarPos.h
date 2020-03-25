/* SolarPos.h Alex Malov 2020
 * Based on Solarlib.h		Luke Miller December 2012
 * No warranty given or implied.
 * 
 * A library of functions for Arduino to calculate aspects of solar position
 * in the sky using a time value, latitude, longitude, and time zone.
 * Output includes estimates of current sun elevation and azimuth (position in 
 * the sky), sunrise, solar noon, and sunset times for the current day. Results should be
 * accurate for the years 1901 to 2099, for locations between +/- 72 latitude.
 * Calculations are based on spreadsheet and information found at:
 * http://www.esrl.noaa.gov/gmd/grad/solcalc/calcdetails.html
 * 
 * Initialize the solar calculator using the initSolarCalc() function, which
 * lets you specify:
 * tzOffset 	- time zone Offset from Greenwich Mean Time (UTC). Time zones 
 * 				west of GMT should be given negative values. 
 * 				For example, Pacific Standard Time is -8 
 * 				
 * lat			- Latitude of the location you want to use. Values north of the
 * 				equator are positive, given in decimal degrees.
 * 				 				
 * lon			- Longitude of the location you want to use. Values in the 
 * 				western hemisphere have negative values (0 to -180), given in 
 * 				decimal degrees. 
 * 				For example: Monterey, California has lat/lon (36.62, -121.904)
 * 				
 * Each extractor function requires a Time value as input, from the Time 
 * library. The Time is given as seconds since 1970-1-1 00:00 (unix epoch).
 * Useful functions, supplied with a time value t as the sole argument:
 * 		getSAA(t)		- Solar Azimuth (degrees clockwise from North)
 * 		getSEC_Corr(t)	- Solar Elevation, corrected for diffraction (degrees)
 * 		getSZA(t)		- Solar Zenith angle (degrees below straight up)
 * 		getSunriseTime(t)	- Sunrise Time for the current day 	(Time object)
 * 		getSunsetTime(t)	- Sunset Time for the current day	(Time object)
 * 		getSolarNoonTime(t)	- Solar Noon for the current day	(Time object)
 * 		getSunDuration(t)	- Minutes of Sunlight for the current day	
 *
 * You can check your results against the NOAA calculator:
 * http://www.esrl.noaa.gov/gmd/grad/solcalc/
*/

#ifndef SolarPos_h
#define SolarPos_h

#include "Arduino.h"

#define julianUnixEpoch  2440587.5 // julian days to start of unix epoch


class SolarPos
{
  private:
    int _tzOffset;               // Time zone Offset, zones west of GMT are negative
    float _lat;                  // Latitude of site, values north of equator are positive
    float _lon;                  // Longitude of site, values west of GMT are negative
    uint32_t _SolarNoonTime;     // Solar Noon time (unix Time)
    uint32_t _SunriseTime;       // Sunrise time (unix Time)
    uint32_t _SunsetTime;        // Sunset time (unix Time)
    float _SunDuration;          // Sunlight Duration (minutes)
    float _SEC_Corr;             // Solar Elevation, Corrected (degrees)
    float _SAA;                  // Solar Azimuth Angle (degrees)
    uint32_t _lastTime;          // время последнего вычисления
    void calcSolar(uint32_t t);  // Main function to update the contents of the Solar parameters using the given Time t input.

  public:
    // Initialization function to put time zone offset, latitude, and longitude in
    SolarPos(int tzOffset, float lat, float lon) { initSolarCalc(tzOffset, lat, lon); }
    void initSolarCalc(int tzOffset, float lat, float lon);
    float getlat();                     // Return latitude used in solar calculations
    float getlon();                     // Return longitude used in solar calculations
    int gettzOffset();                  // Return time zone offset. Zones west of GMT are negative
    uint32_t getSolarNoonTime(uint32_t t);  // Extract Solar Noon Time (Time object, seconds since 1970-1-1)
    uint32_t getSunriseTime(uint32_t t);    // Extract Sunrise as Time object (seconds since 1970-1-1, local time zone)
    uint32_t getSunsetTime(uint32_t t);     // Extract Sunset as Time object (seconds since 1970-1-1, local time zone)
    float getSunDuration(uint32_t t);     // Extract Sunlight Duration (day length, minutes)
    float getSEC_Corr(uint32_t t);        // Extract Solar Elevation Corrected for Atmospheric refraction (degrees)
    float getSAA(uint32_t t);             // Extract Solar Azimuth Angle (degrees clockwise from North)
};

#endif