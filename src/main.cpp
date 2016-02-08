#include <string>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string.h>
using namespace std;
int main()
{
	cout << "$ ";
	string commandEntered;
	getline(cin, commandEntered);                                       
	
	//user input string


	//if #(comment) is detected, it will ignore everything that is commented

	std::size_t found = commandEntered.find('#');
	
	if(found != std::string::npos)
	{
		commandEntered = commandEntered.substr(0,commandEntered.find("#"));
	
	}
	


	int mysize = commandEntered.size() +1;	
	char* charCom = new char[mysize];
	
	//convert string to char array
	
	strcpy(charCom,commandEntered.c_str());
	
	charCom[mysize] = '\0';


	char * cutter;
	
	//parse string
	
	cutter = strtok(charCom, "||&&;");
	
	char* args = new char[commandEntered.size()];
	
	vector<string> mycommands;

	//creates single commands and adds them to vector as strings

	while(cutter!=NULL)
	{		
		string singlecommand(cutter);
		mycommands.push_back(singlecommand);
		cutter = strtok(NULL, "|&;");	
	}
	

	for(unsigned k = 0; k<mycommands.size(); k++)
	{
		cout<<mycommands.at(k)<<endl;

	}


			
	return 0;



}
