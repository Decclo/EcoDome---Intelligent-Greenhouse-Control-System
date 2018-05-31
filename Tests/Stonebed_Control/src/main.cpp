// Standard Libraries
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <string>
#include <wiringPi.h>		// Library that includes classes to interface with the RPi's sorroundings
#include <softPwm.h>
#include <signal.h>
#include <time.h>
#include <string>


// Custom Libraries
#include "../include/picontrol.h"
#include "../include/debug_logger.h"

// Define namespaces
using namespace std;

// Main
int main(void)
{
    wiringPiSetup();
    tercon = new TERMINAL_CONTROLLER();
    tercon->StartInternalThread();

    pinMode(L298N_1_2_STONE, OUTPUT);
	digitalWrite(L298N_1_2_STONE, LOW);

    cout << "starting... " << endl;

    bool dir = false;
    while (tercon->pos())
    {

        if(tercon->dir())
        {
            digitalWrite(L298N_1_2_STONE, HIGH);
        }
        else
        {
            digitalWrite(L298N_1_2_STONE, LOW);
        }
        usleep(200000);
    }

    tercon->WaitForInternalThreadToExit();
    return 0;
}