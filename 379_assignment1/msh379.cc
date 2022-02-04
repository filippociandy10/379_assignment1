#include <inttypes.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// Global variables
struct sample {
    int index;
    pid_t pid;
    string cmd;
};
int terminate_counter = 0;

// Prints current path directory
void pdir(char *ptr, size_t size) {
    if (getcwd(ptr, size) == NULL) {
        std::cout << "get cwd failed";
    } else {
        std::cout << ptr << endl;
    }
}

// Change current path directory
void cdir(vector<string> &arguments) {
    const char *chdir_command;
    chdir_command = (arguments[1]).c_str();
    if (chdir(chdir_command) < 0) {
        std::cout << "cdir: failed (pathname = " << chdir_command << ")"
                  << endl;
    } else {
        std::cout << "cdir: done (pathname = " << chdir_command << ")" << endl;
    }
}

// Run programs by forking
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
            if (execv("myclock", argv.data()) < 0) {
                std::cout << "execv error!" << endl;
            };
        } else if (arguments[0] == "mMyclock") {
            if (execlp("sh", "sh", argv[0], NULL) < 0) {
                std::cout << "execlp error !" << endl;
            };
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

// Prints a list of tasks
void lstasks(vector<sample> tasks) {
    for (const auto &elem : tasks) {
        std::cout << elem.index << ":    (pid= " << elem.pid
                  << ", cmd= " << elem.cmd << ")" << endl;
    }
}

// Exit program by terminating running programs
void exit(vector<sample> &tasks) {
    for (auto sample : tasks) {
        pid_t pid_terminate = sample.pid;
        std::cout << "task " << sample.pid << " terminated." << endl;
        kill(pid_terminate, 1);
    }
}

// Helper function for check to print processes
void print_check(string target_pid, vector<vector<string> > real_records) {
    vector<string> temp;
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
            for (auto y : real_records) {
                if (y[2] == target_pid) {
                    temp.push_back(y[1]);
                    std::cout << y[0] + "  " << y[1] + "  " << y[2] + "  "
                              << y[3] + "  " << y[4] + "  " << y[5] << endl;
                }
            }
            for (auto z : temp) {
                for (auto b : real_records) {
                    if (z == b[2]) {
                        std::cout << b[0] + "  " << b[1] + "  " << b[2] + "  "
                                  << b[3] + "  " << b[4] + "  " << b[5] << endl;
                    }
                }
            }
        }
    }
}
// Check status of processes
vector<vector<string> > check(string target_pid, bool check_bool) {
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
        if ((string)line != "\n") {
            temp = temp + line;
        }
        fake_records.push_back(temp);
        temp.clear();
    }
    pclose(fp);
    for (int i = 0; i < (int)fake_records.size(); i++) {
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
    // If check_bool is false. Continue, stop, terminate functions called it.
    if (check_bool) {
        print_check(target_pid, real_records);
    } else {
        return real_records;
    }
}
// Terminate process
void terminate(vector<sample> &tasks, string taskno, int &terminate_counter) {
    vector<vector<string> > real_records;
    intmax_t ymax;
    pid_t y;
    char *tmp;
    sample tasks_obj = tasks[stoi(taskno) - terminate_counter];
    pid_t pid_terminate = tasks_obj.pid;

    real_records = check(to_string(pid_terminate), false);
    // Get pid where ppid is pid_terminate
    for (auto x : real_records) {
        if (x[2] == to_string(pid_terminate)) {
            const char *c = x[1].c_str();
            ymax = strtoimax(c, &tmp, 10);
            y = (pid_t)ymax;
            kill(y, 1);
        }
    }
    kill(pid_terminate, 1);
    tasks.erase(tasks.begin() + (stoi(taskno) - terminate_counter));
    std::cout << tasks_obj.index << ":    (pid= " << tasks_obj.pid
              << ", cmd= " << tasks_obj.cmd << ")"
              << " terminated" << endl;
    terminate_counter++;
}

// Stop processes
void stop(vector<sample> &tasks, string taskno) {
    vector<vector<string> > real_records;
    intmax_t ymax;
    pid_t y;
    char *tmp;
    sample tasks_obj = tasks[stoi(taskno)];
    pid_t pid_stop = tasks_obj.pid;
    real_records = check(to_string(pid_stop), false);

    kill(pid_stop, SIGSTOP);
    // Get pid where ppid is pid_stop
    for (auto x : real_records) {
        if (x[2] == to_string(pid_stop)) {
            // Changing string to pid_t
            const char *c = x[1].c_str();
            ymax = strtoimax(c, &tmp, 10);
            y = (pid_t)ymax;
            kill(y, SIGSTOP);
        }
    }
}

// Continue task which was stopped
void cont(vector<sample> &tasks, string taskno) {
    vector<vector<string> > real_records;
    intmax_t ymax;
    pid_t y;
    char *tmp;
    sample tasks_obj = tasks[stoi(taskno)];
    pid_t pid_continue = tasks_obj.pid;
    real_records = check(to_string(pid_continue), false);

    for (auto x : real_records) {
        if (x[2] == to_string(pid_continue)) {
            // Changing string to pid_t
            const char *c = x[1].c_str();
            ymax = strtoimax(c, &tmp, 10);
            y = (pid_t)ymax;
            kill(y, SIGCONT);
        }
    }
    kill(pid_continue, SIGCONT);
}
// Print times on quit/exit
void pr_times(clock_t real, struct tms *tmsstart, struct tms *tmsend) {
    static long clktck = 0;
    if (clktck == 0) {
        if ((clktck = sysconf(_SC_CLK_TCK)) < 0) {
            std::cout << "sysconf error" << endl;
        }
    }
    std::cout << (" real: ") << (real / (double)clktck) << " sec." << endl;
    std::cout << (" user: ")
              << ((tmsend->tms_utime - tmsstart->tms_utime) / (double)clktck)
              << " sec." << endl;
    std::cout << (" sys: ")
              << ((tmsend->tms_stime - tmsstart->tms_stime) / (double)clktck)
              << " sec." << endl;
    std::cout << (" child user:  ")
              << ((tmsend->tms_cutime - tmsstart->tms_cutime) / (double)clktck)
              << " sec." << endl;
    std::cout << (" child sys:  ")
              << ((tmsend->tms_cstime - tmsstart->tms_cstime) / (double)clktck)
              << " sec." << endl;
}
int main() {
    // Start time of program
    struct tms tmsstart, tsmend;
    clock_t start = times(&tmsstart);
    // Set limit on CPU time
    rlimit rlim_time;
    rlim_time.rlim_cur = 600;
    rlim_time.rlim_max = 600;
    if (setrlimit(RLIMIT_CPU, &rlim_time) == -1) {
        cout << "SETR LIMIT ERROR!" << endl;
    };

    // Path
    size_t size = 1024;
    char *ptr = (char *)malloc(size);
    bool check_bool = true;
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
        // quit command
        if (arguments[0] == "quit") {
            break;
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
            terminate(tasks, taskno, terminate_counter);
        }
        // check command
        if (arguments[0] == "check") {
            string target_pid = arguments[1];
            check_bool = true;
            // If check_bool is true, it means check command
            check(target_pid, check_bool);
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
