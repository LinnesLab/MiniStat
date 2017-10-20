/*
 * FILENAME:    MiniStat.h
 * AUTHOR:      Orlando S. Hoilett
 * EMAIL:       ohoilett@purdue.edu
 * VERSION:     0.0.0.p
 
 
 * DESCRIPTION
 
 
 
 * A FEW INSTRUCTIONS
 
 
 * UPDATES
 * Version 0.0
 * 2015/02/11:2220>
 *			Initialization of code development.

 
 * DISCLAIMER
 * Copyright (C) Linnes Lab 2016, Purdue University, All rights reserved
 *
 */


#ifndef MINISTAT_H
#define MINISTAT_H
//typedef unsigned short uint8_t;
//typedef unsigned int uint16_t;
//typedef unsigned long uint32_t;
#include "Arduino.h"
#include "Wire.h"
#include "SPI.h"

/*#include "avr/interrupt.h"
#include "avr/power.h"
#include "avr/sleep.h"
#include "avr/io.h"*/

#include "Writetest.h"
#include "LMP91000.h"
#include "EEPROM_24C64A.h"
#include "DAC_MCP49xx.h"
#include "Waveforms.h"
#include "Mux.h"

//#include "cstdint.h"



const uint8_t pStat_Ctrl= 4; //Arduino Digital Pin 4
const uint8_t pStat_Sensor = A0; //Arduino Analog In pin 0
const uint8_t channel_Ctrl = 3; //Arduino Digital Pin 3

//uint16_t scan_rate = 50;
//uint8_t gain = 3; //for acid clean
//uint8_t tiaGain = 6; //for measurements
const double gain_resistors[] = {2750,3500,7000,14000,35000,120000,350000};
const int sensor = A0;
const double bias_incr[] = {0, 0.01, 0.02, 0.04, 0.06, 0.08,
    0.1,0.12, 0.14, 0.16, 0.18, 0.2, 0.22, 0.24};

const uint8_t REFERENCE = 0;
const uint8_t EXPERIMENTAL = 1;

const double ADC_REF = 3.36;
const double ADC_BITS = 10;

const uint8_t sineWave = 0;
const uint8_t triWave = 1;


class MiniStat {
    
private:
    LMP91000 pStat;
    EEPROM_24C64A memory;
    DAC_MCP49xx dac;
    Mux channel;
	Writetest wtest;
    
    
public:
    
    //CONSTRUCTORS
    MiniStat();
    
    void initialize();
    void begin();
    void sleep();
    
    void setTarget(uint8_t target) const;

    void runRef() const; //runRef
    void runExp() const; //runExp
    
    //void runCV(double cycles, uint16_t scan_rate); //for debug purposes
    void runCV(uint8_t user_gain, uint8_t cycles, uint16_t scan_rate);
    void runACV(uint8_t user_gain, uint8_t cycles, uint16_t scan_rate, uint16_t startV, uint16_t endV, uint16_t amplitude, uint16_t freq);
    void runPulseV(uint8_t user_gain, uint8_t cycles, uint16_t frequency, uint16_t pulse_width, int pulse_amplitude, uint8_t pulse_per_cycle);
	void runSWV(uint8_t user_gain, uint8_t cycles, uint16_t startV, uint16_t endV,  int pulse_amp, uint16_t volt_step, uint16_t pulse_freq);
	void runDPV(uint8_t user_gain, uint8_t cycles, uint16_t startV, uint16_t endV, int step_size, int pulse_amp, uint16_t sample_period, uint16_t pulse_freq);
	void method(uint8_t bias, uint16_t scan_rate, int polarity);
	void runAMP(uint16_t user_gain, int voltage, uint16_t time, int samples);
    void save();
	int calcDACValue(int  vout);
    void print();
	int calcCurrent(uint16_t voltage,  int polarity, bool print, int bias);
	int getPolarity(int voltage);
	void WTest();
    
};

#endif