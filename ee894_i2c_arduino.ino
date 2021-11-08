/*
Example script reading measurement values from the EE894 sensor via I2C interface.

Copyright 2021 E+E Elektronik Ges.m.b.H.

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

#define REQUEST_INTERVAL_MS     25000 								//Definition of measurement interval in ms - Minimum time: 15000 ms
#define CSV_DELIMITER          	','
#define CRC8_ONEWIRE_POLY 		0x31
#define CRC8_ONEWIRE_START 		0xff


unsigned char i2cResponse[9];
int i = 0;
float co2Raw, co2Average, temperature, pressure, humidity;


static unsigned char calcCrc8 (unsigned char buf[], unsigned char from, unsigned char to)
{
  unsigned char crcVal = CRC8_ONEWIRE_START;
  unsigned char i = 0;
  unsigned char j = 0;
  for (i = from; i <= to; i ++)
  {
    int curVal = buf[i];
    for (j = 0; j < 8; j ++)
    {
      if (((crcVal ^ curVal) & 0x80) != 0) 						//if MSBs are not equal
      {
        crcVal = ((crcVal << 1) ^ CRC8_ONEWIRE_POLY);
      }
      else
      {
        crcVal = (crcVal << 1);
      }
      curVal = curVal << 1;
    }
  }
  return crcVal;
}


static float readTimeInterval()   //send a request to receive the time interval between measurements
{
  float sec = 0;
  Wire.beginTransmission(0x33); 									   
  Wire.write(0x71);
  Wire.write(0x54);
  Wire.write(0x00);
  Wire.endTransmission(true);
  Wire.requestFrom(0x33, 2, 1);
  i = 0;
  while (Wire.available())
  {
    i2cResponse[i++] = Wire.read();
  }
  sec = ((float)(i2cResponse[0]) * 256 + i2cResponse[1]) / 10;
  return sec;
}


static void changeTimeInterval(unsigned char timeInterval[]) 
{
/* there are more data (like offset, gain, lower limit, upper limit) that can be changed or read,
check the data sheet: https://www.epluse.com/en/products/co2-measurement/co2-sensor/ee894/
*/
  delay(1000);
  Wire.beginTransmission(0x33);
  Wire.write(0x71);
  Wire.write(0x54);
  Wire.write(timeInterval[0]);
  Wire.write(timeInterval[1]);
  Wire.write(timeInterval[2]);
  Wire.write(calcCrc8(timeInterval, 0, 2));
  Wire.endTransmission(true);
  delay(500);
}


static int requestCommandA() 										
/*function to send Command A (check the data sheet), 
check if the crc8 checksum is correct, otherwise incorrect values will be returned
*/
{
  int crc8Error = 0;
  Wire.beginTransmission(0x33); 							//Command A to receive temperature and relative humidity
  Wire.write(0xE0);
  Wire.write(0x00);
  Wire.endTransmission(true);
  Wire.requestFrom(0x33, 6, true);
  i = 0;
  while (Wire.available())
  {
    i2cResponse[i++] = Wire.read();           //read data from slave device
  }
  // Check the data with CRC8
  if (i2cResponse[2] != calcCrc8(i2cResponse, 0, 1))
  {
    crc8Error++;
  }
  if (i2cResponse[5] != calcCrc8(i2cResponse, 3, 4))
  {
    crc8Error++;
  }
  return crc8Error;
}


static int requestCommandB() 										
/*function to send Command B (check the data sheet),
and check if the crc8 checksum is correct, otherwise incorrect values will be returned
*/
{
  int crc8Error = 0;
  Wire.beginTransmission(0x33); 							//Command B to receive CO2 average, CO2 raw, pressure 
  Wire.write(0xE0);
  Wire.write(0x27);
  Wire.endTransmission(true);
  Wire.requestFrom(0x33, 9, true); 
  i = 0;
  while (Wire.available())
  {
    i2cResponse[i++] = Wire.read();           //read data from slave device
  }
  // Check the data with CRC8
  if (i2cResponse[2] != calcCrc8(i2cResponse, 0, 1))
  {
    crc8Error++;
  }
  if (i2cResponse[5] != calcCrc8(i2cResponse, 3, 4))
  {
    crc8Error++;
  }
  if (i2cResponse[8] != calcCrc8(i2cResponse, 6, 7))
  {
    crc8Error++;
  }
  return crc8Error;
}


void setup()
{
  Serial.begin(9600); 										//start Serial communication
  Wire.begin(); 												  //initialize I2C peripheral (SDA..A4, SCL..A5)
  unsigned char interval[3] = {0x00, 0x00, 0xFA}; 	//[0]= MEM Adresse,[1] MSB Data, [2] LSB Data //in this case 25 s //for more information check the data sheet
  changeTimeInterval(interval);
  Serial.print("measurement time interval: ");
  Serial.print(readTimeInterval());
  Serial.println(" s");
  Serial.print("temperature");
  Serial.print(CSV_DELIMITER);
  Serial.print("relative humidity");
  Serial.print(CSV_DELIMITER);
  Serial.print("CO2 average");
  Serial.print(CSV_DELIMITER);
  Serial.print("CO2 raw");
  Serial.print(CSV_DELIMITER);
  Serial.println("pressure");
  delay(9000); 														//delay for the first measurement
}


void loop()
{
  if (requestCommandA() == 0)
  {
    temperature = (((float)(i2cResponse[0]) * 256 + i2cResponse[1]) / 100) - 273.15; 	//calculates Celsius value						                      
    Serial.print(temperature);
    Serial.print(" °C");
    Serial.print(CSV_DELIMITER);
    humidity = ((float)(i2cResponse[3]) * 256 + i2cResponse[4]) / 100;      //caluclates the relative humidity value
    Serial.print(humidity);
    Serial.print(" %RH");
    Serial.print(CSV_DELIMITER);
  }
  else
  {
    Serial.println("CRC8 error (Command A: read data for temperature and humidity)");
  }
  if (requestCommandB() == 0)
  {
    co2Average = ((float)(i2cResponse[0]) * 256 + i2cResponse[1]);	        //calculates the CO2 average value
    Serial.print(co2Average, 0);
    Serial.print(" ppm");
    Serial.print(CSV_DELIMITER);
    co2Raw = ((float)(i2cResponse[3]) * 256 + i2cResponse[4]); 		          //calculates the CO2 raw value
    Serial.print(co2Raw, 0);
    Serial.print(" ppm");
    Serial.print(CSV_DELIMITER);
    pressure = ((float)(i2cResponse[6]) * 256 + i2cResponse[7]) / 10;       //calculates the pressure value
    Serial.print(pressure);
    Serial.println(" mbar");
  }
  else
  {
    Serial.println("CRC8 error (Command B: read data for co2Raw, CO2average and pressure)");
  }
  delay(REQUEST_INTERVAL_MS); 										//wait 25 s for the next measuremnet to be done
}
