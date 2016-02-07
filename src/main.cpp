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
	
	char charCom[commandEntered.size()+1];
	
	//convert string to char array
	
	strcpy(charCom,commandEntered.c_str());
	
	charCom[commandEntered.size()+1] = '\0';


	char * cutter;
	
	//parse string
	
	cutter = strtok(charCom, "||&&;");
	
	char* args[commandEntered.size()];
	
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
