#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include "helper.h"

int main(int argc, char** argv) {

   Manager package;
   cout << "Argument 1: " << argv[1] << endl;
   cout << "Argument 2: " << argv[2] << endl << endl;
   package.initilize(argv[1], argv[2]);

   int sid; // semaphore id
   sid = semget(123,2,IPC_CREAT | 0666); //declaring the semaphore 2nd argument determines number of semaphores made in this case 2 are made
   //1st arguement is the id 3rd is a flag
   struct sembuf sb[2]; // used to control the semaphore somewhat like the 2 size array for a pipe
   sb[0].sem_num = 0;// selecting semaphore 0
   sb[0].sem_op = -1;//decrement semaphore by 1 (wait) sb[0] will be our wait operation
   sb[0].sem_flg = 0;
   sb[1].sem_num = 0;
   sb[1].sem_op =  1;//increment semaphore by 1 (release) sb[1] will be our release operation
   sb[1].sem_flg = 0;

   semop(sid,&sb[1],1); // default value of a semaphore is 0 (waiting for a release) so you have to increment it by one like here
   // in order for the first process that hit's semaphore 0 to keep running.

   int shmid; // shared memory ID
   int *allocated;
   vector<int*> allocation;

   for (int i = 0; i < package.r; i++ ) {
      shmid = shmget(321,sizeof(allocated),IPC_CREAT | 0666); // making a shared variable for bank account (this is basically memory allocation).

      if(shmid<0){
      cout<<"error with shmid"<<endl;
      exit(1);
      }

      allocated = (int *)shmat(shmid,NULL,0);// shared memory bank account being declared
      if (allocated == (int *) (-1))
      {
      cout<<"allocated error"<<endl;
      exit(1);
      }
      allocation.push_back(allocated);
      *allocation[i] = 0;
   }
   int pnum = -1;
   int pid;
   for(int k =0; k<package.p+1; k++)  { //will create the number of processes in package
      pid = fork();
	   if(pid ==0) {
	      pnum = k;
	      break;
	   }
   }

   //pnum 0 - package.p are children
   //pnum package.p + 1 is the process manager
   if (pid == 0 && pnum < package.p && pnum != -1)  {//is a child process
      /*---functions---
      request //makes are request for resources and if allocated will sit waiting to be 'used' passes 1 time unit
      calculate //just passes time in ()
      use_resource //commits the allocated resources to master string and passes time in ()
      print //prints current process master string passes 1 unit time
      release //releases indicated resources passes 1 unit time
      end. //ends process, shuts down
      */

      //a matrix that will tell me how many of what resource i have as well as the instance of that resource
      vector<vector<int>> myResources;//current resources i have allocated to this process
      vector<vector<int>> masterString;//index of stored resources and the instances used in process
      for (int i = 0; i < package.r; i++) {
         vector<int> temp;
         for (int j = 0; j < package.available[i]; j++) {
            temp.push_back(0);
         }
         myResources.push_back(temp);
      }
      int instr = -1;
      int time = 1;
      int data = 0;
      string prompt;
      for (int i = 0; i < package.processes[pnum].instructions.size(); i ++)  {
         prompt = package.processes[pnum].instructions[i];
         if (prompt.find("request") != string::npos) {instr = 0;}
         if (prompt.find("calculate") != string::npos) {instr = 1;}
         if (prompt.find("use") != string::npos) {instr = 2;}
         if (prompt.find("print") != string::npos) {instr = 3;}
         if (prompt.find("release") != string::npos) {instr = 4;}
         if (prompt.find("end") != string::npos) {instr = 5;}
         switch (instr) {
            case 0:
               int response; //0 request rejected, 1 request approved
               while(bool makeRequest = true) {
                  // this will look at the available resources and its own max, if either is false this is rejected
                  //request in the format of request(0,0,...m) where m is the number of resources requested in the resource index order set
                  cout << "process " << pnum << " calling request function..." << endl;
                  //write pnum
                  write(package.processes[pnum].pipeWrite[1], &pnum, sizeof(pnum));
                  //write instruction id 0
                  write(package.processes[pnum].pipeWrite[1], &instr, sizeof(instr));
                  //write each resouce request one after another
                  for (int j = 0; j < package.r; j++) {
                     //write the resource being requested to management process
                  }
                  read(package.processes[pnum].pipeRead[0], &response, sizeof(int));
                  if (response == 1)   {
                     //write the resource being requested to my resources
                     //break out of while
                     makeRequest = false;
                  }
               }
               break;
            case 1:

               cout << "process " << pnum << " calling calculate function..." << endl;
               //write pnum
               write(package.processes[pnum].pipeWrite[1], &pnum, sizeof(pnum));
               //write instruction id 1
               write(package.processes[pnum].pipeWrite[1], &instr, sizeof(instr));
               //write number of cylces to run calculations
               data = stoi(prompt.substr(prompt.find('('), prompt.find(')') - prompt.find('(')));
               cout << data << endl;
               write(package.processes[pnum].pipeWrite[1], &data, sizeof(data)); 

               break;
            case 2:
               cout << "process " << pnum << " calling use function..." << endl;
               //write pnum
               write(package.processes[pnum].pipeWrite[1], &pnum, sizeof(pnum));
               //write instruction id 2
               write(package.processes[pnum].pipeWrite[1], &instr, sizeof(instr));
               //write number of cylces to run use
               //data = stoi(prompt.substr(prompt.find('('), prompt.find(')') - prompt.find('(')));
               write(package.processes[pnum].pipeWrite[1], &data, sizeof(data));
               //add myResources to master string
               for (int j = 0; j < package.r; j++) {
                  for (int k = 0; k < package.resources.size(); k++) {
                     masterString[j][k] = masterString[j][k] + myResources[j][k];
                  }
               }
               break;
            case 3:
               // this process will print out the process calling the print function as well as the process' master string
               cout << "process " << pnum << " calling print function..." << endl;
               //write pnum
               write(package.processes[pnum].pipeWrite[1], &pnum, sizeof(pnum));
               //write instruction id 3
               write(package.processes[pnum].pipeWrite[1], &instr, sizeof(instr));
               //write time to add 1
               //print pnum
               //print local master string
               break;
            case 4:
               cout << "process " << pnum << " calling release function..." << endl;
               //write pnum
               write(package.processes[pnum].pipeWrite[1], &pnum, sizeof(pnum));
               //write instruction id 4
               write(package.processes[pnum].pipeWrite[1], &instr, sizeof(instr));
               //write time to add
               //clear my resources
               //increment package allocated and available
               break;
            case 5:
               cout << "process " << pnum << " calling end function..." << endl;
               //write pnum
               write(package.processes[pnum].pipeWrite[1], &pnum, sizeof(pnum));
               //write instruction id 5
               break;
            default:
               break;
         }
         //wait for signal to run next instruction
      }
      //cout << "PROCESS " << pnum + 1 << " instruction: " << package.processes[pnum].instructions[0] << endl;
      return 0;
   }
   else if (pnum != -1) {
      int timePassed = 0;
      int data;
      int instr;
      cout << "im the management process " << pnum << endl;
      cout << "after every request function called in the other process' i will print the current allocated, need, and deadline misses..." << endl;
      //read from all three processes and try to complete their requests
      //wait for all three processes to get their unit time in, == to number of processes
      //then signal for next process
      for (int i = 0; i < package.r; i++) {
         *allocation[i] = i;
         cout << "from shared: " << *allocation[i] << endl;
      }
      return 0;
   }

   //main function waiting for child processes to stop running
   for (int i = 0; i < package.p+1; i++){
      wait(0);
   }

   //read the time data sent from the children
   for (int i = 0; i < package.p; i++){
      int unit;
      read(package.processes[i].pipeWrite[0], &unit, sizeof(unit));
   }

   //close all pipes
   for (int i = 0; i < package.p; i ++)   {
      close(package.processes[i].pipeRead[0]);
      close(package.processes[i].pipeRead[1]);
      close(package.processes[i].pipeWrite[0]);
      close(package.processes[i].pipeWrite[1]);
   }

   //	Destroy the semaphores
	semctl(sid, 0, IPC_RMID);

   shmdt((void *) allocated);//needs to delete all shared memory in vector shared

   return 0;
}

/*
REFERENCES:
1.https://pubs.opengroup.org/onlinepubs/009695399/functions/semop.html#:~:text=%20%20%20%20Member%20Type%20%20,%20%20sem_flg%20%20%20Operation%20flags.%20
2.
*/