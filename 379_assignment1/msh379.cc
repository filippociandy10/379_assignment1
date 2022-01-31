#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdarg>  //Handling of variable length argument lists
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

struct sample {
    int index;
    pid_t pid;
    string cmd;
};

void pdir(char *ptr, size_t size) {
    if (getcwd(ptr, size) == NULL) {
        std::cout << "get cwd failed";
    } else {
        std::cout << ptr << endl;
    }
}

void cdir(vector<string> &arguments) {
    const char *chdir_command;
    chdir_command = (arguments[1]).c_str();
    if (chdir(chdir_command) < 0) {
        std::cout << "cdir failed\n";
    }
}

void run(vector<sample> &tasks, vector<string> &arguments, int counter) {
    pid_t pid;
    string cmd;
    int index;
    arguments.erase(arguments.begin());
    vector<char *> argv = vector<char *>(arguments.size());
    transform(arguments.begin(), arguments.end(), argv.begin(),
              [](const string &arg) { return (char *)arg.c_str(); });

    argv.push_back(NULL);

    if ((pid = fork()) < 0) {
        std::cout << "Fork error!";
    }
    if (pid == 0) {
        if (arguments[0] == "myclock") {
            execlp("sh", "sh", argv[0], argv[1], NULL);
        } else if (arguments[0] == "mMyclock") {
            execlp("sh", "sh", argv[0], NULL);
        } else {
            if ((execvp(argv.data()[0], argv.data()) < 0)) {
                std::cout << "execvp error!" << endl;
            }
        }
    } else {
        for (int i = 0; i < counter - 1; i++) {
            cmd = cmd.append(arguments[i]);
            cmd = cmd.append(" ");
        }
        index = tasks.size();
        sample s = {index, pid, cmd};
        tasks.push_back(s);
    }
}

void lstasks(vector<sample> tasks) {
    for (const auto &elem : tasks) {
        std::cout << elem.index << ":    (pid= " << elem.pid
                  << ", cmd= " << elem.cmd << ")" << endl;
    }
}

void terminate(vector<sample> &tasks, string taskno) {
    sample tasks_obj = tasks[stoi(taskno)];
    pid_t pid_terminate = tasks_obj.pid;
    kill(pid_terminate, 1);
    tasks.erase(tasks.begin() + stoi(taskno));
    std::cout << tasks_obj.index << ":    (pid= " << tasks_obj.pid
              << ", cmd= " << tasks_obj.cmd << ")"
              << " terminated" << endl;
}

void exit(vector<sample> &tasks) {
    for (auto sample : tasks) {
        pid_t pid_terminate = sample.pid;
        std::cout << "task " << sample.pid << " terminated." << endl;
        kill(pid_terminate, 1);
    }
}

void stop(vector<sample> &tasks, string taskno) {
    sample tasks_obj = tasks[stoi(taskno)];
    pid_t pid_stop = tasks_obj.pid;
    signal(SIGSTOP, SIG_IGN);
    kill(pid_stop, SIGSTOP);
}

void cont(vector<sample> &tasks, string taskno) {
    sample tasks_obj = tasks[stoi(taskno)];
    pid_t pid_continue = tasks_obj.pid;
    signal(SIGCONT, SIG_IGN);
    kill(pid_continue, SIGCONT);
}
void check(string target_pid) {
    const int MAX_LINE = 1234;
    char line[MAX_LINE];
    FILE *fp;
    vector<string> fake_records;
    vector<vector<string> > real_records;
    vector<string> temp_arr;
    int index = 0;
    char space = ' ';
    string word = "";
    if ((fp = popen("ps -u $USER -o user,pid,ppid,state,start,cmd --sort start",
                    "r")) == NULL) {
        std::cout << "cant open!" << endl;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        string temp = "";
        if (line != "\n") {
            temp = temp + line;
        }
        fake_records.push_back(temp);
        temp.clear();
    }
    for (int i = 0; i < fake_records.size(); i++) {
        string line = fake_records[i];
        while (temp_arr.size() < 5) {
            while ((line[index]) != space) {
                word = word + line[index];
                index++;
            }
            if (word != "") {
                temp_arr.push_back(word);
            }

            word = "";
            index++;
        }
        temp_arr.push_back(line.substr(index));
        real_records.push_back(temp_arr);
        temp_arr.clear();
        index = 0;
    }
    for (auto x : real_records) {
        if (x[1] == target_pid) {
            if (x[3] == "Z") {
                cout << "target_pid= " + target_pid + " terminated" << endl;
            } else if (x[3] == "S") {
                cout << "target_pid= " + target_pid + " is running:" << endl;
            }
            std::cout << "USER:      "
                      << "PID:  "
                      << "PPID: "
                      << "S:  "
                      << "STARTED: "
                      << "CMD:" << endl;
            std::cout << x[0] + "  " << x[1] + "  " << x[2] + "  "
                      << x[3] + "  " << x[4] + "  " << x[5] << endl;
        }
    }
}
void pr_times(clock_t real, struct tms *tmsstart, struct tms *tmsend) {
    static long clktck = 0;
    if (clktck == 0) {
        if ((clktck = sysconf(_SC_CLK_TCK)) < 0) {
            std::cout << "sysconf error" << endl;
        }
    }
    std::cout << (" real: ") << (real / (double)clktck) << endl;
    std::cout << (" user: ")
              << ((tmsend->tms_utime - tmsstart->tms_utime) / (double)clktck)
              << endl;
    std::cout << (" sys: ")
              << ((tmsend->tms_stime - tmsstart->tms_stime) / (double)clktck)
              << endl;
    std::cout << (" child user:  ")
              << ((tmsend->tms_cutime - tmsstart->tms_cutime) / (double)clktck)
              << endl;
    std::cout << (" child sys:  ")
              << ((tmsend->tms_cstime - tmsstart->tms_cstime) / (double)clktck)
              << endl;
}
int main() {
    // Start time of program
    struct tms tmsstart, tsmend;
    clock_t start = times(&tmsstart);
    // Set limit on CPU time
    rlimit rlim_time;
    rlim_time.rlim_cur = 10;
    rlim_time.rlim_max = 2;
    if (setrlimit(RLIMIT_CPU, &rlim_time) == 0) {
        std::cout << "Success\n";
    }
    // Path
    size_t size = 1024;
    char *ptr = (char *)malloc(size);

    string input = "";
    string command = "";
    vector<string> arguments;
    vector<sample> tasks;
    string line;
    int counter = 0;
    pid_t pid_process = getpid();
    std::cout << "msh379 [" + to_string(pid_process) + "]:";
    while (command != "quit") {
        // Get all the arguments in cin and puts in a vector
        getline(cin, line);
        std::istringstream stream(line);
        while (stream.good()) {
            string temp;
            stream >> temp;
            arguments.push_back(temp);
            counter++;
        }

        // pdir command
        if (arguments[0] == "pdir") {
            pdir(ptr, size);
        }
        // cdir command
        if (arguments[0] == "cdir") {
            cdir(arguments);
        }
        // run command
        if ((arguments[0] == "run") && (tasks.size() <= 32)) {
            run(tasks, arguments, counter);
        }
        // lstasks command
        if (arguments[0] == "lstasks") {
            lstasks(tasks);
        }
        // exit command
        if (arguments[0] == "exit") {
            exit(tasks);
            break;
        }
        // stop command
        if (arguments[0] == "stop") {
            string taskno = arguments[1];
            stop(tasks, taskno);
        }
        // continue command
        if (arguments[0] == "continue") {
            string taskno = arguments[1];
            cont(tasks, taskno);
        }
        // terminate command
        if (arguments[0] == "terminate") {
            string taskno = arguments[1];
            terminate(tasks, taskno);
        }
        // check command
        if (arguments[0] == "check") {
            string target_pid = arguments[1];
            check(target_pid);
        }
        // Clear vector and counter after each execution
        arguments.clear();
        counter = 0;
        std::cout << "msh379 [" + to_string(pid_process) + "]:";
    }
    // Exit program
    clock_t end = times(&tmsstart);
    pr_times(end - start, &tmsstart, &tsmend);
}
