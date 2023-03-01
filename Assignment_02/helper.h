#ifndef HELPER_H
#define HELPER_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

struct process {
    int deadline;
    int computationTime;
    vector<string> instructions;
    int pipeRead[2];//process will read from parent
    int pipeWrite[2];//process will wrtie to parent
};

struct resource {
    string type;
    vector<string> instances;
    vector<int> allocated; //if 1 not allocated if 0 this resource is unavailable
};

class Manager {
    public:
    int r; //number of resources
    int p; //number of processes
    vector<int> available; //number of instances available of a particular resources from resource 1, 2, ... r
    vector<vector<int>> max; //maximum demand for resource r by process n. takes the form max[n,m]
    vector<process> processes; //a vector of processes with deadlines, compTimes, and their instructions
    vector<resource> resources; //a vector of all resources and their instances
    void initilize(string argument_1, string argument_2); //argument is the argv[1] from the input and 2 is the second argv[2]
    void initilize_alt(string argument_1, string argument_2); //argument is argv[1]
};

void Manager::initilize (string argument_1, string argument_2) {
    ifstream input(argument_1);
    ifstream words(argument_2);
    ofstream packageInfo("Output/Info.txt");
    string temp;

    //number of resources
    getline(input, temp);
    r = stoi(temp);
    packageInfo <<"r: " << r << endl;

    //number of processes
    getline(input, temp);
    p = stoi(temp);
    packageInfo <<"p: " << p << endl << endl;

    //first line of available[] input, will determine if isMatrix remains true;
    getline(input, temp);
    if (temp.find("available") != string::npos) {
        //call alternative initilize
        initilize_alt(argument_1, argument_2);
        return;
    }

    //matrix info
    packageInfo << "---matrix info---" << endl << endl;

    //available---how many instances of each resources there are
    packageInfo << "available info: " << endl;
    //record how many instances of resource type i there are available
    getline(input, temp);
    //preping input with delimiter ":"
    temp.insert(temp.size()-1,":");//some wierd endline character is at the end of my input and lobing off characters in the next line couldnt figure out what that was so i added delimiters
    while (temp.find(' ') != string::npos)  {
        temp[temp.find(' ')] = ':';
    }

    for (int i = 0; i < r; i++) {
        available.push_back(stoi(temp.substr(0,temp.find(':'))));
        temp.erase(0,temp.find(':') + 1);
    }

    for (int i = 0; i < available.size(); i++)  {
        packageInfo << available[i] << " ";
    }
    packageInfo << endl;

    for(int i = 0; i < r; i++)  {
        resource resource_temp;

        //recording the word file input with instances of resource type i
        getline(words, temp);
        for (int j = 0; j < temp.size(); j++) {
            if (temp[j] == ' ') {
                temp.erase(temp.begin()+j);
            }
        }
        resource_temp.type = temp.substr(temp.find(':') + 1, temp.find(':', temp.find(":") + 1) - (temp.find(':') + 1));
        packageInfo << "resource " << i << " type: " << resource_temp.type << endl;
        temp.erase(0,temp.find(':',temp.find(':') + 1) + 1);
        temp.insert(temp.size()-1,",");
        packageInfo << "resource " << i << " has " << available[i] << " instances: ";
        for (int j = 0; j < available[i]; j++)  {
            resource_temp.instances.push_back(temp.substr(0, temp.find(',')));
            resource_temp.allocated.push_back(1);
            packageInfo << resource_temp.instances[j] << " ";
            temp.erase(0,temp.find(',') + 1);
        }
        resources.push_back(resource_temp); //place resource into the manager object's vector for storage
        packageInfo << endl << endl;
    }

    getline(input, temp);//eat a line

    //max--defines how many resource units of type j that process i can demand at one time
    packageInfo << "max info:" << endl;
    for(int i = 0; i < p; i++)  {
        vector<int> demand;
        max.push_back(demand);
        getline(input, temp);
        //prep line for parsing
        temp.insert(temp.size()-1,":");//some wierd endline character is at the end of my input and lobing off characters in the next line couldnt figure out what that was so i added delimiters
        while (temp.find(' ') != string::npos)  {
            temp[temp.find(' ')] = ':';
        }
        for (int j = 0; j < r; j++) {
            packageInfo << "maximum demand for resource " << j << " by process " << i << ": ";
            max[i].push_back(stoi(temp.substr(0,temp.find(':'))));
            temp.erase(0,temp.find(':') + 1);
            packageInfo << max[i][j] << endl;
        }
    }
    packageInfo << endl;

    getline(input, temp); //eat a line

    packageInfo << "---process definition and instructions---" << endl << endl;
    for (int i = 0; i < p; i++) {
        process process_temp;
        getline(input, temp); //eat the name of the process
        packageInfo << temp << endl;

        getline(input, temp);
        process_temp.deadline = stoi(temp);
        packageInfo << "deadline: " << process_temp.deadline << endl;

        getline(input, temp);
        process_temp.computationTime = stoi(temp);
        packageInfo << "computation time: " << process_temp.computationTime << endl;

        packageInfo << endl;

        while (temp.find("end.") == string::npos) {
            getline(input, temp);
            process_temp.instructions.push_back(temp);
        }

        for (int i = 0; i < process_temp.instructions.size(); i++) {
            packageInfo << process_temp.instructions[i] << endl;
        }

        pipe(process_temp.pipeRead);
        pipe(process_temp.pipeWrite);

        processes.push_back(process_temp);
        packageInfo << endl << "---------------------" << endl << endl;
    }

    return;
}

void Manager::initilize_alt (string argument_1, string argument_2) {
    ifstream input(argument_1);
    ifstream words(argument_2);
    ofstream packageInfo("Output/Info.txt");
    string temp;

    packageInfo << "---Entering alternative initilize function---" << endl << endl;

    //number of resources
    getline(input, temp);
    r = stoi(temp);
    packageInfo <<"r: " << r << endl;

    //number of processes
    getline(input, temp);
    p = stoi(temp);
    packageInfo <<"p: " << p << endl << endl;

    //matrix info
    packageInfo << "---matrix info---" << endl << endl;

    //available---how many instances of each resources there are
    packageInfo << "available info: " << endl;
    for(int i = 0; i < r; i++)  {
        resource resource_temp;

        //record how many instances of resource type i there are available
        getline(input, temp);
        available.push_back(stoi(temp.substr(13)));

        //recording the word file input with instances of resource type i
        getline(words, temp);
        for (int j = 0; j < temp.size(); j++) {
            if (temp[j] == ' ') {
                temp.erase(temp.begin()+j);
            }
        }

        resource_temp.type = temp.substr(temp.find(':') + 1, temp.find(':', temp.find(":") + 1) - (temp.find(':') + 1));
        packageInfo << "resource " << i << " type: " << resource_temp.type << endl;
        temp.erase(0,temp.find(':',temp.find(':') + 1) + 1);
        temp.insert(temp.size()-1,",");
        packageInfo << "resource " << i << " has " << available[i] << " instances: ";
        for (int j = 0; j < available[i]; j++)  {
            resource_temp.instances.push_back(temp.substr(0, temp.find(',')));
            resource_temp.allocated.push_back(1);
            packageInfo << resource_temp.instances[j] << " ";
            temp.erase(0,temp.find(',') + 1);
        }
        resources.push_back(resource_temp); //place resource into the manager object's vector for storage
        packageInfo << endl << endl;
    }

    //max--defines how many resource units of type j that process i can demand at one time
    packageInfo << "max info:" << endl;
    for(int i = 0; i < p; i++)  {
        vector<int> demand;
        max.push_back(demand);
        for (int j = 0; j < r; j++) {
            getline(input, temp);
            //packageInfo << " maximum demand for resource " << j << " by process " << i << ": " << temp << endl;
            packageInfo << "maximum demand for resource " << j << " by process " << i << ": ";
            max[i].push_back(stoi(temp.substr(9)));
            packageInfo << max[i][j] << endl;
        }
    }
    packageInfo << endl;

    packageInfo << "---process definition and instructions---" << endl << endl;
    for (int i = 0; i < p; i++) {
        process process_temp;
        getline(input, temp); //eat the name of the process
        packageInfo << temp << endl;

        getline(input, temp);
        process_temp.deadline = stoi(temp);
        packageInfo << "deadline: " << process_temp.deadline << endl;

        getline(input, temp);
        process_temp.computationTime = stoi(temp);
        packageInfo << "computation time: " << process_temp.computationTime << endl;

        packageInfo << endl;

        while (temp != "end.") {
            getline(input, temp);
            process_temp.instructions.push_back(temp);
        }

        for (int i = 0; i < process_temp.instructions.size(); i++) {
            packageInfo << process_temp.instructions[i] << endl;
        }

        processes.push_back(process_temp);
        packageInfo << endl << "---------------------" << endl << endl;
    }

    return;
}

#endif