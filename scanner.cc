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
	//initialise the state to the first line and first character
	currentCharacter = 0;
	lineNumber = 1;
}

/*
//default constructor required for compiling on some compilers
Input_Buffer::Input_Buffer()
{
}
*/

int Input_Buffer::getCharPosition()
{
	return currentCharacter;
}


//gets the next character of the file, the internal character position is advanced by one
bool  Input_Buffer::getChar(char &ch)
{
	if(currentCharacter < CurrentLine.length())
	{
		ch = CurrentLine[currentCharacter];
		
		//	if the eof of file has been reached and we are at the last character of the
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
	//if all the line has been read go to the next
	else if(currentCharacter == CurrentLine.length() && eof == false)
	{
		eof = getNextLine();
		ch = CurrentLine[currentCharacter];
		currentCharacter++;
		return false;
	}
	//eof reached
	else
	{	
		return true;
	}
	
}


//reads the next line from the file
bool Input_Buffer::getNextLine()
{
	PreviousLine = CurrentLine;
	getline(inpf, CurrentLine);
	CurrentLine += '\n';
	currentCharacter = 0;
	lineNumber++;
	
	return inpf.eof();
}

//returns the current character of the file
//returns '\0' if at the end of  line used for comment skipping
char Input_Buffer::getCurChar()
{
	int next_c = currentCharacter-1;
	if(next_c < CurrentLine.length())
	{
		return CurrentLine[next_c];
	}
	else
	{
		return '\0';
	}
}

//returns the current line number
int Input_Buffer::getCurrentLineNumber()
{
	return lineNumber;
}

//returns the current string
string Input_Buffer::getCurrentLine()
{
	return CurrentLine;
}

//used to skip single line comments, skips to the next line
bool Input_Buffer::moveToNextLine(char &ch)
{
	eof = getNextLine();
	return eof;
}

//returns the next character. again used for comment detecting and skipping
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
//returns the next character after comments
//different than next character because after comments the 
//internal position of the current character is not moved
//to be improved
char Input_Buffer::afterCommentCharacter()
{
	int next_c = currentCharacter+1;
	if(next_c < CurrentLine.length())
	{
		return CurrentLine[next_c];
	}
	else
	{
		return '\0';
	}
}

//returns the previous line, used for error reporting
string Input_Buffer::getPreviousLine()
{
	return PreviousLine;
}


// ==== Implementing scanner ====

//Constructor
Scanner::Scanner(names *nm, string file)
{
	nmz = nm;
	inBuffer = new Input_Buffer(file);
}


//Returns number line maintained by the input buffer
int Scanner::GetCurrentLineNumber()
{
	return inBuffer->getCurrentLineNumber() ;
}

//Returns current line string maintained by the input buffer
string Scanner::GetCurrentLine()
{
	// removes last character which is a  new line character
	string str = inBuffer->getCurrentLine();
	str.pop_back(); 
	return str;
}

//Read the following digits and converts them to a number stored in numb
//the next character after the last digit is placed in ch
//returns the eof flag
bool Scanner::GetNextNumber(int &numb, char &ch)
{
	string s; 
	
	while(isdigit(current_char))
	{
		s += current_char;
		eof_flag = inBuffer->getChar(current_char);
	}
	
	try{
		numb = stoi(s);
	}
	catch(invalid_argument ia){
		numb = -1; //No number found
	}
	
	ch = current_char;
	if(isspace(ch))
	{
		eof_flag = GetNextChar(ch);
	}
	return eof_flag;
}

//returns the eof flag and palces the next character of the file to ch
bool Scanner::GetNextChar(char &ch)
{
	eof_flag = inBuffer->getChar(current_char);
	
	//skip spaces
	if(isspace(current_char))
	{
		skipspaces();
	}
	
	//if a comment is found skip the line
	if((current_char == '/') && (inBuffer->NextCharacter() == '/'))
	{
		skipSingleLineComment();
		if(eof_flag == false)
		{
			eof_flag = inBuffer->getChar(current_char);
			
			
			if(isspace(current_char)) skipspaces();
			
			
		}
	}
	//multi line comment. skip it
	if((current_char == '/') && (inBuffer->NextCharacter() == '*'))
	{
		skipMultiLineComment();
		if(eof_flag == false && isspace(current_char))
		{
			skipspaces();
		}
	}
	
	ch = current_char;
	
	return eof_flag;
}


//gets the next string on the text file and retunrs it in string
//the character following the string is returned in ch
//returns the eof flag
bool Scanner::GetNextString(string &str, char &ch)
{
	string s;
	s = "";
	if(current_char == '\0')
	{
		eof_flag = inBuffer->getChar(current_char);
	}

	
	while(eof_flag == false)
	{

		if(isspace(current_char))
		{

			skipspaces();
		}
		//if a comment is found skip the line
		else if((current_char == '/') && (inBuffer->NextCharacter() == '/'))
		{
			skipSingleLineComment();

			if(eof_flag == false)
			{
				eof_flag = inBuffer->getChar(current_char);
			}
			if(isspace(current_char))
			{

				skipspaces();
			}

		}
		//multi-line comment detected
		else if((current_char == '/') && (inBuffer->NextCharacter() == '*'))
		{

			skipMultiLineComment();
		}
		else
		{
			break;
		}		
	}
	
	if(isspace(current_char)) skipspaces();

	
	if(isalpha(current_char))//first character must be a
	{
		//numbers and underscores allowed in the identifier
		while(isalnum(current_char) || current_char == '_')
		{
			s += current_char;
			eof_flag = inBuffer->getChar(current_char);

			if(eof_flag)
			{
				break;
			}
		}
	}
	
	str = s;
	ch = current_char;
	
	if(isspace(ch))
	{
		eof_flag = GetNextChar(ch);
	}
	//some strings that resulted in bugs were caugth, and tried to reread the string
	if((str == "" || str == "\n" || str==" " || str == "\0"|| str=="\t") && eof_flag == false)
	{
		 GetNextString(str,ch);
	}
	//scanner sometimes read 2-3 empty lines after the eof and returned empty strings
	if(str == "") eof_flag = true;
	return eof_flag;
}


//skips all the spaces until a non white space character is detected
void Scanner::skipspaces()
{
	while(isspace(current_char) && (eof_flag == false))
	{
		eof_flag = inBuffer->getChar(current_char);
	}
}

//skips singleline comment
void Scanner::skipSingleLineComment()
{
	eof_flag = inBuffer->moveToNextLine(current_char); 
	
	current_char = inBuffer->getCurChar();
	

	if((int)current_char == 0 && (int)inBuffer->NextCharacter() == 10) 
	{
		eof_flag = inBuffer->moveToNextLine(current_char); 
	
		current_char = inBuffer->getCurChar();
	}
	
	//different cases that led to bugs when many different combinations of 
	//single line comments, multi line comments, tabs, line-feeds and new lines
	//were used
	if(isspace(current_char)) skipspaces();
	if((int)current_char == 10) eof_flag = inBuffer->getChar(current_char);
	if(current_char == '\0')
	{
		if(inBuffer->NextCharacter() == '/' && inBuffer->afterCommentCharacter() =='/') skipSingleLineComment();
	}
	if(current_char == '/' && inBuffer->afterCommentCharacter() =='/') skipSingleLineComment();
	
	if(current_char == '/' && inBuffer->NextCharacter() == '/') skipSingleLineComment();
	
	if(isspace(current_char)) skipspaces();

	if(current_char == '/' && inBuffer->afterCommentCharacter() =='*')  skipMultiLineComment();
	if(current_char == '/' && inBuffer->NextCharacter() == '*') skipMultiLineComment();
	
	if(isspace(current_char)) skipspaces();
	if((int)current_char == 0 && (int)inBuffer->NextCharacter() == 10)
	
	cout << "after single cur character = " << (int)current_char << "  next char = " <<(int)inBuffer->NextCharacter() << endl;
}

//different cases that led to bugs when many different combinations of 
//single line comments, multi line comments, tabs, line-feeds and new lines
//were used
void Scanner::skipMultiLineComment()
{	
	 while(eof_flag == false)
	 {
	 	if(current_char == '*' && inBuffer->NextCharacter() == '/')
	 	{
	 		eof_flag = inBuffer->getChar(current_char);
	 		eof_flag = inBuffer->getChar(current_char);
	 		break;
	 	}
	 	else
	 	{
	 		eof_flag = inBuffer->getChar(current_char);
	 	}
	 }
	 //different cases that led to bugs when many different combinations of 
	//single line comments, multi line comments, tabs, line-feeds and new lines
	//were used
	if(current_char == '\0') eof_flag = inBuffer->getChar(current_char);
	if(isspace(current_char)) skipspaces();
	if(current_char == '\0') eof_flag = inBuffer->getChar(current_char);
	if(current_char == '/' && inBuffer->NextCharacter() =='/') skipSingleLineComment();
	if(isspace(current_char)) skipspaces();
	if(current_char == '\0') eof_flag = inBuffer->getChar(current_char);
	if(current_char == '/' && inBuffer->NextCharacter() =='*')  skipMultiLineComment();
	if(isspace(current_char)) skipspaces();
	if(current_char == '\0')
	{
		if(inBuffer->NextCharacter() == '/' && inBuffer->afterCommentCharacter() =='/') skipSingleLineComment();
	}
	if(current_char == '\0') eof_flag = inBuffer->getChar(current_char);
	if(current_char == '/' && inBuffer->NextCharacter() =='*')  skipMultiLineComment();
}

//returns the current character without moving the internal pointer to the next
bool Scanner::GetCurrentChar(char &ch)
{
	ch = current_char;
	return eof_flag;
}

//returns the previous line used for debugging
string Scanner::GetPreviousLine()
{
	return inBuffer->getPreviousLine();
}

//returns the character position used for debugging
int Scanner::GetCharPosition()
{
	return inBuffer->getCharPosition();
}

// main used for testing and debugging
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
		if(isalpha(ch))
		{
			eof = scan.GetNextString(str, ch);
			if(str.length() != 0)
			{
				cout <<"Current String: " << str << "\tNext char: " << ch <<endl;
			}
		}
		else
		{
			eof = scan.GetNextChar(ch);
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
