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
#include <sys/stat.h>
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
		~Connector();
        protected:
                bool firstRun;
                Base* rt;
};
class Test {
	private:
		int flag;
        public:
		bool didFind;
                Test(vector<string> testArgs) {
			if (testArgs[0] == "-e") {
				testArgs.erase(testArgs.begin());
				flag = 1;
			}
			else if (testArgs[0] == "-f") {
				testArgs.erase(testArgs.begin());
				flag = 2;
			}
			else if (testArgs[0] == "-d") {
				testArgs.erase(testArgs.begin());
				flag = 3;
			}
			else {
				flag = 1;
			}
			vector<char *> a2;
                       	a2.push_back(const_cast<char *>(testArgs[0].c_str()));
                        a2.push_back('\0');
                        char** array = &a2[0];
			struct stat fileStat;
    			if(stat(const_cast<char *>(array[0]) ,&fileStat) < 0) {    
        			didFind = false;
			}
			else {
				didFind = true;
				if (flag == 2) {
                         	       (S_ISREG(fileStat.st_mode)) ? didFind = true : didFind = false;
                        	}
                        	else if (flag == 3) {
                                	(S_ISDIR(fileStat.st_mode)) ? didFind = true : didFind = false;
                        	}
			}

                        for (unsigned int i = 0; i < testArgs.size(); i++) {
                                cout << testArgs[i] << " ";
                        }
                        cout << endl;
                }
};
//Command will serve as our leaf node in our composite pattern design
class Command : public Base {
        private:
		vector<string> args;
        public:
		//constructor takes in unformatted vector that holds command and arguments
		Command(vector<string> a){args = a;}
		~Command();
		bool evaluate() {
			if (args[0] == "exit") {
				exit(0);
			}
			//custom test command
			else if (args[0] == "test") {
				args.erase(args.begin());
				Test* testCommand = new Test(args);
				if (testCommand->didFind) {
					cout << "true" << endl;
					return true;
				}
				else {
					cout << "false" << endl;
					return false;
				}
			}
			else {
				//formatting the vector
				vector<char *> a2;
				for (unsigned int i = 0; i < args.size(); i++) {
					a2.push_back(const_cast<char *>(args.at(i).c_str()));
				}
				a2.push_back('\0');
				char** array = &a2[0];

				//fork so multiple processes can happen at once
				int status;
				pid_t pid = fork();
				if (pid < 0) {
					perror("fork failed");
					exit(1);
				}
				else if (pid == 0) {
					//can now pass into execvp() with some final touches to the formatting
					execvp(const_cast<char *>(array[0]), array);
					//if reaches this part then error
					exit(127);
				}
				else if (pid > 0) {
					//wait until child finishes or else bad things will happen
					//waitpid(pid, &status, 0);
					wait(&status);
					if (wait(&status) != -1) {
						perror("wait error");
					}
					if(WIFEXITED(status)){
    						if(WEXITSTATUS(status) == 0){
						//program succeeded
						return true;
						}
						else {
						//program failed but exited normally
						return false;
						
						}
					}
					else{
					//program exited abnormally
					perror("abormal child exit");
					return false;
					}
				}
			
				return false;
			}
			return false;			
		} 
};
class ConnectAnd : public Connector {
        public:
                ConnectAnd(bool f, Base* r) {firstRun = f; rt = r;}
		~ConnectAnd();
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
               	~ConnectOr();
		//attempt to run right child only if first command fails to run
		bool evaluate(){
			if(!firstRun) {
				return rt->evaluate();
			}
			return false;
        	}
};
class ConnectSem : public Connector {
        public:
                ConnectSem(bool f, Base* r) {firstRun = f; rt = r;}
         	~ConnectSem();
	        //always attempt to run next right child
		bool evaluate(){
                        return rt->evaluate();
        	}       
};
//function that splits string into vector of substrings
vector<string> split(string s, const char* delim) {
	//s is initial string
	char* charCom = new char[s.size() + 1];
	//convert string to char array
        strcpy(charCom, s.c_str());
        charCom[s.size() + 1] = '\0';
        char* cutter;
	//parse string
        cutter = strtok(charCom, delim);
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
        	gethostname(hostname, 100);
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
			string commandEntered = initCommand.substr(0, initCommand.find('#', 1));
			//check for test brackets
			int index1;
			int index2;
			if (commandEntered.find('[') != string::npos) {
				index1 = commandEntered.find('[');
				if (commandEntered.find(']') != string::npos) {
					index2 = commandEntered.find(']');
				}
				else cout << "No closing bracket" << endl;
				commandEntered.erase(index1,1);
				commandEntered.erase(index2 - 1,1);
				commandEntered.insert(0, "test ");
			}					
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
			vector<string> myCommands = split(commandEntered, "||&&;");
			//run first command and get  boolean value
			vector<string> firstCommand = split(myCommands.at(0), " ");
			Base* first = new Command(firstCommand);
			bool c0 = first->evaluate();

			//instantiates connectors (filters subsequent commands based on successful first command run) 	
			for (unsigned int i = 0; i < connectorVector.size(); i ++) {
				Base* nextCommand;
				//formats single command to be passed into execvp() function
				vector<string> argList = split(myCommands.at(i + 1), " ");
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
