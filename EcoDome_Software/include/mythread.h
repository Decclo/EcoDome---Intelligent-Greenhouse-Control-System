#pragma once

/*
* mythread.h
* Author:		Hans V. Rasmussen
* Created:		13/04-2018 15:00
* Modified:		13/04-2018 15:00
* Version:		1.0
*
* Description:
*	This header includes a handy class that runs a thread, creating its own little environment.
*
* NOTE:
*	Taken from https://stackoverflow.com/questions/1151582/pthread-function-from-a-class
*	All credit goes to Jeremy Friesner from stackoverflow.com
*
*/

#include <unistd.h>
#include <pthread.h>

	/*! @brief	Base class to be used when making threads
	*
	*
	*	@use	
	*
	@code{.cpp}
	*
	* @endcode
	*
	*/
class MyThreadClass
{
public:
	MyThreadClass() {/* empty */}
	virtual ~MyThreadClass() {/* empty */}

	/** Returns true if the thread was successfully started, false if there was an error starting the thread */
	bool StartInternalThread()
	{
		return (pthread_create(&_thread, NULL, InternalThreadEntryFunc, this) == 0);
	}

	/** Will not return until the internal thread has exited. */
   	void WaitForInternalThreadToExit()
	{
		(void) pthread_join(_thread, NULL);
	}

protected:
	/** Implement this method in your subclass with the code you want your thread to run. */
	virtual void InternalThreadEntry() = 0;

private:
	static void * InternalThreadEntryFunc(void * This) {((MyThreadClass *)This)->InternalThreadEntry(); return NULL;}

	pthread_t _thread;
};