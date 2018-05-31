// Standard Libraries
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>

// Custom Libraries
#include "../include/panalysis.h"

// Define namespaces
using namespace std;
using namespace libconfig;

const string currentTime(void);

// Main
int main(void)
{
    //  ########## Preparing configuration ##########   //
    vector< prognosis_downlaod_structure > _progdown_data;
    vector< prognosis_data_structure > _proganal_data;
    int prog_number = 0;

    CONFLOAD cfgload("./Config.cfg");
    cfgload.get_progdata(_progdown_data);
    cfgload.get_prog_number(prog_number);


    //  ########## Doing Analysis ##########   //
    PANALYSIS panalyser;

    float _mean_val = 0;
    // Prepare object for loading data:
	PROGLOAD prognosis_loader;
    prognosis_loader.initiate(_progdown_data[0], prog_number);
	while(1)
	{
        cout << "Timestamp: " << currentTime() << endl;
        
        // Update the files
        progdownload(_progdown_data);

        // read the files for changes
   		prognosis_loader.update();

        // Load data:
		_proganal_data = prognosis_loader.get_data();

		// Do some magic~~~~
        _mean_val = panalyser.panalyse(_proganal_data, prog_number);

        // pritn result and wait a while
        cout << "Prognosis Analyser result is " << _mean_val << "\n\n\n";
        sleep(10);
    }
    
    
    return 0;
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
const string currentTime(void)
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
