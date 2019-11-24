#ifndef parser_h
#define parser_h

#include "names.h"
#include "scanner.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"

#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

typedef struct 			//Device structure to store all defined devices to facilitate error correcting
	{		
	string Name;
	int id;
	devicekind kind;
	int input_number;
	int initState;
	bool isMonitored;	//These facilitate the setting and resetting of monitor points in the GUI
	bool bar = false;	//This is only true for QBAR in the Dtype output
	} dev;
	
typedef struct 			//Connections structure to check if the device input has already been used
	{
	name inDevice;
	name inDevicePin;	
	} cons;

typedef enum {DATA, CLK, SET, CLEAR} dInputs;	//Possible inputs for Dtype

typedef enum {devi, conn, mon, finito} phase; 	//Distinguish between the three input phases

//Possible Syntax and Semantic Errors
enum er { 	
		//Syntax Errors
		DEVICES_not_present = 1, Nonexistent_device_type, Illegal_identifier,
		Wrong_bracket_type, Missing_bracket, Illegal_gate_inputs,
		Invalid_switch_position, Invalid_switch_definition, semicolumn,
		CONNECTIONS_not_present, D_output_illegal, TO_missing,
		no_number_after_I, no_I_before_number, Input_to_Input,
		Input_to_Output, Output_to_Output, MONITOR_not_present,
		Invalid_character, Abrupt_end, Empty_file,
		
		//Semantic Errors
		Reserved_identifier = 101, Invalid_gate_inputs, XOR_inputs,
		Single_output_device, Invalid_gate_syntax, Invalid_switch_syntax,
		Device_has_no_input, D_input_only, D_output_only,
		Undefined_device, Duplicate_device, Muptiple_connections,
		Unused_input, dot_missing, Invalid_D_Input, Invalid_D_Output,
		Connection_overwrite, Invalid_device_input, Nonexistent_pin,
		Monitor_input, Too_many_devices, Too_few_devices,
		
		//Warnings
		Long_identifier = 201, No_connections, No_monitor_points,
		
		//Internal Error
		Internal_error = 301		//We should never get this error in theory
	 };




class parser {

	private:
		network* netz; 						// instantiations of various classes for parser to use.
		devices* dmz; 
		monitor* mmz;
		Scanner* smz;
		names* nmz;

		vector<dev> devList;					//Internal store of created devices
		vector<cons> conList;					//internal store of created connections
		
		bool inputInUse(name device, name pin);

		er error_type;						//Used to report errors
		int error_line;
		string current_line;
		string error_string;
		int nerrors;						//Tracks the number of errors in the definition file
		int nconnections;					//Tracks the number of connections in the definition file

		bool eof;						//Becomes true when we reach the end of file
		phase current_phase;					//Keeps track of the phase we are in

		bool defineDevice(char& ch);				//Master function for Phase I
		devicekind isDeviceType(namestring str);		//Translates Device Defintion to internal representation
		bool readDevice(char &ch, devicekind kind);		//Function to sort devices according to type then call correct function to create device
		bool readSwitch(char &ch);				//Function that defines Switches
		bool readFixedDevice(char &ch, devicekind kind);	//Function that defines XOR and Dtype
		bool readVariableDevice(char &ch, devicekind kind);	//Function that defines logic gates and clocks
		bool readNot(char &ch);
		bool defineConnections(char &ch);			//Master function for Phase II
		devicekind getKind(string identifier);			//Function to retrive internal representation of device from string
		string isDeviceType(int index);				//Function to convert internal representation of device to string
		int ConnectionsRequired();				//Counts the total number of available connections to display warning about any unused devices
		
		bool createMonitor(char &ch);				//Master function for Phase III
		
		bool end_of_file(int &nerrors);				//Final error report before entering Phase IV

		void error_report(er error_type, int error_line, string current_line, string error_string); 	//Reports error on command prompt
		void proceed(char& cur_char, bool& eof); 							//Move to next semicolumn
		
		bool debugging = false;					//If true, displays all devices defined, connections made and monitor points declared
		
	

	public:
		bool readin ();
		/* Reads the definition of the logic system and builds the             */
		/* corresponding internal representation via calls to the 'Network'    */
		/* module and the 'Devices' module.                                    */

		parser (network* network_mod, devices* devices_mod,
		  monitor* monitor_mod, Scanner* scanner_mod, names* names_mod);
		/* the constructor takes pointers to various other classes as parameters */
		
		vector<dev> getDevList();				//This is used by the GUI people
};

#endif /* parser_h */
