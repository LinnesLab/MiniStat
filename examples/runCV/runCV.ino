#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "avr/interrupt.h"
#include "avr/power.h"
#include "avr/sleep.h"
#include "avr/io.h"

#include "MiniStat.h"
#include "LMP91000.h"
#include "EEPROM_24C64A.h"
#include "DAC_MCP49xx.h"
#include "Mux.h"
#include "Waveforms.h"


MiniStat miniStat = MiniStat();
//MiniStat miniStat = MiniStat();

boolean active = false;
unsigned long startTime = 0;

void setup()
{
  Wire.begin();
  delay(10);
  Serial.begin(115200);
  delay(10);
  while(!Serial.available());
  miniStat.initialize();
  miniStat.begin();
  //miniStat.runExp();
  miniStat.runRef();
  miniStat.runCV(2,4,50);
  //miniStat.print();

}

void loop()
{

  
}

