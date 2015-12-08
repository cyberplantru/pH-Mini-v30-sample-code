/*
  Example code for the pH to I2C module v2.0
  
  http://www.cyber-plant.com
  by CyberPlant LLC, 14 November 2015
  This example code is in the public domain.
  
  upd. 08.12.2015

*/

#include "Wire.h"
#include <SPI.h>
#include <EEPROM.h>
#define pHtoI2C 0x48
#define PT_TC               0.3850  
#define PT_RES_NOMINAL      100.781  // +- 0.39 for pt100  
#define T 273.15                    // degrees Kelvin

float voltage, data;
byte highbyte, lowbyte, configRegister;

float pH;
float pH25;
float temp;
float PT_CAL;
const float tempManual= 25.0;

const int numReadingstemp = 30;
float readingstemp[numReadingstemp];      // the readings from the analog input
int indextemp = 0;                  // the index of the current reading
float totaltemp = 0;                  // the running total
float averagetemp = 0;                // the average

unsigned int Interval = 250;
long previousMillis = 0;
unsigned long Time;

int incomingByte = 0;   

int sequence = 0;

enum { REG = 10 }; // pin D10 is SS line for LMP91200

  float IsoP;
  float AlphaL;
  float AlphaH;
  
 struct MyObject {
  float IsoP;
  float AlphaL;
  float AlphaH;
};

void writeLMP91200(int ss_pin, uint16_t value)
{
  digitalWrite(ss_pin, LOW);
  SPI.transfer(highByte(value));
  SPI.transfer(lowByte(value));
  digitalWrite(ss_pin, HIGH);
  
}
void setup()
{
  Wire.begin();
  Serial.begin(9600);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE3);
  SPI.begin();

  pinMode(REG, OUTPUT);
  writeLMP91200(REG, 0x0000); // read pH

  int eeAddress = 0; 

  MyObject customVar;
  EEPROM.get(eeAddress, customVar);
  
  IsoP = (customVar.IsoP);
  AlphaL = (customVar.AlphaL);
  AlphaH = (customVar.AlphaH);
  
  Time=millis();

  for (int thisReadingtemp = 0; thisReadingtemp < numReadingstemp; thisReadingtemp++)
  readingstemp[thisReadingtemp] = 0;

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

void PT_CAL_read()
{
  ADSread();
  PT_CAL = voltage;
}

void pt100_read()
{
  ADSread();
  totaltemp= totaltemp - readingstemp[indextemp];
  readingstemp[indextemp] = ((voltage/PT_CAL)* PT_CAL - PT_RES_NOMINAL) / PT_TC;
  totaltemp= totaltemp + readingstemp[indextemp];
  indextemp = indextemp + 1;
  // if we're at the end of the array...
  if (indextemp >= numReadingstemp)              
  // ...wrap around to the beginning: 
  indextemp = 0;    
  averagetemp = totaltemp / numReadingstemp;
  temp = averagetemp;
  
  if (temp>200 || temp<-20)
    {
    temp = tempManual;
    }  
}

void ADSread() // read ADS
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
  int eeAddress = 0 + sizeof(float);
  EEPROM.put(eeAddress,  AlphaL);
  Serial.println(" complete");
 }
 else if (incomingByte == 55) // press key "7"
 {
  Serial.print("Cal. pH 7.00 ...");
  IsoP = (IsoP - pH + 7.00);
  int eeAddress = 0;
  EEPROM.put(eeAddress, IsoP);
  Serial.println(" complete");
 }
  else if (incomingByte == 57) // press key "9"
 {
  Serial.print("Cal. pH 10.00 ...");
  AlphaH = (IsoP - 10) / voltage / (T + tempManual); 
  int eeAddress = 0 + (sizeof(float)*2); 
  EEPROM.put(eeAddress,  AlphaH);
  Serial.println(" complete");
 }
}

void Reset_pH()
{
  int eeAddress = 0;

  Serial.print("Reset pH ...");

  MyObject customVar = {
    7.5099949836,
    0.0778344535,
    0.0850976657
  };

  EEPROM.put(eeAddress, customVar);

  EEPROM.get(eeAddress, customVar);
  
  IsoP = (customVar.IsoP);
  AlphaL = (customVar.AlphaL);
  AlphaH = (customVar.AlphaH);
  Serial.println(" complete");
}


  void loop()
{
if (millis()-Time>=Interval)
{  
  Time = millis();
  
  sequence ++;
  if (sequence > 3)
  sequence = 0;

  if (sequence == 0)
  {
    writeLMP91200(REG, 0xE880); // switch on R_CAL reading in the next step
  }
  
  if (sequence == 1)
  {
    PT_CAL_read();
    writeLMP91200(REG, 0xA880); // switch on temperature sensor pt100 reading in the next step
  }
  
  if (sequence == 2)
  {
    pt100_read();
    writeLMP91200(REG, 0x0000); // switch on pH reading in the next step
  }
  
  if (sequence == 3)
  {
    pH_read();
    pH_calculate();
  Serial.println("  ");  
  Serial.print("  Temp ");
  Serial.print(temp, 2);
  Serial.print(" *C ");
  Serial.print("  pH ");
  Serial.print(pH);
  Serial.print("  R_CAL ");
  Serial.println(PT_CAL);
  }
}

       if (Serial.available() > 0) //  function of calibration E.C.
    {  
        incomingByte = Serial.read();
        cal_sensors();
    }
        
}

/*-----------------------------------End loop---------------------------------------*/ 


