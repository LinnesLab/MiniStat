/*
 * FILENAME:    MiniStat.cpp
 * AUTHOR:      Orlando S. Hoilett, Nicholas J. Jaras
 * EMAIL:       ohoilett@purdue.edu, njaras@purdue.edu
 * VERSION:     0.0.0.p
 *
 *
 * DISCLAIMER
 * Copyright (C) Linnes Lab 2017, Purdue University, All rights reserved
 *
 */


#include "MiniStat.h"


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
void MiniStat::begin()
{
    //wakes up ATmega328
 //   sleep_disable();
    
    //wakes up LMP91000
//   // pStat.setMENB(pStat_Ctrl);
   pStat.standby();
   pStat.disableFET();
    pStat.setThreeLead();

    delay(1000);


    //initiates MCP4922
    dac.setSPIDivider(SPI_CLOCK_DIV16);
	
    dac.setPortWrite(true);
	
    dac.setAutomaticallyLatchDual(false);
	

	

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
 //   set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    delay(100);
//    sleep_enable();
    delay(100);
//    sleep_mode();
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
void MiniStat::runCV(uint8_t user_gain, uint8_t cycles, uint16_t scan_rate)
{


    //dac.outputA(4095); // this is for Nick
    dac.outputA(0);
    dac.outputB(2048);  //Disables them, sets it up as a triangle wave

	//originally 0, 2048
    delay(10);
    pStat.setGain(user_gain);
    pStat.setRLoad(0);
    pStat.setExtRefSource();
    pStat.setIntZ(1);
	
    int current = 0;  //Not needed 
    uint8_t index = 0;
    
    //for (int i = 0; i < 50; i++) //for acid clean
    for (int i = 0; i < 2*cycles; i++) //for measurements
    {
        int polarity = 0; // Just positive or negative
        
        if (i%2 == 0)
        {
			pStat.setNegBias();
			polarity = -1; 

		}
        else
        {
			
			pStat.setPosBias();
			polarity = 1;

			
        }
        
        for (int j = 1; j <= 10; j++)
        {
            method(j, scan_rate, polarity);
            //Serial.print("j: ");
            //Serial.println(j);
        }
        
        for (int k = 9; k >= 0; k--)
        {
            method(k, scan_rate, polarity);
            //Serial.print("k: ");
            //Serial.println(k);
        }
    }

	//dac.outputA(0);
	//dac.outputB(0);
            
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


void MiniStat::runACV(uint8_t user_gain, uint8_t cycles, uint16_t scan_rate, uint16_t startV, uint16_t endV, uint16_t amplitude, uint16_t freq)
{
    //pStat.setTwoLead();
    pStat.setGain(user_gain);
    pStat.setRLoad(0);
    pStat.setExtRefSource();
    pStat.setIntZ(1);
//    pStat.setPosBias();
    pStat.setNegBias();
    pStat.setBias(13);
    delay(1000); //warm-up time for the gas sensor
    int current = 0;
    uint8_t index = 0;
    
    
    unsigned long time0 = millis();
    unsigned long sample = 100;
    int i = 0;
    int j = 0;
    int k = 0;
    
    
    uint32_t sum = 0;
    
    while (i > -1)
    {
        //Serial.println("hello there!");
        if((micros() - time0) > 50)
        {
            sum += analogRead(pStat_Sensor);
            i++;
            time0 = micros();
        
            if (i%(12-1) == 0)
            {
                double data = waveformsTable[sineWave][j];
                data = ((data - 2048.0)/25.0) + 2048.0;
                dac.outputB(data);
                j++;
            }
            if (i%(120-1) == 0)
            {
                double data = waveformsTable[triWave][k];
                data = ((data - 2048.0)*.8) + 2048.0;
                //data = ((data - 2048.0)*.6) + 1248.0;
                dac.outputA(data);
                k++;
            }
            
            if(j == maxSamplesNum-1) j = 0;
            if(k == maxSamplesNum-1)
            {
                k = 0;
               // i = 0;
            }
            
            //average(sum / max_samples);
            //current = getCurrent(pStat_Sensor, ADC_REF, ADC_BITS)*pow(10,8);
            //current = getCurrent(average, ADC_REF, ADC_BITS)*pow(10,8);
//            memory.write(memory.getCurReg(), (current & 0xFF));
//            memory.write(memory.getCurReg(), ((current >> 8) & 0xFF));
//            memory.write(memory.getCurReg(), (((int)(polarity*bias_incr[bias]*ADC_REF*1000))&0xFF));
//            memory.write(memory.getCurReg(), ((((int)(polarity*bias_incr[bias]*ADC_REF*1000)) >> 8) & 0xFF));
        }
    }
    
}

/*void MiniStat::runPulseV(uint8_t user_gain, uint8_t cycles, uint16_t frequency, uint16_t pulse_width, int pulse_amplitude, uint8_t pulse_per_cycle)
//
//
//@param            user_gain: the gain set by the user
//
  @Param			cycles: the number of cycles the voltammetry is done for
  @param			frequency: the frequency of the pulses

  @param			pulse_with: the width of the pulses active high time

  @param			pulse_amplitude: the maximum height of the pulses

  @param			pulse_per_cycle: the number of pulses in the cycle

//
  Runs the pulse voltammetry protocol. The DAC is changed to reflect the desired voltage value while the LMP91000 balance is set to a fixed maximum value.
  To increase the accuracy of the downtime between pulses, the wait is split into to milliseconds and microseconds. The pulses increase an even amount based on the number of pulses per cycle and the maximum amplitude
  The final pulse will be the maximum value. The current is taken at the falling edge of the pulse after it has had time to be active high
*/
void MiniStat::runPulseV(uint8_t user_gain, uint8_t cycles, uint16_t frequency, uint16_t pulse_width, int pulse_amplitude, uint8_t pulse_per_cycle)
{
    
	//sets both DAC values to zero and waits to ensure they are defaulted to zero
	dac.outputA(0);
	dac.outputB(0);  
	delay(10);

	pStat.setGain(user_gain);
	pStat.setRLoad(0);
	pStat.setExtRefSource();
	pStat.setIntZ(1);

	int current1 = 0; //The down time current for each calculations
	unsigned short volt = 0;  //The current voltage
	int polarity = 0; //The polarity the current
	int ms = 0;  //The milliseconds of wait time
	int us = 0; //The microseconds of wait time

	//Calculates the downtime between pulses, needs to convert to two seperate variables for the issue of accuracy
	int down_time = (1000000 / frequency - pulse_width);  //The downtime is the inverse of the frequency(The period of the wave in microseconds) and minus the pulse width
	ms = down_time / 1000; //Truncates off all but the milliseconds
	us = down_time - 1000 * ms;  // leaves just the microseconds

	//Prints out what the downtime is, this does not need to be kept
	Serial.print("Down time: ");
	Serial.println(down_time);




	for (int i = 1; i <= cycles; i++) //For the number of total cycles
	{
		for (int j = 1; j <= pulse_per_cycle; j++)	//For the number of pulses in the cycle
		{
			dac.outputB(0);	//set to zero to wait
			
			//create wait based on duty cycles
			delay(ms);
			delayMicroseconds(us); //Delay us can only do 16000, do two delays for more accuracy
			
			//The pulses increase evenly towards the max value on the last pulse of the cycle
			volt = (((double)j / pulse_per_cycle)) * calcDACValue(pulse_amplitude);
	
			dac.outputB(volt);

			//delays to the falling edge and then gets the voltage and current
			delayMicroseconds(pulse_width);
			polarity = getPolarity((((double)j / pulse_per_cycle)) * pulse_amplitude);
			current1 = calcCurrent((((double)j / pulse_per_cycle)) * pulse_amplitude, polarity, true,13);

			//{rintout the values

		}
		dac.outputB(0);//Default back to zero
	}
}
/*void MiniStat::runDPV(uint8_t user_gain, uint8_t cycles, int startV, int endV, int step_size, int pulse_amp, uint16_t sample_period, uint16_t pulse_freq)
//
//
//@param            user_gain: the gain set by the user
//
  @param		cycles: the number of cycles the voltammetry is done for

@param			starV: the frequency of the pulses

@param			endV: the width of the pulses active high time

@param			step_size: The size of the step increase to the next voltage value

@param			pulse_amp: The amplitude of the pulse for each voltage bias point

@param			sample_period: the time the pulse is being samples for

@param			pulse_freq: the frequency of each pulse

//
Runs the Different pulse  voltammetry protocol. The DAC is changed to reflect the desired voltage value while the LMP91000 balance is set to a fixed maximum value.
For the number of cycles given by the user, the voltage is ranged from the start vale to the end value, being increased by the step_size
The voltage is set to that base value and then waits till right before the voltage is pulsed. Right before this hapens, the crrent is take
The voltage is pulsed up by the amplitude and the another wait happens, the current is taken right at the falling edge of the pulse
The differene in these two currents is printed out
*/
void MiniStat::runDPV(uint8_t user_gain, uint8_t cycles, int startV, int endV, int step_size, int pulse_amp, uint16_t sample_period, uint16_t pulse_freq)
{

	
	//Defaults the output of the DAC to zero and waits to ensure it is changed
	dac.outputA(0);
	dac.outputB(0); 
	delay(10);

	pStat.setGain(user_gain);
	pStat.setRLoad(0);
	pStat.setExtRefSource();
	pStat.setIntZ(1);

	int current1 = 0; //The current of the wait before the pulse
	int current2 = 0;  //The current of the pulse
	int volt = 0;	//The current voltage
	int polarity = 0;  //Polarity of the pulse

	for (int i = 1; i <= cycles; i++)	//For the number of cycles being done
	{
		for (int j = startV; j <= endV; j += step_size)	//The voltage ranges form the start voltage and goes to the end voltage, havng the biased being increased from the step voltage
		{
			//Sets the dac output to the base voltage
			volt = calcDACValue(j);
			dac.outputB(volt);
			polarity = getPolarity(j);

			//wait for the prestep sample
			delay((1000 / pulse_freq)); 


			current1 = calcCurrent(j, polarity, false,13);  //Reads in the current for the resting low current

			//Increases the voltage the pulse amplitude value
			volt = calcDACValue(j + pulse_amp);
			dac.outputB(volt);
			polarity = getPolarity(j + pulse_amp);

			//wait till the end value of time
			delay(1000 / pulse_freq);

			current2 = calcCurrent(j + pulse_amp, polarity, false,13);  //Measures the current on the falling edge of the pulse
			
			//prints the base voltage value and the current difference
			Serial.print(j);
			Serial.print(",");
			Serial.println(-((double)(current2) - current1));  //Output is the current difference

			

		}
	}
	dac.outputB(0);	//Defaults the voltage back to zero

}


/*void MiniStat::runSWV(uint8_t user_gain, uint8_t cycles, int  startV, int endV,  int pulse_amp, int volt_step, uint16_t pulse_freq)
@param		user_gain: the gain set by the user
@param		cycles: the number of cycles to be compleated
@param		startV: starting votage of the cycles
@param		endV: The end voltage of the cycles
@param		pulse_amp: the amplitude of each pules
@param		volt_step: the voltage step for the bias point
@param		pulse_freq: the frequency of the pulse, used to calculate the period of the pulse

Protocal for the Square Wave Voltammerty. The bias point is set originally to the startV, and at each bias point, the voltage is pulesed up by the pulse_amp from the bias point.
The current is measured at the falling edge of the pulse. The voltage is then set to the bias point minus the pulse_amp, where again the current is measured at the falling edge.
The bias point is increase by the volt_step till it reaches the endV. The current is the difference of the two measured currents, and is printe out with the voltage.
*/
void MiniStat::runSWV(uint8_t user_gain, uint8_t cycles, int  startV, int endV,  int pulse_amp, int volt_step, uint16_t pulse_freq)
{

	//Sets the DAC outputs to zero at the start and delays
	dac.outputA(0);
	dac.outputB(0); 
	delay(10);
	
	pStat.setGain(user_gain);
	pStat.setRLoad(0);
	pStat.setExtRefSource();
	pStat.setIntZ(1);
	int volt = 0; //in code to send to the DAC
	int polarity = 0; //The polarity of the voltage / current

	int current1 = 0; //The pre step current
	int current2 = 0; //The current at the amplitude


	
	for (int i = 1; i <= cycles; i++)  //Looped for the number of cycles
	{
	
		volt = calcDACValue(startV); //Sets it to the DAC code value based on the voltage inputed
		polarity = getPolarity(volt);
		dac.outputB(volt); //Sets the voltage to the number given
		for (int j = startV; j <= endV; j += volt_step)		//Steps from the starting voltage to the end voltage increased by the volt_step
		{
			Serial.print(j);
			Serial.print(", ");  //Prints out the bias voltage

			//increase to J + pulse amp
			//Serial.print(j + pulse_amp);
			//Serial.print(", ");
			
			//Pulses up to the high pulese
			volt = calcDACValue(j + pulse_amp); //Base Value + Amplitude
			//Serial.println(volt);
			polarity = getPolarity(j + pulse_amp);
			dac.outputB(volt);

			
			delay(1000 / pulse_freq); //Added to accomidate for the wait time possible
			current1 = calcCurrent(j + pulse_amp,  polarity, false,13);  //Calculates the pules up current


			//Pulses down to the down pulse
			volt = calcDACValue(j - pulse_amp);
			polarity = getPolarity(j- pulse_amp);
			dac.outputB(volt);

			delay(1000 / pulse_freq);		//Delays to the falling edge to measure the current
			current2 = calcCurrent(j-pulse_amp,  polarity, false,13); //Calculates the pulsed down current
			//Find the difference in current and print it
//			Serial.print(j);
//			Serial.print(",");
			//Serial.print(adcVal);
			//serial.print(",");
			Serial.println(-((double)(current2) - current1)); //Prints out the difference in the currents


		}
		dac.outputB(0);//resets to zero
	}
	dac.outputA(0);//resets to zero
}
	


int MiniStat::calcCurrent(uint16_t voltage, int polarity, bool print, int bias)
{
	//Based on the method above written by Orlando

	//pStat.setBias(bias);
	//delay(scan_rate);
	pStat.setBias(bias);  //sets the bias
	//int volt = polarity * voltage * ADC_REF * 1000; //Check if this is correct
	//int volt = polarity * bias_incr[bias] * ADC_REF * 1000;
	long volt = polarity *  voltage;  //sets the voltage
	//set the bias to max value, compensated for this by making the outputted voltage larger

	//    memory.write(memory.getCurReg(), (voltage & 0xFF));
	//    memory.write(memory.getCurReg(), ((voltage >> 8) & 0xFF));

	uint16_t adcVal = pStat.getOutput(pStat_Sensor);
	long current = pStat.getCurrent(adcVal, ADC_REF, ADC_BITS)*pow(10, 8);
	//    memory.write(memory.getCurReg(), (current & 0xFF));
	//    memory.write(memory.getCurReg(), ((current >> 8) & 0xFF));
	
	//Prints out the value if
	if (print)
	{
	
	Serial.print(volt);
	Serial.print(",");
	Serial.print(adcVal);
	Serial.print(",");
	Serial.println(-current);
	}

	return current;
}


/*void MiniStat::runAMP(uint16_t user_gain, int voltage, uint16_t time, int samples)
@param		user_gain: the gain set by the user
@param		voltage: the voltage to pulse up to
@param		time: the time the pulse is high
@Param		samples: how many sample to take in the time the pulse is high

This runs the Chronoampeometry protocol. The wait time between samples is calculated, then the current is set to zero and a couple samlpes are taken before pulsing up to the desired value.
From that point the voltage is kept constant, and sampes are taken based on the in[ut given by the user. The voltage is then set back dwn to zero for a couple samples.
*/
void MiniStat::runAMP(uint16_t user_gain, int voltage, uint16_t time, int samples)
{
	//Defaults to zero at the start
	dac.outputA(0);
	dac.outputB(0); 
	delay(10);

	pStat.setGain(user_gain);
	pStat.setRLoad(0);
	pStat.setExtRefSource();
	pStat.setIntZ(1);
	
	int polarity = 0; //The polarity of the voltage and current
	int current = 0;  //The current to be read in
	int t = 0; //The time variable
	
	uint16_t wait_time = (time  / samples); //Time in MS delayed between readings
	uint16_t volt = calcDACValue(voltage);  //The volt value
	polarity = getPolarity(voltage);		//Sets polarity

	//Waits initially at zero before jumping up
	for (int k = 0; k < (samples / 4); k++)  //For one half quarter of the samples, can be changed to a different amount
	{
		Serial.print(t);		//PRitns the time
		Serial.print(", ");
		calcCurrent(0, polarity, true,0);	//Prints the current
		delay(1);		//Delays one ms
		t += 1;		//Increasees t by 1 ms since that was delayed
	}


	dac.outputB(volt);  //Steps up to the user voltage
	
	for (int i = 0; i < samples; i++)  //Done for the number of samples, t can be changed just to start here
	{
		Serial.print(t);		//Prints out the time
		Serial.print(", ");
		current = calcCurrent(voltage, polarity, true,13);	//Prints out the current
		delay(wait_time);		//Waits for the wait time
		t += wait_time;			//Increases t by this wait time

		
	}

	//Sets the voltage back to zero
	dac.outputB(0);	
	for (int k = 0; k < (samples / 4); k++)  //Done for a quarter of the total samples, this can be changed
	{
		//Like above, the time and current is printed out, delayed, and t is increased
		Serial.print(t);
		Serial.print(", ");
		calcCurrent(0, 1, true,0);
		delay(1);
		t += 1;
	}

	

}


//void MiniStat::print()
//
//Reads the voltage and current data from memory storage and prints to the
//serial monitor.
void MiniStat::print()
{
    for (int i = 0; i < 256; i=i+4)
    {
        int voltage = (int)((memory.read(i+1) << 8) | memory.read(i));
        int current = (int)((memory.read(i+3) << 8) | memory.read(i+2));
        
        Serial.print(i); //for debug purposes
        Serial.print(","); //for debug purposes
        Serial.print(voltage);
        Serial.print(",");
        Serial.println(current);
    }
}

/*int MiniStat::calcDACValue(int vout)
@param		vout: the desired output voltage
returns		The DAC code for the specific Vout
The desired output voltage is passed into the function. The funciton first accounts for the fact that only .24 of that voltage will be used and scales it up.
The Dac value is then solved for based on the formula in the DAC documentation
*/
int MiniStat::calcDACValue(int vout)
{
	double x = 4.1666 * vout;  //final voltage is divided timed by .24, so this adjusts for that
	
	return ((int)(x  * 2048 / 3300));  //Divided by half of the vref, formula given in documentation
	
}

/*int MiniStat::getPolarity(int volt)
@param		volt: the voltage
return		1 or -1
The function returns the polarity (1 or -1). The polarity is 1 if it is zero or positive, -1 for negative.
*/
int MiniStat::getPolarity(int volt)
{
	int polarity = 0; //Return value of the polarity
	if (volt >= 0)  //If the voltage is zero or positive, the polarity is 1
	{
		polarity = 1;
		pStat.setPosBias();//Bias is set as positive

	}
	else {		//If not the above, then it is negative, and the bias is set appropriatly
		polarity = -1;
		pStat.setNegBias();
	}

	return polarity;
}


/*
 Convert to current
 
 set parameters for LMP91000
 
 set parameters for MPC4922
 */


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
 
 
 ISR(TIMER2_COMPA_vect)
 {
 counter++;
 if(counter > 2hrs)
 {
 sleep_disable();
 counter = 0;
 }
 }
 
 
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


