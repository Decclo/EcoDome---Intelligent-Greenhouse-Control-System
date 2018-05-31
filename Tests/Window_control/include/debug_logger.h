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
#include <condition_variable>

#include "mythread.h"

using namespace std;

#define TIME_STEP		10

mutex mux_log;
condition_variable cv_log;
long unsigned int Ts_counter=0;

class LOGGER
{
public:
    LOGGER(vector < string >* _des) : data_descriptor(*_des)
    {
    	logdata.open (prepare_file_name());
    	logdata << "Test started at " << currentDateTime() << endl << "Time step is " << TIME_STEP << endl << "ite#";
		for (int i=0; i < data_descriptor.size(); i++)
		{
			logdata << "\t" << data_descriptor[i];
		}
		logdata << endl;
    }

    void update(vector< string > _dat)
    {
		Ts_counter++;
		logdata << Ts_counter;
        for(int i=0; i < _dat.size(); i++)
		{
			logdata << "\t" << _dat[i];
		}
        logdata << endl;
    }


protected:

private:
    /*! @brief Function to print the time
	*
	* 
	*
	* @param void
	*
	* @returns const string
	*
	*/
    const string currentDateTime()
	{
    	time_t     now = time(0);
    	struct tm  tstruct;
    	char       buf[80];
    	tstruct = *localtime(&now);
    	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    	// for more information about date/time format
    	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    	return buf;
	}

    /*! @brief Function to check if file exists
	*
	* 
	*
	* @param const string& name
	*
	* @returns inline bool
	*
	*/
	inline bool file_exists (const string& name) 
	{
		struct stat buffer;   
		return (stat (name.c_str(), &buffer) == 0);
	}

    /*! @brief Function to check if the folder to store old log data exists, and create it if not
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	string prepare_file_name(void)
	{
		// Make sure that there is file structure for old logs
	    struct stat statbuf;
		int isDir = 0;
		if (stat("./logs/", &statbuf) != -1) 
		{
			if (S_ISDIR(statbuf.st_mode))
			{
	    		isDir = 1;
			}
		}	
		else 
		{
			if(system("mkdir ./logs/"))
			{
				exit(1);
			}
		}

		// if a logfile already exists, copy it to a safe space
		int file_counter = 0;
		bool file_count = 1;

		while(file_count)
		{
			if(file_exists("./logs/logdata" + to_string(file_counter) + ".txt"))
			{
				file_counter++;
			}
			else
			{
				file_count = 0;
			}
		}

		return "./logs/logdata" + to_string(file_counter) + ".txt";
	}

    vector < string > data_descriptor;
    ofstream logdata;
	int tcounter = 0;
	std::mutex data_allocation_mutex;

};

class TERMINAL_CONTROLLER : public MyThreadClass
{
public:
	TERMINAL_CONTROLLER()
	{

	}

	void term_write(string _str)
	{
		lock_guard <mutex> terminal_lock(terminal_mutex);

		cout << '\b' << '\b' << _str << endl << "> ";
	}

	bool pos(void)
	{
		lock_guard <mutex> pos_lock(pos_mutex);
		return program_operation_state;
	}

	bool dir(void)
	{
		lock_guard <mutex> dir_lock(dir_mutex);
		return win_dir;
	}

protected:
    void InternalThreadEntry()
    {
		help_msg();
		while(this->pos())
		{
			getline(cin, terminal_input);

			if(terminal_input == "help" || terminal_input == "h" || terminal_input == "?")
			{
				help_msg();
			}
			else if(terminal_input == "q" || terminal_input == "quit" || terminal_input == "exit")
			{
				term_write_control("Program shutting down on next iteration...");
				pos_mutex.lock();
				program_operation_state = false;
				pos_mutex.unlock();
			}
			else if(terminal_input == "f")
			{
				dir_mutex.lock();
				win_dir = true;
				dir_mutex.unlock();
			}
			else if(terminal_input == "r")
			{
				dir_mutex.lock();
				win_dir = false;
				dir_mutex.unlock();
			}
			else
			{
				term_write_control("Command not recognized!");
			}
		}

    }

private:
	void help_msg(void)
	{
		string h_msg;
		h_msg += "--Help message--\n";
		h_msg += "Valid commands are:\n";
		h_msg += "	'help' 'h' '?'		- Shows Help message\n";
		h_msg += "	'q' 'quit' 'exit'	- Exits this program by stopping all processes and actuators\n";
		h_msg += "	'f'					- Make motor controller run forward\n";
		h_msg += "	'r'					- Make motor controller run backward\n";
		term_write_control(h_msg);
	}

	void term_write_control(string _str)
	{
		lock_guard <mutex> terminal_lock(terminal_mutex);

		cout << endl << _str << endl << "> ";
	}

	string terminal_input;

	std::mutex pos_mutex;
	bool program_operation_state = true;

	std::mutex terminal_mutex;

	std::mutex dir_mutex;
	bool win_dir = 1;
};


LOGGER* LOGGER_object;
TERMINAL_CONTROLLER* tercon;