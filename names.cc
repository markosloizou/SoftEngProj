#include "names.h"



/* Name storage and retrieval routines */

names::names()  /* the constructor */
{
  /* over to you */
  Table table;
}

name names::lookup (namestring str)
{
  /* over to you */
  return table.lookup(str);
}

name names::cvtname (namestring str)
{
  /* over to you */
  return table.findNamestring(str);
}

void names::writename (name id)
{
  /* over to you */
  cout << table[(int) id];
}

string names::getName(name id)
{
	return table[(int) id];
}
int names::namelength (name id)
{
  /* over to you */
  string str = table[(int) id];
  return str.length();
}

void names::printEntries()
{
	table.print_entries();
}



namestring Table::operator[](int i)
{
	try{
		return Names[i];
	}
	catch(const out_of_range oor)
	{
		//Perhaps throw exception instead of handling
		namestring badnm = "-NonExistingName-";
		return badnm;
	}
}

name Table::lookup(namestring str)
{
	int i =0;
	for(i = 0; i < Names.size(); i++)
	{
		if(Names[i] == str)
		{
			return (name) i;
		}
	}
	Names.push_back(str);
	
	return (name)(Names.size()-1);
	
}

//Prints all entries		Used for Debugging
void Table::print_entries()
{
	cout << setw(10) << "Index" << setw(10) << "Names" << endl;
	for(int i = 0; i < Names.size(); i++)
	{
		cout << setw(10) << i << setw(10) << Names[i] << endl;
	}
}

name Table::findNamestring(namestring str)
{
	int i;
	for(i = 0; i < Names.size(); i++)
	{
		if(Names[i] == str)
		{
			return (name) i;
		}
	}
	
	return (name)blankname;
}


//Main used for testing and debugging
/*
int main()
{
	names n;
	vector<namestring> v = {"abc", "name", "smth"};
	for(int i = 0; i < v.size(); i++)
	{
		n.lookup(v[i]);
	}
	n.printEntries();
	
	cout << (int)n.cvtname((namestring)"name");
	cout << endl;
	cout << (int)n.cvtname((namestring)"abs");
	cout << endl;
	cout << "Writing 0 = ";
	 n.writename((name)0); 
	 cout << endl;
	cout << "Length of 0th el = " << n.namelength((name)0) << endl;
	
	
}
*/
