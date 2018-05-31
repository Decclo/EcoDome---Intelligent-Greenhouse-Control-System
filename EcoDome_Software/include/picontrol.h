#pragma once

/*
* picontrol.h
* Author:		Hans V. Rasmussen
* Created:		13/04-2018 15:00
* Modified:		25/04-2018 16:30
* Version:		1.5
*
* Description:
*	This header includes the control system for the EcoDome project, complete with pin control for the Raspberry Pi 3
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
#include<semaphore.h>
#include <exception>

#include "mythread.h"
#include "ECO_DS18B20.h"
#include "debug_logger.h"
#include "panalysis.h"


// ###############################################		DEFINES		#################################################### //

// pin on which the onewire operates
#define ONEWIRE_PIN		7

// Pins for the L298N Motor Driver
#define L298N_3_ENA		22
#define L298N_3_IN1		23
#define L298N_3_IN2		24
#define L298N_STONE		25

#define WINDOW_FEEDBACK	29
#define WINDOW_TIME		30

#define RELAY_1_P1		30
#define RELAY_1_P2		21

// Commands for the L298N Motor Driver
#define off				0
#define fw				1
#define rw				2
#define hi 				3

using namespace std;

// Thread syncronization
sem_t sem_mcontroller;


// ###############################################		FUNCTIONS	#################################################### //


// ###############################################		CLASSES		#################################################### //

	/*! @brief	exception class that supports the ringbuffer
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

	/*! @brief	exception class that supports the ringbuffer
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

	/*! @brief	Class that contains a ringbuffer, heavily inspired by Søren Top's teachings
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
class Queue
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
	Queue() : queue_start(0),queue_end(queue_size-1),actual_size(0){}

	/*! @brief places new item at front of queue, has exception support.
	*
	* 
	*
	* @param T
	*
	* @returns void
	*
	*/
    void enqueue(T a)
	{
		if (actual_size == queue_size) throw FullQueue<T>(a);	// throw means that it will try to find the catch.
	    QueueArray[(++queue_end) % queue_size] = a; // modulus, will repeat the queue_size.
		actual_size++; 
	}

	/*! @brief returns oldest element in queue and frees its place, has exception support.
	*
	* 
	*
	* @param void
	*
	* @returns T
	*
	*/
	T dequeue(void)
	{
		if (actual_size == 0) throw EmptyQueue();
	    actual_size--; 
		return QueueArray[(queue_start++) % queue_size]; 
	}

	/*! @brief returns sum of all items in queue without changing the queue itself.
	*
	* 
	*
	* @param void
	*
	* @returns T
	*
	*/
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
	T QueueArray[queue_size] = {0};
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

	/*! @brief alternative constructor, containing support for duty cycle.
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
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

	/*! @brief function used to set the motor duty cycle when alternative constructor is used, will return error otherwise
	*
	* 
	*
	* @param int(1-100)
	*
	* @returns void
	*
	*/
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

	/*! @brief	Thread class that control sthe window
	*
	*
	*	@use	
	*
	@code{.cpp}
	*
	* @endcode
	*
	*/
class WINDOW_CONTROLLER : public MyThreadClass
{
public:
	/*! @brief Constructor
	*
	* 
	*
	* @param TERMINAL_CONTROLLER* _tc, int _IN1, int _IN2, int _wpin
	*
	* @returns void
	*
	*/
	WINDOW_CONTROLLER(TERMINAL_CONTROLLER* _tc, int _IN1, int _IN2, int _wpin) : tercon(_tc), wdriver(_IN1, _IN2), wfp(_wpin), w_dir(1), w_todo(0), w_counter(0)
	{

	}

	/*! @brief Function that opens the window
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	void open(void)
	{
		unique_lock<mutex> w_lock(w_mutex);
		if(!w_dir)
		{
			w_dir = 1;
			w_todo = true;
			w_counter = 0;
		}
	}

	/*! @brief Function that closes the window
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	void close(void)
	{
		unique_lock<mutex> w_lock(w_mutex);
		if(w_dir)
		{
			w_dir = 0;
			w_todo = true;
			w_counter = 0;
		}
	}

protected:
	/** Implement this method in your subclass with the code you want your thread to run. */
	void InternalThreadEntry()
	{
		// Keep running as long as the program is not being terminated
		while(tercon->pos())
		{
			// lock mutex
			w_mutex.lock();
			// Check if there is anything to do, if not sleep 1 second
			if(w_todo)
			{
				// check direction to be set, and set window to run in that direction.
				if(w_dir)
				{
					wdriver.set_dir(fw);
				}
				else
				{
					wdriver.set_dir(rw);
				}

				// if window is closing and either button has been pressed or timer expired, stop window.
				if(!w_dir && ((!digitalRead(wfp) || (w_counter > (WINDOW_TIME*10)))))
				{
					wdriver.set_dir(hi);
					w_counter = 0;
					w_todo = false;
				}
				//if window is opening and timer has expired.
				else if (w_counter > ((WINDOW_TIME/TIME_STEP)*100))
				{
					wdriver.set_dir(hi);
					w_counter = 0;
					w_todo = false;
				}
				// else increase the counter.
				else
				{
					w_counter++;
				}
				// unlock mutex and sleep for 100 milliseconds.
				w_mutex.unlock();
				usleep(100000);
			}
			else
			{
				w_mutex.unlock();
				sleep(1);
			}
		}

		// if program is shutting down, open window and shut down.
		wdriver.set_dir(fw);
		while(w_counter < WINDOW_TIME)
		{
			sleep(1);
			w_counter++;
		}
		wdriver.set_dir(off);
	}

private:
	TERMINAL_CONTROLLER* tercon;
	L298N_Driver wdriver;
	int wfp;
	bool w_dir;
	bool w_todo;
	int w_counter;

	mutex w_mutex;
	
};


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
    Main_Controller(TERMINAL_CONTROLLER* _tc, DS18B20* _tm, sem_t* _sc, sem_t* _str, vector< prognosis_downlaod_structure > _dstruct, int _pn, float _tmax, float _tmin, float _topt) : 
		tercon(_tc),
		tempobj(_tm),
		sem_control(_sc),
		sem_temp_ready(_str),
		_down_data(_dstruct), 
		_prognosis_number(_pn),
		Tmax(_tmax), 
		Tmin(_tmin), 
		Tdes(_topt),
		window(tercon, L298N_3_IN1, L298N_3_IN2, WINDOW_FEEDBACK), 
		inTempQ(),
		p_analyser(Tmin, Tmax, Tdes),
		p_loader()
    {

		progdownload(_down_data);
		p_loader.initiate(_down_data[0], _prognosis_number);
		
		// Prepare the Main Fan
    	pinMode(RELAY_1_P1, OUTPUT);
		digitalWrite(RELAY_1_P1, LOW);

		// Prepare the stone beds
		pinMode(L298N_STONE, OUTPUT);
		digitalWrite(L298N_STONE, LOW);

		window.StartInternalThread();

    }

	/*! @brief Function to acquire u for logging purposes
	*
	* 
	*
	* @param void
	*
	* @returns float
	*
	*/
	float get_u(void)
	{
		lock_guard <mutex> u_lock(u_mutex);
		return u;
	}

	/*! @brief Function to acquire stonefan status for logging purposes
	*
	* 
	*
	* @param void
	*
	* @returns bool
	*
	*/
	bool get_stoneFAN(void)
	{
		lock_guard <mutex> u_lock(u_mutex);
		return stoneFAN;
	}

	/*! @brief Function to acquire mainfan and window status for logging purposes
	*
	* 
	*
	* @param void
	*
	* @returns bool
	*
	*/
	bool get_mainFAN(void)
	{
		lock_guard <mutex> u_lock(u_mutex);
		return mainFAN;
	}

	/*! @brief Function to acquire termperature reference for logging purposes
	*
	* 
	*
	* @param void
	*
	* @returns float
	*
	*/
	float get_ref(void)
	{
		lock_guard <mutex> r_lock(r_mutex);
		return r;
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
			sem_wait(sem_control);



			// Do prognosis analysis if need be
			_prog_counter++;
			if(_prog_counter > (1800/TIME_STEP))
			{
				// Update the prognosis
        		progdownload(_down_data);

        		// read the prognosis for changes
   				p_loader.update();

        		// Load data:
				_prog_anal_data = p_loader.get_data();
			}

			// Wait for the temperature to finish its iteration and load the updated temperature
			sem_wait(sem_temp_ready);
			get_temp();

			if(_prog_counter > (1800/TIME_STEP))
			{
				// Let the Prognosis analyser do its magic:
				r = p_analyser.panalyse(_prog_anal_data, tm.T_inside, tm.T_outmean, _prognosis_number);
				// Reset counter
				_prog_counter = 0;
			}

			// Do regular controlling jobs
			y = tm.T_inside;
			controller();
			plant();
		}
		digitalWrite(RELAY_1_P1, LOW);
		digitalWrite(L298N_STONE, LOW);
		window.WaitForInternalThreadToExit();

    }
	

private:
	/*! @brief Function to ask DS18B20 class for temperature structure
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	void get_temp()
	{
		tempobj->meas_get(&tm);
	}

	/*! @brief Function that takes care of controller.
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
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

	/*! @brief Function that akes ccare of plant
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	void plant(void)
	{
		lock_guard <mutex> u_lock(u_mutex);

		// main fan & window
		if((u > 20) && (tm.T_outmean > tm.T_inside))
		{
			window.open();
			mainFAN = true;

			if(u > 40)
			{
				digitalWrite(RELAY_1_P1, HIGH);
			}
			else
			{
				digitalWrite(RELAY_1_P1, LOW);
			}
		}
		else if((u < -20) || (tm.T_inside > Tmax))
		{
			window.open();
			mainFAN = true;

			if(u < -40)
			{
				digitalWrite(RELAY_1_P1, HIGH);
			}
			else
			{
				digitalWrite(RELAY_1_P1, LOW);
			}
		}
		else 
		{
			digitalWrite(RELAY_1_P1, LOW);
			window.close();
			mainFAN = false;
		}
		
		// stonebed
		if(((r > Tdes) || (u < -5)) && (tm.T_stonemean < tm.T_inside))
		{
			digitalWrite(L298N_STONE, HIGH);
			stoneFAN = true;
		}
		else if(((r < Tdes) || (u > 5)) && (tm.T_stonemean > tm.T_inside))
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
	int _prog_counter = 1800/TIME_STEP;

	// Constants
	float K = 1.2;
	float Ke = 0.32;
	float Ts = 10;
	float r;
	float Tmax;
	float Tmin;
	float Tdes;

	// Input/output
	TERMINAL_CONTROLLER* tercon;
	DS18B20* tempobj;
	sem_t* sem_control;
	sem_t* sem_temp_ready;
	Temp_measurement tm;
	WINDOW_CONTROLLER window;
	PANALYSIS p_analyser;
	PROGLOAD p_loader;
	Queue < float, 10 > inTempQ;
	vector< prognosis_downlaod_structure > _down_data;
	int _prognosis_number;
	vector< prognosis_data_structure > _prog_anal_data;

	
	mutex u_mutex;
	mutex r_mutex;

	bool stoneFAN = false;
	bool mainFAN = false;
};