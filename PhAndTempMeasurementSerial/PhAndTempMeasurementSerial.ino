/*
  Example code for the pH Mini v3.0
  Works in Arduino IDE 1.6.8
  http://www.cyber-plant.com
  by CyberPlant LLC, 14 November 2015
  This example code is in the public domain.

  upd. 27.03.2016
*/

#include <SimpleTimer.h>
#include "Wire.h"
#include <SPI.h>
#include <EEPROM.h>

#define pHtoI2C 0x48
#define alpha              0.32
#define PtRES_nominal      101.172 // +- 0.39 for pt100
#define T 273.15                    // degrees Kelvin
#define PGA_GAIN 10

float voltage, pHvoltage, pH, Temp;
float VOUT_RREF0, VOUT_RREF1;
float VOUT_PtRES, I_true, Vos;
float PtRES_calculated;
float RREF;
float IsoP;
float Alpha;
int ReadSensors = 0;
int incomingByte = 0;
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
  writeLMP91200(0xF680);
  Read_EE();
  timer.setInterval(900L, cicleRead);
  Serial.println("pH Mini v3.0");
  Serial.println("\n\      Cal. pH 4.00 ---- 4");
  Serial.println("      Cal. pH 6.86 ---- 7");
  Serial.println("      Reset pH ---------8");
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

void showResults ()
{
  Serial.print("\n\Temp ");
  Serial.print(Temp, 2);
  Serial.print(" *C ");
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

      ADSread(2); // pt1000 ADSread(3);
      VOUT_RREF0 = voltage;
      writeLMP91200(0xEE80); // table 6, pt100
      //writeLMP91200(0xCE80); // table 6, pt1000
      break;

    case 2: // VOUT_RREF1

      ADSread(2); // pt1000 ADSread(3);
      VOUT_RREF1 = voltage;
      writeLMP91200(0xAE80); // table 7, pt100, VCM 1/4
      //writeLMP91200(0x8E80); // table 7, pt1000, VCM 1/4
      break;

    case 3: // Temp

      ADSread(2); // pt1000 ADSread(3);
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
      pHvoltage = voltage;
      pH = IsoP - Alpha * (T + Temp) * pHvoltage;
      writeLMP91200(0xF680); // table 5, pt100, VCM 1/4
      //writeLMP91200(0xD680); // table 5, pt1000, VCM 1/4
      break;

    case 5:
      showResults ();
      break;
  }
  ReadSensors ++;
}

void ADSread(int rate) // read ADS
{

  byte highbyte, lowbyte, configRegister;
  float data;

  Wire.requestFrom(pHtoI2C, 3);
  //Wire.requestFrom(pHtoI2C, 3, sizeof(byte) * 3);
  if (Wire.available())
  {
    highbyte = Wire.read();
    lowbyte = Wire.read();
    configRegister = Wire.read();

    data = highbyte * 256;
    data = data + lowbyte;
    voltage = data * 2.048 ;


    switch (rate)
    {
      case 0:
        voltage = voltage / 32768;
        break;
      /*
          case 1:
            voltage = voltage / 3276.8;
            break;
      */
      case 2:
        voltage = voltage / 327.68;
        break;

      case 3:
        voltage = voltage / 32.768;
        break;
    }
  }
}

void cal_sensors()
{
  Serial.println(" ");
  if (incomingByte == 56) // press key "8"
  {
    Serial.print("Reset pH ...");
    IsoP = 7.14;
    Alpha = 0.05916;
  }
  else if (incomingByte == 52) // press key "4"
  {
    Serial.print("Cal. pH 4.00 ...");
    Alpha = (IsoP - 4) / pHvoltage / (T + Temp);
  }
  else if (incomingByte == 55) // press key "7"
  {
    Serial.print("Cal. pH 6.86 ...");
    IsoP = (IsoP - pH + 6.86);
    //IsoP = (IsoP - pH + 7.00);
  }
  SaveSet();
  Serial.println(" complete");
}

void loop()
{
  if (Serial.available() > 0) //  function of calibration
  {
    incomingByte = Serial.read();
    cal_sensors();
  }
  timer.run();
}

