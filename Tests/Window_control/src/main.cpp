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
#include "../include/panalysis.h"
#include "../include/picontrol.h"
#include "../include/ECO_DS18B20.h"
#include "../include/debug_logger.h"

// Define namespaces
using namespace std;
using namespace libconfig;

// Main
int main(void)
{
    wiringPiSetup();
    WINDOW_CONTROLLER window_test(L298N_3_IN1, L298N_3_IN2, WINDOW_FEEDBACK);
    tercon = new TERMINAL_CONTROLLER();
    tercon->StartInternalThread();
    digitalWrite(L298N_3_ENA, HIGH);
    window_test.StartInternalThread();

    sleep(5);
    cout << "starting... " << endl;

    int duty = 0;
    bool dir = false;
    int counter = 0;
    while (tercon->pos())
    {
        if(tercon->dir() && !dir)
        {
            window_test.open();
            dir = 1;
        }
        else if(!tercon->dir() && dir)
        {
            window_test.close();
            dir = 0;
        }
        sleep(1);

    }

    tercon->WaitForInternalThreadToExit();
    window_test.WaitForInternalThreadToExit();
    return 0;
}