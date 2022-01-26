#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <ctime>
#include <cassert>
#include <cstring>
#include <cstdarg>             //Handling of variable length argument lists
#include <sys/time.h>

#include <unistd.h>		
#include <sys/types.h>
#include <sys/stat.h>	
#include <sys/wait.h>
#include <sys/times.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <vector>
using namespace std;


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

    cout << "msh379 [pid]: ";
    cin >> input;
    while (command != "quit"){
        cout << "msh379 ["+input+"]:";
        cin >> command;
        //pdir command
        if (command == "pdir"){
            if (getcwd(ptr,size) == NULL){
                cout << "get cwd failed";
            }
            else{
                cout << ("cwd = %s", ptr);
                cout << "\n";
            }
        }
        //cdir command
        if (command == "cdir"){
            const char* chdir_command;
            cin >> command;
            chdir_command = command.c_str();
            if (chdir(chdir_command)<0){
                cout << "cdir failed\n";
            }
            else{
                cout << ("cdir to %s successful\n",command);
                cout << "\n";
            }
        }
    }
}