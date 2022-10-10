/*
Example script reading measurement values from the EE894 sensor via I2C interface.

Copyright 2022 E+E Elektronik Ges.m.b.H.

Disclaimer:
This application example is non-binding and does not claim to be complete with regard
to configuration and equipment as well as all eventualities. The application example
is intended to provide assistance with the EE894 sensor module design-in and is provided "as is".
You yourself are responsible for the proper operation of the products described.
This application example does not release you from the obligation to handle the product safely
during application, installation, operation and maintenance. By using this application example,
you acknowledge that we cannot be held liable for any damage beyond the liability regulations
described.

We reserve the right to make changes to this application example at any time without notice.
In case of discrepancies between the suggestions in this application example and other E+E
publications, such as catalogues, the content of the other documentation takes precedence.
We assume no liability for the information contained in this document.
*/

#include <Wire.h>
#include <ee894I2c.h>
ee894I2c ee;

#define REQUEST_INTERVAL_MS 15000
#define CSV_DELIMITER ','

char Sensorname[16];
float temperature, humidity, co2Aver, co2Raw, pressure, measuringInterval;
int offset;
char errorString[200];
uint8_t errorcode, dayCAM, monthCAM, yearCAM;
uint16_t gain, lowerLimit, upperLimit;



void setup()
{
  Serial.begin(9600);
  Wire.begin();                // initialize I2C peripheral (SDA..A4, SCL..A5)
  ee.changeSensorname("best CO2 sensor!");
  ee.readSensorname(Sensorname);
  Serial.print("Sensorname:");
  Serial.println(Sensorname);
  errorcode = ee.changeCAMDate(2, 24, 12, 18);
  if (errorcode != 0)
  {
    ee.getErrorString(errorcode, errorString);
    Serial.println("CAM Date:");
    Serial.println(errorString);
  }
  ee.readCAMDate(2, dayCAM, monthCAM, yearCAM);
  Serial.print(dayCAM);
  Serial.print(monthCAM);
  Serial.println(yearCAM);
  /*errorcode = ee.changeCAM(2, 0, 32768, 0, 65535);
  if (errorcode != 0)
  {
    ee.getErrorString(errorcode, errorString);
    Serial.println("CAM:");
    Serial.println(errorString);
  }*/
  ee.readCAM(2, offset, gain, lowerLimit, upperLimit);
  Serial.print("offste: ");
  Serial.print(offset);
  Serial.print(" gain: ");
  Serial.print(gain);
  Serial.print(" lowerLimit: ");
  Serial.print(lowerLimit);
  Serial.print(" upperLimit: ");
  Serial.println(upperLimit);
  errorcode = ee.changeCo2MeasuringInterval(150);// in 100ms steps 
  if (errorcode != 0)
  {
    ee.getErrorString(errorcode, errorString);
    Serial.println("Measuring interval:");
    Serial.println(errorString);
  }
  ee.readCo2MeasuringInterval(measuringInterval);
  Serial.print("Measuring Interval:");
  Serial.println(measuringInterval);
  Serial.print("temperature"); // print CSV header
  Serial.print(CSV_DELIMITER);
  Serial.print("humidity");
  Serial.print(CSV_DELIMITER);
  Serial.print("co2 Average");
  Serial.print(CSV_DELIMITER);
  Serial.print("co2 Raw");
  Serial.print(CSV_DELIMITER);
  Serial.println("pressure");
}

void loop()
{
  errorcode = ee.getTempHum(temperature, humidity);
  if (errorcode != 0)
  {
    ee.getErrorString(errorcode, errorString);
    Serial.println(errorString);
  }
  else
  {
    Serial.print(temperature);
    Serial.print(" °C");
    Serial.print(CSV_DELIMITER);
    Serial.print(humidity);
    Serial.print(" %RH");
    Serial.print(CSV_DELIMITER);
  }
  errorcode = ee.getCo2AverCo2RawPressure(co2Aver, co2Raw, pressure);
  if (errorcode != 0)
  {
    ee.getErrorString(errorcode, errorString);
    Serial.println(errorString);
  }
  else
  {
    Serial.print(co2Aver);
    Serial.print(" ppm");
    Serial.print(co2Raw);
    Serial.print(" ppm");
    Serial.print(pressure);
    Serial.println(" mbar");
  }
  delay(REQUEST_INTERVAL_MS);
}
