#include <iostream>
#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
using namespace std;


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
//class AlwaysRuns : public Command {
//	public:
//		AlwaysRuns() : Command(){};
//		bool evaluate(){cout << "ran successfully" << endl; return true;}
//};
//class NeverRuns : public Command {
//	public:
//		NeverRuns() : Command(){};
//       	bool evaluate(){cout << "run unsuccessful" << endl; return false;}
//};
int main(){
	//very first command to be entered runs successfully
	//bool c0 = true;
	//Base* c1 = new Command(new AlwaysRuns);
	//Base* c2 = new Command(new AlwaysRuns);
	//Base* c3 = new Command(new AlwaysRuns);
	
	//Base* c0andc1 = new ConnectAnd(c0, c1);
	//Base* c0orc2 = new ConnectOr(c0, c2);
	//Base* c0semc3 = new ConnectSem(c0, c3); 	
	
	//cout << "c1...";
	//c0andc1->evaluate();
	//cout << "c2...";
	//c0orc2->evaluate();
	//cout << "c3...";
	//c0semc3->evaluate();
	//char * argument[] = {"ls", NULL};
	//Base* test = new Command(argument);
	//test->evaluate();
	

	vector<string> args {"ls"};
	bool c0 = true;
	Base* c1 = new Command(args);
	Base* c0andc1 = new ConnectAnd(c0, c1);
	c0andc1->evaluate();

	


        return 0;
}
