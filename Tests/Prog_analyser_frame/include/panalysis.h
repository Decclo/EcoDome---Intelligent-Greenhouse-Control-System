#pragma once

/*
* panalysis.h
* Author:		Hans V. Rasmussen
* Created:		07/03-2018 13:00
* Modified:		17/05-2018 13:30
* Version:		1.1
*
* Description:
*	This library includes everything one needs to analyse prognoses from weather stations for use with control systems.
*
* NOTE:
*	http://www.cplusplus.com/forum/general/833/
*	For the code to compile one needs to install 'python3' and 'libconfig++-dev' on their system
*
*/

// standard includes
#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <string>
#include <pthread.h>

// includes required for Config parsing
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>

using namespace std;
using namespace libconfig;



// ###############################################		DEFINES		#################################################### //

#define	DEBUGSTATE		0		// Debugstate describes if there should happen debugging, 0 for minimal, 1 for maximal


struct structureintegrity
{
	bool dirExists = 0;
	bool wdatExists[100] = {0};
};

struct prognosis_downlaod_structure
{
	string url;
	string file_path;
	string file_name;
};

struct prognosis_data_structure
{
	string valid;
	float wind_dir = 0;
	float wind_speed = 0;
	float temperature = 0;
	float air_pressure = 0;
};



// ###############################################		FUNCTIONS		#################################################### //

	/*! @brief Downlaods the files used by PROGLOAD
	*
	* 
	*
	* @param 
	*		vector< vector<string> > _progdown_v
	*
	* @returns void
	*
	*/
void progdownload(vector< prognosis_downlaod_structure > _progdown_v)
{
    // Make a string to hold the command for the DataDown python script
    string DDpy;

    // Read out the data if debugstate is true
    #if DEBUGSTATE
		for(int i=0; i<_progdown_v.size(); ++i)
		{
			cout << "_progdown is " << i << endl;
			cout << "\t" << _progdown_v[i].url << "\n";
			cout << "\t" << _progdown_v[i].file_path << "\n";
			cout << "\t" << _progdown_v[i].file_name << "\n";
			cout << endl;
		}
	#endif	// DEBUGSTATE
		
	// Prepare the string that holds the command to run the DataDown Python script
	for(int i=0; i<_progdown_v.size(); ++i)
	{
		// Call the python script
		DDpy = "python3 ./DataDown.py ";
		// Add the arguments
		DDpy += "-wl " + _progdown_v[i].url + " -dn " + _progdown_v[i].file_path + _progdown_v[i].file_name;
		#if !DEBUGSTATE
			// Suppress output
			DDpy += " > /dev/null"; 
		#endif	// DEBUGSTATE

		// Write out complete script before running it
		#if DEBUGSTATE
			cout << "Running command:\n" << DDpy << "\n\n\n";
		#endif	// DEBUGSTATE

		// Forward the DataDown Python command to the system and wait for it to finish
		system(DDpy.c_str());
	}
}


// ###############################################		CLASSES		#################################################### //

	/*! @brief	Class that contains the code for doing the actual analysis
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
class PANALYSIS
{
public:

	/*! @brief Constructor
	*
	* 
	*
	* @param 
	*		void
	*
	* @returns void
	*
	*/
	PANALYSIS()
	{

	}

	/*! @brief takes in the prognoses, does some magic, and returns result
	*
	* 
	*
	* @param vector< vector<float> > _progin_v, int _pn
	*
	* @returns void
	*
	*/
	float panalyse(vector< prognosis_data_structure > _progin_v, int _pn)
	{
	// make temporary variable
	float _presult_tmp = 0;

		// Run trough all the datasets
		for(int i = 0; i < _pn; i++)
		{
			#if 1
            	cout << "tmp " << to_string(i) << " is " << _progin_v[i].temperature << endl;
			#endif	// DEBUGSTATE
			_presult_tmp += _progin_v[i].temperature;
		}
		return _presult_tmp/_pn;
	}

protected:

private:
	float Tmin;
	float Tmax;
	float Tdes;
	float Tref;

};


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

private:
	string conf_file;
	Config cfg;
};


	/*! @brief	
	*
	*
	*	@use	
	*
	@code{.cpp}
	*
	* @endcode
	*
	*/
class PROGLOAD
{
public:


    /*! @brief Constructor
	*
	* 
	*
	* @param 
	*		string _directoryName = "./Data/", 
	*		string _fileName = "wData", 
	*		int _items = 38
	*
	* @returns void
	*
	*/
    PROGLOAD() : _structureintegrityPass(0), _items(0)
	{   

	}

	void initiate(prognosis_downlaod_structure _pcf, int _i)
	{
		
		_pconf = _pcf;
		_items = _i;
		self_check();
	}

	/*! @brief Updates the contents of the vector containing the data
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	void update(void)
	{
	    // check the integrity
	    self_check();
	    // Check if the integrity check passed, if not give a headsup
		if(_structureintegrityPass == false)
		{
			cout << "Previous Data integrity test failed, rerunning test..." << endl;
		}

	    // Destroy all elements in the vector containing the prognosis data
		_pdata.clear();	
		// fill vector again with the data from the files
		for(int i = 0; i < _items; i++)
		{
			if(_structureintegrity.wdatExists[i])
			{
				_pdata.push_back(load_file(_pconf.file_path + _pconf.file_name + to_string(i)));
			}
			else
			{
				_pdata.push_back(load_fallback());
			}
		}
	}

	void print_data(void)
	{
 	   // For debugging, print the contents of the loaded data
		for(int i=0; i<_pdata.size(); ++i)
		{
			cout << "\nDataset " << i << endl;
			cout << "\t" << _pdata[i].valid << "\n";
			cout << "\t" << _pdata[i].wind_dir << "\n";
			cout << "\t" << _pdata[i].wind_speed << "\n";
			cout << "\t" << _pdata[i].temperature << "\n";
			cout << "\t" << _pdata[i].air_pressure << "\n";
		}
	}


	/*! @brief Checks if all neccessary files are present
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	void self_check(void)
	{
		// Check if Directory exists:
		if(does_exist(_pconf.file_path))
		{
			#if DEBUGSTATE
				cout << "Checking Directory..." << endl;
				cout << "\t'" << _pconf.file_path << "'" << "\t[PASSED]" << endl;
				cout << "Checking Files..." << endl;
			#endif	// DEBUGSTATE
			_structureintegrity.dirExists = true;

				// If directory does exist, check if individual files exist:
			int _passTracker = 0;
			for(int i = 0; i < _items; i++)
			{
				if(does_exist(_pconf.file_path + _pconf.file_name + to_string(i)))
				{
					_structureintegrity.wdatExists[i] = true;
	    			_passTracker++;
					#if DEBUGSTATE
						cout << "\t'" << _pconf.file_path + _pconf.file_name + to_string(i) << "'" << "\t[PASSED]" << endl;
					#endif	// DEBUGSTATE
				}
				else
				{
					#if DEBUGSTATE
						cout << "\t'" << _pconf.file_path + _pconf.file_name + to_string(i) << "'" << "\t[!FAILED!]" << endl;
					#endif	// DEBUGSTATE
					_structureintegrity.wdatExists[i] = false;
				}
			}
	        // if everything 
			if(_passTracker == (_items))
			{
				_structureintegrityPass = true;
			}
			else
			{
				_structureintegrityPass = false;
			}
		}
		else
		{
	        // If directory was not found, set integrity to failed
			#if DEBUGSTATE
				cout << "Checking Directory..." << endl;
				cout << "\t'" << _pconf.file_path << "'" << "\t[!FAILED!]" << endl;
			#endif	// DEBUGSTATE
			_structureintegrity.dirExists = false;

			for(int i = 0; i < _items; i++)
			{
				_structureintegrity.wdatExists[i] = false;
			}

			_structureintegrityPass = false;
		}

		#if DEBUGSTATE
			if(_structureintegrityPass == true)
			{
				cout << "\nData integrity test\t[PASSED]\n" << "\n\n";
			}
			else
			{
				cout << "\nData integrity test\t[!FAILED!]\n" << endl;
			}
		#endif // DEBUGSTATE
	}


	/*! @brief checks if element exists in filesystem
	*
	* 
	*
	* @param string _elementPath
	*
	* @returns bool
	*
	*/
	static inline bool does_exist (const string& s)
	{
	    // Check if directory exists
		struct stat buffer;   
		return (stat (s.c_str(), &buffer) == 0); 
	}
	

	/*! @brief returns vector containing datasets in float format
	* 
	* 
	*
	* @param void
	*
	* @returns vector< vector<string> >
	*
	*/
	vector< prognosis_data_structure > get_data(void)
	{
		return _pdata;
	}


private:

	/*! @brief loads contents of a file
	*
	* 
	*
	* @param string _elementPath
	*
	* @returns vector<string>
	*
	*/
	prognosis_data_structure load_file(const string _fpath)
	{
	    // Make a string vector and fill it with the contents of a file
		prognosis_data_structure _fdata;
		string line;
		ifstream _fpointer(_fpath);

		if(_fpointer.is_open())
		{
			getline(_fpointer, line);
			_fdata.valid = line;
			getline(_fpointer, line);
			_fdata.wind_dir = stof(line);
			getline(_fpointer, line);
			_fdata.wind_speed = stof(line);
			getline(_fpointer, line);
			_fdata.temperature = stof(line);
			getline(_fpointer, line);
			_fdata.air_pressure = stof(line);

			_fpointer.close();
		}
		else
		{
	        // If the file cannot be opened, give a headsup
			cout << "Unable to open file " << _fpath << endl;
		}

		return _fdata;
	}


	/*! @brief loads a fallback error message
	*
	* 
	*
	* @param void
	*
	* @returns prognosis_data_structure
	*
	*/
	prognosis_data_structure load_fallback(void)
	{
	    // If it is not possible to get the data from a file, fill it with this instead:
		prognosis_data_structure _fbdata;

		_fbdata.valid = "[ERROR!]";
		_fbdata.wind_dir = 0;
		_fbdata.wind_speed = 0;
		_fbdata.temperature = 0;
		_fbdata.air_pressure = 0;

		return _fbdata;
	}


	// Input
	prognosis_downlaod_structure _pconf;
	int _items;								// How many items are there (from 0 to...)?

	// Put-put
	structureintegrity _structureintegrity;			// Does the path and all files exist?
	bool _structureintegrityPass;					// If everything is all-right put this bool =1

	// Output
	vector< prognosis_data_structure > _pdata;	// Vector to hold data

};