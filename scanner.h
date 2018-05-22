#ifndef scanner_h
#define scanner_h

#include <string>
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <fstream>

#include "names.h"

using namespace std;


//   An internal software buffer between the input file and the program will be used so 
//  that the ability to get the current line number and complete lines will be available 
//  for error reporting



class Input_Buffer{
	private:
		// gets next line of the input file
		bool getNextLine();
	
		string filename;	// name of the file
		ifstream inpf;		//input file stream
	
		unsigned int lineNumber; //stores the current line number
		bool eof; //eof flag
		string CurrentLine;	//string Containing the current line
		unsigned int currentCharacter;//contains current character position in the string
	
	public:
		Input_Buffer(string filestring); //constructor
		Input_Buffer();
		bool getChar(char &ch);	//gets next character of the input file
		int getCurrentLineNumber();	//returns current line number
		string getCurrentLine(); //returns the current line if eof has not been reached
		bool moveToNextLine(); //Move to next line in case of a comment
		char NextCharacter();//USed to detect comments
};


class Scanner{
	
	private:
		Input_Buffer inBuffer;	//input buffer class for managing input
		names* nmz;	//pointer to nmz
		char current_char ='\0'; //current character
		bool eof_flag = false; 
		
		void skipspaces();	//skips spaces
		void skipSingleLineComment();//skips single line comment
		void skipMultiLineComment();//skips Multi line comment
	
	public:
		
		Scanner(names *nm, string file); //constructor
		
		//Fucntion used to get the next string of the file, next character is placed in 
		// ch. Returns false if eofile has been reached
		bool GetNextString(string &str, char &ch); 
		
		//Gets character and places it in ch. Returns false if eofile has been reached
		bool GetNextChar(char &ch);
		
		//Gets next number and writes it to numb, character following numb is placed in ch
		//Returns false if eofile has been reached
		bool GetNextNumber(int &numb, char&ch);
		
		//Returns the current character
		bool GetCurrentChar(char &ch);
		
		//Retunrs the Whole Current Line
		string GetCurrentLine();
		
		//Returns the current line number
		int GetCurrentLineNumber();
		
};
#endif
