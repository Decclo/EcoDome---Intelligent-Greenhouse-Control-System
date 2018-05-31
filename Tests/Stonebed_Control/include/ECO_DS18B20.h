/*
* panalysis.h
* Author:		Hans V. Rasmussen
* Created:		07/05-2018 13:00
* Modified:		07/05-2018 14:30
* Version:		0.5
*
* Description:
*	This library includes a class used for measuring and storing the output from the DS18B20 temperature sensors
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

#include "mythread.h"
#include "debug_logger.h"

using namespace std;

struct Temp_measurement
{
	float T_inside = 0;
	float T_out1 = 0;
	float T_out2 = 0;
	float T_outmean = 0;
	float T_stone1 = 0;
	float T_stone2 = 0;
	float T_stonemean = 0;
};


class DS18B20 : public MyThreadClass
{
public:
    DS18B20(vector < string >* _dev) : temp_devices(*_dev)
    {
		update();
    }

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
		tm.T_out1 = temp_meas.at(1);
		tm.T_out2 = temp_meas.at(2);
		tm.T_outmean = (temp_meas.at(1)+temp_meas.at(2))/2;
		tm.T_stone1 = temp_meas.at(3);
		tm.T_stone2 = temp_meas.at(4);
		tm.T_stonemean = (temp_meas.at(3)+temp_meas.at(4))/2;
		meas_get_mutex.unlock();
	}



protected:
    void InternalThreadEntry()
    {
		while(tercon->pos())
		{
			unique_lock<mutex> lk(mux_log);
			cv_log.wait(lk);

			update();
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


	vector < string > temp_devices;
	vector < float > temp_meas;
	Temp_measurement tm;

	std::mutex meas_get_mutex;
};


DS18B20* DS18B20_object;