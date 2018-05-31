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
#include <mutex>
#include <thread>
#include<exception>

#include "mythread.h"
#include "ECO_DS18B20.h"
#include "debug_logger.h"


// ###############################################		DEFINES		#################################################### //

// pin on which the onewire operates
#define ONEWIRE_PIN		7

// Pins for the L298N Motor Driver
#define L298N_3_ENA		22
#define L298N_3_IN1		23
#define L298N_3_IN2		24
#define L298N_1_2_STONE	25

#define RELAY_1_P1		30
#define RELAY_1_P2		21

// Commands for the L298N Motor Driver
#define off				0
#define fw				1
#define rw				2
#define hi 				3

using namespace std;


// ###############################################		FUNCTIONS	#################################################### //


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
template<class T>	// generic type, can be multiple things, used when one has multiple functions that are very similar.
class FullQueue : public exception	// make custom exception using the exception class as parent.
{
public:
	FullQueue(T t) : element(t){}

	T get()
	{ 
		return element; 
	}	// return the overflow element.

	virtual const char* what() const throw()
	{
		return "Queue is full!";
	}

private:
	T element;
};

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
class EmptyQueue : public exception
{
  public:
	EmptyQueue(void) : exception(){}	

	virtual const char* what() const throw()
	{
		return "Queue is empty!";
	}
};

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
template<class T, int queue_size>
class Queue	// same as before, but with an unknown type:
{
public:

	Queue() : queue_start(0),queue_end(queue_size-1),actual_size(0){}

    void enqueue(T a)
	{
		if (actual_size == queue_size) throw FullQueue<T>(a);	// throw means that it will try to find the catch.
	    QueueArray[(++queue_end) % queue_size] = a; // modulus, will repeat the queue_size.
		actual_size++; 
	}

	T dequeue(void)
	{
		if (actual_size == 0) throw EmptyQueue();
	    actual_size--; 
		return QueueArray[(queue_start++) % queue_size]; 
	}

	float get_sum(void)
	{
		float sum = 0;
		for (int i=0; i<queue_size; i++)
		{
			sum += QueueArray[i];
		}
		return sum;
	}

private:
	T QueueArray[queue_size];
	int queue_start,queue_end,actual_size;
};


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
	L298N_Driver(int _IN1, int _IN2) : IN1(_IN1), IN2(_IN2), EN(0)
	{
		// Prepare pins for choosing direction
		pinMode(IN1, OUTPUT);
		digitalWrite(IN1, LOW);
		pinMode(IN2, OUTPUT);
		digitalWrite(IN2, LOW);
		set_dir(off);
	}

	L298N_Driver(int _IN1, int _IN2,int _EN) : IN1(_IN1), IN2(_IN2), EN(_EN)
	{
		// Prepare pins for choosing direction
		pinMode(IN1, OUTPUT);
		digitalWrite(IN1, LOW);
		pinMode(IN2, OUTPUT);
		digitalWrite(IN2, LOW);
		set_dir(off);

		// Prepare PWM
		softPwmCreate(EN, 0, 500);
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
	int set_dir(int _dir)
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
			case 3:
				digitalWrite(IN1, HIGH);
				digitalWrite(IN2, HIGH);
			break;
		}
		return 0;
	}


	void set_duty(int _duty)
	{
		if(!EN)
		{
			cout << "No PWM output set, please use the constructor to intialize the PWM!\n";
		}
		else
		{
			softPwmWrite(EN, _duty*5);
		}
	}



protected:

private:

	int IN1;
	int IN2;
	int EN;
};


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
class Main_Controller : public MyThreadClass
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
    Main_Controller(float _tmax, float _tmin, float _topt) : Tmax(_tmax), Tmin(_tmin), r(_topt), window(L298N_3_IN1, L298N_3_IN2, L298N_3_ENA), inTempQ()
    {
		// Prepare the Main Fan
    	pinMode(RELAY_1_P1, OUTPUT);
		digitalWrite(RELAY_1_P1, LOW);

		// Prepare the stone beds
		pinMode(L298N_1_2_STONE, OUTPUT);
		digitalWrite(L298N_1_2_STONE, LOW);
    }

	float get_u(void)
	{
		lock_guard <mutex> u_lock(u_mutex);
		return u;
	}

	bool get_stoneFAN(void)
	{
		lock_guard <mutex> u_lock(u_mutex);
		return stoneFAN;
	}

	bool get_mainFAN(void)
	{
		lock_guard <mutex> u_lock(u_mutex);
		return mainFAN;
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
		while(tercon->pos())
		{
			unique_lock<mutex> lk(mux_log);
			cv_log.wait(lk);

			get_temp();
			y = tm.T_inside;
			controller();
			plant();
			sleep(TIME_STEP/2);
		}
		digitalWrite(RELAY_1_P1, LOW);
		digitalWrite(L298N_1_2_STONE, LOW);
    }


private:
	void get_temp()
	{
		DS18B20_object->meas_get(&tm);
	}

	void controller()
	{
		lock_guard <mutex> u_lock(u_mutex);

		if(Qsetup)
		{
			try
			{
				inTempQ.enqueue(tm.T_inside);
			}
			catch( FullQueue<float> t)	// if queue is full, continue here
			{
				inTempQ.dequeue();
				inTempQ.enqueue(t.get());
				Qsetup = false;
			}
		}
		else
		{
			inTempQ.dequeue();
			inTempQ.enqueue(tm.T_inside);
		}
		
		Integral = ((10*r) - inTempQ.get_sum()) * 10*Ts;


		// Calculate our u
		u = K*(r-y)+(Ke/Ts)*Integral;
	}

	void plant(void)
	{
		lock_guard <mutex> u_lock(u_mutex);
		// main fan & window
		if((u > 10) && (tm.T_outmean > tm.T_inside))
		{
			digitalWrite(RELAY_1_P1, HIGH);
			mainFAN = true;
		}
		else if(u < 10)
		{
			digitalWrite(RELAY_1_P1, HIGH);
			mainFAN = true;
		}
		else 
		{
			digitalWrite(RELAY_1_P1, LOW);
			mainFAN = false;
		}
		
		// stonebed
		if((tm.T_inside > 25) && (tm.T_stonemean < tm.T_inside))
		{
			digitalWrite(L298N_1_2_STONE, HIGH);
			stoneFAN = true;
		}
		else if((tm.T_inside < 21) && (tm.T_stonemean > tm.T_inside))
		{
			digitalWrite(L298N_1_2_STONE, HIGH);
			stoneFAN = true;
		}
		else
		{
			digitalWrite(L298N_1_2_STONE, LOW);
			stoneFAN = false;
		}
	}

	// Variables
	float y = 0;
	float u = 0;
	Queue < float, 10 > Y;
	bool Qsetup = true;
	float Integral = 0;

	// Constants
	float K = 1.2;
	float Ke = 0.32;
	float Ts = 10;
	float r;
	float Tmax;
	float Tmin;

	// Input/output
	Temp_measurement tm;
	L298N_Driver window;
	Queue < float, 10 > inTempQ;

	mutex u_mutex;

	bool stoneFAN = false;
	bool mainFAN = false;
};


Main_Controller* Main_Controller_object;


/*
class myclass
{
public:

protected:

private:

};
*/