#ifndef PPG_H
#define PPG_H
#include <string>
#include <list>
#include "PPG.h"


using namespace std;

struct item {
   string name = "";
   int count = 1;

   public:
   item() {
      name = "";
   }
   item(string temp) {
      name = temp;
   }
   item operator=(const item other) {
      name = other.name;
      count = other.count;
      return *this;
   }

   bool setName(string temp)  {
      name = temp;
      return true;
   }
   bool setCount(int temp)  {
      count = temp;
      return true;
   }
   int increment()   {
      count++;
      return count;
   }
   int getCount() {
      return count;
   }
   void printDetail()   {
      cout << count << " " << name << endl;
   }
};

struct directedPipe {
   //vertex association
   public:
   int vertex;
   int destination;
   int tempPipe[2];
};

struct process {
   public:
   int pid;
   list<int> myReadPipes;
   list<int> myWritePipes;

};

class PPG {
   int size;
   int pipeNum = 0;

   public:
   list<int> inputVertex;//list that will store the input vertex
   list<int> outputVertex;//list that will store the output vertex
   int** graph;
   PPG (int k);//constructor
   bool addEdge (int r, int c);
   bool deleteEdge (int r, int c);
   int getSize ();
   int numberOfPipes ();
   void print();
   void setInputVertex();
   void printInputList();
   void setOutputVertex();
   void printOutputList();
   int numOfInput();
   int numOfOutput();
   void generatePipes();
};
PPG::PPG (int k)  {
   size = k;
   graph = new int*[k];
   for (int i = 0; i < k; i++) {
      graph[i] = new int[k];
   }
   for (int i = 0; i < k; i++) {
      for (int j = 0; i < k; i++) {
         graph[i][j] = 0;
      }
   }
}
bool PPG::addEdge  (int r, int c)   {
   graph[r][c] = 1;
   pipeNum++;
   return true;
}
bool PPG::deleteEdge  (int r, int c)   {
   if (graph[r][c] = 1) {
      graph[r][c] = 0;
      pipeNum--;
      return true;
   }
   else
      return false;
}
int PPG::getSize ()  {
   return size;
}
int PPG::numberOfPipes ()  {
   return pipeNum;
}
void PPG::print() {
   for (int i = 0; i < size; i++)   {
      for (int j = 0; j < size; j++)   {
         cout << graph[i][j] << " ";
      }
      cout << endl;
   }
   cout << endl;
}
void PPG::setInputVertex()  {
   //input vertex are found if the vertex column is all 0
   int r = 0;
   int c = 0;
   int isInput = true;
   for (c = 0; c < size; c++)   {
      //cout << "CHECKING VERTEX: " << c << endl;
      isInput = true;
      for (r = 0; r < size; r++) {
         //cout << graph[r][c] << endl;
         if (graph[r][c] == 1)   {
            //cout << "VERTEX " << c << " IS NOT AN INPUT" << endl << endl;
            isInput = false;
            break;
         }
      }
      if (!isInput)  {
         continue;
      }
      //cout << "VERTEX " << c << " IS AN INPUT. ADDING TO LIST..." << endl << endl;
      inputVertex.push_back(c);
   }
}
void PPG::printInputList()   { 
   if (inputVertex.size() == 0)  {
      cout << "input list is empty..." << endl << endl;
      return;
   }
   cout << "INPUT VERTEX: ";
   for (list<int>::iterator it = this->inputVertex.begin(); it != this->inputVertex.end(); it++) { 
      cout << *it << " ";
   }
   cout << endl << endl;
}
void PPG::setOutputVertex() {
   //output vertex are found if the vertex row is all 0
   int r = 0;
   int c = 0;
   int isOutput = true;
   for (r = 0; r < size; r++)   {
      //cout << "CHECKING VERTEX: " << r << endl;
      isOutput = true;
      for (c = 0; c < size; c++) {
         //cout << graph[r][c] << endl;
         if (graph[r][c] == 1)   {
            //cout << "VERTEX " << r << " IS NOT AN OUTPUT" << endl << endl;
            isOutput = false;
            break;
         }
      }
      if (!isOutput)  {
         continue;
      }
      //cout << "VERTEX " << r << " IS AN OUTPUT. ADDING TO LIST..." << endl << endl;
      outputVertex.push_back(r);
   }
}
void PPG::printOutputList()   { 
   if (outputVertex.size() == 0)  {
      cout << "output list is empty..." << endl << endl;
      return;
   }
   cout << "OUTPUT VERTEX: ";
   for (list<int>::iterator it = this->outputVertex.begin(); it != this->outputVertex.end(); it++) { 
      cout << *it << " ";
   }
   cout << endl << endl;
}
int PPG::numOfInput()  {return inputVertex.size();}
int PPG::numOfOutput() {return outputVertex.size();}

#endif
