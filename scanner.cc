#include <string>
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <fstream>

#include "scanner.h"


Input_Buffer::Input_Buffer(string filestring)
{
	//initialise class
	filename = filestring;
	
	//try opening the file
	inpf.open(filename);
	if (!inpf) {
		cout << "Error: cannot open file " << filename << " for reading" << endl;
		exit(1);
	}
	
	//initialise reading
	getline(inpf, CurrentLine);
	CurrentLine += '\n';
	eof = inpf.eof();
	currentCharacter = 0;
	// TODO check what happens with empty file
	lineNumber = 1;
}

Input_Buffer::Input_Buffer()
{
}


bool  Input_Buffer::getChar(char &ch)
{
	if(currentCharacter < CurrentLine.length())
	{
		ch = CurrentLine[currentCharacter];
		
		//	if the enf of file has been reached and we are at the last character of the
		// current(which is the last) line then return false indicating that the end of 
		// file has been reached
		if(eof && (currentCharacter == (CurrentLine.length()-1) ))
		{
			return true;	//return false to indicate the eof
		}
		else
		{
			currentCharacter++;
			return false;	//return true to indicate that the eof has not been reached
		}
		
	}
	
	else if(currentCharacter == CurrentLine.length())
	{
		eof = getNextLine();
		ch = CurrentLine[currentCharacter];
		currentCharacter++;
	}
	return false;
}

bool Input_Buffer::getNextLine()
{
	getline(inpf, CurrentLine);
	CurrentLine += '\n';
	currentCharacter = 0;
	lineNumber++;
	
	return inpf.eof();
}


int Input_Buffer::getCurrentLineNumber()
{
	return lineNumber;
}

string Input_Buffer::getCurrentLine()
{
	return CurrentLine;
}

bool Input_Buffer::moveToNextLine()
{
	eof = getNextLine();
	return eof;
}

char Input_Buffer::NextCharacter()
{
	int next_c = currentCharacter;
	if(next_c < CurrentLine.length())
	{
		return CurrentLine[next_c];
	}
	else
	{
		return '\0';
	}
}





// ==== Implementing scanner ====

//Constructor
Scanner::Scanner(names *nm, string file)
{
	nmz = nm;
	inBuffer = Input_Buffer(file);
}


//Returns number line maintained by the input buffer
int Scanner::GetCurrentLineNumber()
{
	return inBuffer.getCurrentLineNumber();
}

//Returns current line string maintained by the input buffer
string Scanner::GetCurrentLine()
{
	// removes last character which is a  new line character
	string str = inBuffer.getCurrentLine();
	str.pop_back(); 
	return str;
}

//Read the following characters and converts them to a string
bool Scanner::GetNextNumber(int &numb, char &ch)
{
	string s; 
	
	while(isdigit(current_char))
	{
		s += current_char;
		eof_flag = inBuffer.getChar(current_char);
	}
	
	try{
		numb = stoi(s);
	}
	catch(invalid_argument ia){
		numb = -1; //No number found
	}
	
	ch = inBuffer.NextCharacter();
	return eof_flag;
}

bool Scanner::GetNextChar(char &ch)
{
	eof_flag = inBuffer.getChar(current_char);
	
	//skip spaces
	if(isspace(current_char))
	{
		skipspaces();
	}
	
	//if a comment is found skip the line
	if((current_char == '/') && (inBuffer.NextCharacter() == '/'))
	{
		skipSingleLineComment();
		if(eof_flag == false)
		{
			eof_flag = inBuffer.getChar(current_char);
		}
	}
	
	if((current_char == '/') && (inBuffer.NextCharacter() == '*'))
	{
		skipMultiLineComment();
		if(eof_flag == false)
		{
			eof_flag = inBuffer.getChar(current_char);
		}
	}
	
	ch = current_char;
	return eof_flag;
}


bool Scanner::GetNextString(string &str, char &ch)
{
	string s;
	s = "";
	if(current_char == '\0')
	{
		eof_flag = inBuffer.getChar(current_char);
	}

	
	while(eof_flag == false)
	{

		if(isspace(current_char))
		{

			skipspaces();
		}
		//if a comment is found skip the line
		else if((current_char == '/') && (inBuffer.NextCharacter() == '/'))
		{
			skipSingleLineComment();
			if(eof_flag == false)
			{
				eof_flag = inBuffer.getChar(current_char);
			}

		}
		else if((current_char == '/') && (inBuffer.NextCharacter() == '*'))
		{
			skipMultiLineComment();
			if(eof_flag == false)
			{
				eof_flag = inBuffer.getChar(current_char);
			}

		}
		else
		{
			break;
		}		
	}
	
	if(isalpha(current_char))//first character must be a
	{
		//numbers and underscores allowed in the identifier
		while(isalnum(current_char) || current_char == '_')
		{
			s += current_char;
			eof_flag = inBuffer.getChar(current_char);

			if(eof_flag)
			{
				break;
			}
		
			//if a comment is found skip the line
			if((current_char == '/') && (inBuffer.NextCharacter() == '/'))
			{
				skipSingleLineComment();
				if(eof_flag == false)
				{
					eof_flag = inBuffer.getChar(current_char);
					break;
				}
			}
	
			if((current_char == '/') && (inBuffer.NextCharacter() == '*'))
			{
				skipMultiLineComment();
				if(eof_flag == false)
				{
					eof_flag = inBuffer.getChar(current_char);
					break;
				}
			}
		}
	}
	str = s;
	ch = inBuffer.NextCharacter();
	return eof_flag;
}



void Scanner::skipspaces()
{
	while(isspace(current_char) && (eof_flag == false))
	{
		eof_flag = inBuffer.getChar(current_char);
	}
}

void Scanner::skipSingleLineComment()
{
	eof_flag = inBuffer.moveToNextLine(); 
}

void Scanner::skipMultiLineComment()
{	
	while(eof_flag == false)
	{
		
		if((current_char == '*') && (inBuffer.NextCharacter() == '/'))
		{
			eof_flag = inBuffer.getChar(current_char);
			break;
		}
		eof_flag = inBuffer.getChar(current_char);
	}
}

bool Scanner::GetCurrentSymbol(char &ch)
{
	ch = current_char;
	return eof_flag;
}


// main used for debugging
/*
int main()
{
	bool eof = false;
	char ch;
	string str;
	int numb;
	
	names *nmz = new names();
	string filename = "test.txt";
	Scanner scan = Scanner(nmz,filename);
	
	
	while(eof == false)
	{
		eof = scan.GetNextSymbol(ch);
		if(isalpha(ch))
		{
			eof = scan.GetNextString(str, ch);
			if(str.length() != 0)
			{
				cout << str << endl;
			}
		}	
	}
	
	/*
	while(eof == false)
	{
		eof = scan.GetNextSymbol(ch);
		if(isdigit(ch))
		{
			eof=scan.GetNextNumber(numb,ch);
			cout << "Number = " << numb << endl;
		}
	}
	
	
	
	return 0;
}

*/
