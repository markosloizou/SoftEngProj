#ifndef parser_h
#define parser_h

#include "names.h"
#include "scanner.h"
#include "network.h"
#include "devices.h"
#include "monitor.h"

typedef enum {devi, conn, mon} block; //Distinguish between the three input phases

//Possible Syntax and Semantic Errors
enum er { no_DEV = 1, dev = 2, nam = 3, brack = 4, brack_miss = 5,   			//Syntax Errors
	     input_no = 6, sw_pos = 7, sw_def = 8, semi = 9, no_CON = 10,
	     D_out = 11, TO = 12, in_no = 13, in_I = 14, in_to_in = 15,
	     in_to_out = 16, out_to_out = 17,  no_MON = 18, ch = 19,

	     keyword = 101, inp_no = 102, xor_in = 103, one_in = 104, cl_synt = 105,   	//Semantic Errors
	     log_synt = 106, sw_synt = 107, out_no_in = 108, in_D = 110,
	     out_D = 111, undef_dev = 112, same_name = 113, mult_in = 114, no_in = 115,
	     
	     unused = 201, truncate = 202, no_monitor = 203								//Warnings
	    };

using namespace std;


class parser {
  network* netz; // instantiations of various classes for parser to use.
  devices* dmz; 
  monitor* mmz;
  Scanner* smz;

  /* put other stuff that the class uses internally here */
  /* also declare internal functions                     */
 private:
  er error_type;
  int error_line;
  char cur_char;
  string current_line;
  string error_string;
  bool neof;
  
  void error_report(er error_type, int error_line, string current_line, string error_string); //Reports error on command prompt
  void proceed(char& cur_char, bool& neof); //Move to next semicolumn

 public:
  bool readin ();
    /* Reads the definition of the logic system and builds the             */
    /* corresponding internal representation via calls to the 'Network'    */
    /* module and the 'Devices' module.                                    */

  parser (network* network_mod, devices* devices_mod,
	  monitor* monitor_mod, Scanner* scanner_mod);
    /* the constructor takes pointers to various other classes as parameters */
};

#endif /* parser_h */
