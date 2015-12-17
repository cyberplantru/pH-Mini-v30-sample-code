/*
  Example code for the pH to I2C module v2.0
  for Adruino IDE 1.6.6
  http://www.cyber-plant.com
  by CyberPlant LLC, 14 November 2015
  This example code is in the public domain.
  
  upd. 17.12.2015

*/
#include "Wire.h"
#include <EEPROM.h>
#define pHtoI2C 0x48
#define T 273.15 // degrees Kelvin

float voltage, data;
byte highbyte, lowbyte, configRegister;

float pH;
float tempManual= 25.0;
  
unsigned int Interval = 1000;
long previousMillis = 0;
unsigned long Time;

int incomingByte = 0;

  float IsoP;
  float AlphaL;
  float AlphaH;
  
void setup()
{
  Serial.begin(9600);
  Wire.begin();
  Time=millis();
  Read_EE();
  Serial.println("Calibrate commands:");
  Serial.println("pH :");
  Serial.println("      Cal. pH 4.00 ---- 4");
  Serial.println("      Cal. pH 7.00 ---- 7");
  Serial.println("      Cal. pH 10.00 --- 9");
  Serial.println("      Reset pH ---------8");
  Serial.println("  ");
  delay(250);
}

 struct MyObject {
float IsoP;
float AlphaL;
float AlphaH;
};

void Read_EE()
{
  int eeAddress = 0; 

  MyObject customVar;
  EEPROM.get(eeAddress, customVar);
  
  IsoP = (customVar.IsoP);
  AlphaL = (customVar.AlphaL);
  AlphaH = (customVar.AlphaH);
}

void Reset_pH()
{
  Serial.print("Reset pH ...");

  IsoP = 7.5099949836;
  AlphaL = 0.0778344535;
  AlphaH = 0.0850976657;
  SaveSet();
  Serial.println(" complete");
}

void SaveSet()
{
  int eeAddress = 0;
  MyObject customVar = {
    IsoP,
    AlphaL,
    AlphaH
  };
  EEPROM.put(eeAddress, customVar);
}


void pH_read() // read ADS
{
  Wire.requestFrom(pHtoI2C, 3);
  while(Wire.available())
  {
    highbyte = Wire.read();
    lowbyte = Wire.read();
    configRegister = Wire.read();
  }
  data = highbyte * 256;
  data = data + lowbyte;
  voltage = data * 2.048 ;
  voltage = voltage / 32768;
}

void pH_calculate() // calculate pH
{
  if (voltage > 0)
  pH = IsoP - AlphaL * (T + tempManual) * voltage;
  else if (voltage < 0)
  pH = IsoP - AlphaH * (T + tempManual) * voltage;
}

void cal_sensors()
{
 Serial.println(" ");
 if (incomingByte == 56) // press key "8"
 {
  Reset_pH();
 }
 else if (incomingByte == 52) // press key "4"
 {
  Serial.print("Cal. pH 4.00 ...");
  AlphaL = (IsoP - 4) / voltage / (T + tempManual);
  SaveSet();
 }
 else if (incomingByte == 55) // press key "7"
 {
  Serial.print("Cal. pH 7.00 ...");
  IsoP = (IsoP - pH + 7.00);
  SaveSet();
 }
  else if (incomingByte == 57) // press key "9"
 {
  Serial.print("Cal. pH 10.00 ...");
  AlphaH = (IsoP - 10) / voltage / (T + tempManual); 
  SaveSet();
 }
}


void loop() {
 if (millis()-Time>=Interval)
{  
  Time = millis();
  
  pH_read();
  pH_calculate();
  

  //Serial.print(" *C");
  Serial.print("    pH ");
  Serial.println(pH);
}

       if (Serial.available() > 0) //  function of calibration E.C.
    {  
        incomingByte = Serial.read();
        cal_sensors();
    }
        
}






