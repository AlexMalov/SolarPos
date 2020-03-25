/* SolarPos_example
  This sketch illustrates the basic use of the SolarPos library.
  You must also have the DS3231M.h library installed for RTC clock device. 
  This sketch will output a series of values and times to the
  Serial monitor to demonstrate the usage of the SolarPos 
  calculator object. 
   
  The output can be checked against the NOAA calculator here:
  http://www.esrl.noaa.gov/gmd/grad/solcalc/
*/

#define timeZoneOffset 3              //часовой пояс
#define mylat 54.2192               // широта
#define mylon 37.6974               //долгота

#include "SolarPos.h"               //моя либа
#include "DS3231M.h"                //библиотека часов реального времени

DateTime now1;                    //текущее время
SolarPos sp1(timeZoneOffset, mylat, mylon);     //объект SolarPos

void setup() {
  Serial.begin(115200);
  now1 = DateTime(2020,3,4,12,47,0);
}

void loop() {
  float angle = sp1.getSEC_Corr( now1.unixtime() );
  float azimuth = sp1.getSAA( now1.unixtime() );
  Serial.print("Time: ");
  printDateTime(now1);
  Serial.print("Elevation: ");
  Serial.print(angle);
  Serial.print("Azimuth: ");
  Serial.print(azimuth);

  Serial.print("Sunrise: ");
  printDateTime(sp1.getSunriseTime(now1.unixtime()));
  Serial.print("Sunset: ");
  printDateTime(sp1.getSunsetTime(now1.unixtime()));
  Serial.print("Day length, minutes: ");
  Serial.println(sp1.getSunDuration(now1.unixtime()));
  Serial.println();
  now1 = now1 + TimeSpan(0,1,0,0); //увеличиваем текущее время на час
  delay(1000);
}

void printDateTime(DateTime t){       // Utility function to print time and date
  Serial.print(t.year());
  printDateDigits(t.month());
  printDateDigits(t.day());
  Serial.print(" ");
  Serial.print(t.hour());
  printDigits(t.minute());
  printDigits(t.second());
  Serial.println();
}

void printDateDigits(int digits){   // Utility function to print month/day digits nicely
  Serial.print("-");
  if(digits < 10) Serial.print("0");
  Serial.print(digits); 
}

void printDigits(int digits){     // Utility function for time value printing
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}