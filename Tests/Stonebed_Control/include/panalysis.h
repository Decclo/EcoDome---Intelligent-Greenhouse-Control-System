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

#define	DEBUGSTATE		1		// Debugstate describes if there should happen debugging, 0 for minimal, 1 for maximal


struct structureintegrity
{
	bool dirExists = 0;
	bool wdatExists[100] = {0};
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
void progdownload(vector< vector<string> > _progdown_v);


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
	PANALYSIS();

	/*! @brief takes in the prognoses, does some magic, and returns result
	*
	* 
	*
	* @param vector< vector<float> > _progin_v, int _pn
	*
	* @returns void
	*
	*/
	float panalyse(vector< vector<float> > _progin_v, int _pn);

protected:

private:

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
	CONFLOAD(string _s);

	/*! @brief looks in the config for any datasets, converts them into vectors and adds the path for download
	*
	* 
	*
	* @param vector<string> progdat_v, string _path
	*
	* @returns void
	*
	*/
	void get_progdata(vector< vector<string> >& _progdat_v, string _path = "./data");

	/*! @brief looks in the config for any datasets
	*
	* 
	*
	* @param vector<string> progdat_v
	*
	* @returns void
	*
	*/
	void get_prog_number(int& _pn);

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
    PROGLOAD(vector<string> _prog_conf_v, int _i = 38);

	/*! @brief Updates the contents of the vector containing the data
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	void update(void);

	void print_data(void);


	/*! @brief Checks if all neccessary files are present
	*
	* 
	*
	* @param void
	*
	* @returns void
	*
	*/
	void self_check(void);


	/*! @brief checks if element exists in filesystem
	*
	* 
	*
	* @param string _elementPath
	*
	* @returns bool
	*
	*/
	static inline bool does_exist (const string& s);


	/*! @brief returns vector containing datasets in string format
	*
	* 
	*
	* @param void
	*
	* @returns vector< vector<string> >
	*
	*/
	vector< vector<string> > get_data_string(void);
	

	/*! @brief returns vector containing datasets in float format
	* 
	* 
	*
	* @param void
	*
	* @returns vector< vector<string> >
	*
	*/
	vector< vector<float> > get_data_float(void);


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
	vector<string> load_file(const string _fpath);


	/*! @brief loads a fallback error message
	*
	* 
	*
	* @param void
	*
	* @returns vector<string>
	*
	*/
	vector<string> load_fallback(void);


	// Input
	const string _dirName;						// Where is the data located?
	const string _fileName;						// What are the names of the files (without numbers)?
	const int _items;							// How many items are there (from 0 to...)?

	// Put-put
	structureintegrity _structureintegrity;		// Does the path and all files exist?
	bool _structureintegrityPass;				// If everything is all-right put this bool =1

	// Output
	vector< vector<string> > _data;					// Vector to hold data

};