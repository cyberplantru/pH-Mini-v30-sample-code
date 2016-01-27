/*
  Example code for the pH to I2C module v2.0
  Works in Arduino IDE 1.6.7
  http://www.cyber-plant.com
  by CyberPlant LLC, 14 November 2015
  This example code is in the public domain.

  upd. 27.01.2016
*/

#include "Wire.h"
#include <SPI.h>
#include <EEPROM.h>
#define pHtoI2C 0x48
#define PT_TC               0.3850
#define PT_RES_NOMINAL      102.75  // +- 0.39 for pt100  
#define T 273.15                    // degrees Kelvin

float data, voltage, pH, temp, PT_CAL;
int tempManual = 25;

const unsigned long Interval = 250;  // 1/4 second
long previousMillis = 0;
unsigned long Time;

int incomingByte = 0;
int ReadSensors = 0;

float IsoP;
float AlphaL;
float AlphaH;

SPISettings mySettting(16000000, MSBFIRST, SPI_MODE3);
const int ss_pin = 9;
void writeLMP91200(uint16_t value)
{
  SPI.beginTransaction(mySettting);
  digitalWrite(ss_pin, LOW);
  SPI.transfer16(value);
  digitalWrite(ss_pin, HIGH);
  SPI.endTransaction();
}

void setup()
{
  Wire.begin();
  Serial.begin(115200);
  SPI.begin();
  pinMode(ss_pin, OUTPUT);
  writeLMP91200(0xE880);
  Read_EE();

  Serial.println("Calibrate commands:");
  Serial.println("pH :");
  Serial.println("      Cal. pH 4.00 ---- 4");
  Serial.println("      Cal. pH 7.00 ---- 7");
  Serial.println("      Cal. pH 10.00 --- 9");
  Serial.println("      Reset pH ---------8");
  Serial.println("  ");

  Time = millis();
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

void ADSread() // read ADS
{
  byte highbyte, lowbyte, configRegister;
  Wire.requestFrom(pHtoI2C, 3, sizeof(byte) * 3);
  while (Wire.available()) // ensure all the data comes in
  {
    highbyte = Wire.read(); // high byte * B11111111
    lowbyte = Wire.read(); // low byte
    configRegister = Wire.read();
  }
  data = highbyte * 256;
  data = data + lowbyte;
  voltage = data * 2.048 ;
  voltage = voltage / 32768; // mV
}

void cal_sensors()
{

  switch (incomingByte)
  {
    case 56:
      Serial.print("Reset pH ...");
      IsoP = 7.5099949836;
      AlphaL = 0.0778344535;
      AlphaH = 0.0850976657;
      break;

    case 52:
      Serial.print("Cal. pH 4.00 ...");
      AlphaL = (IsoP - 4) / voltage / (T + tempManual);
      break;

    case 55:
      Serial.print("Cal. pH 7.00 ...");
      IsoP = (IsoP - pH + 7.00);
      break;

    case 57:
      Serial.print("Cal. pH 10.00 ...");
      AlphaH = (IsoP - 10) / voltage / (T + tempManual);
      break;
    default:
      SaveSet();
      Serial.println(" complete");
  }
}

void showResults ()
{
  Serial.println("  ");
  Serial.print("  Temp ");
  Serial.print(temp, 2);
  Serial.print(" *C ");
  Serial.print("  pH ");
  Serial.print(pH);
  Serial.print("  R_CAL ");
  Serial.println(PT_CAL);
}

void loop()
{
  if (millis() - Time >= Interval)
  {
    Time = millis();

    switch (ReadSensors)
    {
      case 0:
        ADSread();
        PT_CAL = voltage * 100;
        writeLMP91200(0xA880);
        break;

      case 1:
        ADSread();
        temp = (((voltage * 100) / PT_CAL) * PT_CAL - PT_RES_NOMINAL) / PT_TC;
        writeLMP91200(0x0000);
        break;

      case 2:
        ADSread();
        if (voltage > 0)
          pH = IsoP - AlphaL * (T + temp) * voltage;
        else if (voltage < 0)
          pH = IsoP - AlphaH * (T + temp) * voltage;
        writeLMP91200(0xE880);
        break;

      case 3:
        showResults ();
        break;
    }

    ReadSensors ++;
    if (ReadSensors > 3)
    {
      ReadSensors = 0;
    }
  }

  if (Serial.available() > 0) //  function of calibration
  {
    incomingByte = Serial.read();
    cal_sensors();
  }
}

