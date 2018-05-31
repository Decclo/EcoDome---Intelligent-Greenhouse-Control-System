/*
* ECO_DS18B20.h
* Author:		Hans V. Rasmussen
* Created:		07/05-2018 13:00
* Modified:		16/05-2018 14:30
* Version:		1.1
*
* Description:
*	This header includes a class used for measuring and storing the output from the DS18B20 temperature sensors
*
* NOTE:
*
*/

#pragma once

#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <fstream>
#include <signal.h>
#include <time.h>
#include <vector>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <thread>
#include <mutex>
#include<semaphore.h>

#include "mythread.h"
#include "debug_logger.h"

using namespace std;

struct Temp_measurement
{
	float T_inside = 0;
	float T_in_window = 0;
	float T_out1 = 0;
	float T_out2 = 0;
	float T_outmean = 0;
	float T_stone1 = 0;
	float T_stone2 = 0;
	float T_stoneF = 0;
	float T_stonemean = 0;
	float T_extra1 = 0;
};

	/*! @brief	Class that reads data from sensors, builds upon the mythread class.
	*
	*
	*	@use	
	*
	@code{.cpp}
	*
	* @endcode
	*
	*/
class DS18B20 : public MyThreadClass
{
public:
	/*! @brief Constructor
	*
	* 
	*
	* @param 
	*		TERMINAL_CONTROLLER*, sem_t*, sem_t*, vector < string >*
	*
	* @returns void
	*
	*/
    DS18B20(TERMINAL_CONTROLLER* _tc, sem_t* _st, sem_t* _sc, vector < string >* _dev) : tercon(_tc), sem_temp(_st), sem_control(_sc), temp_devices(*_dev)
    {
		update();
    }

	/*! @brief Function that spits out the measured data.
	*
	* 
	*
	* @param Temp_measurement*
	*
	* @returns void
	*
	*/
	void meas_get(Temp_measurement* _ext_tm)
	{
		meas_get_mutex.lock();
		*_ext_tm = tm;
		meas_get_mutex.unlock();
	}

	/*! @brief Function to be called when alarm happens
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	void update()
	{
		meas_get_mutex.lock();
		temp_meas.clear();
		for(int i=0; i < temp_devices.size(); i++)
		{
			temp_meas.push_back(Read_DS18B20(temp_devices[i]));
		}
		tm.T_inside = temp_meas.at(0);
		tm.T_in_window = temp_meas.at(1);
		tm.T_out2 = temp_meas.at(3);
		tm.T_outmean = tm.T_out2;
		tm.T_stone1 = temp_meas.at(4);
		tm.T_stone2 = temp_meas.at(5);
		tm.T_stonemean = (temp_meas.at(4)+temp_meas.at(5))/2;
		tm.T_stoneF = temp_meas.at(2);
		//tm.T_extra1 = temp_meas.at(6);
		meas_get_mutex.unlock();
	}



protected:
	/** Implement this method in your subclass with the code you want your thread to run. */
    void InternalThreadEntry()
    {
		while(tercon->pos())
		{
			sem_wait(sem_temp);

			update();

			sem_post(sem_control);

		}
    }
private:

	/*! @brief Function to read the DS18B20
	*
	* 
	*
	* @param string device
	*
	* @returns float
	*
	*/
    float Read_DS18B20(string device)
    {
		// Define variables to be used
		string tmp = "/sys/bus/w1/devices/" + device + "/w1_slave";
		const char* addr = tmp.c_str();
		int BUFSIZE = 128;

		float temp;
		int i, j;
	    int fd;
		int ret;

		char buf[BUFSIZE];
		string strBuf;
		std::string::size_type sz;

		// open the file
		fd = open(addr, O_RDONLY);
		if(-1 == fd)
		{
			perror("open device file error");
			return 1;
		}

		// read the contents
		while(1)
		{
			ret = read(fd, buf, BUFSIZE);
			if(0 == ret){
				break;	
			}
			if(-1 == ret)
			{
				if(errno == EINTR)
				{
					continue;	
				}
				perror("read()");
				close(fd);
				return 0;
			}
		}

		// convert to a float (and devide by 1000 to ge the actual temperature)
		strBuf = buf;
		temp = stof( strBuf.substr( strBuf.find("t=") + 2), &sz) / 1000;

		// close the file and return the temperature
		close(fd);
		return temp;
	}

	TERMINAL_CONTROLLER* tercon;	// class pointer to the terminal controller
	sem_t* sem_temp;				// semaphore for knwoing when to start next measurement
	sem_t* sem_control;				// semaphore for signaling that measurement finished
	vector < string > temp_devices;	// string vector containing the addresses of the sensors
	vector < float > temp_meas;		// float vector for temporarely storing the data from the sensors.
	Temp_measurement tm;			// structure to hold the data once processed.

	std::mutex meas_get_mutex;		// mutex for protecting teh temp_measurement structure.
};


