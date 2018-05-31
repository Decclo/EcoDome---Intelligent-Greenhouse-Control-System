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
#include <softPwm.h>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <exception>

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
#define L298N_STONE		25

#define WINDOW_FEEDBACK	29
#define WINDOW_TIME		20

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


class WINDOW_CONTROLLER : public MyThreadClass
{
public:
	WINDOW_CONTROLLER(int _IN1, int _IN2, int _wpin) : wdriver(_IN1, _IN2), wfp(_wpin), w_dir(0), w_todo(0), w_counter(0)
	{

	}

	void open(void)
	{
		if(!w_dir)
		{
			w_dir = 1;
			w_todo = true;
			w_counter = 0;
		}
	}

	void close(void)
	{
		if(w_dir)
		{
			w_dir = 0;
			w_todo = true;
			w_counter = 0;
		}
	}

protected:
	void InternalThreadEntry()
	{
		while(tercon->pos())
		{
			if(w_todo)
			{
				if(w_dir)
				{
					wdriver.set_dir(fw);
				}
				else
				{
					wdriver.set_dir(rw);
				}
				tercon->term_write("Window sensor is: ");
				tercon->term_write(to_string(digitalRead(wfp)));

				if(!w_dir && (!digitalRead(wfp) || (w_counter > ((WINDOW_TIME/TIME_STEP)*100))))
				{
					wdriver.set_dir(hi);
					w_counter = 0;
					w_todo = false;
				}
				else if (w_dir && (w_counter > ((WINDOW_TIME/TIME_STEP)*100)))
				{
					wdriver.set_dir(hi);
					w_counter = 0;
					w_todo = false;
				}
				else
				{
					w_counter++;
				}
				usleep(100000);
			}
			else
			{
				sleep(1);
			}
		}

		wdriver.set_dir(fw);
		while(w_counter < (WINDOW_TIME/TIME_STEP))
		{
			sleep(TIME_STEP);
			w_counter++;
		}
		wdriver.set_dir(off);
	}

private:
	L298N_Driver wdriver;
	int wfp;
	bool w_dir;
	bool w_todo;
	int w_counter;
	
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
    Main_Controller(float _tmax, float _tmin, float _topt) : 
		Tmax(_tmax), 
		Tmin(_tmin), 
		Tdes(_topt), 
		window(L298N_3_IN1, L298N_3_IN2, WINDOW_FEEDBACK), 
		inTempQ()
		p_analyser(Tmin, Tmax, Tdes),
		p_loader()
    {
		// Prepare the Main Fan
    	pinMode(RELAY_1_P1, OUTPUT);
		digitalWrite(RELAY_1_P1, LOW);

		// Prepare the stone beds
		pinMode(L298N_STONE, OUTPUT);
		digitalWrite(L298N_STONE, LOW);

		window.StartInternalThread();
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

	void set_ref(float _r)
	{
		lock_guard <mutex> r_lock(r_mutex);
		r = _r;
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

			sleep(TIME_STEP/2);
			get_temp();
			y = tm.T_inside;
			controller();
			plant();
		}
		digitalWrite(RELAY_1_P1, LOW);
		digitalWrite(L298N_STONE, LOW);
		window.WaitForInternalThreadToExit();
    }


private:
	void get_temp()
	{
		DS18B20_object->meas_get(&tm);
	}

	void controller()
	{
		lock_guard <mutex> u_lock(u_mutex);
		lock_guard <mutex> r_lock(r_mutex);

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
		/*
		if((u > 10) && (tm.T_outmean > tm.T_inside))
		{
			digitalWrite(RELAY_1_P1, HIGH);
			window.open();
			mainFAN = true;
		}
		else if(u < -10)*/
		if(tm.T_inside > 30)
		{
			digitalWrite(RELAY_1_P1, HIGH);
			window.open();
			mainFAN = true;
		}
		else 
		{
			digitalWrite(RELAY_1_P1, LOW);
			window.close();
			mainFAN = false;
		}
		
		// stonebed
		if((u < -5) && (tm.T_stonemean < tm.T_inside))
		{
			digitalWrite(L298N_STONE, HIGH);
			stoneFAN = true;
		}
		else if((u > 5) && (tm.T_stonemean > tm.T_inside))
		{
			digitalWrite(L298N_STONE, HIGH);
			stoneFAN = true;
		}
		else
		{
			digitalWrite(L298N_STONE, LOW);
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
	float Tdes;

	// Input/output
	Temp_measurement tm;
	WINDOW_CONTROLLER window;
	PANALYSIS p_analyser;
	PROGLOAD p_loader;
	Queue < float, 10 > inTempQ;

	mutex u_mutex;
	mutex r_mutex;

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