#include <iostream>
#include <vector>
#include <list>
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
                Command* command;
        public:
		Command(){}
                Command (Command* c) {command = c;}
                bool evaluate(){
			return command->evaluate();
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
class AlwaysRuns : public Command {
	public:
		AlwaysRuns() : Command(){};
		bool evaluate(){cout << "ran successfully" << endl; return true;}
};
class NeverRuns : public Command {
	public:
		NeverRuns() : Command(){};
        	bool evaluate(){cout << "run unsuccessful" << endl; return false;}
};
int main(){
	//very first command to be entered runs successfully
	bool c0 = true;
	Base* c1 = new AlwaysRuns;
	Base* c2 = new AlwaysRuns;
	Base* c3 = new AlwaysRuns;
	
	Base* c0andc1 = new ConnectAnd(c0, c1);
	Base* c0orc2 = new ConnectOr(c0, c2);
	Base* c0semc3 = new ConnectSem(c0, c3); 	
	
	cout << "c1...";
	c0andc1->evaluate();
	cout << "c2...";
	c0orc2->evaluate();
	cout << "c3...";
	c0semc3->evaluate();


	
        return 0;
}
