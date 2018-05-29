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
/*
Input_Buffer::Input_Buffer()
{
}
*/
int Input_Buffer::getCharPosition()
{
	return currentCharacter;
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
	
	else if(currentCharacter == CurrentLine.length() && eof == false)
	{
		eof = getNextLine();
		ch = CurrentLine[currentCharacter];
		currentCharacter++;
		return false;
	}
	else
	{	
		return true;
	}
	
}

bool Input_Buffer::getNextLine()
{
	PreviousLine = CurrentLine;
	getline(inpf, CurrentLine);
	CurrentLine += '\n';
	currentCharacter = 0;
	lineNumber++;
	
	return inpf.eof();
}

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

int Input_Buffer::getCurrentLineNumber()
{
	return lineNumber;
}


string Input_Buffer::getCurrentLine()
{
	return CurrentLine;
}

bool Input_Buffer::moveToNextLine(char &ch)
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

//Read the following characters and converts them to a string
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
		else if((current_char == '/') && (inBuffer->NextCharacter() == '*'))
		{

			skipMultiLineComment();
			 
			//if(eof_flag == false)
			//{
			//	eof_flag = GetNextChar(current_char);
			//}
			//if(isspace(current_char))
			//{

				//skipspaces();
			//}
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
			/*
			//if a comment is found skip the line
			if((current_char == '/') && (inBuffer->NextCharacter() == '/'))
			{
				skipSingleLineComment();
				if(eof_flag == false)
				{
					eof_flag = inBuffer->getChar(current_char);
					break;
				}
			}
	
			if((current_char == '/') && (inBuffer->NextCharacter() == '*'))
			{
				skipMultiLineComment();
				if(eof_flag == false)
				{
					eof_flag = inBuffer->getChar(current_char);
					break;
				}
			}*/
		}
	}
	str = s;
	ch = current_char;
	if(isspace(ch))
	{
		eof_flag = GetNextChar(ch);
	}
	if((str == "" || str == "\n" || str==" " || str == "\0"|| str=="\t") && eof_flag == false)
	{
		 GetNextString(str,ch);
	}
	if(str == "") eof_flag = true;
	return eof_flag;
}



void Scanner::skipspaces()
{
	while(isspace(current_char) && (eof_flag == false))
	{
		eof_flag = inBuffer->getChar(current_char);
	}
}

void Scanner::skipSingleLineComment()
{
	//cout << "Skipping s line" << endl;
	eof_flag = inBuffer->moveToNextLine(current_char); 
	
	current_char = inBuffer->getCurChar();
	
	
	//cout << "cur character = " << current_char << "  next char = " <<inBuffer->NextCharacter() << endl;
	//current_char = inBuffer->getCurChar();
	if((int)current_char == 0 && (int)inBuffer->NextCharacter() == 10) 
	{
		eof_flag = inBuffer->moveToNextLine(current_char); 
	
		current_char = inBuffer->getCurChar();
	}
	
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

void Scanner::skipMultiLineComment()
{	
	//cout << "Skipping m line" << endl;
/*
	bool s = false;
	while(eof_flag == false)
	{
		eof_flag = inBuffer->getChar(current_char);
		if((current_char == '*') && (inBuffer->NextCharacter() == '/'))
		{
			eof_flag = inBuffer->getChar(current_char);
			//eof_flag = inBuffer->getChar(current_char);
			//eof_flag = GetNextChar(ch);
			
			while(isspace(inBuffer->NextCharacter()))
			{
				//cout << "c char: " << current_char << endl;
				eof_flag = inBuffer->getChar(current_char);
				s = true;
			}
			if(s) eof_flag = inBuffer->getChar(current_char);
			
			//cout << "c char: " << current_char << endl;
			
			break;
		}
		
	}*/
	/*
	char previous_char = current_char;
	
	 while(eof_flag == false)
	 {
	 	if(previous_char == '*' && current_char == '/')
	 	{
	 		eof_flag = inBuffer->getChar(current_char);
	 		break;
	 	}
	 	else
	 	{
	 		previous_char = current_char;
	 		eof_flag = inBuffer->getChar(current_char);
	 	}
	 }
	 */ 
	 
	
	
	
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
	 /*
	if(isspace(current_char)) skipspaces();
	if(current_char == '/' && inBuffer->afterCommentCharacter() =='/') skipSingleLineComment();
	if(isspace(current_char)) skipspaces();
	if(current_char == '/' && inBuffer->afterCommentCharacter() =='*')  skipMultiLineComment();
	
	if(isspace(current_char)) skipspaces();*/
	 
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
	
	//cout << "After multi cur character = " << current_char << "  next char = " <<inBuffer->NextCharacter() << endl;
}

bool Scanner::GetCurrentChar(char &ch)
{
	ch = current_char;
	return eof_flag;
}

string Scanner::GetPreviousLine()
{
	return inBuffer->getPreviousLine();
}

int Scanner::GetCharPosition()
{
	return inBuffer->getCharPosition();
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
