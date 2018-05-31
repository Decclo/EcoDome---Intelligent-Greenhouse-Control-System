/*
* main.cpp
* Author:		Hans V. Rasmussen
* Created:		13/04-2018 15:00
* Modified:		23/05-2018 15:00
* Version:		1.7
*
* Description:
*	main file for the EcoDome prototype code.
*
* NOTE:
*
*/


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
#include<semaphore.h>
#include <libconfig.h++>


// Custom Libraries
#include "../include/panalysis.h"
#include "../include/picontrol.h"
#include "../include/ECO_DS18B20.h"
#include "../include/debug_logger.h"

// Define namespaces
using namespace std;
using namespace libconfig;

void alarm_handle(int sig);

// Gloabal variables for the alarm handle
sem_t sem_controller;
sem_t sem_DS18B20;
TERMINAL_CONTROLLER* tercon_object;
DS18B20* DS18B20_object;
Main_Controller* Main_Controller_object;
LOGGER* LOGGER_object;

// Main
int main(void)
{
    //  ########## Preparing Configuration ##########   //
    vector< prognosis_downlaod_structure > _progconf_data;
    int prog_number = 0;
    destemp t_evalues;

    CONFLOAD cfgload("./Config.cfg");
    cfgload.get_progdata(_progconf_data);
    cfgload.get_prog_number(prog_number);
    cfgload.get_minmaxdes(t_evalues);
    cout << "t_evalues are \nmax: " << t_evalues.T_max << "\ndes: " << t_evalues.T_des << "\nmin: " << t_evalues.T_min << endl;


    // Preparing temperature and general logging
    vector < string > DS18B20_Devices;
	vector < string > log_Descriptions;

    DS18B20_Devices.push_back("28-0317200e5cff");
	log_Descriptions.push_back("in_soil__");

    DS18B20_Devices.push_back("28-031730398bff");
	log_Descriptions.push_back("in_window");

	DS18B20_Devices.push_back("28-051685213dff");
    log_Descriptions.push_back("StoneFan");

    DS18B20_Devices.push_back("28-041720a4a2ff");
	log_Descriptions.push_back("outside_2");

    DS18B20_Devices.push_back("28-0416850db6ff");
	log_Descriptions.push_back("stone_close");

    DS18B20_Devices.push_back("28-031645884cff");
	log_Descriptions.push_back("stone_far");

    //DS18B20_Devices.push_back("28-0417207ba2ff");
	//log_Descriptions.push_back("Freddy_ex");

    log_Descriptions.push_back("___u____");
    log_Descriptions.push_back("__Tref__");
    log_Descriptions.push_back("Sb_F");
    log_Descriptions.push_back("M_F");


    // prepare real-world interfaces
    wiringPiSetup();

    // initiate semaphores
    sem_t sem_temp_ready;
    sem_init(&sem_DS18B20, 0, 0);
    sem_init(&sem_controller, 0, 0);
    sem_init(&sem_temp_ready, 0, 0);

    // make objects
    tercon_object = new TERMINAL_CONTROLLER();
    DS18B20_object = new DS18B20(tercon_object, &sem_DS18B20, &sem_temp_ready, &DS18B20_Devices);
    Main_Controller_object = new Main_Controller(tercon_object, DS18B20_object, &sem_controller, &sem_temp_ready, _progconf_data, prog_number, t_evalues.T_max, t_evalues.T_des, t_evalues.T_min);
    LOGGER_object = new LOGGER(&log_Descriptions);
    
    // starts threads
    DS18B20_object->StartInternalThread();
    Main_Controller_object->StartInternalThread();
    tercon_object->StartInternalThread();

    // Giving the other threads time to start
    int p_counter = 0;
    alarm(5);
    while(tercon_object->pos())
    {
        signal(SIGALRM, alarm_handle);
    }

   DS18B20_object->WaitForInternalThreadToExit();
   Main_Controller_object->WaitForInternalThreadToExit();
   tercon_object->WaitForInternalThreadToExit();
    
    
    return 0;
}


void alarm_handle(int sig)
{
    // start by setting the next alarm
	alarm(TIME_STEP);

    // prepare variables to be used for data preparations
    Temp_measurement tm;
    vector < string > data;

    // acquire data and put it into a string vector for the logger
    DS18B20_object->meas_get(&tm);
    data.push_back(to_string(tm.T_inside));
    data.push_back(to_string(tm.T_in_window));
    data.push_back(to_string(tm.T_stoneF));
    data.push_back(to_string(tm.T_out2));
    data.push_back(to_string(tm.T_stone1));
    data.push_back(to_string(tm.T_stone2));
    //data.push_back(to_string(tm.T_extra1));
    data.push_back(to_string(Main_Controller_object->get_u()));
    data.push_back(to_string(Main_Controller_object->get_ref()));
    data.push_back(to_string(Main_Controller_object->get_stoneFAN()));
    data.push_back(to_string(Main_Controller_object->get_mainFAN()));

    // pass string vector to logger
    LOGGER_object->update(data);

    // signal other threads that they can start their part of this iterations work
    sem_post(&sem_DS18B20);
    sem_post(&sem_controller);
}