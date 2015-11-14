/*
  Example code for the pH to I2C module v2.0
  
  http://www.cyberplant.info
  by CyberPlant LLC, 14 November 2015
  This example code is in the public domain.

*/

#include "Wire.h"
#include <SPI.h>
#include <EEPROM.h>
#define pHtoI2C 0x48
#define PT_TC               0.3850     


int addressCentrePoint = 0;
int addressAlphaL = addressCentrePoint+sizeof(float);
int addressAlphaH = addressAlphaL+sizeof(float);
int addressPT_RES_NOMINAL = addressAlphaH+sizeof(float);

float CentrePoint = EEPROM_float_read(addressCentrePoint);
float alphaL = EEPROM_float_read(addressAlphaL);
float alphaH = EEPROM_float_read(addressAlphaH);
float PT_RES_NOMINAL = EEPROM_float_read(addressPT_RES_NOMINAL);

float voltage, data;
byte highbyte, lowbyte, configRegister;

float pH;
float temp;
float PT_CAL;
int err_temp = 0;

const int numReadings = 10;

float readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
float total = 0;                  // the running total
float average = 0;                // the average

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

enum { REG = 9 }; // pin D9 is SS line for LMP91200

void writeLMP91200(int ss_pin, uint16_t value)
{
  digitalWrite(ss_pin, LOW);
  /* Фокус вот в чём: сначала шлём старший байт */
  SPI.transfer(highByte(value));
  /* А потом младший */
  SPI.transfer(lowByte(value));
  digitalWrite(ss_pin, HIGH);
  
}
void setup()
{
  Wire.begin();
  Serial.begin(9600);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV32);
  SPI.setDataMode(SPI_MODE3);
  SPI.begin();

  pinMode(REG, OUTPUT);
  writeLMP91200(REG, 0x0000); // read pH

  Time=millis();

  for (int thisReading = 0; thisReading < numReadings; thisReading++)
  readings[thisReading] = 0;

  for (int thisReadingtemp = 0; thisReadingtemp < numReadingstemp; thisReadingtemp++)
  readingstemp[thisReadingtemp] = 0;

  Serial.println("Send command for: Reset > R || Cal. pH 4.00 > 4 || Cal. pH 7.00 > 7 || Cal. pH 10.00 > 9 || PT_RES_NOMINAL -- > 1 || PT_RES_NOMINAL ++ > 2");
  delay(250);
  Serial.println("______________________");
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
    voltage = ADSread();
    PT_CAL = voltage;
    writeLMP91200(REG, 0xA880); // switch on temperature sensor pt100 reading in the next step
  }
  
  if (sequence == 2)
  {
    temp = temp_read();
    
    if (temp>200 || temp<-20)
    {
    temp = 25;
    err_temp = 1;
    }
    else
    {
      err_temp = 0;
    }
    writeLMP91200(REG, 0x0000); // switch on pH reading in the next step
  }
  
  if (sequence == 3)
  {
   pH = pHread();

  if (err_temp == 1)
  {
  Serial.println("Error pt100 reading, temp as default 25*C");
  Serial.println("______________________");
  }
  Serial.print("pH ");
  Serial.print(pH);
  Serial.print(", Temp ");
  Serial.print(temp, 1);
  Serial.println(" *C");
  //Serial.print("R_CAL ");
  //Serial.println(PT_CAL);
  Serial.println("______________________");
  }
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
  EEPROM_float_write(addressPT_RES_NOMINAL, 103.51); // pt100 table: 100.0, 100.39, 100.78, 101.17, 101.95, 102.34, 102.73, 103.12
  alphaL = EEPROM_float_read(addressAlphaL);
  alphaH = EEPROM_float_read(addressAlphaH);
  CentrePoint = EEPROM_float_read(addressCentrePoint);  
  PT_RES_NOMINAL = EEPROM_float_read(addressPT_RES_NOMINAL);
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
  if (incomingByte == 49) // press key "1"
 {
  Serial.println("PT_RES_NOMINAL --");
  PT_RES_NOMINAL -= 0.39;
  EEPROM_float_write(addressPT_RES_NOMINAL, PT_RES_NOMINAL);
  PT_RES_NOMINAL = EEPROM_float_read(addressPT_RES_NOMINAL);
  Serial.println(PT_RES_NOMINAL);
  Serial.println("Settings saved");
  Serial.println("______________________");
 }
 if (incomingByte == 50) // press key "2"
 {
  Serial.println("PT_RES_NOMINAL ++");
  PT_RES_NOMINAL += 0.39;
  EEPROM_float_write(addressPT_RES_NOMINAL, PT_RES_NOMINAL);
  PT_RES_NOMINAL = EEPROM_float_read(addressPT_RES_NOMINAL);
  Serial.println(PT_RES_NOMINAL);
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

float temp_read() // calculate pH
{
    voltage = ADSread();
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
