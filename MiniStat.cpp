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
	wtest = Writetest();
    memory = EEPROM_24C64A();
    dac = DAC_MCP49xx(DAC_MCP49xx::MCP4922, 10); //int SS_PIN = 10;
    channel = Mux(channel_Ctrl);
    
    pStat.setMENB(pStat_Ctrl);
}

void MiniStat::WTest() {
	Serial.println(pStat.read(LMP91000_REFCN_REG));
	wtest.read();
	wtest.write();
	delay(100);
	wtest.read();
	Serial.println(pStat.read(LMP91000_REFCN_REG));

	//Giving me -1 8 times. should print 8 zeros
	//Possible wired wrong
}

//void MiniStat::begin()
//
//Wakes all devices from sleep mode.
void MiniStat::begin()
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
void MiniStat::runCV(uint8_t user_gain, uint8_t cycles, uint16_t scan_rate)
{
    dac.outputA(0);
	dac.outputB(2047)
   // dac.outputB(2047);  //Disables them, sets it up as a triangle wave
	//originally 0, 2048
    delay(10);
    
    pStat.setGain(user_gain);
	Serial.print("User Gain: ");
	Serial.println(pStat.getGain());
    pStat.setRLoad(0);
	Serial.print("R load: ");
	Serial.println(pStat.read(LMP91000_TIACN_REG));
    pStat.setExtRefSource();
	Serial.print("ExtRefSource: ");
	Serial.println(pStat.read(LMP91000_REFCN_REG));
    pStat.setIntZ(1);
	Serial.print("Set Int Z: ");
	Serial.println(pStat.getIntZ());
	delay(100);
    int current = 0;  //Not needed 
    uint8_t index = 0;
    
    //for (int i = 0; i < 50; i++) //for acid clean
    for (int i = 0; i < 2*cycles; i++) //for measurements
    {
        int polarity = 0; // Just positive or negative
        
        if (i%2 == 0)
        {
			Serial.print("Bias: ");
			Serial.print(pStat.read(LMP91000_REFCN_REG));
			Serial.print(", ");
			pStat.setNegBias();
			
			polarity = -1; 
			Serial.println(pStat.read(LMP91000_REFCN_REG));
        }
        else
        {
			Serial.print("Bias: ");
			Serial.print(pStat.read(LMP91000_REFCN_REG));
			Serial.print(", ");
			pStat.setPosBias();
			pStat.setBias(1);
			polarity = 1;
			Serial.println(pStat.read(LMP91000_REFCN_REG));
        }
        
        for (int j = 1; j <= 13; j++)
        {
            method(j, scan_rate, polarity);
            //Serial.print("j: ");
            //Serial.println(j);
        }
        
        for (int k = 12; k >= 0; k--)
        {
            method(k, scan_rate, polarity);
            //Serial.print("k: ");
            //Serial.println(k);
        }
    }

	dac.outputA(0);
	dac.outputB(0);
            
}


//void MiniStat::method(uint8_t bias, uint16_t scan_rate, int polarity)
//
//
void MiniStat::method(uint8_t bias, uint16_t scan_rate, int polarity)
{
    pStat.setBias(bias);
    delay(scan_rate);

	Serial.print("New Bias: ");
	Serial.println(pStat.read(LMP91000_REFCN_REG));
    
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


void MiniStat::runPulseV(uint8_t user_gain, uint8_t cycles, uint16_t frequency, uint16_t pulse_width, int pulse_amplitude, uint8_t pulse_per_cycle) //Add duty cycle
{
    /*
	Set DAC output values (Determine what they are)
	Delay

	Set gain based on user input
	SetRLoad (Determine what RLoad is)
	Set ExtRefSource (Determine)
	SetIntZ (Determine)
	


	revised option
	for number of cycles
		for number of divisions
			Pulse to zero
			wait for pulse width
			pulse up to i/number of divisions
			wait till end (99%) of the pulse
			Call a new method function to calculate the current
				-Can assume the polarity is one, no scan rate, Will not need the bias
		
	
	option 1
	For loop for Bias incriment
		set first value to zero, incriment the rest to the peak values based on number of cycles
	
	For number of user cycles
		method(0, Scan rate, 1);
		method(j, scan rate, 1);


	Option 2
	Pulse height value variable initalized to user gain)

	for loop (0 to number of cycles)
		Set voltage to low for half of the period, post incriment the base voltage (Determine how long the period is going to be (1/Scan Rate?))
		Turn voltage to high for rest of period
		Measure the current at 95% of the period
		Method(i, scan rate, 1)

		
	
	*/

	dac.outputA(0);
	dac.outputB(0);  //Disables them
	delay(10);

//	pStat.setGain(user_gain);
//	pStat.setRLoad(0);
//	pStat.setExtRefSource();
//	pStat.setIntZ(1);

	int current1 = 0;
	unsigned short volt = 0;
	int polarity = 0;
	int ms = 0;
	int us = 0;
	int down_time = (1000000 / frequency - pulse_width); //Down time in us
	ms = down_time / 1000;
	us = down_time - 1000 * ms;


	Serial.print("Down time: ");
	Serial.println(down_time);




	for (int i = 1; i <= cycles; i++)
	{
		for (int j = 1; j <= pulse_per_cycle; j++)
		{
			dac.outputB(0);
			
			//create wait based on duty cycles
			delay(ms);
			delayMicroseconds(us); //Delay us can only do 16000, do two delays for more accuracy
			
			volt = (((double)j / pulse_per_cycle)) * calcDACValue(pulse_amplitude);
	
			dac.outputB(volt);
		
			delayMicroseconds(pulse_width); //Find the accuracy of this
			polarity = getPolarity((((double)j / pulse_per_cycle)) * pulse_amplitude);
			current1 = calcCurrent((((double)j / pulse_per_cycle)) * pulse_amplitude, polarity);

			//{rintout the values

		}
		dac.outputB(0);
	}
}

void MiniStat::runDPV(uint8_t user_gain, uint8_t cycles, uint16_t startV, uint16_t endV, int step_size, int pulse_amp, uint16_t sample_period, uint16_t pulse_freq)
{

	/*
		*Set DAC output Values
		*Delay
		
		*Set Gain
		*SetRload
		*Set extRefSourc
		*SetIntZ

		
		Revised:
		*for cycles
		*	for(start, end, step size)
				wait(sample minus pulse time)
				Record current
				increase voltage pulse size
				Wait for pulse time
				Record current
				differentiat current through a new method_


		
		
		Set variable for the height Value

		For Number of cycles
			Set the voltage to the base value
			Sample right before the voltage is increased
			Incriment voltage the desired amount
			wait till before the end
			Sample the end value
			Incriment the base value

		//will probably need a new comparison method to measure the dfference in currents
	*/

	dac.outputA(0);
	dac.outputB(0);  //Disables them
	delay(10);

//	pStat.setGain(user_gain);
//	pStat.setRLoad(0);
//	pStat.setExtRefSource();
//	pStat.setIntZ(1);

	int current1 = 0;
	int current2 = 0;
	int volt = 0;
	int polarity = 0;
	for (int i = 1; i <= cycles; i++)
	{
		for (int j = startV; j <= endV; j += step_size)
		{
			volt = calcDACValue(j);
			dac.outputB(volt);
			polarity = getPolarity(j);

			//wait for the prestep sample
			delay((1000 / pulse_freq)); //Will need to do minus the sample period later


			current1 = calcCurrent(j, polarity);

			volt = calcDACValue(j + pulse_amp);
			dac.outputB(volt);
			polarity = getPolarity(j + pulse_amp);

			//wait till the end value of time
			delay(1000 / pulse_freq);

			current2 = calcCurrent(j + pulse_amp, polarity);
			
			//prints the base voltage value and the current difference
//			Serial.print(j);
//			Serial.print(",");
			Serial.println(-((double)(current2) - current1));

			

		}
	}
	dac.outputB(0);

}

void MiniStat::runSWV(uint8_t user_gain, uint8_t cycles, uint16_t startV, uint16_t endV,  int pulse_amp, uint16_t volt_step, uint16_t pulse_freq)
{
	
	dac.outputA(0);
	dac.outputB(0);  //Disables them //May need to change to half
	delay(10);
	
	pStat.setGain(user_gain);
	pStat.setRLoad(0);
	pStat.setExtRefSource();
	pStat.setIntZ(1);
	int volt = 0; //in code to send to the DAC
	int polarity = 0; //This should all be good

	int current1 = 0;
	int current2 = 0;


	/*possible wait time solutions
	Frequency in Hz
	1/hz = Sec
	1000/ Frequency - Time in ms, can simply input into delay
	*/
	for (int i = 1; i <= cycles; i++)
	{
		volt = calcDACValue(startV); //Sets it to the DAC code value based on the voltage inputed
		polarity = getPolarity(volt);
		dac.outputB(volt); //Sets the voltage to the number given
		for (int j = startV; j <= endV; j += volt_step)
		{
	

			Serial.println(j);
			//increase to J + pulse amp
			//Serial.print(j + pulse_amp);
			//Serial.print(", ");
			volt = calcDACValue(j + pulse_amp); //Base Value + Amplitude
			//Serial.println(volt);
			polarity = getPolarity(j + pulse_amp);
			dac.outputB(volt);
			//Wait for (.99 * 1/pulse_freq);
			//delay(0.99 / pulse_freq); //will need a new method to account for the inverse
			//Scan the current and store
			
			delay(1000 / pulse_freq); //Added to accomidate for the wait time possible
			current1 = calcCurrent(j + pulse_amp,  polarity);


			//Decrease to (J - Pulse amp)
			//Serial.print(j - pulse_amp);
			//Serial.print(", ");
			volt = calcDACValue(j - pulse_amp);
			//Serial.println(volt);
			polarity = getPolarity(j- pulse_amp);
			dac.outputB(volt);
			//Wait for (.99 * 1/pulse_freq);
			//delay(0.99 / pulse_freq);
			//scan the current and store
			delay(1000 / pulse_freq);
			current2 = calcCurrent(j-pulse_amp,  polarity);
			//Find the difference in current and print it
//			Serial.print(j);
//			Serial.print(",");
			//Serial.print(adcVal);
			//serial.print(",");
			Serial.println(-((double)(current2) - current1)); //See if this is how the printing works


		}
		dac.outputB(0);
	}
	dac.outputA(0);
}
	


int MiniStat::calcCurrent(uint16_t voltage, int polarity)
{
	
	//pStat.setBias(bias);
	//delay(scan_rate);
	pStat.setBias(13);
	//int volt = polarity * voltage * ADC_REF * 1000; //Check if this is correct
	//int volt = polarity * bias_incr[bias] * ADC_REF * 1000;
	long volt = polarity *  voltage;
	//set the bias to max value, compensated for this by making the outputted voltage larger
	
	//    memory.write(memory.getCurReg(), (voltage & 0xFF));
	//    memory.write(memory.getCurReg(), ((voltage >> 8) & 0xFF));

	uint16_t adcVal = pStat.getOutput(pStat_Sensor);
	long current = pStat.getCurrent(adcVal, ADC_REF, ADC_BITS)*pow(10, 8);
	//    memory.write(memory.getCurReg(), (current & 0xFF));
	//    memory.write(memory.getCurReg(), ((current >> 8) & 0xFF));

	Serial.print(volt);
	Serial.print(",");
	Serial.print(adcVal);
	Serial.print(",");
	Serial.println(-current);

	return current;
}

void MiniStat::runAMP(uint16_t user_gain, int voltage, uint16_t time, int samples)
{
	/*
		Set DAC output Values
		Delay
		Set Gain
		SetRload
		Set extRefSourc
		SetIntZ
		
		Change up to the voltage
		for time
			read in the current
			find out how to continuously read in the current

	*/
	dac.outputA(0);
	dac.outputB(0);  //Disables them
	delay(10);

//	pStat.setGain(user_gain);
//	pStat.setRLoad(0);
//	pStat.setExtRefSource();
//	pStat.setIntZ(1);
	
	int polarity = 0;
	int current = 0;
	
	uint16_t wait_time = (time  / samples); //Time in MS
	uint16_t volt = calcDACValue(voltage);
	Serial.println(calcDACValue(voltage));
	Serial.println(volt);
	dac.outputB(volt);
	
	for (int i = 0; i < samples; i++)
	{
		delay(wait_time);
		current = calcCurrent(voltage, polarity);
//		uint16_t adcVal = pStat.getOutput(pStat_Sensor);
//		int current = pStat.getCurrent(adcVal, ADC_REF, ADC_BITS)*pow(10, 8);

//		Serial.print(voltage);
//		Serial.print(",");
//		Serial.print(adcVal);
//		Serial.print(",");
//		Serial.println(-current);
		
	}
	dac.outputB(0);


	

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

int MiniStat::calcDACValue(int vout)
{
	long x = 4.1666 * vout;
	
	return ((int)(x  * 4096 / 1650));  //Divided by half of the vref
	
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


