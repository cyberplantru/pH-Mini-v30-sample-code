/*
  Example code for the pH Mini v3.0
  Works in Arduino IDE 1.6.8
  http://www.cyber-plant.com
  by CyberPlant LLC, 14 November 2015
  This example code is in the public domain.

  upd. 27.03.2016
*/

#include "Wire.h"
#include <EEPROM.h>
#define pHtoI2C 0x48
#define T 273.15                    // degrees Kelvin

float pHvoltage, pH;
int TempManual = 25;

const unsigned long Interval = 3000;
long previousMillis = 0;
unsigned long Time;

int incomingByte = 0;

float IsoP;
float Alpha;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  Read_EE();
  Time = millis();

  Serial.println("pH Mini v3.0");
  Serial.println("\n\      Cal. pH 6.86 ---- 7");
  Serial.println("      Cal. pH 4.00 ---- 4");
  //Serial.println("      Cal. pH 9.18 ---- 4");
  Serial.println("      Reset pH ---------1");
}

struct MyObject {
  float IsoP;
  float Alpha;
};

void Read_EE()
{
  int eeAddress = 0;
  MyObject customVar;
  EEPROM.get(eeAddress, customVar);
  IsoP = (customVar.IsoP);
  Alpha = (customVar.Alpha);
}

void SaveSet()
{
  int eeAddress = 0;
  MyObject customVar = {
    IsoP,
    Alpha
  };
  EEPROM.put(eeAddress, customVar);
}

void pH_read() // read ADS
{
  byte highbyte, lowbyte, configRegister;
  float data;
  Wire.requestFrom(pHtoI2C, 3);
  while (Wire.available()) // ensure all the data comes in
  {
    highbyte = Wire.read(); // high byte * B11111111
    lowbyte = Wire.read(); // low byte
    configRegister = Wire.read();
  }
  data = highbyte * 256;
  data = data + lowbyte;
  pHvoltage = data * 2.048 ;
  pHvoltage = pHvoltage / 32768; // mV
  pH = IsoP - Alpha * (T + TempManual) * pHvoltage;
}

void cal_sensors()
{

  switch (incomingByte)
  {

    case 49:
      Serial.print("\n\Reset pH ...");
      IsoP = 7.14;
      Alpha = 0.05916;
      break;

    case 52:
      Serial.print("\n\Cal. pH 4.00 ...");
      Alpha = (IsoP - 4) / pHvoltage / (T + TempManual);
      //Alpha = (IsoP - 9.18) / pHvoltage / (T + TempManual);
      break;

    case 55:
      Serial.print("\n\Cal. pH 6.86 ...");
      IsoP = (IsoP - pH + 6.86);
      //IsoP = (IsoP - pH + 7.00);
      break;
  }
  SaveSet();
  Serial.println(" complete");
}

void showResults ()
{
  Serial.print("\n\pH ");
  Serial.println(pH);
}

void loop()
{
  if (millis() - Time >= Interval)
  {
    Time = millis();
    pH_read();
    showResults();
  }

  if (Serial.available() > 0) //  function of calibration
  {
    incomingByte = Serial.read();
    cal_sensors();
  }
}

