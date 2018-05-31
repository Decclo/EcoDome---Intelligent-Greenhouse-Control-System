#pragma once

/*
* debug_logger.h
* Author:		Hans V. Rasmussen
* Created:		07/05-2018 13:00
* Modified:		16/05-2018 14:30
* Version:		1.2
*
* Description:
*	This header includes functionality to syncronize threads, as well as controlling the terminal and logging data to a file.
*
* NOTE:
*
*/

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

// includes required for Config parsing
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>

#include "mythread.h"

using namespace std;
using namespace libconfig;


// ###############################################		DEFINES		#################################################### //

#define TIME_STEP		10

mutex mux_log;

// structures
struct destemp
{
	float T_max = 0;
	float T_des = 0;
	float T_min = 0;
};


// ###############################################		CLASSES		#################################################### //

	/*! @brief	Class that handles everything regarding the Config.cfg
	*	https://github.com/hyperrealm/libconfig
	*
	*
	*	@use	
	*
	@code{.cpp}
	*
	* @endcode
	*
	*/
class CONFLOAD
{
public:

	/*! @brief Constructor
	*
	* 
	*
	* @param 
	*		string config_path
	*
	* @returns void
	*
	*/
	CONFLOAD(string _s) : conf_file(_s)
	{
	    // try to read the configuration
		try
		{
			cfg.readFile(conf_file.c_str());
		}
	    // If reading the configuration fails bacause of not being able to access the file, write an error message
		catch(const FileIOException &fioex)
		{
			std::cerr << "I/O error while reading configuration file, please make sure that it exists." << std::endl;
			exit;
		}
	    // If the config file exists and is readable, but shows errors regarding syntax, return another error message
		catch(const ParseException &pex)
		{
			std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
			exit;
		}
	}

	/*! @brief looks in the config for any datasets, converts them into vectors and adds the path for download
	*
	* 
	*
	* @param vector<string> progdat_v, string _path
	*
	* @returns void
	*
	*/
	void get_progdata(vector< prognosis_downlaod_structure >& _progdat_v, string _path = "./data")
	{
	    // Make a vector to hold the url and path of the datasets
		prognosis_downlaod_structure _progtmp;
		// Prepare the parser and try retrieving the datasets
		const Setting& root = cfg.getRoot();
		try
		{
			const Setting &progdat = root["data"]["progdata"];
			int count = progdat.getLength();
			cout << "Found " << count << " dataset(s)." << endl;
			string url;
			string tmp_path;
			string dat_name = "wdat";

			// if there are no datasets available exit the program
			if(!count)
			{
				cout << "No datasets registered in config file, please specify where to take prognosis from." << endl;
				exit;
			}

			// go trough the datasets and put them into the given vector 
			for(int i = 0; i < count; ++i)
			{
				// Load the url into the tmp string
				const Setting &dataset = progdat[i];
				dataset.lookupValue("dataset" + to_string(i+1), url);
				tmp_path = _path + "/dataset" + to_string(i+1) + "/";

				// Load tmp into our string vector
			    for(int k = 0; k < 2; ++k)
	            {
	                _progtmp.url = url;
	                _progtmp.file_path = tmp_path;
					_progtmp.file_name = dat_name;
	            }
	            // Load our string into the given vector    
				_progdat_v.push_back(_progtmp);
			}
		}
	    // Give a headsup if any errors occur, and exit program
		catch(const SettingNotFoundException &nfex)
		{
			cout << "Fatal error: Not able to load dataset URL from configuration file\n Exiting..." << endl;
			exit(1);
		}
	}

	/*! @brief looks in the config for any datasets
	*
	* 
	*
	* @param vector<string> progdat_v
	*
	* @returns void
	*
	*/
	void get_prog_number(int& _pn)
	{
		// Prepare the parser and try retrieving the number of prognoses to be used during calculations
		const Setting& root = cfg.getRoot();
		try
		{
			const Setting &data = root["data"];
			if(!data.exists("prog_number"))
			{
				cout << "No prog_number registered in config file." << endl;
				exit;
			}

			data.lookupValue("prog_number", _pn);
		}
		catch(const SettingNotFoundException &nfex)
		{
			// Ignore.
		}
	}

	/*! @brief looks in the config for the minimum, maximum and desired temperatures
	*
	* 
	*
	* @param destemp&
	*
	* @returns void
	*
	*/
	void get_minmaxdes(destemp& _t_vals)
	{
		// Prepare the parser and try retrieving the number of prognoses to be used during calculations
		const Setting& root = cfg.getRoot();
		try
		{
			const Setting& general = root["general"];

			// Look for the max temperature
			if(!general.exists("max"))
			{
				cout << "No maximum registered in config file." << endl;
				exit;
			}
			general.lookupValue("max", _t_vals.T_max);

			// look for the desired tempperature
			if(!general.exists("des"))
			{
				cout << "No desired registered in config file." << endl;
				exit;
			}
			general.lookupValue("des", _t_vals.T_des);

			// look for the minimum tempperature
			if(!general.exists("min"))
			{
				cout << "No minimum registered in config file." << endl;
				exit;
			}
			general.lookupValue("min", _t_vals.T_min);
		}
		catch(const SettingNotFoundException &nfex)
		{
			// Ignore.
		}
	}

private:
	string conf_file;
	Config cfg;
};


class LOGGER
{
public:
    /*! @brief Constructor
	*
	* 
	*
	* @param vector< string > _det
	*
	* @returns void
	*
	*/
    LOGGER(vector < string >* _des) : data_descriptor(*_des)
    {
    	logdata.open (prepare_file_name());
    	logdata << "Test started at " << currentDateTime() << endl << "Time step is " << TIME_STEP << endl << "TimeStamp_DateTime";
		for (int i=0; i < data_descriptor.size(); i++)
		{
			logdata << "\t" << data_descriptor[i];
		}
		logdata << endl;
    }

    /*! @brief Function to update the logfile
	*
	* 
	*
	* @param vector< string > _det
	*
	* @returns void
	*
	*/
    void update(vector< string > _dat)
    {
		logdata << currentDateTime();
        for(int i=0; i < _dat.size(); i++)
		{
			logdata << "\t" << _dat[i];
		}
        logdata << endl;
    }


protected:

private:

    /*! @brief Function to print the date and time
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
    	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    	return buf;
	}

	/*! @brief Function to print the time
	*
	* 
	*
	* @param void
	*
	* @returns const string
	*
	*/
    const string currentTime()
	{
    	time_t     now = time(0);
    	struct tm  tstruct;
    	char       buf[80];
    	tstruct = *localtime(&now);
    	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    	// for more information about date/time format
    	strftime(buf, sizeof(buf), "%X", &tstruct);

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


// ###############################################		THREADS 	#################################################### //

	/*! @brief	Class that helps syncronizing threads by controlling acces to terminal and using commands from terminal.
	*
	*	@use	
	*
	@code{.cpp}
	*
	* @endcode
	*
	*/
class TERMINAL_CONTROLLER : public MyThreadClass
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
	TERMINAL_CONTROLLER()
	{

	}

    /*! @brief Function to write to terminal
	*
	* 
	*
	* @param string
	*
	* @returns void
	*
	*/
	void term_write(string _str)
	{
		lock_guard <mutex> terminal_lock(terminal_mutex);

		cout << '\b' << '\b';
		cout << _str;
		cout << "\n> ";
	}

    /*! @brief Function to retrieve current program status, should be checked by all while(1) loops.
	*
	* 
	*
	* @param void
	*
	* @returns bool
	*
	*/
	bool pos(void)
	{
		lock_guard <mutex> pos_lock(pos_mutex);
		return program_operation_state;
	}

protected:
	/** Implement this method in your subclass with the code you want your thread to run. */
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
			else
			{
				term_write_control("Command not recognized!");
			}
		}

    }

private:
    /*! @brief Function that prints the help message
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	void help_msg(void)
	{
		string h_msg;
		h_msg += "--Help message--\n";
		h_msg += "Valid commands are:\n";
		h_msg += "	'help' 'h' '?'		- Shows Help message\n";
		h_msg += "	'q' 'quit' 'exit'	- Exits this program by stopping all processes and actuators\n";
		term_write_control(h_msg);
	}

    /*! @brief Function to write to the terminal, does not delete the ">" before writing
	*
	* 
	*
	* @param string
	*
	* @returns void
	*
	*/
	void term_write_control(string _str)
	{
		lock_guard <mutex> terminal_lock(terminal_mutex);

		cout << endl << _str << endl << "> ";
	}

	string terminal_input;

	std::mutex pos_mutex;
	bool program_operation_state = true;

	std::mutex terminal_mutex;
};


