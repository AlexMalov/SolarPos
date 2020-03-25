#include "SolarPos.h"
#include "DS3231M.h"

void SolarPos::initSolarCalc(int tzOffset, float lat, float lon){   // initSolar function
	_tzOffset = tzOffset;      // Set time zone offset
	_lat = lat;	               // Set current site latitude
	_lon = lon;	               // Set current site longitude
}

int SolarPos::gettzOffset(){   // Return time zone offset when user asks for it. Zones west of GMT are negative.
	return _tzOffset;
}

float SolarPos::getlat(){      // Return latitude when user asks for it.
	return _lat;
}

float SolarPos::getlon(){      // Return longitude when user asks for it.
	return _lon;
}

uint32_t SolarPos::getSolarNoonTime(uint32_t t){  // Extract Solar Noon Time (Time object, seconds since 1970-1-1)
	calcSolar(t);
	return _SolarNoonTime;
}

uint32_t SolarPos::getSunriseTime(uint32_t t){    // Extract Sunrise as Time object (seconds since 1970-1-1, local time zone)
	calcSolar(t);
	return _SunriseTime;
}

uint32_t SolarPos::getSunsetTime(uint32_t t){   // Extract Sunset as Time object (seconds since 1970-1-1, local time zone)
	calcSolar(t);
	return _SunsetTime;
}

float SolarPos::getSunDuration(uint32_t t){     // Extract Sunlight Duration (day length, minutes)
	calcSolar(t);
	return _SunDuration;
}

float SolarPos::getSEC_Corr(uint32_t t){        // Solar Elevation Corrected for Atmospheric refraction (degrees)
	calcSolar(t);
	return _SEC_Corr;
}

float SolarPos::getSAA(uint32_t t){             // Extract Solar Azimuth Angle (degrees clockwise from North)
	calcSolar(t);
	return _SAA;
}

// Main function to calculate solar values. Requires a time value (seconds since 1970-1-1) as input. 
void SolarPos::calcSolar(uint32_t t){
    if (_lastTime == t) return;         //  do'nt need to do it next time if t is the same
    _lastTime = t;
    DateTime dt1 = DateTime(t);
    // Calculate the time past midnight, as a fractional day value  e.g. if it's noon, the result should be 0.5.
	float timeFracDay = ((((dt1.second()/60.0) + dt1.minute())/60.0) + dt1.hour())/24.0;
    // unixDays is the number of whole days since the start of the Unix epoch. The division sign will truncate any remainder
    // since this will be done as integer division.
    long unixDays = t / 86400;
    // calculate Julian Day Number. Add the fractional day value to the Julian Day number. If the
    // input value was in the GMT time zone, we could proceed directly with this value. Adjust JDN to GMT time zone
    float offsetDay = _tzOffset / 24.0;
    float JDN = julianUnixEpoch + unixDays + timeFracDay - offsetDay;
    float JCN = (JDN - 2451545) / 36525.0;                              // Calculate Julian Century Number
    float GMLS = (280.46646 + JCN * (36000.76983 + JCN * 0.0003032));   // Geometric Mean Longitude of Sun (degrees)
    // Finish GMLS calculation by calculating modolu(GMLS,360) as it's done in R or Excel.
    // C's fmod doesn't work in the same way. The floor() function is from the math.h library.
    GMLS -= (360 * (floor(GMLS/360)) );
    float GMAS = 357.52911 + (JCN * (35999.05029 - 0.0001537 * JCN));       // Geometric Mean Anomaly of Sun (degrees)
    float EEO = 0.016708634 - (JCN * (0.000042037 + 0.0000001267 * JCN));   // Eccentricity of Earth Orbit
    // Sun Equation of Center
    float SEC = sin(GMAS * DEG_TO_RAD) * (1.914602 - (JCN * (0.004817 + 0.000014 * JCN))) +
        sin((2*GMAS)* DEG_TO_RAD)*(0.019993-0.000101*JCN) + sin((3*GMAS)* DEG_TO_RAD) * 0.000289;
    float STL = GMLS + SEC;                                                  // Sun True Longitude (degrees)
    // Sun Apparent Longitude (degrees)
    float SAL = STL - 0.00569 - (0.00478 * sin((125.04 - 1934.136 * JCN) * DEG_TO_RAD));
    // Mean Oblique Ecliptic (degrees)
    float MOE = 23 + (26 + (21.448-JCN * ( 46.815 + JCN * (0.00059 - JCN * 0.001813)))/60)/60;
    float OC = MOE + 0.00256 * cos((125.04-1934.136*JCN)*DEG_TO_RAD);       // Oblique correction (degrees)
    // Sun Declination (degrees)
    float SDec = (asin(sin(OC * DEG_TO_RAD) * sin(SAL * DEG_TO_RAD))) * RAD_TO_DEG;
    float vy = tan(((float)OC/2) * DEG_TO_RAD) * tan(((float)OC/2) * DEG_TO_RAD);   // var y
    // Equation of Time (minutes)
    float EOT = 4 * ((vy * sin(2 * (GMLS * DEG_TO_RAD)) - 2 * EEO * sin(GMAS * DEG_TO_RAD) +
                4 * EEO * vy * sin(GMAS * DEG_TO_RAD) * cos(2*(GMLS*DEG_TO_RAD)) -
                0.5 * vy * vy * sin(4*(GMLS * DEG_TO_RAD)) -
                1.25 * EEO * EEO * sin(2*(GMAS* DEG_TO_RAD))) *  RAD_TO_DEG);
    // Hour Angle Sunrise (degrees)
    float HAS = acos((cos(90.833*DEG_TO_RAD) / (cos(_lat*DEG_TO_RAD) * cos(SDec*DEG_TO_RAD))) -
               tan(_lat * DEG_TO_RAD) * tan(SDec * DEG_TO_RAD)) * RAD_TO_DEG ;
    // Solar Noon - result is given as fraction of a day. Time value is in GMT time zone
    float SolarNoonfrac = (720 - 4 * _lon - EOT) / 1440 ;
    // SolarNoon is given as a fraction of a day. Add this to the unixDays value, which currently holds the
    // whole days since 1970-1-1 00:00. SolarNoonDays is in GMT time zone, correct it to the input time zone
    _SolarNoonTime = (unixDays + SolarNoonfrac + offsetDay) * 86400;
    // Sunrise Time, given as fraction of a day. Convert Sunrise to days since 1970-1-1    
    // Correct Sunrise to local time zone from GMT
    _SunriseTime = (SolarNoonfrac - HAS * 4/1440 + unixDays + offsetDay) * 86400.0;
    // Convert Sunrise to seconds since 1970-1-1. Convert Sunrise to a uint32_t object (Time library)
    // Convert Sunset to days since 1970-1-1. Correct Sunset to local time zone from GMT
    _SunsetTime = (SolarNoonfrac + HAS * 4/1440 + unixDays + offsetDay) * 86400.0;
    // Convert Sunset to seconds since 1970-1-1. Convert Sunset to a uint32_t object (Time library)
    // Sunlight Duration (day length, minutes)
    _SunDuration = 8 * HAS;
    float TST = (timeFracDay * 1440 + EOT + 4 * _lon - 60 * _tzOffset);     // True Solar Time (minutes)
    // Finish TST calculation by calculating modolu(TST,360) as it's done in R or Excel. C's fmod doesn't work in the same
    // way. The floor() function is from the math.h library.
    TST -= (1440 * (floor(TST/1440)) );
    float HA;                   // Hour Angle (degrees)
    if (TST/4 < 0) {
        HA = TST/4 + 180;
    } else if (TST/4 >= 0) {
        HA = TST/4 - 180;
    }
    // Solar Zenith Angle (degrees)
    float SZA = (acos(sin(_lat * DEG_TO_RAD) * sin(SDec* DEG_TO_RAD) +
                cos(_lat * DEG_TO_RAD) * cos(SDec * DEG_TO_RAD) * cos(HA * DEG_TO_RAD))) * RAD_TO_DEG;
    // Solar Elevation Corrected for Atmospheric refraction (degrees)
    _SEC_Corr = 90 - SZA + 0.1;// AAR;
    // Solar Azimuth Angle (degrees clockwise from North)
    if (HA > 0) {
        _SAA = (((acos((sin(_lat * DEG_TO_RAD) * cos(SZA * DEG_TO_RAD) -
                       sin(SDec * DEG_TO_RAD)) / (cos(_lat * DEG_TO_RAD) *
                       sin(SZA * DEG_TO_RAD))) ) * RAD_TO_DEG) + 180);
    } else {
        _SAA = (540 - (acos((((sin(_lat * DEG_TO_RAD) * cos(SZA * DEG_TO_RAD))) -
                            sin(SDec * DEG_TO_RAD)) / (cos(_lat * DEG_TO_RAD) *
                            sin(SZA * DEG_TO_RAD)))) * RAD_TO_DEG);
    }
    _SAA -= (360 * (floor(_SAA/360)));

}