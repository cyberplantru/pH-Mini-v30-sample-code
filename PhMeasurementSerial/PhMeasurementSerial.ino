/*
  Example code for the pH Mini v3.0
  Works in Arduino IDE 1.6.8
  http://www.cyber-plant.com
  by CyberPlant LLC, 14 November 2015
  This example code is in the public domain.
  upd. 29.03.2016
*/
#include <SimpleTimer.h>
#include "Wire.h"
#include <EEPROM.h>
#define pHmini 0x48
#define T 273.15                    // degrees Kelvin

byte highbyte, lowbyte, configRegister;
float pHvoltage, pH, IsoP, Alpha, data;
int TempManual = 25;
int incomingByte, flag = 0;

SimpleTimer timer;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  Read_EE();
  Serial.println("pH Mini v3.0");
    Serial.println("\n\       Reset pH -------> 0");
  Serial.println("     Cal. pH 6.86 ---> 1");
  Serial.println("      Cal. pH 4.00 ---> 2");
  for (int i = 0; i < 14; i++) {
    Serial.print(". ");
    delay(100);
  }
  Serial.println();
  timer.setInterval(2000L, pH_read);
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
  Serial.println(" . . . complete");
}

void pH_read() // read ADS
{
  ADSread();
}

void ADSread ()
{
  Wire.requestFrom(pHmini, 3);
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
  Serial.print("\n\pH ");
  Serial.println(pH);
  flag = 0;
}

void cal_sensors()
{
  flag = 1;
  switch (incomingByte)
  {
    case 48:
      Serial.print("Reset pH ");
      IsoP = 7.00;
      Alpha = 0.05916;
      SaveSet();
      break;

    case 49:
      Serial.print("Cal. pH 6.86 . . . ");
      IsoP = (IsoP - pH + 6.86);
      //IsoP = (IsoP - pH + 7.00);
      Serial.print(IsoP);
      SaveSet();
      break;

    case 50:
      Serial.print("Cal. pH 4.00 . . . ");
      Alpha = (IsoP - 4) / pHvoltage / (T + TempManual);
      //Serial.print("\n\Cal. pH 9.18 ...");
      //Alpha = (IsoP - 9.18) / pHvoltage / (T + TempManual);
      //Serial.print("\n\Cal. pH 10.00 ...");
      //Alpha = (IsoP - 10.00) / pHvoltage / (T + TempManual);
      Serial.print(Alpha);
      SaveSet();
      break;
  }
}

void loop()
{
  if (Serial.available() > 0 && flag == 0) {
    incomingByte = Serial.read();
    cal_sensors();
  }
  timer.run();
}
