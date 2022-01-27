#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <ctime>
#include <cassert>
#include <cstring>
#include <cstdarg>             //Handling of variable length argument lists
#include <sys/time.h>
#include <fstream>
#include <unistd.h>		
#include <sys/types.h>
#include <sys/stat.h>	
#include <sys/wait.h>
#include <sys/times.h>
#include <fcntl.h>
#include <iostream>
#include <chrono>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

struct sample {int index; pid_t pid; string command;};

void pdir(char *ptr,size_t size){
    if (getcwd(ptr,size) == NULL){
        cout << "get cwd failed";
    }
    else{
        cout << ("cwd = %s", ptr);
        cout << "\n";
    }
}

void cdir (vector<string>& arguments){
    const char* chdir_command;
    chdir_command = (arguments[1]).c_str();
    if (chdir(chdir_command)<0){
        cout << "cdir failed\n";
    }
}

void run (vector<sample> &tasks,vector<string>& arguments, int counter, int index){
    pid_t pid;
    string temp;
    if ((pid == fork())<0){
        cout << "Fork error!";
    }
    else{
        cout << pid <<endl;
        for (int i=1; i<counter;i++){
            temp = temp.append(arguments[i]);
            temp = temp.append(" ");
        }
        sample s = {index,pid,temp};
        tasks.push_back(s);
    }
    // cout << index << pid << temp << endl;
}

int main(){
    // Set limit on CPU time
    rlimit rlim_time;
    rlim_time.rlim_cur = 10;
    rlim_time.rlim_max = 2;
    if (setrlimit(RLIMIT_CPU,&rlim_time) == 0){
        cout << "Success\n";
    }
    //Path
    size_t size= 1024;
    char *ptr = (char*) malloc(size);


    string input = "" ;
    string command = "";
    vector<string> arguments;
    vector <sample> tasks;
    string line;
    int counter = 0;
    int index = 0;

    cout << "msh379 [pid]: ";
    cin >> input;
    while (command != "quit"){
        
        // Get all the arguments in cin and puts in a vector
        getline(cin,line);
        std::istringstream stream(line);
        while(stream.good()){
            string temp;
            stream >> temp;
            arguments.push_back(temp);
        }

        //pdir command
        if (arguments[0] == "pdir"){
            pdir(ptr,size);
        }
        //cdir command
        if (arguments[0] == "cdir"){
            cdir(arguments);
        }
        //run command
        if (arguments[0] == "run"){
            cout << "in run";
            run(tasks,arguments, counter,index);
        }
        //Clear vector and counter after each execution
        arguments.clear();
        counter = 0;
        cout << "msh379 ["+input+"]:";
    }
}

