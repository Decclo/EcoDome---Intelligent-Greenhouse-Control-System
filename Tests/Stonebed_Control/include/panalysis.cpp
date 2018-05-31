#include <sys/stat.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <unistd.h>
#include <string>
#include <pthread.h>

// Libraries for configuration parsing
#include <iomanip>
#include <cstdlib>
#include <libconfig.h++>

// Headerfile of this library
# include "panalysis.h"

// Definition of namespaces
using namespace std;
using namespace libconfig;



// ###############################################		FUNCTIONS		#################################################### //

void progdownload(vector< vector<string> > _progdown_v)
{
    // Make a string to hold the command for the DataDown python script
    string DDpy;

    // Read out the data if debugstate is true
    #if DEBUGSTATE
		for(int i=0; i<_progdown_v.size(); ++i)
		{
			cout << "_progdown_v " << i << endl;
			for(int k=0; k<_progdown_v[i].size(); ++k)
			{
				cout << "\t" << _progdown_v[i][k] << "\n";
			}
			cout << endl;
		}
	#endif	// DEBUGSTATE
		
	// Prepare the string that holds the command to run the DataDown Python script
	for(int i=0; i<_progdown_v.size(); ++i)
	{
		// Call the python script
		DDpy = "python3 ./DataDown.py ";
		// Add the arguments
		DDpy += "-wl " + _progdown_v[i][0] + " -dn " + _progdown_v[i][1] + "wdat";
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

PANALYSIS::PANALYSIS()
{

}

float PANALYSIS::panalyse(vector< vector<float> > _progin_v, int _pn)
{
	// make temporary variable
	float _presult_tmp = 0;

		// Run trough all the datasets
		for(int i = 0; i < _pn; i++)
		{
			#if DEBUGSTATE
            	cout << "tmp " << to_string(i) << " is " << _progin_v[i][3] << endl;
			#endif	// DEBUGSTATE
			_presult_tmp += _progin_v[i][3];
		}
		return _presult_tmp/_pn;
}


CONFLOAD::CONFLOAD(string _s) : conf_file(_s)
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

void CONFLOAD::get_progdata(vector< vector<string> >& _progdat_v, string _path)
{
    // Make a vector to hold the url and path of the datasets
	vector<string> _progtmp_v;
	// Prepare the parser and try retrieving the datasets
	const Setting& root = cfg.getRoot();
	try
	{
		const Setting &progdat = root["data"]["progdata"];
		int count = progdat.getLength();
		cout << "Found " << count << " dataset(s)." << endl;
		string tmp;
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
			dataset.lookupValue("dataset" + to_string(i+1), tmp);
			tmp_path = _path + "/dataset" + to_string(i+1) + "/";

			// Load tmp into our string vector
		    for(int k = 0; k < 2; ++k)
            {
                _progtmp_v.clear();
                _progtmp_v.push_back(tmp);
                _progtmp_v.push_back(tmp_path);
				_progtmp_v.push_back(dat_name);
            }
            // Load our string into the given vector    
			_progdat_v.push_back(_progtmp_v);
		}
	}
    // Give a headsup if any errors occur, and exit program
	catch(const SettingNotFoundException &nfex)
	{
		cout << "Fatal error: Not able to load dataset URL from configuration file\n Exiting..." << endl;
		exit(1);
	}
}

void CONFLOAD::get_prog_number(int& _pn)
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


PROGLOAD::PROGLOAD(vector<string> _prog_conf_v, int _i) : _dirName(_prog_conf_v[1]),  _fileName(_prog_conf_v[2]), _items(_i)
{   
    // Force an integrity check
	_structureintegrityPass = 0;
	self_check();
}

void PROGLOAD::update(void)
{
    // check the integrity
    self_check();
    // Check if the integrity check passed, if not give a headsup
	if(_structureintegrityPass == false)
	{
		cout << "Previous Data integrity test failed, rerunning test..." << endl;
	}

    // Destroy all elements in the vector containing the prognosis data
	_data.clear();	
	// fill vector again with the data from the files
	for(int i = 0; i < _items; i++)
	{
		if(_structureintegrity.wdatExists[i])
		{
			
			_data.push_back(load_file(_dirName + _fileName + to_string(i)));
		}
		else
		{
			_data.push_back(load_fallback());
		}
	}
}

void PROGLOAD::print_data(void)
{
    // For debugging, print the contents of the loaded data
	for(int i=0; i<_data.size(); ++i)
	{
		cout << "\nDataset " << i << endl;
		for(int k=0; k<_data[i].size(); ++k)
		{
			cout << "\t" << _data[i][k] << "\n";
		}
	}
}

void PROGLOAD::self_check(void)
{
	// Check if Directory exists:
	if(does_exist(_dirName))
	{
		#if DEBUGSTATE
			cout << "Checking Directory..." << endl;
			cout << "\t'" << _dirName << "'" << "\t[PASSED]" << endl;
			cout << "Checking Files..." << endl;
		#endif	// DEBUGSTATE
		_structureintegrity.dirExists = true;

		// If directory does exist, check if individual files exist:
		int _passTracker = 0;
		for(int i = 0; i < _items; i++)
		{
			if(does_exist(_dirName + _fileName + to_string(i)))
			{
				_structureintegrity.wdatExists[i] = true;
    			_passTracker++;
				#if DEBUGSTATE
					cout << "\t'" << _dirName + _fileName + to_string(i) << "'" << "\t[PASSED]" << endl;
				#endif	// DEBUGSTATE
			}
			else
			{
				#if DEBUGSTATE
					cout << "\t'" << _dirName + _fileName + to_string(i) << "'" << "\t[!FAILED!]" << endl;
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
			cout << "\t'" << _dirName << "'" << "\t[!FAILED!]" << endl;
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

inline bool PROGLOAD::does_exist (const string& s) 
{
    // Check if directory exists
	struct stat buffer;   
	return (stat (s.c_str(), &buffer) == 0); 
}

vector< vector<string> > PROGLOAD::get_data_string(void)
{
    // Return the data gathered as a string
	return _data;
}

vector< vector<float> > PROGLOAD::get_data_float(void)
{
    // Convert the data gathered to float and return
	vector< vector<float> > _fdata;
	vector<float> _fdata_tmp;
	for(int i=0; i<_data.size(); ++i)
	{
		_fdata_tmp.clear();
		_fdata_tmp.push_back(0.0);
		for(int k=1; k<_data[i].size(); ++k)
		{
			_fdata_tmp.push_back( stof(_data[i][k]) );
		}
		_fdata.push_back(_fdata_tmp);
	}
	return _fdata;
}

vector<string> PROGLOAD::load_file(const string _fpath)
{
    // Make a string vector and fill it with the contents of a file
	vector<string> _fdata;
	string line;
	ifstream _fpointer(_fpath);

	if(_fpointer.is_open())
	{
		while(getline(_fpointer, line))
		{
			_fdata.push_back(line);
		}
		_fpointer.close();
	}
	else
	{
        // If the file cannot be opened, give a headsup
		cout << "Unable to open file " << _fpath << endl;
	}

	return _fdata;
}

vector<string> PROGLOAD::load_fallback(void)
{
    // If it is not possible to get the data from a file, fill it with this instead:
	vector<string> _fbdata;

	_fbdata.push_back("[ERROR!]");
	_fbdata.push_back("1");
	_fbdata.push_back("1");
	_fbdata.push_back("1");
	_fbdata.push_back("1");

	return _fbdata;
}