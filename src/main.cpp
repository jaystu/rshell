#include <iostream>
#include <vector>
#include <list>
#include <stack>
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
		//true if file found and/or matches filetype specified
		//false if file not found or filetype does not match what was specified 
		bool didFind;

		//formats argument so that only file name is left
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
			//tests if file was located
    			if(stat(const_cast<char *>(array[0]) ,&fileStat) < 0) {
        			didFind = false;
			}
			else {//if file located and filetype specified then test to see if filetype matches
				didFind = true;
				if (flag == 2) {
                         	       (S_ISREG(fileStat.st_mode)) ? didFind = true : didFind = false;
                        	}
                        	else if (flag == 3) {
                                	(S_ISDIR(fileStat.st_mode)) ? didFind = true : didFind = false;
                        	}
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
				//if no file passed then exit()
				if (args.empty() || (args.size() == 1 && (args[0] == "-e" || args[0] == "-f" || args[0] == "-d"))) {
					cout << "no file passed, exiting..." << endl;
					exit(0);
				}
				Test* testCommand = new Test(args);
				//if file was found
				if (testCommand->didFind) {
					cout << "(true)" << endl;
					return true;
				}
				//either file wasn't found or wasn't file type specified by flag
				else {
					cout << "(false)" << endl;
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
//function that takes in position of opening paren and spits out pos of its corresponding closing paren
unsigned getEndParenPos(string commandEntered, unsigned pos) {
	stack<char> stack;
	unsigned m = pos;
	stack.push('f');//'f' stands for the first opening paren
	m++;
	for(; m < commandEntered.size(); m++) {
		if(commandEntered.at(m) == '(') {
			stack.push('(');
		}
		else if(commandEntered.at(m) == ')') {
			char openParen = stack.top();
			stack.pop();
			if (stack.empty() && openParen == 'f') {//found closing paren that matches first opening paren
				return m;		
			}
		}
	}
	return m;
}
//function that deletes unneeded parenthesis on both sides of string example "(echo 1 && echo 2)" does not need parenthesis
string trimParens(string commandEntered) {
        stack<char> stack;
        bool stillEncapsulated = false;
        do {
        	trim(commandEntered);
        	if (commandEntered.find('(') != 0) {//no open paren in 0 position so no need to go further
        		return commandEntered; 
                }
                else if (getEndParenPos(commandEntered, 0) == commandEntered.size() - 1) {//if there is open paren and its closing paren is the very last char 
			commandEntered.erase(0, 1);
			commandEntered.erase(commandEntered.size() - 1);
			if (getEndParenPos(commandEntered, 0) == commandEntered.size() - 1) {
				stillEncapsulated = true;
			}
			else {
				return commandEntered;
			}
		}
		else {
			return commandEntered;
		}
	} while (stillEncapsulated == true); return commandEntered;
}
//function to make sure correct amount and order of parenthesis passed into command line
void checkParens(string commandEntered) {	
	stack<char> myStack;
	for(unsigned m = 0; m < commandEntered.size(); m++) {
		if(commandEntered.at(m) == '(') {
			myStack.push('(');
		}
		else if(commandEntered.at(m) == ')') {
			if(!myStack.empty()) {
				myStack.pop();
			}
			else{
				cout<<"Error: parenthesis mismatch. Missing \'('. " << endl;
				exit(0);
			}
		}
	}
	if(!myStack.empty()) {
        	cout<<"Error: paranthesis mismatch. Missing \')'." << endl;
        	exit(0);
        }
}
class Group : public Base {
        private:
                string commandEntered;
                vector<bool> trueTracker;
		bool hasNestedGroup;
        public:
                Group(string commandEntered) {
                        this->commandEntered = commandEntered;
                }
		//if single group(doesn't have nested groups) then evaluates normally, else evaluates recursively
                bool evaluate() {
			trim(commandEntered);
			
			//if single command but encapsulated with parenthesis erase parenthesis
			commandEntered = trimParens(commandEntered);
			//if somehow empty command gets passed in
			if (commandEntered == "" ) {
				return true;
			}
			//check if nested groups present
			hasNestedGroup = false;
                        for(unsigned u = 0; u < commandEntered.size(); u++) {
                                if (commandEntered.at(u) == '(') {
					hasNestedGroup = true;
                                }
			}
			
			if (hasNestedGroup) {
				//vector to hold connectors between groups
				vector<string> connectorVector;
				//vector to hold all the groups
				vector<string> myGroups;
				
				unsigned startOfGroup;
				unsigned endOfGroup;
				string groupToPush;
				for(unsigned m = 0; m < commandEntered.size();) {
					if(commandEntered.at(m) == '(') {//whatever is inside parenthesis is considered group
						startOfGroup = m;
						endOfGroup = getEndParenPos(commandEntered, m);
						groupToPush = commandEntered.substr(startOfGroup, endOfGroup - startOfGroup + 1);//extract group
						myGroups.push_back(groupToPush);
						m += endOfGroup - startOfGroup + 1;//skip ahead so don't read over characters that have already been read
					}
					else if (commandEntered[m] == '&') {//push connectors and then skip over them
						if (commandEntered[m + 1] == '&') {
							connectorVector.push_back("&&");
						}
						m += 2;
					}
					else if (commandEntered[m] == '|') {
						if (commandEntered[m + 1] == '|') {
							connectorVector.push_back("||");
						}
						m += 2;
					}
					else if (commandEntered[m] == ';') {
						connectorVector.push_back(";");
						m++;
					}
					else if (commandEntered[m] == ' ') {//skip a space
						m++;
					}
					else {
						startOfGroup = m;
						unsigned a;
						unsigned o;
						unsigned s;
						//next group found if connector found
						a = commandEntered.find("&&", m);
						o = commandEntered.find("||", m);
						s = commandEntered.find(";", m);
						if (commandEntered.find("&&", m) == string::npos && commandEntered.find("||", m) == string::npos && commandEntered.find(";", m) == string::npos) {
							//no more groups found
							endOfGroup = commandEntered.size();
						}
						else {//new group found
							if (a < o && a < s) {
								endOfGroup = a - 1;
							}
							else if (o < a && o < s) {
								endOfGroup = o - 1;
							}	
							else if (s < a && s < o) {
								endOfGroup = s - 1;
							}
						}
						groupToPush = commandEntered.substr(startOfGroup, endOfGroup - startOfGroup);
                                                m += endOfGroup - startOfGroup;
                                                myGroups.push_back(groupToPush);
					}
				}
				
				/*cout << "groups in myGroups: " << endl;
				cout << "------------------" << endl;
				for (unsigned i = 0; i < myGroups.size(); i++) {
					cout << myGroups.at(i) << endl;
				}
				cout << "------------------" << endl;

				cout << "connectors in connectorVector: " << endl;
				cout << "------------------" << endl;
				for (unsigned i = 0; i < connectorVector.size(); i++) {
					cout << connectorVector.at(i) << endl;
				}
				cout << "------------------" << endl;*/
				
				Base* first = new Group(myGroups[0]);
				bool c0 = first->evaluate();

				for (unsigned int i = 0; i < connectorVector.size(); i ++) {
					Base* nextGroup;

					if (connectorVector.at(i) == "&&") {
						nextGroup = new ConnectAnd(c0, new Group(myGroups[i + 1]));
					}
					else if (connectorVector.at(i) == "||") {
						nextGroup = new ConnectOr(c0, new Group(myGroups[i + 1]));
					}
					else if (connectorVector.at(i) == ";") {
						nextGroup = new ConnectSem(c0, new Group(myGroups[i + 1]));
					}
					return nextGroup->evaluate();
				}

			}
			
			else if (!hasNestedGroup) {
				//check for test brackets
				int index1;
				int index2;
				if (commandEntered.find('[') != string::npos) {
					index1 = commandEntered.find('[');
					if (commandEntered.find(']') != string::npos) {
						index2 = commandEntered.find(']');
					}
					else {
						cout << "No closing bracket, exiting..." << endl;
						exit(0);
					}
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

				vector<string> firstCommand = split(myCommands.at(0), " ");
				Base* first = new Command(firstCommand);
				bool c0 = first->evaluate();
				trueTracker.push_back(c0);

				//instantiates connectors (filters subsequent commands based on successful first command run)
				for (unsigned int i = 0; i < connectorVector.size(); i ++) {
					Base* nextCommand;

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
					bool cNext = nextCommand->evaluate();
					trueTracker.push_back(cNext);
				}
				//if at least one command is true then whole group considered true
				for (unsigned int t = 0; t < trueTracker.size(); t++) {
					if (trueTracker[t] == true) {
						return true;
					}
				}
				return false;
			}
			return false;
                }

};
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
			//get comment out	
			string commandEntered = initCommand.substr(0, initCommand.find('#', 1));
			
			Base* entireLine = new Group(commandEntered);
			entireLine->evaluate();
	
			//ready for next command
			noCommand = true;
		}
	}	
        return 0;
}
