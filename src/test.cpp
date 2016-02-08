#include <iostream>
#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <netdb.h>
#include <pwd.h>
#include <sys/socket.h>
#include <boost/algorithm/string.hpp>
using namespace std;
using namespace boost::algorithm;

class Base {
    public:
        Base(){};
        virtual bool evaluate() = 0;
};

//super to "&&" "||" and ";" connectors
class Connector : public Base {
        public:
                Connector() : Base (){};
        protected:
                bool firstRun;
                Base* rt;
};
//Command will serve as our leaf node in our composite pattern design
class Command : public Base {
        private:
		vector<string> args;
        public:
		//constructor takes in unformatted vector that holds command and arguments
		Command(vector<string> a){args = a;}
		bool evaluate() {
			if (args[0] == "exit") {
				exit(0);
			}
			//formatting the vector
			vector<char *> a2;
			for (int i = 0; i < args.size(); i++) {
				a2.push_back(const_cast<char *>(args.at(i).c_str()));
			}
			a2.push_back('\0');
			char** array = &a2[0];

			//fork so multiple processes can happen at once
			pid_t pid = fork();
			if (pid < 0) {
				perror("fork failed");
				return false;
				exit(1);
			}
			else if (pid == 0) {
				//can now pass into execvp() with some final touches to the formatting
				execvp(const_cast<char *>(array[0]), array);
				return true;
			}
			else if (pid > 0) {
				//wait until child finishes or else bad things will happen
				wait(NULL);	
			}
			return true;			
		} 
};
class ConnectAnd : public Connector {
        public:
                ConnectAnd(bool f, Base* r) {firstRun = f; rt = r;}
                //attempt to run right child only if first command runs 
		bool evaluate(){
			if (firstRun){
				return rt->evaluate(); 
			}
			return false;
		}
};
class ConnectOr : public Connector {
        public:
                ConnectOr(bool f, Base* r) {firstRun = f; rt = r;}
                //attempt to run right child only if first command fails to run
		bool evaluate(){
			if(!firstRun) {
				return rt->evaluate();
			}
			return false;
        	}
};class ConnectSem : public Connector {
        public:
                ConnectSem(bool f, Base* r) {firstRun = f; rt = r;}
                //always attempt to run next right child
		bool evaluate(){
                        return rt->evaluate();
        	}       
};
//function that splits string into vector of substrings
vector<string> split(string s, const char* delim) {
	//s is initial string
	char charCom[s.size()+1];
	//convert string to char array
        strcpy(charCom,s.c_str());
        charCom[s.size()+1] = '\0';
        char * cutter;
	//parse string
        cutter = strtok(charCom, delim);
        char* args[s.size()];
        vector<string> subStrings;
	//creates substrings and puts them in vector
        while (cutter != NULL) {
		string word(cutter);
		trim(word);
		subStrings.push_back(word);
                cutter = strtok(NULL, delim);
        }
	return subStrings;
}
int main(){

	//begin taking commands
	string initCommand;
	
	//infinitely loop unless exit is found (which returns 0)
	while (1) {
		//get extra info for prompt (extra credit)
		string login = getlogin();
        	char hostname[100];
        	int temp = gethostname(hostname, 100);
        	cout << "[" << login << " " << hostname << "] $ ";
		
		getline(cin, initCommand);
		trim(initCommand);
		bool noCommand;
		if (initCommand == "") {
			noCommand = true;
		}
		else {
			noCommand = false;	
		}
		while (noCommand == false) {
			//if there exists comment, delete it
			string commandEntered = initCommand.substr(0, initCommand.find("#", 0));		
			//fills vector of connectors so we can refer to them when instantiating connector classes
			vector<string> connectorVector;
			for(unsigned int i = 0; i < commandEntered.length(); i++) {
				if (commandEntered[i] == '&') {
					if (commandEntered[i + 1] == '&') {
						connectorVector.push_back("&&");		
					} 
				}
				else if (commandEntered[i] == '|') {
					if (commandEntered[i + 1] == '|') {
						connectorVector.push_back("||");
					}
				}
				else if (commandEntered[i] == ';') {
					connectorVector.push_back(";");
				}
			}
			//splits statement with multiple commands into seperate commands
			vector<string> mycommands = split(commandEntered, "||&&;");
			
			//run first command and get  boolean value
			vector<string> firstCommand = split(mycommands.at(0), " ");
			Base* first = new Command(firstCommand);
			bool c0 = first->evaluate();	
			
			//instantiates connectors (filters subsequent commands based on successful first command run) 	
			for (int i = 0; i < connectorVector.size(); i ++) {
				Base* nextCommand;
				//formats single command to be passed into execvp() function
				vector<string> argList = split(mycommands.at(i + 1), " ");
			
				if (connectorVector.at(i) == "&&") {
					nextCommand = new ConnectAnd(c0, new Command(argList));
				}
				else if (connectorVector.at(i) == "||") {
					nextCommand = new ConnectOr(c0, new Command(argList));
				}
				else if (connectorVector.at(i) == ";") {
					nextCommand = new ConnectSem(c0, new Command(argList));						
				}
				nextCommand->evaluate();
			}
			//ready for next command
			noCommand = true;
		}
	}
	
        return 0;
}
