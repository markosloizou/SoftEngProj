#ifndef names_h
#define names_h

#include <string>
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <iomanip> 

using namespace std;

const int maxnames  = 200;  /* max number of distinct names */
const int maxlength = 8;    /* max chars in a name string   */
const int blankname = -1;   /* special name                 */

typedef int name;
typedef string namestring;



/*
**	 Table is a class where the names are stored as strings and an internal representation 
**	of a name (integer) is returned for the program to use. 
**
**	 This was implemented for the preliminary excercises and thus used as an internal  
**	class of names.
*/

class Table{
	private:
		// Vector used to store all the used defined devices names
		vector<namestring> Names;
		
	public:
		//Overload operator to return the name of the device at position i
		//If the integer i is out of bounds then the string "-NonExistingName"
		//will be returned
		namestring operator[](int i);
		
		//Looks if str exists in the table, if it exists the device is added and a new
		//name is returned. If the name does not exist then it is added to the list 
		// containing all the namestrings and its name is returned
		name lookup(namestring str);
		
		//It looks for the namestring str in the stored list, if it does not exist
		// blankname (-1) is returned
		name findNamestring(namestring str);
		
		//Functions used for debugging. Prints all the entries of the table
		void print_entries();
};	



class names{

  /* put stuff that the class uses internally here */
private:

	Table table; 


public:
  name lookup (namestring str);
    /* Returns the internal representation of the name given in character  */
    /* form.  If the name is not already in the name table, it is          */
    /* automatically inserted.                                             */

  name cvtname (namestring str);
    /* Returns the internal representation of the name given in character  */
    /* form.  If the name is not in the name table then 'blankname' is     */
    /* returned.                                                           */

  void writename (name id);
    /* Prints out the given name on the console                            */

  int namelength (name id);
    /* Returns length ie number of characters in given name                */
    
  void printEntries();

  names (void);
    /* names initialises the name table.  This procedure is called at      */
    /* system initialisation before any of the above procedures/functions  */
    /* are used.                                                           */
};


#endif /* names_h */
