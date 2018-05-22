#include <iostream>
#include <iomanip>
#include "parser.h"

using namespace std;

namespace Color { 				//Colour namespace for error printing
    enum Code {
    	FG_BLACK		= 30,	//Foreground colors
        FG_RED      	= 31,
        FG_GREEN    	= 32,
        FG_YELLOW 		= 33,
        FG_BLUE     	= 34,
        FG_MAGENTA		= 35,
        FG_CYAN 		= 36,
        FG_WHITE		= 37,
        FG_DEFAULT		= 39,
        BG_BLACK		= 40,	//Background colors
        BG_RED      	= 41,
        BG_GREEN    	= 42,
        BG_YELLOW 		= 43,
        BG_BLUE     	= 44,
        BG_MAGENTA		= 45,
        BG_CYAN 		= 46,
        BG_WHITE		= 47,
        BG_DEFAULT		= 49,
        RESET			= 0,	//Resets to original state
        BOLD_AND_BRIGHT	= 1,	//combination of bold and bright letters
        UNDERLINE		= 4,
        INVERSE			= 7,	//swaps FG and BG colors
        BOLD_AND_BRIGHT_OFF = 21,
        UNDERLINE_OFF 	= 24,
        INVERSE_OFF 	= 27
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

bool parser::readin (void)
{
  bool neof = true; //Not End Of File - Becomes false when end of file is reached
  char cur_char;
  string str;
  int number;
  block current_block;
  
  neof = smz->GetNextChar(cur_char);
  while(neof == false)
  {
	  if(isalpha(cur_char)) 
	  {
		  neof = smz->GetNextString(str, cur_char); 
		  cout << "str: " <<str << "  char: " << cur_char <<endl;//returns string that starts with current character and moves to the next non-space character
	  }
	  else if(isdigit(cur_char)) neof = smz->GetNextNumber(number, cur_char); //returns string that cointains the next number segment
	  else if(cur_char == '(') //Clock input
		{
		cout << "((((((" << endl;
		neof = smz->GetNextChar(cur_char); //Obtain next character
		if(isdigit(cur_char)) neof = smz->GetNextNumber(number, cur_char); //Obtain number
		else 
			{
			cout << "Nooo" << endl;
			error_report(input_no, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), string{'('}); //Expected Number
			proceed(cur_char,neof); //Proceed to next semicolumn
			}
		  
		neof = smz->GetNextChar(cur_char); //Get next character to see if bracket closes
		if(cur_char != ')')
			{
			error_report(brack_miss, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), "("); //Expected closing brackets
			proceed(cur_char,neof); //Proceed to next semicolumn
			}
		}
	  else if(cur_char == '[') //Gate input
		{
		neof = smz->GetNextChar(cur_char);
		if(isdigit(cur_char)) neof = smz->GetNextNumber(number, cur_char);
		else
			{
			error_report(input_no, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), "[");
			proceed(cur_char,neof);
			}
		
		neof = smz->GetNextChar(cur_char);
		if(cur_char != ']') 
			{
			error_report(brack_miss, smz->GetCurrentLineNumber(), smz->GetCurrentLine(), "[");
			proceed(cur_char,neof);
			}
		}
	  else if(cur_char == ';') //Complete circuit component defined. Build network
	  {
		/*  if(block == devices) //Define device
		  {
			  cout << 'Connections Reached' << endl;//Define Device
		  }
		  if(block == connections) //Define connections
		  {
			  cout << 'Connections Reached' << endl;//Define Connections
		  }
		  if(block == monitor) //Define monitor points
		  {
			  cout << 'Monitor Reached' << endl;//Define Monitor Points
		  }*/
		  neof = smz->GetNextChar(cur_char);
	  }
	  else neof = smz->GetNextChar(cur_char);
	  cout << cur_char << endl;
  }
}

parser::parser (network* network_mod, devices* devices_mod,
		monitor* monitor_mod, Scanner* scanner_mod)
{
  netz = network_mod;  /* make internal copies of these class pointers */
  dmz = devices_mod;   /* so we can call functions from these classes  */
  mmz = monitor_mod;   /* eg. to call makeconnection from the network  */
  smz = scanner_mod;   /* class you say:                               */
                       /* netz->makeconnection (i1, i2, o1, o2, ok);   */

  /* any other initialisation you want to do? */

}

devicekind isDevice(namestring str) //Translates Device Defintion to internal representation
{
	if(str == "SWITCH") 	return aswitch;
	if(str == "CLOCK")		return aclock;
	if(str == "AND")		return andgate;
	if(str == "NAND")		return nandgate;
	if(str == "OR")			return orgate;
	if(str == "NOR")		return norgate;
	if(str == "XOR")		return xorgate;
	if(str == "DTYPE")		return dtype;
	else   					return baddevice;
}

void parser::error_report(er error_type, int error_line, string current_line, string error_string) //Reports error on command prompt
{
	
	Color::Modifier red(Color::FG_RED);					//For Errors
	Color::Modifier green(Color::FG_GREEN);				//For Pointer
	Color::Modifier cyan(Color::FG_CYAN);				//For Warnings
	Color::Modifier def(Color::FG_DEFAULT);
	Color::Modifier bbr(Color::BOLD_AND_BRIGHT);
	Color::Modifier bbr_off(Color::BOLD_AND_BRIGHT_OFF);
	
	size_t err_pos = current_line.find(error_string);
	

	switch(error_type)
	{
		//Syntax Errors
		case no_DEV :
			cout << red << bbr << "Syntax Error 1: Expected DEVICES at the beginning of definition file (no ;)" << bbr_off << def << endl 	//Display error message
			<< "Line " << error_line << ": " << current_line << endl																			//Display error line
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;																	//Place pointer underneath error
			
		case dev :
			cout << red << bbr << "Syntax Error 2: Device type not recognised" << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case nam :
			cout << red << bbr << "Syntax Error 3: Illegal device name (names must start with a letter and contain no special characters)" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case brack :
			cout << red << bbr << "Syntax Error 4: Expected **BRACKET TYPE** before declaring number of inputs for this device" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case brack_miss :
			cout << red << bbr << "Syntax Error 5: Expected **BRACKET TYPE** when declaring this device" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case input_no :
			cout << red << bbr << "Syntax Error 6: Expected positive integer for number of inputs" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case sw_pos :
			cout << red << bbr << "Syntax Error 7: Expected 0 or 1 for switch position" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case sw_def :
			cout << red << bbr << "Syntax Error 8: Invalid switch definition (Expected “= 0” or “= 1”)'" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case semi :
			cout << red << bbr << "Syntax Error 9: Expected “;” at end of expression" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case no_CON :
			cout << red << bbr << "Syntax Error 10: Expected ”CONNECTIONS” before connection definitions (no “;”)" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case D_out :
			cout << red << bbr << "Syntax Error 11: Expected “.Q” or “.QBAR” at output device" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case TO :
			cout << red << bbr << "Syntax Error 12: Expected “TO” after output device" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case in_no :
			cout << red << bbr << "Syntax Error 13: Invalid connection input. Need G1.I1 for example" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case in_I :
			cout << red << bbr << "Syntax Error 14: Expected “I” after “.” for input device. Need G1.I1 for example" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case in_to_in :
			cout << red << bbr << "Syntax Error 15: Device input connected to device input" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case in_to_out :
			cout << red << bbr << "Syntax Error 16: Device input connected to device output" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case out_to_out :
			cout << red << bbr << "Syntax Error 17: Device output connected to device output" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case no_MON :
			cout << red << bbr << "Syntax Error 18: Expected “MONITOR” before monitor points are declared (no “;”)" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case ch :
			cout << red << bbr << "Syntax Error 19: Illegal character used" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		//Semantic Errors	
		case keyword :
			cout << red << bbr << "Semantic Error 1: Reserved keyword used as device identifier" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case inp_no :
			cout << red << bbr << "Semantic Error 2: Logic gates limited to 16 inputs" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case xor_in :
			cout << red << bbr << "Semantic Error 3: XOR gate must have two inputs" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case one_in :
			cout << red << bbr << "Semantic Error 4: Logic gates only have a single output (no need for “.”)" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case cl_synt :
			cout << red << bbr << "Semantic Error 5: Invalid device definition. Use CLOCK CK(1) or AND G[2] or SWITCH SW" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case log_synt :
			cout << red << bbr << "Semantic Error 6: Invalid device definition. Use CLOCK CK(1) or AND G[2] or SWITCH SW" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case sw_synt :
			cout << red << bbr << "Semantic Error 7: Invalid device definition. Use CLOCK CK(1) or AND G[2] or SWITCH SW" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case out_no_in :
			cout << red << bbr << "Semantic Error 8: Cannot connect logic output to clock" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case in_D :
			cout << red << bbr << "Semantic Error 10: Logic gate input must be of the form G1.I1" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case out_D :
			cout << red << bbr << "Semantic Error 11: Logic gate only has one output (no .Q or .QBAR output)" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case undef_dev :
			cout << red << bbr << "Semantic Error 12: Device **DEVICE NAME** not defined" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case same_name :
			cout << red << bbr << "Semantic Error 13: Duplicate device definition **DEVICE NAME**" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case mult_in :
			cout << red << bbr << "Semantic Error 14: Multiple outputs connected to the same input" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		case no_in :
			cout << red << bbr << "Semantic Error 15: Device input is unused" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl
			<<  setw(err_pos) << green << bbr << "^" << bbr_off << def << endl;
			
		//Warnings	
		case unused :
			cout << cyan << bbr << "Warning: Unused device **DEVICE NAME**" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl;
			
		case truncate :
			cout << cyan << bbr << "Warning: Device name **DEVICE NAME** truncated to 8 characters" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl;
			
		case no_monitor :
			cout << cyan << bbr << "Warning: No monitor points specified" << bbr_off << def << endl 
			<< "Line " << error_line << ": " << current_line << endl;
		}
}

void parser::proceed(char& cur_char, bool& neof) //Move to next semicolumn
{
	while (cur_char != ';' && neof == true)  neof = smz->GetNextChar(cur_char);
}

// main function used for debugging

int main()
{
	bool eof = false;
	char ch;
	string str;
	int numb;
	
	names *names_mod = new names();
	string filename = "test.txt";
	Scanner *scanner_mod = new Scanner(names_mod,filename);
	network* network_mod = new network(names_mod);
	devices* devices_mod = new devices(names_mod, network_mod);
	monitor* monitor_mod = new monitor(names_mod, network_mod);
	parser *prs = new parser (network_mod,devices_mod, monitor_mod, scanner_mod);
	
	prs->readin();
	
	
	
	return 0;
}
