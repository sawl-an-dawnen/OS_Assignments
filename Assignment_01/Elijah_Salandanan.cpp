#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <sys/wait.h>
#include <unistd.h>
#include "PPG.h"
#include "nameForNumber.h"

using namespace std;

int main(int argc, char** argv) {

   ofstream vertexInfo("vertexInfo.txt");
   ofstream pipeInfo("pipeInfo.txt");
   ofstream pipeLog("pipeLog.txt");
   ofstream output("output.txt");
   ifstream matrixInput(argv[1]);
   ifstream wordInput(argv[2]);
   
   int k = 0;//size of kxk matrix and number of processes to create
   int n = 0;//number of items that serve as inputs
   int r = 0;//for row count
   int c = 0;//for column count
   string stringTemp;//used for parsing
   item itemTemp;
   vector<item> basket;//basket
   vector<item> finalBasket;//basket
   list<int> tempList;//used to traverse list//////should have used vectors from beginning but i intergrated lists into the datastructures oopss lmao lets go

   //establish the size required for the matrix and number of processes
   getline(matrixInput, stringTemp);
   for (int i = 0; i < stringTemp.length(); i++)  {
      if (stringTemp[i] == ' ')
         continue;
      if (stringTemp[i] == '1' || stringTemp[i] == '0')  {
         k++;
      }
   }
   matrixInput.clear();
   matrixInput.seekg(0);

   //generate the graph
   PPG matrix(k);

   //fill graph and establish number of pipes
   while(!matrixInput.eof())  {
      getline(matrixInput, stringTemp);
      c = 0;
      for (int i = 0; i < stringTemp.length(); i++)  {
         if (stringTemp[i] == ' ')
            continue;
         if (stringTemp[i] == '1')  {
            matrix.addEdge(r,c);
            c++;
         }
         if (stringTemp[i] == '0')  {
            c++;
         }
      }
      r++;
   }
   //matrix.print();

   matrix.setInputVertex();
   //matrix.printInputList();

   matrix.setOutputVertex();
   //matrix.printOutputList();

   //establish basket of items
   while(!wordInput.eof())  {
      getline(wordInput, stringTemp);
      if (stringTemp == "")
         continue;
      while((n = stringTemp.find(",")) != string::npos) {
         itemTemp.setName(stringTemp.substr(0,n));
         basket.push_back(itemTemp);
         stringTemp.erase(0, n + 1);
         if(stringTemp.substr(0,1) == " ") {
            stringTemp.erase(0,1);
         }
      }
      if(stringTemp.substr(0,1) == " ") {
         stringTemp.erase(0,1);
      }
      itemTemp.setName(stringTemp);
      basket.push_back(itemTemp);
   }

//at this point the matrix and the words that need to be processed are all in a useable state
//below here is the development for the vertices and pipes
//we know how many pipes we need and the number of processes needed
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------//

   int processNum = matrix.getSize();
   int pipeNum = matrix.numberOfPipes() + matrix.numOfInput() + matrix.numOfOutput();;
   process pids[processNum];
   directedPipe pipes[pipeNum];
   pipeNum = 0;

   //making pipes for each process for the parent pipe//this will need
   for(int i = 0; i < processNum; i++) {
      for(int j = 0; j < processNum; j++) {
         if (matrix.graph[i][j] == 1)  {
            pipes[pipeNum].vertex = i;
            pids[j].myReadPipes.push_back(pipeNum);
            pipes[pipeNum].destination = j;
            pids[i].myWritePipes.push_back(pipeNum);
            if(pipe(pipes[pipeNum].tempPipe)){
               perror("invalid pipe");
            }
            pipeNum++;
         }
      }
   }
   //pipes for input
   tempList = matrix.inputVertex;
   for (int i = 0; i < matrix.numOfInput(); i++)   {
            pids[tempList.front()].myReadPipes.push_back(pipeNum);
            pipes[pipeNum].vertex = -1;//-1 stand in for main function
            pipes[pipeNum].destination = tempList.front();
            if(pipe(pipes[pipeNum].tempPipe)){
               perror("invalid pipe");
            }
            tempList.pop_front();
            pipeNum++;
   }
   //pipes for output
   tempList = matrix.outputVertex;
   for (int i = 0; i < matrix.numOfOutput(); i++)  {
            pids[tempList.front()].myWritePipes.push_back(pipeNum);
            pipes[pipeNum].vertex = tempList.front();
            pipes[pipeNum].destination = -2;//-2 stand in for output
            if(pipe(pipes[pipeNum].tempPipe)){
               perror("invalid pipe");
            }
            tempList.pop_front();
            pipeNum++;
   }
   //PIPE INFO PRINT
   for (int i = 0; i < pipeNum; i++) {
      pipeInfo << "PRINTING INFO FOR PIPE " << i << ":\n";
      pipeInfo << "vertex: " << pipes[i].vertex << endl;
      pipeInfo << "destination: " << pipes[i].destination << endl;
      pipeInfo << "pipe read side: " << pipes[i].tempPipe[0] << endl;
      pipeInfo << "pipe write side: " << pipes[i].tempPipe[1] << endl;
      pipeInfo << endl;
   }
   //PROCESS INFO PRINT + pids initilization
   for (int i = 0; i < processNum; i++)   {

      vertexInfo << "PRINTING INFO FOR PROCESS " << i << ":\n";
      //
      tempList = pids[i].myReadPipes;
      int size = tempList.size();
      vertexInfo << "Pipes I can read from: ";
      for (int j = 0; j < size; j++)   {
         vertexInfo << tempList.front() << " ";
         tempList.pop_front();
      }
      //
      tempList = pids[i].myWritePipes;
      size = tempList.size();
      vertexInfo << endl;
      vertexInfo << "Pipes I will write to: ";
      for (int j = 0; j < size; j++)   {
         vertexInfo << tempList.front() << " ";
         tempList.pop_front();
      }
      vertexInfo << endl << endl;
   }
   
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   //initilizing the input pipes with their values
   n = 2;//reusing n -- this variable now is the number of items sent into the pipe
   c = 0;//reusing c -- this is now the value being passed through
   int id;
   vector<int> functionBasket; //odds are count even are item IDs
   vector<int> tempBasket;
   //beginning will only read 1 thing in
   //the root parent process will write to all the input pipes
   pipeLog << "WRITING TO INPUT PIPES: " << endl;
   tempList = matrix.inputVertex;
   for (int j = 0; j < matrix.numOfInput(); j++)   {
      pipeLog << tempList.front() << " " << pids[tempList.front()].myReadPipes.front() << " " << basket[j].name << " ";
      write(pipes[pids[tempList.front()].myReadPipes.front()].tempPipe[1], &n, sizeof(int));
      write(pipes[pids[tempList.front()].myReadPipes.front()].tempPipe[1], &basket[j].count, sizeof(int));
      write(pipes[pids[tempList.front()].myReadPipes.front()].tempPipe[1], &j, sizeof(int)); //j is the item id, when you call basket[j].name you'll get the name of the fruit
      pipeLog << "working" << endl;
      tempList.pop_front();
   }

   pipeLog << "----------------------------------------------------------------------------------------------------------------------------------------------------------------" << endl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   
   //creating process with for loop
   for (int i = 0; i < processNum; i++)   {

      pids[i].pid = fork(); //creates child process and stores ID in array

      //process creation validation
      if (pids[i].pid == -1)   {
         cout << "ERROR CREATING PROCESS..." << endl;
         return 2;
      }

      //child process
      if (pids[i].pid == 0) { 
         //each process needs to close the majority of the pipes created in it's process.
         //close reading end of pipe
         //this will leave the reading end of the pipe associated with the i'th process open
         for (int j = 0; j < pipeNum; j++)   {
            if (pipes[j].destination != i)   {
               //pipeLog << j << " ";
               close(pipes[j].tempPipe[0]);
            }
         }
         //close writing end of pipe
         //this will leave the writing end the next pipe open for writing from the i'th process
         for (int j = 0; j < pipeNum; j++)   {
            if (pipes[j].vertex != i)   {
               //pipeLog << j << " ";
               close(pipes[j].tempPipe[1]);
            }
         }

         //READING PROCESS with read validation
         //in the i'th process we're reading from the i'th pipe (index 0)
         tempList = pids[i].myReadPipes;
         //j is the pipe i is the process and l is the item being read in
         for (int j = 0; j < pids[i].myReadPipes.size(); j++)  {
            read(pipes[tempList.front()].tempPipe[0], &n, sizeof(int));//read the number of items coming through pipe
            if(n > 2 || n <= 0)  {pipeLog << "->PROCESS " << i << ", " << n/2 <<" ITEMS INCOMING FROM PIPE " << j << endl;}
            else {pipeLog << "->PROCESS " << i << ", " << 1 << " ITEM INCOMING FROM PIPE " << j << endl;}
            pipeLog << "->PROCESS " << i << ", READ";
            for (int k = 0; k < n/2; k++)   {
               read(pipes[tempList.front()].tempPipe[0], &c, sizeof(int));//read the count of current item
               read(pipes[tempList.front()].tempPipe[0], &id, sizeof(int));//read the count of current item
               pipeLog << "->" << c << " " << id;
               functionBasket.push_back(c);
               functionBasket.push_back(id);
            }
            pipeLog << endl;
            tempList.pop_front();
         }

         //consolidate basket
         //divy up and organize the basket, currently contains all elements from every pipe, with duplicates
         while (functionBasket.size() != 0)  {
            c = functionBasket[0];
            id = functionBasket[1];
            functionBasket.erase(functionBasket.begin());
            functionBasket.erase(functionBasket.begin());
            for (int k = 1; k < functionBasket.size(); k = k + 2)   {
               if (functionBasket[k] == id)  {
                  c = c + functionBasket[k - 1];
                  functionBasket.erase(functionBasket.begin() + k);//erase id at k
                  functionBasket.erase(functionBasket.begin() + k - 1);//erase count at k-1
                  k = k-2;//set k back two because the basketsize was reduced by two
               }
            }
            tempBasket.push_back(c);
            tempBasket.push_back(id);
         }

         n = tempBasket.size();//n is equal to the number of elements in basket needing to be written over
         pipeLog << "->PROCESS " << i << ", BASKET ESTABLISHED" << endl;

         //WRITING PROCESS with read validation
         //in the i'th process we're writing to the i'th + 1 pipe's writing (index 1), with the value r once it's been modified
         tempList = pids[i].myWritePipes;
         for (int j = 0; j < pids[i].myWritePipes.size(); j++)  {
            pipeLog << "->PROCESS " << i << ", PIPE " << j << " WRITE->";
            write(pipes[tempList.front()].tempPipe[1], &n, sizeof(int));//write number of items to be passed through
            for (int k = 0; k < n; k++)   {
               pipeLog << tempBasket[k] << " ";
               write(pipes[tempList.front()].tempPipe[1], &tempBasket[k], sizeof(int));//write basket
            }
            tempList.pop_front(); 
         }

         pipeLog << "->PROCESS " << i << " FINISHED" << endl;

         //closing the remaining pipes that we're used in the current process-----------------------------------------------*******
         //close read
         tempList = pids[i].myReadPipes;
         for (int j = 0; j < pids[i].myReadPipes.size(); j++)   {
               close(pipes[tempList.front()].tempPipe[0]);
               tempList.pop_front();
         }
         //close write
         tempList = pids[i].myWritePipes;
         for (int j = 0; j < pids[i].myWritePipes.size(); j++)   {
               close(pipes[tempList.front()].tempPipe[1]);
               tempList.pop_front();
         };
         
         return 0; //could be break too if there is more i'd like it to do
      }
   }

   //wating for all child processes to finish execution
   for (int i = 0; i < matrix.getSize(); i++) {
      wait(NULL);
   } 

   pipeLog << endl << "----------------------------------------------------------------------------------------------------------------------------------------------------------------"
   << endl << "ALL CHILD PROCESSES RETURNED COMPLETE. MAIN FUCNTION PROCESSING IN PROGRESS..." << endl << endl;
   
   pipeLog << "CLOSING PIPES..." << endl;
   //closing all read and writes
   for (int i = 0; i < pipeNum; i++) {
      if(true)  {
         close(pipes[i].tempPipe[1]);
      }
      if(pipes[i].destination != -2)   {
         close(pipes[i].tempPipe[0]);
      }
   }

   tempBasket.clear();
   functionBasket.clear();
   //Main process reads from the last pipe--------------------------------------*****************output read
   //pipeLog << "READING FROM OUTPUT PIPES:" << endl;
   tempList = matrix.outputVertex;
   for (int i = 0; i < matrix.outputVertex.size(); i++)   {
      read(pipes[pids[tempList.front()].myWritePipes.front()].tempPipe[0], &n, sizeof(int));
      for (int j = 0; j < n/2; j++)   {
         read(pipes[pids[tempList.front()].myWritePipes.front()].tempPipe[0], &c, sizeof(int));
         read(pipes[pids[tempList.front()].myWritePipes.front()].tempPipe[0], &id, sizeof(int));
         tempBasket.push_back(c);
         tempBasket.push_back(id);
      }
      tempList.pop_front();
   };

   //function
   //consolidate basket
   //divy up and organize the basket, currently contains all elements from every pipe, with duplicates
   while (tempBasket.size() != 0)  {
      c = tempBasket[0];
      id = tempBasket[1];
      tempBasket.erase(tempBasket.begin());
      tempBasket.erase(tempBasket.begin());
      for (int i = 1; i < tempBasket.size(); i = i + 2)   {
         if (tempBasket[i] == id)  {
            c = c + tempBasket[i - 1];
            tempBasket.erase(tempBasket.begin() + i);//erase id at k
            tempBasket.erase(tempBasket.begin() + i - 1);//erase count at k-1
            i = i-2;//set k back two because the basketsize was reduced by two
         }
      }
      functionBasket.push_back(c);
      functionBasket.push_back(id);
   }

   //closing pipes opened by the main process
   pipeLog << "PROGRAM WINDING DOWN... CLOSING FINAL READ PIPES..." << endl;
   for (int i = 0; i < pipeNum; i++) {
      if(pipes[i].destination == -2)   {
         close(pipes[i].tempPipe[0]);
      }
   }

   pipeLog <<"FINAL BASKET:"<<endl;
   while(functionBasket.size() > 0) {
      pipeLog << functionBasket[0] << " " << functionBasket[1] << " ";
      itemTemp.setCount(functionBasket[0]);
      itemTemp.setName(basket[functionBasket[1]].name);
      functionBasket.erase(functionBasket.begin());
      functionBasket.erase(functionBasket.begin());
      finalBasket.push_back(itemTemp);
   }
   pipeLog << endl;

   basket = finalBasket;
   finalBasket.clear();

   while(basket.size() > 0)   {
      c = 0;//stores the index of our best alphabetically
      for (int k = 0; k < basket.size(); k++)   {
         if (basket[k].name < basket[c].name)   {
            c = k;
         }
      }
      finalBasket.push_back(basket[c]);
      basket.erase(basket.begin()+c);
   }

   for (int i = 0; i < finalBasket.size() - 1; i++)   {
      c = finalBasket[i].count;
      pipeLog << nameForNumber(c) << " " << finalBasket[i].name;
      if(finalBasket[i].count > 1)  {pipeLog << "s, ";}
      else {pipeLog << ", ";}
   }
   for (int i = 0; i < finalBasket.size() - 1; i++)   {
      c = finalBasket[i].count;
      output << nameForNumber(c) << " " << finalBasket[i].name;
      if(finalBasket[i].count > 1)  {output << "s, ";}
      else {output << ", ";}
   }
   for (int i = 0; i < finalBasket.size() - 1; i++)   {
      c = finalBasket[i].count;
      cout << nameForNumber(c) << " " << finalBasket[i].name;
      if(finalBasket[i].count > 1)  {cout << "s, ";}
      else {cout << ", ";}
   }
   n = finalBasket.size() - 1;
   c = finalBasket[n].count;

   pipeLog << nameForNumber(c) << " " << finalBasket[n].name;
   if(c > 1)  {pipeLog << "s" << endl;}
   else {pipeLog << "" << endl;}
   output << nameForNumber(c) << " " << finalBasket[n].name;
   if(c > 1)  {output << "s" << endl;}
   else {output << "" << endl;}
   cout << nameForNumber(c) << " " << finalBasket[n].name;
   if(c > 1)  {cout << "s" << endl;}
   else {cout << "" << endl;}

   return 0;
}

/*
REFERENCES:
1. CodeVault. "Calling fork multiple times" Youtube, 
Apr 28. 2020, https://www.youtube.com/watch?v=94URLRsjqMQ
2. CodeVault. "Calling fork multiple times (part 2)(With a practical example)" Youtube, 
Jan 16. 2020, https://www.youtube.com/watch?v=VzCawLzITh0
3. Vincenzo Pii, "Parse (split) a string in C++ using string delimiter (standard C++)" stackoverflow,
Jan 10, 2013, https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
4. Josh Homann, "C++ converting number to words" stackoverflow, 
Oct 26, 2016, https://stackoverflow.com/questions/40252753/c-converting-number-to-words
*/