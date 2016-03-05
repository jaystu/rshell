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
					cout << "true" << endl;
					return true;
				}
				//either file wasn't found or wasn't file type specified by flag
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
string trimParens(string commandEntered) {
	stack<char> stack;
        cout << "evaluating: " << commandEntered << endl << endl;

        //if single command but encapsulated with parenthesis erase parenthesis
        bool stillEncapsulated = false;
	do {
                                stack.push('f');//push the first open parens to stack
                                cout << "in loop" << endl;
                                for(unsigned m = 1; m < commandEntered.size(); m++) {
                                        if(commandEntered.at(m) == '(') {
                                                stack.push('(');
                                        }
                                        else if(commandEntered.at(m) == ')') {
                                                char openParen = stack.top();
                                                stack.pop();
                                                if (stack.empty() && m == commandEntered.size() - 1 && openParen == 'f') {
                                                        commandEntered.erase(0, 1);
                                                        commandEntered.erase(commandEntered.size() - 1);
                                                        stillEncapsulated = false;
                                                        if (commandEntered.find('(') != 0) {
                                                                cout << "no more parens, complete " << endl << endl;
                                                                break;
                                                        }
                                                        else {
                                                                stack.push('f');
                                                                for(unsigned a = 1; a < commandEntered.size(); a++) {
                                                                        cout << "In third loop" << endl;
                                                                        if(commandEntered.at(a) == '(') {
                                                                                stack.push('(');
                                                                        }
                                                                        else if(commandEntered.at(a) == ')') {
                                                                                openParen = stack.top();
                                                                                stack.pop();
                                                                                if (stack.empty() && a == commandEntered.size() - 1 && openParen == 'f') {
                                                                                        cout << "still encapsulated " << endl << endl;
                                                                                        stillEncapsulated = true;
                                                                                        break;
                                                                                }
                                                                        }
                                                                }
                                                        }
                                                }
                                        }
                                }
                        trim(commandEntered);
                        cout << "deleted parens, new group: " << commandEntered << endl;
                        } while (stillEncapsulated == true);
	return commandEntered;
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
                bool evaluate() {
			trim(commandEntered);
			
			//if single command but encapsulated with parenthesis erase parenthesis
			commandEntered = trimParens(commandEntered);
			cout << "out of loop" << endl << endl;
			cout << "command entered after loop" << commandEntered << endl;
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
			Base* group1;
			Base* group2;
			Base* nextGroup;
			int parenIndex1;
			int parenIndex2;
			bool foundConnector;
			
			//find largest encapsulated group and split into 2 smaller groups and recursively evaluate each
			stack<char> stack;
			parenIndex1 = commandEntered.find('(');
			stack.push('f');
			for(unsigned m = parenIndex1 + 1; m < commandEntered.size(); m++) {
                                if(commandEntered.at(m) == '(') {
					stack.push('(');
                                }
                                else if(commandEntered.at(m) == ')') {//found encapsulated group
					char openParen = stack.top();
					stack.pop();
					parenIndex2 = m;
					vector<char> k;
					if (stack.empty() && openParen == 'f') 
					{
						cout << "found an encapsulated group" << endl << endl;
						for (int r = parenIndex1; r <= parenIndex2; r++) {
							k.push_back(commandEntered[r]);
						}
					
						//group that will split from base group
						string s(k.begin(), k.end() );
						cout << "s trying to be extracted : " << s << endl << endl;
						foundConnector = false;
						//find connector that connects group to other group
						for (unsigned w = 1; w <= 3; w++) {
							if (commandEntered[commandEntered.find(s) - w] == '&') {
								if (commandEntered[commandEntered.find(s) - w - 1] == '&') {
									s.insert(0, "&& ");
									foundConnector = true;
									break;
								}
							}
							else if (commandEntered[commandEntered.find(s) - w] == '|') {
								if (commandEntered[commandEntered.find(s) - w - 1] == '|') {
									foundConnector = true;
									s.insert(0, "|| ");
									break;
								}
							}
							else if (commandEntered[commandEntered.find(s) - w] == ';') {
								foundConnector = true;
								s.insert(0, "; ");
								break;
							}
						}
						if (foundConnector == false) {
							for (unsigned w = 1; w <= 3; w++) {
								if (commandEntered[commandEntered.find(s) + s.size() + w] == '&') {
									if (commandEntered[commandEntered.find(s) + s.size() + w + 1] == '&') {
										s.append(" &&");
										foundConnector = true;
										break;
									}
								}
								else if (commandEntered[commandEntered.find(s) + s.size() + w] == '|') {
									if (commandEntered[commandEntered.find(s) + s.size() + w + 1] == '|') {
										foundConnector = true;
										s.append(" ||");
										break;
									}
								}
								else if (commandEntered[commandEntered.find(s) + s.size() + w] == ';') {
									foundConnector = true;
									s.append(" ;");
									break;
								}
							}

						}
						//format base group so that 2 distinct groups are left
						commandEntered.erase(commandEntered.find(s), s.size());
						cout << "command extracted: " << s << endl << endl;
						trim(commandEntered);
						cout << "what is left of original: " << commandEntered << endl << endl;
						string t = commandEntered;

						//evaluate groups based on connector
						if (s.find(") &&") != string::npos) {
							s.erase(s.find(") &&") + 1, 3);
							group1 = new Group(s);
							group2 = new Group(t);
							nextGroup = new ConnectAnd(group1->evaluate(), group2);
						} 
						else if (s.find(") ||") != string::npos) {
							s.erase(s.find(") ||") + 1, 3);
							cout << "after split : " << s << endl << endl;
							group1 = new Group(s);
							group2 = new Group(t);
							nextGroup = new ConnectOr(group1->evaluate(), group2);
						}
						else if (s.find(") ;") != string::npos) {
							s.erase(s.find(") ;") + 1, 2);
							group1 = new Group(s);
							group2 = new Group(t);
							nextGroup = new ConnectSem(group1->evaluate(), group2);
						}
						else if (s.find("&& (") != string::npos) {
							s.erase(s.find("&& ("), 2);
							group1 = new Group(s);
							group2 = new Group(t);
							nextGroup = new ConnectAnd(group2->evaluate(), group1);
						}
						else if (s.find("|| (") != string::npos) {
							s.erase(s.find("|| ("), 2);
							group1 = new Group(s);
							group2 = new Group(t);
							nextGroup = new ConnectOr(group2->evaluate(), group1);
						}
						else if (s.find("; (") != string::npos) {
							s.erase(s.find("; ("), 1);
							group1 = new Group(s);
							group2 = new Group(t);
							nextGroup = new ConnectSem(group2->evaluate(), group1);
						}
						if (nextGroup->evaluate()) {
							return true;
						}
						else return false;
					}
                                }
                        }
			if (!hasNestedGroup) {
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
			string temp;
			//get comment out	
			string commandEntered = initCommand.substr(0, initCommand.find('#', 1));
			stack<char> myStack;
			
			//make sure parenthesis correct
			int parenCount = 0;
			trim(commandEntered);	
			commandEntered.insert(0,"(");
			commandEntered.append(")");
			for(unsigned m = 1; m < commandEntered.size() - 1; m++) {
        			if(commandEntered.at(m) == '(') {
					parenCount++;
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
				else if (myStack.empty() && (commandEntered.at(m) == '&' || commandEntered.at(m) == '|' || commandEntered.at(m) == ';')) {
					commandEntered.insert(m - 1, ")");
					commandEntered.insert(m + 4, "(");
					m = m + 4;
				}
			}
			cout << "With added Parenthesis: " << commandEntered << endl << endl << endl;
			//exit if parenthisis incorrect
			if(!myStack.empty()) {
        			cout<<"Error: paranthesis mismatch. Missing \')'." << endl;
        			exit(0);
			}

			//run command entered into command prompt
			Base* wholeCommand = new Group(commandEntered);
			(wholeCommand->evaluate()) ? cout << "group true" : cout << "group false";
			cout << endl;

			//ready for next command
			noCommand = true;
		}
	}
	
        return 0;
}
