#pragma once

/*
* picontrol.h
* Author:		Hans V. Rasmussen
* Created:		13/04-2018 15:00
* Modified:		13/04-2018 16:30
* Version:		0.1
*
* Description:
*	This library includes the control system for the EcoDome project, complete with pin control for the Raspberry Pi 3
*
* NOTE:
*
*/

#include <unistd.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>

#include "mythread.h"

using namespace std;


// ###############################################		DEFINES		#################################################### //

// Pins for the L298N Motor Driver
#define L298N_IN1_1		22
#define L298N_IN2_1		23
#define L298N_IN3_1		24
#define L298N_IN4_1		25

// Commands for the L298N Motor Driver
#define off				0
#define fw				1
#define rw				2

// pin on which the onewire operates
#define ONEWIRE_PIN		7



// ###############################################		FUNCTIONS	#################################################### //

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



// ###############################################		THREADS 	#################################################### //

	/*! @brief	Thread that controls the temperature
	*
	*
	*	@use	
	*
	@code{.cpp}
	*
	* @endcode
	*
	*/
class TempController : public MyThreadClass
{
public:

	/*! @brief Constructor
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
    TempController()
    {

    }

protected:

    /*! @brief Actual thread function that contains the code to be run.
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
    void InternalThreadEntry()
    {
        
    }


private:


};


// ###############################################		CLASSES		#################################################### //

	/*! @brief	Class that contains functionality for the L298N Motor Driver
	*
	*
	*	@use	
	*
	@code{.cpp}
	*
	* @endcode
	*
	*/
class L298N_Driver
{
public:

	/*! @brief Constructor
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	L298N_Driver(int _IN1, int _IN2) : IN1(_IN1), IN2(_IN2)
	{
		pinMode(IN1, OUTPUT);
		pinMode(IN2, OUTPUT);
		set(off, 100);
	}

	/*! @brief sets motor direction and speed (speed currently not working)
	*
	* 
	*
	* @param int _dir, int _pctpwr
	*
	* @returns int
	*
	*/
	int set(int _dir, int _pctpwr)
	{
		switch(_dir)
		{
			case 0:
				digitalWrite(IN1, LOW);
				digitalWrite(IN2, LOW);
			break;

			case 1:
				digitalWrite(IN1, HIGH);
				digitalWrite(IN2, LOW);
			break;

			case 2:
				digitalWrite(IN1, LOW);
				digitalWrite(IN2, HIGH);
			break;
		}
		return 0;
	}

protected:

private:

	int IN1;
	int IN2;
};


/*
class myclass
{
public:

protected:

private:

};
*/