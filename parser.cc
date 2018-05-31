#include <iostream>
#include <iomanip>
#include "parser.h"


using namespace std;

namespace Color { 				//Colour namespace for error printing
    enum Code {
    	FG_BLACK		= 30,		//Foreground colors
        FG_RED      		= 31,
        FG_GREEN    		= 32,
        FG_YELLOW 		= 33,
        FG_BLUE     		= 34,
        FG_MAGENTA		= 35,
        FG_CYAN 		= 36,
        FG_WHITE		= 37,
        FG_DEFAULT		= 39,
        BG_BLACK		= 40,		//Background colors
        BG_RED      		= 41,
        BG_GREEN    		= 42,
        BG_YELLOW 		= 43,
        BG_BLUE     		= 44,
        BG_MAGENTA		= 45,
        BG_CYAN 		= 46,
        BG_WHITE		= 47,
        BG_DEFAULT		= 49,
        RESET			= 0,		//Resets to original state
        BOLD_AND_BRIGHT		= 1,		//Combination of bold and bright letters
        UNDERLINE		= 4,
        INVERSE			= 7,		//Swaps FG and BG colors
        BOLD_AND_BRIGHT_OFF 	= 21,
        UNDERLINE_OFF 		= 24,
        INVERSE_OFF 		= 27
    };
    class Modifier {
        Code code;
    public:
        Modifier(Code pCode) : code(pCode) {}
        friend std::ostream&
        operator<<(std::ostream& os, const Modifier& mod) {
            return os << "\033[" << mod.code << "m";
        }
    };
}

bool parser::readin (void)	//Master function that reads in circuit definition file. Returns True if the file is valid according to EBNF definition
{
  bool eof = false; 	//End Of File - Becomes true when end of file is reached
  char cur_char;	//Stores current character
  string str;		//Stores current string
  int number;		//Stores current number
  
  cout << endl << "Loading Device Definition File..." << endl << endl;
  
  eof = smz->GetNextChar(cur_char);							//Get First Character
  
  if(eof == true)
  {
  	error_report(Empty_file, smz->GetCurrentLineNumber(), smz->GetCurrentLine()," ");  //The file contains nothing
  	nerrors++;	//Aggregate the number of errors
  	return false;
  }
  
  if(isalpha(cur_char)) 
  {
	  eof = smz->GetNextString(str, cur_char); 					//Get First String
	  
	  if(str == "DEVICES")								//All definition files must start with declaration for Phase I
		{
			current_phase = devi;						//We are in Phase I
			if (debugging) cout << "Phase I: Define Devices" << endl << endl;
		}
	  else 
		{
			error_report(DEVICES_not_present, smz->GetCurrentLineNumber(), smz->GetCurrentLine()," "); 		//Expected DEVICES at start of file
			proceed(cur_char,eof); 								 			//Proceed to next semicolumn
			nerrors++; 												//Accumulate number of errors
		}	
  }
  
  while(eof == false)
  {

  	switch(current_phase)
  	{
  		case devi:						//Phase I
  			defineDevice(cur_char);				//Master function for Phase I
  			if(cur_char == ';')				
  			{
  				eof = smz -> GetNextChar(cur_char);	//Move to next device definition
  			} 
  			break;
  			
  		case conn:						//Phase II
  			defineConnections(cur_char);				//Master function for Phase II
  			if(cur_char == ';')				
  			{
  				eof = smz -> GetNextChar(cur_char);	//Move to next connection definition
  			} 
  			break;
  			
		case mon:						//Phase III
			createMonitor(cur_char);			//Master function for Phase III
			if(cur_char == ';')				
  			{
  				eof = smz -> GetNextChar(cur_char);	//Move to next monitor definition
  			} 
			break;
			
		case finito:
			eof = true;					//Ignore anything written after END has been declared
			end_of_file(nerrors);				//Phase IV prelude		
			break;
		
		default:
			error_report(Internal_error, smz->GetCurrentLineNumber(), smz->GetCurrentLine()," "); 		//We shouldn't be here
			proceed(cur_char,eof); 								 	
			nerrors++; 
			break;
	}
  }
  if(eof == true)
  {
  	if(current_phase != finito)
  	{
  		end_of_file(nerrors);					//Phase IV prelude in case of abrupt file end
  		nerrors++;
  		return false;
  	}
  }
  if(nerrors > 0) return false;						//There are still some errors in the definition file that must be fixed
  else return true;							//Move to Phase IV (over to GUI)
}

bool parser::defineDevice(char &ch)					//Function to read in device refinitions. Returns true if all ok
{
	string str;
	eof = false;
	while(eof == false)
	{

		if(isalpha(ch))
		{
			
			eof = smz->GetNextString(str, ch);		//Obtains the string str that starts with the character ch that we are currently on
			
			//Check for keywords that will advance program to next Phase
			if(str == "CONNECTIONS")
			{				
				current_phase = conn;
				if (debugging) cout << endl << "Phase II: Define Connections" << endl << endl;				//Move to Phase II
				break;
			}
			else if(str == "MONITOR")											
			{
				if(ConnectionsRequired() != 0)
				{
					error_report(CONNECTIONS_not_present, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); //Need CONNECTIONS before MONITOR points defined
					nerrors++;
				}
				current_phase = mon;
				break;
			}
			
			else if(str == "DEVICES")
			{
				error_report(Nonexistent_device_type, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str);		//We don't need this twice
				nerrors++;
				break;
			}
			else if(str == "END")
			{	
				error_report(No_connections, smz->GetCurrentLineNumber(), smz->GetCurrentLine()," ");
				error_report(No_monitor_points, smz->GetCurrentLineNumber(), smz->GetCurrentLine()," ");
				current_phase = finito;
				break;
			}
			
			devicekind kind = isDeviceType((namestring)str);								//Detect what device we want to define
			if(kind != baddevice)
			{
				readDevice(ch, kind);											//Define the device in other function
			}
			else
			{
				error_report(Nonexistent_device_type, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str);		//Device type is illegal
				proceed(ch,eof); 								 		
				nerrors++; 												
			}
			
		}
		else if(ch == ';') break;
		else if(ch == ',') continue;
		else
		{
			error_report(Nonexistent_device_type, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 		//All devices start with a letter	
			proceed(ch,eof);
			nerrors++;
		}
	}

	if(eof == true)
	{
		error_report(Abrupt_end, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str);
		current_phase = finito;
		return false;
	}
}

devicekind parser::isDeviceType(namestring str) 	//Translates Device Defintion to internal representation
{
	if(str == "SWITCH") 		return aswitch;
	if(str == "CLOCK")		return aclock;
	if(str == "AND")		return andgate;
	if(str == "NAND")		return nandgate;
	if(str == "OR")			return orgate;
	if(str == "NOR")		return norgate;
	if(str == "XOR")		return xorgate;
	if(str == "DTYPE")		return dtype;
	else   				return baddevice;
}

string parser::isDeviceType(int index)			//Translates internal representation of device to string for display
{
	if(index == 0)			return "SWITCH";
	if(index == 1)			return "CLOCK";
	if(index == 2)			return "AND";
	if(index == 3)			return "NAND";
	if(index == 4)			return "OR";
	if(index == 5)			return "NOR";
	if(index == 6)			return "XOR";
	if(index == 7)			return "DTYPE";
      /*if(index == 8)			return "DATA";		//These are just for reference
	if(index == 9)			return "CLK";
	if(index == 10)			return "SET";
	if(index == 11)			return "CLEAR";
	if(index == 12)			return "Q";
	if(index == 13)			return "QBAR";*/
	else				return "ERROR";		//We should not get this
}

bool parser::readDevice(char& ch,devicekind kind)		//Function to sort devices according to type then call correct function to create device
{
	while(eof == false)
	{
		if(kind == aswitch)
		{
				readSwitch(ch);				//Define a switch in other function
			if(ch == ';')   				//All switches have been defined
			{	
				eof = smz-> GetNextChar(ch); 		//Get next non-space character and store it in ch
				break;
			}
			else if(ch == ',') 				//More switches to be defined
			{
				eof = smz->GetNextChar(ch);
				continue;
			}
			else if(isalnum(ch))
			{
				error_report(semicolumn, smz->GetCurrentLineNumber()-1, smz->GetPreviousLine(), string(1, '\n'));
				proceed(ch,eof);
				nerrors++;
				break;
			}
			else
			{
				error_report(Invalid_character, smz->GetCurrentLineNumber()-1, smz->GetCurrentLine(), string(1, ch)); 	
				proceed(ch,eof);
				nerrors++;
				break;
			}
		}
		else if(kind  == xorgate || kind == dtype)
		{
			readFixedDevice(ch, kind); 	//Define devices with fixed inputs and outputs in other function
			if(ch == ';')			//All XORs or Dtypes defined
			{	
				eof = smz-> GetNextChar(ch);
				break;
			}
			else if(ch == ',')		//More to be defined
			{
				eof = smz->GetNextChar(ch);
				continue;
			}
			else if(isalnum(ch))
			{
				error_report(semicolumn, smz->GetCurrentLineNumber()-1, smz->GetPreviousLine(), string(1, '\n')); 	
				proceed(ch,eof);
				nerrors++;
				break;
			}
			else
			{
				error_report(Invalid_character, smz->GetCurrentLineNumber()-1, smz->GetCurrentLine(), string(1, ch)); 	
				proceed(ch,eof);
				nerrors++;
				break;
			}
		}
		else
		{
			readVariableDevice(ch, kind);	//Define Gates or clocks in other function
			if(ch == ';')			//All Gates or clocks defined
			{	
				eof = smz-> GetNextChar(ch);
				break;
			}
			else if(ch == ',')		//More to be defined
			{
				eof = smz->GetNextChar(ch);
				continue;
			}
			else if(isalnum(ch))
			{
				error_report(semicolumn, smz->GetCurrentLineNumber()-1, smz->GetPreviousLine(), string(1, '\n')); 	
				proceed(ch,eof);
				nerrors++;
				break;
			}
			else
			{
				error_report(Invalid_character, smz->GetCurrentLineNumber()-1, smz->GetCurrentLine(), string(1, ch)); 	
				proceed(ch,eof);
				nerrors++;
				break;
			}
		}
	}
	
	if(eof == true)
	{
		current_phase = finito;
		error_report(Abrupt_end, smz->GetCurrentLineNumber()-1, smz->GetCurrentLine(), string(1, ch)); //End of file reached unexpectedly
		return false;
	}
}

bool parser::readSwitch(char &ch)			//Function that defines Switches
{
	string str;
	
	if(isalpha(ch))
	{
		eof = smz->GetNextString(str, ch);
		
		if(str.length() > 8) error_report(Long_identifier, smz->GetCurrentLineNumber()-1, smz->GetCurrentLine(), str); //Issue warning if device name is too long
		
		if(ch == '=')
		{
			eof = smz -> GetNextChar(ch);
			
			if(ch == '0' || ch == '1')						//Check that switch structure is obeyed
			{
				name id;
		
				if(nmz->cvtname(str) == blankname)
				{
					id = nmz->lookup((namestring) str);
				}
				else
				{
					if(str == "CLOCK" || str == "AND" || str == "NAND" || str == "OR" || str == "NOR" || str == "XOR" || str == "DTYPE" || str == "DATA" || str == "CLK" || str == "SET" || str == "CLEAR" || str == "Q" || str == "QBAR" || str == "DEVICES" || str == "CONNECTIONS" || str == "MONITOR" || str == "END")
					{
						error_report(Reserved_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 	//Can't name a device like this
						proceed(ch,eof);										//since this can cause problems later
						nerrors++;
						return false;	
					}
					else
					{
						error_report(Duplicate_device, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 	//Can't name two devices the same thing
						proceed(ch,eof);
						nerrors++;
						return false;
					}
				}
				
				int setting = 0;
				if(ch == '1') setting = 1;
				bool ok;
				
				dmz->makedevice(aswitch, id, setting, ok);			//Create switch internally
			  	
			  	if(!ok)
			  	{
			  		error_report(Internal_error, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 	//We should not be here
					proceed(ch,eof);
					nerrors++;
			  	} 
			  	else
			  	{
			  		eof = smz->GetNextChar(ch);
			  		
			  		dev d;
			  		d.id = id;
			  		d.Name = str;
			  		d.kind = aswitch;
			  		d.input_number = 0;
			  		d.initState = setting;
			  		d.isMonitored = false;
			  		devList.push_back(d);		//Store devices internally in custom data structure for later use (determine if there are unused devices etc)
			  		
			  		if(setting == 0)
			  		{
			  			if (debugging) cout << "Created OFF SWITCH with name " << str << endl;	
			  		}
			  		else
			  		{
			  			if (debugging) cout << "Created ON SWITCH with name " << str << endl;
			  		}
			  		return true;
				}
			}
			else
			{
				error_report(Invalid_switch_position, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch));  	//Switch must be either ON or OFF
				proceed(ch,eof);
				nerrors++;
				return false;
			}
		}
		else
		{
			error_report(Invalid_switch_definition, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 		//Switch definition must follow EBNF structure
			proceed(ch,eof);
			nerrors++;
			return false;
		}
	}
	else
	{
		error_report(Illegal_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 				//Name must start with a letter
		proceed(ch,eof);
		nerrors++;
		return false;
	}
}

bool parser::readFixedDevice(char &ch, devicekind kind)			//Function that defines XOR and Dtype
{
	string str;
	
	if(isalpha(ch))
	{
		eof = smz -> GetNextString(str,ch);
		
		if(str.length() > 8) error_report(Long_identifier, smz->GetCurrentLineNumber()-1, smz->GetCurrentLine(), str); /*Issue warning if device name is too long
																 Problem only in that it can cause the 	
																 device name to leak outside the designated	
																 area in the GUI, so don't issue an error */
		
		name id;
		
		if(nmz->cvtname(str) == blankname)
		{
			id = nmz->lookup((namestring) str);
		}
		else
		{
			if(str == "CLOCK" || str == "AND" || str == "NAND" || str == "OR" || str == "NOR" || str == "XOR" || str == "DTYPE" || str == "DATA" || str == "CLK" || str == "SET" || str == "CLEAR" || str == "Q" || str == "QBAR" || str == "DEVICES" || str == "CONNECTIONS" || str == "MONITOR" || str == "END")
			{
				error_report(Reserved_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
				proceed(ch,eof);
				nerrors++;
				return false;	
			}
			else
			{
				error_report(Duplicate_device, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
				proceed(ch,eof);
				nerrors++;
				return false;
			}
		}

		int setting;	//Create dummy variables to pass to devices.cc (they are ignored for this devices)
		bool ok;
		
		if(ch == '[' && kind == xorgate)		//Since XOR gates only have 2 inputs, allow them to be defined as XOR X1[2]; as well
		{
			eof = smz->GetNextChar(ch);
			
			if(ch == '2')
			{
				eof = smz->GetNextChar(ch);
				
				if(ch == ']')
				{
					eof = smz->GetNextChar(ch);
				}
				else{
					error_report(Missing_bracket, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
					proceed(ch,eof);
					nerrors++;
					return false;
				}
			}
			else
			{
				error_report(XOR_inputs, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
				proceed(ch,eof);
				nerrors++;
				return false;
			}
		}
		
		dmz->makedevice(kind,id,setting,ok);				//if all is well create device internally
		
		if(!ok)
		{
			error_report(Internal_error, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
			proceed(ch,eof);
			nerrors++;
			return false;
		}
		else
		{
			dev d;
			d.id = id;
			d.kind = kind;
			d.Name = str;
			d.initState = -1;
			d.isMonitored = false;
			if(kind == dtype)
			{
				d.input_number = 4;
			}
			else
			{
				d.input_number = 2;
			}
			
			devList.push_back(d);				//Store devices internally in custom data structure for later use (determine if there are unused devices)
			
			if (debugging) cout << "Created " << isDeviceType(kind) << " with name " << str << endl; //Device creation report
			
			return true;
		}
	}
	else
	{
		error_report(Illegal_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
		proceed(ch,eof);
		nerrors++;
		return false;
	}
}

bool parser::readVariableDevice(char& ch, devicekind kind)			//Function that defines logic gates and clocks
{
	string str;
	int number;
	name id;
	bool ok; 								//Checks that device has been successfully created internally in devices.cc

	if(isalpha(ch))								//if next character is alphabetic then we have a valid identifier
	{

		eof = smz -> GetNextString(str,ch);
		
		if(str.length() > 8) error_report(Long_identifier, smz->GetCurrentLineNumber()-1, smz->GetCurrentLine(), str); //Issue warning if device name is too long
		
		if(ch != '[') 							//Number of inputs must be within brackets
		{
			if(ch == '(')
			{
				error_report(Wrong_bracket_type, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 	//Only square brackets allowed
				proceed(ch,eof);
				nerrors++;
				return false;
			}
			else if(ch == '{')
			{
				error_report(Wrong_bracket_type, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch));
				proceed(ch,eof);
				nerrors++;
				return false;
			}
			else
			{
				error_report(Missing_bracket, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 	
				proceed(ch,eof);
				nerrors++;
				return false;
			}
		}

		eof = smz -> GetNextChar(ch);
		
		if(!isdigit(ch))						//Inputs must be a positive integer
		{
			error_report(Illegal_gate_inputs, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch));
			proceed(ch,eof);
			nerrors++;
			return false;
		}
		
		eof = smz -> GetNextNumber(number,ch);				//Gets the current number in the file
		
		if(ch != ']')							//Number of inputs must be within brackets
		{
			if(ch == ')')
			{
				error_report(Wrong_bracket_type, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 	
				proceed(ch,eof);
				nerrors++;
				return false;
			}
			if(ch == '}')
			{
				error_report(Wrong_bracket_type, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 	
				proceed(ch,eof);
				nerrors++;
				return false;
			}
			else
			{
				error_report(Missing_bracket, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 	
				proceed(ch,eof);
				nerrors++;
				return false;
			}
		}
		
		if(number == 0 || number < 0)					//Can't have any device with zero inputs
		{
				error_report(Invalid_device_input, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), to_string(number)); 
				proceed(ch,eof);
				nerrors++;
				return false;	
		}
		
		if(kind != aclock)						//Device is a logic gate
		{
			if(number < 1 || number > 16)				//Number of inputs must be between these two numbers
			{
				error_report(Invalid_gate_inputs, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), to_string(number)); 
				proceed(ch,eof);
				nerrors++;
				return false;
			}
		}
	 
		
		if(nmz->cvtname(str) == blankname)				//Check if device identifier already exists
		{
			id = nmz->lookup((namestring) str);			//if not get its internal identifier from names.cc
		}
		else
		{
			if(str == "CLOCK" || str == "AND" || str == "NAND" || str == "OR" || str == "NOR" || str == "XOR" || str == "DTYPE" || str == "DATA" || str == "CLK" || str == "SET" || str == "CLEAR" || str == "Q" || str == "QBAR" || str == "DEVICES" || str == "CONNECTIONS" || str == "MONITOR" || str == "END")
			{
				error_report(Reserved_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
				proceed(ch,eof);
				nerrors++;
				return false;	
			}
			else
			{
				error_report(Duplicate_device, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
				proceed(ch,eof);
				nerrors++;
				return false;
			}
		}

		dmz -> makedevice(kind,id,number,ok);				//if all is well create device internally
		
		if(!ok)
		{
			error_report(Internal_error, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), " "); 
			proceed(ch,eof);
			nerrors++;
			return false;
		}
		else
		{
			dev d;							//Store device internally
			d.id = id;
			d.kind = kind;
			d.Name = str;
			d.initState = -1;
			d.isMonitored = false;
			d.input_number = number;
			if(kind == aclock) d.input_number = 0;
			devList.push_back(d);
			
			eof = smz->GetNextChar(ch);
			
			if(kind != aclock)
			{
				if (debugging) cout << "Created " << isDeviceType(kind) << " with " << number << " inputs and name " << str << endl; //Device creation report
			}
			else
			{
				if (debugging) cout << "Created CLOCK with frequency " << number << endl;
			}
			return true;
		}		
		
	}
	else
	{
		error_report(Illegal_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
		proceed(ch,eof);
		nerrors++;
		return false;
	}
}

bool parser::defineConnections(char & ch)		//Function to define connections
{
	string str; 
	devicekind kind;
	name id;
	int number;

	
	name outDevID;					//Internal Variable for connection declaration
	name outDevPinID;
	name outDevKind;
	string outDevName;

	name inDevID;
	name inDevPinID;				
	string inDevName;
	
	bool ok;
	
	if(eof == true)
	{
		error_report(Abrupt_end, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
		current_phase = finito;
		return false;
	}
	
	if(isalpha(ch))
	{
		eof = smz -> GetNextString(str,ch);
		
		if(str == "MONITOR")														
		{
			current_phase = mon;
			if (debugging) cout << endl << "Phase III: Define Monitor Points" << endl << endl;		//Move to Phase III
			return true;														
		}
		
		if(str == "END")
		{	
			if(nconnections == 0 && ConnectionsRequired() != 0) 						//Declared end before all necessary connections have been established												
			{
				error_report(No_connections, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str);
				current_phase = finito;									//Move to Phase IV
				return false;								
			}
			else
			{
				error_report(No_monitor_points, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str);
				current_phase = finito;									//Move to Phase IV
				return true;	//Monitor points can be defined in GUI so no need for these here
			}
		}
		
		if(str == "CONNECTIONS")
		{
			error_report(Illegal_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
			proceed(ch,eof);
			nerrors++;
			return false;
		}
		if(str == "DEVICES")
		{
			error_report(Illegal_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
			proceed(ch,eof);
			nerrors++;
			return false;
		}
		
		id = nmz->cvtname(str);

		if(id == blankname)
		{
			error_report(Undefined_device, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
			proceed(ch,eof);
			nerrors++;
			return false;
		}
		
		outDevID = id;
		outDevName = str;
		
		kind = getKind(str);	//Find the device type for the output device
		
		if(kind == dtype)	//Dtype is only device with two outputs
		{
			if(ch != '.')
			{
				error_report(D_output_illegal, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
				proceed(ch,eof);
				nerrors++;
				return false;
			}
			
			eof = smz -> GetNextChar(ch);
			
			if(ch!='Q')																
			{
				error_report(D_output_illegal, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
				proceed(ch,eof);
				nerrors++;
				return false;
			}
			
			eof = smz->GetNextString(str,ch);
			
			if(str == "Q" || str == "QBAR")
			{
				outDevName += "." + str;
				outDevPinID = nmz->cvtname(str);
				
				if(outDevPinID == blankname)
				{
					error_report(Nonexistent_pin, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
					proceed(ch,eof);
					nerrors++;
					return false;
				}
			}
			else
			{
				error_report(D_output_illegal, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
				proceed(ch,eof);
				nerrors++;
				return false;
			}			
		}
		else
		{
			outDevPinID = -1; //THIS SHOULD HAVE BEEN MORE OBVIOUS IN THE DOCUMENTATION...
		}
		
		if(!isalpha(ch) && (kind == aclock || kind == aswitch))
		{
			error_report(Single_output_device, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
			proceed(ch,eof);
			nerrors++;
			return false;
		}
		else if(!isalpha(ch))
		{
			error_report(Input_to_Input, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
			proceed(ch,eof);
			nerrors++;
			return false;
			
		}
		
		eof = smz -> GetNextString(str, ch);
		
		if(str != "TO")
		{
			error_report(TO_missing, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
			proceed(ch,eof);
			nerrors++;
			return false;
		}
		
		if(!isalpha(ch))
		{
			error_report(Illegal_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
			proceed(ch,eof);
			nerrors++;
			return false;
		}
		
		eof = smz -> GetNextString(str,ch);
		
		inDevName = str;
		kind = getKind(str);		//Find the device type for the input device
		
		if (kind == aswitch || kind == aclock)
		{
			error_report(Device_has_no_input, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
			proceed(ch,eof);
			nerrors++;			
			return false;
		}
		
		if(ch != '.')
		{
			error_report(dot_missing, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
			proceed(ch,eof);
			nerrors++;
			return false;
		}
		
		id = nmz->cvtname(str);
		
		if(id == blankname)
		{
			error_report(Undefined_device, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
			proceed(ch,eof);
			nerrors++;
			return false;
		}
		
		inDevID = id;
		
		if(kind == dtype)		//Dtype has 4 inputs
		{
			
			eof = smz->GetNextChar(ch);
			
			if(!isalpha(ch))
			{
				error_report(Illegal_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
				proceed(ch,eof);
				nerrors++;
				return false;
			}

			eof = smz->GetNextString(str,ch);

			if(str != "DATA" && str != "CLK" && str != "SET" && str != "CLEAR")
			{
				error_report(Invalid_D_Input, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
				proceed(ch,eof);
				nerrors++;
				return false;
			}
			
			inDevPinID = nmz->cvtname(str);
			
			if(inDevPinID == blankname)
			{
				error_report(Nonexistent_pin, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
				proceed(ch,eof);
				nerrors++;
				return false;
			}

			inDevName += "." + str;
		}
		
		else
		{
			eof = smz->GetNextChar(ch);
			
			if(ch!='I')
			{
				error_report(no_I_before_number, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
				proceed(ch,eof);
				nerrors++;	
				return false;
			}
			else
			{
				eof = smz->GetNextChar(ch);
				
				if(!isdigit(ch))
				{
					error_report(no_number_after_I, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
					proceed(ch,eof);
					nerrors++;
					return false;
				}
				else
				{
					eof = smz->GetNextNumber(number,ch);
					
					inDevName += ".I" + to_string(number);
					string s = "I" + to_string(number);
					inDevPinID = nmz->cvtname(s);
					
					if(ch != ';')
					{
						s += ch;
						error_report(no_number_after_I, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), s); 
						proceed(ch,eof);
						nerrors++;
						return false;
					}
					
					if(inDevPinID == blankname)
					{
						error_report(Nonexistent_pin, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str); 
						proceed(ch,eof);
						nerrors++;
						return false;
					}
				}
			}
		}
		
		
	}
	else
	{
		error_report(Illegal_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string(1, ch)); 
		proceed(ch,eof);
		nerrors++;
		return false;
	}
	
	if(inputInUse(inDevID,inDevPinID))		//Only allow connections to be created if they don't already exist
	{
		error_report(Connection_overwrite, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), inDevName); 
		proceed(ch,eof);
		nerrors++;
		return false;
	}

	
	netz->makeconnection(inDevID,inDevPinID,outDevID,outDevPinID,ok);
	
	if(!ok) 					//Connection cannot be established if pin does not exist
	{
		error_report(Nonexistent_pin, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), inDevName); 
		proceed(ch,eof);
		nerrors++;
		return false;
	}
	else
	{
		if(!inputInUse(inDevID,inDevPinID))
		{
			cons Con;			//Store connection in internal data structure
			Con.inDevice = inDevID;
			Con.inDevicePin = inDevPinID;
			conList.push_back(Con); 
			nconnections++;
		}
		if (debugging) cout << "Made a connection between " << outDevName << " and " << inDevName << endl;
	}
		
}

bool parser::createMonitor(char &ch)			//Function to set up monitor points on any device outputs
{
	string str;
	string deviceName;
	int number;
	devicekind kind;
	name device;
	name deviceOut;
	bool ok;
	
	while(eof ==  false)
	{
		if(!isalpha(ch))
		{
			error_report(Illegal_identifier, smz->GetCurrentLineNumber(), smz->GetCurrentLine()," ");
			proceed(ch,eof); 								 	
			nerrors++; 
			return false;
		}
		
		eof = smz->GetNextString(str,ch);
		
		if(str.length() == 0)			//Check if we have reached the end of file without the presence of END
		{
			eof = true;
			current_phase = finito;
			return false;
		}
		
		if(str == "END")
		{
			if (debugging) cout << endl;
			current_phase = finito;
			return false;
		}
		
		kind = getKind(str);
		
		if(kind == baddevice)
		{
			if(eof == true) return false;			//We need to check that we have not reached the end of file
			error_report(Undefined_device, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), str);
			proceed(ch,eof); 								 	
			nerrors++; 
			return false;
			
		}
		else if( kind == dtype)					//Dtypes are the only devices with two outputs
		{
			device = nmz->cvtname(str);
			
			if(ch != '.')
			{
				error_report(dot_missing, smz->GetCurrentLineNumber(), smz->GetCurrentLine(),string{'.'});
				proceed(ch,eof); 								 	
				nerrors++; 
				return false;
			}
			
			eof = smz->GetNextChar(ch);
			
			if(ch != 'Q')
			{
				error_report(Invalid_D_Output, smz->GetCurrentLineNumber(), smz->GetCurrentLine(),string{ch});
				proceed(ch,eof); 								 	
				nerrors++;
				return false;
			}
			
			deviceName = str;
			
			eof = smz->GetNextString(str,ch);

			if(str == "DATA" || str == "CLK" || str == "SET" || str == "CLEAR")
			{
				error_report(Monitor_input, smz->GetCurrentLineNumber(), smz->GetCurrentLine(),string{ch});	//Only device outputs can be monitored
				proceed(ch,eof); 								 	
				nerrors++;
				return false;	
			}
			else if(str != "Q" && str != "QBAR")
			{
				error_report(Invalid_D_Output, smz->GetCurrentLineNumber(), smz->GetCurrentLine(),str);
				proceed(ch,eof); 								 	
				nerrors++;
				return false;
			}
			else
			{
				deviceOut = nmz->cvtname(str);
			}
			
			mmz->makemonitor(device,deviceOut,ok);
			
			if(!ok)
			{
				error_report(Internal_error, smz->GetCurrentLineNumber(), smz->GetCurrentLine()," ");
				proceed(ch,eof); 								 	
				nerrors++;
				return false;
			}
			else
			{	
				if (debugging) cout << "Monitoring " << deviceName << "." << str << endl;
				
				for(int i = 0; i < devList.size();i++)								//Store monitor point internally for use in GUI
				{
					if(devList[i].Name == deviceName)
					{
						devList[i].isMonitored = true;
						
						if(devList[i].kind == dtype && deviceOut == nmz->cvtname("QBAR"))
						{
							devList[i].bar = true;
						}
					}
					
				}
			}
		}	
		else
		{
			if(ch == '.')
			{
				error_report(Single_output_device, smz->GetCurrentLineNumber(), smz->GetCurrentLine(),string{ch});
				proceed(ch,eof); 								 	
				nerrors++;
				return false;
			}
			
			device = nmz->cvtname(str);
			deviceOut = -1; //Again, not very obvious...
			
			mmz->makemonitor(device,deviceOut,ok);
			
			if(!ok)
			{
				error_report(Internal_error, smz->GetCurrentLineNumber(), smz->GetCurrentLine()," ");
				proceed(ch,eof); 								 	
				nerrors++;
				return false;
			}
			else
			{
				if (debugging) cout << "Monitoring " << str << endl;
				
				for(int i = 0; i < devList.size();i++)								//Store monitor point internally for use in GUI
				{
					if(devList[i].Name == str)
					{
						devList[i].isMonitored = true;
						
						if(devList[i].kind == dtype && deviceOut == nmz->cvtname("QBAR"))
						{
							devList[i].bar = true;
						}
					}
					
				}
			}
				
		}
		
		if(ch == ';') break;
		else if(ch == ',')
		{
			eof = smz->GetNextChar(ch);	
		}
		else
		{
			error_report(Invalid_character, smz->GetCurrentLineNumber(), smz->GetCurrentLine(),string{ch});
			proceed(ch,eof); 								 	
			nerrors++;
			return false;
		}
		if(eof == true)
		{
			current_phase = finito;
			error_report(Abrupt_end, smz->GetCurrentLineNumber(), smz->GetCurrentLine()," ");
			proceed(ch,eof); 								 	
			nerrors++;
			break;
		}
	}
}




devicekind parser::getKind(string identifier)		//Function to retrive internal representation of device
{
	for(int i = 0; i < devList.size();i++)
	{
		if(devList[i].Name == identifier)
		{
			return devList[i].kind;
		}
	}
	
	return baddevice;
}

bool parser::end_of_file(int nerrors)			//Summarises any errors made in the definition file
{
	Color::Modifier red(Color::FG_RED);				//For Emphasis
	Color::Modifier bbr(Color::BOLD_AND_BRIGHT);
	Color::Modifier bbr_off(Color::BOLD_AND_BRIGHT_OFF);
	Color::Modifier def(Color::FG_DEFAULT);
	
	int n = ConnectionsRequired();					//Counts the total number of connections needed such that there are no unused devices
	
	if(n != nconnections)
	{
  		if(n < nconnections)
  		{
  			error_report(Unused_device, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), ""); 		//Issues a warning about unused devices being present
  			if(debugging) cout << "Devices declared require " <<  ConnectionsRequired() << " connections, but only " << nconnections << " were found" << endl; 
  			cout << endl;
  		}
  		else
  		{
  			error_report(Reused_connection, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), ""); 	//Issues a warning about extra connections being present
  			if(debugging) cout << "Devices declared require " <<  ConnectionsRequired() << " connections, but " << nconnections << " were found" << endl; 
  		}
	}
	
  	if(debugging) cout << endl << "End of Device Definition File" << endl << endl;
  	
  	if(nerrors > 0)
  	{
  	  	if(nerrors == 1)
  	  	{
  	  		cout << "Please" << red << bbr << " review 1 error " << bbr_off << def <<  "in the Device Definition File and Reload it!" << endl << endl;
  	  	}
  	  	else
  	  	{
  	  		cout << "Please" << red << bbr << " review " << nerrors << " errors " << bbr_off << def << "in the Device Definition File and Reload it!" << endl << endl;
  	  	}
  	}
	else if(nerrors == 0)
	{
		cout << "No errors found in Device Definition File!" << endl << endl << "Launching Graphical User Interface..." << endl << endl;
	}
	else
	{
		error_report(Internal_error, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), "");
	}
}

int parser::ConnectionsRequired()	//Counts the total number of connections required by adding up all the valid device inputs for the declared devices											
{
	int sum = 0;
	for(int i = 0; i < devList.size();i++)
	{
		sum += devList[i].input_number;
	}
	return sum;
}

bool parser::inputInUse(name device, name pin)		//Checks if an input pin is already in use
{
	for(int i =0; i<conList.size(); i++)
	{
		if(conList[i].inDevice == device && conList[i].inDevicePin == pin) return true;
	}
	
	return false;
}

parser::parser (network* network_mod, devices* devices_mod, monitor* monitor_mod, Scanner* scanner_mod, names* names_mod)
{
  netz = network_mod;  /* make internal copies of these class pointers */
  dmz = devices_mod;   /* so we can call functions from these classes  */
  mmz = monitor_mod;   /* eg. to call makeconnection from the network  */
  smz = scanner_mod;   /* class you say:                               */
                       /* netz->makeconnection (i1, i2, o1, o2, ok);   */

  /* any other initialisation you want to do? */
  
  nmz = names_mod;
  
  nmz -> lookup("DEVICES");			//Add phase headers to lookup table
  nmz -> lookup("CONNECTIONS");			//to prevent errors such as declaring
  nmz -> lookup("MONITOR");			//a switch named MONITOR which can 
  nmz -> lookup("END");				//otherwise cause problems
  
  nerrors = 0;					//Initialise these to track the number of connections and the number of errors in the definition file
  nconnections = 0;				
}

void parser::error_report(er error_type, int error_line, string current_line, string error_string) //Reports error on command prompt
{
	
	Color::Modifier red(Color::FG_RED);				//For Errors
	Color::Modifier green(Color::FG_GREEN);				//For Pointer
	Color::Modifier cyan(Color::FG_CYAN);				//For Warnings
	Color::Modifier yellow(Color::FG_YELLOW);			//For Internal Errors
	Color::Modifier bbr(Color::BOLD_AND_BRIGHT);
	Color::Modifier bbr_off(Color::BOLD_AND_BRIGHT_OFF);
	Color::Modifier def(Color::FG_DEFAULT);
	
	size_t err_pos = current_line.find(error_string);		/*   Count the number of spaces required for correct placement of ^
									     Additional spaces to accomodate "Line #: " in error reporting  */
 	if(err_pos == string::npos) err_pos = 0;
	string str = "Line " + to_string(error_line) + ": ";
	err_pos += str.size()+2;

	switch(error_type)
	{
		//Syntax Errors
		case DEVICES_not_present :
			cout << red << bbr << "Syntax Error 1:" << bbr_off << def << " Expected DEVICES at the beginning of definition file" << endl 	//Display error message
			<< "Line " << error_line << ": " << current_line << endl									//Display error line		
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;								//Place pointer underneath error	
			break;												
			
		case Nonexistent_device_type :
			cout << red << bbr << "Syntax Error 2:" << bbr_off << def << " Device type \"" << error_string << "\" not recognised" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Illegal_identifier :
			cout << red << bbr << "Syntax Error 3:" << bbr_off << def << " Illegal device name (names must start with a letter and contain no special characters)" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
		case Wrong_bracket_type :
			cout << red << bbr << "Syntax Error 4:"  << bbr_off << def << " Expected “[” or “]” before declaring number of inputs for this device" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Missing_bracket :
			cout << red << bbr << "Syntax Error 5:"  << bbr_off << def << " Expected “[” or “]” when declaring this device" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Illegal_gate_inputs :
			cout << red << bbr << "Syntax Error 6:"  << bbr_off << def << " Expected positive integer for number of inputs" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Invalid_switch_position :
			cout << red << bbr << "Syntax Error 7:" << bbr_off << def << " Expected “0” or “1” for switch position" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Invalid_switch_definition :
			cout << red << bbr << "Syntax Error 8:" << bbr_off << def << " Invalid switch definition (Expected “= 0” or “= 1”)" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case semicolumn :
			cout << red << bbr << "Syntax Error 9:" << bbr_off << def << " Expected “;” at end of expression" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case CONNECTIONS_not_present :
			cout << red << bbr << "Syntax Error 10:" << bbr_off << def << " Expected “CONNECTIONS” before connection definitions" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case D_output_illegal :
			cout << red << bbr << "Syntax Error 11:" << bbr_off << def << " Expected “.Q” or “.QBAR” at output device" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case TO_missing :
			cout << red << bbr << "Syntax Error 12:" << bbr_off << def << " Expected “TO” after output device" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case no_number_after_I :
			cout << red << bbr << "Syntax Error 13:" << bbr_off << def << " Invalid connection input. Need G1.I1 for example" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case no_I_before_number :
			cout << red << bbr << "Syntax Error 14:" << bbr_off << def << " Expected “I” after “.” for input device. Need G1.I1 for example" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Input_to_Input :
			cout << red << bbr << "Syntax Error 15:" << bbr_off << def << " Device input connected to device input" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Input_to_Output :
			cout << red << bbr << "Syntax Error 16:" << bbr_off << def << " Device input connected to device output" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Output_to_Output :
			cout << red << bbr << "Syntax Error 17:" << bbr_off << def << " Device output connected to device output" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case MONITOR_not_present :
			cout << red << bbr << "Syntax Error 18:" << bbr_off << def << " Expected “MONITOR” before monitor points are declared" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Invalid_character :
			cout << red << bbr << "Syntax Error 19:" << bbr_off << def << " Illegal character used" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
			
		case Abrupt_end :
			cout << red << bbr << "Syntax Error 21:" << bbr_off << def << " Definition File is incomplete! (There are missing sections)" << endl; 
			break;
			
		case Empty_file :
			cout << red << bbr << "Syntax Error 22:" << bbr_off << def << " Definition File is empty!" << endl; 
			break;
			
		//Semantic Errors	
		case Reserved_identifier :
			cout << red << bbr << "Semantic Error 1:" << bbr_off << def << " Reserved keyword used as device identifier" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Invalid_gate_inputs :
			cout << red << bbr << "Semantic Error 2:" << bbr_off << def << " Logic gates must have between 1 and 16 inputs" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case XOR_inputs :
			cout << red << bbr << "Semantic Error 3:" << bbr_off << def << " XOR gate must have two inputs" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Single_output_device :
			cout << red << bbr << "Semantic Error 4:" << bbr_off << def << " Device only has a single output (no need for “.” something)" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Invalid_gate_syntax :
			cout << red << bbr << "Semantic Error 5:" << bbr_off << def << " Invalid device definition. Use CLOCK CK[1] or AND G[2] or SWITCH SW" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Invalid_switch_syntax :
			cout << red << bbr << "Semantic Error 7:" << bbr_off << def << " Invalid device definition. Use CLOCK CK[1] or AND G[2] or SWITCH SW" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Device_has_no_input:
			cout << red << bbr << "Semantic Error 8:" << bbr_off << def << " Device has no legal input" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case D_input_only :
			cout << red << bbr << "Semantic Error 10:" << bbr_off << def << " Logic gate input must be of the form G1.I1" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case D_output_only :
			cout << red << bbr << "Semantic Error 11:" << bbr_off << def << " Logic gate only has one output (no “.Q” or “.QBAR” output)" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Undefined_device :
			cout << red << bbr << "Semantic Error 12:" << bbr_off << def << " Device \"" << error_string << "\" not defined" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Duplicate_device :
			cout << red << bbr << "Semantic Error 13:" << bbr_off << def << " Duplicate device definition \"" << error_string << "\""<< endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Muptiple_connections :
			cout << red << bbr << "Semantic Error 14:" << bbr_off << def << " Multiple outputs connected to the same input" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Unused_input :
			cout << red << bbr << "Semantic Error 15:" << bbr_off << def << " Device input is unused" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case dot_missing :
			cout << red << bbr << "Semantic Error 16:" << bbr_off << def << " Device input needs to be specified using “.”" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Invalid_D_Input :
			cout << red << bbr << "Semantic Error 17:" << bbr_off << def << " Valid D-type inputs include “DATA”, “CLK”, “SET” and “CLEAR”" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Invalid_D_Output :
			cout << red << bbr << "Semantic Error 18:" << bbr_off << def << " Valid D-type outputs include “Q” and “QBAR”" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Connection_overwrite :
			cout << red << bbr << "Semantic Error 19:" << bbr_off << def << " Connection to " << error_string << " already exists" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Invalid_device_input :
			cout << red << bbr << "Semantic Error 20:" << bbr_off << def << " Device inputs must be strictly positive" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case Nonexistent_pin :
			cout << red << bbr << "Semantic Error 21:" << bbr_off << def << " Pin \"" << error_string << "\" does not exist" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;

		case Monitor_input :
			cout << red << bbr << "Semantic Error 22:" << bbr_off << def << " Inputs to devices cannot be monitored" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
			
		//Warnings	
		case Unused_device :
			cout << cyan << bbr << "Warning:" << bbr_off << def << " Unused device " << endl;
			break;
			
		case Long_identifier :
			cout << cyan << bbr << "Warning:" << bbr_off << def << " Device name " << error_string << "\" is more than 8 characters long" << endl 
			<< "Line " << error_line << ": " << current_line << endl;
			break;
			
		case No_connections :
			cout << cyan << bbr << "Warning:" << bbr_off << def << " No connections specified in definition file" << endl
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			break;
			
		case No_monitor_points :
			cout << cyan << bbr << "Warning:" << bbr_off << def << " No monitor points specified" << endl 
			<< "Line " << error_line << ": " << current_line << endl;
			break;
			
		case Reused_connection :
			cout << cyan << bbr << "Warning:" << bbr_off << def << " There are too many connections specified for the devices specified. A connection was ignored" << endl;
			break;
			
			
		//Internal Error (we should never get here)
		case Internal_error :
			cout << yellow << bbr << "INTERNAL ERROR: ABORTING!" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl;
			exit(404);
			break;
			
		default:
			cout << "@@" << endl;
			break;
		}
}

void parser::proceed(char& cur_char, bool& eof) 	//Move to next semicolumn
{
	while (cur_char != ';' && eof == false)
	{
	 	eof = smz->GetNextChar(cur_char);
 	}
}

vector<dev> parser::getDevList(){			//Used in GUI
	return devList;
}

/*
// main function used for debugging
int main()
{
	string filename = "test.txt";			//Name of test file
	
	names* names_mod     = new names();
	network* network_mod = new network(names_mod);
	devices* devices_mod = new devices(names_mod, network_mod);
	monitor* monitor_mod = new monitor(names_mod, network_mod);
	Scanner* scanner_mod = new Scanner(names_mod,filename);
	parser* prs          = new parser (network_mod,devices_mod, monitor_mod, scanner_mod, names_mod);


	prs->readin();
	
	return 0;
}
*/
