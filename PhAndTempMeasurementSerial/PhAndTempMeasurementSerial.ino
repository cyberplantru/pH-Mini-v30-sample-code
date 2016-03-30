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
#include <SPI.h>
#include <EEPROM.h>

#define pHmini 0x48               // I2C adress for request
#define alpha              0.38   // +- 
#define PtRES_nominal      100.00 // 100R at 0*C, +- alpha
#define T 273.15                  // degrees Kelvin
#define PGA_GAIN 10

float voltage, pHvoltage, pH, Temp;
float VOUT_RREF0, VOUT_RREF1;
float VOUT_PtRES, I_true, Vos;
float PtRES_calculated;
float RREF;
float IsoP;
float Alpha;
int ReadSensors, flag, incomingByte = 0;
int TempManual = 25;

SPISettings mySettting(16000000, MSBFIRST, SPI_MODE3);
const int ss_pin = 10;
void writeLMP91200(uint16_t value)
{
  SPI.beginTransaction(mySettting);
  digitalWrite(ss_pin, LOW);
  SPI.transfer16(value);
  digitalWrite(ss_pin, HIGH);
  SPI.endTransaction();
}

SimpleTimer timer;

void setup()
{
  Wire.begin();
  Serial.begin(9600);
  SPI.begin();
  pinMode(ss_pin, OUTPUT);
  Read_EE();
  timer.setInterval(1000L, cicleRead);
  Serial.println("pH Mini v3.0");
  Serial.println("\n\      Cal. pH 6.86 ---> 7");
  Serial.println("      Cal. pH 4.00 ---> 4");
  Serial.println("      Reset pH -------> 5");
  writeLMP91200(0x0000); // reset lmp91200
  for (int i = 0; i < 14; i++)
  {
    Serial.print(". ");
    delay(100);
  }
  writeLMP91200(0xF680); // table 5, pt100, VCM 1/4
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
  Serial.println(" ... complete");
}

void showResults ()
{
  Serial.print("\n\ t ");
  Serial.print(Temp, 1);
  Serial.print("*C ");
  Serial.print("  pH ");
  Serial.println(pH);
}

void cicleRead()
{
  if (ReadSensors > 5)
    ReadSensors = 1;
  calcResult();
}

void calcResult()
{
  switch (ReadSensors)
  {
    case 1: // VOUT_RREF0

      ADSread(1); // pt100
      VOUT_RREF0 = voltage;
      writeLMP91200(0xEE80); // table 6, pt100
      //writeLMP91200(0xCE80); // table 6, pt1000
      break;

    case 2: // VOUT_RREF1

      ADSread(1); // pt1000 ADSread(3);
      VOUT_RREF1 = voltage;
      writeLMP91200(0xAE80); // table 7, pt100, VCM 1/4
      //writeLMP91200(0x8E80); // table 7, pt1000, VCM 1/4
      break;

    case 3: // Temp

      ADSread(1); // pt1000 ADSread(3);
      VOUT_PtRES = voltage;
      Vos = (VOUT_RREF0 - VOUT_RREF1) / 5;
      I_true = (2 * VOUT_RREF1 - VOUT_RREF0) / (10 * VOUT_RREF1);
      RREF += (VOUT_RREF0 - VOUT_RREF1);
      PtRES_calculated = ((VOUT_PtRES / PGA_GAIN) - Vos) / I_true;
      Temp = (PtRES_calculated - ((VOUT_RREF1 + VOUT_RREF0) - PtRES_nominal)) / alpha;
      writeLMP91200(0x0600); // 1/4 Vref
      break;

    case 4:

      if (-25 > Temp || Temp > 250) {
        Temp = TempManual;
      }
      ADSread(0);
      pH = IsoP - Alpha * (T + Temp) * pHvoltage;
      writeLMP91200(0xF680); // table 5, pt100, VCM 1/4
      //writeLMP91200(0xD680); // table 5, pt1000, VCM 1/4
      break;

    case 5:
      flag = 0;
      showResults ();
      break;
  }
  ReadSensors ++;
}

void ADSread(int rate) // read ADS
{
  byte highbyte, lowbyte, configRegister;
  float data;
  Wire.requestFrom(pHmini, 3);
  while (Wire.available()) {
    highbyte = Wire.read();
    lowbyte = Wire.read();
    configRegister = Wire.read();
    data = highbyte * 256;
    data = data + lowbyte;
    voltage = data * 2.048 ;
  }
  switch (rate)
  {
    case 0:
      pHvoltage = voltage / 32768;
      break;

    case 1:
      voltage = voltage / 327.68;   // pt 100
      //voltage = voltage / 32.768; // pt 1000
      break;
  }
}

void cal_sensors()
{
  flag = 1;
  switch (incomingByte)
  {
    case 53:
      Serial.print("Reset pH ...");
      IsoP = 7,00;
      Alpha = 0.05916;
      SaveSet();
      break;

    case 52:
      Serial.print("Cal. pH 4.00 ...");
      Alpha = (IsoP - 4) / pHvoltage / (T + Temp);
      //Serial.print("\n\Cal. pH 9.18 ...");
      //Alpha = (IsoP - 9.18) / pHvoltage / (T + Temp);
      //Serial.print("\n\Cal. pH 10.00 ...");
      //Alpha = (IsoP - 10.00) / pHvoltage / (T + Temp);
      Serial.print(Alpha);
      SaveSet();
      break;

    case 55:
      Serial.print("Cal. pH 6.86 ... ");
      IsoP = (IsoP - pH + 6.86);
      //IsoP = (IsoP - pH + 7.00);
      Serial.print(IsoP);
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
