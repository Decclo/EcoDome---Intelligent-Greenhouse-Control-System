// Standard Libraries
#include <unistd.h>
#include <iostream>
#include <wiringPi.h>		// Library that includes classes to interface with the RPi's sorroundings
#include <ds18b20.h>		// One-wire library specifically for the ds18b20 temperature sensor

#include "../include/picontrol.h"

// Define namespaces
using namespace std;

//#define LED_GPIO		17
//#define BUTTON_GPIO		27

// Main
int main(void)
{
	// Prepare WiringPi
	wiringPiSetup();

	L298N_Driver L298N_1(L298N_IN3_1, L298N_IN4_1);

	float tmp1 = 0;
	float tmp2 = 0;

	while(1)
	{
		tmp1 = Read_DS18B20("28-031645884cff");
		tmp2 = Read_DS18B20("28-0416850db6ff");
		cout << "Temp sensor 1 is " << tmp1 << endl;
		cout << "Temp sensor 1 is " << tmp2 << endl;

		if(tmp1 > tmp2)
		{
			L298N_1.set(fw, 100);
		}
		else
		{
			L298N_1.set(rw, 100);
		}

	}
	

	/*
	// setup a PWM
	pinMode (18, PWM_OUTPUT) ;
	pwmSetMode (PWM_MODE_MS);
	pwmSetClock(384); //clock at 50kHz (20us tick)
	pwmSetRange(1024); //range at 1000 ticks (20ms)
	pwmWrite(18, 512);  //theoretically 50 (1ms) to 100 (2ms) on my servo 30-130 works ok

	// Setup threads
	TempController tc;
	tc.StartInternalThread();
	*/


	/*
	wiringPiSetupGpio();
	pinMode(LED_GPIO, OUTPUT);
	pinMode(BUTTON_GPIO, INPUT);

	digitalWrite(LED_GPIO, LOW);

	int model, rev, mem, maker, overVolted;
	piBoardId(&model, &rev, &mem, &maker, &overVolted);
	cout << "This is an RPi: " << piModelNames[model] << endl;
	cout << " with revision number: " << piRevisionNames[rev] << endl;
	cout << " manufactured by: " << piMakerNames[maker] << endl;
	cout << " it has: " << mem << " RAM and o/v: " << overVolted << endl;
	cout << "Button GPIO has ALT mode: " << getAlt(BUTTON_GPIO);
	cout << " and value: " << digitalRead(BUTTON_GPIO) << endl;
	cout << "LED GPIO has ALT mode: " << getAlt(LED_GPIO);
	cout << " and value: " << digitalRead(LED_GPIO) << endl;
	*/

	// Wait for threads to exit
	//tc.WaitForInternalThreadToExit();

	return 0;
}
