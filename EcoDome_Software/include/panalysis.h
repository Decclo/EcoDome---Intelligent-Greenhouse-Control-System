#pragma once

/*
* panalysis.h
* Author:		Hans V. Rasmussen
* Created:		07/03-2018 13:00
* Modified:		01/04-2018 14:30
* Version:		1.0
*
* Description:
*	This library includes everything one needs to analyse prognoses from weather stations for use with control systems.
*
* NOTE:
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

using namespace std;



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
	}
	// Forward the DataDown Python command to the system and wait for it to finish
	system(DDpy.c_str());
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
	PANALYSIS(float _tmi, float tma, float tde) : Tmin(_tmi), Tmax(tma), Tdes(tde)
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
	float panalyse(vector< prognosis_data_structure > _progin_v, float _Tin, float _Tout, int _pn)
	{
		if (_progin_v[_pn-1].temperature > (Tmax-(_Tin - _Tout)))
		{
			Tref = Tdes - ((Tmax - (_Tin - _Tout)) + _progin_v[_pn-1].temperature);
			if(Tref < (Tmin + 0.5))
			{
				Tref = Tmin + 1;
			}
		}
		else if (_progin_v[_pn-1].temperature < (Tmin + (_Tin - _Tout)))
		{
			Tref = Tdes + ((Tmin + (_Tin - _Tout)) - _progin_v[_pn-1].temperature);
			if(Tref > (Tmax - 0.5))
			{
				Tref = Tmax - 1;
			}
		}
		else
		{
			Tref = Tdes;
		}
		return Tref;
	}

protected:

private:
	float Tmin;
	float Tmax;
	float Tdes;
	float Tref;

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