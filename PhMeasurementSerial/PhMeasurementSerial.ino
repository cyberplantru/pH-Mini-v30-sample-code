/*
  Example code for the pHtoI2C v2.0
  
  http://www.cyberplant.info
  by CyberPlant LLC, 14 November 2015
  This example code is in the public domain.

*/

#include "Wire.h"
#include <EEPROM.h>
#define pHtoI2C 0x48


int addressCentrePoint = 0;
int addressAlphaL = addressCentrePoint+sizeof(float);
int addressAlphaH = addressAlphaL+sizeof(float);

float CentrePoint = EEPROM_float_read(addressCentrePoint);
float alphaL = EEPROM_float_read(addressAlphaL);
float alphaH = EEPROM_float_read(addressAlphaH);

float voltage, data;
byte highbyte, lowbyte, configRegister;

float pH;
float temp = 25.0; // *C

const int numReadings = 10;

float readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
float total = 0;                  // the running total
float average = 0;                // the average

unsigned int Interval = 1000;
long previousMillis = 0;
unsigned long Time;

int incomingByte = 0;   // переменная для хранения полученного байта


void setup()
{
  Wire.begin();
  Serial.begin(9600);
  Time=millis();

  for (int thisReading = 0; thisReading < numReadings; thisReading++)
readings[thisReading] = 0;

  Serial.println("Send command for: Reset > R || Cal. pH 4.00 > 4 || Cal. pH 7.00 > 7 || Cal. pH 10.00 > 9");
  delay(250);
  Serial.println("______________________");
  }
  
  void loop()
{

if (millis()-Time>=Interval)
{  
  Time = millis();
  pH = pHread();
  Serial.print("pH ");
  Serial.println(pH);
  Serial.println("______________________");
}

    if (Serial.available() > 0) //  function of calibration pH
    {  
      
        incomingByte = Serial.read();
        
 if (incomingByte == 114) // press key "R"
 {
  Serial.println("Reset Settings...");
  EEPROM_float_write(addressCentrePoint, 7.00);
  EEPROM_float_write(addressAlphaL, -59.16);
  EEPROM_float_write(addressAlphaH, -59.16);
  alphaL = EEPROM_float_read(addressAlphaL);
  alphaH = EEPROM_float_read(addressAlphaH);
  CentrePoint = EEPROM_float_read(addressCentrePoint);  
  Serial.println("Complete");
  Serial.println("______________________");
 }
 if (incomingByte == 52) // press key "4"
 {
  Serial.println("Cal. pH 4.00");
  alphaL = ((4.00 - CentrePoint) * (273.15 + temp) / voltage); 
  EEPROM_float_write(addressAlphaL, alphaL);
  Serial.println("Settings saved");
  Serial.println("______________________");
 }
 if (incomingByte == 55) // press key "7"
 {
  Serial.println("Cal. pH 7.00");
  CentrePoint = (CentrePoint - pH + 7.00);
  EEPROM_float_write(addressCentrePoint, CentrePoint);
  Serial.println("Settings saved");
  Serial.println("______________________");
 }
  if (incomingByte == 57) // press key "9"
 {
  Serial.println("Cal. pH 10.00");
  alphaH = ((10.00 - CentrePoint) * (273.15 + temp) / voltage); 
  EEPROM_float_write(addressAlphaH, alphaH);
  Serial.println("Settings saved");
  Serial.println("______________________");
 }
    }

}

float ADSread() // read ADS
{
  Wire.requestFrom(pHtoI2C, 3);
  while(Wire.available()) // ensure all the data comes in
  {
    highbyte = Wire.read(); // high byte * B11111111
    lowbyte = Wire.read(); // low byte
    configRegister = Wire.read();
  }
  data = highbyte * 256;
  data = data + lowbyte;
  voltage = data * 2.048 ;
  voltage = voltage / 327.68; // mV
  return voltage;
}

float pHread() // calculate pH
{
    voltage = ADSread();
    total= total - readings[index];
    if (voltage > 0)
    readings[index] = ((alphaL * voltage / (273.15 + temp)) + CentrePoint);
    else if (voltage < 0)
    readings[index] = ((alphaH * voltage / (273.15 + temp)) + CentrePoint);
    total= total + readings[index];
    index = index + 1;
    // if we're at the end of the array...
    if (index >= numReadings)              
    // ...wrap around to the beginning: 
    index = 0;    
    average = total / numReadings;
    pH = average;
}

void EEPROM_float_write(int addr, float val) // write to EEPROM
{  
  byte *x = (byte *)&val;
  for(byte i = 0; i < 4; i++) EEPROM.write(i+addr, x[i]);
}

float EEPROM_float_read(int addr) // read of EEPROM
{    
  byte x[4];
  for(byte i = 0; i < 4; i++) x[i] = EEPROM.read(i+addr);
  float *y = (float *)&x;
  return y[0];
}
