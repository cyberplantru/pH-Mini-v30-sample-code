/*
  Example code for the pH to I2C module v2.0
  
  http://www.cyberplant.info
  by CyberPlant LLC
  This example code is in the public domain.
  upd. 06.12.2015

*/

#include "Wire.h"
#include <SPI.h>
#include <EEPROM.h>
#define pHtoI2C 0x48
#define PT_TC               0.3850  
#define PT_RES_NOMINAL      100.781  // +- 0.39 for pt100  
#define T 273.15                    // degrees Kelvin

int addressIsoP = 0;
int addressAlphaL = addressIsoP+sizeof(float);
int addressAlphaH = addressAlphaL+sizeof(float);

float IsoP = EEPROM_float_read(addressIsoP);
float alphaL = EEPROM_float_read(addressAlphaL);
float alphaH = EEPROM_float_read(addressAlphaH);

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
    pH25_calculate();
  Serial.println("  ");  
  Serial.print("  Temp ");
  Serial.print(temp, 2);
  Serial.print(" *C ");
  Serial.print("  pH ");
  Serial.print(pH);
  Serial.print("  pH25 ");
  Serial.println(pH25);
  //Serial.print("  R_CAL ");
  //Serial.println(PT_CAL);
  }
}

       if (Serial.available() > 0) //  function of calibration E.C.
    {  
        incomingByte = Serial.read();
        cal_sensors();
    }
        
}

/*-----------------------------------End loop---------------------------------------*/ 

float PT_CAL_read()
{
  ADSread();
  PT_CAL = voltage;
  return PT_CAL;
}

float pt100_read()
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
    
  return temp;
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
  pH = IsoP - alphaL * (T + temp) * voltage;
  else if (voltage < 0)
  pH = IsoP - alphaH * (T + temp) * voltage;
  return pH;
}

float pH25_calculate() // calculate pH
{
  if (voltage > 0)
  pH25 = IsoP - alphaL * (T + tempManual) * voltage;
  else if (voltage < 0)
  pH25 = IsoP - alphaH * (T + tempManual) * voltage;
  return pH25;
}

void cal_sensors()
{
  Serial.println("  ");  
  if (incomingByte == 56) // press key "8"
 {
  Reset_pH();
 }
 else if (incomingByte == 52) // press key "4"
 {
  Serial.print("Cal. pH 4.00 ...");
  alphaL = (IsoP - 4) / voltage / (T + temp); 
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
  alphaH = (IsoP - 10) / voltage / (T + temp); 
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
