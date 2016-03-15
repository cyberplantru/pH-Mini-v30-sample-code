
/*
  Example code for the pH to I2C module v2.0
  Works in Arduino IDE 1.6.7
  http://www.cyber-plant.com
  by CyberPlant LLC, 14 November 2015
  This example code is in the public domain.

  upd. 27.01.2016
*/

#include <SimpleTimer.h>
#include "Wire.h"
#include <SPI.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>  // include LCD library
#include <SoftwareSerial.h>

#define pHtoI2C 0x48
#define alpha              0.342
#define PtRES_nominal      99.6 // pt1000, 106.545   +- 0.39 for pt100
#define T 273.15                    // degrees Kelvin
#define PGA_GAIN 10


#define PumpDown 5
#define PumpUp 6
#define RelayPin 8        // pin 8

#define I2C_ADDR    0x38 // <<----- Add your address here.  Find it from I2C Scanner
#define En_pin  4
#define Rw_pin  5
#define Rs_pin  6
#define D4_pin  0
#define D5_pin  1
#define D6_pin  2
#define D7_pin  3

#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define SAVE_10BIT_ADC           0  // right
#define NEXT_10BIT_ADC            145  // up
#define UP_10BIT_ADC          328  // down
#define DOWN_10BIT_ADC          504  // left
#define PREV_10BIT_ADC        741  // right
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window
#define BUTTON_NONE               0  // 
#define BUTTON_SAVE              1  // 
#define BUTTON_NEXT                 2  // 
#define BUTTON_UP               3  // 
#define BUTTON_DOWN               4  // 
#define BUTTON_PREV             5  // 

LiquidCrystal_I2C  lcd(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);
SoftwareSerial portOne(2, 3);

int menuItem = 0;

float voltage, pHvoltage, pH, pHlcd, pHStateL, pHStateH, Temp, TempStateL, TempStateH;
float VOUT_RREF0, VOUT_RREF1;
float VOUT_PtRES, I_true, Vos;
float PtRES_calculated;
float RREF;

int ReadSensors = 0;
unsigned int minutes = 0;
int incomingByte = 0;


bool ProbeTest = false;

float IsoP;
float AlphaL;
float AlphaH;

//float PtRES_nominal;
//float alpha = 0.38;
int TempManual = 25;
int LastPHProbeTest;


long LogProbe;
bool TempMode;
bool CalTube;
bool Set = false;
float maxPh = pH;
float minPh = pH;
int HealthPHE;
int logprobeSelect;
int flag;
int cal = 8;

unsigned long CointError, CointFine, CointRepair, TestProbeResult, NumCheck = 0;

char* selectStrings[] = {"pH", "t ", "Level  ", " pH Min ", " pH Max ", "Interval", "One Dose", " Log"};
char* calStrings[] = {"Cal.Pump", " Cal.pH ", " Cal.pH ", " Cal.pH ", " Cal.pH ", " Cal.pH ", "  Reset  ", "  Reset  ", "  Reset  ", " Bridge "};
String calnStrings[] = {"", "4.00   ", "6.86   ", "7.00   ", "9.18   ", "10.00  ", "cal.ph  ", "all     ", "log     ", "enable "};

char* logprobeStrings[] = {"Health: ", "Er:", "Rp:", "OK:"};
char* pumpStrings[] = {" Down", " Up "};
char * mode[2] = {"manual ", "auto   "};
char * Tmode[2] = {"manual ", "pt100  "};
char * Rmode[2] = {"Relay  ", "TopUp  "};
char* resultStrings[] = {"NotFound", "Replace!", "NeedCal.", "Probe OK"};


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

  //writeLMP91200(0xD680); // table 5, pt1000, VCM 1/4
  writeLMP91200(0xF680); // table 5, pt100, VCM 1/4
  //writeLMP91200(0xE880);


  Read_EE();


  timer.setInterval(900L, cicleRead);

  Serial.println(IsoP);
  Serial.println(AlphaL, DEC);
  Serial.println(AlphaH, DEC);
  Serial.println(PtRES_nominal);

}

struct MyObject {

  float IsoP;
  float AlphaL;
  float AlphaH;
  int LastPHProbeTest;

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
/*
  void contCheck()
  {
  if (s >= 1)
  {
    s++;
    float phA;
    ADSread(0);
    if (s == 8)
      writeLMP91200(0x0640);
    phA = IsoP - AlphaH * (T + Temp) * voltage;
    if (phA > maxPh) maxPh = phA;
    if (phA < minPh) minPh = phA;
    if (s == 32)
      writeLMP91200(0x0240);
    if (i >= 70)
      continue;
  }
  }
*/

/*
  void PHETEST()
  {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test ");
  lcd.blink();
  maxPh = pH;
  minPh = pH;
  ReadSensors = -12;
  writeLMP91200(0x0040);
  delay(250);
  writeLMP91200(0x0240);
  for (int i = 0; i < 96; i++) {
    delay(5);
    ADSread(0);
    float phA = IsoP - AlphaH * (T + Temp) * voltage;
    if (phA > maxPh) maxPh = phA;
    if (phA < minPh) minPh = phA;
    if (i == 32) writeLMP91200(0x0340);
    if (i == 64) writeLMP91200(0x0640);
    //lcd.setCursor(5, 0);
    //lcd.print(i);
  }

  long mP = maxPh * 100;
  long mL = minPh * 100;
  HealthPHE = mP % mL;
  //HealthPHE /= 2;

  HealthPHE = map(HealthPHE, 0, 150, 0, 100);

  //HealthPHE = map(voltage, voltageMaxPh, voltageMinPh, 0, 99); Ok
  if (HealthPHE < 1 || HealthPHE > 200)
  {
    //terminal.println("pH electrode is not found");
    CointError++;
    TestProbeResult = 0;
    HealthPHE = 0;
    PHMode = 0;
  }
  else
  {
    HealthPHE = min(HealthPHE, 100);

    if (HealthPHE == constrain(HealthPHE, 1, 15)) {
      //terminal.println("pH electrode need to replace");
      CointRepair++;
      TestProbeResult = 1;
      PHMode = 0;
    }
    else if (HealthPHE > (LastPHProbeTest + 50) || HealthPHE < (LastPHProbeTest - 50) ) {
      //terminal.println("pH electrode need to calibrate");
      // HealthPHE = min(HealthPHE, 100);
      CointRepair++;
      TestProbeResult = 2;
      PHMode = EEPROM.read(0);
    }
    else {
      //terminal.println("pH electrode is Ok");
      //   HealthPHE = min(HealthPHE, 100);
      CointFine++;
      TestProbeResult = 3;
      PHMode = EEPROM.read(0);
    }
  }
  //Blynk.virtualWrite(V6, String(trselectStrings[prmSelect]) + PHMIN + " Save");
  NumCheck++;
  writeLMP91200(0xF680);
  //writeLMP91200(0xD680);
  ReadSensors = -10;

  lcd.noBlink();
  if (HealthPHE < 100)
    lcd.setCursor(5, 0);
  else
    lcd.setCursor(4, 0);
  lcd.print(HealthPHE);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print(resultStrings[TestProbeResult]);
  delay(500);
  }

*/

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

void showResults ()
{
  Serial.println("  ");
  Serial.print("  Temp ");
  Serial.print(Temp, 2);
  Serial.print(" *C ");
  Serial.print("  pH ");
  Serial.print(pH);
  Serial.print("  VOUT_PtRES ");
  Serial.print(VOUT_PtRES);
  Serial.print("  VOUT_RREF0 ");
  Serial.print(VOUT_RREF0);
  Serial.print("  VOUT_RREF1 ");
  Serial.println(VOUT_RREF1);
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
      //writeLMP91200(0xCE80); // table 6, pt1000
      writeLMP91200(0xEE80); // table 6, pt100
      //Serial.println(VOUT_RREF0);
      break;

    case 2: // VOUT_RREF1

      ADSread(2); // pt1000 ADSread(3);
      VOUT_RREF1 = voltage;
      //writeLMP91200(0x8E80); // table 7, pt1000, VCM 1/4
      writeLMP91200(0xAE80); // table 7, pt100, VCM 1/4
      //Serial.println(VOUT_RREF1);
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
      //Serial.println(Temp);
      break;

    case 4:

      if (-20 > Temp || Temp > 200) {
        Temp = TempManual;
      }

      ADSread(0);
      pHvoltage = voltage;
      if (pHvoltage > 0)
        pH = IsoP - AlphaL * (T + Temp) * pHvoltage;
      else if (pHvoltage < 0)
        pH = IsoP - AlphaH * (T + Temp) * pHvoltage;

      //writeLMP91200(0xD680); // table 5, pt1000, VCM 1/4
      writeLMP91200(0xF680); // table 5, pt100, VCM 1/4
      break;

    case 5:
      showResults ();
      break;
  }
  ReadSensors ++;
}

void cal_sensors()
{
  Serial.println(" ");
  if (incomingByte == 56) // press key "8"
  {
    Serial.print("Reset pH ...");
    IsoP = 7.5099949836;
    AlphaL = 0.0778344535;
    AlphaH = 0.0850976657;
  }
  else if (incomingByte == 52) // press key "4"
  {
    Serial.print("Cal. pH 4.00 ...");
    AlphaL = (IsoP - 4) / pHvoltage / (T + Temp);
  }
  else if (incomingByte == 55) // press key "7"
  {
    Serial.print("Cal. pH 7.00 ...");
    IsoP = (IsoP - pH + 7.00);
  }
  else if (incomingByte == 57) // press key "9"
  {
    Serial.print("Cal. pH 10.00 ...");
    AlphaH = (IsoP - 10.00) / pHvoltage / (T + Temp);
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

