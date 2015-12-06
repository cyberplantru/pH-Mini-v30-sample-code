/*
  Example code for the pHtoI2C v2.0
  
  http://www.cyberplant.info
  by CyberPlant LLC
  This example code is in the public domain.
  upd. 06.12.2015
*/

#include "Wire.h"
#include <EEPROM.h>
#define pHtoI2C 0x48
#define T 273.15 // degrees Kelvin

int addressIsoP = 0;
int addressAlphaL = addressIsoP+sizeof(float);
int addressAlphaH = addressAlphaL+sizeof(float);

float IsoP = EEPROM_float_read(addressIsoP);
float alphaL = EEPROM_float_read(addressAlphaL);
float alphaH = EEPROM_float_read(addressAlphaH);

float voltage, data;
byte highbyte, lowbyte, configRegister;

float pH;
float tempManual= 25.0;

unsigned int Interval = 1000;
long previousMillis = 0;
unsigned long Time;

int incomingByte = 0;


void setup()
{
  Wire.begin();
  Serial.begin(9600);
  Time=millis();
  
if (IsoP < 1 || IsoP > 10) // Primary auto setting
{
  Reset_pH();
}


  Serial.println("Calibrate commands:");
  Serial.println("pH :");
  Serial.println("      Cal. pH 4.00 ---- 4");
  Serial.println("      Cal. pH 7.00 ---- 7");
  Serial.println("      Cal. pH 10.00 --- 9");
  Serial.println("      Reset pH ---------8");
  Serial.println("  ");
  delay(250);
}
  
  void loop()
{

if (millis()-Time>=Interval)
{  
  Time = millis();
  
  pH_read();
  pH_calculate();
  
  Serial.println("  ");
  //Serial.print("t ");
  //Serial.print(tempManualManual);
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

/*-----------------------------------End loop---------------------------------------*/ 

float pH_read() // read ADS
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
  voltage = voltage / 32768;
  return voltage;
}

float pH_calculate() // calculate pH
{
  if (voltage > 0)
  pH = 7.50 - alphaL * (T + tempManual) * voltage;
  else if (voltage < 0)
  pH = 7.50 - alphaH * (T + tempManual) * voltage;
  return pH;
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
  alphaL = (IsoP - 4) / voltage / (T + tempManual); 
  EEPROM_float_write(addressAlphaL, alphaL);
  Serial.println(" complete");
 }
 else if (incomingByte == 55) // press key "7"
 {
  Serial.print("Cal. pH 7.00 ...");
  IsoP = (IsoP - pH + 7.00);
  EEPROM_float_write(addressIsoP, IsoP);
  Serial.println(" complete");
 }
  else if (incomingByte == 57) // press key "9"
 {
  Serial.print("Cal. pH 10.00 ...");
  alphaH = (IsoP - 10) / voltage / (T + tempManual); 
  EEPROM_float_write(addressAlphaH, alphaH);
  Serial.println(" complete");
 }
}

void Reset_pH()
{
  Serial.print("Reset pH ...");
  EEPROM_float_write(addressIsoP, 7.5099949836);
  EEPROM_float_write(addressAlphaL, 0.0778344535);
  EEPROM_float_write(addressAlphaH, 0.0850976657);
  alphaL = EEPROM_float_read(addressAlphaL);
  alphaH = EEPROM_float_read(addressAlphaH);
  IsoP = EEPROM_float_read(addressIsoP);  
  Serial.println(" complete");
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

