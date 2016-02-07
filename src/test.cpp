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
class Connector : public Base {
        public:
                Connector() : Base (){};
        protected:
                bool firstRun;
                Base* rt;
};
class Command : public Base {
        private:
		vector<string> args;
        public:
		Command(vector<string> a){args = a;}
		bool evaluate() {
			vector<char *> a2;
			for (int i = 0; i < args.size(); i++) {
				a2.push_back(const_cast<char *>(args.at(i).c_str()));
			}
			char** array = &a2[0];
			pid_t pid = fork();
			if (pid < 0) {
				perror("fork failed");
				return false;
				exit(1);
			}
			else if (pid == 0) {
				cout << "in child" << endl;
				execvp(const_cast<char *>(array[0]), array);
				return true;
			}
			else if (pid > 0) {
				cout << "in parent, waiting" << endl;
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
//function to format string to pass into execvp()
vector<string> separateWords(string s) {
	char charCom[s.size()+1];
        strcpy(charCom,s.c_str());
        charCom[s.size()+1] = '\0';
        char * cutter;
        cutter = strtok(charCom, " ");
        char* args[s.size()];
        vector<string> argList;
        while (cutter != NULL) {
		string word(cutter);
		trim(word);
		argList.push_back(word);
                cutter = strtok(NULL, " ");
        }
	return argList;
}
int main(){

	//get extra info for prompt

	/*string login = getlogin();
 	char hostname[100];
 	int temp = gethostname(hostname, 100);
 	cout << "[" << login << " " << hostname << "] $";*/

	//begin taking commands
	string commandEntered;
	while (commandEntered != "exit") {
		cout << "$ ";
		getline(cin, commandEntered); 
	
		//fills vector of connectors so we can refer to them when instantiating connector classes
		vector<string> connectorVector;
		for(unsigned int i = 0; i < commandEntered.length(); i++) {
    			if (commandEntered[i] == '#') {
				break;
			}
			else if (commandEntered[i] == '&') {
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
			trim(singlecommand);
			mycommands.push_back(singlecommand);
			cutter = strtok(NULL, "|&;");	
		}
		
		//get boolean value of first command
		vector<string> firstCommand = separateWords(mycommands.at(0));
		Base* first = new Command(firstCommand);
                bool c0 = first->evaluate();	
		
		//instantiates connectors (filters subsequent commands based on successful first command run) 	
		for (int i = 0; i < connectorVector.size(); i ++) {
			Base* nextCommand;
			vector<string> argList = separateWords(mycommands.at(i + 1));
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
	}
	
        return 0;
}
