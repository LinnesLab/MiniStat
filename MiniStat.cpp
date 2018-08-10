/*
 * FILENAME:    MiniStat.cpp
 * AUTHOR:      Orlando S. Hoilett
 * EMAIL:       ohoilett@purdue.edu
 * VERSION:     0.0.0.p
 *
 *
 * DISCLAIMER
 * Copyright (C) Linnes Lab 2016, Purdue University, All rights reserved
 *
 */


#include "MiniStat.h"

const uint16_t opVolt = 3300;
const uint16_t resolution = 1023;
const uint8_t MENB = 2;


//DEFAULT CONSTRUCTOR
//MiniStat::MiniStat()
MiniStat::MiniStat()
{
}


//void MiniStat::initialize()
//
//Initializes private objects. This method only needs to be called once in the
//program and should be called in the setup() function in the Arduino software.
void MiniStat::initialize()
{
    pStat = LMP91000();
    memory = EEPROM_24C64A();
    dac = DAC_MCP49xx(DAC_MCP49xx::MCP4922, 10); //int SS_PIN = 10;
    channel = Mux(channel_Ctrl);
    
    pStat.setMENB(pStat_Ctrl);
}


//void MiniStat::begin()
//
//Wakes all devices from sleep mode.
void MiniStat::begin(uint16_t mV, uint8_t MENB, uint8_t resolution = 10)
{
	//wakes up ATmega328
    sleep_disable();

	delay(100);

    
    //wakes up LMP91000
    pStat.setMENB(pStat_Ctrl);

	delay(50);
	pStat.standby();


	delay(50);
	pStat.disableFET();
    pStat.setThreeLead();
    delay(1000);


    //initiates MCP4922
    dac.setSPIDivider(SPI_CLOCK_DIV16);
    dac.setPortWrite(true);
    dac.setAutomaticallyLatchDual(false);
}


void MiniStat::setReferenceVoltge(uint16_t mV)
{
	refmilliVolts = mV;
}


uint16_t MiniStat::getReferenceVoltge() const
{
	return refmilliVolts;
}


void MiniStat::setResolution(uint8_t bits)
{
	resolution = bits;
}


uint8_t MiniStat::getResolution() const
{
	resolution;
}


//void MiniStat::sleep()
//
//Places all devices in sleep mode for power conservation.
void MiniStat::sleep()
{
    dac.shutdown();
    pStat.enableFET();
    pStat.sleep();
    //set_sleep_mode(SLEEP_MODE_PWR_SAVE); //SLEEP_MODE_PWR_DOWN
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    delay(100);
    sleep_enable();
    delay(100);
    sleep_mode();
}


//void MiniStat::setTarget(uint8_t target) const
//
//@param            target: Mux channel to open
//
//Sets the target of the potentiostat to the reference electrodes or measurement
//electrodes.
void MiniStat::setTarget(uint8_t target) const
{
    channel.open(target);
}


//void MiniStat::runRef() const
//
//Sets the target of the potentiostat to the reference electrodes.
void MiniStat::runRef() const
{
    setTarget(REFERENCE);
}


//void MiniStat::runExp() const
//
//Sets the target of the potentiostat to the measurement electrodes.
void MiniStat::runExp() const
{
    setTarget(EXPERIMENTAL);
}



//void MiniStat::runCV(uint8_t scan_rate, int high_limit, int low_limit) const
//void MiniStat::runCV(double cycles, uint16_t scan_rate)
//
//@param            cycles: number of cycles for the CV sweep
//
//@param            scan_rate: delay between mesurements
//
//Runs cyclic voltammetry protocol. Sets the digital to analog converter to provide
//a 1/2 Vdd reference voltage. The LMP91000 is then programmed to increment the
//bias on the reference electrode form 0 to 0.24 times the reference voltage.
void MiniStat::runCV(uint8_t user_gain, uint8_t cycles, uint16_t rate,
					 int16_t startV, int16_t endV)
{
	pStat.disableFET();
	pStat.setGain(user_gain);
	pStat.setRLoad(0);
	pStat.setIntRefSource();
	pStat.setIntZ(1);
	pStat.setThreeLead();
	
	startV = determineLMP91000Bias(startV);
	endV = determineLMP91000Bias(endV);
	
	if(v1 < 0) pStat.setNegBias();
	else pStat.setPosBias();
	
	
	for (uint8_t i = 0; i < cycles; i++)
	{
		pstat.setNegBias();
		for (int j = 1; j <= 11; j++)
		{
			pstat.setBias(j);
			//delay(50);
			delay(settling_time);
			Serial.print(j*-1);
			Serial.print(",");
			delay(1);
			Serial.println(analogRead(A0));
			delay(rate);
		}
		for (int j = 10; j >= 0; j--)
		{
			pstat.setBias(j);
			//delay(50);
			delay(settling_time);
			Serial.print(j*-1);
			Serial.print(",");
			delay(1);
			Serial.println(analogRead(A0));
			delay(rate);
		}
		pstat.setPosBias();
		for (int j = 1; j <= 11; j++)
		{
			pstat.setBias(j);
			//delay(50);
			delay(settling_time);
			Serial.print(j*1);
			Serial.print(",");
			delay(1);
			Serial.println(analogRead(A0));
			delay(rate);
		}
		for (int j = 10; j >= 0; j--)
		{
			pstat.setBias(j);
			//delay(50);
			delay(settling_time);
			Serial.print(j*1);
			Serial.print(",");
			delay(1);
			Serial.println(analogRead(A0));
			delay(rate);
		}
	}
	pstat.setBias(0);
	
	
	
//    //for (int i = 0; i < 50; i++) //for acid clean
//    for (uint8_t i = 0; i < 2*cycles; i++) //for measurements
//    {
//        uint8_t polarity = 0; // Just positive or negative
//
//        if (i%2 == 0)
//        {
//            pStat.setNegBias();
//            polarity = -1;
//        }
//        else
//        {
//            pStat.setPosBias();
//            polarity = 1;
//        }
//
//        //for (int j = 1; j <= 13; j++)
//        for (uint8_t j = 1; j <= 9; j++)
//        {
//            method(j, scan_rate, polarity);
//            //Serial.print("j: ");
//            //Serial.println(j);
//        }
//
//        //for (int k = 12; k >= 0; k--)
//        for (uint8_t k = 8; k >= 0; k--)
//        {
//            method(k, scan_rate, polarity);
//            //Serial.print("k: ");
//            //Serial.println(k);
//        }
//    }
}


//void MiniStat::method(uint8_t bias, uint16_t scan_rate, int polarity)
//
//
void MiniStat::method(uint8_t bias, uint16_t scan_rate, int polarity)
{
    pStat.setBias(bias);
    delay(scan_rate);
    
    long voltage = polarity*bias_incr[bias]*ADC_REF*1000;
//    memory.write(memory.getCurReg(), (voltage & 0xFF));
//    memory.write(memory.getCurReg(), ((voltage >> 8) & 0xFF));
    
	uint16_t adcVal = pStat.getOutput(pStat_Sensor);
	long current = pStat.getCurrent(adcVal, ADC_REF, ADC_BITS) *pow(10, 8);
//    memory.write(memory.getCurReg(), (current & 0xFF));
//    memory.write(memory.getCurReg(), ((current >> 8) & 0xFF));
    
    Serial.print(voltage);
    Serial.print(",");
    Serial.print(adcVal);
    Serial.print(",");
    Serial.println(-current);
    
    
    //Serial.println(millis());

//    Serial.print(current, DEC);
//    Serial.print(": ");
//    Serial.println((current), BIN);
//    Serial.print(current & 0xFF, BIN);
//    Serial.print(", ");
//    Serial.println((current >> 8) & 0xFF, BIN);
//
//    Serial.print(voltage, DEC);
//    Serial.print(": ");
//    Serial.println((voltage), BIN);
//    Serial.print(voltage & 0xFF, BIN);
//    Serial.print(", ");
//    Serial.println((voltage >> 8) & 0xFF, BIN);
//    Serial.println();
//    Serial.println();

    
//    Serial.print((current & 0xFF), BIN);
//    Serial.print(", ");
//    Serial.print(((current >> 8) & 0xFF), BIN);
//    Serial.print(", ");
//    Serial.println(current, BIN);
//    
//    Serial.print((((int)(polarity*bias_incr[bias]*ADC_REF*1000))&0xFF), BIN);
//    Serial.print(", ");
//    Serial.print(((((int)(polarity*bias_incr[bias]*ADC_REF*1000)) >> 8) & 0xFF), BIN);
//    Serial.print(", ");
//    Serial.println((((int)(polarity*bias_incr[bias]*ADC_REF*1000))), BIN);
    
}



//doesn't work for pulse_widths less than 1ms
//still need to figure out where to sample
//the input parameters are in microseconds
//the delays are done in milliseconds, but may need to use microseconds for more accuracy

//void MiniStat::runNPV(uint8_t user_gain, uint8_t cycles, uint8_t freq,
//					  unsigned long pulse_width, int16_t startV, int16_t endV,
//					  int16_t stepV, uint8_t pulse_per_cycle)
//
//user_gain			gain of the LMP91000
//
//cycles			how many times the series of pulses will be applied
//
//pulse_period		how long each pulse lasts, the sum of the on time and the
//						off time (also 1/frequency) (in microseconds)
//
//pulse_width		how long each pulse is applied (in microseconds)
//
//startV			user set start voltage (in milliVolts)
//
//endV				user specfied end voltage (in milliVolts)
//
//stepV				user specified step voltage (currently not being used)
//						(in milliVolts)
//
//pulse_per_cycle	how many pulses are applied to get from startV to endV.
//						this is currently limited by the resolution of the bias
//						generator of the LMP91000
//
//This method runs a Normal Pulse Voltammetry sweep. This is easier to explain
//using pictures intead of with words. Bioanalytical Sciencies explains it
//fairly well, however. "The potential wave form consists of a series of pulses
//of increasing amplitude, with the potential returning to the initial value
//after each pulse."
//<https://www.basinc.com/manuals/EC_epsilon/Techniques/Pulse/pulse#normal>
void MiniStat::runNPV(uint8_t user_gain, uint8_t cycles, unsigned long pulse_period,
					  unsigned long pulse_width, int16_t startV, int16_t endV,
					  int16_t stepV, uint8_t pulse_per_cycle)
{
	pStat.disableFET();
	pStat.setGain(user_gain);
	pStat.setRLoad(0);
	pStat.setIntRefSource();
	pStat.setIntZ(1);
	pStat.setThreeLead();
	
	pStat.setBias(0);
	if(endV < 0) pStat.setNegBias();
	else pStat.setPosBias();
	
	long off_time = pulse_period - pulse_width;
	long pulse_width_in_ms = pulse_width/1000; //pulse-width in milliseconds
	long off_time_in_ms = off_time/1000; //off time in milliseconds
	//long off_time_in_us = ; //off time in microseconds
	
	uint8_t pulses = abs((endV-startV)/(3300*0.02));
	
	for (uint8_t i = 0; i < cycles; i++)
	{
		for (uint8_t j = 0; j < pulses; j++)
		{
			unsigned long startTime = millis();
			pStat.setBias(0);
			while(millis() - startTime < off_time_in_ms);
			
			startTime = millis();
			pStat.setBias(j);
			while(millis() - startTime < pulse_width_in_ms)
			{
				Serial.print(millis());
				Serial.print(",");
				Serial.println(pStat.getOutput(pStat_Sensor));
			}
		}
		pStat.setBias(0);
	}
}


//void MiniStat::runDPV(uint8_t user_gain, uint8_t cycles, unsigned long pulse_period,
//						unsigned long pulse_width, int16_t startV, int16_t endV,
//						int16_t stepV, uint8_t pulse_per_cycle)
//
//user_gain			gain of the LMP91000
//
//cycles			how many times the series of pulses will be applied
//
//pulse_period		how long each pulse lasts, the sum of the on time and the
//						off time (also 1/frequency) (in microseconds)
//
//pulse_width		how long each pulse is applied (in microseconds)...same as
//						"on time"
//
//startV			user set start voltage (in milliVolts)
//
//endV				user specfied end voltage (in milliVolts)
//
//stepV				user specified step voltage (currently not being used)
//						(in milliVolts)
//
//pulse_per_cycle	how many pulses are applied to get from startV to endV.
//						this is currently limited by the resolution of the bias
//						generator of the LMP91000
//This method runs a Differential Pulse Voltammetry sweep. This is easier to explain
//using pictures intead of with words. Bioanalytical Sciencies explains it
//fairly well, however. "The potential wave form consists of small pulses (of
//constant amplitude) superimposed upon a staircase wave form. Unlike NPV, the
//current is sampled twice in each Pulse Period (once before the pulse, and at
//the end of the pulse), and the difference between these two current values is
//recorded and displayed."
//<https://www.basinc.com/manuals/EC_epsilon/Techniques/Pulse/pulse#normal>
void MiniStat::runDPV(uint8_t user_gain, uint8_t cycles, unsigned long pulse_period,
					  unsigned long pulse_width, int16_t startV, int16_t endV,
					  int16_t stepV, uint8_t pulse_per_cycle)
{
	pStat.disableFET();
	pStat.setGain(user_gain);
	pStat.setRLoad(0);
	pStat.setIntRefSource();
	pStat.setIntZ(1);
	pStat.setThreeLead();

	
	long off_time = pulse_period - pulse_width;
	long pulse_width_in_ms = pulse_width/1000; //pulse-width in milliseconds
	long off_time_in_ms = off_time/1000; //off time in milliseconds
	//long off_time_in_us = ; //off time in microseconds
	
	uint8_t pulses = abs((endV-startV)/(3300*0.02));
	
	for (uint8_t i = 0; i < cycles; i++)
	{
		for (uint8_t j = 0; j < pulses-3; j++)
		{
			Serial.print(millis());
			Serial.print(",");
			Serial.println(pStat.getOutput(pStat_Sensor));
			
			unsigned long startTime = millis();
			pStat.setBias(j+2);
			while(millis() - startTime < off_time_in_ms);

			
			Serial.print(millis());
			Serial.print(",");
			Serial.println(pStat.getOutput(pStat_Sensor));
			
			startTime = millis();
			pStat.setBias(j+1);
			while(millis() - startTime < pulse_width_in_ms);
		}
		
		pStat.setBias(0);
	}
	
}



//void MiniStat::runSWV(uint8_t user_gain, uint8_t cycles, uint16_t startV,
//						uint16_t endV, int pulse_amp, uint16_t volt_step, uint16_t freq)
//
//user_gain			gain of the LMP91000
//
//cycles			how many times the series of pulses will be applied
//
//startV			user set start voltage (in milliVolts)
//
//endV				user specfied end voltage (in milliVolts)
//
//pulse_amp
//
//freq				the frequency of the square wave (in Hertz)
//
//This method runs a Square Wave Voltammetry sweep. This is easier to explain
//using pictures intead of with words. Bioanalytical Sciencies explains it
//fairly well, however:
//
//"The potential wave form consists of a square wave of constant amplitude
//superimposed on a staircase wave form. The current is measured at the end of
//each half-cycle, and the current measured on the reverse half-cycle (ir) is
//subtracted from the current measured on the forward half-cycle (if). This
//difference current (if - ir) is displayed as a function of the applied potential."
//<https://www.basinc.com/manuals/EC_epsilon/Techniques/Pulse/pulse#normal>
//
void MiniStat::runSWV(uint8_t user_gain, uint8_t cycles, uint16_t startV,
					  uint16_t endV, int pulse_amp, uint16_t volt_step, uint16_t freq)
{
	pStat.disableFET();
	pStat.setGain(user_gain);
	pStat.setRLoad(0);
	pStat.setIntRefSource();
	pStat.setIntZ(1);
	pStat.setThreeLead();
	
	unsigned long pulse_width = (1000000/freq)/2;
	unsigned long pulse_width_in_ms = pulse_width/1000;
	unsigned long off_time_in_ms = pulse_width_in_ms;
	
	long off_time = pulse_period - pulse_width;
	long pulse_width_in_ms = pulse_width/1000; //pulse-width in milliseconds
	long off_time_in_ms = off_time/1000; //off time in milliseconds
	//long off_time_in_us = ; //off time in microseconds
	
	uint8_t pulses = abs((endV-startV)/(3300*0.02));
	
	for (uint8_t i = 0; i < cycles; i++)
	{
		for (uint8_t j = 0; j < pulses-3; j++)
		{
			unsigned long startTime = millis();
			pStat.setBias(j+3);
			while(millis() - startTime < off_time_in_ms);
			
			Serial.print(millis());
			Serial.print(",");
			Serial.println(pStat.getOutput(pStat_Sensor));
			
			startTime = millis();
			pStat.setBias(j+1);
			while(millis() - startTime < pulse_width_in_ms);
			
			Serial.print(millis());
			Serial.print(",");
			Serial.println(pStat.getOutput(pStat_Sensor));
		}
		
		pStat.setBias(0);
	}
	
}


////void MiniStat::runAMP(uint8_t user_gain, int16_t v1, int16_t v2, uint32_t t1,
////						uint32_t t2, uint32_t waitTime, uint16_t samples)
////
////user_gain			gain of the LMP91000
////
////v1				the first step potential applied (in milliVolts)
////
////v2				the second step potential applied (in milliVolts)
////
////t1				the length of time the first step is applied (in milliseconds)
////
////t2				the length of time the second step is applied (in milliseconds)
////
////waitTime			the period of time (in milliseconds) before the first
////						voltage is applied
////
////samples			the amount of measurements taken during each pulse period
////
//void MiniStat::runAmp(uint8_t user_gain, int16_t v1, int16_t v2, uint32_t t1,
//					  uint32_t t2, uint32_t quietTime, uint16_t samples)
//{
//	pStat.disableFET();
//	pStat.setGain(user_gain);
//	pStat.setRLoad(0);
//	pStat.setIntRefSource();
//	pStat.setIntZ(1);
//	pStat.setThreeLead();
//
//	uint32_t fs = t1/samples;
//
//	Serial.println("Voltage,Time(ms),Current");
//
//	v1 = determineLMP91000Bias(v1);
//
//	delay(quietTime);
//
//	if(v1 < 0) pStat.setNegBias();
//	else pStat.setPosBias();
//
//	unsigned long startTime = millis();
//	pStat.setBias(v1);
//	while(millis() - startTime < t1)
//	{
//		Serial.print((uint16_t)(opVolt*TIA_BIAS[v1]*(v1/abs(v1))));
//		Serial.print(",");
//		Serial.print(millis());
//		Serial.print(",");
//		Serial.println(pStat.getOutput(A0));
//		delay(fs);
//	}
//
//	fs = t2/samples;
//	v2 = determineLMP91000Bias(v2);
//
//	if(v2 < 0) pStat.setNegBias();
//	else pStat.setPosBias();
//
//	startTime = millis();
//	pStat.setBias(v2);
//	while(millis() - startTime < t2)
//	{
//		Serial.print((uint16_t)(opVolt*TIA_BIAS[v2]*(v2/abs(v2))));
//		Serial.print(",");
//		Serial.print(millis());
//		Serial.print(",");
//		Serial.println(pStat.getOutput(A0));
//		delay(fs);
//	}
//
//	pStat.setBias(0);
//}



//range = 12 is picoamperes
//range = 9 is nanoamperes
//range = 6 is microamperes
//range = 3 is milliamperes
void runAmp(uint8_t user_gain, int16_t pre_stepV, uint32_t quietTime, int16_t v1,
			uint32_t t1, int16_t v2, uint32_t t2, uint16_t samples, uint8_t range)
{
	pStat.disableFET();
	pStat.setGain(user_gain);
	pStat.setRLoad(0);
	pStat.setIntRefSource();
	pStat.setIntZ(1);
	pStat.setThreeLead();
	
	//Print column headers
	String current = "";
	if(range == 12) current = "Current(pA)";
	else if(range == 9) current = "Current(nA)";
	else if(range == 6) current = "Current(uA)";
	else if(range == 3) current = "Current(mA)";
	else current = "SOME ERROR";
	
	Serial.println("Voltage(mV),Time(ms)," + current);
	
	int16_t voltageArray[3] = {pre_stepV, v1, v2};
	int16_t timeArray[3] = {quietTime, t1, t2};
	
	//i = 0 is pre-step voltage
	//i = 1 is first step potential
	//i = 2 is second step potential
	for(uint8_t i = 0; i < 3; i++)
	{
		//For pre-step voltage
		uint32_t fs = timeArray[i]/samples;
		voltageArray[i] = determineLMP91000Bias(voltageArray[i]);
		
		if(voltageArray[i] < 0) pStat.setNegBias();
		else pStat.setPosBias();
		
		unsigned long startTime = millis();
		pStat.setBias(voltageArray[i]);
		while(millis() - startTime < timeArray[i])
		{
			Serial.print((uint16_t)(opVolt*TIA_BIAS[voltageArray[i]]*(voltageArray[i]/abs(voltageArray[i]))));
			Serial.print(",");
			Serial.print(millis());
			Serial.print(",");
			Serial.println(pow(10,range)*pStat.getCurrent(pStat.getOutput(A0), opVolt/1000.0, resolution));
			delay(fs);
		}
	}
	
	//End at 0V
	pStat.setBias(0);
}


signed char MiniStat::determineLMP91000Bias(int16_t voltage)
{
	signed char polarity = 0;
	if(voltage < 0) polarity = -1;
	else polarity = 1;
	
	int16_t v1 = 0;
	int16_t v2 = 0;
	
	voltage = abs(voltage);
	
	if(voltage == 0) return 0;
	
	for(int i = 0; i < NUM_TIA_BIAS-1; i++)
	{
		v1 = opVolt*TIA_BIAS[i];
		v2 = opVolt*TIA_BIAS[i+1];
		
		if(voltage == v1) return i;
		else if(voltage > v1 && voltage < v2)
		{
			if(abs(voltage-v1) < abs(voltage-v2)) return polarity*i;
			else return polarity*i+1;
		}
	}
	return 0;
}


int MiniStat::getPolarity(int volt)
{
	int polarity = 0;
	if (volt >= 0)
	{
		polarity = 1;
		pStat.setPosBias();

	}
	else {
		polarity = -1;
		pStat.setNegBias();
	}

	return polarity;
}



/*
 *
 * MiniStat program structure
 *
 *
 
 volatile unsigned long counter = 0;
 const unsigned long 2hrs = ___;
 
 //500 Hz
 TCCR2A = 0;
 TCCR2B = 0;
 TCNT2  = 0;
 OCR2A = 249;// = (8*10^6) / (500*64) - 1 (must be <256)
 TCCR2A |= (1 << WGM21);
 TCCR2B |= (1 << CS22);
 TIMSK2 |= (1 << OCIE2A);
 
 
 
every two hours
{
    1. Wake up from sleep
    a. Wake up microcontroller        sleep_disable();
    b. Wake LMP91000                  LMP91000.setThreeLead(); LMP91000.disableFET();
    c. Give LMP91000 time to warm up  delay(2000);
    d. Wake up MCP4922                dac.output();
    2. Measure reference
    a. Set muxes to reference         mux.open(reference);
    b. CV, ACV or Pulse V protocol    LMP91000.runTest();
    3. Store data                       myEEPROM.write(voltage); myEEPROM.write(current);
    
    myEEPROM.write(myEEPROM.getCurReg()+1, (int)(1*bias_incr[i]*3.3*100));
    
    4. Measure experimental
    a. Set muxes to experimental      mux.open(experimental);
    b. CV, ACV or Pulse V protocol    LMP91000.runTest();
    5. Store data                       myEEPROM.write(voltage); myEEPROM.write(current);
    6. Run statistical analysis         runTtest();
    7. Stores result                    myEEPROM.write(result); myEEPROM.write(p-value);
    8. Go back to sleep
    a. Shut down MCP4922              dac.shutdown();
    b. Shutdown LMP91000              LMP91000.enableFET(); LMP91000sleep();
    c. Shutdown ATmega                set_sleep_mode(SLEEP_MODE_PWR_SAVE);
}


How to handle serial connection


Sending data over Bluetooth
*
*
*/


